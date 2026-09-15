"""범위 가드 훅 (PreToolUse) — 역할 울타리(tripwire) + 에디터/빌드 락 극성.

정본: docs/goal/session-orchestration.md C·G절. `.claude/settings.json` 등록:
  "PreToolUse": [{"matcher": "Edit|Write|MultiEdit|NotebookEdit|Bash|PowerShell|mcp__unreal-mcp__call_tool|mcp__blender-mcp__.*",
                  "hooks": [{"type": "command", "command": "py -3 \"$CLAUDE_PROJECT_DIR/.claude/hooks/cl9_scope_guard.py\"", "timeout": 100}]}]
  훅은 세션 셸의 cwd 에서 돈다 — 상대 경로면 cwd 가 하위 폴더일 때 파이썬이 종료 코드 2(=차단)를 내 모든 셸이 막힌다(2026-09-13 cl9-2d 보고).
  훅 셸은 Git Bash 라 $CLAUDE_PROJECT_DIR(D:/Work/unreal/CL9) 이 풀린다.

stdin(JSON): session_id, agent_type, tool_name, tool_input, cwd. stdout: 문서화된 결정 JSON, exit 0.
- `cl9-<role>` 에이전트: Edit/Write 는 owns ∪ shared ∪ grants 안에서만, 셸은 읽기 전용 허용 목록만, MCP 는 전부 거부.
- 그 밖(리드·일반 서브에이전트): 경로 울타리 없음. 에디터/빌드 파괴 명령·StartPIE/StopPIE 는 배타 잠금 보유 세션만.
  `cl9_lock.py acquire|release` 는 훅이 stdin 세션 ID 로 직접 처리한다. call_tool 마다 editor 임대를 갱신한다.
- 내부 오류: 역할은 deny, 리드는 ask(사유 포함) + .claude/runs/guard-errors.log. 훅은 tripwire — 진실은 실행 후 감사.
"""
import json
import os
import re
import shlex
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cl9_lock as L  # noqa: E402
import cl9_roles as R  # noqa: E402

EDIT_TOOLS = ("Edit", "Write", "MultiEdit", "NotebookEdit")
SHELL_TOOLS = ("Bash", "PowerShell")
UNREAL_TOOL = "mcp__unreal-mcp__call_tool"
BLENDER_PREFIX = "mcp__blender-mcp__"
TERMINAL_STATES = ("ABORTED", "REPORTED")
DENY_ANCHOR = "docs/workflow/sessions.md#거부"
# 리드 내부 오류 극성(프로브 2026-09-13): `ask` 는 bypass 모드에서도 프롬프트를 띄워 세션을 세운다(16분 정지 실측),
# `additionalContext` 는 모델에게 system-reminder 로 보인다. 그래서 파괴 동사·PIE·잠금 명령은 deny(검사를 못 했으니
# 막는다), 그 밖은 allow + additionalContext(모델이 사유를 본다) + guard-errors.log.
RISKY_TOOL_NAMES = ("StartPIE", "StopPIE")
LOCK_SCRIPT_REL = ".claude/hooks/cl9_lock.py"
READ_ONLY_PROGRAMS = {"ls", "dir", "cat", "type", "head", "tail", "grep", "findstr", "find", "wc", "tasklist",
                      "get-childitem", "get-content", "select-string", "rg", "echo", "pwd"}
FIND_WRITE_FLAGS = {"-delete", "-exec", "-execdir", "-ok", "-okdir", "-fprint", "-fprint0", "-fprintf", "-fls"}
GIT_READ_VERBS = {"status", "diff", "log", "show", "ls-files", "grep", "blame", "rev-parse"}
COMPOUND = re.compile(r"[;&|`\n<>]|\$\(")
SHELL_DENY_TAIL = "에디터·빌드·커밋·그 밖의 셸은 리드가 한다 — 필요하면 보고서에 적어라. " + DENY_ANCHOR


# ---------------------------------------------------------------- 결정 JSON

def decide(decision, reason):
    return {"hookSpecificOutput": {"hookEventName": "PreToolUse", "permissionDecision": decision,
                                   "permissionDecisionReason": reason}}


def allow():
    return {}


def deny(reason):
    return decide("deny", reason)


def is_deny(out):
    return bool(out) and out.get("hookSpecificOutput", {}).get("permissionDecision") == "deny"


# ---------------------------------------------------------------- 유틸

def ts():
    return time.strftime("%Y-%m-%d %H:%M:%S")


def log_line(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "a", encoding="utf-8") as f:
        f.write("%s %s\n" % (ts(), text))


def find_root(cwd):
    """cwd 에서 위로 올라가며 레지스트리가 있는 저장소 루트를 찾는다(리드가 cd 했어도)."""
    d = os.path.abspath(cwd or os.getcwd())
    while True:
        if os.path.isfile(os.path.join(d, R.REGISTRY_REL)):
            return d
        parent = os.path.dirname(d)
        if parent == d:
            break
        d = parent
    env = os.environ.get("CLAUDE_PROJECT_DIR")
    if env and os.path.isfile(os.path.join(env, R.REGISTRY_REL)):
        return env
    raise R.RegistryError("레지스트리를 못 찾음(cwd=%s)" % cwd)


def prog_name(token):
    return re.sub(r"\.(exe|bat|cmd|ps1)$", "", os.path.basename(str(token)).lower())


def split_tokens(command):
    """토큰화 — 역슬래시는 Windows 경로로 보고 살린다(bash 이스케이프 의미는 여기서 필요 없다)."""
    try:
        return shlex.split(command.replace("\\", "\\\\"), posix=True)
    except ValueError:
        return command.split()


# ---------------------------------------------------------------- 실행(run) 상태

def find_run(root, role):
    """역할이 들어 있는 최신 run.json. (run dict | None, terminal_run | None)."""
    runs = os.path.join(root, ".claude", "runs")
    live, terminal = None, None
    try:
        names = os.listdir(runs)
    except FileNotFoundError:
        return None, None
    for name in names:
        p = os.path.join(runs, name, "run.json")
        try:
            with open(p, "r", encoding="utf-8") as f:
                run = json.load(f)
        except (OSError, ValueError):
            continue
        if not isinstance(run, dict) or role not in (run.get("roles") or []):
            continue
        entry = {"slug": name, "dir": os.path.join(runs, name), "state": str(run.get("state", "")).upper(),
                 "mtime": os.path.getmtime(p)}
        if entry["state"] in TERMINAL_STATES:
            if terminal is None or entry["mtime"] > terminal["mtime"]:
                terminal = entry
        elif live is None or entry["mtime"] > live["mtime"]:
            live = entry
    return live, terminal


def load_grants(root, run):
    """grants.json → {role: [정규화 패턴]}. 손상은 빈 것으로 치고 guard-errors.log 에 남긴다."""
    p = os.path.join(run["dir"], "grants.json")
    try:
        with open(p, "r", encoding="utf-8") as f:
            raw = json.load(f)
        if not isinstance(raw, dict):
            raise ValueError("객체가 아님")
        out = {}
        for role, lst in raw.items():
            if not isinstance(lst, list) or not all(isinstance(x, str) for x in lst):
                raise ValueError("역할 '%s' 항목이 문자열 목록이 아님" % role)
            name = role[4:] if role.startswith("cl9-") else role
            out[name] = [R.check_pattern(x) for x in lst]
        return out
    except FileNotFoundError:
        return {}
    except (OSError, ValueError, R.RegistryError) as ex:
        log_line(os.path.join(root, ".claude", "runs", "guard-errors.log"),
                 "grants.json 손상(%s): %s — 레지스트리만 적용" % (run["slug"], ex))
        return {}


# ---------------------------------------------------------------- 역할 (cl9-*)

def role_edit(reg, role, root, run, grants_all, path):
    grants = grants_all.get(role, [])
    rel = R.to_rel(path, root)
    if rel is None:
        return deny("`%s`는 저장소 밖 — 역할은 저장소 안 자기 영역만 편집한다. %s" % (path, DENY_ANCHOR))
    if rel == R.to_rel(os.path.join(run["dir"], "report-%s.md" % role), root):
        return allow()
    kind, _ = R.classify(reg, role, rel, grants)
    if kind == "owns":
        return allow()
    if kind in ("shared", "grant"):
        log_line(os.path.join(run["dir"], "shared-edits.log"), "%s %s (%s)" % (role, rel, kind))
        return allow()
    if kind == "gated":
        others = [r for r, lst in grants_all.items() if r != role and R.matches_any(rel, lst)]
        who = ("이번 실행에서는 %s 것" % "·".join(others)) if others else "이번 실행의 grants.json 에 없다"
        reason = "`%s`는 grant-gated 공유 경로 — %s. 보고서 '다른 역할에 요청'에 적어라. %s" % (rel, who, DENY_ANCHOR)
    elif kind == "append_only":
        reason = "`%s`는 append-only — 리드가 항목을 붙인다. 보고서 'decisions.md에 남길 결정'에 적어라. %s" % (rel, DENY_ANCHOR)
    elif kind == "lead_only":
        reason = ("`%s`는 리드 전용(Config·.claude·.uproject·.uasset·.umap) — 에셋·설정 변경은 보고서 "
                  "'다른 역할에 요청(표 행·에셋)'에 적어라. %s" % (rel, DENY_ANCHOR))
    else:
        reason = "`%s`는 %s — 보고서 '다른 역할에 요청'에 적어라. %s" % (rel, R.owner_label(reg, rel), DENY_ANCHOR)
    return deny(reason)


def role_python(root, args):
    """`py -3 …` / `python …` 인자 검사: -m unittest, Tools/*.py, .claude/hooks/*.py(cl9_lock.py 는 status 만)."""
    if not args:
        return deny("파이썬 대화 모드 금지 — " + SHELL_DENY_TAIL)
    if args[0] == "-m":
        return allow() if len(args) > 1 and args[1] == "unittest" else deny("`python -m` 은 unittest 만 — " + SHELL_DENY_TAIL)
    if args[0].startswith("-"):
        return deny("`python -c`·옵션 실행 금지 — " + SHELL_DENY_TAIL)
    rel = R.to_rel(args[0], root)
    if rel is None or not rel.endswith(".py") or not (rel.startswith("tools/") or rel.startswith(".claude/hooks/")):
        return deny("허용 스크립트는 Tools/*.py 와 .claude/hooks/*.py 뿐 — " + SHELL_DENY_TAIL)
    if rel == LOCK_SCRIPT_REL and (len(args) < 2 or args[1] != "status"):
        return deny("역할은 잠금을 잡거나 놓지 않는다 — `cl9_lock.py status` 만. " + SHELL_DENY_TAIL)
    return allow()


def role_shell(root, command):
    if not isinstance(command, str) or not command.strip():
        return deny("빈 명령 — " + SHELL_DENY_TAIL)
    if COMPOUND.search(command):
        return deny("복합 명령(; & | ` $( < > 줄바꿈) 금지 — 단일 명령 하나만. " + SHELL_DENY_TAIL)
    toks = split_tokens(command)
    if not toks:
        return deny("빈 명령 — " + SHELL_DENY_TAIL)
    prog = prog_name(toks[0])
    if prog == "py":
        if len(toks) >= 2 and toks[1] == "-3":
            return role_python(root, toks[2:])
        return deny("`py` 는 `py -3` 로만 — " + SHELL_DENY_TAIL)
    if prog in ("python", "python3"):
        return role_python(root, toks[1:])
    if prog == "blender":
        rel = R.to_rel(toks[2], root) if len(toks) >= 3 and toks[1] == "-b" else None
        if rel and rel.startswith("tools/blender/") and rel.endswith(".py"):
            return allow()
        return deny("Blender 는 `blender -b Tools/Blender/<script>.py` 로만 — " + SHELL_DENY_TAIL)
    if prog == "git":
        if len(toks) >= 2 and toks[1] in GIT_READ_VERBS:
            return allow()
        return deny("git 은 읽기 동사(%s)만 — 커밋·복원은 리드가 한다. %s" % ("|".join(sorted(GIT_READ_VERBS)), DENY_ANCHOR))
    if prog in READ_ONLY_PROGRAMS:
        if prog == "find" and any(t.lower() in FIND_WRITE_FLAGS for t in toks[1:]):
            return deny("`find` 의 -delete/-exec 류 금지 — " + SHELL_DENY_TAIL)
        return allow()
    return deny("`%s` 는 허용 목록 밖 — %s" % (toks[0], SHELL_DENY_TAIL))


def handle_role(data, reg, role, root):
    tool = data.get("tool_name", "")
    ti = data.get("tool_input") or {}
    run, terminal = find_run(root, role)
    if run is None:
        if terminal is not None:
            return deny("실행 `%s` 은 %s 상태 — 이 역할은 더 편집하지 않는다. 리드가 다시 배정한다. %s"
                        % (terminal["slug"], terminal["state"], DENY_ANCHOR))
        return deny("실행 상태 없음 — 이 역할을 담은 run.json 이 .claude/runs/ 에 없다(리드가 dispatch 하지 않았다). " + DENY_ANCHOR)
    touch = os.path.join(run["dir"], "touch-%s" % role)
    with open(touch, "w") as f:
        f.write(str(time.time()))
    grants_all = load_grants(root, run)

    if tool in EDIT_TOOLS:
        for key in ("file_path", "notebook_path"):
            path = ti.get(key)
            if isinstance(path, str):
                out = role_edit(reg, role, root, run, grants_all, path)
                if is_deny(out):
                    log_line(os.path.join(run["dir"], "denied.log"), "%s %s %s :: %s" % (
                        role, tool, path, out["hookSpecificOutput"]["permissionDecisionReason"]))
                return out
        return deny("편집 도구 입력에 경로가 없다 — " + DENY_ANCHOR)
    if tool in SHELL_TOOLS:
        out = role_shell(root, ti.get("command"))
        if is_deny(out):
            log_line(os.path.join(run["dir"], "denied.log"), "%s %s %r :: %s" % (
                role, tool, ti.get("command"), out["hookSpecificOutput"]["permissionDecisionReason"]))
        return out
    if tool == UNREAL_TOOL:
        out = deny("에디터 MCP(%s/%s)는 리드만 — 표 행·에셋 변경은 보고서 '다른 역할에 요청'에 적어라. %s" % (
            ti.get("toolset_name", "?"), ti.get("tool_name", "?"), DENY_ANCHOR))
    elif tool.startswith(BLENDER_PREFIX):
        out = deny("Blender MCP 는 리드만 — 역할은 `blender -b Tools/Blender/<script>.py` 로 돌린다. " + DENY_ANCHOR)
    else:
        return allow()
    log_line(os.path.join(run["dir"], "denied.log"), "%s %s :: %s" % (role, tool, out["hookSpecificOutput"]["permissionDecisionReason"]))
    return out


# ---------------------------------------------------------------- 리드 (그 밖 전부)

def parse_lock_command(root, command):
    """정확히 `py -3 .claude/hooks/cl9_lock.py <args…>`(단일 명령)이면 args, 아니면 None. --session/--root 가 있으면 None."""
    if not isinstance(command, str) or COMPOUND.search(command):
        return None
    toks = split_tokens(command)
    if len(toks) < 3:
        return None
    prog = prog_name(toks[0])
    if prog == "py" and toks[1] == "-3":
        script, args = toks[2], toks[3:]
    elif prog in ("python", "python3"):
        script, args = toks[1], toks[2:]
    else:
        return None
    if R.to_rel(script, root) != LOCK_SCRIPT_REL:
        return None
    if "--session" in args or "--root" in args:
        return None
    return args


def mediate_lock(root, session_id, args):
    """훅이 세션 ID 를 찍어 acquire/release 를 대신 하고, 실패는 deny 사유로 모델에 보낸다."""
    sub = args[0] if args else ""
    if sub == "acquire":
        purpose, minutes, pid, wait = (args[1] if len(args) > 1 else ""), None, None, 0
        i = 2
        while i < len(args):
            if args[i] == "--minutes" and i + 1 < len(args):
                minutes, i = int(args[i + 1]), i + 2
            elif args[i] == "--pid" and i + 1 < len(args):
                pid, i = int(args[i + 1]), i + 2
            elif args[i] == "--wait" and i + 1 < len(args):
                wait, i = int(args[i + 1]), i + 2
            else:
                i += 1
        try:
            L.acquire(root, session_id, purpose, minutes, pid, wait)
        except L.LockError as ex:
            return deny("잠금 획득 실패 — %s" % ex)
        L.hook_lock_mark(root, "acquire", purpose)
        return allow()
    if sub == "release":
        try:
            L.release(root, session_id, force=("--force" in args))
        except L.LockError as ex:
            return deny("잠금 해제 실패 — %s" % ex)
        L.hook_lock_mark(root, "release")
        return allow()
    if sub == "status" and "--nonce" in args:
        idx = args.index("--nonce")
        if idx + 1 < len(args):
            L.hook_seen_mark(root, re.sub(r"[^A-Za-z0-9_-]", "", args[idx + 1]))
    return allow()


def is_destructive(command):
    """에디터/빌드 파괴 동사 — 세그먼트마다 프로그램 토큰 기준(읽기 전용 프로그램의 인자는 무시)."""
    if not isinstance(command, str):
        return False
    for seg in re.split(r"[;&|\n]+", command):
        toks = split_tokens(seg)
        if not toks:
            continue
        prog = prog_name(toks[0])
        if prog in READ_ONLY_PROGRAMS or prog == "git":
            continue
        low = seg.lower()
        if prog == "taskkill" and "unrealeditor" in low:
            return True
        if prog == "stop-process" and "unreal" in low:
            return True
        if re.search(r"(^|[\s\"'/\\])build\.bat(\b|$)", low) or re.search(r"(^|[\s\"'/\\])unrealeditor-cmd(\.exe)?(\b|$)", low):
            return True
    return False


def lock_required_reason(root, what):
    ex, _ = L.holder(root)
    if ex:
        return "%s — 잠금은 %s | 끝났으면 상대가 `py -3 .claude/hooks/cl9_lock.py release`, 죽었으면 `release --force`(살아 있는 프로세스면 거부). %s" % (
            what, L.holder_line(ex), L.DOC_ANCHOR)
    return ("%s — 배타 잠금을 잡은 세션만 할 수 있다. 먼저 `py -3 .claude/hooks/cl9_lock.py acquire build --pid <PID>`(빌드) 또는 "
            "`acquire pie|compile|commandlet`. 아무도 안 잡았어도 잡아야 한다(09-07 사고 예방). %s" % (what, L.DOC_ANCHOR))


def handle_lead(data, root):
    tool = data.get("tool_name", "")
    ti = data.get("tool_input") or {}
    session_id = data.get("session_id") or ""
    L.holder(root)  # 락 파일이 깨졌으면 여기서 LockError — 다음 도구 호출에서 모델에게 사유가 보인다(내부 오류 극성)
    if tool in SHELL_TOOLS:
        command = ti.get("command")
        args = parse_lock_command(root, command)
        if args is not None:
            return mediate_lock(root, session_id, args)
        if is_destructive(command):
            if L.holds_exclusive(root, session_id):
                return allow()
            return deny(lock_required_reason(root, "에디터 종료·빌드·커맨드릿 명령"))
        return allow()
    if tool == UNREAL_TOOL:
        name = str(ti.get("tool_name") or "")
        if "save" in name.lower():
            return allow()  # save_assets 는 절대 거부하지 않는다
        if name in ("StartPIE", "StopPIE"):
            if L.holds_exclusive(root, session_id):
                return allow()
            return deny(lock_required_reason(root, "%s" % name))
        L.renew_lease(root, session_id)
        return allow()
    return allow()


# ---------------------------------------------------------------- 진입

def run(data):
    tool = data.get("tool_name", "")
    agent_type = str(data.get("agent_type") or "")
    root = find_root(data.get("cwd"))
    if L.guard_is_off(root):
        return allow()
    if agent_type.startswith("cl9-"):
        reg = R.load(root)
        role = R.resolve_role(reg, agent_type)
        if role is None:
            return deny("알 수 없는 역할 `%s` — 레지스트리(.claude/cl9-roles.json)에 없다. %s" % (agent_type, DENY_ANCHOR))
        return handle_role(data, reg, role, root)
    if tool in EDIT_TOOLS:
        return allow()
    return handle_lead(data, root)


def lead_internal_error(data, line):
    """리드 내부 오류: 위험한 호출(파괴 동사·PIE·잠금 CLI)은 deny, 그 밖은 allow + additionalContext."""
    reason = "가드 훅 오류: 잠금·역할 검사 미적용 — %s (.claude/runs/guard-errors.log)" % line
    tool = data.get("tool_name", "") if isinstance(data, dict) else ""
    ti = data.get("tool_input") if isinstance(data, dict) else None
    ti = ti if isinstance(ti, dict) else {}
    risky = False
    if tool in SHELL_TOOLS:
        cmd = ti.get("command")
        risky = is_destructive(cmd) or (isinstance(cmd, str) and "cl9_lock.py" in cmd)
    elif tool == UNREAL_TOOL:
        risky = str(ti.get("tool_name") or "") in RISKY_TOOL_NAMES
    if risky:
        return deny(reason + " — 검사를 못 했으니 에디터·빌드·PIE·잠금 명령은 막는다. 오류를 고치거나 `guard off`.")
    out = decide("allow", reason)
    out["hookSpecificOutput"]["additionalContext"] = reason
    return out


def main():
    raw = sys.stdin.buffer.read().decode("utf-8", "replace")
    role_hint = ""
    data = None
    try:
        data = json.loads(raw)
        if not isinstance(data, dict):
            raise ValueError("stdin JSON 이 객체가 아님")
        role_hint = str(data.get("agent_type") or "")
        out = run(data)
    except Exception as ex:  # noqa: BLE001 — 내부 오류는 극성대로 처리한다
        first = str(ex).splitlines()[0] if str(ex) else ""
        line = "%s: %s" % (type(ex).__name__, first)
        try:
            root = find_root(data.get("cwd") if isinstance(data, dict) else None)
        except Exception:  # noqa: BLE001
            root = os.getcwd()
        try:
            log_line(os.path.join(root, ".claude", "runs", "guard-errors.log"), "%s %s" % (role_hint or "lead", line))
        except OSError:
            pass
        if role_hint.startswith("cl9-"):
            out = deny("가드 훅 오류(역할 %s): %s — 리드에게 보고하라" % (role_hint, line))
        else:
            out = lead_internal_error(data, line)
    sys.stdout.write(json.dumps(out, ensure_ascii=True))
    sys.stdout.flush()


if __name__ == "__main__":
    main()
