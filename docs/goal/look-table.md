# goal: look-table

> **Status**: implemented and verified (2026-10-16 01:45, autogoal LIGHT). Not committed (shared files carry other sessions' work).
> Follows [look-rework.md](look-rework.md): every value that decides how the game *looks* moves into one table, and a live mode
> re-applies the table while PIE runs.

## One-line Goal

One asset, `/Game/Data/LookTable` (`UCL9LookDef`), holds every look value — screen filter, lighting profiles, lamp kinds, HUD
brightness and structure-surface detail — and editing it during PIE changes the running game immediately.

## Background / Motivation

User, 2026-10-16: "이 모든거 그냥 하나로 퉁쳐서 테이블로 관리하고싶은데" + "테이블에서 값바꾸면 실시간으로 적용되는 모드도". After
look-rework the values are spread over six places: `CL9Rules` Look* columns, three `FCL9LightingPhase` per site row (four sites carry
identical copies), `LampSurviveShare` per site, seven light columns repeated on 36 work-lamp placements in `ModuleTable`, four light
columns on two `ItemTable` rows, yard-door lamp columns in `CL9Rules`, C++ quickslot defaults and 16 material instances. The user
chose "stage 2 — everything" (2026-10-16).

## In-scope

- `UCL9LookDef` (single-row def asset, auto-registered like `CL9Rules`) with five groups:
  1. **Screen** — max resolution, AA, posterize bands/mix/material, grain.
  2. **Lighting profiles** — named sets of Lobby/Night/Blackout `FCL9LightingPhase` + `LampSurviveShare`. Site rows keep only a
     `LightingProfile` name.
  3. **Lamp kinds** — named rows: intensity (cd), radius, colour, scattering, cone half-angle (0 = point), flicker amplitude/speed.
     Module prop placements keep `LightOffset` and gain `LampKind`; item rows gain `LampKind`; the yard-door lamp reads kind `YardDoor`.
  4. **HUD** — quickslot active/inactive colours.
  5. **Surface** — global `bSurfaceDetail` + families (wood/concrete/metal): textures, tiling, normal strength, roughness min/max and
     the material instances that belong to the family.
- **Live mode (editor only)**: `UCL9LookDef::PostEditChangeProperty` and a `CallInEditor` `ApplyNow` broadcast `OnLookChanged`;
  listeners re-apply: retro cvars (local PlayerController), facility lamps and survivor groups (site map), item lights, yard-door lamps,
  quickslot widgets; the def itself pushes surface values into its material instances. Lighting phases and the post filter already read
  the table every frame.
- Migration script that builds the asset from today's values, points sites/props/items at names, then the old columns are deleted.
- Tools and docs that wrote the old columns (`outdoor_look.py`, `setup_stage.py`, `setup_henhouse_review_stage.py`, the look-rework
  tune/apply scripts) are rewritten against the new table.

## Out-of-scope

- `LampDensity` stays on the site row: it is how many lamp bodies of a generated building are lit, tuned with the module mix per site
  (henhouse 0.5, farm 0.35, pigfarm 0.2); putting it in a profile would need one profile per site with duplicated phases.
- `YardLampMesh`/`YardLampHeight` (placement) and item gameplay light columns (`bLightLostInPocket`, `CarrierSeenRadius`) stay where
  they are.
- Changing a lamp kind between point and spot while PIE runs: the component class is fixed at build time — it applies next round.
  Every other kind value is live.
- Shipping-build live editing; replicating anything.

## Results (2026-10-16)

- `UCL9LookDef` + registry `GetLook()`; two builds (new columns, then old columns deleted); log shows no look-table error at load.
- Migration (`migrate_look_table.py`): profiles `default` (henhouse, farm, pigfarm, henhouse_review) and `flat`; lamp kinds WorkLamp,
  Campfire, Flashlight, Headlamp, YardDoor; 40 lit prop placements renamed to kinds; surface families wood 11 / concrete 2 / metal 3.
- Live mode, PIE `Lvl_Henhouse`, values read back from the running components after each `tune_look.py` call (`Saved/Look/live_probe.json`,
  stills `Saved/Look/live_*.png`):

  | Change | Before → after (in the running game) |
  |---|---|
  | (a) profile night `FogDensity` 0.5 → 0.15 | fog component 0.5 → 0.153 (lerping) |
  | (b) WorkLamp intensity 10 → 40, colour, cone 60 → 35 | lamp 10 → 40, colour (255,234,203) → (255,149,124), cone 60 → 35 |
  | (c) `LampSurviveShare` 0.3 → 1.0 | visible work lamps 4 → 14 of 14 |
  | (d) `MaxResolution` 520 → 240, posterize 6 × 0.9 | cvar 520 → 240; frame visibly coarser |
  | (e) quickslot colours | boxes turned red/green on the next tick |
  | (f) Headlamp intensity 16000 → 3000 | the two headlamps 3000, the flashlight stays 16000 |
  | (g) surface wood normal 0.5 → 1.5 | `MI_CL9_Struct_Wood.NormalStrength` 0.5 → 1.5 |

  Values were restored and saved afterwards.
- Automation `CL9.Map*`, `CL9.Night*`, `CL9.Feel*`: 20 passed, `CL9.Map.Districts` failed (known, map session's).
  `CL9.Replication.Registration` was not run (engine assertion from other sessions' uncommitted replication work — separate card).

## Acceptance Criteria

- [x] `UCL9LookDef` exists, is loaded by the registry (`GetLook()`), validated, and `/Game/Data/LookTable` holds today's values
      (no visual change before vs after migration in the switch-room still).
- [x] No reader uses the removed columns; the removed columns are gone from `CL9SiteDef`, `FCL9PropPlacement`, `CL9ItemDef`,
      `CL9RulesDef`, `CL9QuickSlotWidget`.
- [x] PIE live: changing (a) a night profile value, (b) `WorkLamp` intensity/colour/cone, (c) `LampSurviveShare`, (d) screen max
      resolution/posterize, (e) quickslot colours, (f) headlamp intensity, (g) a surface family's normal strength — each shows on screen
      without restarting PIE (stills under `Saved/Look/live_*`).
- [x] `CL9.Map.*` automation passes (Districts excepted, known); build clean.
- [x] Docs: new `docs/plan/data/look.md` (column reference), light.md / site.md / module.md / rules.md / ui.md / item docs point to it;
      decisions.md entry.

## Related Files / Modules

| File | Role |
|------|------|
| `Source/CL9/Data/CL9LookDef.{h,cpp}` (new) | The table, validation, live broadcast, surface push |
| `Source/CL9/Data/CL9DefRegistry.{h,cpp}` | `GetLook()` |
| `Source/CL9/Data/CL9SiteDef.{h,cpp}` | `LightingProfile` name; phases/`LampSurviveShare` removed (`FCL9LightingPhase` moves to LookDef header) |
| `Source/CL9/Data/CL9ModuleDef.h`, `CL9ItemDef.h`, `CL9RulesDef.h` | `LampKind` names; old light/look columns removed |
| `Source/CL9/World/CL9NightLighting.cpp`, `CL9SiteMap.*`, `CL9YardTension.*` | Readers + live re-apply |
| `Source/CL9/Items/CL9LightItem.*`, `CL9FlashItem.cpp` | Item light from kind + live re-apply |
| `Source/CL9/GameMode/CL9GameUserSettings.cpp`, `CL9LookModifier.cpp`, `CL9PlayerController.cpp` | Screen values from LookDef, live cvars |
| `Source/CL9/UI/CL9QuickSlotWidget.*` | Colours from LookDef, live |
| `Tools/Unreal/migrate_look_table.py` (new) | Builds the asset from current data |

## Must-Preserve

- Same picture after migration (values copied, not retuned).
- Static tables stay local, nothing replicates; survivor choice stays seeded (same lamps on every machine).
- Lamp state touched only on change (performance.md tick rule); the live handlers run only on an edit, never per frame.
- No asset paths in C++; Korean comments; tables are data files edited through editor Python with explicit saves.

## Execution Notes

- Recommended model: Claude Fable 5.1 for the migration ordering and live-mode wiring; Sonnet-class acceptable for the doc moves.
  The session runs Opus 5.5 (user's `/model` choice) — acceptable for this refactor since every value is copied, not judged.
- This document cannot enforce the model — the executing session's `/model` setting decides.
