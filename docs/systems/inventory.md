# 운반 시스템 (손·품·수레)

> 슬롯 인벤토리가 아니다. **손 하나 + 품 + 수레** — 운반 마찰이 이 게임의 축이다.
> **설계 정본은 plan이다** — [../plan/data/item/common.md](../plan/data/item/common.md) ·
> [../plan/data/rules.md](../plan/data/rules.md) "던지기와 운반".

## 개요

- 아이템은 **월드의 실물 액터**다. 줍는다고 목록에 들어가는 게 아니라 손에 들린다.
- `CarrySlot`이 운반 방식을 가른다 — `Hand` 손 전용 / `Pocketable` 품에 넣을 수 있음 /
  `CartOnly` 수레만.
- **손↔품 교체에 `SwapTime` 1.2초가 걸린다** — 의도된 운반 마찰이다 (rules.md).
- 무게는 감속으로 작용한다 — 이진 페널티("초과 시 50% 감소")가 아니라 연속 함수다.

## 상태 모델 (PlayerState)

```cpp
// ACL9PlayerState — 운반 상태 (A1 결정: 판정 상태는 PlayerState에)
UPROPERTY(Replicated)
FCL9CarriedItem HandItem;              // 손에 든 것 (Alias+SubAlias, 없으면 None)

UPROPERTY(Replicated)
TArray<FCL9CarriedItem> PocketItems;   // 품 (Pocketable만 — 용량 규칙 미정)

UPROPERTY(Replicated)
float SwapLockUntil;                   // 손↔품 교체 잠금 (1.2초) — GetServerWorldTimeSeconds 기준

UPROPERTY(Replicated)
FCL9CarriedItem HeadItem;              // 머리에 쓴 것 (CarrySlot Head — 헤드램프, 2026-09-05). 캐릭터 WornItem의 미러
```

- **머리 슬롯은 손과 별개다** — `Head` 아이템은 E로 주우면 쓰이고(손이 차 있어도), 손·품·수레
  규칙을 타지 않는다. 벗기 없음(현재 설계) — [../plan/data/item/headlamp.md](../plan/data/item/headlamp.md).

- 손에 든 아이템은 실물 액터가 **소지자 전신 메시의 손 소켓 `HandGrip_R`**(마네킹 스켈레톤
  공용, 없으면 `hand_r` 뼈)에 Attach되어 **보인다** — 모든 아이템이 예외 없이 같은 규칙이다
  (2026-09-12, 사용자: "모든 아이템들도 일괄로 손에 붙을 수 있게"). 행의 `GripOffset`·`GripRotation`이
  손잡이 위치를 맞춘다(손전등은 렌즈 +X가 앞). 1인칭 팔 메시는 전신 포즈를 복사(`ABP_FP_Copy`)하므로
  자기 화면에서도 같은 손에 들려 보이고, 소유자 화면에서만 아이템을 팔과 같은 1인칭 렌더 공간
  (`FirstPersonPrimitiveType::FirstPerson` — FOV 70·스케일 0.6)으로 그려 팔과 어긋나지 않게 한다.
  팔 메시(OnlyOwnerSee)에 붙이지 않는 이유: 남의 화면에서는 그 메시가 그려지지 않아 애니메이션이
  멈추고 소켓이 기준 포즈에 남는다. `CarryHeight`(120cm)는 이제 **놓는 높이**에만 쓰인다.
  진실은 액터 하나다 (PlayerState의 HandItem은 미러).
- **들면 팔이 올라온다 — 행의 `HoldPose` 범주가 자세를 고른다** (2026-09-12). `ECL9HoldPose`
  `Small`(한 손, 계란·고기·팬) · `Flashlight`(앞을 향해 쥠) · `Large`(두 손, 냄비·램프) · `None`(빈손 —
  전신은 템플릿 Idle 그대로, **1인칭 팔은 `AN_Farmer_FPHoldIdle`로 두 손을 화면 아래 양쪽에 둔다**.
  사용자 2026-09-12: "idle일 때도 손을 볼 수 있어야 돼", 리썰컴퍼니 참고 — 빈손이어도 손이 보인다). 아이템마다 클립을 만들지 않고 범주로 묶는다. 클립은 두 벌이다 — 전신용 `AN_Farmer_Hold*`
  (남이 보는 자연스러운 높이, 가슴 앞)과 1인칭 팔용 `AN_Farmer_FPHold*`(카메라 안에 들어오도록
  눈높이 가까이 올린 것). 전신 ABP(`ABP_Unarmed`)는 `spine_01` 위 상체에 전신 클립을, 팔 ABP
  (`ABP_FP_Copy`)는 전신 포즈 복사 위에 팔 클립을 같은 방식으로 레이어한다 — FPS 뷰모델 관행이다.
  `ACL9Character::GetHoldPose()`가 `HeldItem` 행에서 값을 읽고, HeldItem이 복제되니 모든 머신이
  같은 자세를 낸다. 제작은 `Tools/Blender/animate_farmer_hold.py`(+`farmer_hold_spec.json`), 임포트는
  `Tools/Unreal/import_farmer_hold.py` — [../asset/character/farmer_a.md](../asset/character/farmer_a.md).
  ABP 배선(레이어·부울 블렌드·변수 갱신)은 [animation.md](animation.md) "Hold 상체 레이어 배선".
  행 값(2026-09-12 PIE로 확정): 달걀·고기·팬 `Small`, 손전등·헤드램프 `Flashlight`, 냄비·작업등 `Large`,
  수레 `None`. `GripRotation`/`GripOffset`은 1인칭 Hold 자세에서 실측해 역산했다 — 손 소켓 공간에서
  손전등은 "렌즈(+X)=카메라 정면"이 되는 회전, 두 손 물건은 "위치=양손 중점, X=카메라 정면,
  Z=월드 위"가 되는 값(냄비·작업등 공통). 남이 보는 전신 Hold 클립은 손 방향(fingers)이 1인칭 클립과
  같아 같은 값으로 맞는다. 알려진 것: 달걀은 행 `Radius` 20cm라 손에 들면 화면 하단을 크게 가리고,
  작업등은 1.5m 높이라 똑바로 들면 위로 빠져나간다(둘 다 자세가 아니라 크기 문제). 수레는 손에
  드는 게 아니라 앞에서 미는 액터(`ACL9Cart` Pushed)라 팔 자세가 없다.
- **품에 넣으면 실물 액터는 파괴하고 데이터(Alias+SubAlias)만 보존한다** — 꺼낼 때 재스폰.
  숨김 부착보다 복제 비용이 없고 상태가 단순하다 (2026-09-01 확정).
- **품에 넣으면 빛만 잃고 들키는 건 그대로다** (`bLightLostInPocket`) — 손전등이 이 규칙의
  대표 사용자다 ([../plan/data/item/flashlight.md](../plan/data/item/flashlight.md)).
- 수레(`cart`)는 별도 액터다 — `CartOnly` 아이템을 싣고 끌면 `CartMult` 0.75
  ([../plan/data/item/cart.md](../plan/data/item/cart.md)).

## 조작

| 조작 | 규칙 |
|---|---|
| 줍기 | `PickRadius` 120cm. 걸으면서 주우면 조용, 뛰다 주우면 시끄럽다 (rules.md 소음 반경) |
| 놓기 | 손에 든 것을 앞에 놓는다 |
| 사용 (좌클) | **탭** = 행의 `UseAction`: 손전등 `ToggleLight`, 프라이팬·고기 `Swing`(정면 200cm 부채꼴의 몬스터에 행의 `HitDamage`·`HitStun`), 달걀·냄비·작업등·빈손 무동작. **길게(0.4초)** = `Dish`면 먹기 (2026-09-12, [input.md](input.md)) |
| 던지기 (G) | 차징 — `ThrowMinSpeed` 600 → `ThrowMaxSpeed` 1680 (만충 0.85초). 원점에 점프 높이 불가산 |
| 손↔품 교체 | `SwapTime` 1.2초 잠금 — 교체 중에는 다른 조작 불가 |
| 수레 (E) | 빈손으로 조준해 잡으면 **앞에서 밀고 다닌다** — 연결 거리(캡슐+수레 반경+20cm)에 서서 내 yaw를 따라 돌고, 벽에 막히면 나도 멈춘다. 밀리는 동안 수레는 미는 사람을 이동 충돌에서 빼고(`IgnoreActorWhenMoving`), 시작부터 바닥 등에 묻혀 있으면(손으로 놓은 수레가 반경보다 낮은 경우) 법선 방향으로 빼낸 뒤 민다. 다시 E로 놓는다 (2026-09-12: 뒤에서 끌기 → 밀기, [../plan/data/item/cart.md](../plan/data/item/cart.md)) |
| 반납 | 항아리에 넣는다 — 정전 시계가 `DarkAccel`만큼 당겨진다 ([raiding.md](raiding.md)) |

## 무게와 감속

```
유효 무게 = Σ(든 것의 Weight) × 캐릭터 CarryMult
감속 = f(유효 무게)          ← 식·값 미정 (전역 시스템의 몫 — plan/data/item/common.md)
```

- 캐릭터·먹기 버프는 속도가 아니라 **무게 축**으로 작용한다 (`CarryMult`).
- **감속 적용은 CMC 예측 경로에서** — 유효 무게에서 파생한 MaxSpeed를 PlayerState 복제에서
  읽으면 호스트·클라 적용 시점이 갈라져 줍고 놓을 때마다 러버밴딩이 난다 (conventions.md § 4 예외).
- 고기는 안 깨지는 대신 무겁다 — 그게 고기의 마찰이다.
- "무게 초과로 추가 불가" 같은 상한 규칙이 없다 — 느려질 뿐이다.

## 달리기 흔들림

들고 달리면 흔들림이 쌓인다 — `ShakeFullTime` 3.0초 만충, `ShakeDrainTime` 1.5초 회복.
**만충 시 손의 취약물이 금 간다 (초안 — 이미 금 갔으면 깨짐)** — 판정·표현 상세는
[character.md](character.md) 흔들림 절. **바를 만들지 않는다** — 화면 흔들림 진폭으로만
읽힌다 ([../plan/data/rules.md](../plan/data/rules.md)).

## 네트워크 동기화

- 줍기·놓기·던지기·교체 입력: **Server RPC** → 호스트 판정 ([input.md](input.md))
- 운반 상태: PlayerState의 Replicated 변수 + OnRep으로 부착 메시·HUD 갱신
- 아이템 액터의 소유 전환: 호스트가 액터를 캐릭터에 Attach/Detach — 클라는 표현만

## 현재 범위에 없는 것 (이전 판과의 차이)

- ❌ 슬롯 그리드 인벤토리 (20칸, 드래그앤드롭)
- ❌ 스택·수량 관리 UI — 실물이 전부다
- ❌ 장비 착용 슬롯 (Head/Hand/Feet) — 착용형 장비가 없다 (현재 설계 — plan/data/equipment.md)
- ❌ kg 상한·이진 감속

## 참고

- 아이템 물리(깨짐·반발): [../plan/data/item/egg.md](../plan/data/item/egg.md)
- 운반 상수(`SwapTime` 등): [../plan/data/rules.md](../plan/data/rules.md)
- 입력 바인딩·차징 구현: [input.md](input.md)
- HUD 짐 슬롯 표시: [ui.md](ui.md)
