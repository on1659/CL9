---
name: cl9-session
description: |
  새 Claude Code 세션을 별도 터미널 창에 띄워 지시서(md)를 주고 일을 시킨다. 부사장 세션이 큰 일을 직접 하지 않고 나눠 줄 때 쓴다.
  도구: Tools/Session/launch_session.py. 어느 PC 에서든 같은 방법.
  Triggers: /cl9-session, 새 세션 띄워, 세션 만들어서 시켜, 세션 띄우기
argument-hint: "<지시서.md> <세션 이름> [에이전트]"
user-invocable: true
---

# /cl9-session — 새 세션 띄우기

## 한 줄

```
py -3 C:\Work\CL9\Tools\Session\launch_session.py <지시서.md 절대 경로> "<세션 이름>" [cl9-gameplay|cl9-art|cl9-map|cl9-ui|cl9-docs|cl9-balance]
```

예: `py -3 C:\Work\CL9\Tools\Session\launch_session.py C:\Work\CL9\Saved\Briefs\brief_bot_sonar.md "봇 보강" cl9-gameplay`

출력 `launched <이름> <세션 id>` 가 나오면 새 Windows Terminal 탭(없으면 cmd 창)에서 그 세션이 지시서를 읽고 시작한다.

## 절차

1. **지시서를 파일로 쓴다.** 긴 프롬프트를 명령줄에 넣지 않는다. 지시서에 꼭 들어갈 것:
   - 왜(사장 말 그대로), 할 일(순서), 참고 파일(절대 경로), 규칙, 보고 형식.
   - 규칙에 꼭 적을 것: 커밋 금지·파괴적 git 금지, 잠금(`py -3 .claude/hooks/cl9_lock.py status --nonce X` 단독 명령, PIE 는 `acquire pie` 안에서만), 빌드는 브로커(`Tools/Build/build_request.py`), 손대도 되는 파일 범위, 막히면 30분 안에 '부사장' 에게 SendMessage.
   - 보고: 끝낸 것 / 못 한 것과 이유 / 바뀐 파일 / 영상 링크 / 사장 확인 필요 항목.
   - 본보기: 2026-10-28 밤 지시서(`docs/goal/overnight-full-round-and-monsters.md` 와 짝인 brief_night_*.md). 임시 지시서는 scratchpad 에, 다시 쓸 것은 `C:\Work\CL9\Saved\Briefs\` 에 둔다(Saved 는 git 밖).
2. **띄운다** (위 한 줄). 에이전트는 역할 계약(`.claude/agents/cl9-*.md`)으로 고른다 — 봇·게임 코드는 cl9-gameplay, 모델·애니는 cl9-art.
3. **메모리 work-progress 에 한 줄** — 어느 세션(이름·id)에 무엇을 시켰는지, 지시서 경로.
4. **기다린다.** 그 세션이 `SendMessage` 로 '부사장'(또는 `--boss` 로 준 이름)에게 보고한다. 보고가 오면 사장에게 요지와 영상 링크를 전한다. 폴링하지 않는다.
5. **끊긴 세션은 같은 지시서로 다시 띄운다.** 지시서에 "이미 된 것은 건너뛴다"를 한 줄 더한다.

## 스크립트가 하는 일 (왜 다른 데서는 안 됐나)

- `claude --dangerously-skip-permissions --session-id <uuid> -n <이름> --agent <역할> "<지시서> 를 읽고 수행 … 끝나면 SendMessage 로 보고"` 를 **운영체제 프로세스로** 띄운다. 서브에이전트가 아니라 독립 세션이다.
- **부모의 `CLAUDE_CODE_*`·`CLAUDECODE`·`CLAUDE_PID` 환경 변수를 지우고** 띄운다. 이걸 안 지우면 새 claude 가 "이미 Claude Code 안에서 돈다"고 보고 거부하거나 부모 세션에 얽힌다. 다른 PC 에서 못 띄운 원인은 대개 이것이다.
- 작업 폴더는 저장소 루트(스크립트 위치에서 계산) — PC 마다 경로가 달라도 된다.
- `wt`(Windows Terminal)가 있으면 새 탭, 없으면 `cmd /c start` 새 창. `--profile` 로 wt 프로필을 줄 수 있다(기본은 기본 프로필).
- `-n` 이름이 있어야 상대가 `SendMessage` 로 찾는다. 이름은 짧은 한글("봇 보강")로, 띄우는 세션·`--boss` 와 다른 이름이어야 한다(같으면 스크립트가 멈춘다). 부모의 `CLAUDE*` 환경 변수는 넘기지 않는다 — 백그라운드 세션의 `CLAUDE_BG_*` 가 새면 새 세션이 부모 이름을 단다.

## 함정

- **같은 에디터를 여러 세션이 쓴다.** 지시서에 잠금 규약을 빼먹으면 남의 PIE 를 끊는다 — `docs/workflow/sessions.md`.
- **bypass 로 뜬다.** 금지 사항은 전부 지시서에 적는다. 스크립트가 막아 주지 않는다.
- **새 PC 첫 실행**: `claude` 가 PATH 나 `~\.local\bin\claude.exe` 에 있어야 하고, 한 번은 사람이 `claude` 를 실행해 로그인해 둬야 한다.
- 지시서가 scratchpad(Temp)에 있으면 세션 재시작 뒤 지워질 수 있다 — 다시 띄울 지시서는 `Saved/Briefs/` 로.
