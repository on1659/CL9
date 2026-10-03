# goal: giant-stage

Status: spec draft (2026-10-04) — for a **new session** (`/autogoal`, expect HEAVY: planning panel). Open Questions gate implementation.

## One-line Goal
A stage whose generated grid is **5–10× the farm** (farm = 200 ground-floor modules → 1,000–2,000 modules, ~1.1 km a side) that
still builds, navigates, performs and plays as one night.

## Background / Motivation
- User (2026-10-04): "make it big — the map can be 5× or 10× bigger than now", for **both** the generated grid and the outdoor
  backdrop (docs/goal/outdoor-backdrop.md); asked which base → **farm** (not henhouse).
- Current biggest: `farm` 200 modules (day 1 = 170 + upper), ~350–390 m grid, night 480 s, ~25 monsters day 1, editor PIE ~10 ms.
  How stages are made: docs/systems/stage.md (`Tools/Unreal/stages/<alias>.json` + `setup_stage.py`).

## What breaks at this size (facts from the farm work — the design must answer each)
| Area | Today | At 1,000–2,000 modules |
|---|---|---|
| Grid cap | `Span = clamp(ceil(sqrt(N·6))·2, 16, 128)` cells (CL9SiteLayout.cpp) | needs ~156–220 cells → cap must rise, or a different layout scheme (districts) |
| Generation | branch phase + fill pass; 200 modules fine (`CL9.Map.LargeSite`) | stall/fill behaviour unmeasured; time per layout unmeasured; all clients generate it |
| Nav | dynamic navmesh over the whole bounds; farm ~2,000 tiles built in < 20 s | ~12,000+ tiles; likely needs navigation invokers / streaming nav, or a tile-size change |
| Rendering | ISM per seg kind; farm ~3,400 instances + ~650 props, 10 ms | 5–10× instances; needs HLOD / culling / distance strategy; lights (`LampDensity`) |
| Walk time | spine ~60 modules ≈ 1 min one way | spine 300–600 modules ≈ 4–6 min one way at ~4.6 m/s |
| Night | 480 s for farm (stage.md estimate `160 + 5.3 × spine`) | estimate gives 30–50 min — the "one night" loop changes |
| Population | zones by depth, counts × day curve | counts, spawn cost, AI tick cost; maybe spawn/sleep by distance |
| Return jars | `SpecialCopies` 4 | many more delivery rooms, or vehicles/shortcuts |

## In-scope
- Design decisions for every row above (planning panel), then implementation of what the panel settles.
- A new stage row + level (name TBD) made through `setup_stage.py`, with measured QA (`qa_farm_stage_pie.py`: nav coverage,
  monsters on navmesh, frame time; layout tests at the new size).
- Coordinated with docs/goal/outdoor-backdrop.md — the backdrop must sit outside this stage's largest grid.

## Out-of-scope
- Shrinking or changing henhouse/farm/pigfarm.
- Economy redesign beyond what the night-length decision forces (flag to balance if needed).

## Acceptance Criteria (to be finalised after the panel)
- [ ] Layout tests at the target size: every seed builds, connected, ≥ 90% of budget, generation time recorded per layout.
- [ ] Scripted PIE: monsters all on navmesh, nav covers the grid within an agreed time, frame time within an agreed budget.
- [ ] One night is playable start to finish (truck → deep zone → back) within the agreed night length.

## Execution Notes
- Lead: Claude Fable 5.1, reasoning high (architecture-heavy). Planning panel recommended (HEAVY).
- Subagents: docs/workflow/sessions.md "모델 배분" — generator/nav/perf design and root-cause work = opus; table rows, stage
  JSON, docs = sonnet.
- This document cannot enforce the model — the executing session's `/model` decides; confirm with the user if below.
- Pitfalls: see docs/goal/outdoor-backdrop.md "Known pitfalls" (locks, commandlets, saved navmesh in duplicated levels).

## Open Questions (ask one at a time)
1. **Exact size** — 5× (1,000 modules) or 10× (2,000)? Or 5× first, then 10× if it holds?
2. **Night flow** — keep one long night (30–50 min), or change the loop (e.g. the truck can move to inner entrances, multiple
   jars/exits, or the night is split into several trips)? This decides walk-time and night-length design.
3. **Shape** — one huge connected building, or several building clusters (districts) joined by outdoor yards/roads? Clusters
   are friendlier to nav/streaming and read better at this size.
4. **Population** — scale monsters/items with area (5–10×), or keep density lower and rely on depth zones?
5. **Multiplayer target** — still 1–4 players on a listen server; is that the size we optimise for (performance budget)?
