# 계란 — `egg`

첫 번째 재료이자 **모든 눈금의 기준.** 깨지고, 빛나고, 익는다 — 이 게임의 마찰 셋이 다 들어 있다.

컬럼 정의는 [common.md](common.md).

---

## 물리 (깨진다)

| 컬럼 | 값 | 구 CCL8 상수 |
|---|---|---|
| `Radius` | 21 | `EggRadiusU 7` |
| `bFragile` | true | |
| `CrackSpeed` | 450 | `CrackSpdU 150` |
| `BreakSpeed` | 1260 | `BreakSpdU 420` |
| `CrackedBreakSpeed` | 750 | `CrackedBreakU 250` |
| `Gravity` | 1950 | `EggGravityU 650` |
| `Restitution` | 0.34 | `EggRestitution` |
| `Friction` | 0.62 | `EggFriction` |

`450 < 750 < 1260` — 금 가면 더 약해진다 (세 문턱 순서 — 로드 검사가 확인).

## 빛 (들면 빛난다)

| 컬럼 | 값 | 구 CCL8 상수 |
|---|---|---|
| ~~`bEmitsLight`~~ | **없음** — 2026-09-11 발광 폐기(빛은 손전등만), 컬럼도 제거 | |
| ~~`LightRadius`~~ | 192 → **없음** — 2026-10-16 정정: 빛 수치는 아이템 행 `LampKind` 한 칸으로 옮겼다(`CL9ItemDef.h`; 값은 룩 표 램프 종류 [../look.md](../look.md) §3) | `LanternDistU 64` |
| ~~`bLightLostInPocket`~~ | **없음** — 2026-10-17 컬럼 제거 ([common.md](common.md)) | |
| `CarrierSeenRadius` | 276 | `CarrierSeenU 92` |

정전 때 계란을 든 사람은 276cm에서 몬스터에게 팔린다 ([../monster/common.md](../monster/common.md)).

## 가치와 요리

| Alias | SubAlias | 무엇 | 가격 (이더) |
|---|---|---|---|
| `egg` | — | 계란 | **10~15 — 눈금 기준 (배율 1.0)** (2026-10-17 눈금 1/8, 옛 80~120 — [../../economy.md](../../economy.md)) |
| `egg` | `cooked` | 익은 계란 | 19~25 (초안, 옛 150~200) |
| `egg` | `burnt` | 탄 계란 | 0 |

- 요리 체인: `계란 → (CookTime 12초) → 익은 계란 → (BurnTime 24초) → 탄 계란`
- `bWorthlessIfBroken` = true, `bWorthlessIfBurnt` = true

## 몸체 (익으면 계란후라이 — 2026-10-09)

| Alias | SubAlias | `bMeshCollisionBody` | `Mass` | 몸체 |
|---|---|---|---|---|
| `egg` | — · `cracked` · `broken` | false | 0 | 반경 구(`Radius` 21) |
| `egg` | `cooked` | **true** | 0 (엔진 계산) | `SM_Egg_Cooked_A`의 UCX 원반 1개 — 상판·바닥에 납작하게 눕는다 |
| `egg` | `burnt` | **true** | 0 (엔진 계산) | `SM_Egg_Burnt_A`의 UCX 원반 1개 |

- 익는 순간(상판 위) 몸체가 구에서 원반으로 넘어간다 — 원반 밑면이 구 밑면 자리에 놓여 튀거나 상판을 뚫지 않는다([../../../systems/cooking.md](../../../systems/cooking.md)).
- `Mass` 0은 엔진이 원반 부피로 정한 값이다(날계란의 구도 엔진 계산이다). 무게를 맞출지는 balance가 정한다 — 미결.
- 나머지 모든 가격은 계란에서 역산한다 ([../../economy.md](../../economy.md))

## 미결

- `Weight` 값 (무게→감속 식과 함께 — common.md 미결)
- 익은·탄 계란의 `Mass` — 엔진 계산(0)으로 둘지, 날계란 구의 엔진 질량과 맞출지 (balance)
