"""실행(run) 관리 — `/cl9 task` 리드가 부르는 결정적 단계들. 정본: docs/goal/session-orchestration.md D절.

  py -3 .claude/hooks/cl9_run.py init <slug> --task "<설명>" --roles ui,gameplay [--grants <json>]   실행 폴더·run.json·grants.json (TRIAGE)
  py -3 .claude/hooks/cl9_run.py overlap --grants <json> [--exclude <slug>]        다른 살아 있는 실행·같은 실행 안 역할끼리 파일 겹침 검사 (rc 1)
  py -3 .claude/hooks/cl9_run.py dispatch <slug>                                    기준선 스냅샷 + DISPATCHED
  py -3 .claude/hooks/cl9_run.py state <slug> <STATE>                               상태 전이 (잘못된 전이는 거부)
  py -3 .claude/hooks/cl9_run.py validate-report <slug> <role>                      보고서 제목 7개(+balance 2개) 순서 검사 (rc 1)
  py -3 .claude/hooks/cl9_run.py audit <slug>                                        기준선 대비 델타 감사 + balance diff 모양 → audit.json (rc 1 위반)
  py -3 .claude/hooks/cl9_run.py finish <slug> --state REPORTED|ABORTED [--wall-min N] [--builds N] [--pie "…"] [--violations N]
  py -3 .claude/hooks/cl9_run.py show <slug>                                         run.json + 보고서·touch 상태
  py -3 .claude/hooks/cl9_run.py latest [--not-reported]                            가장 최근 실행 slug (resume 용)

grants.json: {"cl9-ui": ["Source/CL9/UI/CL9HUDWidget.cpp", …], "cl9-gameplay": [...]} — brief 의 파일 목록 그대로.
run.json(리드 전용): slug, task, state, roles, created, updated, flags{build,pie,questions,violations}, denials, wall_min, builds, pie_result, history[].
"""
import json
import os
import re
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cl9_audit as A  # noqa: E402
import cl9_lock as L  # noqa: E402
import cl9_roles as R  # noqa: E402

STATES = ["INTAKE", "TRIAGE", "DISPATCHED", "BLOCKED_ON_QUESTION", "AUDITED", "REVIEWED", "BUILDING", "PIE", "REPORTED", "ABORTED", "MANUAL"]
TERMINAL = ("REPORTED", "ABORTED")
TRANSITIONS = {
    "INTAKE": {"TRIAGE", "ABORTED"},
    "TRIAGE": {"DISPATCHED", "ABORTED"},
    "DISPATCHED": {"BLOCKED_ON_QUESTION", "AUDITED", "ABORTED"},
    "BLOCKED_ON_QUESTION": {"DISPATCHED", "ABORTED"},
    "AUDITED": {"REVIEWED", "DISPATCHED", "ABORTED"},
    "REVIEWED": {"BUILDING", "PIE", "REPORTED", "ABORTED"},
    "BUILDING": {"PIE", "REPORTED", "AUDITED", "ABORTED"},
    "PIE": {"REPORTED", "ABORTED"},
    "MANUAL": {"REPORTED", "ABORTED"},
}
REPORT_SECTIONS = ["한 일", "바꾼 파일", "다른 역할에 요청(인터페이스·표 행·에셋)", "빌드 필요 여부",
                   "PIE 확인 항목", "decisions.md에 남길 결정", "질문(막힌 것)"]
BALANCE_EXTRA = ["표 행", "검증(부등식)"]
SLUG_RE = re.compile(r"^[a-z0-9][a-z0-9\-]{1,40}$")


class RunError(Exception):
    pass


def run_dir(root, slug):
    return os.path.join(L.runs_dir(root), slug)


def load_run(root, slug):
    p = os.path.join(run_dir(root, slug), "run.json")
    try:
        with open(p, "r", encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        raise RunError("실행 없음: %s" % slug)
    except ValueError as ex:
        raise RunError("run.json 손상(%s): %s" % (slug, ex))


def save_run(root, run):
    run["updated"] = time.time()
    p = os.path.join(run_dir(root, run["slug"]), "run.json")
    tmp = p + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(run, f, ensure_ascii=False, indent=1)
    os.replace(tmp, p)


def load_grants_file(path):
    with open(path, "r", encoding="utf-8") as f:
        raw = json.load(f)
    if not isinstance(raw, dict):
        raise RunError("grants 는 {역할: [경로…]} 객체여야 한다")
    out = {}
    for role, lst in raw.items():
        name = role[4:] if role.startswith("cl9-") else role
        if not isinstance(lst, list) or not all(isinstance(x, str) for x in lst):
            raise RunError("grants 역할 '%s' 값이 문자열 목록이 아니다" % role)
        out[name] = [R.check_pattern(x) for x in lst]
    return out


def live_runs(root, exclude=None):
    d = L.runs_dir(root)
    out = []
    try:
        names = os.listdir(d)
    except FileNotFoundError:
        return out
    for name in names:
        if name == exclude:
            continue
        p = os.path.join(d, name, "run.json")
        if not os.path.isfile(p):
            continue
        try:
            with open(p, "r", encoding="utf-8") as f:
                run = json.load(f)
        except (OSError, ValueError):
            continue
        if str(run.get("state", "")).upper() in TERMINAL:
            continue
        gp = os.path.join(d, name, "grants.json")
        grants = {}
        if os.path.isfile(gp):
            try:
                grants = load_grants_file(gp)
            except (RunError, ValueError, R.RegistryError):
                grants = {}
        out.append({"slug": name, "state": run.get("state"), "roles": run.get("roles") or [], "grants": grants})
    return out


def overlaps(grants, root, exclude=None):
    """[(경로, '이 실행의 a·b' | '실행 <slug> 의 role')]."""
    problems = []
    seen = {}
    for role, lst in grants.items():
        for p in lst:
            if p in seen and seen[p] != role:
                problems.append((p, "이 실행 안에서 %s 와 %s 둘 다" % (seen[p], role)))
            seen[p] = role
    for other in live_runs(root, exclude):
        for orole, lst in other["grants"].items():
            for p in lst:
                for role, mine in grants.items():
                    if p in mine or any(R.match(p, g) or R.match(g, p) for g in mine):
                        problems.append((p, "실행 `%s` 의 %s 가 이미 쓰는 중" % (other["slug"], orole)))
    return problems


def cmd_init(root, args):
    if not SLUG_RE.match(args.slug):
        raise RunError("slug 는 소문자·숫자·하이픈 2~41자: %s" % args.slug)
    d = run_dir(root, args.slug)
    if os.path.exists(os.path.join(d, "run.json")):
        raise RunError("이미 있는 실행: %s" % args.slug)
    reg = R.load(root)
    roles = []
    for r in args.roles.split(","):
        name = R.resolve_role(reg, r.strip())
        if name is None:
            raise RunError("없는 역할: %s (역할: %s)" % (r, ", ".join(R.role_names(reg))))
        roles.append(name)
    grants = load_grants_file(args.grants) if args.grants else {r: [] for r in roles}
    for r in grants:
        if r not in roles:
            raise RunError("grants 의 역할 '%s' 가 --roles 에 없다" % r)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, "grants.json"), "w", encoding="utf-8") as f:
        json.dump({"cl9-" + r: grants.get(r, []) for r in roles}, f, ensure_ascii=False, indent=1)
    run = {"slug": args.slug, "task": args.task, "state": "TRIAGE", "roles": roles, "created": time.time(),
           "flags": {}, "denials": 0, "history": [{"state": "TRIAGE", "at": time.time()}]}
    save_run(root, run)
    print("실행 생성: %s (역할 %s) → %s" % (args.slug, ", ".join(roles), os.path.relpath(d, root)))
    for r in roles:
        print("  brief 경로: %s" % os.path.relpath(os.path.join(d, "brief-%s.md" % r), root))
    return 0


def cmd_overlap(root, args):
    grants = load_grants_file(args.grants)
    problems = overlaps(grants, root, args.exclude)
    if not problems:
        print("겹침 없음")
        return 0
    for p, why in problems:
        print("겹침: `%s` — %s" % (p, why))
    return 1


def cmd_dispatch(root, args):
    run = load_run(root, args.slug)
    if run["state"] != "TRIAGE":
        raise RunError("DISPATCHED 로 갈 수 있는 상태는 TRIAGE 뿐 (지금 %s)" % run["state"])
    grants = load_grants_file(os.path.join(run_dir(root, args.slug), "grants.json"))
    problems = overlaps(grants, root, args.slug)
    if problems:
        for p, why in problems:
            print("겹침: `%s` — %s" % (p, why))
        raise RunError("겹침이 있어 dispatch 거부")
    base = A.snapshot(root, run_dir(root, args.slug))
    run["state"] = "DISPATCHED"
    run["dispatched"] = time.time()
    run["history"].append({"state": "DISPATCHED", "at": time.time()})
    save_run(root, run)
    print("DISPATCHED — 기준선: 더러운 경로 %d, HEAD %s, grant-gated 문서 사본 %d" % (len(base["porcelain"]), base["head"][:8], len(base["gated_files"])))
    return 0


def cmd_state(root, args):
    run = load_run(root, args.slug)
    new = args.state.upper()
    if new not in STATES:
        raise RunError("모르는 상태 %s (%s)" % (new, "|".join(STATES)))
    cur = run["state"]
    if cur in TERMINAL:
        raise RunError("%s 는 끝난 상태 — 새 실행을 만든다" % cur)
    if new not in TRANSITIONS.get(cur, set()):
        raise RunError("전이 거부: %s → %s (허용: %s)" % (cur, new, ", ".join(sorted(TRANSITIONS.get(cur, set())))))
    if new == "BUILDING" and not L.holds_exclusive(root, os.environ.get("CLAUDE_CODE_SESSION_ID", "")):
        raise RunError("BUILDING 은 이 세션이 배타 잠금을 잡고 있어야 한다 (`cl9_lock.py acquire build --pid …`)")
    run["state"] = new
    run["history"].append({"state": new, "at": time.time()})
    save_run(root, run)
    print("%s → %s" % (cur, new))
    return 0


def validate_report_text(text, role):
    sections = REPORT_SECTIONS + (BALANCE_EXTRA if role == "balance" else [])
    heads = [l.strip()[3:].strip() for l in text.splitlines() if l.startswith("## ")]
    missing = [s for s in sections if s not in heads]
    order_ok = [h for h in heads if h in sections] == [s for s in sections if s in heads]
    return missing, order_ok


def cmd_validate_report(root, args):
    p = os.path.join(run_dir(root, args.slug), "report-%s.md" % args.role)
    if not os.path.isfile(p):
        print("보고서 없음: %s" % os.path.relpath(p, root))
        return 1
    with open(p, "r", encoding="utf-8") as f:
        text = f.read()
    missing, order_ok = validate_report_text(text, args.role)
    if not missing and order_ok:
        print("보고서 OK: %s" % os.path.relpath(p, root))
        return 0
    if missing:
        print("빠진 제목: " + ", ".join("`## %s`" % m for m in missing))
    if not order_ok:
        print("제목 순서가 다르다 — 정해진 순서: " + " → ".join(REPORT_SECTIONS + (BALANCE_EXTRA if args.role == "balance" else [])))
    return 1


def cmd_audit(root, args):
    run = load_run(root, args.slug)
    d = run_dir(root, args.slug)
    reg = R.load(root)
    grants = load_grants_file(os.path.join(d, "grants.json"))
    res = A.audit(root, d, reg, run["roles"], grants)
    bal = A.balance_audit(root, d, res["changed"]) if "balance" in run["roles"] else {}
    out = {"changed": res["changed"], "claimed": res["claimed"], "violations": res["violations"],
           "guard_off": res["guard_off"], "balance": bal, "at": time.time()}
    with open(os.path.join(d, "audit.json"), "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=1)
    print("바뀐 경로 %d — 역할에 대응 %d, 위반 %d, 밸런스 모양 위반 %d" % (
        len(res["changed"]), len(res["claimed"]), len(res["violations"]), sum(len(v) for v in bal.values())))
    for p, role in sorted(res["claimed"].items()):
        print("  %-9s %s" % (role, p))
    for p, why in res["violations"]:
        print("  위반      %s — %s" % (p, why))
    for p, lst in bal.items():
        for ln, why in lst:
            print("  밸런스    %s:%d — %s" % (p, ln, why))
    bad = bool(res["violations"] or bal)
    run["flags"]["violations"] = len(res["violations"]) + sum(len(v) for v in bal.values())
    if run["state"] == "DISPATCHED":
        run["state"] = "AUDITED"
        run["history"].append({"state": "AUDITED", "at": time.time()})
    save_run(root, run)
    return 1 if bad else 0


def cmd_finish(root, args):
    run = load_run(root, args.slug)
    new = args.state.upper()
    if new not in TERMINAL:
        raise RunError("finish 는 REPORTED|ABORTED")
    run["state"] = new
    run["history"].append({"state": new, "at": time.time()})
    if args.wall_min is not None:
        run["wall_min"] = args.wall_min
    if args.builds is not None:
        run["builds"] = args.builds
    if args.pie is not None:
        run["pie_result"] = args.pie
        run["flags"]["pie"] = args.pie
    if args.violations is not None:
        run["flags"]["violations"] = args.violations
    denied = os.path.join(run_dir(root, args.slug), "denied.log")
    if os.path.isfile(denied):
        with open(denied, "r", encoding="utf-8", errors="replace") as f:
            run["denials"] = sum(1 for _ in f)
    save_run(root, run)
    print("%s → %s" % (args.slug, new))
    return 0


def cmd_show(root, args):
    run = load_run(root, args.slug)
    d = run_dir(root, args.slug)
    print(json.dumps({k: v for k, v in run.items() if k != "history"}, ensure_ascii=False, indent=1))
    for r in run["roles"]:
        rep = os.path.exists(os.path.join(d, "report-%s.md" % r))
        touch = os.path.join(d, "touch-%s" % r)
        ago = int((time.time() - os.path.getmtime(touch)) / 60) if os.path.exists(touch) else None
        print("  %-9s brief %s · 보고서 %s · 마지막 편집 %s" % (
            r, "있음" if os.path.exists(os.path.join(d, "brief-%s.md" % r)) else "없음", "있음" if rep else "없음",
            ("%d분 전" % ago) if ago is not None else "없음"))
    return 0


def cmd_latest(root, args):
    d = L.runs_dir(root)
    best = None
    try:
        names = os.listdir(d)
    except FileNotFoundError:
        names = []
    for name in names:
        p = os.path.join(d, name, "run.json")
        if not os.path.isfile(p):
            continue
        try:
            with open(p, "r", encoding="utf-8") as f:
                run = json.load(f)
        except (OSError, ValueError):
            continue
        if args.not_reported and str(run.get("state", "")).upper() in TERMINAL:
            continue
        t = run.get("updated") or os.path.getmtime(p)
        if best is None or t > best[0]:
            best = (t, name, run.get("state"))
    if best is None:
        print("없음")
        return 1
    print("%s %s" % (best[1], best[2]))
    return 0


def main(argv=None):
    import argparse
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser(prog="cl9_run.py", description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=os.getcwd())
    sub = ap.add_subparsers(dest="cmd")
    p = sub.add_parser("init"); p.add_argument("slug"); p.add_argument("--task", required=True); p.add_argument("--roles", required=True); p.add_argument("--grants")
    p = sub.add_parser("overlap"); p.add_argument("--grants", required=True); p.add_argument("--exclude")
    p = sub.add_parser("dispatch"); p.add_argument("slug")
    p = sub.add_parser("state"); p.add_argument("slug"); p.add_argument("state")
    p = sub.add_parser("validate-report"); p.add_argument("slug"); p.add_argument("role")
    p = sub.add_parser("audit"); p.add_argument("slug")
    p = sub.add_parser("finish"); p.add_argument("slug"); p.add_argument("--state", required=True); p.add_argument("--wall-min", type=int)
    p.add_argument("--builds", type=int); p.add_argument("--pie"); p.add_argument("--violations", type=int)
    p = sub.add_parser("show"); p.add_argument("slug")
    p = sub.add_parser("latest"); p.add_argument("--not-reported", action="store_true")
    args = ap.parse_args(argv)
    if not args.cmd:
        ap.print_help()
        return 2
    fn = {"init": cmd_init, "overlap": cmd_overlap, "dispatch": cmd_dispatch, "state": cmd_state,
          "validate-report": cmd_validate_report, "audit": cmd_audit, "finish": cmd_finish, "show": cmd_show, "latest": cmd_latest}[args.cmd]
    try:
        return fn(args.root, args)
    except (RunError, R.RegistryError, ValueError, OSError) as ex:
        print("오류: %s" % ex)
        return 1


if __name__ == "__main__":
    sys.exit(main())
