# 몬스터 공통 — `UCL9MonsterDef`

재료의 주인이자 위협의 **컬럼 정의**. 개별 몬스터의 값은 이 폴더의 각 문서에 있다 —
[chicken.md](chicken.md) · [goose.md](goose.md) · [raccoon.md](raccoon.md) · [pig.md](pig.md) · [sheep.md](sheep.md).

공통 컬럼은 [../README.md](../README.md) 참조. 몬스터는 `SkeletalMesh`·`Skeleton`·`AnimBP`를
추가로 갖는다 (베이스 `Mesh`는 아이템용 StaticMesh라 별도 필드 — character.md와 동일).

여기에 `MeshYawOffset`(float, 도)이 하나 더 붙는다. **동물 팩마다 메시가 내보내는 정면 축이
달라서**, 코드로 종을 특판하지 않고 데이터로 보정한다 ([../../../systems/animation.md](../../../systems/animation.md)
원칙 1). 값이 틀리면 몬스터가 이동 방향을 보지 않고 옆으로 미끄러지듯 걷는다. 같은 팩에서 온
종은 같은 값을 쓴다 — 돼지·양이 둘 다 −90이다.

---

## 층 (2026-10-13 사용자 결정 — 컬럼은 초안, 구현 전)

한 곳의 위협은 세 층이고, 행마다 어느 층인지가 있어야 한다 — [../../story.md](../../story.md) "위협의 세 층"이 정본이다.

**같은 날 뒤에 번복 — 그냥 동물과 빈 것은 행이 하나다.** 2026-10-13 사용자: "닭들이 광폭화 게이지가 꽉 차면 괴물로 변신한다.
광폭화 전이랑 후랑 다른 것뿐. 닭마다 게이지 기준이 달라서 어떤 닭은 금방 변하고 어떤 닭은 이번 게임 내내 변신을 못 한다."
그래서 행은 두 종류다. 이야기의 세 층은 그대로이고, 그냥 동물과 빈 것은 **같은 행의 광폭화 전·후**다.

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `Tier` | enum `ECL9MonsterTier` | `Beast`(그 곳의 동물 — 탈을 쓴 것이 섞여 있다, **게이지 있음**) · `Monster`(탈 없는 일반 몬스터, **게이지 없음**) |

| 값 | 어느 행 | 규칙 |
|---|---|---|
| `Beast` | `chicken` · `pig` | 많다. 잡으면 HP를 깎고 0이면 죽는다(돼지는 잡으면 처형 — 그 행의 값). 개체마다 광폭화 문턱을 굴리고, 문턱이 판 안에 닿으면 터진다(빈 것), 안 닿으면 그냥 동물이다. 실밥은 모델에 있고 기록은 그 곳의 소품이다. **판 중간 스폰(7e)은 이 층을 뽑는다** |
| `Monster` | `goose` · `raccoon` | 한 판 0~1. 사연·게이지·광폭화 없음. 종마다의 규칙(경보·스토킹·처형) 그대로. 7e가 안 뽑는다 |

세 층은 곳마다 다 있다 — 양계장은 닭 행 하나(그중 몇 마리가 터진다), 돼지우리는 돼지 행 하나. **양(`sheep`)은 뺀다**(2026-10-13 사용자) —
어느 층에도 없고, 행·레벨 배치·문서 정리는 별도 작업이다.
스폰 표([../site.md](../site.md) `MonsterSpawns`·구역)는 그대로 행 단위다 — 빈 것 수를 따로 적지 않고 문턱 분포가 정한다.

### 광폭화 게이지 — 컬럼 초안 (구현 전, 값은 밸런스 때)

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `RageThresholdMin` | float | 개체가 굴리는 문턱의 하한. 0이면 이 종은 게이지가 없다(`Monster`) |
| `RageThresholdMax` | float | 〃 상한. 판 안에 닿을 수 없을 만큼 크면 "이번 판 내내 못 터지는" 개체가 나온다 |
| `RageGainPerEvent` | float | 그 곳의 광폭화 조건 사건 하나가 채우는 양 (닭: 둥지 가까이서 알이 깨지는 소리 — 조건 자체는 story.md 틀의 "광폭화 조건") |
| `RageDecayPerSec` | float | 초당 줄어드는 양. 0이면 안 줄어든다 |
| 광폭화 후 값 | — | 속도·시야·치명 여부 등 — story.md "이 스토리가 기존 기획에 요구하는 것" 첫 행. 컬럼은 그때 정한다 |

HP처럼 **문턱은 개체 스폰 때 `Min~Max`에서 굴린 값**이다. 행 값이 같아도 개체마다 다르게 터진다.

## 체력

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `HpMin` | int | 최소 체력 |
| `HpMax` | int | 최대 체력 |
| `HitStunMs` | int | 죽지 않는 타격의 경직 (1000 = 1초). 넉백(아이템 행 `HitKnockback`)은 히트스톱(`CL9Rules.HitStopTime`) 뒤 `CL9Rules.HitKnockbackTime` 동안 밀리고 경직 안에서 끝난다 — `HitStopTime + HitKnockbackTime ≤ HitStunMs` (2026-09-22) |

HP는 `HpMin`~`HpMax`에서 굴린 정수다. 즉사를 피하고 싶은 종은 HP 값으로 조절한다 —
원칙이 아니라 값의 문제다. (일자별 HP 배율 곡선은 현재 progression.md에 없다 — 필요해지면 추가.)

체력이 0이 되면 죽고 죽는 모션을 재생한다 (기본 — 래그돌 등 다른 연출은 종별 선택).
죽지 않는 타격은 `HitStunMs`만큼 경직하고, 경직에는 전용 애니메이션이 따로 있어야 한다.

**HP는 몬스터 액터의 Replicated `int32` 필드다** (호스트에서만 수정 —
[../../../conventions.md](../../../conventions.md) § 4) — `TakeDamage`·`UDamageType` 대신
단순 HP 값을 쓴다 (conventions.md § 6).

---

## 이동

| 컬럼 | 뜻 |
|---|---|
| `WanderSpeed` | 배회 (cm/s) |
| `PatrolSpeed` | 순찰 (cm/s) |
| `ChaseSpeed` | 추격 (cm/s) |
| `RetreatSpeed` | 물러날 때 (cm/s) |
| `TurnRate` | 회전 (도/초) |
| `TurnDrag` | 꺾을수록 느려지는 정도 |
| `OutdoorChaseMult` | 마당에서의 배율 |
| `Radius` | 몸 반경 (cm) |
| `SeparationDist` | 서로 벌어지는 거리 (cm) |

**`TurnDrag`가 미로를 무기로 만든다** — 90도 코너 하나가 시간을 벌어준다.

---

## 감각

| 컬럼 | 뜻 |
|---|---|
| `SightIn` | 발견 거리 (cm) |
| `SightOut` | 놓치는 거리 (cm) |
| `SightDark` | 정전 때 시야 (cm) |
| `HearRadius` | 소리 들리는 거리 (cm) |
| `DetectNearRadius` | 근접 판정 거리 (cm) |
| `DetectRateNear` | 근접 시 게이지 상승 (/초) |
| `DetectRateFar` | 원거리 게이지 상승 (/초) |
| `DetectDecay` | 게이지 하강 (/초) |
| `DetectGrace` | 시야에서 벗어난 뒤 유예 (초) |

**`SightIn` > `SightOut`이 히스테리시스다.** 경계선에서 발견과 해제가 깜빡이지 않게 한다.

정전 때는 몬스터 시야도 줄지만, 발광 중인 소지자는 `CarrierSeenRadius`에서 팔린다 — 다만 **발광
아이템은 2026-09-11에 폐기**돼(빛은 손전등·헤드램프에서만) 지금 이 값을 쓰는 행이 없다
([../item/common.md](../item/common.md) 소리와 빛).

---

## 추격

| 컬럼 | 뜻 |
|---|---|
| `ChaseBudget` | 추격을 유지하는 한계 (초) |
| `ChaseLoseTime` | 시야를 잃고 포기까지 (초) |
| `ChaseBreakTime` | 포기하고 서서 두리번 (초) |
| `ChaseCooldown` | 포기 후 재추격 금지 (초) |
| `HomeRange` | 집에서 멀어질 수 있는 한계 (cm) |
| `bRehomeOnLeash` | 켜면 `HomeRange` 를 넘어 추격을 놓을 때 옛 집으로 돌아가지 않고 그 자리를 새 집으로 삼는다 (닭만 켬, 2026-10-23) |
| `DeaggroRadius` | 짐을 다 놓았을 때 풀리는 거리 (cm) |
| `CatchRadius` | 잡는 반경 (cm) — 0이면 잡지 않는 종 |
| `CatchDamage` | 잡았을 때 플레이어 타격량 (HP) — 즉사 여부는 이 값과 HP가 정한다 |
| `PounceRadius` | 잡기 예고가 시작되는 거리 (cm) — 이 안에서 멈춰 서서 예고한다 |
| `PounceWindup` | 예고 길이 (초) — 이동 0, 타깃을 향해 `TurnRate`로 돌며 기다린다. **0이면 예고 없음** |
| `PounceLunge` | 덮치는 거리 (cm) — 벽이면 거기까지. 기본 즉시 이동 또는 아래 시간 이동 |
| `PounceMoveTime` | 0: 기존 즉시 덮침. 양수: 예고 뒤 이 시간 동안 연속 이동, 출발 방향 고정, 매 스텝 스윕/명중 검사 |
| `PounceRecoveryTime` | 시간 이동이 빗나간 뒤 Pounce에서 제자리 복귀하는 시간. 명중 뒤 Catch 휴지기도 최소 이 시간 |
| `PounceMissCooldown` | 헛친 뒤 다시 예고할 수 있기까지 (초) — `KillCooldown`과 무관 |
| `SearchHops` | 수색 홉 수 |
| `SearchDwellTime` | 한 지점에서 머무는 시간 (초) |

**`ChaseCooldown`은 새로 경계 상태에 들어가는 것만 막는다.** 정전·복수·기절에서 깨어나는 건
못 막는다 — 쿨다운이 만능 방패가 되면 안 된다.

### 잡기 예고 (Pounce, 2026-09-20)

2026-10-17: `PounceMoveTime > 0`인 행은 예고 끝의 **몸 방향**을 고정해 연속 이동한다. 타깃을 계속 쫓아 방향을 꺾지 않으며 이동 중 매 스텝 명중을 검사한다. 빗나가면 `PounceRecoveryTime` 동안 정지한 뒤 Chase로 복귀한다. 소 Cow/닭 revealed_attack 검수 행만 사용하며 기본 0인 기존 행은 아래 즉시 덮침 동작을 유지한다. 새 두 열도 유한·비음수 검사.

잡기는 예고 뒤에 온다 ([../../../systems/monster.md](../../../systems/monster.md) 잡기 예고):
추격 중 `Dist2D <= PounceRadius`이고 `KillCooldown`·`PounceMissCooldown`이 둘 다 다 닳았으면
`Pounce` 상태로 들어가 `PounceWindup` 동안 제자리에서 타깃을 향해 돈 뒤, 타깃의 **지금** 위치
방향으로 `PounceLunge`만큼 덮치고(몸체 구 스윕 — 벽·문이면 거기까지) **도달 위치에서 재검사**한다
— `Dist2D <= CatchRadius`이고 높이 차가 몸체 반경 + 캡슐 반높이 안이고 시야가 막히지 않고 타깃이
넘어져 있지 않으면 잡힘, 아니면 **헛침**(`PounceMissCooldown` 뒤 재예고 가능, `KillCooldown`은
그대로). 예고 동안 추격 예산·놓침 시계는 계속 흐른다(Pounce는 Chase의 부분 상태).

- **`CatchRadius` 0 또는 `PounceWindup` 0이면 Pounce 없음** — 거위처럼 잡지 않는 종, 또는 예고를
  끈 종은 옛 규칙(`CatchRadius` 안에서 즉시 잡기) 그대로다. 네 열은 기본값이 전부 0이라 행을 채우지
  않은 종은 예고 없이 동작한다.
- 로드 검사: 네 열 전부 **유한·비음수**. 예고가 켜진 종(`CatchRadius > 0 ∧ PounceWindup > 0`)은
  **`PounceRadius >= CatchRadius`** — 아니면 예고 없이 잡히는 거리가 생긴다.
- 종별 값은 각 문서(닭이 기준, 나머지는 닭 비율 — balance). 비율 규칙: `PounceLunge` ∝ `ChaseSpeed`(닭 110/396),
  `PounceRadius` = `CatchRadius + PounceLunge − 64`(이상적 후퇴 거리 64cm), `PounceWindup`·`PounceMissCooldown`은
  종 공통 0.6 — balance 초안.

---

## 스토킹과 처형 (Execute, 2026-09-20)

Category `Execute`. 처형에는 두 갈래가 있고 표 값으로만 갈린다(2026-09-20 저녁 사용자 결정) — **스토커**
(`ExecuteRadius > 0`: 몰래 뒤에서 처형, 값은 [raccoon.md](raccoon.md))와 **잡히면 처형**(`bExecuteOnCatch`:
추격·예고·잡기 그대로, 잡는 순간이 처형, 값은 [pig.md](pig.md)). 둘 다 아니면(`ExecuteRadius` 0 ∧
`bExecuteOnCatch` false) **처형 없는 종** — 나머지 열은 읽히지 않고 옛 추격·예고·잡기 그대로다(빌드 없는 롤백).
기본값은 전부 0/None/Zero/false. 값은 balance.

| 컬럼 | 뜻 |
|---|---|
| `ExecuteRadius` | 스토킹 처형 진입 거리 (cm) — **0 = 스토킹 없음**(Stalk 도 없다; `bExecuteOnCatch` 도 아니면 처형 없음) |
| `ExecuteBehindAngle` | 타깃 등 뒤 원뿔의 반각 (도) — 타깃 전방 · (타깃→몬스터) ≤ −cos(이 값)이면 등 뒤 |
| `ExecuteUnseenTime` | 아무도 보지 않은 채 이어져야 하는 시간 (초) |
| `ExecuteDuration` | 처형 진입부터 즉사(`HostKill`)까지 (초) — 클립이 이 값을 따른다(규칙 2) |
| `ExecuteAnchorDistance` | 앵커 = 타깃 위치 − 타깃 전방(요) × 이 값 (cm) — 로드 검사 `≥ Radius + 39`(캡슐 반경) |
| `ExecuteCameraSocket` | 처형 카메라를 붙일 메시 소켓(본 이름 허용). **None이면 액터 루트(구 콜리전)에 붙는다** — 메시는 `MeshYawOffset`만큼 돌고 `Radius`만큼 내려가 있어 소켓 공간과 액터 공간이 다르다(2026-09-20 밤: `head` 소켓안은 "뒷다리 사이만 보임"으로 폐기, 지금 셀은 두 종 다 None) |
| `ExecuteCameraOffset` | 처형 카메라 위치 (cm) — 소켓이 있으면 소켓 공간, 없으면 **액터 공간(X = 전방·타깃 쪽, Z = 위)** |
| `ExecuteCameraRotation` | 처형 카메라 회전 (도) — 같은 기준 공간 |
| `StalkSpeed` | 안 보일 때 타깃 등 뒤로 다가가는 속도 (cm/s) |
| `StalkGazeHalfAngle` | "본다" 판정의 시선 반각 (도) — 플레이어 조준 요와 몬스터 방향 요의 차가 이 안이면 본다 |
| `StalkGazeRadius` | "본다" 판정이 유효한 거리 (cm) |
| `StalkStareAnger` | 이만큼 응시당하면 격분해 Chase (초) — 안 보이면 같은 속도로 0 까지 감쇠 |
| `StalkRetreatRadius` | 보이는 동안 타깃과 이 거리 안이면 물러난다 (cm) — 속도는 기존 `RetreatSpeed`. 0 이면 정지만 |
| `bExecuteOnCatch` | 잡는 순간(즉시 잡기·덮침 성공)이 처형이다 — 스토킹 없이, `CatchDamage`·`Catch` 휴지기 대신 잡은 자리에서(텔레포트 없음) `Execute` 로. `ExecuteDuration` 필요. 기본 false — 값은 [pig.md](pig.md)(잡히면 처형)·[raccoon.md](raccoon.md)(스토커) |

규칙 표 `CL9Rules.ExecuteCameraBlend`(초, 0 = 즉시): 피해자 시점이 처형 카메라로 넘어가는 블렌드
([../rules.md](../rules.md)).

- **진입** (스토커): 감각이 타깃을 잡아 `Chase` 로 가던 자리(Wander/Patrol/Return/Investigate)에서
  `Stalk` 로 간다. `Chase` 는 **격분 뒤에만** — 응시 `StalkStareAnger`, 피격(Stun 뒤), 죽음의 비명(Rage),
  무리 경보.
- **잡히면 처형** (`bExecuteOnCatch`): 사냥은 닭과 같다 — `Chase`·예고·덮침·즉시 잡기, `CatchRadius`·`KillCooldown`
  그대로(`CatchRadius` 0 이면 잡지 못하니 처형도 없다). 잡기가 성사되는 순간 `CatchDamage`·`Catch` 휴지기 대신
  그 자리에서 처형에 들어간다 — 텔레포트 없음, 아래 "처형" 흐름은 같다. 넘어진·처형 중 타깃은 잡기 자체가
  시작되지 않는다(기존 규칙). 스토킹 열(`Stalk*`·`ExecuteRadius`·`ExecuteBehindAngle`·`ExecuteUnseenTime`·
  `ExecuteAnchorDistance`)은 읽히지 않는다.
- **본다 판정** (호스트, 매 틱): 살아 있고 처형 중 아닌 **어느 플레이어든** `StalkGazeRadius` 안에서
  `|NormalizeAxis(조준 요 − (몬스터−플레이어) 방향 요)| ≤ StalkGazeHalfAngle` ∧ 시야 트레이스 안 막힘.
  2D 요만 본다 — 바닥을 보든 천장을 보든 화면 가로 안이면 본 것이다. 원격 폰의 조준 요는 복제 지연(~100ms)
  그대로다.
- **보이면** 정지. 타깃이 `StalkRetreatRadius` 안이면 타깃 반대쪽의 그 반경 지점으로 `RetreatSpeed` 로
  물러난다(경로 없으면 정지). 응시 계수기가 쌓이고 미발견 계수기는 0.
- **안 보이면** 타깃 등 뒤 지점(위치 − 전방 × `ExecuteAnchorDistance`)으로 `StalkSpeed` 로 간다(경로 실패면
  타깃 자체로). 미발견이 쌓이고 응시는 같은 속도로 감쇠. 넘어진 타깃이면 서서 기다린다(계수기는 흐른다).
- **계수기는 타깃별** — 감각이 다른 산 사람을 잡으면 타깃을 옮기고 둘 다 0 부터.
- **처형 진입** (전부 ∧): 안 보임 ∧ 미발견 ≥ `ExecuteUnseenTime` ∧ Dist2D ≤ `ExecuteRadius` ∧ 등 뒤 원뿔 ∧
  시야 ∧ `KillCooldown` 다 닳음 ∧ 타깃이 넘어지지 않았고·처형 중 아니고·살아 있음 ∧ **앵커 스윕 통과**
  (몸체 구를 타깃 자리에서 앵커까지 WorldStatic|WorldDynamic 로 스윕, 캐릭터·아이템·몬스터·타깃의 수레 무시 —
  막히면 그 틱은 스토킹 계속).
- **처형**: 스토커는 앵커로 텔레포트해, 잡히면 처형인 종은 잡은 자리에서 타깃을 향한다(피해자 캡슐·위치는 그대로). 진입 순간 피해자는 수레를 놓고
  이동·시선·조작이 잠기며 시점이 처형 카메라로 넘어간다. `ExecuteDuration` 뒤 HP 와 무관하게 즉사(짐 흘림·
  넘어짐 ∞·관전 흐름은 보통 죽음과 같다) + `KillCooldown` 재장전. 죽인 뒤에도 피해자의 관전이 시작될 때까지
  (최대 `ExecuteDuration + SpectateDelay`) 머물다 `Break`.
- **구조 없음**: 처형 중에는 맞아도 피해만 받고 경직·넉백·상태 변경이 없다. 처형 중 죽으면 피해자도 죽는다.
- **롤백**: `ExecuteRadius` 0 ∧ `bExecuteOnCatch` false → 그 종은 옛 추격·예고·잡기 종으로 돌아간다(빌드 없음).
  돼지는 `bExecuteOnCatch` false 만으로 보통 잡기(`CatchDamage`)다.
- **로드 검사** (`UCL9MonsterDef::Validate`, Error): 열 전부 유한·비음수; `ExecuteRadius > 0 ⇒ ExecuteDuration > 0 ∧
  StalkSpeed > 0 ∧ StalkGazeRadius > 0 ∧ ExecuteAnchorDistance ≥ Radius + 39`; `bExecuteOnCatch ⇒ ExecuteDuration > 0`;
  `StalkRetreatRadius > 0 ⇒ RetreatSpeed > 0`.
- 코드: [../../../systems/monster.md](../../../systems/monster.md) 스토킹과 처형 ·
  [../../../systems/character.md](../../../systems/character.md) 처형.

---

## 핵심 행동 (2026-10-20 사용자 결정)

사용자 결정(2026-10-20): "초안에 집착하지 말고, 기믹을 정해서 딱 하나의 핵심 행동 패턴만 만들어서 몬스터를 집어넣자." 그래서 종마다
행동을 하나만 둔다. `docs/plan/draft/monster*.md` 초안의 보조 규칙(광폭화·문 밖 추격·격분 등)은 이 표에 들지 않는다.
제작 1묶음 10종 가운데 모델이 있는 6종을 먼저 넣고, 어미 닭·횃대 닭·토끼·쥐는 모델이 나온 뒤에 넣는다(같은 날 사용자 결정).

| 종 | 핵심 행동 | 플레이어가 할 일 | 켜는 열 |
|---|---|---|---|
| 닭 | 켜진 빛에 몰려든다 — 평소처럼 보고 듣는 데 더해, 빛을 켠 사람은 시야 밖에서도 알아채고 쫓는다. 아무도 못 느끼면 바닥에 켜 둔 빛으로 모여 맴돈다 (2026-10-20 오후 사용자: "닭이 나를 봐도 아무 반응이 없어" → 눈·귀를 되살림) | 닭 곁에서는 불을 끈다. 바닥에 켠 빛을 두면 미끼가 된다 | `LightLureRadius` |
| 돼지 | 먹을 것을 든 사람만 노리고, 잡으면 처형한다 | 짐을 내려놓거나 던진다 | `bTargetCarriersOnly` (+ 기존 `bExecuteOnCatch`) |
| 소 | 누가 보고 있으면 움직이지 않고, 아무도 안 볼 때만 다가온다 | 눈을 떼지 않는다 | `bFreezeWhenWatched` |
| 거위 | 자다가 가까이서 걷거나 뛰면 깨서 울고 경보를 낸다 | 깨우지 않고 돌아간다 | 기존 거위 열 그대로 |
| 너구리 | 아무도 안 보는 사람의 등 뒤로 와서 처형한다 | 자주 돌아본다 | 기존 스토킹 열, `StalkStareAnger` 0 (격분 없음) |
| 양 | 혼자 떨어진 사람만 노린다 | 혼자 다니지 않는다 | `IsolationRadius` |

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `LightLureRadius` | float, cm | 0 보다 크면 평소 눈·귀(`SightIn`·행동 소음)에 더해, 이 안에서 든 손전등·쓴 헤드램프·주머니 빛(비활성 칸의 켜진 손전등)을 켠 사람을 시야 거리 밖이어도 시야선만 있으면 "보인" 것으로 친다. 시야 밖에서 불을 끄면 놓침 시계(`ChaseLoseTime`)가 흐른다. 사람을 못 느끼면 바닥에 켜 둔 빛(`ACL9LightItem`)으로 가서 맴돈다 — 탄 요리 미끼 대신이다. 0 = 끔 |
| `bTargetCarriersOnly` | bool | 재료·요리(`ItemType` ≠ `Tool`)를 손이나 퀵슬롯에 든 사람만 느낀다. 추격 중인 사람이 먹을 것을 다 내려놓으면 거리와 무관하게 바로 놓는다(`DeaggroRadius`보다 앞선다) |
| `IsolationRadius` | float, cm | 0 보다 크면 이 안에 다른 산 사람이 없는 사람만 느끼고, 추격 중인 사람 곁에 동료가 들어오면 바로 놓는다. 0 = 끔 |
| `bFreezeWhenWatched` | bool | 누가 보고 있는 동안(시선 판정 = `StalkGazeHalfAngle`·`StalkGazeRadius`, 스토킹과 같은 판정) 배회·조사·추격·덮치기 예고의 틱을 건너뛴다 — 이동과 시계가 다 멈춘다. 로드 검사: 켜면 두 시선 열이 0 보다 커야 한다 |
| `PounceWallStunTime` | float, s | **싸움 패턴(2026-10-20, [../../../goal/chicken-fight-pattern.md](../../../goal/chicken-fight-pattern.md))**. 시간형 덮침(`PounceMoveTime` > 0)이 사람을 못 잡고 벽에 막히면 이 초 동안 경직(밀침 클립) — 반격 창. 경직 중 맞아도 남은 시간을 깎지 않는다. 0 = 벽에서 그냥 멈춤 |
| `FleeHpThreshold` | int | HP가 이 값 이하면 사냥 상태(추격·스토킹·조사·덮치기)로 들어가려 할 때 `EnterState`가 도망으로 돌린다. 도망은 Return 상태의 부분 모드다(새 상태 값 없음 — ABP 가지가 없다). HP는 회복하지 않는다. 0 = 끔. 로드 검사: 켜면 아래 둘이 0 보다 커야 한다 |
| `FleeSpeed` | float, cm/s | 도망 속도 |
| `FleeSafeDistance` | float, cm | 산 사람 모두가 이 거리 밖이고 아무도 시야선이 없으면 도망을 멈추고 그 자리를 새 집(`HomeLocation`)으로 삼는다 |
| `ChaseRampRate` · `ChaseMaxSpeed` | float, cm/s² · cm/s | **덤퍼식 추격(2026-10-21, [../../../goal/chicken-thumper-chase.md](../../../goal/chicken-thumper-chase.md))**. 추격 속도가 0 에서 이 빠르기로 상한까지 오른다. 추격 시작·경직·잡은 뒤 0 으로. 0 = `ChaseSpeed` 등속. A1 부등식은 `ChaseSpeed` 만 보므로 이 상한에는 걸리지 않는다(덤퍼는 일부러 달리기보다 빠르다) |
| `ChaseHitRampBonus` | float, cm/s² | 맞은 뒤 1초 동안 가속에 더한다 |
| `TurnRateAtMaxSpeed` | float, °/s | 추격 중 속도에 비례해 `TurnRate` 에서 이 값으로 줄어든다 — 빠를수록 코너를 못 돈다. 0 = `TurnRate` 고정 |
| `ChaseWallSlamSpeed` · `ChaseWallSlamKeep` · `ChaseWallStunTime` | cm/s · 0~1 · s | 이 속도 이상으로 달리다 벽(몬스터·사람 아닌 것)에 막히면 속도가 `Keep` 배만 남고, `StunTime` > 0 이면 경직 |
| `SightHalfAngle` · `ChaseSightHalfAngle` · `SightNearRadius` | ° · ° · cm | 시야각(평소·추격 중). `SightNearRadius` 안은 각도 무시. 켜진 빛(빛 끌림)과 소리는 각도와 무관하다. 0 = 360° |
| `IndividualSpeedSpread` · `NoticeScaleMin` | 0~0.5 · 0~1 | **개체 차이(2026-10-21, [../../../goal/monster-individual-spread.md](../../../goal/monster-individual-spread.md))**. 태어날 때 호스트가 HP 와 같이 한 번 굴린다. 이동 속도에 1±spread(걷기·추격·상한 모두), 감지 속도에 [min, 1]. 0 = 모두 같다 |
| `ChaseFlankOffset` · `ChaseFlankDistance` | cm · cm | 태어날 때 왼쪽·오른쪽·정면(-1·0·+1) 중 하나를 고르고, 추격 경로 목표를 타깃 옆 `Offset × min(거리/Distance, 1)` 로 둔다 — 붙을 때는 정면. 그 지점이 내비 밖이면 타깃 자체. 0 = 곧장 |
| `ChaseRepathJitter` | 0~0.9 | 추격 재경로 간격(0.5초)에 매번 1±값 |
| `OwnedItemAlias` | 아이템 별칭 | 이 종이 주인인 아이템 — 판 시작에 놓인 것을 누가 처음 주우면 살아 있는 이 종 무작위 한 마리가 주운 사람을 찾으러 간다 (2026-10-21) |
| `ThiefHuntSpeed` | cm/s | 도둑을 찾으러 가는 속도 |
| `ThiefDetourStep` | cm | 도둑 쪽으로 한 번에 나아가는 거리 |
| `ThiefDetourRadius` | cm | 그 자리 둘레에서 다음 지점을 고르는 반지름 — 곧은 길에서 비껴 가는 폭, 이 안이면 곧장 |
| `StalkStareAnger` | float, s | (위 스토킹 절) 0 이면 격분하지 않는다 — 2026-10-20부터 |

값(2026-10-20 기본값, 플레이 뒤 조절): 닭 `LightLureRadius` 900 · `KillAlarmRadius` 2100 → 0(맞아 죽은 닭이 무리를 부르는 옛 핵심 행동을 끈다) ·
돼지 `bTargetCarriersOnly` true · 양 `IsolationRadius` 600 · 소 `bFreezeWhenWatched` true · 너구리 `StalkStareAnger` 6 → 0.
코드: `Source/CL9/Monsters/CL9Monster.cpp` `SenseTick`·`TickChaseGiveUp`·`HostTickAI`, 열 설명은 데이터 패널 `Tools/DataPanel/effects/monsters.csv`.

---

## 각성과 경보

| 컬럼 | 뜻 |
|---|---|
| `WakeTelegraph` | 깨어나기 전 예고 — 퍼덕임과 울음 (초) |
| `WakeSightRadius` | 이 안에 시선이 있으면 기상 보류 (cm) |
| `WakeRetryTime` | 보류 재시도 주기 (초) |
| `PackAlarmRadius` | 무리 경보 반경 (cm) |
| `KillAlarmRadius` | 사살 경보 반경 (cm) |
| `bAlertsOthers` | 다른 몬스터를 부르나 (경보꾼) |

**소수 종만 쓰는 컬럼은 그 종의 문서에서 정의한다.** 잠·경보꾼 컬럼 5종(`WakeRadius` ·
`HearRunRadius` · `AlertWindup` · `SleepTime` · `AlertCooldown`)은 [goose.md](goose.md)에 있다 —
코드에는 공통 필드로 존재하고(클래스는 `UCL9MonsterDef` 하나, 안 쓰는 종은 0/false),
**다른 종이 쓰게 되면 이 문서로 승격한다.**

**`WakeTelegraph`는 전 플레이어에게 보인다** — 호스트만 보는 게 아니다.
`WakeSightRadius`는 보고 있는 곳에서 갑자기 생겨나는 것을 막는다.

사살 경보가 층/존을 넘어 퍼질지는 맵의 존 세팅과 함께 정한다 — 지하 사살이 1층을 깨우지
않게 하려면 사살 경로에 "같은 존인가" 검사를 둔다 (맵 세팅의 몫).

---

## 반격과 회복 (전역 기본값)

| 컬럼 | 뜻 | 기본값 | 구 CCL8 상수 |
|---|---|---|---|
| `RageTime` | 죽는 소리를 들은 뒤 사냥 모드 (초) | 9.0 | `KillRageT` |
| `KillCooldown` | 다시 죽일 수 있을 때까지 재장전 (초) | 9.0 | `KillCooldown` |
| `RevengeTime` | 복수 지속 (초) | 5.0 | `RevengeT` |
| `RevengeRadius` | 복수가 퍼지는 거리 (cm) | 276 | `RevengeRadiusU 92` |
| `PushStun` | 밀쳐졌을 때 경직 (초) — 남겨 두되 사용 행 없음(2026-09-22, 팬이 `Hit` 로 옮겨감 — 삭제는 별건) | 0.8 | `PushStun` |
| `PushKnockback` | 밀려나는 거리 (cm) — 남겨 두되 사용 행 없음(2026-09-22, 삭제는 별건). 타격 넉백은 아이템 행 `HitKnockback` 이 갖는다 | 1560 | `PushKnockbackU 520` |

**`KillCooldown` = `RageTime`은 하나로 묶인 손잡이다.** 따로 움직이지 않는다 — 로드 검사가
이 등식을 확인한다.

---

## 표시

| 컬럼 | 뜻 | 기본값 |
|---|---|---|
| `bShowOnMinimap` | 미니맵에 그려지나 | false |

미니맵 표시는 전역 원칙이 아니라 **종별 값**이다 — 미니맵 시스템이 이 값을 읽는다
([../../../systems/ui.md](../../../systems/ui.md)).

---

## 가치와 드롭

| 컬럼 | 뜻 |
|---|---|
| `ValueMin` / `ValueMax` | 몬스터 자체의 값 (이더) |
| `DropItems` | 잡으면 떨어뜨리는 아이템 — [../item/common.md](../item/common.md)에 있는 것만 참조 |

**사냥의 보상은 이더 직접 지급이 아니라 `DropItems`로 떨어지는 아이템이다.** 어떤 몬스터가
뭘 떨구는지는 개발자가 표에서 직접 연결한다. 몬스터 값 컬럼은 시체 판매 같은 기능이 생기기
전까지 쓰지 않는다. 계산식은 [../../economy.md](../../economy.md).

---

## 스폰 예산 (2026-10-20, 결정 #38 A4)

목적지의 실내 위협 풀(`MapTable.Zones[].ThreatPool`)에서 뽑힐 때 쓰는 열이다. 가축 줄(`Zones[].MonsterSpawns`)로만 놓이는 종은 쓰지 않는다.
뽑는 순서와 목적지 값은 [C:\Work\CL9\docs\goal	hreat-budget-spawns.md](C:\Work\CL9\docs\goal	hreat-budget-spawns.md).

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `SpawnPower` | float | 구역 예산에서 빼는 파워 — 리썰 `PowerLevel` 눈금(0.5~3). 0보다 커야 한다 |
| `SpawnMaxCount` | int | 한 판에 위협으로 나올 수 있는 이 종의 수(모든 구역 합, 나온 수로 센다). 0 = 상한 없음 |

| 종 | `SpawnPower` | `SpawnMaxCount` | 리썰 짝 |
|---|---|---|---|
| `goose` | 1 | 2 | Hoarding bug |
| `sheep` | 1 | 3 | Centipede |
| `cow` | 1 | 2 | Spring(Coil-head) |
| `pig` | 2 | 2 | Bunker spider · Butler |
| `raccoon` | 3 | 1 | Flowerman(Bracken) |

## 지금 있는 것

| Alias | 문서 | 상태 |
|---|---|---|
| `chicken` | [chicken.md](chicken.md) | 이동·감각·추격·HP 값 있음. 드롭 연결 미정 |
| `goose` | [goose.md](goose.md) | 값 있음 (양계장 기본 스폰 0~1 — site.md) |
| `raccoon` | [raccoon.md](raccoon.md) | 역할 = 스토커(`ExecuteRadius > 0`, 2026-09-20 사용자 결정) — 전 열 값 있음(확정 `StalkSpeed` 450·`StalkStareAnger` 6·`SleepTime` 0, 나머지 초안), `check_inequalities.py` A1·A5·A6·A7 통과 |
| `pig` | [pig.md](pig.md) | 역할 = 잡히면 무조건 처형(`bExecuteOnCatch`, 2026-09-20 사용자 결정) — 그 밖 값은 닭 복사 (2026-09-05) |
| `sheep` | [sheep.md](sheep.md) | 시스템 검증용 — 값은 닭 복사. **2026-10-13 뺀다**(사용자) — 행·배치 제거는 별도 작업 |
