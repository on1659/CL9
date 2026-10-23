# 양 — `sheep`

> **2026-10-13 사용자 결정 — 뺀다.** 위협 세 층([common.md](common.md) "층")의 어느 층에도 들지 않는다. 이 문서는 기록으로 남기고,
> `MonsterTable` 행·`Lvl_Flat` 배치·에셋 정리는 별도 작업이다.

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
| `PounceRadius` | 130 (초안) — 닭과 동일 (`ChaseSpeed` 396 · `CatchRadius` 84가 닭과 같으니 회피 산수도 같다, 2026-09-20) |
| `PounceWindup` | 0.6 (초안) — 닭과 동일 |
| `PounceLunge` | 110 (초안) — 닭과 동일 |
| `PounceMissCooldown` | 0.6 (초안) — 닭과 동일 |

## 에셋 (자체 제작 — 2026-10-09 확인)

`Content/Monsters/SheepA/` — 제작 기록은 [../../../asset/monster/sheep_a.md](../../../asset/monster/sheep_a.md), 원본은 `ArtSource/monster/`.

| 컬럼 | 에셋 |
|---|---|
| `SkeletalMesh` | `SK_Sheep_A` |
| `Skeleton` | `SK_Sheep_A_Skeleton` |
| `AnimBP` | `ABP_Sheep_A` (`Content/Monsters/`) |

클립(폴더 실물): `AN_Sheep_Idle` · `AN_Sheep_Walk` · `AN_Sheep_Run` · `AN_Sheep_LookAround` · `AN_Sheep_WakeTelegraph` · `AN_Sheep_Catch` · `AN_Sheep_HitReact` · `AN_Sheep_PushReact` · `AN_Sheep_Death`.

Dreamscape Farmlands 팩의 `SK_Sheep`·`A_Sheep_*`는 쓰지 않는다 — 팩은 2026-10-04에 지웠다.

## 미결

- 양의 역할·값 전부 — 지금은 닭의 복제다.
