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
| `DeaggroRadius` | 짐을 다 놓았을 때 풀리는 거리 (cm) |
| `CatchRadius` | 잡는 반경 (cm) — 0이면 잡지 않는 종 |
| `CatchDamage` | 잡았을 때 플레이어 타격량 (HP) — 즉사 여부는 이 값과 HP가 정한다 |
| `PounceRadius` | 잡기 예고가 시작되는 거리 (cm) — 이 안에서 멈춰 서서 예고한다 |
| `PounceWindup` | 예고 길이 (초) — 이동 0, 타깃을 향해 `TurnRate`로 돌며 기다린다. **0이면 예고 없음** |
| `PounceLunge` | 예고 끝에 타깃의 지금 위치 방향으로 한 번에 덮치는 거리 (cm) — 벽이면 거기까지 |
| `PounceMissCooldown` | 헛친 뒤 다시 예고할 수 있기까지 (초) — `KillCooldown`과 무관 |
| `SearchHops` | 수색 홉 수 |
| `SearchDwellTime` | 한 지점에서 머무는 시간 (초) |

**`ChaseCooldown`은 새로 경계 상태에 들어가는 것만 막는다.** 정전·복수·기절에서 깨어나는 건
못 막는다 — 쿨다운이 만능 방패가 되면 안 된다.

### 잡기 예고 (Pounce, 2026-09-20)

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

## 지금 있는 것

| Alias | 문서 | 상태 |
|---|---|---|
| `chicken` | [chicken.md](chicken.md) | 이동·감각·추격·HP 값 있음. 드롭 연결 미정 |
| `goose` | [goose.md](goose.md) | 값 있음 (양계장 기본 스폰 0~1 — site.md) |
| `raccoon` | [raccoon.md](raccoon.md) | 역할 = 스토커(`ExecuteRadius > 0`, 2026-09-20 사용자 결정) — 전 열 값 있음(확정 `StalkSpeed` 450·`StalkStareAnger` 6·`SleepTime` 0, 나머지 초안), `check_inequalities.py` A1·A5·A6·A7 통과 |
| `pig` | [pig.md](pig.md) | 역할 = 잡히면 무조건 처형(`bExecuteOnCatch`, 2026-09-20 사용자 결정) — 그 밖 값은 닭 복사 (2026-09-05) |
| `sheep` | [sheep.md](sheep.md) | 시스템 검증용 — 값은 닭 복사, 역할 미결 (2026-09-05) |
