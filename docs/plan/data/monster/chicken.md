# 닭 — `chicken`

첫 번째 위협이자 **틀을 검증하는 1호 몬스터.** 통로 중앙으로 뭉툭하게 움직인다 —
무엇을 하려는지 읽을 수 있어야 대응이 실력이 된다.

**층: 동물(`Beast`) — 2026-10-13.** 이 행 하나가 그냥 닭이자 둥지 주인(빈 것)이다 — 개체마다 광폭화 문턱을 굴려서
판 안에 차는 닭이 터지고 안 차는 닭은 판 내내 그냥 닭이다([common.md](common.md) "층"·"광폭화 게이지",
[../../story.md](../../story.md) "위협의 세 층"). 같은 날 앞서 적었던 "빈 것 닭 별도 행"은 번복됐다.
잡히면 HP 한 칸(`CatchDamage` 1)이고 플레이어 HP 0이면 죽는다. 게이지 컬럼 값은 아직 없다.

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
| `SightIn` | **600** (2026-10-17 결정 D, 옛 330) | `SightInU 110` |
| `SightOut` | **400** (2026-10-17, 옛 219) | `SightOutU 73` |
| `SightDark` | 111 | `SightDarkU 37` |
| `HearRadius` | 180 (코드가 읽지 않는 열 — 듣는 거리는 규칙 표 소리 반경이 정한다) | `HearSpotRU 60` |
| `DetectNearRadius` | 120 | `DetNearU 40` |
| `DetectRateNear` | 2.5 | `DetRateNear` |
| `DetectRateFar` | 0.5 | `DetRateFar` |
| `DetectDecay` | 0.4 | `DetDecay` |
| `DetectGrace` | 0.5 | `DetGrace` |

정전 때 시야 600 → 111(`SightDark` 는 그대로).

> **2026-10-21 사용자 "좀 멀리서도 알아봐서 따라오게 해, 리썰 봐 봐 존나 멀리서도 오잖아"** — 위 표 값을 바꿨다(데이터 패널로 저장, 빌드 없음).
> `SightIn` 600 → **2500** · `SightOut` 400 → **1800** · `DetectRateFar` 0.5 → **2.0**(벽 없이 보이면 0.5초에 알아챈다) · `DetectRateNear` 2.5 → **5.0** ·
> `HomeRange` 840 → **6000**(집 끈 8.4m 에서 쫓다 돌아서던 것을 없앰) · `LightLureRadius` 900 → **3500**(빛은 25m 시야보다 더 멀리서 부른다) ·
> `ChaseLoseTime` 6 → **8**. 시야는 여전히 360°·벽 가림(시야선) 기준이다. 정전 `SightDark` 111 은 그대로.

### 덤퍼식 추격 (2026-10-21 사용자 "리썰 덤퍼랑 비슷한 느낌", 명세 [../../../goal/chicken-thumper-chase.md](../../../goal/chicken-thumper-chase.md))

리썰 `CrawlerAI` 를 그대로 옮겼다 — **앞만 본다**(시야 반각 55°, 쫓을 때 65°, 2m 안은 등 뒤도), **0.2초에 알아챈다**, 속도가 **0 에서 320cm/s² 로 올라
1000cm/s(달리기 654 의 1.5배)까지**, 빠를수록 못 꺾는다(회전 800 → 230°/s), **500cm/s 이상으로 달리다 벽에 박으면 속도가 20% 만 남는다**,
**예고 없이 몸으로 부딪쳐 문다**(`PounceWindup` 0), 맞으면 멈췄다가 1초 동안 더 빨리 붙는다, HP 8(프라이팬 네 대), 2초 못 보면 놓친다.
곧은 길에서는 달려도 따라잡히고, **코너를 꺾어 벽에 박게 하거나 시야각 밖으로 빠지는 것**이 피하는 길이다. 위 돌진(웅크림 0.6초) 값은 이것으로 대체됐다.
`DeaggroRadius` 510 → **0** — 빈손이면 5.1m 밖에서 바로 놓아 주던 규칙이 "봐도 안 따라온다"의 큰 원인이었다(PIE 에서 찾음).
2026-10-21 저녁 사용자 "잡기 쉬우면 안 된다 — 닭이 유저를 빨리 찾으면 될 듯": 시야 반각 55° → **90°**(쫓을 때 65° → **100°**), 등 뒤도 아는 거리 2m → **5m**
(`SightNearRadius` 500), 알아채는 데 **0.2~0.33초**(`NoticeScaleMin` 0.25 → 0.6). 정면 반원은 다 보지만 등 뒤 5m 밖은 여전히 못 본다.
열 정의는 [common.md](common.md), 값은 데이터 패널 몬스터 표 chicken 행.

### 달걀 주인 (2026-10-21 사용자, 명세 [../../../goal/egg-thief-hunt.md](../../../goal/egg-thief-hunt.md))

> "닭은 자기 소유의 달걀이 몇 개 있는데, 그걸 누군가 먹으면 얼마나 멀리 떨어져 있든 바로 걔를 타게팅해서 찾으러 간다.
> 최단 경로로는 가지 않아. 하지만 가는 도중에 다른 유저를 만난다면 그 유저한테 어그로가 다시 가."

- 판 시작에 놓인 달걀을 **누가 처음 주우면**(사용자 선택 "주울 때"), 살아 있는 닭 가운데 **무작위 한 마리**(사용자 "랜덤하게, 가장 가깝지 않아도 돼")가
  거리와 상관없이 주운 사람을 찾으러 간다(`OwnedItemAlias` egg). 자고 있으면 깨어나는 예고부터 한다.
- 곧장 오지 않는다 — 도둑 쪽으로 10m(`ThiefDetourStep` 1000)씩, 그 자리 둘레 6m(`ThiefDetourRadius` 600) 안의 아무 지점을 거쳐 비껴 온다.
  걷는 속도 4m/s(`ThiefHuntSpeed` 400 — 플레이어 걷기와 비슷하다). 도둑이 6m 안이면 곧장 간다.
- 가는 길에 **누구든 눈에 띄면 그 사람을 쫓는다**(보통 추격) — 어그로가 옮는다. 그 추격이 끝나면 다시 도둑에게로 간다.
- 도둑을 한 번 물거나 도둑이 죽으면 끝난다. 한 번 주운 달걀(트럭에 넣었다 다시 꺼낸 것, 칸을 바꾼 것)은 다시 깨우지 않는다.
- 주인이 정해지는 때는 줍는 순간이다 — 판 시작에 미리 정해 두지 않으므로, 닭을 먼저 잡아도 남은 닭 가운데 하나가 온다.

### 개체 차이 (2026-10-21 사용자 "두 마리가 똑같이 한 마리처럼 움직인다", 명세 [../../../goal/monster-individual-spread.md](../../../goal/monster-individual-spread.md))

태어날 때 한 마리씩 굴린다 — 속도 **±10%**(`IndividualSpeedSpread` 0.1), 알아채는 데 **0.2~0.33초**(`NoticeScaleMin` 0.6, 처음 0.25),
**왼쪽·오른쪽·정면 중 하나로 돌아서** 온다(`ChaseFlankOffset` 400cm, 15m 밖에서 다 돌고 가까울수록 줄어 붙을 때는 정면 — `ChaseFlankDistance` 1500),
추격 경로를 다시 긋는 간격도 매번 **±30%**(`ChaseRepathJitter` 0.3). 두 마리가 붙으면 하나는 먼저, 하나는 옆에서 늦게 들어온다.

> 2026-10-17 결정 D: "몬스터가 멍청하다"의 절반은 감지 값이 작은 데서 왔다([../../../reference/combat-gap.md](../../../reference/combat-gap.md) 2.1 — 리포 10m·1초).
> `SightIn` 330 → **600**, `SightOut` 219 → **400**(히스테리시스 비 0.66 유지). 걷기 소음도 같은 날 켰다(`CL9Rules.WalkNoiseRadius` 200 — [../rules.md](../rules.md) 소리).
> 회피 산수(A5 후퇴 여유·0cm 여유)는 `PounceRadius`·`PounceWindup`·속도만 보므로 그대로다 — 멀리서 **시작**하는 추격이 늘지만 닭 396 = 걷기 396 이라
> 걸어서 유지하는 약속은 깨지지 않는다. **종별 순서가 뒤집혔다**: 돼지 480·너구리 390 은 D 범위 밖이라 그대로 두었다 — 밸런스 때 다시 본다.

## 추격

| 컬럼 | 값 | 구 CCL8 상수 |
|---|---|---|
| `ChaseBudget` | 45 | `ChaseBudgetT` |
| `ChaseLoseTime` | 6.0 | `ChaseLoseT` |
| `ChaseBreakTime` | 1.2 | `ChaseBreakT` |
| `ChaseCooldown` | 3.0 | `ChaseCdT` |
| `HomeRange` | 840 | `ChickenLeashU 280` |
| `bRehomeOnLeash` | **켬** (2026-10-23 사용자 — 60m 를 넘어 놓으면 그 자리가 새 집) | — |
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

### 싸움 패턴 — 돌진·벽 박힘·도망 (2026-10-20 사용자, 명세 [../../../goal/chicken-fight-pattern.md](../../../goal/chicken-fight-pattern.md))

위 덮침 값(130·110)은 **돌진으로 바뀐다**. 4.5m에서 0.6초 웅크린 뒤 출발 방향을 고정하고 0.4초에 5m를 곧게 달린다 — 뒤로 걸어서는 못 피하고
**옆으로 비켜야** 한다. 비켜서 닭이 벽·기둥에 박히면 1.5초 어지러워 서고, 그동안 맞아도 어지러움이 줄지 않는다(반격 창). 빈 곳으로 빗나가면
0.8초 제자리 회복(작은 반격 창). HP가 1 남으면 쫓지 않고 달아나며, 산 사람 모두 15m 밖이고 아무도 안 보일 때 멈춰 그 자리를 새 집으로 삼는다.
HP는 놓쳐도 회복하지 않는다. 시험장은 `Lvl_Flat` 경기장(`TestArena/`)이다. 값은 전부 초안이고 경기장에서 데이터 패널로 맞춘다.

| 컬럼 | 값 | 뜻 |
|---|---|---|
| `PounceRadius` | 450 (초안, 옛 130) | 돌진을 시작하는 거리 |
| `PounceWindup` | 0.6 | 웅크림(예고) |
| `PounceLunge` | 500 (초안, 옛 110) | 돌진 거리 |
| `PounceMoveTime` | 0.4 (초안, 옛 0 = 즉시 덮침) | 돌진에 걸리는 초 — 1250cm/s |
| `PounceRecoveryTime` | 0.8 (초안) | 빈 곳으로 빗나간 뒤 회복 |
| `PounceMissCooldown` | 1.0 (초안, 옛 0.6) | 다음 돌진까지 |
| `PounceWallStunTime` | 1.5 (초안, 신규) | 벽에 박힌 뒤 어지러움 |
| `FleeHpThreshold` | 1 (초안, 신규) | 이 HP 이하면 도망 — 프라이팬(2)로는 HP 3 닭만, 고기(1)로는 모든 닭이 거친다 |
| `FleeSpeed` | 560 (초안, 신규) | 걷기 396 < 560 < 달리기 654 — 뛰어야 따라잡는다 |
| `FleeSafeDistance` | 1500 (초안, 신규) | 도망 멈춤 거리 |

위 "회피 산수(A5)"는 옛 덮침 값의 것이다 — 돌진은 일부러 뒤로 걷기를 막는다(옆걸음이 답).

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

## 에셋 (자체 제작 — 2026-10-09 확인)

`Content/Monsters/ChickenA/` — 제작 기록은 [../../../asset/monster/chicken_a.md](../../../asset/monster/chicken_a.md), 원본은 `ArtSource/monster/`.

| 컬럼 | 에셋 |
|---|---|
| `SkeletalMesh` | `SK_Chicken_A` |
| `Skeleton` | `SK_Chicken_A_Skeleton` |
| `AnimBP` | `ABP_Chicken_A` (`Content/Monsters/`) |

클립(폴더 실물): `AN_Chicken_Idle` · `AN_Chicken_Walk` · `AN_Chicken_Run` · `AN_Chicken_LookAround` · `AN_Chicken_PounceWindup` · `AN_Chicken_Catch` · `AN_Chicken_HitReact` · `AN_Chicken_PushReact` · `AN_Chicken_WakeTelegraph` · `AN_Chicken_Death`.

Dreamscape Farmlands 팩의 `SK_Chicken`·`A_Chicken_*`는 쓰지 않는다 — 팩은 2026-10-04에 지웠다.

## 가치와 드롭

- `ValueMin`/`ValueMax`: 0 — 시체 판매 없음
- `DropItems`: **미정** — [../item/common.md](../item/common.md)에 있는 것만 연결한다

## 스폰 기준값

양계장 기본 8~12마리 × 일자 곡선 ([../site.md](../site.md) · [../progression.md](../progression.md)).

> **2026-10-23 사용자 "몬스터는 피하는 게임"** — `HitStunMs` 300 → **100**, `KillCooldown`·`RageTime` 9 → **1.5**. 프라이팬 다음 휘두르기(1.08초)를 기다리는 사이에 다시 문다. decisions.md 2026-10-23.
