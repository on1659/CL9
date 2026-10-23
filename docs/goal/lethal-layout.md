# goal: lethal-layout

> **Status**: implemented and verified headless + editor + PIE spawn check (2026-10-15, autogoal HEAVY, user asleep — "맵 알아서 잘 판단해서 리썰처럼 넣어").
> Not committed — the touched C++/table files also carry other sessions' uncommitted changes (see Results). Full bot round not run (Lvl_Henhouse nav bounds).
> Decisions 1–8 are the user's (2026-10-15 chat). Q3/Q4/Q5 were left to the lead and were settled by a four-lens design panel plus a
> cross-examination (read-only workflow `lethal-layout-panel`). Measurement sources: [../reference/lethal/lethal-map.md](../reference/lethal/lethal-map.md),
> [../reference/repo/repo-map.md](../reference/repo/repo-map.md), and the step-by-step visualisation artifact
> "리썰·리포 맵 생성기" (https://claude.ai/artifact/SKty4qmerXbr2iFEBeLwHw).

## One-line Goal

Generate the henhouse map the way Lethal Company's DunGen does: a main path whose length sets the map size, a branch on every free door
of every main-path module, branches that always end in a room, big rooms at most once per round, a maze section, depth windows that keep
stairs in the middle of the run, and data-driven door slots — all as module-table and site-table columns.

## Background / Motivation

User, 2026-10-15: "우리게임 진짜문제가 맵이 만들어지는데 그냥 크게만들어지는거지 뭔가 제대로 만들어졌다라고 생각되지않아" and
"계단도 지멋대로고".

The current generator (`GrowFloor`, `CL9SiteLayout.cpp`) picks modules by type pool + `Rarity` only (`PickCandidate`). It has no depth,
section, repeat or tip rules, so growing the module count just scatters the same 19 rows; stairs are mixed into the branch dice.

Measured from the Lethal install (2026-10-15, `sharedassets3.assets` + decompiled `DunGen.dll`), the designed feel comes from data rules:

| Rule | Lethal (factory `Level1Flow`) | CL9 before |
|---|---|---|
| Sections | 0–12% normal · 12–43% maze (T-junction weight 15) · 43–100% normal | none |
| Per-tile weights | `MainPathWeight` / `BranchPathWeight` × `DepthWeightScale` curve | type pool + `Rarity` |
| Repeats | big rooms `RepeatMode = Disallow` (once per dungeon) | unlimited |
| Branches | `BranchMode.Local`: each main tile rolls `BranchCount` 11–15 clamped to its unused doorways → every free door, depth 2–4 | budget shares |
| Branch tips | last tile from the room set (`BranchCapType.InsteadOf`), then `BranchPruneTags` | DeadEnd by chance |
| Loops | `DoorwayConnectionChance` 1.0, but doorways must coincide exactly | 0.25 on any open side |
| Size | main-path length × moon multiplier | total module count |
| Stairs | stair tiles have depth windows (manor stair 0 before 0.11 / after 0.79), main weight ≫ branch weight | no placement rule |

Correction recorded here: "22–25 branches" in `lethal-map.md` / `module.md` is the Global-mode value; all five Lethal flows are Local.

## Design Decisions

User (2026-10-15):

1. **Lethal style only.** No R.E.P.O. mode; one can be added later as another `ECL9Generator` value.
2. **Map size = main-path length.** Branches follow the main path's free doors. `SiteSizeCurve` multiplies the length (Lethal `LengthMultiplier`).
3. **Big rooms once per round; corridors fill the rest; a smaller map is accepted.** More room modules later = more table rows.
4. **Keep floors; add stair placement rules.** No Lethal height propagation.
5. **Everything indoors.** Flow uses no `bOutdoor` row; `yard_open` is not used; the start is a new indoor entrance row.
6. **Explicit door slots** per module (`DoorSlots`: cell + side, plus `Level` for stairwells). Matching happens only at slots; "free doors" = unused slots.
7. **Stairs by weight, guaranteed minimum, low cap** (henhouse 1–2; "계단 너무 많으면 위아래 와리가리 심해져").
8. **Specials are injected by depth** (delivery on the main path, kitchen as a one-module branch).

Panel-settled (lead, 2026-10-15 — do not relitigate during implementation):

- **Separate generator.** `ECL9Generator::Flow` + `FCL9SiteParams::bFlow`; `Generate()` dispatches; the old body moved verbatim into
  `GenerateHenhouse`, which never reads Flow columns. *(Rejected: inferring the generator from Theme or filled columns.)*
- **Henhouse isolation (Q5).** farm and pigfarm stay on the Henhouse generator. Existing rows' Henhouse-read fields (Type, Size, Sockets,
  Rarity, bOutdoor, Height, PointPurposes) are not edited. The new indoor `entrance` row has `Rarity 0`; Henhouse skips Rarity ≤ 0 in its
  pools *and* `ACL9SiteMap::Build` drops those rows before measuring the tallest room, so farm/pigfarm floor height cannot move.
  *(Rejected: converting `yard_gate` in place — it is shared.)*
- **Two-stage pick.** Pick a row by integer permille weight among rows that have at least one valid placement, then a placement uniformly.
  Per-placement weights would make rows with more ways to fit win more often than the table says.
- **Columns (final).** Module: `DoorSlots`, `MainWeight`, `BranchWeight`, `CapWeight`, `MazeWeight`, `DepthMin`/`DepthMax` (main-depth
  window), `Repeat` (Allow / NoImmediate / Once), `bInjectOnMain`. Site: `Generator = Flow`, `MainPathMin/Max`, `MazeMin/Max`,
  `BranchDepthMin/Max`; `LoopChance`, `UpperFloorShare`, `StairCountMin/Max` are reused. *(Rejected for now: tag lists, prune tags,
  reject pairs, named section arrays, curve assets — the spec's curves were all windows; structure below replaces prune/reject.)*
- **Atomic branches instead of pruning.** A branch is built under a snapshot; its last module comes only from `CapWeight` rows; if the
  cap cannot stand, the branch is shortened and retried, and rolled back entirely at length 0. No post-hoc deletion, so no upper-floor
  cell is ever left without support. *(Rejected: prune-then-compact — the riskiest code in the plan.)*
- **Stairs are side rooms off the main path.** Roll S in `StairCountMin..Max`; pick main steps by stair `MainWeight` × depth window
  (0.1–0.8), at least ⌈L/4⌉ apart; attach `stair_up` (rotation 0 — treads always rise toward +X) as a one-module branch before the
  specials and general branches; its ground-floor slots take no branches and no loops. A stair whose upper floor cannot reach
  `StairMinReach` (3) modules is demoted to a plain room. Keeping stairs off the main path keeps `FindCellPath` / intro camera
  candidates beyond them reachable.
- **Specials.** Delivery: `bInjectOnMain`, one main step pre-rolled in its window (0.45–0.65), only that row there. Kitchen: one-module
  branch on a main step in its window (0.4–0.8). A missing special fails the attempt.
- **Main path & retries.** Start = entrance at rotation 0, its west slot on the grid's west edge (truck side). Each main module attaches to
  the previous one only; up to 8 backtracks per attempt; last module from cap rows. Attempt k seeds `Seed` (k=0) or `HashCombine(Seed, k)`;
  20 attempts; if none succeeds, the best-scoring attempt is used (main length, specials, stairs, exit). Grid span `clamp(4L+12, 24, 128)`.
- **Upper floor (Q4).** In Flow, `UpperFloorShare` multiplies the rolled main length: `U = max(3·stairs, round(L × share))`, split per
  stair; each stair grows a short upper main path from its level-1 slots (cap at the end) and then branches. Henhouse value 0.5.
- **Loops (Q3).** Judged once after everything is built: only two facing free slots qualify; entrance slots and stair ground slots are
  excluded; one `FRand` per candidate. Henhouse Flow value 1.0 (Lethal's), to be tuned by play. Loops matter for our monsters: chicken and
  pig chase at 396 = walk speed and the pig executes players stuck at dead ends (pigfarm deliberately uses 0.15).
- **Hops = walking door distance.** After loops, every module's `Hops` is recomputed by BFS from the entrance (stairs span both floors).
  Depth bands, the emergency exit and the power switch read `Hops`; placement rules read the new `MainIndex` / `BranchDepth` /
  `RootModule` fields.
- **Exit & switch.** `CarveEmergencyExit` is unchanged (Sockets sides, DeadEnd first). `PickSwitchCell` returns `EntryCell` when the switch
  module is the (indoor) start; Henhouse never picks its outdoor start, so it is unchanged.
- **Hash.** `LayoutHash` mixes `FCrc::StrCrc32(Alias)` instead of `GetTypeHash(FName)` (name-pool order is process-local).

## Henhouse values (starting point — balance may retune after play)

Site row `henhouse`: `Generator Flow`, `MainPathMin/Max 9/11`, `MazeMin/Max 0.15/0.45`, `BranchDepthMin/Max 2/4`, `LoopChance 1.0`,
`UpperFloorShare 0.5`, `StairCountMin/Max 1/2`, `MaxFloors 2` (unchanged).

| Row | Door slots (unrotated cell, side) | Main | Branch | Cap | Maze× | Depth | Repeat |
|---|---|---|---|---|---|---|---|
| `entrance` (new, Start, 2×2, Rarity 0) | (0,0)W truck · (0,1)N · (1,1)E · (1,0)S | — | — | — | | | Once |
| `corridor` 1×2 | (0,1)N · (0,0)S | 1 | 0.6 | 0 | | | NoImmediate |
| `corridor_3` 1×3 | (0,2)N · (0,0)S | 1 | 0.6 | 0 | | | NoImmediate |
| `corridor_4` 1×4 | (0,3)N · (0,0)S | 0.5 | 0.3 | 0 | | | NoImmediate |
| `corner` 2×2 | (0,1)N · (1,0)E | 0.3 | 0.6 | 0 | | | NoImmediate |
| `stair_pass` 2×2 | (0,1)N · (0,0)S | 0.8 | 0.3 | 0 | | | NoImmediate |
| `cross` 2×2 | (0,1)N · (1,1)E · (1,0)S · (0,0)W | 4 | 4 | 0 | 3.75 (=15) | | Allow |
| `coop_hall` 3×3 | (1,2)N · (2,1)E · (1,0)S · (0,1)W | 1 | 0.8 | 1 | 0 | ≥0.15 | Once |
| `pen` 3×2 | (1,1)N · (2,0)E · (1,0)S · (0,1)W | 1 | 0.8 | 1 | 0 | ≥0.15 | Once |
| `feed_store` 2×3 | (1,2)N · (1,1)E · (0,0)W | 1 | 0.8 | 1 | 0 | ≥0.15 | Once |
| `incubator` · `tool_room` · `office` · `coop_small` | one slot on their open side | 0 | 0 | 1 | | | Allow |
| `storage` | (0,1)W | 0 | 0 | 0.6 | | | Allow |
| `delivery` (Special) | (1,1)N · (0,0)S | inject main 0.45–0.65 | | | | | Once |
| `kitchen` (Special) | (0,1)W · (1,1)E | inject branch 0.4–0.8 | | | | | Once |
| `stair_up` (Stair) | ground (0,0)W · (0,1)N · (0,0)S; upper (1,1)E · (1,1)N · (1,0)S | 1 (step pick) | | | | 0.1–0.8 | Allow |
| `yard_gate`, `yard_open` | none (outdoor — unused by Flow) | | | | | | |

Slots were checked against the real `PropSets`/`Fixtures` dump: the only `Always` props are interior pillars/beams in `coop_hall` and
`cross`; the kitchen counters sit on the north/south walls, away from the east/west kitchen slots. Side-ruled props already react to
whichever boundary cell becomes a door.

## Out-of-scope

- New room modules (art/level work) — decision 3.
- R.E.P.O. mode, height propagation, more than two floors in Flow.
- farm / pigfarm / districts on Flow (Validate rejects Flow + Districts).
- Spawn-count formulas. The smaller map raises monster/ingredient density per band; reported for balance (A5).

## Acceptance Criteria

Automation (`CL9.Map.Flow.*`, synthetic `FTestFlowModules` with the values above, seeds 1–200):

- [x] A1 a `Once` row appears at most once per run.
- [x] A2 every branch tip is a cap row, the kitchen, or a stair.
- [x] A3 every stair hangs off a main module with depth 0.1–0.8 and never opens onto the entrance.
- [x] A4 every module reachable; no shared cells; exactly one emergency exit; upper floor stands in ≥ 75% of runs.
- [x] A5 ground main path within 9–11 in ≥ 95% of runs; delivery + kitchen + full main path in ≥ 95%; module count, loop doors and stairs reported.
- [x] A6 the maze section holds a higher crossroads share than the rest.
- [x] A7 same seed → same layout and hash; `CL9.Map.Flow.RotateCell` agrees with `ModuleLocalToSite`.
- [x] A10 every door sits on slots on both sides; `Hops` changes by ≤ 1 across any door; entrance `Hops` 0.
- [x] Henhouse isolation: a Rarity-0 row does not change a Henhouse layout hash; all existing `CL9.Map.*` tests still pass.

Editor / PIE:

- [ ] A8 walk length re-measured with `Tools/Unreal/measure_site_traverse.py` and reported next to Lethal's smallest moon (62–97 s).
- [ ] A9 one PIE round on `Lvl_Henhouse`: arrival → power switch → ingredients → cook → deliver → home works; site log shows `Flow`.

## Related Files / Modules

| File | Role |
|---|---|
| `Source/CL9/World/CL9SiteLayout.h/.cpp` | Flow generator, dispatch, `RotateCell`, hash, switch cell |
| `Source/CL9/World/CL9SiteLayoutTests.cpp` | `CL9.Map.Flow.Rules` · `.Determinism` · `.RotateCell` |
| `Source/CL9/Data/CL9ModuleDef.h/.cpp` | `FCL9DoorSlot`, `ECL9ModuleRepeat`, Flow columns, validation |
| `Source/CL9/Data/CL9SiteDef.h/.cpp`, `CL9Types.h` | `ECL9Generator::Flow`, Flow site columns, validation |
| `Source/CL9/World/CL9SiteMap.cpp` | copies Flow params, filters rows per generator |
| `Source/CL9/Data/CL9DefRegistry.cpp` | Flow stage completeness check |
| `Content/Data/ModuleTable.uasset`, `MapTable.uasset` | henhouse values (applied in the editor) |

## Must-Preserve

- Seed replication: the host sends only the seed; every machine builds the same layout.
- Henhouse generator layouts (farm, pigfarm) — unchanged; existing tests untouched.
- Floors: `StairMinReach` 3, upper cells only over indoor ground modules, every floor reachable through stairs.
- One entrance + one emergency exit; `SealEdges`, `BuildPoints`, depth bands, power switch and intro camera helpers.
- Strings: none added.

## Execution Notes

- Recommended model: the strongest current Claude model for the generator and its tests (judgment-heavy, determinism-sensitive);
  a cheaper model is fine for applying table values and doc sync. This document cannot enforce the model — the session's `/model` decides;
  if the session model is below the recommendation, tell the user and confirm before proceeding.
- Build with `C:\Program Files\Epic Games\UE_5.8`, target `CL9Editor`; editor/build/PIE locks per [../workflow/sessions.md](../workflow/sessions.md).
- Table values are applied by an editor Python script that first writes a backup of the henhouse site row.

## Results (2026-10-15)

- **Build**: game-target compile check OK, `CL9Editor` build OK (75 s). Editor relaunched; no registry/validation errors.
- **Automation** (`Automation RunTests CL9.Map`, 16 tests): all pass except `CL9.Map.Districts`, which already failed before this work
  (same message as the 2026-10-14 record: "slot 1: the smallest ground floor had 105 modules of a wanted 238").
  `CL9.Map.Flow.Rules` (200 seeds): 18–51 modules (avg 32.7), 0.7 loop doors per run, 1.04 stairs, upper floor in 200/200 runs,
  200/200 complete (main path in range + delivery + kitchen); crossroads share 82% in the maze section vs 41% elsewhere.
  Existing Henhouse tests unchanged (Assembly 24.0 modules, 29% dead ends, 4.7 loop doors — same numbers as before).
- **Tables applied** (editor Python, backup of the henhouse site row first): 17 henhouse module rows got Flow values, new row `entrance`;
  `henhouse` site row → `Generator Flow`, main path 9–11, maze 0.15–0.45, branch depth 2–4, loop 1.0, upper share 0.5, stairs 1–2.
  farm and pigfarm rows unchanged (Henhouse). `Tools/Unreal/stages/farm.json` and `pigfarm.json` now pin `"generator": "HENHOUSE"`
  so re-running `setup_stage.py` (which inherits from henhouse) cannot flip them to Flow; `setup_stage.py` sizes a Flow grid as `4L+12`.
- **A8 walk length** (`measure_site_traverse.py`, seeds 873/101/202 × day 1/10): 28 modules on day 1, 31–42 on day 10, all two floors with
  one stair; truck pad → farthest cell navmesh path 63–108 m. The same measurement on the Henhouse generator at 40 modules gave 148–218 m
  (837c10). The map is roughly half the walking length — decision 3 accepted a smaller map; main-path length is the knob.
- **A9 PIE** (`Lvl_Henhouse`, seed 1299094273): site built by Flow (37 modules, two floors, one stair), power switch in the entrance at the
  truck-door cell, kitchen counter placed in the kitchen module, one jar in the delivery room, eggs 20 · meat 10 · six monsters spawned,
  no LogCL9 errors or warnings. Oblique editor screenshot (ceilings off for the shot only): `Saved/LethalLayout/henhouse_flow_seed873_day1.png`.
  A full bot round (`/cl9film`) was not run: the level's saved NavMeshBoundsVolume covers only part of the grid (known pitfall; decided on
  2026-10-14 to fix it with `setup_stage.py` NavBounds during the henhouse indoor rebuild — the tool now sizes Flow grids correctly).

### Open after this run

- Loops are rare (0.7 doors per run vs 4.7 before) because door slots must face each other exactly. With walk-speed chasers and the pig's
  dead-end executions this makes the henhouse a near-tree. First knob if play feels too deadly: add a second slot on the long sides of
  `coop_hall` / `pen` / `feed_store` / `cross`, or lower `NoImmediate` on corridors.
- Stairs average 1.04 although the cap is 2 — the second stair's upper wing usually cannot find three supported cells. Acceptable for
  "few stairs"; raise `UpperFloorShare` or relax spacing if two stairs are wanted.
- Navmesh bounds of `Lvl_Henhouse` must be refit (`setup_stage.py` NavBounds) before monsters can path across the whole Flow grid.
  **Done 2026-10-20** (`Tools/Unreal/fit_henhouse_nav_20261020.py`, [henhouse-supply-quota.md](C:\Work\CL9\docs\goal\henhouse-supply-quota.md)). The loop knob was tried the same day and reverted — see that doc, decision 4.
- Commit: the C++ files, `ModuleTable.uasset`, `MapTable.uasset`, `setup_stage.py` and the stage JSONs already contained other sessions'
  uncommitted work at the start of this run, so they are left for a hunk-level `/cl9 commit`. This run's own hunks: the Flow sections of
  `CL9SiteLayout.h/.cpp`, `CL9SiteLayoutTests.cpp` (from "Flow 생성기 (리썰 방식" to the end), `CL9ModuleDef.h/.cpp` (DoorSlot, Repeat,
  Flow columns, validation), `CL9SiteDef.h/.cpp` (Flow columns, validation), `CL9Types.h` (`Flow`), `CL9SiteMap.cpp` (Flow params, row filter,
  warning guard), `CL9DefRegistry.cpp` (Flow completeness check).
