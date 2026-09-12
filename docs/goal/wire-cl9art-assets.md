# goal: wire-cl9art-assets

Written 2026-09-11 (autogoal, HEAVY track — panel of 4 lenses + cross-exam settled the design below).

## One-line Goal
Every mesh under `Content/CL9Art` that is not referenced by any game asset becomes consumed by the game — through
data rows, BP/level edits, and the smallest generator changes that give the map parts and props a real consumer.

## Background / Motivation
`docs/asset/catalog.md` lists 45 delivered assets; only 12 (items, monsters, egg) are permanently wired. A grep of the
uasset import tables shows 33 CL9Art assets with zero referencers outside `CL9Art`. Worse, `DA_PieceTable` still
points at Dreamscape Farmlands meshes that commit `5da02ff` removed from the repository — the henhouse map is broken
for any clone. The user asked: "지금 안 쓰는 거 있으면 싹 다 사용 가능하게 적용해줘" and chose the full scope
including the prop placement system (module.md decision 7, previously 미구현).

## In-scope
- **A. Piece / seg-kind data (theme `henhouse`)** — `DA_PieceTable`: `wall`→Wall_500, `wallHalf`→Wall_250,
  new `wallQuarter`→Wall_125, `fence`→Fence_250, `door`→Door_A, new `floor`→Floor_500, new `ceiling`→Ceiling_500,
  new `yard`→YardGround_500, new `tread`→Tread_A. `DA_SegKindTable`: `floor`/`ceiling` Piece→own role, Placement
  `Grid`; new `yard` (Piece yard, Grid, H 30, blocks sight), new `tread` (Piece tread, **Tile**, H 30), new
  `barnStub` (Piece wallQuarter, Tile, same H as barn), new `wallStub` (Piece fence, Tile, 288);
  `barn`/`plug`/`flapIn` Height 630→**580** (user decision 2026-09-11).
- **B. Generator C++ (one commit)** — `ECL9SegPlacement::Grid` (two-axis tiling, bounds-centred, Z scaled to the box
  height, per-kind fit log line but no run-fit Warning); downward-growth rule extended to `Floor()||Yard()||Tread()`;
  `BuildSegments` emits `yard` for outdoor module floors, `tread` for stair treads, `barnStub`/`wallStub` for the two
  boxes flanking a door/flap gap; `door` exempted from the MissingKinds Error (it is absent by design); registry
  Needed list + `CL9.Map.Segments` assertions updated; `ACL9FlapIn::Setup` fits the leaf **width only** to the gap
  (yaw 90° when the opening runs along Y), keeps thickness/height natural, anchors it at the opening bottom, Body
  box unchanged (still blocks the full 580).
- **C. Prop system (own commit)** — `UCL9ModuleDef.PropSets : TArray<FCL9PropSet>`; `FCL9PropSet.Props :
  TArray<FCL9PropPlacement>{ Mesh, Local (pre-rotation cm from the module min corner, Z above floor top), Yaw,
  bSolid, Side, SideRule{Always, SideClosed, SideOpen} }`. `ACL9SiteMap` picks one set per module instance with
  `PickVariant(module centre, Floor*Storey)`, maps Local through `ModuleLocalToSite`/`RotateSide` helpers (unit-tested
  against `RotateSockets`), evaluates SideRule on every boundary cell the prop footprint overlaps, places ISM
  instances (BlockAll / NoCollision+Visibility-ignore), and warns when a solid prop AABB intersects a LevelPoint
  keep-out (260×260 around the cell centre) or a door/flap gap. Author 1–2 sets per henhouse module using Nest,
  Crate, FeedSack, FeedTrough, WorkLamp, Workbench, Signs ×4, Beam, Pillar (dims from `StaticMeshTools.get_bounds`).
- **D. Facility BPs + levels** — `BP_CL9Kitchen/Counter/Jar/TruckPad/TruckPanel` Mesh → CL9Art meshes, relative
  transform identity; `ACL9Kitchen` ctor adds `HeatZone->SetUsingAbsoluteRotation(true)`; placed instances in
  `Lvl_Flat` (5) and `Lvl_Henhouse` (pad, jar) re-grounded at z 0 / scale 1 / kitchen yaw 90; food truck body+steps
  as StaticMeshActors on the pad's −X side, yaw 180 (steps toward the pad), body centre ≈ pad.X − 650 — **provisional:
  the user will judge the spot visually** ("배치해봐야 알 거 같은데"); panel stays where it is.
- **E. Farmer A** — `DA_CharacterTable.farmer_a` SkeletalMesh→SK_Farmer_A_Body, ArmsMesh→SK_Farmer_A_Arms
  (Skeleton/ABPs unchanged).
- **F. Items** — `DA_ItemTable` rows `pot` (copy of frying_pan: Hand, Radius 30, no physics, Mesh SM_Pot_A) and
  `standing_lamp` (Hand, Radius 60, no physics, LightRadius 800, Intensity 5000 — values provisional, plan says
  미정) with `ActorClass` = new `BP_CL9LampItem` (parent `ACL9LampItem : ACL9LightItem`, the other session's
  2026-09-11 light-item class split): spawns lit and stays lit when placed/thrown (user: "내려두어도 불은 켜져
  있어야지"). (Replaces the earlier `bLitWhenFree` column idea — the row-level light flags were removed that day.)
- **G. Docs** — catalog `UE 연결` cells, asset-swap.md roles table (+ Grid mode, stub kinds, DoorWidth 250 note),
  module.md PropSets column = implemented, decisions.md entries, production-progress 재개 지점.

## Out-of-scope
- New meshes (Fence_125, Floor Stretch100, Wall_184). `wallStub` will warn at 0.50 — accepted.
- `door` seg kind row / DoorWidth 132 (user decision: keep 250).
- Moving the kitchen into the truck; equipment (shop) rows for pot / standing_lamp (price undecided).
- Deleting the now-redundant `*_Stretch100_*` meshes and test assets (`SK_RigProbe*`, `SM_CL9_UnitCube_100`).
- Apron yard between pad and gate; prop-provided LevelPoints; navmesh changes.

## Design Decisions (panel-settled — do not relitigate during implementation)
- Treads use **Tile**, not Stretch — `FitPieceToCell` already yaws 90° and centres by bounds; Stretch would smear the
  anti-slip top 1.89×. (Rejected: bounds-aware Stretch + auto-yaw — duplicate of FitPieceToCell, dead code once Grid
  lands; never put XRun/YRun variants in the tread row — hash-picked at random.)
- Floor/ceiling/yard use **Grid**; Stretch branch stays Size/100 for `box`. (Rejected: bounds-aware Stretch — the
  textured yard stretched 3× is the first thing seen every run.) Floor kind Height stays 30 even though the mesh is
  20 thick — changing it moves FloorHeight to 600, steps to 30 and un-fits the 27.42 tread.
- Door stubs are **distinct seg kinds** (`barnStub`/`wallStub`) emitted by `AddEdge`. (Rejected: `ShortPieces`
  search column — only one candidate can ever match at 125, muddles the per-kind fit report.) Wall_250 therefore
  still has no consumer after this pass — say so in the catalog.
- DoorWidth stays **250** (no `door` row). Door_A (132) and Tread (264) were authored for 132; consumed at 250 the
  tread tiles 2-up at 0.947 and the leaf is width-fitted 1.89×. Recorded in asset-swap.md.
- barn/plug/flapIn Height **580** = tallest module 550 + ceiling plate 30. (Rejected: 630 — pokes 20 cm through the
  floor-1 slab now that ceilings are off.)
- Prop sets live **inline on the module row** (module.md:51 as written). (Rejected: separate UCL9PropSetDef table —
  registry map + DefList rescan trap for sharing nobody asked for.)
- One `bSolid` flag per prop. (Rejected: separate bBlocksMove/bBlocksSight — no listed prop needs the split.)
- SideRule is evaluated on **every** boundary cell the prop footprint overlaps (a 300-wide nest spans two 250 gaps).
- Prop set pick reuses `PickVariant` (coordinate hash) — no second hashing scheme.
- Kitchen HeatZone fix is **one line in the C++ ctor**, not a BP CDO delta.
- Order: (1) data + BP/level + farmer + items, (2) generator C++ + new piece rows + seed run, (3) props — each a
  separate commit, each verifiable alone.

## Acceptance Criteria
- [x] `grep` of uasset import tables: every mesh in `Content/CL9Art` except tests, `*_Stretch100_*` and Wall_250 has
      ≥1 referencer outside `CL9Art`.
- [x] `cl9.seed 873` on `Lvl_Henhouse`: LogCL9 has no `has no usable piece` / `has no usable row` Errors; no run-fit
      Warning for `barn`, `barnStub`, `floor`, `yard`, `tread`; `wallStub` may warn at 0.50.
- [x] Automation `CL9.Map.*` green (Assembly·Determinism·Floors·Segments 2026-09-11; `CL9.Replication.Registration` crashes the editor on an engine assert unrelated to this work — do not run it in-editor) (Segments asserts Floor+Yard == modules, Yard == outdoor modules, tread tops on
      the step curve, ModuleLocalToSite/RotateSide vs RotateSockets).
- [~] PIE on Lvl_Henhouse: tread tops unchanged from the floor-kind geometry (first top 19.7 cm), prop validator clean on seeds 873/4242, door leaf width-fit verified numerically; a full walk-through is left to the user (first riser 19.7 cm), no solid prop inside a door/flap gap or on a jar
      spawn, signs/work lamps hang on closed walls only, henhouse walls/fences/door leaf visible.
- [~] PIE on Lvl_Flat: facilities grounded and truck placed (screenshot checked); heat/counter/jar/pad/panel interactions not re-run this pass (BP logic untouched, only meshes/transforms) items at yaw 90 (300×120 area), counter/jar interact, pad heals in 5 s inside
      the 400×400 zone, panel opens the shop, truck body/steps walkable, nothing overlaps the wake spot.
- [x] Farmer A body/arms on the PIE pawn; `cl9.additem pot` / `standing_lamp` spawn; lamp ItemLight visible while free (blackout not re-tested) on a second client / arms in first person; `cl9.additem pot`, `cl9.additem standing_lamp`
      spawn, the lamp lights its surroundings after being set down and goes dark in a blackout.
- [x] Docs updated (G).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/World/CL9SiteMap.cpp/.h` | Tile/Stretch placement → add Grid, downward kinds, stub/door handling, prop placement + validator |
| `Source/CL9/World/CL9SiteLayout.cpp/.h` | BuildSegments kinds (yard/tread/stubs), `ModuleLocalToSite`, `RotateSide`, `GetEdge` |
| `Source/CL9/World/CL9SiteLayoutTests.cpp` | Segments/points assertions |
| `Source/CL9/World/CL9FlapIn.cpp` | leaf width-fit |
| `Source/CL9/Data/CL9SegKindDef.h` | `Grid` placement enum |
| `Source/CL9/Data/CL9ModuleDef.h/.cpp` | `PropSets`, `FCL9PropSet`, `FCL9PropPlacement` |
| `Source/CL9/Data/CL9ItemDef.h`, `Items/CL9Item.cpp` | `bLitWhenFree` |
| `Source/CL9/Items/CL9Kitchen.cpp` | HeatZone absolute rotation |
| `Source/CL9/Data/CL9DefRegistry.cpp` | Needed seg-kind list |
| `Content/Data/DA_PieceTable, DA_SegKindTable, DA_ModuleTable, DA_ItemTable, DA_CharacterTable` | data rows |
| `Content/Items/BP_CL9Kitchen/Counter/Jar/TruckPad/TruckPanel` | mesh swaps |
| `Content/Maps/Lvl_Flat.umap, Lvl_Henhouse.umap` | instance fixes, truck actors |
| `docs/asset/catalog.md`, `docs/systems/asset-swap.md`, `docs/plan/data/module.md`, `docs/decisions.md`, `docs/asset/production-progress.md` | records |

## Must-Preserve
- Seed determinism across machines: all picks via coordinate hash (`PickVariant`), never RNG order.
- `CL9SiteMap` measures dimensions from mesh bounds — no hand-typed sizes in any table.
- Existing kinds keep their behaviour: `box` Stretch = Size/100; `barn` Tile; `flapIn` Body still blocks the full gap.
- LevelPoint spawns (monster/loot/jar) and door/flap lanes stay clear of solid props.
- Facility XY positions in Lvl_Flat unchanged (only Z/scale/rotation fixed).
- Kitchen HeatZone extent still derived from the world AABB at BeginPlay; heal-zone judgement 400×400×240.
- Skeleton/ABP contracts of farmer_a unchanged; monster rows untouched.

## Execution Notes
- Recommended model: Claude Opus 5 (this session) for B and C (generator/prop maths, validator, tests); a cheaper
  model is acceptable for A/D/E/F data entry and the docs pass.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is
  below the recommendation, surface it to the user and confirm before proceeding.
- Editor work goes through the unreal-mcp toolsets (memory `unreal-mcp-usage`): set_properties → explicit
  save_assets; BP CDO → compile → save; level instances edited directly; StopPIE before saving.
- Truck placement is provisional — show the user a PIE screenshot before committing the umaps.
