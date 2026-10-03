# goal: performance-baseline-and-guidelines

> Status: implemented 2026-09-21 (same session). Measurements below are the record; the living rules are in
> `docs/systems/performance.md`.
> **Follow-up the same night (2026-09-21 02:00–03:00, user: "the lag is when monsters come out; test freely"):** the
> drop is not a frame-rate problem but single-frame stalls from **first-use synchronous loads of soft references**.
> `stat dumphitches` stack of the 694 ms hitch at the first catch: character tick → `Flush Async Loading` 577 ms
> (camera-shake Blueprint class load + compile) + `AddCameraShake` 30 ms. Same family: first wave ring ≈ 500 ms (Niagara
> system load, once per process), first hit ≈ 400 ms (hit-flash material / puff / shake), first spawn per species
> 35–41 ms (mesh + ABP). Sustained costs are small: 5/10/20 chasing monsters +0.7/+1.5/+3 ms; hardware-RT, VSM,
> skeletal RT and flashlight toggles all < 1 ms; meshes are 8–11k verts, one LOD, one material — not the cause.
> **Fix:** `UCL9GameInstance::Init` now `LoadSynchronous`-loads every soft reference of the tables (shake classes, sounds,
> wave Niagara systems, rules hit-flash/puff, monster and character meshes/ABPs) and keeps them alive in a
> `UPROPERTY TArray<TObjectPtr<UObject>> WarmAssets` (loading without a hard reference is undone by the next GC).
> Use-site `LoadSynchronous` calls stay as a safety net. Rule written in `performance.md` "첫 사용 동기 로드 금지".
> Verification scenario: `docs/testing/pie-scenarios.md` U (dumphitches, no non-GC hitch ≥ 30 ms at first catch/ring/hit).

## One-line Goal
Answer "does CL9 need optimizing right now?" with measurements instead of a feeling, fix what the measurements
actually blame, and leave a repeatable measuring tool plus written rules so every later feature keeps the frame
budget in mind.

## Background / Motivation
The user reports PIE getting slower as work goes on ("작업할 때마다 자꾸 느려진다") and asked whether to optimize.
Nobody had ever measured: no `docs/systems/performance.md`, no budget, no procedure. Prior sessions recorded
"this machine runs PIE at 3~8 fps" and designed around it (e.g. the swing sweep note in `inventory.md`,
`slomo 0.15` in automation). The renderer config is the UE 5.8 first-person template default: Lumen with hardware
ray tracing, Lumen reflections, Virtual Shadow Maps, ray tracing + proxies, Substrate, Nanite, TSR, Epic scalability.
Machine: RTX 3070 Ti, Ryzen 5 5600 (6c/12t), 16 GB RAM.

## What was measured (2026-09-21 00:52–01:08, editor PIE in viewport, 1 client, listen server, `Lvl_Henhouse`)
Tool: `Tools/Unreal/perf_probe.py` (slate post-tick callback sampling PIE world delta time, 5 s per config after a
2 s settle, then `ProfileGPU` into the log) + `Tools/Unreal/perf_parse.py`. Viewport 1466×504.

| Scene | Frame avg / p95 (ms) | fps | GPU graphics + async compute (ms) |
|---|---|---|---|
| Lobby, day, at the truck | 16.4 / 21.3 | 61 | 6.2 + 3.6 |
| Night, at the truck (site built, 7 monsters asleep) | 12.6 / 15.0 | 79 | 6.2 + 2.7 |
| Night, inside the henhouse under a work lamp | 13.0 / 15.2 | 77 | 4.8 + 2.5 |
| Same spot, `r.ScreenPercentage 200` (≈2932×1008, 1080p-class pixel count) | 13.5 / 16.0 | 74 | (not profiled) |

Toggles at the night-interior spot (avg ms, baseline 13.0): Lumen off 12.0 · VSM off 13.1 · both off 12.0 ·
Lumen software RT 12.1 · volumetric fog off 12.9 · screen percentage 50 12.9 · `sg.*` Medium 13.5 · High 13.3.
At 200 % screen percentage the same toggles land within 12.0–13.7. Conclusion: **the frame is CPU-bound at
≈12 ms on this machine; the GPU has headroom even at 1080p-class resolution.** No renderer feature buys more than
about 1 ms, and Lumen is a lighting-design dependency (`SkyLeak` is the whole indoor ambient — `light.md`).

Game-thread frame dump (night, truck): 61 tick functions ≈ 4.9 ms of which 7 monster skeletal meshes ≈ 1.6 ms and
the player `ABP_Unarmed` ≈ 1.5 ms; a single 23 ms `IpNetDriver Socket RecvFrom` spike was seen once (watch item).
Render thread ≈ 5.7 ms scene render + 1.1 ms end-of-frame.

**The real slowdown found:** with the editor window minimized (it was) or in the background, the engine throttles
to exactly 333 ms/frame (3 fps) whatever the CVars say — `UEditorPerformanceSettings.bThrottleCPUWhenNotForeground`
defaults to true, and a minimized editor throttles even when the flag is off. Every "3~8 fps" automation number in
the memory/docs came from this, not from the game. The first probe run reproduced it exactly (all configs 333.3 ms).

## In-scope
- Project-level editor setting: `bThrottleCPUWhenNotForeground=False` in `Config/DefaultEditorSettings.ini` (the class is
  `config=EditorSettings`, so `DefaultEditorPerProjectUserSettings.ini` would be ignored — verified in the engine header)
  so PIE keeps its frame rate when the editor is not the foreground window (automation, alt-tab to the terminal).
- Reusable measuring tool: `Tools/Unreal/perf_probe.py` (runs inside a PIE via `run_python.py`, config JSON,
  results under `Saved/Perf/`) and `Tools/Unreal/perf_parse.py` (ProfileGPU table → pipeline totals + top passes).
- `docs/systems/performance.md`: budget, the baseline table above, how to measure (procedure + tool), the
  "why is it slow for me" checklist (minimized editor, concurrent compiles, two PIE clients, shader compiles, RAM),
  rules for new code/assets (tick hygiene, per-frame iteration, lights, Niagara, traces, UI ticks, replication),
  and the lever order with measured savings for when the GPU does become the bottleneck.
- Index/convention hooks: two rows in `docs/systems/README.md`, one checklist line in `docs/conventions.md` §6,
  one `docs/decisions.md` entry, PIE scenario section "U. 성능 기준선" in `docs/testing/pie-scenarios.md`.

## Out-of-scope
- Changing renderer defaults (Lumen/VSM/hardware RT/Substrate/Nanite/TSR). Measured gain ≤ 1 ms while the GPU is not
  the bottleneck; Lumen is load-bearing for the night look. Revisit only when a measurement shows GPU-bound frames.
- Steam minimum-spec decision (hardware-RT Lumen needs an RT-capable GPU; software Lumen fallback; `r.RayTracing`
  permutations). That belongs to the Steam release gate (`docs/goal/steam-release.md` §7), not here.
- Code optimizations. Nothing in the tick paths measured as a problem (monster senses iterate ≤4 players, wave traces
  fire only on emission, physics items sleep). No speculative rewrites.
- Build-time/editor-load optimization (Intermediate 11 GB, shader compile time) — not what the user reported.

## Acceptance Criteria
- [x] Baseline numbers exist in a document with the exact procedure that produced them.
- [x] The 3 fps throttle cause is written down where the next session will find it (performance.md, memory) and the
      project setting removes the not-foreground case.
- [x] `perf_probe.py` + `perf_parse.py` live in `Tools/Unreal/` with no scratchpad paths, and performance.md tells
      how to run them (lock → StartPIE → run_python → parse → StopPIE → release).
- [x] performance.md has a checklist a feature author can apply before writing a tick, a light, an effect or a trace.
- [x] systems/README.md (both tables), conventions.md §6 checklist, decisions.md, pie-scenarios.md updated.
- [ ] User confirms the setting change is acceptable (editor keeps rendering in the background) — reversible by
      deleting the ini section.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/GameMode/CL9GameInstance.h` · `.cpp` | the fix — `WarmAssets` + `Warm()` and the preload block in `Init` (shake classes, sounds, wave Niagara, rules flash/puff, monster mesh/ABP/class, character meshes/ABPs, item class/mesh/icon) |
| `Source/CL9/FX/CL9WaveRenderer.h` · `.cpp` | `OnWorldBeginPlay` activates each wave Niagara system once — the editor compiles Niagara scripts on first activation (≈400 ms per process) |
| `docs/testing/pie-scenarios.md` U · `docs/systems/character.md` (shake) · `sound.md` · `wave.md` | where the preload is referenced from |
| `Config/DefaultEditorSettings.ini` | project layer of the `EditorSettings` ini — new file with the `EditorPerformanceSettings` section |
| `Tools/Unreal/perf_probe.py` | in-PIE frame-time sampler + ProfileGPU trigger, config-driven |
| `Tools/Unreal/perf_parse.py` | parses the ProfileGPU table out of `Saved/Logs/CL9.log` |
| `docs/systems/performance.md` | the rules + baseline + procedure (new) |
| `docs/systems/README.md` · `docs/conventions.md` · `docs/decisions.md` · `docs/testing/pie-scenarios.md` | index/convention/decision/test hooks |
| `docs/systems/light.md` | Lumen dependency of the night look (read-only reference) |
| `docs/workflow/sessions.md` | lock protocol the measuring procedure must follow |

## Must-Preserve
- Lock protocol: StartPIE/StopPIE only with the `pie` lock, taken with a single `py -3 .claude/hooks/cl9_lock.py …`
  command (chaining with `cd … &&` makes the hook miss it).
- Renderer config in `Config/DefaultEngine.ini` unchanged.
- No CVar left changed after a probe run: the tool restores every CVar it touches (including `r.AntiAliasingMethod`)
  and the throttle flag.
- Docs language: project docs in Korean; this goal doc in English (autogoal rule).

## Execution Notes
- Recommended model: Claude Fable 5.1 (current top model) for the measurement interpretation and the rules text;
  Sonnet acceptable for the index rows and ini edit.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is
  below the recommendation, surface it to the user and confirm before proceeding.
