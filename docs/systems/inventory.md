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
```

- 손에 든 아이템은 실물 액터가 캐릭터에 Attach되어 **보인다** — 타인 시점은 풀바디의
  `CarryHeight`(120cm) 위치, **자기 1인칭 화면에서는 손(팔) 뷰모델의 소켓에 붙는다**
  ([character.md](character.md) 시점). 진실은 액터 하나다 (PlayerState의 HandItem은 미러).
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
| 던지기 | 차징 — `ThrowMinSpeed` 600 → `ThrowMaxSpeed` 1680 (만충 0.85초). 원점에 점프 높이 불가산 |
| 손↔품 교체 | `SwapTime` 1.2초 잠금 — 교체 중에는 다른 조작 불가 |
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
