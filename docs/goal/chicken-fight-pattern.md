# goal: chicken-fight-pattern

**Status**: spec + implementation (2026-10-20) — user: "닭 패턴이 지금 따로 없잖아, 그 간이로 만든 미로 있으니까 거기서 실제 닭이랑 싸워 보면서 해 봐야지". Test ground: the `Lvl_Flat` test arena (`TestArena/` — 40×40 m fence with 11 wall structures, 2026-10-05).

## One-line Goal
Give the chicken a fight loop — a telegraphed straight charge that leaves it dazed when it hits a wall, and a flee at low HP that keeps its HP — tunable from the monster table and testable in the `Lvl_Flat` arena.

## Background / Motivation
- The chicken's only attack is a 0.6 s windup and a 110 cm lunge; walking backwards always dodges it, so the fight is "approach, hit twice". The core behaviour (2026-10-20, light lure) decides *when* chickens come, not *how* they fight.
- User picks (2026-10-20): ① charge → wall daze, ② flee at low HP, ③ "어그로 풀려도 HP 는 그대로, 유저 시야에서 사라지고 특정 거리 이상 될 때까지 도망간다".

## In-scope
- New `UCL9MonsterDef` columns (category Pattern, all 0 = old behaviour): `PounceWallStunTime` (s), `FleeHpThreshold` (int), `FleeSpeed` (cm/s), `FleeSafeDistance` (cm). Load checks: finite, non-negative; flee on needs speed and distance.
- Charge = the existing timed pounce (`PounceMoveTime > 0`: direction locked at launch, swept straight move). New: if the move is blocked by world geometry and the target was not caught, the monster enters Stun for `PounceWallStunTime` with the long react clip (`bPushStun`).
- Daze is kept through hits: a hit or shove during Stun takes `max(new stun, remaining)`.
- Flee: when HP ≤ `FleeHpThreshold`, any transition into a hunt state (Chase, Stalk, Investigate, Pounce) is redirected in `EnterState` to Return with `bFleeing`. While fleeing it paths away from the nearest living player (goal = away × `FleeSafeDistance`, tries 0/±45/±90°) at `FleeSpeed`, repath 0.5 s. It stops when every living player is beyond `FleeSafeDistance` and none has line of sight; that spot becomes its new home. HP never regenerates (already true — HP is rolled once).
- Chicken row values (initial, tuned in the arena via the data panel).
- Panel column descriptions (`Tools/DataPanel/effects/monsters.csv`), `docs/plan/data/monster/common.md` and `chicken.md`.

## Out-of-scope
- New animation clips (the charge reuses the windup clip; daze reuses PushReact and holds its last frame).
- Other species, the revealed chicken row, the scenario/roadmap order.
- A new `ECL9MonsterState` value — rejected because `ABP_<species>_A` is a per-state boolean chain and a new value would have no branch.

## Design Decisions
- Flee is a sub-mode of Return, not a new state — Return emits no threat signals (no chase music, no stinger in `CL9FearComponent`) and locomotion follows speed. (Rejected: a Chase sub-mode — it would trip the chase stinger and give-up clocks.)
- The redirect lives in `EnterState`, one place, so senses, pack/kill alarms and stun recovery all honour it.
- Daze triggers only on a blocked timed move; an open-air miss keeps `PounceRecoveryTime` (that is the dodge-to-the-side punish window).

## Acceptance Criteria
- [x] `CL9Editor` compiles and builds (broker: compile 72 s, build 52 s, 2026-10-20 23:41).
- [x] Chicken row (written and saved through the data panel 23:48; editor log has no load-check error): `PounceRadius` 450 · `PounceWindup` 0.6 · `PounceLunge` 500 · `PounceMoveTime` 0.4 · `PounceRecoveryTime` 0.8 · `PounceMissCooldown` 1.0 · `PounceWallStunTime` 1.5 · `FleeHpThreshold` 1 · `FleeSpeed` 560 · `FleeSafeDistance` 1500 — saved; load check clean.
- [ ] `Lvl_Flat` PIE: a chicken charges; sidestepping so it hits an arena wall logs `charged into a wall — dazed 1.50s` and it stands dazed; hitting it in the daze does not shorten the daze.
- [ ] A chicken at HP 1 logs `flees (HP 1)`, runs away, and logs `got away (HP 1) — settles here.` once out of sight and beyond 15 m; re-sensing a player makes it flee again, not chase.
- [ ] Other species unchanged (all four columns 0).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9MonsterDef.{h,cpp}` | four columns + load checks |
| `Source/CL9/Monsters/CL9Monster.{h,cpp}` | `HostWallDaze`, `ShouldFlee`, `TickFlee`, `EnterState` redirect, daze kept through hits |
| `Content/Data/MonsterTable` | chicken row values |
| `Tools/DataPanel/effects/monsters.csv` | panel descriptions |
| `docs/plan/data/monster/common.md`, `chicken.md` | column definitions, values |

## Must-Preserve
- Rows with the four columns at 0 behave exactly as before. Instant pounce (`PounceMoveTime` 0) unchanged.
- Host-only decisions; nothing new replicated (`bFleeing` is host state; clients see Return + speed).
- Execute "no rescue" rule: daze/flee never fire from Execute.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above for the AI changes; a cheaper model is fine for docs and table values.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
