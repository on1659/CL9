# 인계 — 거대 스테이지(몽장) 구역 작업 (2026-10-04)

이 문서 하나로 다른 컴퓨터에서 작업을 이어갈 수 있게 만든 인계 문서다. 저장소(git)에 없는 것 — 실행 기록(brief·보고서), 메모리, 전역 규칙,
`/autogoal` 스킬 — 의 내용을 아래 "부록" 에 전부 실었다. 새 세션은 이 문서를 읽고 "이어가는 순서" 대로 한다.

세션 원본 기록은 같은 폴더에 있다:
`docs/sessionhistory/ca50ec2a-1a83-4185-bc20-4f29beec9b67.jsonl`(대화) · `docs/sessionhistory/ca50ec2a-1a83-4185-bc20-4f29beec9b67/`(서브에이전트·계획 패널).

---

## 1. 지금 상태

- 브랜치 `giant-districts-wip` (origin 에 푸시됨). `master` 는 원격보다 커밋 1개(`908e9b2`) 앞선 채 푸시하지 않았고, 그 커밋은 이 브랜치에 들어 있다.
- 명세 `docs/goal/giant-stage.md` 확정(미결 0). 사용자 결정과 계획 패널의 설계 결정이 거기 있다 — 다시 따지지 않는다.
- `/cl9 task` 실행 `giant-districts`:
  - **map 역할 완료** — 구역(Districts) C++ 지원과 테스트. 보고서는 부록 C.
  - **gameplay 역할 중단** — 사용자가 도중에 멈췄다. 보고서 없음. `Source/CL9/GameMode/CL9GameMode.h/.cpp`,
    `Source/CL9/Data/CL9RulesDef.h/.cpp`(새 열 `MonsterSpawnPerFrame`), `Source/CL9/Monsters/CL9Monster.cpp`,
    `docs/systems/balance.md`·`monster.md`·`raiding.md` 가 **미완 상태로 커밋돼 있다**. 어디까지 됐는지는 확인되지 않았다 —
    `git show 0b2b7cc -- <파일>` 로 읽고 brief(부록 B)와 대조해야 한다.
- **컴파일은 한 번도 끝까지 확인되지 않았다.** 컴파일 검사 3번이 전부 메모리 부족(다른 프로그램이 13GB 점유)으로 20분 제한에 끝났다.
  오류 없이 지난 것: `CL9Cheats.cpp`·`CL9GameState.cpp`·`CL9SiteDef.cpp`·`CL9SiteLayoutTests.cpp`. `CL9SiteMap.cpp` 는 미검사.
  2차 검사에서 `CL9GameMode.cpp(750): 'MonsterSpawnPerFrame' 은 'UCL9RulesDef' 의 멤버가 아닙니다` 와 `CL9GameMode.h(35)` 오류가 찍혔다 —
  gameplay 가 편집하던 도중이라 그런 것인지, 실제 오류인지는 확인되지 않았다.
- 자동화 테스트(`CL9.Map.Districts`·`CL9.Map.DistrictSlots`·기존 `LargeSite`·`DepthBand`)는 한 번도 돌리지 않았다.
- `mongjang` 행·`Lvl_Mongjang` 레벨·`Tools/Unreal/stages/mongjang.json` 은 아직 없다(실행 2 의 일).

## 2. 사용자 결정 (2026-10-04)

| 항목 | 결정 |
|---|---|
| 크기 | 5배(1,000 모듈) 먼저, 측정을 통과하면 10배 |
| 밤 길이 | 600초 (리썰 분석 문서의 코드 기본값으로 계산) |
| 모양 | 구역 묶음 — 건물 덩어리 여러 개를 바깥 마당으로 잇는다 |
| 몬스터 | 멀리 있어도 전부 움직인다(거리로 재우지 않는다). 수는 리썰과 비교해 1일차 약 32, 10일차 약 80 |
| 합격 기준 | 프레임 시간만(평균 16.7ms · p95 25ms · GPU 10ms 이하). 게임 스레드 ms 는 보고만 |
| 로드 | 한꺼번에 올리지 않는다 — 구역은 프레임당 하나씩, 몬스터는 나눠 스폰 |
| 이름 | 별칭 `mongjang` · 레벨 `Lvl_Mongjang` · 표시 이름 "몽장" |
| 멀티 목표 | 1~4인 리슨 서버 (묻지 않고 프로젝트 기존 목표를 적용) |

## 3. 미결·주의

- **먼 구역 슬롯이 초기 값으로는 안 들어간다**(부록 C 질문 1). `CL9.Map.Districts` 가 재는 점유 경계 상자를 본 뒤 구역을 당기거나 먼 구역의 모듈 수를 줄인다.
- **입구·비상구 해석**: "입구 하나, 비상구 하나"(module.md 결정 3)를 "구역 건물마다 하나씩" 으로 읽었다. 사용자 확인을 받지 않았다.
- **비상구(flap)는 명세와 다르게 구현됐다**: 액터를 회전시키지 않고 스폰한다(부록 C).
- 구역을 고를 단서(표지·UI)는 범위 밖이다.
- 훅 상태 점검(`py -3 .claude/hooks/cl9_lock.py status --nonce …`)은 **다른 명령과 이어 치면 "비활성" 으로 잘못 나온다** — 단일 명령으로 친다.

## 4. 새 컴퓨터 준비

1. 저장소를 받는다: `git clone https://github.com/on1659/CL9.git` → `git checkout giant-districts-wip`.
   문서·스크립트에 절대 경로 `D:\Work\unreal\CL9` 가 많다 — 같은 경로에 두는 것이 가장 덜 고친다.
2. UE 5.8 을 설치하고 `GenerateProjectFiles.bat` 10행의 엔진 경로(`D:\Program Files\Epic Games\UE_5.8`)를 그 컴퓨터에 맞춘다.
3. `Content\DreamscapeSeries`(Dreamscape Farmlands 팩)는 **필요 없다** — 2026-10-04 에 레벨 네 개에 놓여 있던 팩 닭 액터를 지우고 팩 폴더를 삭제했다.
   옛 `ABP_Chicken`·`ABP_Pig`·`ABP_Sheep`(아무도 참조하지 않는 고아)만 사라진 팩 스켈레톤·애님을 가리킨 채 남아 있다. 새 엔진은 5.8.3 이어도 된다(이 PC 는 5.8.1).
4. `.mcp.json` 의 uvx 경로(`C:/Users/user/.local/bin/uvx.exe`)를 그 컴퓨터의 사용자 이름에 맞춘다.
5. 전역 규칙과 스킬을 되살린다(부록 E·F): `C:\Users\<사용자>\.claude\CLAUDE.md`, `C:\Users\<사용자>\.claude\skills\autogoal\SKILL.md`.
   `/cl9` 스킬과 역할 에이전트·훅은 저장소 `.claude/` 안에 있어 git 으로 따라온다.
6. 메모리를 되살린다(부록 D): `C:\Users\<사용자>\.claude\projects\D--Work-unreal-CL9\memory\` 에 파일 이름대로 쓴다
   (폴더 이름은 프로젝트 경로에서 만들어진다 — 경로가 다르면 그에 맞춘다).

## 5. 이어가는 순서

1. `.claude/runs/giant-districts/` 를 만들고 부록 A·B·C 의 파일 다섯 개(`run.json`·`grants.json`·`brief-map.md`·`brief-gameplay.md`·`report-map.md`)를 그대로 쓴다.
   기준선 파일(`baseline.json`·`baseline.patch`·`baseline-files`)은 옮기지 않았다 — `/cl9 resume giant-districts` 가 기준선이 없다고 거부하면
   새 실행(`/cl9 task`)을 같은 brief 로 다시 만든다.
2. gameplay 역할을 새로 띄운다. brief 는 부록 B. "커밋 `0b2b7cc` 에 들어 있는 미완 변경을 먼저 읽고 이어서" 를 덧붙인다.
3. 감사 → `CL9Editor` 정식 빌드(메모리를 비운 상태에서) → 자동화 테스트 → 기존 스테이지 PIE(L·D·C·U 절).
4. 테스트의 점유 경계 상자로 구역 슬롯을 확정하고, 실행 2: `setup_stage.py` 구역 지원·`mongjang.json`·`Lvl_Mongjang`·QA 스크립트(map),
   구역별 수치(balance), 문서(docs). 명세 "Work split" 참조.

새 세션에 붙여 넣을 말:

```
docs/sessionhistory/HANDOFF-2026-10-04-giant-districts.md 를 읽고 "이어가는 순서" 대로 이어서 하자.
```

---

# 부록

부록의 각 블록은 원본 파일 내용 그대로다(이 PC 의 사용자 이름 `user` 가 들어간 경로는 새 컴퓨터에서 맞지 않을 수 있다).

## 부록 A. 실행 기록 — run.json · grants.json

### run.json

`.claude/runs/giant-districts/run.json`

~~~~
{
 "slug": "giant-districts",
 "task": "거대 스테이지(몽장) 1차 — 구역(Districts) C++ 지원: 구역마다 SiteMap 액터, 프레임당 한 구역 짓기, 구역별 밤 스폰·나눠 스폰·내비 대기, 테스트",
 "state": "DISPATCHED",
 "roles": [
  "map",
  "gameplay"
 ],
 "created": 1791097139.2920852,
 "flags": {},
 "denials": 0,
 "history": [
  {
   "state": "TRIAGE",
   "at": 1791097139.2920852
  },
  {
   "state": "DISPATCHED",
   "at": 1791097180.06926
  }
 ],
 "updated": 1791097180.06926,
 "dispatched": 1791097180.06926
}
~~~~

### grants.json

`.claude/runs/giant-districts/grants.json`

~~~~
{
 "cl9-map": [
  "source/cl9/data/cl9sitedef.h",
  "source/cl9/data/cl9sitedef.cpp",
  "source/cl9/world/cl9sitemap.h",
  "source/cl9/world/cl9sitemap.cpp",
  "source/cl9/world/cl9sitelayouttests.cpp",
  "source/cl9/gamemode/cl9gamestate.h",
  "source/cl9/gamemode/cl9gamestate.cpp",
  "source/cl9/gamemode/cl9cheats.cpp"
 ],
 "cl9-gameplay": [
  "source/cl9/gamemode/cl9gamemode.h",
  "source/cl9/gamemode/cl9gamemode.cpp",
  "source/cl9/monsters/cl9monster.h",
  "source/cl9/monsters/cl9monster.cpp"
 ]
}
~~~~

## 부록 B. brief

### brief-map.md

`.claude/runs/giant-districts/brief-map.md`

~~~~
# brief — map · 실행 giant-districts

## 목표
사이트 하나가 **구역(district) 여러 개**로 설 수 있게 한다. 구역 하나 = `ACL9SiteMap` 액터 하나이고, 각 구역은 **기존 생성기
(`CL9SiteLayout::Generate`)를 고치지 않고** 자기 하위 시드·모듈 수·월드 변환으로 돌린다. 정본은 `docs/goal/giant-stage.md` 의
"Design Decisions" 와 "Gate answers" — 거기서 정해진 것은 다시 따지지 않는다.

해야 할 것:
1. `FCL9SiteDistrict { Offset(트럭 패드 기준 cm, FVector2D), Yaw(0·90·180·270), ModuleCount(기준 모듈 수), Zones(비면 사이트 Zones) }` 와
   `UCL9SiteDef::Districts` 배열. **`Districts` 가 비면 지금의 단일 사이트 경로가 바이트 단위로 그대로여야 한다**(양계장·대농장·돼지 농장 불변).
2. `ACL9SiteMap`: 구역 인덱스·변환을 갖는다. 구역 짓기는 그 구역의 `ModuleCount`(일자 배율은 지금처럼 곱한다)와 하위 시드
   `HashCombine(Seed, i)`(0 이면 1 로 — 시드 0 은 아무것도 안 짓는다)를 쓴다. `SiteToWorld` 는 액터 변환을 거치게 하고, 비상구(flap)는
   액터 회전과 월드로 돌린 `Outward` 로 스폰한다. 격자 상한 128 은 건드리지 않는다.
3. **한꺼번에 짓지 않는다(사용자 결정)** — 구역이 여럿이면 `Rebuild` 는 **프레임당 구역 하나씩** 짓는다. 단일 사이트(Districts 빔)는 지금처럼 즉시 짓는다.
   호스트·클라이언트 모두 같은 경로다(복제되는 시드는 지금처럼 하나).
4. 로그: 구역마다 `LayoutHash`, 모듈·인스턴스·컴포넌트·소품·등·깜빡이 등 수, 단계별(Generate / segments / props+lamps / register) 빌드 ms.
5. `GameState::RebuildSiteMap`·`Clear`, 치트 `LayEditorSiteMap` 이 구역 전부를 다루게 한다.
6. 테스트: `CL9.Map.Districts` — 160·190·230 기준 모듈의 10일차 수(×1.25)로 30 시드: 전부 서고, `IsConnected`, 겹침 없음, flap 정확히 1,
   납품 방 ≥ 2, 예산의 ≥ 90%, 레이아웃당 생성 ms 와 **점유 경계 상자**(격자 span 이 아니라 실제 모듈이 차지한 범위)를 로그로.
   슬롯 검사 함수(회전한 10일차 span 사각형끼리 안 겹침, `1.59·N + 0.444·L ≤ 440`)는 순수 함수로 만들고 테스트한다 — 실제 `mongjang` 행은
   다음 실행에서 생기므로 여기서는 명세의 초기 슬롯 값으로 검증한다.
   기존 `CL9.Map.LargeSite`·`CL9.Map.DepthBand` 는 고치지 않는다(불변 증명용).

## 파일 (이 목록 = grants.json — 다른 파일은 훅이 막는다)
- Source/CL9/Data/CL9SiteDef.h
- Source/CL9/Data/CL9SiteDef.cpp
- Source/CL9/World/CL9SiteMap.h
- Source/CL9/World/CL9SiteMap.cpp
- Source/CL9/World/CL9SiteLayoutTests.cpp
- Source/CL9/GameMode/CL9GameState.h
- Source/CL9/GameMode/CL9GameState.cpp
- Source/CL9/GameMode/CL9Cheats.cpp

`CL9SiteLayout.h/.cpp` 는 목록에 없다 — 생성기는 고치지 않는다. 꼭 고쳐야 한다고 판단되면 고치지 말고 보고서 '질문' 에 적는다.

## 받는 인터페이스
- 없음

## 주는 인터페이스 (gameplay 가 이어서 쓴다 — 이 시그니처 그대로 만든다. 바꿔야 하면 보고서 맨 위에 바뀐 시그니처를 적는다)
- `static void ACL9SiteMap::FindAll(UWorld* World, TArray<ACL9SiteMap*>& OutMaps)` — 구역 인덱스 순. 단일 사이트면 하나.
- `static bool ACL9SiteMap::IsBuildPending(UWorld* World)` — 프레임 나눠 짓기가 아직 안 끝났으면 true.
- `int32 ACL9SiteMap::GetDistrictIndex() const` — 단일 사이트면 `INDEX_NONE`.
- `const TArray<FCL9SiteZone>& ACL9SiteMap::GetZones() const` — 구역 `Zones` 가 있으면 그것, 비면 사이트 `Zones`.
- 기존 `Find`·`GetLayout`·`GetParams`·`GetPoints`·`PointToWorld` 는 시그니처 그대로(단일 경로 호환).

## 읽을 문서
- docs/goal/giant-stage.md (전체 — 특히 Design Decisions, Gate answers, Known weak points)
- docs/systems/map.md, docs/systems/stage.md
- docs/systems/performance.md "규칙" 절
- docs/conventions.md (주석은 한글)

## PIE 확인
- L (맵 생성) — 기존 스테이지가 그대로 서는가
- U (성능 기준선) — 밤 실내 avg + `stat dumphitches` 30ms 히치 없음
- 구역 경로 자체의 PIE 는 다음 실행(`mongjang` 행·레벨)에서 한다 — 이번에는 자동화 테스트가 증명이다.

## 보고서
- .claude/runs/giant-districts/report-map.md

~~~~

### brief-gameplay.md

`.claude/runs/giant-districts/brief-gameplay.md`

~~~~
# brief — gameplay · 실행 giant-districts

## 목표
밤 스폰이 **구역(district) 여러 개**를 다루게 하고, 큰 판에서 밤 시작이 한꺼번에 무겁지 않게 한다. 정본은 `docs/goal/giant-stage.md` 의
"Design Decisions" 와 "Gate answers" — 거기서 정해진 것은 다시 따지지 않는다.

해야 할 것:
1. `SpawnNight`(CL9GameMode.cpp:504~646 부근)이 `ACL9SiteMap::FindAll` 의 **구역마다** 돈다: 구역별 지점 목록, 구역의 `GetZones()`,
   항아리 중복 제거 집합은 **구역마다 초기화**(모듈 인덱스가 레이아웃별이다). 구역이 하나(단일 사이트)면 결과가 지금과 같아야 한다.
2. **몬스터는 나눠 스폰한다(사용자 결정 — 한꺼번에 올리지 않는다)**: 밤 시작 때 스폰 목록을 만들어 프레임당 몇 마리씩 스폰한다.
   한 프레임 몇 마리인지는 코드 상수로 두지 말고 기존 규칙 표(`CL9Rules`)에 열이 필요하면 보고서 '표 행' 으로 요청하고 자리표시 기본값을 쓴다.
   단일 사이트(작은 판)도 같은 경로를 타도 된다 — 다만 스폰 총수·위치 규칙은 바뀌지 않아야 한다.
3. `StartNight` 는 호스트에서 **구역 짓기가 안 끝났거나(`ACL9SiteMap::IsBuildPending`) 내비 빌드가 진행 중이면** 재시도 타이머로 미루고,
   기다린 시간을 로그로 남긴다. UI 는 없다.
4. 몬스터 메시 `VisibilityBasedAnimTickOption = OnlyTickMontagesWhenNotRendered`. **먼저** 몬스터 C++ 과 `Content` 의 몬스터 ABP/BP 가
   노티파이·포즈에 기대는 게임플레이를 갖는지 확인한다(에셋을 못 열면 그 사실을 보고서에 적는다). 포즈에 기대는 종이 있으면 그 종만 기존 값.
5. **몬스터는 거리로 재우거나 없애거나 느리게 하지 않는다**(사용자 결정). AI·감각·이동은 항상 돈다.

## 파일 (이 목록 = grants.json — 다른 파일은 훅이 막는다)
- Source/CL9/GameMode/CL9GameMode.h
- Source/CL9/GameMode/CL9GameMode.cpp
- Source/CL9/Monsters/CL9Monster.h
- Source/CL9/Monsters/CL9Monster.cpp

## 받는 인터페이스 (map 이 제공, 먼저 끝난다 — 실제 시그니처는 `.claude/runs/giant-districts/report-map.md` 와 `CL9SiteMap.h` 로 확인)
- `static void ACL9SiteMap::FindAll(UWorld* World, TArray<ACL9SiteMap*>& OutMaps)` — 구역 인덱스 순. 단일 사이트면 하나.
- `static bool ACL9SiteMap::IsBuildPending(UWorld* World)`
- `int32 ACL9SiteMap::GetDistrictIndex() const` — 단일 사이트면 `INDEX_NONE`.
- `const TArray<FCL9SiteZone>& ACL9SiteMap::GetZones() const`

## 주는 인터페이스
- 없음

## 읽을 문서
- docs/goal/giant-stage.md (전체)
- .claude/runs/giant-districts/report-map.md
- docs/systems/raiding.md, docs/systems/monster.md, docs/systems/stage.md
- docs/systems/performance.md "규칙" 절
- docs/conventions.md (주석은 한글)

## PIE 확인
- D (밤 수명주기) — 기존 스테이지에서 밤이 시작되고 스폰 수·항아리 수가 전과 같다
- C (몬스터) — 화면 밖 몬스터가 여전히 움직이고 추격한다
- U (성능 기준선) — 밤 실내 avg + `stat dumphitches` 30ms 히치 없음

## 보고서
- .claude/runs/giant-districts/report-gameplay.md

~~~~

## 부록 C. map 보고서

### report-map.md

`.claude/runs/giant-districts/report-map.md`

~~~~
# report — map · 실행 giant-districts

(map 에이전트가 보고서 파일 쓰기에서 막혀 본문을 리드에 돌려줬고, 리드가 그 내용을 옮겨 적었다.)

주는 인터페이스는 brief 그대로다: `static void ACL9SiteMap::FindAll(UWorld*, TArray<ACL9SiteMap*>&)`,
`static bool ACL9SiteMap::IsBuildPending(UWorld*)`, `int32 ACL9SiteMap::GetDistrictIndex() const`,
`const TArray<FCL9SiteZone>& ACL9SiteMap::GetZones() const`. `Find`·`GetLayout`·`GetParams`·`GetPoints`·`PointToWorld`·`Rebuild`·`Clear` 시그니처 불변.

## 한 일
1. 데이터: `FCL9SiteDistrict { FVector2D Offset(트럭 패드 기준 cm, 월드 축), int32 Yaw(0·90·180·270), int32 ModuleCount(기준), TArray<FCL9SiteZone> Zones }`
   와 `UCL9SiteDef::Districts`. `Validate` 가 잘못된 Yaw, ModuleCount < 2, 구역 Zones 의 잘못된 띠·행, Generator None 인데 Districts 가 있는 행을 거부한다.
2. 순수 함수 `namespace CL9SiteDistrict`(`CL9SiteDef.h/.cpp`): `SubSeed`(= `HashCombine(Seed, i)`, 0 → 1. 복제 시드가 0 이면 0 그대로 —
   `cl9.seed 0` 은 구역에서도 "안 짓는다"), `QuarterTurns`·`Rotate`(90° 단위, 삼각함수 없음), `GridSpan`(생성기 격자 식의 사본 — 테스트가 실제
   `GridSize` 와 대조), `SpanRect`, `GateDistance`, `NightEstimate`(160 + 5.3 × 본줄기 비율 × N + 마당 왕복), `CheckSlots`(겹침·밤 초과 보고, 맞닿음은 허용).
   입력은 `FCL9DistrictSlotRules` 로 받는다.
3. `ACL9SiteMap`
   - `Districts` 가 비면 전과 같은 흐름. 달라진 것 셋: 월드에 남은 구역 액터를 먼저 치운다 / `SiteToWorld` 가 `GetActorTransform().TransformPosition`
     을 거친다(회전 없으면 같은 값) / 빌드마다 로그 한 줄 추가. 생성기는 건드리지 않았다.
   - 구역 사이트: `Rebuild` 가 구역마다 액터 하나(위치 = 트럭 패드 + Offset, 회전 = Yaw). 액터가 구역 인덱스·하위 시드·일자·사이트·레지스트리를
     `UPROPERTY(Transient)` 로 든다. 구역 0 은 `Rebuild` 안에서, 나머지는 `FTSTicker` 코어 티커로 프레임당 하나(`GFrameCounter` 로 같은 프레임 중복 방지).
     에디터 맵(`cl9.seed`)도 같은 경로를 타야 해서 액터 Tick 이 아니라 코어 티커다. 티커는 `Destroyed`/`EndPlay` 에서 뗀다. 호스트·클라 같은 경로.
   - 구역 `Build` 는 구역 ModuleCount × SiteSizeCurve, 구역 Zones(비면 사이트 Zones)를 `Zones` 에 복사.
   - **비상구(flap) — brief 와 다르다**: `ACL9FlapIn::Setup` 은 안쪽 방향을 월드 방향(밀고 지나감 검사)과 로컬 축(트리거 상자 크기·오프셋) 양쪽으로
     쓴다. flap 액터를 돌리면 90°/270° 에서 트리거가 엉뚱한 축에, 180° 에서 바깥에 놓인다. 그래서 flap 은 회전 없이 스폰하고 상자 크기(홀수 회전이면
     X/Y 교환)와 안쪽 방향을 월드로 돌려 넘긴다. 위치는 액터 변환을 거친다. 회전 0 이면 전과 같은 값이고 `CL9FlapIn.cpp` 는 불변.
   - 로그: 빌드마다 `Site map build district i of K (seed s) — hash %08X` + 모듈·인스턴스·컴포넌트·소품·등·깜빡이 등 수 + generate / segments /
     props+lamps / register / total ms. 단일 사이트는 `single site (seed s)`. 구역 사이트는 시작 줄과 "all K districts built" 줄. 기존 `Site map built —` 줄 불변.
   - `Find` 는 구역 0, `FindAll` 은 구역 순 전부, `Clear` 는 전부 제거. 격자 상한 128 불변.
4. GameState·치트: 정적 `Rebuild`/`Clear` 가 구역 전부를 다루므로 흐름 불변, 주석만 갱신.
5. 테스트
   - `CL9.Map.Districts`: 200·238·288 모듈(160·190·230 × 1.25), 시드 1~30 의 하위 시드, farm 설정. 슬롯마다: 전부 섬, 연결, 겹침 없음, flap 1,
     납품 방 ≥ 2, 1층 ≥ 90%, 격자 식 사본 = 생성기. 정보 줄: 생성 ms(평균·최대), 점유 경계 상자(m). 하위 시드 검사 포함.
   - `CL9.Map.DistrictSlots`: 회전이 `FRotator` Yaw 와 일치, 어림식 = `1.59·N + 0.444·L ≤ 440`, 북쪽 사각형 좌표, 명세의 동·서(230, 문 20 m)·
     남·북(190, 문 210 m) 통과, 겹침·밤 초과 검출, 기준 합 1,000 ± 5%. 먼 슬롯은 로그만(질문 1).
   - `CL9.Map.LargeSite`·`CL9.Map.DepthBand` 는 고치지 않았다.

**테스트는 아직 한 번도 돌리지 않았다.**

## 바꾼 파일
Source/CL9/Data/CL9SiteDef.h
Source/CL9/Data/CL9SiteDef.cpp
Source/CL9/World/CL9SiteMap.h
Source/CL9/World/CL9SiteMap.cpp
Source/CL9/World/CL9SiteLayoutTests.cpp
Source/CL9/GameMode/CL9GameState.h
Source/CL9/GameMode/CL9GameState.cpp
Source/CL9/GameMode/CL9Cheats.cpp

## 다른 역할에 요청(인터페이스·표 행·에셋)
- (gameplay) 구역 사이트에서 `Find` 는 구역 0 만 준다. `SpawnNight` 은 `FindAll` 을 돌고 사이트 `Zones` 대신 `Map->GetZones()` 를 쓴다.
  항아리 중복 제거 집합은 구역마다 초기화. 밤 시작은 `IsBuildPending(World) == false` 를 기다린다. 단일 사이트면 `FindAll` 이 하나,
  `GetDistrictIndex()` 는 `INDEX_NONE`, `GetZones()` 는 사이트 Zones.
- 표 행·에셋 변경 없음. MapTable 에 `Districts` 열이 생기고 기존 세 행은 비어 있다.

## 빌드 필요 여부
예 (헤더 변경 — 새 USTRUCT, `UCL9SiteDef`·`ACL9SiteMap` 의 UPROPERTY). 컴파일 검사는 에이전트가 끝날 때 10개 중 4개(Cheats, GameState, SiteDef,
LayoutTests)만 오류 없이 지났고 `CL9SiteMap.cpp`·생성 파일·링크는 미확인이었다 — `.claude/runs/compile/compile-20261004-161136.log`.

## PIE 확인 항목
- L: `Lvl_Henhouse`·`Lvl_Farm`·`Lvl_PigFarm` 이 전처럼 선다. 새 `Site map build single site … hash` 값 = `Site map built … hash` 값 = 변경 전 같은
  시드·일자의 해시. flap 은 안에서만 열린다.
- U: 밤 실내 avg, `stat dumphitches` 30ms 히치 없음.
- 자동화 테스트: Districts·DistrictSlots 통과, LargeSite·DepthBand 는 전과 같은 정보 값.
- 구역 경로 PIE 는 다음 실행: 프레임당 해시 한 줄, 클라 해시 = 호스트 해시, 90°/180°/270° 구역의 flap 이 안에서만 열림.

## decisions.md에 남길 결정
- 2026-10-04 구역 런타임: 구역 하나 = `ACL9SiteMap` 하나, 생성기 불변. 하위 시드 `HashCombine(Seed, i)`(0 → 1, 복제 시드 0 은 0 유지),
  ModuleCount × SiteSizeCurve, 액터 변환(트럭 패드 + Offset, Yaw). `Districts` 가 비면 단일 경로. 구역 0 은 `Rebuild` 안에서, 나머지는 `FTSTicker` 로 프레임당 하나.
- 2026-10-04 flap 은 회전 없이 스폰: `ACL9FlapIn::Setup` 이 안쪽 방향을 월드·로컬 양쪽으로 쓰므로, 회전 없이 스폰하고 월드로 돌린 상자 크기와 방향을 넘긴다.

## 질문(막힌 것)
1. **먼 구역이 명세의 초기 값으로는 들어가지 않는다.** 10일차 격자 사각형 기준 — 동·서 288 모듈 = 84칸 = 420 m(문 20 m), 남·북 238 모듈 = 76칸 = 380 m
   (문 210 m): 이 넷은 서로 맞닿기만 해서 통과. 먼 구역(200 모듈 = 350 m, 문 300 m)은 네 방향 어디서든 가까운 슬롯과 겹친다. 안 겹치는 가장 가까운 자리는
   문 약 429 m, 그때 밤 어림 약 605초로 600초를 넘는다. 명세 "Measure first 2" 의 선택(점유 상자가 span 보다 작으면 당기기, 아니면 N 줄이기)이라 여기서
   정하지 않았다. 먼 슬롯의 방향도 명세에 없다. 슬롯 값을 정하는 실행(balance)이 `CL9.Map.Districts` 의 점유 상자 실측을 쓴다.
2. **L 의 뜻**: 트럭 패드에서 구역 문(구역 원점 + EntryOffset, Yaw 회전)까지의 직선으로 잡았다. 실제 문은 한 칸쯤 치우칠 수 있으나 어림이라 무시했다.

~~~~

## 부록 D. 메모리 (`C:\Users\<사용자>\.claude\projects\D--Work-unreal-CL9\memory\`)

work-progress.md 는 2,000줄이 넘어 2026-10-04 항목(1993행부터 끝)만 실었다. 전체는 세션 기록에서도 복구되지 않으니 필요하면 이 PC 에서 따로 복사한다.

### MEMORY.md

`memory/MEMORY.md`

~~~~
# Memory Index

- [Unreal MCP 사용법](unreal-mcp-usage.md) — 포트 8000 자동 시작, camelCase·refPath 규칙, StartPIE 모달 함정
- [에디터 강제 종료 허가](editor-kill-allowed.md) — 빌드 시 taskkill OK, 저장 먼저·5초 지연 후 빌드
- [작업 진행 현황](work-progress.md) — 최신: 2026-10-04 전부 커밋·푸시(origin/master 10aa6da), 노트북(Windows 사용자 radar) 이전 준비. Lvl_Henhouse 내비 경계 문제는 미결. 훅 잠금 CLI는 단일 명령으로. 파일 끝 재개 지점부터 읽을 것
- [세션 연속성 규칙](session-continuity.md) — 작업 단위 끝날 때마다 work-progress 갱신, "이어서 하자"면 그것부터 읽기
- [애님 배율 규칙](anim-rate-rule.md) — 로코모션 PlayRate=속도/기준속도 + 문턱 정합, 모든 새 애님의 기본
- [주석은 한글로](comment-language-korean.md) — 정본은 docs/decisions.md·ue-cpp-conventions.md, Copyright 헤더만 예외
- [Steam 출시 계획](steam-release-plan.md) — docs/goal/steam-release.md 작성·/autoplan 리뷰 완료(2026-09-06), 최종 게이트 결정 대기, 엔진 소스로 확정한 사실 목록
- [맵 모듈 재작성](map-module-rewrite.md) — 격자 미로 폐기·모듈 조립. 미결 5개 + C++ + **ModuleTable 16종까지 완료, 실제 맵이 선다**(2026-09-08). 소품 세트(PropSets)·문 옆 토막(barnStub/wallStub)·Grid 바닥은 2026-09-11 완료 — work-progress 참조
- [참고 게임 분석](reference-games-analysis.md) — 디컴파일(ilspycmd)+에셋덤프(UnityPy) 방법, 결과는 저장소 docs/reference/, 1차 조사에서 정정된 것들
- [문자열은 ko만](loc-korean-only.md) — 문구 작업은 ko 열만, 번역은 사용자가 정한 시점에 몰아서 (2026-09-12)
- [전문 용어는 이름 그대로](plain-korean-wording.md) — rules를 "기제"로 바꿔 부르지 않는다, 괄호 설명도 붙이지 않는다 (2026-09-13 지적)
- [퀵슬롯 이름](quickslot-naming.md) — 하단 아이템 칸은 "퀵슬롯". rename 완료(2026-09-13 저녁, 코드·에셋·문서)
- [PIE 3fps 스로틀 함정](pie-throttle-3fps.md) — "이 머신 PIE 3~8fps" 는 창 최소화·백그라운드 스로틀이었다(2026-09-21). 측정 전 창 복원, 잠금 CLI 단일 명령, 성능 정본 docs/systems/performance.md
- [Codex 스킬 금지](no-codex-skill.md) — 사용자가 다시 쓰라고 할 때까지 /codex·codex:rescue 안 씀 (2026-09-22)
- [서브에이전트 모델 배분](subagent-model-allocation.md) — 일의 성격으로 opus/sonnet/haiku, 띄울 때 model 명시 (2026-10-04)

~~~~

### anim-rate-rule.md

`memory/anim-rate-rule.md`

~~~~
---
name: anim-rate-rule
description: 로코모션 애님은 PlayRate=속도/기준속도로 구동 — 새 애니메이션 작업의 기본 규칙 (사용자 확정)
metadata: 
  node_type: memory
  type: feedback
  originSessionId: ce99b7ea-3df1-4c0f-9c1d-5f287190d847
  modified: 2026-09-04T16:26:33.064Z
---

사용자 확정 (2026-09-05): "앞으로 모든 애니메이션 만들 때 이런 식으로 맞추는 걸 기본으로."

**Why:** 제자리 애님을 고정 1.0배속으로 틀면 실제 이동 속도와 발걸음이 어긋나 풋 슬라이딩이
생긴다 — 닭에서 실제로 발생했고 사용자가 바로 알아챘다.

**How to apply:** 새 종·새 로코모션 애님을 배선할 때마다 ① PlayRate 핀 노출
(ShowPinForProperties에서 bShowPin — MCP set_properties로 가능) → SafeDivide(Speed/기준속도)
연결, ② 블렌드 문턱을 테이블 속도와 정합(배회=완전 걷기, 추격=완전 달리기), ③ 기준 속도는
PIE 실측 보정. 정본: `D:\Work\unreal\CL9\docs\systems\animation.md` "로코모션 재생 배율" 절.
관련: [[work-progress]], [[unreal-mcp-usage]]

~~~~

### comment-language-korean.md

`memory/comment-language-korean.md`

~~~~
---
name: comment-language-korean
description: 코드 주석 언어 규칙(한글)은 docs/decisions.md·ue-cpp-conventions.md에 정본이 있다
metadata: 
  node_type: memory
  type: reference
  originSessionId: 5037b4b4-1c33-4879-adaf-5fd28d544d34
  modified: 2026-09-06T08:02:07.168Z
---

새로 쓰거나 고치는 모든 코드 주석은 한글로 쓴다 — 이 프로젝트가 승계한 Epic 공식 표준의
"코드와 코멘트는 미국 영어" 방침을 코멘트에 한해 뒤집은 것. 정본은 다음 두 곳:

- `D:\Work\unreal\CL9\docs\decisions.md` (2026-09-06 항목 "코드 주석은 한글로 쓴다")
- `D:\Work\unreal\CL9\docs\ue-cpp-conventions.md` (명명 규칙 절, 미국 영어 규칙 옆에 예외 명시)

코드 식별자(변수·함수·클래스명)는 계속 영어 PascalCase. 파일 최상단 Epic Games 저작권 고지
줄만 한글화 예외로 원문 유지.

~~~~

### editor-kill-allowed.md

`memory/editor-kill-allowed.md`

~~~~
---
name: editor-kill-allowed
description: CL9 빌드 사이클에서 언리얼 에디터 프로세스를 사용자 확인 없이 강제 종료해도 된다 (2026-09-02 사용자 허가) — 단 잠금을 잡고
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 5698782d-d8ba-4b24-910b-2b240c853261
  modified: 2026-09-12T15:55:27.367Z
---

빌드 때문에 에디터를 꺼야 할 때 사용자에게 닫아달라고 요청하고 기다릴 필요 없이 `taskkill /IM UnrealEditor.exe`로
바로 종료해도 된다 (2026-09-02 사용자: "앞으로 에디터 프로세스 강제로 종료해도돼"). 세션끼리는 알아서 조율한다
(2026-09-12: "너가 알아서 대화하고 알아서 끄고 빌드해"). UI 세션도 직접 빌드해도 된다 (2026-09-06).

**Why:** 에디터 종료 대기가 워크플로의 병목이었다 — 사용자가 자리를 비우면 수십 분씩 멈췄다.

**How to apply:** 절차 전문은 저장소 `docs/workflow/sessions.md`(`#잠금` 절 — 잠금 → save_assets → 통지 → taskkill → 5초 →
분리 프로세스 빌드 → 재기동 → release)가 정본이다. 2026-09-13부터 훅이 강제한다: **`py -3 .claude/hooks/cl9_lock.py acquire build --pid <PID>`
를 잡은 세션만** taskkill·Build.bat·StartPIE 가 통과한다. 통지(SendMessage)는 그대로 한다 — 잠금은 "안 된다"를, 메시지는 "왜·언제"를 말한다.

~~~~

### loc-korean-only.md

`memory/loc-korean-only.md`

~~~~
---
name: loc-korean-only
description: 문자열 작업은 ko 열만 채우고 번역은 사용자가 정한 시점에 몰아서 — 평소 번역 신경 쓰지 않기 (2026-09-12 사용자 지시)
metadata:
  type: feedback
---

화면 문자열을 넣거나 고칠 때는 `Content/Strings/CL9_Strings.csv`의 `ko` 열만 채운다. en·ja·zh-Hans 열은 비워 두고,
번역은 사용자가 "번역 한 번 하자"고 할 때 빈 칸을 몰아서 채우고 `python Tools/Loc/sync_strings.py --build`를 돌린다.

**Why:** 사용자 2026-09-12 — "번역은 어느 순간에 한 번 할 거라 기본은 한국어로만 작업하고, 번역은 신경 쓰지 말라".
문구는 자주 바뀌니 매번 번역하면 낭비다. 빈 언어 열은 그 언어에서 ko로 보일 뿐 게임은 정상이다.

**How to apply:** 새 문구 = 시트에 `No,Key,DevNotes(의미),ko` 한 줄 + `sync_strings.py`(원문만, 1초). 번역 열을 채우지도,
채우라고 제안하지도 않는다. 정본은 docs/systems/localization.md "번역" 절 — [[work-progress]].

~~~~

### map-module-rewrite.md

`memory/map-module-rewrite.md`

~~~~
---
name: map-module-rewrite
description: "맵 생성을 R.E.P.O. 방식 모듈 조립으로 교체 — 미결 5개 확정 + C++ 구현 + ModuleTable 16종까지 완료, 실제 맵이 선다(2026-09-08). 남은 것은 소품과 문 옆 벽 토막."
metadata: 
  node_type: memory
  type: project
  originSessionId: ee12e45a-7094-4ac9-8c5d-3ded5da11c70
  modified: 2026-09-08T03:52:35.078Z
---

**맵 생성기 재작성이 돌아간다.** `cl9.seed 873`으로 `Lvl_Henhouse`에 실제 맵이 선다.

사용자 확인(2026-09-08): "미로 형식이라는 게 진짜 미로를 만들라는 게 아니라, **미로처럼 매번 맵이
달라지는 걸** 만들어 달란 거야. 가구도 배치하고." / **"이전에 CCL8에서 만든 미로는 잊어"**

**Why:** 격자 미로(Kruskal·OpenH/OpenV·케이지 열·플러그)는 CCL8 유산이고 목표가 아니었다.

**How to apply:**
- 정본은 `docs/plan/data/module.md`, 구현 상태는 `docs/systems/map.md` 끝 절이다.
- **미결 5개 확정(2026-09-08 사용자)**: ① 소켓은 **경계 칸 단위**(표는 N/S/E/W 4비트) — 크기가
  홀·짝 섞여 있어 "변 중앙 하나"가 성립하지 않는다. ② 높이차 **전파 안 함(net 0)** — 기준은
  "나중에 에셋 추가돼도 잘 적용돼야". 고리(우회로)를 살릴 수 있다. ③ **입구 1 + 비상구 1.**
  ④ 칸 목록 위에 **`LevelPoint`**, 용도는 모듈 표의 `PointPurposes` 컬럼이 정한다.
  ⑤ **20 → 24 → 30모듈**.
- **실측으로 확인된 것(2026-09-08)**: `cl9.seed 873` = 1일차 20모듈 · `… henhouse 10` = 10일차
  30모듈 · `cl9.seed 4242 henhouse 5` = 5일차 24모듈. **일자 커브가 설계값을 그대로 낸다.**
  20모듈 판이 60m × 95m. 유닛 테스트 200판은 목표 24에 평균 24.0 · 막다른 방 29% · 고리 문 4.7개.
- **구현 파일**: `CL9ModuleDef.h/.cpp`(신규) · `CL9SiteLayout.h/.cpp`(전면 재작성) · `CL9SiteMap`
  (`Rebuild`에 Day 인자, `PointToWorld`) · `CL9DefRegistry`(`FindModule`/`GatherModules`) ·
  `CL9SiteDef`(맵 컬럼 13개 삭제, `CellSize`·`BaseModuleCount`·`EntryOffset` 추가) ·
  `CL9ProgressionDef`(`SiteSizeCurve`) · `CL9GameMode::SpawnNight` · `cl9.seed <N> [site] [day]`.
  테스트 `CL9.Map.Determinism`/`Assembly`/`Segments`.
- **에셋**: `ModuleTable`(신규, 16행) · `CL9DefList`(등록) · `CL9Progression`
  (`SiteSizeCurve` 0.85/1.0/1.25) · `SegKindTable`(`ceiling` → `__ceiling_off__`).
- **곁가지 단계에서 배치 실패로 예산을 깎으면 안 된다** — 깎았더니 평균이 24.0 → 20.6으로 떨어졌다.
  자리가 안 맞은 것과 쓸 모듈이 없는 것은 다르므로, 연달아 48번 막힐 때만 판을 닫는다.
- **`door` 조각을 문틀로 쓰지 말 것** — 그 메시를 개구부에 채우면 콜리전이 생겨 문이 통째로 막힌다.
  지금은 문이 문틀 없는 개구부다.
- **천장은 꺼 둔 상태**(사용자: "천장없는건 일단 유지해. 나중에 맵만들어지는거보고 천장 만들던말던").
  그래서 로그에 `seg kind 'ceiling' has no usable row` 에러가 뜨는데 **의도된 것이다.**
- **남은 것**: ① 문 옆 벽 토막이 −75%로 눌린다(칸 500에 문 250 → 토막 125에 500짜리 조각 한 장).
  온전한 벽은 1.00으로 맞는다. ② 소품(`PropSets`, REPO `ModulePropSwitch`) — 설계만 있다.
  cl9-26 정정: REPO의 소품 교체는 확률이 아니라 **"그 변이 연결됐는가"**로 결정된다 — 우리 결정 7과
  다르므로 구현 전에 사용자와 정할 것.

관련: [[work-progress]] · [[reference-games-analysis]] · [[unreal-mcp-usage]] · [[editor-kill-allowed]]

~~~~

### no-codex-skill.md

`memory/no-codex-skill.md`

~~~~
---
name: no-codex-skill
description: "Codex 스킬(/codex, codex:rescue, codex 리뷰)을 사용자가 다시 쓰라고 할 때까지 쓰지 않는다 (2026-09-22)"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 21945be2-cf3f-4ee2-bf35-7a7f80c2a322
  modified: 2026-09-22T03:41:43.310Z
---

Codex 관련 스킬·에이전트(`/codex`, `codex:rescue`, `codex:codex-rescue`, `codex:setup`, Codex 리뷰 핸드오프 문서 작성 요청 등)를 **사용자가 "다시 써" 라고 할 때까지 쓰지 않는다.** 정지 게이트는 이미 꺼져 있다(`reviewGateEnabled: false`).

**Why:** 2026-09-22 사용자 지시 "codex 스킬 맞음, 쓰지 말자 — 내가 다시 쓰라고 할 때까지".

**How to apply:** 막히거나 2차 의견이 필요해도 Codex 를 부르지 않는다 — 대신 Claude 서브에이전트(fable/opus)나 Workflow 패널을 쓴다. 기존 Codex 리뷰 문서(`docs/review/combat-feel-claude-handoff-2026-09-21.md`)를 읽는 것은 무방하다.

~~~~

### pie-throttle-3fps.md

`memory/pie-throttle-3fps.md`

~~~~
---
name: pie-throttle-3fps
description: "PIE 3~8fps 의 정체 = 에디터 창 최소화·백그라운드 스로틀(333ms/프레임). 측정·자동화 전에 창 복원, 잠금 CLI 는 단일 명령"
metadata: 
  node_type: memory
  type: project
  originSessionId: f52a14d7-f07c-4e57-b8a0-e24c7472e2e6
  modified: 2026-09-20T16:21:27.484Z
---

2026-09-21 성능 측정에서 확인: 에디터 창이 **최소화**되거나 포그라운드가 아니면 엔진이 정확히 333ms/프레임(3fps)로 스로틀된다
(`UEditorPerformanceSettings.bThrottleCPUWhenNotForeground` 기본 true; 최소화된 창은 플래그를 꺼도 스로틀). 그동안 메모·문서의
"이 머신 PIE 는 3~8fps"·`slomo 0.15` 보정·0.33초 샘플은 전부 이것이었다. 실제 게임은 **정지 장면** 밤 실내 13.0ms(77fps), GPU 여유, CPU 바닥 ≈12ms — 단 사용자 보고 "몬스터에게 다가갈 때 급락" 은 별개로 미측정(work-progress 09-21).

**Why:** 자동화 PIE 수치를 게임 성능으로 오해해 물리·판정 설계를 저프레임에 맞추거나(inventory.md 스윕 근거), 불필요한 렌더 최적화로
가게 된다. 사용자의 "느려진다" 는 이것이 **아니었다** — 몬스터에게 다가갈 때의 급락이었고(2026-09-21 정정), 그건 파동 링·손전등 VSM·HWRT 스켈레탈 순으로 잰다.

**How to apply:**
- 재거나 PIE 자동화를 돌리기 전에 에디터 창을 복원한다 — PowerShell `ShowWindow(h, 9)` + `SetWindowPos` TOPMOST→NOTOPMOST(PID 로 핸들 잡기).
  `Config/DefaultEditorSettings.ini`(클래스가 `config=EditorSettings` — PerProjectUserSettings 에 넣으면 안 읽힌다) 에 `bThrottleCPUWhenNotForeground=False` 를 박아 두었다(에디터 재시작 뒤 적용) — 최소화만 피하면 된다.
- 프레임을 잴 때는 `Tools/Unreal/perf_probe.py`(PIE 안, run_python.py) + `perf_parse.py`. 절차·기준선·규칙은 저장소 `docs/systems/performance.md`.
- 잠금 CLI(`cl9_lock.py acquire/release/status`)는 **단일 명령으로만** 친다 — `cd … && py -3 …` 로 이으면 훅이 못 보고 "훅 비활성"으로 찍힌다
  (이번 세션 실측; [[unreal-mcp-usage]]·[[work-progress]]).
- 다른 세션이 컴파일 중(`cl.exe` CPU 65%)이면 재지 않는다 — CPU 바닥 프레임이 그대로 늘어난다.

~~~~

### plain-korean-wording.md

`memory/plain-korean-wording.md`

~~~~
---
name: plain-korean-wording
description: "전문 용어는 정해진 이름 그대로 부른다 — rules를 \"기제\"라고 바꿔 부르는 식의 자의적 치환 금지"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c670ade9-4bff-4cee-be7b-5a3c21b9779d
  modified: 2026-09-12T15:50:33.279Z
---

정해진 이름이 있는 것(Claude Code의 rules·hook, UE의 String Table·IMC 등)은 **그 이름 그대로** 쓴다. 뜻을 풀어 쓰겠다고 "기제", "장치" 같은 다른 말로 바꿔 부르지 않는다.

**Why:** 2026-09-13 rules 기능을 설명하며 "기제는 이미 내장돼 있다"고 써서 사용자가 그게 rules를 가리키는지 되물었다. 처음엔 "어려운 한자어 문제"로 잘못 정리했는데, 사용자가 다시 바로잡았다: "한자어랑 일상어 문제가 아니지, 전문 용어를 이상하게 말하는 게 문제지."

**How to apply:** 용어는 이름만 쓴다 — "rules", "root signature". **괄호 설명을 덧붙이지 않는다**(2026-09-13 "() 설명은 하지 마"). 뜻은 사용자가 물을 때만 답한다. 이름을 다른 단어로 대체하지 않는다. 사용자가 "X가 뭐야?"라고 물으면 사전 뜻이 아니라 "내가 말한 X가 무엇을 가리키느냐"를 답한다. 관련: [[comment-language-korean]].

~~~~

### quickslot-naming.md

`memory/quickslot-naming.md`

~~~~
---
name: quickslot-naming
description: 하단 아이템 칸의 이름은 "퀵슬롯" — 2026-09-13 저녁 rename 완료(코드·에셋·문서), 옛 decisions 문안만 "핫바" 기록으로 남음
metadata: 
  node_type: memory
  type: project
  originSessionId: 45c6f188-fb1f-4565-8351-29de62c8dfa6
  modified: 2026-09-13T11:38:23.006Z
---

2026-09-13 밤샘 실행 1(`hotbar-input`)에서 만든 화면 하단 4칸 아이템 칸을 리드가 "핫바"라고 불렀는데, 사용자가 **"퀵슬롯"**으로 부르기로 정했다.
"전부 싹 다 바꿀 건데 당장 진행할 건 아님" — 문서·주석·문자열 84곳 + C++ 식별자(`HotbarSlots`·`HotbarBox`·`HotbarSlotClass`·`UCL9HotbarSlotWidget`) + 에셋(`WBP_HotbarSlot`, `WBP_HUD.HotbarBox`, `CL9Rules.HotbarSlots`)까지.

**Why:** 사용자가 쓰는 말이 문서·코드의 정본 이름이어야 한다(plain-korean-wording과 같은 원칙).

**How to apply:** 새로 쓰는 문서·주석·대화에서는 "퀵슬롯"이라고 부른다. 일괄 rename은 사용자가 시킬 때 별건 실행으로 — C++ rename은 정식 빌드 + `WBP_HotbarSlot` 리패런팅 + `CL9Rules.HotbarSlots` 프로퍼티 이동(코어 리다이렉트 또는 값 재입력 4)이 필요하다. 관련: [[work-progress]], [[plain-korean-wording]].

~~~~

### reference-games-analysis.md

`memory/reference-games-analysis.md`

~~~~
---
name: reference-games-analysis
description: "리썰컴퍼니·R.E.P.O. 설치본을 디컴파일·에셋덤프로 분석하는 방법과, 그 결과 문서가 있는 곳 (2026-09-08 전면 갱신)"
metadata: 
  node_type: memory
  type: reference
  originSessionId: db5f535e-e622-4b7e-90b0-c6d01210c4ef
  modified: 2026-09-15T13:28:20.768Z
---

사용자가 참고 게임으로 지목한 둘을 설치본에서 직접 분석했다. **에셋은 가져오지 않는다 —
구조와 수치만 참고한다.** 결과 전문은 저장소의 `docs/reference/` 폴더에 영역별로 있고,
색인은 그 안의 `README.md`다.

- 리썰컴퍼니 `C:\Program Files (x86)\Steam\steamapps\common\Lethal Company`
- R.E.P.O. `D:\SteamLibrary\steamapps\common\REPO`

**Why:** 다시 조사할 일이 생길 때 방법을 재발견하지 않기 위해서다. 2026-09-07~08의 1차
조사는 DLL 문자열만 훑는 방식이라 추정이 많이 섞였고, 2026-09-08에 아래 방법으로 다시 해서
여러 개를 **정정**했다. 문자열 훑기로 끝내지 말 것.

**How to apply (유니티 게임 뜯는 법 — 이 순서대로):**
1. `*_Data/Managed/`의 DLL 목록으로 엔진 구조를 본다. 리썰은 `DunGen.dll`(기성품), REPO는
   없어서 자체 `LevelGenerator`다.
2. **`Assembly-CSharp.dll`을 통째로 디컴파일한다.** `ilspycmd`가 이미 설치돼 있다
   (`C:\Users\user\.dotnet\tools\ilspycmd.exe`). `ilspycmd -p -o <출력폴더> -r <Managed폴더> <dll>`
   → 클래스당 `.cs` 하나. REPO 963개, 리썰 386개. 동작 설명은 전부 여기서 나온다.
   리썰의 `PlayerControllerB.cs`만 하위 폴더 `GameNetcodeStuff\`에 있다.
3. **에셋의 실제 수치는 `UnityPy` + `TypeTreeGeneratorAPI`로 읽는다**(둘 다 pip로 설치해 둠).
   두 게임 다 IL2CPP가 아니라 Mono라 타입트리를 DLL에서 생성할 수 있다. 핵심 코드:
   `gen = TypeTreeGenerator(env.assets[0].unity_version); gen.load_local_game(게임루트)`
   → `env.typetree_generator = gen` → `o.read_typetree()`. 클래스 이름은
   `o.parse_monobehaviour_head().m_Script.deref_parse_as_object().m_ClassName`으로 얻는다
   (`o.read()`는 엉뚱하게 파싱되니 쓰지 말 것). `load_local_game`에는 `_Data`가 아니라
   **게임 루트**를 넘긴다.
   **코드의 필드 초기값 ≠ 에셋의 실제 값이다. 밸런스 수치는 반드시 이쪽을 봐야 한다.**
   (2026-09-15 실례: 리썰 `movementSpeed` 코드 0.5f, 프리팹 직렬화 실값 **4.6** — 문서에 0.5로 적혀 있던 것을 정정했다.)
   2026-09-15 정정 둘: import 는 `from UnityPy.helpers.TypeTreeGenerator import TypeTreeGenerator`
   (`TypeTreeGeneratorAPI` 패키지 자체에는 `load_local_game` 이 없다). 기본 백엔드(AssetsTools)·AssetRipper 로
   `RoundManager` 같은 큰 NetworkBehaviour 는 EOFError 가 나니 `TypeTreeGenerator(ver, generator="AssetStudio")` 로.
   리썰 던전 데이터(Tile/TileSet/DungeonFlow/DoorwaySocket/SelectableLevel)는 전부 `sharedassets3.assets` 한 파일,
   `RoundManager`/`StartOfRound` 는 `level3`. 던전 축척 실측 보고서: 저장소 `.claude/runs/manual-map-20260914/lc-scale.md`.
4. 에셋 이름만 훑어도 설계도가 나온다. 명명 규칙이 곧 구조다.

**1차 조사에서 정정된 것 (문자열만 보고 틀렸던 것들)**
- REPO 모듈은 308개가 아니라 **311개**(+디버그 20). 테마는 스테이지 4 + 상점 1.
- 이름 넷째 칸은 희귀도가 아니라 **난이도 티어 1·2·3**이다. `DifficultyCurve1..3`을 완료
  레벨 수로 평가해 가중치를 만든다.
- `ModulePropSwitch`는 확률이 아니라 **그 변이 연결됐는지**로 결정된다.
- `LevelSizeMultiplier`는 게임 곡선이 아니라 **디버그 노브**다. 맵 크기에 플레이어 수는 안 들어간다.
- REPO는 **시드를 공유하지 않는다.** 호스트가 만들고 룸 오브젝트로 복제한다.

**두 게임의 핵심 차이**: 리썰은 자유 타일 그래프(DunGen), REPO는 정사각 격자 + 변 중앙
사방 소켓. **CL9은 REPO 쪽을 주 모델로 삼았다**([[map-module-rewrite]]).
트럭·우주선은 둘 다 레벨 밖이다.

관련: [[map-module-rewrite]] · [[work-progress]]

~~~~

### session-continuity.md

`memory/session-continuity.md`

~~~~
---
name: session-continuity
description: 작업 진행 기록을 항상 남겨 새 세션에서 바로 이어서 작업하게 한다 (사용자 요청)
metadata: 
  node_type: memory
  type: feedback
  originSessionId: ce99b7ea-3df1-4c0f-9c1d-5f287190d847
  modified: 2026-09-04T15:33:47.374Z
---

사용자는 새 세션을 시작해도 직전 작업을 바로 이어서 하길 원한다 (2026-09-05 요청).

**Why:** 매 세션 git log·문서를 다시 뒤져 현재 위치를 재구성하는 비용을 없애기 위해.

**How to apply:**
- 의미 있는 작업 단위가 끝나거나 세션을 마무리할 때마다 [[work-progress]]를 갱신한다
  — 완료된 것, 다음 작업, 미결 사항(튜닝 값·의도적 미커밋 등)을 짧게.
- 새 세션에서 사용자가 "이어서 하자" 류의 말을 하면 [[work-progress]]부터 읽고 바로 착수한다.
- 상세 내용은 repo(docs/, git log)가 정본 — work-progress에는 포인터와 미결만 남기고 중복 서술하지 않는다.

## 레벨 커밋 규칙 (2026-09-06 변경)
- `.umap`도 커밋한다. 커밋 전 크기를 확인해 100MB 근처면 원인(라이팅 빌드·큰 에셋 참조)을 먼저 잡는다.
  예전 "Lvl_Flat.umap 미커밋" 규칙은 폐기(CCL8 177MB 사고의 예방책이었음). 여전히 제외: .claude/settings.json,
  T_Plaster_02_C1.uasset(작업과 무관한 팩 텍스처 변경), UI 세션 도메인 파일.

~~~~

### steam-release-plan.md

`memory/steam-release-plan.md`

~~~~
---
name: steam-release-plan
description: Steam 출시 계획(docs/goal/steam-release.md)이 2026-09-06 /autoplan으로 작성·리뷰됨 — 사용자 최종 게이트 결정과 검증된 엔진 사실
metadata: 
  node_type: memory
  type: project
  originSessionId: a742006d-e2af-4706-87ca-39a398a17152
  modified: 2026-09-06T13:53:26.198Z
---

2026-09-06 사용자 질문("스팀에 올릴껀데 뭐해야돼 / 서로 어떻게 방에 들어가 / 서버 운영")에 대한 답으로
`D:\Work\unreal\CL9\docs\goal\steam-release.md`(W0 서류 → W1 엔진 배선 → W2 세션 층 → W3 화면 →
W4 패키징·업로드 → W5 문서)를 쓰고 /autoplan 3단계 리뷰(CEO·디자인·엔지니어링, Codex 불가로 Claude 단독)를 돌렸다.
보조 산출물: `D:\Work\unreal\CL9\TODOS.md`(신규), `C:\Users\user\.gstack\projects\Work\ceo-plans\2026-09-06-steam-release.md`,
테스트 플랜 `...\Work\user-master-eng-review-test-plan-20260906.md`, 태스크 JSONL(tasks-*-review-20260906-*.jsonl, 39개).

**최종 게이트 결정(사용자 몫, 답이 오면 여기 갱신):** 취향 4개(단일 레벨 로비/세션 enum 없음 · 밤중 참가 거부 ·
첫 스팀 산출물 Playtest/데모 vs 출시 · 로스터 조작은 키 vs 트럭 E) + 사용자 도전 1개(W2~W4를 4e~4i·세이브
이후로 미룰지). 아직 미답.

**Why:** 세션 층은 build-order 상 ④ 착수 전에 있어야 했는데 아직 코드가 0줄이고, 1f는 PIE 전용이었다.
서버 운영은 없다(리슨 서버 + Steam 릴레이).

**How to apply:** Steam 작업을 시작하면 이 계획 파일의 본문(첫 ~330줄)과 Implementation Tasks를 정본으로 삼는다.
엔진 소스로 확정한 사실: `bInitServerOnClient` 불필요, `IOnlineSubsystem::Get()`은 Null 폴백 때문에 null이 아님,
UE URL 옵션은 `?` 구분, 로비 검색엔 `SEARCH_LOBBIES` 키, ini 인라인 `;` 주석 금지, Development 빌드는 스팀에 올리지
말 것(치트). 관련: [[work-progress]] [[session-continuity]]

~~~~

### subagent-model-allocation.md

`memory/subagent-model-allocation.md`

~~~~
---
name: subagent-model-allocation
description: CL9 서브에이전트 모델은 역할이 아니라 일의 성격으로 고르고 띄울 때 model 을 명시한다 — 정본 docs/workflow/sessions.md "모델 배분"
metadata:
  type: feedback
---

서브에이전트를 띄울 때 일의 성격으로 모델을 고른다: 판단·설계·첫 형태 = opus, 승인된 것의 반복·기계적 적용(변형·임포트·배치 스크립트·표 행·정해진 문서) = sonnet, 색인 한 줄 같은 단순한 일 = haiku. 리드는 사용자가 고른 모델(보통 Fable). 역할 정의 기본값이 opus 라서 sonnet/haiku 는 Agent 호출에 `model` 을 명시해야 한다.

**Why:** 2026-10-04 사용자 "뭐 만들 때 다 fable 아니고 몇몇 서브에이전트는 딴 거로", "1회용으로 하지 말고 그 규칙대로 앞으로 진행하게 남기자".

**How to apply:** `/cl9 task` 로 역할을 dispatch 할 때마다 brief 별로 모델을 정해 명시하고 이유 한 줄을 실행 기록에 남긴다. 애매하면 한 단계 위. 정본 문서가 바뀌었으면 그쪽을 따른다.

~~~~

### unreal-mcp-usage.md

`memory/unreal-mcp-usage.md`

~~~~
---
name: unreal-mcp-usage
description: "CL9 언리얼 에디터 MCP(포트 8000) 사용법 — 도구 등록 조건, 툴셋 API 규칙, 함정"
metadata: 
  node_type: memory
  type: project
  originSessionId: 5698782d-d8ba-4b24-910b-2b240c853261
  modified: 2026-09-16T12:36:50.495Z
---

CL9의 unreal-mcp는 UE 5.8 내장 ModelContextProtocol 플러그인이다 (외부 서버 아님).
`Config/DefaultEditorPerProjectUserSettings.ini`의 `bAutoStartServer=True`로 에디터 기동 시
`http://127.0.0.1:8000/mcp` 자동 시작 (2026-09-02 설정).

**Why:** 세션마다 재발견하면 시간이 크게 낭비되는 API 규칙·함정들이 있다.

**How to apply:** 에디터·PIE·빌드를 여러 세션이 나눠 쓰는 규약(잠금·통지·유령 에디터)은 저장소 `docs/workflow/sessions.md`가 정본이다 —
2026-09-13부터 훅이 `cl9_lock.py` 잠금 없는 StartPIE/StopPIE·taskkill 을 막는다. 아래는 MCP API 요령.
- Claude Code 세션 시작 시 에디터가 꺼져 있으면 `mcp__unreal-mcp__*` 네이티브 도구가 등록되지
  않는다 → 에디터를 먼저 켜고 세션을 열거나, PowerShell로 JSON-RPC를 직접 호출한다
  (initialize → notifications/initialized → tools/call, `Mcp-Session-Id` 헤더 유지).
- 도구 검색 모드: 메타 도구 3종(list_toolsets / describe_toolset / call_tool)만 노출.
  실제 호출은 `call_tool {toolset_name, tool_name(접두사 없이), arguments}`.
  **toolset_name은 list_toolsets가 보여주는 점 포함 전체 이름**이다 (`UMGToolSet.UMGToolSet`,
  `editor_toolset.toolsets.object.ObjectTools`, `EditorToolset.EditorAppToolset`) — 짧은 이름은
  'not found'. (2026-09-05 실측)
- 툴셋 API 규칙: 프로퍼티 이름은 **camelCase**(`defaultPawnClass`), 오브젝트 참조는
  `{"refPath":"/Game/.../Asset.Asset"}`, BP 클래스는 `...Asset.Asset_C`,
  `set_properties`의 `values`는 **JSON을 다시 문자열로 감싼 것**. 인스턴스드 서브오브젝트는
  클래스 경로(`/Script/EnhancedInput.InputModifierNegate`)를 넣으면 생성된다.
- UE 5.8 IMC의 키 매핑은 `mappings`(레거시, 비어 있음)가 아니라
  `defaultKeyMappings.mappings`에 있다.
- **StartPIE 함정:** 컴파일 실패 BP가 있으면 모달 대화상자가 게임 스레드를 잠가 MCP 전체가
  멈춘다. 해결: 창 핸들 열거 후 WM_CLOSE(0x0010) 전송. PIE 도구 호출은 타임아웃을 길게.
- `CaptureViewport`는 `captureTransform:null, annotations:null`을 명시해야 하고, 결과가
  1MB급 base64 JSON이므로 반드시 파일로 저장해 처리한다.
- **set_properties는 패키지를 dirty로 안 만든다** (2026-09-02 실측): `save_assets([])`(dirty만
  저장)에 안 걸리고, PIE의 GameMode 스폰이 CDO 수정 전 저장 상태를 따라가는 현상까지 확인.
  해결: CDO/에셋을 set_properties로 고친 뒤 **반드시 경로를 명시해 save_assets** + BP면
  compile_blueprint 후 저장. (BP CDO 수정 → PIE 반영은 이 순서를 지켜야 확실하다.)
- 네이티브 도구가 등록돼 있으면 `mcp__unreal-mcp__call_tool {toolset_name, tool_name(접두사
  없이), arguments}`로 호출 — 파라미터명은 describe_toolset의 스키마를 따른다 (list_properties는
  `instance`, get_properties는 `instance`+`properties`).
- PIE 실행 중에는 AssetTools의 is_dirty/save 계열이 "Asset does not exist"로 실패한다 —
  StopPIE 후 호출.
- **콘솔 명령 실행 경로**: EditorAppToolset에 exec 도구가 없다 → SlateInspector로 에디터
  하단 Cmd 바에 타이핑한다 (Snapshot에서 "Cmd" 콤보 옆 textbox 찾기 → Click으로 포커스 →
  Type submit:true). 이 경로로 `LiveCoding.Compile`을 치면 **에디터 켠 채 .cpp 로직 수정을
  라이브 컴파일**할 수 있다 (2026-09-02 실증 — 헤더 변경은 여전히 에디터 끄고 정식 빌드).
- 배열 프로퍼티에 항목 추가는 "기존+신규" 한 번에 쓰면 ArrayAdd 모호성 오류가 난다 —
  빈 배열로 한 번 지우고 전체를 다시 쓴다.
- **save_assets([]) 전체 저장 금지** (2026-09-02 사고): 삭제했던 에셋의 인메모리 패키지가
  dirty로 남아 있다가 전체 저장 때 디스크로 부활한다. 항상 경로를 명시해 저장한다.
  BP CDO 수정은 compile_blueprint + 명시 저장을 **PIE 시작 전에** 마쳐야 인스턴스에 반영된다.
- 자동화 테스트는 AutomationTestToolset: DiscoverTests → RunTestsByFilter("StartsWith:CL9").
  실행하면 돌던 PIE가 종료된다.
- **SlateInspector Windows close 금지** (2026-09-02 사고): 인덱스 기반이라 창 순서가 밀리면
  에디터 본체를 종료시킨다 — 실제로 메시지 로그를 닫으려다 에디터가 통째로 꺼졌다.
  가리는 창은 그냥 두고, 특정 위젯만 Screenshot(ref)으로 찍는다.
- IMC defaultKeyMappings를 배열째 다시 쓰면 기존 인스턴스드 모디파이어 refPath에
  "찾지 못했습니다" 경고가 뜨지만 기존 서브오브젝트는 유지된다 (값 재확인으로 검증).
- **get_node_type_pins의 프로브 노드는 일시적** (2026-09-03 사고): 그래프에 실존 refPath로
  생성돼 연결까지 되지만 컴파일/저장 즈음 소멸한다 — ABP 루트 체인이 끊겨 T포즈가 났다.
  실제 노드는 반드시 create_node로 만들고, 저장 후 find_nodes로 생존 확인.
- **AnimBP 애님 그래프는 MCP로 저작 가능** (2026-09-03 실증): 팔레트에 "애니메이션|시퀀스|
  'A_X'재생", "애니메이션|블렌드|양방향블렌드" 등이 잡힘 → create_node+connect_pins로
  포즈 체인 구성 (스테이트 머신 없이 속도 알파 2단 블렌드면 로코모션 충분). 에셋 생성은
  py AnimBlueprintFactory(target_skeleton 지정). write_graph_dsl은 exec 그래프 전용이고,
  (event X) 폼은 "AddEvent|X"를 찾는데 한국어 에디터 팔레트는 "이벤트추가|"라 실패 —
  이벤트 노드도 create_node("이벤트추가|이벤트X")로 직접 만든다.
- **CaptureViewport는 에디터 카메라 씬을 찍는다** — PIE 게임 화면이 아니다. PIE 실제
  픽셀은 SlateInspector Screenshot(ref=뷰포트 위젯/창)으로 찍는다 (base64 1MB급 →
  파일 저장 후 Read).
- PIE 중 게임 월드 검사는 py: `unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
  .get_game_world()` + GameplayStatics.get_all_actors_of_class. protected UPROPERTY는
  get_editor_property가 거부한다 → BlueprintPure 게터(get_detect_gauge 등)를 쓴다.
  set_actor_location으로 순간이동시켜 추격 해제 같은 거리 조건도 실측 가능.
- StartPIE "Timed out waiting" ≠ 실패 — 기동이 도구 대기보다 길면(35s) 타임아웃이 떠도
  PIE는 켜진다. 로그(PIE: 클라이언트가 로그인)로 확인.
- **Live Coding은 신규 static 등록물(FAutoConsoleCommand 등)을 반영 못 한다** (2026-09-03):
  기존 함수 로직 패치는 되지만 새 콘솔 명령의 정적 초기화는 안 돌아 명령이 미등록으로 남는다
  (콘솔에 쳐도 `Cmd:`만 뜨고 핸들러 로그 0). 신규 등록물은 정식 빌드로 굳힌다. 단일 .cpp
  변경이면 kill→Build.bat이 10초.
- **py로 호출 가능한 건 UFUNCTION만** — 순수 virtual(예: OnDamage)은 파이썬에서 못 부른다.
  런타임 판정을 py로 찔러보려면 BlueprintCallable이거나 콘솔 치트 경로가 필요하다.
- **클라 표시용 Tick이 OnRep 표현을 덮어쓰지 않게 하라** (2026-09-03): 몬스터 클라 Tick이
  매 프레임 메시 월드 트랜스폼을 갱신하면 OnRep_State(ApplyState)가 준 죽음 눕힘·회전이
  즉시 지워진다 — 표현 상태에서는 Tick 갱신을 건너뛰거나 스무딩에 반영해야 한다.
- 네이티브 MCP 도구가 미등록인 세션에서 JSON-RPC 직접 호출이 실증됨 (2026-09-05): initialize
  응답 헤더의 `Mcp-Session-Id`를 유지하며 tools/call. 헬퍼는 scratchpad에 mcp.sh/wire_sleep.py
  패턴으로 만들면 된다 (curl 또는 urllib).
- BlueprintTools 파라미터 함정: `find_nodes`는 `title` 필수(전체 조회는 `""`),
  `find_node_types`는 `context_pins` 필수(빈 배열 OK — 스포너 title이 create_node의 type_id).
  `save_assets`는 `asset_paths`(문자열 배열, `/Game/...` 확장자 없음). 로그는
  LogsToolset.GetLogEntries(category, pattern, maxEntries) — pattern은 정규식.
- **PIE 실행 중 add_variable/compile_blueprint 금지** (2026-09-05 사고): 플레이 중 ABP에 변수
  추가→컴파일하면 리인스턴싱으로 스폰된 몬스터가 T포즈가 된다 (이동 코드는 계속 돌아 "T자로
  달리기만"). BP 구조 변경 전 IsPIERunning으로 확인하고, PIE 정지 후 편집→컴파일→저장.
- **UMG 저작은 UMGToolSet으로** (2026-09-05 실증): CreateWidgetBlueprint(parentClass에 C++ 클래스
  refPath) → AddWidget(트리 구성) → ObjectTools.set_properties(슬롯 앵커·텍스트 — 위젯/슬롯마다
  list_properties 먼저) → BindToEventProperty(버튼 OnClicked 핸들러 노드 생성) →
  BlueprintTools create_node/connect_pins(핸들러→함수 연결) → CompileWidgetBlueprint → save.
  BindWidget 이름 불일치는 CompileWidgetBlueprint가 잡아준다.
- **py에서 GameInstanceSubsystem 접근**: SubsystemBlueprintLibrary가 py에 미노출 →
  `gi=GameplayStatics.get_game_instance(world)` 후 `unreal.find_object(gi,'클래스명_0')`.
- **런타임 UserWidget의 트리는 'WidgetTree_0'** (에셋은 'WidgetTree') — 서브위젯 경로 탐색은
  `unreal.ObjectIterator(unreal.Button)`로 path_name을 찾는 게 확실하다.
- **SlateInspector Click은 게임 내 UMG 버튼에 안 통한다** (지오메트리가 desired size 기반이라
  실제 페인트와 어긋남) — 버튼 검증은 `btn.on_clicked.broadcast()`로 델리게이트를 직접 발동.
- **OS 레벨 클릭(SetCursorPos+mouse_event)은 이 터미널 창이 에디터를 덮고 있으면 터미널에
  떨어진다** (2026-09-05 실측 — WindowFromPoint로 확인) — 블라인드 OS 클릭 금지.
- Cmd 콘솔 py 한 줄에 `|` 문자가 들어가면 거기서 잘려 SyntaxError가 난다.
- PIE 플레이어 수 변경: `unreal.load_object(None,'/Script/UnrealEd.Default__LevelEditorPlaySettings')`
  에 `set_editor_property('PlayNumberOfClients', n)` (snake_case는 실패, 원표기 사용).
- **PIE 게임 입력 자동화 레시피** (2026-09-05 확립, scratchpad/sendkey.ps1):
  1. 뷰포트 내 PIE는 게임 키가 에디터 단축키와 겹친다(F=선택 액터 포커스) → 키 검증은
     `PlayMode_InEditorFloating`(별도 창)으로 돌린다.
  2. 창 핸들은 **제목이 아니라 UnrealEditor.exe PID로 매칭**한다(재기동마다 핸들이 바뀌고,
     .ps1은 ANSI로 읽혀 한글 제목 매칭이 깨진다 — 스크립트는 ASCII만).
  3. SetForegroundWindow만으로는 포그라운드 탈취가 거부된다 → SetWindowPos TOPMOST로 올리고
     입력 후 NOTOPMOST로 되돌린다. 클릭 전에 그 좌표의 최상위 창이 에디터인지 검사(오클릭 방지).
  4. **게임에 포커스를 줄 때 좌클릭 금지**(IA_Throw가 발동한다) — 우클릭으로 준다.
  4-1. **모디파이어(Shift)는 스캔코드 없이는 안 먹는다** — `keybd_event(vk, MapVirtualKey(vk,0), ...)`로
     스캔코드를 실어야 달리기가 등록된다(안 그러면 W만 들어가 걷기만 함). 길게 누르는 조합키는
     200ms마다 down을 재전송해야 눌린 상태가 유지된다.
  5. **콘솔에 py를 칠 때마다 포커스가 에디터로 넘어가 게임 입력이 끊긴다** — 키 입력과 측정을
     번갈아 할 수 없다. 상태를 얼리려면 PIE 일시정지… 인데 **일시정지 중에는 Cmd 콘솔이 응답을
     멈춘다**(실측). 대신 데이터 값을 임시로 늘려 측정 창을 벌리는 편이 낫다
     (예: StaminaRegenTime 8→15로 올려 측정 후 원복).
  6. **밸런스 값을 임시로 만질 때 관계 검사(A1/A2)를 같이 봐야 한다** (2026-09-05 사고):
     StaminaRegenTime을 180으로 올렸더니 A1(지친 지속속도 > 몬스터 추격)이 깨져 **farmer_a 행이
     레지스트리에서 통째로 제외**됐고, Def가 안 붙어 스태미나가 아예 안 닳았다. 증상이 "기능이
     안 되는 것"처럼 보이니 LogCL9의 A1/A2 에러부터 확인할 것. farmer_a 기준 A1 통과 상한은
     regen ≈ 17.4초.
- **PIE 게임 입력은 Enhanced Input 주입이 가장 확실하다** (2026-09-05 실증, cl9-7d): OS 키 입력(keybd_event)은
  PIE 창을 포그라운드로 올려도(AttachThreadInput+ALT 트릭으로 성공) 게임에 안 들어갔고, 우클릭 포커스는
  시야까지 돌려 포커스 대상이 바뀐다. 대신 py로 `[x for x in unreal.ObjectIterator(
  unreal.EnhancedInputLocalPlayerSubsystem) if x.get_name()=='EnhancedInputLocalPlayerSubsystem_0'][0]`를 잡고
  `inject_input_vector_for_action(unreal.load_asset('/Game/Input/Actions/IA_X'), unreal.Vector(1,0,0), [], [])`
  (한 프레임 = Started/Completed 한 번). 홀드는 `builtins.h=unreal.register_slate_post_tick_callback(lambda dt: ...inject...)`
  로 매 프레임 주입하고 `unreal.unregister_slate_post_tick_callback(builtins.h)`로 해제 — Completed가 뜬다.
  `start_continuous_input_injection_for_action`은 raw_value(InputActionValue) 변환 실패로 py에서 못 쓴다.
  콘솔에 py를 쳐도 주입은 유지되므로 홀드 중 값을 읽을 수 있다(OS 입력 홀드와 달리).
  py 주의: `pc.get_pawn()` 없음 → `GameplayStatics.get_player_pawn(w,0)`; `actor.get_root_component()` 없음 →
  `actor.root_component`; Server RPC(UFUNCTION(Server))는 py 미노출. 아이템(계란)은 물리 시뮬로 굴러다니니
  `root_component.set_physics_linear_velocity(Vector(0,0,0))` 후 테스트.
- **PIE 화면 스크린샷을 컨텍스트 없이 파일로**: SlateInspector Screenshot(ref=PIE 창 ref, 예 w7)을 JSON-RPC로 직접
  호출해 base64를 PNG로 저장(scratchpad/shot.ps1 패턴) → Read로 본다. 네이티브 도구로 부르면 400KB base64가
  컨텍스트에 그대로 들어온다. PIE 창 ref는 Windows list → Snapshot(ref "", maxDepth 6)에서 "CL9 프리뷰" 창.
  Cmd 바 textbox ref는 메인 창 Observe 후 Snapshot(w1, 40)에서 "Cmd" 콤보 옆 textbox(이번 세션 tb2).
- **카메라 셰이크 검증법**: `get_camera_rotation()`에는 셰이크가 안 실린다(항상 델타 0).
  대신 `unreal.ObjectIterator(unreal.CameraShakeBase)`로 인스턴스 존재를 확인하고
  `shake_scale`을 읽어 설계 공식과 대조한다(CameraModifier_CameraShake 아래 생성됨).
- 카메라 셰이크 BP 생성: asset_type은 `/Script/GameplayCameras.DefaultCameraShakeBase`,
  패턴 클래스는 `/Script/EngineCameras.PerlinNoiseCameraShakePattern` (Engine 모듈 아님).
- 워크플로 순서는 [[cl9-editor-build-cycle]] — 에디터 끄기→빌드→켜기.
- **백그라운드 빌드는 턴이 끝나면 죽는다** (2026-09-05 실측 2회): Bash/PowerShell `run_in_background`
  빌드가 턴 종료와 함께 killed → UBA "not gracefully shutdown". 정식 빌드는 **포그라운드**(timeout
  600000)로 돌리고 출력은 Out-File로 로그 파일에 남긴 뒤 Select-String으로 요약만 본다.
  이 머신 전체 빌드 ≈ 8분(MaxProcessorCount 4). Live Coding도 .cpp 하나(CheatManager)에 20분 걸렸다 —
  `LogLiveCoding` "Live coding succeeded"로 완료 확인.
- **SceneTools add_to_scene_from_asset은 xform에 없는 필드를 항등으로 덮어쓴다** → BP CDO의 루트
  스케일(예: 패드 4×4×0.2)이 인스턴스에서 1로 초기화된다. 배치 직후 ActorTools.set_actor_transform으로
  스케일을 다시 지정하고 get_actor_bounds로 확인. 자식 컴포넌트는 `bAbsoluteScale`로 부모 스케일에서
  분리할 수 있다(ObjectTools set_properties, 프로퍼티명 그대로 `bAbsoluteScale`).
- **list_properties를 프리미티브 컴포넌트에 치지 말 것** — 수천 줄이 쏟아진다. 엔진 프로퍼티는 원명
  그대로(`bAbsoluteScale`, `staticMesh`, `relativeScale3D`) 추측해서 get_properties로 확인하는 편이 싸다.
- **py 게임 조회 관용구** (2026-09-05): `pc.get_pawn()`/`pc.pawn`은 미노출 →
  `GameplayStatics.get_player_pawn(w,0)`; GameState는 `GameplayStatics.get_game_state(w)`;
  다른 플레이어 폰은 `gs.player_array[i].get_pawn()`; 2인 PIE의 클라 세계는
  `unreal.find_object(None,'/Game/Maps/UEDPIE_1_Lvl_Flat.Lvl_Flat')` → `get_player_controller(cw,0)`로
  클라 쪽 OnRep 결과(is_spectating 등 BlueprintPure)를 읽을 수 있다. Cmd 콘솔 치트(cl9.*)는 서버 세계에서
  실행되므로 클라 대상 판정은 인자로 플레이어 번호를 받는 치트(cl9.damage N idx)로 낸다.
- **PIE 중 DA 값 set_properties는 즉시 반영된다**(레지스트리가 같은 에셋 객체) — "값 바꾸면 반영"
  검증에 쓰고, 끝나면 원복 + StopPIE 후 save_assets.
- **CL9DefList는 MCP 저장으로 재스캔되지 않는다** (2026-09-05): 새 Def 에셋을 만들고 save_assets로
  DefList를 저장해도 PreSave RefreshFromFolder가 새 에셋을 안 잡는다(패키지가 dirty가 아니라 저장이
  건너뛰는 듯). 해결: `defs`를 `[]`로 비운 뒤 전체 목록을 `{"refPath":...}`로 다시 set_properties →
  save_assets. 데이터 에셋 생성은 `DataAssetTools.create(folder_path, asset_name, asset_type refPath
  /Script/CL9.XxxDef)`, 인스턴스드 행 추가는 `rows`에 `{"refPath":"/Script/CL9.CL9SiteDef"}`(클래스 경로)
  → get_properties로 생성된 서브오브젝트 이름(CL9SiteDef_0) 확인 → 값 set. FRuntimeFloatCurve는
  `{"editorCurveData":{"keys":[{"time":1,"value":0.6,"interpMode":"RCIM_Linear"}]}}`로 바로 들어간다.
- **py로 부른 함수 안의 Server RPC는 호스트에 안 간다** (2026-09-05 실측): 파이썬 호출은 에디터 스크립트
  실행 가드(GAllowActorScriptExecutionInEditor) 아래서 돌아 `GetFunctionCallspace`가 Local을 돌려준다 —
  클라 세계의 PC에서 `accept_orders`·`execute_console_command('cl9.pick …')`를 불러도 서버에 로그가
  안 남는다. 클라→호스트 판정을 검증하려면 **Enhanced Input 주입**(다음 틱에 정상 경로로 실행)이나
  실제 입력이 필요하다. 호스트 쪽 판정·복제 결과는 py로 읽어도 된다(BlueprintPure/UPROPERTY).
- PIE 창이 2개(Server 0·Client 1)로 뜨면 PlayNumberOfClients가 2로 남아 있던 것 — StartPIE 전에 값을 확인.
- **Cmd 바 py 한 줄에 `|` 문자 금지** (2026-09-05): 파이프 뒤가 잘려 SyntaxError. `;;`도 파이썬 SyntaxError다
  (2026-09-10 실측) — 여러 문장은 scratchpad에 .py로 써서 `py "경로"`로 돌린다.
  PIE 창이 포커스를 잡고 있으면 SlateInspector Type이 false를 돌려준다 → 먼저 Click(tb1)으로 포커스.
  PIE 재시작 후 PIE 창 ref가 바뀐다(w43→w44) — Snapshot("",1)로 다시 읽는다. 메인 창 Cmd textbox는 tb1.
- **몬스터 애님 육안 검증 레시피**: `slomo 0.2`로 늦추고, py로 플레이어를 몬스터 앞 250~300cm에 set_actor_location +
  set_control_rotation으로 세운 뒤 scratchpad/shot.sh(JSON-RPC Screenshot → PNG, base64는 컨텍스트 밖)로 찍는다.
  몬스터 상태는 `get_state()`, 속도 `get_ground_speed()`, 게이지 `get_detect_gauge()`, 애님 클래스는
  `get_component_by_class(SkeletalMeshComponent).get_anim_instance().get_class().get_name()`
  (`get_editor_property('mesh')`는 BP 인스턴스에서 실패). 시야 330cm라 12m 밖에선 발각되지 않는다.
- **빌드는 분리 프로세스 + 포그라운드 대기** (2026-09-06): 도구 호출이 10분을 넘겨 백그라운드로 넘어가면 턴 종료 때
  죽는다(cl.exe까지 사라짐). `Start-Process cmd.exe -ArgumentList '/c ""…Build.bat" CL9Editor Win64 Development
  -project="…" -waitmutex"' -RedirectStandardOutput build.log`로 띄운 뒤, 9분짜리 `Start-Sleep` 루프로 "Total execution
  time"을 기다리는 호출을 반복한다. 메모리가 모자라면(커밋 32GB/물리 16GB) 파일 하나에 2~3분씩 걸려 30분도 나온다.
  PowerShell 도구는 `Remove-Item`과 같은 줄에 "D:\Program Files" 문자열이 있으면 차단한다 — 로그 이름을 새로 쓴다.
- `UCL9DefBase`에 이미 `Mesh`·`ActorClass`·`Icon` 컬럼이 있다 — 파생 Def에 같은 이름을 선언하면 UHT 섀도잉 오류.
  `UGameMapsSettings`는 EngineSettings 모듈이라 링크가 깨진다 → `GConfig->GetString("/Script/EngineSettings.GameMapsSettings", …)`.
- **ActorTools.set_actor_transform은 빠뜨린 필드를 항등으로 되돌린다** (2026-09-06 사고): location만 넘기면 scale이
  (1,1,1)로 초기화된다(Floor_0 400m 바닥이 10m로 줄어 밤 스폰 트레이스가 실패했다). 항상 location·rotation·scale
  셋을 다 넘긴다. add_to_scene_from_asset도 같다.
- **에디터를 닫고 빌드하면 70초, 에디터 켠 채 라이브 코딩은 20~30분** (2026-09-06 실측, 메모리 16GB 머신): cpp 한 줄
  고쳐도 에디터가 떠 있으면 컴파일러가 스왑에 빠진다. 변경이 작아도 "닫고 정식 빌드 → 재기동(200초)"이 더 빠르다.
  라이브 코딩은 wait_livecoding.sh가 옛 "succeeded" 줄에 속는다 → scratchpad/wait_lc2.sh(최신 줄이 Starting이 아닐 때까지).
- 네이티브 클래스를 가리키는 `ActorClass`는 그 클래스가 빌드된 뒤에만 set_properties가 받는다.
- **WBP 클래스 기본값(CDO)에 set_properties 해도 다른 WBP 디자이너에 이미 배치된 인스턴스 템플릿에는 전파되지
  않는다** (2026-09-06 사고): WBP_HUD CDO의 ScanLabelClass를 넣었는데 WBP_Main 안 HudLayer는 None → 런타임 HUD도 None.
  자식 WBP에 새 프로퍼티를 넣어 값을 줄 때는 **부모 WBP의 배치 인스턴스**(`/Game/UI/WBP_Main.WBP_Main:WidgetTree.HudLayer`)
  에도 set_properties 하고 부모를 컴파일·저장한다. C++ 기본값(UPROPERTY 초기값)은 문제없다.
- **자동 검증 중 PIE는 3fps 수준으로 돈다**(콘솔 py 타이핑이 게임 스레드를 막음) — 에디터 프레임 스무딩이 델타를
  줄여 "게임 시간 3초"가 실시간 10초 이상이 된다. 시간 기반 표현(라벨 수명·페이드)은 자동 검증 수치로 판단하지 말고
  사용자 플레이로 본다. 스캔 같은 순간 입력은 py 파일(`py "경로"`)로 주입+틱 콜백 로깅이 가장 정확하다.

- **MCP 클라이언트가 끊겼을 때(에디터 재기동 뒤 unreal-mcp ConnectionRefused)**: 포트 8000이 살아 있으면 scratchpad의
  `rpc.ps1 -Toolset "<풀 툴셋명>" -Tool <툴> -ArgsJson '<JSON>'`로 JSON-RPC를 직접 쳐서 StartPIE·Observe·Type·GetLogEntries·
  StopPIE 전부 된다(세션 id는 mcp-session.txt, 실패 시 자동 재초기화). 스크린샷은 shot.ps1. Snapshot의 maxDepth는 이 경로에서
  안 먹으니 결과를 PowerShell에서 `^window`·`textbox`로 걸러 창 ref(PIE 창)와 Cmd 텍스트박스(tb2)를 찾는다 (2026-09-06).
- **CDO 프로퍼티를 set_properties로 바꾼 뒤엔 반드시 CompileWidgetBlueprint**(2026-09-06 발견): BP 클래스는 컴파일 때 만든
  "CDO≠네이티브 기본값인 프로퍼티 목록"만 새 인스턴스에 복사하므로, 컴파일 없이 CDO만 바꾸면 CreateWidget 인스턴스는
  C++ 기본값을 받는다(WBP_Scan ScanKinds 사례). PIE 도중 바꿔도 마찬가지.
- 원시 JSON-RPC(rpc.ps1)로 set_properties를 부를 때 인자 이름은 `values`(JSON 문자열)다 — `properties`는 get_properties용.
  Snapshot 창 목록에서 Cmd 텍스트박스 ref는 창 개수에 따라 tb2/tb3으로 바뀐다.
- **`unreal.Rotator(a, b, c)`의 인자 순서는 (roll, pitch, yaw)다** (2026-09-06 실측): `Rotator(0, -90, 0)`으로 yaw를
  주려다 pitch −90(뷰 클램프에 걸려 290)이 들어가 캐릭터가 바닥만 보고 있었고, 조준 대상이 안 잡혀 자동 검증이 통째로
  헛돌았다. 헷갈리면 `r = unreal.Rotator()` 뒤에 `r.pitch/r.yaw/r.roll`을 따로 넣는다.
- **PIE 입력 자동화는 py 파일 + 틱 상태 기계**로 짠다 (scratchpad/deliver_test.py): 한 틱에 한 단계씩
  (텔레포트 → 콘솔 치트 → 아이템 이동 → `inject_input_vector_for_action` ×N), 단계마다 3틱쯤 여유를 준다.
  `has_mapping_context`가 False를 돌려줘도 주입은 먹는다 — 그 값으로 판단하지 말 것.
- **"패키지 복원" 모달이 MCP를 통째로 막는다** (2026-09-07, 세 세션이 동시에 겪음): 강제 종료 뒤 재기동하면
  언리얼이 자동저장본 복원 여부를 묻는 모달을 띄우는데, 게임 스레드를 잡아 **포트 8000은 LISTENING인데 HTTP 연결은
  거부되는** 상태가 된다. 프로세스가 살아 있고 메모리도 3GB씩 올라가 있어 "아직 로딩 중"으로 오해하기 쉽다.
  **판별법: 로그의 프레임 카운터가 `[ 0]`에서 안 늘어난다.** 해결은 그 창 HWND에만 `WM_CLOSE`(복원 생략).
  **SlateInspector의 Windows close는 쓰지 말 것**(인덱스 기반이라 에디터 본체를 끈다). 복원 목록에 남의 세션
  패키지가 있으면 먼저 물어본다 — 낡은 자동저장본이 커밋된 에셋을 덮을 수 있다.
- **py로 레벨을 바꿀 때 월드·패키지 파이썬 참조를 들고 있으면 에디터가 죽는다** (2026-09-16 사고): `w = get_editor_world(); pkg = w.get_outermost()` 를
  변수에 둔 채 `EditorLoadingAndSavingUtils.load_map()` 을 부르면 FPyReferenceCollector 가 옛 패키지를 붙들어 "World Memory Leaks" 치명 오류로 즉사한다.
  load_map 전에 `del w, pkg` (모든 월드/액터/패키지 변수) 하고 부르거나, 아예 `SceneTools.load_level` 로 바꾼다. 더티 레벨이면 저장 프롬프트가 뜨니
  먼저 그 레벨 주인에게 묻는다.
- **에디터를 재기동하면 Slate ref가 전부 바뀐다** — 창 `w1`→`w2`, Cmd 텍스트박스 `tb1`→`tb2`. 매번
  `Snapshot(ref="", maxDepth=1)`로 창을 찾고 `Snapshot("m1", 10)`으로 Cmd 텍스트박스를 다시 잡는다.
- **BP CDO에 새 프로퍼티 값을 넣어도 레벨에 이미 놓인 인스턴스는 안 따라온다** (2026-09-07 실측): 에디터가
  레벨을 로드할 때 그 시점의 CDO 값(보통 빈 값)을 이미 복사해 갔기 때문이다. **스폰되는 액터는 CDO를 따르므로
  괜찮고, 레벨에 손으로 놓은 것만 인스턴스에 직접 넣어야 한다.** 컴파일·저장을 해도 소용없다 —
  시설물 `DisplayName`이 라벨에 액터 이름으로 뜨던 원인이었다.
- **`CL9.Replication.Registration`을 에디터 자동화로 돌리지 말 것** (2026-09-11): `GetLifetimeReplicatedProps`를 직접 부르는
  테스트라 엔진 단언(`bIsPushBased == Other.bIsPushBased`)으로 에디터가 통째로 죽는다. `RunTestsByFilter`는 `StartsWith:CL9.Map`처럼
  좁혀서. 또 RunTestsByFilter는 MCP 도구 대기 5분을 넘겨 백그라운드 태스크로 넘어가니 결과는 Saved/Logs의 `Test Completed`로 본다.
- ISM 컴포넌트에 처음 쓰인 재질은 `bUsedWithInstancedStaticMeshes`가 자동으로 켜져 dirty가 된다 — "N개 저장되지 않음"이 재질뿐이면
  그것이고, 경로를 명시해 저장하면 된다(`EditorLoadingAndSavingUtils.get_dirty_content_packages()`로 목록).
- 에디터 py에서 `get_actor_scale3d`(소문자 d), 컴포넌트 상대 변환은 `c.relative_location` 프로퍼티. `SceneTools.get_current_level`·
  `load_level(level_path)`가 있다. `AssetTools.write_file/read_file`은 확장자 .txt/.json만 받고 경로는 프로젝트 Saved/ 아래.

- **AnimGraph 자동화 (2026-09-12)**: `BlendPoses(Enum)`/`BlendPoses(Int)`는 `add_node_pin`이 "does not support adding pins" —
  항목 핀 노출이 에디터 메뉴 전용이다. 고정 모양인 `Animation|Blends|부울로포즈블렌딩`(BlendPose_0=True, _1=False)을 중첩해
  대체한다. `Utilities|Enum|Equal(Enum)`은 A를 먼저 연결하면 B가 enum이 되어 `set_pin_value(B, "Large")`가 먹는다.
  `본별로레이어드블렌딩`의 LayerSetup은 py로 `node.get_editor_property('node')` → `layer_setup=[InputBlendPose(branch_filters=[BranchFilter])]`
  → `set_editor_property('node', s)`. ABP enum 변수는 MCP `add_variable`이 못 만든다 → py `BlueprintEditorLibrary.add_member_variable(bp,
  name, t)` with `t=EdGraphPinType(); t.import_text('(PinCategory="byte",PinSubCategoryObject="/Script/CL9.ECL9HoldPose")')`.
- Git Bash에서 `/Game/...` 인자는 MSYS가 `C:/Program Files/Git/Game/...`으로 바꾼다 → `export MSYS_NO_PATHCONV=1`. rpc.ps1 응답은
  `$resp.RawContentStream`을 UTF-8로 디코드해야 한글 type_id가 안 깨진다(`$resp.Content`는 Latin-1). 파이썬 출력은 `PYTHONIOENCODING=utf-8`.
- PIE 화면 픽셀: 도킹 뷰포트에선 SlateInspector Screenshot이 빈 data를 돌려준다 → 게임 월드에 `HighResShot 1` 콘솔 →
  `Saved/Screenshots/WindowsEditor/HighresScreenshot0000N.png`. 치트(`cl9.additem`)는 `execute_console_command(W, cmd, PC)`로 PC를
  넘겨야 CheatManager에 닿는다. `project_world_location_to_screen`은 Vector2D 하나만 돌려준다(튜플 아님).
- **DA_* 접두사 제거 (2026-09-12 낮, cl9-37)**: 데이터 에셋 refPath가 `/Game/Data/ItemTable.ItemTable`, `MapTable`(옛 DA_SiteTable), `ModuleTable`,
  `PieceTable`, `SegKindTable`, `CharacterTable`, `MonsterTable`, `OrderTable`, `EquipmentTable`, `BuffTable`, `ShakeTable`, `CL9Rules`, `CL9Progression`,
  `CL9DefList`, `/Game/UI/UITable.UITable`로 바뀌었다(행 서브오브젝트 이름 CL9ItemDef_N은 그대로). 옛 스크립트의 `/Game/Data/DA_*` 경로는 전부 무효.

- **PIE 자동화 요령(2026-09-15 swing-hitbox)**: 자동화 PIE 는 3~8fps 라 0.35초짜리 동작이 틱 하나에 끝난다 — 판정을 보려면 콘솔
  `slomo 0.15` 로 게임 시간을 늦춘다(판정·애님·AI 모두 같은 시계). 에디터 파이썬은 private/protected UPROPERTY(몬스터 `Hp`·`State`,
  캐릭터 `HeldItem`)를 `get_editor_property` 로 못 읽는다 → `UFUNCTION(BlueprintPure)` 게터(`m.get_state()`)·붙은 액터 검색·
  `LogsToolset.GetLogEntries`(`LogCL9` Verbose 켜고 코드의 UE_LOG) 가 오라클. 파이썬 원격 실행 노드는 없다(`run_python.py` 실패) →
  SlateInspector `Snapshot ref=""` 로 하단 Cmd 텍스트박스(`tb2`)를 찾아 `py "<절대경로>"` 를 `Type submit:true`. 스크립트는
  `register_slate_post_tick_callback` 으로 단계를 돌리고 JSON 을 남긴다. MCP **읽기** 호출만 해도 훅이 그 세션에 에디터 임대를 잡는다
  (다른 세션이 pie/build 를 못 잡음 — 동료에게 `release` 요청). pie 잠금은 30분 만료라 긴 검증은 `--minutes 40`. Lvl_Flat 에는 레벨 닭
  5마리가 x 1100~4400·y 800~1200 에 있다 — 몬스터 시험은 (-900,-1100) 쪽에서, 닭은 65cm 안에서 0.3게임초 안에 잡아 짐을 흘린다.
- **PIE 중인지·어느 레벨인지 읽기만으로 알기** (2026-09-15): `SceneTools.get_current_level`은 ""를 돌려주지만
  `SceneTools.find_actors(actor_type={"refPath":"/Script/CL9.CL9Counter"})`가 `/Game/Maps/UEDPIE_0_Lvl_Flat...` 경로를 돌려주면
  그 순간 다른 세션의 PIE가 그 레벨에서 돌고 있다는 뜻이다(잠금 없이 확인 가능). 단, 읽기 MCP 호출도 훅이 에디터 임대를 자동으로
  잡으니 동료 빌드 전에 `cl9_lock.py release`로 놓는다.
- `HighResShot 1`은 HUD(UMG)를 찍지 않는다 — 3D 씬만. HUD 문구 검증은 py로 런타임 `TextBlock`(`get_name()=='OrdersText'`이고
  path가 `/Game/`로 시작하지 않는 것)의 `get_text()`를 읽는 편이 확실하다.
- `cl9.additem`은 플레이어 전방 200cm에 스폰한다 — 창구 앞에 서 있으면 계란이 창구 옆에 떨어져 줍기 주입(IA_PickPlace)이
  창구를 눌러 **빈손 판매**가 나간다(재고가 팔린다). 스폰 전에 창구에서 5m 떨어져 등을 돌린다.
- Cmd 텍스트박스 ref는 `Observe(w1, 40)` 뒤 rpc.ps1로 `Snapshot(sp1, 40)`을 파일에 받아 `textbox`만 걸러 찾는다(이번 세션 tb2,
  "Cmd" 텍스트 x19 옆). 네이티브 Snapshot은 출력이 커서 컨텍스트를 잡아먹는다.
- **`call_tool` 의 `toolset_name` 은 `list_toolsets` 가 찍는 전체 이름** (2026-09-15 낮, 재기동 뒤 확인): `editor_toolset.toolsets.object.ObjectTools`,
  `editor_toolset.toolsets.asset.AssetTools`, `EditorToolset.EditorAppToolset`, `SlateInspectorToolset.SlateInspectorToolset`, `UMGToolSet.UMGToolSet`.
  `ObjectTools` 만 주면 "Toolset not found". get_properties 인자는 `instance:{refPath}` + `properties:[…]`, set_properties 는 `instance` + `values`(JSON 문자열).
  `TSoftObjectPtr` 열은 `{"refPath":"/Game/Effects/M_CL9_HitFlash.M_CL9_HitFlash"}` 로 넣으면 받는다. `StartPIE` 는 `options:{bSimulate,playMode,warmupSeconds}` 필수.
- **PIE 파이썬 오라클 (2026-09-15 hit-feedback)**: `mesh.get_overlay_material()`(SkinnedMeshComponent, BlueprintCallable) 로 오버레이 플래시를, `pc.player_camera_manager.get_camera_rotation()` −
  `pc.get_control_rotation()` 편차(normalized_delta_rotator)로 카메라 셰이크를 잰다(넘어짐 카메라는 위치만 내리므로 회전 편차 = 셰이크). HUD 위젯은 `unreal.WidgetLibrary.get_all_widgets_of_class(world, UserWidget, False)`
  (`WidgetBlueprintLibrary` 라는 이름은 없다), `get_widget_from_name` 도 없다. **BindWidgetOptional/UPROPERTY 만인 위젯 프로퍼티는 파이썬에 노출되지 않는다**(CPF_Edit·BlueprintVisible 없음) →
  `unreal.ObjectIterator(unreal.Image)` 에서 `get_name()=='HurtVignette'` 이고 경로가 HUD 인스턴스 경로로 시작하는 것. enum 의 str 은 `"COLLAPSED: 1>"` 꼴이라 `startswith` 로 비교.
  Cmd 텍스트박스는 `Observe("m2")` 뒤 `Snapshot("m2")` 이 작다(상태바 메뉴 m2 안 `tb2`). 파괴한 액터를 계속 폴링하면 `get_component_by_class` 가 None → 오라클에 잡히니 watch 를 지운다.
- **PIE 자동화 함정 모음 (2026-09-16 keep-items-cart-lob)**: ① `unreal.Rotator(a,b,c)` 위치 인자는 **(roll, pitch, yaw)** — `Rotator(0, yaw, 0)` 은 피치다.
  키워드 `Rotator(roll=0, pitch=0, yaw=y)` 로 쓴다(폰은 CMC 가 피치를 지워 티가 안 나지만 물리 수레는 코를 박는다). ② 물리 수레 옆으로 폰을 순간이동하면 캡슐이
  수레를 밀고 들어 올린다 — 시험 중엔 수레 `StaticMeshComponent.set_simulate_physics(False)` 로 고정하고 **수레를 손 앞으로** 옮긴다(짐칸 판정은 위치 겹침이라 무관).
  수레 바운드(`get_actor_bounds`)로 짐칸 중심→면 거리를 재고(+x 101.7 손잡이/−x 66/±y 48.6, 높이 103) 얇은 면을 폰 쪽으로 돌려 거리 = 면 + 캡슐 34 + 10.
  ③ 손 소켓은 폰을 옮기고 조준한 **다음 틱**에 따라온다 — 배치 스텝과 G 스텝을 나눈다. ④ Slate post-tick 누적 시간은 게임 시간보다 느리게 흐른다(1.1 "초" 에 게임은 1.5초 지남) —
  넘어짐처럼 게임 시계에 걸린 검사는 `GameplayStatics.get_time_seconds(world)` 로 잰다. ⑤ 컴포넌트 위치는 `get_world_location()`/`get_world_transform()`(get_component_* 없음),
  PlayerState 는 `pc.player_state`(`pawn.get_player_state()` 없음), BlueprintReadOnly 프로퍼티는 `get_editor_property('hp')`. ⑥ Monitor `tail -f` 는 로그가 나중에 생기면 이벤트를 놓쳐
  타임아웃까지 침묵했다 — 빌드 대기는 `run_in_background` Bash 의 `until grep -q "BUILD EXIT|error" log` 루프가 확실하다. ⑦ 에디터 재기동 뒤 SlateInspector 참조는 초기화 —
  `Snapshot("w1",30)` 으로 상태바 메뉴 ref(m1)를 찾고 `Observe(m1)`+`Snapshot(m1)` 에서 textbox(tb1)를 얻는다.

~~~~

### work-progress.md (1993행~끝)

`memory/work-progress.md`

~~~~
## 2026-10-04 — 노트북 이전 준비: 미커밋 전부 커밋·푸시 (cl9-ff)
- 사용자 지시("다른 노트북에서 할 것 → 커밋푸시")로 워킹트리 전부를 역할별 7커밋으로 묶어 푸시. origin/master = `10aa6da`
  (C++ 1f8cb34 · 아트 038bddc · 도구 bba4b50 · 밸런스 687fb0d · 문서 3930c3f · 에셋 747ca8c · 세션·설정 10aa6da).
  위 항목들의 "미커밋" 은 이것으로 해소. 빌드·PIE 는 다시 돌리지 않았다(사람 플레이 수치 확인·Lvl_Henhouse 내비 경계는 미결 그대로).
- 사용자 결정: `graphify-out/`·`Source/graphify-out/` 은 커밋하지 않고 둔다, stash@{0}(9-08 디버그 값)은 이 PC 에만 둔다.
- 노트북: Windows 사용자 이름 `radar`(이 PC 는 `user`), D: 드라이브 유무는 미확인. 저장소 밖이라 직접 복사할 것 —
  `Content/DreamscapeSeries`(gitignore), 이 메모리 폴더, 전역 `.claude`(CLAUDE.md·skills·agents), `.claude/runs`(이어갈 실행이 있을 때).
  `.mcp.json` 의 uvx 경로(`C:/Users/user/.local/bin/uvx.exe`)와 문서 속 `C:\Users\user\...` 는 노트북에서 안 맞는다 — 아직 안 고침.
- 2026-10-04 추가: 사용자 "맵 5~10배 크게, 판과 바깥 둘 다, 대농장 기준" → `docs/goal/giant-stage.md` 신설(미결 5: 정확한 배수·밤 흐름·
  한 덩어리 vs 구역 묶음·개체 수·성능 목표). outdoor-backdrop.md 미결 3 갱신 + giant-stage 의존 명시. 새 세션은 giant-stage 먼저 정하고 배경 순.

## 재개 지점 (2026-10-04, cl9-a0 — giant-stage 명세 확정, 구현 미착수) — 새 세션은 여기부터
- `/autogoal` HEAVY 로 `docs/goal/giant-stage.md` 를 확정했다(미커밋). 미결 0. 계획 패널(5 에이전트) 결과가 "Design Decisions" 에 있다.
- 사용자 결정: 5배(1,000 모듈) 먼저→버티면 10배 · 밤 600초(리썰 코드 기본값 계산) · 구역 묶음(5개 구역, 트럭 마당 중심) ·
  몬스터는 멀어도 전부 활성, 수는 리썰 비교(1일 ~32, 10일 ~80) · 합격 기준은 프레임 시간만(게임 스레드 4ms 는 보고만) ·
  한꺼번에 로드 금지(구역은 프레임당 하나, 몬스터는 나눠 스폰) · 별칭 `mongjang` / `Lvl_Mongjang` / 표시 이름 "몽장".
- 구조: 구역 = `ACL9SiteMap` 액터 K개가 기존 생성기를 그대로 돌린다(`UCL9SiteDef::Districts`, 비면 기존 경로 그대로). 격자 상한 128 유지.
- **구현은 시작 못 했다** — `/cl9 task` 가 "훅: 비활성" 으로 막혔다(cl9-a0 세션). 새 세션에서
  `/cl9 task docs/goal/giant-stage.md 구현 — Work split 순서대로` 로 시작. 1단계 = map 역할의 구역 C++ + 테스트.
- 그때 unreal-mcp 는 ECONNREFUSED 였다(에디터 꺼짐).

## 재개 지점 (2026-10-04 저녁, cl9-a0 — 다른 컴으로 이전) — 새 세션은 여기부터
- 위 "구현은 시작 못 했다" 는 틀렸다(훅 비활성은 잠금 CLI 를 다른 명령과 이어 쳐서 난 오판). 실행 `giant-districts` 를 돌렸다.
- 브랜치 `giant-districts-wip` = `0b2b7cc` 푸시(origin). master 는 `908e9b2` 로 원격보다 1 앞선 채 푸시 안 함 — 그 커밋은 브랜치에 들어 있다.
- map 역할 완료(보고서 `.claude/runs/giant-districts/report-map.md` — 리드가 옮겨 적음). gameplay 역할은 사용자가 중단, 보고서 없음 —
  `CL9GameMode.h/.cpp`·`CL9RulesDef.h/.cpp`(새 열 `MonsterSpawnPerFrame`)·`CL9Monster.cpp`·docs/systems 3개가 미완 상태로 커밋됐다.
- **컴파일 미확인**: 컴파일 검사 3번 모두 메모리 부족(다른 프로그램이 13GB 점유)으로 20분 제한에 끝났다. `CL9SiteMap.cpp` 미검사, 테스트 미실행.
- 미결: 먼 구역 슬롯이 초기 값(문 300 m)으로는 겹친다 — `CL9.Map.Districts` 점유 상자 실측 뒤 당기거나 N 을 줄인다(report-map 질문 1).
- 이어가기: 브랜치 체크아웃 → 빌드 → `/cl9 resume giant-districts`(gameplay 를 새로 띄운다). `.claude/runs/giant-districts` 는 gitignore 라 직접 복사.

~~~~

## 부록 E. 전역 규칙

### CLAUDE.md

`C:\Users\<사용자>\.claude\CLAUDE.md`

~~~~
Never guess intent — ask when uncertain.
- 어떠한 질문에 대한 대답으로 추측하지 않습니다. 추측으로 작업을 진행하지 않습니다.
- 명시적으로 지시한 것만 실행합니다. "확인해봐"는 읽고 분석·보고까지만 — 수정하지 않습니다.
  지시받지 않은 작업을 덧붙이지 않고, 파괴적 작업(git reset·checkout·파일 삭제)은 추측으로
  절대 실행하지 않습니다.
- 번역은 제대로 합니다. 전보체·기계번역투를 쓰지 않고, 명사만 나열하지 말고 완결된 문장으로 씁니다.

# 작업 원칙 (Karpathy CLAUDE.md 승계 — forrestchang/andrej-karpathy-skills)
- 단순함 우선: 요청 안 한 기능·선제 추상화·안 쓰는 설정성·일어날 수 없는 케이스의 에러 처리를 만들지 않는다.
- 수술적 변경: 작업이 요구하는 것만 바꾼다. 범위 밖 코드·주석·포맷을 다듬지 않고, 기존 컨벤션을 따르고, 내 수정이 고아로 만든 것만 지운다.
- 목표 주도: 모호한 요청은 검증 가능한 성공 기준으로 먼저 번역하고, 구현 후 그 기준으로 스스로 검증한다.

# 컨텍스트 절약
- 파일을 읽을 때 전체를 읽지 않는다. Grep으로 위치를 먼저 찾은 뒤 Read에 offset/limit을 지정해 필요한 범위만 읽는다.
- 이미 읽은 파일은, 그 뒤에 수정하지 않았다면 다시 읽지 않는다.
- 로그·빌드 산출물처럼 큰 출력은 head/tail/grep으로 필요한 부분만 확인한다.

# Paths
- 경로를 적을 때는 항상 풀(절대) 경로로 적으세요. (예: `C:\Users\user\Desktop\file.txt`, `D:\Work\vibe\project\src\index.ts`)
- 상대 경로나 `~`, `.`, `..` 같은 축약 표기는 사용하지 마세요.
- 사용자에게 경로를 보여줄 때, 도구를 호출할 때, 코드/문서에 적을 때 모두 동일하게 풀 경로를 사용합니다.

# gstack

## Browser Usage
- 모든 웹 브라우징에는 gstack의 `/browse` 스킬을 사용하세요.
- `mcp__claude-in-chrome__*` 도구는 절대 사용하지 마세요.

## Available Skills
/office-hours, /plan-ceo-review, /plan-eng-review, /plan-design-review, /design-consultation, /design-shotgun, /design-html, /review, /ship, /land-and-deploy, /canary, /benchmark, /browse, /connect-chrome, /qa, /qa-only, /design-review, /setup-browser-cookies, /setup-deploy, /retro, /investigate, /document-release, /codex, /cso, /autoplan, /plan-devex-review, /devex-review, /careful, /freeze, /guard, /unfreeze, /gstack-upgrade, /learn

## Installation Path
D:\Work\vibe\gstack
# graphify
- **graphify** (`~/.claude/skills/graphify/SKILL.md`) - any input to knowledge graph. Trigger: `/graphify`
When the user types `/graphify`, use the installed graphify skill or instructions before doing anything else.

~~~~

## 부록 F. /autogoal 스킬

### SKILL.md

`C:\Users\<사용자>\.claude\skills\autogoal\SKILL.md`

~~~~
---
name: autogoal
description: |
  Single entry point for ALL feature development, big or small: probe the request
  office-hours style, self-route it LIGHT or HEAVY (HEAVY adds a multi-perspective
  planning panel), write the goal spec doc, then implement it through the
  project's harness — no manual /goal step. Unlike /goaldoc (doc only), autogoal
  CONTINUES into implementation once the spec has no open questions.

  Triggers: /autogoal, autogoal, 자동 goal, 알아서 goal, 문서부터 구현까지
  Keywords: goal, autogoal, spec, 명세, 구현, 자동, 개발 프로세스
argument-hint: "<feature request text>"
user-invocable: true
---

# autogoal — probe → spec → implement, in one run

`/goaldoc`는 문서만 만들고 멈춘다. `autogoal`은 **office-hours식으로 캐묻고 → goal 문서를 쓰고 → 미결이 없으면 곧바로 구현까지** 한 흐름으로 간다.

**autogoal은 크고 작은 기능 모두의 단일 진입문이다.** 규모를 사용자가 미리 나누지 않는다 — Phase 0에서 autogoal이 스스로 LIGHT/HEAVY를 판정하고, HEAVY면 계획 패널(Phase 3.5)을 끼운다.

핵심 차이 한 줄: **autogoal은 구현한다.** 단, 명세에 풀리지 않은 질문(Open Questions)이 남으면 구현 직전에 멈추고 물어본다.

```
autogoal <요청>
  → Phase 0   규모 라우팅   (LIGHT | HEAVY — autogoal이 스스로 판정, 기본값 LIGHT)
  → Phase 1   분해 + 캐묻기 (office-hours 자세 — 모호/이상/가정을 한 항목씩 질문·수정)
  → Phase 2   슬러그
  → Phase 3   가벼운 정찰 (읽기 전용)  ← HEAVY 신호 발견 시 여기서 상향 재라우팅
  → Phase 3.5 계획 패널   (HEAVY만 — 다관점 토론으로 설계 결정 확정, 읽기 전용)
  → Phase 4   goal 문서 작성 (영문, docs/goal/<slug>.md)
  → Phase 5   게이트        ← Open Questions 남으면 STOP하고 질문 / 없으면 통과
  → Phase 6   구현          (프로젝트 하네스가 있으면 그걸 탄다 / 없으면 일반 구현)
  → Phase 7   보고
```

생성되는 goal 문서는 **영어로만** 작성한다 (goaldoc과 동일).

---

## Phase 0: 규모 라우팅 (LIGHT / HEAVY)

**이 라우팅은 "계획 단계를 얼마나 무겁게 갈지"만 정한다.** 구현 규모 판정(SIMPLE/STANDARD/COMPLEX)은 Phase 6에서 프로젝트 하네스의 트리아지가 별도로 한다 — 두 축을 섞지 마라. (계획은 가벼운데 구현이 COMPLEX인 작업도 흔하다: 예를 들어 등록 16곳짜리 기계적 신규 게임.)

**기본값은 LIGHT.** 아래 HEAVY 신호가 하나라도 뚜렷하면 HEAVY로 올린다:

- 새 게임 / 새 시스템 / 새 모드 등 **메커니즘 설계 자체**가 요청인 경우
- DB 스키마 신설·변경 또는 소켓 이벤트 **계약 신설**이 예상되는 경우
- 설계 선택지가 여럿인데 **정답이 코드에도 사용자에게도 없는** 경우 (있다면 캐묻기로 해결될 문제 — LIGHT)
- 사용자가 "설계부터", "제대로", "꼼꼼히" 류의 신호를 준 경우
- 잘못 설계하면 롤백 비용이 큰 경우 (공정성 정책, 경제/재화 구조, 크로스게임 공유 계약)

판정을 Phase 7 보고에 한 줄로 남긴다. **애매하면 LIGHT로 시작하고 Phase 3 정찰에서 상향한다** — 정찰 없이 HEAVY를 남발하면 작은 기능마다 패널 비용을 낸다.

---

## Phase 1: 요청 분해 + 캐묻기 (office-hours 자세)

`$ARGUMENTS`(또는 대화 맥락)를 독립 항목으로 분해한다. 각 항목은 "무엇을 / 어떤 조건에서 / 결과가 무엇인지"가 명확해야 한다.

그다음, **각 항목에 office-hours식 캐묻기를 적용한다.** 이건 YC 스타트업 진단이 아니라 **명세 정밀화용 probing**이다. 항목마다 다음을 자문한다:

- **모호어 검출.** "잘 되게", "자연스럽게", "적절히", "상황에 맞게" 같은 말은 측정 불가다. 무엇을 어떤 기준으로 판단하는지 구체화한다.
- **숨은 가정.** 이 요청이 당연하게 여기는 것은? (예: "1마리일 때") 그 가정이 틀리면 구현이 달라지는가?
- **누락 케이스.** 요청이 다루지 않은 상태/경계는? (0개, 1개, N개, 전부 같음, 충돌 등) 각 케이스에서 무엇이 일어나야 하는가?
- **기존 코드와의 충돌.** 정찰(Phase 3) 결과가 요청과 어긋나면 그 사실을 들이댄다.

### 캐묻기 게이트 (중요)

- **진짜로 막히는 모호함만 물어본다.** 합리적 기본값이 있으면 묻지 말고 그 기본값을 골라 명세에 적고 진행한다.
- 물어볼 땐 **한 번에 한 항목씩**, `AskUserQuestion`으로. 추천안을 첫 옵션에 두고 이유를 붙인다.
- 사용자 답으로 해소된 항목은 명세에 **결정된 사실**로 반영한다 (Open Questions로 남기지 않는다).
- 끝까지 사용자만 정할 수 있는 취향/판단(예: 정확한 카피 톤)이라 즉답이 안 나오면 → 그건 **Open Question**으로 남긴다 (Phase 5에서 게이트가 잡는다).

목표: probing을 통해 **Open Questions가 비도록** 만든다. 그래야 Phase 5에서 멈추지 않고 자동 구현으로 간다.

## Phase 2: 슬러그 결정

- 영문 kebab-case, 짧고 명확하게.
- 게임 요청이면 `<game-name>-` 접두 (예: `horse-race-vote-ux`).
- 일반 기능이면 동사-명사 (예: `export-csv`).

## Phase 3: 가벼운 정찰 (읽기 전용)

명세를 정확히 쓸 만큼만 탐색한다. **여기서는 코드 수정 금지.**

- 관련 파일이 이미 있는지 (중복 기능 여부)
- 연동 대상 모듈/함수/이벤트 이름과 시그니처
- 게임 요청이면 진입점·상태 관리·소켓 계약
- 정찰로 드러난 충돌/제약은 Phase 1 캐묻기로 되먹임한다 (필요하면 추가 질문)
- **재라우팅 체크**: 정찰 결과가 Phase 0의 LIGHT 판정을 뒤집으면 (숨어 있던 계약 신설, 예상보다 넓은 설계 선택지) HEAVY로 상향하고 Phase 3.5를 탄다. 하향은 하지 않는다.

정찰 중 "이건 그냥 조사가 아니라 수정이 필요하네" 싶어도, **여기서 고치지 않는다.** 명세에 반영하고 Phase 6에서 하네스로 처리한다.

## Phase 3.5: 계획 패널 (HEAVY만)

goal 문서를 쓰기 전에, 설계 초안을 **서로 다른 관점의 독립 에이전트들이 두들기게 한다.** 목적은 명세의 품질이다 — 사용자도 생각 못 한 리스크/대안을 문서에 반영되기 전에 찾는 것.

**기본 실행: 네이티브 Workflow 다관점 패널.**

- 관점 3~4개를 요청 성격에 맞게 뽑는다 (예: 메커니즘 정합성 / 운영·리스크 / UX·모바일 / 단순화-반론). 각 관점은 Phase 1~3의 결과(요청 분해 + 정찰 요약 + 설계 초안)를 받고 **찬성 논거가 아니라 반박·대안**을 내게 한다
- 마지막에 교차 심문 1개를 붙여 패널 합의의 약점을 공격시킨다
- **패널은 읽기 전용이다.** 파일 수정 금지, 산출물은 구조화된 결정 목록뿐
- 프로젝트에 lessons 문서가 있으면 (예: `docs/GameGuide/lessons/`) 관련 파일을 패널 프롬프트에 포함한다

**대체 실행:** 사용자가 다른 모델의 반론을 원하면 `/meeting-codex` 토론으로 대체할 수 있다 (Claude×Codex — 역할극이 아닌 진짜 이견). 역할극형 meeting 변형은 쓰지 않는다.

**산출 → 명세 반영:** 패널이 끝나면 결과를 세 갈래로 나눠 goal 문서에 싣는다:

1. **확정 결정** → `## Design Decisions` 섹션 (아래 Phase 4 참조). 구현 단계에서 재론 금지가 원칙
2. **미해소 쟁점** → `## Open Questions` (Phase 5 게이트가 사용자에게 묻는다)
3. **반려된 대안** → 결정 항목에 한 줄로 병기 ("~안은 ~이유로 반려") — 다음 세션이 같은 대안을 재발굴하지 않게

## Phase 4: goal 문서 작성

`docs/goal/<slug>.md`(프로젝트 루트 기준 상대경로)를 아래 영문 템플릿대로 작성한다. `docs/goal/`이 없으면 만든다.

### Standard feature template

```markdown
# goal: <slug>

## One-line Goal
<What we're building, in one sentence.>

## Background / Motivation
<Why it's needed. Omit if obvious.>

## In-scope
- <item 1>
- <item 2>

## Out-of-scope
- <what we explicitly won't do>

## Acceptance Criteria
- [ ] <done condition 1>
- [ ] <done condition 2>

## Related Files / Modules
| File | Role |
|------|------|
| <path> | <one-line description> |

## Must-Preserve
- <existing contracts that must not be broken>

## Execution Notes
- Recommended model: <strongest current Claude model> for <judgment-heavy items> — <one-line reason>. <cheaper model> acceptable for <mechanical items>.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.

## Open Questions
- <undecided items. OMIT if none — autogoal probed these in Phase 1, so this should usually be empty>
```

### Execution Notes 작성 가이드 (모든 goal 문서에 표준 포함)

- **판단 집약 항목**(설계 해석, 지표/기준선 정의, 사용자 노출 카피 톤, COMPLEX 트리아지 예상) → 작성 시점 최상위 Claude 모델 권장 (2026-07 기준 Claude Fable 5 — Opus 상위 티어. 낡은 "Opus 4.8" 문구로 다운그레이드 유도하지 말 것).
- **기계적 항목**(보일러플레이트, 단순 연결/export, STANDARD 이하) → 하위 모델(예: Sonnet) 허용 명시.
- 두 문장 항상 포함: ① 문서는 모델을 강제 못 한다(실행 세션 `/model`이 결정), ② 권장 미만 모델로 시작하면 사용자에게 알리고 확인 후 진행.
- 항목별 난이도가 갈리면 항목 단위로 추천을 나눠 적는다.

### Game / interactive feature — append after standard template

```markdown
## Fairness Constraints
- <RNG seed policy, anti-cheat conditions, score-integrity rules, etc.>

## Existing Integration Contract
- <contracts with current game loop / state / events that must not be broken>
```

### HEAVY only — append after In-scope/Out-of-scope

```markdown
## Design Decisions (panel-settled — do not relitigate during implementation)
- <decision 1> — <one-line rationale>. (Rejected: <alternative> — <why>)
- <decision 2> — <one-line rationale>
```

LIGHT 트랙이면 이 섹션은 넣지 않는다 (빈 의례 금지).

작성 직후 사용자에게 짧게 알린다: `📄 docs/goal/<slug>.md 작성 완료` + 핵심 결정 1~3줄.

## Phase 5: 게이트 — 구현으로 넘어갈지 판정

**Open Questions가 비어 있으면** → 바로 Phase 6.

**Open Questions가 하나라도 남아 있으면** → 여기서 STOP. 남은 질문을 한 항목씩 `AskUserQuestion`으로 묻는다. 전부 해소되면 명세를 갱신(Open Questions 제거)하고 Phase 6로. 사용자가 "일단 문서까지만"이라고 하면 거기서 멈추고 `/goal docs/goal/<slug>.md 해줘` 안내만 남긴다.

> 자동 구현은 명세가 깨끗할 때만 일어난다. 애매한 채로 코드를 건드리지 않는다.

## Phase 6: 구현

`docs/goal/<slug>.md`를 **구현의 source of truth**로 삼고 구현한다. 구현 경로는 프로젝트가 하네스를 갖고 있는지에 따라 갈린다.

### 6-A. 프로젝트 하네스가 있으면 (예: LAMDiceBot)

`.claude/rules/harness.md`(또는 동급 오케스트레이션 문서)가 있으면 **그 하네스를 그대로 탄다.** 직접 마구 고치지 않는다.

1. **트리아지를 한 줄로 선언한다** — 이 프로젝트는 Edit/Write 직전 훅(`check-triage.sh`)이 `[트리아지: …]` 형식을 강제할 수 있으므로, 코드 수정 전에 반드시 선언한다.
2. 하네스 규칙대로 진행: SIMPLE이면 직접 구현, STANDARD/COMPLEX면 Scout → Coder → Reviewer →(COMPLEX는) QA.
3. 하네스의 불변조건(must-preserve), 공정성 원칙, 모바일/PC 대응을 지킨다. goal 문서의 Must-Preserve / Fairness Constraints / Existing Integration Contract 섹션을 Scout/Coder 지시서에 그대로 싣는다.
4. 트리아지가 애매하면 낮추지 말고 높인다.

### 6-B. 하네스가 없으면

일반 구현으로 처리한다: 수정 대상과 그 호출부를 읽고 → 기존 패턴에 맞춰 구현 → 문법/정적 검증 → 수동 QA 체크리스트 제시. goal 문서의 Acceptance Criteria를 검증 기준으로 쓴다.

### 6-C. 완료 처리

- 프로젝트가 goal 아카이브 큐 관례를 가지면(예: LAMDiceBot은 CLAUDE.md에서 `.claude/.goal-applied-queue`에 완료된 goal 경로를 한 줄 append하도록 규정 — Stop 훅이 `docs/goal/applied/`로 이동), **goal을 실제로 끝냈을 때만** 그 경로(프로젝트 루트 기준)를 큐에 append 한다. 미완이면 적지 않는다.
- 그런 관례가 없으면 이 단계는 건너뛴다.

## Phase 7: 보고

사용자에게 정리해서 보고한다:

```
✅ autogoal 완료

🧭 트랙:  LIGHT | HEAVY — <판정 근거 1줄. HEAVY면 패널 핵심 결정 1줄 추가>
📄 명세:  docs/goal/<slug>.md
🔧 구현:  <트리아지 수준> — <수정 파일 N개>
✔ 검증:  <문법/리뷰/QA 결과 또는 수동 QA 체크리스트>
⚠ 미결:  <남은 게 있으면. 없으면 생략>
```

구현까지 끝났으면 `/goal …` 후속 안내는 하지 않는다 (이미 했으므로). 게이트에서 멈췄으면 `/goal docs/goal/<slug>.md 해줘`만 안내한다.

---

## autogoal이 지키는 것 / 하지 않는 것

- 명세에 **풀리지 않은 질문이 남은 채로 구현하지 않는다.**
- 정찰 단계에서 **코드를 고치지 않는다** (정찰은 읽기 전용, 수정은 Phase 6).
- 프로젝트 하네스가 있으면 **우회하지 않는다** — 트리아지·Scout·Reviewer·QA를 건너뛰지 않는다.
- **규모 라우팅은 autogoal이 한다** — 사용자에게 LIGHT/HEAVY를 미리 정해오라고 요구하지 않는다. 단, 사용자가 명시하면 그 판정이 우선한다.
- **LIGHT/HEAVY는 계획의 무게이고, SIMPLE/STANDARD/COMPLEX는 구현의 무게다** — 패널 결과로 하네스 트리아지를 대체하거나 선점하지 않는다.
- 계획 패널(Phase 3.5)은 **읽기 전용**이며, 산출은 goal 문서 반영으로만 수렴한다 — 패널이 직접 구현을 이어가게 두지 않는다.
- `/office-hours` 스킬을 직접 호출하지 않는다 — 그 **캐묻는 자세만** Phase 1에 녹였다 (gstack 프리앰블·텔레메트리·모드선택·HARD GATE 비의존).
- goal 문서는 영어로 쓴다. 사용자에게 보고/질문하는 말은 한국어로 한다.

~~~~
