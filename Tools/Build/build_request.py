"""빌드 요청 — 세션은 빌드를 직접 협상하지 않고 브로커에 요청만 한다. 정본: docs/goal/build-broker.md.

  py -3 Tools/Build/build_request.py --kind build|compile --reason "무엇을 바꿨나" [--wait 540]
  py -3 Tools/Build/build_request.py --ticket 12 [--wait 540]      # 아직 줄 서 있을 때 이어서 기다린다

- build  = 에디터 타깃 정식 빌드 (에디터를 끄고 다시 켠다). **요청 전에 내 에셋을 저장한다** — 브로커는 대신 저장하지 않는다.
- compile = 게임 타깃 문법 검사 (에디터 유지).
- 브로커가 안 떠 있으면 이 스크립트가 띄운다. 기다리는 동안 출력이 없으니 **Bash timeout 600000 으로 부른다**.
- 종료 코드: 0 성공 · 1 실패(오류 줄·파일 주인이 출력된다) · 2 아직 줄 서 있음(--ticket 으로 이어서) · 3 취소됨 · 4 브로커 오류.
- 실패하면: 오류 파일이 내가 바꾼 것이면 고치고 같은 명령으로 다시 요청한다. 내 것이 아니면 출력의 주인 역할/요청 세션에게 알린다.
"""
import json
import os
import subprocess
import sys
import time
import urllib.error
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
STATE_DIR = os.path.join(ROOT, ".claude", "runs", "build-broker")
PORT = int(os.environ.get("CL9_BROKER_PORT", "8765"))
BASE = "http://127.0.0.1:%d" % PORT


def call(method, path, body=None, timeout=10):
    data = json.dumps(body).encode("utf-8") if body is not None else None
    req = urllib.request.Request(BASE + path, data=data, method=method, headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.loads(r.read().decode("utf-8"))


def broker_up():
    try:
        call("GET", "/status", timeout=2)
        return True
    except (urllib.error.URLError, OSError, ValueError):
        return False


def ensure_broker():
    if broker_up():
        return True
    os.makedirs(STATE_DIR, exist_ok=True)
    with open(os.path.join(STATE_DIR, "broker.log"), "a", encoding="utf-8") as log:
        subprocess.Popen([sys.executable, os.path.join(HERE, "build_broker.py"), "serve"], cwd=ROOT, stdout=log, stderr=log,
                         stdin=subprocess.DEVNULL, close_fds=True,
                         creationflags=subprocess.DETACHED_PROCESS | subprocess.CREATE_NEW_PROCESS_GROUP | subprocess.CREATE_NO_WINDOW)
    for _ in range(40):
        time.sleep(0.25)
        if broker_up():
            return True
    return False


def session_id():
    return os.environ.get("CLAUDE_CODE_SESSION_ID") or "%s-%d" % (os.environ.get("COMPUTERNAME", "host"), os.getppid())


def report(v, me):
    """티켓 조회 결과를 출력하고 종료 코드를 돌려준다."""
    if v["state"] == "cancelled":
        print("CANCELLED #%d — 사용자가 취소했다" % v["id"])
        return 3
    if v["state"] != "done":
        why = v.get("blocked") or ("진행 중 (묶음 #%s)" % v.get("batch") if v["state"] == "running" else "곧 시작")
        print("QUEUED #%d — 앞 %s건 · %s · 이어서 기다리기: py -3 Tools/Build/build_request.py --ticket %d" % (v["id"], v.get("ahead", 0), why, v["id"]))
        return 2
    r = v["result"]
    who = ", ".join("%s%s(%s)" % (q["session"][:8], "[나]" if q["session"] == me else "", q["reason"][:60]) for q in r["requesters"])
    head = "%s %s %ds — 묶음 #%d, 요청 %d건: %s" % (r["kind"].upper(), "OK" if r["ok"] else "FAIL", r["elapsed"], r["batch"], len(r["requesters"]), who)
    print(head)
    for key in ("editor", "note"):
        if r.get(key):
            print("  " + r[key])
    if r.get("log"):
        print("  로그 " + r["log"])
    if r["ok"]:
        return 0
    for e in r["errors"]:
        tags = []
        if e["owns"] or e["shared"]:
            tags.append("주인 " + "·".join(e["owns"] or ["-"]) + (" / 공유 " + "·".join(e["shared"]) if e["shared"] else ""))
        else:
            tags.append("주인 미상")
        if e["dirty"]:
            tags.append("수정 중")
        if e["newer_than_last_ok"]:
            tags.append("마지막 성공 이후 수정됨")
        loc = "%s(%d)" % (e["file"], e["line"]) if e["file"] else "(파일 없음)"
        print("  %s %s: %s  [%s]" % (loc, e["code"], e["msg"], " · ".join(tags)))
    for line in r.get("tail", []):
        print("  " + line)
    if r["needs_user"]:
        print("같은 오류로 연속 실패했다 — 사용자 판단을 기다린다(제어 페이지 http://127.0.0.1:%d/). 더 시도하지 말고 사용자에게 알린다." % PORT)
    else:
        print("내가 바꾼 파일의 오류면 고치고 다시: py -3 Tools/Build/build_request.py --kind %s --reason \"…\" · 내 것이 아니면 위 주인 역할/요청 세션에 알린다." % r["kind"])
    return 1


def wait_ticket(tid, seconds, me):
    deadline = time.time() + seconds
    while True:
        left = deadline - time.time()
        v = call("GET", "/ticket/%d?wait=%d" % (tid, max(min(left, 60), 0)), timeout=max(min(left, 60), 0) + 15)
        if v["state"] in ("done", "cancelled") or left <= 60:
            return report(v, me)


def main(argv=None):
    import argparse
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser(prog="build_request.py")
    ap.add_argument("--kind", choices=("build", "compile"), default="build")
    ap.add_argument("--reason", default="")
    ap.add_argument("--ticket", type=int)
    ap.add_argument("--wait", type=int, default=540, help="초. 0 이면 줄만 서고 돌아온다")
    args = ap.parse_args(argv)
    me = session_id()
    try:
        if not ensure_broker():
            print("브로커를 못 띄웠다 — %s 를 본다" % os.path.join(STATE_DIR, "broker.log"))
            return 4
        if args.ticket:
            tid = args.ticket
        else:
            if not args.reason.strip():
                print("--reason 이 필요하다 — 무엇을 바꿔서 빌드하는지 한 줄(기록에 남는다)")
                return 4
            v = call("POST", "/request", {"session": me, "kind": args.kind, "reason": args.reason.strip()})
            tid = v["id"]
            if args.kind == "build":
                print("(build 는 에디터를 끈다 — 내 에셋은 요청 전에 저장해 둬야 한다)")
        return wait_ticket(tid, args.wait, me)
    except (urllib.error.URLError, OSError, ValueError, KeyError) as ex:
        print("브로커 오류: %s" % ex)
        return 4


if __name__ == "__main__":
    sys.exit(main())
