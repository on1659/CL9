# goal: cooking-progress-feedback

Status: implemented and PIE-verified (2026-10-09, run `cooking-progress-feedback`) — uncommitted; owner review of the video pending; stove drop issue open.

## One-line Goal
Make cooking visible and audible while it happens — the burner flares and the plate glows, heat haze rises, the ingredient browns,
steam then smoke rises, it sizzles and crackles, and a cooked egg becomes a fried egg — all derived on every machine from state that
is already replicated, without touching how cooking is judged.

## Background / Motivation
Follow-up of `kitchen-identity` (same day). Owner after watching the cooking video (`Saved/Video/kitchen_cooking.mp4`):
"익을 때 주변에 열기가 나온다던가 그런 거 없어?" → "ㅇㅇ 요리하는 거 좀 표현해줘". Today the only signs of cooking are the cook gauge and
the mesh swap at the end; the always-on pilot flame does not react.

Owner-accepted defaults (2026-10-09, asked before `kitchen-identity` was split off — see that spec's Out-of-scope) plus heat haze (owner "ㅇㅇ"):
flame grows only while something cooks · plate heat glow · gradual colour shift + mesh swap per step · cooked egg = fried egg
(sunny-side up), burnt egg = charred fried egg · sizzle loop, cooked cue, harsher sizzle + crackle from `BurnWarnTime`, "poof" when burnt ·
white steam while cooking, grey smoke from `BurnWarnTime`, black puff when burnt, thin smoke from a burnt item while it is not held ·
monsters do not hear any of it · heat haze (shimmer) above the plate while cooking.

## In-scope
1. **Station states** (per `ACL9Kitchen`, presentation only, every non-dedicated machine), derived each tick from the replicated
   `Cooking` item and its replicated `CookPhase`/`CookProgress`/alias:
   - *Idle* (nothing cooking): today's look — small blue pilot flames, no glow, no haze, ambience only.
   - *Cooking* (`CookPhase == Cooking`): flames flare (taller, warmer — the same `PilotFlameMesh` driven by a material scalar, no light
     component, no GI), the plate gains a dull orange heat glow, a heat-haze card shimmers above the cooking item, white steam rises from it
     (density grows with progress), sizzle loop plays.
   - *Burning, early* (`CookPhase == Burning` before the warn window): flames/glow/haze as Cooking, steam thins, sizzle continues.
   - *Burn warning* (last `BurnWarnTime` seconds before burnt — computed from progress and the row's `BurnTime`/`BurnWarnTime`):
     steam turns to grey smoke, sizzle gets harsher and a crackle layer joins.
   - *Transitions* (one-shot, detected by each machine comparing the cooking item's sub-alias with last tick — conventions § 9):
     raw → cooked: a bright "치익" cue + a small steam burst; → burnt: a dull "퍽" cue + a black smoke puff.
2. **Ingredient browning** (per `ACL9Item`): while on heat, the item's mesh darkens gradually — toward a cooked brown during Cooking,
   toward char black during Burning — via a generic overlay/tint that works on every cookable mesh (no per-material edits). Resets when
   the row changes (the new mesh already looks cooked/burnt) and when it leaves the heat.
3. **Burnt items smoke**: an item whose row is burnt (`bWorthlessIfBurnt` row reached through `BurnResult`, i.e. sub-alias `burnt`) emits a
   thin smoke wisp while it is free in the world (on the plate, floor, counter); none while held/in a quickslot/in a cart. Cheap — no new tick
   class (items already tick).
4. **Egg meshes**: `SM_Egg_Cooked_A` (fried egg, sunny-side up — white disc with a yolk dome) and `SM_Egg_Burnt_A` (same shape, charred,
   crisped edges), wired to item rows `egg/cooked` and `egg/burnt`. Collision fits a flat disc (it must rest on the plate and be pickable).
5. **Sounds** (synthesized like the other SFX; all machines, attenuated like `KitchenAmbience` — the sizzle/crackle are loops on the station,
   cues are one-shots at the item; none on a dedicated server; monsters do not hear):
   `CookSizzle` (loop), `CookCrackle` (loop, burn-warning layer), `CookDone` (one-shot), `CookBurnt` (one-shot).
6. **Docs**: `systems/cooking.md` (states table, what drives what), `systems/sound.md` (4 rows), `asset/map/kitchen.md` + item asset cards
   (egg cooked/burnt, VFX cards/materials), `testing/pie-scenarios.md` (new section), `decisions.md`.

## Out-of-scope
- Placing stations in stage levels (owner: later). Verification in `Lvl_Flat`.
- Burner on/off, flipping, recipes, changing cook timings, the one-item rule, the gauge, replication, cooking judgment.
- Niagara systems — role agents cannot drive the editor's Niagara tools; VFX are mesh cards + animated materials made by the art import
  script (the same route as the pilot flame). Swap to Niagara later if the cards look poor.
- New meat meshes (raw/cooked/burnt already exist).

## Design defaults (lead, from the owner's accepted list — revisit only if the owner objects)
- One presentation component/method on the station drives flame/glow/haze/steam/smoke/loops; one-shots use the replicated alias edge.
- Material scalars are driven through `CustomPrimitiveData` (or dynamic material instances) on the existing/new components — no new
  replicated state, no new RPC, no new ticking class (kitchen and item already tick).
- VFX cards (`SM_Kitchen_HeatHaze_A`, `SM_Cook_Steam_A` reused for smoke/puff by material/tint) are separate components on the station,
  positioned over the cooking item each tick (the item can sit anywhere on the 300 cm plate). Translucent, unlit, no shadow, no collision.
- Heat haze uses a refraction/distortion translucent material; if refraction is unavailable in the current rendering path it falls back to a
  faint shimmering tint (report which one shipped).
- Browning uses `UMeshComponent::SetOverlayMaterial` with one shared translucent overlay material whose colour/opacity come from
  custom primitive data (draft: cooked tint opacity 0 → 0.45 over Cooking; char tint 0 → 0.6 over Burning).

## Acceptance Criteria
- [ ] B1 In `Lvl_Flat`, placing raw meat on a station: within 1 s the flames flare, the plate glows, haze shimmers and steam rises above the
      meat; the meat browns gradually; sizzle loop audible near the station. Removing it returns the station to Idle within 1 s.
- [ ] B2 At the cooked transition: "치익" cue + steam burst once (not every tick); the meat swaps to the cooked mesh.
- [ ] B3 From `BurnWarnTime` before burnt: grey smoke replaces steam, crackle layer audible; at burnt: "퍽" cue + black puff once; burnt mesh.
- [ ] B4 A burnt item left on the plate or floor keeps a thin smoke wisp; picking it up stops it; dropping it resumes it.
- [ ] B5 A raw egg on the plate becomes a fried egg (`SM_Egg_Cooked_A`) and then a charred fried egg (`SM_Egg_Burnt_A`); both rest flat on
      the plate and can be picked up.
- [ ] B6 Idle stations look and sound exactly as after `kitchen-identity` (pilot flames, ambience) — no stray steam/haze.
- [ ] B7 Two stations cooking at once each show their own effects; concurrency keeps the sizzle from stacking louder than one station
      per station cluster (same rule as ambience).
- [ ] B8 2-player (listen + client): the client sees/hears the same states and one-shots; no new RPC or replicated property.
- [ ] B9 Cooking judgment unchanged: `CookTime`/`BurnTime`, one item per station, HeatZone and gauge as before (kitchen-identity A2/A3).
- [ ] B10 No light component added; flames/glow/haze excluded from Lumen GI; no new ticking class; no 30 ms hitch when cooking starts
      (assets warmed via the existing SoundTable warm-up / soft refs warmed like other rows).
- [ ] B11 Build succeeds; `CL9.Map` result unchanged; docs updated (In-scope 6).
- [ ] B12 Human: a cooking video (same method as `kitchen_cooking.mp4`) shows the states clearly; owner confirms it reads as cooking.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Items/CL9Kitchen.h/.cpp` | Station presentation states, VFX components, sizzle/crackle loops, transition cues |
| `Source/CL9/Items/CL9Item.h/.cpp` | Browning overlay, burnt-item smoke, cook getters (`GetCookPhase`, `GetCookProgress`, `IsNextBurnt`) |
| `Source/CL9/Data/CL9SoundTable.h/.cpp` | Row names `CookSizzle`, `CookCrackle`, `CookDone`, `CookBurnt` |
| `Source/CL9/Data/CL9ItemDef.h` | `CookTime`/`BurnTime`/`BurnWarnTime`, `CookResult`/`BurnResult` (read only) |
| `Content/Items/BP_CL9Kitchen`, `Content/Items/BP_CL9Item` | Component mesh/material assignment (lead applies) |
| `Content/Data/ItemTable` | `egg/cooked`, `egg/burnt` Mesh column (lead applies) |
| `Content/Data/SoundTable` | 4 new rows (art import script) |
| `Tools/Blender/*`, `Tools/Unreal/*`, `ArtSource/item/egg/*`, `ArtSource/map/kitchen/*`, `ArtSource/audio/sfx/*` | Art sources & import |
| `docs/systems/cooking.md`, `docs/systems/sound.md`, `docs/asset/*`, `docs/testing/pie-scenarios.md` | Docs |

## Must-Preserve
- Cooking judgment, HeatZone, `GetTopLocation()`, gauge, replication (kitchen-identity Must-Preserve).
- Presentation from replicated state only (conventions § 9): no RPC, no Multicast, no new replicated property; dedicated server plays/spawns nothing.
- Content via DataAssets/BP (no asset paths in C++). Darkness balance: no light components; emissive excluded from GI.
- The uncommitted `kitchen-identity` / `round-truck-core` work in the tree is built upon, not reverted.

## Execution Notes
- Harness: `/cl9 task` — roles art (egg meshes, VFX cards + materials, overlay material, 4 sounds, import script), gameplay (station/item
  presentation C++, sound row names, cooking.md/sound.md), docs (pie-scenarios section). Lead: imports, BP/table assignment, build, PIE, video.
- Recommended model: the strongest current Claude model (Opus 5.5 / Fable 5.1) for art (VFX look) and gameplay (state derivation, edges).
  Sonnet acceptable for docs. This document cannot enforce the model — the session's `/model` decides; if below the recommendation, tell the owner.

## Results (2026-10-09, run `cooking-progress-feedback`)
One build (first try). `Lvl_Flat` PIE by script oracle — egg on K1 and meat on K2 at once, sampled every 0.5 s; then listen server + 1 client.
- [x] B1 Cooking: flame flare 1 within 0.5 s, plate glow/haze/steam visible, steam 0.33→1, sizzle on, ingredient tint rising (0→0.34 by 9 s); removing returns to Idle.
- [x] B2 At 12 s: `CookDone` once per item (+2 for egg+meat), white puff once, egg → `SM_Egg_Cooked_A`.
- [x] B3 30.5 s burn warning: smoke 0.3→1, crackle on; 36 s: `CookBurnt` +2, black puff, Idle, loops off, all hidden by 39 s.
- [x] B4 Burnt egg: `is_burnt_row` and burnt smoke on while free (cart cargo also smokes — lead correction to owner's wording).
- [x] B5 Fried egg body: root `Sphere` → `BodyMesh` at 12 s, z 111 → 90 (= plate top), speed 0; mass 2.15 kg (draft). Close-up in the video reads as a fried egg.
- [x] B6 Idle unchanged (flame 0, all hidden, ambience only, 0 lights).
- [x] B7 K1 and K2 cooked simultaneously with their own effects and cues.
- [x] B8 2-player: client (no authority) look states match the server tick for tick; client egg `BodyMesh` at z 90; `CookBurnt` +1 per world.
- [x] B9 Timings unchanged (cooked 12 s, burnt 36 s).
- [x] B10 0 lights; effects excluded from GI. Hitch measurement not done (automated PIE).
- [x] B11 Build OK; docs updated. (`CL9.Map` not re-run — no map code touched.)
- [ ] B12 Owner review of `Saved/Video/kitchen_cooking_v2.mp4`.
- Found / open: (1) **G at the stove drops the item ~20 cm in front of the plate — eggs fall to the floor and crack** (pre-existing hand-drop behaviour; the video moved the egg onto the plate by script). (2) The burnt fried egg renders golden rather than charred in-game. (3) A raw egg on heat looks very dark at night (overlay + night light).

## Follow-up (2026-10-09, owner decisions after the v2 video) — run `stove-place-fix`
- **F1 G at the stove places, it does not drop.** While "[G] 올려서 익히기" is valid (cookable item held, facing a station within reach), G puts the item onto the
  plate at the nearest spot on the plate (inset so the whole item rests inside the plate footprint), resting on the plate surface with zero velocity — no fall, no crack.
  The host decides the station itself (no new RPC parameter trusted from the client beyond today's drop request); everywhere else G drops exactly as today.
- **F2 The burnt fried egg reads as charred** in the game lighting (it renders golden today — still `Saved/Video/stills/burnt_egg_closeup_golden.jpg`).
- **F3 A raw egg on heat stays readable at night** (today it turns very dark — `Saved/Video/stills/raw_egg_on_heat_dark.jpg`); browning should read as browning, not as a dark blob.
- Acceptance: F1 — from the front of K1 (centre, both ends, 40–110 cm from the front edge) G puts a raw egg on the plate uncracked and it cooks; G away from a
  station still drops from the hand; throws unchanged. F2/F3 — human check on stills/video. Build + PIE + video (same method).
- **Follow-up results (2026-10-09, run `stove-place-fix`, 1 build):** F1 — real `Server_Drop` at K1 front centre/both ends × 40/70/110 cm: all 9 placed on the plate
  (21 cm inside the front edge, z 111), uncracked, cooking at once; second egg moved 45 cm along the plate, no overlap; client pawn the same in listen 2-player;
  facing away still hand-drops. F2 — burnt fried egg reads as charcoal on the plate (vertex-colour sRGB decode + darker palette); **when aimed, the focus outline
  (`M_CL9_Highlight`) still paints it gold — open, out of scope.** F3 — raw egg on heat now reads light, not a dark blob (`CookedFloor` 0.15, draft).
  Video `Saved/Video/kitchen_cooking_v3.mp4` uses the real G (no scripted move).

