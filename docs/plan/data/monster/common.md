# 몬스터 공통 — `UCL9MonsterDef`

재료의 주인이자 위협의 **컬럼 정의**. 개별 몬스터의 값은 이 폴더의 각 문서에 있다 —
[chicken.md](chicken.md) · [goose.md](goose.md) · [raccoon.md](raccoon.md).

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
| `HitStunMs` | int | 죽지 않는 타격의 경직 (1000 = 1초) |

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

정전 때는 몬스터 시야도 줄지만, 발광 아이템을 든 사람은 `CarrierSeenRadius`에서 팔린다
([../item/egg.md](../item/egg.md)).

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
| `SearchHops` | 수색 홉 수 |
| `SearchDwellTime` | 한 지점에서 머무는 시간 (초) |

**`ChaseCooldown`은 새로 경계 상태에 들어가는 것만 막는다.** 정전·복수·기절에서 깨어나는 건
못 막는다 — 쿨다운이 만능 방패가 되면 안 된다.

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
| `PushStun` | 밀쳐졌을 때 경직 (초) | 0.8 | `PushStun` |
| `PushKnockback` | 밀려나는 거리 (cm) | 1560 | `PushKnockbackU 520` |

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
| `raccoon` | [raccoon.md](raccoon.md) | 값 전부 미결 (③에서) |
| `pig` | [pig.md](pig.md) | 시스템 검증용 — 값은 닭 복사, 역할 미결 (2026-09-05) |
| `sheep` | [sheep.md](sheep.md) | 시스템 검증용 — 값은 닭 복사, 역할 미결 (2026-09-05) |
