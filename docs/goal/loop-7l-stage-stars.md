# goal: loop-7l-stage-stars

**Status**: done (2026-10-13, listen 2-player PIE verified). Build order ⑦ step **7l** ([../plan/build-order.md](../plan/build-order.md)) — added after 7k.
Parent concept spec: [loop-rework.md](loop-rework.md). Answers the open item "the run has no end" left by 7a (wipe = penalty only) and 7i (quota miss = penalty only).

## One-line Goal
Turn each settlement period (one destination, three rounds, settled at the company) into a **stage** that earns 0–3 stars; stars open the next destination, and settling the **last** destination with a star **clears the run** (clear screen → start level). Failure never ends the run.

## Background / Motivation
- Today nothing ends a run: a wipe costs 30 % of the coins (7a), a quota miss costs 30 % and the run goes on (7i). With 0 coins a failure costs nothing.
- User answers (2026-10-13):
  - failure end: **none** — keep "penalty and continue";
  - when a run ends: **game-over screen → start level** (the existing path);
  - structure: the user weighed Overcooked-style stages against Lethal-style endless runs and chose the **mixed plan** — a settlement period is a stage, settled with stars, behind a single switch cell so the current behaviour stays one cell away;
  - between stages **coins, equipment and day carry on** (the run keeps going);
  - stars are **period income ÷ quota** (draft 1★ 100 %, 2★ 150 %, 3★ 200 %, table cells; 0★ = miss, same 30 % penalty as now);
  - unlock: the **next destination in table order opens when the previous one earned ≥ 1★** (cell);
  - run end: **settling the last destination with ≥ 1★ = clear**.

## Design Decisions
- **Switch** `CL9Rules.bStageStars` (header and cell true). False = exactly the 7i behaviour (no stars, every site open, no clear).
- **Cells** `CL9Rules.StageStarRatios` (`[1.0, 1.5, 2.0]`, ascending, > 0 — star *n* needs `Earned ≥ Target × Ratios[n-1]`) and `CL9Rules.StageUnlockStars` (1). Pure function `UCL9RulesDef::StageStarsFor(Earned, Target)`.
- **The stage's destination** is the site the period was played at: at the company it is the site the truck came from (`UCL9GameInstance::GetRunCarryFromSite`); without a company (settling in the lobby) it is the level's site (`ActiveSiteAlias`).
- **Order** is the site table order of sites that have a `Level` (`GetHostableSites` — today henhouse → farm → pigfarm).
- **Progress belongs to the run**: best stars per site live in `ACL9GameState::StageStars` (replicated) and in the run save (`UCL9SaveGame` v3 `StageStars`), so they cross levels with the run and a new run starts empty. A site is **open** when it is first in the order, already has an entry (played in this run — covers a run started at a later site from the title), or the previous site holds ≥ `StageUnlockStars`.
- **Settlement** (`ACL9GameMode::Settle`): pass/miss and the miss penalty stay as they are; stars are added beside them. The stage result (stars, site, newly opened site, cleared) rides in the replicated `FCL9SettlementResult` so the company screen (derived from replication) shows it on every machine.
- **Clear**: right after a clearing settlement the host records the best day and sets `FCL9RunOver` with `bCleared` and the per-site stars. The existing game-over screen shows a clear variant (title, per-site stars) and the same buttons back to the start level. The depart lever already refuses once the run is over.
- **Destination board**: the host cycles through **open** sites only; the prompt shows the destination's stars in this run.
- **Strings** ko only (translation later): star glyphs are string-table keys (`Stage.StarOn` / `Stage.StarOff`) so they can be swapped if a font lacks them.
- Rejected: per-machine permanent unlocks (a cleared save would open everything for the next run and the run would no longer be a climb); a separate stage table (the mixed plan reuses periods and sites).

## Out-of-scope
- Balance of ratios / quota curve, a stage select map screen, Overcooked-style fixed-difficulty replays, permanent records of stars (the record file keeps only `BestDay`).

## Acceptance Criteria
- [x] `CL9Editor` builds; automation test `CL9.Run.StageStars` passes (ratios, edge cases).
- [x] Company settlement with income ≥ 150 % of the quota → screen shows the site with ★★☆ and "next destination opened"; the destination board then cycles henhouse ↔ farm only (pigfarm closed).
- [x] Settling the last open site with ≥ 1★ → clear screen (title, per-site stars) on host and client; lever refuses; button returns to the start level.
- [x] `bStageStars` false → no star lines, all sites cycle, no clear (7i behaviour).
- [x] Stars survive the company ↔ site travel (run save v3).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | cells + `StageStarsFor` + validation |
| `Source/CL9/Data/CL9Types.h` | `FCL9SiteStars`, settlement/run-over fields |
| `Source/CL9/GameMode/CL9GameState.{h,cpp}` | replicated `StageStars`, open check, run-over clear |
| `Source/CL9/GameMode/CL9GameMode.cpp` | `Settle` stars/unlock/clear, save/load, new run |
| `Source/CL9/GameMode/CL9SaveGame.h` | run save v3 |
| `Source/CL9/Items/CL9DestinationBoard.{h,cpp}` | open sites only, stars in the prompt |
| `Source/CL9/UI/CL9DawnWidget.cpp`, `Source/CL9/UI/CL9GameOverWidget.cpp` | stage lines, clear variant |
| `Source/CL9/GameMode/CL9StageStarsTests.cpp` (new) | arithmetic test |
| `Content/Data/CL9Rules.uasset`, `Content/Strings/CL9_Strings.csv` | cells, ko strings |

## Must-Preserve
- 7i settlement semantics (pass/miss, penalty, company travel), 7a wipe penalty, game-over path for `bWipeEndsRun`, other sessions' uncommitted changes.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (run-structure semantics). A cheaper model is fine for strings/docs.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.

## Results (2026-10-13)
- Build: the first attempts stopped before compiling — Smart App Control blocked `dotnet.exe` from loading `Intermediate/Build/BuildRules/CL9ModuleRules.dll` (CodeIntegrity 3077, 0x800711C7). After the user turned SAC off: compile check OK, `CL9Editor` build OK (222 s).
- Automation: `CL9.Run.StageStars`, `CL9.Run.Imposter`, `CL9.Run.WipePenalty` — all Success.
- Listen 2 players (PIE, `Lvl_Henhouse` → company ↔ sites; henhouse rounds ended by a 10 s round clock, farm/pigfarm rounds by `cl9.night` because those site rows set their own round length):
  - henhouse period 1300 / 800 → `Stage: 'henhouse' settled with 2 star(s) — 'farm' opens`; company screen on host and client: `이번 목적지: 양계장 ★★☆`, `다음 목적지가 열렸다: 대농장 — 목적지 판에서 고른다` (glyphs render).
  - Board prompt `[E] 목적지: 양계장 ★★☆` / `[E] 목적지: 대농장`; three presses cycle farm → henhouse → farm (pigfarm closed).
  - farm 900 / 2600 → 0 stars, miss penalty as 7i, nothing opens (screen `대농장 ☆☆☆` + miss lines); farm again 10000 / 4550 → 3 stars, `'pigfarm' opens`; best stars kept (farm shows ★★★ afterwards).
  - pigfarm 20000 / 5200 → `the last destination: the run is cleared` → `Run cleared — day 12 (best day 12, 3 destination(s))`. Company settlement screen first; after closing it the clear screen on host and client: `클리어`, `12일차에 마지막 목적지까지 마쳤습니다`, `양계장 ★★☆ / 대농장 ★★★ / 돼지 농장 ★★★`, best day, host `[시작 화면으로]`, client `[나가기]`. `cl9.depart` → `NotLobby` (refused). The leave button itself was not pressed (unchanged start-lobby path).
  - Stars crossed three level travels (run save v3) — the clear screen lists henhouse from the first period.
  - `bStageStars` false: no `Stage:` line, no star lines on the screen, board cycles all three sites.
- Test cells restored (`bStageStars` true, `NightLength` 245), `CL9Rules` saved, PIE players back to 1, editor level back to `Lvl_Flat`.
- Noticed, not 7l: coins on screen were 0 at the first company visit because the day's orders expired with their late penalties (order system) — period income is separate and unaffected.
