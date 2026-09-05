# 진행·난이도 시스템 (밸런스)

> 일자(day)가 오르면 무엇이 어려워지는가, 밤 하나는 어떻게 흐르는가, 언제 실패하는가.
> **수치의 정본은 plan이다** — [../plan/data/progression.md](../plan/data/progression.md) ·
> [../plan/data/rules.md](../plan/data/rules.md) · [../plan/data/site.md](../plan/data/site.md).
> 이 문서는 그 값을 코드가 어떻게 읽고 적용하는지만 다룬다.

## 개요

- **"스테이지"는 난이도 단계가 아니라 목적지다.** 난이도는 일자 곡선이 만든다.
- **기준값 × 곡선**: 목적지가 기준값(닭 8~12)을 주고, 일자 곡선이 배율(3일차 0.8)을 곱한다.
- **밤 하나의 시계는 `NightLength` 하나뿐이다** (기본 245초). 페이즈 분할 시계는 없다 (현재 설계).
- **정산은 이틀 주기**(`SettlementIntervalDays`), 판정 대상은 **기간 수입**이지 보유 잔액이 아니다.
- **무한 진행** (`FinalDay` 0). 클리어는 없고 며칠까지 버텼는지가 기록이다.

## 설정 에셋 구성

plan이 정의한 세 에셋을 그대로 쓴다 — 별도의 "스테이지 설정" 에셋은 없고, 그 역할은 이
세 에셋이 나눠 맡는다 (현재 설계).

| 에셋 | 내용 | 정본 |
|---|---|---|
| `DA_CL9Rules` | 소리·죽음·밤 시계·운반 마찰 — 게임에 하나 | [../plan/data/rules.md](../plan/data/rules.md) |
| `DA_CL9Progression` | 일자 곡선 6종 · 주문 N/M · 일자 덮어쓰기 · 정산 | [../plan/data/progression.md](../plan/data/progression.md) |
| `DA_Site_*` (`UCL9SiteDef`) | 목적지별 맵 생성·몬스터/아이템 기준값·밤 시계 덮어쓰기 | [../plan/data/site.md](../plan/data/site.md) |

곡선은 전부 `UCurveFloat`이다 — 에디터에서 그래프로 만진다.

## 밤 하나의 흐름 (GameMode 상태 기계)

```
로비(정차) → 주문서 수령(N개 중 M개 선택) → 목적지 선택 → 밤 시작
  ↓
원정 (NightLength 초 — 단일 시계)
  · 정전 시계가 병행: DarkBase - 반납횟수 × DarkAccel (하한 DarkMin)
  · 귀환·요리·납품은 밤 안에서 자유롭게 일어난다 (트럭이 같은 맵에 있다 — map.md)
  ↓
밤 종료 (전원 귀환/탈출 또는 NightLength 만료 = 새벽 실패)
  ↓
새벽 화면 (DawnLength) → 정산일이면 기간 수입 판정 → 로비로
```

**페이즈 전환이 없다.** 원정·요리·판매는 시간으로 구분되는 단계가 아니라 밤 안에서 플레이어가
오가는 활동이다. "요리 시간" "판매 시간" 같은 시계를 두지 않는다.

## 실패 판정 (GameMode)

실패 축은 셋뿐이다 (plan/data/rules.md · progression.md):

```cpp
// 1. 밤 만료 — NightLength 초과 = 새벽 실패
// 2. 전멸 — 전원 사망 시 트럭이 일행을 두고 떠난다
// 3. 정산 미달 — 정산일에 기간 수입 < QuotaCurve(일차)

// 정산 판정 (이틀 주기, 기간 수입 기준)
if (Day % SettlementIntervalDays == 0) {
  if (PeriodIncome < QuotaCurve->GetFloatValue(Day)) {
    // 그날로 실패. 유예 없음. 새 판이 자동으로 열린다
  }
  PeriodIncome = 0;  // 다음 기간
}
```

**보유 잔액이 아니라 기간 수입이다** — 장비를 사도 정산이 불리해지지 않는다.

## 난이도 곡선 적용

| 곡선 | 무엇에 곱하나 | 초안 방향 |
|---|---|---|
| `MonsterCountCurve` | 목적지의 몬스터 수 | 오른다 |
| `DangerCurve` | 추격 예산·발각 속도·시야 | 오른다 |
| `ItemCountCurve` | 목적지의 재료 수 | 평평 |
| `ValueCurve` | 아이템·몬스터 가격 | 오른다 |
| `QuotaCurve` | 정산일의 목표 이더 | 오른다 (10일 이후에도 계속) |
| `NightLengthCurve` | 밤 길이 | 평평 |

값과 곡선 모양의 정본·근거는 [../plan/data/progression.md](../plan/data/progression.md)와
[../plan/economy.md](../plan/economy.md).

```cpp
// 밤 시작 시 (GameMode, 호스트 전용)
int32 ChickenCount = FMath::RoundToInt(
  FMath::RandRange(Site->MonsterSpawns[chicken].MinCount,
                   Site->MonsterSpawns[chicken].MaxCount)
  * Progression->MonsterCountCurve->GetFloatValue(Day));
```

**구현 (4b, 2026-09-05)**: 위 식 그대로 `ACL9GameMode::SpawnNightPopulation`(밤 시작, 호스트).
`RoundToInt(RandRange(Min,Max) × MonsterCountAt(Day))` — 배율은 `UCL9ProgressionDef`가 덮어쓰기 →
곡선 순으로 준다. 재료도 같은 식(`ItemCountAt`). 위치는 맵 생성기 전까지 평지 스탠드인(트럭 패드
앵커 원환, `ThreatActMinRadius` 안 회피, 바닥 트레이스). `DangerScale = Site.BaseDanger ×
DangerAt(Day)`를 GameState에 실어 몬스터가 ChaseBudget·DetectRate·Sight에 곱한다(monster.md).
`NightLength × NightLengthAt(Day)`. 로비 복귀(`EnterLobby`) 때 밤 스폰분 철거·DangerScale 1 복귀.
로그 `Night N: chicken ×P placed (base 8~12 × 0.60 = C)`로 곡선 반영을 읽는다.

## 일자 덮어쓰기

`DayOverrides` 배열로 특정 일자를 손으로 잡는다 — 1일차는 튜토리얼처럼
(목적지 하나·주문 하나·정전 없음·몬스터 적게). 컬럼은 progression.md의 `FCL9DayOverride`.
구현(4b): `MonsterCountMult`·`DangerMult`(0 = 곡선)·`bNoBlackout`만 먼저 — DA_CL9Progression 1일차
행은 `bNoBlackout`만 켰다(`cl9.blackout`은 여전히 강제 발동).

## 네트워크·상태

- 설정(곡선·기준값)은 에셋 — 각 머신이 로컬로 읽고, 파생 런타임 값(오늘의 마릿수·시계)만
  GameState로 공유한다 (conventions.md § 5).
- 진행 상태(현재 일차·누적 자금·기간 수입·진행 중 주문)는 **GameState** — 판 공유,
  호스트가 소유하고 세이브에 들어간다 ([session.md](session.md)).
- 밤 시계·정전 시계는 GameState의 Replicated 값으로 모든 클라에 동기화된다.

## 디버깅/테스트

- `cl9.day <N>`: 특정 일차로 이동 — 곡선 적용 확인 ([cheats.md](cheats.md))
- 에디터의 DataAsset 편집 창: 값·곡선 수정 후 즉시 테스트 (빌드 불필요)

## 참고

- 습격(원정): `raiding.md`
- 요리: `cooking.md`
- 판매·주문: `economy.md` · `order.md`
- 세이브·로비: `session.md`
