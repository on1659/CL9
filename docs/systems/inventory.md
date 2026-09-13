# 운반 시스템 (퀵슬롯·수레)

> **리썰식 4칸 퀵슬롯이다.** 활성 칸 하나만 손에 실물로 들리고, 나머지 칸은 데이터로 따라다닌다.
> 2026-09-13 사용자 결정으로 "손 하나 + 품 + R 교체 + `SwapTime` 1.2초"를 폐기했다 — 전환은 즉시다.
> **설계 정본은 plan이다** — [../plan/data/item/common.md](../plan/data/item/common.md) ·
> [../plan/data/rules.md](../plan/data/rules.md) "던지기와 운반".

## 개요

- 아이템은 **월드의 실물 액터**다. 줍는다고 목록에 들어가는 게 아니라 손에 들린다 — 단, 활성이
  아닌 칸의 것은 액터를 파괴하고 데이터(Alias·SubAlias·Value)만 남겼다가 꺼낼 때 다시 스폰한다.
- 칸 수는 `CL9Rules.QuickSlots`(4). 휠·숫자키로 전환하고 **잠금 시간이 없다**.
- `CarrySlot` enum(`Hand`/`Pocketable`)은 남아 있지만 **칸 규칙은 `HoldPose`가 정한다** — `Large`(두 손)
  아이템도 칸 하나를 차지하되 들고 있는 동안 전환·줍기가 잠긴다. `CartOnly`는 칸에 못 들어간다(손에도
  못 든다 — 지금과 같다). `Head`는 머리에 쓰며 퀵슬롯과 무관하다.
- 무게는 감속으로 작용한다 — 이진 페널티가 아니라 연속 함수이고, **네 칸의 합**이다.

## 상태 모델 (PlayerState)

```cpp
// ACL9PlayerState — 운반 상태 (A1 결정: 판정 상태는 PlayerState에)
UPROPERTY(ReplicatedUsing=OnRep_Slots)
TArray<FCL9CarriedItem> Slots;         // 퀵슬롯 칸들 — 크기 = CL9Rules.QuickSlots(호스트가 BeginPlay에서 맞춘다)

UPROPERTY(ReplicatedUsing=OnRep_ActiveSlot)
uint8 ActiveSlot;                      // 손에 들린 칸 번호(0부터) — 빈 칸이면 빈손

UPROPERTY(ReplicatedUsing=OnRep_HeadItem)
FCL9CarriedItem HeadItem;              // 머리에 쓴 것 (CarrySlot Head — 헤드램프, 2026-09-05). 캐릭터 WornItem의 미러

int32 FindEmptySlot() const;           // 첫 빈 칸, 없으면 INDEX_NONE
bool IsSlotEmpty(int32 Slot) const;

// Data/CL9Types.h
struct FCL9CarriedItem : FCL9ItemRef { int32 Value; };   // 개체가 굴린 판매가 — 칸에 넣었다 꺼내도 다시 굴리지 않는다
```

- **진실은 액터 하나다.** 캐릭터의 `HeldItem`(복제)이 활성 칸의 실물이고, `Slots[ActiveSlot]`은 UI와
  판정을 위한 미러다. 비활성 칸은 액터가 없다 — 숨김 부착보다 복제 비용이 없고 상태가 단순하다
  (2026-09-01 확정, 옛 품 규칙을 그대로 잇는다).
- **값 보존**: 칸에 넣을 때 `ACL9Item::GetValue()`를 `Value`에 저장하고, 꺼내 다시 스폰할 때
  `ACL9GameMode::SpawnItem(..., PresetValue)` → `ACL9Item::HostSetValue`로 되돌린다. 칸 전환으로 가격이 다시
  굴려지지 않는다. 행이 바뀌면(익음·탐·깨짐) 새 범위로 다시 굴리는 규칙은 그대로다 ([item.md](item.md)).
- **HUD 훅**: `OnRep_Slots`·`OnRep_ActiveSlot`·`OnRep_HeadItem` 셋 다 `OnCarryChanged`를 발동하고, 리슨
  호스트는 캐릭터가 기록 직후 `NotifyCarryChanged()`로 같은 델리게이트를 발동한다. PlayerState의 복제
  빈도는 엔진 기본 1Hz가 아니라 **20Hz**다(2026-09-13) — 칸 전환이 클라 HUD에 1초 늦게 보이면 먹통으로 느껴진다.
- **머리 슬롯은 손과 별개다** — `Head` 아이템은 E로 주우면 쓰이고(퀵슬롯이 꽉 차거나 잠겨 있어도), 퀵슬롯·수레
  규칙을 타지 않는다. 벗기 없음(현재 설계) — [../plan/data/item/headlamp.md](../plan/data/item/headlamp.md).

- 손에 든 아이템은 실물 액터가 **소지자 전신 메시의 손 소켓 `HandGrip_R`**(마네킹 스켈레톤
  공용, 없으면 `hand_r` 뼈)에 Attach되어 **보인다** — 모든 아이템이 예외 없이 같은 규칙이다
  (2026-09-12, 사용자: "모든 아이템들도 일괄로 손에 붙을 수 있게"). 행의 `GripOffset`·`GripRotation`이
  손잡이 위치를 맞춘다(손전등은 렌즈 +X가 앞). **1인칭 팔은 카메라의 자식(뷰모델)이다**(2026-09-13,
  리썰 방식 — 몸 포즈 복사 `ABP_FP_Copy`·`CtrlRig_FPWarp` 폐기): 팔 메시 `FirstPersonMesh`가
  `FirstPersonCameraComponent`에 상대 위치 `(0,0,−EyeHeight)`·요 −90°로 붙어 시선 피치·요를 통째로 따르고,
  팔 ABP(`ABP_FP_View`)는 몸 포즈를 보지 않고 카메라 로컬 클립만 재생한다. 소유자 본인 화면에서는
  `ACL9Item::UpdateFirstPersonView`가 아이템 **메시만** 팔의 같은 소켓 `HandGrip_R`로 옮기고(컴포넌트 부착은
  복제되지 않는다) 팔과 같은 1인칭 렌더 공간(`FirstPersonPrimitiveType::FirstPerson` — FOV 70·스케일 0.6)으로
  그린다 — 팔 소켓이 카메라를 따라가니 든 물건도 시선을 따라간다. 액터 자체를 팔 메시(OnlyOwnerSee)에
  붙이지 않는 이유: 남의 화면에서는 그 메시가 그려지지 않아 애니메이션이 멈추고 소켓이 기준 포즈에 남는다.
  `CarryHeight`(120cm)는 **놓는 높이**와 **칸에서 꺼내 스폰하는 높이**에 쓰인다.
- **들면 팔이 올라온다 — 행의 `HoldPose` 범주가 자세를 고른다** (2026-09-12). `ECL9HoldPose`
  `Small`(한 손, 계란·고기·팬) · `Flashlight`(앞을 향해 쥠 — 팔을 뻗어 렌즈가 화면에 들어온다) ·
  `Large`(두 손, 냄비·램프) · `None`(빈손 — 전신은 템플릿 Idle 그대로, **1인칭 팔은 화면 밖이다**:
  서 있거나 걸을 때 `AN_Farmer_FPArmsDown`으로 팔을 내려 손이 보이지 않고, 달리면(`IsRunning()`)
  `AN_Farmer_FPRun`으로 팔을 흔들며 손이 화면에 들어온다 — PlayRate = `GetGroundSpeed()` ÷ 클립 기준
  속도. 2026-09-13 사용자: "리썰처럼 최초 화면에서는 안 보이다가 뛸 때 손을 흔들면서" — 2026-09-12의
  "빈손이어도 손이 보인다"(`AN_Farmer_FPHoldIdle`)를 뒤집는다). 아이템마다 클립을 만들지 않고 범주로 묶는다. 클립은 두 벌이다 — 전신용 `AN_Farmer_Hold*`
  (남이 보는 자연스러운 높이, 가슴 앞)과 1인칭 팔용 `AN_Farmer_FPHold*`(카메라 로컬 공간에서 저작한
  전용 클립 — 리그 root가 원점, 눈이 (0,0,EyeHeight)에서 +X를 보는 공간). 전신 ABP(`ABP_Unarmed`)는
  `spine_01` 위 상체에 전신 클립을 레이어하고, 팔 ABP(`ABP_FP_View`)는 몸 포즈 복사·FPWarp 없이 팔 클립을
  그대로 재생한다(2026-09-13 — 팔은 카메라 자식이라 시선을 저절로 따르고, 팔 모션은 몸과 무관하다.
  옛 `ABP_FP_Copy`의 CopyPoseFromMesh 위 레이어 방식은 폐기, 기존 FP 클립은 이 공간으로 다시 만든다).
  `ACL9Character::GetHoldPose()`가 `HeldItem` 행에서 값을 읽고, HeldItem이 복제되니 모든 머신이
  같은 자세를 낸다. 칸을 전환하면 손의 모델과 자세가 그 칸의 아이템으로 바뀐다. 제작은 `Tools/Blender/animate_farmer_hold.py`(+`farmer_hold_spec.json`), 임포트는
  `Tools/Unreal/import_farmer_hold.py` — [../asset/character/farmer_a.md](../asset/character/farmer_a.md).
  ABP 배선(레이어·부울 블렌드·변수 갱신)은 [animation.md](animation.md) "Hold 상체 레이어 배선".
  행 값(2026-09-12 PIE로 확정): 달걀·고기·팬 `Small`, 손전등·헤드램프 `Flashlight`, 냄비·작업등 `Large`,
  수레 `None`. `GripRotation`/`GripOffset`은 1인칭 Hold 자세에서 실측해 역산했다 — 손 소켓 공간에서
  손전등은 "렌즈(+X)=카메라 정면"이 되는 회전, 두 손 물건은 "위치=양손 중점, X=카메라 정면,
  Z=월드 위"가 되는 값(냄비·작업등 공통). 남이 보는 전신 Hold 클립은 손 방향(fingers)이 1인칭 클립과
  같아 같은 값으로 맞는다. 알려진 것: 달걀은 행 `Radius` 20cm라 손에 들면 화면 하단을 크게 가리고,
  작업등은 1.5m 높이라 똑바로 들면 위로 빠져나간다(둘 다 자세가 아니라 크기 문제). 수레는 손에
  드는 게 아니라 앞에서 미는 액터(`ACL9Cart` Pushed)라 팔 자세가 없다.
- **비활성 칸에서는 빛만 잃고 들키는 건 그대로다** (`bLightLostInPocket` — 열 이름은 옛 품 시절 것이지만
  뜻은 "활성이 아닌 칸에 있을 때"다). 액터가 없으니 빛은 자연히 꺼지고, 손전등이 이 규칙의 대표 사용자다
  ([../plan/data/item/flashlight.md](../plan/data/item/flashlight.md)).
- 수레(`cart`)는 별도 액터다 — 물리 강체를 스프링으로 밀고, 짐칸 위에 떨어뜨린 아이템이 곧 짐이다. 밀면 `CartMult` 0.75
  ([../plan/data/item/cart.md](../plan/data/item/cart.md), 아래 "수레 (E)").

## 조작

| 조작 | 키 | 규칙 |
|---|---|---|
| 줍기 | **E** (`IA_PickPlace`) | `PickRadius` 120cm. **활성 칸이 비었으면 활성 칸에, 아니면 첫 빈 칸에 넣고 그 칸을 활성으로**(리썰 규칙 — 손에 든 것이 새 아이템으로 바뀐다). 네 칸이 꽉 차면 "[E] 줍기"가 뜨지 않고 E도 무시된다. 걸으면서 주우면 조용, 뛰다 주우면 시끄럽다 (rules.md 소음 반경) |
| 칸 전환 | **마우스 휠** (`IA_SlotScroll`, Axis1D) · **1~4** (`IA_Slot1..4`) | **즉시** — 잠금 시간 없음. 휠 **아래(−) = 다음 칸(오른쪽)**, 위(+) = 이전 칸, 양 끝은 순환하고 한 노치에 한 칸. 숫자키는 그 번호 칸으로. **빈 칸을 골라도 된다 = 빈손.** 휠은 리바인딩 대상이 아니고(IMC 행 `IgnoreSettings`) 숫자키는 리바인딩된다. 전환 자체는 호스트가 활성 칸 액터를 파괴하고 새 활성 칸을 다시 스폰해 손에 붙이는 것이다 |
| 놓기 | **G** (`IA_Drop`) | 활성 칸의 아이템을 앞에(`CarryHeight`) 살며시 놓는다. **그 칸은 비고 활성 칸 번호는 그대로** — 다른 칸으로 자동 전환하지 않는다. 프롬프트가 없다(들고만 있을 때 "[E] 놓기"를 띄우지 않는다, 2026-09-13 사용자) |
| 던지기 | **달리기 키 + G** | G를 누를 때 달리기 키가 눌려 있으면 차징 — `ThrowMinSpeed` 600 → `ThrowMaxSpeed` 1680(만충 `ThrowChargeTime` 0.85초), 떼면 던진다. 차징 중 달리기 키를 먼저 떼도 차징은 유지된다. 조합은 고정이고 설정에는 놓기(G) 행만 있다 — G를 리바인딩하면 던지기도 따라간다. 원점에 점프 높이 불가산 |
| 사용 (좌클) | `IA_Use` | **누르면** 행의 `UseAction`대로: 손전등 `ToggleLight`는 누르는 순간 켜고 끈다(대기 없음), 프라이팬·고기 `Swing`은 **공격 대기**에 들어간다(`Server_UseStart` → `SwingWindupServerTime` 복제, 무한 대기, 팔이 대기 자세). **떼면** 휘두른다(`Server_UseRelease` → `HostSwing`: 정면 200cm 부채꼴의 몬스터에 행의 `HitDamage`·`HitStun` — 대기 시간과 무관하게 동일). 평범하게 클릭하면 대기 한 틱 뒤 바로 타격이라 평범한 공격이고, 누르고 있으면 타이밍을 고른다(리썰 삽 방식, 2026-09-13). **대기 중 칸 전환·놓기·던지기·먹기·잡힘이 오면 대기가 취소**되어 떼도 타격이 없다. 달걀·냄비·작업등·빈손 무동작. 먹기는 여기 없다 |
| 먹기 (우클 길게) | `IA_Scan` | 우클을 `CL9Rules.UseHoldTime`(0.4초) 넘게 누르면 활성 칸이 `Dish`일 때 먹는다(`Server_Eat` → `HostEat`, 호스트 재검사). 판정은 문턱을 넘는 순간이고 먹기 애니(0.6초, `GetEatElapsed()`)는 뒤따른다 — 먹는 동안 칸 전환을 막지 않는다. 먹은 칸은 비고 활성 칸은 그대로. 문턱 전에 떼면 탭 = 스캔(`ACL9PlayerController::RequestScan`) — 스캔이 최대 0.4초 늦을 수 있다. 문턱을 넘긴 뒤 떼도 스캔은 나가지 않는다 (2026-09-13 사용자: "우클 길게 = 먹기", [input.md](input.md)) |
| 잠금 | — | `ACL9Character::CanSwitchSlot()`: **활성 칸에 `HoldPose == Large`(두 손: 냄비·작업등)가 들려 있거나 수레를 미는 중이면 false** → 휠·숫자키·줍기 전부 무시. 판정은 호스트가 같은 함수로 다시 하고, 클라의 검사는 프롬프트·헛 RPC 거르기의 보조다. G(놓기·던지기)는 잠기지 않는다 — 놓아야 풀린다 |
| 수레 (E) | `ACL9Cart` | **활성 칸이 비어 있어야(빈손) 민다** — 다른 칸에 물건이 있어도 된다. 활성 칸에 물건이 든 채 조준하면 **프롬프트가 없고 E도 무시**된다("[E] 싣기"는 없다). 수레는 **실물 크기의 물리 강체**(행 `bMeshIsBody` — 메시 UCX 콜리전이 몸체, 반경 구·자동 스케일 없음)이고, 잡으면 호스트가 매 틱 손잡이 소켓 `Grip`을 **내 캡슐 앞 `GripDistance`의 목표점으로 끄는 스프링·감쇠 힘**(`PushSpring`·`PushDamping`, 감쇠는 나와의 상대 속도)과 **yaw 스프링 토크**(`PushYawSpring`·`PushYawDamping`)를 준다 — 걷기 시작하면 반 박자 늦게 따라오고, 멈추면 미끄러지다 서고, 제자리 회전이면 원호를 그린다. 수레는 폰을 막으므로(Pawn Block) 벽에 밀어붙이면 나도 수레에 막혀 선다; 손잡이가 목표점에서 `GripBreakDistance`보다 벌어지면(옆·뒤로 벗어남, 턱 아래로 떨어짐) **손을 놓친다**(`StopPull`, "[E] 수레 밀기"가 다시 뜬다). 다시 E로 놓으면 제 속도로 미끄러지다 선다 (2026-09-13 물리 수레, [../plan/data/item/cart.md](../plan/data/item/cart.md)). **싣기는 G(놓기)·달리기 키+G(던지기)로 짐칸 위에 떨어뜨리는 것뿐** — 소켓 `Cargo` 중심 `CargoExtent` 상자(`CargoVolume`)와 겹치는 자유 상태 아이템이 곧 실린 것이고(부착 없음, 미는 동안 매 틱 깨운다), 꺼내기는 바닥 아이템과 같은 "[E] 줍기"다(미는 중에는 줍기가 잠긴다). 짐칸 안에서도 깨짐 판정은 그대로다 |
| 반납 | 항아리 E | 활성 칸의 것(+ 미는 수레의 짐칸 안 것 전부)을 넣는다 — 정전 시계가 `DarkAccel`만큼 당겨진다 ([raiding.md](raiding.md)) |
| 잡힘 | — | 몬스터에게 잡혀 넘어지면 **네 칸 전부** 발밑에 흘린다(비활성 칸은 저장된 값 그대로 다시 스폰해 흘린다). 활성 칸 번호는 그대로다 ([character.md](character.md) 피격·넘어짐) |

## 판정 경로 (호스트)

```
E   → Server_PickPlace   → HostPickUp(Item, FindPickUpSlot())     // 활성 칸이 차 있으면 HostStowHeld로 먼저 데이터화
휠  → Server_ScrollSlot(±1) ─┐
1~4 → Server_SelectSlot(n)  ─┴→ HostSelectSlot(n): HostStowHeld → ActiveSlot=n → SpawnItem(…, Value) → HostPickUp
G   → Server_Drop                 → OnPlaced          → HostClearActiveSlot
Shift+G → Server_ThrowChargeStart / Server_ThrowRelease → OnThrown → HostClearActiveSlot
좌클 누름 → Server_UseStart   : Swing이면 SwingWindupServerTime = 지금 / ToggleLight면 즉시 토글
좌클 뗌   → Server_UseRelease : 대기 중이었으면 HostSwing (칸 전환·놓기·던지기·먹기·잡힘은 HostCancelSwingWindup)
우클 길게 → Server_Eat (활성 칸 Dish) → HostEat → EatStartServerTime = 지금 → HostConsumeHeldItem
먹기·항아리·창구 → HostConsumeHeldItem → HostClearActiveSlot     // 수레 싣기 경로는 없다 — G/던지기로 떨어뜨린다
```

- 휠은 목표 칸을 클라가 계산하지 않는다 — 클라의 `ActiveSlot` 사본은 복제가 늦을 수 있어, 호스트가 자기 값에서
  한 칸 옮긴다(연속 노치가 같은 칸을 겨누지 않는다). 숫자키는 절대 번호라 `Server_SelectSlot`이다.
- `ResolveFocus`·`GetFocusPrompt`·`Server_PickPlace`가 같은 `CanPickUpItem()`(= `CanSwitchSlot() && 빈 칸 있음`)으로
  판단하므로 하이라이트·프롬프트·실제 결과가 어긋나지 않는다.

## 무게와 감속

```
유효 무게 = Σ(네 칸의 Weight) × 캐릭터 CarryMult
감속 = f(유효 무게)          ← 식·값 미정 (전역 시스템의 몫 — plan/data/item/common.md)
```

- 캐릭터·먹기 버프는 속도가 아니라 **무게 축**으로 작용한다 (`CarryMult`).
- **감속 적용은 CMC 예측 경로에서** — 유효 무게에서 파생한 MaxSpeed를 PlayerState 복제에서
  읽으면 호스트·클라 적용 시점이 갈라져 줍고 놓을 때마다 러버밴딩이 난다 (conventions.md § 4 예외).
- 고기는 안 깨지는 대신 무겁다 — 그게 고기의 마찰이다.
- "무게 초과로 추가 불가" 같은 상한 규칙이 없다 — 느려질 뿐이다. 칸 수(4)가 유일한 상한이다.

## 달리기 흔들림

들고 달리면 흔들림이 쌓인다 — `ShakeFullTime` 3.0초 만충, `ShakeDrainTime` 1.5초 회복.
**만충 시 손의 취약물이 금 간다 (초안 — 이미 금 갔으면 깨짐)** — 판정·표현 상세는
[character.md](character.md) 흔들림 절. **바를 만들지 않는다** — 화면 흔들림 진폭으로만
읽힌다 ([../plan/data/rules.md](../plan/data/rules.md)).

## 네트워크 동기화

- 줍기·놓기·던지기·칸 전환 입력: **Server RPC** → 호스트 판정 ([input.md](input.md))
- 운반 상태: PlayerState의 `Slots`·`ActiveSlot`·`HeadItem` Replicated + OnRep으로 HUD 갱신; 손의 실물은
  캐릭터 `HeldItem` 복제 + 아이템 액터 부착 복제
- 아이템 액터의 소유 전환: 호스트가 액터를 캐릭터에 Attach/Detach — 클라는 표현만. 비활성 칸은 액터가 없으니
  복제할 것도 없다

## 현재 범위에 없는 것 (이전 판과의 차이)

- ❌ 손 하나 + 품 + R 교체 + `SwapTime` 1.2초 — 2026-09-13 퀵슬롯으로 대체
- ❌ 슬롯 그리드 인벤토리 (20칸, 드래그앤드롭) — 퀵슬롯 4칸이 전부, 드래그·정렬 없음
- ❌ 리썰의 유틸리티 벨트(한 손 도구 전용 5번째 칸)·전환 딜레이(0.2~0.3초) — 칸은 넷뿐이고 전환은 즉시
- ❌ 스택·수량 관리 UI — 실물이 전부다
- ❌ 장비 착용 슬롯 (Hand/Feet) — Head(헤드램프)만 있다 (plan/data/equipment.md)
- ❌ kg 상한·이진 감속

## 참고

- 아이템 물리(깨짐·반발): [../plan/data/item/egg.md](../plan/data/item/egg.md)
- 운반 상수(`QuickSlots` 등): [../plan/data/rules.md](../plan/data/rules.md)
- 입력 바인딩·차징 구현: [input.md](input.md)
- HUD 퀵슬롯 표시: [ui.md](ui.md)
