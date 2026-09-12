# 애니메이션 규칙

> 애니메이션은 표현이다 — 판정에 들어가지 않고, 상태에서 파생된다. 이 문서는 스켈레톤·AnimBP
> 연결 규칙과 상태→애님 흐름을 정의한다.

## 규칙 셋

1. **C++에 애님·메시 경로 0줄** (build-order 원칙 2). 연결은 전부 데이터다 —
   캐릭터는 `UCL9CharacterDef`의 `SkeletalMesh`/`Skeleton`/`AnimBP` 소프트 참조, 몬스터도 동일
   ([../plan/data/character.md](../plan/data/character.md) · [../plan/data/monster/common.md](../plan/data/monster/common.md)).
   `BP_CL9Character`(블루프린트 자식)가 메시·AnimBP를 갖고, C++은 캡슐과 로직만.
2. **애님은 판정에 못 들어간다.** 애님 노티파이로 상태를 바꾸지 않는다 — 발소리 사운드·파티클
   같은 렌더 전용 노티파이만 허용. 판정 타이밍(경직 길이 등)은 값(`HitStunMs`)이 정하고
   애님이 값에 맞춘다, 반대가 아니다.
3. **캡슐이 기준이고 메시가 맞춘다** (39/90). 산 에셋이 커도 캡슐을 키우지 않는다 —
   통로 폭 255cm가 걸려 있다.

## 보유 에셋 (2026-09-02 확인 — 프로젝트 Content에 이미 있음)

| 에셋 | 위치 | 용도 |
|---|---|---|
| UE5 마네킹 (Manny/Quinn) + 애님 (Walk/Jog/Jump/Attack/Death/HitReact) | `Content/Characters/Mannequins/` | 1c 임시 캐릭터 메시 — 계획 그대로 |
| FPP 팔 (ABP_FP + 컨트롤릭) | `Content/FirstPerson/Anims/` | 1인칭 손 뷰모델의 출발점 |
| 동물 5종 자체 제작 SK + 클립 (닭·거위·너구리·돼지·양) | `Content/Monsters/<종>A/` | 2026-09-11 연결 — `Content/Monsters/ABP_<종>_A`(ABP_Chicken 복제·종별 스켈레톤), `MonsterTable` 행이 가리킴. 제작 기록은 [docs/asset/monster/](../asset/monster/README.md), 결정은 decisions.md 2026-09-11 |
| (구) Dreamscape 닭·돼지·양 SK | `Content/DreamscapeSeries/.../Chars/` | 09-10 팩 축소로 애님 시퀀스가 사라져 **더 이상 쓰지 않는다** — `ABP_Chicken`·`ABP_Pig`·`ABP_Sheep`은 빈 플레이어인 채 남아 있음 |
| Farmlands 환경 (농장 건물·지형·폴리지) + Horror 조명 깜빡임 | `DreamscapeFarmlands/` · `Variant_Horror/` | 5a 맵·light.md 참고 |

거위·너구리도 2026-09-11부터 전용 메시(`SK_Goose_A`·`SK_Raccoon_A`)를 쓴다 — 닭 빌려 쓰기·AVP 여우는 끝.

## 스켈레톤 규칙

- **캐릭터: UE5 마네킹 스켈레톤 호환 에셋을 산다 (권장 구매 기준).** build-order 5e —
  임시 메시로 만든 애님이 산 캐릭터와 뼈대가 다르면 다시 만들게 된다. 마네킹 호환이면
  1c 임시 메시(마네킹)의 애님 작업을 그대로 승계한다. 리타깃이 필요한 에셋은 그 비용을
  알고 산다.
- **몬스터: 종별 스켈레톤 1개.** 닭 100마리가 스켈레톤 하나를 공유한다 — `Skeleton` 컬럼이
  가리키고, 같은 종의 변형(에셋 교체)은 같은 스켈레톤을 쓰는 게 기본.

## 상태 → 애님 흐름

애님은 Replicated 상태에서 **파생**된다. RPC로 애님을 직접 트리거하지 않는다 (상태가 정본,
애님은 구독자). 일회성 연출만 Multicast를 쓴다 (사운드 규칙과 동일 — conventions.md § 9).

### 캐릭터 (AnimBP가 읽는 값)

| 값 | 출처 |
|---|---|
| 속도·방향·공중 여부 | CMC (예측 이동 — 자연히 전 머신 동기) |
| 달리기 여부 | CMC 커스텀 플래그 (`FSavedMove`에 실린 것) |
| 손에 든 것 | PlayerState.HandItem (OnRep) — 들기 포즈 전환 |
| 넘어짐·비틀(완충타)·탈진 | PlayerState 상태 (OnRep) |

### 몬스터 (AnimBP가 읽는 값)

| 값 | 출처 |
|---|---|
| `ECL9MonsterState` | Replicated + OnRep ([monster.md](monster.md)) — 상태별 로코모션/포즈 |
| 이동 속도 | 액터 속도 (복제 위치에서 파생 — 클라 보간) |
| 경직·기상 예고 | 상태 전이 (WakeTelegraph는 전 플레이어에게 보인다) |

### 로코모션 재생 배율 — 풋 슬라이딩 방지 (기본 규칙)

제자리(in-place) 이동 애님을 고정 배속으로 틀지 않는다. **모든 로코모션 애님은
`PlayRate = 현재 속도 ÷ 기준 속도`로 구동한다** (2026-09-05 확정 — 이후 만드는 모든
이동 애님의 기본). 기준 속도는 "1.0배속에서 발이 미끄러지지 않는 이동 속도"로, 팩 애님엔
명시가 없으므로 PIE에서 눈으로 실측 보정한다.

- **블렌드 문턱은 테이블 속도와 정합**시킨다 — 배회 속도 = 완전 걷기, 추격 속도 = 완전
  달리기. 그 사이 속도는 포즈 블렌드가 아니라 재생 배율이 흡수한다.
- 기준 속도 (2026-09-11, 자체 제작 클립은 표 속도에 맞춰 발을 심었으므로 manifest의
  `reference_speed_cm_s`가 곧 SafeDivide 상수): Walk = WanderSpeed(닭·거위·돼지·양 126, 너구리 140),
  Run = ChaseSpeed(닭·돼지·양 396, 거위 528, 너구리 462). 팩 애님 시절의 눈대중 값(닭 165/396,
  여우 210/462)은 폐기.
- 종마다 ABP가 따로 있으므로(`ABP_<종>_A`) 속도 상수도 종마다 제 값이다 — 이제 공유 ABP는 없다.

## 1인칭 팔 (FPP Arms)

시점이 1인칭이고 **자기 화면에는 손(팔)만 보인다** ([character.md](character.md)):

- **팔 전용 메시 + 팔 전용 AnimBP**를 풀바디와 별도로 둔다 (표준 FPS 관행) — 카메라에는
  팔 메시만, 타인에게는 풀바디만 보이게 Owner See / Owner No See로 분리.
- 든 오브젝트는 **전신 메시의 손 소켓 `HandGrip_R`**에 부착하고(2026-09-12 구현 — 팔 메시가
  아니라 전신인 이유는 [inventory.md](inventory.md)), 팔 메시는 FP Copy로 같은 포즈를 따라간다.
  줍기·던지기·교체·손전등 토글의 1인칭 애님과 Hold 자세 클립이 여기 산다. Hold 자세는 범주별
  (`ECL9HoldPose` Small/Flashlight/Large) 전신 클립 + 1인칭 팔 클립 한 쌍으로 2026-09-12에 들어갔다 —
  [inventory.md](inventory.md) "들면 팔이 올라온다". 줍기·던지기 동작 클립은 아직 없다.
- **Hold 상체 레이어 배선** (2026-09-12, ABP_Unarmed·ABP_FP_Copy 동일 구조): 기존 포즈 체인 사이에
  `본별 레이어드 블렌딩`(`spine_01`, 로컬 공간, BlendDepth 0)을 끼운다 — 전신은 `Slot(DefaultSlot)` →
  레이어 → `ControlRig(발 IK)`, 팔은 `CopyPoseFromMesh` → 레이어 → `ControlRig(FPWarp)`. 레이어의
  블렌드 포즈는 **부울 블렌드 2단**: `HoldPose == Large ? HoldLarge : (HoldPose == Flashlight ?
  HoldFlashlight : HoldSmall)` (팔은 `FPHold*` 클립), 전신의 가중치는 `ToFloat(NOT(HoldPose == None))`.
  **팔 ABP는 가중치 1 고정 + `HoldPose == None ? FPHoldIdle : (위 체인)` 한 단을 더 둔다** (2026-09-12 낮,
  사용자 "idle에서도 손이 화면에 보여야 한다 — 리썰컴퍼니처럼 오른쪽 아래"): 빈손에서도 두 손이 화면 아래
  양쪽에 보인다. 전신(남이 보는 것)은 빈손이면 템플릿 Idle(손 내림) 그대로.
  `BlendPoses(ECL9HoldPose)` 노드를 안 쓴 이유: 항목 핀 노출(`ExposeEnumElementAsPin`)이 에디터
  컨텍스트 메뉴 전용이라 MCP/파이썬으로 만들 수 없다 — 부울 블렌드는 모양이 고정이라 자동화가 된다.
  ABP 변수 `HoldPose`(ECL9HoldPose)는 EventGraph가 매 틱 `Cast To CL9Character → GetHoldPose`로
  채운다(전신은 `SetIsFalling` 뒤, 팔은 `BlueprintUpdateAnimation → TryGetPawnOwner`). 변수 생성은
  `BlueprintEditorLibrary.add_member_variable` + `EdGraphPinType.import_text('(PinCategory="byte",
  PinSubCategoryObject="/Script/CL9.ECL9HoldPose")')`(MCP `add_variable`은 프리미티브만 받는다).
  PIE(Lvl_Flat) 확인: 손전등·달걀·냄비·작업등 모두 두 ABP의 `HoldPose`가 행 값과 일치하고 아이템이
  화면 하단(x 0.55~0.64, y 0.75~0.89)에 들어온다. 카메라·팔 메시 상대 위치는 건드리지 않았다.
- 데이터 연결: `UCL9CharacterDef`에 팔 메시·팔 AnimBP 컬럼 추가 필요 (미결 — 1c에서).

## 몽타주 사용처

전부 렌더 전용 — 길이는 판정 값에 맞춘다:

| 몽타주 | 판정 값 | 비고 |
|---|---|---|
| 죽음 | HP 0 | 기본은 같은 메시의 죽는 모션 — 래그돌 등 다른 연출은 종별 선택 |
| 경직 | `HitStunMs` / `PushStun` | 전용 애니메이션 필수 (plan/data/monster/common.md) |
| 잡힘·넘어짐 | CatchRadius 판정 | 짐 흘림과 함께 |
| 기상 예고 | `WakeTelegraph` 0.8s | 퍼덕임 + 울음 |
| 거위 울음 예비 | `AlertWindup` 0.55s | |

## 구현 시점

- 1c: 임시 메시(마네킹) + 기본 로코모션 — "몸이 보인다"
- ③: 몬스터 상태별 애님 (죽음·경직·예고가 상태 기계와 함께)
- 5d/5e: 산 에셋으로 교체 + 본 애니메이션 — 데이터 컬럼만 갈아끼우면 되는 게 이 규칙의 목적

## 참고

- 데이터 컬럼: [../plan/data/character.md](../plan/data/character.md) ·
  [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 상태 정의: [monster.md](monster.md) · [inventory.md](inventory.md)
- 표현 계층 규칙: conventions.md § 9 (사운드와 같은 원칙)
