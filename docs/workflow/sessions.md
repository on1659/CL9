# 세션 조율 — 역할·잠금·에디터/빌드/PIE 규약

여러 Claude Code 세션(그리고 `/cl9`가 띄우는 역할 서브에이전트)이 **에디터 하나, PIE 하나, 빌드 하나, 작업
트리 하나**를 나눠 쓰는 규칙이다. 설계와 결정 기록은 [docs/goal/session-orchestration.md](../goal/session-orchestration.md),
동작 설명은 [session-orchestration.walkthrough.md](../goal/session-orchestration.walkthrough.md). 이 문서가 규약의 정본이고,
훅의 거부 사유가 여기의 `#거부`·`#잠금`을 가리킨다. Claude Code **2.1.201**에서 검증(2026-09-13).

구성 요소(전부 `.claude/` 아래, 파이썬 3.9 `py -3`):

| 파일 | 역할 |
|---|---|
| `.claude/cl9-roles.json` | 역할 레지스트리 — 누가 무엇을 소유·공유하나. **유일한 정본**, 아래 표는 요약 |
| `.claude/hooks/cl9_scope_guard.py` | PreToolUse 훅 — 역할 울타리 + 잠금 극성. `.claude/settings.json`에 `py -3 "$CLAUDE_PROJECT_DIR/.claude/hooks/cl9_scope_guard.py"`로 등록 — **반드시 절대 경로**(훅은 세션 셸의 cwd에서 돌고, 스크립트를 못 열면 종료 코드 2 = 차단이라 모든 셸이 막힌다) |
| `.claude/hooks/cl9_roles.py` | 역할 레지스트리 로더 — `.claude/cl9-roles.json` 을 읽고 경로를 역할에 대응한다. 훅·렌더러·테스트가 이 모듈 하나만 쓴다(테스트 `test_cl9_roles.py`) |
| `.claude/hooks/cl9_lock.py` | 에디터/빌드 잠금 CLI |
| `.claude/hooks/render_roles.py` | 레지스트리 → 역할 계약/에이전트 본문 렌더 |
| `.claude/hooks/cl9_status.py` | `/cl9 status` 출력 |
| `.claude/hooks/cl9_audit.py` | 실행 후 감사(기준선 대비 델타) |
| `.claude/hooks/cl9_run.py` | `/cl9 task` 실행 관리 — init·overlap·dispatch·state·validate-report·audit·finish·show·latest |
| `.claude/agents/cl9-*.md` | 역할 서브에이전트 6개 — `render_roles.py --write` 산출물(손으로 고치지 않는다) |
| `Tools/Build/compile_check.py` | 에디터 켠 채 게임 타깃 컴파일 검사(잠금 자동) |
| `Tools/Balance/check_inequalities.py` | docs/plan 표로 A1/A2 판정, A4 미결 — balance 보고서 '검증(부등식)' |
| `Tools/Unreal/apply_table_rows.py` | 보고서 '표 행' 을 데이터 표 에셋에 적용·검증(에디터 안 또는 커맨드릿) |
| `.claude/runs/` | 실행 산출물(gitignore) — `editor.lock`, `<slug>/brief-*.md`, `report-*.md`, `grants.json`, `run.json`. 하위 폴더: `compile/` = `Tools/Build/compile_check.py` 의 빌드 로그(`compile-<시각>.log`) · `hook-seen/` = `cl9_lock.py` 가 남기는 훅 활성 확인용 nonce 표식과 "훅이 acquire/release 를 대신 처리함" 표식(`last-lock-cmd.json`) · `probe/` = 롤아웃 1 의 훅·환경 조사 기록(`agent-report.md`·`after-ask.txt`·`env-*.json`, session-orchestration.md "Rollout 1") |
| `.claude/skills/cl9/SKILL.md` | `/cl9` 스킬 |

테스트: `py -3 -m unittest discover .claude/hooks`.

---

## 역할

여섯 역할. 이름은 영문, 별칭(`전투`·`시스템`→gameplay, `유아이`→ui, `맵`·`레벨`→map, `아트`·`에셋`→art, `문서`→docs,
`밸런스`·`수치`→balance)도 통한다. 표는 요약 — 정확한 패턴은 레지스트리, 계약 전문은 `/cl9 role <이름>`.

| 역할 | 소유(자유 편집) | 공유(편집 가능, 보고서에 적음) |
|---|---|---|
| gameplay | `Source/CL9/{Character,Items,Monsters,GameMode,Core,Data}/*`(Site/Module/SegKind/Piece Def 제외), `docs/systems/`의 게임플레이 문서, `docs/plan/data/sequence.md` | `Source/CL9/UI/*`(게터), **grant-gated**: `docs/plan/data/{rules,progression,buff,equipment,order}.md`, `data/item/*`, `data/monster/*` |
| balance | `docs/plan/economy.md` | **grant-gated** 위와 같음(수치 셀만), `docs/plan/data/character.md`(몸 수치), `data/site.md`(부지 개수·밤 시계), `docs/plan/loop.md`(페이싱 수치) |
| ui | `Source/CL9/UI/*`, `Content/Strings/*`, `Config/Localization/*`, `Tools/Loc/*`, `docs/systems/{ui,ui_system,input,settings,localization}.md`, `docs/plan/settings.md` | `CL9PlayerController.*`(입력 바인딩), `Source/CL9/Character/*`(입력만) |
| map | `Source/CL9/World/*`, `CL9{Site,Module,SegKind,Piece}Def.*`, `ArtSource/map/*`, `docs/systems/{map,light,asset-swap}.md`, `docs/plan/data/{module,site,segkind}.md` | `CL9GameMode.*`(스폰·시드) |
| art | `ArtSource/{character,item,monster,validation}/*`, `Tools/Blender/*`, `Tools/Unreal/*`, `docs/asset/*`, `docs/systems/animation.md`, `docs/plan/data/character.md` | `CL9ItemDef.*`, `CL9CharacterDef.*`(메시·애님 열) |
| docs | `docs/plan/{README,overview,loop,build-order}.md`, `docs/plan/data/README.md`, `docs/{conventions,ue-cpp-conventions}.md`, `docs/{reference,review,goal,testing}/*`, `TODOS.md`, `CLAUDE.md` | `docs/systems/*`(일관성 수정) |

전 역할 공통:
- **append-only** `docs/decisions.md` — 역할은 못 고친다. 항목 문안을 보고서에 쓰면 리드가 붙인다.
- **리드 전용** `Config/*`(Localization 제외), `.claude/*`, `CL9.uproject`, `CL9.Build.cs`, `CL9.{h,cpp}`, 모든 `.uasset`·`.umap` —
  바이너리 에셋은 에디터로만 고치고, 에디터는 리드만 몬다. 역할은 "표 행: `ItemTable.egg.HoldPose: None → Small`"처럼 요청한다.
- **grant-gated 공유**: 두 역할의 공유 목록에 같이 있는 경로(gameplay ↔ balance 의 수치 문서). 실행마다 `grants.json`에 **한 역할 이름으로만**
  올라가야 편집할 수 있다 — 같은 파일에 두 손이 못 들어간다.
- 우선순위: append-only > 리드 전용 > 소유(제외 반영) > 공유 > grant > 밖.

역할이 grant 밖 경로를 만지면 훅이 막고 사유에 소유 역할을 적는다 → [`#거부`](#거부).

### 모델 배분 (2026-10-04)

서브에이전트 모델은 **역할이 아니라 일의 성격**으로 고른다 — 같은 art 역할이라도 새 에셋의 첫 모양은 판단이고, 확정된 모양의 변형은
반복이다. 리드(메인 세션)는 사용자가 고른 모델(`/model`)로 돈다. 역할 서브에이전트의 정의 파일 기본값은 `opus`(`.claude/cl9-roles.json`
`defaults.model`)라서, **sonnet·haiku 로 돌릴 일은 띄울 때 `model` 을 명시**한다. 명시하지 않으면 opus 로 돈다.

| 일의 성격 | 예 | 모델 |
|---|---|---|
| 판단·설계·첫 형태 | 새 에셋의 첫 모양, 생성기·시스템 설계, 원인 조사, 밸런스 수치 근거, 리뷰 | `opus` |
| 정해진 것의 반복·기계적 적용 | 승인된 모양의 변형·LOD, 임포트·배치 스크립트, 표 행 적용, 정해진 문서 갱신, 명세대로의 C++ 보일러플레이트 | `sonnet` |
| 아주 단순한 일 | 색인 한 줄, 파일 목록·상태 수집 | `haiku` |

- 애매하면 한 단계 위로 고른다 — 다시 돌리는 비용이 모델 차이보다 크다.
- brief 에 쓴 모델과 이유 한 줄을 실행 기록(`run.json` 또는 보고서)에 남긴다 — 다음 실행이 같은 판단을 다시 하지 않게.
- 리드의 권장 모델·추론 강도는 작업 명세(`docs/goal/<slug>.md` "Execution Notes")가 정한다.

---

## 잠금

에디터 하나·PIE 하나·빌드 하나(UBT 전역 뮤텍스)·MCP 포트 8000 하나. **끄거나 세우거나 돌리는 쪽이 잠금을 잡는다** —
"아무도 안 잡았으니 괜찮다"가 아니라 **내가 잡고 있어야** 훅이 `taskkill`·`Build.bat`·`UnrealEditor-Cmd`·`StartPIE`·`StopPIE`를
허용한다(2026-09-07 사고: 잠금 없이 에디터를 끄고 동료 작업 6분 날림).

```
py -3 .claude/hooks/cl9_lock.py acquire <build|compile|pie|commandlet|editor> [--minutes N] [--pid P] [--wait S]
py -3 .claude/hooks/cl9_lock.py release [--force]
py -3 .claude/hooks/cl9_lock.py status [--nonce X]
py -3 .claude/hooks/cl9_lock.py guard off [--minutes N] | on
```

- **배타 용도** `build`·`compile`·`pie`·`commandlet`: 한 세션만. `build`·`commandlet`은 `--pid`(분리 실행한 .cmd 의 PID) **필수** —
  만료(기본 90분, 그 밖 30분)가 지나도 그 프로세스가 살아 있으면 잠금이 유지된다(Live Coding 빌드는 어떤 만료보다 길다).
  PID 는 (pid, 생성 시각, 이미지 이름) 삼중항으로 확인한다 — Windows 는 PID 를 재사용한다.
- **공유 임대** `editor`: 에디터를 MCP 로 모는 세션은 훅이 `call_tool`마다 자동 갱신한다(30분). 임대가 살아 있는 동안 다른 세션은
  배타 잠금을 못 잡는다 → 에디터를 못 끈다. `/cl9 role`로 역할을 맡은 세션도 임대를 하나 받는다.
- `acquire` 는 원자적이다(`O_EXCL` 생성 → 검증 → 임시 파일 교체 → 재확인). `--wait` 는 90초까지(Bash 도구 120초 제한) — 넘으면 "busy"로
  돌아오니 다시 친다.
- `commandlet` 은 `UnrealEditor.exe`·`UnrealEditor-Cmd.exe` 가 하나라도 떠 있으면 거부된다(헤드리스 저장은 에디터를 닫은 창에서만 —
  안 그러면 유령 에디터).
- `release --force` 는 기록된 프로세스가 살아 있으면 거부된다. 죽은 잠금만 뺏는다.
- `save_assets` 는 **절대 거부되지 않는다** — "에디터 끈다"는 통지를 받은 쪽이 저장은 할 수 있어야 한다.
- **훅은 세션 시작 때 실린다.** 훅이 없던 세션은 `.claude/settings.json`에 훅이 생겨도 재시작 전까지 잠금을 지키지도 따르지도 않는다(훅이 이미 실린 세션은 명령 문자열 변경을 파일 감시로 받는다 — 2026-09-13 실측).
  `/cl9 status`(또는 `cl9_lock.py status --nonce <아무 값>`)가 "훅: 활성/비활성"을 알려 준다.
- 세션 ID 는 훅이 stdin 에서 찍는다 — 셸에서 `--session` 을 줄 필요가 없다. `--session` 은 테스트용.
- 잠금은 *안 된다*를 말하고, `SendMessage` 는 *왜·언제*를 말한다 — 둘 다 한다.
- 사람이 손으로 에디터를 닫거나 Claude Code 밖 PowerShell 창에서 빌드하는 건 훅 밖이다 — 아래 체크리스트를 따른다.

### 빌드 브로커 — 빌드는 여기에 요청만 한다 (2026-10-15)

세션이 동료를 찾고 통지하고 잠금을 잡던 협상을 없앴다. 정본: [docs/goal/build-broker.md](../goal/build-broker.md).

```
py -3 Tools/Build/build_request.py --kind build|compile --reason "무엇을 바꿨나"     # Bash timeout 600000
py -3 Tools/Build/build_ctl.py status | hold | resume | now | start-in N | coalesce S | cancel ID | history
제어 페이지: http://127.0.0.1:8765/
```

- **세션은 요청만 한다.** `build` 는 에디터 타깃 정식 빌드(에디터를 끄고 다시 켠다), `compile` 은 게임 타깃 문법 검사. 브로커가 없으면
  요청이 띄운다. 요청 전에 **내 에셋을 저장한다** — 브로커는 대신 저장하지 않는다.
- **묶기.** 첫 요청 뒤 30초 안에 들어온 같은 종류의 요청은 한 번에 빌드한다. 같은 세션의 같은 종류 중복 요청은 하나로 합친다.
- **사용자가 제어한다.** 보류·재개, "N분 뒤 시작", 묶음 대기 조정, 티켓 취소, **지금 시작**(보류·시각·묶음 대기·남의 에디터 임대를 무시 —
  살아 있는 남의 배타 잠금은 못 뺏는다). 에디터를 쓰는 세션의 임대가 살아 있으면 브로커는 시작하지 않고 큐에 사유를 보인다.
- **실패하면** 결과에 오류 줄마다 파일 · 줄 · 코드 · **주인 역할**(`cl9-roles.json`) · 수정 중 여부 · 마지막 성공 빌드 이후 수정 여부가 붙어
  요청 세션 전원에게 돌아간다. **누구 것인지는 받은 세션이 판단**한다 — 내 파일이면 고치고 같은 명령으로 다시 요청하고, 아니면 주인 역할/요청
  세션에 알린다. 같은 오류로 연속 2회 실패하면 `needs_user` 로 제어 페이지에 뜨고, 세션은 더 시도하지 않고 사용자에게 알린다.
  정식 빌드가 실패하면 에디터는 꺼진 채 남는다(고쳐서 다시 요청 — 제어 페이지에 '에디터 켜기').
- **기록.** 빌드마다 `.claude/runs/build-broker/builds.jsonl` 에 git HEAD · 수정 중 파일 · 요청 세션/사유 · 결과 · 시간이 남는다
  (`build_ctl.py history`).
- **꺼짐.** 큐가 비고 15분간 요청·제어가 없으면 스스로 종료한다. 브로커는 파이썬 프로세스라 토큰을 쓰지 않는다.
- 브로커는 `cl9_lock` 을 세션 ID `broker` 로 잡는다 — 아래 수동 절차를 따르는 세션도 잠금을 본다(병행). 훅으로 직접 `Build.bat` 을 막는
  대체 단계는 아직 아니다.

### 빌드 — 복사해서 쓰는 수동 절차 (브로커를 못 쓸 때)

```
# 0. 동료 확인 + 잠금 (PID 는 아래 3에서 띄운 .cmd 의 것 — 먼저 띄우고 잡거나, acquire compile 로 잡고 build 로 승격)
ListAgents → 에디터를 쓰는 세션에 SendMessage "N분 뒤 에디터 끈다, 저장할 것 있나" (2분 무응답이면 진행)
py -3 .claude/hooks/cl9_lock.py acquire build --pid <PID>
# 1. 저장 (경로 명시 — save_assets([]) 전체 저장 금지: 지운 에셋이 부활한다)
call_tool AssetTools save_assets {"asset_paths":["/Game/…"]}
# 2. 끄기 → 5초 대기(Live Coding 뮤텍스)
taskkill /IM UnrealEditor.exe /F
# 3. 분리 프로세스 빌드 — Bash/PowerShell 도구의 백그라운드는 10분 제한에 죽는다
#    scratchpad 에 build.cmd(Build.bat CL9Editor Win64 Development -Project=… → 성공 시 UnrealEditor.exe 재기동) 를 쓰고
Start-Process cmd.exe -ArgumentList '/c','"<scratchpad>\build.cmd"' -WindowStyle Hidden -PassThru   # ← 이 PID 로 acquire
Monitor 로 로그의 BUILD EXIT / Result: 와 UnrealEditor.exe·포트 8000 을 감시
# 4. 에디터가 다시 뜨면 MCP 대기 → 동료에게 "에디터 올라옴" → 해제
py -3 .claude/hooks/cl9_lock.py release
```

에디터를 끈 상태의 빌드는 이 머신에서 1~3분, 켠 채 Live Coding 은 20~48분. 여러 세션 변경분은 **모아서 한 번에** 빌드한다.
빌드 전 `tasklist`로 에디터가 이미 떠 있는지 확인한다 — 두 개가 뜨면 나중 것이 포트 8000을 못 잡고 레벨만 붙든 **유령**이 되어
`.umap` 저장을 `Error Code 32`로 막는다(포트 소유 PID 는 `netstat -ano`).

### PIE

```
py -3 .claude/hooks/cl9_lock.py acquire pie
call_tool … StartPIE   →   검증(docs/testing/pie-scenarios.md 절 문자)   →   StopPIE
py -3 .claude/hooks/cl9_lock.py release
```

PIE 중에는 다른 세션의 에디터 편집이 위험하다(ABP 변수 추가→컴파일이 T포즈를 만든 사고). 잠금이 그것을 막는다.

### 컴파일 검사 (에디터 켠 채 — 역할 에이전트도 쓴다)

```
py -3 Tools/Build/compile_check.py          # Bash 도구 timeout 600000 으로. COMPILE OK|FAIL <초> + 오류 줄
```

게임 타깃(`CL9 Win64 Development`)은 에디터가 로드한 `UnrealEditor-CL9.dll` 과 바이너리·중간물이 달라 **에디터를 끄지 않고**
같은 소스를 컴파일한다(2026-09-13 실측: 첫 빌드 10분, 무변경 16~85초, 한 파일 증분 2~3분 — 단 메모리가 3 GB 아래면 cl.exe 가
수십 분 멈춘다. 에디터가 8 GB 를 커밋하니 게임·브라우저를 닫는다). **문법 검사다** — `WITH_EDITOR=0` 이라 에디터 전용 코드는
안 잡히고, 에디터에 반영하려면 여전히 정식 빌드가 필요하다. 스크립트가 `compile` 잠금을 스스로 잡고(세션 ID 는
`CLAUDE_CODE_SESSION_ID`) 놓는다. 역할 에이전트의 셸 허용 목록(`py -3 Tools/*.py`)을 통과한다.

### 헤드리스 커맨드릿 (에디터 닫힌 창)

```
py -3 .claude/hooks/cl9_lock.py acquire commandlet --pid <UnrealEditor-Cmd 를 띄운 프로세스 PID>
UnrealEditor-Cmd.exe <CL9.uproject> -run=pythonscript -script="Tools/Unreal/xxx.py apply"
py -3 .claude/hooks/cl9_lock.py release
```

### 잠금이 막혔을 때

거부 사유에 보유자·용도·만료·프로세스 상태가 적혀 있다.
- 상대가 끝났으면 상대가 `release`. `SendMessage` 로 부탁한다.
- 상대 세션이 죽었고 기록된 프로세스도 죽었으면 `release --force`.
- 프로세스가 살아 있으면(빌드 중) 기다린다 — force 도 거부된다.
- 훅 자체가 고장이면(`.claude/runs/guard-errors.log`) `guard off --minutes 30` 으로 전부 허용하고, 끝나면 `guard on`.
  꺼져 있는 동안은 `/cl9 status` 가 알려 주고, 실행 후 감사가 위반으로 잡는다.

---

## 거부

훅(`cl9_scope_guard.py`)이 도구 호출을 막으면 사유가 도구 오류로 그대로 보인다. 종류:

| 사유 | 뜻 | 할 일 |
|---|---|---|
| `…는 gameplay 영역 — 보고서 '다른 역할에 요청'에 적어라` | 역할이 남의 경로를 편집하려 함 | 보고서에 요청으로 적는다. 정말 이번 실행 몫이면 리드가 `grants.json` 에 넣는다 |
| `…는 grant-gated 공유 경로 — 이번 실행에서는 balance 것` | 두 역할이 공유하는 수치 문서가 다른 역할에 배정됨 | 그 역할에 요청 |
| `…는 append-only` / `…는 리드 전용` | `decisions.md` / `Config`·`.claude`·에셋 | 보고서 해당 절에 적는다 |
| `실행 상태 없음` | 역할을 담은 `run.json` 이 없다 — 리드가 dispatch 하지 않았다 | 리드에게 |
| `실행 … 은 REPORTED/ABORTED 상태` | 끝난 실행의 역할이 계속 편집하려 함 | 리드가 다시 배정 |
| `복합 명령 금지` / `허용 목록 밖` | 역할의 셸은 읽기 전용 단일 명령만 | Edit/Write 도구로 고치고, 나머지는 보고서에 |
| `에디터 종료·빌드·커맨드릿 명령 — 배타 잠금을 잡은 세션만` | 리드/수동 세션이 잠금 없이 파괴 명령 | `acquire` 먼저 → [`#잠금`](#잠금) |
| `잠금 보유: build — 세션 xxxx · PID 1234 살아 있음 …` | 남이 잡고 있다 | 기다리거나 `release` 부탁, 죽었으면 `--force` |
| `가드 훅 오류: … 미적용` | 훅 내부 오류(레지스트리·락 파일 손상·stdin) | 파괴 명령은 막히고 나머지는 통과한다. `guard-errors.log` 보고 고친다 |

역할 서브에이전트는 **우회하지 않는다**(`python -c`, `Set-Content`, 다른 경로로 복사). 훅은 경보선이고 진실은 실행 후 감사다 —
감사에서 brief 밖 파일이 바뀐 것은 전부 위반으로 보고된다.

---

## 실행 — brief 와 보고서

`/cl9 <작업>` 리드가 `.claude/runs/<slug>/` 에 역할마다 `brief-<역할>.md` 를 쓰고, 역할은 `report-<역할>.md` 를 남긴다.
수동 세션(`/cl9 role`)은 `.claude/runs/manual-<역할>-<YYYYMMDD>/` 에 같은 형식으로 쓴다.

### brief 예시 (`brief-ui.md`)

```markdown
# brief — ui · 실행 hud-stamina-threshold

## 목표
HUD 스태미나 바가 `CL9Rules.StaminaBarWarnRatio` 문턱 아래에서 붉게 바뀐다. 문턱은 gameplay 가 만드는 게터로 읽는다.

## 파일 (이 목록 = grants.json — 다른 파일은 훅이 막는다)
- Source/CL9/UI/CL9HUDWidget.h
- Source/CL9/UI/CL9HUDWidget.cpp
- docs/systems/ui.md

## 받는 인터페이스 (gameplay 가 제공, 먼저 끝난다)
- `float UCL9RulesDef::GetStaminaBarWarnRatio() const` — 0~1

## 주는 인터페이스
- 없음

## 읽을 문서
- docs/systems/ui.md#스태미나-바, docs/systems/ui_system.md

## PIE 확인
- F2 (HUD 스태미나 바)

## 보고서
- .claude/runs/hud-stamina-threshold/report-ui.md
```

### 보고서 예시 (`report-ui.md`)

```markdown
## 한 일
문턱 비율을 `GetStaminaBarWarnRatio()` 로 읽어 바 색을 바꾼다. 하드코딩 0.25 제거.

## 바꾼 파일
Source/CL9/UI/CL9HUDWidget.h
Source/CL9/UI/CL9HUDWidget.cpp
docs/systems/ui.md

## 다른 역할에 요청(인터페이스·표 행·에셋)
- (리드) 표 행: `CL9Rules.Default.StaminaBarWarnRatio: (신설) → 0.25 (비율)` — 자리표시, balance 가 확정
- (balance) 문턱 값 근거 — `docs/plan/data/rules.md` 에 자리가 없음

## 빌드 필요 여부
예 (헤더 변경)

## PIE 확인 항목
- F2: 달리다 스태미나 25% 아래에서 바가 붉게 — 문턱을 0.5 로 바꾸면 50% 에서

## decisions.md에 남길 결정
- 스태미나 바 경고 문턱은 CL9Rules 컬럼(초안 0.25) — 하드코딩 폐기

## 질문(막힌 것)
없음
```

balance 보고서는 `## 표 행`, `## 검증(부등식)` 두 절이 더 붙는다(`/cl9 role balance` 참고).

---

## `/cl9` 와 수동 세션의 공존

- `/cl9 <작업>` 리드는 `ListAgents` 에 보이는 **한 동료일 뿐**이다 — 같은 잠금, 같은 규약. 리드가 에디터를 몰면 임대가 잡히고, 빌드하면
  배타 잠금이 잡혀 수동 세션도 기다린다.
- 수동 세션은 `/cl9 role <이름>` 으로 계약을 받아 채택한다 — 사용자가 손으로 치던 "내 세션은 UI 담당이다. 영역: …" 를 레지스트리에서
  찍어 준다. 역할 알림 한 줄을 동료에게 보낸다.
- 리드가 dispatch 할 때 수동 세션이 만지는 파일과 겹치면 안 된다 — 리드가 `ListAgents`/`SendMessage` 로 물어 목록을 받는다.
- 세션 이름(`cl9-xx`)은 자동이다. 별칭 혼동(cl9-5b vs cl9-2d)을 막으려면 메시지 첫 줄에 역할을 쓴다.

## 커밋

- 역할(또는 수동 세션)마다 **한 커밋**, 파일 목록은 보고서 '바꾼 파일' 그대로. 리드가 `/cl9 commit <역할>` 로 스테이징·메시지 초안을 만들고
  **사용자가 예라고 할 때만** 커밋한다.
- 메시지는 한국어 한 줄 제목 + 본문. 다른 세션의 미커밋 파일은 건드리지 않는다.
- `.umap` 도 커밋한다(100MB 근처면 원인 먼저). `.claude/settings.local.json` 은 추적을 끊을 예정(TODOS).

## 인계 (`/cl9 handoff`)

컨텍스트 75% 즈음: `memory/work-progress.md`(Claude 메모리 폴더 `C:\Users\user\.claude\projects\D--Work-unreal-CL9\memory\work-progress.md` — 저장소 밖) 갱신 → `.claude/runs/<slug>/handoff.md` → 새 터미널용 재개 프롬프트 출력.

```
CL9 이어서 하자 — memory/work-progress.md(C:\Users\user\.claude\projects\D--Work-unreal-CL9\memory\work-progress.md, 저장소 밖) 끝 항목부터. 실행 <slug> 의 handoff.md 를 읽고 `/cl9 resume <slug>`.
역할 <이름> 이면 먼저 `/cl9 role <이름>`.
```

---

## 런북

**유령 에디터** — 저장이 `Error Code 32`, 또는 MCP 가 안 붙는다: `tasklist /FI "IMAGENAME eq UnrealEditor.exe"` 로 둘 이상이면 `netstat -ano | findstr :8000` 으로
포트 주인을 찾고, 주인이 아닌 쪽을 `taskkill /PID`. 잠금이 필요하다(`acquire compile` 로 잡는다 — pid 없이 되는 배타 용도).

**빌드 실패** — 로그의 첫 `error C`/`error LNK` 를 보고 **바꾼 파일의 역할**에 돌려준다(`SendMessage` 로 그 에이전트 이름에, 수동이면 그 세션에).
리드가 직접 고치지 않는다 — 감사에서 리드 편집이 위반으로 잡힌다. 잠금은 빌드가 끝날 때까지 유지.

**말 없는 서브에이전트** — `/cl9 status` 의 "마지막 편집 N분 전" 이 10분을 넘고 보고서가 없으면 `SendMessage "보고서를 지금 써라"` 한 번 →
그래도 없으면 `TaskStop` + `run.json` 을 ABORTED 로(훅이 그 역할의 편집을 막는다). 권한 프롬프트에 걸린 경우가 가장 흔하다 —
백그라운드 서브에이전트의 프롬프트는 리드 세션에 뜬다.

**낡은 잠금** — `status` 에 보유자가 있는데 `ListAgents` 에 그 세션이 없고 프로세스도 죽음이면 `release --force`. 프로세스가 살아 있으면
그 PID 가 뭔지 `tasklist /FI "PID eq N"` 으로 확인하고 정말 고아면 프로세스를 끝낸 뒤 force.

**훅 비활성** — `/cl9 status` 가 "훅: 비활성" 이면 이 세션은 규약 밖이다. `/cl9 handoff` 후 세션을 재시작한다. `.claude/settings.json` 을
바꾼 직후 모든 세션이 이 상태다.

**settings.json 을 바꾼 뒤** — 열린 세션 전부에 아래를 보낸다:
```
.claude/settings.json 의 훅이 바뀌었다. 지금 하던 단위를 마치고 memory/work-progress.md(C:\Users\user\.claude\projects\D--Work-unreal-CL9\memory\work-progress.md, 저장소 밖)를 갱신한 뒤 세션을 재시작하라.
재시작 전까지 네 세션은 잠금을 지키지도 따르지도 않는다 — 에디터 종료·빌드·PIE 를 하지 마라.
```
