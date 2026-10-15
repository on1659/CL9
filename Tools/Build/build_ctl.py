"""빌드 브로커 제어 — 사용자(와 리드)가 대기·시작 시각을 정한다. 같은 동작이 제어 페이지(http://127.0.0.1:8765/)에도 있다.

  py -3 Tools/Build/build_ctl.py status
  py -3 Tools/Build/build_ctl.py hold | resume          # 보류 / 재개
  py -3 Tools/Build/build_ctl.py now                    # 보류·시각·묶음 대기·남의 에디터 임대를 무시하고 지금 시작
  py -3 Tools/Build/build_ctl.py start-in 10             # 10분 뒤에 시작 (clear-start 로 해제)
  py -3 Tools/Build/build_ctl.py coalesce 60             # 묶음 대기 60초
  py -3 Tools/Build/build_ctl.py cancel 7                # 줄 서 있는 티켓 취소
  py -3 Tools/Build/build_ctl.py start-editor | stop
  py -3 Tools/Build/build_ctl.py history [N]             # 빌드 기록 (브로커가 꺼져 있어도 읽는다)
"""
import json
import os
import sys
import time
import urllib.error

from build_request import STATE_DIR, call, PORT


def show_status(s):
    print("%s · 묶음 대기 %ds · 자동 종료 %d분" % ("보류" if s["held"] else "보류 아님", s["coalesce_s"], s["idle_exit_s"] // 60))
    if s["start_at"]:
        print("시작 지정: %s" % time.strftime("%H:%M:%S", time.localtime(s["start_at"])))
    if s["running"]:
        print("빌드 중: 묶음 #%d (%s, 티켓 %s)" % (s["running"]["id"], s["running"]["kind"], s["running"]["tickets"]))
    if s["blocked"] and s["queue"]:
        print("대기 사유: " + s["blocked"])
    if s["needs_user"]:
        print("!! 같은 오류로 %d회 연속 실패 — 사용자 판단 필요" % s["fail_streak"])
    for t in s["queue"]:
        print("  #%d %s %s [%s] %s" % (t["id"], t["session"][:8], t["kind"], t["state"], t["reason"]))
    if not s["queue"]:
        print("대기열 비어 있음")
    print("제어 페이지: http://127.0.0.1:%d/" % PORT)


def history(n):
    path = os.path.join(STATE_DIR, "builds.jsonl")
    if not os.path.exists(path):
        print("기록 없음")
        return 0
    with open(path, "r", encoding="utf-8") as f:
        rows = [json.loads(l) for l in f if l.strip()][-n:]
    for r in rows:
        who = ", ".join("%s:%s" % (q["session"][:8], q["reason"][:40]) for q in r["requesters"])
        print("%s #%s %s %s %ds HEAD %s 수정 %d개 오류 %d · %s" % (r["at"], r["batch"], r["kind"], "OK" if r["ok"] else "FAIL",
                                                           r["elapsed"], r["head"], len(r["dirty"]), r["errors"], who))
    return 0


def main(argv=None):
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    argv = sys.argv[1:] if argv is None else argv
    if not argv:
        print(__doc__)
        return 1
    cmd, rest = argv[0], argv[1:]
    if cmd == "history":
        return history(int(rest[0]) if rest else 10)
    try:
        if cmd == "status":
            show_status(call("GET", "/status"))
            return 0
        body = {"hold": {"action": "hold"}, "resume": {"action": "resume"}, "now": {"action": "now"},
                "clear-start": {"action": "clear_start"}, "start-editor": {"action": "start_editor"}, "stop": {"action": "stop"},
                "start-in": {"action": "start_in", "minutes": rest[0] if rest else 0},
                "coalesce": {"action": "coalesce", "seconds": rest[0] if rest else 0},
                "cancel": {"action": "cancel", "ticket": rest[0] if rest else 0}}.get(cmd)
        if body is None:
            print(__doc__)
            return 1
        r = call("POST", "/ctl", body)
        print(r.get("message") or ("완료" if r.get("ok") else "거부: " + r.get("error", "")))
        return 0 if r.get("ok") else 1
    except (urllib.error.URLError, OSError):
        print("브로커가 꺼져 있다 — 다음 build_request.py 가 다시 띄운다")
        return 1


if __name__ == "__main__":
    sys.exit(main())
