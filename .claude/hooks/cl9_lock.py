"""에디터/빌드 락 — `.claude/runs/editor.lock`.

정본: docs/goal/session-orchestration.md G절. 사용법(docs/workflow/sessions.md 와 동일):

  py -3 .claude/hooks/cl9_lock.py acquire <build|compile|pie|commandlet|editor> [--minutes N] [--pid P] [--wait S]
  py -3 .claude/hooks/cl9_lock.py release [--force]
  py -3 .claude/hooks/cl9_lock.py status [--nonce X] [--json]
  py -3 .claude/hooks/cl9_lock.py guard off [--minutes N] | on

파일 모양(JSON): 배타 보유자 하나 + 공유 `editor` 임대 목록.
  {"exclusive": {"session_id", "purpose", "since", "expires", "proc": {"pid", "create_time", "image"}} | null,
   "leases": [{"session_id", "since", "expires"}]}

세션 ID는 훅이 stdin 에서 찍는다(--session 은 훅·테스트용). 훅이 비활성인 세션에서 --session 없이
acquire/release 를 치면 파일을 건드리지 않고 "훅 비활성"을 알린다.
"""
import ctypes
import ctypes.wintypes as w
import errno
import json
import os
import sys
import time
# argparse·subprocess 는 쓰는 자리에서 늦게 임포트한다 — 훅 지연 예산(100 ms) 때문

RUNS_REL = os.path.join(".claude", "runs")
LOCK_NAME = "editor.lock"
GUARD_OFF_NAME = "guard.off"
HOOK_SEEN_DIR = "hook-seen"
EXCLUSIVE = ("build", "compile", "pie", "commandlet")
PURPOSES = EXCLUSIVE + ("editor",)
PID_REQUIRED = ("build", "commandlet")
DEFAULT_MINUTES = {"build": 90, "commandlet": 90, "compile": 30, "pie": 30, "editor": 30}
GUARD_DEFAULT_MINUTES = 60
WAIT_CAP_S = 90
EDITOR_IMAGES = ("UnrealEditor.exe", "UnrealEditor-Cmd.exe")
DOC_ANCHOR = "docs/workflow/sessions.md#잠금"


def now():
    """테스트가 바꿔치기하는 시계."""
    return time.time()


def fmt(epoch):
    return time.strftime("%H:%M:%S", time.localtime(epoch))


# ---------------------------------------------------------------- 경로

def runs_dir(root):
    return os.path.join(root, RUNS_REL)


def lock_path(root):
    return os.path.join(runs_dir(root), LOCK_NAME)


def guard_off_path(root):
    return os.path.join(runs_dir(root), GUARD_OFF_NAME)


# ---------------------------------------------------------------- 프로세스 (Windows)

class LockError(Exception):
    """사용자가 고칠 수 있는 실패 — 메시지가 곧 안내문."""


def proc_info(pid):
    """(create_time, image) — 살아 있지 않으면 None. PID 재사용은 create_time 으로 걸러진다."""
    if not pid:
        return None
    k = ctypes.windll.kernel32
    PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
    h = k.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, int(pid))
    if not h:
        return None
    try:
        code = w.DWORD()
        if not k.GetExitCodeProcess(h, ctypes.byref(code)) or code.value != 259:  # STILL_ACTIVE
            return None
        ct, et, kt, ut = w.FILETIME(), w.FILETIME(), w.FILETIME(), w.FILETIME()
        if not k.GetProcessTimes(h, ctypes.byref(ct), ctypes.byref(et), ctypes.byref(kt), ctypes.byref(ut)):
            return None
        create = (ct.dwHighDateTime << 32) | ct.dwLowDateTime
        buf = ctypes.create_unicode_buffer(1024)
        size = w.DWORD(1024)
        image = ""
        if k.QueryFullProcessImageNameW(h, 0, buf, ctypes.byref(size)):
            image = os.path.basename(buf.value)
        return {"create_time": create, "image": image}
    finally:
        k.CloseHandle(h)


def proc_triple(pid):
    info = proc_info(pid)
    if info is None:
        return None
    return {"pid": int(pid), "create_time": info["create_time"], "image": info["image"]}


def proc_alive(triple):
    """기록된 (pid, create_time, image) 삼중항이 지금도 같은 프로세스인가."""
    if not triple or not triple.get("pid"):
        return False
    info = proc_info(triple["pid"])
    return bool(info) and info["create_time"] == triple.get("create_time") \
        and info["image"].lower() == str(triple.get("image", "")).lower()


def tasklist_pids(images=EDITOR_IMAGES):
    """`tasklist /FO CSV /NH` 로 해당 이미지의 PID 목록 — 로캘 무관하게 CSV 행만 센다."""
    import subprocess
    found = []
    for image in images:
        out = subprocess.run(
            ["tasklist", "/FI", "IMAGENAME eq %s" % image, "/FO", "CSV", "/NH"],
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=False,
        ).stdout.decode("utf-8", "replace")
        found.extend(parse_tasklist_csv(out, image))
    return found


def parse_tasklist_csv(text, image):
    """CSV 행("이미지","PID",…)만 골라 (image, pid) 로. '정보:' 같은 안내 줄은 따옴표로 시작하지 않는다."""
    pids = []
    for line in text.splitlines():
        line = line.strip()
        if not line.startswith('"'):
            continue
        cols = [c.strip('"') for c in line.split('","')]
        if len(cols) >= 2 and cols[0].lower() == image.lower() and cols[1].isdigit():
            pids.append((image, int(cols[1])))
    return pids


# ---------------------------------------------------------------- 파일 I/O (원자성)

def _read(path):
    """락 파일을 읽는다. 없으면 빈 상태, 깨졌으면 LockError."""
    try:
        with open(path, "r", encoding="utf-8") as f:
            raw = f.read()
    except FileNotFoundError:
        return {"exclusive": None, "leases": []}
    try:
        data = json.loads(raw) if raw.strip() else {}
    except ValueError as ex:
        raise LockError("락 파일이 깨짐(%s): %s — 내용을 확인하고 지운 뒤 다시. %s" % (path, ex, DOC_ANCHOR))
    if not isinstance(data, dict):
        raise LockError("락 파일 모양이 이상함(%s) — 지운 뒤 다시. %s" % (path, DOC_ANCHOR))
    data.setdefault("exclusive", None)
    data.setdefault("leases", [])
    if data["exclusive"] is not None and not isinstance(data["exclusive"], dict):
        raise LockError("락 파일 exclusive 가 객체가 아님 — 지운 뒤 다시. %s" % DOC_ANCHOR)
    if not isinstance(data["leases"], list):
        raise LockError("락 파일 leases 가 목록이 아님 — 지운 뒤 다시. %s" % DOC_ANCHOR)
    return data


def _write(path, data):
    """임시 파일 + os.replace. 읽는 쪽이 열어둔 순간 PermissionError 가 나면 잠깐 뒤 재시도."""
    tmp = "%s.%d.%s.tmp" % (path, os.getpid(), os.urandom(3).hex())
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=1)
    for attempt in range(20):
        try:
            os.replace(tmp, path)
            return
        except PermissionError:
            time.sleep(0.05 * (attempt + 1))
    os.unlink(tmp)
    raise LockError("락 파일 교체 실패(다른 프로세스가 붙잡고 있음): %s" % path)


class _Mutex:
    """락 파일 읽기-수정-쓰기 구간의 상호배제 — mkdir 은 원자적이다. 10초 넘은 고아 뮤텍스는 걷어낸다."""

    def __init__(self, path):
        self.dir = path + ".mutex"

    def __enter__(self):
        deadline = time.time() + 5.0
        while True:
            try:
                os.mkdir(self.dir)
                return self
            except FileExistsError:
                try:
                    if time.time() - os.stat(self.dir).st_mtime > 10:
                        os.rmdir(self.dir)
                        continue
                except OSError:
                    pass
                if time.time() > deadline:
                    raise LockError("락 뮤텍스가 안 풀림(%s) — 고아 폴더면 지운다" % self.dir)
                time.sleep(0.02 + os.urandom(1)[0] / 255 * 0.03)

    def __exit__(self, *_):
        try:
            os.rmdir(self.dir)
        except OSError:
            pass


# ---------------------------------------------------------------- 상태 판단

def _exclusive_valid(ex):
    """배타 보유가 아직 유효한가 — 만료 전이거나 기록된 프로세스가 살아 있으면 유효."""
    if not ex:
        return False
    if ex.get("expires", 0) > now():
        return True
    return proc_alive(ex.get("proc"))


def _live_leases(leases):
    return [l for l in leases if l.get("expires", 0) > now()]


def holder_line(ex):
    proc = ex.get("proc") or {}
    if proc.get("pid"):
        state = "살아 있음" if proc_alive(proc) else "죽음"
        proc_txt = " · PID %s %s %s" % (proc.get("pid"), proc.get("image", ""), state)
    else:
        proc_txt = ""
    return "%s — 세션 %s%s · %s부터 · 만료 %s" % (
        ex.get("purpose"), short(ex.get("session_id")), proc_txt, fmt(ex.get("since", 0)), fmt(ex.get("expires", 0)))


def short(session_id):
    s = str(session_id or "?")
    return s[:8]


def busy_reason(ex, leases_other):
    """거부 사유 한 줄 — 보유자·용도·만료·프로세스 상태·고치는 법."""
    parts = []
    if ex:
        parts.append("잠금 보유: " + holder_line(ex))
    if leases_other:
        parts.append("에디터 임대: " + ", ".join(
            "세션 %s(만료 %s)" % (short(l["session_id"]), fmt(l["expires"])) for l in leases_other))
    fix = ("끝났으면 상대가 `py -3 .claude/hooks/cl9_lock.py release`, 죽었으면 `release --force` — "
           "살아 있는 프로세스가 있으면 force 도 거부된다. " + DOC_ANCHOR)
    return " | ".join(parts) + " | " + fix


# ---------------------------------------------------------------- 명령

def acquire(root, session_id, purpose, minutes=None, pid=None, wait=0):
    """배타 잠금(또는 editor 임대)을 잡는다. 성공하면 상태 dict, 실패하면 LockError(사유)."""
    if purpose not in PURPOSES:
        raise LockError("용도는 %s 중 하나" % "|".join(PURPOSES))
    if not session_id:
        raise LockError("세션 ID 없음 — 훅이 비활성이면 세션을 재시작하고, 테스트면 --session 을 준다")
    minutes = minutes or DEFAULT_MINUTES[purpose]
    path = lock_path(root)
    os.makedirs(runs_dir(root), exist_ok=True)

    proc = None
    if purpose in PID_REQUIRED:
        if not pid:
            raise LockError("`acquire %s` 는 --pid 가 필수다(분리 실행한 .cmd 의 PID — Start-Process -PassThru). "
                            "만료보다 프로세스 생존이 잠금을 지킨다. %s" % (purpose, DOC_ANCHOR))
        proc = proc_triple(pid)
        if proc is None:
            raise LockError("PID %s 프로세스가 없다 — 먼저 띄우고 그 PID 를 준다" % pid)
    elif pid:
        proc = proc_triple(pid)
    if purpose == "commandlet":
        alive = tasklist_pids()
        if alive:
            raise LockError("에디터가 떠 있다(%s) — 커맨드릿은 에디터를 닫은 창에서만. %s" % (
                ", ".join("%s PID %d" % (img, p) for img, p in alive), DOC_ANCHOR))

    wait = min(max(int(wait or 0), 0), WAIT_CAP_S)
    deadline = time.time() + wait
    while True:
        try:
            return _try_acquire(path, session_id, purpose, minutes, proc)
        except LockError as ex:
            if time.time() >= deadline:
                if wait:
                    raise LockError("%d초 기다렸지만 여전히 busy — %s" % (wait, ex))
                raise
            time.sleep(1.0)


def _try_acquire(path, session_id, purpose, minutes, proc):
    fresh = {"exclusive": None, "leases": []}
    # 첫 생성은 O_CREAT|O_EXCL — 동시에 둘이 만들 수 없다
    try:
        fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        with os.fdopen(fd, "w", encoding="utf-8") as f:
            json.dump(fresh, f)
    except OSError as ex:
        if ex.errno != errno.EEXIST:
            raise
    with _Mutex(path):
        data = _read(path)
        data["leases"] = _live_leases(data["leases"])
        ex = data["exclusive"] if _exclusive_valid(data["exclusive"]) else None
        t = now()
        if purpose == "editor":
            data["leases"] = [l for l in data["leases"] if l.get("session_id") != session_id]
            data["leases"].append({"session_id": session_id, "since": t, "renewed": t, "expires": t + minutes * 60})
            data["exclusive"] = ex
            _write(path, data)
            return _read(path)
        if ex and ex.get("session_id") != session_id:
            raise LockError(busy_reason(ex, []))
        others = [l for l in data["leases"] if l.get("session_id") != session_id]
        if others:
            raise LockError(busy_reason(None, others))
        since = ex.get("since", t) if ex else t
        data["exclusive"] = {"session_id": session_id, "purpose": purpose, "since": since, "renewed": t,
                             "expires": t + minutes * 60, "proc": proc}
        _write(path, data)
        confirm = _read(path)
        if not confirm.get("exclusive") or confirm["exclusive"].get("session_id") != session_id:
            raise LockError("획득 확인 실패 — 다른 세션이 동시에 잡았다: %s" % busy_reason(confirm.get("exclusive"), []))
        return confirm


def release(root, session_id, force=False):
    """자기 배타 잠금·임대를 놓는다. --force 는 남의 것도 놓되 기록된 프로세스가 살아 있으면 거부."""
    path = lock_path(root)
    if not os.path.exists(path):
        return {"exclusive": None, "leases": []}
    with _Mutex(path):
        data = _read(path)
        ex = data.get("exclusive")
        if ex:
            mine = session_id and ex.get("session_id") == session_id
            if mine or not _exclusive_valid(ex):
                data["exclusive"] = None
            elif force:
                if proc_alive(ex.get("proc")):
                    raise LockError("force 거부 — 기록된 프로세스가 살아 있다: %s" % holder_line(ex))
                data["exclusive"] = None
            else:
                raise LockError("남의 잠금이다: %s | 상대가 release 하거나, 죽었으면 `release --force`. %s"
                                % (holder_line(ex), DOC_ANCHOR))
        leases = _live_leases(data.get("leases", []))
        if force:
            leases = []
        elif session_id:
            leases = [l for l in leases if l.get("session_id") != session_id]
        data["leases"] = leases
        _write(path, data)
        return _read(path)


def renew_lease(root, session_id, minutes=None):
    """훅이 mcp__unreal-mcp__call_tool 마다 부른다 — 에디터를 모는 세션을 자동 보호."""
    return acquire(root, session_id, "editor", minutes=minutes)


def holder(root):
    """(유효한 배타 보유자 dict 또는 None, 유효 임대 목록). 파일이 깨졌으면 LockError."""
    data = _read(lock_path(root))
    ex = data["exclusive"] if _exclusive_valid(data["exclusive"]) else None
    return ex, _live_leases(data["leases"])


def holds_exclusive(root, session_id):
    ex, _ = holder(root)
    return bool(ex) and ex.get("session_id") == session_id


# ---------------------------------------------------------------- 가드 스위치·훅 표식

def guard_off(root, minutes=None):
    os.makedirs(runs_dir(root), exist_ok=True)
    expires = now() + (minutes or GUARD_DEFAULT_MINUTES) * 60
    with open(guard_off_path(root), "w", encoding="utf-8") as f:
        f.write("%f\n" % expires)
    return expires


def guard_on(root):
    try:
        os.unlink(guard_off_path(root))
    except FileNotFoundError:
        pass


def guard_is_off(root):
    """꺼져 있으면 만료 epoch, 아니면 None. 만료된 파일은 지운다."""
    p = guard_off_path(root)
    try:
        with open(p, "r", encoding="utf-8") as f:
            expires = float(f.read().strip() or 0)
    except (FileNotFoundError, ValueError):
        return None
    if expires <= now():
        guard_on(root)
        return None
    return expires


def hook_seen_mark(root, nonce):
    d = os.path.join(runs_dir(root), HOOK_SEEN_DIR)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, nonce), "w") as f:
        f.write(str(now()))


LOCK_CMD_MARK = "last-lock-cmd.json"


def hook_lock_mark(root, cmd, purpose=None):
    """훅이 acquire/release 를 대신 처리한 뒤 남기는 표식 — CLI 가 '훅이 처리함'을 안다."""
    d = os.path.join(runs_dir(root), HOOK_SEEN_DIR)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, LOCK_CMD_MARK), "w", encoding="utf-8") as f:
        json.dump({"cmd": cmd, "purpose": purpose, "time": now()}, f)


def hook_lock_handled(root, cmd, purpose=None):
    """5초 안에 같은 명령으로 훅이 처리한 표식이 있으면 True(표식은 지운다)."""
    p = os.path.join(runs_dir(root), HOOK_SEEN_DIR, LOCK_CMD_MARK)
    try:
        with open(p, "r", encoding="utf-8") as f:
            mark = json.load(f)
        os.unlink(p)
    except (OSError, ValueError):
        return False
    return mark.get("cmd") == cmd and mark.get("purpose") == purpose and now() - float(mark.get("time", 0)) < 5


def hook_seen_check(root, nonce):
    p = os.path.join(runs_dir(root), HOOK_SEEN_DIR, nonce)
    seen = os.path.exists(p)
    if seen:
        try:
            os.unlink(p)
        except OSError:
            pass
    return seen


# ---------------------------------------------------------------- 출력

def status_text(root, nonce=None, hook_active=None):
    lines = []
    try:
        ex, leases = holder(root)
        lines.append("잠금: " + (holder_line(ex) if ex else "없음(free)"))
        lines.append("에디터 임대: " + (", ".join("세션 %s(만료 %s)" % (short(l["session_id"]), fmt(l["expires"]))
                                        for l in leases) if leases else "없음"))
    except LockError as ex:
        lines.append("잠금: 오류 — %s" % ex)
    off = guard_is_off(root)
    lines.append("가드: " + ("꺼짐(만료 %s) — `guard on` 으로 켠다" % fmt(off) if off else "켜짐"))
    if nonce is not None:
        if hook_active is None:
            hook_active = hook_seen_check(root, nonce)
        lines.append("훅: " + ("활성(이 세션이 잠금을 지키고 따른다)" if hook_active
                              else "비활성 — 이 세션은 잠금을 지키지도 따르지도 않는다. 세션을 재시작하라. " + DOC_ANCHOR))
    return "\n".join(lines)


def status_json(root):
    try:
        ex, leases = holder(root)
        err = None
    except LockError as e:
        ex, leases, err = None, [], str(e)
    return {"exclusive": ex, "leases": leases, "guard_off_until": guard_is_off(root), "error": err}


def main(argv=None):
    import argparse
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser(prog="cl9_lock.py", description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=os.getcwd(), help="저장소 루트(기본 cwd)")
    ap.add_argument("--session", default=None, help="세션 ID(훅·테스트용 — 보통 훅이 찍는다)")
    sub = ap.add_subparsers(dest="cmd")
    a = sub.add_parser("acquire")
    a.add_argument("purpose", choices=PURPOSES)
    a.add_argument("--minutes", type=int, default=None)
    a.add_argument("--pid", type=int, default=None)
    a.add_argument("--wait", type=int, default=0)
    r = sub.add_parser("release")
    r.add_argument("--force", action="store_true")
    s = sub.add_parser("status")
    s.add_argument("--nonce", default=None)
    s.add_argument("--json", action="store_true")
    g = sub.add_parser("guard")
    g.add_argument("state", choices=["off", "on"])
    g.add_argument("--minutes", type=int, default=None)
    args = ap.parse_args(argv)
    root = args.root

    try:
        if args.cmd == "acquire":
            if args.session:
                acquire(root, args.session, args.purpose, args.minutes, args.pid, args.wait)
                print("획득: %s (세션 %s)" % (args.purpose, short(args.session)))
            else:
                # 훅이 stdin 의 세션 ID 로 이미 획득하고 표식을 남겼어야 한다 — 없으면 훅 비활성
                if not hook_lock_handled(root, "acquire", args.purpose):
                    print("훅 비활성 — 이 세션에서는 잠금을 잡을 수 없다. 세션을 재시작하라(훅은 세션 시작 때 실린다). " + DOC_ANCHOR)
                    print(status_text(root))
                    return 1
                print("획득: %s (훅이 세션 ID 를 찍음)" % args.purpose)
            print(status_text(root))
            return 0
        if args.cmd == "release":
            if args.session or args.force:
                release(root, args.session, args.force)
                print("해제" + (" (force)" if args.force else ""))
            else:
                if not hook_lock_handled(root, "release"):
                    print("훅 비활성 — 이 세션에서는 잠금을 놓을 수 없다. 세션을 재시작하라(훅은 세션 시작 때 실린다). " + DOC_ANCHOR)
                    print(status_text(root))
                    return 1
                print("해제됨(훅이 세션 ID 를 찍음)")
            print(status_text(root))
            return 0
        if args.cmd == "status":
            if args.json:
                print(json.dumps(status_json(root), ensure_ascii=True))
            else:
                print(status_text(root, nonce=args.nonce))
            return 0
        if args.cmd == "guard":
            if args.state == "off":
                exp = guard_off(root, args.minutes)
                print("가드 꺼짐 — 만료 %s. 모든 세션의 훅이 전부 허용한다." % fmt(exp))
            else:
                guard_on(root)
                print("가드 켜짐")
            return 0
        ap.print_help()
        return 2
    except LockError as ex:
        print("거부: %s" % ex)
        return 1


if __name__ == "__main__":
    sys.exit(main())
