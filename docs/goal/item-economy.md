# goal: item-economy

**Status**: 명세 (2026-10-19, 구현 전). 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름.
기획 정본: `C:\Work\CL9\docs\plan\draft\item-economy.md`(문지기 3회째 통과, 6/6 — `C:\Work\CL9\.claude\runs\item-recal\gate-기획.md`). 이 명세는 그 초안의 절 번호를 "초안 1.4 T1"처럼 가리킨다.
트랙: **HEAVY**(경제·재화 구조와 새 표 열 다수 — 롤백 비용이 크다). 다관점 패널은 따로 돌리지 않았다 — 초안이 몬스터·조작·UI·메타와 통합 넷(`C:\Work\CL9\docs\plan\draft\_integration-2.md`·`_integration-4.md` 등)을 거치며 이미 다관점 반박을 받았고, 그 결과를 아래 Design Decisions 에 옮겼다.

## One-line Goal

몬스터 공략 도구 다섯(무전기·딱총·깡통 딸랑이·포대·갈고리 장대, 단계 2에 훈연기)과 닭 처치 드롭(닭고기)·미끼 규칙·새 재료와 주문을 **표 열과 행으로** 넣고, 코인과 이더의 흐름을 초안 5절대로 맞추되 모든 튜닝 수치는 표에 두어 빌드 없이 바꿀 수 있게 한다.

## Background / Motivation

- 처치하면 아무것도 떨어지지 않는다 — 어느 종의 `MonsterTable` · `DropItems`에도 연결이 없다(초안 4.1). 닭이 "치는 것이 정답일 수 있는 유일한 종"인데 정답이 되는 판이 방어 말고 없다.
- 상점에 있는 장비는 프라이팬·냄비·손전등·헤드램프·수레·스탠드 램프 여섯뿐이고 가격이 정해진 것은 프라이팬 `Price` 25 하나다(초안 2.1).
- 몬스터 종 카드 23종이 준비물로 적은 도구가 없다. 결정 #32(2026-10-18)가 새 도구의 범위를 아이템 기획에 맡겼고, 초안 1.7이 먼저·다음·나중·보류로 줄을 세웠다(초안 7.3 B1).
- 도구의 원칙(초안 0절 1~3): 도구는 몬스터를 지우는 것이 아니라 시간·길·사람을 사는 것이다. 지우는 도구는 프라이팬 하나로 두고 늘리지 않는다. 한 도구가 모든 종에 듣지 않는다.
- 결정 #1(준비 책임 — 배터리 없음), #17(장소 고유는 재료와 주인), #19(다섯 집), #9→통합 B8(닭 드롭 = 새 닭고기 행), #41(짐 결과는 발밑), #42(빈 것 값은 같은 행의 별도 열), 2026-10-12 루프 개편 7i(장비 구매는 회사로, 트럭은 판매 창구만)를 전제로 한다.
- 초안이 나온 뒤 결정이 넷 더 났다: #45(쥐 = 쥐 떼 몰이 — 쥐덫은 보류 유지), #47(몰래 역할 임무마다 5이더, 값은 표에서), #48(첫 제작 묶음 = 필수 몬스터 10종 + 맵 하나 = 양계장, 재미 확인용), #58(무전기는 켜져 있으면 어디 있든 보내고 받는다). 이 명세는 그 넷을 반영했다.

## In-scope

범위를 둘로 나눈다. **단계 1**은 결정 #48의 첫 묶음(양계장 + 10종: 닭·어미 닭·횃대 닭·돼지·거위·너구리·양·토끼·쥐·소)에서 쓰이는 것이다. **단계 2**는 다른 집(외양간·찬장·벌집)이 서야 쓰이는 것이다. 단계 2를 이 goal 에 남길지는 Open Questions 4번이다.

### 단계 1

1. **`ItemTable` 새 열(아이템 쪽)** — `UCL9ItemDef`(`C:\Work\CL9\Source\CL9\Data\CL9ItemDef.h`)에 더한다. 비면 지금 동작과 같아야 한다.
   - 딱총: `FuseTime`(초), `StunRadius`(cm), `StunTime`(초), `BurstNoiseRadius`(cm).
   - 깡통 딸랑이: `EmitNoiseRadius`(cm), `EmitNoiseInterval`(초), `EmitNoiseDuration`(초).
   - 갈고리 장대: `RescueReach`(cm).
   - 미끼: `FoodTag`(enum `None`·`Raw`·`Cooked`·`Burnt`·`Grain` — `None`이면 먹을 것이 아니다), `EatTime`(초 — 0이면 `CL9Rules` · `DecoyEatTime`을 쓴다).
   - 판매: `bSellable`(bool — false 면 트럭 판매 창구에서 받지 않는다. 병아리·꾸러미).
2. **`CL9Rules` 새 열** — `UCL9RulesDef`(`C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h`): `DropBudgetPerRound`(개, 0 = 상한 없음), `RadioClearRange`(cm), `RadioMaxRange`(cm), `RadioStaticMax`(비율), `RadioFloorPenalty`(배율, 0 = 층 무시), `ItemLandNoiseRadius`(cm), `CartNoiseRadius`(cm).
3. **`CL9Progression` · `DayOverrides` 새 칸** `StartItems`(그 일차 판 시작에 지급하는 아이템과 수 — "사람 수만큼"을 나타낼 수 있어야 한다). 1일차 행에 손전등 사람 수만큼(초안 7.3 B8 — 추천대로 진행).
4. **`EquipmentTable` 행** — `UCL9EquipmentDef`:
   - 새 행 다섯: `walkie`(무전기), `firecracker`(딱총, 1회용), `rattle_can`(깡통 딸랑이), `burlap_sack`(포대), `hook_pole`(갈고리 장대). 각 행은 `ItemDef`로 같은 별칭의 `ItemTable` 행을 가리킨다.
   - 냄비 `pot`은 회사 상점 목록에서 뺀다(초안 7.3 B5 — 추천대로 진행). 아이템 행은 남긴다.
   - 프라이팬 `frying_pan`·손전등·헤드램프·수레·스탠드 램프의 `Price`·`UnlockDay`·`MaxOwned`는 아래 Tuning Values 의 초안 값과 밸런스 몫으로 채운다.
5. **`ItemTable` 행** — 도구 다섯의 아이템 행, 닭고기 `chicken_meat`(날·`cooked`·`burnt` 세 행, `CookResult`·`BurnResult` 체인, 탄 행 `bWorthlessIfBurnt` true), 병아리 `chick`(값 0, `bSellable` false, 퀵슬롯 한 칸 — 몬스터 쪽 값은 몬스터 명세가 가리킨다).
6. **닭 처치 드롭** — `MonsterTable`(닭) · `DropItems`에 `chicken_meat` 한 항목. 확률은 **이미 있는 필드** `FDropItem::DropChance`(`C:\Work\CL9\Source\CL9\Data\CL9Types.h` 539줄)를 쓴다 — 초안 9.3이 "`MonsterTable` 새 열 `DropChance`"로 적었지만 드롭 항목 구조체에 이미 있다. 한 판 상한 `CL9Rules` · `DropBudgetPerRound`는 호스트가 판마다 센다(모든 종의 드롭 합). 빈 닭의 드롭은 결정 #42대로 같은 행의 빈 것 열을 비운다 — 그 열 이름은 몬스터 명세 몫이다(초안 9.2 M5).
7. **미끼 규칙을 아이템 행 · 종 행의 짝으로 바꾼다**(통합 B16) — 먹는 시간은 아이템 행 `EatTime`(0이면 `DecoyEatTime`), 먹는가는 종 행 `EatsTags`와 아이템 행 `FoodTag`의 일치. 지금 탄 요리 미끼(`C:\Work\CL9\docs\goal\burnt-bait.md`)의 동작은 탄 행 `FoodTag` `Burnt` + 닭 `EatsTags` `Burnt`로 그대로 재현되어야 한다. 종 쪽 열(`EatsTags`·`bBaitDuringChase`)은 몬스터 명세가 만든다 — 의존.
8. **새 부등식 E1 로드 검사**(초안 7.3 B11 — 추천대로 진행): `EquipmentTable` 행이 가리키는 `ItemTable` 행의 `ValueMax`가 그 행의 `Price` 이상이면 `UCL9EquipmentDef::Validate`가 오류를 낸다(사서 바로 팔면 코인이 무한히 생기는 구멍). `C:\Work\CL9\docs\plan\economy.md` "지켜야 하는 부등식" 표에 한 줄 더한다.
9. **도구 동작**(각 행의 열만 읽는다 — 수치 상수 없음):
   - 딱총: 손을 떠난 순간 심지가 붙고(통합4 B7 — 조작 몫, 추천대로 진행된 것을 따른다) `FuseTime` 뒤 터진다. `StunRadius` 안의 몬스터에 `StunTime` × 종 `StunMult` 동안 기절을 건다(종 열은 몬스터 명세). `BurstNoiseRadius`의 `Burst` 소음 하나를 낸다. 1회용 — 터지면 아이템이 사라진다.
   - 깡통 딸랑이: 놓거나 던진 뒤 `EmitNoiseDuration` 동안 `EmitNoiseInterval`마다 `EmitNoiseRadius`의 `Rattle` 소음을 낸다. 다시 주우면 멈추고, 다시 쓸 수 있다.
   - 포대: 잠든 거위에 던져 맞으면 거위가 "덮임" 상태가 된다(종 쪽 `bCanBeHooded`·`HoodedRunHearMult`와 깨는 규칙은 몬스터 명세). 빗나가면 바닥에 떨어진다.
   - 갈고리 장대: 든 사람의 구출 입력 반경이 `PinRescueRadius` 대신 `RescueReach`가 된다. 손으로 푸는 붙잡힘(양 묶임·횃대 닭 붙음 — 단계 2에 허수아비 꽂힘·빈 다람쥐 갇힘)만 푼다. 쳐서 푸는 붙잡힘(`RescueHits` — 여우·플레이어 탈)과 거미줄은 풀지 않는다(초안 7.3 B2). 동료를 끌어당기지 않는다.
   - 무전기: 퀵슬롯 칸의 스위치(손전등 `bLightOn`과 같은 방식)를 켜 두면 손에 들었든 주머니에 있든 보내고 받는다(결정 #58). 두 스피커 거리가 `RadioClearRange` 안이면 깨끗하고, `RadioClearRange`~`RadioMaxRange` 사이에서 잡음 섞임이 0에서 `RadioStaticMax`까지 선형으로 오르고, `RadioMaxRange` 밖이면 끊긴다. 다른 층이면 거리를 (1 + `RadioFloorPenalty`)배로 센다. 연결 규칙은 지금 핫라인(두 무전기의 양방향 연결, 한 사람은 핫라인 하나 — `C:\Work\CL9\docs\plan\voice.md` 2절)을 그대로 쓴다 — Open Questions 2번이 정해지기 전에는 바꾸지 않는다.
10. **주문** — `OrderTable` 새 행 `roast_chicken`(구운 닭고기 ×2, 마감 1). 보상 `RewardMin`~`RewardMax`는 식(구운 닭고기 `ValueMin`·`ValueMax` × 2 × 한 집 배율, 5 단위)으로 밸런스가 채운다. `honey_butter` 행은 `Weight` 0으로 둔다(초안 7.3 B9 — 버터는 다음 동네까지 미룬다).
11. **문서**: `C:\Work\CL9\docs\plan\economy.md`(5절 흐름의 바뀌는 곳 — 판매 전용 물건·처치 드롭·1회용 도구 지출·몰래 역할 임무 보상 줄, 부등식 E1), `C:\Work\CL9\docs\plan\data\equipment.md`, `C:\Work\CL9\docs\plan\data\item\`(새 행), `C:\Work\CL9\docs\plan\data\order.md`, `C:\Work\CL9\docs\plan\data\rules.md`(새 셀), `C:\Work\CL9\docs\plan\data\progression.md`(`StartItems`), `C:\Work\CL9\docs\systems\item.md`·`drop.md`·`shop.md`·`economy.md`, `C:\Work\CL9\docs\decisions.md`.

### 단계 2 (Open Questions 4번에 달렸다)

12. `ItemTable` 새 열 `PickupNoiseRadius`(cm — 0이면 지금 줍기 소음 규칙), `PickHoldTime`(초 — 0이면 바로 집힌다, 초안 7.3 B13 — 꿀 단지 행에 둔다), `CalmRadius`(cm), `CalmRageMult`(배율).
13. 새 재료 행과 체인: 우유 단지 `milk_jug`·견과 `nut`·말린 과일 `dried_fruit`·꿀 단지 `honey_pot`(각 날·`cooked`·`burnt`), 잼병 `jam_jar`(익지 않음, 계란 행의 깨짐 세 문턱 값과 같은 `CrackSpeed`·`BreakSpeed`·`CrackedBreakSpeed`를 쓴다, 주문에 넣지 않는다), 꾸러미 `bundle`(`HoldPose` Large, 값 0, `bSellable` false, 판에 하나 — 통합 B12). 새 집 셋의 재료는 그 집과 몽장 사이트 행의 `MapTable` · `ItemSpawns`에만 둔다(초안 3.1). 밀랍은 넣지 않는다(초안 7.3 B10).
14. 훈연기 `smoker` 장비·아이템 행: 켜고 든 동안 `CalmRadius` 안의 벌이 쫓지 않고, 벌통 광폭화 사건이 `CalmRageMult`만큼만 찬다. 시간은 줄이지 않는다.
15. 주문 새 행: `warm_milk`·`roasted_nuts`·`fruit_compote`·`honey_glaze`(한 집), `custard`·`honey_nuts`(섞인 주문).
16. 갈고리 장대의 대상에 허수아비 꽂힘·빈 다람쥐 갇힘을 더한다(종이 들어올 때).

## Out-of-scope

- **피해를 주는 새 도구, 몬스터를 판 끝까지 묶는 도구, 뒤를 보여 주는 도구, 몬스터 위치 탐지기, 회복·되살리기 도구, 칸을 늘리는 가방**(초안 1.6·6.5).
- **배터리·충전 품목**(결정 #1).
- **조명탄 `flare`** — 2026-09-11 결정 "빛은 손전등(·헤드램프)에서만 나온다 — 발광 아이템 폐기"(`C:\Work\CL9\docs\decisions.md` 939줄)를 유지하는 쪽이 기본이다. 넣을지는 Open Questions 1번.
- **모이 자루 `grain_sack`**(나중, 초안 7.3 B4), **쥐덫 `mousetrap`**(보류 — 결정 #45 쥐 떼 몰이에도 쥐덫이 붙을 자리가 없다), **문 열쇠 `door_key`**(보류 — 결정 #33, 여는 조건은 판 루프·맵 몫).
- **`honey_butter`의 버터 재료**(다음 동네까지 미룬다).
- **드는 방식과 입력** — 무전기 송신 키, 딱총 점화, 깡통 딸랑이를 끄는 법, 포대 조준 판정, 훈연기 토글, 갈고리 장대·우유 단지·꾸러미의 두 손 여부, 꿀 단지를 누르는 동안의 이동 잠금과 손을 뗄 때, 헤드램프 벗기(초안 8.1 — 조작 기획자 몫).
- **화면 표시** — 상점 패널·켜진 도구 점·무전기 신호 세기·꿀 단지 진행·0값 아이템 표시·새벽 이더 줄·옷장 잠금 이유·드롭 확률 숨김(초안 8.2 — UI 기획자 몫).
- **종 쪽 열** — `StunMult`·`EatsTags`·`bBaitDuringChase`·`bCanBeHooded`·`HoodedRunHearMult`·`NoiseReaction`·`HearMask`의 `Burst`·`Rattle`·빈 것 드롭 열·`PinRescueRadius`·`PinRescueTime`·`NestRadius`(몬스터 명세 몫). 이 명세는 그 열을 읽기만 한다.
- **칭호가 스킨을 여는 연결 열**(`SkinTable` · `UnlockTitle` 또는 칭호 표 쪽) — 방향(칭호는 스킨을 "살 수 있게" 연다, 억울한 희생 칭호에는 걸지 않는다 — 초안 7.3 B12)은 진행했지만 열의 자리는 메타 몫.
- **몰래 역할 임무 보상 `CL9Rules` · `ImposterMissionReward`의 구현**(결정 #47 — 메타 명세 몫. 이 명세는 흐름 문서에 줄만 적는다), **몰래 역할 성공 보상을 "생존 보상 + 덤"으로 바꾸는 안**(번복이 드는 안 — 메타 몫, 초안 5.3).
- **해금 일차의 뜻**(회사는 `CL9Progression` · `SettlementIntervalDays` 3판마다라 `UnlockDay`가 회사 날 사이에 놓일 때), **죽을 때 머리 칸**, 목적지가 열리는 날, 주문의 `MinDay`·`MaxDay`·`Weight`, 판 수입의 크기(판 루프 몫).
- **제작량 합계의 꼴**(Open Questions 3번), 모델·소리·아이콘 제작(아트·시퀀스·사운드 몫).
- **`MonsterTable` · `ValueMin/Max`(시체 판매)** — 쓰지 않는다. 처치 보상은 `DropItems`만.

## Design Decisions (기획 단계에서 정해진 것 — 구현 중에 다시 따지지 않는다)

- **지우는 도구는 프라이팬 하나** — 스폰 예산이 쓴 것 기준(결정 A4 #38)이라 죽여도 다른 종이 오지 않으므로 지우는 도구의 값이 리썰보다 크다. (반려: 피해 도구 추가 — 원칙 ②를 "도구를 사면 산다"로 바꾼다.)
- **이번 범위 도구 = 먼저(무전기·딱총·깡통 딸랑이) + 다음(훈연기·포대·갈고리 장대)** — 종 카드가 준비물로 적은 수로 줄을 세웠다(초안 1.7, 7.3 B1, 결정 #32). (반려: 전부 넣기 — 제작량이 늘고 역효과 규칙이 겹친다.)
- **갈고리 장대는 손으로 푸는 붙잡힘만 거리를 늘려 푼다, 끌지 않는다** — 새 복제 후보가 없어지고 쳐서 푸는 쪽의 타격 소음 비용이 남는다(초안 7.3 B2, 통합4 2절 4번). (반려: 1차 안의 동료 끌기 — 새 복제가 들고 여우·거미줄 규칙을 지운다.)
- **냄비는 상점에서 뺀다** — 소리 도구는 깡통 딸랑이가 맡는다(초안 7.3 B5).
- **무전기 보유 상한 4**(`EquipmentTable` · `MaxOwned`) — 무전기가 둘뿐이면 무전기 없는 진짜 동료도 "무전에 답하지" 못해 플레이어 탈 규칙이 흐려진다(초안 7.3 B6). (반려: 2대.)
- **1일차에만 손전등을 사람 수만큼 준다** — 첫 판에 처음부터 짐꾼만 하는 사람이 없게, 그 뒤는 준비 책임(초안 7.3 B8).
- **버터는 다음 동네까지 미룬다** — `honey_butter`는 `Weight` 0(초안 7.3 B9). (반려: 데운 우유를 "버터"로 부르기.)
- **새 재료의 마찰은 기존 장치만** — 두 손·깨짐·줍기 소음·무게, 새 장치는 `PickupNoiseRadius`·`PickHoldTime` 둘. 밀랍은 넣지 않는다(초안 7.3 B10).
- **판매가 < 구매가를 로드 검사로 막는다**(초안 7.3 B11).
- **벌통 여는 시간은 꿀 단지 행의 `PickHoldTime`** — 새 상호작용 없이 줍기에 시간만 더한다(초안 7.3 B13). (반려: 벌통 소품 `OpenTime`, 전역 `HiveOpenTime`.)
- **먹을 것은 새 열 `FoodTag`로 가른다**(초안 7.3 B15). (반려: 익힘 상태에서 읽기 — 잼병·모이처럼 체인이 없는 행을 적을 수 없다.)
- **닭 드롭 = 새 닭고기 행, 확률·상한은 표**(통합 B8). 다른 종은 드롭이 없다(초안 4.5) — 드롭이 있으면 처치가 해법이 된다.
- **탄 것마다 먹는 시간을 다르게 두지 않는다** — 탄 행 `EatTime`은 비워 `DecoyEatTime`을 쓴다(초안 7.4 C2 기본값).
- **이더 흐름에 몰래 역할 임무 보상 줄을 더한다**(결정 #47 — 임무마다, 값은 표에서).

## Acceptance Criteria

모든 항목은 PIE(리슨 2인은 표시한 곳만)·자동화 테스트·로그로 예/아니오가 난다. "몬스터 명세 뒤"라고 적은 항목은 종 쪽 열이 들어온 뒤에 확인한다.

### 단계 1

- [ ] `CL9Editor`가 빌드되고, 새 자동화 테스트 `CL9.Items.Economy`(이름은 초안)가 통과한다. 테스트는 아래 넷을 본다.
  - [ ] E1: `ItemTable` 행 `ValueMax`가 그 행을 가리키는 `EquipmentTable` 행 `Price` 이상인 테스트 데이터로 `UCL9EquipmentDef::Validate`가 오류 1건을 낸다. 미만이면 0건이다.
  - [ ] 드롭 상한: `DropBudgetPerRound` 4, 닭 `DropChance` 1.0으로 처치 10번을 굴리면 드롭은 4개, `DropBudgetPerRound` 0이면 10개다.
  - [ ] 미끼 시간: `EatTime` 0인 행은 `DecoyEatTime`을, 0이 아닌 행은 그 값을 돌려준다.
  - [ ] 먹을 것: `FoodTag` `None`인 행은 어느 종의 미끼 대상도 아니다.
- [ ] `C:\Work\CL9\Source\CL9` 아래 이 goal 이 더한 코드에 튜닝 수치 리터럴이 없다 — 모든 값을 위 표 열에서 읽는다(리뷰로 확인).
- [ ] PIE 양계장: `cl9.addmonster chicken 10` 뒤 `cl9.hitmonster 9`로 열 마리를 치면 호스트 로그의 드롭 줄이 `DropBudgetPerRound` 이하이고, 떨어진 것은 모두 `chicken_meat`다. 같은 판에서 다른 종을 쳐도 드롭이 없다.
- [ ] PIE 로비(회사): `cl9.buy` 목록에 `walkie`·`firecracker`·`rattle_can`·`burlap_sack`·`hook_pole`이 있고 `pot`은 없다. `frying_pan`의 가격이 `Price` 값(초안 값 25)으로 뜬다.
- [ ] PIE 리슨 2인, `cl9.day 1` → `cl9.night`: 두 플레이어의 퀵슬롯에 손전등이 하나씩 있다(`StartItems`). `cl9.day 2`로 같은 절차면 지급이 없다.
- [ ] PIE: 탄 계란을 던지면 닭이 와서 `DecoyEatTime` 동안 먹는다 — 지금 `burnt-bait` 동작과 같다(로그의 먹기 시작·끝 시각 차가 `DecoyEatTime` ± 0.1초).
- [ ] PIE: 딱총을 던지면 손을 떠난 시각부터 `FuseTime` ± 0.1초 뒤 터짐 로그 한 줄과 `Burst` 소음 파동(`cl9.shownoise` 반경 = `BurstNoiseRadius`)이 나고, 아이템이 월드에서 사라진다. (몬스터 명세 뒤) `StunRadius` 안의 닭이 `StunTime` × `StunMult` 동안 움직이지 않는다는 로그가 난다.
- [ ] PIE: 깡통 딸랑이를 놓으면 `Rattle` 파동이 `EmitNoiseInterval` ± 0.05초 간격으로 floor(`EmitNoiseDuration` ÷ `EmitNoiseInterval`) ± 1번 나고 멈춘다. 울리는 중에 주우면 그 순간 멈춘다.
- [ ] PIE(몬스터 명세 뒤): 포대를 잠든 거위에 맞히면 덮임 상태 로그가 나고, 걷기 소음으로는 깨지 않는다.
- [ ] PIE 리슨 2인(몬스터 명세 뒤): 양에 묶인 동료에게서 `PinRescueRadius` 밖·`RescueReach` 안에 서서 갈고리 장대를 들고 구출 입력을 `PinRescueTime` 동안 하면 풀린다. 장대 없이 같은 자리에서는 풀리지 않는다. 여우에게 끌리는 동료는 장대로 풀리지 않는다.
- [ ] PIE 리슨 2인: 두 무전기를 켜고 거리를 `RadioClearRange` 안 → 사이 → `RadioMaxRange` 밖으로 옮기면 `cl9.voice` 덤프의 무전기 잡음 섞임이 0 → 0 초과 `RadioStaticMax` 이하 → 끊김으로 바뀐다. 주머니의 켜진 무전기로도 보내진다(결정 #58). 꺼진 무전기는 보내지도 받지도 않는다.
- [ ] PIE: `chicken_meat`를 조리대에 두면 `CookTime` 뒤 `cooked`, 그 뒤 `BurnTime` 뒤 `burnt`가 되고, `burnt`의 값은 0이다. `chick`·`bundle`(단계 2)은 트럭 판매 창구가 받지 않는다(`bSellable` false 로그).
- [ ] `cl9.orders`를 여러 일차에 돌리면 `roast_chicken`이 제시될 수 있고 `honey_butter`는 제시되지 않는다(`Weight` 0).
- [ ] Must-Preserve 의 기존 동작 회귀 없음: 프라이팬 타격 2·넉백, 손전등 정전 소등, 기존 세 주문 보상, 몰래 역할 보상 지급이 이 goal 전과 같다(기존 테스트 `CL9.Run.Imposter`·`CL9WipePenaltyTests` 통과).

### 단계 2

- [ ] 새 재료 넷이 조리대에서 `CookTime`/`BurnTime`대로 익고 타며, 탄 행 값이 0이다. 잼병은 조리대에서 바뀌지 않는다.
- [ ] 견과를 주우면 `PickupNoiseRadius` 반경의 줍기 파동이 난다(`cl9.shownoise`). `PickupNoiseRadius` 0인 행은 지금 줍기 소음과 같다.
- [ ] 꿀 단지는 줍기를 `PickHoldTime` 동안 눌러야 집히고, 그보다 짧으면 집히지 않는다. `PickHoldTime` 0인 다른 행은 바로 집힌다.
- [ ] 훈연기를 켜 든 사람의 `CalmRadius` 안에서 벌통 광폭화 사건이 일어나면 증가량이 `CalmRageMult`배라는 로그가 난다(몬스터 명세 뒤).
- [ ] 새 집 셋의 재료가 그 집과 몽장 사이트 행에서만 스폰된다(`MapTable` · `ItemSpawns` 검사 — 양계장 행에 없다).
- [ ] 새 주문 여섯이 `cl9.orders`로 제시될 수 있고, 보상이 식(재료 익힌 값 합 × 배율, 5 단위)과 맞는다.

## Tuning Values (초안 값 — 전부 표 열, 코드 상수 금지)

근거가 있는 값만 "초안 값"으로 둔다. 근거가 없는 값은 아래 "밸런스 몫"에 비운 채 모았다.

| 표 · 열 | 행 | 단위 | 초안 값 | 근거 |
|---|---|---|---|---|
| `ItemTable` · `FuseTime` | `firecracker` | 초 | 2.25 | 리썰 `StunGrenadeItem` · `TimeToExplode` 2.25(`C:\Work\CL9\docs\reference\lethal\lethal-items.md` 213줄) |
| `ItemTable` · `StunTime` | `firecracker` | 초 | 2.0 | 미메시스 투척 기절 2초(`C:\Work\CL9\docs\reference\mimesis\mimesis-items.md` 53줄) |
| `ItemTable` · `EmitNoiseRadius` | `rattle_can` | cm | 800 | 리썰 `WhoopieCushionItem` 8 m(`lethal-items.md` 202줄) |
| `ItemTable` · `EmitNoiseInterval` | `rattle_can` | 초 | 0.75 | 리썰 `AnimatedItem` 0.75초마다 소음(`lethal-items.md` 193줄) |
| `ItemTable` · `RescueReach` | `hook_pole` | cm | 300 | 몬스터 쪽 답 W15 "3m 밖에서 푸는 보조"(초안 9.1) |
| `ItemTable` · `EatTime` | 탄 행 전부, 그 밖의 비먹이 행 | 초 | 0(→ `CL9Rules` · `DecoyEatTime` 5.0) | 지금 값(초안 7.4 C2) |
| `ItemTable` · `FoodTag` | 재료 행 `Raw`, 익힌 행 `Cooked`, 탄 행 `Burnt`, 도구·병아리·꾸러미 `None` | enum | 왼쪽 | 초안 3.2 |
| `ItemTable` · `bSellable` | `chick`·`bundle` false, 나머지 true | bool | 왼쪽 | 통합 B18·B12 "보상 없음·0코인" |
| `ItemTable` · `ValueMin`~`ValueMax` | `chick`·`bundle`, 모든 탄 행 | 코인 | 0 | 통합 B18·B12, `bWorthlessIfBurnt` |
| `ItemTable` · `CookTime` / `BurnTime` | `meat`·`chicken_meat`(단계 2: `milk_jug`·`nut`·`dried_fruit`·`honey_pot`) | 초 | 12 / 24 | 계란 행 지금 값 — 다르게 둘 근거 없음(초안 3.2) |
| `ItemTable` · `PickupNoiseRadius` | `nut`(단계 2) | cm | 390 | `CL9Rules` · `PickupLoudRadius` 390과 같게(초안 3.1) |
| `ItemTable` · `CrackSpeed`·`BreakSpeed`·`CrackedBreakSpeed` | `jam_jar`(단계 2) | 지금 계란 행 단위 | 계란 행 값 그대로 | 초안 2.2 |
| `ItemTable` · `HoldPose` | `bundle`(단계 2) | enum | `Large` | 통합 B12 |
| `MonsterTable`(닭) · `DropItems[]` · `DropChance` | `chicken_meat` 항목 | 비율 | 0.5 | 통합 B8 |
| `CL9Rules` · `DropBudgetPerRound` | — | 개 | 4 | 통합 B8 |
| `CL9Rules` · `ItemLandNoiseRadius` | — | cm | 390 | 달리기 `RunNoiseRadius` 390과 같은 눈금(초안 7.4 C5) |
| `CL9Rules` · `CartNoiseRadius` | — | cm | 390 | 같음 |
| `CL9Rules` · `ImposterMissionReward` | — (메타 명세가 만든다) | 이더 | 5 | 결정 #47 "임무마다 5이더, 값은 표에서" |
| `EquipmentTable` · `Price` | `frying_pan` | 코인 | 25 | 지금 값 |
| `EquipmentTable` · `UnlockDay` | `frying_pan` 1, `flashlight` 1, `headlamp` 2, `walkie` 1, `rattle_can` 2 | 일차 | 왼쪽 | 초안 2.1·1.7(닭이 1일차부터 / 결정 #1 / 1일차에 정전 없음 / 플레이어 탈 첫 등장일 미정이라 가장 이른 날 / 결정 #36으로 1일차 마당이 빔) |
| `EquipmentTable` · `MaxOwned` | `frying_pan` 4, `flashlight` 0(무제한), `headlamp` 4, `walkie` 4 | 개 | 왼쪽 | 최대 인원 4, 결정 #1(배터리 없음 — 다시 사는 것이 유일한 보충) |
| `OrderTable` · `DeadlineDays` | `roast_chicken`(단계 2: 한 집 주문 넷) | 일 | 1 | `fried_egg` 지금 값 |
| `OrderTable` · `Weight` | `honey_butter` | 가중치 | 0 | 초안 7.3 B9 |
| `CL9Progression` · `DayOverrides`(1일차) · `StartItems` | `flashlight` | 개 | 사람 수만큼 | 초안 7.3 B8 |
| `SA_CL9_VoiceSpeaker` · `Inner` / `SilentAt` | — | cm | 150 / 1500(지금 값 그대로) | `C:\Work\CL9\docs\plan\data\rules.md` 음성 감쇠 표 |

**식으로 정해지는 값** — 익힌 행 `ValueMin`~`ValueMax` = 날것 × 1.7~1.9(지금 계란·고기의 비, 초안 2.2). 한 집 주문 `RewardMin`~`RewardMax` = 재료 익힌 값 합 × 2.2~2.4(지금 세 주문의 배율, 초안 3.3), 5 단위로 맞춘다. 날것 값이 비어 있으면 함께 비어 있다.

## 밸런스 몫 (숫자만 정하면 되는 것 — 근거가 없어 비웠다)

`cl9-balance` 역할이 채운다. "필요 시점"은 그 값이 없으면 해당 수용 기준을 확인할 수 없는 때다. 지킬 조건은 값이 아니라 범위이므로 그대로 따른다.

| # | 표 · 열 · 행 | 단위 | 상태 | 지킬 조건 · 참고 | 필요 시점 | 초안 위치 |
|---|---|---|---|---|---|---|
| 1 | `ItemTable` · `StunRadius`(`firecracker`) | cm | 비움 | — | 단계 1 딱총 | 7.4 C7 |
| 2 | `ItemTable` · `BurstNoiseRadius`(`firecracker`) | cm | 비움 | 거위 `HearRunRadius` 1260보다 크고 닭 `KillAlarmRadius` 2100보다 작게 | 단계 1 딱총 | 7.4 C7 |
| 3 | `ItemTable` · `FuseTime`(`firecracker`) | 초 | 초안 값 2.25 | 조작 초안 C6 기본값 2.0과 갈린다 — 둘 중 하나로 맞춘다(`C:\Work\CL9\docs\plan\draft\_integration-4.md` 4.3 C6) | 단계 1 | 1.4 T1 |
| 4 | `ItemTable` · `EmitNoiseDuration`(`rattle_can`) | 초 | 비움 | 조작 초안 C6 기본값 6.0이 있다. 함께 볼 값: 길이 ÷ `EmitNoiseInterval` 0.75가 눈먼 개 `SameSpotIgnoreCount` 4를 넘으면 뒷소리를 개가 무시한다 | 단계 1 딸랑이 | 7.4 C8 |
| 5 | `CL9Rules` · `RadioClearRange` · `RadioMaxRange` | cm | 비움 | `RadioClearRange` < `RadioMaxRange` | 단계 1 무전기 | 7.4 C15 |
| 6 | `CL9Rules` · `RadioStaticMax` | 비율 0~1 | 비움 | — | 단계 1 무전기 | 7.4 C15 |
| 7 | `CL9Rules` · `RadioFloorPenalty` | 배율 | 비움 | 0이면 층 무시 | 단계 1 무전기 | 7.4 C15 |
| 8 | `SA_CL9_VoiceSpeaker` · `Inner`·`SilentAt`를 줄일지와 비율 | 비율 | 지금 값 유지 | 줄이면 두 값을 같은 비율로(`rules.md` 65줄) | 언제든 | 7.4 C16 |
| 9 | `EquipmentTable` · `Price` — `flashlight`·`headlamp`·`cart`·`standing_lamp`·`walkie`·`firecracker`·`rattle_can`·`burlap_sack`·`hook_pole`(단계 2 `smoker`) | 코인 | 비움 | 판매가 < 구매가(E1) | 단계 1 상점 | 7.4 C11 |
| 10 | `EquipmentTable` · `UnlockDay` — `cart`·`standing_lamp`·`firecracker`·`burlap_sack`·`hook_pole`(단계 2 `smoker`) | 일차 | 비움 | 딱총: 막을 이유도 늦출 근거도 없다. 포대: 거위가 자주 나오는 날, 장대: 양·허수아비가 처음 나오는 날, 훈연기: 벌집이 열리는 날(모두 미확인) | 단계 1 상점 | 7.4 C10·C11, 2.1 |
| 11 | `EquipmentTable` · `MaxOwned` — `cart`·`standing_lamp`·`firecracker`·`rattle_can`·`burlap_sack`·`hook_pole`(단계 2 `smoker`) | 개 | 비움 | 0 = 무제한. 팀 전체로 세는지 사람마다인지 미확인 | 단계 1 상점 | 7.4 C11 |
| 12 | `ItemTable` · `ValueMin`~`ValueMax`(`flashlight`) | 코인 | 비움 | < 손전등 `Price`(E1) | 단계 1 | 7.4 C11 |
| 13 | `ItemTable` · `ValueMin`~`ValueMax`(`chicken_meat`) | 코인 | 비움 | 평균 < 성한 계란 평균 12.5 | 단계 1 드롭 | 7.4 C12 |
| 14 | `ItemTable` · `ValueMin`~`ValueMax`(`milk_jug`·`nut`·`dried_fruit`·`jam_jar`·`honey_pot`) | 코인 | 비움 | 잼병은 다른 새 재료보다 비싸게 | 단계 2 | 7.4 C12 |
| 15 | `MapTable` · `ItemSpawns`(새 재료의 한 판 산출) | 개 | 비움 | 꿀 단지 = 벌통 지점 수(맵 기획과 함께) | 단계 2 | 7.4 C13 |
| 16 | `OrderTable` · `RewardMin`~`RewardMax`(섞인 주문 `custard`·`honey_nuts`의 배율) | 배율 | 비움 | `economy.md` 2~3배 안, 한 집 주문(2.2~2.4)보다 높게 | 단계 2 | 7.4 C14 |
| 17 | `OrderTable` · `DeadlineDays`(섞인 주문) | 일 | 비움(초안의 2는 근거 없음) | — | 단계 2 | 7.4 C14 |
| 18 | `ItemTable` · `EatTime`(날재료 행) | 초 | 비움(0이면 5.0) | 5.0보다 짧게 — 탄 요리가 더 좋은 미끼로 남는다 | 단계 1(고기)·단계 2 | 7.4 C2 |
| 19 | `ItemTable` · `PickHoldTime`(`honey_pot`) | 초 | 비움 | 조작 초안 C6 기본값 3.0이 있으나 근거는 옛 추정값뿐(초안 3.4) | 단계 2 | 7.4 C1 |
| 20 | `ItemTable` · `CalmRadius`·`CalmRageMult`(`smoker`) | cm · 배율 | 비움 | — | 단계 2 | 7.4 C9 |
| 21 | `MonsterTable` · `HoodedRunHearMult`(거위) | 배율 | 비움 | 몬스터 명세의 열 — 값만 여기에 모아 둔다 | 단계 1 포대(몬스터 명세 뒤) | 7.4 C5 |
| 22 | `CL9Rules` · `ImposterWinReward` | 이더 | 30 그대로 | 판 수입이 375코인(= 30 ÷ `WalletIncomeRate` 0.08)을 넘는 날이 실제로 오는지 관찰한 뒤 올린다 | 관찰 뒤 | 7.4 C3, 5.3 |

## Related Files / Modules

| File | Role |
|------|------|
| `C:\Work\CL9\Source\CL9\Data\CL9ItemDef.{h,cpp}` | `ItemTable` 행 — 새 열, Validate |
| `C:\Work\CL9\Source\CL9\Data\CL9EquipmentDef.{h,cpp}` | `EquipmentTable` 행 — E1 검사(`Validate`) |
| `C:\Work\CL9\Source\CL9\Data\CL9Types.h` | `FDropItem`(`DropChance` 이미 있음), `ECL9HoldPose`, 새 enum `FoodTag` |
| `C:\Work\CL9\Source\CL9\Data\CL9RulesDef.{h,cpp}` | `CL9Rules` 새 셀 |
| `C:\Work\CL9\Source\CL9\Data\CL9ProgressionDef.{h,cpp}` | `DayOverrides` · `StartItems` |
| `C:\Work\CL9\Source\CL9\Data\CL9OrderDef.{h,cpp}` | `OrderTable` 새 행 |
| `C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h` | `DropItems` — 닭 행 데이터만 |
| `C:\Work\CL9\Source\CL9\Monsters\CL9Monster.{h,cpp}` | 드롭 굴림·상한, 미끼 먹기(`DecoyEatTime` 경로) |
| `C:\Work\CL9\Source\CL9\Items\CL9Item.{h,cpp}` | 도구 동작(심지·딸랑·덮개), 판매 가능 |
| `C:\Work\CL9\Source\CL9\Character\CL9Character.{h,cpp}` | 구출 반경(`RescueReach`), 무전기 스위치, 줍기 |
| `C:\Work\CL9\Source\CL9\GameMode\CL9GameMode.{h,cpp}` | 1일차 지급, 판마다 드롭 상한 카운터 |
| `C:\Work\CL9\Content\Data\ItemTable.uasset`·`EquipmentTable.uasset`·`MonsterTable.uasset`·`OrderTable.uasset`·`CL9Rules.uasset`·`CL9Progression.uasset` | 행·값 |
| `C:\Work\CL9\docs\plan\economy.md`, `C:\Work\CL9\docs\plan\data\equipment.md`·`order.md`·`rules.md`·`progression.md`·`item\` | 기획 정본 갱신 |
| `C:\Work\CL9\docs\systems\item.md`·`drop.md`·`shop.md`·`economy.md`·`voice.md`·`cheats.md` | 시스템 설계 갱신 |

## Must-Preserve

- **2026-09-11 "빛은 손전등(·헤드램프)에서만 나온다 — 발광 아이템 폐기"**(`C:\Work\CL9\docs\decisions.md` 939줄) — 스스로 빛을 내는 아이템 행을 만들지 않는다. 스탠드 램프는 그 결정이 인정한 광원이라 그대로다.
- **2026-10-11 핫라인 결정** — 핫라인 = 두 무전기 스피커의 양방향 연결, 한 사람에 하나, 새 연결이 옛 연결을 끊는다, 무전기는 사람과 무관(`C:\Work\CL9\docs\decisions.md` 1794줄, `C:\Work\CL9\docs\plan\voice.md` 2절). Open Questions 2번이 정해지기 전에는 바꾸지 않는다.
- **결정 #58** — 켜진 무전기는 어디 있든 보내고 받는다.
- **2026-10-12 루프 개편 7i** — 장비는 회사에서만 산다, 트럭에는 판매 창구만(`C:\Work\CL9\docs\decisions.md` 1867줄). 판 중에는 사지 못한다.
- **결정 #1** — 손전등에 배터리가 없다. 정전은 손에 든 손전등과 주머니 빛을 끈다.
- **몰래 역할 보상** — 성공 보상 `ImposterWinReward`는 생존 보상 **대신**, 처치 보상 `ImposterKillReward`는 덤, 한 사람 한 판 지급은 한 번(`C:\Work\CL9\docs\goal\loop-7g-imposter.md`, `C:\Work\CL9\docs\goal\loop-7f-out-game-currency.md`).
- **사냥 보상은 아이템만** — `DropItems`는 이더를 직접 주지 않는다(`CL9MonsterDef.h` 345줄 주석).
- **결정 #41** — 넘어짐으로 흘린 짐은 0~2개, 항상 발밑(`CL9Rules` · `KnockdownDropWeight0/1/2`).
- **결정 #42** — 빈 것의 값은 같은 행의 별도 열이다(가리키는 행을 두지 않는다).
- **탄 요리 미끼**(`C:\Work\CL9\docs\goal\burnt-bait.md`)와 기존 세 주문(`fried_egg`·`omelette`·`meat_dish`)의 보상·동작.
- 퀵슬롯 4칸(`CL9Rules` · `QuickSlots`) — 칸 수를 바꾸지 않는다.
- 다른 세션의 미커밋 변경.

## Fairness Constraints

- 드롭 굴림·상한 카운트·도구의 기절과 소음 판정·구출 판정은 호스트에서만 한다.
- 딱총 심지와 딸랑이 울림 시각은 호스트 시각 하나로 모든 머신이 같은 순간을 본다(통합4 5.1 S18).
- 드롭 확률과 한 판 상한은 화면에 띄우지 않는다(초안 8.2 권고 — 표시 여부의 최종은 UI 몫).
- E1 로드 검사로 사서 바로 파는 코인 구멍을 막는다.

## Existing Integration Contract

- 소음은 기존 소음 파동 경로(`cl9.wave`·`cl9.shownoise`로 보이는 것)에 새 종류 `Burst`·`Rattle`을 더해 쓴다. 감각 쪽 처리는 몬스터 명세.
- 구출은 기존 구출 입력(통합4 2절 4번 — 손으로 푸는 붙잡힘은 E 길게)을 쓰고 반경만 바꾼다.
- 무전기 스위치는 칸 데이터의 스위치(손전등 `bLightOn` 방식)를 넓혀 쓴다 — 새 복제 없음(추정, 기술 검토로 확인).
- 상점은 `cl9.buy`·회사 패널 경로를, 판매는 트럭 판매 창구를, 조리는 기존 상태 변화 체인(`CookResult`·`BurnResult`)을 그대로 쓴다.

## 범위 밖 / 의존

| 받는 쪽 | 무엇 | 초안 위치 |
|---|---|---|
| 몬스터 명세 | 종 열 `StunMult`·`EatsTags`·`bBaitDuringChase`·`bCanBeHooded`·`HoodedRunHearMult`·`NoiseReaction`·`HearMask`(`Burst`·`Rattle`)·빈 것 드롭 열. 되물음 M1~M8: 쥐의 새 규칙(#45 쥐 떼 몰이)과 손전등·딱총·딸랑이 반응(M1), 스피커 큰 목소리가 토끼도 쫓나(M2), 놓는 빛에 대한 여덟 종 반응(M3), 어미 닭 쪼아 붙잡기가 통제권인가(M4), 빈 것 드롭 열 이름(M5), 다 자란 병아리(M6), 벌의 자리와 터진 벌 떼에 훈연기(M7), 장대가 규칙을 지우지 않나(M8) | 9.2 |
| 조작 명세 | 드는 방식과 입력(Out-of-scope 목록). 통합4가 이미 추천대로 진행한 것: 딱총 점화 = 손을 떠나는 순간(B7), 손 떨림 = 컨트롤 회전(B10 — 던지기 조준도 흔들린다) | 8.1 |
| UI 명세 | 상점 패널·켜진 도구 점(통합4 B17)·무전기 신호 세기(통합4 B27 — 소리만)·꿀 단지 진행·0값 아이템·새벽 이더 줄·옷장 잠금 이유 | 8.2 |
| 판 루프 명세 | 해금 일차의 뜻(회사 3판마다), 첫 판 전 회사 방문 여부, 목적지가 열리는 날, 플레이어 탈 첫 등장일(무전기 `UnlockDay`가 그 이전), 주문 `MinDay`·`MaxDay`·`Weight`(`roast_chicken`은 가끔), 판 수입, 죽을 때 머리 칸(아이템 쪽 추천은 "남긴다") | 8.3 |
| 메타 명세 | `ImposterMissionReward`(결정 #47), 칭호 → 스킨 연결 열, 칭호 사건(빈 것 쓰러뜨림·닭 처치 수·병아리·꾸러미를 트럭에), 성공 보상 "생존 보상 + 덤" 번복 안 | 8.3 |
| 맵 명세 | 벌통 하나에 꿀 단지 하나, 새 재료 자리와 산출, 꾸러미 `Bundle` 지점(맵 초안의 "꾸러미는 소품" 문장 맞추기), 갈라놓는 문의 여는 조건 | 8.3 |
| 기술 검토 | X1 줍기에 누르는 시간과 누르는 동안 빛 판정, X2 손 떨림의 던지기 조준(통합4 B10으로 방향은 섰다), X3 장대 원격 구출에 새 복제가 없는가, X4 놓인 딸랑이의 반복 소음이 감각 사건인가, X5 미끼 처리를 행 짝으로, X6 E1 로드 검사. 통합본 8절 6·7·8·18·25번 | 9.3 |
| 아트 · 시퀀스·사운드 · 스토리 | 새 모델(무전기·딱총·깡통·훈연기·포대(씌운 모습)·갈고리 장대·새 재료·닭고기·잼병), 소리 목록, 꾸러미 = 짚단 장면 | 8.4 |

## Execution Notes

- Recommended model: Claude Fable 5 이상 — 미끼 경로 교체(X5), 드롭 상한의 호스트 카운트, 무전기 거리·잡음과 핫라인의 맞물림은 판단이 무겁다. 표 열·행 추가, 문서 갱신, 치트 확인은 Sonnet 으로 충분하다.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- 구현 전 순서: ① Open Questions 를 사용자에게 받는다 ② "밸런스 몫"의 "단계 1" 값을 `cl9-balance`가 채운다 ③ 몬스터 명세의 종 열이 들어왔는지 확인한다(안 들어왔으면 "몬스터 명세 뒤" 수용 기준은 남겨 둔다) ④ 기술 검토 X1~X6 결과를 확인한다.
- 빌드는 브로커(`C:\Work\CL9\Tools\Build\build_request.py`)로 요청한다. 에디터 Python 실행 전에 잠금을 확인한다.
- 문자열은 ko 열만 쓴다.

## Open Questions

1. **조명탄 `flare`를 넣나 — 번복 필요(2026-09-11 발광 아이템 폐기)**(초안 7.2 A2). 이 명세의 기본은 ①이다.
   - ① 넣지 않는다(결정 유지) — 번복이 없고 정전이 무섭게 남는다. 놓는 빛이 듣는 종(소·벌)은 스탠드 램프로 풀린다.
   - ② 결정을 번복하고 1회용 조명탄을 둔다 — 멀리 던지는 빛이 생기지만 스탠드 램프와 하는 일이 겹친다.
   - 추천: ①. 근거 — 초안 1.6 ②, 1.4 T8, `C:\Work\CL9\docs\decisions.md` 939줄.
2. **무전기를 어떻게 잇나 — 번복 필요(2026-10-11 핫라인 결정)**(초안 7.2 A3). 이 명세의 기본은 ②(결정 유지)다.
   - ① 켜진 무전기가 전부 한 채널 — 네 사람 모두가 같은 무전을 들어 플레이어 탈의 "무전에 답하는가"가 모두에게 선다. 핫라인 `Target`을 배열로 바꿔야 한다(`voice.md` 103줄).
   - ② 지금처럼 둘씩 짝 — 번복이 없고 지금 구현 그대로다. 한 사람이 확인할 수 있는 무전은 자기 짝 하나다.
   - 추천: 초안의 아이템 쪽 추천은 ①(플레이어 탈의 ○ 준비물이 무전기라서). 다만 `voice.md` 열린 질문 2의 답이라 사용자가 정한다. 근거 — 초안 6.3, `C:\Work\CL9\docs\plan\voice.md` 2절·열린 질문 2.
3. **제작량 합계를 어떤 꼴로 내나**(초안 7.2 A1, 1차 통합 C19).
   - ① 아트 기획이 처음 합친다.
   - ② 역할마다 같은 칸(메시·클립·소리·아이콘)으로 적는다.
   - ③ ②에 단계(먼저 · 다섯 집 전부 · 나중)를 더한다.
   - 추천: ③ — 합계와 "무엇부터 만드나"가 함께 나온다. 다섯 역할의 형식을 함께 바꾸는 정리라 아이템 쪽에서 정하지 않는다. 근거 — 초안 1.7 끝.
4. **이 goal 의 구현 범위를 결정 #48의 첫 묶음(양계장 + 10종)에 맞추나.** 초안은 결정 #48 전에 쓰여 다섯 집 재료·훈연기까지 이번 범위로 잡았다.
   - ① 단계 1만 이 goal 에 두고, 단계 2(외양간·찬장·벌집의 재료·주문·훈연기·꾸러미)는 그 맵이 설 때 따로 goal 로 뗀다.
   - ② 단계 1·2를 모두 이 goal 에 둔다(초안 범위 그대로).
   - 추천: ① — 결정 #48이 "맵 하나(양계장)로 재미를 먼저 본다"고 했고, 단계 2의 재료는 그 집 맵이 없으면 스폰될 자리가 없다. 근거 — `C:\Work\CL9\docs\decisions.md` 결정 #48, 초안 1.7 "이번 범위".
