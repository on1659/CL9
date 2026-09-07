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

## 구현 상태 (④ 4f — 2026-09-07)

- **데이터**: `UCL9EquipmentDef`(Source/CL9/Data) — `Price` · `UnlockDay` · `MaxOwned` · `bPhysicalProp` ·
  `Slot`(Kitchen/Carryable) · `bUsableAsTool` · `ItemDef`(실물이 될 아이템 행). 이름·아이콘·전용 BP는 베이스 컬럼을
  쓴다. 로드 검사가 `ItemDef`가 실제 아이템 행인지 확인한다 — 끊어져 있으면 사도 실물이 안 나온다.
  에셋은 `DA_EquipmentTable`(Content/Data).
- **판정(호스트)**: `ACL9GameMode::CanBuy(alias)` → `ECL9BuyResult`(Ok/NoRow/**NotInLobby**/Locked/MaxOwned/
  NotEnoughMoney). `BuyEquipment(alias)`가 그 판정을 다시 돌린 뒤 `GameState.Deduct(Price)`하고
  `SpawnEquipmentAtTruck`으로 트럭 옆에 실물을 스폰한다. **스폰이 실패하면 이더를 빼지 않는다.**
- **보유 수**: `GetOwnedCount(alias)` — 월드에 나와 있는 실물을 센다. 세어 둔 숫자를 들고 있지 않는 이유는
  장비를 두고 오면 잃기 때문이다.
- **네트워크**: `ACL9PlayerController::BuyEquipment`(BlueprintCallable → Server RPC) — **클라이언트도 살 수 있다.**
  자금이 판 공유라 누가 눌러도 같은 지갑에서 나가고 산 물건은 전원에게 실물로 보인다(주문 확정이 호스트 전용인
  것과 다르다). 거절은 `Client_BuyRejected` → `OnBuyRejected`로 **누른 사람에게만** 간다. 성공은 `OnMoneyChanged`로 안다.
- **화면 여는 문**: `ACL9TruckPanel`(Source/CL9/Items) — 정차 중에만 상호작용이 열리고, 누른 사람의 컨트롤러로
  `Client_OpenScreen(ShopUIRow)`를 보낸다. **행 이름은 액터 프로퍼티**(기본값 `Shop`)라 C++에 상수로 박지 않았다.
- **원정과 분실**: 새벽(로비 진입)에 `LoseEquipmentLeftBehind()`가 트럭에서 `EquipmentKeepRadius`(기본 1200cm)
  밖에 있는 장비를 파괴한다. 손에 들거나 수레에 싣고 돌아온 것은 사람과 함께 트럭 곁에 있으므로 거리가 알아서 걸러낸다.
- **세이브**: `UCL9SaveGame.OwnedEquipment`(alias 배열, 버전 2) — 새벽에 살아남은 것만 적히고, 불러오면 트럭 옆에
  다시 스폰된다. 표에서 사라진 alias는 로그를 남기고 버린다.
- **미구현**: 상점 화면(`WBP_Shop` — UI 세션), 구매 연출(Multicast·소리), `Slot=Kitchen` 장비(주방 고정물),
  `bUsableAsTool`을 읽는 곳(밀치기는 지금 맨손이어도 된다).
- 치트: `cl9.buy`(인자 없으면 목록·가격·보유·구매 가능 여부 출력), `cl9.buy <alias|번호>`(구매).

## 참고

- 장비 컬럼·품목 초안: [../plan/data/equipment.md](../plan/data/equipment.md)
- 가격식·자금: [economy.md](economy.md)
- 패널 UI: [ui_system.md](ui_system.md)
