# 주문서 시스템

> 오늘 만들어 팔아야 하는 것. **이 루프의 엔진이다** — 선택적 보너스가 아니다.
> **설계 정본은 plan이다** — [../plan/data/order.md](../plan/data/order.md) ·
> [../plan/loop.md](../plan/loop.md).

## 개요

- **주문이 필요한 재료를 정하고, 재료가 목적지를 정한다.** 쿼터는 무게가 아니라 레시피다.
- **하루에 N개를 제시하고 M개를 고른다** (초안 3 제시 2 선택 —
  [../plan/data/progression.md](../plan/data/progression.md)). 고르는 행위가 그날의 계획이다.
- **주문은 완성물을 요구한다** (`egg+cooked` 같은) — 생재료 주문 없음 (현재 설계).
- **마감과 위약금이 있다.** "따르지 않아도 되는" 주문이 아니다.

## 데이터 — `UCL9OrderDef`

컬럼 정본은 [../plan/data/order.md](../plan/data/order.md). 핵심:

| 컬럼 | 뜻 |
|---|---|
| `Ingredients` | `TArray<FCL9ItemCount>` `{ ItemAlias, ItemSubAlias, Count }` — 필요한 완성물 |
| `RewardMin` / `RewardMax` | 보상 (이더, int) — 납품 시 굴림 |
| `DeadlineDays` | 며칠 안에 (1 = 오늘) |
| `FailPenaltyRate` | 마감을 놓치면 보상의 몇 배를 위약금으로 (초안 0.3) |
| `MinDay` / `MaxDay` / `Weight` | 등장 일차 범위 · 뽑힐 가중치 |
| `Gimmicks` | Instanced — 이 주문에만 있는 조건 |

고정 "보너스 배율", 주문별 "TimeLimit" 컬럼은 없다 — 시간 압박은 `DeadlineDays`다 (현재 설계).

## 주문 흐름

```
정차(로비): 후보 추첨 — MinDay/MaxDay 필터 → Weight 가중 추첨 → N개 제시
  ↓
플레이어가 M개 선택 (호스트가 확정, 전원에게 표시)
  ↓
원정 → 요리 → 납품: 완성물을 창구에 올리면 주문과 대조
  ↓
충족: RewardMin~RewardMax 굴림 → GameState.SharedMoney·PeriodIncome에 가산
  ↓
마감(DeadlineDays) 경과: 미납 주문은 보상 × FailPenaltyRate를 누적 이더에서 차감
```

- **추첨·납품·위약금 판정은 GameMode(호스트) 전용이다.**
- 진행 중 주문은 판의 진행 상태다 — GameState에 Replicated, 세이브에 들어간다
  ([session.md](session.md)).

## 납품 판정

```cpp
// 호스트 전용 — 창구에 올린 실물과 주문 대조
if (!HasAuthority()) return;
for (const FCL9ItemCount& Need : Order->Ingredients) {
  // 창구의 아이템 중 Alias+SubAlias가 일치하고 깨지지/타지 않은 것을 센다
}
// 전부 충족 → 보상 굴림 → 지급, Multicast로 표현
```

같은 완성물을 여러 주문에 이중으로 세지 않는다.

## UI

- 주문서 선택 화면: N개 카드 중 M개 선택 (로비/정차 — `WBP_Main` 하위,
  [ui_system.md](ui_system.md))
- 진행 중 주문: HUD에 남은 재료·마감 일수 표시 ([ui.md](ui.md))
- 납품 결과: 창구 연출 + 이더 증가 표시

## 참고

- N/M 값과 곡선: [../plan/data/progression.md](../plan/data/progression.md)
- 보상 눈금·위약금 근거: [../plan/economy.md](../plan/economy.md)
- 주문 초안 목록 (`omelette` · `fried_egg` · `meat_dish`): [../plan/data/order.md](../plan/data/order.md)
