# goal: round-start-truck-intro

**Status**: spec settled 2026-10-06 by `/autogoal` (HEAVY, 4-seat panel + cross-examination). Run `round-truck-core` REPORTED (2026-10-06); run `round-intro-camera` (camera K10 + docs) REPORTED (2026-10-06) — see "Results" at the end. Nothing committed; human checks (A10, two players) pending. Follow-up K10-F (continuous intro camera, 2026-10-10) at the end. Execution: `/cl9 task` roles (gameplay, ui, art, docs) + lead.

## One-line Goal
The round starts the moment the stage loads (orders picked by the system), the depart lever becomes an ON/OFF switch inside a new walkable truck, whoever is outside the truck when the round ends is left behind and dies (everyone left behind = game over), and the building opens pitch dark with a lit power switch whose flip plays a per-player camera fly-through to a different landmark before the lights flicker and die.

## Background / Motivation
Owner reports and decisions, 2026-10-05/06 (this spec's session):
- Report: "no items, no monsters" — the night never started because nobody found the depart lever (a borrowed, unlabeled exit-sign mesh next to the pad, no on-screen guidance). Owner: "the game should just start — monsters right away"; "orders are picked by the system at random (the host-pick screen was only left because changing it was a hassle)".
- Lever: "keeps its switch function, but default ON". Pulling it = "go home" = round over → settlement → next round. When the round timer runs out "the lever goes up by itself". Blackout stays.
- Owner's target structure (Lethal Company ship): "we must be able to walk into the truck — a wide truck for 4+ people; turning the lever inside the truck moves the truck and fakes an arrival; settle inside the truck, choose the next map; back to the map; every 3 rounds a shop (later)". People left in the building die; revive only when the truck reaches a special place.
- Split agreed with the owner: **this goal** = immediate round start + lever switch + new walkable truck (Blender) + "outside the truck at round end = dies, all dead = game over" + building power intro. **Later goal** = truck travel sequence, settlement and destination choice inside the truck, revive at a special place, shop every 3 rounds. Until then, after a round: settlement screen → lobby at the truck (cook, shop) → host pulls the lever down → next round on the same site with a new seed.
- Power intro (owner's words): "the first space after entering is completely dark; there's one switch in a lit spot. Turn it on and a sequence starts: every light turns on, the camera quickly shows the current map, flying to where the kitchen is. When the camera gets there, the light suddenly goes strange and dies. That tells you the game has started. After it dies it's still brighter than the first darkness (what entering looks like now)." Lamps are all off afterwards. Destination per player: "show everyone a different one — kitchen / deepest place / return jar or any such point, one at random, and everyone must be different; if there are fewer points than people, so be it" — so each player remembers where theirs was. During the camera sequence players are invulnerable.
- The owner calls the round "판", not "night". Code names (`ECL9NightPhase::Night`, `StartNight`) stay.

Recon facts this spec relies on (file:line in Source/CL9 unless noted):
- Round start is `ACL9GameMode::StartNight` (GameMode.cpp:204-288; defers until site build + nav, `DeferNightStartWhileBuilding` :290-317). `StartPlay` :984-1001 sets the site, rolls the seed, `OfferOrders`, spawns the lever. `EndNight` :326-361 → Dawn → `EnterLobby` :363-395 (AdvanceDay, ExpireOrders, LoseEquipmentLeftBehind, ClearNightPopulation, new seed, ReviveDead, Lobby, OpenDawnScreen, Settle, SaveRun, OfferOrders). Alive players are never moved at round end today, so the next map can be built around them.
- `OfferOrders` :1109-1171 draws without replacement in draw order; `AcceptOrders` :1693-1757 needs exactly `Required` distinct indices and warns on an empty offer (:1708).
- Interior brightness is `ACL9NightLighting`'s `LumenSkylightLeaking` only (NightLighting.cpp:212-242; light.md:106-109). `SelectTarget` :200-209, lerp at 1.25/s, `ApplyDiscrete` covers only the sky material/clouds. Lamps: `ACL9SiteMap` creates shadowless movable point lights per lamp prop (SiteMap.cpp:1021-1050, `ModuleIndex` in scope :919), `Tick` :463-471 → `SetLampsOn(!IsBlackout())` with an unchanged-state early-out (performance.md:115-116 requires that pattern), `TickFlicker` :473-490.
- The site map is local (`bReplicates=false`), built from the replicated seed; single sites have `DistrictIndex == INDEX_NONE` (SiteMap.h:72-73), `ACL9SiteMap::Find` returns the primary map. `FCL9SiteLayout` (SiteLayout.h:184-258): Modules[0] = start module (`yard_gate`, outdoor), cells/edges per floor, `Points` (one per cell center, SiteLayout.cpp:474-475), loot spawns within ±130 cm of a point (GameMode.cpp:636-647), emergency exit = the single `Flap` edge on the outer boundary (SiteLayout.cpp:456-470), stair modules fill their cells with treads.
- Interactables: `ACL9Fixture` + `ICL9Interactable`; `ACL9ActorBase` replicates by default (ActorBase.cpp:8); locally spawned fixtures must set `bReplicates=false` like `ACL9FlapIn` (FlapIn.cpp:21); the host re-resolves focus (Character.cpp:787, 1403); `ResolveFocus` prefers items over interactables (:838).
- View target: server-side `SetViewTarget` paths (death cam Character.cpp:2543, execution :2630, spectate GameMode.cpp:495, revive :585) reach the client as `ClientSetViewTarget`. Destroying an outgoing blend target snaps (PlayerCameraManager.cpp:899-903).
- Invulnerability precedent: knocked down (`OnDamage` Character.cpp:2456-2475; monster gates Monster.cpp:671, 883, 1312, 1933, 2015; CMC speed 0 CharacterMovement.cpp:36-42). Stalkers accumulate "unseen" time and execute without a reaction window (Monster.cpp:882-893, 1909, 1933).
- Truck today: hand-placed `SM_FoodTruck_A` + `SM_FoodTruck_Steps_A` StaticMeshActors in Lvl_Flat/Henhouse/Farm/PigFarm (Flat: body (-1250,0,0) yaw 180, rear at X -935; pad (-600,0,0)); interior aisle 119 cm (too tight for 4); C++ never references it. All anchors use the first `ACL9TruckPad` (SiteMap.cpp:156-176; GameMode.cpp:558, 924, 1012, 1443, 1490). Capsule r39 hh90, eye 156, MaxStepHeight 45, door width 250.
- Pipeline: Blender 5.2 headless via `py -3 Tools/Blender/run_headless.py`, `StaticKit` (`Tools/Blender/static_meshkit.py`), importer template pattern `Tools/Unreal/import_henhouse_roof.py` (REPO-relative paths, SHA guard). Old truck scripts hard-code `D:\Work\...` and must not be reused. Blender MCP is not used.

## Flow
```
Lvl_Start Title -> [방 만들기] -> stage level
StartPlay: site + seed, OfferOrders (auto-accepts the first Required offers), spawn truck + lever, StartNight (waits for build + nav)
Round (Phase Night)  lever pose DOWN (= ON)
  building: Dark stage — indoor SkyLeak 0, indoor lamps off, the power switch glows in the first indoor room; clock suffix "전원 스위치를 켜라"
  anyone flips the switch -> PowerOnServerTime = T, IntroTargets assigned (one per participant, all different while points last)
     T .. T+LeadIn            every indoor lamp on, ambient back to the round level; each participant's camera hard-cuts to the switch
     .. +Fly                  camera flies the cell path to that player's own landmark (fixed duration for everyone)
     .. +Flicker              lamps strobe (<= 3 big flashes/s) then die
     .. +Return               fade out -> back to the pawn -> fade in      [movement/look/actions locked T .. here]
     .. +Grace                still invulnerable
  stage Dead: indoor lamps off, ambient = the round row (brighter than Dark)
  blackout (existing clock) overrides every stage
host pulls the lever UP (two presses; first press shows how many are outside the truck)  or  timer ends (lever goes up by itself)
  EndNight -> Dawn -> left-behind pass: every alive player outside the truck interior loses their carry and dies
      nobody alive + bWipeEndsRun -> run over (GameOver), as a wipe does
      else -> EnterLobby (revives the dead as today) -> Dawn screen also names who was left behind
Lobby  lever pose UP (= OFF); clock suffix "레버를 내려 출발" (host) / "호스트가 레버로 출발" (client); cook, shop
  host pulls the lever DOWN (two presses) -> StartNight -> next round (new seed)
```

## In-scope (must-land)
- **gameplay**: auto-accept in `OfferOrders`, auto `StartNight` in `StartPlay`; lever ON/OFF switch (host-only both ways, two-press, arm cleared on phase change, pose derived every tick, Night branch calls `EndNight`); `ACL9Truck` runtime actor with an interior box + `IsInside`; lever frame moves into the truck; left-behind pass + all-left-behind run over + `LeftBehindNames`; power state on GameState (`PowerOnServerTime`, `IntroTargets`, stages, lock/invulnerability windows, resets); `ACL9PowerSwitch` local fixture + `RequestPowerOn`; site-map lamp groups (indoor/outdoor), Dark/LightsOn/Flicker/Dead lamp states, switch light; NightLighting stage input + SkyLeak snap; switch cell removed from spawn points; layout helpers (switch cell, cell path, candidates, world→cell); intro lock/invulnerability wiring (IsIncapacitated, CMC, OnDamage, monster gates, look, jump, focus); `cl9.power`; layout automation tests; rules columns.
- **gameplay (camera)**: per-player intro camera driver (cell-path BFS, timing, look-ahead, rate limit, sweep lift, fades, abort rules, third-person own body while active).
- **ui**: clock suffixes (Lobby lever hint, Dark switch hint), lever and switch prompts, Dawn screen left-behind line, string rows (ko only).
- **art**: new walkable truck `SM_FoodTruck_B` (+ steps/ramp), lever `SM_DepartLever_Base_A` + `SM_DepartLever_Handle_A`, power switch `SM_PowerSwitch_A`; Blender build scripts (REPO-relative), manifests, importers, previews; capsule walk QA with 4 capsules inside.
- **lead**: imports in the running editor, Rules cells (meshes, truck offset/interior), delete the old `FoodTruck_Body`/`FoodTruck_Steps` actors from the four stage umaps, builds, PIE gates, decisions.md, work-progress.
- **docs**: decisions.md `## 2026-10-06`, plan/loop.md, systems/order.md, systems/session.md, systems/light.md, plan/data/rules.md, plan/data/sequence.md (`power_intro` row, not skippable), asset card `docs/asset/map/food_truck.md` (B), testing/pie-scenarios.md (order/load/cheat scenarios rewritten for auto-start), systems/cheats.md.

## Out-of-scope
- Truck travel sequence, settlement and destination choice inside the truck, level travel between sites, revive at a special place, shop every 3 rounds (next goal).
- Moving kitchen/counter/shop panel into the truck (they exist only in Lvl_Flat today); new PlayerStart placement; changing `WakeSpot` (revive stays at the pad's +X face); changing the equipment-loss rule (`EquipmentKeepRadius`).
- Intro sounds (no assets) — recorded as a risk; destination-name caption; extra flashlights for 4 players; Steam; 2-player verification beyond what the lead can run.
- Rewriting the pre-existing input-mapping arbitration (closing a screen while being executed re-enables input) and the mouse-capture report (owner: later).

## Design Decisions (panel-settled — do not relitigate during implementation)
- **K1 Auto start** — `OfferOrders` ends with `AcceptOrders({0..Required-1})` when the offer is non-empty (no call, no warning, when empty); it is the single spot so StartPlay, EnterLobby, LoadRun and `cl9.day` all stay consistent. `StartPlay` calls `StartNight()` last; the existing defer waits for build + nav. OrderSelect, the HUD derive and the lever's OrdersPending branch stay untouched (they simply never trigger). No config flag. Known quirks accepted: `cl9.day` stacks one more accepted batch; `cl9.load` while the first start is still deferred cancels it (the host then uses the lever).
  - Rejected: `bAutoStartFirstRound` ini flag (configurability nobody asked for); accepting in StartPlay/EnterLobby only (LoadRun and `cl9.day` would leave an offer pending).
- **K2 Lever = switch** — Host-only in both directions (unchanged rule), two-press arm (`DepartArmTime`) in both directions. The server clears `ArmedUntilServerTime` from `OnNightPhaseChanged` so an arm never survives a phase change. Lobby → `RequestDepart` (unchanged); Night → `EndNight()` directly. Pose: a handle component pitched DOWN in Night, UP otherwise, derived every tick on every machine (no new replicated state). Night first press prompt names how many players are outside the truck.
  - Rejected: any-player go-home (owner never asked; host rule kept until the truck goal); `ArmedPhase` replicated field (the phase delegate is enough); BasicShapes composite and hard-coded asset paths (conventions.md:195).
- **K3 Truck** — New native `ACL9Truck` (non-abstract, replicated static actor) spawned by the game mode in `StartPlay` at `CL9Rules.TruckOffset` relative to the first `ACL9TruckPad`'s actor transform (pad origin is on its floor). Truck local frame: origin = ground point under the centre of the rear opening, local +X = into the cargo box, so yaw 180 makes the opening face the pad; the steps mesh and the body share that origin (both components at zero relative offset), mesh from `CL9Rules.TruckMesh` (+ `TruckStepsMesh`), interior = `CL9Rules.TruckInteriorCenter/Extent` in truck space (a query-only box, also drawn by `cl9.truck` debug). `ACL9Truck::IsInside(Location)`. Null mesh = invisible but `IsInside` still works (testable before art lands). The lever is spawned inside the truck at `DepartLeverOffset` reinterpreted relative to the truck actor transform; lever base and handle meshes share one origin = the hinge, so the handle component rotates at zero relative offset. The old hand-placed truck actors are deleted from the four stage umaps by the lead; the old assets stay on disk. The pad, its heal zone, `WakeSpot`, PlayerStarts and every pad-anchored lookup stay as they are.
  - Rejected: hand-placing the new truck in four umaps (drift between levels, later levels forget it); replacing the pad (six C++ lookups depend on it).
- **K4 Truck size** — Cargo interior clear ≥ 640 (L) × 340 (W) × 250 (H) cm, floor top 64 cm, rear opening ≥ 220 × 220, steps or ramp ≥ 220 wide with risers ≤ 20 cm (MaxStepHeight 45). Exterior about 900 × 400 × 380 including the cab. Rear opening faces +X (toward the pad) like today; rear sits where the old rear was (pad −X ≈ 335 cm). Lever on the front (cab-side) wall of the cargo box so players must walk in. Walkable collision with UCX hulls, materials `M_CL9_FoodTruck_*` reused where possible, 2,000–8,000 tris.
- **K5 Left behind** — In `EndNight`, **after** `SetPhase(Dawn)`: every alive player whose pawn is not `IsInside` the truck has their carry destroyed (held, quickslots, worn — worn needs a new destroy path because `HostDropEverything` never drops it), then `HostApplyDamage(Hp, nullptr, "left behind")`. Names go to replicated `ACL9GameState::LeftBehindNames` (OnRep + direct host call, like `OnSettled`), cleared next to `ClearLastSettlement`. If no player is alive afterwards and `Rules->bWipeEndsRun`, the run ends exactly as a wipe does (RunOver, GameOver screen) without `EnterLobby`; otherwise `EnterLobby` revives them as today. No truck in the level (no pad) → nobody is left behind.
  - Rejected: pad radius 800/1200 cm or "on a layout cell" (owner: the boundary is the inside of the new truck); killing before Dawn (re-enters `EndNight` through `HandlePlayerDeath`).
- **K6 Power state** — `ACL9GameState`: replicated `float PowerOnServerTime` (0 = not flipped) and `TArray<FCL9IntroTarget> IntroTargets` {int32 PlayerId; int32 Module}, set together in one frame + `ForceNetUpdate`, both reset in `StartNightClock` and `ClearNightClock`. `ECL9PowerStage {None, Dark, LightsOn, Flicker, Dead}` derived on every machine from (phase, T, server time, "this world has a switch"); None outside Night or when the primary layout has no switch module (Lvl_Flat). Windows from Rules: `IntroLeadIn 0.5`, `IntroFlyTime 4.0`, `IntroFlickerTime 1.5`, `IntroReturnTime 0.4`, `IntroGrace 1.0`, `IntroMaxSpeed 1500`. `IsIntroLocked(PS)` = participant && now in [T, T+Lead+Fly+Flicker+Return); `IsIntroInvulnerable(PS)` = participant && now in [T, …+Grace). Participants = alive, not executed, not spectating at T. No Multicast.
- **K7 Lighting** — Site map keeps a parallel per-lamp indoor flag (module `!bOutdoor`) and computes an indoor and an outdoor group state once per tick, touching lamps only when a group state changes (performance.md:115-116). Indoor: Dark off / LightsOn on / Flicker strobe from `floor((now−T)·k)` with ≤ 3 big flashes per second / Dead off / blackout off; Lobby and Dawn on (unchanged). Outdoor lamps (campfire) keep the old rule. NightLighting's target takes the stage: Dark → the round row with SkyLeak 0; SkyLeak snaps (no lerp) on any edge into or out of Dark, including the late-join bind. The switch carries its own small shadowless cool light, on only in Dark and never in blackout. Every district follows the one switch in the primary map. **Amended 2026-10-15 (look-rework D2)**: a seeded share of the lit indoor lamps (`LampSurviveShare`, henhouse 0.3, lamp group `IndoorSurvivor`) survives `Dead` and stays on until blackout; the flicker-and-die show is unchanged for the rest.
- **K8 Switch** — `ACL9PowerSwitch : ACL9Fixture, ICL9Interactable`, `bReplicates=false`, spawned by the primary site map (`DistrictIndex <= 0`) into `SpawnedActors` (rebuild cleanup), owner = site map, mesh `CL9Rules.PowerSwitchMesh`, the lever's focus-box recipe. Module = floor-0, indoor, non-stair module with the minimum `Hops` (ties: lowest index). Cell = that module's cell with a non-Wall edge to a cell of a lower-Hops module (else its first cell); the switch stands at that cell's centre as a non-blocking pedestal; that cell's point is removed from every spawn purpose (no loot/monster on the switch). Any player may flip it; `CanInteract` = stage Dark && !blackout; `Interact` checks for the auth game mode (not `HasAuthority`, which is true on a client's local copy) and calls `ACL9GameMode::RequestPowerOn`. Clock suffix in Dark via the `Hud.Night` `{3}` slot (not `HintText`, which belongs to item hints).
  - Rejected: "Hops == 1" (can be empty when `yard_open` sits at hop 1); wall-mount with line traces (more code, props along walls); site columns for darkness.
- **K9 Targets** — From the primary layout only: every floor-0 `Special` module (kitchen, delivery — no alias strings) + the emergency-exit module (found by scanning edges for the `Flap` edge) if it is on floor 0, else the highest-Hops floor-0 non-stair module; dedup; shuffle with a server RNG; assign distinct targets to participants, wrapping only when there are fewer targets than participants. Look-at point = the module's Jar point if it has one, else its centre cell.
- **K10 Camera** — Every machine computes its own local player's path from the shared layout: BFS over floor-0 cells (Open and Door pass; Wall and Flap block; stair modules excluded except as an endpoint) from the switch cell to the first reached cell of the target module; points = cell centres at 170 cm; each segment sphere-swept once against world static and lifted in Z if blocked. Motion is a function of (server now − T): constant speed L/Fly when L ≤ MaxSpeed·Fly, else fly the first part at MaxSpeed, dip to black 0.15 s, fly the last part at MaxSpeed into the target. Look at a point 400 cm ahead on the path, yaw/pitch rate-limited to 180°/s, no roll, no FOV change. Engine `ACameraActor` spawned locally; hard cut in; return = 0.2 s camera fade out → `SetViewTarget(pawn)` → 0.2 s fade in; return only if the current view target is still our camera. Abort (and return) when Phase ≠ Night, the player dies or is executed, or a server view-target call took over. The owner's own body renders in third person while the intro camera is active (`ApplyThirdPersonMeshes`).
  - Rejected: replicating a host nav path (Lvl_Henhouse nav bounds do not cover the map — map.md:197 — and clients have no nav); LevelSequence (the map is random per round); a spline class.
- **K11 Lock** — `IsIntroLocked` is OR'd into `IsIncapacitated()` (every server action RPC), CMC speed 0 (CharacterMovement.cpp:39), look input (Character.cpp:409), jump, and the focus gate (:899). `IsIntroInvulnerable` gates `OnDamage` and the monster gates at Monster.cpp:671, 1312, 1933, 2015 — the longer window gives a reaction second after control returns (stalker execution). `HostKill` of an execution already in progress is unaffected.
- **K12 Cheats/tests** — `cl9.power` (host flips the switch with the host character, logs targets and stage); `cl9.truck` (logs the truck transform, interior box and who is inside). `cl9.night` keeps ending the round (it now also exercises K5). Layout automation tests on synthetic module sets (`FTestModules`, SpecialCopies 1/2/4, with and without a hop-1 outdoor module and a stair): switch module exists, its cell has no spawn point, BFS reaches every candidate, candidates are floor 0.

## Parameters
| Name | Default | Note |
|---|---|---|
| `UCL9RulesDef::IntroLeadIn/IntroFlyTime/IntroFlickerTime/IntroReturnTime/IntroGrace` | 0.5 / 4.0 / 1.5 / 0.4 / 1.0 s | header defaults; also written to the CL9Rules asset |
| `UCL9RulesDef::IntroMaxSpeed` | 1500 cm/s | camera speed cap |
| `UCL9RulesDef::TruckMesh`, `TruckStepsMesh`, `PowerSwitchMesh`, `DepartLeverHandleMesh` | null in header; cells set by the lead after import | null = invisible but functional |
| `UCL9RulesDef::TruckOffset` | from the art manifest (rear at pad −X ≈ 335 cm, yaw 180) | pad +X face frame |
| `UCL9RulesDef::TruckInteriorCenter/Extent` | from the art manifest (cargo box) | truck space |
| `UCL9RulesDef::DepartLeverOffset` | from the art manifest (front cargo wall) | now truck space |

## Interfaces (implement verbatim — roles code against these in parallel)
- `CL9Types.h`: `UENUM enum class ECL9PowerStage : uint8 { None, Dark, LightsOn, Flicker, Dead };` `USTRUCT struct FCL9IntroTarget { int32 PlayerId = INDEX_NONE; int32 Module = INDEX_NONE; };`
- `ACL9GameState`: `UPROPERTY(Replicated) float PowerOnServerTime; UPROPERTY(Replicated) TArray<FCL9IntroTarget> IntroTargets; UPROPERTY(ReplicatedUsing=OnRep_LeftBehind) TArray<FString> LeftBehindNames;` `ECL9PowerStage GetPowerStage() const; float GetIntroElapsed() const; bool IsIntroLocked(const APlayerState*) const; bool IsIntroInvulnerable(const APlayerState*) const; const FCL9IntroTarget* FindIntroTarget(int32 PlayerId) const;` `const TArray<FString>& GetLeftBehindNames() const;` host setters `void BeginPowerIntro(float ServerTime, TArray<FCL9IntroTarget>&& Targets);`, `void SetLeftBehind(TArray<FString>&& Names);`; `DECLARE_MULTICAST_DELEGATE(FCL9OnLeftBehindChanged);` member `FCL9OnLeftBehindChanged OnLeftBehindChanged;` (fired from OnRep and directly on the host).
- `ACL9GameMode`: `bool RequestPowerOn(ACL9Character* Who);` `void SpawnTruck();` `ACL9Truck* GetTruck() const;` left-behind pass inside `EndNight`.
- `ACL9Truck` (`Source/CL9/World/CL9Truck.{h,cpp}`): `bool IsInside(const FVector& Location) const; FTransform GetInteriorFrame() const; static ACL9Truck* Find(const UWorld*);`
- `ACL9PowerSwitch` (`Source/CL9/Items/CL9PowerSwitch.{h,cpp}`): Fixture + Interactable as K8, plus `void SetGlow(bool bOn);` (owns a small shadowless cool `UPointLightComponent`; the site map calls it from its once-per-tick group pass).
- `CL9SiteLayout` (pure, engine-free, tested): `bool FindSwitchCell(const FCL9SiteLayout&, int32& OutModule, FIntPoint& OutCell);` `bool FindCellPath(const FCL9SiteLayout&, FIntPoint From, int32 TargetModule, TArray<FIntPoint>& OutCells);` `void IntroCandidates(const FCL9SiteLayout&, int32 SwitchModule, TArray<int32>& OutModules);` `int32 FindExitModule(const FCL9SiteLayout&);`
- `ACL9SiteMap`: `bool WorldToCell(const FVector& World, int32& OutFloor, FIntPoint& OutCell) const;` `bool HasPowerSwitch() const;` `FVector CellToWorld(FIntPoint Cell, float Z) const;`
- Strings (ko only; ui adds rows): `Prompt.PowerSwitch` "[{0}] 전원 켜기", `Facility.PowerSwitch` "전원 스위치", `Hud.Dark` " · 전원 스위치를 켜라", `Hud.Lobby` gains `{1}` ← `Hud.LobbyHost` " · 레버를 내려 출발" / `Hud.LobbyClient` " · 호스트가 레버로 출발", `Prompt.GoHome` "[{0}] 귀가 준비", `Prompt.GoHomeConfirm` "[{0}] 한 번 더 눌러 귀가 · 트럭 밖 {1}명은 남는다", `Prompt.DepartHostOnly` reworded "호스트만 레버를 움직일 수 있습니다", `Dawn.LeftBehind` "트럭에 못 탄 사람: {0}".

## Acceptance Criteria
- [ ] A0 build: CL9Editor builds with the UE 5.8 engine at `D:\Program Files\Epic Games\UE_5.8`; editor starts; zero Blueprint compile errors in Saved/Logs/CL9.log before any PIE; `CL9.Replication.Registration` and the new layout tests pass.
- [ ] A1 auto start (PIE `Lvl_Start` → host henhouse, no cheats): within 10 s of the stage loading, Phase == Night, accepted orders shown in the HUD, monsters and loot exist, OrderSelect never opened, lever pose DOWN.
- [ ] A2 Dark: before the flip, `GetPowerStage() == Dark`; indoor lamps invisible, campfire lamp visible; NightLighting SkyLeak == 0; exactly one `ACL9PowerSwitch` in the first indoor room, its light visible from the yard-side doorway; no loot or monster spawn on its cell; clock shows the switch hint.
- [ ] A3 Flip (real input E on the switch, then `cl9.power` on a second seed): T set, one target per participant; stage LightsOn → Flicker → Dead at the configured times; Dead: indoor lamps off, SkyLeak at the round value; the camera leaves the pawn, follows the cell path without passing through walls, ends looking at the target, and returns to the pawn; movement/look/actions refused during the lock window; `OnDamage` and monster catch/execute refused until lock + Grace; logs show no Error.
- [ ] A4 Targets: on henhouse/farm/pigfarm seeds the candidate list is ≥ 2, all on floor 0; with 2 PIE players the two targets differ.
- [ ] A5 Lever OFF (go home): host first press in Night shows "트럭 밖 N명"; second press within 5 s ends the round; a player standing outside the truck dies with an empty inventory and is named on the Dawn screen; a player inside the truck survives with their carry; Lobby → lever pose UP, clock shows the lever hint; host presses twice → next round starts with a new seed and Dark again.
- [ ] A6 Timer end: with `NightLength` shortened via the rules cell or cheat, the timer ending runs the same left-behind pass and lever pose goes UP.
- [ ] A7 All left behind: solo host outside the truck when the round ends with `bWipeEndsRun` true → GameOver screen, RunOver active; with it false → lobby, revived.
- [ ] A8 Truck: 4 capsules fit inside the cargo box at once and walk to the lever; steps/ramp climbable with a real capsule; `IsInside` true inside, false 50 cm outside the rear opening; old truck actors absent from the four stage umaps; lever reachable inside.
- [ ] A9 Regression `Lvl_Flat` (EditorStartupMap): round auto-starts, no switch, `GetPowerStage() == None`, lighting as before minus nothing; shop opens in the lobby after `cl9.night`; blackout still turns lamps off.
- [ ] A10 Human checks (owner, real mouse): find the switch cold from the yard; the fly-through is readable and not nauseating; the flicker reads as "the power died", not a bug; the truck feels big enough for 4.

## Related Files / Modules
| File | Role |
|---|---|
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | auto start, truck/lever spawn, `RequestPowerOn`, left-behind pass, run-over branch |
| `Source/CL9/GameMode/CL9GameState.{h,cpp}` | power state, intro targets, windows, left-behind names |
| `Source/CL9/GameMode/CL9PlayerController.{h,cpp}` | intro camera driver |
| `Source/CL9/GameMode/CL9Cheats.cpp`, `CL9CheatManager.*` | `cl9.power`, `cl9.truck` |
| `Source/CL9/World/CL9Truck.{h,cpp}` | new truck actor |
| `Source/CL9/World/CL9SiteMap.{h,cpp}`, `CL9SiteLayout.{h,cpp}`, `CL9SiteLayoutTests.cpp` | switch spawn, lamp groups, layout helpers, tests |
| `Source/CL9/World/CL9NightLighting.{h,cpp}` | stage-aware target + SkyLeak snap |
| `Source/CL9/Items/CL9PowerSwitch.{h,cpp}`, `CL9DepartLever.{h,cpp}` | new switch; lever switch + pose |
| `Source/CL9/Character/CL9Character.{h,cpp}`, `CL9CharacterMovement.cpp`, `Source/CL9/Monsters/CL9Monster.cpp` | lock / invulnerability gates, worn-item destroy |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}`, `CL9Types.h` | columns, enums, structs |
| `Source/CL9/UI/CL9HUDWidget.cpp`, Dawn widget, `Content/Strings/CL9_Strings.csv` (+ generated `CL9.csv`) | suffixes, prompts, left-behind line |
| `Tools/Blender/build_food_truck_b.py`, `build_depart_lever.py`, `build_power_switch.py`; `Tools/Unreal/import_food_truck_b.py` (etc.) | art pipeline (REPO-relative) |
| `ArtSource/map/food_truck_b/…`, `Content/Maps/FoodTruck/…` | new truck, lever, switch assets |
| `Content/Data/CL9Rules.uasset`, four stage `.umap` files | lead-only edits |

## Must-Preserve
- The listen host gets no OnRep: every new presentation is derived each tick from replicated server times/state; no Multicast added.
- Server view-target paths win (death cam, execution, spectate, revive); the intro camera only undoes itself.
- `OnDamage` stays the single damage door; `HostKill` bypasses it only for an execution already in progress; the left-behind kill uses `HostApplyDamage` after Dawn.
- The movement speed formula uses only replicated inputs, identical on the server and the owning client.
- Locally spawned interactables are non-replicated, built from the seed, listed in `SpawnedActors`; their effects go through host-side replicated fields; the host re-resolves focus.
- `EndNight` keeps its run-over guard; the wipe still fires only in Night; `ClearNightPopulation` ends executions before monsters go.
- Lobby-only gates stay (shop, `AcceptOrders`, `LoadRun`, `cl9.day`); save before `OfferOrders`.
- New replicated fields registered (CL9ReplicationCheck); `StartNight`/`EndNight`/`EnterLobby`/`ECL9NightPhase` kept, no new phase value.
- `EditorStartupMap` stays `Lvl_Flat`; `GameDefaultMap`/`StartLevel` stay `Lvl_Start`; nothing is committed; the five Tools path-fix files are never committed.
- Blackout behaviour and its clock unchanged; `bWipeEndsRun` stays a data switch.

## Fairness Constraints
- Round start, go-home, power-on, target assignment and left-behind deaths are host rulings (GameMode); clients only derive from replicated state.
- Target shuffle uses a server RNG once per flip; every participant gets a distinct target while targets last.

## Existing Integration Contract
- Interactables go through `ICL9Interactable` (`CanInteract` gates the prompt; `Interact` runs on the host).
- HUD derives from state every tick; screens only through `UCL9UIManager` + UITable rows.
- Site map lamp/flicker ownership stays in `ACL9SiteMap`; global light rows stay in `ACL9NightLighting`.

## Execution Notes
- Recommended model: the strongest current Claude model for the lead (integration, editor/asset steps, gates, review) and Claude Opus for the gameplay role (replication timing, camera, run-over branch) and the art role (first form of a new walkable truck); Sonnet is acceptable for the ui role (strings, suffix plumbing, one Dawn line) and the docs role (applying settled decisions).
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Order: gameplay (K1-K9, K11-K12) and art (K3-K4 meshes) run in parallel → ui after gameplay's interfaces compile → gameplay camera (K10) → lead imports + Rules cells + umap actor deletion → build → gates A0-A9 → docs. The editor may be closed for builds after saving (owner-approved 2026-10-05).
- Editor-side Python: the editor console `py "<path>"` or the editor MCP (`Tools/Unreal/run_python.py` is broken on this machine). Blender: `py -3 Tools/Blender/run_headless.py <script>`; no Blender MCP.

## Results (2026-10-06)

### Run 1 — `round-truck-core` (REPORTED 2026-10-06, nothing committed)
- Build: #1 green on the first try (186 s). Role compile checks all ended `COMPILE OK` (gameplay 144 s, map 118 s; the ui role's two FAILs were gameplay-side errors that gameplay fixed). Two new source files: `CL9Truck`, `CL9PowerSwitch`.
- Landed: K1 auto start, K2 lever switch + handle pose, K3 truck actor, K5 left behind (+ `EndRunAtDawn`), K6 power state, K7 lamp groups and lighting stage, K8 switch and its spawn cell, K9 targets, K11 lock/invulnerability, K12 `cl9.power` / `cl9.truck` and the layout tests (`CL9.Map.PowerSwitch`, `CL9.Map.CellPath`), the ui clock suffixes / prompts / Dawn line (string rows No 231-239, ko only), and the art: five new meshes (truck, steps, lever base, lever handle, power switch) built with Blender headless, imported by the lead into `/Game/Maps/FoodTruck/B/*`. The lead also set the `CL9Rules` cells (truck, lever, switch, intro times), deleted the two old hand-placed truck actors from each of the four stage levels, and wrote decisions.md `## 2026-10-06`. **Not landed in run 1: the K10 intro camera** (run 2).
- Verified in PIE as a solo host (lead), zero errors: **A1, A2, A3** (lights, stages, flicker, lock and invulnerability windows — the camera flight is not part of this run), **A4** (3 candidates), **A5, A7**, **A8 (the `IsInside` boundary only)**, **A9**. Automation `CL9.Map.*`: 11 of 12 pass; `CL9.Map.Districts` fails and looks like the giant-stage work in progress, not verified.
- Changed from the spec as written (details in decisions.md `## 2026-10-06`):
  - **Truck exterior width 424 cm**, not "about 400": the wheels sit outside the interior wall so the 340 cm interior stays clear (interior X 10-714 / Y ±170 / floor 64 / ceiling 320, rear opening 260 × 236, steps with 16 cm risers — art manifest).
  - **Intro candidates are only the modules the cell path reaches** from the switch cell (K9 amended): a stair-room dead end such as the emergency-exit room cannot be flown to without clipping the treads, so it is dropped and its slot goes to the deepest reachable floor-0 non-stair module. `CL9.Map.PowerSwitch` asserts "every candidate is reachable".
  - **Switch light = code constants** (6 cd, radius 450 cm, cool colour), not `CL9Rules` columns — the spec gave no values. Moving them to columns waits for the owner's "is it visible from the doorway" look in PIE.
  - The lever console is baked into the truck body mesh (with its UCX) because the lever mesh itself is query-only; hinge 45 cm off the front wall so the ±35° handle clears it.
  - Side decisions: the client's lever prompt during a round is always "host only"; the left-behind line is appended to `ResultText` (a new `BindWidget` would break the `WBP_Dawn` compile) with a separator key `Dawn.LeftBehindSep`; all-left-behind ends the run with no `SpectateDelay` beat (already dawn); the intro lock/invulnerability windows are true only in Night; a layout with no intro candidate only runs the lights (on, then dead) with nobody locked and no camera; the switch cell's spawn point is removed in every layout; `HostRevive` resets the knockdown time and the hit shake/sound fire only on a rising edge.
- **Still open — needs a human or a second player** (never claimed as working):
  - **A10** (owner, real mouse): finding the switch cold from the yard, the fly-through reading well and not nauseating, the flicker reading as "power died", the truck feeling big enough for four.
  - **Two players**: distinct targets (A4), the client's lever prompt, the Dawn left-behind line and revive presentation on the client, the replicated power/left-behind state on a real client.
  - **A6** (timer end) is not in the lead's pass list; **A8 beyond the boundary** (four capsules at once, steps with a real capsule, handle clearance) is not recorded.
  - `CL9.Replication.Registration` was not run (it is known to crash the editor when run in-editor); the three new replicated fields are covered by the automatic scan according to the gameplay report.
  - Not measured: the performance gate (lever tick + switch light, +1 ms rule in performance.md) and whether `WBP_Dawn`'s `ResultText` holds two or more lines.
  - Seen by reading code only: on the host the order screen could open and close within one frame at lobby entry (the offer is set and cleared in one call) — check for a flash or a captured cursor.

### Run 2 — `round-intro-camera` (REPORTED 2026-10-06, nothing committed)
- Build #2 green on the first try (122 s). Roles: gameplay (K10 intro camera in `ACL9PlayerController::PlayerTick`, third-person own body while it is active), docs (loop, order, session, cheats, character, performance, sequence, pie-scenarios W section, this Results section).
- PIE on `Lvl_Henhouse`, solo host, automated through editor Python + `cl9.*` cheats:
  - Flip (`cl9.power`): the view target hard-cuts to a local `CameraActor` (`IsIntroCameraActive()` true), the camera follows the cell path at 170 cm (sampled ~1 s apart: (650,368) → (1150,182) → (1150,−485) → (1576,−750) → (1650,−750)), looks along the path, holds at the target, and the view returns to the pawn by +6.4 s with movement restored. Log: `Intro camera: start — 6 cells, 2500 cm, target module 11 … 0/5 segment(s) lifted` and `Intro camera: return`; zero errors or warnings.
  - Long path (7000 cm on a second seed) logs `over the speed cap — dip cut`.
  - Ending the round mid-flight logs `Intro camera: aborted (phase)` and the view is back on the pawn the same tick.
- Not verified by automation (human checks): whether the flight reads well and causes no motion sickness, wall/door-frame clipping by eye, the dip cut itself (the abort test ended before +2.5 s), the third-person body during the flight, two-player distinct targets (A4), client-side camera timing, performance (U).

## Follow-up K10-F — continuous intro camera (2026-10-10, owner decision after the video)
Owner, watching `Saved/Video/power_intro.mp4`: "누른 상태에서 갑자기 위치가 바뀌면서 가는 게 아니라, 거기서부터 카메라가 돌고 앞으로 쭉 이어져야지 — 지금은 순간이동해서 가는 것 같다."
Two jumps were measured on the filmed seed (`Intro camera: start … 17 cells, 8000 cm … dip cut`): the hard cut in (the camera spawned at the switch cell centre, 170 cm, while the presser stood 78 cm away looking at the switch) and the 0.15 s dip that skipped the middle of every henhouse path (8000–10500 cm > 1500 cm/s × 4 s). This follow-up replaces those two parts of K10; everything else in K10 (cell path, sweep/lift, look-ahead 400, turn rate 180°/s, return fade, abort rules, third-person body) stays.

- **F1 No hard cut in.** When the intro camera starts inside the lead-in (`Elapsed < IntroLeadIn`), it spawns at the local player's current view — `PlayerCameraManager->GetCameraLocation()` / `GetCameraRotation()` (= S) — so the switch to it is invisible. The rotation starts at S's rotation (no snap) and turns toward the look-ahead point at ≤ 180°/s from the first tick. A late start (`Elapsed >= IntroLeadIn`, e.g. a client that received T late) keeps the old behaviour (evaluate at Elapsed, snap the first rotation).
- **F2 Gather during the lead-in.** In [0, IntroLeadIn) every participant's camera travels at constant speed from S to the switch cell centre P0 (170 cm) along a gather path G, so that **all participants leave P0 together at `IntroLeadIn`** (owner: "스위치 칸까지 빠르게 간 후 거기서부터 같이 시작", "출발 전에, 스위치 켜는 연출이 끝나기 전까지" = the lead-in). G:
  - S maps to a floor-0 cell (`ACL9SiteMap::WorldToCell`, floor 0): G = [S, centre of that cell, …cell centres…, P0] using a new cell-to-cell BFS with the same edge rules as `FindCellPath` (Open/Door pass, Wall/Flap block, stair modules only as an endpoint). If S's cell is the switch cell (the presser): G = [S, P0].
  - S has no floor-0 cell (in the truck, on floor 1, outside the site) or no cell path exists: G = [S, P0] straight — it may pass through walls (owner accepted, 2026-10-10).
  - G's segments get the same once-per-segment sphere sweep and Z lift as the flight path, except that **S itself is never lifted** (the first segment slopes to the lifted next point).
  - Long gathers are fast (e.g. 20 m in 0.5 s); `IntroLeadIn` is a Rules cell and can be lengthened later without code.
- **F3 Flight time 7.0 s.** `IntroFlyTime` 4.0 → **7.0** (header default and the `CL9Rules` asset cell); `IntroMaxSpeed` stays 1500 cm/s, so flights up to 10500 cm are continuous. Longer flights keep the K10 dip (owner accepted). The flight itself is unchanged: from P0 along the K10 cell path, starting at `IntroLeadIn` for everyone.
- **F4 Windows follow.** No formula changes; with the defaults 0.5 / 7.0 / 1.5 / 0.4 / 1.0 the lights stay on until +7.5 s, flicker until +9.0 s, lock until +9.4 s, invulnerability until +10.4 s.
- Interface: `bool CL9SiteLayout::FindCellPathToCell(const FCL9SiteLayout& Layout, FIntPoint From, FIntPoint To, TArray<FIntPoint>& OutCells);` (pure, engine-free, layout test added beside `CL9.Map.CellPath`).

- **F5 Own body (2026-10-11, owner decision after `power_intro_v2.mp4`).** For about 0.05–0.2 s after the press the owner's third-person body (hat, shoulders) covered the lower half of the screen, because the body switched on while the camera was still at the eye. Now the owner keeps the first-person presentation (arms and held item visible, own body hidden) until the intro camera is at least `IntroCamBodyClearance` (120 cm, code constant) from the owner's first-person camera, then switches to the third-person body once (one way) until the intro ends. The switch is decided by `ACL9PlayerController::IsIntroBodyShown()`; `IsIntroCameraActive()` keeps its meaning. The intro camera inherits the first-person FOV/scale settings so the arms do not change projection on the first frame. Log: `Intro camera: body on at <D> cm (+<E> s)`.
- **F6 Minimum speed (2026-10-11, owner decision).** With the fixed 7 s flight a short path crawled (1500 cm ≈ 2 m/s). New Rules column `IntroMinSpeed` (600 cm/s, draft — the example value of the owner's choice; `Validate`: positive and ≤ `IntroMaxSpeed`). If L ≤ `IntroMinSpeed × IntroFlyTime` the camera flies at `IntroMinSpeed`, arrives at `L / IntroMinSpeed` and holds at the target looking at the jar until `IntroLeadIn + IntroFlyTime`; otherwise even speed, and over the cap the dip. Windows unchanged. The `start —` log ends with `flight min speed, arrive +X s | even | dip cut (over the speed cap)`.

### Acceptance (K10-F)
- [ ] G0 build green; `CL9.Map.CellPath` and the new cell-to-cell test pass.
- [ ] G1 Presser, real E on `Lvl_Henhouse`: the first intro-camera frame equals the pawn view at the press (≤ 1 cm, ≤ 0.5°); sampling every tick through [0, LeadIn + Fly), no tick moves the camera more than twice the expected step (no teleport) on a path ≤ 10500 cm; the log has no `Intro camera: dip`.
- [ ] G2 At `IntroLeadIn` (± one tick) every participant's camera is at P0 (≤ 5 cm).
- [ ] G3 Two-player listen PIE: a client standing in another floor-0 room gathers along cells (log shows the gather cell count > 1 and no blocked-segment warning) and then flies to its own target; a client inside the truck logs a straight gather.
- [ ] G4 `cl9.power` prints the new windows (+7.50 / +9.00 / +9.40 / +10.40 s); lock and invulnerability hold for those windows.
- [ ] G5 Regressions: dying mid-intro still aborts and returns the view; `Lvl_Flat` (no switch) has no intro; the return fade is unchanged.
- [ ] G6 The lead re-films `Saved/Video/power_intro.mp4` (presser view) for the owner.
- [ ] G7 (F5) The first 0.3 s after the press show no own body; `body on at D cm` with D ≥ 120, once per intro, never back.
- [ ] G8 (F6) A path ≤ 4200 cm logs `flight min speed, arrive +X s` (X = 0.5 + L/600) and holds after X; longer paths as before.

### Results (K10-F, run `intro-camera-continuous`, REPORTED 2026-10-11, nothing committed)
- Roles: gameplay only (opus); the split and review were done by the lead (Opus) because the Fable weekly limit was exhausted. Two rounds (F1–F4 on 2026-10-10, F5–F6 on 2026-10-11) in one run; `CL9Character.{h,cpp}` were added to the grants for round 2, and map-owned `CL9SiteLayout.{h,cpp}`/tests were lent for `FindCellPathToCell`.
- Builds: two, both green on the first try (40 s, 153 s). After the first one Windows Smart App Control blocked the new `UnrealEditor-CL9.dll` once (editor stuck at the splash, 0xc0e90002); the same DLL loaded fine a day later. `CL9Rules`: `IntroFlyTime` 7.0 and `IntroMinSpeed` 600 saved and re-read.
- G0: `CL9.Map.CellPath`, `CL9.Map.CellPathToCell`, `CL9.Map.PowerSwitch` Success.
- G1: solo host, real E, three seeds (4500 / 1500 / 2000 cm): the first intro frame equals the pawn view (0.0 cm, 0.00°); per-tick steps at most 10.8 cm, none over 200 cm; no `dip`.
- G2: `leave switch cell` within 0–7 cm of P0, matching (overshoot time × flight speed); both machines share P0 in two-player runs.
- G3: listen 2-player — a client in another floor-0 room logged `gather 6 cell(s), 2717 cm in 0.50 s` with no blocked-segment warning (fast gather, up to 120 cm per tick); a client in the truck logged `gather straight`.
- G4: `cl9.power` printed `LightsOn until +7.50s, Flicker until +9.00s, lock until +9.40s, invulnerable until +10.40s`.
- G5: ending the round at +2 s logged `aborted (phase)` on both machines and the view returned to first person (body hidden); `Lvl_Flat` has no switch and no intro.
- G6: videos `Saved/Video/power_intro_v2.mp4` (after F1–F4) and `power_intro_v3.mp4` (after F5–F6), recorded with PrintWindow so other windows cannot cover the PIE window.
- G7: `body on at 123.2 cm (+0.65 s)` solo, 137–198 cm in two-player runs; frames after the press show no body.
- G8: a 2000 cm path logged `flight min speed, arrive +3.83 s` and held at the jar.
- Human checks left: whether the arms leaving the frame during the first 120 cm read well; the very fast gather from far rooms (≈54 m/s for 0.5 s) — `IntroLeadIn` is a Rules cell.
