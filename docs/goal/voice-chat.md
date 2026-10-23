# goal: voice-chat

Status: implemented 2026-10-11 (run `voice-chat`) — log oracles pass; human-ear checks pending; monster hearing (decision 6, In-scope 7, A7) withdrawn by the owner. Several decisions below were changed after approval — see **Results → Post-approval changes**; `docs/systems/voice.md` is the canonical description of what was built. **Superseded (transport) by [voice-eos-redpoint.md](voice-eos-redpoint.md), 2026-10-12** — rules/options/HUD/data decisions still apply.

## One-line Goal
Mic voice chat over the engine's own VOIP path (mic → Opus → listen host relay → per-talker playback) delivered as a **routing capability plus its interface**: a *proximity* route (3D at the talker's head, heard only nearby) and a *direct hotline* route (one speaker → one chosen listener, 2D), Lethal-style spectator rules, voice as monster-audible noise, options (on/off, voice-activity vs push-to-talk, sensitivity, mic gain, volume), a HUD "who is talking" indicator, and a Null-OSS development session so all of it is testable today without Steam. No item, UI or presentation chooses the route yet — a public API and a cheat do.

## Background / Motivation
Owner (2026-10-10): "우리 게임은 협동 게임이고, 인게임 마이크도 구현해야 돼. 리썰에도 마이크 있잖아. 그런 거 구현하기 전에 어떻게 해야 하는지 문서 쓰자. 구현은 문서 보고 Opus 시킬게."
Owner (2026-10-11), scoping: "여기서는 마이크니까 인터페이스랑 기능만 만드는 거지. 특정인과 핫라인으로 연결되는 거 하나 필요하고, 주변에만 마이크 나가는 거 이런 거. 이걸 무전기로 할지 말지 이런 건 다음 얘기지."

Owner-settled decisions (AskUserQuestion, 2026-10-10/11):
1. Routes: **proximity (3D) + direct hotline to one chosen player**. Function and interface only; whether a walkie-talkie item, a global channel or anything else drives the route is the next spec.
2. Default mic mode: **voice activity**, push-to-talk as an option.
3. Spectators: **Lethal rule** — dead players hear each other in 2D; living players do not hear the dead; the dead hear the living in 3D from the spectated camera.
4. Monsters hear voice as noise: **in scope**.
5. Session prerequisite: **a Null-OSS development session is part of this spec** (verifiable now; the Steam session from `steam-release` later replaces the same functions).
6. Walkie-talkie purchase/shop: **deferred** (follows from 1).
7. Document language: English body, owner quotes in Korean.

### Verified current state (recon 2026-10-10)
Repository:
- No voice/mic code, config, plugin or module anywhere. `OnlineSubsystem`, `OnlineSubsystemUtils`, `OnlineSubsystemNull/Steam`, `SteamSockets` are all unwired (`CL9.uproject`, `Source/CL9/CL9.Build.cs`, `Config/DefaultEngine.ini`, `Config/DefaultGame.ini`).
- `docs/goal/steam-release.md:452` (awaiting approval, not started) lists "voice chat" as out of scope — this spec fills that gap and must not depend on steam-release landing first.
- Session: `UCL9GameInstance::HostGame/JoinGame/LeaveGame/CancelJoin` (`Source/CL9/GameMode/CL9GameInstance.*`) are a dev-only IP path — `OpenLevel(…, "listen")` and `ClientTravel(ip)`, compiled out in Shipping (`docs/systems/session.md:42-60`). **No OSS session (`CreateSession`) exists.** No `AGameSession` subclass (engine default, MaxPlayers 16). `ACL9GameMode` has no `PostLogin`/`Logout` override.
- Sound: `SC_VO` is the *character reaction voice* class (planned `VoiceSet`), not mic voice. The only attenuation asset is `SA_CL9_Kitchen` (200→1000 cm). `CL9Sound::PlayAt` has no attenuation (`docs/systems/sound.md`). AudioSettings `VoiPSoundClass` is unset. Mix/class references live in `UCL9SettingsTable` (`Content/Data/SettingsTable`).
- Options: audio section has six sliders (`Master/Effects/UI/Ambience/Music/VOVolume`). Recipe "옵션 추가" in `docs/systems/settings.md`: a config field on `UCL9GameUserSettings` (+ `SetToDefaults`), one builder line in `UCL9SettingsWidget::Rebuild()` (`AddSlider/AddToggle/AddChoice/AddKey`) with `LOCTABLE("CL9","Settings.<Name>")`, one row in `Content/Strings/CL9_Strings.csv`. Volume sliders map to a SoundClass via `SettingsTable` + `ApplyAudio`.
- Input: keys R, V, T, Q, C are free (`docs/systems/input.md`). A new `UInputAction` with `PlayerMappableKeySettings` shows up in the key-rebind screen automatically. IMCs: `IMC_Gameplay`, `IMC_UI`, `IMC_Spectate`.
- Noise/wave: `ACL9GameState::HostReportNoise(ECL9NoiseSource, Location, Radius, Instigator)` (`Source/CL9/GameMode/CL9GameState.h:78`, host only) → `Multicast_Noise` (radius recomputed per machine from `WaveTable`). Adding a noise source = one `ECL9NoiseSource` value (`Source/CL9/Data/CL9Types.h:108`) + one `HostReportNoise` call + one `WaveTable` row (`docs/systems/wave.md:495`); a Rules-driven radius also needs an `ECL9NoiseRadiusSource` value (`Data/CL9WaveTable.h:29-44`) and a case in `FX/CL9WaveRenderer.cpp:123-161 ResolveRadius`. Wave rows have `RepeatInterval`/`MinInterval`. Radii are in cm (`CL9Rules` Sound category: `RunNoiseRadius` 390, `PickupLoudRadius` 390, …).
- **Monsters do not consume noise events.** `ACL9Monster::SenseTick` (`Source/CL9/Monsters/CL9Monster.cpp:984-987`) polls `Distance <= Rules->RunNoiseRadius && IsRunning()`. `docs/plan/wave.md:187-191` (open item 1) keeps event-based hearing as a separate decision. `MonsterDef.HearRadius` is declared but never read.
- HUD: **no teammate name tags exist.** World→screen projection examples: `RefreshItemLabel`, `RefreshCookGauges`, `RefreshHead` (`Source/CL9/UI/CL9HUDWidget.h:323-349`). Spectate widget shows the spectated player's name (`CL9SpectateWidget`).
- Death/revive/spectate hooks: `ACL9GameMode::HandlePlayerDeath` (.h:144), `BeginSpectating` (.h:237), `ReviveDead` (.h:243); `ACL9Character::HostRevive` (.h:178), `HostKill`, `IsDead`; `ACL9PlayerController::bSpectating` (owner-only replicated), `HostSetSpectating`.
- `ACL9PlayerState` replicates `Hp/MaxHp/Slots/ActiveSlot/HeadItem` at 20 Hz; `GetSlotColor()` gives the slot tint.
- `ACL9Character`: `FirstPersonCameraComponent` (`CL9Character.h:1245`), `FirstPersonMesh`, `HeldItem`, `WornItem`. No head socket constant.
- Conventions: §2 host authority, `Server_*` only on owned actors, `Multicast_*` only for instantaneous events; §9 sound playback only in the presentation layer, sync by replicated server time, the only sound multicast is `Multicast_Noise`; §5 no asset paths in C++. Standing decisions: waves are never an option (`docs/plan/wave.md:169-178`); "walking has aggro 0" (`docs/plan/data/rules.md:26`).
- Reference games: Lethal spectator voice rules (`docs/reference/lethal/lethal-frontend.md:364-371`), Lethal options (mic on, device, voice-activity/PTT, level meter — `:43,72-73,178`), distance/wall/radio filters (`lethal-run.md:608`); REPO monsters hear voice chat as noise, radius 5, 1 s cooldown (`docs/reference/repo/repo-monsters.md:98`); REPO audio tab (proximity voice 75, mic device, mic volume 100, PTT off — `repo-frontend.md:165`).

### Engine facts (UE 5.8 source, verified 2026-10-10 — implement against these, do not re-derive)
Engine root: `C:\Program Files\Epic Games\UE_5.8` (the `D:\…` path in `CLAUDE.md` is stale; `GenerateProjectFiles.bat:10` is right). Paths below are relative to `Engine/`.

| # | Fact | Where |
|---|------|-------|
| E1 | **Steam voice is the generic engine VOIP path.** `FVoiceEngineSteam : FVoiceEngineImpl` only adds `ISteamFriends::SetInGameVoiceSpeaking` on start/stop. Capture = `Voice` module (Windows: DirectSound `FVoiceCaptureWindows`), codec = Opus. Steam's own voice API is not used → Null OSS runs the **same code**; everything here is verifiable without Steam. | `Plugins/Online/OnlineSubsystemSteam/Source/Private/VoiceEngineSteam.cpp:21-44`, `Source/Runtime/Online/Voice/Private/Windows/VoiceCaptureWindows.*`, `VoiceCodecOpus.*` |
| E2 | **Packets ride the net connection's `UVoiceChannel`.** A packet arriving at the server is re-sent to every other connection by `UNetDriver::ReplicateVoicePacket`. The listen host is the hub; no separate P2P voice path, SteamSockets or not. | `Source/Runtime/Engine/Private/VoiceChannel.cpp:29-58`, `NetDriver.cpp:1684-1730` |
| E3 | **Mute is a server-side relay filter.** `UNetConnection::ShouldReplicateVoicePacketFrom(Sender)` → `APlayerController::IsPlayerMuted`. `GameplayMutePlayer / GameplayUnmutePlayer(FUniqueNetIdRepl)` (server) cut a listener↔speaker pair. Packets are dropped until `MuteList.bHasVoiceHandshakeCompleted` (`ClientVoiceHandshakeComplete`). | `NetConnection.cpp:5525-5553`, `Classes/GameFramework/PlayerController.h:980-1008`, `VoiceChannel.cpp:65-68` |
| E4 | **Remote talkers are registered by the OSS *session* in `RegisterPlayers`.** `APlayerState::RegisterPlayerWithSession` calls `RegisterPlayer` only if `DoesSessionExist(SessionName)`. → **Without `CreateSession`, nobody's voice is ever played** (root cause of the forum "voice does not work over direct IP"). `DestroySession` → `RemoveAllRemoteTalkers`. | `Plugins/Online/OnlineSubsystemNull/Source/Private/OnlineSessionInterfaceNull.cpp:858,473`, `OnlineSubsystemSteam/…/OnlineSessionInterfaceSteam.cpp:1405,200`, `Source/Runtime/Engine/Private/PlayerState.cpp:367-381` |
| E5 | **The local talker is registered there too** (`RegisterLocalTalker(Index)` for local players inside the session's `RegisterPlayers`); that is when the capture device is created. Nothing in the engine calls `RegisterLocalTalkers()` on its own. → **Without a session the mic is never even opened.** | `OnlineSessionInterfaceNull.cpp:843`, `OnlineSessionInterfaceSteam.cpp:1503`, `Plugins/Online/OnlineSubsystemUtils/…/Private/VoiceEngineImpl.cpp:359-367` |
| E6 | **Spatialised playback = `UVOIPTalker`** (alive in 5.8). `RegisterWithPlayerState(PS)`; `Settings.ComponentToAttachTo` + `Settings.AttenuationSettings` make the engine attach a `UVoipListenerSynthComponent` to that component → 3D. Unset → 2D. **Settings are read when a talker goes active (next utterance after ≥1 s silence), not live.** One synth per remote talker → a talker cannot be 2D and 3D at once. `GetVoiceLevel()`, `OnTalkingBegin(UAudioComponent*)`/`OnTalkingEnd()` (BP events) for UI. | `Source/Runtime/Engine/Public/Net/VoiceConfig.h:45-139` (`FVoiceSettings`, `UVOIPTalker`, `UVOIPStatics`), `VoiceEngineImpl.cpp:590-617`, `VoiceEngineImpl.cpp:632-651` (1 s `GetRemoteTalkerTimeoutDuration`) |
| E7 | **Talking control** = `APlayerController::StartTalking/StopTalking/ToggleSpeaking(bool)` (exec) → `StartNetworkedVoice/StopNetworkedVoice`, which work regardless of `bRequiresPushToTalk`. `AGameModeBase::PostLogin` calls `ClientEnableNetworkVoice(!GameSession->RequiresPushToTalk())`. `[/Script/Engine.GameSession] bRequiresPushToTalk` defaults **true** (`BaseGame.ini:71`); the voice module **warns if the key is missing from DefaultGame.ini**. | `PlayerController.cpp:3760-3799`, `GameModeBase.cpp:986`, `OnlineSubsystemUtils/…/Private/VoiceInterfaceImpl.cpp:137-175,177-226`, `OnlineSubsystemUtils.cpp:377-380` |
| E8 | **Who is talking — no RPC needed on the host.** `IOnlineVoice::OnPlayerTalkingStateChanged(FUniqueNetIdRef, bool)` (period `VoiceNotificationDelta`), `IsLocalPlayerTalking(LocalUserNum)`, `IsRemotePlayerTalking(Id)`. The host is the relay hub, so it sees every living speaker. | `Plugins/Online/OnlineSubsystem/Source/Public/Interfaces/VoiceInterface.h:45,157,166,446` |
| E9 | **Mic processing cvars**: `voice.SilenceDetectionThreshold` 0.08, `voice.SilenceDetectionAttackTime` 2 ms, `voice.SilenceDetectionReleaseTime` 1100 ms; noise gate `voice.MicNoiseGateThreshold/MicNoiseAttackTime/MicNoiseReleaseTime`; `voice.MicInputGain`; `voice.MicStereoBias`; `UVOIPStatics::SetMicThreshold`. Debug: `voice.debug.PrintAmplitude`, `OSS.VoiceLoopback 1`, console `ONLINE VOICE DUMP`, `vcbitrate`/`vcvbr`/`vccomplexity`. | `Source/Runtime/Engine/Private/VoiceConfig.cpp:12-24`, `VoiceCaptureWindows.cpp:438-460`, `OnlineSubsystemUtils.cpp:817,963`, `VoiceEngineImpl.cpp:743-789` |
| E10 | **ini keys**: `[OnlineSubsystem] bHasVoiceEnabled=true` (default true), `MaxLocalTalkers`, `MaxRemoteTalkers`, `VoiceNotificationDelta`; `[Voice] bEnabled=true` (**default false — must be set**); `[/Script/Engine.AudioSettings] VoiPSampleRate=Low16000Hz|Normal24000Hz` (no 48 k) and `VoiPSoundClass` (soft path to the voice SoundClass); `[/Script/Engine.GameSession] bRequiresPushToTalk`. | `Config/BaseEngine.ini:93-94,2490-2495`, `Classes/Sound/AudioSettings.h:20-27,154-176`, `VoiceConfig.cpp:100` |
| E11 | **Mic device selection is not exposed.** `RegisterLocalTalker` creates the capture with device name `""` (Windows default). `IVoiceCapture::ChangeDevice` exists but nothing calls it. | `VoiceEngineImpl.cpp:363`, `Source/Runtime/Online/Voice/Public/Interfaces/VoiceCapture.h:93,121` |
| E12 | **Plugin locations**: `OnlineSubsystemSteam` (`Plugins/Online/`, EnabledByDefault false; depends on `OnlineSubsystem`, `OnlineSubsystemUtils`, `SteamShared`), `SteamSockets` and `SteamShared` (**`Plugins/Runtime/Steam/`**), `OnlineSubsystemNull`, `OnlineSubsystemUtils` (VOIP engine, `UVoipListenerSynthComponent`). `VoiceChat`/`EOSVoiceChat` (`IVoiceChat`) are the EOS stack — not used. Build.cs modules: `OnlineSubsystem`, `OnlineSubsystemUtils` (they pull `Voice`). | `Plugins/Online/OnlineSubsystemSteam/OnlineSubsystemSteam.uplugin`, `Plugins/Runtime/Steam/SteamSockets/SteamSockets.uplugin` |
| E13 | **Community pitfalls, cross-checked**: voice over direct IP never works (= E4/E5); with PTT nothing is sent until `ToggleSpeaking 1` (= E7); packaged Steam builds failing to init (AppID/DLL) is steam-release's problem; Steam Audio occlusion on the VOIPTalker attenuation crashed for one team (keep `SpatializationPlugin`/`OcclusionPlugin` empty as they are); 4+ simultaneous talkers dropped packets on a listen server (we cap at 4 — watch tick rate/bandwidth, see Known weak points). | forum threads, 2019–2025 |

## Design decisions (owner-settled + derived from the engine facts)
1. **Routing model — the interface this spec exists for.**
   ```
   UENUM() enum class ECL9VoiceRoute : uint8 { Proximity, Direct };
   USTRUCT() struct FCL9VoiceRoute { ECL9VoiceRoute Mode = Proximity; TWeakObjectPtr<ACL9PlayerState> Target; };   // Target used by Direct only
   ```
   `ACL9PlayerState::VoiceRoute` is **replicated, host-written**. Three ways to set it, nothing else:
   - `ACL9PlayerState::HostSetVoiceRoute(ECL9VoiceRoute Mode, ACL9PlayerState* Target)` — host-only gate (`HasAuthority`), the function future items/UI will call.
   - `ACL9PlayerController::Server_SetVoiceRoute(ECL9VoiceRoute, ACL9PlayerState*)` — the **one new Server RPC**, owner → host, so a local player can request its own route.
   - cheat `cl9.voiceroute prox|direct [slot]` (local player; `slot` = entry order index of the target, see cheats.md).
   *Who receives* is derived on the host as a mute table (E3): Proximity → every living player receives (distance is handled by attenuation); Direct → only `Target` receives, everybody else is gameplay-muted for that speaker. *How it sounds* is derived on each listening machine from the speaker's replicated route (E6): Proximity → `Talker.Settings.ComponentToAttachTo = speaker pawn's FirstPersonCameraComponent`, `AttenuationSettings = SettingsTable.VoiceAttenuation`; Direct-at-me → both null (2D). One playback per speaker, applied from the speaker's next utterance (E6) — accepted.
2. **Spectator rule (Lethal)** on the same two mechanisms. Mute table rows: living listener × dead speaker → muted; dead listener × anyone → not muted. Listener-side settings: dead listener hearing a dead speaker → 2D; dead listener hearing a living speaker → 3D attached to the living pawn (the spectator camera is the audio listener, so this is "3D from the spectated camera" for free); living listener hearing living → 3D or Direct as in 1. Spectator volume 0.8: apply `VoiceChatVolume × 0.8` to `SC_Voice` while the local player is dead (no extra mix asset).
   The host rebuilds the whole table in one function, `ACL9GameMode::RebuildVoiceMutes()`, called from: `HostSetVoiceRoute`, `HandlePlayerDeath`, `ReviveDead`/`HostRevive`, and new `PostLogin`/`Logout` overrides. It diffs against the current state and logs every change as `Voice mute: <listener> -/+ <speaker> (<reason>)`. The 4×4 table for (alive/dead) × (prox/direct) is written out in systems/voice.md.
3. **Talker lifetime.** `ACL9PlayerState` owns its `UVOIPTalker` (`CreateTalkerForPlayer` or a default subobject) and registers it once `UniqueId` is valid — server right after `SetUniqueId`, client in `OnRep_UniqueId`. On `APlayerState::OnPawnSet` (spawn, respawn after `ReviveDead`) every machine re-points `ComponentToAttachTo` to the new pawn's `FirstPersonCameraComponent`. Attenuation asset reference: **`UCL9SettingsTable::VoiceAttenuation` (TSoftObjectPtr<USoundAttenuation>)** — no asset path in C++ (§5).
4. **Session prerequisite (Null OSS, dev).** `UCL9GameInstance::HostGame` creates a Null session before `OpenLevel(listen)`: `CreateSession(0, NAME_GameSession, {bIsLANMatch=true, NumPublicConnections=4, bShouldAdvertise=true, bUsesPresence=false})`; `JoinGame` gains a LAN path: `FindSessions(LAN)` → `JoinSession` → `ClientTravel(resolved address)`; `LeaveGame` → `DestroySession`. The existing `JoinGame(ip)` stays as is, and logs one line `Voice: no session — remote voice disabled on this connection` when it is used (A10). ini: `[OnlineSubsystem] DefaultPlatformService=Null`. When `steam-release` W1/W2 lands, only the bodies of these three functions change (session.md:60 rule) — document that touch point in systems/voice.md.
   **Measured, not assumed:** two processes on one PC finding each other over the Null LAN beacon. If that fails, dev fallback: the joining client calls `CreateSession` locally (no listen) *and then* connects by address — E4/E5 only check that a session named `GameSession` exists on that machine.
5. **Talk modes.** Keep `bRequiresPushToTalk=true` in `DefaultGame.ini` (blocks the engine's automatic enable at PostLogin and silences the missing-key warning, E7) and let our code own the state: after the local talker is registered (E5 — poll `IsHeadsetPresent`/`IsLocalPlayerTalking` availability or react to the session's register-complete), *voice-activity mode* calls `StartTalking()` once and leaves it on (the silence detector gates what is sent, E9); *PTT mode* starts in `StopTalking()` and `IA_Talk` pressed/released calls `StartTalking/StopTalking`. `bVoiceChatEnabled=false` → `StopTalking()` plus `UnregisterLocalTalker(0)`; re-enable → `RegisterLocalTalker(0)`.
6. **Monster hearing — keep the polling pattern** (the event merge stays open, wave.md item 1). Host subscribes to `OnPlayerTalkingStateChanged` (E8) and stamps `ACL9PlayerState::LastTalkServerTime` (**host-local, not replicated** — monsters are judged on the host, the HUD uses Talker events locally). `ACL9Monster::SenseTick` adds one branch next to running: `Distance <= Rules->VoiceNoiseRadius && PS->IsTalking(Now)` (talking = stamped within `VoiceTalkHold`, draft 0.5 s). The wave ring: `HostReportNoise(ECL9NoiseSource::Voice, Location, Rules->VoiceNoiseRadius, PS)` on talking-begin and then every `RepeatInterval` (WaveTable row `voice`, draft 1.0 s = REPO cooldown) while talking. Route does not matter — noise is at the speaker's position only; dead speakers make no noise. New Rules cells: `VoiceNoiseRadius` (draft 300 cm), `VoiceTalkHold` (0.5 s); ensure header default = `CL9Rules` asset cell (rules.md convention).
7. **Audio assets.** New SoundClass **`SC_Voice`** under `SC_Master` (SC_VO stays for reaction voice), set as AudioSettings `VoiPSoundClass`; new attenuation **`SA_CL9_Voice`** (draft: 1.0 up to 150 cm, 0 at 1500 cm, natural-log or linear — the asset cell decides; keep the plugins empty, E13). `VoiPSampleRate=Normal24000Hz` (drop to `Low16000Hz` only if packet-drop shows up).
8. **Options** (audio section, settings.md recipe; all local, immediate, saved on change):
   | Field (`UCL9GameUserSettings`) | Control | Applies to |
   |---|---|---|
   | `bVoiceChatEnabled` (true) | AddToggle | register/unregister local talker (5) |
   | `VoiceChatMode` (VoiceActivity / PushToTalk) | AddChoice | Start/Stop ownership (5) |
   | `VoiceSensitivity` (0.08) | AddSlider | `voice.SilenceDetectionThreshold` |
   | `MicGain` (1.0) | AddSlider | `voice.MicInputGain` |
   | `VoiceChatVolume` (1.0) | AddSlider | `SC_Voice` via `SettingsTable` (+×0.8 while dead) |
   Applied in a new `ApplyVoice()` next to `ApplyAudio()`. The settings screen shows a live mic level line (`IVoiceEngine::GetMicrophoneAmplitude(0)` through `IOnlineVoice`, or `UVOIPTalker::GetVoiceLevel` of the local talker) so the player can see the mic works. Passes the plan/settings.md checklist: local-only, fair, immediate, no restart.
9. **Input.** `IA_Talk` (PTT), default key **R**, in both `IMC_Gameplay` and `IMC_Spectate` (the dead may talk), `PlayerMappableKeySettings` Name `Talk`, string `Input.Talk.Name`. No input changes the route in this spec.
10. **HUD.** No name tags exist, so the indicator is a **talking list** in a HUD corner: one line per currently-talking player (slot colour dot + player name), driven by each remote `UVOIPTalker::OnTalkingBegin/End`; the local player gets a mic icon while `IsLocalPlayerTalking(0)` (polled with the HUD's existing tick). Nothing replicated, nothing projected into the world.
11. **Conventions.** §9 gets an explicit exception: the voice stream is the engine `VoiceChannel`, not subject to server-time tick comparison; no new Multicast (the ring reuses `Multicast_Noise`); exactly one new Server RPC (`Server_SetVoiceRoute`). Dedicated server paths are not written (project has none).

## In-scope
1. **Wiring (W0).** `CL9.uproject`: enable `OnlineSubsystem`, `OnlineSubsystemUtils`, `OnlineSubsystemNull` (Steam plugins stay for steam-release W1). `CL9.Build.cs`: `OnlineSubsystem`, `OnlineSubsystemUtils`. `DefaultEngine.ini`: `[OnlineSubsystem] DefaultPlatformService=Null`, `bHasVoiceEnabled=true`, `MaxLocalTalkers=1`, `MaxRemoteTalkers=3`, `VoiceNotificationDelta=0.2`; `[Voice] bEnabled=true`; `[/Script/Engine.AudioSettings] VoiPSampleRate=Normal24000Hz`, `VoiPSoundClass=/Game/Sounds/Master/SC_Voice.SC_Voice`. `DefaultGame.ini`: `[/Script/Engine.GameSession] bRequiresPushToTalk=true`, `MaxPlayers=4`.
2. **Null OSS session** in `UCL9GameInstance` (decision 4) + the A10 log line on the sessionless IP path + cheats `cl9.host`/`cl9.join` keep working (`cl9.join lan` added for the LAN search path).
3. **Talker + 3D** on `ACL9PlayerState` (decision 3), `SettingsTable.VoiceAttenuation`, `SC_Voice`, `SA_CL9_Voice`.
4. **Talk modes + input** (decisions 5, 9): `IA_Talk`, `ACL9PlayerController` binds press/release, `UCL9GameUserSettings` fields, `ApplyVoice`.
5. **Routing API** (decision 1): `ECL9VoiceRoute`, `FCL9VoiceRoute`, `ACL9PlayerState::VoiceRoute` + `HostSetVoiceRoute`, `ACL9PlayerController::Server_SetVoiceRoute`, cheat `cl9.voiceroute`.
6. **Mute table** (decisions 1–2): `ACL9GameMode::RebuildVoiceMutes()`, `PostLogin`/`Logout` overrides, hooks in `HandlePlayerDeath`/`ReviveDead`/`HostRevive`, `Voice mute:` log lines; listener-side `ACL9PlayerState::RefreshVoiceSettings()` (recomputes every remote talker's `Settings` from routes + alive flags; called on `OnRep_VoiceRoute`, `OnRep_Hp` death/revive edges, `OnPawnSet`, own death/revive).
7. **Monster hearing + wave** (decision 6): Rules cells `VoiceNoiseRadius`, `VoiceTalkHold`; `ECL9NoiseSource::Voice`, `ECL9NoiseRadiusSource::VoiceNoise`, `ResolveRadius` case, `WaveTable` row `voice` (`RepeatInterval` 1.0, colour = instigator slot); `SenseTick` branch; host `OnPlayerTalkingStateChanged` subscription in `ACL9GameMode` (or GameState) stamping `LastTalkServerTime`.
8. **Options UI + HUD** (decisions 8, 10): five settings rows, mic level line, talking list widget in `WBP_HUD` / `CL9HUDWidget`.
9. **Strings (ko only)**: `Settings.VoiceChatEnabled`, `Settings.VoiceChatMode` (+ two choice labels), `Settings.VoiceSensitivity`, `Settings.MicGain`, `Settings.VoiceChatVolume`, `Settings.MicLevel`, `Input.Talk.Name`, `Hud.Talking`.
10. **Cheats** (cheats.md procedure): `cl9.voice` (wraps `ONLINE VOICE DUMP` + one line per player: route, alive, muted-by list), `cl9.voiceloop 0|1` (`OSS.VoiceLoopback`), `cl9.voiceroute prox|direct [slot]`, `cl9.talk on|off` (host: fakes `LastTalkServerTime` + fires the local Talker events — lets the automated PIE oracle exercise 7 and 8 without a mic).
11. **Docs**: new `docs/systems/voice.md` (routing model, mute table 4×4, talker lifetime, session dependency, options, cheats, known limits) + one row in both tables of `docs/systems/README.md`; `sound.md` (SC_Voice, SA_CL9_Voice, §9 exception), `settings.md`, `input.md`, `ui.md`, `cheats.md`, `wave.md` (source `voice`), `monster.md` (hearing line), `session.md` (Null session; fix "아직 없는 것"), `plan/data/rules.md` + `plan/data/wave.md` rows, `conventions.md` §9 exception, `decisions.md` 2026-10-11 entry, `steam-release.md:452` dated correction, `CLAUDE.md` engine path correction.

## Out-of-scope
- **Anything that *chooses* the route for the player** — walkie-talkie item, shop row, `UseAction`, global channel presentation, a UI to pick the hotline target. Next spec; its only touch point is `HostSetVoiceRoute` / `Server_SetVoiceRoute`.
- Mic device selection (E11), per-player volume sliders, distance/wall low-pass or pitch filters, world name tags, making waves optional (standing decision), EOS/Vivox/ODIN, translations (ko only), the Steam session itself / AppID / packaging (steam-release), merging monster senses onto noise events (wave.md item 1), voice recording or reporting, dedicated-server support.

## Acceptance Criteria
- [ ] A1 Build OK. `ONLINE VOICE DUMP` on the host shows the voice interface enabled, `Voice Requires Push To Talk: true`, one local talker registered after hosting.
- [ ] A2 Two standalone processes on one PC (`-game -windowed`; host via `cl9.host`, client via `cl9.join lan` or the title screen) both list one remote talker in the DUMP and hear each other. Fallback of decision 4 documented if the LAN beacon fails.
- [ ] A3 Proximity attenuation: full within 150 cm, inaudible beyond 1500 cm (human check); the attenuation asset values are logged at talker setup.
- [ ] A4 Switching `VoiceChatMode` applies immediately (voice-activity → PTT needs R; back → hands-free); sensitivity and gain sliders change the cvars (logged).
- [ ] A5 Spectator rule with three processes: dead A and dead B hear each other in 2D; living C does not hear A (`Voice mute: C - A (dead)` logged); A hears C in 3D; after `ReviveDead` the mutes are lifted (logged).
- [ ] A6 Routing: A runs `cl9.voiceroute direct <B>` → B hears A in 2D regardless of distance, C does not hear A (logged mute); A runs `cl9.voiceroute prox` → from A's next utterance C hears A in 3D again. `VoiceRoute` reads the same on all three machines (`cl9.voice`).
- [ ] A7 Monster hearing: with `cl9.talk on` on the host, an awake monster within `VoiceNoiseRadius` reacts through the same branch as running (log), a `voice` wave ring fires every 1.0 s while talking, a dead player's talking produces neither. Automated PIE (`Lvl_Flat`) oracle.
- [ ] A8 HUD: the talking list shows the remote talker on both machines while they speak and clears within ~1 s after; the local mic icon follows `IsLocalPlayerTalking`.
- [ ] A9 The five options persist across restart (`GameUserSettings.ini`) and the mic level line moves when speaking.
- [ ] A10 `cl9.join <ip>` (no session) logs `Voice: no session — remote voice disabled on this connection` and the game otherwise behaves as before.
- [ ] A11 No new `Multicast_*`; exactly one new Server RPC (`Server_SetVoiceRoute`); `grep` evidence in the Results. No dedicated-server branch added.
- [ ] A12 Docs of In-scope 11 updated.

## Related Files / Modules
| File | Role |
|------|------|
| `CL9.uproject`, `Source/CL9/CL9.Build.cs` | Plugins + modules (In-scope 1) |
| `Config/DefaultEngine.ini`, `Config/DefaultGame.ini` | E10 keys, `DefaultPlatformService=Null`, `bRequiresPushToTalk=true`, `MaxPlayers=4` |
| `Source/CL9/GameMode/CL9GameInstance.h/.cpp` | Null session create/find/join/destroy inside `HostGame/JoinGame/LeaveGame` |
| `Source/CL9/GameMode/CL9PlayerState.h/.cpp` | `UVOIPTalker` ownership, `VoiceRoute` (+`OnRep`), `HostSetVoiceRoute`, `RefreshVoiceSettings`, `LastTalkServerTime`, `IsTalking` |
| `Source/CL9/GameMode/CL9PlayerController.h/.cpp` | `IA_Talk` bind, Start/Stop ownership, `Server_SetVoiceRoute`, mute calls (server side) |
| `Source/CL9/GameMode/CL9GameMode.h/.cpp` | `RebuildVoiceMutes`, `PostLogin`/`Logout`, hooks in `HandlePlayerDeath`/`ReviveDead`, talking-state subscription |
| `Source/CL9/GameMode/CL9GameState.h/.cpp` | `HostReportNoise(Voice, …)` call site (or from GameMode) |
| `Source/CL9/GameMode/CL9GameUserSettings.h/.cpp` | Five fields, `ApplyVoice` |
| `Source/CL9/UI/CL9SettingsWidget.*`, `Source/CL9/UI/CL9HUDWidget.*`, `Content/UI/WBP_HUD` | Option rows + mic level, talking list |
| `Source/CL9/Character/CL9Character.h` | `FirstPersonCameraComponent` as attach target (read only) |
| `Source/CL9/Monsters/CL9Monster.cpp` | `SenseTick` voice branch |
| `Source/CL9/Data/CL9RulesDef.h`, `Content/Data/CL9Rules` | `VoiceNoiseRadius`, `VoiceTalkHold` |
| `Source/CL9/Data/CL9Types.h`, `CL9WaveTable.h`, `Source/CL9/FX/CL9WaveRenderer.cpp`, `Content/Data/WaveTable` | `ECL9NoiseSource::Voice`, `ECL9NoiseRadiusSource::VoiceNoise`, `ResolveRadius`, row `voice` |
| `Source/CL9/Data/CL9SettingsTable.h`, `Content/Data/SettingsTable` | `VoiceAttenuation`, `SC_Voice` reference for the volume slider |
| `Content/Sounds/Master/SC_Voice`, `Content/Sounds/Master/SA_CL9_Voice` | New assets (art) |
| `Content/Input/Actions/IA_Talk`, `Content/Input/IMC_Gameplay`, `Content/Input/IMC_Spectate` | PTT key R |
| `Content/Strings/CL9_Strings.csv` (+ `CL9.csv` via `sync_strings.py`) | Strings of In-scope 9 |
| `Source/CL9/…/CL9Cheats*` (per cheats.md) | `cl9.voice`, `cl9.voiceloop`, `cl9.voiceroute`, `cl9.talk`, `cl9.join lan` |
| `Content/Maps/Lvl_Flat.umap` | Verification level |

## Must-Preserve
- Host authority (§2); no new Multicast; the single new Server RPC is `Server_SetVoiceRoute`; no asset paths in C++ (§5 — `VoiPSoundClass` lives in ini, `VoiceAttenuation` in `SettingsTable`).
- §9 tick-comparison sync for every sound except the voice stream (documented exception).
- Waves never become an option; "walking has aggro 0" is untouched (voice is a new source, not walking); `RunNoiseRadius` and the other Sound cells unchanged.
- `SC_VO`, `VOVolume`, `SA_CL9_Kitchen` and the kitchen ambience untouched.
- The dev IP path (`cl9.join <ip>`, `JoinGame(address)`) keeps working, just without voice.
- `SpatializationPlugin` / `OcclusionPlugin` stay empty (E13).
- The uncommitted working tree (kitchen identity, intro video, giant-stage WIP) is neither reverted nor reformatted.
- Performance: no new ticking actor; HUD work rides the existing HUD tick; monster branch is one distance check per player per `SenseTick`.

## Verification (one PC, one Steam account, one mic, headphones)
- **T1 — one process (capture + codec).** `Lvl_Flat` with `-game`; `cl9.host` to create the session; `cl9.voiceloop 1` → you hear yourself (human); `cl9.voice` shows one local talker (oracle). Headphones are mandatory (speaker → mic feedback).
- **T2 — two or three processes on one PC (relay, 3D, spectator, routing).** Editor Play settings: *Run Under One Process* **off**, Play As Listen Server, 2–3 players — or launch `-game -windowed` windows by hand. A single-process multi-client PIE cannot judge playback (one capture device) — logs only. One mic captured by two processes is fine (DirectSound shared capture); to choose the sending window put the other one in PTT mode and do not press R. A talks, B listens: walk apart (A3), kill A (A5), `cl9.voiceroute direct` (A6, three windows), mode/slider changes (A4). Mute changes are oracle-readable from the `Voice mute:` lines.
- **T3 — automated PIE without a mic (monster, wave, HUD).** `cl9.talk on|off` on the host drives A7 and A8 through the existing PIE oracle (`docs/testing/pie-scenarios.md`), and stays as a regression test.
- **Human-only checks:** attenuation feel, 2D↔3D switch feel, sample-rate quality. Mark them like kitchen-identity A1/A5.
- **Steam path:** same code (E1) but needs a second account + PC → steam-release's acceptance, not this spec's.
- Order: W0 build → T1 → T2 (A2 → A3 → A5 → A6 → A4) → T3 (A7, A8) → A9 → A10 → A11 → docs (A12).

## Known weak points (measure first, do not assume)
1. Two processes on one PC discovering each other via the Null LAN beacon (decision 4 fallback if not).
2. Route / spectator switches take effect only from the speaker's next utterance after ≥1 s silence (E6). If this feels wrong in T2, the follow-up is a `FVoiceEngineImpl` subclass overriding `GetVoiceSettingsOverride` — out of scope here.
3. The full alive/dead × prox/direct mute table when the *host* is dead — all four rows must be exercised in T2 with three windows and logged.
4. Listen-server bandwidth with 3–4 simultaneous talkers (E13): if packets drop, first try `Low16000Hz`, then raise `[/Script/OnlineSubsystemUtils.IpNetDriver] MaxClientRate/MaxInternetClientRate`.

## Execution Notes
- Run through the project harness: `/cl9 task` — roles gameplay (wiring, session, Talker, routes, mute table, monster branch, Rules/Wave cells, cheats), ui (IA, options rows, mic level, talking list, strings), art (`SC_Voice`, `SA_CL9_Voice`, HUD mic icon), docs. Lead does BP/asset assignment, build, PIE gates.
- Recommended model: Opus 5.5 (or Fable 5.1) for gameplay; Sonnet is acceptable for ui strings/widget and docs. This document cannot enforce the model — the executing session's `/model` decides; if lower than recommended, tell the owner and confirm first.
- Build with the editor closed (save first); new `UPROPERTY`s → Full Rebuild. Editor/PIE/build locks per `docs/workflow/sessions.md`.
- Engine root is `C:\Program Files\Epic Games\UE_5.8` (fix `CLAUDE.md`, which says `D:\…`).
- Do not touch steam-release files beyond the dated out-of-scope correction; do not enable the Steam plugins in this goal.

## Open Questions
- Widen `Direct` to several targets (a channel)? Structurally `Target` → `TArray`; deferred until the item/UI spec needs it.
- `VoiceNoiseRadius` 300 cm draft — and whether talking should also wake sleeping monsters (`TryWake`) or only alert awake ones (current draft: awake only, like running).
- Attenuation curve numbers (150/1500 cm draft) and spectator 0.8 (multiply vs. a mix snapshot).
- PTT default key R — confirm.
- Steam switch-over: beyond `DefaultPlatformService=Steam` and the session bodies, what else (merge into steam-release W2 when that spec is approved).

## Results (2026-10-11, run `voice-chat`)

Run folder `.claude/runs/voice-chat/` (briefs, contract with §13–§16, five role reports, audit). Roles: gameplay·balance·art (opus), ui·docs (sonnet).
Builds: 2 — the first failed at link (`Audio::FPatchOutput::PopAudio` unresolved; the game-target compile check links monolithically and missed it), fixed by adding
`SignalProcessing` to `CL9.Build.cs` private dependencies; second build OK (144 s). The editor relaunched normally (no Smart App Control block).
Evidence logs: `.claude/runs/voice-chat/evidence/` — `t2_host.log`, `t2_client.log`, `t2_c2.log` (standalone windows), `a1_host.log`, `pie_voice.log` (PIE lines), `build.log`.

### Post-approval changes (owner decisions — contract §13–§16; these override the body above)
- **Hotline = a two-way link between two walkie speakers** (no earpiece mode). One link per player; a new link drops both players' old links; `prox` or a leave drops both.
  The speaker's surroundings always hear the speaker in 3D. The partner hears 2D; everyone else hears one placement — the speaker or the partner's walkie, whichever is
  louder at their ear (gain from each attenuation, re-picked every 0.25 s and on events, applied from the next utterance — E6). Routes never mute anyone.
- **The walkie is independent of the person**: while linked it plays at the partner's pawn even if the partner is dead (body). A dead *speaker* is still unheard by the living.
- `ECL9VoiceOutput`, `SA_CL9_Voice` and `SettingsTable.VoiceAttenuation` were never shipped: attenuation (voice and walkie, inner/silent-at/curve) and mic processing
  (silence attack/release, noise gate threshold/attack/release) are **CL9Rules cells**, and each machine builds the attenuation objects at runtime.
- The sensitivity slider is **"마이크 감도"** (higher = quieter sounds get through), mapped dB-linearly between two Rules cells (0.20 → 0.01, default 0.31 ≈ 0.079).
  The effective noise gate = min(gate cell, sensitivity threshold), so whatever is sent is audible to teammates.
- **Monster hearing withdrawn (§16)** — "몬스터 청각은 나중에 몬스터 쪽에서 음성 채팅을 사용할지 말지 결정하는 것". All noise/monster/wave code and table rows added by this run were reverted; `cl9.talk` is a local HUD-only switch.
- **E9 correction**: `voice.SilenceDetectionReleaseTime` 1100 ms is a 40 dB envelope decay, not "1.1 s more sending"; the real send tail at default sensitivity is 0.3–0.55 s.

### Acceptance
| # | Result | Evidence |
|---|---|---|
| A1 | **PASS (log)** | `a1_host.log`: `Voice: session created GameSession` → `Voice: local talker registered`; `cl9.voice` DUMP `Voice Requires Push To Talk: true`, Local Talkers `[0] … Registered: 1`. |
| A2 | **PASS (log); hearing = human pending** | Two `-game` windows on one PC: client `Voice: lan search found 1` → `session joined` → `connecting to 172.30.1.52:7777 (LAN session)`; both sides `RegisterRemoteTalker … 0x00000000` and `settings 3D@<p> (prox)` (R1 re-placement after `2D (no pawn)`). |
| A3 | **log PASS; feel = human pending** | `Voice: talker <p> attenuation VoiceAttenuation inner=150 falloff=1350 silentAt=1500 curve=…Linear` on every machine. |
| A4 | **human pending** | `Voice: apply … sensitivity=0.31 threshold=0.079 gate=0.079 …` at every apply; mode/slider changes and PTT (R) need the settings UI + a mic. |
| A5 | **PASS (log)** | PIE 3 players: host killed by chickens → `Voice mute: C1 - H (dead)`, `C2 - H (dead)` (host-dead row, weak point 3); further deaths unmute exactly the pairs where the listener died; all-dead → everyone `2D (dead speaker)`, no mutes. **Revive** (2026-10-11 night, `Lvl_Henhouse` listen-server 2-player PIE, host inside the truck, `cl9.night`): `'BP_CL9Character_C_1' revived at the truck.` → `Voice mute: H + C1 (revive)`, both talkers back to `3D@… (prox)`, `Voice: apply … volume=1.00`. Evidence `evidence/pie_revive.log`. (`Lvl_Flat` cannot show this — no truck → wipe.) |
| A6 | **PASS (log); 2D↔3D feel = human pending** | `Voice route: H -> direct(C1)` and `C1 -> direct(H)` on all three worlds; relink drops the old partner (`C1 -> prox`); `prox` drops both; second `prox` → `has no hotline to drop.`; partners `2D (direct)`, third player `3D@H (speaker)`; dead partner → corpse walkie `3D@<body> (spectator)`. |
| A7 | **out of scope (§16)** | Monster hearing withdrawn by the owner. |
| A8 | **PASS (local, log + widget); remote list = human pending** | `cl9.talk on` → `HUD talking: +H`, host `MicIcon` `HIT_TEST_INVISIBLE` (clients stay `COLLAPSED`); `off` → `HUD talking: -H`. Remote lines need real voice (T2). |
| A9 | **human pending** | Persistence across restart and the moving mic level line need the UI + a mic. |
| A10 | **PASS (log)** | `t2_c2.log`: `JoinGame: connecting to 127.0.0.1.` → `Voice: no session — remote voice disabled on this connection`, join otherwise normal. |
| A11 | **PASS** | Added lines in `Source/CL9`: exactly one `UFUNCTION(Server, Reliable, WithValidation)` = `Server_SetVoiceRoute` (`CL9PlayerController.h:140-141`, Validate always true — checks in `_Implementation`); `Multicast_` added = 0; no dedicated-server branch and no `/Game/` path in the voice C++. |
| A12 | **PASS** | `docs/systems/voice.md` (new) + README, sound, settings, input, ui, ui_system, cheats, session, conventions §9, steam-release dated note, pie-scenarios Z section, CLAUDE.md engine path. `monster.md`/`wave.md` unchanged after §16. |

### Known weak points — measured
1. **Same-PC LAN beacon: works** — `lan search found 1`, no fallback needed (decision 4 fallback not implemented).
2. Route switch from the next utterance: logic verified (re-pick every 0.25 s); **feel pending (human)**.
3. Mute table with the host dead: **verified** in PIE logs (see A5).
4. Bandwidth with 3–4 simultaneous talkers: **not measured** (needs real voices).

### Human checks pending (owner)
T1 `cl9.voiceloop 1` hears yourself · T2 two/three `-game` windows (`cl9.host henhouse` / `cl9.join lan`): hear each other, attenuation 150/1500 cm feel, hotline 2D↔3D and
corpse-walkie feel, spectator ×0.8, settings mode/sensitivity/gain/PTT (A4), persistence + mic level line (A9), packet drops with 3 talkers. Procedure: `docs/testing/pie-scenarios.md` Z.
Note: `flat` has no Level → host with `henhouse`; headphones are required.

### Follow-ups found
- `Tools/Unreal/create_voice_assets.py` looks up `voice_class`, but on `CL9SettingsTable` the Python name is `VoiceClass` → it logged "absent - skipped"; the lead set the cell by hand. Fix the script name before reuse.
- The existing "음성" slider (`Settings.Volume.VO`, character voice) sits next to "음성 채팅 볼륨" — consider renaming it "캐릭터 음성" (out of scope).
