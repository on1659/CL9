# goal: outdoor-backdrop

Status: spec draft (2026-10-04) — written at the end of the farm-stage session for a **new session** to pick up with `/autogoal`.
2026-10-11 update: the user delegated monster/background art choices. See
[the art production plan](../asset/art-production-plan.md), E00 and the asset bundles, for the selected mood, asset list,
shared/near layering, asset sourcing, and test-level treatment (questions 1, 2, 4, 5, 6 below).
Those art choices no longer require re-confirmation. The existing large walkable-area intent remains;
exact placement/bounds still depend on current giant-stage geometry. This note records direction, not implementation completion.

## One-line Goal
Build the world **outside** the generated site grid like a normal hand-made game level — a horizon, terrain, trees, fences, a road
the truck comes in on — shared by every stage, so no stage looks like an empty test zone.

## Background / Motivation
- User (2026-10-03): "the outside is just a test zone with a bright skybox" → that session made it dark (dusk sky
  `MI_CL9_DuskSky` + dark dirt ground `M_CL9_GroundDark`, `Tools/Unreal/outdoor_look.py`, docs/systems/light.md "바깥 모습").
  It is still an empty flat plane to the horizon.
- User then asked "can't we build a level outside?" and wants it made "as if building a normal game level".
- Stages today: `henhouse` (Lvl_Henhouse), `farm` (Lvl_Farm, ~390 m grid at day 10), `pigfarm` (Lvl_PigFarm, ~280 m), `flat`
  (Lvl_Flat, test). How stages work: docs/systems/stage.md. The site grid is generated per seed and its footprint changes per
  stage and per day — the outside must never overlap it.

## Proposed approach (recommended in the previous session — confirm in Open Questions)
Hybrid of two layers:
1. **Shared backdrop sub-level `Lvl_Outdoor`** streamed into every stage level — far scenery that does not need to fit the
   grid exactly: low hills ringing the horizon, treeline, distant utility poles / farm silhouettes, the dirt road from the
   horizon to the truck pad. Placed outside the **largest** stage grid (+margin). Edit once → all stages.
2. **Near dressing generated around the actual grid** (data-driven, seed-based, like the rest of the map): sparse dead trees,
   bushes, broken fence runs, rocks in a band just outside the built grid's outline. A site-row column (e.g. `OutdoorSet` +
   `OutdoorDensity`) picks what and how dense, so each stage can differ and new stages get it automatically through
   `Tools/Unreal/setup_stage.py` + `Tools/Unreal/stages/<alias>.json`.

Rejected in discussion: hand-placing per level (repeats per stage, gets covered by the grid on big days).

## In-scope
- Art: low-poly outdoor assets in the project style (docs/asset/README.md, design-concept rules) — Blender-made or approved
  free sources (see Open Questions). Each with collision where walkable, LODs, imported under `/Game/Maps/Outdoor/`.
- `Lvl_Outdoor` backdrop sub-level + streaming hookup in all stage levels and in `setup_stage.py` (new stages get it).
- Near-dressing generator pass + site-row columns + stage JSON fields + docs/plan/data/site.md rows.
- Walk boundary (invisible blocking volume) at the agreed distance.
- Lighting/fog tuned so the backdrop reads at dusk and at night (docs/systems/light.md values).
- Docs: docs/systems/stage.md, light.md, map.md, site.md; decisions.md entry; asset cards under docs/asset/.

## Dependency
- docs/goal/giant-stage.md (2026-10-04): the grid becomes 5–10× the farm (~1.1 km a side). The backdrop must sit outside the
  largest grid of every stage, so decide giant-stage's size/shape (its Open Questions 1 and 3) before placing the far backdrop.
  Near dressing (generated around the built outline) is unaffected by the size.

## Out-of-scope
- Changing the generated site interior, module set, monsters, economy.
- Fixing `Lvl_Henhouse` nav bounds (separate pending user decision — see docs/systems/map.md).

## Acceptance Criteria
- [ ] Every stage level (Lvl_Henhouse, Lvl_Farm, Lvl_PigFarm; Lvl_Flat if agreed) shows the backdrop in lobby and at night.
- [ ] A new stage made with `setup_stage.py` gets the backdrop and near dressing with no extra steps.
- [ ] Near dressing never overlaps the built grid or blocks the truck path / entrance door (check 10 seeds × day 1 and day 10).
- [ ] Monsters still on navmesh (`Tools/Unreal/qa_farm_stage_pie.py`: `on_navmesh_500 == monsters`), nav map covers the grid.
- [ ] Editor PIE average frame time stays within +2 ms of the current baseline (~10 ms on Lvl_Farm, same QA script).
- [ ] Screenshot set (truck side / side / from inside the grid looking out) at lobby and night, shown to and approved by the user.

## Related Files / Modules
| File | Role |
|---|---|
| Tools/Unreal/outdoor_look.py · apply_outdoor_look.py | current dusk sky + dirt ground (shared function, called by setup_stage.py) |
| Tools/Unreal/setup_stage.py · Tools/Unreal/stages/*.json | stage creation from a spec (row + level); hook the backdrop in here |
| Tools/Unreal/qa_farm_stage_pie.py · qa_outdoor_shot.py | scripted PIE QA (night sample, nav map, frame time) · lobby screenshots |
| Source/CL9/World/CL9SiteMap.{h,cpp} · CL9SiteLayout.{h,cpp} | grid build (outline of the built grid lives here) |
| Source/CL9/Data/CL9SiteDef.{h,cpp} | site row — new outdoor columns go here |
| Source/CL9/World/CL9NightLighting.cpp | per-phase lighting/sky/cloud switching |
| docs/systems/stage.md · light.md · map.md, docs/plan/data/site.md | how-to and values |

## Must-Preserve
- Seed-only replication; generated actors `bReplicates=false`; generation deterministic (near dressing too — same seed, same props).
- Henhouse layouts/hashes unchanged; `CL9.Map.*` tests stay green.
- Truck pad / food truck / kitchen / counter positions and the entry door path stay clear.
- Night lighting multipliers are calibrated on the level's baseline lights — do not change baseline light intensities.
- Other sessions' uncommitted work in shared files (Lvl_Flat, MapTable, Source) — surgical edits only, commit only own files.

## Execution Notes
- **Lead (main session): Claude Fable 5.1, reasoning effort high.** Art direction, screenshot judgement, layout decisions and
  the generator design are judgement-heavy.
- **Subagents — follow docs/workflow/sessions.md "모델 배분"** (standing rule: choose by the nature of the work, pass `model`
  explicitly; role agents default to `opus`). Applied to this task:
  | Work | Agent | Model |
  |---|---|---|
  | First pass of each new asset shape (tree, rock, fence, hill silhouettes) | cl9-art | opus |
  | Variants of an approved shape, LODs, import scripts | cl9-art | sonnet |
  | Sub-level placement scripts, streaming hookup, setup_stage.py wiring | cl9-map | sonnet |
  | Near-dressing generator C++ + site columns | cl9-map (C++) | opus |
  | Doc updates | cl9-docs | sonnet (haiku fine for index rows) |
- This document cannot enforce the model — the executing session's `/model` decides. If the session model is below the
  recommendation, tell the user and confirm before proceeding.
- Known pitfalls from 2026-10-03 (see memory work-progress + docs):
  - Locks: `py -3 .claude/hooks/cl9_lock.py …` must be its **own single command**. Order: `acquire compile` → write the .cmd
    (start it with `ping -n 15 127.0.0.1 > nul`) → `Start-Process` → `acquire commandlet --pid <PID>` (or `build --pid`).
  - Commandlet stdout shows Display+ only → log from Python with `unreal.log_warning`.
  - A level duplicated from another keeps the source's saved RecastNavMesh tile capacity → delete it (setup_stage.py does).
  - Material Noise input pin connects with `""`, not `"Position"`.
  - Automated PIE without MCP: `UnrealEditor.exe CL9.uproject /Game/Maps/Lvl_X -ExecCmds="py <script>"` with a
    `register_slate_post_tick_callback` state machine that ends by `quit_editor()`; screenshots via `HighResShot 1280x720`.
  - Editor build: `D:\Program Files\Epic Games\UE_5.8` only (registry lacks 5.8); editor closed → ~1–15 min.

## Open Questions (ask the user, one at a time, before implementing)
1. **Mood / reference** — e.g. "abandoned night farm, sparse and eerie like Lethal Company" vs "rural village surrounded by
   forest". Any reference image paths?
2. **What's out there** — confirm/edit the list: horizon hills, treeline, dirt road to the truck, dead trees, bushes, broken
   fences, rocks, utility poles, a ruined barn or two.
3. ~~Walkable range~~ — **answered 2026-10-04: make it big.** The outside area should be wide — on the order of 5–10× the
   grid's footprint, walkable — and the grid itself grows 5–10× from the farm (docs/goal/giant-stage.md). Still ask: where the
   invisible wall sits, and whether the far horizon beyond it is backdrop-only.
4. **Shared vs per stage** — confirm the hybrid (shared far backdrop + per-stage near dressing via site columns).
5. **Asset sourcing** — Blender-made only, or free sources allowed (Poly Haven CC0 / Poly Pizza CC-BY with credit /
   Sketchfab by license)?
6. **Lvl_Flat** — include the test level or leave it bare?
