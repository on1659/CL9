# 주문서 시스템

> 오늘 만들어 팔아야 하는 것. **이 루프의 엔진이다** — 선택적 보너스가 아니다.
> **설계 정본은 plan이다** — [../plan/data/order.md](../plan/data/order.md) ·
> [../plan/loop.md](../plan/loop.md).

## 개요

- **주문이 필요한 재료를 정하고, 재료가 목적지를 정한다.** 쿼터는 무게가 아니라 레시피다.
- **하루에 N개를 뽑고 그 앞 M개가 자동으로 확정된다** (초안 3 뽑기 2 확정 —
  [../plan/data/progression.md](../plan/data/progression.md)). **고르는 쪽은 시스템이다** — 2026-10-06 사용자:
  "호스트가 고르게 둔 건 바꾸기 귀찮아서였다". 플레이어가 고르는 행위는 없어졌다(옛 "고르는 행위가 그날의 계획이다"는 폐기).
- **주문은 완성물을 요구한다** (`egg+cooked` 같은) — 생재료 주문 없음 (현재 설계).
- **마감과 위약금이 있다.** "따르지 않아도 되는" 주문이 아니다.

## 데이터 — `UCL9OrderDef`

컬럼 정본은 [../plan/data/order.md](../plan/data/order.md). 핵심:

| 컬럼 | 뜻 |
|---|---|
| `Ingredients` | `TArray<FCL9ItemCount>` `{ ItemAlias, ItemSubAlias, Count }` — 필요한 완성물 |
| `RewardMin` / `RewardMax` | 보상 (이더, int) — **수락할 때** 굴려 확정한다 |
| `DeadlineDays` | 며칠 안에 (1 = 오늘) |
| `FailPenaltyRate` | 마감을 놓치면 보상의 몇 배를 위약금으로 (초안 0.3) |
| `MinDay` / `MaxDay` / `Weight` | 등장 일차 범위 · 뽑힐 가중치 |
| `Gimmicks` | Instanced `UCL9Gimmick` 파생(예시는 메시를 돌리는 `UCL9Gimmick_Spin`, `CL9Gimmicks.h`) — 이 주문에만 있는 조건 |

고정 "보너스 배율", 주문별 "TimeLimit" 컬럼은 없다 — 시간 압박은 `DeadlineDays`다 (현재 설계).

## 주문 흐름

```
레벨 시작(첫 판) · 로비 진입 · 불러오기 · cl9.day — 전부 ACL9GameMode::OfferOrders 한 곳:
  후보 추첨 — MinDay/MaxDay 필터 → Weight 가중 추첨(중복 없음) → N개 제시
  ↓ 같은 호출 끝에서 제시가 비어 있지 않으면 앞 M개(추첨 순서 0..M-1)를 AcceptOrders — 플레이어가 고르는 단계 없음 (2026-10-06)
  ↓ 확정하는 순간 RewardMin~RewardMax를 굴려 확정한다 (2026-09-06 결정)
원정 → 요리 → 납품: 완성물을 창구에 올리면 주문과 대조
  ↓
충족: 확정 보상을 GameState.SharedMoney·PeriodIncome에 가산
  ↓
마감(DeadlineDays) 경과: 미납 주문은 확정 보상 × FailPenaltyRate를 누적 이더에서 차감
```

- **추첨·납품·위약금 판정은 GameMode(호스트) 전용이다.**
- 진행 중 주문은 판의 진행 상태다 — GameState에 Replicated, 세이브에 들어간다
  ([session.md](session.md)).

## 납품 판정

> **2026-10-13 — 판 레벨의 창구는 트럭 옆면 바깥에 있다**: `ACL9GameMode::SpawnTruckCounter` 가 StartPlay 에서 트럭 옆에 `BP_CL9Counter` 를 세운다(셀 `CL9Rules.TruckCounterClass`·`TruckCounterOffset`). 그 전에는 창구가 Lvl_Flat 에만 있어 판 레벨에서 납품할 곳이 없었다([../goal/truck-counter.md](../goal/truck-counter.md)).

```cpp
// 호스트 전용 — 창구에 올린 실물과 주문 대조
if (!HasAuthority()) return;
for (const FCL9ItemCount& Need : Order->Ingredients) {
  // 창구의 아이템 중 Alias+SubAlias가 일치하고 깨지지/타지 않은 것을 센다
}
// 전부 충족 → 수락 때 굴려둔 확정 보상을 지급, Multicast로 표현
```

같은 완성물을 여러 주문에 이중으로 세지 않는다.

## UI

- 주문서 선택 화면: N개 카드 중 M개 선택 (로비/정차 — `WBP_Main` 하위, [ui_system.md](ui_system.md)).
  **2026-10-06부터 열리지 않는다** — 제시가 뽑힌 자리에서 바로 확정돼 비기 때문이다. 화면·HUD 파생·레버의 `OrdersPending` 가지는
  손대지 않고 남겼다
- 진행 중 주문: HUD 좌하단에 한 줄씩 — 재료마다 "창구에 올린 수/필요 수"와 마감 일수
  ([ui.md](ui.md)). 올린 수는 창구 재고에서만 센다. 배정은 두 단계다: ① 아래 납품 판정과
  똑같이 마감이 급한 주문부터 돌며 **재료를 전부 채울 수 있는 주문만** 통째로 가져가고,
  ② 그러고도 남은 재고를 ①에서 못 채운 주문들에 다시 마감 순으로 `min(필요, 남은 수)`씩
  나눠 진행도로 보여준다(②는 미리보기일 뿐 정산에는 없는 단계다)
- 납품 결과: 창구 연출 + 이더 증가 표시

## 구현 상태 (4c 2026-09-05 · 4e 2026-09-06)

- **데이터**: `UCL9OrderDef`(Source/CL9/Data) — Ingredients·RewardMin/Max·DeadlineDays·FailPenaltyRate·
  MinDay/MaxDay/Weight (Gimmicks는 베이스). 에셋 `OrderTable`(Content/Data) 행 omelette·fried_egg·
  meat_dish. 로드 검사가 재료 alias+sub가 아이템 행인지 확인한다. N/M은 `CL9Progression`의
  `OrdersOfferedPerDay`(3)·`OrdersRequiredPerDay`(2).
- **추첨**: `ACL9GameMode::OfferOrders` — 게임 시작(StartPlay)·로비 진입·`cl9.day`·불러오기 때 호스트가
  MinDay/MaxDay로 거른 뒤 Weight 가중 무작위(중복 없음)로 N개. 결과는 `GameState.OfferedOrders`
  (alias 배열)·`OrdersRequired`(M)로 복제.
- **자동 확정 (2026-10-06, round-start-truck-intro K1)**: `OfferOrders` 의 **맨 끝**에서 제시가 비어 있지 않으면
  `AcceptOrders({0 … Required−1})` 를 부른다. 제시가 비었으면(그 일차에 나갈 주문 행이 없다) 부르지 않으므로 경고도 남지 않는다.
  확정의 입구가 이 한 곳이라 StartPlay·EnterLobby·LoadRun·`cl9.day` 가 모두 같은 길을 탄다. 설정 열은 없다(`bAutoStartFirstRound`
  같은 것은 반려 — 아무도 요청하지 않은 설정성). **받아들인 부수 효과**:
  `cl9.day` 와 `cl9.load` 는 이미 확정된 주문 위에 확정 묶음을 하나 더 쌓는다. 첫 판 시작을 기다리는 중에 `cl9.load` 를 치면
  그 시작이 취소된다(호스트는 레버를 쓴다).
- **확정**: `ACL9PlayerController::AcceptOrders(indices)`(BlueprintCallable → Server RPC) — 호스트의
  컨트롤러만 확정한다(클라 요청은 거부·로그). `ACL9GameMode::AcceptOrders`가 로비에서만, 정확히 M개
  서로 다른 인덱스일 때 받아 `GameState.AcceptedOrders{OrderAlias, AcceptedDay, DeadlineDay = 받은 날 +
  DeadlineDays − 1}`에 누적하고 제시를 비운다. 변경은 `OnOrdersChanged`(BlueprintAssignable)로 알린다.
  **이제 사람이 부르는 길은 `cl9.pick` 뿐이고, 제시가 이미 비어 있으므로 늘 "nothing is on offer" 경고로 거부된다.** 호스트에서는
  제시를 싣는 호출과 비우는 호출이 한 호출 안에서 잇따라 `OnOrdersChanged` 를 쏘므로, 주문 화면이 한 프레임 열렸다 닫힐 수
  있다 — PIE 에서 깜빡임·커서 잡힘이 없는지 본다 ([../testing/pie-scenarios.md](../testing/pie-scenarios.md) K1).
- **보상 확정(4e)**: `AcceptOrders`가 수락하는 순간 `[RewardMin, RewardMax]`를 굴려
  `FCL9AcceptedOrder.Reward`에 넣는다 — 카드와 HUD가 범위 대신 이 값을 보여주고, 위약금도 여기서 나온다.
- **창구(4e)**: `ACL9Counter`(Source/CL9/Items, BP `Content/Items/BP_CL9Counter`) — 손에 든 것과
  끄는 수레의 짐을 받아 보관한다(실물은 파괴하고 alias+sub+굴림값만 복제해 들고 있는다). 임시 부엌 옆에
  놓인 상자이며 4f의 정식 주방·트럭이 이 자리를 대체한다.
- **납품 판정(4e)**: `ACL9GameMode::SettleOrdersAt` — 호스트 전용. 마감이 급한 주문부터 창구 보관분과
  대조하고, 재료가 전부 있을 때만 그 자리에서 빼내며(반쯤 낸 주문 없음, 이중 계산 없음) 확정 보상을
  `GameState.AddIncome`으로 지급한다.
- **위약금(4e)**: `ACL9GameMode::ExpireOrders` — 로비에서 날짜가 오른 직후, `DeadlineDay < CurrentDay`인
  주문을 지우고 `Reward × FailPenaltyRate`를 `GameState.Deduct`로 뺀다(잔액 하한 0 — 채무 없음).
- **주문 선택 화면**: K3(2026-09-06) — 위 "UI" 절 참조. **HUD 진행 중 주문**: 2026-09-15 —
  `UCL9HUDWidget::RefreshOrders`가 창구 재고를 정산과 같은 순서(마감 오름차순)로 배정해
  재료마다 "올린 수/필요 수"를 쓴다.
- **세이브**: 진행 중 주문은 판 세이브에 들어간다(`ACL9GameMode::SaveRun`/`RestoreRun` — [save.md](save.md)).
- `StartNight`는 주문 수를 보지 않는다 — 그날 나갈 주문 행이 하나도 없으면(제시가 빔) 확정 없이, 경고도 없이 판이 열린다.
  (옛 "주문을 안 골라도 밤은 시작된다"는 개발 편의였다 — 이제 고르는 일 자체가 없다.) 레버의 `OrdersPending` 가지
  (제시가 남아 있으면 출발 대신 주문 화면)는 코드에 남아 있지만 제시가 곧바로 비므로 거치지 않는다([session.md](session.md)).
- 치트: `cl9.orders`(제시·수락 목록 + 확정 보상 + 자금), `cl9.pick <n> [m]`(번호로 확정 — 지금은 늘 거부되는 빈 제시),
  `cl9.money [양]`(자금 확인·지급·차감).

## 참고

- N/M 값과 곡선: [../plan/data/progression.md](../plan/data/progression.md)
- 보상 눈금·위약금 근거: [../plan/economy.md](../plan/economy.md)
- 주문 초안 목록 (`omelette` · `fried_egg` · `meat_dish`): [../plan/data/order.md](../plan/data/order.md)
