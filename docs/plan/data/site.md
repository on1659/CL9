# 목적지 — `UCL9SiteDef`

원정을 가는 곳. 양계장 · (나중에) 벌집 · 그 밖.

"스테이지"는 난이도 단계가 아니라 **어디로 가는가**다.
주문서가 필요한 재료를 정하고, 재료가 목적지를 정한다.

---

## 맵 생성

> **2026-09-08 재작성.** 격자 미로를 버리고 모듈 조립으로 갈아엎으면서 이 절의 컬럼 열세 개가
> 사라졌다 — 설계 정본은 [module.md](module.md)다. 아래는 그 뒤에 남은 것이다.

| 컬럼 | 타입 | 뜻 | 양계장 |
|---|---|---|---|
| `Generator` | enum `ECL9Generator` | **어떤 생성기를 쓰나** | `Henhouse` |
| `Theme` | `FName` | **모듈 표와 조각 표를 함께 가리킨다** | `henhouse` |
| `CellSize` | float | 격자 칸 하나의 크기 (cm) — 조각 한 장과 같아야 벽이 안 늘어난다 | 500 |
| `BaseModuleCount` | int | 한 판의 모듈 수 기준값 — 진행 표의 `SiteSizeCurve`가 곱해진다 | 24 |
| `EntryOffset` | float | 트럭 패드에서 시작 모듈 바깥면까지 (cm) | 1000 |

**`Theme`가 두 표를 한 이름으로 가리킨다.** 조각 표는 테마에 없는 역할이 공용 행으로 떨어지지만,
모듈 표는 폴백이 없다 — 스테이지의 모듈 세트는 통째로 한 벌이다.

**크기는 일자가 정한다** (module.md 결정 5). `BaseModuleCount` 24 × `SiteSizeCurve`
(1일차 0.85 → 5일차 1.0 → 10일차 1.25) = 20 · 24 · 30. 실측으로 평균 24.0모듈이 나온다.

**치수는 컬럼이 아니라 실측이다.** 벽 두께는 `barn` 조각의 얇은 축에서, 문 폭은 `door` 조각의
넓은 축에서 `ACL9SiteMap`이 읽는다 — 팩이 바뀌면 개구부가 그 팩 문짝을 따라간다
([../../systems/asset-swap.md](../../systems/asset-swap.md)).

**격자 치수가 시드와 무관하게 고정이던 시절은 끝났다.** 이제 손으로 짓는 기하가 없고 마당까지
전부 모듈이라, **시드가 바꾸는 것이 곧 맵 전체다.**

### 사라진 컬럼 (2026-09-08)

`AisleCount` · `ColCount` · `AisleCountB1` · `ColCountB1` · `AisleWidth` · `ColWidth` ·
`RowDepth` · `OpenWidth` · `PlugWidth` · `TargetOpen1F` · `TargetOpenB1` · `JarCount1F` ·
`JarCountB1`. 개구부 수가 난이도 손잡이였는데, 그 자리를 **모듈 수와 막다른 방 비율**이
대신한다. 항아리 수는 `delivery` 모듈이 몇 개 놓이느냐로 정해진다.

2026-09-07의 격자 확대·미로 밀도 조정 기록은 그 컬럼들과 함께 의미를 잃었다 — 경위는
[../../decisions.md](../../decisions.md)와 git log에 남아 있다.

---

## ★ 목적지마다 생성기가 다르다

맵 생성기는 목적지마다 다르다 — 양계장은 통로와 케이지 열이 격자로 늘어선 구조에 맞춰진
생성기가 필요하고, 벌집은 격자 구조가 달라 **별개의 생성기가 필요하다.**
`Generator` 컬럼이 어떤 생성기를 쓸지 가리킨다 — **목적지 하나 추가 = 맵 생성기 하나.**

레이아웃을 복제하지 않고 시드만 보내는 구조는 생성기가 바뀌어도 그대로다.

---

## 서식 동물

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `MonsterSpawns` | `TArray<FCL9SpawnEntry>` | `{ Alias, MinCount, MaxCount }` (schema.md 정본 필드명) |

양계장 기본값 (**미정 — 아래 참조**):

| 몬스터 | 최소 | 최대 |
|---|---|---|
| `chicken` | 8 | 12 |
| `goose` | 0 | 1 |
| `raccoon` | 0 | 0 |

**이 값은 기준값이고, 일자 곡선이 배율을 곱한다.** 3일차 곡선이 0.8이면 실제로는 6~10마리가
나온다. 상세는 [progression.md](progression.md).

거위를 되살릴 때는 요충지 술어를 통과해야 한다 — 거위가 좁은 길목에 서면 그 길이 막힌다.

---

## 산출 재료

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `ItemSpawns` | `TArray<FCL9SpawnEntry>` | `{ Alias, MinCount, MaxCount }` (schema.md 정본 필드명) |

양계장 기본값:

| 아이템 | 최소 | 최대 |
|---|---|---|
| `egg` | 16 | 20 |
| `meat` | 8 | 12 |

구 CCL8 값은 밤당 28개 고정(계란 18 + 고기 10)이었다 — 그 값을 출발점으로 삼았다.

---

## 진행

| 컬럼 | 타입 | 뜻 | 기본값 |
|---|---|---|---|
| `UnlockDay` | int | 몇 일차부터 갈 수 있나 | 1 (양계장) |
| `BaseDanger` | float | 기본 위험도 — 일자 곡선의 기준 | 1.0 |
| `ValueRate` | float | 가격식의 목적지 배율 ([../economy.md](../economy.md)) | 1.0 (양계장 — 눈금 기준) |
| `TravelDays` | int | 가는 데 걸리는 일수 | 0 |

`TravelDays`가 0보다 크면 이동에 날이 걸린다. **당분간 0으로 둔다** — 먼 목적지가 생기면 켠다.

---

## 밤 시계

| 컬럼 | 뜻 | 기본값 |
|---|---|---|
| `NightLength` | 밤 길이 (초) | 245 |
| `DarkBase` | 정전까지 (초) | 185 |
| `DarkAccel` | 반납 1회가 앞당기는 시간 (초) | 4.0 |
| `DarkMin` | 정전 최소 시각 (초) | 125 |

**비우면 [rules.md](rules.md)의 전역값을 쓴다.** 목적지마다 덮어쓸 수 있다.

---

## 지금 있는 것

| Alias | 상태 |
|---|---|
| `flat` | **테스트 전용** (2026-09-06) — `Generator=None`, 서식·재료는 양계장과 같은 값. 평지 레벨 `Lvl_Flat`의 게임모드 `BP_CL9GameMode_Flat`이 가리킨다. 기획상의 목적지가 아니다 |
| `henhouse` | 격자·개구부 값 전부 있음. **4b 구현(2026-09-05)**: `UCL9SiteDef`(Source/CL9/Data)에 `MonsterSpawns`·`ItemSpawns`·`BaseDanger`·`UnlockDay`. **5a 선행(2026-09-06)**: 격자 컬럼(`Generator`·`AisleCount`…·`TargetOpen1F/B1`·`JarCount1F/B1`, 위 표 값이 C++ 기본값) + 생성기 — [../../systems/map.md](../../systems/map.md) 구현 상태. `ValueRate`는 4e(2026-09-06)에서 들어갔다 — 판매가의 목적지 배율이며 `ACL9GameMode::ComputeSellPrice`가 읽는다. `TravelDays`·밤 시계 덮어쓰기는 아직. 에셋 `DA_SiteTable`(Content/Data) 행 `henhouse` — BP_CL9GameMode `DefaultSiteAlias`가 가리킨다(4g 목적지 선택 전까지) |

---

## 미결

- **몬스터·아이템 개수의 정확한 범위** — 위 표의 8~12 / 16~20은 **초안**이다
- 벌집을 비롯한 다른 목적지 — 재료 목록부터 정해야 하고, 그게 정해져야 주문서가 채워진다
