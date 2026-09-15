"""`/cl9 status` 가 부르는 상태 출력 — 잠금·가드·훅 활성(nonce)·최근 48시간 실행 표.

  py -3 .claude/hooks/cl9_status.py [--hours 48] [--json]

동료 세션 목록(ListAgents)은 도구라서 스킬이 따로 부른다. 이 스크립트는 디스크만 읽는다.
run.json 필드(리드가 쓴다): slug, state, roles, created, updated, flags{build,pie,questions,violations}, denials, report_paths.
"""
import json
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cl9_lock as L  # noqa: E402

TERMINAL = ("ABORTED", "REPORTED")


def runs(root, hours):
    d = L.runs_dir(root)
    out = []
    try:
        names = os.listdir(d)
    except FileNotFoundError:
        return out
    cutoff = time.time() - hours * 3600
    for name in sorted(names):
        p = os.path.join(d, name, "run.json")
        if not os.path.isfile(p):
            continue
        mtime = os.path.getmtime(p)
        if mtime < cutoff:
            continue
        try:
            with open(p, "r", encoding="utf-8") as f:
                run = json.load(f)
        except (OSError, ValueError):
            run = {"state": "손상", "roles": []}
        entry = {"slug": name, "state": str(run.get("state", "?")), "roles": list(run.get("roles") or []),
                 "updated": mtime, "flags": run.get("flags") or {}, "denials": 0, "elapsed": {}, "reports": []}
        denied = os.path.join(d, name, "denied.log")
        if os.path.isfile(denied):
            with open(denied, "r", encoding="utf-8", errors="replace") as f:
                entry["denials"] = sum(1 for _ in f)
        for role in entry["roles"]:
            t = os.path.join(d, name, "touch-%s" % role)
            if os.path.isfile(t):
                entry["elapsed"][role] = int((time.time() - os.path.getmtime(t)) / 60)
            if os.path.isfile(os.path.join(d, name, "report-%s.md" % role)):
                entry["reports"].append(role)
        out.append(entry)
    return out


def text(root, hours):
    nonce = os.urandom(6).hex()
    hook_active = L.hook_seen_check(root, nonce)  # 이 프로세스는 훅 밖에서 돈다 — 스킬이 먼저 `cl9_lock.py status --nonce` 를 쳐서 표식을 남긴다
    lines = [L.status_text(root)]
    lines.append("실행(최근 %d시간):" % hours)
    rs = runs(root, hours)
    if not rs:
        lines.append("  없음")
    for r in rs:
        flags = ", ".join("%s=%s" % kv for kv in r["flags"].items()) or "-"
        elapsed = ", ".join("%s %d분 전" % kv for kv in r["elapsed"].items()) or "-"
        reports = ",".join(r["reports"]) or "-"
        lines.append("  %-24s %-10s 역할 %-28s 보고서 %-14s 거부 %d  플래그 %s  마지막 편집 %s" % (
            r["slug"], r["state"], ",".join(r["roles"]), reports, r["denials"], flags, elapsed))
    err = os.path.join(L.runs_dir(root), "guard-errors.log")
    if os.path.isfile(err):
        with open(err, "r", encoding="utf-8", errors="replace") as f:
            tail = f.read().splitlines()[-3:]
        lines.append("가드 오류 로그(마지막 3줄): " + " | ".join(tail))
    return "\n".join(lines)


def main(argv=None):
    import argparse
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser(prog="cl9_status.py")
    ap.add_argument("--root", default=os.getcwd())
    ap.add_argument("--hours", type=int, default=48)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args(argv)
    if args.json:
        print(json.dumps({"lock": L.status_json(args.root), "runs": runs(args.root, args.hours)}, ensure_ascii=True))
    else:
        print(text(args.root, args.hours))
    return 0


if __name__ == "__main__":
    sys.exit(main())
