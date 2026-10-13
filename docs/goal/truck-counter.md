# goal: truck-counter

**Status**: done (2026-10-13, PIE verified on Lvl_Farm and Lvl_Flat).

## One-line Goal
Every stage level gets a delivery window on the truck — the existing delivery counter (`BP_CL9Counter`) stands outside the truck's side — so orders can be delivered and coins earned outside the test level.

## Background / Motivation
- 2026-10-13: the counter was found only in `Lvl_Flat` (`.umap` strings and Source — nothing spawns `ACL9Counter`). Stage levels (`Lvl_Henhouse`, `Lvl_Farm`, `Lvl_PigFarm`) let players cook but never deliver, so no coins are earned without cheats.
- User answers (2026-10-13): **deliver at a truck window** ("트럭 창구에서 납품하게 만들자"); the window sits **outside the truck's side** (the food-truck service window), not in the cargo box.

## Design Decisions
- `ACL9GameMode::SpawnTruckCounter` runs in `StartPlay` right after the truck and the depart lever, on stage levels only (the company has no orders to deliver). It spawns `CL9Rules.TruckCounterClass` at `CL9Rules.TruckCounterOffset × truck transform`.
- New cells (Truck category):
  - `TruckCounterClass` (soft class; cell = `/Game/Items/BP_CL9Counter`, empty = no window);
  - `TruckCounterOffset` (truck space; header default (350, −267, 0) = beside the cargo middle, outside the B body half-width 212 plus the counter half-width 50 and a 5 cm gap).
- If a counter is already placed in the level (`Lvl_Flat`), none is spawned — one counter so stock is never split.
- The counter itself is unchanged: E with an item delivers (stock → orders settle → coins), E empty-handed sells leftover stock. The counter replicates like any fixture.
- Stand-in look: the 100 cm counter box beside the truck. When the truck body gets a real service window, only the offset cell moves to that anchor.

## Out-of-scope
- A window cut into the truck mesh, order balance, delivering at the company.

## Acceptance Criteria
- [x] `CL9Editor` builds; the cell points at `BP_CL9Counter`.
- [x] Stage level PIE: log `Truck counter: … spawned … beside 'CL9Truck_0'`; the counter stands beside the truck, reachable from the yard.
- [x] A cooked egg delivered there settles a one-egg order (fried egg) and the coins rise; `Lvl_Flat` keeps its single placed counter (log `already placed`).
- [x] `/cl9film` round template delivers at the window (video).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.h` | `TruckCounterClass`, `TruckCounterOffset` |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | `SpawnTruckCounter` |
| `Content/Data/CL9Rules.uasset` | class cell |
| `Tools/Film/shots/round_farm.json` | delivery steps |

## Must-Preserve
- Counter semantics (`docs/systems/order.md`, `economy.md`), `Lvl_Flat` layout, the depart lever and cargo box.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above; a cheaper model is fine for docs.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.

## Results (2026-10-13)
- Compile check OK, `CL9Editor` build OK. Cell `CL9Rules.TruckCounterClass` = `/Game/Items/BP_CL9Counter.BP_CL9Counter_C` (saved); `TruckCounterOffset` keeps the header default (350, −267, 0).
- `Lvl_Farm` PIE: `Truck counter: 'BP_CL9Counter_C_0' spawned at (−1285, 267) beside 'CL9Truck_0'`. The `/cl9film` round bot picked an egg, cooked it, walked to the window and pressed E → `Order delivered: fried_egg — 377 ether (day 1 of 1). Money 377.` HUD coins 0 → 377 and the fried-egg order line left the list; then the lever took the round home. Video `Saved/Video/truck_counter_round_farm_20261013.mp4`.
- `Lvl_Flat` PIE: `Truck counter: 'BP_CL9Counter_C_0' is already placed in this level — none on the truck.`
- Note: the window stands just outside the yard navmesh — irrelevant to players (character movement does not use the navmesh); the film bot walks to the nearest nav point and then straight.
