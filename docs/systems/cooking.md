# 요리 시스템

> 열원 위에서 시간이 지나면 재료가 변한다 — 날것 → 익음 → 탄 것. 그게 전부다.
> **설계 정본은 plan이다** — [../plan/loop.md](../plan/loop.md)의 "요리",
> [../plan/data/item/common.md](../plan/data/item/common.md)의 요리 컬럼.

## 개요

- **두 번째 게임이 아니다.** 요리는 미니게임이 아니라 원정과 같은 물리 조작이다 (plan/loop.md).
- 하는 일은 **놓고 · 기다리고 · 집기**뿐이다. 원정과 같은 물리 조작이다.
- 새로 생기는 시스템은 **열원 위 상태 변화** 하나뿐이다.
- **성공률·난이도·실패 판정이 없다.** 방치하면 타는 것이 유일한 실패다.

## 상태 변화 체인

데이터는 아이템 표의 요리 컬럼이다 (`CookResult` / `BurnResult` / `CookTime` / `BurnTime`):

```
계란 → (CookTime 12초) → 익은 계란(egg+cooked) → (BurnTime 24초) → 탄 계란(egg+burnt)
```

- 상태 변화는 시간이 흐르면 저절로 일어난다. 뒤집기·불 조절은 없다 (현재 설계).
- **탄 요리는 값이 0이다.** 그리고 연기가 난다 — `BurnWarnTime`(6초) 연기 기둥 예고는
  **렌더 전용**이라 판정에 안 들어간다.
- 탄 요리는 미끼가 된다 (미끼에 연결된 시스템은 현재 2건 — rules.md).

## 구현 구조

요리는 별도 시스템이 아니라 **열원 액터 + 아이템 상태 전이**다.

```cpp
// 조리대 위에 아이템이 놓이면 — 호스트 전용 판정
void ACL9Kitchen::TickCooking(float Now) {
  if (!HasAuthority()) return;

  for (ACL9Item* Item : ItemsOnHeat) {
    const UCL9ItemDef* Def = DefRegistry->FindItem(Item->GetAlias(), Item->GetSubAlias());
    if (Def->CookResult.IsNone()) continue;   // 익지 않는 물건

    if (Now - Item->HeatStartTime >= Def->CookTime) {
      Item->SetAlias(Def->CookResult.ItemAlias, Def->CookResult.ItemSubAlias);  // egg → egg+cooked
      // SubAlias 교체 → OnRep으로 전 클라 메시/표현 갱신 (actor-base.md)
    }
  }
}
```

- 상태 전이는 아이템의 `Alias`+`SubAlias` 교체다 — 새 액터를 만들지 않는다.
- 놓기·집기는 운반 시스템의 조작 그대로다 ([inventory.md](inventory.md)). 요리 전용 조작이 없다.

## 열원은 조리대다

**요리는 부엌에서 한다.** 열원은 조리대(`ACL9Kitchen`)이고, **조리대 하나가 한 번에 하나를
익힌다** — 동시 요리 수는 조리대가 몇 개인가로 정해진다 (2026-09-06 사용자 결정).

- 상판에 여러 개를 올려도 먼저 자리를 잡은 하나만 시계가 돌고, 그것이 내려가면 남은 것 중
  하나가 이어받는다. 기다리는 재료는 열을 받지 않는다(게이지도 뜨지 않는다).
- **프라이팬·냄비는 열원이 아니다.** 그냥 아이템 하나일 뿐이며 요리와 무관하다
  ([../plan/data/equipment.md](../plan/data/equipment.md)).
- 조리대는 지금 레벨에 손으로 놓는다. **나중에는 던전 안에 맵 생성기가 배치한다**
  ([map.md](map.md)) — 요리하는 자리가 원정지 안에 있다는 뜻이다.

## 네트워크 동기화

- **상태 전이 판정**: 호스트 전용 (열원 액터에서, HasAuthority 관문)
- **표현**: Alias/SubAlias가 Replicated이므로 OnRep → LoadData로 자동 반영
  ([actor-base.md](actor-base.md))
- 연기·소리는 표현 계층에서 (conventions.md § 9)

## UI

진행도 바·취소 버튼·레시피 창이 없다 (현재 설계). 익음/탐은 **월드에서 읽힌다** —
메시 변화와 연기 기둥이 정보의 전부다.

## 참고

- 무엇이 무엇이 되는지: [../plan/data/item/common.md](../plan/data/item/common.md) 요리 절 (계란 체인은 [../plan/data/item/egg.md](../plan/data/item/egg.md))
- 완성물 판매·납품: [economy.md](economy.md) · [order.md](order.md)
- 먹기 효과 (Dish 타입): [../plan/data/item/common.md](../plan/data/item/common.md) 타입과 먹기 절

---

## 구현 상태 (④ 4d — 2026-09-06, 열원 재설계 반영)

`ACL9Kitchen`(Source/CL9/Items, BP `Content/Items/BP_CL9Kitchen`) — 레벨에 놓는 고정물이며 상호작용이 없다
(놓기·집기가 곧 조작이라 `ICL9Interactable`을 구현하지 않는다).

- 상판 위 `HeatZone`(몸체 크기에서 만들어지는, 상판 위 60cm 상자)에 **놓인**(Free) 아이템을 호스트가 매 틱 질의한다.
  열 구역은 몸체 스케일과 무관하게(절대 스케일) 계산되므로 조리대를 넓혀도 열 높이는 그대로다.
- **한 번에 하나만 익는다.** 자리가 빈 동안에만 다음 재료를 집어 들고, 기다리는 재료는 `None`으로 지워 게이지가
  뜨지 않게 한다. 익히던 것이 집혀 가거나 굴러 나가면 시계는 0으로 돌아간다.
- 행에 `CookResult`가 있으면 `CookTime` 뒤 그 행으로, 없고 `BurnResult`만 있으면 `BurnTime` 뒤 그 행으로 `SetAlias`
  (액터 교체 없음 — OnRep으로 클라 메시 갱신). 탄 것이 되면 자리를 비워 다음 재료가 올라온다.
- **진행도는 아이템에 실려 복제된다** — `ACL9Item`의 `GetCookProgress()`(0~1) · `IsOnHeat()` · `IsNextBurnt()` ·
  `GetCookPhase()`(`ECL9CookPhase` None/Cooking/Burning). 복제 실체는 `uint8` 하나와 enum 하나로, 표현(게이지) 전용이다.
  HUD의 요리 게이지가 이걸 읽는다(UI 세션).
- **"무엇을 익히는 중인가"는 조리대에 실려 복제된다** — `ACL9Kitchen::GetCookingItem()`. HUD가 익히는 중인
  조리대마다 그 꼭대기(`GetTopLocation()`) 위에 게이지를 띄우는데, 클라도 조리대↔재료 짝을 알아야 해서다
  (2026-09-10 — ui.md 조리 게이지).
- `BurnWarnTime` 전 "연기" 로그만 있다 — **연기 VFX·소리는 미구현** (표현 계층, 에셋 없음).
- **불(버너)은 아직 없다** — 조리대는 늘 뜨겁다.

조작은 운반 그대로: 재료를 들고 조리대 앞에서 놓기(E)를 누르면 상판 위로 떨어진다.

**손 높이 낙하로 계란이 금 가는 문제(4d 미결)는 조리대 높이로 푼다.** 놓기는 손 높이
(`CarryHeight` 120)에서 떨어뜨리므로 상판이 낮을수록 낙차가 커지고, 팬이 바닥에 있던 때는
낙차 90cm(충돌 ≈590 > `CrackSpeed` 450)로 늘 금이 갔다. 조리대 상판을 사람 허리 높이(≈90cm)로
두면 낙차가 30cm로 줄어든다 — **다만 이전 실측에서 45cm 낙하로도 금이 간 적이 있어(2026-09-06),
계산만 믿지 말고 PIE로 확인한다.** 그래도 금이 가면 후보는 ① 금 간 계란도 익게 표에 `CookResult`
추가, ② 놓기를 낙하 없이 안착으로 바꾸기다.
