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
| `PounceRadius` | 130 (초안) | 신규 |
| `PounceWindup` | 0.6 (초안) | 신규 |
| `PounceLunge` | 110 (초안) | 신규 |
| `PounceMissCooldown` | 0.6 (초안) | 신규 |
| `SearchHops` | 3 | `InvestChainN` |
| `SearchDwellTime` | 2.0 | `InvestDwellT` |

**짐을 다 놓으면 510cm에서 풀려난다.**

### 잡기 예고 값의 근거 (2026-09-20, 전부 초안 — PIE C3-예고 관찰 전)

사용자 결정 D2 "걷기로 물러나도 피한다"([../../../goal/monster-pounce-hurt-readability.md](../../../goal/monster-pounce-hurt-readability.md)
A 절)를 숫자로 옮긴 값이다. 닭이 **기준 종**이고 다른 잡는 종은 이 값의 비율로 잡는다.

| 값 | 근거 |
|---|---|
| `PounceWindup` 0.6 | 사람 반응 0.25초 + 방향 전환 0.15초 + 여유 0.2초. 이상적 후퇴 소요는 `(CatchRadius + PounceLunge − PounceRadius) / WalkSpeed` = 64/396 = **0.16초**라 반응을 더해도 0.41초 < 0.6 — 여유 0.19초는 가속·복제 지연·장애물 몫이다. 종에 붙는 값이 아니라 **사람 쪽 상수**라 다른 종도 0.6을 쓴다 |
| `PounceRadius` 130 | `CatchRadius` 84 + 46 — 잡히는 거리보다 닭 한 걸음(≈ 추격 속도 396 × 0.12초) 먼저 멈춰 선다. 로드 검사 `PounceRadius >= CatchRadius` 여유 46 |
| `PounceLunge` 110 | 추격 속도 396 × 0.28초 — 예고 없이 추격했다면 0.28초에 갔을 거리를 한 번에 덮친다. `WalkSpeed × PounceWindup` = 396 × 0.6 = 238 > `CatchRadius + PounceLunge` = 194 이므로 **어느 거리에서 예고가 시작되든**(재장전 중에 붙어 있다 0cm에서 예고해도) 걸어서 물러나면 헛침 — 여유 44cm |
| `PounceMissCooldown` 0.6 | 예고 길이와 같게 — 헛친 뒤 최소 1.2초(예고 0.6 + 휴지 0.6) 주기로 다시 예고한다. 걸어서 물러나는 사람에겐 헛친 뒤 간격이 ≈ 258cm(368 − 110)라 닭(396 = 걷기 396)은 다시 못 좁힌다 — 재예고는 사람이 멈추거나 돌 때만 |

회피 산수(A5, [../../economy.md](../../economy.md)): 예고 진입 130cm에서 —
뒤로 걷기 130 + 238 = **368 > 194** 헛침(여유 174) · 옆걸음 √(130² + 238²) = **271 > 194** 헛침 ·
제자리 130 ≤ 194 잡힘. 캐릭터 가속은 표에 없어 산수는 즉시 가속 가정이다 — 실측은 PIE
C3-예고 ②③([../../../testing/pie-scenarios.md](../../../testing/pie-scenarios.md)).

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
