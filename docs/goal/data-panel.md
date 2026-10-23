# goal: data-panel

> **Status**: stage 1 (look table) implemented and verified 2026-10-16 23:27. Stage 2 (every table + per-column effects) — see
> "Stage 2" below. Not committed. How to use and how to add a table: [../systems/data-panel.md](../systems/data-panel.md).

## One-line Goal

A small local web server plus one hand-made HTML page per data table lets the user read and edit CL9 data-asset values in a browser;
while PIE runs, each edit shows on screen at once, and a Save button writes the asset.

## Background / Motivation

User, 2026-10-16: "데이터에셋 세팅을 HTML 로 다 하는 것". Decided by the user: the editor is assumed to be running; every table gets its
own page (no generic auto-generated screen); an edit in the page applies live. Today the same work is done in the editor's details
panel or with CLI scripts (`Tools/Unreal/tune_look.py`, `set_look_brightness.py`) that need a JSON file per change.

## In-scope

- `Tools/DataPanel/data_panel.py` — local HTTP server on `127.0.0.1:8766` (Host header checked like the build broker), no LLM, no
  tokens. Serves the index page, the table pages and a JSON API. Talks to the editor with the engine's `remote_execution` module.
- `Tools/DataPanel/panel_bridge.py` — editor-side module (imported inside the editor's Python, reloaded once per server start):
  generic read (asset → JSON, field names/types/tooltips taken from the reflected docstrings), generic write by path, per-table
  apply hook, save.
- `Tools/DataPanel/tables.json` — the table registry: key, asset path, page, title, live level, apply hook, column doc.
- `Tools/DataPanel/pages/panel.css`, `panel.js` — shared widgets (number with slider, colour, checkbox, asset path, read-only text),
  change markers, save/revert bar, editor/PIE status line.
- `Tools/DataPanel/pages/look.html` — the look table (`/Game/Data/LookTable`, column reference `docs/plan/data/look.md`): screen,
  lighting profiles (brightness, lamp survive share, Lobby/Night/Blackout phases), brightness endpoints `NightDark`/`NightBright`,
  lamp kinds, HUD colours, surface (`bSurfaceDetail` + families).
- Docs: this spec, `docs/systems/data-panel.md` (how to run it, how it works, **how to add the next table page**), pointer rows in
  `docs/systems/README.md` and `docs/plan/data/look.md` (tools section).

## Out-of-scope

- Adding, removing or renaming array rows (profile, lamp kind, surface family names are keys other tables point at — renaming breaks
  references). Names are shown read-only; row edits stay in the details panel.
- Undo history. The page offers "revert to last saved" (the values the server read at start or at the last save), nothing finer.
- Starting/stopping PIE, loading maps, building. The panel never does those (sessions.md lock rules stay intact).
- ~~Pages for other tables — added one at a time after this, the user picks the order.~~ Superseded 2026-10-17: the user asked for a
  page for every table at once ("테이블마다 만들자는건데") — see Stage 2.
- Shipping builds; running without the editor.

## Stage 2 — every table, with what each value does (2026-10-17)

User, 2026-10-17: "테이블마다 만들자는건데" (a page for every table, not one at a time) and "수치 바꾸면 어떤식으로 바뀌는지도 써줘"
(say, for each value, how the game changes when it is changed).

### In-scope

- A page for each of the 20 data assets: the 19 under `/Game/Data` plus `/Game/UI/UITable` (the string table is out — its CSV is the source).
  `CL9DefList` and `SettingsTable` are read-only (the list refills itself on save; the settings table is sound-mix/class references).
- Shared page builders in `panel.js`: `Panel.table` (row list + grouped columns for `UCL9DefTable`), `Panel.whole` (one card per top-level
  column for assets of mixed shape), plus `P.auto`/`P.rows`/`P.groups` that render any value shape (structs, arrays, maps, inline rows,
  vectors, enums, curves). Groups follow each class's C++ `Category`; columns not in a group appear under "그 밖의 열" — none is dropped.
- `Tools/DataPanel/effects/<table>.csv` (`path,effect,timing,evidence`) — for every editable column: what changes in the game when the value
  goes up/down, when it takes effect, and the C++ reader (file:line). Written from the readers, not guessed. Shown next to each column.
- Reading types that Python does not expose (`UCLASS()`/`USTRUCT()` without `BlueprintType`, `EditAnywhere`-only columns): column lists come
  from the C++ headers (`cpp_schema.py`), properties are addressed by their C++ name.
- C++ additions on `UCL9DefBase` (all for the panel): `ValidateForPanel` (Validate as a UFUNCTION), `GetPanelCurveKeys`/`SetPanelCurveKeys`
  (`FRuntimeFloatCurve` body is protected), static `ClearPanelDirty`. `ECL9UILayer` became `UENUM(BlueprintType)` (Python could not read it).

### Design Decisions

- **C++ Validate gates every edit.** Python writes bypass `ClampMin` and `Validate`; the bridge tries the edit, runs the touched rows'
  `ValidateForPanel`, restores, and applies for real only when there are no errors (otherwise `rejected` with the C++ messages).
  (Rejected: copying Validate ranges into slider limits — Validate has cross-column rules such as `SwingHitStart < SwingHitEnd`.)
- **Columns that must be equal are linked** in the page (`spec.link`, sent in one request) — monster `rage_time` = `kill_cooldown`.
- **Server-side guards for tables without Validate** (`tables.json` `guards`, e.g. wave `dots.ring_spacing > 0` — 0 hangs a C++ loop).
- **When two editors run, use the one owning MCP port 8000** (the later one is a ghost — sessions.md).
- **A table back at its saved values is marked clean** (`ClearPanelDirty`): a leftover dirty mark made the broker's editor restart stop at a
  modal "패키지 복원" dialog, which froze editor Python (observed 2026-10-17 after the API test).
- **Reply wait is bounded** (60 s per request, 10 s queue wait): the engine's `remote_execution` waits on `recv` with no timeout, which froze
  the server while the editor was loading.
- Effects text lives in CSV (edited in Excel), not in the page source — tables-are-data-files rule.

### Acceptance Criteria (stage 2)

- [x] All 20 tables read without error; every page renders with no console error.
- [x] Per-column effect text shows next to the columns (CSV present for all 20 tables).
- [x] An edit that breaks C++ Validate is refused and the value stays (rules `throw_charge_time` 0, monster `kill_cooldown` alone).
- [x] Inline rows, maps with C++ names, nested arrays, enums and curves can be edited and reverted.
- [x] PIE: an edit in a non-look table changes the running game at once (a "바로" column), shown in a screenshot.
- [x] Save writes a `UCL9DefTable` asset; a revert leaves no dirty package.

### Results (2026-10-17)

- Three broker builds (36 s, 26 s, 30 s): `ValidateForPanel`; curve keys + `ECL9UILayer` BlueprintType; `ClearPanelDirty`.
- Reads: 20/20 ok, 0.1–0.6 s (`ModuleTable` 125 KB is the largest). Unreadable values: none after the enum fix.
- API test (edit → read back → revert): rules, items (float, enum), monsters, wave map row (C++ names), wave struct, shake, sites, modules
  (`Rows[0].prop_sets[0].props[0].yaw`), segkinds, progression int, progression curve keys, UI enum — all pass. Rejected as expected:
  rules `throw_charge_time` 0 (C++ "Throw needs … a positive ThrowChargeTime"), monster `kill_cooldown` alone ("must equal RageTime"),
  wave `Dots.RingSpacing` 0 and `Rows.RUN.Speed` 0 (server guards). `rage_time` + `kill_cooldown` together: accepted.
- PIE `Lvl_Henhouse`, through the rules page: `outdoor_fog_scale` 0.06 → 1 — the yard filled with fog and the lamps bloomed at once;
  "저장값으로 되돌리기" brought the clear yard back. Stills `Saved/DataPanel/verify/rules_fog_{before,after,reverted}.png`.
- Save during PIE: `MonsterTable` written (timestamp 10-04 → 10-17 01:23), value restored and saved again. Side effect: the re-saved
  `MonsterTable.uasset` (values identical) now shows as modified in git; `UITable.uasset` also shows modified (re-saved to clear a dirty
  mark — whether another session had changed it before is not known). Not reverted (no destructive git commands without the user).
- Effects CSV: 20 files, about 600 rows, written by five read-only research passes from the C++ readers. Findings they surfaced are in the
  text (e.g. buff `CarryMult`/`JumpVelocity` only show in the HUD; `ShakeFullTime`/`LeaverGraceTime` have no C++ reader).
- Incident: after the first API test the broker restart showed a modal "패키지 복원" dialog (9 tables the test had edited and restored)
  that froze editor Python; closed it (skip — autosaves equal to disk) and fixed the cause (`ClearPanelDirty`, `mark_dirty`).

## Design

- **Transport — remote_execution, one connection per request.** The editor's MCP server (port 8000) cannot call a UFUNCTION
  (`ApplyNow`), so it cannot make an edit live. The engine's remote execution accepts exactly one command connection at a time and a new
  open replaces the old (`FPythonScriptRemoteExecution::OpenCommandConnection`), so the server keeps only the UDP discovery running and
  opens/closes the TCP command connection around each request, on its own command port (6777) so it never collides with
  `Tools/Unreal/run_python.py` (6776). Requests are serialized by a lock; a failed open is retried once.
- **Values JSON.** float/int/bool/str as is; `Name`/`Text` as string; `LinearColor` `[r,g,b,a]` (linear); `Color` `[r,g,b,a]` 0–255;
  object and soft-object refs as asset path or `null`; structs as objects keyed by Python field name; arrays as lists; enums as their
  name. Field order, types and Korean tooltips come from the `**Editor Properties:**` list of the reflected class/struct docstring —
  the page shows the tooltip under each field, so the C++ comment stays the one source of the meaning.
- **Write by path.** `{"path": ["profiles", 0, "night", "fog_density"], "value": 0.3}`; several edits go in one request. The bridge walks
  the path with `get_editor_property`, converts the leaf by the current value's type, then writes every copy back up the chain (Python
  gets struct and array copies). After the edits it calls the table's apply hook and returns the re-read table in the same reply.
- **Live hook for the look table.** Any edit under `profiles[*].brightness`, `night_dark` or `night_bright` → `ApplyBrightness()`
  (rewrites every profile's Night numbers from the endpoints — same as the details panel for endpoints); otherwise `ApplyNow()` (pushes
  surface values into the material instances and bumps `Revision`; listeners re-apply next tick — look.md "실시간 모드").
- **Save.** `EditorLoadingAndSavingUtils.save_packages([...], only_dirty=False)` on the table package, plus — for the look table — the
  dirty packages among `surface_families[*].materials` (the surface push edits them). `EditorAssetLibrary.save_loaded_asset` refuses
  during PIE ("The Editor is currently in a play mode", measured 2026-10-16); `save_packages` saves during PIE. The server then takes
  the saved values as the new baseline.
- **Reply through a file.** The bridge writes its JSON to `Saved/DataPanel/reply.json` and prints one marker line — editor Python output
  also goes to the editor log, and printing the whole table would add tens of KB to the log per slider step.
- **Page.** Edits send on slider `input` (throttled to one request in flight + the latest pending value) and on field `change`. Fields
  that differ from the baseline are marked; the bar shows the count, "저장" and "저장값으로 되돌리기". The status line shows editor
  connection, PIE running, and the current `cl9_lock` holder (information only — the user drives the panel; editing during someone
  else's filming PIE is the user's call).

## Acceptance Criteria

- [x] `py -3 Tools/DataPanel/data_panel.py serve` starts; `http://127.0.0.1:8766/` lists the look table page.
- [x] The look page shows every look field listed above with current values and the Korean tooltip.
- [x] PIE on `Lvl_Henhouse`: changing (a) a night profile value (fog / exposure), (b) a lamp kind's intensity or colour, (c) screen
      posterize/max resolution in the page changes the running game without restarting PIE — screenshots before/after.
- [x] "저장" writes `/Game/Data/LookTable` (file timestamp changes, value survives a fresh read); "되돌리기" restores the baseline.
- [x] Values restored to the pre-test state and saved afterwards.
- [x] `docs/systems/data-panel.md` explains how to add the next table page (registry row, page, apply hook, live level).

## Results (2026-10-16)

- Round trip page → editor → reply ≈ 0.1 s (one command connection opened and closed per request).
- PIE `Lvl_Henhouse` (yard, night), edits made through the page's own inputs; stills `Saved/DataPanel/verify/`:

  | Change in the page | Running game |
  |---|---|
  | (a) profile `default` Night `ExposureBias` 1.8 → -1.5 | screen goes dark (`pie_a_exposure.png`) |
  | (b) `YardDoor`/`Campfire` colour red, intensity 40 (slider) | lamp over the building door turns red (`pie_b_lamps.png`) |
  | (c) `MaxResolution` 520 → 180, `PosterizeBands` 4, `PosterizeMix` 1 | coarse pixels, four tone bands (`pie_c_screen.png`) |
  | "저장값으로 되돌리기" | picture back to the start (`pie_d_reverted.png`) |
  | `Grain` 0.15 → 0.2 → save, then 0.15 → save (during PIE) | `LookTable.uasset` timestamp 13:57 → 23:27, page "저장된 상태"; 17 packages saved (table + 16 surface MIs) |

- Page console: no errors. Values are back to the pre-test state and saved.

## Related Files / Modules

| File | Role |
|------|------|
| `Tools/DataPanel/*` (new) | Server, editor bridge, registry, pages |
| `Source/CL9/Data/CL9LookDef.{h,cpp}` | `ApplyNow`, `ApplyBrightness`, `Revision` — used as is, no C++ change |
| `Tools/Unreal/tune_look.py`, `set_look_brightness.py` | Same job from the CLI; stay as they are |
| `Tools/Build/build_broker.py` | Pattern for the local server (Host check, JSON API, inline status) — not modified |
| `docs/plan/data/look.md` | Column reference the page follows |

## Must-Preserve

- No C++ or asset-format change; the details panel and the CLI scripts keep working.
- Asset saves name the asset explicitly (never `save_assets([])`).
- The panel never loads maps, starts/stops PIE or holds the editor's remote-execution connection between requests.
- Korean comments and Korean page text; values stay in the asset (the page holds no tuning numbers of its own except slider ranges).

## Execution Notes

- Recommended model: the strongest current Claude model for the bridge (path writes over Python struct copies) and the live-check;
  a cheaper model is fine for page layout of later tables. This session runs Opus 5.5.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the
  recommendation, surface it to the user and confirm before proceeding.
