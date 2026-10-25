"""매일 새벽 패키징 — Windows 작업 스케줄러가 부른다(2026-10-22 사용자 "새벽 5시마다 빌드예약").

  py -3 Tools/Build/nightly_package.py              # 지금 한 번 패키징한다
  py -3 Tools/Build/nightly_package.py --register   # 작업 스케줄러에 매일 run_time 으로 등록(다시 부르면 덮어쓴다)
  py -3 Tools/Build/nightly_package.py --unregister # 등록을 지운다

- 조절 값은 Tools/Build/nightly_values.csv (시각·빌드 구성·잠금 대기 시간).
- 잠금(cl9_lock)을 세션 ID `nightly`·용도 build 로 잡고 RunUAT BuildCookRun 을 돌린다 — 다른 세션의 빌드·PIE 와 겹치지 않는다.
  잠금은 마지막 acquire 뒤 한 시간이 지나면 무효가 되므로 도는 동안 15분마다 다시 잡는다.
- 에디터 타깃은 빌드하지 않는다(-nocompileeditor) — 떠 있는 에디터의 DLL 을 건드리지 않는다. 쿠킹은 마지막으로 빌드된 에디터 DLL 로 한다.
- 결과물은 Packaged/Windows/CL9.exe(매번 덮어쓴다). upload 가 1 이면 압축해서 GitHub 릴리스(release_tag, 매일 덮어씀)에 올린다.
  로그는 Saved/Nightly/<날짜>.log, 마지막 결과는 Saved/Nightly/last.json.
"""
import argparse
import datetime
import json
import os
import re
import subprocess
import sys
import time
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(ROOT, ".claude", "hooks"))
import cl9_lock as L  # noqa: E402

ENGINE = r"C:\Program Files\Epic Games\UE_5.8"
RUNUAT = os.path.join(ENGINE, "Engine", "Build", "BatchFiles", "RunUAT.bat")
UPROJECT = os.path.join(ROOT, "CL9.uproject")
ARCHIVE = os.path.join(ROOT, "Packaged")
LOG_DIR = os.path.join(ROOT, "Saved", "Nightly")
VALUES = os.path.join(HERE, "nightly_values.csv")
TASK_NAME = "CL9_NightlyPackage"
SESSION = "nightly"
DEFAULTS = {"run_time": "05:00", "config": "Development", "lock_wait_minutes": "30", "upload": "1", "release_tag": "nightly"}
PART_BYTES = 1900 * 1024 ** 2  # GitHub 릴리스 파일 하나의 한도 2GB 보다 조금 작게
RENEW_S = 15 * 60
ERROR_RE = re.compile(r"Error:|ERROR:|: error ")  # 쿠커·UAT 의 Error 줄과 컴파일러 오류 줄


def values():
    out = dict(DEFAULTS)
    try:
        with open(VALUES, "r", encoding="utf-8-sig") as f:
            for line in f:
                cols = [c.strip() for c in line.split(",")]
                if len(cols) >= 2 and cols[0] in out:
                    out[cols[0]] = cols[1]
    except OSError:
        pass
    return out


def log(f, msg):
    line = "[%s] %s" % (datetime.datetime.now().strftime("%H:%M:%S"), msg)
    if sys.stdout:  # pythonw(작업 스케줄러)에는 콘솔이 없다
        print(line, flush=True)
    f.write(line + "\n")
    f.flush()


def write_last(result, started, log_path, detail="", errors=None):
    data = {"result": result, "started": started, "ended": datetime.datetime.now().isoformat(timespec="seconds"),
            "log": log_path, "detail": detail}
    if errors:
        data["errors"] = errors
    with open(os.path.join(LOG_DIR, "last.json"), "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=1)


def first_errors(log_path, offset, count=2):
    """이번 실행분 로그(offset 부터)에서 처음 나온 Error 줄 count 개 — 로그를 안 열어도 원인이 보이게"""
    found = []
    with open(log_path, "r", encoding="utf-8", errors="replace") as f:
        f.seek(offset)
        for line in f:
            if ERROR_RE.search(line):
                line = line.strip()[:300]
                if line not in found:
                    found.append(line)
                    if len(found) >= count:
                        break
    return found


def grab_lock(f, wait_minutes):
    """줄을 서서 잠금을 잡는다 — 잡으면 참, 기다리다 시간이 다 되면 거짓"""
    deadline = time.time() + wait_minutes * 60
    last_reason = ""
    while True:
        try:
            L.acquire(ROOT, SESSION, "build", minutes=90, pid=os.getpid())
            return True
        except L.LockError as ex:
            if str(ex) != last_reason:
                log(f, "잠금 대기: %s" % ex)
                last_reason = str(ex)
        if time.time() >= deadline:
            return False
        time.sleep(30)


def package(f, config):
    cmd = [RUNUAT, "BuildCookRun", "-project=" + UPROJECT, "-platform=Win64", "-clientconfig=" + config,
           "-build", "-cook", "-stage", "-pak", "-prereqs", "-archive", "-archivedirectory=" + ARCHIVE,
           "-nocompileeditor", "-unattended", "-utf8output", "-nop4"]
    log(f, "실행: " + " ".join(cmd))
    proc = subprocess.Popen(cmd, cwd=ROOT, stdout=f, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
                            creationflags=subprocess.CREATE_NO_WINDOW)
    next_renew = time.time() + RENEW_S
    while proc.poll() is None:
        time.sleep(10)
        if time.time() >= next_renew:
            try:
                L.acquire(ROOT, SESSION, "build", minutes=90, pid=os.getpid())
            except L.LockError as ex:
                log(f, "잠금 다시 잡기 실패(패키징은 계속한다): %s" % ex)
            next_renew = time.time() + RENEW_S
    return proc.returncode


def gh(f, *args):
    r = subprocess.run(["gh"] + list(args), cwd=ROOT, capture_output=True, text=True, encoding="utf-8",
                       errors="replace", creationflags=subprocess.CREATE_NO_WINDOW)
    if r.returncode != 0:
        log(f, "gh %s 실패: %s" % (args[0:2], r.stderr.strip()[:500]))
    return r


def make_zip(f):
    """Packaged/Windows 를 압축하고 GitHub 의 파일당 한도(2GB)보다 크면 조각으로 자른다 — 올릴 파일 목록"""
    src = os.path.join(ARCHIVE, "Windows")
    out_dir = os.path.join(ARCHIVE, "Upload")
    if os.path.isdir(out_dir):
        for name in os.listdir(out_dir):
            os.remove(os.path.join(out_dir, name))
    os.makedirs(out_dir, exist_ok=True)
    zip_path = os.path.join(out_dir, "CL9-Win64.zip")
    # pak 은 이미 압축돼 있어서 다시 압축하지 않는다(빠르다)
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_STORED, allowZip64=True) as z:
        for base, _, files in os.walk(src):
            for name in files:
                full = os.path.join(base, name)
                z.write(full, os.path.join("CL9", os.path.relpath(full, src)))
    size = os.path.getsize(zip_path)
    log(f, "압축 %.2f GB" % (size / 1024 ** 3))
    if size <= PART_BYTES:
        return [zip_path]
    parts = []
    with open(zip_path, "rb") as zf:
        i = 1
        while True:
            chunk = zf.read(PART_BYTES)
            if not chunk:
                break
            part = "%s.%03d" % (zip_path, i)
            with open(part, "wb") as p:
                p.write(chunk)
            parts.append(part)
            i += 1
    os.remove(zip_path)
    log(f, "2GB 를 넘어 %d조각으로 자름" % len(parts))
    return parts


def upload(f, tag):
    """GitHub 릴리스 하나(tag)를 매일 덮어쓴다 — 올린 릴리스 주소, 실패하면 빈 문자열"""
    files = make_zip(f)
    head = subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    dirty = subprocess.run(["git", "status", "--porcelain"], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    notes = "%s 새벽 패키징 · 커밋 %s%s\n\n압축을 풀고 CL9\\CL9.exe 를 실행한다. 런타임 오류가 나면 CL9\\Engine\\Extras\\Redist\\en-us\\UEPrereqSetup_x64.exe 를 한 번 실행한다." % (
        datetime.date.today().isoformat(), head, " (커밋 안 된 수정 포함)" if dirty else "")
    if len(files) > 1:
        notes += "\n\n조각 파일은 모두 받은 뒤 같은 폴더에서 `copy /b CL9-Win64.zip.001+CL9-Win64.zip.002 CL9-Win64.zip` 처럼 이어 붙여 푼다."
    if gh(f, "release", "view", tag, "--json", "tagName").returncode == 0:
        old = json.loads(gh(f, "release", "view", tag, "--json", "assets").stdout or "{}").get("assets", [])
        for a in old:
            gh(f, "release", "delete-asset", tag, a["name"], "-y")
        if gh(f, "release", "edit", tag, "--notes", notes, "--title", "CL9 nightly %s" % datetime.date.today().isoformat()).returncode != 0:
            return ""
    elif gh(f, "release", "create", tag, "--prerelease", "--title", "CL9 nightly %s" % datetime.date.today().isoformat(),
            "--notes", notes).returncode != 0:
        return ""
    for path in files:
        log(f, "올리는 중: %s" % os.path.basename(path))
        if gh(f, "release", "upload", tag, path, "--clobber").returncode != 0:
            return ""
    url = gh(f, "release", "view", tag, "--json", "url").stdout
    url = json.loads(url).get("url", "") if url else ""
    log(f, "GitHub 릴리스에 올림: %s" % url)
    return url


def run():
    os.makedirs(LOG_DIR, exist_ok=True)
    v = values()
    started = datetime.datetime.now().isoformat(timespec="seconds")
    log_path = os.path.join(LOG_DIR, datetime.date.today().isoformat() + ".log")
    with open(log_path, "a", encoding="utf-8") as f:
        offset = f.tell()
        log(f, "새벽 패키징 시작 — 구성 %s" % v["config"])
        if not grab_lock(f, float(v["lock_wait_minutes"])):
            log(f, "잠금을 %s분 안에 못 잡아 오늘은 건너뛴다" % v["lock_wait_minutes"])
            L.queue_leave(ROOT, SESSION)
            write_last("skipped", started, log_path, "잠금 대기 시간 초과")
            return 2
        t0 = time.time()
        try:
            code = package(f, v["config"])
        finally:
            L.release(ROOT, SESSION)
        minutes = (time.time() - t0) / 60
        if code == 0:
            log(f, "성공 — %.0f분, 결과물 %s" % (minutes, os.path.join(ARCHIVE, "Windows", "CL9.exe")))
            detail = "%.0f분" % minutes
            if v["upload"] == "1":
                url = upload(f, v["release_tag"])
                detail += ", 올림 %s" % url if url else ", GitHub 올리기 실패(로그 참고)"
            write_last("ok", started, log_path, detail)
            return 0
        log(f, "실패 — 종료 코드 %d, %.0f분. 위 로그에서 Error 줄을 본다" % (code, minutes))
        f.flush()
        write_last("failed", started, log_path, "종료 코드 %d" % code, first_errors(log_path, offset))
        return 1


def ps(script):
    return subprocess.run(["powershell", "-NoProfile", "-Command", script], capture_output=True, text=True)


def register():
    v = values()
    py = sys.executable.replace("python.exe", "pythonw.exe")  # 새벽에 콘솔 창을 띄우지 않는다
    if not os.path.exists(py):
        py = sys.executable
    script = (
        "$a = New-ScheduledTaskAction -Execute '{py}' -Argument '\"{me}\"' -WorkingDirectory '{root}';"
        "$t = New-ScheduledTaskTrigger -Daily -At '{at}';"
        "$s = New-ScheduledTaskSettingsSet -WakeToRun -StartWhenAvailable -ExecutionTimeLimit (New-TimeSpan -Hours 6);"
        "Register-ScheduledTask -TaskName '{name}' -Action $a -Trigger $t -Settings $s -Force | Out-Null;"
        "(Get-ScheduledTaskInfo -TaskName '{name}').NextRunTime"
    ).format(py=py, me=os.path.abspath(__file__), root=ROOT, at=v["run_time"], name=TASK_NAME)
    r = ps(script)
    if r.returncode != 0:
        print("등록 실패: " + r.stderr.strip())
        return 1
    print("등록함: 작업 '%s', 매일 %s — 다음 실행 %s" % (TASK_NAME, v["run_time"], r.stdout.strip()))
    return 0


def unregister():
    r = ps("Unregister-ScheduledTask -TaskName '%s' -Confirm:$false" % TASK_NAME)
    print("지움" if r.returncode == 0 else "지우기 실패: " + r.stderr.strip())
    return r.returncode


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--register", action="store_true")
    ap.add_argument("--unregister", action="store_true")
    a = ap.parse_args()
    if sys.stdout:  # 콘솔이 cp949 라도 한글·줄표를 찍는다
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.exit(register() if a.register else unregister() if a.unregister else run())
