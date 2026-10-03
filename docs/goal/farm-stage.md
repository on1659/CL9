# goal: farm-stage

Status: implemented and verified headless + scripted PIE (2026-10-03, autogoal LIGHT — user away, decisions taken autonomously and recorded below). Not committed. Human play-feel pass still pending.

## One-line Goal
Ship one playable level, `Lvl_Farm`, that runs the whole loop (truck arrives → generated map → monsters and ingredients
placed → raid → cook/sell at the food truck) on a generated map about **5× the henhouse ground floor**, with **where things
are placed driven by the site row** (depth zones), and with **the level → stage mapping held in data**.

## Background / Motivation
User (2026-10-03): "We can build maps, place items, have monsters, cook, and the truck can come to the start — I want it all
merged into one play. Make a new level with a map about 5× bigger than the warehouse(=henhouse) one and place things in a
balanced way. Placement must be data-driven too, so we need something that manages stages."

What already exists: `UCL9SiteDef` (MapTable row) already is the per-destination stage row — generator params, monster/item
counts, lighting. What is missing for a 5× map:
- Placement is hard-coded: monsters go to the deepest 60% of points, items anywhere. On a 5× map that gives no structure —
  no "the deeper you go, the richer and the more dangerous".
- The night clock is global (`CL9Rules`); site.md already specs per-site overrides ("비우면 rules 전역값") but they were never
  implemented. A 5× map cannot be raided in 245 s.
- Which site a level plays is `BP_CL9GameMode.DefaultSiteAlias` — one value for every level that uses that game mode. A new
  level would need a new game-mode BP just to point at a different row.
- The generator caps the grid at 64 cells a side.

## In-scope
1. **Stage resolution by level (data).** New `UCL9SiteDef.Level` column (soft `UWorld`). `ACL9GameMode` picks the site row
   whose `Level` is the current map; if none matches it falls back to `DefaultSiteAlias` (so `Lvl_Flat` / old setups keep
   working). Adding a stage = one MapTable row + one level.
2. **Depth zones (data).** New `UCL9SiteDef.Zones` (`TArray<FCL9SiteZone>`): each zone = `{ Name, DepthMin, DepthMax,
   MonsterSpawns, ItemSpawns }`, depth = point hop distance / max hop distance (0 = entrance, 1 = deepest). When a site has
   zones, each zone spawns its own entries on the Monster/Loot points inside its band; the site-level `MonsterSpawns` /
   `ItemSpawns` still spawn the old way on top (henhouse has no zones → unchanged).
3. **Per-site night clock.** `UCL9SiteDef.NightLength`, `DarkBase`, `DarkMin` (0 = use `CL9Rules`). Day curve still multiplies
   night length.
4. **Generator size cap** 64 → 128 cells a side (formula unchanged; henhouse sizes ≤ 36 so its layouts/hashes don't move).
4b. **Generator fill pass** (found in testing): on a 200-module budget the branch phase stalls (48 consecutive misfits) and
   left some seeds at 70/200. A fill pass runs **only when budget is left**: rescan open sockets, try Passage → Normal →
   DeadEnd per spot. Henhouse runs that already meet their budget never enter it.
4c. **`SpecialCopies` column** (found in testing): Special rows (delivery = jar room, kitchen) are placed this many times.
5. **Data:** MapTable row `farm` (values below), `henhouse.Level = Lvl_Henhouse`.
6. **Level:** `Content/Maps/Lvl_Farm.umap` duplicated from `Lvl_Henhouse` (truck pad / food truck / kitchen / counter /
   lighting / flashlight start kit come along), nav bounds and ground resized to the farm grid.
7. **Tests:** `CL9.Map.LargeSite` (200-module generation: connected, no overlap, one exit, size reached) and
   `CL9.Map.DepthBand` (band filter partitions points, depth monotone).
8. **Docs:** site.md (new columns + farm row), map.md / raiding.md (zones, level resolution), decisions.md entry.

## Out-of-scope
- A new module set / theme for the farm (no farm art exists; it reuses `henhouse` modules — `Theme` is the swap point later).
- Destination-select UI (build-order 4g). Level choice = which map you open.
- Economy re-tune (prices, orders). Values below are drafts for PIE.
- Nav build-time optimization beyond sizing the bounds.

## Farm row (draft values — rationale)
| Column | Value | Why |
|---|---|---|
| Generator / Theme / CellSize | Henhouse / henhouse / 500 | only module set we have |
| BaseModuleCount | 200 | 5 × henhouse 40 (ground floor) |
| MaxFloors / UpperFloorShare / Stairs | 2 / 0.25 / 3~5 | keeps stairs content without multiplying the map again (~50 upper modules) |
| MainPathShare | 0.30 | 40% of 200 = 80-module single spine is a corridor march; 60 is enough depth |
| LoopChance | 0.35 | bigger map needs more escape loops |
| PassageShare / DeadEndShare | 0.40 / 0.30 | henhouse-like rhythm |
| LampDensity | 0.35 | 5× rooms → keep light count (perf) near henhouse |
| SpecialCopies (new column) | 4 | first headless run showed 1 jar on the whole 5× map — 4 delivery rooms + 4 kitchens |
| bBuildCeilings / lighting / EntryOffset | inherited from the henhouse row | same look as henhouse |
| EntryOffset | 1000 | same truck yard |
| NightLength / DarkBase / DarkMin | 480 / 360 / 240 | deep zone one-way ≈ 3.7× henhouse spine; ~2× clock leaves the same "go deep and come back" budget |
| BaseDanger / ValueRate / UnlockDay | 1.0 / 1.0 / 1 | no economy change |
| Zone `outer` 0.00–0.35 | chicken 8–10, sheep 3–4 · egg 14–18 | safe-ish onboarding near the truck, eggs only |
| Zone `middle` 0.35–0.70 | chicken 10–12, sheep 2–3, goose 2–3 · egg 16–20, meat 8–10 | alarm birds guard the middle |
| Zone `deep` 0.70–1.00 | chicken 6–8, pig 2–3, raccoon 1–2 · egg 10–12, meat 14–18 | killers + most meat at the bottom |
| Site-level MonsterSpawns / ItemSpawns | empty | everything comes from zones |

Totals ≈ 34–45 monsters, 40–50 eggs, 22–28 meat (henhouse: 8–13 / 16–20 / 8–12).

## Acceptance Criteria
- [x] Editor build `CL9Editor` succeeds.
- [x] `CL9.Map.*` automation tests pass, including the two new ones (6/6, headless `Automation RunTests CL9.Map`).
- [x] `MapTable` has a valid `farm` row (no validate error after switching DisplayName to string key `Site.farm.Name`) and `henhouse.Level` set.
- [x] `Lvl_Farm` exists; play logs `Stage: level /Game/Maps/Lvl_Farm plays site 'farm'`, `213 of 213 modules` (day 1 = 170 ground +
      upper), zone lines for outer/middle/deep, `Night 1 at 'farm' started — 480s`, `4 jars placed in 4 delivery rooms`.
- [x] `Lvl_Henhouse` PIE still resolves `henhouse`.
- [x] Docs updated (site.md columns + farm row, map.md, raiding.md, decisions.md).
- [x] Added during verification: monsters actually move on the farm — after removing the level-saved navmesh, 25/25 monsters are on
      navmesh and the nav map covers the whole grid (`Tools/Unreal/qa_farm_stage_pie.py`, `Saved/QA/stage_pie_Lvl_Farm.json`); editor PIE
      average frame 10 ms.

## Verification Findings (outside scope — for the user)
- `Lvl_Henhouse` nav bounds are far smaller than its generated map; the same QA shows 1/5 monsters on navmesh and none moving. Same fix
  as the farm (resize bounds + drop saved navmesh) would apply; not done — the level is shared with other sessions' work.
- `wallStub` pieces stretch −50% (pre-existing warning, also on henhouse).

## Related Files / Modules
| File | Role |
|---|---|
| Source/CL9/Data/CL9SiteDef.{h,cpp} | site row — new `Level`, `Zones`, night clock columns + validation |
| Source/CL9/World/CL9SiteLayout.{h,cpp} | grid cap, `PointsInDepthBand` |
| Source/CL9/World/CL9SiteLayoutTests.cpp | new tests |
| Source/CL9/GameMode/CL9GameMode.{h,cpp} | site resolution by level, zone spawning, night length override |
| Source/CL9/GameMode/CL9GameState.{h,cpp} | DarkBase/DarkMin override |
| Content/Data/MapTable.uasset | `farm` row, `henhouse.Level` |
| Content/Maps/Lvl_Farm.umap | the new level |
| Tools/Unreal/setup_stage.py + Tools/Unreal/stages/*.json | headless script that creates a stage's row and level from a JSON spec (repeatable; replaced the farm-only `setup_farm_stage.py` the same day) |

## Must-Preserve
- Seed-only replication: layout must stay deterministic; spawns stay host-only.
- Henhouse layouts (hash per seed) must not change.
- Sites without zones behave exactly as before; `Lvl_Flat` still uses `flat` via `DefaultSiteAlias`.
- Other sessions' uncommitted edits in the touched files stay intact (surgical edits only, no commit of others' work).

## Execution Notes
- Recommended model: Claude Opus 5.5 / Fable 5 for the spawn-zone and generator-scale judgement; mechanical items (columns,
  docs) fine on Sonnet.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below
  the recommendation, surface it to the user and confirm before proceeding.

## Fairness Constraints
- Spawn rolls are host-only (`FMath::Rand*` on the server), layout is seed-deterministic on every machine.

## Existing Integration Contract
- `ACL9GameState::SetSiteAndDanger` / `SetSiteMap(seed)` → `RebuildSiteMap()` on every machine reading `CurrentSiteAlias`.
- `ACL9SiteMap::Rebuild(World, Site, Registry, Seed, Day)` unchanged.

## Follow-up (2026-10-03, same day — user: "make one more stage and write a guide from it")
- Generalised the farm script into `Tools/Unreal/setup_stage.py <spec.json> [inspect|apply]`; stage specs live in `Tools/Unreal/stages/`
  (`farm.json`, `pigfarm.json`). Re-applying `farm.json` reproduced the same row and level.
- Second stage `pigfarm` (돼지 농장, `Lvl_PigFarm`): single floor, 100 modules, few loops, dark, pig-heavy meat run — values and rationale
  in docs/plan/data/site.md. Scripted PIE: site resolved, 85/85 modules day 1, all monsters on navmesh, 2 delivery rooms, ~10 ms frames.
- Guide: docs/systems/stage.md (linked from docs/systems/README.md and site.md).
