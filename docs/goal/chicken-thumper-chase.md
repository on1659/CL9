# goal: chicken-thumper-chase

**Status**: spec + implementation (2026-10-21). User: "리썰의 덤퍼랑 비슷한 느낌으로 만들려는 애야. 밸런스를 그렇게 — 때리기도 힘들고 피하는 건 되지만 그 정도 조작 난이도. 부딪칠 때 걷다가 하나 둘 고고가 아니라 그냥 무작정 박게, 아니면 시전 시간을 진짜 짧게. 몬스터 하나 제대로 만들자."
Follows [chicken-fight-pattern.md](chicken-fight-pattern.md) (charge/daze/flee) and replaces its charge with Thumper's contact rush.

## One-line Goal
Make the chicken chase like Lethal Company's Thumper — notices in 0.2 s inside a forward cone, accelerates from standstill to faster than a sprint, turns worse the faster it goes, loses its speed when it slams a wall, and bites on contact with no windup — all from table columns.

## Background — what the Thumper actually does (read from the installed game, `CrawlerAI.cs` + prefab `sharedassets1.assets` path 19019)
- Notice: `CheckLineOfSightForPlayer(55f)` — 55° half-angle; chase starts when `noticePlayerTimer > 0.2` (0.2 s). While chasing it looks with `(65f, 80)`.
- Speed: chase starts at `agent.speed = 0`, `agentSpeedWithNegative += dt × SpeedIncreaseRate(5)` clamped to 16 m/s. Acceleration `Clamp(BaseAcceleration(42) − avgVel × 2.4, 4, 40)`; angular speed 800 when acceleration > 22, else 230 — fast means it can't corner.
- Wall: a sudden velocity drop while chasing → `agentSpeedWithNegative *= 0.2` + wall sound. No stun.
- Bite: `OnCollideWithPlayer` → 40 damage (of 100), 0.65 s between bites, speed reset to −2…0.25 after a bite. No windup.
- Hit by player: speed set to −2.8…−2 (stops/backs), next 1 s ramps at +4.25 extra. `enemyHP` 4 (four shovel hits).
- Lose: 1.8 s without sight → search at the last-seen spot.
- Player for scale: walk 4.6 m/s, sprint lerps to ×2.25 (10.35 m/s).

## In-scope
- New `UCL9MonsterDef` columns (Pattern, all 0 = old behaviour): `ChaseRampRate`, `ChaseMaxSpeed`, `ChaseHitRampBonus`, `TurnRateAtMaxSpeed`, `ChaseWallSlamSpeed`, `ChaseWallSlamKeep`, `ChaseWallStunTime`, `SightHalfAngle`, `ChaseSightHalfAngle`, `SightNearRadius`. Load checks: finite, non-negative, keep ≤ 1, angles ≤ 180, ramp needs a max.
- Chase: speed ramps from 0 (reset on chase start, stun, catch); +bonus for 1 s after being hit; turn rate lerps from `TurnRate` to `TurnRateAtMaxSpeed` by speed/max; a world hit (not a monster/character) at ≥ `ChaseWallSlamSpeed` keeps `ChaseWallSlamKeep` of the speed (+ optional stun).
- Sight cone: outside `SightNearRadius` a candidate must be within the half-angle of the monster's facing; lit players (light lure) ignore the cone; hearing ignores it.
- Chicken row values below; `PounceWindup` 0 → contact catch (the existing instant path).

## Chicken values (speeds scaled by our sprint ÷ Lethal sprint = 654 ÷ 10.35 ≈ 63 cm per Lethal m/s)
| Column | Value | From |
|---|---|---|
| `ChaseRampRate` | 320 | 5 m/s² |
| `ChaseMaxSpeed` | 1000 | 16 m/s (1.5 × our sprint) |
| `ChaseHitRampBonus` | 270 | 4.25 |
| `TurnRate` / `TurnRateAtMaxSpeed` | 800 / 230 | angular speed |
| `ChaseWallSlamSpeed` / `Keep` / `Stun` | **500** / 0.2 / 0 | ×0.2, no stun. 200 (≈ Thumper's 3 m/s) was tried first: chickens brushing walls at ~200 kept losing speed and never ramped — raised to 500 so only a real high-speed stop counts |
| `SightHalfAngle` / `ChaseSightHalfAngle` / `SightNearRadius` | 55 / 65 / 200 | 55°, 65°, close range |
| `DetectRateFar` / `DetectRateNear` | 5 / 10 | 0.2 s notice |
| `ChaseLoseTime` | 2.0 | 1.8 s |
| `PounceWindup` | 0 | no windup — contact bite |
| `DeaggroRadius` | **0** (was 510) | Thumper has no "empty-handed → let go" rule. 510 dropped every empty-handed player beyond 5.1 m the moment the chase started — found in PIE |
| `HpMin`/`HpMax` | 8 / 8 | four pan hits (pan 2 dmg) = four shovel hits |

Kept from earlier passes: `SightIn` 2500, `HomeRange` 6000, light lure 3500, flee at HP 1 (meat hits only reach it).

## Out-of-scope
- Thumper's search-at-last-seen after losing (ours goes Break → Return), its body-eating, its vent spawn.
- Per-individual randomness (separate request).
- New animation; wall-slam sound row.

## Acceptance Criteria
- [x] Build OK (broker 2026-10-21 01:05).
- [x] Chicken row saved with the values above (data panel); no load-check error.
- [x] `Lvl_Flat` PIE (`Tools/Unreal/test_chicken_thumper_20261021.py`, 01:20–01:40): back-turned at 8 m — not seen; facing — chase after 0.25 s; speed 16 → >1000 cm/s; contact bite with no POUNCE state; wall slams logged at 500–573 cm/s. Original criterion: a chicken behind you does not notice you beyond 2 m; in front it starts chasing within ~0.2 s, out-runs a sprint on a straight, overshoots a corner and logs `slammed a wall at … keeps …`, and bites on contact with no crouch.
- [x] Other species unchanged (all new columns 0 on their rows).

## Must-Preserve
- Rows with the new columns at 0 behave as before; A1 (`지친 빈손 > ChaseSpeed`) is untouched because the ramp uses `ChaseMaxSpeed` — the chicken breaking A1 is intended (Thumper out-runs a sprint; you escape by corners).
- Host-only; nothing new replicated (clients see position and state).

## Execution Notes
- Recommended model: Claude Opus 5.5 or above for AI changes; a cheaper model is fine for docs/values.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
