# Epic C++ 코딩 표준 준수 감사 — `Source/CL9` 전체

작성일: 2026-09-13 02:14
기준: `D:\Work\unreal\CL9\docs\ue-cpp-conventions.md` (Epic Coding Standard 한글판) 전 항목
범위: `D:\Work\unreal\CL9\Source\CL9` .h/.cpp 123개, 7개 폴더 전량 정독
성격: **읽기 전용 보고.** 이 문서는 어떤 파일도 고치지 않는다.
계기: "언리얼 표준이 있으면 그에 맞춰 리팩토링해야 하지 않나"는 질문. 커뮤니티 스킬
(quodsoler·maystudios 등)은 표준이 아니라 개인 취향이므로 대조 기준에서 제외하고,
Epic 공식 표준(=이 프로젝트의 `ue-cpp-conventions.md`)만으로 감사했다.

---

## 총평

코드베이스는 Epic 표준을 대체로 잘 따르고 있다. 기계적 규칙은 위반이 사실상 0이고,
위반이 몰린 곳은 스타일 판단이 들어가는 규칙 셋(복합 조건 분리·익명 리터럴·긴 함수)이다.
**대규모 리팩토링이 필요한 상태가 아니다.** 아래 "해야 할 것"은 A→B→C 순서로 나눴고,
C는 일괄 처리하지 않기를 권한다.

---

## 통과한 항목 (위반 0건)

| 규칙 | 결과 |
|---|---|
| `#pragma once` | 전 헤더 있음 |
| 파일명에 클래스 접두사(U/A/F) 금지 | 위반 없음 |
| `NULL` 금지 → `nullptr` | 위반 없음 |
| 표준 라이브러리 컨테이너·스트링 금지 | `std::` 사용 없음 |
| `TEnumAsByte` 금지, 네임스페이스 enum 금지 | 위반 없음 (전부 `enum class`) |
| 탭 들여쓰기 | 스페이스 들여쓰기 파일 없음 |
| `override`에 `virtual` 동반 | 위반 없음 |
| `TMap` 이터레이터 대신 range-for | `CreateIterator` 2건은 모두 `RemoveCurrent`용 — 정당 |
| `auto` 허용 범위 | 18건 전부 람다 바인딩·이터레이터 — 정당 |
| 생성자 안 액터 초기화 로직(월드 질의·스폰·타이머) | 0건 (자기 컴포넌트 델리게이트 바인딩은 템플릿 관행) |
| `.cpp` 전역 `using namespace` / 접두사 없는 전역 `#define` | 0건 |
| Get*/Is*/Has*가 `const`가 아닌 경우 | 0건 |
| 비-인터페이스 public UPROPERTY | 명확한 사례 0건 |

---

## 해야 할 것

### A. 버그·잔재 — 표준과 무관하게 고쳐야 함 (4곳, 직접 확인함)

- [ ] **A-1. 배열 범위 밖 쓰기** — `D:\Work\unreal\CL9\Source\CL9\Data\CL9DefRegistry.cpp:433`
  `bool bSeenType[5]`인데 `ECL9ModuleType`은 6개(`Stair`=5). Stair 모듈 행이 있으면 `:436`에서
  배열 밖 쓰기. 같은 헤더의 `CL9ModuleTypeCount`(=6)를 써야 한다.
- [ ] **A-2. 고아 `UFUNCTION()`** — `D:\Work\unreal\CL9\Source\CL9\UI\CL9HUDWidget.h:202-205`
  `UFUNCTION()` 다음에 빈 줄·주석이 오고 `void TryWire();`가 붙어 있다. 다른 Refresh* 계열은
  UFUNCTION이 아니므로 잔재로 보임 — 의도 확인 후 제거 또는 붙이기.
- [ ] **A-3. 코드와 어긋나는 주석** — `D:\Work\unreal\CL9\Source\CL9\World\CL9NightLighting.cpp:134-137`
  `OnNightPhaseChanged` 본문이 비어 있는데 "즉각적인 반응을 보장"이라 서술.
- [ ] **A-4. 미호출 함수** — `D:\Work\unreal\CL9\Source\CL9\UI\CL9SettingsWidget.h:180` / `.cpp:747`
  `RefreshLanguageButtons()` 선언·정의만 있고 호출처 없음.

### B. 기계적 위반 — 동작 변경 없음, **서식 전용 커밋 하나로** (~70곳)

표준 원칙: "동작 변경 없는 대규모 형식 변경은 동작 변경 커밋과 섞지 마라."

- [ ] **B-1. `switch`에 `default:` 없음** (2건)
  `UI\CL9HUDWidget.cpp:51` `StatLabel()`, `:508` `FillItemLabel()`의 `switch (Def->CarrySlot)`.
- [ ] **B-2. 단일 구문 블록 중괄호 생략** (2건)
  `World\CL9SiteLayout.cpp:1196-1197` `for (uint8 Edge : Layout.EdgeX) FnvMix(Hash, Edge);` ×2.
- [ ] **B-3. 람다 자동 캡처 `[&]` → 명시 캡처** (7건)
  `Character\CL9Character.cpp:1124`, `World\CL9SiteLayout.cpp:296, 460, 977, 1027, 1030, 1163`.
  전부 즉시 실행이라 실질 위험은 낮으나 규칙 위반.
- [ ] **B-4. 전방 선언으로 대체 가능한 include** (7건)
  `GameMode\CL9GameMode.h:7`, `UI\CL9ShopWidget.h:6` — `Data/CL9Types.h`가 `ECL9BuyResult` 하나 때문
  `UI\CL9HUDWidget.h:7`, `World\CL9NightLighting.h:8` — `ECL9NightPhase` 때문
  `UI\CL9MainWidget.h:7` — `ECL9UILayer` 때문
  `World\CL9SiteLayout.h:6` — `const UCL9ModuleDef*` + `ECL9PointPurpose` 때문
  프로젝트에 이미 `CL9Character.h:11` `enum class ECL9HoldPose : uint8;` 패턴이 있다.
- [ ] **B-5. 미사용·중복 include** (3건)
  `Data\CL9ShakeTable.h:7` `SubclassOf.h`(TSubclassOf 미사용), `Items\CL9Item.cpp:5` `CapsuleComponent.h`,
  `Items\CL9Item.cpp:16` `Data/CL9Types.h` 헤더와 중복.
- [ ] **B-6. 코드를 반복하는 주석** (~20건)
  `GameMode\CL9CameraManager.h:20` `/** 생성자 */`, `.cpp:7`;
  `Character\CL9Character.h:188, 197, 209, 212, 291, 295, 299`; `Character\CL9Character.cpp:839`;
  `GameMode\CL9SaveGame.h:79`; `Data\CL9CharacterDef.h:93, 97`; `Data\CL9RulesDef.h:36, 40`;
  `Items\CL9LightItem.h:50`; `UI\CL9OrderSelectWidget.cpp:201`(헤더 `:113`과 중복);
  `UI\CL9SettingsWidget.cpp:160, 163`.
  빈 생성자 안 무관한 주석: `Items\CL9Counter.cpp:12-16`, `Items\CL9Jar.cpp:11-13`, `Items\CL9TruckPanel.cpp:10-12`.
- [ ] **B-7. 헤더의 한 줄 중괄호 함수** (26건) — 표준은 `int32 GetSize() const { return Size; }`를 Bad로 명시
  `GameMode\CL9GameState.h:86, 96, 164, 168, 195, 209, 212`; `GameMode\CL9GameMode.h:144`;
  `Items\CL9Item.h:153-154`; `World\CL9SiteLayout.h:201-203, 251-258`.
  엔진 코드도 흔히 쓰는 형태라 **프로젝트 예외로 선언하는 선택지**도 있다 (아래 D 참조).
- [ ] **B-8. 줄 끝 공백** — `UI\CL9UIManager.cpp:11`.

### C. 스타일 규율 — 새 코드부터 지키고, 기존은 그 파일을 고칠 때 (일괄 처리 비권장)

- [ ] **C-1. 3절 이상 복합 조건을 이름 붙인 bool로 분리** (약 70건 — 가장 많은 위반)
  대표: `Monsters\CL9Monster.cpp:374` 6절(Break 전이); `Character\CL9Character.cpp:307-309` 5절이
  `:597-599`에 **같은 식으로 중복**(`:302-303`/`:586-587`도 중복 — 함수로 뽑을 후보);
  `UI\CL9ScanWidget.cpp:84-85` 5절.
  분포: Character 13, GameMode 6, Items 11, Monsters 12, Data 9, UI 7, World 6 (+null 체크 체인 7).
  `Items\CL9Item.cpp:343-346`, `Monsters\CL9Monster.cpp:201-203, 368-371`은 이미 규칙대로 — 두 스타일 혼재.
- [ ] **C-2. 익명 리터럴 인자에 이름 주기** (약 60건)
  절반은 엔진 API 후행 bool(`SetTimer(..., false)`, `FCollisionQueryParams(..., false, this)`,
  `AddActorWorldOffset(..., true, &Hit)`) — 엔진 자체도 그렇게 쓰므로 실효성 낮음.
  프로젝트 고유 상수라 이름을 줘야 할 것:
  `GameMode\CL9GameMode.cpp:537, 559` 스폰 거리 `900/2400`, `400/1800`, DeepShare `0.6f`;
  `Items\CL9Cart.cpp:203-205` 슬롯 배치 `60/±26/46/23`, `:149, 215` 반경 폴백 `20.f` 중복;
  `Monsters\CL9Monster.cpp:610, 618, 624` `0.5f` 재시도 지연 3회 중복;
  `World\CL9SiteLayout.cpp:798` · `World\CL9SiteMap.cpp:293` `Clamp(MaxFloors, 1, 8)` **두 파일 중복 상수**;
  `World\CL9SiteMap.cpp:576` `- 120.f` GameMode 상수와 중복;
  `GameMode\CL9Cheats.cpp` 17곳 `RouteCL9Cheat(..., false/true)`.
- [ ] **C-3. 150줄 초과 함수 분할** (5건)
  `Monsters\CL9Monster.cpp:181-464` `HostTickAI` **284줄** (상태별 switch 본문 전부 인라인);
  `World\CL9SiteMap.cpp:277-561` `Build` **285줄**; `World\CL9SiteLayout.cpp:911-1088` `BuildSegments` 178줄;
  `World\CL9SiteMap.cpp:563-719` `PlaceProps` 157줄; `World\CL9SiteLayoutTests.cpp:327-539` `RunTest` 213줄(테스트).
  근접: `GameMode\CL9GameMode.cpp:466-605` `SpawnNightPopulation` 140줄, `Data\CL9DefRegistry.cpp:127-268` `RegisterDef` 142줄.
- [ ] **C-4. bool 플래그 파라미터 → `enum class`** (후보 4건, setter·RPC 상태 전달은 예외)
  `GameMode\CL9GameState.h:70` `StartNightClock(..., bool bNoBlackout)`;
  `UI\CL9SettingsWidget.h:159` `AddSectionHeader(..., bool bWithButton = false)`;
  `UI\CL9UIManager.h:38` `OpenWidget(FName, bool bResetHistory = false)`;
  `World\CL9NightLighting.h:70` `SelectTarget(ECL9NightPhase, bool bBlackout)`.
- [ ] **C-5. 캡슐화 비일관성** — `GameMode\CL9GameState.h:110-220` `OfferedOrders` 등은 public 원시 멤버,
  같은 클래스의 `SharedMoney`/`CurrentDay`는 protected+getter. 한쪽으로 통일.

### D. 프로젝트가 "허용"으로 결정해도 되는 항목 — 결정하면 `ue-cpp-conventions.md`에 예외로 적고 `decisions.md`에 기록

엔진 코드 자체도 흔히 어기는 항목이라, 고치기보다 예외 선언이 합리적일 수 있다.

- **D-1. `Handle`로 시작하는 델리게이트 수신 함수** (20건) — `Character\CL9Character.h:358, 362`,
  `GameMode\CL9CheatManager.h:27`, `GameMode\CL9GameMode.h:60`, `Items\CL9Item.h:179`, `UI\*` 15건.
  엔진도 `AGameModeBase::HandleStartingNewPlayer` 등 같은 관례.
- **D-2. UObject 참조 전달** (13건 — 규칙 출처는 코딩 표준 본문이 아니라 Objects 문서)
  비-const: `GameMode\CL9GameMode.h:85, 92`, `Monsters\CL9Monster.h:151`;
  const: `Character\CL9Character.h:268`, `Data\CL9DefRegistry.h:196`, `GameMode\CL9GameMode.h:172, 187`,
  `GameMode\CL9GameState.h:70`, `Monsters\CL9Monster.h:142`, `UI\CL9CookGaugeWidget.h:27`,
  `UI\CL9HUDWidget.h:219`, `UI\CL9ScanLabelWidget.h:31, 43`.
  "null 불가"를 타입으로 표현하려는 의도적 선택일 수 있다.
- **D-3. 헤더의 한 줄 getter** (B-7과 동일 26건).

---

## 검증 (수정을 진행할 경우)

- A·B 후 `CL9Editor` 빌드(`D:\Program Files\Epic Games\UE_5.8`) 경고 0 유지.
- A-1은 Stair 모듈이 있는 사이트로 에디터 기동 시 `RegisterDef` 검증 로그 확인.
- B는 동작 변경이 없어야 하므로 `CL9SiteLayoutTests` 자동화 테스트 통과 + PIE 한 바퀴.
- C는 파일 단위로 손댈 때마다 해당 기능 PIE 확인. 자동 테스트는 SiteLayout 외에 없다.
