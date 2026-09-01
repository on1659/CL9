# 닭 — `chicken`

첫 번째 위협이자 **틀을 검증하는 1호 몬스터.** 통로 중앙으로 뭉툭하게 움직인다 —
무엇을 하려는지 읽을 수 있어야 대응이 실력이 된다.

컬럼 정의는 [common.md](common.md).

---

## 체력

| 컬럼 | 값 |
|---|---|
| `HpMin` | 3 (초안) |
| `HpMax` | 4 (초안) |
| `HitStunMs` | 300 (초안) |

## 이동

| 컬럼 | 값 | 구 CCL8 상수 |
|---|---|---|
| `WanderSpeed` | 126 | `ChickenWanderSpeedU 42` |
| `PatrolSpeed` | 195 | `PatrolSpeedU 65` |
| `ChaseSpeed` | 396 | `ChickenChaseSpeedU 132` |
| `TurnRate` | 280 | `ChaseTurnRad` |
| `TurnDrag` | 0.45 | `TurnDrag` |
| `OutdoorChaseMult` | 0.80 | `OutChaseMult` |
| `Radius` | 36 | `ChickenRadiusU 12` |
| `SeparationDist` | 90 | `ChickenSepU 30` |

**닭 추격 396 = 플레이어 걷기 396이다.** 우연이 아니라 의도된 값이다 — 걸어서는 못 도망치고,
달려야 벌어진다. 그리고 지친 빈손 지속 속도 506이 이보다 크다는 게 **"하드락 없음" 보증**이다.

**`TurnDrag` 0.45가 미로를 무기로 만든다** — 90도 코너 하나가 시간을 벌어준다
(구 CCL8 실측 1.38초 — CL9 감속 모델 확정 후 ③에서 재검증).

## 감각

| 컬럼 | 값 | 구 CCL8 상수 |
|---|---|---|
| `SightIn` | 330 | `SightInU 110` |
| `SightOut` | 219 | `SightOutU 73` |
| `SightDark` | 111 | `SightDarkU 37` |
| `HearRadius` | 180 | `HearSpotRU 60` |
| `DetectNearRadius` | 120 | `DetNearU 40` |
| `DetectRateNear` | 2.5 | `DetRateNear` |
| `DetectRateFar` | 0.5 | `DetRateFar` |
| `DetectDecay` | 0.4 | `DetDecay` |
| `DetectGrace` | 0.5 | `DetGrace` |

정전 때 시야 330 → 111.

## 추격

| 컬럼 | 값 | 구 CCL8 상수 |
|---|---|---|
| `ChaseBudget` | 45 | `ChaseBudgetT` |
| `ChaseLoseTime` | 6.0 | `ChaseLoseT` |
| `ChaseBreakTime` | 1.2 | `ChaseBreakT` |
| `ChaseCooldown` | 3.0 | `ChaseCdT` |
| `HomeRange` | 840 | `ChickenLeashU 280` |
| `DeaggroRadius` | 510 | `DeaggroRadiusU 170` |
| `CatchRadius` | 84 | |
| `CatchDamage` | 1 (초안) | |
| `SearchHops` | 3 | `InvestChainN` |
| `SearchDwellTime` | 2.0 | `InvestDwellT` |

**짐을 다 놓으면 510cm에서 풀려난다.**

## 각성과 경보

| 컬럼 | 값 | 구 CCL8 상수 |
|---|---|---|
| `WakeTelegraph` | 0.8 | `ChkWakeTele` |
| `WakeSightRadius` | 900 | `WakeSightR 300` |
| `WakeRetryTime` | 2.0 | `WakeRetryT` |
| `PackAlarmRadius` | 600 | `PackRadiusU 200` |
| `KillAlarmRadius` | 2100 | `KillAlarmRadiusU 700` |
| `bAlertsOthers` | false | |

잠·경보꾼 컬럼(WakeRadius 등 — [goose.md](goose.md) 전용 절)은 닭에서 전부 0/false다.

## 에셋 (보유 — 2026-09-02 확인)

Dreamscape Farmlands 팩이 프로젝트에 들어 있다 (`Content/DreamscapeSeries/DreamscapeFarmlands/Meshes/Chars/Chicken/`):

| 컬럼 | 에셋 |
|---|---|
| `SkeletalMesh` | `SK_Chicken` |
| `Skeleton` | `SKEL_Chicken` (+ `PHYS_Chicken`) |
| `AnimBP` | 미제작 — ③에서 만든다 |

애님 커버리지 (상태 기계 요구 대조 — systems/monster.md · systems/animation.md):

| 상태 기계 요구 | 팩 애님 | 비고 |
|---|---|---|
| 배회·순찰 (Walk) | `A_Chicken_Walk` | ✓ |
| 추격 (Run) | `A_Chicken_Run` | ✓ |
| 평시 (Idle) | `A_Chicken_Idle_Anim` | ✓ |
| 잠듦 | `A_Chicken_IdleSit` | 앉은 루프를 잠으로 사용 (초안) |
| 기상 예고 (WakeTelegraph 0.8s) | `A_Chicken_SitIdle` | 일어나는 전이를 재생 배율로 0.8s에 맞춤 (초안) |
| 잡기 (Catch) | `A_Chicken_Attack` | ✓ |
| 경직 (HitStunMs — 전용 필수) | `A_Chicken_Hurt` | ✓ |
| 죽음 | `A_Chicken_Death` | ✓ |

빠진 것 없음 — 닭은 이 팩만으로 ③ 전 구간을 돈다.

## 가치와 드롭

- `ValueMin`/`ValueMax`: 0 — 시체 판매 없음
- `DropItems`: **미정** — [../item/common.md](../item/common.md)에 있는 것만 연결한다

## 스폰 기준값

양계장 기본 8~12마리 × 일자 곡선 ([../site.md](../site.md) · [../progression.md](../progression.md)).
