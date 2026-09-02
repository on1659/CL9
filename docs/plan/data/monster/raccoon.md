# 너구리 — `raccoon`

세 번째 몬스터. **틀 검증용 초안 값이 채워진 상태다** (3g에서 기입 — 2026-09-03).
컨셉(도둑 행동 등)은 미결이라, 지금은 "닭보다 조금 빠르고 멀리 도는 잡는 몬스터"다.

컬럼 정의는 [common.md](common.md).

---

## 공통 컬럼 값 (전부 초안 — 플레이해보고 조정)

| 컬럼 | 값 | 근거 |
|---|---|---|
| `HpMin` / `HpMax` | 4 / 5 | 닭(3~4)보다 한 대 더 버틴다 |
| `HitStunMs` | 300 | 닭과 동일 |
| `WanderSpeed` | 140 | 닭 126보다 약간 빠름 |
| `PatrolSpeed` | 210 | |
| `ChaseSpeed` | **462** | 닭 396 < 너구리 462 < 지친 빈손 506 — **A1 부등식 유지** (잡는 종이라 필수) |
| `RetreatSpeed` | 220 | |
| `TurnRate` | 320 | 닭 280보다 민첩 |
| `TurnDrag` | 0.35 | 코너 손해가 닭(0.45)보다 작다 — 미로가 덜 통하는 종 |
| `OutdoorChaseMult` | 0.9 | |
| `Radius` | 40 | |
| `SeparationDist` | 100 | |
| `SightIn` / `SightOut` / `SightDark` | 390 / 260 / 130 | 닭보다 눈이 좋다 |
| `HearRadius` | 210 | |
| `DetectNearRadius` | 130 | |
| `DetectRateNear` / `Far` / `Decay` / `Grace` | 2.8 / 0.6 / 0.4 / 0.5 | |
| `ChaseBudget` | 30 | 닭 45보다 짧게 — 빠른 대신 끈기가 없다 |
| `ChaseLoseTime` | 5.0 | |
| `ChaseBreakTime` | 1.2 | |
| `ChaseCooldown` | 3.0 | |
| `HomeRange` | 1200 | 닭 840보다 넓게 돈다 |
| `DeaggroRadius` | 510 | |
| `CatchRadius` | 84 | 잡는 종 |
| `CatchDamage` | 1 | |
| `SearchHops` / `SearchDwellTime` | 3 / 2.0 | |
| 각성·경보 | 닭과 동일 (WakeTelegraph 0.8 · WakeSightRadius 900 · WakeRetryTime 2 · PackAlarmRadius 600 · KillAlarmRadius 2100 · bAlertsOthers false) | |
| 잠 컬럼 5종 | 전부 0/false — 잠들지 않는 종 | |

## 에셋 (임시 대타)

무료 Animal Variety Pack의 **Fox**를 임시 메시로 쓴다 (2026-09-03 —
`Content/AnimalVarietyPack/Fox/`). 진짜 너구리 에셋은 ⑤에서 갈아끼운다 (원칙 2).

| 컬럼 | 에셋 |
|---|---|
| `SkeletalMesh` | `SK_Fox` |
| `Skeleton` | `SK_Fox_Skeleton` |
| `AnimBP` | `ABP_Fox` (Idle/Walk/Run 속도 블렌드 — 3g에서 제작) |

## 미결

- **컨셉 자체** (도둑 행동? 밀치기 반응?) — 값 조정과 함께 별도 결정
- 드롭 연결 (`DropItems`) — 닭과 함께 미정
