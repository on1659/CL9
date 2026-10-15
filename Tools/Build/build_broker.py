"""빌드 브로커 — 세션이 빌드를 직접 협상하지 않고 이 데몬 하나에 요청만 한다.

정본: docs/goal/build-broker.md. 세션 쪽은 build_request.py, 사용자 쪽은 build_ctl.py 와 제어 페이지(http://127.0.0.1:8765/).

  py -3 Tools/Build/build_broker.py serve [--port 8765] [--coalesce 30] [--idle-min 15]

- 큐: 같은 종류(build|compile) 요청을 묶음 대기(기본 30초) 동안 모아 한 번에 빌드한다.
- 실행: cl9_lock 을 세션 ID `broker` 로 잡고 UBT 를 돌린다(옛 절차를 따르는 세션도 잠금을 본다).
- 오류: 오류 줄 → 파일 → 역할 주인·수정 중 여부를 묶어 요청 세션에 돌려준다. 누구 것인지는 받은 세션이 판단한다.
- 꺼짐: 큐가 비고 idle-min 분 동안 요청·제어가 없으면 스스로 종료한다. 첫 요청이 다시 띄운다.
이 프로세스는 LLM 을 부르지 않는다 — 토큰을 쓰지 않는다.
"""
import json
import os
import re
import socket
import subprocess
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlparse

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, ".claude", "hooks"))
sys.path.insert(0, HERE)
import cl9_lock as L  # noqa: E402
import cl9_roles as R  # noqa: E402
import compile_check as C  # noqa: E402

PORT = int(os.environ.get("CL9_BROKER_PORT", "8765"))
STATE_DIR = os.path.join(ROOT, ".claude", "runs", "build-broker")
BROKER_SESSION = "broker"
KINDS = ("build", "compile")
EDITOR_EXE = os.path.join(C.ENGINE, "Engine", "Binaries", "Win64", "UnrealEditor.exe")
MCP_PORT = 8000
TIMEOUT_MIN = {"build": 40, "compile": 20}
MAX_ERRORS = 40
WAIT_CAP_S = 590  # Bash 도구 timeout 600000 안에서 한 번에 기다릴 수 있는 최대

FILE_ERR = re.compile(
    r"^(?P<path>(?:[A-Za-z]:)?[^():]+?\.(?:cpp|h|hpp|inl|cs|c))\((?P<line>\d+)(?:,\d+)?\)\s*:\s*"
    r"(?:fatal\s+)?error\s*(?P<code>[A-Z]+\d+)?\s*:?\s*(?P<msg>.*)$", re.I)
CODE_RE = re.compile(r"error\s+([A-Z]+\d+)", re.I)


# ---------------------------------------------------------------- 오류 분석 (순수 함수)

def git_head(root):
    try:
        out = subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=root, stdout=subprocess.PIPE,
                             stderr=subprocess.DEVNULL, check=False).stdout.decode("utf-8", "replace").strip()
        return out or "?"
    except OSError:
        return "?"


def git_dirty(root):
    """`git status --porcelain -z` 의 경로 목록(이름 바꾸기는 새 경로)."""
    try:
        raw = subprocess.run(["git", "status", "--porcelain", "-z"], cwd=root, stdout=subprocess.PIPE,
                             stderr=subprocess.DEVNULL, check=False).stdout.decode("utf-8", "replace")
    except OSError:
        return []
    tokens = raw.split("\0")
    out, i = [], 0
    while i < len(tokens):
        tok = tokens[i]
        i += 1
        if len(tok) < 4:
            continue
        out.append(tok[3:])
        if tok[0] in "RC":
            i += 1  # 원래 경로 토큰
    return out


def load_registry(root):
    try:
        return R.load(root)
    except R.RegistryError:
        return None


def analyze(lines, root, reg, dirty, last_ok):
    """오류 줄 → 항목. dirty 는 소문자 상대 경로 집합, last_ok 는 마지막 성공 빌드 시각(없으면 None)."""
    entries, seen = [], set()
    for raw in lines:
        s = raw.strip()
        if not C.ERROR_RE.search(s):
            continue
        m = FILE_ERR.match(s)
        if m:
            path, line, code, msg = m.group("path"), int(m.group("line")), m.group("code") or "", m.group("msg")
        else:
            cm = CODE_RE.search(s)
            path, line, code, msg = None, 0, cm.group(1) if cm else "", s
        rel = R.to_rel(path, root) if path else None
        key = (rel or path, line, code, msg[:60])
        if key in seen:
            continue
        seen.add(key)
        owners = R.owners_of(reg, rel) if (reg and rel) else {"owns": [], "shared": []}
        newer = False
        if rel and last_ok:
            try:
                newer = os.path.getmtime(os.path.join(root, rel)) > last_ok
            except OSError:
                pass
        entries.append({
            "file": os.path.relpath(path, root).replace("\\", "/") if (rel and path) else (path or ""),
            "line": line, "code": code, "msg": msg[:200],
            "owns": owners["owns"], "shared": owners["shared"],
            "dirty": bool(rel and rel in dirty), "newer_than_last_ok": newer,
        })
        if len(entries) >= MAX_ERRORS:
            break
    return entries


def signature(entries):
    return tuple(sorted({(e["file"], e["code"]) for e in entries}))


# ---------------------------------------------------------------- 브로커 (큐·스케줄러)

class Broker:
    def __init__(self, root, runner, clock=time.time, coalesce_s=30, idle_exit_s=900, inline=False, editor_starter=None):
        self.root, self.runner, self.clock = root, runner, clock
        self.coalesce_s, self.idle_exit_s, self.inline = coalesce_s, idle_exit_s, inline
        self.editor_starter = editor_starter
        self.cv = threading.Condition()
        self.tickets, self.next_id, self.batch_seq = {}, 1, 0
        self.held, self.start_at, self.force = False, None, False
        self.running, self.blocked, self.stopping = None, "", False
        self.last_activity = clock()
        self.last_ok = None
        self.fails = {}  # 종류 → [오류 서명, 연속 횟수] — compile 성공이 build 의 연속 실패를 지우지 않는다
        self.recent = []  # 최근 묶음 결과 (제어 페이지용)
        self.history_path = os.path.join(root, ".claude", "runs", "build-broker", "builds.jsonl")

    # ---- 세션 쪽
    def submit(self, session, kind, reason):
        if kind not in KINDS:
            raise ValueError("kind 는 %s 중 하나" % "|".join(KINDS))
        with self.cv:
            self.last_activity = self.clock()
            for t in self.tickets.values():  # 같은 세션·종류가 이미 줄 서 있으면 새로 안 만든다
                if t["state"] == "queued" and t["session"] == session and t["kind"] == kind:
                    t["reason"] = reason or t["reason"]
                    return self._view(t)
            t = {"id": self.next_id, "session": session, "kind": kind, "reason": reason or "", "state": "queued",
                 "t_req": self.clock(), "batch": None, "result": None}
            self.next_id += 1
            self.tickets[t["id"]] = t
            self.cv.notify_all()
            return self._view(t)

    def wait(self, tid, seconds):
        deadline = self.clock() + min(max(seconds, 0), WAIT_CAP_S)
        with self.cv:
            t = self.tickets.get(tid)
            if not t:
                return None
            while t["state"] in ("queued", "running") and not self.stopping:
                left = deadline - self.clock()
                if left <= 0:
                    break
                self.cv.wait(min(left, 5))
            return self._view(t)

    def _view(self, t):
        v = {k: t[k] for k in ("id", "session", "kind", "reason", "state", "batch")}
        v["age_s"] = int(self.clock() - t["t_req"])
        if t["state"] == "queued":
            v["ahead"] = sum(1 for o in self.tickets.values() if o["state"] == "queued" and o["id"] < t["id"]) + (1 if self.running else 0)
            v["blocked"] = self.blocked
            v["held"] = self.held
            v["start_at"] = self.start_at
        if t["result"]:
            v["result"] = t["result"]
        return v

    # ---- 사용자 쪽
    def ctl(self, action, **kw):
        with self.cv:
            self.last_activity = self.clock()
            queued = [t for t in self.tickets.values() if t["state"] == "queued"]
            if action == "hold":
                self.held = True
            elif action == "resume":
                self.held = False
            elif action == "now":
                if not queued:
                    return {"ok": False, "error": "큐가 비었다"}
                self.force, self.held = True, False
            elif action == "start_in":
                self.start_at = self.clock() + max(float(kw.get("minutes", 0)), 0) * 60
            elif action == "clear_start":
                self.start_at = None
            elif action == "coalesce":
                self.coalesce_s = max(float(kw.get("seconds", 0)), 0)
            elif action == "cancel":
                t = self.tickets.get(int(kw.get("ticket", 0)))
                if not t or t["state"] != "queued":
                    return {"ok": False, "error": "줄 서 있는 티켓이 아니다"}
                t["state"] = "cancelled"
            elif action == "stop":
                if self.running:
                    return {"ok": False, "error": "빌드 중이라 끌 수 없다"}
                self.stopping = True
            elif action == "start_editor":
                if not self.editor_starter:
                    return {"ok": False, "error": "에디터 기동기가 없다"}
                return {"ok": True, "message": self.editor_starter()}
            else:
                return {"ok": False, "error": "모르는 동작: %s" % action}
            self.cv.notify_all()
            return {"ok": True}

    def _streak(self):
        return max((v[1] for v in self.fails.values()), default=0)

    def status(self):
        with self.cv:
            queue = [self._view(t) for t in sorted(self.tickets.values(), key=lambda t: t["id"]) if t["state"] in ("queued", "running")]
            return {"held": self.held, "start_at": self.start_at, "coalesce_s": self.coalesce_s, "force": self.force,
                    "running": self.running, "blocked": self.blocked, "stopping": self.stopping,
                    "needs_user": self._streak() >= 2, "fail_streak": self._streak(),
                    "idle_exit_s": self.idle_exit_s, "queue": queue, "recent": self.recent[-10:],
                    "now": self.clock()}

    # ---- 스케줄러
    def tick(self):
        with self.cv:
            if self.running or self.stopping:
                return
            now = self.clock()
            queued = sorted((t for t in self.tickets.values() if t["state"] == "queued"), key=lambda t: t["id"])
            if not queued:
                self.blocked = ""
                if now - self.last_activity > self.idle_exit_s:
                    self.stopping = True
                    self.cv.notify_all()
                return
            if not self.force:
                if self.held:
                    self.blocked = "보류 중 — 사용자가 재개해야 시작한다"
                    return
                if self.start_at and now < self.start_at:
                    self.blocked = "지정 시각까지 %d초 대기" % (self.start_at - now)
                    return
                if now - queued[0]["t_req"] < self.coalesce_s:
                    self.blocked = "묶음 대기 %d초" % (self.coalesce_s - (now - queued[0]["t_req"]))
                    return
            kind = queued[0]["kind"]
            batch = [t for t in queued if t["kind"] == kind]
            self.batch_seq += 1
            bid, force = self.batch_seq, self.force
            self.blocked = ""
            for t in batch:
                t["state"], t["batch"] = "running", bid
            self.running = {"id": bid, "kind": kind, "tickets": [t["id"] for t in batch], "since": now}
        if self.inline:
            self._run(kind, [t["id"] for t in batch], bid, force)
        else:
            threading.Thread(target=self._run, args=(kind, [t["id"] for t in batch], bid, force), daemon=True).start()

    def _run(self, kind, ids, bid, force):
        started = self.clock()
        try:
            res = self.runner(kind, force)
        except Exception as ex:  # noqa: BLE001 — 실행기 버그가 데몬과 대기 중인 세션을 같이 죽이지 않게
            res = {"ok": False, "rc": -2, "elapsed": 0, "log": "", "lines": ["runner exception: %r" % ex]}
        if res.get("blocked"):
            with self.cv:
                for tid in ids:
                    self.tickets[tid]["state"], self.tickets[tid]["batch"] = "queued", None
                self.running, self.blocked = None, res["blocked"]
                self.cv.notify_all()
            return
        ok = bool(res.get("ok"))
        dirty = git_dirty(self.root)
        errors = [] if ok else analyze(res.get("lines", []), self.root, load_registry(self.root),
                                       {p.replace("\\", "/").lower() for p in dirty}, self.last_ok)
        with self.cv:
            sig = signature(errors) or (("rc", res.get("rc")),)
            if ok:
                self.fails.pop(kind, None)
                self.last_ok = self.clock()
            else:
                prev = self.fails.get(kind)
                self.fails[kind] = [sig, prev[1] + 1 if prev and prev[0] == sig else 1]
            streak = self.fails.get(kind, [None, 0])[1]
            requesters = [{"ticket": i, "session": self.tickets[i]["session"], "reason": self.tickets[i]["reason"]} for i in ids]
            result = {"batch": bid, "kind": kind, "ok": ok, "rc": res.get("rc"), "elapsed": int(res.get("elapsed", self.clock() - started)),
                      "log": res.get("log", ""), "errors": errors, "needs_user": streak >= 2, "forced": force,
                      "note": res.get("note", ""), "editor": res.get("editor", ""), "requesters": requesters}
            if not ok and not errors:
                result["tail"] = [l.strip()[:200] for l in res.get("lines", [])[-15:]]
            for tid in ids:
                self.tickets[tid]["state"], self.tickets[tid]["result"] = "done", result
            self.running, self.blocked, self.last_activity = None, "", self.clock()
            if self.start_at and self.start_at <= self.clock():
                self.start_at = None
            if force:
                self.force = False
            self.recent.append({k: result[k] for k in ("batch", "kind", "ok", "elapsed", "needs_user", "note")}
                               | {"at": self.clock(), "n": len(ids), "errors": len(errors)})
            self._append_history(result, dirty)
            self.cv.notify_all()

    def _append_history(self, result, dirty):
        rec = {"at": time.strftime("%Y-%m-%d %H:%M:%S"), "head": git_head(self.root), "dirty": dirty[:200],
               "batch": result["batch"], "kind": result["kind"], "ok": result["ok"], "elapsed": result["elapsed"],
               "forced": result["forced"], "requesters": result["requesters"], "errors": len(result["errors"]),
               "log": result["log"]}
        try:
            os.makedirs(os.path.dirname(self.history_path), exist_ok=True)
            with open(self.history_path, "a", encoding="utf-8") as f:
                f.write(json.dumps(rec, ensure_ascii=False) + "\n")
        except OSError:
            pass  # 기록 실패가 빌드 결과를 막지 않는다


# ---------------------------------------------------------------- 실제 실행기 (UBT)

def editor_running():
    return [p for img, p in L.tasklist_pids(("UnrealEditor.exe",))]


def mcp_up():
    try:
        socket.create_connection(("127.0.0.1", MCP_PORT), timeout=1).close()
        return True
    except OSError:
        return False


def start_editor():
    if editor_running():
        return "에디터가 이미 떠 있다"
    subprocess.Popen([EDITOR_EXE, C.PROJECT], cwd=ROOT, close_fds=True,
                     creationflags=subprocess.DETACHED_PROCESS | subprocess.CREATE_NEW_PROCESS_GROUP)
    return "에디터를 띄웠다"


def ubt_runner(kind, force):
    """build = CL9Editor(에디터를 끄고 다시 켠다), compile = CL9 게임 타깃(에디터 유지). {blocked} 또는 결과 dict."""
    sid = BROKER_SESSION
    if force:  # 남의 에디터 임대만 비운다 — PIE·빌드 같은 남의 배타 잠금은 force 여도 뺏지 않는다
        try:
            ex, _ = L.holder(ROOT)
        except L.LockError:
            ex = None
        if ex and ex.get("session_id") != sid:
            return {"blocked": "배타 잠금 보유: " + L.holder_line(ex)}
        try:
            L.release(ROOT, sid, force=True)
        except L.LockError:
            pass
    try:
        L.acquire(ROOT, sid, "compile")
    except L.LockError as ex:
        return {"blocked": str(ex)[:300]}
    note, editors, proc = "", [], None
    try:
        os.makedirs(os.path.join(STATE_DIR, "logs"), exist_ok=True)
        log_path = os.path.join(STATE_DIR, "logs", time.strftime("%s-%%Y%%m%%d-%%H%%M%%S.log" % kind))
        mb = C.free_mb()
        if mb < 3000:
            note = "물리 메모리 여유 %d MB — cl.exe 가 멈출 수 있다" % mb
        if kind == "build":
            editors = editor_running()
            if editors:
                subprocess.run(["taskkill", "/IM", "UnrealEditor.exe", "/F"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                time.sleep(5)  # Live Coding 뮤텍스
        target = "CL9Editor" if kind == "build" else "CL9"
        t0, rc = time.time(), None
        with open(log_path, "w", encoding="utf-8", errors="replace") as log:
            proc = subprocess.Popen(["cmd.exe", "/c", C.BUILD_BAT, target, "Win64", "Development", "-Project=%s" % C.PROJECT, "-WaitMutex"],
                                    stdout=log, stderr=subprocess.STDOUT, cwd=ROOT, creationflags=subprocess.CREATE_NO_WINDOW)
            try:
                L.acquire(ROOT, sid, kind, pid=proc.pid)
            except L.LockError:
                pass
            deadline = t0 + TIMEOUT_MIN[kind] * 60
            while proc.poll() is None:
                if time.time() > deadline:
                    C.kill_tree(proc.pid)
                    rc = -1
                    break
                time.sleep(2)
            if rc is None:
                rc = proc.returncode
        with open(log_path, "r", encoding="utf-8", errors="replace") as f:
            lines = f.read().splitlines()
        res = {"ok": rc == 0, "rc": rc, "elapsed": int(time.time() - t0), "log": os.path.relpath(log_path, ROOT), "lines": lines,
               "note": note + (" · %d분 안에 안 끝나 빌드 체인을 죽였다" % TIMEOUT_MIN[kind] if rc == -1 else "")}
        if kind == "build" and editors:
            if rc == 0:
                start_editor()
                for _ in range(150):  # 포트 8000 이 열릴 때까지 최대 5분
                    if mcp_up():
                        break
                    time.sleep(2)
                res["editor"] = "다시 떴다 · MCP 8000 %s" % ("준비됨" if mcp_up() else "아직 안 열림")
            else:
                res["editor"] = "꺼진 채다 — 고쳐서 다시 요청하거나 제어 페이지의 '에디터 켜기'"
        return res
    finally:
        try:
            L.release(ROOT, sid)
        except L.LockError:
            pass


# ---------------------------------------------------------------- HTTP

PAGE = """<!doctype html><html lang="ko"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>CL9 빌드 브로커</title><style>
:root{--bg:#fff;--fg:#1c1c1c;--mut:#6b6b6b;--line:#ddd;--ok:#1a7f37;--bad:#c62828;--warn:#9a6700;--btn:#f2f2f2}
@media(prefers-color-scheme:dark){:root{--bg:#161616;--fg:#e8e8e8;--mut:#9a9a9a;--line:#333;--ok:#4cc26a;--bad:#ff6b6b;--warn:#e3b341;--btn:#262626}}
body{background:var(--bg);color:var(--fg);font:14px/1.5 system-ui,sans-serif;margin:0;padding:16px;max-width:900px}
h1{font-size:18px;margin:0 0 4px}h2{font-size:14px;margin:20px 0 6px;color:var(--mut)}
.row{display:flex;flex-wrap:wrap;gap:8px;align-items:center;margin:8px 0}
button{background:var(--btn);color:var(--fg);border:1px solid var(--line);border-radius:6px;padding:6px 10px;cursor:pointer;font:inherit}
input{background:var(--bg);color:var(--fg);border:1px solid var(--line);border-radius:6px;padding:5px 8px;width:70px;font:inherit}
table{border-collapse:collapse;width:100%}td,th{border-bottom:1px solid var(--line);padding:5px 6px;text-align:left;vertical-align:top}
.ok{color:var(--ok)}.bad{color:var(--bad)}.warn{color:var(--warn)}.mut{color:var(--mut)}
</style></head><body>
<h1>CL9 빌드 브로커</h1><div id="state" class="mut">연결 중…</div>
<div class="row">
 <button onclick="ctl('now')">지금 시작</button>
 <button id="hold" onclick="ctl(held?'resume':'hold')">보류</button>
 <span>시작을 <input id="min" type="number" value="10" min="0"> 분 뒤로</span><button onclick="ctl('start_in',{minutes:+min.value})">지정</button><button onclick="ctl('clear_start')">지정 해제</button>
 <span>묶음 대기 <input id="coal" type="number" value="30" min="0"> 초</span><button onclick="ctl('coalesce',{seconds:+coal.value})">적용</button>
 <button onclick="ctl('start_editor')">에디터 켜기</button>
</div>
<h2>대기열</h2><table id="queue"></table>
<h2>최근 빌드</h2><table id="recent"></table>
<script>
let held=false;
async function ctl(action,extra){const r=await fetch('/ctl',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action,...(extra||{})})});
 const j=await r.json();if(!j.ok&&j.error)alert(j.error);else if(j.message)alert(j.message);load()}
const fmt=t=>new Date(t*1000).toLocaleTimeString();
async function load(){try{const s=await (await fetch('/status')).json();held=s.held;
 hold.textContent=s.held?'재개':'보류';
 let h=[];if(s.running)h.push('<b>빌드 중</b> #'+s.running.id+' ('+s.running.kind+', '+s.running.tickets.length+'건)');
 else if(s.queue.length)h.push('<span class="warn">대기: '+(s.blocked||'곧 시작')+'</span>');else h.push('비어 있음 — '+Math.round(s.idle_exit_s/60)+'분 아무 일 없으면 꺼진다');
 if(s.start_at)h.push('시작 지정 '+fmt(s.start_at));if(s.needs_user)h.push('<b class="bad">같은 오류로 '+s.fail_streak+'회 연속 실패 — 사용자 판단 필요</b>');
 state.innerHTML=h.join(' · ');
 queue.innerHTML='<tr><th>#</th><th>세션</th><th>종류</th><th>사유</th><th>상태</th><th></th></tr>'+s.queue.map(t=>'<tr><td>'+t.id+'</td><td>'+t.session.slice(0,8)+'</td><td>'+t.kind+'</td><td>'+esc(t.reason)+'</td><td>'+t.state+(t.ahead!=null?' (앞 '+t.ahead+')':'')+'</td><td>'+(t.state=='queued'?'<button onclick="ctl(\\'cancel\\',{ticket:'+t.id+'})">취소</button>':'')+'</td></tr>').join('');
 recent.innerHTML='<tr><th>묶음</th><th>종류</th><th>결과</th><th>시간</th><th>요청</th><th>메모</th></tr>'+s.recent.slice().reverse().map(r=>'<tr><td>#'+r.batch+'</td><td>'+r.kind+'</td><td class="'+(r.ok?'ok':'bad')+'">'+(r.ok?'성공':'실패 (오류 '+r.errors+')')+'</td><td>'+r.elapsed+'초</td><td>'+r.n+'건</td><td class="mut">'+esc(r.note||'')+'</td></tr>').join('')
}catch(e){state.innerHTML='<span class="bad">브로커에 연결할 수 없다 — 꺼졌을 수 있다(다음 요청이 다시 띄운다)</span>'}}
const esc=x=>String(x).replace(/[&<>]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;'}[c]));
load();setInterval(load,2000);
</script></body></html>"""


class Handler(BaseHTTPRequestHandler):
    server_version = "CL9Broker"

    def log_message(self, *args):  # 접근 로그를 남기지 않는다
        pass

    def _host_ok(self):
        return self.headers.get("Host", "").split(":")[0] in ("127.0.0.1", "localhost")

    def _send(self, code, body, ctype="application/json; charset=utf-8"):
        data = body if isinstance(body, bytes) else json.dumps(body, ensure_ascii=False).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if not self._host_ok():
            return self._send(403, {"error": "Host 거부"})
        u = urlparse(self.path)
        b = self.server.broker
        if u.path == "/":
            return self._send(200, PAGE.encode("utf-8"), "text/html; charset=utf-8")
        if u.path == "/status":
            return self._send(200, b.status())
        m = re.fullmatch(r"/ticket/(\d+)", u.path)
        if m:
            wait = float(parse_qs(u.query).get("wait", ["0"])[0])
            v = b.wait(int(m.group(1)), wait)
            return self._send(200 if v else 404, v or {"error": "없는 티켓"})
        self._send(404, {"error": "없는 경로"})

    def do_POST(self):
        if not self._host_ok():
            return self._send(403, {"error": "Host 거부"})
        if "application/json" not in self.headers.get("Content-Type", ""):
            return self._send(415, {"error": "JSON 만 받는다"})
        try:
            body = json.loads(self.rfile.read(int(self.headers.get("Content-Length", "0"))) or b"{}")
        except ValueError:
            return self._send(400, {"error": "JSON 이 아니다"})
        b = self.server.broker
        path = urlparse(self.path).path
        if path == "/request":
            try:
                return self._send(200, b.submit(str(body.get("session", "")), body.get("kind", ""), str(body.get("reason", ""))))
            except ValueError as ex:
                return self._send(400, {"error": str(ex)})
        if path == "/ctl":
            action = body.pop("action", "")
            return self._send(200, b.ctl(action, **body))
        self._send(404, {"error": "없는 경로"})


def serve(port=PORT, coalesce_s=30, idle_min=15):
    os.makedirs(STATE_DIR, exist_ok=True)
    try:
        httpd = ThreadingHTTPServer(("127.0.0.1", port), Handler)
    except OSError:
        print("이미 떠 있다 (포트 %d)" % port)
        return 0
    broker = Broker(ROOT, ubt_runner, coalesce_s=coalesce_s, idle_exit_s=idle_min * 60, editor_starter=start_editor)
    httpd.broker = broker
    threading.Thread(target=httpd.serve_forever, daemon=True).start()
    print("빌드 브로커 시작: http://127.0.0.1:%d/" % port, flush=True)
    while not broker.stopping:
        broker.tick()
        time.sleep(1)
    httpd.shutdown()
    print("빌드 브로커 종료", flush=True)
    return 0


def main(argv=None):
    import argparse
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser(prog="build_broker.py")
    sub = ap.add_subparsers(dest="cmd", required=True)
    sv = sub.add_parser("serve")
    sv.add_argument("--port", type=int, default=PORT)
    sv.add_argument("--coalesce", type=float, default=30)
    sv.add_argument("--idle-min", type=float, default=15)
    args = ap.parse_args(argv)
    return serve(args.port, args.coalesce, args.idle_min)


if __name__ == "__main__":
    sys.exit(main())
