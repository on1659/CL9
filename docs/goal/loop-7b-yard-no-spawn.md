# goal: loop-7b-yard-no-spawn

**Status**: done (2026-10-12) — see Results. Build order ⑦ step **7b** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decision 6 ("마당에는 아이템·몬스터가 없다 — 모든 아이템은 건물 안이다").

## One-line Goal
Outdoor (yard) modules stop being spawn spots for monsters and items, and the `outer` depth-zone counts of the farm and pig-farm stages are re-set for the indoor spots that remain.

## Background / Motivation
- Spawn spots are layout points; each module row's `PointPurposes` bitmask (`Monster` 1 · `Loot` 2 · `Jar` 4 · `Patrol` 8) decides which purposes its cells offer (`CL9SiteLayout::BuildPoints`). The rule "where things may spawn" already lives in the module table, not in C++.
- Recon (2026-10-12, ModuleTable read in the editor): only two modules are outdoor — `yard_gate` (Start, already `0x0`) and `yard_open` (Normal, `0xB` = Monster | Loot | Patrol). `yard_open` can be placed anywhere by the generator, not only at the entrance. `Patrol` is not read by any code today.
- User decision 2026-10-12 (asked in this run): "outer 구역 수치를 건물 안 기준으로" means **re-set the `outer` zone counts** for the indoor spots that remain (not a docs-only change, not a depth-origin change).

## Measurement (2026-10-12, PIE, day fixed at 5, seeds 101–106, `yard_open` 0xB vs 0x8 in memory)
| Site | zone | points before → after | ratio |
|---|---|---|---|
| farm | outer (0.00–0.35) | 1753 → 1627 | 0.928 |
| farm | middle | 3066 → 2922 | 0.953 |
| farm | deep | 1653 → 1608 | 0.973 |
| pigfarm | outer (0.00–0.40) | 846 → 801 | 0.947 |
| pigfarm | middle | 1001 → 947 | 0.946 |
| pigfarm | deep | 662 → 635 | 0.959 |
Source: log lines `Night N zone <z> (depth …, M monster / L loot points)`; per-seed numbers in the run transcript. Keeping `Patrol` leaves the yard points in the layout, so the depth scale (`MaxHops` over all points) is unchanged — only spawn membership changes.

## In-scope
1. **Module table**: `yard_open.PointPurposes` 0xB → **0x8** (Patrol only). `yard_gate` stays 0x0.
2. **Outer zone counts** — keep the same density per remaining indoor spot: new = round(old × outer ratio).
   - farm outer (×0.928): chicken 8–10 → **7–9**, sheep 3–4 → **3–4**, egg 14–18 → **13–17**.
   - pigfarm outer (×0.947): chicken 4–6 → 4–6, pig 1–2 → 1–2, egg 8–10 → **8–9**, meat 4–6 → 4–6.
   - Written to the `MapTable` rows `farm` / `pigfarm` (Zones[outer]) and to the stage specs `Tools/Unreal/stages/farm.json` / `pigfarm.json` (the specs are the source of truth for those rows; `setup_stage.py apply` is not re-run because it also rewrites levels).
3. **Docs**: plan/data/site.md (decision implemented, outer rows, the old "safe yard with only eggs" line marked as superseded), plan/data/module.md (`yard_open` Patrol only), systems/map.md (yard no-spawn), decisions.md, build-order 7b → done, goal README row.

## Out-of-scope
- middle / deep counts (they also lose 3–5 % of spots — reported, not changed), henhouse site-level spawns (no zones), `Lvl_Flat` ring spawns around the truck (test level without a building).
- The yard "tension element" (open question 8 of loop-rework), mid-round spawns (7e), monster patrol paths.
- A C++ rule or load check tying `bOutdoor` to spawn bits (the table already owns this rule).

## Acceptance Criteria
- [x] ModuleTable saved with `yard_open` 0x8, read back.
- [x] MapTable `farm` / `pigfarm` outer zone rows read back with the new counts; stage JSONs match.
- [x] PIE `Lvl_Farm` and `Lvl_PigFarm`: the night-start logs show the new outer bases, and no spawned item stands on a yard floor piece (floor trace under every spawned actor); monsters seen on the yard are wandering ones that walked there.
- [x] PIE `Lvl_Henhouse` still starts a night with items and monsters placed (regression).

## Results (2026-10-12)
- **Data** (editor Python + MCP `save_assets`, read back): ModuleTable `yard_open.PointPurposes` 0xB → 0x8; MapTable farm outer chicken 7–9 · sheep 3–4 · egg 13–17, pigfarm outer egg 8–9 (others unchanged); `farm.json` / `pigfarm.json` outer lines match. No code change, no build.
- **Logs**: farm `zone outer … chicken … (base 7~9 …)`, `egg … (base 13~17 …)`; pigfarm `zone outer … egg ×9 placed (base 8~9 …)`.
- **Floor trace** (`SystemLibrary.line_trace_single` down from every `CL9Monster` / `CL9Item`, hit static mesh name):
  - Detector check — farm night with `yard_open` back at 0xB in memory: **2 night items on `SM_Henhouse_YardGround_500_A`**.
  - With 0x8: farm 4 nights, pigfarm 4 nights, henhouse 3 nights — **0 items on yard ground**. One farm monster on yard ground (state `WANDER`) in both the 0xB and the 0x8 runs — walked there, not spawned.
  - The 3 items on `SM_CL9_ExteriorGround_100_A` every night are the level-placed start items near the truck (`BP_CL9Item_C_0`, `BP_CL9FlashItem_C_0/1`), not night spawns.
- **Henhouse regression**: night 1 egg ×17 · meat ×12 · 8 monsters, all on indoor floor.
- Editor left on `Lvl_Henhouse`, no dirty packages, PIE play settings back to ListenServer · 2 clients.


## Related Files / Modules
| File | Role |
|------|------|
| `Content/Data/ModuleTable.uasset` | `yard_open.PointPurposes` |
| `Content/Data/MapTable.uasset` | `farm` / `pigfarm` Zones[outer] counts |
| `Tools/Unreal/stages/farm.json`, `pigfarm.json` | stage specs (source of truth for those rows) |
| `Source/CL9/World/CL9SiteLayout.cpp` | `BuildPoints`, `PointsInDepthBand` (read only) |
| `Source/CL9/GameMode/CL9GameMode.cpp` | `SpawnNightPopulation` (read only) |

## Must-Preserve
- Depth bands keep the same scale (yard points stay as Patrol points).
- Jar placement (delivery rooms) and the power-switch cell exclusion are untouched.
- Other sessions' uncommitted changes in MapTable / stage JSONs / docs are kept.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (measurement design and balance arithmetic); a cheaper model is fine for the doc edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Editor / PIE via the lock CLI (`docs/workflow/sessions.md` "잠금"), lock commands issued alone.
