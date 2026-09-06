# 일자 진행 — `DA_CL9Progression`

**"몇 일차에 닭이 몇 마리인지"를 정하는 자리다.** 게임에 하나뿐인 에셋이다.

---

## 곡선 × 기준값

일자마다 행을 손으로 채우면 20일차까지 20행을 관리해야 하고, 목적지를 추가할 때마다 전부
다시 손봐야 한다. 그래서 **두 층으로 나눈다.**

```
목적지가 기준값을 준다        양계장 = 닭 8~12마리
        ×
일자 곡선이 배율을 준다       3일차 = 0.8
        =
실제                          6~10마리
```

---

## 곡선

전부 `UCurveFloat` 에셋이다. 에디터에서 **그래프로 만진다** — 숫자 표가 아니라 곡선을 끌어서
모양을 본다.

| 컬럼 | 무엇에 곱하나 | 초안 모양 |
|---|---|---|
| `MonsterCountCurve` | 목적지의 몬스터 수 | 1일차 0.6 → 5일차 1.0 → 10일차 1.5 |
| `DangerCurve` | 추격 예산 · 발각 속도 · 시야 | 1일차 0.8 → 10일차 1.3 |
| `ItemCountCurve` | 목적지의 재료 수 | 1일차 1.0 → 10일차 1.0 (평평) |
| `ValueCurve` | 아이템·몬스터 가격 | 1일차 1.0 → 10일차 1.4 |
| `QuotaCurve` | 정산일의 목표 이더 | 첫 정산(2일차) 800 → 급격히 상승 |
| `NightLengthCurve` | 밤 길이 | 평평 (1.0 고정) |

**초안일 뿐이다.** 곡선 모양은 플레이해보고 정한다.
각 축이 무엇을 움직이는지는 [../economy.md](../economy.md).

---

## 주문서

| 컬럼 | 뜻 | 초안 |
|---|---|---|
| `OrdersOfferedPerDay` | 하루에 제시하는 개수 N | 3 |
| `OrdersRequiredPerDay` | 골라야 하는 개수 M | 2 |
| `OrdersOfferedCurve` | 일자에 따라 N을 늘리려면 | 평평 |

**N = M이면 선택이 사라진다.** M이 너무 작으면 쉬운 것만 골라 밤이 늘어진다.
상세는 [order.md](order.md).

---

## 특정 일자 덮어쓰기

곡선만으로는 "1일차는 튜토리얼처럼" 같은 걸 못 만든다. 그래서 예외 목록을 둔다.

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `DayOverrides` | `TArray<FCL9DayOverride>` | 일자별 예외 |

```
FCL9DayOverride
  Day                 몇 일차
  ForcedSite          강제 목적지 (비우면 선택 가능)
  ForcedOrders        강제 주문 목록 (비우면 무작위)
  MonsterCountMult    곡선 대신 이 값 (0이면 곡선 사용)
  DangerMult
  QuotaOverride
  bNoBlackout         이 날은 정전 없음
```

**1일차는 여기서 손으로 잡는다** — 목적지 하나·주문 하나·정전 없음·몬스터 적게.
규칙을 배우는 날이라 무작위에 맡기지 않는다.

---

## 종료 조건 — 무한 + 실패

| 컬럼 | 타입 | 뜻 | 값 |
|---|---|---|---|
| `FinalDay` | int | 마지막 일차 (0 = 무한) | **0** |
| `SettlementIntervalDays` | int | 며칠에 한 번 정산하나 | 2 (초안) |

**정산일에 그 정산 기간 동안 번 이더(납품·판매 수입)가 `QuotaCurve`의 목표에 미달하면
그날로 실패다.** 판정 대상은 기간 수입이지 보유 잔액이 아니다 — 아무리 쥐고 있어도 이번
기간에 못 벌었으면 끝이고, 장비를 사도 정산이 불리해지지 않는다. 유예는 없다.
클리어는 없고, 며칠까지 버텼는지가 기록이다.

**곡선의 10일 이후 구간은 `QuotaCurve`만 계속 오른다.** 나머지 다섯은 마지막 키 값으로
고정된다 — 난이도에는 상한이 있지만 요구액이 계속 올라 언젠가는 진다.

하룻밤 안의 **시간** 실패는 밤 만료 하나다(전멸은 별개 축) — 정산 실패는 여러 날에 걸친
또 다른 축이다.

---

## 진행 상태는 어디 저장하나

곡선은 설정이고, "지금 며칠째인가"는 진행 상태다. 이건 에셋이 아니라 세이브에 들어간다.

```
현재 일차 · 누적 자금 · 보유 장비 · 진행 중인 주문 · 해금된 목적지
```

**진행은 호스트의 것이다.** 판 하나가 세이브 하나이고 호스트가 소유한다 — 누가 접속하든
같은 판을 이어간다. 새 판은 정산에 실패하면 자동으로 열리고, 로비에서 수동으로도 시작할 수
있다. 세이브 슬롯은 나중에 3개 정도로 늘릴 예정이므로, 저장 구조가 슬롯 하나를 전제로
굳으면 안 된다.

---

## 미결

- **곡선 모양 전부** — 위 초안은 짐작이다. 플레이해봐야 나온다

---

## 구현 상태 (4b, 2026-09-05)

`UCL9ProgressionDef`(Source/CL9/Data) — 레지스트리에 규칙처럼 **단일** 행으로 등록된다
(`DA_CL9Progression`, Content/Data). 들어간 컬럼: `MonsterCountCurve`·`DangerCurve`·`ItemCountCurve`·
`NightLengthCurve` + `DayOverrides{Day, MonsterCountMult, DangerMult, bNoBlackout}`. 곡선은
`FRuntimeFloatCurve`다 — 에셋 디테일에서 **그래프로 끌거나** 외부 `UCurveFloat`를 가리킬 수 있다.
빈 곡선은 1.0, 마지막 키 뒤는 마지막 값 유지(위 "10일 이후" 규칙). 주문서 N/M(`OrdersOfferedPerDay` 3
· `OrdersRequiredPerDay` 2)은 4c에서, `ValueCurve`(+ `ValueAt(일자)`)는 4e에서 들어갔다 — 판매가의 일자
배율이며 `ACL9GameMode::ComputeSellPrice`가 읽는다. `QuotaCurve`·`OrdersOfferedCurve`·
`ForcedSite/ForcedOrders/QuotaOverride`·`FinalDay`·`SettlementIntervalDays`는 각 소비처(⑥·4g)가 생길 때
넣는다.

적용 지점 — 밤 시작(`ACL9GameMode::StartNight`, 호스트): 목적지 `MonsterSpawns/ItemSpawns` ×
곡선 → 스폰, `DangerScale = BaseDanger × DangerCurve`를 GameState에 실어 몬스터가 추격 예산·
발각 속도·시야에 곱한다, `NightLength × NightLengthCurve`, `bNoBlackout`이면 정전 미장착.
로비 복귀 시 밤 스폰분 철거. 검증 치트 `cl9.day <N>`.
