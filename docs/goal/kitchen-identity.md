# goal: kitchen-identity

Status: implemented and PIE-verified (2026-10-09, run `kitchen-identity`) — uncommitted; human checks A1/A5 pending.

## One-line Goal
Make the cooking station read as a kitchen at a glance and announce itself when a player walks past — a dressed stove set, an always-on pilot flame, a kitchen sign, a gas/fan ambience loop, a "부엌" place banner and a "[G] 올려서 익히기" prompt — without touching how cooking is judged.

## Background / Motivation
Owner (2026-10-09): "부엌에서 요리를 해야 되는데, 부엌인걸 인지를 못하겠고 요리되는 과정을 전혀 모르겠어" →
"그러니까 요리를 해야 되는데 달랑 식탁 같은 거 하나 있어서. 저게 요리해야 되는 곳인지 그냥 흔한 책상 같은 건지 구분이 안 간다고.
프라이팬이라든지 지나가면 부엌이다라고 알려주는 뭐 그런 게 필요해."

Order the owner set: **placement later** ("배치보다 일단 기능부터"), and **the kitchen itself before per-dish cooking feedback**
("요리 하나하나보다 부엌 자체 시스템부터"). This goal is the first step; per-dish feedback is the next goal (see Out-of-scope).

Facts found by recon (2026-10-09):
- `BP_CL9Kitchen` (mesh `SM_Kitchen_A`, a flat 120×300×90 cm griddle) exists **only in `Lvl_Flat`** (3 instances). The stage levels
  `Lvl_Henhouse`/`Lvl_Farm`/`Lvl_PigFarm` have no station, counter, truck panel or cart. The building's `kitchen` module carries only the
  sign `SM_Henhouse_Sign_Kitchen_A` — no station, although `docs/plan/data/module.md:91` lists "조리대 · 냄비 · 선반".
  `docs/plan/data/site.md:174` and `docs/systems/stage.md:110` wrongly say the kitchen/counter come along when a level is duplicated.
- `ACL9Kitchen` derives `HeatZone` and `GetTopLocation()` (cook gauge anchor) from `MeshComponent->Bounds` (`CL9Kitchen.cpp:30-38, 52-56`).
  **Anything added to that mesh's height moves the heat zone and the gauge** — dressing must live in separate components.
- The station's scan label already reads "조리대" (`DisplayName` set in `BP_CL9Kitchen`).
- Sound: `CL9Sound::PlayAt` is one-shot with no attenuation (`CL9SoundTable.h`, `docs/systems/sound.md:34-36`); no attenuation or
  concurrency asset for gameplay sounds exists yet. All SFX are synthesized (`Tools/Audio/synth_sfx.py`, `Tools/Unreal/synth_feel_sfx.py`).
- Focus: `ACL9Character::ResolveFocus` picks items and `ICL9Interactable`s only; the kitchen is neither. The prompt chain
  (`GetFocusPromptPattern`, `CL9Character.cpp:570-596`) says "there is no drop prompt" (2026-09-13 owner) — this goal adds one exception.
- HUD already walks every `ACL9Kitchen` each tick for cook gauges (`CL9HUDWidget.cpp:685-714`, allowed by performance.md:147 — small bound).

## In-scope
1. **Dressed stove set (art + gameplay).** The station becomes one kitchen set so that wherever it is placed later it reads as a kitchen:
   - Front: burner ports along the front lip under the plate and 2–4 control dials on the red front panel.
   - Back: a backsplash panel along the back long edge with a hanging frying pan, ladle and spatula.
   - Above: a range hood with a short flue; beside: a pot on a side shelf or at the plate's end that does **not** sit on the heat zone.
   - Inside the plate's footprint (120×300) nothing of the dressing sits between 90 cm and 200 cm — the heat zone (90–150 cm) and
     placing/picking on the plate are unchanged. The hood's underside is ≥ 200 cm and it reaches at most half the plate depth from the back.
   - The work surface stays `MeshComponent` = `SM_Kitchen_A`, unchanged. Dressing is a **separate** static mesh component
     (`DressingMesh`) on `ACL9Kitchen`, so `HeatZone`/`GetTopLocation()` keep their current values.
   - Dressing collision: backsplash blocks; utensils, hood and flue have no collision (no head bumps, no snagging the camera).
2. **Always-on pilot flame.** Small blue flames at every burner port, always lit on every machine (cosmetic — the station is always hot today).
   Emissive, flickering material (same approach as `M_CL9_Campfire_Flame`); **no light component** — it must not change darkness balance.
   Separate mesh component (`PilotFlameMesh`) so the next goal can drive it.
3. **Kitchen sign on the hood.** The same icon as the building kitchen-room sign (`SM_Henhouse_Sign_Kitchen_A`), mounted on the hood front,
   readable from ~10 m. Reuse the existing sign mesh as a component if it fits; otherwise art bakes the same icon into the dressing.
4. **Gas/fan ambience loop.** A quiet loop (soft gas hiss + range-hood fan hum) playing at the station on every non-dedicated machine from
   `BeginPlay`, heard by all players, fading with distance. Needs:
   - a looping sound asset (synthesized, like the other SFX) referenced by a new `SoundTable` row `KitchenAmbience` (no path in C++);
   - the first gameplay **attenuation asset** (draft: full volume ≤ 200 cm, silent ≥ 1200 cm) set on that sound asset;
   - a concurrency setting so adjacent stations (Lvl_Flat has 3 side by side) do not stack louder than one station (draft: max 1 voice,
     stop farthest).
   - Monsters do **not** hear it (no noise event).
5. **"부엌" place banner (ui).** When the local, alive player's pawn comes within `KitchenNoticeRadius` (HUD property edited in `WBP_HUD`,
   like `CookGaugeLift` — a presentation value, not a balance cell; draft 600 cm)
   of any station after being outside `KitchenNoticeRadius × 1.25` of all stations, the HUD shows "부엌" as a place name for ~2.5 s with a fade,
   on that player's screen only. Spawning inside the radius counts as entering. Not shown while dead/spectating or while intro-locked.
   Computed in the HUD's existing kitchen walk — no new world scan, no replication.
6. **Placing prompt.** When the local player holds a cookable item (its row has `CookResult` or `BurnResult`) and stands within `PickRadius`
   of a station, facing it (`FocusMinAimDot`), the prompt reads "[G] 올려서 익히기" (G = the current `DropAction` key label, rebind-aware).
   Existing prompts keep priority (wear > pick up > interactable); this one comes next, before cart release. It is the only drop prompt.
7. **Strings (ko only):** `Hud.PlaceKitchen` "부엌", `Prompt.KitchenPlace` "[{0}] 올려서 익히기" ({0} = drop key).
8. **Docs:** `systems/cooking.md` (kitchen set, pilot flame, ambience, banner, prompt), `asset/map/kitchen.md` (new parts, measurements),
   `systems/sound.md` (fixture ambience loop + first attenuation/concurrency asset), `systems/ui.md` (place banner, kitchen prompt),
   `decisions.md` (2026-10-09 entry: kitchen identity first, drop-prompt exception),
   and correct `plan/data/site.md:174` / `systems/stage.md:110` to say stage levels have no station yet.

## Out-of-scope
- **Placing stations** in stage levels, the truck or the `kitchen` module (owner: placement later). Verification uses `Lvl_Flat`.
- **Per-dish cooking feedback — next goal (`cooking-progress-feedback`).** Owner already accepted these defaults (2026-10-09):
  flame grows only while something cooks; plate heat glow; ingredient color shifts gradually (raw → cooked → burnt) plus a mesh swap at each
  step; **cooked egg = fried egg (sunny-side up), burnt egg = charred fried egg**; sizzle loop while cooking, a one-shot cue when it turns
  cooked, harsher sizzle + crackle from `BurnWarnTime`, a one-shot "poof" when burnt; white steam while cooking, grey smoke from
  `BurnWarnTime`, a black puff when burnt, thin smoke from a burnt item while it is not held; monsters do not hear any of it.
- Burner on/off control, flipping, recipes/combining, `Slot=Kitchen` shop equipment.
- Changing cook timings, the one-item-per-station rule, the heat zone, the cook gauge, replication or the drop physics.
- Translations (ko column only — loc-korean-only).

## Acceptance Criteria
- [ ] A1 In `Lvl_Flat` PIE, each of the 3 stations shows burner ports, dials, backsplash with pan/ladle/spatula, hood with flue and the kitchen
      sign; a pot sits beside, off the heat zone. Human check: "it reads as a kitchen, not a table".
- [ ] A2 `HeatZone` world extent/location and `GetTopLocation()` per station are equal (±1 cm) before and after the change (logged by QA).
- [ ] A3 Cooking still works unchanged: an egg dropped with G from the front lands on the plate, cooks at `CookTime`, burns at `BurnTime`,
      the cook gauge shows over the plate (existing 020/4d path).
- [ ] A4 Pilot flames are visible and flicker on every station at all times, including when the level is dark; the station adds no light
      component (QA counts light components on `BP_CL9Kitchen` = 0).
- [ ] A5 `KitchenAmbience` resolves (`CL9Sound` resolved count or an audio component with a valid sound per station) on the listen host and a
      client; nothing plays on a dedicated server. Human check: audible when near, fades by ~12 m, three adjacent stations are not louder
      than one.
- [ ] A6 Walking into 600 cm of a station shows "부엌" once; staying inside does not repeat it; leaving past 750 cm and returning shows it
      again. Log line `Kitchen notice` on each show. Not shown while dead/spectating.
- [ ] A7 Holding a raw egg and facing a station within reach shows "[G] 올려서 익히기"; holding a frying pan (no cook rows) or nothing shows no
      such prompt; rebinding drop to another key changes the label.
- [ ] A8 2-player (listen host + 1 client, PIE): A1, A4, A5, A6, A7 hold on the client; no new RPC or replicated property was added.
- [ ] A9 Build succeeds; existing automation `CL9.Map` unchanged result (the known `Districts` failure is giant-stage WIP, not this goal).
- [ ] A10 Docs updated as listed in In-scope 8.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Items/CL9Kitchen.h/.cpp` | Station actor — add `DressingMesh`, `PilotFlameMesh`, ambience audio (presentation only) |
| `Source/CL9/World/CL9Fixture.h` | Base: `MeshComponent`, `DisplayName` |
| `Content/Items/BP_CL9Kitchen` | Assign dressing/flame/sign meshes, keep `SM_Kitchen_A` on `MeshComponent` |
| `ArtSource/map/kitchen/`, `Tools/Blender/build_kitchen.py`, `Tools/Unreal/import_kitchen.py` | Kitchen art source and import |
| `Content/Maps/Henhouse/Props/M_CL9_Campfire_Flame` | Reference for the flickering emissive flame |
| `Content/Maps/Henhouse/.../SM_Henhouse_Sign_Kitchen_A` | Kitchen sign icon to reuse |
| `Source/CL9/Data/CL9SoundTable.h/.cpp`, `Content/Data/SoundTable` | New row name `KitchenAmbience` |
| `Tools/Audio/synth_sfx.py`, `Tools/Unreal/synth_feel_sfx.py`, `Tools/Unreal/import_feel_sfx.py` | SFX synthesis/import pattern |
| `Content/UI/WBP_HUD` | `KitchenNoticeRadius` (HUD property, edited in WBP_HUD) |
| `Source/CL9/Character/CL9Character.h/.cpp` | `GetFocusPromptPattern` / key text — kitchen prompt |
| `Source/CL9/UI/CL9HUDWidget.h/.cpp`, `Content/UI/WBP_HUD` | Place banner (new optional text) in the existing kitchen walk |
| `Content/Strings/CL9.csv` (+ `CL9_Strings.csv` per string workflow) | `Hud.PlaceKitchen`, `Prompt.KitchenPlace` |
| `Content/Maps/Lvl_Flat.umap` | Verification level (3 stations) |

## Must-Preserve
- Cooking judgment: host-only `TickCooking`, one item per station, `CookTime`/`BurnTime`/`BurnWarnTime`, `SetAlias` transitions.
- `HeatZone` size/position/absolute rotation & scale, `GetTopLocation()`, cook gauge behaviour (ui.md 조리 게이지).
- No new RPC, no new replicated property; presentation derives locally (conventions § 9). Dedicated server plays no sound.
- Prompt priority for existing prompts; E stays pick/interact, G stays drop.
- Content paths only through DataAssets (`SoundTable`, `CL9Rules`) — no hard-coded asset paths in C++ (conventions § 5).
- Performance: no new ticking class; the banner reuses the HUD's existing bounded kitchen walk.
- The uncommitted round-truck-core / round-intro-camera work in the tree is not reverted or reformatted.

## Execution Notes
- Run through the project harness: `/cl9 task` — roles art (dressing, flame, sign, ambience synth + attenuation/concurrency assets, import),
  gameplay (kitchen components, prompt, rules cell), ui (banner widget, strings), docs; lead does BP assignment, build, PIE gates.
- Recommended model: the strongest current Claude model (Opus 5.5 / Fable 5.1) for art (shape that "reads as a kitchen") and gameplay
  (prompt priority, component split around `HeatZone`). Sonnet is acceptable for ui strings/widget and docs.
- This document cannot enforce the model — the executing session's `/model` decides. If the session model is below the recommendation,
  tell the owner and confirm before proceeding.
- Build with the editor closed (save first). Editor/PIE/build locks per `docs/workflow/sessions.md`.

## Results (2026-10-09, run `kitchen-identity`)
One build (first try), `Lvl_Flat` PIE by script oracle (1 player, then listen server + 1 client). Evidence: `Saved/QA/import_kitchen_identity_{assets,bp,verify}.json`, `Saved/Logs/CL9.log` (`Kitchen notice`).
- [ ] A1 Dressing reads as a kitchen — **human check pending** (Blender renders in `ArtSource/map/kitchen/preview/`; all 3 stations carry `SM_Kitchen_Dressing_A` + `SM_Kitchen_PilotFlame_A`; neighbours K1–K2 do not overlap, gap 7.4 cm).
- [x] A2 HeatZone 300×120×60 at z 90–150 and `GetTopLocation()` z 90 on all 3 — identical to the 020 QA baseline.
- [x] A3 Cooking unchanged — a raw meat dropped with G started cooking at once (`GetCookingItem`) and ended `meat/burnt` on the plate.
- [x] A4 Pilot flame on all 3; 0 light components; flame `affect_dynamic_indirect_lighting=false`, no shadow. (Flicker look — human.)
- [x] A5 `KitchenAmbience` resolved 3/3 (1P), +6 in listen+client; all audio components playing; asset = looping, `SC_Ambience`, `SA_CL9_Kitchen` 200/1000, `SCC_KitchenAmbience` max 1. Loudness/fade by ear — **human check pending**.
- [x] A6 Banner: spawn 535 cm → 1 notice; enter 400 → 1; 680↔400 → none; out 900 → in 400 → 1 (3 log lines total). Client-only in 2P.
- [x] A7 Holding a cookable item: "[G] 올려서 익히기" at plate centre, one end, 100 cm; none at 140 cm, turned away, empty hand, frying pan. A nearby item's "[E] 줍기" wins (priority). Rebind label (S1) not exercised — code path only.
- [x] A8 Client (no authority) sees dressing/flame/audio, gets its own banner, and its own "[G] 올려서 익히기" while holding meat. No new RPC or replicated property.
- [x] A9 Build OK; `CL9.Map` 11/12 — `Districts` fails as before (giant-stage WIP, unrelated).
- [x] A10 Docs: cooking.md, sound.md, ui.md, asset/map/kitchen.md, pie-scenarios.md §X, site.md/stage.md corrections, decisions.md 2026-10-09.
- Not done / follow-ups: `docs/systems/input.md` DropKeyText note (ui), `docs/goal/farm-stage.md:41` same duplication error, pie-scenarios M1 "놓기(E)" wording, performance U not measured (automated PIE is throttled).

