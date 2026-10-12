# goal: loop-7j-round-start

**Status**: done (2026-10-12 — listen 2-player PIE verified, uncommitted). Build order ⑦ step **7j** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — round flow steps 1 (truck arrives), 2 (today's order is shown), 4 (tension on the way to the building); open questions 8 and 11 answered 2026-10-12.

## One-line Goal
When a stage's first round starts the truck backs into its spot while a local camera watches; every round then opens with a centred "today's orders" card; and the walk across the empty yard is tense — flickering lamps over the building doors, denser fog until the power is on, distant cries from inside, and a dark silhouette crossing a doorway — with the round clock already running.

## Background / Motivation
- User answers 2026-10-12: open question 11 — **a centred card at the end of the arrival, folding into the HUD order line after a few seconds**; open question 8 — **all three**: sound/lighting ("cries from inside the building, flickering yard lamps, low fog"), **visible-only presence** ("a monster silhouette passing a building window, no attack"), **time pressure** ("the round clock runs from the truck's arrival").
- Recon (2026-10-12):
  - The power intro is a local camera driven by a replicated server time (`PowerOnServerTime`); input lock and invulnerability go through `ACL9GameState::IsIntroLocked/IsIntroInvulnerable`, which every gate already calls.
  - The truck is replicated but its movement is not; meshes are movable.
  - **Buildings have no windows** — doors are bare gaps between modules. The silhouette therefore crosses a **doorway** between the yard and the building (the nearest equivalent; noted for the user).
  - Yard modules carry only a campfire; the only hanging lamp mesh is `SM_Henhouse_WorkLamp_A` (corridor).
  - Fog density is rewritten every tick by `ACL9NightLighting` from the site row.
  - The round clock (`StartNightClock`) already starts when the round starts.

## Design Decisions
- **Arrival only when a stage level comes up** (session start, arrival after travel — 7i). Rounds started by the lobby lever happen with the truck already parked, so they skip it. `ACL9GameState::ArrivalServerTime` (replicated, = the round's start time) drives everything; `CL9Rules.ArrivalTime` (draft 5 s, 0 = off).
- **Truck motion is visual**: every machine offsets the truck's body and steps meshes along the truck's own axis (`ArrivalDistance`, draft 2500 cm, eased) — the actor, cargo box and lever stay at the final spot, so gameplay checks never see a moving truck.
- **Camera**: a local camera at `ArrivalCameraOffset` (pad space) looks at the truck body; characters are hidden locally until it ends; then the view blends back to the pawn. Lock and invulnerability: the arrival window is folded into `IsIntroLocked/IsIntroInvulnerable`.
- **Time pressure** is the round clock running from the arrival's start (no extra system).
- **Order card**: HUD-built at runtime in the existing `CookLayer` canvas (no WBP change): title + the current order line text, `OrderCardDuration` (6 s) with fades, shown once per round start after the arrival (immediately on lever rounds).
- **Fog**: `CL9Rules.PrePowerFogScale` (draft 2.0) multiplies the site row's fog density in the dark stage before the power is switched on.
- **Yard tension actor** (`ACL9YardTension`, local per machine, cosmetic): finds the doors between yard and building from the generated layout each round and
  - hangs a flickering work lamp over each door outside (`YardLamp*` cells);
  - while the local player is in the yard before power-on, plays a cry (sound row `YardCry`) from inside the building every `YardCryInterval*` seconds;
  - when the local player in the yard looks at a door within `SilhouetteRange`, once per door per round, walks a dark silhouette (`Silhouette*` cells — mesh, walk anim, black material, scale, speed) across the doorway, lit from behind.

## Out-of-scope
- Windows in the building kit; the truck driving with players inside; HUD clock redesign; the order card's own UMG asset (built at runtime for now).

## Acceptance Criteria
- [x] `CL9Editor` builds.
- [x] First round on `Lvl_Henhouse` (listen 2 players): both machines show the truck backing in (camera), players hidden, input locked; after `ArrivalTime` the view returns, the truck sits at its spot, then the order card shows and fades.
- [x] A lever-started round: no arrival, the order card shows right away.
- [x] Yard: lamps over each building door flicker; fog denser before power-on and back after; cries heard (log) while in the yard before power-on; looking at a door from the yard → a silhouette crosses once (log).
- [x] 7i follow-up: the dawn screen lines refresh on the lobby phase change (`오늘은 정산일` line on a settlement-day lobby).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | arrival / fog / yard cells |
| `Source/CL9/GameMode/CL9GameState.{h,cpp}` | `ArrivalServerTime`, lock/invulnerability window |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | arrival on the first round of a level |
| `Source/CL9/World/CL9Truck.{h,cpp}` | visual back-in |
| `Source/CL9/GameMode/CL9PlayerController.{h,cpp}` | arrival camera, spawns the yard tension actor |
| `Source/CL9/World/CL9YardTension.{h,cpp}` (new) | lamps, cries, silhouette |
| `Source/CL9/World/CL9NightLighting.cpp` | pre-power fog |
| `Source/CL9/UI/CL9HUDWidget.{h,cpp}` | order card |
| `Source/CL9/Data/CL9SoundTable.h`, `Content/Data/SoundTable.uasset` | `YardCry` row |
| `Content/Data/CL9Rules.uasset`, `Content/Strings/CL9_Strings.csv` | cell values, strings |

## Must-Preserve
- Power intro behaviour, K1 immediate round start, round-end judgements (the truck actor never moves).
- Other sessions' uncommitted changes in the touched files.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (local cinematic + networked lock). A cheaper model is fine for strings/data.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.

## Results (2026-10-12)
- Builds: 4 — (1) shadowed names (`Slot`, `bHidden`); (2) first test: no arrival and no card — the first round of a level starts at server time **0.0**, so `NightStartServerTime` / `ArrivalServerTime` were 0 and read as "none" → arrival now keyed by a replicated flag + sequence, the card by the transition into the round; (3) the client's camera was dropped 0.1 s after start because the server resets the view target to the pawn on possession → the camera re-takes it during the arrival (other takers still win), and characters joining mid-arrival are hidden too.
- Assets: `CL9Rules` (yard lamp mesh, silhouette mesh/anim/black material) and `SoundTable` row `YardCry` → `S_MonsterNear`, saved.
- Listen 2 players (PIE, `Lvl_Henhouse`):
  - Arrival: `Arrival: the truck backs in — 5.0s` → `Arrival camera: start` on host and client → `end (arrived)` on both at +5.0 s → `HUD: order card shown` on both. Mid-arrival screenshot: truck body backing towards the pad, seen from the arrival camera.
  - Lever round (`cl9.depart` from the lobby): no arrival, card shown at once on both.
  - Yard: `9 door(s) … 9 lamp(s)` (seed 1432726273) / `3 door(s)` (another seed), lamp intensities flickering (0.6–8.6 cd sampled), cries every 8–16 s from inside near door 5, silhouettes on doors 0/1/2/5 at 7–20 m, once per door.
  - Fog density 0.10 before power-on → 0.05 after `cl9.power`.
  - 7i follow-up: day-3 lobby dawn screen shows `오늘은 정산일 — 출발 레버를 당기면 회사로 간다` (the widget re-draws on the phase change). Its title still reads the previous dawn if the screen stayed open — pre-existing.
- Deviation from the user's wording: no windows exist in the building kit, so the silhouette crosses a **doorway**; the "yard lamp" is a work lamp hung over each door (yard modules have no lamp posts).
