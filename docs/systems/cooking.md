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

---

## 구현 상태 (④ 4d — 2026-09-06)

`ACL9FryingPan : ACL9Item`(Source/CL9/Items) — 아이템 표의 `frying_pan` 행(Tool · Hand · 물리 없음 · `ActorClass` =
네이티브 클래스)이 이 액터로 스폰된다. 위 "구현 구조"의 열원 코드 그대로다:

- 팬 위 `HeatZone`(상판 위 60cm 상자)에 **놓인**(Free) 아이템을 호스트가 매 틱 질의해서 시간을 누적한다. 행에
  `CookResult`가 있으면 `CookTime` 뒤 그 행으로, 없고 `BurnResult`만 있으면 `BurnTime` 뒤 그 행으로 `SetAlias`
  (액터 교체 없음 — OnRep으로 클라 메시 갱신). 둘 다 없는 행(탄 것·도구)은 그대로 둔다.
- 팬에서 내려오면(집거나 굴러 나가면) 시계가 0으로 돌아간다. 손에 든 팬은 요리하지 않는다.
- `BurnWarnTime` 전 "연기" 로그만 있다 — **연기 VFX·소리는 미구현** (표현 계층, 에셋 없음).
- 팬 상판은 별도 `Bed` 상자 콜리전이다 — 아이템의 자체 콜라이더가 구(球)라 그냥 두면 재료가 굴러 떨어진다.
  팬은 데이터를 읽은 뒤 그 구를 팬 두께(≈8cm)로 줄인다(2026-09-06) — 반지름 30 구를 그대로 두면 납작한 메시 위에
  보이지 않는 둥근 껍질이 남아 얹힌 재료가 굴러 떨어졌다.
- **불(버너)은 아직 없다** — 팬은 놓인 자리에서 늘 뜨겁다. 트럭 주방(4f)이 생기면 "팬을 불에 올려야 익는다"로
  좁힌다. 동시 요리 수 = 팬 수는 그대로 성립한다.

조작은 운반 그대로: 계란을 들고 팬 앞에서 놓기(E)를 누르면 팬 위로 떨어진다. 검증 치트 없음 —
`cl9.additem frying_pan` + `cl9.additem egg`로 만든다 (testing/pie-scenarios.md M절).

**미결 — 손 높이 낙하가 계란을 깨뜨린다.** 놓기는 손 높이(`CarryHeight` 120)에서 떨어뜨리는데 바닥의 팬까지 ~90cm 낙하면
충돌 속도 ≈ 590 cm/s > `CrackSpeed` 450이라 `egg+cracked`가 되고, 금 간 행에는 `CookResult`가 없어 익지 않는다
(2026-09-06 PIE 실측: 45cm 낙하도 금이 감). 후보: ① 금 간 계란도 익게 표에 `CookResult` 추가, ② 놓기를 낙하 없이
바닥에 안착, ③ 4f 트럭 주방에서 팬을 손 높이 조리대에 둠. **임시 조치(2026-09-06)**: `Lvl_Flat` 트럭 패드 남쪽에 임시 부엌
(회색 조리대 300×120×60 + 버너 자리)을 두고 팬을 그 위에 올렸다 — 손 높이(120)에서 팬 상판(≈79)까지 20cm 낙하라
금이 가지 않는다. 정식 결정은 4f 트럭 주방과 함께.
