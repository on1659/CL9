# 양 — `sheep`

**몬스터 시스템 검증용 3호 종 (2026-09-05, 초안).** [돼지](pig.md)와 같은 이유로 넣었다 —
기획에는 없고, 값은 전부 [닭](chicken.md)을 복사했다.

컬럼 정의는 [common.md](common.md).

---

## 값

| 구분 | 값 |
|---|---|
| 체력·이동·감각·추격·경보 전 컬럼 | 닭과 동일 (chicken.md 표 그대로) |
| `MeshYawOffset` | -90 (같은 팩, 같은 축) |
| `SleepTime` | 0 — 잠들지 않는다 |

## 에셋 (보유 — 2026-09-05 확인)

`Content/DreamscapeSeries/DreamscapeFarmlands/Meshes/Chars/Sheep/`:

| 컬럼 | 에셋 |
|---|---|
| `SkeletalMesh` | `SK_Sheep` (`SK_Sheep_HairTop`은 별도 메시 — 미사용) |
| `Skeleton` | `SKEL_Sheep` |
| `AnimBP` | `ABP_Sheep` (Content/Monsters — ABP_Chicken 복제 후 스켈레톤·애님 교체) |

애님 커버리지 (상태 기계 요구 대조):

| 상태 기계 요구 | 팩 애님 | 비고 |
|---|---|---|
| 배회·순찰 (Walk) | `A_Sheep_Walkcycle` | ✓ |
| 추격 (Run) | `A_Sheep_Run` | ✓ |
| 평시 (Idle) | `A_Sheep_Idle` | ✓ |
| 잠듦 | `A_Sheep_SleepLoop` | ✓ (SleepTime 0이라 미사용) |
| 기상 예고 | `A_Sheep_Sleep_Idle` | ✓ (동상) |
| 잡기 (Catch) | `A_Sheep_Attack_Anim` | ✓ |
| 경직 (HitStunMs) | `A_Sheep_Hurt` | ✓ |
| 죽음 | `A_Sheep_Death` | ✓ |

미사용 여분: `A_Sheep_Eat`, `Idle_Rest`/`RestLoop`/`Rest_Idle`, `Idle_Sleep`.

## 미결

- 양의 역할·값 전부 — 지금은 닭의 복제다.
