# 돼지 — `pig`

**몬스터 시스템 검증용 2호 종 (2026-09-05, 초안).** 기획에는 아직 없는 종이지만, 보유 팩에
메시·애님 풀셋이 있어 "새 몬스터 = 표에 행 하나 + AnimBP 하나"라는 틀이 실제로 도는지
확인하려고 넣었다. 값은 전부 [닭](chicken.md)을 복사했다 — 돼지다운 값(느리고 무겁게 등)은
플레이해보고 정한다.

컬럼 정의는 [common.md](common.md).

---

## 값

| 구분 | 값 |
|---|---|
| 체력·이동·감각·추격·경보 전 컬럼 | 닭과 동일 (chicken.md 표 그대로) |
| `MeshYawOffset` | -90 (같은 팩, 같은 축) |
| `SleepTime` | 0 — 잠들지 않는다 (잠듦·기상 애님은 연결만 되어 있음) |

## 에셋 (보유 — 2026-09-05 확인)

`Content/DreamscapeSeries/DreamscapeFarmlands/Meshes/Chars/Pig/`:

| 컬럼 | 에셋 |
|---|---|
| `SkeletalMesh` | `SK_Pig` |
| `Skeleton` | `SKEL_Pig` |
| `AnimBP` | `ABP_Pig` (Content/Monsters — ABP_Chicken 복제 후 스켈레톤·애님 교체) |

애님 커버리지 (상태 기계 요구 대조):

| 상태 기계 요구 | 팩 애님 | 비고 |
|---|---|---|
| 배회·순찰 (Walk) | `A_Pig_Walkcycle` | ✓ |
| 추격 (Run) | `A_Pig_Runcycle` | ✓ |
| 평시 (Idle) | `A_Pig_Idle` | ✓ |
| 잠듦 | `A_Pig_SleepLoop` | ✓ (SleepTime 0이라 미사용) |
| 기상 예고 | `A_Pig_SleepIdle` | ✓ (동상) |
| 잡기 (Catch) | `A_Pig_Eat` | 공격 애님이 없어 먹기로 대타 (초안) |
| 경직 (HitStunMs) | `A_Pig_Hurt` | ✓ |
| 죽음 | `A_Pig_Death` | ✓ |

미사용 여분: `A_Pig_IdleRoll`/`RollLoop`/`RollIdle`, `IdleSit`/`SitLoop`/`SitIdle`.

## 미결

- 돼지의 역할·값 전부 — 지금은 닭의 복제다.
- 공격 애님 부재 — 잡기 연출을 먹기로 두는 게 어색하면 별도 에셋이 필요하다.
