"""컴파일 검사 — 에디터를 켠 채 게임 타깃(CL9 Win64 Development)을 빌드해 C++ 가 통과하는지 본다.

역할 에이전트(cl9-*)와 리드가 같이 쓴다. 정본: docs/goal/session-orchestration.md H절, docs/workflow/sessions.md#잠금.

  py -3 Tools/Build/compile_check.py [--timeout-min 20] [--no-lock]

- 게임 타깃은 에디터가 쓰는 UnrealEditor-CL9.dll 과 바이너리·중간물이 달라 에디터를 끄지 않아도 된다(2026-09-13 실측:
  첫 빌드 10분, 무변경 85초 — 메모리 여유가 있을 때). **문법 검사다**: 게임 타깃은 WITH_EDITOR=0 이라 에디터 전용 코드는 안 잡힌다.
- 잠금: `compile` 용도로 잡는다(세션 ID 는 CLAUDE_CODE_SESSION_ID, 없으면 hostname-pid). 남이 잡고 있으면 90초 기다렸다 포기한다.
  UBT 는 전역 뮤텍스가 있어 다른 빌드와 겹치면 어차피 기다린다.
- 빌드는 분리 프로세스가 아니라 이 스크립트가 직접 돌린다 — 도구 타임아웃(Bash 120초 기본)을 넘기니 **timeout 600000 으로 부른다**.
  --timeout-min 을 넘기면 빌드 체인(cmd → UBT → cl/link)을 죽이고 실패로 보고한다(이 머신은 메모리 부족 때 cl.exe 가 멈추는 일이 있다).
- 출력: 마지막에 `COMPILE OK <초>` 또는 `COMPILE FAIL <초>` + 오류 줄(최대 40줄). 종료 코드 0/1.
"""
import os
import re
import subprocess
import sys
import time

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ENGINE = r"D:\Program Files\Epic Games\UE_5.8"
BUILD_BAT = os.path.join(ENGINE, "Engine", "Build", "BatchFiles", "Build.bat")
PROJECT = os.path.join(ROOT, "CL9.uproject")
LOG_DIR = os.path.join(ROOT, ".claude", "runs", "compile")
sys.path.insert(0, os.path.join(ROOT, ".claude", "hooks"))
import cl9_lock as L  # noqa: E402

ERROR_RE = re.compile(r"(error C\d+|error LNK\d+|fatal error|: error|Result: Failed|Multiple input files)", re.I)


def session_id():
    return os.environ.get("CLAUDE_CODE_SESSION_ID") or "%s-%d" % (os.environ.get("COMPUTERNAME", "host"), os.getppid())


def kill_tree(pid):
    subprocess.run(["taskkill", "/PID", str(pid), "/T", "/F"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def free_mb():
    """물리 메모리 여유(MB) — GlobalMemoryStatusEx."""
    import ctypes
    import ctypes.wintypes as w

    class MS(ctypes.Structure):
        _fields_ = [("dwLength", w.DWORD), ("dwMemoryLoad", w.DWORD), ("ullTotalPhys", ctypes.c_ulonglong),
                    ("ullAvailPhys", ctypes.c_ulonglong), ("ullTotalPageFile", ctypes.c_ulonglong),
                    ("ullAvailPageFile", ctypes.c_ulonglong), ("ullTotalVirtual", ctypes.c_ulonglong),
                    ("ullAvailVirtual", ctypes.c_ulonglong), ("ullAvailExtendedVirtual", ctypes.c_ulonglong)]
    ms = MS()
    ms.dwLength = ctypes.sizeof(ms)
    ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(ms))
    return int(ms.ullAvailPhys / (1024 * 1024))


def main(argv=None):
    import argparse
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser(prog="compile_check.py")
    ap.add_argument("--timeout-min", type=int, default=20)
    ap.add_argument("--no-lock", action="store_true", help="잠금 없이(테스트용)")
    args = ap.parse_args(argv)

    if not os.path.isfile(BUILD_BAT):
        print("엔진을 못 찾음: %s" % BUILD_BAT)
        return 1
    os.makedirs(LOG_DIR, exist_ok=True)
    log_path = os.path.join(LOG_DIR, time.strftime("compile-%Y%m%d-%H%M%S.log"))
    sid = session_id()
    if not args.no_lock:
        try:
            L.acquire(ROOT, sid, "compile", wait=90)
        except L.LockError as ex:
            print("COMPILE BUSY — %s" % ex)
            return 1

    mb = free_mb()
    if mb < 3000:
        print("경고: 물리 메모리 여유 %d MB — 에디터(약 8 GB)와 겹치면 cl.exe 가 몇 분~수십 분 멈춘다(2026-09-13 실측: 한 파일 증분 20분). "
              "급하지 않으면 메모리를 비우고 다시." % mb)
    t0 = time.time()
    rc = None
    try:
        with open(log_path, "w", encoding="utf-8", errors="replace") as log:
            proc = subprocess.Popen(
                ["cmd.exe", "/c", BUILD_BAT, "CL9", "Win64", "Development", "-Project=%s" % PROJECT, "-WaitMutex"],
                stdout=log, stderr=subprocess.STDOUT, cwd=ROOT,
            )
            if not args.no_lock:
                try:
                    L.acquire(ROOT, sid, "compile", pid=proc.pid)  # 프로세스 생존으로 잠금 유지
                except L.LockError:
                    pass
            deadline = t0 + args.timeout_min * 60
            while proc.poll() is None:
                if time.time() > deadline:
                    kill_tree(proc.pid)
                    rc = -1
                    break
                time.sleep(2)
            if rc is None:
                rc = proc.returncode
    finally:
        if not args.no_lock:
            try:
                L.release(ROOT, sid)
            except L.LockError:
                pass
    elapsed = int(time.time() - t0)
    with open(log_path, "r", encoding="utf-8", errors="replace") as f:
        lines = f.read().splitlines()
    if rc == 0:
        print("COMPILE OK %ds (로그 %s)" % (elapsed, os.path.relpath(log_path, ROOT)))
        return 0
    if rc == -1:
        print("COMPILE FAIL %ds — %d분 안에 안 끝나 빌드 체인을 죽였다(이 머신은 메모리 부족 때 cl.exe 가 멈춘다). 로그 %s"
              % (elapsed, args.timeout_min, os.path.relpath(log_path, ROOT)))
        return 1
    errs = [l for l in lines if ERROR_RE.search(l)]
    print("COMPILE FAIL %ds (rc %s, 로그 %s)" % (elapsed, rc, os.path.relpath(log_path, ROOT)))
    for l in (errs or lines[-15:])[:40]:
        print("  " + l.strip()[:220])
    return 1


if __name__ == "__main__":
    sys.exit(main())
