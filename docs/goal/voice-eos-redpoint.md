# goal: voice-eos-redpoint

Status: drafted 2026-10-12 — awaiting owner approval. **2026-10-16 정정**: C++ 쪽은 구현·컴파일 검사가 끝났다 — `UCL9GameInstance::BP_HostLobby/BP_FindAndJoinLobby/BP_LeaveLobby` 와 `NotifyLobbyReady/Joined/Left`, `ACL9PlayerState::BP_ApplyVoicePlacement`, 마이크 감도 줄 제거(Execution Notes 참조). Redpoint 플러그인은 아직 `CL9.uproject` 에 없고(W0), 블루프린트 브리지와 귀로 듣는 검증이 남아 있다. 위 "drafted" 는 구현 전에 쓴 줄이다. Supersedes the transport half of [voice-chat.md](voice-chat.md) (implemented 2026-10-11 on the engine's built-in VOIP); that document's rules, options, HUD and data decisions carry over unchanged unless a line below says otherwise.

## One-line Goal
Swap the voice **transport and playback layer** from Unreal's built-in VOIP (Opus over `UVoiceChannel`, listen-host relay, `UVOIPTalker`) to **EOS Voice (EOS RTC) through Redpoint's EOS Online Framework, Free Edition (Blueprint-only)** — so that echo cancellation, noise suppression and auto gain control exist — while keeping every game-side rule, option, HUD element, data cell and cheat that `voice-chat` delivered, with one rule simplification: **while a hotline is on, the speaker's voice is anchored at the receiver's walkie-talkie only** (owner's "option ①").

## Background / Motivation
Owner (2026-10-11 night → 2026-10-12), after the built-in VOIP was verified by logs:
- "그거 없으면 안 돼, 있어야 돼" — echo cancellation / noise suppression are **mandatory**. The engine's Voice module has only a noise gate and silence detection (source-verified: `Engine/Source/Runtime/Online/Voice/Private/Windows/VoiceCaptureWindows.cpp` — DirectSound capture, `voice.MicNoiseGate*`, nothing else). EOS RTC has echo cancellation, noise suppression and AGC **on by default** (`EOS_RTC_SetSettingOptions` doc comment in `ThirdParty/EOSSDK/SDK/Include/eos_rtc_types.h:372-380`).
- Research (2026-10-12): no shipped UE title was found using the built-in VOIP. Epic's own `EOSVoiceChat` plugin leaves 3D placement unimplemented (`EOSVoiceChatUser.cpp:652` `Set3DPosition` — "Unimplemented"; receives **mixed** audio, `bUnmixedAudio = false`). MECCHA CHAMELEON (UE5, 2-person team, 20M copies, Steam ↔ Switch 2 cross-play voice, zero server cost) ships `OnlineSubsystemRedpointEOS` + `EOSSDK-Win64-Shipping.dll` under `Binaries/Win64/RedpointEOS/`, and its executable imports `EOS_RTC_JoinRoom`, `EOS_RTCAudio_AddNotifyAudioBeforeRender`, `EOS_RTC_SetSetting` with Redpoint symbols `VoiceChatProximityType`, `VoiceChatAttenuationModel`, `EchoCancellation`, `NoiseSuppression` — i.e. Redpoint does the per-talker 3D placement Epic's plugin lacks.
- Owner: "1번으로 가자. 명세 써줘. 무료로 하지 뭐" — option ① (one anchor per speaker; hotline moves the anchor to the receiver's walkie-talkie), **Free Edition**.

Owner-settled decisions (2026-10-12):
1. Transport: **EOS Voice via Redpoint EOS Online Framework, Free Edition** (Blueprint API only — no C++ headers ship with the Free Edition).
2. Hotline: **option ①** — while connected, the speaker's voice plays at the receiver's walkie-talkie position only; people near the speaker do not hear them for that time. Everything else about the hotline (bidirectional, one hotline per person, receiver's corpse still rings, leaver disconnects) stays as contract §15.
3. Rules, options (5), HUD, strings, cheats, CL9Rules cells that are still meaningful, and the spectator mute table **carry over**.
4. Monster hearing stays out of scope (contract §16 — the monster spec decides later).

### Verified current state (recon 2026-10-12)
Repository (all uncommitted, run `voice-chat`):
- Voice code lives in `Source/CL9/GameMode/`: `CL9VoiceTalker.{h,cpp}` (`UCL9VoiceTalker : UVOIPTalker`), `CL9PlayerState.{h,cpp}` (replicated `FCL9VoiceRoute`, `HostSetVoiceRoute`, `Server_SetVoiceRoute`, `OnRep_VoiceRoute` → `RefreshVoiceSettings`, `RepointVoiceTalker`, `IsVoiceTalking`), `CL9GameMode.{h,cpp}` (`RebuildVoiceMutes` + `Voice mute:` log, hooks in `HandlePlayerDeath`/`ReviveDead`, applied via `GameplayMutePlayer`), `CL9GameUserSettings.{h,cpp}` (`ApplyVoice`: `voice.*` cvars, `VoiceClass` volume mix override, `SyncLocalTalker`, `ApplyVoiceTalking`), `CL9GameInstance.{h,cpp}` (Null-OSS session: `HostGame`/`JoinGame("lan")`/`LeaveGame`, `HasVoiceSession`, `IsLocalTalkerRegistered`, `SyncLocalTalker`), `CL9Cheats.cpp` (`cl9.voice`, `cl9.talk`, `cl9.voiceloop`, `cl9.voiceroute`), `CL9PlayerController` (`TalkAction` → talking state).
- Engine-API-bound lines: ~40 of ~470 voice-related lines (grep 2026-10-12). Everything else is rules, replication, UI, data.
- Wiring (W0 of `voice-chat`): `CL9.uproject` enables `OnlineSubsystem`, `OnlineSubsystemUtils`, `OnlineSubsystemNull`; `CL9.Build.cs` adds `OnlineSubsystem`, `OnlineSubsystemUtils` (Public) and `SignalProcessing` (Private); `DefaultEngine.ini` `[OnlineSubsystem] DefaultPlatformService=Null`, `bHasVoiceEnabled=true`, `[Voice] bEnabled=true`, `[/Script/Engine.AudioSettings] VoiPSoundClass=/Game/Sounds/Master/SC_Voice.SC_Voice`; `DefaultGame.ini` `[/Script/Engine.GameSession] bRequiresPushToTalk=true`.
- Assets: `SC_Voice`, `T_Icon_mic`, `IA_Talk` (+ R in `IMC_Gameplay`/`IMC_Spectate`), `BP_CL9PlayerController.TalkAction`, `WBP_HUD` `MicIcon`/`TalkingBox`, `SettingsTable.VoiceClass`, `CL9Rules` 13 voice cells (`VoiceSpectatorVolume`, attenuation ×5, sensitivity endpoints ×2, mic processing ×5).
- Blueprints that exist: `Content/GameMode/BP_CL9GameInstance`, `BP_CL9GameMode`, `BP_CL9GameMode_Flat`, `BP_CL9PlayerController`. **No `BP_CL9PlayerState`** — one must be created for the Blueprint bridge (decision 3 below).
- Docs: `docs/plan/voice.md` (design, unchanged by this goal except the transport line and the hotline rule), `docs/systems/voice.md` (implementation — rewritten by this goal), `docs/testing/pie-scenarios.md` Z section, `docs/goal/steam-release.md:452-458` (voice note that must be re-dated), `docs/systems/session.md` (Null session text).
- Steam: no AppID yet (`steam-release` not started). Redpoint's Steam identity provider needs the project's own AppID (480 is not accepted by EOS). Until then development signs in through the **Developer Authentication Tool** with Epic accounts.

### Redpoint / EOS facts (docs.redpoint.games, read 2026-10-12 — implement against these, verify R6/R7 first)
| # | Fact | Source |
|---|---|---|
| R1 | Free Edition: no C++ headers; supports only the latest engine (UE 5.8 stable since 2026-06-18); Fab listing condition is an annual income threshold (listing summary: under 30k USD) — confirm on the listing before installing. | `docs/systems/voice_chat/`, changelog 2026.06.18, Fab |
| R2 | Voice channels ("RTC rooms") come in two kinds: **managed** channels are created alongside an **EOS lobby** and players are connected by default; **unmanaged** channels need a dedicated server to mint tokens. ⇒ On a listen server the voice channel is the lobby's managed channel. | `docs/systems/voice_chat/` |
| R3 | Listen servers must use **P2P** connections; P2P requires every player to be signed into **their own** EOS account. IP connections allow one account but do no NAT traversal. | `docs/setup/networking/p2p_vs_ip/` |
| R4 | Redpoint Voice Chat component: per-player **Proximity Type** `None` / `Possessed Pawn` / `Custom` (server-set anchor actor), an **Attenuation Settings** asset when proximity ≠ None, **voice chat group** → automatic channel, "Silenced when no Anchor Point" setting (2026-07-10), anchor recomputed after respawn. | `docs/framework/voice_chat/`, changelog |
| R5 | Audio features toggled globally: `EnableEchoCancellation`, `EnableNoiseSuppression`, `EnableAutoGainControl`, `EnableDtx`, `EnablePlatformAEC` (Blueprint: *Set/Is Global Voice Chat Audio Feature Enabled*). Per-player local mute `SetPlayerMuted`; input volume 0–200 (only 0 silences); device enumeration/selection; transmit routing `TransmitToAllChannels`/`TransmitToNoChannels`/`TransmitToSpecificChannel`; talking state by polling `IsPlayerTalking` ("only works with ≥2 people in a channel"). | `docs/ossv1/voice_chat/control/` |
| R6 | **No documented voice-activity sensitivity / threshold and no input level meter** in the control API. The "마이크 감도" slider and the level bar may have no backend — see Open Questions 2. | same page (absence) |
| R7 | Developer Authentication Tool: PIE contexts map to credentials `Context_1..N`; each credential must be a **distinct Epic account** in the organisation; default credential `Default`/`DevAuthToolCredentialName`; port 6300. Credential selection for standalone `-game` windows is **not documented**. | `docs/auth/dev_auth_tool_usage/` |
| R8 | Steam identity provider requires the game's own Steamworks AppID (session tickets; 480 rejected). Auth graph auto-picks Steam when present; `OnlineSubsystemSteam` can stay packaged (MECCHA CHAMELEON ships both). | `docs/setup/platforms/steam/` |
| R9 | EOS Developer Portal: product + client with **Peer2Peer** policy (listen-server games); some features need Custom policy with extra permissions; Epic Account Services app with Online Presence / Friends / Country permissions is required by the plugin. | `docs/` getting started |

## Design decisions (derived from the owner decisions and R1–R9)
1. **Session = EOS lobby via Redpoint (replaces the Null-OSS LAN session now, and the Steam session planned by `steam-release` later).** `UCL9GameInstance::HostGame/JoinGame/LeaveGame` keep their names and callers (session.md "개발용 전송이다" rule); their bodies delegate to Blueprint events implemented in `BP_CL9GameInstance` with Redpoint/Online Subsystem Blueprints nodes (create lobby with voice enabled → open level `listen` over P2P; find/join lobby → client travel; leave → destroy lobby). `JoinGame(ip)` stays for address joins (no voice, logs the existing `Voice: no session` line). `HasVoiceSession()` = "a lobby with a managed voice channel is joined".
2. **Blueprint bridge (Free Edition has no C++ API).** C++ keeps all decisions and exposes them to Blueprint as `BlueprintImplementableEvent`s; a new `BP_CL9PlayerState` (subclass of `ACL9PlayerState`, set as `PlayerStateClass` on both GameMode BPs) and `BP_CL9GameInstance` implement them with Redpoint nodes. Events (names final, signatures gameplay's call):
   - PlayerState (runs on every machine for every player): `BP_ApplyVoicePlacement(EProximity Kind /*None|Pawn|Anchor*/, AActor* Anchor, USoundAttenuation* Attenuation)`; `BP_ApplyVoiceMute(bool bMuted)` (listener-local mute of this player); `BP_ReadVoiceTalking() → bool` (polled on a timer; replaces `IsVoiceTalking`'s source).
   - GameInstance (local user): `BP_ApplyVoiceOptions(bool bEnabled, bool bPushToTalk, float InputVolume, bool bTransmitting)`; `BP_ApplyVoiceAudioFeatures(bool bEcho, bool bNoise, bool bAGC)`; `BP_SetVoiceInputDevice(FString Id)` (may stay unwired — Out-of-scope 1 of `voice-chat` still holds).
   - Push-to-talk = `TransmitToAllChannels` while R is held, `TransmitToNoChannels` otherwise; voice activity = always transmitting (EOS DTX/VAD decides what is sent — see R6).
   - **Correction 2026-10-12 (as implemented by the `gameplay` role of this run; the rest of this decision stands):** the signatures above changed in four places.
     (a) `BP_ApplyVoicePlacement`'s `Kind` is the new enum `ECL9VoicePlacement { None, Pawn, Anchor }`.
     (b) Mute is applied from the **listener's** PlayerState: `BP_ApplyVoiceMute(APlayerState* Speaker, bool bMuted)` — not `BP_ApplyVoiceMute(bool)` on the speaker's PlayerState.
     (c) Options are five arguments: `BP_ApplyVoiceOptions(bool bEnabled, bool bPushToTalk, bool bTransmit, float InputVolume, float OutputVolume)` (volumes 0..200; `bTransmit` true → `TransmitToAllChannels`, false → `TransmitToNoChannels`) — the four-argument form above is obsolete.
     (d) `BP_SetVoiceInputDevice(FString)` was **not** created (device selection stays out of scope).
     Decision 1's lobby bridge got its names: on `UCL9GameInstance`, `BP_HostLobby(int32 MaxPlayers)`, `BP_FindAndJoinLobby()`, `BP_LeaveLobby()` (each must finish by calling the BlueprintCallable `NotifyLobbyReady(bool)`, `NotifyLobbyJoined(bool, FString ConnectAddress)`, `NotifyLobbyLeft()` on both its success and its failure path).
     Open Question 2: the implementation applied its default (b) — the 마이크 감도 row and the level bar are removed; whether the installed Redpoint version has a threshold or level node (the W0 check) is still pending the lead. Canonical description: [../systems/voice.md](../systems/voice.md) "블루프린트 브리지".
3. **Placement rules → Redpoint proximity, one anchor per speaker (option ①):**
   | Speaker state | Proximity Type | Anchor | Attenuation |
   |---|---|---|---|
   | alive, no hotline | Possessed Pawn | — | `SA_CL9_Voice` |
   | alive, hotline to T | Custom | T's current pawn (alive) or T's last pawn/corpse (dead); none → silenced (R4 setting) | `SA_CL9_VoiceSpeaker` |
   | dead | None (2D) | — | — |
   The host decides (`HostSetVoiceRoute` unchanged, contract §15 bidirectional pairing unchanged); every machine applies through `BP_ApplyVoicePlacement` on `OnRep_VoiceRoute`, `OnPawnSet`, `OnRep_Hp` edges — the same call sites `RefreshVoiceSettings` uses today. The receiver hears the speaker at their own pawn, which is the 2D-at-the-receiver feel of §14 without a second placement.
4. **Spectator mute table unchanged** (living listener × dead speaker = muted; dead hear dead in 2D; dead hear living in 3D from the spectated camera — automatic because Unreal's audio listener follows the view target). Host computes pairs in `RebuildVoiceMutes` as today; the application line changes from `GameplayMutePlayer` to a replicated per-listener mute list the client applies with `BP_ApplyVoiceMute` (Redpoint `SetPlayerMuted` is listener-local). Log lines `Voice mute: <listener> ± <speaker> (<reason>)` keep their wording.
5. **Attenuation becomes two assets** (`SA_CL9_Voice`, `SA_CL9_VoiceSpeaker` — spatialised, sphere, no occlusion, no plugins) because Redpoint takes an asset, not numbers. The five `CL9Rules` attenuation cells are **removed** (values move into the assets; `docs/plan/data/rules.md` rows deleted with a dated note). `VoiceSpectatorVolume` stays (applied as output volume × spectator factor through `BP_ApplyVoiceOptions`). The five mic-processing cells and the two sensitivity-endpoint cells are **removed** (EOS owns gating; see Open Questions 2 for the slider).
6. **Options (settings screen) keep their five rows**: voice on/off, 말하기 방식 (voice activity / push-to-talk), 마이크 감도, 마이크 증폭, 음성 채팅 볼륨. 증폭 → EOS input volume (0–200 mapped from the slider); 볼륨 → `SC_Voice` mix override **if** Redpoint renders through Unreal audio components (verify in W1; otherwise EOS output volume). 감도 and the level bar depend on Open Questions 2.
7. **Development sign-in = Developer Authentication Tool with one Epic account per client**; the owner creates 2–3 Epic accounts in the EOS organisation. Steam sign-in is `steam-release`'s (needs the AppID). Address joins keep working without EOS for non-voice testing.
8. **Wiring**: remove the engine-VOIP ini lines (`bHasVoiceEnabled`, `[Voice]`, `VoiPSoundClass`, `bRequiresPushToTalk`), drop `OnlineSubsystemNull` from `CL9.uproject` only if Redpoint does not need it (keep otherwise), add `OnlineSubsystemRedpointEOS` (Free Edition from Fab/Redpoint licensing), set the EOS product IDs in Project Settings (owner creates the product — Open Questions 1), keep `OnlineSubsystemSteam` disabled until `steam-release`. `SignalProcessing` stays only if the level meter survives (Open Questions 2).
9. **What is deleted**: `CL9VoiceTalker.{h,cpp}`, `RefreshVoiceSettings`'s VOIPTalker/attenuation-object body, `SyncLocalTalker`/`IsLocalTalkerRegistered` (replaced by lobby-joined state), `ApplyVoice`'s `voice.*` cvar writes, `cl9.voiceloop` (engine loopback; Redpoint's echo test is marked unreliable — replace with the T1 two-account check), Null session create/find/join code, engine facts E1–E13 as implementation guidance (kept in `voice-chat.md` as history).

## In-scope
1. **W0 — accounts & plugin (owner + lead)**: EOS product/client (Peer2Peer policy, voice/RTC enabled), Epic Account Services app with the three permissions, 2–3 developer Epic accounts in the organisation, Redpoint Free Edition installed under `Plugins/`, Project Settings IDs, Developer Authentication Tool credentials `Context_1..3`. Build passes with the plugin.
2. **W1 — session (gameplay + BP)**: decision 1; `HasVoiceSession` semantics; `Voice: session …` log lines re-pointed to lobby events; `cl9.host <alias>` / `cl9.join lan` cheats keep working (`lan` now means "find lobby").
3. **W2 — placement & mute bridge (gameplay + BP)**: decisions 2–4; `BP_CL9PlayerState`; `SA_CL9_Voice`/`SA_CL9_VoiceSpeaker`; hotline option ①; dead = None; mute list replication + listener-local apply; talking-state polling feeding `IsVoiceTalking` → HUD `TalkingBox`/`MicIcon` unchanged.
4. **W3 — options (gameplay + ui + BP)**: decision 6; `ApplyVoice` rewritten to call the GameInstance bridge events; audio features echo/noise/AGC on at startup (logged once: `Voice: audio features echo=1 noise=1 agc=1`); settings rows rebound; strings unchanged (ko only).
5. **W4 — data & wiring (gameplay + balance + art)**: decision 5 and 8; `CL9Rules` cells removed/kept as listed; `rules.md` rows; `create_voice_assets.py` extended to create the two attenuation assets (keeps `SC_Voice`, `T_Icon_mic`, `VoiceClass`).
6. **W5 — docs (docs)**: `docs/systems/voice.md` rewritten (transport, bridge events, placement table, mute, options, Redpoint settings, dev sign-in, known limits), `docs/systems/session.md` (EOS lobby replaces Null; `steam-release` W1/W2 session items marked superseded with a dated correction in `steam-release.md`), `docs/plan/voice.md` (transport line + hotline rule ①, dated), `docs/testing/pie-scenarios.md` Z section rewritten for two-account testing, `docs/decisions.md` 2026-10-12 entry, `voice-chat.md` Status line "superseded by voice-eos-redpoint (transport)", `docs/goal/README.md` row.
7. **W6 — verification** (below).

## Out-of-scope
- Walkie-talkie item, shop, switch on/off, drop/pick-up (next spec — "연결 = 켜짐" stays).
- Monster hearing of voice (contract §16).
- Steam sign-in, AppID, packaging, Switch/console (`steam-release`; Redpoint Paid Edition for consoles).
- Mic device selection UI, per-player volume sliders, world name tags, voice recording/reporting, text chat, dedicated servers, unmanaged channels.
- Any C++ against Redpoint (Free Edition has none) — if a need appears, it is a Paid Edition decision for the owner.
- Translations (ko only).

## Acceptance Criteria
- [ ] **A1 session**: two standalone windows on one PC signed in as two Epic accounts (Dev Auth Tool), `cl9.host henhouse` + `cl9.join lan` → both in one lobby over P2P, logs `Voice: session created …` / `Voice: session joined …`; address join logs `Voice: no session — remote voice disabled on this connection`.
- [ ] **A2 hear**: with headphones, each window hears the other; speaking shows the HUD mic icon locally and the talker's name in `TalkingBox` on both.
- [ ] **A3 proximity**: full volume inside `SA_CL9_Voice` inner radius, silent beyond its falloff (same 150/1500 cm draft values as before); audio comes from the speaker's pawn.
- [ ] **A4 options**: 말하기 방식 switch works (PTT: nothing is heard unless R is held); 증폭 and 볼륨 sliders audibly change level; settings persist across restart; 감도 / level bar per Open Questions 2.
- [ ] **A5 spectator**: `cl9.damage 99 1` → living stop hearing the dead (`Voice mute: H - B (dead)`), dead hear living in 3D from the spectated camera at ×`VoiceSpectatorVolume`; two dead hear each other in 2D; `cl9.night` from inside the truck → `Voice mute: H + B (revive)` and 3D returns (Lvl_Henhouse listen 2-player PIE as measured 2026-10-11).
- [ ] **A6 hotline ①**: `cl9.voiceroute direct <n>` → both logs `Voice route: … -> direct(…)`; A's voice is heard by B at B's position and by C only when C is near **B** (not near A); switching partners disconnects the old pair; the receiver's corpse still rings; a leaver disconnects the pair.
- [ ] **A7 audio features**: with speakers instead of headphones on one window, the other window does not hear its own voice returned (echo cancellation) — owner's ear; `Voice: audio features echo=1 noise=1 agc=1` logged.
- [ ] **A8 no engine VOIP left**: no `UVOIPTalker`/`IOnlineVoice`/`voice.*` cvar references in `Source/CL9`; ini voice lines removed; build has no `OnlineSubsystemUtils` voice dependency.
- [ ] **A9 docs**: W5 complete; `pie-scenarios.md` Z section runs end-to-end on the new stack.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/GameMode/CL9PlayerState.{h,cpp}` | route replication, placement/mute bridge events, talking poll |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | `RebuildVoiceMutes` (unchanged) + mute-list replication |
| `Source/CL9/GameMode/CL9GameInstance.{h,cpp}` | session functions → lobby bridge; options bridge events |
| `Source/CL9/GameMode/CL9GameUserSettings.{h,cpp}` | `ApplyVoice` rewrite |
| `Source/CL9/GameMode/CL9VoiceTalker.{h,cpp}` | **deleted** |
| `Source/CL9/GameMode/CL9Cheats.cpp` | `cl9.voiceloop` removed; others unchanged |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}`, `Content/Data/CL9Rules.uasset`, `docs/plan/data/rules.md` | cell removals (decision 5) |
| `Content/GameMode/BP_CL9PlayerState` (new), `BP_CL9GameInstance`, `BP_CL9GameMode*` | Blueprint bridge, `PlayerStateClass` |
| `Content/Sounds/Master/SA_CL9_Voice`, `SA_CL9_VoiceSpeaker` (new) | attenuation assets |
| `Tools/Unreal/create_voice_assets.py` | extended for the two assets |
| `CL9.uproject`, `Source/CL9/CL9.Build.cs`, `Config/DefaultEngine.ini`, `Config/DefaultGame.ini` | wiring (decision 8) |
| `Plugins/OnlineSubsystemRedpointEOS/` (new, Free Edition) | transport |
| `docs/systems/voice.md`, `docs/systems/session.md`, `docs/plan/voice.md`, `docs/testing/pie-scenarios.md`, `docs/decisions.md`, `docs/goal/voice-chat.md`, `docs/goal/steam-release.md`, `docs/goal/README.md` | W5 |

## Must-Preserve
- Public names and callers: `HostGame`/`JoinGame`/`LeaveGame`/`HasVoiceSession`, `HostSetVoiceRoute`/`Server_SetVoiceRoute`/`RequestVoiceRoute`, `IsVoiceTalking`, `RebuildVoiceMutes`, `UCL9GameUserSettings` voice fields and `ApplyVoice`, cheats `cl9.voice`/`cl9.talk`/`cl9.voiceroute`/`cl9.host`/`cl9.join`.
- Log wording oracles: `Voice mute: … ± … (dead|revive|…)`, `Voice route: … -> prox|direct(…)`, `Voice: session …`, `Voice: no session …`.
- Still exactly **one** new Server RPC in the voice feature (`Server_SetVoiceRoute`); no new Multicasts; no dedicated-server branches.
- HUD widgets `MicIcon`/`TalkingBox`, `IA_Talk` + R bindings, string keys, `SC_Voice`, `SettingsTable.VoiceClass`.
- Contract §15 hotline semantics except the placement simplification (option ①); §16 (no monster hearing).
- Other runs' uncommitted changes in shared files (kitchen, intro, giant-stage WIP) — edit surgically, commit per role via `/cl9 commit`.

## Verification (one PC, 2–3 Epic developer accounts, one mic, headphones; internet required)
- **T0** editor PIE 2–3 clients with Dev Auth Tool credentials `Context_1..3` → A1 (log), A5/A6 (log), HUD.
- **T1** two standalone `-game` windows → A2, A3, A4, A6 by ear. Credential selection for standalone windows is undocumented (R7): the run must find the mechanism (command-line/ini) or fall back to PIE "Run Under One Process off" with 2 contexts and report which worked.
- **T2** one window on speakers → A7 by ear.
- Logs per window as before (`CL9_2.log`…); evidence under `.claude/runs/voice-eos-redpoint/evidence/`.

## Known weak points (measure first, do not assume)
- Talking state is **polling only** and needs ≥2 people in the channel (R5) — HUD may lag; pick a poll interval (draft 0.1 s) and verify the mic icon for the local player when alone (fallback: PTT key state / local input level if any).
- Hotline anchor on a **corpse** relies on "last pawn" surviving until the lobby revive; if the pawn is destroyed earlier, the "Silenced when no Anchor Point" path applies — log it.
- P2P over EOS replaces the direct listen connection — session.md's "개발용 전송" rule now points at Redpoint's NetDriver; measure join time and tick rate once.
- Free Edition tracks only the latest engine — an engine minor update may require a plugin update first.

## Execution Notes
- Recommended model: Claude Fable 5.1 for W1–W3 (Blueprint bridge design, option-① placement edge cases, mute replication) — the decisions sit at the boundary between C++ and a Blueprint-only API and need judgment. Sonnet acceptable for W4 (cell removal, asset script) and W5 (docs).
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Run through `/cl9 task` as one run `voice-eos-redpoint` with roles gameplay (C++ + BP bridge), ui (settings rebinding), art (assets script), balance (asset values / cell removals), docs; lead does W0 with the owner and all editor work (Blueprint graphs are lead/MCP work — roles cannot open the editor).
- Do not start W1+ before W0 is confirmed in a running editor (plugin loads, Dev Auth Tool signs in `Context_1`).

## Open Questions
1. **EOS product**: who creates the product/client/Epic Account Services app in the Developer Portal — the owner (needs the Epic account that owns the organisation) — and the 2–3 developer Epic accounts for testing. Blocking for W0.
2. **마이크 감도 slider and mic level bar**: Redpoint's documented controls expose input volume (0–200) but no voice-activity threshold or level meter (R6). Options: (a) keep 감도 only if a Redpoint/EOS node exists in the installed version (check first), else (b) remove the 감도 row and the level bar and keep 증폭 (input volume) + the two modes. Lead checks in W0 and reports; default if nothing exists = (b).
3. **Volume path**: whether Redpoint plays voice through Unreal audio components (then `SC_Voice`/`VoiceClass` mix override still works) or through EOS's own output (then use EOS output volume). Decided by W1 inspection, no owner input needed.
4. Keep `OnlineSubsystemNull` enabled for non-voice offline testing of `JoinGame(ip)`? Default yes unless Redpoint conflicts.
