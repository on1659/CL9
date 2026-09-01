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
// 열원(팬) 위에 아이템이 놓이면 — 호스트 전용 판정
void ACL9HeatSource::TickCooking(float Now) {
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

## 조리 용기

**동시 요리 수 = 보유한 조리 용기 수.** "조리대 1개, 1명 1요리" 같은 별도 제한은 없다 —
용기 수가 곧 제한이다.

- 프라이팬(`frying_pan`)이 기본. 냄비(`pot`)를 사면 두 번째 조리 용기가 생긴다
  ([../plan/data/equipment.md](../plan/data/equipment.md))
- 용기는 실물 장비다 — 들고 다니고, 원정에 두고 오면 잃는다

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
