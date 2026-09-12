# 세션 조율 시스템(/cl9) — 실행하면 이렇게 돈다

2026-09-12 · 정본은 [session-orchestration.md](session-orchestration.md)(영문 계획 + 리뷰 기록)이고,
이 문서는 그 계획이 **실제로 실행될 때 사용자 눈에 어떻게 보이는지**를 시나리오로 풀어 쓴 것이다.
아직 구현된 것은 없다. 아래의 명령·메시지·파일은 계획이 약속한 모양이다.

## 0. 무엇이 생기나

| 파일 | 역할 |
|---|---|
| `.claude/cl9-roles.json` | 역할 여섯 개(gameplay·ui·map·art·docs·balance)가 각각 어느 폴더를 소유하고, 어느 문서를 정본으로 읽고, 어느 PIE 절을 확인하는지 적은 표. 모든 것의 원본. |
| `.claude/agents/cl9-*.md` | 위 표에서 **렌더된** 역할 에이전트 여섯 개. 서브에이전트로 띄울 때의 시스템 프롬프트. |
| `.claude/hooks/cl9_scope_guard.py` | 모든 세션의 모든 편집·셸·MCP 호출 앞에서 도는 훅. 역할 에이전트를 자기 영역에 가두고, **에디터/빌드 락**을 모두에게 적용한다. |
| `.claude/hooks/cl9_lock.py` | 락 명령(`acquire / release / status / guard`). |
| `.claude/skills/cl9/SKILL.md` | `/cl9` 스킬. 작업 분할·디스패치·통합, 역할 계약 출력, 상태, 재개, 인계, 커밋. |
| `Tools/Balance/check_inequalities.py` | 기획 표를 파싱해 A1/A2 부등식을 계산하는 스크립트. 밸런스 역할의 검증 도구. |
| `Tools/Unreal/apply_table_rows.py` | 밸런스 보고서의 표 행 변경을 에디터 닫힌 상태에서 적용하는 커맨드릿 스크립트. |
| `docs/workflow/sessions.md` | 사람이 읽는 규약. 역할 표, 락 규칙, 브리프·보고서 양식, 막혔을 때의 절차. |
| `.claude/runs/<slug>/` | 실행 하나의 흔적(브리프·grants·보고서·로그·run.json). git에 올리지 않는다. |

## 1. 쓰는 방법은 둘이다

**수동 모드** — 지금처럼 터미널을 여러 개 열되, 계약을 손으로 치지 않는다. 각 터미널에서
`/cl9 role ui`처럼 역할을 고르면 계약이 출력되고 그 세션이 그 역할을 맡는다.

**자동 모드** — 터미널 하나에서 `/cl9 <작업>`을 치면 그 세션이 **리드**가 되어 작업을 역할별로
나누고, 에디터가 필요 없는 부분은 역할 서브에이전트로 병렬 실행하고, 에디터·빌드·PIE는 리드가
한 번에 처리한다.

두 모드는 같은 레지스트리·같은 훅·같은 락을 쓴다. 자동 모드의 리드도 `ListAgents`에 보이는 세션
하나일 뿐이라, 수동 세션과 같은 규칙으로 에디터를 나눠 쓴다.

## 2. 시나리오 A — 수동 모드로 UI 작업

```
> /cl9 role ui
```

세션이 이렇게 답한다.

```
이 세션은 UI/UMG 담당이다.
소유: Source/CL9/UI/*, Content/Strings/*, Config/Localization/*, Tools/Loc/*, docs/systems/{ui,ui_system,input,settings,localization}.md
공유(보고서에 적을 것): Source/CL9/GameMode/CL9PlayerController.* (입력 바인딩), Source/CL9/Character/* (입력만)
정본: docs/systems/ui_system.md · ui.md · input.md
PIE: G, I 절
지금 다른 세션: cl9-2d(gameplay, HoldPose 작업 중) · cl9-37(map, 조명 표)
에디터 리스: 이 세션이 MCP를 쓰면 자동으로 `editor` 리스를 잡는다 (30분, 호출마다 갱신)
빌드·PIE·에디터 종료는 `py -3 .claude/hooks/cl9_lock.py acquire build --pid <빌드 PID>` 뒤에만 된다.
```

작업하다가 C++를 고쳐서 빌드가 필요해졌다. 예전이라면 `SendMessage`로 "끄겠다"고 알리고
2분을 기다렸다. 이제는 먼저 락을 잡는다.

```
> py -3 .claude/hooks/cl9_lock.py acquire build --pid 18832
```

훅이 이 명령을 보고 **이 세션의 session_id를 소유자로 찍어** 락을 만든다. 다른 세션(cl9-2d)이
MCP로 에디터를 만지고 있어 `editor` 리스를 들고 있으면, 그 리스가 풀리거나 만료될 때까지
기다린다(최대 90초, 그 뒤엔 "busy"). 그 사이 `SendMessage`로 "저장할 것 있으면 저장해라"는 인사는
그대로 보낸다 — 락은 *안 된다*를 말하고, 메시지는 *왜·언제*를 말한다.

락을 잡지 않은 채 `taskkill /IM UnrealEditor.exe`를 치면 훅이 막는다.

```
[cl9] 에디터 종료·빌드·PIE는 락을 잡은 세션만 할 수 있다. 지금 소유자: 없음 —
`py -3 .claude/hooks/cl9_lock.py acquire build --pid <PID>` 먼저. docs/workflow/sessions.md#잠금
```

다른 세션이 락을 들고 있으면 이렇게 나온다.

```
[cl9] cl9-2d 세션이 빌드 중(PID 18832 살아 있음, 15:05까지) — 끝나면 상대가 release,
죽었으면 `release --force`(살아 있는 빌드가 있으면 force도 거부된다). docs/workflow/sessions.md#잠금
```

`save_assets`는 절대 막지 않는다. "에디터가 곧 내려간다"는 말을 들은 세션이 저장은 할 수 있어야
하기 때문이다.

빌드가 끝나면 `release`. 커밋할 때는 `/cl9 commit ui` — 이 세션이 만진 파일만 스테이징하고
한글 커밋 메시지 초안을 보여 준 뒤, 사용자가 "그래"라고 해야 커밋한다.

## 3. 시나리오 B — 자동 모드로 작업 하나 던지기

```
> /cl9 새 아이템 '호박' — 줍고, 요리하고, 팔 수 있게
```

### 3-1. 인테이크 (물어볼 것은 지금 다 묻는다)

리드가 `docs/plan/build-order.md` 진행 상태, `memory/work-progress.md`, `ListAgents`, 락 상태를
읽고, 모호한 것을 **하나씩** 묻는다. 서브에이전트는 나중에 질문할 수 없으니 여기서 끝낸다.

```
Q1. 호박은 익혀야 요리인가(계란처럼 상태 변화) 아니면 그대로 납품하는 재료인가?
Q2. 모델은 새로 만드나(art), 기존 Dreamscape 메시를 쓰나?
```

새 시스템급 작업이면 여기서 `/autogoal`로 넘겨 `docs/goal/<slug>.md`를 먼저 만들고 그걸로 나눈다.

### 3-2. 분할표 (파일 단위로)

```
역할      | 할 일                                        | 파일(grants)                                   | 필요한 것            | 빌드 | PIE
gameplay  | ItemTable 행 스키마·요리 상태 전이·UseAction  | Source/CL9/Items/CL9ItemDef.*, docs/plan/data/item/pumpkin.md(신규) | —          | 예   | B, M
balance   | 가격·무게·요리 시간 값                        | docs/plan/economy.md, docs/plan/data/item/pumpkin.md(수치 셀) | gameplay: 컬럼 이름·단위 | 아니오 | K, P
ui        | 스캔 라벨·HUD 슬롯에 새 아이템 표시 확인       | Source/CL9/UI/CL9ScanLabel.*                    | gameplay: GetDisplayKind() | 예 | G, R
art       | 호박 메시 + 세 가지 익힘 상태                 | ArtSource/item/pumpkin/*, Tools/Blender/build_pumpkin.py, Tools/Unreal/import_pumpkin.py | — | 아니오 | —
docs      | item/common.md 표에 행 추가, build-order 진행 | docs/plan/data/item/common.md, docs/plan/build-order.md | —          | 아니오 | —
표 행 요청(리드가 적용): ItemTable.pumpkin 신규 행, ItemTable.pumpkin.cooked 행
공유 파일 배정: docs/plan/data/item/pumpkin.md → 본문은 gameplay, 수치 셀은 balance (실행마다 한쪽에만 grant)
```

```
A) 서브에이전트로 실행  B) 분할 수정  C) 수동 터미널용 계약만 출력  D) 취소
권고: art·docs·balance·gameplay는 A. ui는 라벨을 눈으로 봐야 하니 C — 지금 열려 있는 UI 세션이 있으면 그쪽에 넘긴다.
```

역할이 하나뿐인 작업이면 이 질문은 건너뛰고 바로 실행한다.

### 3-3. 디스패치

리드가 `.claude/runs/pumpkin-item/` 아래에 역할별 브리프, `grants.json`(역할별 허용 파일),
`run.json`(상태 기계: INTAKE → TRIAGE → DISPATCHED …)을 쓰고, `git status`와 tracked diff를
**기준선**으로 저장한 뒤(다른 세션이 남긴 189개 미커밋 파일이 "위반"으로 잡히지 않도록) 역할
에이전트를 띄운다. 의존이 있으면 순서를 둔다 — ui가 쓸 getter는 gameplay가 먼저.

에이전트가 도는 동안 사용자는 다른 일을 해도 된다. `/cl9 status`를 치면 이렇게 보인다.

```
세션: cl9-2d(gameplay 수동) · cl9-37(map 수동) · 이 세션(리드, pumpkin-item)
락: cl9-37 editor 리스(15:20까지) · 배타 락 없음
훅: 이 세션 활성 ✓
실행 pumpkin-item (DISPATCHED, 6분): gameplay 진행 중(2분 전 편집) · balance 보고서 완료 · art 진행 중 · docs 완료
거부 1건: art가 Source/CL9/Items/CL9Item.cpp 편집 시도 → gameplay 영역 (보고서 '요청' 절 확인)
```

역할 에이전트가 자기 영역 밖을 만지려 하면 훅이 막고 이유를 말한다.

```
[cl9] Source/CL9/Items/CL9Item.cpp는 gameplay 영역 — 보고서 '다른 역할에 요청'에 적어라. docs/workflow/sessions.md#거부
```

에이전트는 `taskkill`·`Build.bat`·`git commit`·에디터 MCP를 쓸 수 없다. 셸은 허용목록만 된다
(`py -3 Tools/*`, `blender -b Tools/Blender/*`, `python -m unittest`, git 읽기 명령, `grep/ls/cat`류).
그 밖의 것은 "리드가 한다 — 필요하면 보고서에 적어라"로 거부된다.

### 3-4. 보고서

각 역할은 `.claude/runs/pumpkin-item/report-<역할>.md`에 일곱 절을 쓴다.
**한 일 / 바꾼 파일 / 다른 역할에 요청 / 빌드 필요 여부 / PIE 확인 항목 / decisions.md에 남길 결정 / 질문**.

밸런스 보고서는 여기에 두 절이 더 있다.

```
## 표 행
ItemTable.pumpkin.Value: — → 6~9 이더 (economy.md "재료는 요리의 절반 이하" 줄, order.md 호박 수프 보상 18)
ItemTable.pumpkin.Weight: — → 1.2 (rules.md 운반 감속 표; 계란 0.3, 고기 0.8 사이 위)
ItemTable.pumpkin.CookSeconds: — → 12 (cooking.md 계란 8·고기 10, 큰 재료라 위)
PIE에서 볼 것: 호박 두 개 들고 달릴 때 지친 빈손 속도보다 느린지(A1 관계), 12초에 익는지
## 검증
py -3 Tools/Balance/check_inequalities.py
A1 지친 빈손 속도 300 > 닭 추격 260 ✓ (DangerCurve 최대 1.3 적용 시 338 — 여전히 ✓... 아님: 338 > 300 ✗ 10일차부터 깨짐 → 질문 절 참조)
A2 질주 예금 900 > 시야 반경 800 ✓
A4 미결 — 무게→감속 식 없음
## 질문
10일차 이후 A1이 깨진다. 닭 추격 속도 곡선을 낮출지, 지친 속도를 올릴지는 사용자 결정.
```

값을 지어내지 않는다. 문서에 근거가 없으면 **질문** 절에 적고 멈춘다.

### 3-5. 통합 (리드만, 직렬)

1. 보고서 검사 — 일곱 절이 다 있는지. 10분 넘게 조용한 역할은 한 번 "보고서를 지금 써라"고
   찌르고, 그래도 없으면 중단(ABORTED)한다.
2. **감사** — 기준선 이후 바뀐 파일을 전부 모아 어느 브리프가 허용한 것인지 대조한다. 아무도
   허용하지 않은 파일은 "위반"으로 보고서에 남긴다(조용히 삼키지 않는다). 밸런스가 만진 공유
   문서는 "기존 행의 숫자 셀만 바뀌었는지"까지 본다.
3. 요청 처리 — 작은 것("getter 하나")은 해당 역할 에이전트에 `SendMessage`로 되돌려 이어서
   하게 한다(맥락 유지). 분할이 바뀌는 요청이면 다시 나눈다.
4. **사용자 확인** — 역할별 요약과 질문을 보여 주고 한 번 묻는다: A) 빌드·PIE 진행 B) 고칠 것
   있음 C) 여기서 멈춤. 서브에이전트 결과를 안 보고 빌드하는 일은 없다.
5. 표 행 적용 — 에디터가 켜져 있으면 MCP로, 꺼져 있으면 `apply_table_rows.py`를 커맨드릿으로
   (`acquire commandlet` — 에디터가 떠 있으면 PID를 대며 거부된다). 적용 뒤 에셋을 다시 읽어
   보고서 값과 대조하고, 어긋나면 밸런스에 문서 수정으로 되돌린다.
6. C++가 바뀌었으면 **락 아래 빌드** — `acquire build --pid` → `save_assets` → 다른 세션에
   통지 → `taskkill` → 분리 프로세스 빌드 → 에디터 재기동 → `release`.
7. PIE — 각 역할이 적은 절(B·M·K·P·G·R)을 MCP와 `cl9.*` 치트로 돌린다. 밸런스 값의 결정은
   PIE 관찰이 기록되기 전까지 `decisions.md`에 **초안** 표시로 들어간다.
8. `decisions.md` 추가, `memory/work-progress.md` 갱신, `run.json`에 벽시계·거부 수·빌드
   수·PIE 결과 기록.

### 3-6. 보고와 커밋

```
pumpkin-item 완료 (벽시계 41분; 같은 일을 한 세션에서 직렬로 했을 때 기록: 미측정 → F에서 잰다)
gameplay: 행 스키마·요리 전이 (파일 4) · balance: 값 3개, A1 10일차 문제 → 질문 1 · art: 메시 3종 (파일 9) · docs: 표 2줄
빌드 1회(2분 40초, 락 15:31~15:36) · PIE B·M·K·G·R 통과, P 미실행(정산은 값 확정 뒤)
커밋 준비: /cl9 commit gameplay · /cl9 commit art · /cl9 commit docs · balance는 질문 해결 뒤
```

커밋은 사용자가 `/cl9 commit <역할>`을 치고 초안에 "그래"라고 할 때만 된다.

## 4. 막혔을 때

- **서브에이전트가 질문을 남기고 멈췄다** → 리드가 사용자에게 묻고, 답을 `SendMessage`로 그
  에이전트에 보내면 맥락을 유지한 채 이어서 한다.
- **리드 세션 컨텍스트가 찼다** → `/cl9 handoff`가 `work-progress.md`와 `handoff.md`를 쓰고
  새 터미널용 재개 프롬프트를 출력한다. 새 터미널에서 `/cl9 resume`(인자 없으면 끝나지 않은
  최근 실행)이 `run.json`·브리프·보고서를 읽고 그 상태에서 계속한다.
- **훅이 내 편집을 막았는데 이해가 안 된다** → 메시지 끝의 `sessions.md#거부` 절. 역할
  에이전트가 아닌 리드·수동 세션의 편집은 막지 않는다. 훅 자체를 끄려면
  `cl9_lock.py guard off`(60분 뒤 자동 복구, `/cl9 status`에 표시).
- **락이 죽은 세션 것 같다** → `/cl9 status`가 소유자·목적·만료·프로세스 생존을 보여 준다.
  프로세스가 죽었으면 만료 전이라도 `release --force`, 살아 있으면 force도 거부된다.
- **세션을 새로 열었는데 락이 안 걸린다** → 훅은 세션 시작 때 읽힌다. 훅이 들어간 뒤 열린
  세션만 적용된다. `/cl9 status`가 "이 세션은 훅 미적용"이라고 알려 준다.

## 5. 이 시스템이 못 하는 것

- 에디터를 두 개 띄우거나 PIE를 병렬로 돌리지 않는다. 그건 언리얼의 한계라 분할이 아니라
  직렬화로 푼다.
- 자동으로 커밋·푸시하지 않는다.
- 훅은 걸림선이지 담장이 아니다. `python -c`나 `Set-Content`로는 어디든 쓸 수 있다 — 그래서
  감사가 진실이고, 기준선 패치가 복구 수단이다.
- 사람이 에디터 창을 직접 닫거나 Claude Code 밖의 PowerShell에서 죽이는 것은 못 막는다.
- 밸런스 값은 문서로 정합할 뿐 재미를 보장하지 않는다. 그래서 값마다 "PIE에서 볼 것"이 붙고
  관찰 전까지 초안이다.

## 6. 처음 켤 때 순서

1. 프로브 — 이 PC에서 훅이 어느 셸로 도는지, `py -3`가 잡히는지, 서브에이전트의 훅 입력이
   실제로 어떤 모양인지(픽스처로 저장), `agent_type`이 오는지.
2. 레지스트리·락·훅·테스트 → **열려 있는 세션 전부 재시작**.
3. `sessions.md`·`CLAUDE.md` 포인터 → `/cl9 role`, `status`, `handoff`, `commit`.
4. 컴파일 검사 프로브(에디터 켠 채 게임 타깃이 컴파일되는지 — 되면 에이전트가 문법 검사로 씀).
5. 역할 에이전트 렌더 → `/cl9 <작업>` → 드라이런 F(밸런스가 질문을 내야 하는 작업)와
   F2(A1 피연산자 하나 바꾸는 작업).
6. Agent Teams 스파이크 — 되면 눈으로 조정하는 역할은 팀원으로 띄운다.
