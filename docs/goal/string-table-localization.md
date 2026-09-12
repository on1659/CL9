# goal: string-table-localization

Written 2026-09-12 (autogoal, LIGHT track — design choices settled from engine source + one user decision).

## One-line Goal
Every player-visible string in CL9 is a key into one String Table (`Content/Strings/CL9.csv`) — code, UMG
and data rows carry the key, never the Korean text — so switching the culture (`ko` → `en`) changes all of them
at once, and a `docs/systems/localization.md` tells every future change how to add a string.

## Background / Motivation
Earlier today (session `cl9-fd`, 17:16) the decision was "keep `NSLOCTEXT("CL9", key, 한글)`, no String Table".
The user reversed it: "한글을 박는게 아니라 string alias를 박아서 언어설정 바뀔때마다 싹다 바뀌어야". Recon:

- C++: 86 `NSLOCTEXT` sites, all namespace `CL9`, Korean source inline. 5 `FText::FromString` (names, `\n` joins).
- Data: `UCL9DefBase::DisplayName`/`Description` (FText) hold Korean directly in every table row
  (items, monsters, orders, seg kinds, sites, buffs, characters, equipment, modules).
- UMG: 7 widget blueprints contain designer-typed Korean. `WBP_Pause` (메뉴·설정·로비로 나가기·게임 종료…) and
  `WBP_Spectate`/`WBP_Shop`/`WBP_OrderSelect`/`WBP_ScanLabel` labels are real static text; `WBP_HUD`/`WBP_Dawn`
  hold placeholders that C++ overwrites.
- No localization target, no culture config, no settings class (`UCL9GameUserSettings` is doc-only).

Engine facts (UE 5.8 source) that fix the design:
- `LOCTABLE_FROMFILE_GAME(ID, NAMESPACE, "Strings/CL9.csv")` registers a CSV as a String Table at module startup;
  `GatherTextFromSourceCommandlet` parses that macro and gathers every CSV row (`GatherTextFromSourceCommandlet.cpp:513`).
- The editor's text-property String Table picker enumerates registry tables, not only assets
  (`STextPropertyEditableTextBox.cpp:385`) — so UMG text and DataAsset FText can reference the CSV table.
- `UKismetTextLibrary::TextFromStringTable(TableId, Key)` exists → editor Python can convert existing asset text.
- The requested culture is read from `GGameUserSettingsIni [Internationalization] Culture`
  (`TextLocalizationManager.cpp:294`) — a saved per-user language setting needs nothing custom.
- `FTextInspector::GetTableIdAndKey` lets `Validate()` reject a row whose text is not a table reference.

## In-scope
- **A. String Table** — `Content/Strings/CL9.csv` (`Key,SourceString,DevNotes`; UTF-8 with BOM). Registered by a
  real module class `FCL9GameModule` (`Source/CL9/CL9.cpp`) via `LOCTABLE_FROMFILE_GAME("CL9", "CL9", "Strings/CL9.csv")`,
  unregistered in `ShutdownModule`. Staged for packaging via `+DirectoriesToAlwaysStageAsUFS=(Path="Strings")`.
- **B. Key scheme** (dotted, PascalCase segments; data keys embed the row alias verbatim):
  `Prompt.PickUp`, `Hud.Money`, `Shop.Title`, `Dawn.Passed`, `OrderSelect.Title`, `Spectate.Target`, `Stat.RunSpeed`,
  `Pause.Menu`, `ScanLabel.Name`; data rows `Item.egg.Name`, `Item.egg.cooked.Name` (SubAlias inserted),
  `Item.egg.Desc`, `Monster.goose.Name`, `Order.omelet.Name`, `SegKind.floor.Name`, `Site.henhouse.Name`,
  `Buff.legs.Name`, `Character.farmer_a.Name`, `Equipment.pan.Name`, `Module.<alias>.Name`.
  Separators/punctuation that differ per language (`" · "`, `", "`) are table entries too.
- **C. C++ conversion** — all 86 `NSLOCTEXT` → `LOCTABLE("CL9", "Key")` with the new keys; `FText::Format` calls
  unchanged. `FText::FromString` stays only for player names and `\n` joins (non-translatable) — the `\n` join
  becomes `FText::AsCultureInvariant`-free literal via `FText::FromString` (unchanged, already correct).
  `ACL9ActorBase::GetDisplayName` fallback `FText::FromString(GetName())` stays (debug name).
- **D. Data rows** (user decision: keep the FText columns) — every `DisplayName`/`Description` value becomes a
  reference to table `CL9` with the key from B. `UCL9DefBase::Validate` adds an **error** when `DisplayName` is
  non-empty and not a String Table reference (row becomes Broken → visible in `cl9.list`). Existing rows are
  converted once by an editor Python script `Tools/Unreal/convert_text_to_string_table.py` that also appends any
  missing data keys to the CSV with the current Korean text as source.
- **E. UMG** — every designer-typed text in the 7 WBPs becomes a table reference (same script, walking
  `WidgetTree`); placeholders that C++ overwrites use the key C++ will write (`WBP_HUD` money → `Hud.Money`…).
  If a widget's Python access fails, the fallback is the editor picker by hand — listed in the report.
- **F. Localization target `CL9`** — `Config/Localization/CL9*.ini` (gather source `Source/CL9`, assets `Content`,
  native `ko`, cultures `ko`,`en`), registered in `DefaultEditor.ini` so the Localization Dashboard shows it;
  `DefaultGame.ini` `[Internationalization] +CulturesToStage=ko/en`, `NativeGameCulture=ko`. Gather → export →
  fill **en** draft translations for every key (flagged as draft copy in the doc) → import → compile → commit
  `Content/Localization/CL9/**` (manifest, archives, `.locres`, `.po`).
- **G. Runtime switch** — cheat `cl9.lang <ko|en>` (cheats.md pattern): `FInternationalization::Get().
  SetCurrentLanguageAndLocale`, persist `Culture` to `GGameUserSettingsIni`, flush. Existing widgets update because
  UMG text bindings re-resolve on culture change; widgets that cache `FText` in members re-read on next refresh.
- **H. Docs** — new `docs/systems/localization.md` (the reference: table location, key scheme, how to add a string
  in C++/UMG/data, gather/compile commands, `cl9.lang`, what is *not* translated); `docs/conventions.md` § 6 rule
  replaced to point at it; `docs/decisions.md` entry recording the reversal; `docs/systems/settings.md` 언어 section,
  `docs/systems/cheats.md` row, `docs/data/schema.md` DisplayName/Description rows updated.

## Out-of-scope
- Settings screen / language dropdown (settings system is not implemented; `cl9.lang` is its stand-in).
- Translating anything beyond the two cultures ko (native) and en (draft).
- Engine/plugin strings, log/error messages, cheat output, editor-only text (`DevNotes`, test names).
- Number/date formatting policy beyond what `FText::AsNumber` already does.

## Acceptance Criteria
- [ ] `grep -rn 'NSLOCTEXT\|LOCTEXT(' Source/CL9` returns 0 lines; every `LOCTABLE("CL9", key)` key exists in the CSV.
- [ ] `grep -rn 'FText::FromString' Source/CL9` shows only the player-name, `\n`-join and actor-debug-name uses.
- [ ] `CL9Editor` builds (Development Editor, UE 5.8).
- [ ] Every table row's `DisplayName` (and non-empty `Description`) is a String Table reference — `cl9.list` shows no
      Broken row caused by the new validation, and a row with typed Korean shows `✗` with the localization message.
- [ ] `Content/Strings/CL9.csv` has no duplicate keys, and the gather commandlet reports 0 conflicts.
- [ ] After `cl9.lang en` in PIE: HUD, prompts, pause menu, shop, order select, dawn, spectate and item scan labels
      show English; `cl9.lang ko` restores Korean; the choice survives a restart (GameUserSettings.ini).
- [ ] `docs/systems/localization.md` exists and `docs/conventions.md` § 6 points to it as the only string rule.

## Related Files / Modules
| File | Role |
|------|------|
| `Content/Strings/CL9.csv` | the String Table (new) |
| `Source/CL9/CL9.cpp` / `CL9.h` | module class registering the table (replaces `FDefaultGameModuleImpl`) |
| `Source/CL9/UI/*.cpp`, `Items/*.cpp`, `Character/CL9Character.cpp` | 86 `NSLOCTEXT` sites → `LOCTABLE` |
| `Source/CL9/Data/CL9DefBase.cpp` | `Validate`: DisplayName/Description must be table references |
| `Source/CL9/Core/CL9Cheats.cpp` (or where `REGISTER_CL9_CHEAT` lives) | `cl9.lang` |
| `Content/Data/DA_*Table.uasset`, `Content/UI/WBP_*.uasset` | text → table references (editor script) |
| `Tools/Unreal/convert_text_to_string_table.py` | one-shot converter (new) |
| `Config/Localization/CL9*.ini`, `Config/DefaultEditor.ini`, `Config/DefaultGame.ini` | target + cultures + staging |
| `Content/Localization/CL9/**` | gathered manifest/archives/locres (new, committed) |
| `docs/systems/localization.md`, `docs/conventions.md`, `docs/decisions.md`, `docs/systems/settings.md`, `docs/systems/cheats.md`, `docs/data/schema.md` | docs |

## Must-Preserve
- `FText::Format` argument order/count at every converted site (the `{0}`… placeholders move into the CSV unchanged).
- `UCL9DefBase` column names and types — `DisplayName`/`Description` stay FText (user decision); no schema change.
- Registry behaviour: a Broken row is excluded exactly as before (schema.md `EDefStatus`), only the reason set grows.
- Peer sessions own `CL9Character.cpp`, `CL9Item.*`, `DA_ItemTable` and the running editor (cl9-5b / cl9-2d) —
  those files are edited only after coordinating; asset conversion runs only with the editor closed or through the
  owning session.

## Execution Notes
- Recommended model: Claude Opus 5 (current top tier) for the key scheme, en draft copy and the localization
  target config — copy tone and gather-config mistakes are expensive to redo. Sonnet acceptable for the mechanical
  `NSLOCTEXT` → `LOCTABLE` rewrite once the CSV exists.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is
  below the recommendation, surface it to the user and confirm before proceeding.
