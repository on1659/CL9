# goal: editor-ops — 운영 담당(에디터 자리·잠금·데이터 패널 조율)

> **Status**: 명세 초안(2026-10-18). 사용자 답 둘("둘 다", "실제로 조율해야되는데 명세로 정리하고 진행하자")을 받고 쓴 문서다.
> 미결 질문에 답이 오면 구현으로 간다. 부모 명세: [autonomous-production-loop.md](C:\Work\CL9\docs\goal\autonomous-production-loop.md).
> 잠금 규약 정본: [sessions.md 잠금 절](C:\Work\CL9\docs\workflow\sessions.md). 데이터 패널 정본: [data-panel.md](C:\Work\CL9\docs\systems\data-panel.md).

## 한 줄 목표

에디터 하나를 두고 다투는 네 손(루프의 제작·검수 단계, 사용자의 수동 세션, 데이터 패널, 빌드 브로커)에 **차례를 정해 주는 자리**를 루프에 더하고,
그 차례·잠금·임대·데이터 패널 상태를 **사무실 페이지 한 곳**에서 보고 만지게 한다.

## 배경 — 지금은 무엇이 따로 노나

| 부품 | 지금 | 틈 |
|---|---|---|
| 세션 잠금 `C:\Work\CL9\.claude\hooks\cl9_lock.py` | 배타 잠금(build·compile·pie·commandlet) 하나 + 공유 임대(editor) 목록을 `.claude\runs\editor.lock` 에 둔다. 훅이 파괴 명령·PIE 를 잠금 보유자에게만 허용한다 | 잠금은 "지금 누가 쥐었나"만 안다. **다음은 누구인가**가 없다 — 막히면 `--wait 90초` 뒤 "busy", 각 세션이 알아서 다시 친다 |
| 데이터 패널 `http://127.0.0.1:8766` | 표 20개를 브라우저에서 고치고 저장한다. 잠금 쥔 세션을 **문자열 한 줄로 알릴 뿐** 쓰기·저장을 막지 않는다(2026-10-17 "잠금은 알림일 뿐, 판단은 사용자의 몫") | 다른 세션이 촬영 중인 PIE 에 값이 들어가 영상에 찍힌다. 패널이 임대를 안 잡아, 사용자가 패널로 고치는 중에도 다른 세션이 에디터를 끌 수 있다 |
| 사무실 `http://127.0.0.1:8770` | `probes.cl9_lock` 로 `status --json` 전체(`exclusive`·`leases`·`guard_off_until`)를 받지만 화면은 `filming`(pie 잠금 여부) 하나만 쓴다 | 임대·보유 세션·가드·만료가 안 보이고, 바꿀 수도 없다. 데이터 패널로 가는 길이 없다(반대 방향, 패널 → 사무실 탭만 있다) |
| 루프 `/office step` | 구현·검수 단계의 잠금은 `/cl9 task`·`/cl9film` 이 각자 잡는다 | 두 단계가 에디터를 두고 사용자·패널과 부딪히면 그 단계가 실패로 끝나고, 아무도 순서를 매기지 않는다 |

## 사용자 결정 (2026-10-18)

1. "같이 관리"는 **둘 다** — 사무실 페이지에 붙이고, 루프에 역할도 더한다.
2. 담당자는 **실제로 조율**한다. 상태만 보여 주는 자리가 아니다. 명세로 정리한 뒤 진행한다.

## 설계

### 1. 운영 담당 (루프의 새 역할)

이름은 **운영 담당**(`project.json` 의 `roles` 에 `kind: "ops"`, 이름·동물은 표 값이라 바꾸면 된다). 하는 일은 네 가지다.

| 일 | 언제 | 무엇으로 |
|---|---|---|
| 에디터 자리 줄 관리 | 매 `step` 의 0단계(전제) 다음 | 줄 파일을 읽어 죽은 항목(세션·프로세스 둘 다 없음, 만료 지남)을 지우고, 머리 항목에 차례를 준다 |
| 단계 앞뒤의 줄 서기·나가기 | 구현·검수(·아트의 UE 왕복) 단계를 집었을 때와 끝났을 때 | `queue join` / `queue leave`. 머리가 아니면 `loop.py wait <항목> --on "에디터 자리 줄 N번째"` 로 두고 다른 항목을 집는다 |
| 죽은 잠금·임대 정리 | 매 step | 기록된 프로세스가 죽었고 세션도 없으면 `release --force`(도구가 산 프로세스는 거부하므로 안전). 임대는 만료로 스스로 빠진다 |
| 충돌 보고 | 사람 판정이 필요할 때 | 산 프로세스가 상한을 넘겨 쥐고 있음, 가드가 꺼져 있음, 패널 저장이 막혔는데 사용자가 "그래도" 를 눌렀음 — 이런 것은 소식 피드 한 줄 + 보고서 4 항목(결정이 필요한 것) |

안 하는 일: 잠금을 대신 잡아 주지 않는다(잠금은 쓰는 세션이 자기 훅으로 잡는다 — 규약 그대로). 빌드 순서는 브로커 몫이다. 값을 고치지 않는다.

운영 담당은 **LLM 역할이 아니라 절차**다. 기계 부분(줄 파일, 정리, 상태 판정)은 `cl9_lock.py` 와 `loop.py` 가 하고, `/office step` 의 조율자가 그 절차를 부른다.
사무실에는 동물 한 명으로 보이고 상태는 이렇게 판정한다 — 줄이 비고 배타 잠금이 없으면 **쉬는 중**, 줄이나 잠금이 있으면 **일하는 중**, 사람 판정 항목이 있으면 **막힘**.

### 2. 에디터 자리 줄 (`cl9_lock.py` 확장)

파일: `C:\Work\CL9\.claude\runs\editor-queue.json` — `{"entries": [{"id", "who", "purpose", "item", "minutes", "since", "expires"}]}`.
`who` 는 세션 ID(훅이 찍음), 또는 고정 ID `panel`·`broker`·`user`(사무실 페이지에서 사용자가 선 줄).

```
py -3 .claude/hooks/cl9_lock.py queue join <pie|build|compile|commandlet|panel> [--item N] [--minutes M]
py -3 .claude/hooks/cl9_lock.py queue leave [--id X]
py -3 .claude/hooks/cl9_lock.py queue list [--json]
py -3 .claude/hooks/cl9_lock.py queue sweep        # 죽은 항목 정리 (운영 담당이 매 step 부른다)
```

- `acquire` 가 줄 서기를 겸한다: 먼저 `sweep`(만료·죽은 항목 제거) → 내가 줄에 없으면 끝에 선다(`since` 지금) → **머리가 나이고** 기존 규칙(산 배타 잠금 없음·남의 임대 없음)을 통과하면 잡고 줄에서 빠진다 → 아니면 거부("busy: 줄 머리는 <who>·<purpose>, 내 차례는 N번째"). 줄이 비면 지금과 똑같이 동작한다 — 기존 세션·스크립트는 그대로다.
- 머리 판정: `who` 가 `user`·`panel` 인 항목이 먼저(사용자 손), 그 안에서는 `since` 순. 나머지는 `since` 순.
- 머리 만료: 머리가 된 뒤 `queue_head_minutes`(기본 10분) 안에 잠금을 안 잡으면 `sweep` 이 지운다(`head_since` 필드). 세션 ID 항목은 그 세션의 훅이 남긴 표식·프로세스가 없으면 바로 지운다. `release` 뒤 다음 머리의 `head_since` 가 지금이 된다.
- `status --json` 에 `queue`(항목 배열, 머리부터)와 `values`(읽은 조절 값)를 더한다. 사무실·데이터 패널은 이 키로 읽는다(새 읽기 경로를 만들지 않는다).
- 조절 값은 `C:\Work\CL9\.claude\hooks\lock_values.csv`(`queue_head_minutes`, `panel_lease_minutes`)에 두고 `cl9_lock.py` 가 읽는다. 없으면 기본값.
- 고정 ID: `broker`(브로커, 있음)·`panel`(데이터 패널)·`user`(사무실에서 사장이 직접). 이 셋은 `--session <id>` 로 쓰고, 규약에 적는다.

### 3. 데이터 패널 연결

- **임대**: 패널이 `write`·`save` 마다 고정 ID `panel` 로 `editor` 임대를 받거나 갱신한다(`panel_lease_minutes`, 기본 5분 — 마지막 쓰기 뒤 5분이면 빠져 촬영을 오래 막지 않는다). 임대가 살아 있는 동안 다른 세션은 배타 잠금을 못 잡는다 — 브로커가 `broker` ID 로 하는 것과 같은 방식이다. 서버를 끄면 임대를 놓는다.
- **막기**(답 1): 다른 보유자의 배타 잠금(pie·build·compile·commandlet)이 살아 있으면 `write`·`save` 를 거부하고 보유자·용도·만료를 돌려준다(HTTP 423, 본문에 사유). 페이지는 칸을 잠그고 머리글에 "촬영 중 — 세션 xxxx, 12:40 까지" 를 띄운다. 사용자가 머리글의 **"그래도 쓰기"** 를 켜면 요청에 `force: true` 가 붙어 막기를 건너뛰고, 서버가 `.claude\runs\editor-queue.json` 의 `overrides` 에 `{time, table, holder}` 를 남겨 운영 담당이 보고서 4 항목에 올린다.
- `/api/status` 에 `lock` 문자열 옆에 `lock_json`(`status --json` 그대로)을 더한다. 머리글에 보유자·임대 수·내 차례를 짧게 보인다.
- 사무실에 끼울 때 머리글이 이중이 되므로 `?embed=1` 이면 패널 머리글의 브랜드·테마 탭 줄을 숨기고 표 탭부터 그린다. `tables.json` 의 `office` 행(패널 → 사무실 탭)은 그대로 둔다 — 그 안에서 다시 패널을 열면 중첩되니 `embed=1` 일 때는 `office` 행을 안 그린다.

### 4. 사무실 페이지

- 오른쪽 탭에 **에디터실** 과 **데이터** 두 탭을 더한다(`TABS`·`showTab`·HTML 세 곳 — 지금 탭은 코드 고정이다).
- **에디터실** 탭: 배타 잠금(용도·세션 앞 8자·프로세스 생사·만료), 임대 목록(세션·만료, `panel`·`broker` 는 이름으로), 가드(켜짐/꺼짐·언제까지), **줄**(순서·who·purpose·항목·선 시각), 패널 서버 생사(8766 `/api/status` 를 사무실 서버가 대신 읽음: 에디터 연결·PIE·저장 안 한 칸 수).
  단추는 CLI 가 할 수 있는 것 전부(답 3) — **잠금 잡기**(`--session user`, 용도 pie·compile 고르기, 분), **놓기**, **죽은 잠금 뺏기**(`release --force`; 산 프로세스는 도구가 거부하므로 단추가 실패 사유를 그대로 보인다), **가드 끄기(분)/켜기**, **줄 서기**(`who=user`, 용도), **줄 나가기**, **줄 정리**(`queue sweep`). 모든 단추는 도구의 거부 사유를 그대로 띄운다 — 사무실이 도구보다 더 할 수 있는 것은 없다.
  사무실 서버 API: `GET /api/<프로젝트>/editor`(`status --json` + 패널 `/api/status`), `POST /api/<프로젝트>/lock {op: acquire|release|force|guard_off|guard_on|queue_join|queue_leave|queue_sweep, purpose?, minutes?}` — 서버는 `cl9_lock.py --root <root> --session user <명령>` 을 서브프로세스로 부른다. 프로젝트의 `probes.cl9_lock` 이 꺼져 있으면 탭은 "안 씀".
- **데이터** 탭: `<iframe src="http://127.0.0.1:8766/?embed=1">`. 주소(포트)는 `project.json` 의 `data_panel_url` 로 두고, 없으면 탭을 안 그린다. 패널 서버가 안 떠 있으면 "데이터 패널이 꺼져 있다 — `py -3 Tools/DataPanel/data_panel.py serve`" 한 줄.
- 맵의 촬영실 ON AIR 는 그대로이고, 운영 담당 동물이 촬영실 옆 자리에 앉는다(자리는 `project.json` `roles` 의 `desk`·`seat`, 비우면 `spare`).
- 폴링: 에디터실 탭이 보일 때만 `editor` 를 읽는다(사무실 `status` 5초 폴링과 합쳐 서브프로세스 부하를 두 배로 만들지 않는다 — `status` 의 `film_lock` 에 이미 `queue` 가 실리므로 에디터실 탭은 그것을 쓰고, 패널 상태만 따로 읽는다).

#### 4절 덧붙임 — 에디터 켜기·데이터 탭 넓게 보기·테마 (2026-10-18, 사용자 "이거 데이터도 탭으로 빼서 컨트롤할 수 있게 하고, 디자인도 우리 거랑 통일해야 되고, 데이터 컨트롤은 에디터 꺼져 있으면 안 되니까 켜게 하는 것도 연결해야 돼")

- **에디터 켜기** — 명령과 준비 판정은 `project.json` 의 `editor` 절에 데이터로 둔다: `start_command`(CL9 는 `py -3 Tools/Build/build_ctl.py editor start`)·`status_command`(`… editor status` → `{"running", "pids", "mcp"}`)·`ready_probe`(`{"kind": "tcp", "port": 8000}`)·`ready_timeout_s`(240). 절이 없는 프로젝트는 "에디터 켜기를 안 씀"이다.
  2026-10-18: 포트를 8000 에서 설정값(기본 8100)으로 바꿨다. 이유는 8000 이 다른 앱과 겹쳤기 때문이다. 정본은 `C:\Work\CL9\Config\DefaultEditorPerProjectUserSettings.ini` 의 `ServerPortNumber` 이고, `ready_probe` 는 JSON 이라 값을 읽어 오지 못하므로 `{"kind": "tcp", "port": 8100, "note": …}` 로 손으로 맞춘다.
  `build_ctl.py editor start` 는 브로커와 같은 [editor_proc.py](C:\Work\CL9\Tools\Build\editor_proc.py) 한 곳(같은 `EDITOR_EXE`·`C.PROJECT`·분리 실행)으로 띄운다. 이미 떠 있으면 "이미 떠 있다"(0), 살아 있는 **build** 잠금이 있으면 "거부: 빌드 중 — 끝나면 켜진다"(1), 잠금 파일을 못 읽으면 거부(1)다. 남의 임대·PIE·compile 잠금은 켜기를 막지 않는다.
  사무실 서버 `POST /api/<이름>/editor/start` 가 그 명령을 셸 없이 루트에서 돌려 출력을 그대로 돌려주고, `status`·`editor` 응답의 `editor` 칸(`running`·`mcp`·`starting_since`·`error`)으로 상태를 낸다. 켜기를 받고 `ready_timeout_s` 안에 준비되지 않으면 "켜기 실패(시간 초과)"다. 상태 명령은 `server.json` `editor_status_ttl_s`(10초)마다 한 번만 돈다.
- **화면** — 에디터실 탭 맨 위 "에디터" 칸(꺼짐 / 켜는 중 N초 / 켜짐·MCP 준비됨 / 켜짐·MCP 아직, 꺼짐·실패일 때만 "에디터 켜기"), 데이터 탭의 띠("에디터가 꺼져 있어 값을 못 바꿔요 — [에디터 켜기]"), 맵 촬영실의 에디터 램프(꺼짐 어둡게·켜는 중 깜빡·켜짐 초록. 맵에 에디터실이 따로 없어 에디터를 쓰는 촬영실에 둔다, 자리는 `office.config.json` `edlamp`).
- **패널 켜기** — 데이터 패널이 꺼져 있으면 데이터 탭에 `project.json` `data_panel_start`(`{"command": "py -3 Tools/DataPanel/data_panel.py serve", "detach": true}`)의 "패널 켜기" 단추. 서버가 분리 실행으로 띄우고 바로 돌아온다(`POST /api/<이름>/panel/start`).
- **넓게 보기** — 데이터 탭 머리의 "넓게 보기" 를 누르면 가운데 상위 탭에 "데이터" 가 생겨 맵 자리에 패널이 넓게 뜨고, 다시 누르면 사이드바로 돌아온다. 패널 틀은 하나만 만들어 자리 위에 겹쳐 두므로(틀을 옮기면 패널을 처음부터 다시 읽어 보던 표를 잃는다) 오가도 보던 표가 그대로다.
- **테마** — 데이터 패널의 기본 테마가 사무실 팔레트가 됐고, 예전 밤 남색은 밤 테마(`?theme=night`)다. 사무실이 밤이면 틀 주소에 `&theme=night` 를 붙이고, 시각이 바뀌면 `postMessage` 로 테마만 바꾼다. 사무실 `base.css` 와 패널 `panel.css` 는 맨 위 "공용 바탕" 덩어리(변수 이름·값·머리글·알약·단추·입력·알림)를 글자 그대로 같이 쓴다 — 두 서버가 따로 떠서 한 파일을 가져다 쓰지 못하므로 같은 덩어리를 두 곳에 두고, 고칠 때는 같이 고친다.

### 5. 루프 연결 (`/office step`·`loop.py`·`project.json`)

- `step` 0단계 뒤에 **운영 절차** 한 줄: `cl9_lock.py queue sweep` → `status --json` 의 `queue`·`exclusive`·`overrides` 로 운영 담당 상태를 `loop.py` 가 판정(새 kind `ops`). 사람 판정 항목(상한 넘긴 산 잠금·가드 꺼짐·"그래도 쓰기" 기록)이 있으면 보고서 4 항목에 적는다.
- 구현·검수 단계의 실행기(`/cl9 task`·`/cl9film`)가 `acquire` 하면 그것이 곧 줄 서기다. 거부 사유가 "busy: 줄 …" 이면 조율자가 `loop.py wait N --on "에디터 자리 줄 k번째"` 로 두고 다음 항목을 집는다. 단계가 끝나 `release` 하면 다음 머리에게 차례가 간다. 조율자가 따로 `queue join` 을 치는 것은 자리를 미리 받아 두고 싶을 때뿐이다.
- `status.py` 의 역할 상태에 `ops` 판정을 더한다(1 절의 세 상태). `README.md` 의 kind 목록에 `ops` 를 적는다.
- `project.json`: `roles` 에 운영 담당 한 줄, `data_panel_url: "http://127.0.0.1:8766"`.

## 사람이 맡는 자리

| 자리 | 어떻게 |
|---|---|
| 산 프로세스가 상한(1시간)을 넘겨 쥔 잠금 | 도구는 뺏지 않는다. 보고서 4 항목에 올리고 사용자가 그 세션에 말하거나 손으로 끈다 |
| 패널 막기의 "그래도 쓰기" | 사용자만 켠다. 켠 사실은 기록되고 보고서에 오른다 |
| 줄의 순서 | 사용자 손(`user`·`panel`)은 늘 앞이다. 루프 항목끼리의 순서를 바꾸는 것은 사용자가 채팅에서 말하면 조율자가 `queue leave`/`join` 으로 옮긴다 |
| 가드 끄기·죽은 잠금 뺏기 | 사무실 단추(사용자 손) 또는 CLI. 둘 다 같은 도구를 부르고 같은 거부를 받는다 |

## 범위 밖

- 빌드 순서(브로커 몫), Blender 와 Unreal 의 GPU 순서(루프 명세 "긴 단계 규칙" 그대로).
- 훅이 없는 세션(비활성)을 줄에 세우는 것 — 그 세션은 재시작이 답이다.
- 사무실에서 데이터 값을 고치는 별도 UI — 데이터 탭은 패널을 그대로 보일 뿐이다.
- 다른 프로젝트 — `probes.cl9_lock` 이 꺼진 프로젝트는 에디터실·데이터 탭이 "안 씀" 이다.

## 검증 기준

1. `test_cl9_lock.py` 기존 검사 전부 통과 + 새 검사: 줄이 비면 `acquire` 가 지금과 같다 · 머리가 아닌 세션의 `acquire` 거부 · 머리가 잡으면 줄에서 빠짐 · `sweep` 이 죽은 항목만 지움 · 만료 항목 제거.
2. 패널: 다른 ID 의 `pie` 잠금을 파일에 넣고 `write` → 거부(미결 1 이 막기일 때) · `panel` 임대가 `editor.lock` `leases` 에 생기고 갱신됨 · 서버 종료 때 빠짐.
3. 사무실: `test_office_server.py` 통과 + `GET /api/CL9/editor` 가 잠금·줄·패널 상태를 돌려줌 · `POST queue join/leave/sweep` 이 줄 파일을 바꿈 · 에디터실·데이터 탭이 그려짐(브라우저로 확인, 스크린샷).
4. 루프: `loop.py status --json` 의 `roles` 에 운영 담당이 세 상태로 나옴. `/office step` 이 구현 단계 앞에서 `queue join` 하고 머리가 아니면 `wait` 로 두는 것은 훅이 활성인 세션에서만 확인할 수 있다(이 세션은 비활성이라 그 확인은 남긴다).

## 관련 파일

| 파일 | 바꾸는 것 |
|---|---|
| `C:\Work\CL9\.claude\hooks\cl9_lock.py` · `test_cl9_lock.py` · 새 `lock_values.csv` | `queue` 명령, `acquire` 의 줄 규칙, `status --json` 의 `queue` |
| `C:\Work\CL9\.claude\hooks\cl9_scope_guard.py` | `queue` 하위 명령을 훅이 세션 ID 를 찍어 중재(지금 `acquire`·`release` 처럼) |
| `C:\Work\CL9\Tools\DataPanel\data_panel.py` · `pages\*.html` 의 공용 JS/CSS | `panel` 임대, 막기, `lock_json`, `embed=1` |
| `C:\Work\Tools\AgentOffice\office\office_server.py` · `page\office.html`·`office.js`·`office.css` · `test_office_server.py` | `editor`·`queue` API, 에디터실·데이터 탭 |
| `C:\Work\Tools\AgentOffice\loop\status.py` · `README.md` · `skills\office\SKILL.md` | `ops` 역할 상태, step 의 운영 절차 |
| `C:\Work\CL9\.agent-office\project.json` | 운영 담당 역할 줄, `data_panel_url` |
| `C:\Work\CL9\docs\workflow\sessions.md` · `docs\systems\data-panel.md` · `docs\decisions.md` | 줄 규칙 한 절, 패널 막기(미결 1 의 답), 결정 기록 |

## 미결이었던 것의 답 (2026-10-18, 사용자)

1. **패널 막기** — 사용자: "이거 조율해야지 앞으로 잠금은 여기내에서 이루워질꺼임". → **막는다**(2026-10-17 "알림일 뿐" 번복, `decisions.md` 에 번복으로 기록). 패널도 줄의 한 손이고, 잠금의 조작은 앞으로 사무실(운영 담당) 안에서 한다.
   "그래도 쓰기" 스위치는 남긴다 — 사장의 손이므로. 켠 사실은 기록되어 보고서에 오른다.
2. **줄 순서** — 사용자: "제일효율적인방법". → **사용자 손(`user`·`panel`)이 먼저, 그 뒤는 선 순서.** 이유: 루프는 차례를 기다리는 동안 다른 항목을 집을 수 있지만 사용자는 못 기다린다. 같은 이유로 패널 임대는 마지막 쓰기 뒤 5분으로 짧게 둬 촬영을 오래 막지 않는다. 줄 머리가 10분 안에 잠금을 안 잡으면 다음으로 넘긴다.
   `acquire` 가 **줄 서기를 겸한다**(줄에 없으면 끝에 서고, 머리면 바로 잡는다) — `/cl9 task`·`/cl9film` 의 잠금 코드는 바꾸지 않는다.
3. **사무실 단추** — 사용자: "할수잇는거다". → **CLI 가 할 수 있는 것 전부**: 잠금 잡기(`user` 로, 용도 고르기)·놓기·죽은 잠금 뺏기(`--force`, 산 프로세스는 도구가 거부)·가드 끄기/켜기·줄 서기/나가기/정리. 규약(`sessions.md`)에 사무실을 잠금의 정식 창구로 적고, 고정 ID `user` 를 더한다.

기본값으로 둔 것: 역할 이름 "운영 담당"·동물은 `spare` 그림, 줄 머리 만료 10분(`queue_head_minutes`), 패널 임대 5분(`panel_lease_minutes`), 에디터실 폴링은 탭이 보일 때만. 전부 `lock_values.csv` 와 `project.json` 의 표 값이다.
