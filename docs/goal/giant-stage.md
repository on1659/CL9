# goal: giant-stage

Status: spec settled by the planning panel (2026-10-04, HEAVY track). Open Questions gate implementation. **2026-10-16 정정**: 이 문서에 Open Questions 절은 없고, 맨 아래 "Gate answers (user, 2026-10-04)" 로 답이 끝났다. 구현은 부분이다 — `UCL9SiteDef::Districts`·`FCL9SiteDistrict` 와 테스트 `CL9.Map.Districts`·`CL9.Map.DistrictSlots` 가 커밋 0b2b7cc(WIP, gameplay 중단·컴파일 미확인)에 있고, `CL9.Map.Districts` 는 이후 세션들이 "알려진 실패"로 기록하고 있다(look-table.md Results). `Lvl_Mongjang`·`Tools/Unreal/stages/mongjang.json` 은 없다.

## One-line Goal
A stage of **five building districts around one truck yard, ~1,000 ground-floor modules in total (5× the farm)**, that builds,
navigates, performs and plays as one 600 s night — and that can be raised to 10× (2,000) if the 5× measurements pass.

## Background / Motivation
- User (2026-10-04): "make it big — the map can be 5× or 10× bigger than now", for **both** the generated grid and the outdoor
  backdrop (docs/goal/outdoor-backdrop.md); base = **farm** (not henhouse).
- Current biggest: `farm` 200 modules (day 1 = 170 + upper), ~350–390 m grid, night 480 s, ~25 monsters day 1, editor PIE ~10 ms.
  How stages are made: docs/systems/stage.md (`Tools/Unreal/stages/<alias>.json` + `setup_stage.py`).

## User decisions (2026-10-04 — not to be relitigated)
1. **Size** — 5× first (~1,000 base modules). 10× only after 5× passes its measurements.
2. **Night** — one night, `NightLength` 600 s ("same as Lethal's day": docs/reference/lethal/lethal-map.md code defaults,
   totalTime 700, landing at 100, speed 1, auto-leave at 0.998 → ~600 s; the scene-serialised speed was not verified).
3. **Shape** — districts: several building clusters joined by outdoor yard, not one huge building. A night cannot clear
   everything; players choose which district to raid.
4. **Monsters** — all monsters stay active however far they are from players (no distance sleep/despawn). Total count chosen
   by comparison with Lethal Company, not a blind 5× of the farm.
5. **Target** — 1–4 players, Steam listen server (project default; not asked). Every machine generates the map from the seed.

## In-scope
- District support in the site data and runtime (below), a new stage row + level through `setup_stage.py`, measured QA.
- Coordinated with docs/goal/outdoor-backdrop.md — the backdrop must sit outside the union bounds of all districts.

## Out-of-scope
- Changing henhouse/farm/pigfarm (their output must stay byte-identical).
- Economy redesign (`ValueRate` stays 1.0), vehicles/shortcuts, yard content or yard monsters, district-choice UI/signage,
  new art or new rows in the `henhouse` theme, HLOD, navigation invokers.
- The 10× step itself — a follow-up once this goal's measurements pass.

## Design Decisions (panel-settled — do not relitigate during implementation)
- **Architecture: K districts = K `ACL9SiteMap` actors, each running the unchanged generator** on its own sub-seed, module
  count and world transform. New `FCL9SiteDistrict {Offset (cm from truck pad), Yaw ∈ {0,90,180,270}, ModuleCount (base),
  Zones (empty = site Zones)}` and `TArray<FCL9SiteDistrict> Districts` on `UCL9SiteDef`. Empty `Districts` = today's
  single-site path, byte-identical. (Rejected: multi-root growth inside one grid — changes the placement core, unmeasured at
  N ≥ 1000, still needs district-keyed ISM; "stitch K layouts into one" — same rotation work plus a trunk generator.)
- **Grid cap stays 128.** Largest district at day 10 = 230 × 1.25 = 288 modules → span 84. (Rejected: raising the cap — only
  the single-grid design needs it.)
- **Seeds:** `SubSeed_i = HashCombine(Seed, i)`, 0 remapped to 1 (seed 0 builds nothing, CL9SiteLayout.cpp:842). One seed is
  replicated as today. `LayoutHash` and build ms logged per district; a scoped per-phase timer in `ACL9SiteMap::Build`.
  Build stays synchronous (it runs at lobby entry, not at night start). (Rejected: async/streamed generation before measuring.)
- **Rotation by actor yaw:** `SiteToWorld` goes through `GetActorTransform()` (CL9SiteMap.cpp:238-242); the flap is spawned
  with the actor rotation and a world-rotated `Outward` (CL9SiteMap.cpp:465-468). Nothing else — ISM instances, components
  and lamps are already actor-local. (Rejected: a layout rotation pass inside the generator.)
- **Districts: 5 fixed slots in the stage JSON; the seed fills only their contents.** Initial values: East (yaw 0) and West
  (yaw 180) ~230 modules, gates 10–30 m from the truck; North and South (yaw 90/270) ~190, ~210 m; one far district ~160,
  ~300 m. Sum ≈ 1,000. Size rule per district (stage.md:58-59 with night 600, `MainPathShare` 0.30, 4.5 m/s):
  `1.59·N_i + 0.444·L_i ≤ 440` (N = base modules, L = one-way yard distance in m). Rotated day-10 span rectangles may not
  overlap. Final offsets are set after the footprint measurement. (Rejected: equal 5 × 200 — far districts break the 600 s budget.)
- **Truck and yard:** one existing `ACL9TruckPad` at the yard centre anchors every district. The yard is the level's bare
  floor, scaled by `setup_stage.py` to the union of rotated day-10 district rectangles + margin. (Rejected: yards built from
  `yard_open` modules — fenced paddocks; a new road row — changes the pools of the three existing stages.)
- **Entrances/exits:** each district keeps the generator's own `yard_gate` (facing the truck after yaw) and its single flap.
  module.md decision 3 ("one entrance, one flap") is recorded as applying **per district building**. (Rejected: one
  entrance/flap for the whole stage — only the single-grid design can express it.)
- **Depth zones** are evaluated per district (per-layout hop normalisation, unchanged). Districts differ through the optional
  per-district `Zones` override; far districts get heavier rows (pig, raccoon, meat). (Rejected: global hop depth; a new
  danger-tier column.)
- **Nav:** dynamic runtime navmesh, default tile size, **no invokers**. One `NavMeshBoundsVolume` over the union bounds (yard
  included). On the host, `StartNight` defers with a retry timer while the navigation build is in progress, and logs the wait.
  (Rejected: invokers — violate decision 4 or move the build into the night; larger tiles before measuring. Fallback if the
  union build is slower than ~100 s: per-slot bounds volumes with no yard navmesh, then compare tile size 1000 vs 2000.)
- **Rendering:** no HLOD and no district show/hide in the first cut — per-district actors already give per-district
  components for frustum/occlusion culling. Lamp and flicker-lamp counts are logged. Collision stays on for every district
  (far monsters walk against it). (Fallback only if yard-view GPU is over budget: lamp `MaxDrawDistance` + skip `TickFlicker`.)
- **Monster cost under decision 4:** AI, senses and movement always run. Monster mesh
  `VisibilityBasedAnimTickOption = OnlyTickMontagesWhenNotRendered`, after checking the five monster ABPs/BPs for
  notify-driven gameplay. No think-rate LOD, no distance sleep. If the measurement fails, the only lever is lower counts.
- **Population:** per district ≈ one Lethal interior. Base zone sums ~9 (East/West), ~11 (North/South), ~13 (far) ≈ 53;
  through the existing `MonsterCountCurve` → ~32 on day 1, ~53 on day 5, ~80 on day 10 (per district on day 10: 13–20,
  against Titan's indoor power 18 at 1–3 power per enemy — lethal-map.md:204, lethal-monsters.md:595-629). Items keep the
  farm's per-module density (~0.35/module at base). (Rejected: a Lethal-style power budget — new code; 5× farm density.)
- **Night row:** `NightLength` 600, `DarkBase` 450, `DarkMin` 300 (farm's 360/240 × 600/480).
- **Jars:** `SpecialCopies` (inherited 4) applies per district → ~20 delivery rooms. The jar dedupe set in `SpawnNight`
  resets per district (module indices are per layout).
- **Relevancy:** default `NetCullDistance` kept; the client hitch when crossing districts is measured in 2-client PIE.

## Measure first (with fallbacks)
1. `Lvl_Farm` `perf_probe` baseline at three equivalent spots (lobby-day truck, night yard, night deep interior) — before
   any giant-stage frame numbers.
2. District footprint (occupied bounding box, 30 seeds, 160–290 modules) → pull slots closer if the building is much smaller
   than the span; otherwise place by full rotated span rectangles and shrink far districts per the size rule.
3. Fill and generation time at each slot's day-10 count; if < 90% or a seed fails, move modules to the near slots.
4. Food truck / kitchen / counter positions relative to the pad in the source level — if the West arm collides, move its
   gate outward and cut its N.
5. Nav: seconds from `SetSiteMap` to build done, tile count, memory (union box).
6. Host frame with the day-10 monster count forced awake, after the anim change; per-phase site build ms and memory for K=5
   on host and client; lamp counts and GPU Lights/VolumetricFog in the yard view; night-start spawn spike.
7. Scripted walk truck → gate → deepest point → nearest jar → truck per district against 600 s.
8. 2-client PIE: client `LayoutHash` per district equals the host's; hitches while crossing the yard.

## Acceptance Criteria
- [ ] henhouse/farm/pigfarm unchanged: `CL9.Map.LargeSite` and `CL9.Map.DepthBand` pass with identical logged hashes.
- [ ] `CL9.Map.Districts`: per slot, 30 seeds at day-10 count all build, connected, no overlap, exactly 1 flap,
      ≥ 2 delivery rooms, ≥ 90% of budget; generation ms logged per layout.
- [ ] Slot test: base `ModuleCount`s sum to 1,000 ± 5%; no two rotated day-10 rectangles overlap; every district satisfies
      `1.59·N_i + 0.444·L_i ≤ 440`.
- [ ] 2-client PIE: every client logs the host's `LayoutHash` for each of the 5 districts.
- [ ] Nav: night does not start while the host nav build is in progress; every spawned monster projects onto the navmesh
      (100-unit extent); a path exists from the truck pad to every district gate; nav-done seconds recorded.
- [ ] Population: ~32 monsters on day 1 and ~80 on day 10 in the night log; in a forced-awake pass every monster is still
      spawned and ticking AI at +90 s.
- [ ] Frame (1-client editor PIE, day-10 count, three spots): avg ≤ 16.7 ms, p95 ≤ 25 ms, GPU ≤ 10 ms (performance.md:16,19);
      CL9 game-thread ms handled per Open Question 1.
- [ ] Night 600/450/300; the scripted walk completes within 600 s for every district at base size.
- [ ] Build logs (and QA JSON) show per district: modules, instances, components, props, lamps, flicker lamps, per-phase ms.

## Related Files / Modules
| File | Role |
|------|------|
| Source/CL9/Data/CL9SiteDef.h | `FCL9SiteDistrict`, `Districts` |
| Source/CL9/World/CL9SiteMap.h/.cpp | district index + transform, sub-seed, FindAll/Rebuild/Clear loops, logs, timers |
| Source/CL9/GameMode/CL9GameState.cpp, CL9Cheats.cpp | loop over districts |
| Source/CL9/GameMode/CL9GameMode.h/.cpp | `SpawnNight` per district, `StartNight` nav wait |
| Source/CL9/Monsters/CL9Monster.cpp | anim tick option |
| Source/CL9/World/CL9SiteLayoutTests.cpp | `CL9.Map.Districts`, slot test |
| Tools/Unreal/setup_stage.py, stages/giant.json, qa_farm_stage_pie.py | districts in the row, union bounds, QA additions |
| docs/decisions.md, docs/systems/stage.md · map.md · performance.md, docs/plan/data/site.md | records |

## Work split (`/cl9 task`)
1. **map** — C++ district support + tests (opus).
2. **map** — `setup_stage.py` districts/union bounds, `giant.json`, level, QA script; **gameplay** — `SpawnNight` loop, nav
   wait, anim tick option (after 1).
3. **balance** — slot offsets/counts by the size rule and measured footprint, per-district zone rows (after measurements; sonnet for rows).
4. **docs** — decisions and system docs (sonnet).
- art, ui: none in the first cut.

## Must-Preserve
- Empty `Districts` → the existing single-site path, byte-identical layouts for henhouse/farm/pigfarm.
- One replicated seed; every machine builds the same layouts.
- User decision 4: never sleep, despawn or throttle monsters by distance.
- performance.md: no optimisation before measurement.

## Fairness Constraints
- All layout randomness comes from the replicated seed through deterministic sub-seeds; spawning stays server-side.

## Existing Integration Contract
- `MapTable` row → `ResolveSiteAlias` → `SetSiteAndDanger`/`SetSiteMap` flow (stage.md) is unchanged; districts are extra row data.
- Stage JSON remains the source of truth; `setup_stage.py apply` stays idempotent.

## Known weak points (verify during implementation)
- The size rule rests on a two-point fit and an unmeasured 4.5 m/s yard speed; carts move at 2.97 m/s.
- Only C++ was grepped for zero-rotation site-frame assumptions — Python QA math, backdrop placement and debug drawing were not.
- ~80 always-on monsters: per-monster host cost after the anim change is unmeasured.
- Union navmesh build time is a linear extrapolation from the farm.
- `setup_stage.py:79` hand-copies the span formula.
- District choice may be a blind pick without signage (out of scope here).

## Execution Notes
- Lead: Claude Fable 5.1 (architecture-heavy). Subagents per docs/workflow/sessions.md "모델 배분": district C++, nav wait
  and root-cause work = opus; table rows, stage JSON, docs = sonnet.
- This document cannot enforce the model — the executing session's `/model` decides; confirm with the user if below.
- Pitfalls: docs/goal/outdoor-backdrop.md "Known pitfalls" (locks, commandlets, saved navmesh in duplicated levels).
- unreal-mcp was not connected when this spec was written; editor work needs it back.

## Gate answers (user, 2026-10-04)
- **Performance gate = frame time only** (avg ≤ 16.7 ms, p95 ≤ 25 ms, GPU ≤ 10 ms). CL9 game-thread ms is measured and
  reported; the new budget is recorded in decisions.md. 4 ms is not a gate for this stage.
- **No all-at-once loading** ("loading everything at once is obviously slow — build it with optimisation in mind"): districts
  are built **one per frame** and night monsters are spawned **time-sliced (a few per frame)** from the start, not as
  measurement fallbacks. This supersedes "build stays synchronous" and "spawning stays synchronous" above. Night start still
  waits for all districts and the nav build.
- **Name** — alias `mongjang`, level `Lvl_Mongjang`, spec `Tools/Unreal/stages/mongjang.json`, Korean display name
  "몽장" (string key `Site.mongjang.Name`). Wherever this document says `giant.json`, read `mongjang.json`.
