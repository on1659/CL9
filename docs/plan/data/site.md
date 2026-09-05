# 목적지 — `UCL9SiteDef`

원정을 가는 곳. 양계장 · (나중에) 벌집 · 그 밖.

"스테이지"는 난이도 단계가 아니라 **어디로 가는가**다.
주문서가 필요한 재료를 정하고, 재료가 목적지를 정한다.

---

## 맵 생성

| 컬럼 | 타입 | 뜻 | 양계장 |
|---|---|---|---|
| `Generator` | enum `ECL9Generator` | **어떤 생성기를 쓰나** | `Henhouse` |
| `AisleCount` | int | 통로 수 (1F) | 8 |
| `ColCount` | int | 열 수 (1F) | 6 |
| `AisleCountB1` / `ColCountB1` | int | 지하 격자 | 6 / 4 |
| `AisleWidth` | float | 통로 폭 (cm) | 255 |
| `ColWidth` | float | 열 폭 (cm) | 900 |
| `RowDepth` | float | 케이지 열 깊이 (cm) | 120 |
| `OpenWidth` | float | 케이지 열 관통 개구 (cm) | 360 |
| `PlugWidth` | float | 통로 마개 폭 (cm) | 180 |
| `TargetOpen1F` | int | **1층 개구부 목표 수** | 50 |
| `TargetOpenB1` | int | 지하 개구부 목표 수 | 25 |
| `JarCount1F` / `JarCountB1` | int | 항아리 수 | 4 / 3 |

**개구부 수가 난이도 손잡이다** — 줄이면 막다른 길이 늘고 도망칠 곳이 준다.

**격자 치수는 시드와 무관하게 고정이다.** 시드가 바꾸는 건 미로 내부(케이지·마개·주방·둥지·
개구멍)뿐이고 마당·트럭·계단·축사 외곽은 손으로 짓는다 — 아티스트가 만질 수 있는 영역과
해시에 물린 영역의 경계선이다.

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
| `henhouse` | 격자·개구부 값 전부 있음. **4b 구현(2026-09-05)**: `UCL9SiteDef`(Source/CL9/Data)에 `MonsterSpawns`·`ItemSpawns`·`BaseDanger`·`UnlockDay`. **5a 선행(2026-09-06)**: 격자 컬럼(`Generator`·`AisleCount`…·`TargetOpen1F/B1`·`JarCount1F/B1`, 위 표 값이 C++ 기본값) + 생성기 — [../../systems/map.md](../../systems/map.md) 구현 상태. `ValueRate`·`TravelDays`·밤 시계 덮어쓰기는 아직. 에셋 `DA_SiteTable`(Content/Data) 행 `henhouse` — BP_CL9GameMode `DefaultSiteAlias`가 가리킨다(4g 목적지 선택 전까지) |

---

## 미결

- **몬스터·아이템 개수의 정확한 범위** — 위 표의 8~12 / 16~20은 **초안**이다
- 벌집을 비롯한 다른 목적지 — 재료 목록부터 정해야 하고, 그게 정해져야 주문서가 채워진다
