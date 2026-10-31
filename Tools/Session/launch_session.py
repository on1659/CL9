"""새 Claude Code 세션을 별도 터미널 창에 띄운다.

    py -3 Tools/Session/launch_session.py <지시서.md> <세션 이름> [에이전트] [--boss <보고 받을 세션 이름>]

- 지시서는 파일로 준다. 프롬프트에는 "이 파일을 읽고 수행하고, 끝나면 SendMessage 로 보고하라"만 넣는다.
- 부모 세션의 CLAUDE_CODE_* 환경 변수를 지우고 띄운다. 안 지우면 새 claude 가 "이미 Claude Code 안"으로 보고 거부하거나 부모에 얽힌다.
- Windows Terminal(wt)이 있으면 새 탭, 없으면 cmd 의 start 로 새 창. 둘 다 이 저장소 루트를 작업 폴더로 한다.
- 권한은 bypass 로 띄우므로 금지 사항(커밋 금지·잠금 규약 등)은 지시서에 적는다.
"""
import argparse, os, shutil, subprocess, sys, uuid

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("brief", help="지시서 .md (절대 경로 권장)")
    ap.add_argument("name", help="세션 이름 (-n). 보고 받는 쪽이 SendMessage 로 찾는 이름")
    ap.add_argument("agent", nargs="?", default=None, help=".claude/agents 의 역할 이름 (예: cl9-gameplay). 없으면 기본 에이전트")
    ap.add_argument("--boss", default="부사장", help="끝났을 때 SendMessage 로 보고할 세션 이름")
    ap.add_argument("--profile", default=None, help="wt 프로필 이름 또는 GUID (없으면 기본 프로필)")
    a = ap.parse_args()

    brief = os.path.abspath(a.brief)
    assert os.path.isfile(brief), "지시서가 없다: " + brief
    prompt = (brief + " 를 읽고 그 지시를 그대로 수행한다. 질문이 생기면 그 파일에 적힌 규칙과 CL9 의 CLAUDE.md 를 따른다. "
              "묻지 말고 기본값으로 진행하고, 끝나면 SendMessage 로 '" + a.boss + "' 에게 보고한다.")

    exe = shutil.which("claude") or os.path.expanduser(r"~\.local\bin\claude.exe")
    assert os.path.isfile(exe), "claude CLI 를 못 찾았다: " + exe
    sid = str(uuid.uuid4())
    claude = [exe, "--dangerously-skip-permissions", "--session-id", sid, "-n", a.name]
    if a.agent:
        claude += ["--agent", a.agent]
    claude.append(prompt)

    env = {k: v for k, v in os.environ.items()
           if not k.startswith(("CLAUDE_CODE_", "CLAUDECODE", "CLAUDE_PID", "CLAUDE_EFFORT", "CLAUDE_PLUGIN_", "NO_COLOR", "GIT_TERMINAL_PROMPT"))}

    wt = shutil.which("wt")
    if wt:
        args = [wt, "-w", "new", "new-tab"] + (["-p", a.profile] if a.profile else []) + ["-d", ROOT] + claude
    else:
        # shortcut: wt 없는 PC 는 cmd 새 창. 제목은 세션 이름.
        args = ["cmd", "/c", "start", a.name, "/D", ROOT] + claude
    subprocess.Popen(args, close_fds=True, env=env, cwd=ROOT)
    print("launched", a.name, sid)


if __name__ == "__main__":
    main()
