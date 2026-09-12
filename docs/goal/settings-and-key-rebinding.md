# goal: settings-and-key-rebinding

## One-line Goal
A local options system (`UCL9GameUserSettings` + Enhanced Input user settings) with a Settings screen — key rebinding, mouse sensitivity / Y-invert, language, audio volumes — where every user-facing key label (`[E] 줍기`, `(Tab)`) is a `{0}` format argument filled with the currently bound key, and Screen-layer UI navigates as a history stack (M / X = one step back).

## Background / Motivation
User (2026-09-12): "설정에서 단축키를 바꿀 수 있어야 돼. string 보니까 단축키가 [E] 이런 식으로 박혀 있더라고 — {0} 해서 내가 단축키를 바꾸면 같이 적용될 수 있는 식으로", then widened it: "여러 옵션들 넣을 수 있으니까 옵션 시스템도 같이 만들어야 돼 / plan, system 등 모든 문서도 다 만들고 진행하자", "우리 UI에서 레이어가 쌓이면 한 번에 닫히는 게 아니라 하나씩 뒤로 가는 거야. X 버튼도 만들어서… 공통 기능은 전부 UCL9Widget에서". Today there is no settings screen (`WBP_Pause` [설정] shows "준비 중"), IMC mappings carry no player-mappable names, and 11 String Table rows hard-code the key.

Track: **HEAVY** — new system + screen + UI navigation change; a 4-perspective panel (engine / UX / YAGNI / l10n) + cross-examination settled the design below against UE 5.8 source.

## In-scope
1. **Settings storage**
   - Keys: engine `UEnhancedInputUserSettings` (5.8 default-on, own SaveGame slot). No subclass.
   - Everything else: `UCL9GameUserSettings : UGameUserSettings` (`DefaultEngine.ini [/Script/Engine.Engine] GameUserSettingsClassName`). `config` fields: `MouseSensitivity` (1.0), `bInvertY` (false), `MasterVolume/EffectsVolume/VOVolume/UIVolume/AmbienceVolume/MusicVolume` (all 1.0). Language = engine key `[Internationalization] Culture` (same two lines as `cl9.lang`, moved into a shared function).
   - `UCL9SettingsTable` (DataAsset injected by `BP_CL9GameInstance`, like UITable): `SoundMix` + 6 `SoundClass` refs (+ optional `PreviewSound` slot, unused now). New assets `Content/Sounds/Master/` — `SC_Master/Effects/VO/UI/Ambience/Music`, `MasterSoundMix`. Design gains (Effects 0.8 · Ambience 0.5 · Music 0.7) live in `SoundClass.Properties.Volume`; slider defaults are all 1.0.
   - Apply: sensitivity/invert multiplied in `ACL9Character::LookInput`; audio via `SetSoundMixClassOverride` + `PushSoundMixModifier` in `ACL9PlayerController::BeginPlay` (local) and on slider change (not GameInstance Init — needs a world/audio device). Language via `ApplyLanguage`. Save **immediately on each confirmed change** (`UEnhancedInputUserSettings::SaveSettings()` after a key change; `UGameUserSettings::SaveSettings()` on slider release / toggle / language). Never call `ApplySettings()`/`ApplyNonResolutionSettings()` (touches process-wide Scalability in PIE).
2. **Player-mappable input assets**
   - Every IA that is rebindable gets `PlayerMappableKeySettings` (unique `Name`, `DisplayName` = String Table `Input.<Name>.Name`): Jump, Run, PickPlace, Throw, Use, Swap, Flashlight, Scan, Pause, SpectateNext. `IA_Move` → per-mapping `OverrideSettings` in `IMC_Gameplay`: `MoveForward/MoveBack/MoveLeft/MoveRight`. `IA_Look` and gamepad mappings (`Gamepad_*`) → `IgnoreSettings`. Stale `IA_Eat` mapping rows in `IMC_Gameplay` cleaned when saving. Reflect the peer session's current IMC state (LMB = `IA_Use`, G = `IA_Throw`, no `IA_Eat`).
   - Registration: `ACL9PlayerController::BeginPlay` (local) calls `UserSettings->RegisterInputMappingContexts({DefaultMappingContexts…, SpectateMappingContext, UITable->UIMappingContext})`. Never pass `bNotifyUserSettings` to Add/RemoveMappingContext.
   - `CL9_Gather.ini` Step1 includes `Content/Input/*` so a typed (non-table) DisplayName shows up as a GUID key.
3. **Key text helpers** — `Source/CL9/Core/CL9InputText.h/.cpp`, `namespace CL9Input`:
   - `FText GetKeyText(const FKey&)`: sheet row `Key.<FKeyName>` if the table has it, else `FText::AsCultureInvariant(Key.GetDisplayName(false).BuildSourceString())`. Initial `Key.*` rows: SpaceBar(Space), LeftShift(Shift), RightShift, LeftControl(Ctrl), LeftAlt(Alt), Tab, Enter, CapsLock, BackSpace, LeftMouseButton(LMB), RightMouseButton(RMB), MiddleMouseButton(MMB), ThumbMouseButton(M4), ThumbMouseButton2(M5).
   - `FText GetActionKeyText(const APlayerController*, const UInputAction*)`: profile row by `IA->GetPlayerMappableKeySettings()->Name`, first mapping whose key is **not** a gamepad key, → `GetKeyText`. Empty text if no row.
   - `ACL9Character` caches `PickPlaceKeyText` (on possess + `UserSettings->OnSettingsChanged`), formats once at the exit of `GetFocusPrompt`; tick compares the unformatted pattern and `OnSettingsChanged` clears `LastFocusPrompt`. `UCL9SpectateWidget` gets `NextText` (BindWidget) formatted in `NativeConstruct` with the SpectateNext key.
4. **Strings** (`Content/Strings/CL9_Strings.csv`, then `python Tools/Loc/sync_strings.py --build`)
   - `Prompt.*` (10 rows) → `[{0}] 줍기` / `[{0}] Pick up`; `Spectate.Next` → `다음 동료 ({0})` / `Next teammate ({0})`; ko and en in the same commit; DevNotes `{0}=현재 바인딩된 키 표기(예: E)`. Brackets stay in the string (translator's choice).
   - New areas: `Settings.*` (Title, Back, Section.Keys/Mouse/Audio/Language, ResetKeys, MouseSensitivity, InvertY, Volume.Master/Effects/VO/UI/Ambience/Music, PressKey, KeySwapped `{0}: {1} · {2}: {3}`, KeyRejected, CaptureHint), `Input.<MappingName>.Name` (14 rows), `Key.<FKeyName>`. `Pause.LobbyNotReady` new key (meaning changed); `Pause.NotReady` left in place.
   - Language choice labels are `FCulture::GetNativeName()` (한국어 / English) — documented String Table exception (a player who picked the wrong language must be able to read their own).
5. **Settings screen** — `UCL9SettingsWidget : UCL9Widget` + `WBP_Settings`, UI table row `Settings` (Screen). Pause [설정] → `OpenWidget(Settings)`. Rows are built in C++ at runtime (Shop-card style) with 4 builders: `AddSlider` / `AddToggle` (UCheckBox) / `AddChoice` (segmented — one button per option, current one highlighted) / `AddKey` (label + key button). Sections: Keys (with one [키 초기화] button = `ResetKeyProfileIdToDefault` + save), Mouse (sensitivity slider `SensitivityRange` UPROPERTY {0.2, 3.0}, step 0.05, shown as integer percent; Y-invert toggle), Audio (6 sliders), Language (segmented ko/en). No global reset. Language change does not rebuild rows (FText lazy rebuild handles it).
   - Key rows come from the registered IMCs in order (Gameplay → Spectate), using each mapping's name; the displayed/edited `FPlayerKeyMapping` is the first non-gamepad one in that row. A DisplayName that is not a `CL9` table reference → `LogCL9 Error` + key string shown.
   - **Key capture** = Lyra-style `IInputProcessor` registered at index 0 while capturing (button text `Settings.PressKey`). Accept keyboard keys and mouse buttons (on Down). Reject gamepad, mouse axes/wheel, Tilde, F11 → `Settings.KeyRejected` hint, capture continues. Cancel: **Esc, or 5 s without input**. All key/mouse events are consumed while capturing (M cannot close the screen). Processor holds the widget as `TWeakObjectPtr`, passes events through and unregisters itself if the widget is gone; the widget also unregisters in cancel / `OnClosing` / `NativeDestruct`. Rows born from `IMC_UI` (Pause) reject mouse buttons (overlay click would fire it).
   - **Conflict** = swap: find the first other row using the key; `MapPlayerKey` twice with `Slot`/`HardwareDeviceId` taken from the existing mappings and `bCreateMatchingSlotIfNeeded=false` (second call `bDeferOnSettingsChangedBroadcast=true`); hint `Settings.KeySwapped` with 4 args (action that got the new key, that key, action that gave it up, key it received). Hint clears on next capture start. Then `SaveSettings()`.
6. **UI navigation — Screen history stack** (`UCL9UIManager`)
   - `OpenWidget(Screen)`: if the name is already in the history, truncate to it; close the current screen widget; push; create. Only one Screen widget is alive at a time (create-on-open / destroy-on-close stays).
   - `CloseWidget(name)`: if `name` is the top → pop and re-open the previous entry (fresh instance); if it is deeper in the history → just remove it (e.g. Spectate closing while Pause is on top). `GetOpenScreen()` = top.
   - `PauseInput` (M): a Screen is open → `CloseWidget(top)` (one step back); none → `OpenWidget(Pause)`. The Spectate special-case in `PauseInput` goes away — Spectate sits in the history, so Spectate → Pause → Settings → M → M → Spectate, and Pause [계속] (`CloseSelf`) also returns to Spectate (fixes the existing gap).
   - `UCL9Widget` base gains: optional `CloseButton` (`BindWidgetOptional`, wired to `CloseSelf`) and `protected virtual void OnClosing()` called from `PlayOutro` (before the fade). `WBP_Settings` uses the base X; existing WBPs keep their own buttons (they already call `CloseSelf`).
7. **Cheats**: `cl9.resetkeys` (reset active key profile + save). `cl9.lang` body moves to the shared `ApplyLanguage`.
8. **Docs** (all Korean, per repo rules)
   - New `docs/plan/settings.md`: the options policy — what options exist and why, the "gameplay values are never options" rule, and the checklist a feature author runs to decide whether a feature needs an option (and where it goes). Linked from `docs/plan/README.md`.
   - Rewrite `docs/systems/settings.md` (storage, apply/save timing, screen, extension recipe = settings field + builder line + sheet row [+ SoundClass ref], PIE caveats: `Saved/SaveGames/EnhancedInputUserSettings.sav`, shared across PIE instances, one `UGameUserSettings` per engine, language in PIE is preview-only, `GameUserSettingsClassName` change resets stored resolution once).
   - `input.md` (rebinding section: registration in BeginPlay, slot rule, gamepad IgnoreSettings, capture rules, remove non-existent `IA_Interact` row), `localization.md` (key-format rule, `Key.*`/`Input.*`/`Settings.*` areas, `Key.*` fallback exception, native language name exception, `Content/Input` gather), `ui.md` (menu [설정] live, Settings screen section, X button), `ui_system.md` (history stack, `OnClosing`, `CloseButton`), `sound.md` (class gains, mix activation), `cheats.md`, `build-order.md` 4h status, `decisions.md` one entry (incl. Esc capture-cancel exception to 2026-09-05, M = one step back).

## Out-of-scope
- Graphics / accessibility (subtitles, UI scale) options — sections not added; recipe documented.
- Gamepad rebinding and gamepad glyphs.
- Multiple key slots per action, key profiles, per-PIE-instance save files.
- Title/lobby screen and the `ReturnScreen` generalisation beyond the history stack.
- Audio preview sound on slider release (no audio assets exist yet; `PreviewSound` slot only).
- Log-scale sensitivity, `[−][+]` buttons — after playtest.
- Steam-release lobby hints (`Enter: 준비`) — will reuse `GetActionKeyText` later.

## Acceptance Criteria
- [ ] `CL9Editor` builds; existing `CL9.*` automation tests pass.
- [ ] PIE: Pause → [설정] opens the Settings screen with Keys / Mouse / Audio / Language sections; M closes one step (Settings → Pause → game); X on Settings returns to Pause; Pause [계속] returns to game.
- [ ] PIE: click a key button → "키를 누르세요…" → press F → row shows F; the HUD prompt reads `[F] 줍기` immediately when looking at an item; M during capture does **not** close the screen; Esc or 5 s cancels; gamepad/wheel/Tilde/F11 show the rejected hint.
- [ ] PIE: bind Flashlight to E while PickPlace is E → rows swap, hint reads `손전등: E · 줍기: F`; both keys work in game accordingly.
- [ ] PIE: [키 초기화] restores defaults and prompt shows `[E]` again; `cl9.resetkeys` does the same from the console.
- [ ] Rebinding persists across PIE sessions (`Saved/SaveGames/EnhancedInputUserSettings.sav`), sensitivity/invert/volumes persist in `GameUserSettings.ini` under `[/Script/CL9.CL9GameUserSettings]`.
- [ ] PIE: sensitivity 200 % turns twice as fast; Y-invert flips pitch; volume sliders change `SoundMix` overrides (verify via `au.Debug.SoundMixes` or equivalent log).
- [ ] Language segmented control switches ko/en immediately (preview in PIE) and the Settings screen's own labels re-render.
- [ ] Spectating: M → Pause → [설정] → X → [계속] returns to the Spectate screen; Tab key text in Spectate reads the bound key.
- [ ] `sync_strings.py --build` passes validation (arg sets equal across ko/en); no non-table text in `Content/Input` gather output; `CL9_Conflicts.txt` empty.
- [ ] All docs listed in In-scope §8 updated; `decisions.md` entry present.

## Related Files / Modules
| File | Role |
|------|------|
| Source/CL9/GameMode/CL9GameUserSettings.h/.cpp (new) | config fields, ApplyAudio / ApplyLanguage, SetToDefaults |
| Source/CL9/UI/CL9SettingsTable.h (new) | DataAsset: SoundMix + 6 SoundClass refs |
| Source/CL9/GameMode/CL9GameInstance.h/.cpp | `SettingsTableAsset` soft ref + getter |
| Source/CL9/Core/CL9InputText.h/.cpp (new) | `CL9Input::GetKeyText / GetActionKeyText` |
| Source/CL9/UI/CL9SettingsWidget.h/.cpp (new) | screen, row builders, capture processor, swap |
| Source/CL9/UI/CL9Widget.h/.cpp | `CloseButton` optional, `OnClosing()` hook |
| Source/CL9/UI/CL9UIManager.h/.cpp | Screen history stack |
| Source/CL9/UI/CL9UITable.h | `CL9UINames::Settings` |
| Source/CL9/UI/CL9SpectateWidget.h/.cpp | `NextText` formatted with key |
| Source/CL9/GameMode/CL9PlayerController.h/.cpp | register IMCs, ApplyAudio, PauseInput = back |
| Source/CL9/Character/CL9Character.h/.cpp | key text cache, `GetFocusPrompt` format, LookInput sensitivity |
| Source/CL9/GameMode/CL9Cheats.cpp | `cl9.resetkeys`, `cl9.lang` via ApplyLanguage |
| Config/DefaultEngine.ini, Config/DefaultInput.ini, Config/Localization/CL9_Gather.ini | settings class, gather path |
| Content/Input/Actions/IA_* , IMC_Gameplay/UI/Spectate | PlayerMappableKeySettings |
| Content/UI/WBP_Settings (new), WBP_Pause, WBP_Spectate, UITable (asset, ex-DA_UITable) | screen assets, [설정] wiring, `NextText`, `Settings` row |
| Content/Sounds/Master/* (new), Content/GameMode/BP_CL9GameInstance, SettingsTable (new asset, no DA_ prefix — cl9-37 rename) | audio assets + injection |
| Content/Strings/CL9_Strings.csv + generated Localization | strings |
| docs/plan/settings.md (new), docs/plan/README.md, docs/systems/{settings,input,localization,ui,ui_system,sound,cheats}.md, docs/plan/build-order.md, docs/decisions.md | documentation |

## Must-Preserve
- Enhanced Input only; input local, judgement Server RPC (conventions § 2). Rebinding is purely local — nothing replicates.
- Esc is not a game key (2026-09-05) — the only new use is cancelling key capture inside the preprocessor.
- "No gameplay values as options" (settings.md) — nothing in the screen changes balance, darkness, or vision.
- UI table rule: new UI = WBP + table row; only one Screen widget alive; create-on-open / destroy-on-close; input mode + IMC_UI switching stays in `UpdateInputMode`.
- All user-facing text via String Table `CL9` (`LOCTABLE` literals so `sync_strings.py` can check code keys); no `NSLOCTEXT`. Existing keys keep their names.
- Save data separation: options are local (`GameUserSettings.ini` / EI `.sav`), run progress stays in `save.md`.
- Existing `cl9.lang` behaviour in editor (preview only, no save).

## Design Decisions (panel-settled — do not relitigate during implementation)
- IMC registration in `ACL9PlayerController::BeginPlay` via `RegisterInputMappingContexts` — `UEnhancedInputUserSettings` is created in `SetPlayer → ReceivedPlayer → PlayerControllerChanged`, i.e. after `SetupInputComponent`, so `AddMappingContext(bNotifyUserSettings)` silently registers nothing on the first PlayerController. (Rejected: bNotifyUserSettings on Add; registration in SetupInputComponent; `DefaultInput.ini DefaultMappingContexts` — asset path in config, silent failure on rename.)
- Rows are enumerated from IMC mapping order, not `GetPlayerMappingRows()` (TMap hash order; unregistered saved rows would appear with empty names). Displayed mapping = first non-gamepad key, not `Slot::First` (gamepad rows can take First by IMC order). (Rejected: a SettingsTable ordering array.)
- `MapPlayerKey` must carry `Slot` + `HardwareDeviceId` of the existing mapping and `bCreateMatchingSlotIfNeeded=false`, else it appends a second mapping and old+new keys coexist. Missing slot ends silently (no failure tag) — pre-check with `FindKeyMapping`.
- Key capture via `IInputProcessor` — UMG `NativeOnKeyDown` bubbles after `SButton` consumes LMB/Space/Enter, `UUserWidget` is non-focusable by default, and an unhandled overlay click hands focus to the viewport. (Rejected: NativeOnKeyDown/MouseButtonDown; NativeOnPreview* + bIsFocusable; "click the same button again" cancel — indistinguishable from binding LMB.)
- Capture cancel = Esc **or** 5 s idle (user 2026-09-12). Esc consumed by the preprocessor never reaches PIE stop.
- Key labels from sheet `Key.<FKeyName>` with English engine-name fallback — engine ko names mix scripts ("스페이스", "왼쪽 Shift", "LMB") and PIE preview does not load the Engine loc target, so PIE ≠ package. The fallback is why this is an allowed exception to the "no derived keys" rule. (Rejected: `FKey::GetDisplayName(false)` directly.)
- Conflicts swap across all rows as one group; first matching row only, others named in the hint. (Rejected: per-IMC conflict groups — premature generalisation; 2-arg hint — player needs to know which key the displaced action now has.)
- Volumes are 6 named floats (labels as `LOCTABLE` literals so the sheet check catches them), defaults 1.0, design gains in SoundClass. (Rejected: `TMap<FName,float>` + derived `Settings.Volume.<class>` keys — conflicts with the 2026-09-12 "no derived keys" decision; sliders showing 80 % invite players to "fix" the mix.)
- `UCL9SettingsTable` kept as a reference-only DataAsset — conventions § 5 allows a hard-coded path only for `MasterSoundMix`, not SoundClasses. (Rejected: dropping the table and iterating the mix's `SoundClassEffects`.)
- Save immediately on each confirmed change — `PlayOutro` is non-virtual and `NativeDestruct` runs after the fade; server-driven screens (`Client_OpenScreen`, dawn multicast) can evict Settings at any time. (Rejected: save on [뒤로]; save in NativeDestruct.)
- Segmented control for choices, `UCheckBox` for toggles, 4 row builders. (Rejected: ◀▶ arrows / cycling button — hide the other option; dropdown; 2 builders.)
- Only [키 초기화] in the Keys section; no global reset. (Rejected: global [기본값으로]; right-click / double-click row reset — hidden gestures.)
- Screen history stack in `UCL9UIManager` with M/X = one step back (user 2026-09-12) — replaces the Spectate special-case in `PauseInput` and fixes the [계속]-doesn't-restore-Spectate gap. (Rejected: `CloseSelf` + `OpenWidget(Pause)` hard-coded in Settings; `ReturnScreen` field.)
- Audio applied in `ACL9PlayerController::BeginPlay`, not `GameInstance::Init` (`SetSoundMixClassOverride` needs a world with an audio device).
- `cl9.resetkeys` cheat + documented `.sav` path — prompts now print the live key, so "왜 [F] 줍기지?" will recur in development. (Rejected: `UEnhancedPlayerInput` subclass for per-PIE-instance save files — when needed.)
- Interpretation note: the user's "esc 같은 공통 기능" is implemented as the **back key = M (IA_Pause)** handled uniformly through the history stack; Esc stays unused outside capture cancel.

## Execution Notes
- Recommended model: the strongest current Claude model (Claude Fable 5 tier / Opus 5) for the UIManager history stack, the input preprocessor lifetime, the swap logic against `UEnhancedInputUserSettings`, and the Korean docs. A cheaper model is acceptable for the String Table rows, `Key.*` table, and SoundClass asset creation.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Header changes → full `CL9Editor` build. Another session (`cl9-2d`) is using the editor for PIE verification of `IA_Use`; asset edits, editor kill and build must wait for its "끝" message. C++/CSV/docs can be written before that but must not be left uncompilable while its build may run.
- PIE checks flagged by the panel (verify, not assume): ① no cursor flicker when a screen closes and the previous one reopens in the same frame; ② overlay click with Pause bound to LMB closes the menu (basis of the mouse-button rejection for `IMC_UI` rows); ③ the preprocessor is gone after PIE stop / travel; ④ English fallback width for unlisted keys.

## Existing Integration Contract
- `ACL9Character::GetFocusPrompt()` keeps its signature; HUD keeps reading it and `OnFocusPromptChanged`.
- `ICL9Interactable::GetPrompt` implementations keep returning the raw `LOCTABLE` pattern — formatting happens once in the character.
- `UCL9UIManager::OpenWidget / CloseWidget / IsOpen / GetOpenScreen` signatures unchanged; `OnRep_Spectating`, `Client_OpenScreen`, `DeriveOrderScreen` keep calling them.
- `cl9.lang` console behaviour unchanged for users of the cheat.
