# 상점 시스템 (트럭 패널)

> 푸드트럭에서 번 이더로 장비를 사고, 주문 외 물건을 판다.
> **설계 정본은 plan이다** — [../plan/data/equipment.md](../plan/data/equipment.md) ·
> [../plan/economy.md](../plan/economy.md).

## 개요

- **위치**: 푸드트럭의 패널. 별도 상인 NPC가 없다.
- **파는 것**: `bPhysicalProp` = true인 **실물 장비**를 판다 — 착용형·스탯형 장비는 현재
  범위에 없다 ([../plan/data/equipment.md](../plan/data/equipment.md)).
- **사는 것**: 구매한 장비는 인벤토리 항목이 아니라 **월드에 실물로 나온다** — 들고 다니고,
  원정에 두고 오면 잃는다.
- **판매**: 주문 외 물건을 그냥 파는 창구 — 가격은 굴림×배율 그대로 ([economy.md](economy.md)).

## 트럭 패널

패널 항목은 장비 구매 목록이다 — 품목 기준은 `bPhysicalProp` 컬럼이 지킨다.
(시동 같은 트럭 상호작용을 패널에 함께 둘지는 미정 — plan/data/equipment.md)

```
┌────────────────────────────┐
│  트럭 패널                  │
│  보유: 1,240 이더           │
├────────────────────────────┤
│  [1] 프라이팬   200 이더    │
│  [2] 수레       ? 이더      │
│  [3] 냄비       ? 이더      │
│  [4] 스탠드 램프 ? 이더     │
│  [C] 닫기                   │
└────────────────────────────┘
```

- `UnlockDay`가 안 된 품목은 잠금 표시
- `MaxOwned` 초과 품목은 구매 불가
- 잔액 부족 시 구매 거절 (채무 불가)

## 구매 흐름

```
1. 트럭 패널 상호작용 → 품목 선택
2. Server RPC: BuyEquipment_Request(EquipAlias)
3. 호스트 판정: 잔액·UnlockDay·MaxOwned
4. 성공: GameState.SharedMoney 차감 → 장비 액터를 트럭 옆에 스폰 (SetAlias)
5. Multicast: 구매 연출 (소리·표시)
```

**구매 = 실물 스폰이다.** "인벤토리에 추가"가 아니다. 프라이팬을 들면 계란과 같은 운반
규칙을 탄다 (`ItemDef` 연결 — equipment.md).

## 판매 흐름 (창구)

```
1. 팔 물건을 창구에 올린다 (운반 조작 그대로 — 놓기)
2. Server RPC: SellAtCounter_Request
3. 호스트 판정: 창구 위 실물 각각에 가격 굴림 (깨짐·탐은 0)
4. GameState.SharedMoney·PeriodIncome 가산
5. Multicast: 판매 연출
```

주문 납품도 같은 창구를 쓴다 — 주문과 일치하면 납품 보상, 아니면 일반 판매
([order.md](order.md)).

## 현재 범위에 없는 것 (이전 판과의 차이)

- **장비 착용 슬롯·장비 효과 탭** — 스탯 장비가 없다
- **"모두 판매" 버튼** — 판매 대상은 창구에 올린 실물이다
- **아이템 되사기 재고** — 상점은 장비 목록(`UCL9EquipmentDef`)만 판다

## 참고

- 장비 컬럼·품목 초안: [../plan/data/equipment.md](../plan/data/equipment.md)
- 가격식·자금: [economy.md](economy.md)
- 패널 UI: [ui_system.md](ui_system.md)
