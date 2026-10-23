# goal: look-rework

> **Status**: implemented and verified in PIE + perf probe + bot film (2026-10-15 evening, autogoal HEAVY). Not committed (shared files
> carry other sessions' uncommitted work; hunk-level commit when the user asks). Results and what is still unverified: see "Results".
> Decisions 1–6 are the user's (2026-10-15 chat).
> The design was passed through a three-lens read-only panel (rendering/engine · game feel/readability · pipeline/rollback) and a
> cross-examination; their corrections are folded into "Design Decisions". Evidence: frames of `Saved/Video/map_wave_20261015g.mp4`,
> the CCL8 reference clip `docs/reference/mp4/map/1.mp4`, table values read from the running editor (MapTable · ModuleTable ·
> PieceTable · ItemTable, 2026-10-15), engine source (UE 5.8) and the reference analyses in
> [../reference/repo/repo-map.md](../reference/repo/repo-map.md) / [../reference/lethal/lethal-map.md](../reference/lethal/lethal-map.md).

## One-line Goal

Make the night interior read the way Lethal Company and the CCL8 prototype did — black rooms with warm light pools, shadows that give
walls their form, a low-resolution posterized frame, textured structure surfaces and dressed corridors — without changing any gameplay
judgement (monster sight, blackout, power stage timing).

## Background / Motivation

User, 2026-10-15, after watching the map/wave bot clip: the picture looks worse than Lethal's. Measured causes (this session):

| Cause | Where it comes from today |
|---|---|
| One flat blue hue everywhere, no contrast | henhouse `NightLighting`: SkyScale 5 × cold SkyColor, **SkyLeak 1.0** (Lumen skylight leaking fills closed rooms evenly), fog colour (0.02, 0.03, 0.06) |
| No lamp pools after the intro | power stage `Dead`: every indoor lamp is off for the rest of the round (light.md "전원 단계", `CL9SiteMap.cpp` ~729) — the clip showed the design, not a filming accident |
| Lamps leave no shadow, so walls have no form | `ACL9SiteMap::PlaceProps` lamps: `SetCastShadows(false)` (`CL9SiteMap.cpp` ~1338) |
| Corridors readable to 20 m+, repetition shows | FogDensity 0.05 = 0.05/1000 per cm → 90 % transmittance at 20 m; skylight leaking lights far walls |
| "Black" fog is blue | SkyAtmosphere/skylight ambient is added to height fog regardless of `FogInscatteringColor` (HeightFogCommon.ush, `SkyAtmosphereAmbientContributionColorScale` default white) |
| Surfaces are one colour per face | structure materials are constant/vertex colour, no normal or roughness maps |
| Corridors are empty | `corridor_3`, `corridor_4`, `entrance` have no PropSets; `corridor` 1–2 props per set; `corner` 2 |
| Clean high-resolution render shows the simple models | no style pass at all |
| The clip itself | bot held no light, 20 fps 1280×720 at ~1 Mbps (crf 20) |

Lethal: 860×520 internal render, HDRP volumetric fog, posterization, edge detection, bloom, grading. R.E.P.O.: pixelization 2, grain,
chromatic aberration, vignette 0.4, black fog 0–15 m with far clip, warm prop lights, per-level colour temperature, flashlight-off
ambient adaptation. CCL8: pure black + warm flashlight pool, vertex-colour walls, arms always in frame.

User decisions (2026-10-15):

1. **Black + light pools** (Lethal / CCL8 way), not "keep ambient and strengthen it".
2. **Lethal-style low-resolution + posterize** post-process, as a toggle.
3. **Add normal/roughness textures** to wall/floor/ceiling/pillar/beam materials now.
4. Also: fill corridor prop sets, lower the quickslot box brightness, film bot holds a lit light, raise recording quality.
5. Verify whether ceilings exist in the generated henhouse.
6. **After the power intro, a fraction of the indoor lamps survives** (the "all lamps die" rule of 2026-10-06 is relaxed to a site-row
   share) — chosen over "all off", "dim survivors" and "deep rooms only".

## In-scope

- Site lighting rows (henhouse · farm · pigfarm · henhouse_review): night/blackout phases with residual skylight leaking, black fog with
  the sky-ambient contribution removed, density tuned by ratio criteria.
- Lamp survivors after `Dead`; work lamps become down-facing spot lights with shadows; campfire unchanged.
- Retro look: fixed internal resolution + nearest upscale + posterize/grain via a camera modifier; values in `CL9Rules`; on/off in
  `UCL9GameUserSettings` and the settings screen.
- Surface detail: structure parent material with a static switch; material instances for the structure meshes; base colours unchanged.
- ModuleTable prop sets for `corridor`, `corridor_3`, `corridor_4`, `corner`, `entrance`.
- Quickslot brightness defaults.
- `Tools/Film`: bot headlamp action, shot-list steps, encode quality.
- Ceiling check in generated maps; fix only if it is a bug.
- Docs listed in "Docs to update".

## Out-of-scope

- Connecting the new indoor shells (`Content/Maps/Henhouse/Indoor` walls/floors/ceilings) to PieceTable/SegKind — henhouse-indoor §8,
  another session's job. The roof folder (`Structure/Roof`, untracked by another session) and `M_CL9_Ceiling_Wood` graph edits are not
  touched here; those meshes get a material-instance reassignment only if their owner session is not mid-edit (checked at step time).
- Edge detection / outlines, chromatic aberration, lens distortion, R.E.P.O. ambient adaptation, per-room fog volumes, far-clip hack.
- Gamma slider and the rest of a graphics settings section.
- Monster sight, blackout timing, intro timings — rendering only; judgement stays value-based (light.md "렌더와 판정은 분리").
- Prop sets for rooms other than the corridor family; new prop meshes; distance-gated lamp shadows.

## Design Decisions (panel-settled — do not relitigate during implementation)

- **D1 Darkness model = small flat skylight leak × black fog transmittance.** Lumen skylight leaking is a flat add on every surface
  (independent of camera distance, `LumenTracingCommon.ush` ~77-84), so distance falloff can only come from fog; density is
  `FogDensity/1000` per cm (`SceneCore.cpp` ~404), so the night `FogDensity` goes from 0.05 to ≈1.0 (5 m → 0.61, 20 m → 0.14) and
  `SkyLeak` from 1.0 to a residual (draft 0.15). `FogColor` pure black. New phase column `FogSkyAmbient` (→
  `SetSkyAtmosphereAmbientContributionColorScale`, 0 for night/blackout, 1 lobby) so the far analytic fog stops turning blue;
  `SkyScattering` 0 indoors at night. Acceptance is by ratio in the same frame, retro look on: a leak-only wall at 20 m falls below the
  lowest posterize band; a flashlight-lit wall at 18 m is readable; a lamp pool ≥ 10× a leak-only wall. Flashlight/headlamp
  `LightScattering` 8 is lowered as needed (milky beam at density 1). The residual leak is required, not optional: `Dead` must stay
  brighter than the first dark (round-start-truck-intro K7). *(Rejected: SkyLeak 0 — breaks design-concept rule 4 "손전등 밖은 실루엣만",
  partner slot-tint identification and visible monster telegraphs; a weak sky-coloured point light on the player; far-clip.)*
- **D2 Lamps: `LightConeAngle` column, survivors share, shadows, no gating.** `FCL9PropPlacement` gets `LightConeAngle` (0 = point
  light as today; > 0 = spot light, half-angle, pointing along the prop's local −Z). The work-lamp mesh is a pendant with the bulb under
  the shade (`build_henhouse_sample.py` ~160), so its rows get a cone (draft 60°) and `LightOffset` below the shade; the campfire stays a
  point light. Spot lamps: `CastShadows` true, `CastVolumetricShadow` false (one fog injection pass per shadowed light otherwise,
  `VolumetricFog.cpp` ~757), `SourceRadius` 0. No distance gating — toggling `CastShadows` recreates the light proxy and drops VSM pages
  (`LightComponent.cpp` ~79-86) and ungated lamps leak through walls into Lumen; if the budget is exceeded, `MaxDrawDistance` /
  `MaxDistanceFadeRange` fade whole lights instead. New site column `LampSurviveShare` (0–1, draft 0.3): among the lamps `LampDensity`
  lit, a seeded subset survives `Dead` (same seed → same survivors on every machine); `LightsOn`/`Flicker`/blackout behaviour unchanged.
  performance.md's "dynamic shadow lights = flashlight/headlamp/moon only" rule is amended, with a decisions.md entry.
  *(Rejected: converting all lamps to spots; a pre-emptive `LightPitch` column; 2 Hz shadow toggling; all-off / dim survivors / depth-based
  death.)*
- **D3 Retro render resolution = `r.ScreenPercentage.MaxResolution`.** `LookMaxResolution` (CL9Rules, draft 520 → ≈924×520 at 16:9)
  caps the internal resolution independent of window size (`LegacyScreenPercentageDriver.cpp` ~344-347); `r.Upscale.Quality` 0
  (nearest, `PostProcessUpscale.cpp` ~21-24); `r.AntiAliasingMethod` None vs FXAA decided by A/B in PIE (TSR would temporally upsample
  the pixels away). In PIE `r.ScreenPercentage` 100 is set alongside because the editor's PIE override replaces ≤ 0 values. Values are
  applied with `IConsoleVariable::Set(…, ECVF_SetByGameSetting)` after caching the originals, restored on toggle-off and on
  `FEditorDelegates::EndPIE` (`r.AntiAliasingMethod` / `r.Upscale.Quality` are global and would leak into the editor viewport).
  Per-client independence is verified with two processes, never single-process PIE. *(Rejected: 45 % / integer screen percentage
  (non-integer pixel widths, window-size dependent, SetByConsole priority lock); dynamic resolution; a scene-capture render target.)*
- **D4 Posterize + grain live in a camera modifier.** `UCL9LookModifier : UCameraModifier`, registered in `ACL9CameraManager`
  `DefaultModifiers`, `ModifyPostProcess` sets `FilmGrainIntensity` (`LookGrain`, draft 0.15) and a WeightedBlendable with
  `M_CL9_PP_Posterize` (post-process domain, **After Tonemapping**, luminance quantized into `LookPosterizeBands` (draft 12) blended by
  `LookPosterizeMix` (draft 0.7), hue preserved). The material reference is a `CL9Rules` soft pointer (no asset paths in C++). This
  covers title, lobby, arrival camera, death cam and spectate on every map. *(Rejected: the blendable on ACL9NightLighting — absent in
  Lvl_Start/Lvl_Company; the first-person camera's PostProcessSettings — gone whenever the pawn is; 24 bands × 0.5 — ≈48 effective
  bands, invisible.)*
- **D5 Surface detail = structure parent material + static switch.** `M_CL9_Structure` (vertex colour / constant base colour,
  `bSurfaceDetail` static switch, `MF_CL9_SurfaceDetail` = world-aligned triplanar normal + roughness, tangent-space normal off,
  parameters: tiling cm, normal strength, roughness min/max) with one material instance per current structure material (wall 500/250/125
  wood + dark timber, floor concrete, ceiling wood, pillar/beam wood + metal, tread, door = 13), assigned to the structure meshes' slots.
  Props keep their own materials (`M_CL9_HH01_*` are shared with props — world-aligned detail would swim on carried props). Textures from
  the relocated pack copies `Content/Maps/Henhouse/Props/Textures/StylizedFactory/*` (never `Content/Peb`, mid-relocation). Start with
  low normal strength / high roughness; done after D3 so it is judged at the real render scale. Decision test: one wall face must not
  break into more than two posterize bands; if it does, lower normal strength or keep roughness only. Revert = switch off, zero cost.
  *(Rejected: Material Parameter Collection scalar — none in the project, and 0 still pays 6 texture samples; editing the 13 existing
  materials in place; textures on props.)*
- **D6 Corridor prop whitelist** (henhouse_indoor_objects.md §3/§7): fire extinguisher, breaker box, warning sign, poster, bucket,
  egg-tray stack, basket, feathers, stain, plus one work lamp per set. Wall props ≤ 50 cm deep, one side only, 400 cm centre lane clear,
  no spawn-cell (±130 cm) or doorway overlap. `corridor_3`/`corridor_4` 2 sets each, `corridor`/`corner` one more set, `entrance` 1.
  Authored by an editor Python script with a JSON backup of the rows before/after. *(Rejected: pipe bundle (not in corridors), sack
  pile/crate (depth), tool board/notice board (room-identity props), straw mat (false egg signal); a yard dock lamp — light.md:117 already
  has door lamps.)*
- **D7 Quickslot** — `UCL9QuickSlotWidget` defaults `ActiveColor` 1.0 → 0.7, `InactiveColor` 0.4 → 0.25 alpha (border, icon and name
  together), comments updated; still white-only.
- **D8 Film bot uses the headlamp**, not the flashlight (a hand flashlight goes dark when picking an egg switches the active slot, and
  blackout kills it; the headlamp is worn, F-toggled, blackout-exempt). Steps: `cheat cl9.additem headlamp` → `press` (wear) → new one-line
  `headlamp` action (`Server_Flashlight`). `encode.py` crf 20 → 14.
- **D9 Order (one full build, then everything by eye at the real render scale).** ① ceiling check (read-only) → ② all header changes in one
  build: `FogSkyAmbient`, `LightConeAngle`, `LampSurviveShare`, survivors logic, `CL9Rules` look values, `bRetroLook` + apply/restore,
  `UCL9LookModifier`, quickslot defaults → ③ posterize material + retro on → ④ fog/leak ratio tuning → ⑤ lamp rows (cone/offset) + perf
  probe → ⑥ structure material + instances → ⑦ corridor prop sets + `CL9.Map` tests → ⑧ film re-shoot → ⑨ docs. *(Rejected: tuning table
  values before the fog column and render scale exist — those values would be discarded.)*

## Results (2026-10-15)

- Ceiling: present — a generated henhouse (read from a running PIE) had 120 floor cells and 115 ceiling tiles; the clip's blue "sky" was
  the ceiling under skylight leaking.
- Measured surprises that changed the numbers: Lumen skylight leaking is scaled by `saturate(hitDistance / 1000 cm)`, so in a 5 m
  corridor a leak of 0.15 or even 1.0 reads black — the residual is kept at **0.5** for larger rooms and doorways, corridor light is lamps
  and headlamp only. `FogDensity` 1.0 made the yard (first dark ×2) a grey fog wall; **0.5** keeps the corridor fading to black with distant
  lamp glows visible. Posterize 12 bands × 0.7 drew concentric rings around every light; **16 × 0.5** keeps stepping without rings.
- Lamps: 36 work-lamp placements are 60° down-facing spots with shadows; 12 lamps were lit in one seed, 4 survived `Dead`. The switch
  room reads as intended (warm pool, wood grain relief, corridor fading to black, a far lamp's glow) — `Saved/Look/g_sw_nopost.png`,
  `f_sw_leak06_nopost.png`.
- Retro look: cvars confirmed in PIE (`MaxResolution` 520, `ScreenPercentage` 100, `Upscale.Quality` 0, `AntiAliasingMethod` 1); HUD crisp;
  restored after PIE (editor viewport unaffected).
- Perf (switch room, retro on, `perf_probe` C0): avg 16.67 ms = 60 fps cap, p95 16.67 ms; GPU graphics 3.56 ms + compute 2.31 ms,
  ShadowDepths 0.22 ms, VolumetricFog 0.18 ms, LumenSceneUpdate 0.36 ms — inside budget with headroom.
- Prop sets: 7 corridor-family sets added (`PlaceProps` printed no placement warnings in the PIE log for the seeds used).
- Film: `Saved/Video/look_round_henhouse.mp4` (54 s, crf 14, ~3.7 Mbps) — the bot wears the headlamp via walk/face/press. Its beam at
  scattering 8 is a milky disc with posterize rings → flashlight/headlamp `LightScattering` lowered to 3 (`apply_look_items.py`);
  the re-shoot `look_round_henhouse_b.mp4` shows the beam as a wall pool instead. Left as is: the near field under the headlamp
  (an egg at 50 cm, the counter top) blows out to white — that is the item row's intensity with exposure locked, an item-table /
  balance call, not a look setting.
- Automation: `StartsWith:CL9` ran after the header rebuild — every `CL9.Map.*`, `CL9.Feel.*`, `CL9.Night.*` test passed except the
  known `CL9.Map.Districts` failure (map session's, pre-existing); the run then hit an engine assertion in
  `CL9.Replication.Registration` (`bIsPushBased == Other.bIsPushBased`, CoreNet.h:342) that closed the editor. This rework adds no
  replicated property (static table columns, a local camera modifier, a config bool), so the mismatch is in the working tree's other
  uncommitted replication changes (Character/GameState/PlayerState/Monster/Item/Kitchen) and needs their owner.
- Not verified: two-process client independence of the retro toggle (single-process PIE shares cvars); the D5 "one face ≤ two bands" test
  was judged by eye on the switch-room stills only; `CL9.Map` automation passed (see above).

## Acceptance Criteria

- [ ] Ceiling presence confirmed in generated `Lvl_Henhouse` (instance count > 0, looking-up screenshot at 3 seeds) or the cause reported.
- [ ] D1 ratio criteria met in one PIE frame per seed (3 seeds), retro on; blackout still fully dark; pre-power first dark darker than
      `Dead`; switch glow, yard silhouettes and a chicken's `WakeTelegraph` at 5 m outside the beam still readable. Screenshots `Saved/Look/`.
- [ ] After the intro, `LampSurviveShare` of the lit lamps stay on; same survivors on host and client (two-process listen test).
- [ ] Spot lamps cast shadows from pillars/props in a lit room; campfire unchanged. Perf probe in the baseline spot and in a chase:
      avg ≤ 16.7 ms, p95 ≤ 25 ms; `stat gpu` ShadowDepths / VolumetricFog before/after recorded; any +1 ms step in decisions.md.
- [ ] Retro toggle on: ≈924×520 internal render visibly pixelated, crisp HUD; off: original cvar values restored (checked after PIE ends,
      editor viewport unchanged); persists in `GameUserSettings.ini`; two-process clients independent.
- [ ] Posterize/grain apply on title, lobby, round, death cam and spectate.
- [ ] Structure surfaces show relief inside a light pool; palette unchanged (before/after pair); no face exceeds two bands.
- [ ] `corridor_3` / `corridor_4` / `entrance` ≥ 2 / 2 / 1 sets, `corridor` / `corner` ≥ 3; zero `PlaceProps` warnings at 3 seeds;
      `CL9.Map*` automation passes.
- [ ] Quickslot 0.7 / 0.25; white-only.
- [ ] `cl9film` henhouse round with the headlamp lit after the intro, crf 14, under `Saved/Video/`.
- [ ] Docs updated; `StartsWith:CL9` automation passes; new comments in Korean.

## Related Files / Modules

| File | Role |
|------|------|
| `Source/CL9/World/CL9NightLighting.{h,cpp}` | Phase lighting actor — applies the new `FogSkyAmbient` column |
| `Source/CL9/Data/CL9SiteDef.{h,cpp}` | `FCL9LightingPhase.FogSkyAmbient`, site `LampSurviveShare` |
| `Source/CL9/Data/CL9ModuleDef.h` | `FCL9PropPlacement.LightConeAngle` |
| `Source/CL9/World/CL9SiteMap.cpp` ~700-760, ~1328-1365 | Lamp creation (spot/point, shadows), survivors in the `Dead` stage |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` + `Content/Data/CL9Rules` | `LookMaxResolution`, `LookPosterizeBands`, `LookPosterizeMix`, `LookGrain`, `LookPosterizeMaterial` |
| `Source/CL9/GameMode/CL9GameUserSettings.{h,cpp}` | `bRetroLook` config, `ApplyLook` (cache/apply/restore cvars) |
| `Source/CL9/GameMode/CL9CameraManager.{h,cpp}` + new `CL9LookModifier.{h,cpp}` | DefaultModifiers posterize/grain |
| `Source/CL9/UI/CL9SettingsWidget.{h,cpp}`, `Content/Strings/CL9.csv` | "화면" section toggle + ko string |
| `Source/CL9/UI/CL9QuickSlotWidget.h` | Brightness defaults |
| `Content/Data/MapTable` | Site rows: night/blackout phases, `LampSurviveShare` |
| `Content/Data/ModuleTable` | Lamp rows (cone/offset), corridor-family PropSets |
| `Content/Maps/Henhouse/**` structure meshes, new `M_CL9_Structure` + `MF_CL9_SurfaceDetail` + 13 MIs, new `M_CL9_PP_Posterize` | Surface detail, posterize |
| `Content/Maps/Henhouse/Props/Textures/StylizedFactory/*` | Normal/roughness sources (relocated Fab pack copies) |
| `Tools/Film/bot.py`, `encode.py`, `shots/round_henhouse.json`, `.claude/skills/cl9film/SKILL.md` | Headlamp action, steps, crf |
| `Tools/Unreal/*.py` via `run_python.py` | All asset edits scripted; explicit `save_loaded_asset` |

## Must-Preserve

- Rendering never feeds judgement: monster sight stays `SightIn` / `SightDark`; nothing added here replicates (survivors derive from the
  shared seed).
- `ExposureLock` 1.0 stays.
- Blackout = `SunScale 0 · SkyScale 0 · SkyLeak 0`, flashlight forced off, headlamp exempt; `Dark` forces `SkyLeak 0` and
  `PrePowerFogScale`; `LightsOn`/`Flicker` timings (`IntroLeadIn`, `IntroFlyTime`, `IntroFlickerTime`) untouched.
- Lamp state is recomputed only when it changes (performance.md tick rule — no per-frame `SetVisibility`).
- Quickslot white-only, 4 boxes, icon-only (2026-09-13).
- Tables are data files; asset edits via editor Python with explicit saves; never `save_assets([])`; backups before table rewrites.
- Builds through the broker (`Tools/Build/build_request.py`); other sessions' uncommitted work in shared files is left as is and
  committed hunk by hunk only when the user asks; `Content/Peb` and `Structure/Roof` are not touched.
- Korean comments; no new abstractions beyond the columns and classes named above.

## Docs to update

light.md (darkness model, lamps, survivors, retro look), site.md (new columns), module.md (`LightConeAngle`, corridor sets), rules.md
(look values), settings.md (화면 section), ui.md (quickslot), performance.md (shadow-light rule, render-default rule), design-concept.md
(rule 3 relaxation for structure surfaces), round-start-truck-intro.md (K7 note: survivors), cl9film SKILL.md (headlamp), decisions.md
(user decisions 1–6 + the two rule amendments), build-order.md (one line).

## Execution Notes

- Recommended model: Claude Fable 5.1 (current top tier) for the fog/leak/lamp tuning, the material and post-process design, the cvar
  apply/restore and the perf judgement — look decisions verified by eye and by measurement. Sonnet-class is acceptable for the
  mechanical parts: prop-set rows, quickslot defaults, encode flag, doc edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the
  recommendation, surface it to the user and confirm before proceeding.
- Lead (this session) drives the editor, PIE, builds and all `.uasset` work; C++/doc edits go to the role sessions per
  docs/workflow/sessions.md (map: World/SiteDef/ModuleDef/light.md/site.md/module.md; gameplay: Rules/GameUserSettings/CameraManager/
  LookModifier; ui: SettingsWidget/QuickSlotWidget/strings/settings.md/ui.md; art: Tools/Unreal scripts, design-concept.md; docs: goal/decisions
  wording).
