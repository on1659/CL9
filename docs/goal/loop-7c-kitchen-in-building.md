# goal: loop-7c-kitchen-in-building

**Status**: done (2026-10-12) — see Results. Build order ⑦ step **7c** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decision 8 ("부엌은 건물 안 랜덤 위치 — 찾는 것 자체가 숙제다").

## One-line Goal
Every generated `kitchen` module gets one cooking station (`BP_CL9Kitchen`) that the host spawns at round start, so the team has to find the kitchen inside the building to cook, and the intro camera that flies to the kitchen shows that station.

## Background / Motivation (recon 2026-10-12)
- Stations exist only in the test level `Lvl_Flat` (3, hand-placed). `Lvl_Henhouse` / `Lvl_Farm` / `Lvl_PigFarm` have **no station anywhere** — neither in the lobby nor in the building. The `kitchen` module (2×3 cells, sockets E|W, height 400) carries only the kitchen sign, a workbench and a few props, in two prop sets picked per instance by a position hash (`PickVariant`). So "로비 조리대는 뺀다" has nothing to remove in stage levels; `Lvl_Flat` keeps its 3 stations (test level without a generator).
- The intro camera candidates are all 1st-floor `Special` modules + the exit (`CL9SiteLayout::IntroCandidates`), shuffled per power-on — the kitchen module is already one of the destinations.
- Generated geometry and props are built locally on every machine from the seed; a station is a replicated, host-owned actor (`ACL9Kitchen`, replicated `Cooking`), so it must be spawned by the host like the delivery jars.
- User decision 2026-10-12 (asked in this run): **one station per kitchen module**, in the workbench spot next to the sign.

## In-scope
1. **Data column `Fixtures` on `FCL9PropSet`** (module table): `FCL9FixturePlacement { TSoftClassPtr<AActor> ActorClass; FVector Local; float Yaw; }` — actors the host spawns at round start, in the same pre-rotation module-local frame as props. Load check: class set, location inside the footprint.
2. **`ACL9SiteMap::GetFixtureSpawns`**: world transforms of the fixtures of each placed module's chosen prop set. The set is chosen by the same helper the prop builder uses (one function, so props and fixtures can never disagree).
3. **Host spawn** in `ACL9GameMode::SpawnNightPopulation`, per site map, **before** items and monsters (they spawn with `AdjustIfPossibleButAlwaysSpawn`, so anything overlapping a station is pushed clear). Spawned fixtures join `NightSpawned` (removed with the night population). One log line per map.
4. **Kitchen module data**: in both prop sets the workbench is replaced by a `BP_CL9Kitchen` fixture, back to the wall, front into the room:
   - set 0 (south wall, sign at x 500): station at local (250, 94, 0), yaw 90 → footprint x 97–440, y 22–156.
   - set 1 (north wall, sign at x 500): station at local (750, 1406, 0), yaw −90 → footprint x 560–903, y 1346–1480.
   (Station set: pivot at floor centre, +X front, x −74…+59.5, y −190…+152.6 incl. shelf, height 304 — docs/asset/map/kitchen.md. Wall inner faces at y ≈ 20 / 1480 from the sign and workbench placements.)
5. **Docs**: plan/data/module.md (`Fixtures` column, kitchen row), systems/map.md (fixtures), systems/cooking.md ("placed by the generator" now true), decisions.md, build-order 7c → done, goal README row.

## Out-of-scope
- Moving or removing `Lvl_Flat` stations; cooking rules, heat, gauges, kitchen dressing.
- Making the intro camera always show the kitchen to someone (assignment stays random per power-on, existing K9/K10 design).
- Migrating the delivery jar to `Fixtures`.
- Nav-mesh rebuild around the spawned station (same as jars).

## Acceptance Criteria
- [x] `CL9Editor` builds; automation tests `CL9.Map` still pass (except the pre-existing `CL9.Map.Districts` failure — see Results).
- [x] ModuleTable kitchen row read back: each prop set has one `BP_CL9Kitchen` fixture and no workbench.
- [x] PIE `Lvl_Henhouse`: night-start log shows one station placed per kitchen module; the station stands inside the kitchen module against a wall (floor trace / position check), and cooking works on it (egg on the plate → cooked).
- [x] PIE `Lvl_Farm` (4 kitchens) and `Lvl_PigFarm`: one station per kitchen module.
- [x] No station in the lobby / truck area of stage levels.
- [x] Intro: in a round where the power-on assigns the kitchen module to the player, the station is inside the camera's view during the hold (view-cone check) — recorded frame as evidence.

## Added during implementation
- **Intro look-at for fixtures** (`ACL9PlayerController`, intro path setup): the first measurement showed the camera arriving in the kitchen module and holding with the station exactly 90° to the side — the look-at for non-jar modules was the path's end cell, i.e. the camera kept its travel heading. Now, when the target module has no jar but has a fixture, the camera looks at the fixture origin + 130 cm (`IntroCamFixtureLookZ`, between the plate at 90 and the hood at 200–304). Clients compute the same spot from the layout (`GetFixtureSpawns`), so the replicated station does not have to arrive first. Assignment stays random.

## Results (2026-10-12)
- **Builds**: two `CL9Editor` builds, both first-try (42 s, 15 s). Data saved via MCP and read back.
- **Placement** (`Lvl_Henhouse`, log `Night 1: 1 fixture(s) placed — BP_CL9Kitchen_C in module 12 (kitchen) at V(X=2806, Y=-750) yaw 180`): traces from the station — back wall at **74 cm** at heights 50/150/250 (= the set's back depth, flush), floor `SM_Henhouse_Floor_500_A` below, side walls 230 / 730 cm, front open. Screenshot shows the full set (hood with kitchen sign, backsplash tools, pilot flames); the room sign sits beside it.
- **Cooking**: an egg on the plate → `egg/None → egg/cooked after 12.0s on 'BP_CL9Kitchen_C_0'`.
- **Counts**: henhouse 1, farm **4** (modules 13/15/17/19), pigfarm **2** — one per kitchen module; lobby after the round: **0** stations.
- **Intro**: before the look-at change — camera held at the kitchen with the station at 90° (never in view). After — in the round where the kitchen was assigned (log `look-at V(X=2650, Y=-1406, Z=130) (fixture)`), the station was centred (angle 0.0°), unblocked, from 5.84 s to 9.19 s after power-on; hold screenshot shows the set at the far wall.
- **Tests**: `CL9.Map` + `CL9.Run` — 13 Success, `CL9.Map.Districts` Fail (`slot 1: the smallest ground floor had 105 modules of a wanted 238`) — pre-existing (noted in `.claude/runs/giant-districts-laptop/report-gameplay.md`), a pure layout test with test-made modules, untouched by this step.
- 7b still holds on these runs (no night item on yard ground).
- Editor left on `Lvl_Henhouse`, no dirty packages, PIE play settings back to ListenServer · 2 clients.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9ModuleDef.{h,cpp}` | `FCL9FixturePlacement`, `FCL9PropSet::Fixtures`, load check |
| `Source/CL9/World/CL9SiteMap.{h,cpp}` | shared prop-set choice, `GetFixtureSpawns` |
| `Source/CL9/GameMode/CL9GameMode.cpp` | host spawn in `SpawnNightPopulation` |
| `Content/Data/ModuleTable.uasset` | kitchen row prop sets |
| `Content/Items/BP_CL9Kitchen` | the station (unchanged) |

## Must-Preserve
- Prop set choice and prop placement unchanged for every module (same hash, same transforms).
- Jar placement, spawn points, depth bands, power-switch cell rules unchanged.
- `Lvl_Flat` stations and all cooking behaviour unchanged.
- Other sessions' uncommitted changes in the touched files are kept.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (shared generator/spawn code, replication). A cheaper model is fine for doc edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Editor / build / PIE via the lock CLI (`docs/workflow/sessions.md` "잠금"), lock commands issued alone.
