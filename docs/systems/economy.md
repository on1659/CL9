# 경제 시스템 (판매·구매·자금)

> 이더가 어디서 생기고 어디로 나가는지, 코드가 가격을 어떻게 계산하는지.
> **설계 정본은 plan이다** — [../plan/economy.md](../plan/economy.md).

## 화폐

- **단위는 이더. `int32` 정수다.** 소수점·float 화폐를 쓰지 않는다.
- 가격이 붙는 곳 전부 이더다 — 아이템 값 · 몬스터 값 · 주문 보상 · 목표액 · 장비 가격.
- 눈금은 계란이 잡는다 — 성한 계란(배율 1.0)이 80~120이더. 나머지는 여기서 역산한다.

## 자금은 판(호스트)의 것이다

```cpp
// ACL9GameState — 판 공유 자금 (플레이어별 지갑이 아니다)
UPROPERTY(Replicated)
int32 SharedMoney;      // 누적 자금 — 세이브에 들어간다

UPROPERTY(Replicated)
int32 PeriodIncome;     // 이번 정산 기간의 수입 — 정산 판정 대상
```

- **PlayerState에 Money를 두지 않는다.** 진행은 호스트의 것이고 판 하나가 세이브 하나다
  ([../plan/data/progression.md](../plan/data/progression.md)).
- **남는 이더는 다음 날로 넘어간다.** 라운드 리셋·초기 자금 지급이 없다.
- 채무 불가 — 잔액이 음수가 되는 거래는 거절한다.

## 이더의 흐름

**들어오는 곳**
- 주문 납품 보상 (`RewardMin`~`RewardMax` 굴림) — 재료 값 합의 2~3배(초안)로 잡아
  **주문이 항상 주 수입**이 되게 한다
- 주문 외 물건을 그냥 파는 것 — 아래 가격식 그대로

**나가는 곳**
- 장비 구매 (`Price`)
- 주문 위약금 — 마감을 놓치면 보상 × `FailPenaltyRate`를 누적 이더에서 뺀다
- 그 외는 없다. 수리비·유지비를 만들지 않는다

## 가격식

```
최종 가격 = [ValueMin, ValueMax] 굴림 × 목적지 배율(ValueRate) × 일자 배율(ValueCurve)
```

```cpp
// 호스트 전용 — 개체마다 굴린다 (같은 계란이라도 80이거나 120이다)
int32 ComputeSellPrice(const UCL9ItemDef* Def) {
  const int32 Base = FMath::RandRange(Def->ValueMin, Def->ValueMax);
  const float Mult = Site->ValueRate * Progression->ValueCurve->GetFloatValue(Day);
  return FMath::RoundToInt(Base * Mult);
}
```

- 고정 `SellPrice` 컬럼을 두지 않는다 — 값은 `ValueMin`/`ValueMax`(int)다
- 깨진 것·탄 것은 0이다 (`bWorthlessIfBroken` / `bWorthlessIfBurnt`)
- 배율 두 개를 동시에 만지지 않는다 — 한쪽을 평평하게 두고 조정한다

## 거래 흐름 (네트워크)

```
클라: 판매/구매 입력
  ↓ Server RPC (SellItem_Request / BuyItem_Request)
호스트: 판정 — 실물이 창구/트럭에 있는가, 잔액이 충분한가
  ↓
GameState.SharedMoney / PeriodIncome 갱신 (Replicated)
  ↓ Multicast RPC (OnItemSold / OnItemBought — 표현·사운드)
전 클라: UI 갱신
```

- 판정은 호스트 전용 (`if (!HasAuthority()) return;`)
- 거래 대상은 실물 아이템 액터다 — "인벤토리 슬롯에서 판매"가 아니라 창구에 올린 물건을 판다

## 거래 로그 (디버깅용)

```cpp
struct FTransaction {
  float Timestamp;
  ETransactionType Type;   // Sell / Buy / OrderReward / Penalty
  FName ItemAlias;
  int32 Quantity;
  int32 Amount;            // 이더 (int)
};
```

## 참고

- 주문 보상·위약금: [order.md](order.md)
- 정산 판정 (기간 수입): [balance.md](balance.md)
- 상점 UI·트럭 패널: [shop.md](shop.md)
- 값의 근거와 부등식: [../plan/economy.md](../plan/economy.md)
