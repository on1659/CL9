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
| `bEmitsLight` | true | |
| `LightRadius` | 192 | `LanternDistU 64` |
| `bLightLostInPocket` | true | |
| `CarrierSeenRadius` | 276 | `CarrierSeenU 92` |

정전 때 계란을 든 사람은 276cm에서 몬스터에게 팔린다 ([../monster/common.md](../monster/common.md)).

## 가치와 요리

| Alias | SubAlias | 무엇 | 가격 (이더) |
|---|---|---|---|
| `egg` | — | 계란 | **80~120 — 눈금 기준 (배율 1.0)** |
| `egg` | `cooked` | 익은 계란 | 150~200 (초안) |
| `egg` | `burnt` | 탄 계란 | 0 |

- 요리 체인: `계란 → (CookTime 12초) → 익은 계란 → (BurnTime 24초) → 탄 계란`
- `bWorthlessIfBroken` = true, `bWorthlessIfBurnt` = true
- 나머지 모든 가격은 계란에서 역산한다 ([../../economy.md](../../economy.md))

## 미결

- `Weight` 값 (무게→감속 식과 함께 — common.md 미결)
