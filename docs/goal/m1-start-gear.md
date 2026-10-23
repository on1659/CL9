# goal: m1-start-gear

## One-line Goal
At the start of a run the truck's cargo bay already holds a fixed number of flashlights (test-only grant) and one frying pan, driven by a new `CL9Rules` start-gear list column, and the flashlight gets an `EquipmentTable` row because it is, by design, bought equipment.

## Background / Motivation
- Roadmap M1 (`C:\Work\CL9\docs\plan\roadmap.md` line 181, tool table line 134) and scenario section 0 (`C:\Work\CL9\docs\plan\scenario.md` lines 21, 29): "짐칸을 열어 보면 손전등 셋과 프라이팬 하나가 있다."
- Today only one flashlight is hand-placed in the level (`BP_CL9FlashItem` in `Lvl_Henhouse` etc. — the "prototype convenience grant" of `C:\Work\CL9\docs\plan\data\item\flashlight.md`), the pan comes from the `cl9.additem` cheat, and the flashlight has no equipment row, so it cannot be re-bought.
- `C:\Work\CL9\docs\decisions.md` (2026-09-01 entry, lines 207–216) left "the slot for the grant value" undecided ("rules or progression"). The roadmap settles it: a `CL9Rules` column.

## Recon facts this spec relies on
- Equipment ownership is not a counter: `ACL9GameState::GetOwnedCount`, `ACL9GameMode::ListOwnedEquipment`, `LoseEquipmentLeftBehind` and `StartNewRun` all count/destroy **world `ACL9Item` actors whose alias matches an equipment row's `ItemDef`**. Once `flashlight` has an equipment row, every flashlight in the world (granted, bought, restored) is automatically owned equipment: saved at dawn, restored by `LoadRun`, lost if left beyond `EquipmentKeepRadius`, wiped by `StartNewRun`. No new ownership bookkeeping is needed.
- `ACL9GameMode::StartPlay` (non-company level): `SpawnTruck` → `SpawnDepartLever` → `SpawnTruckCounter` → `OfferOrders` (if not resumed) → `StartNight` with arrival. Day 1 has **no `Lobby` phase** — the first round opens on level load; later rounds go through `EnterLobby`.
- Clients join a listen host **after** `StartPlay` (EOS lobby → `ClientTravel`), so at `StartPlay` `GetNumPlayers()` is 1 on a fresh session. This is why the grant does **not** count players (Decision 74 below) — a fixed count avoids any join-timing logic.
- Truck cargo box: `CL9Rules.TruckInteriorCenter` / `TruckInteriorExtent` in truck space (`ACL9Truck`), already used for the left-behind test.
- `SpawnEquipmentAtTruck` drops bought items 200 cm around the first truck pad (not inside the cargo bay).
- Existing equipment string key pattern: `Equipment.frying_pan.Name` in `C:\Work\CL9\Content\Strings\CL9_Strings.csv`.

## Design Decisions
- **Decision 74 (2026-10-20, owner): the flashlight is bought equipment; the start-gear flashlight is a test grant.** Original words: "손전등은 애초에 사는거임. 지금주는건 테스트로 주는거야." Consequences for this spec:
  - The `EquipmentTable.flashlight` row (with `Price`) is the real, intended way to get a flashlight; the final price is set in M20.
  - The `flashlight` entry in `CL9Rules.StartGear` is **test-only — deleting that entry gives the intended flow (buy to use)**. It must be removable with no code change.
  - The flashlight count is a fixed `Count`, not tied to the number of players. (Rejected: per-player counting via `PostLogin` / unique-net-id tracking — elaborate code for a grant that is not part of the real flow.)
  - The `PerPlayer` column is dropped; `FCL9StartGear` has only `EquipAlias` + `Count`. The one-frying-pan fixed grant stays as specified.

## In-scope
1. **`CL9Rules` start-gear list column** — new column `StartGear` (array of a new struct `FCL9StartGear`):
   - `EquipAlias` (`FName`, an `EquipmentTable` alias — validated at load: unknown alias = load error, same style as other alias checks)
   - `Count` (`int32`, ≥ 0) — fixed number, independent of player count
   - Empty list = no start gear (feature off).
   - Default rows: `flashlight` (Count per Balance — **test-only, delete the entry for the intended buy-to-use flow**, Decision 74), `frying_pan` (Count 1).
2. **Cargo-bay placement** — host-only spawn of the granted items inside the truck cargo box (`TruckInteriorCenter`/`TruckInteriorExtent`, truck space), on the floor, spread so they do not overlap. Spread margin is a `CL9Rules` column `StartGearInset` (cm inset from the box walls). If no truck exists, fall back to `SpawnEquipmentAtTruck` (pad ring). Items are spawned through the equipment row's `ItemDef` (same path as buying), so they are equipment immediately.
3. **When the grant happens** — at the start of a fresh run only:
   - `StartPlay` on a non-company level when the run is **not** resumed (`bResume == false`), after `SpawnTruck`.
   - `StartNewRun` (run over → day 1 again): after it destroys the old run's equipment, grant again — "start gear" means the start of every run. (The existing comment "새 판은 빈손으로 연다" refers to bought gear not carrying over; reword it to match.)
   - **Not** on ordinary `EnterLobby` (day 2+), not on `LoadRun` resume, not on the company level. Lost gear stays lost — that is what the re-buy row is for.
   - The grant never looks at the player count (Decision 74), so late joiners need no special path.
4. **Flashlight `EquipmentTable` row** — the intended source of flashlights (Decision 74). Alias `flashlight`, `ItemDef` → item `flashlight`, `Slot` `Carryable`, `bPhysicalProp` true, `bUsableAsTool` false, `UnlockDay` 1, `MaxOwned` and `Price` per the Balance section. `DisplayName` from a new string key `Equipment.flashlight.Name` (ko column only — 손전등).
5. **Remove hand-placed flashlights from site levels** that start a run (`Lvl_Henhouse`, `Lvl_Farm`, `Lvl_PigFarm`, `Lvl_Flat` — the levels found to contain `BP_CL9FlashItem`), so the grant is the only source. Hand-placed headlamps are untouched. Test/review maps (`QA_CartTrial`, `Lvl_HenhouseIndoor_Review`, `Lvl_HybridFiveV01`) are left as they are.
6. **Docs** — `C:\Work\CL9\docs\plan\data\rules.md` (new section/columns), `C:\Work\CL9\docs\plan\data\equipment.md` (flashlight row in "지금 있는 것"), `C:\Work\CL9\docs\plan\data\item\flashlight.md` ("지급·매매·분실" — grant is now the `CL9Rules.StartGear` row), `C:\Work\CL9\Tools\DataPanel\effects\` CSV labels for the new columns, a `C:\Work\CL9\docs\decisions.md` entry, scenario/roadmap M1 status.

## Out-of-scope
- Final prices and the shop price table (roadmap M20). The flashlight row gets a draft price only.
- Headlamp grant, lamp/cart/pot rows, new tools (M17–M19).
- Any truck panel UI change beyond the new row appearing through the existing list.
- Counting players for the grant, `PostLogin` grants, per-player tracking (Decision 74).
- Per-player ownership / binding a flashlight to a specific player — items are just placed; players pick them up (scenario: "각자 손전등을 하나씩 집고").
- Blackout, light values, or flashlight item columns (`Weight`, `ValueMin/Max` stay as in the item table).
- Progression day-overrides for start gear (not requested).

## Acceptance Criteria
- [ ] `CL9Rules` has `StartGear` and `StartGearInset` columns, editable in the data panel with Korean labels; an unknown `EquipAlias` or a negative count fails row validation.
- [ ] Fresh session (host alone, default rows): on level load the cargo bay holds the `StartGear` flashlight `Count` + 1 frying pan, all inside the cargo box, none overlapping, none falling through the truck floor or knocked out by the arriving truck body.
- [ ] Listen 2-player PIE fresh session: the client joining adds nothing — the cargo bay holds the same fixed counts as the solo case.
- [ ] Deleting the `flashlight` entry from `StartGear` (frying pan entry kept) → the next fresh run grants only the pan, and a flashlight can be obtained by buying it (intended flow, Decision 74).
- [ ] `StartPlay` with `bResume == true` (7i carry) spawns no start gear; owned equipment comes only from the save.
- [ ] Day 2 lobby (`EnterLobby`) spawns no start gear; a flashlight left in the site is gone (lost by `LoseEquipmentLeftBehind`).
- [ ] `StartNewRun` destroys old equipment and re-spawns the start gear for day 1.
- [ ] Truck panel / `cl9.buy` lists `flashlight`; buying it deducts the price and spawns a flashlight by the truck; `GetOwnedCount("flashlight")` counts granted + bought ones.
- [ ] Dawn save lists granted flashlights/pan in `OwnedEquipment`; `cl9.load` restores them.
- [ ] Site levels listed in In-scope 5 contain no hand-placed `BP_CL9FlashItem`.
- [ ] Emptying `StartGear` in the data panel → no start gear on the next fresh run (no code change).

## Related Files / Modules
| File | Role |
|------|------|
| `C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h` / `.cpp` | new `FCL9StartGear` struct, `StartGear`, `StartGearInset`, validation |
| `C:\Work\CL9\Source\CL9\GameMode\CL9GameMode.h` / `.cpp` | `StartPlay`, `StartNewRun`, `SpawnEquipmentAtTruck` (fallback), new grant / spawn-in-cargo helper |
| `ACL9Truck` (Source/CL9, truck actor) | truck-space → world transform for the cargo box |
| `C:\Work\CL9\Content\Data\CL9Rules.uasset` | start-gear rows |
| `C:\Work\CL9\Content\Data\EquipmentTable.uasset` | `flashlight` row |
| `C:\Work\CL9\Content\Strings\CL9_Strings.csv` | `Equipment.flashlight.Name` (ko) |
| `C:\Work\CL9\Content\Maps\Lvl_Henhouse.umap`, `Lvl_Farm.umap`, `Lvl_PigFarm.umap`, `Lvl_Flat.umap` | remove hand-placed flashlight |
| `C:\Work\CL9\Tools\DataPanel\effects\` (rules / equipment CSV) | column labels / effects |
| `C:\Work\CL9\docs\plan\data\rules.md`, `equipment.md`, `item\flashlight.md` | table docs |

## Must-Preserve
- Equipment ownership stays "world actors are the truth" — no new counter.
- `LoseEquipmentLeftBehind`, `SaveRun`/`LoadRun` equipment lists and the `StartNewRun` wipe keep their behaviour; granted items just flow through them.
- Buying still spawns by the truck pad (`SpawnEquipmentAtTruck`) — only the grant uses the cargo box.
- Host authority: spawning only on the server; items replicate as today.
- Truck arrival (7j): the actor, cargo box and lever are in place from the start; only body/stair meshes animate — spawn against the actor transform, not the moving mesh.
- No hard-coded counts, aliases or offsets in C++ — all from `CL9Rules` / `EquipmentTable`.
- Strings: ko column only.

## Fairness Constraints
- Host-authoritative: start gear is spawned only on the server (`HasAuthority`), from table rows; clients never request or create it. No new RPC.
- The grant is the same for every player and independent of join order or player count (fixed `Count`, Decision 74) — no one can trigger an extra grant by rejoining, reloading or calling a path other than a fresh run start (`StartPlay` not resumed, `StartNewRun`).
- Granted items are ordinary equipment: they sell at the counter by the normal price rule and are lost when left behind, exactly like bought ones — no free-ether loophole beyond what the table rows grant. Spawn positions are random only within the cargo box and affect nothing but placement.
- No change to monster hearing/sight; spawning the items emits no noise wave.

## Existing Integration Contract
- `ACL9GameMode::StartPlay` keeps its order (`LoadRun` on resume → site map → `SpawnTruck` → `SpawnDepartLever` → `SpawnTruckCounter` → `OfferOrders` → `StartNight`); the grant is inserted after `SpawnTruck` and skipped when resumed or on the company level.
- `StartNewRun` still destroys all equipment first and resets the run; the grant runs after that wipe.
- `EnterLobby`, `LoadRun`, `SaveRun`/`ListOwnedEquipment`, `LoseEquipmentLeftBehind`, `GetOwnedCount`, `CanBuy`/`BuyEquipment` are unchanged — they pick up flashlights only through the new `EquipmentTable` row.
- `SpawnEquipmentAtTruck` keeps its signature and pad-ring behaviour for buying and save restore; the grant only reuses it as the no-truck fallback.
- Truck arrival (7j) and the cargo-box left-behind test keep reading `TruckInteriorCenter`/`TruckInteriorExtent` unchanged; the grant reads the same columns, it does not redefine them.

## Execution Notes
- Recommended model: the strongest current Claude model for the cargo placement vs. truck arrival check (replicated spawn against an animating truck). A cheaper model (Sonnet) is acceptable for the table row, string key, data-panel CSV labels and doc updates.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Editor/PIE/build go through the broker and the session lock (`C:\Work\CL9\docs\workflow\sessions.md`); `.umap` edits need the editor lock.

## Balance (defaults chosen — tune in the table, not questions)
| Where | Default | Note |
|---|---|---|
| `CL9Rules.StartGear` flashlight | Count 4 | **test-only** (Decision 74) — 4 = session cap, so every tester gets one; delete the entry for the intended flow |
| `CL9Rules.StartGear` frying_pan | Count 1 | scenario "프라이팬 하나" |
| `CL9Rules.StartGearInset` | 30 cm | keeps items off the cargo walls |
| `EquipmentTable.flashlight.Price` | 10 (draft) | below the pan's 25 — a light is cheaper than the only weapon; final value in M20 from the ether scale (`C:\Work\CL9\docs\plan\economy.md`) |
| `EquipmentTable.flashlight.MaxOwned` | 0 (unlimited) | same as other rows |
| `EquipmentTable.flashlight.UnlockDay` | 1 | re-buyable from the first lobby |

## Open Questions
- None. (Q1 "how to count one flashlight per player" was closed by Decision 74 — "손전등은 애초에 사는거임. 지금주는건 테스트로 주는거야.")
