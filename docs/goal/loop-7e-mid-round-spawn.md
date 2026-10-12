# goal: loop-7e-mid-round-spawn

**Status**: done (2026-10-12) — see Results. Build order ⑦ step **7e** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decision 7 ("판 중간에 새로 스폰되는 것 — 플레이어에게는 예고 없이 나타난다"); open question 7 answered here.

## One-line Goal
While a round runs, the host adds one monster at a time on a timer, at a building spot no living player can see and that is far from everyone, until a per-round cap tied to the round's starting monster count is reached.

## Background / Motivation (recon 2026-10-12)
- Monsters spawn once, at `StartNight` (`SpawnNightPopulation` queues them over frames from the layout's `Monster` points, site rows + depth zones). Nothing adds monsters later.
- Reference: Lethal Company plots inside spawns in time batches and announces each vent 15 s ahead by sound (`docs/reference/lethal/lethal-monsters.md`); our decision 7 is "no warning".
- User answers 2026-10-12 (open question 7):
  - **Trigger** = time interval (one spawn per interval).
  - **Where** = a building `Monster` point that is hidden from every living player (world geometry blocks the line from the player's eye to the spot) and at least a minimum distance from every living player; depth not restricted.
  - **What / how many** = species drawn in proportion to the round's own start-spawn table (site row + its depth zones), one monster per spawn, per-round total cap = a ratio of the round's starting monster count.

## In-scope
1. **Rules cells** (`CL9Rules`, draft values, header = intended):
   - `MidSpawnInterval` 60 s — seconds between mid-round spawns, first one at +interval after the round starts; 0 = off.
   - `MidSpawnMinDistance` 1500 cm — minimum 3D distance from every living player.
   - `MidSpawnCapRatio` 0.5 — per-round cap = round(starting monsters queued × ratio).
   Load check: finite, interval ≥ 0, distance ≥ 0, ratio ≥ 0.
2. **Host timer** (`ACL9GameMode`): armed right after `SpawnNightPopulation` when the round has a generated map, the interval > 0 and the cap > 0; cleared by `CancelNightWork` (so every round end / run end path stops it); stops itself at the cap.
3. **One spawn per tick**: pick the species by weight (Min+Max of each entry of `Site.MonsterSpawns` and the built maps' zone `MonsterSpawns`), take the built maps' `Monster` points in random order, accept the first that passes distance + hidden for every living player (checking at most 64 candidates), spawn with the existing `SpawnMonster` at the point (+ monster radius, like the start queue), add to `NightSpawned`. No spot → skip this tick (log), try again next interval.
4. **Logs**: arm line (`interval, cap, pool`), one line per spawn (species, location, nearest player distance, count/cap), skip line, cap-reached line.
5. **Docs**: plan/data/rules.md (3 rows), systems/monster.md (mid-round spawn), loop-rework open question 7 marked answered, decisions.md, build-order 7e → done, goal README row.

## Out-of-scope
- Noise- or blackout-driven spawns, vents / warnings, per-site or per-day curves for these cells (balance later), outdoor or `Lvl_Flat` (no generated map → no mid-round spawns).
- Monster AI changes (spawned monsters start like start-of-round ones).

## Acceptance Criteria
- [x] `CL9Editor` builds.
- [x] PIE `Lvl_Henhouse` 1 player (interval shortened in memory for the test): spawns appear one per interval with log lines; each spawn is ≥ `MidSpawnMinDistance` from the player and has no clear line of sight from the player's eye (re-checked by a trace at spawn time); the count stops at round(start × 0.5).
- [x] After the round ends (lobby) no further mid-round spawns happen; the next round re-arms with its own cap.
- [x] With `MidSpawnInterval` 0 nothing is armed.
- [x] Start-of-round spawning unchanged (same queue log).

## Results (2026-10-12)
- **Build**: `CL9Editor` first try (249 s).
- **PIE `Lvl_Henhouse`, 1 player**, `MidSpawnInterval` set to 5 s in memory only (restored to 60 and re-saved clean afterwards); the player was put inside the building (in front of the kitchen station) so hidden and visible spots both exist:
  - Round 1: `mid-round spawns armed — every 5s, cap 3 (5 at start × 0.50), 2 species entries.` → spawns 1/3, 2/3, 3/3 (chicken) at nearest-player 1923 / 6462 / 2793 cm, `hidden from 1 player(s)` → `cap 3 reached — no more this round.` Start queue log unchanged (`5 of 5 queued monsters spawned`).
  - Round end (`cl9.night`) → lobby: no mid-round line during the lobby.
  - Round 2: re-armed `cap 3 (6 at start × 0.50)`; spawns at t = 3.9 / 8.9 / 13.9 s (5 s apart). **Independent recorder** (editor Python, first sight of each new monster): distance from the player's eye 4158 / 5058 / 4795 cm, line of sight **blocked** for all three.
  - `MidSpawnInterval` 0 → `mid-round spawns off (MidSpawnInterval 0).`, start queue still `8 of 8`.
- **Editor incident**: during the build an editor not launched by any Claude session (`Lvl_Flat`, launched 11:44, later relaunched 11:57) held the MCP port, so this session's own editor was a ghost; with the user's OK the session closed only its own editor, ran the test in the other one, and left it on `Lvl_Flat` with PIE settings ListenServer · 2 clients and no dirty packages.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | 3 cells + load check |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | arm / tick / cancel, spot choice |
| `Source/CL9/Monsters/CL9Monster.h` | `TraceWorld` (reused for the hidden check) |

## Must-Preserve
- Start-of-round queue, calm radius, 7b yard rule (yard modules have no `Monster` points), jar / fixture spawns.
- Every round-end path clears night work through `CancelNightWork`.
- Other sessions' uncommitted changes in the touched files.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (spawn rules, timer lifecycle). A cheaper model is fine for doc edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Editor / build / PIE via the lock CLI (`docs/workflow/sessions.md` "잠금"), lock commands issued alone.
