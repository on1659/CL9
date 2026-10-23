# 자율 제작 루프 도구 — 쓰는 법

자율 제작 루프의 다섯 부품(백로그·문지기 기록·결정 함·사장 보고서·상한)을 파일로 다루는 파이썬 도구다. 이 도구는 LLM 을 부르지 않는다. 조율자(`/office CL9 step` 을 도는 LLM 세션)가 명령으로 불러 집기·상태 갱신·기록·보고서 조립을 맡기고, 사무실 페이지는 같은 파일을 읽는다.
명세의 정본은 [C:\Work\CL9\docs\goal\autonomous-production-loop.md](C:\Work\CL9\docs\goal\autonomous-production-loop.md)이다.

## 공용 도구 위치 (2026-10-18 이동)

루프 도구·사무실 서버·조율자 스킬은 여러 프로젝트가 같이 쓰는 공용 도구 `C:\Work\Tools\AgentOffice\` 로 옮겼다(CL9 저장소 밖, 설치·등록·스키마는 [C:\Work\Tools\AgentOffice\README.md](C:\Work\Tools\AgentOffice\README.md)).

| 무엇 | 자리 |
|---|---|
| 루프 도구 | `C:\Work\Tools\AgentOffice\loop\loop.py` (옛 `C:\Work\CL9\Tools\Loop\`) |
| 사무실 서버·페이지 | `C:\Work\Tools\AgentOffice\office\office_server.py`, `office\page\` — `http://127.0.0.1:8770/?project=CL9` (옛 데이터 패널 `/office`) |
| 조율자 스킬 | `C:\Work\Tools\AgentOffice\skills\office\SKILL.md` → 사용자 수준 `/office CL9 step` (옛 `/cl9loop`, `.claude\skills\cl9loop\` 는 지웠다) |
| 프로젝트 등록 | `C:\Work\Tools\AgentOffice\projects.json` 의 `{"name": "CL9", "root": "C:\\Work\\CL9"}` |
| CL9 설정 | [C:\Work\CL9\.agent-office\project.json](C:\Work\CL9\.agent-office\project.json) |

CL9 의 `project.json` 은 루프 파일을 `.agent-office\` 가 아니라 기존 자리에 두도록 `paths` 로 가리킨다: 백로그 `docs\plan\backlog.md`, 결정 함 `docs\decisions-inbox.md`, 결정 기록 `docs\decisions.md`(append 만), 실행 폴더 `.claude\runs`, 상태·사건·지시사항·보고서 `.claude\runs\loop\`, 상한·문지기 표 `.agent-office\limits.csv`·`gate_checks.csv`, 영상 `Saved\Video`.
탐침 `probes` 는 셋 다 켠다(`cl9_runs`: 실행 폴더 `run.json`·`touch-<역할>`·`.claude\cl9-roles.json` / `cl9_lock`: `.claude\hooks\cl9_lock.py` / `build_broker`: 브로커 8765·`builds.jsonl`).
단계 실행기 `stages` 는 기획 = 일반 서브에이전트(분할안 역할 절), 명세 = `/autogoal`, 구현 = `/cl9 task <명세> --unattended`, 아트 = `/cl9 task`(분할안 10-2), 검수 = `/cl9film <기능>` 이고, 역할마다 사무실 동물·책상 배정은 `roles` 에 있다.

## 부르는 법

```
py -3 C:\Work\Tools\AgentOffice\loop\loop.py --root C:\Work\CL9 [--json] <명령> …
```

출력은 사람이 읽는 한국어 줄이고, `--json` 을 붙이면 같은 내용을 JSON 한 덩어리로 돌려준다(`exit` 열쇠에 종료 코드가 들어 있다). `--root` 는 반드시 준다 — 없으면 "프로젝트 루트를 지정하라"로 멈춘다. 파일 위치는 그 루트의 `.agent-office\project.json` 이 정한다.
종료 코드는 0 성공, 1 오류(또는 문지기 반려), 2 할 것 없음·승격 안 함, 3 상한·정지다.

## 명령

| 명령 | 하는 일 |
|---|---|
| `pick [--skip-waiting]` | 먼저 결정 함의 답을 반영(`inbox apply`)하고 상한을 본 뒤, 집기 규칙대로 다음 (항목, 단계)를 정해 상태를 `진행` 으로 바꾸고 `state.holding` 을 갱신한다. 결과의 `brief_notes` 는 그 항목의 다음 brief 머리에 넣을 "결정 N: <답 원문>" 줄이다. 붙든 항목이 기다리는 중이면 그 항목을 `waiting_on` 과 함께 돌려주고, `--skip-waiting` 이면 그 항목은 붙든 채 두고 다른 항목을 집는다(결과의 `held`). 집을 것이 없으면 이유 목록과 함께 2, 상한에 걸리면 3 |
| `stage done <항목> [--next 단계] [--evidence 경로]` | 단계 통과. 기본 다음 단계는 기획→명세, 명세→구현, 구현→검수, 아트→검수다. 명세→구현은 "만들어" 범위 안일 때만 올리고, 밖이면 단계는 명세로 둔 채 상태를 대기로, 증거에 `구현 가능` 을 적고 2. 검수 통과는 `판정 대기`(걸린 것 `판정: 영상`) |
| `block <항목> --on "…"` | 항목 멈춤. `--on` 이 `판정` 으로 시작하면 `판정 대기`, 아니면 `막힘`. 이미 걸려 있던 `결정 #N` 은 지우지 않고 이어 붙인다 |
| `unblock <항목>` | 막힘·판정 대기를 대기로 되돌린다 |
| `done <항목>` | 사용자가 "됐다"고 했을 때. 상태 `끝` |
| `rework <항목> --note "<원문>"` | 판정 뒤 고치기. 단계를 구현으로 되돌리고 증거에 `고칠 것: <원문>` 을 적는다. 같은 원문을 `고칠 것(사용자): <원문>` 으로 brief_notes 에 넣어 다음 `pick` 이 돌려주게 하고, 이 줄은 `done` 때 지운다 |
| `wait <항목> --on "…"` · `unwait <항목>` | 붙든 항목이 빌드·촬영 같은 긴 일을 기다린다는 표시. 상태는 `진행` 그대로, 걸린 것은 `기다림: …` |
| `build <항목> --ok\|--fail [--error "오류 요지"]` | 빌드 결과를 항목 비용과 보고서 비용에 센다. 같은 오류 요지로 연속 실패가 상한에 닿거나 항목당 빌드 횟수가 상한을 넘으면 막힘(3) |
| `inbox add --item N --question "…" --candidates "…" [--candidates "…"] [--blocks "…"] [--branch 양쪽안\|멈춤] [--source "…"] [--no-block]` | 결정 함에 새 항목을 붙이고 그 백로그 항목을 `막힘`(걸린 것 `결정 #번호`)으로 둔다. `--branch` 를 비우면 기획·명세는 양쪽 안, 구현·아트·검수는 멈춤 |
| `inbox answer <번호> "<원문>"` | 답 칸과 답한 날짜를 채운다(채팅으로 들어온 답을 옮길 때) |
| `inbox apply` | 답이 있고 반영되지 않은 항목마다 `C:\Work\CL9\docs\decisions.md` 끝에 `## YYYY-MM-DD (자율 루프 결정 #N)` 절을 **덧붙이고**, 백로그 걸린 것에서 그 결정을 지우고, 걸린 결정이 모두 답해진 `막힘` 항목을 `대기` 로 돌리고, 결정 함 `반영` 칸을 채운다. decisions.md 의 기존 줄은 바꾸지 않는다 |
| `gate record <slug> <단계> --item N --results <JSON 글·.json·.csv>` | 문지기 기록 `C:\Work\CL9\.claude\runs\<slug>\gate-<단계>.md`(slug 는 실행 폴더 밖으로 못 나간다) 에 이번 회차 절을 덧붙이고 판정한다. 0 통과, 1 반려(재시도), 3 반려가 상한에 닿아 막힘. 막힐 때 걸린 것에는 `문지기 반려 N회 — <단계>` 만 적고, 기록 파일 경로는 증거 칸에 둔다 |
| `directive add "<원문>"` · `directive unread` · `directive note "<이렇게 읽었습니다: …>"` | 사장 지시사항 함. unread 는 읽지 않은 줄을 돌려주고 `읽음:` 표시를 붙인다. note 는 마지막 읽은 줄 아래에 조율자 말풍선을 붙인다 |
| `allow <번호…\|전부\|없음>` | 백로그 머리의 `만들어:` 줄을 바꾼다. **사용자가 자기 말로 시킬 때만** 부른다 |
| `report build [--forced] [--video-note 파일이름=설명]` | 사장 보고서를 조립한다. "만든 것"에는 사건 당시의 단계와 그 사건이 만든 증거만 적고, "판정이 필요한 것"에는 영상(.mp4)·이미지(.png/.webp)·sheet.jpg·비교 시트만 적는다. 완성(판정 대기 진입)·사람 자리(집을 것 없음)·상한 정지가 아니면 2. `--forced` 는 지금 시점 보고서("지금 어디야")다 |
| `status [--json] [--no-probe]` | 사무실 페이지용 한 덩어리. 최상위 `items_blocked`(막힘: 번호·제목·단계·걸린 것)와 `items_judging`(판정 대기: 번호·제목·증거), 프로젝트 설명 `project`(이름·루트·실제 경로·탐침·역할 배정·단계 실행기)가 있다. `--no-probe` 는 브로커·잠금을 묻지 않고, project.json 에서 꺼진 탐침은 `"안 씀"` 이다 |
| `heartbeat [--run slug]` | 조율자가 단계마다 부른다. `--run` 은 붙든 항목의 `/cl9 task` 실행 slug 를 적어 역할 상태를 항목에 잇는다. 연속 시간 상한이면 정지(3) |
| `stop --reason "…"` · `resume` | 루프 정지와 해제. resume 은 연속 시간과 실패 집기 횟수를 다시 센다 |

## 집기 규칙 (명세 그대로)

1. 정지 중이면 3. 사용자 답 없이 연속으로 돈 시간이 상한에 닿았거나, 단계를 통과하지 못한 집기가 상한 횟수에 닿았으면 정지하고 3.
2. 명세 단계에 `구현 가능` 으로 둔 항목이 이제 "만들어" 범위 안이면 구현 단계 대기로 올린다.
3. 붙든 항목(`holding`)이 있으면 그 항목의 다음 단계가 먼저다. 다만 막혔다 풀린 항목이 붙든 항목보다 순서가 앞이고 붙든 항목이 단계 경계(대기)에 있으면 그쪽으로 간다.
4. 없으면 상태 `대기` 중에서 순서가 매겨져 있고, 의존 항목이 전부 `끝` 이고, 걸린 것에 미답 결정이 없는 것을 순서·번호 순으로 집는다. `제안`·`막힘`·`판정 대기`·`끝` 은 집지 않는다.

## 파일

| 파일 | 형식 |
|---|---|
| `C:\Work\CL9\docs\plan\backlog.md` | 머리의 `만들어: 12, 15` / `전부` / `없음` 줄과 표 하나. 열은 `번호 · 제목 · 단계 · 순서 · 출처 · 의존 · 상태 · 걸린 것 · 증거 · 비용`. 도구는 바뀐 행만 다시 쓰고 표 밖의 줄과 줄끝(CRLF/LF)은 그대로 둔다. 비용은 `분 35 · 빌드 1 · 반려 0` |
| `C:\Work\CL9\docs\decisions-inbox.md` | 항목마다 `### #<번호> <질문>` 아래 `- 날짜:` · `- 출처:` · `- 후보:`(들여쓴 `  - ① …` 줄, `  - 단서: …` 줄도 된다) · `- 걸린 것:` · `- 미답 분기:` · `- 답:` · `- 답한 날짜:` · `- 반영:` |
| `C:\Work\CL9\.claude\runs\loop\state.json` | `holding`·`last_report`·`cost`·`stopped`·`heartbeat` 에 더해 `side`(기다리는 동안 같이 집은 항목)·`items`(항목별 반려·고치기·빌드 실패 횟수)·`resumed`·`picks_without_pass`·`active_since`·`last_report_at`·`brief_notes` |
| `C:\Work\CL9\.claude\runs\loop\events.jsonl` | 한 줄에 `{"at","kind","item","text"}` 와 필요하면 `stage`·`to`·`record`·`failed`·`decision`. 종류는 `gate_pass`·`gate_reject`·`build_ok`·`build_fail`·`report`·`inbox_add`·`inbox_answer`·`inbox_apply`·`pick`·`stage_done`·`item_blocked`·`item_unblocked`·`item_done`·`rework`·`wait`·`allow`·`stop`·`resume`·`directive` |
| `C:\Work\CL9\.claude\runs\loop\directives.md` | `- [YYYY-MM-DD HH:MM] <원문>` 한 줄씩. 읽으면 `- 읽음: [YYYY-MM-DD HH:MM] <원문>`, 그 아래 `  → 조율자: …` |
| `C:\Work\CL9\.claude\runs\loop\report-<번호>.md` | 사장 보고서. `## 1. 결론 한 줄` 부터 `## 9. 다음에 칠 명령` 까지 아홉 절, 빈 절은 `없음` |
| `C:\Work\CL9\.claude\runs\<slug>\gate-<단계>.md` | `# 문지기 — #N 제목 · 단계` 아래 회차마다 `## N회째 — 통과\|반려 (YYYY-MM-DD HH:MM)` 절을 덧붙인다. 절 안에는 표 `\| 검사 \| 결과(예/아니오) \| 근거 \|` 와 `판정: 통과` 또는 `판정: 반려 (재시도 N회째)` 가 있고, 파일 끝의 판정 줄이 마지막 회차의 것이다. 결과가 오지 않은 검사는 `아니오` 로 적는다 |
| `C:\Work\CL9\.agent-office\gate_checks.csv` | `단계,검사,수단`. 없으면 도구 기본 `C:\Work\Tools\AgentOffice\loop\gate_checks.default.csv`. 명세 문지기 표 그대로이고, 단계 이름은 백로그 단계(기획·명세·구현·아트·검수)를 쓴다. 명령에서 `기획 초안`·`영상` 이라고 써도 된다 |
| `C:\Work\CL9\.agent-office\limits.csv` | `항목,값,단위,넘으면`. 없으면 도구 기본 `limits.default.csv`. 값이 빈칸이면 상한 없음. 단위는 `시간`·`분`·`회`. 항목 이름은 코드가 찾는 열쇠라서 바꾸지 말고 값만 고친다 |
| `C:\Work\Tools\AgentOffice\loop\display.default.csv` | `항목,값,설명`. 최근 사건 수(50)·빌드 기록 줄 수(10)·다음 후보 수(3). CL9 는 자기 표 없이 이 기본을 쓴다(바꾸려면 `C:\Work\CL9\.agent-office\display.csv` 를 둔다) |

## 상한이 걸리는 때

| 상한 | 걸리는 때 | 결과 |
|---|---|---|
| 실행 하나의 벽시계 시간 | 붙든 구현 단계가 그 시간에 닿았을 때(`pick`·`heartbeat` 가 본다) | 그 항목 막힘. 실행의 ABORTED 처리는 조율자가 한다 |
| 사용자 답 없이 연속으로 도는 벽시계 시간 | 마지막 사용자 말(답·지시·됐다·고치기·만들어·resume) 뒤 그 시간에 닿았을 때 | 루프 정지 |
| 같은 오류로 빌드 연속 실패 | 같은 `--error` 요지의 연속 실패가 값에 닿았을 때 | 그 항목 막힘 |
| 한 항목이 완성까지 쓰는 빌드 횟수 | 항목 빌드 횟수가 값을 넘었을 때 | 그 항목 막힘 |
| 같은 항목의 문지기 반려 | 한 단계 안의 반려가 값에 닿았을 때(기본 2 = 재시도 1회 뒤) | 그 항목 막힘 |
| 연속으로 아무 단계도 통과하지 못한 집기 | 마지막 `stage done` 뒤의 집기 횟수가 값에 닿았을 때 | 루프 정지 |
| 같은 항목이 사용자 판정 뒤 고치기로 다시 돈 횟수 | 고치기 횟수가 값을 넘었을 때 | 그 항목 막힘 |
| 토큰·돈 | 측정 수단이 없어 보지 않는다 | — |
| 응답 없음 판정 | 일하는 중이어야 할 역할의 touch 파일이 그 시간 넘게 갱신되지 않았을 때 | 사무실 페이지에 응답 없음 |

## 역할 상태 판정 (`status`)

- 제작 역할(gameplay·ui·map·art·balance·docs, `probes.cl9_runs` 가 켜진 CL9 만): state 가 `DISPATCHED` 인 실행의 `run.json` roles 에 없으면 쉬는 중. 있으면 그 실행의 항목이 결정 함 미답에 걸렸을 때 막힘, 빌드 브로커 큐에 그 역할 이름이나 실행 slug 가 사유에 든 티켓이 있으면 기다리는 중, `touch-<역할>` 이 N분 안에 바뀌었으면 일하는 중, 아니면 응답 없음이다. 실행의 항목은 `run.json` 의 `item` 열쇠나 `heartbeat --run` 으로 적은 slug 로 안다.
- 조율자: 붙든 항목이 없거나 정지면 쉬는 중, 붙든 항목이 기다리는 중이면 기다리는 중, heartbeat 가 N분 안이면 일하는 중, 아니면 응답 없음. 상태 값과 별개로, 명세 단계에서 막힌 항목은 `detail` 에 `#N 명세 — 결정 #M 대기` 로, 판정 대기 항목은 `#N 판정 대기` 로 덧붙는다.
- 문지기: 가장 최근 `gate-*.md` 가 N분 안에 쓰였으면 일하는 중, 아니면 쉬는 중.
- 기획자: 백로그의 기획 단계 항목 상태(진행 = 일하는 중, 막힘 = 막힘, 그 밖 = 쉬는 중).
- 촬영 잠금(`probes.cl9_lock`): `C:\Work\CL9\.claude\hooks\cl9_lock.py status --json` 을 읽고, 배타 잠금 목적이 `pie` 면 `filming: true`. 읽지 못하면 `"모름"`.

## 시험

```
py -3 C:\Work\Tools\AgentOffice\loop\test_loop.py
```

임시 폴더에 가짜 백로그·결정 함·decisions.md 를 만들어 명세 "검증 1겹"의 항목(제안 안 집힘, 의존 미완 안 집힘, 막힘 건너뜀, 답 오면 다시 집힘, 붙든 항목 우선, 명세→구현은 만들어 범위만, 답 반영의 append 와 막힘 해제, 반려 2회 막힘, 상한 정지, 보고서 아홉 항목과 "없음", 집을 것 없으면 2)과 기다림·만들어를 돌린다. `projects.json` 에 등록된 프로젝트(CL9)의 실물 백로그·결정 함을 사본으로 읽고 바이트 그대로 다시 쓰는지도 보고, project.json 이 없는 새 프로젝트에서 파일이 `.agent-office\` 안에만 생기는지·루트 밖 경로가 거부되는지도 본다.

## 사무실 서버로 부르기

사무실 페이지는 공용 사무실 서버(`127.0.0.1:8770`)의 `/api/CL9/…` 를 부르고, 서버는 이 도구를 `--root C:\Work\CL9 --json` 서브프로세스로 불러 그 JSON 을 그대로 돌려준다(백로그 순서 매기기만 도구에 명령이 없어 서버가 `순서` 칸을 직접 고친다).
경로 목록과 오류 규칙은 [C:\Work\Tools\AgentOffice\README.md](C:\Work\Tools\AgentOffice\README.md)의 "실행" 절에 있다. 데이터 패널(8766)의 "사무실" 탭은 이 서버를 틀 안에 연다.

## 흐름 검사 돌리기

`py -3 C:\Work\Tools\AgentOffice\loop\flow_check.py` 는 기획·명세·구현·검수를 가짜 실행기(`C:\Work\Tools\AgentOffice\loop\fake_stages.py`)로 바꾸고 조율자 step 절차를 프로세스 단위로 흉내 내어, 임시 루트에서 백로그 항목 셋을 끝까지 돌리며 넘김·막힘·되돌아옴·보고서 시점·세션 끊김 복구·상한을 단언한다(`loop.py` 는 서브프로세스로만 부른다).
`--trace <경로>` 를 주면 명령·종료 코드·백로그 상태를 한 줄씩 적은 흐름 기록을 그 파일에 쓰고(예: `--trace C:\Work\CL9\.claude\runs\loop-build\flow-trace.md`), `--keep` 은 임시 루트를 남긴다. 시험 전후로 등록된 실제 프로젝트의 루프 파일 지문이 같은지도 본다.
