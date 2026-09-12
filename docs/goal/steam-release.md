<!-- /autoplan restore point: /c/Users/user/.gstack/projects/Work/master-autoplan-restore-20260906-220719.md -->
# goal: steam-release

Status: DRAFT (/autoplan review in progress)
Date: 2026-09-06 · Branch: master

## One-line Goal
Put "양계장 습격" (CL9) on Steam as a 1–4 player listen-server co-op: Steamworks
account + AppID, engine wiring so the Steam overlay appears and two PCs connect
through Steam's relay, the session layer (host / friend invite / join code /
lobby / start), and a build-upload loop — with **no servers to operate**.

## Background / Motivation

The user asked three things (2026-09-06):

1. "스팀에 올릴 건데 뭐 해야 돼? 스팀 오버레이 같은 거 뜨게 해야 되잖아" — what
   is needed to get onto Steam, overlay included.
2. "사람들이 서로 어떻게 알고 방에 들어가?" — how players find each other and
   join a room.
3. "서버 운영을 어떻게 해야 할지 모르겠어" — how to run servers.

**Answer to 3, up front: there are no servers to run.** The project already
decided on a Steam *listen server* (docs/conventions.md § 2, docs/plan/overview.md):
one player hosts inside their own game, the others connect to that host through
Steam's relay network (Steam Datagram Relay, via the engine's SteamSockets net
driver). Valve runs the relay for every Steam game at no charge. No port
forwarding, no hosting bill, no dedicated-server binary, nothing to keep alive at
night. The price of this model is already accepted in docs/systems/session.md:
host leaves = session ends, no host migration.

**Answer to 2:** Steam presence. The host creates a Steam *lobby session*; friends
see "Join Game" on the host in their Steam friends list / overlay, or the host
sends an overlay invite. Because the game is "play with people you know"
(session.md: no random matchmaking, no public server list), the second path is a
6-character **join code** the host reads out; the client types it and the game
searches Steam lobbies for that code.

**Answer to 1:** Steamworks paperwork (account, app fee, AppID, store page) plus
four engine changes (enable two plugins, two Build.cs modules, one ini block).
The overlay appears as soon as the game initializes the Steam API while the Steam
client is running — the OnlineSubsystemSteam plugin does that; nothing else is
needed for the overlay itself.

### Verified current state (2026-09-06)

- **Zero Steam or session code in the project.** `CL9.uproject` enables no Online
  plugins; `Config/DefaultEngine.ini` has no `[OnlineSubsystem*]` or
  `NetDriverDefinitions`; `Source/CL9/CL9.Build.cs` lists no OnlineSubsystem
  module; no `IOnlineSession` / `CreateSession` / `ServerTravel` / `?listen`
  anywhere under `Source/`; `CL9TuningHash` exists only in docs.
- **Save system is in progress in the concurrent gameplay session, uncommitted**
  (re-checked 2026-09-06 22:16): `Source/CL9/GameMode/CL9SaveGame.h` (untracked)
  and `ACL9GameMode::SaveRun/LoadRun` (`CL9GameMode.cpp:723-780`), with
  `SaveRun()` called every dawn in `EnterLobby` (`:260`). This plan does not
  read or show the slot (v1 roster has no 새 판/이어하기 — design decision 30);
  T9 makes that work land before W1 so the two do not collide in `CL9GameMode`.
- Multiplayer so far is tested only inside PIE with two clients
  (docs/testing/pie-scenarios.md A3). Build-order step 1f "2인 리슨 접속" was
  closed on that basis — two separate game processes have never connected.
- The design is already written: docs/systems/session.md (logo → title → mode
  select → lobby; two join paths: Steam friend invite + join code; dev AppID 480;
  presence lobby session; TuningHash check on join; host leaves = session dies;
  mid-night join left undecided).
- **Engine 5.8 probe** (`D:\Program Files\Epic Games\UE_5.8\Engine`):
  `Plugins/Online/OnlineSubsystemSteam` (v1.2, Steamworks SDK **v1.64** bundled at
  `Source/ThirdParty/Steamworks/Steamv164`); `Plugins/Runtime/Steam/SteamSockets`
  (SDR net driver) and `SteamShared` present; `Plugins/Online/SocketSubsystemSteamIP`
  is IP-only ("does NOT use NAT punchthrough") — not the one we want.
  `Config/BaseEngine.ini` defaults: `[OnlineSubsystemSteam] bEnabled=false,
  SteamDevAppId=0, bInitServerOnClient=false, bAllowP2PPacketRelay=true,
  bVACEnabled=1`; a `[/Script/SteamSockets.SteamSocketsNetDriver]` section already
  exists with sane timeouts.
- `ACL9GameState` already replicates night phase / day / map seed / orders /
  money; `ACL9PlayerState` replicates Hp and carry slots. Neither has the session
  fields session.md plans (`bIsReady`, `SlotIndex`, `SelectedCharacter`,
  session state, session code).
- `ACL9GameMode::StartNight()` already is "the game starts" (the `cl9.night`
  cheat calls it); `EnterLobby()` handles the between-nights stop. The whole
  night lifecycle lives in **one level** — there is no per-night level travel.

## In-scope

Work items W0–W5. W0 is paperwork and runs in parallel from day one; W1 → W2 →
W3 → W4 → W5 are sequential. W4 needs W0's AppID.

### W0 — Steamworks registration (no code; start now — the clock is the long pole)
- **Now:** answer Open Questions 1–2, register as a Steamworks partner
  (identity, tax and bank forms — tax verification takes days; per-app fee
  USD 100, credited back after USD 1,000 revenue), create the app → **own
  AppID**. Paying the fee starts Valve's mandatory 30-day wait.
- Verified release sequence (Steamworks docs, checked 2026-09-06 — reconfirm
  on partner.steamgames.com when W0 starts): fee → 30-day wait → store page
  review (3–5 business days) → "Coming Soon" page live ≥ 14 days → build
  review (3–5 business days) → release. Floor ≈ 7 weeks after the fee.
- **Later, separate item (CEO voice finding 4):** the Coming Soon store page.
  It needs capsule art, screenshots and a description — assets this plan
  excludes and ⑤ (art) has not produced. It is not needed for W1–W4: the
  beta-branch upload and test keys work on an unreleased app. Whether Steam
  Playtest can be attached before the page is live is **unverified** —
  decide Playtest vs. test keys at W4 with the Steamworks UI in front of you.
- Steamworks settings that code depends on: Installation → launch option
  (Win64 exe), Application → multiplayer/co-op category flags, and the default
  networking. Steam Datagram Relay is provided to Steam partners at no charge
  per Steamworks documentation (reconfirm at W0).
- Korean prerequisites (사업자등록·게임제작업 등록, GRAC rating vs Steam's IARC
  route) — **Open Question 1**; verify, do not assume.

### W1 — Engine wiring: overlay shows, two PCs connect (this is what 1f meant)
- `CL9.uproject`: enable `OnlineSubsystemSteam` and `SteamSockets` (they pull
  `OnlineSubsystem`, `OnlineSubsystemUtils`, `SteamShared`).
- `Source/CL9/CL9.Build.cs`: add `OnlineSubsystem`, `OnlineSubsystemUtils`
  (public). Game code talks only to the `IOnlineSession` / `IOnlineIdentity`
  interfaces — no `steam_api.h` in `Source/CL9`.
- `Config/DefaultEngine.ini`:
  ```ini
  [/Script/Engine.GameEngine]
  !NetDriverDefinitions=ClearArray
  +NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SteamSockets.SteamSocketsNetDriver",DriverClassNameFallback="/Script/OnlineSubsystemUtils.IpNetDriver")

  [OnlineSubsystem]
  DefaultPlatformService=Steam

  [OnlineSubsystemSteam]
  bEnabled=true
  ; Spacewar test AppID — replaced by the own AppID in W4
  SteamDevAppId=480
  ; engine default; NOT needed for a SteamSockets listen host (see below)
  bInitServerOnClient=false
  ```
  (Comments on their own lines only — UE's ini parser treats just a
  line-leading `;` as a comment; `true ; note` would be parsed as the value.)
  The `[/Script/SteamSockets.SteamSocketsNetDriver]` block stays on the engine
  defaults. **Why PIE keeps working (AC7), verified in 5.8 source:** OSS Steam
  disables itself in the editor unless running `-game`/dedicated
  (`OnlineSubsystemSteam.cpp:481-489`, `UE_EDITOR` block); the SteamSockets
  module logs "SteamSockets: Disabled for editor process." and never enables
  the socket subsystem (`SteamSocketsModule.cpp:16-21, 55-56`); and
  `USteamSocketsNetDriver::IsAvailable()` (`SteamSocketsNetDriver.cpp:41-50`)
  returns false → the engine instantiates `DriverClassNameFallback` =
  IpNetDriver. Putting the definition under `[/Script/Engine.GameEngine]` (read
  by game runs, not by `UEditorEngine`) is belt-and-braces on top of that.
  **W1's first check**: PIE 2-client after the ini change, and the log lines
  above (T14).
  Verified in the 5.8 plugin source (2026-09-06, second pass after the spec
  review contradicted the first): `SteamSocketsSubsystem.cpp:107-139` creates
  the Steam *server* handle and the `OnServerLoginComplete` delegate only
  `if (bIsDedicated)`; on a client process `IsLoggedInToSteam()` (`:809-813`)
  returns `SteamUser()->BLoggedOn()`, and `SteamSocketsNetDriver.cpp:84-92` takes
  the delayed-listener path only when that is false. A listen host with a
  logged-in Steam client therefore opens its P2P listen socket immediately;
  `bInitServerOnClient` stays `false`. It matters only if W1's kill criterion
  triggers and the legacy `OnlineSubsystemSteam.SteamNetDriver` is used instead.
  `bUseSteamNetworking` (default true, Epic docs) makes SteamSockets the default
  socket subsystem; leave it.
- **Known engine bug UE-239936** (SteamSockets cannot create a listen socket
  during *non-seamless server travel* while the previous driver's socket info is
  still registered; community fix confirmed on 5.4, and 5.8 still has no
  world-cleanup hook calling `CleanSocketInformation` — only the periodic clean
  and a dev-only console command). Our flow has no in-session level travel,
  but 나가기 on the host (`OpenLevel(Lvl_Title)` with the live listen driver)
  followed by 방 만들기 is the same "old driver shut down → new listen socket"
  sequence the bug describes. The periodic clean
  (`SteamSocketsSubsystem.cpp:507`) runs during the seconds spent on the title,
  so the risk is lower than an immediate `ServerTravel`, not zero — **AC2 must
  be repeated twice in one process** (host → 나가기 → host again, T7). If it
  reproduces, the game-side workaround is to wait a few world ticks on the
  title before re-hosting; the engine patch is not an option on the launcher
  build.
- `steam_appid.txt` (480) is written by the plugin in non-Shipping builds;
  Shipping builds are launched through the Steam client. `-nosteam` disables
  the Steam OSS at runtime for packaged fallback runs; W5 documents both PIE
  behaviour and `-nosteam` in pie-scenarios.md (neither is documented today).
- **Entry gate (CEO voice finding 1):** a second PC with a *different* Steam
  account is a precondition of W1, not an open question. 1f was closed on a
  PlayerStart placement commit (`058563a`) and PIE; W1 is not done from one
  machine.
- **Time box + kill criterion:** if the SteamSockets listen host cannot be made
  to connect two PCs within two working sessions, fall back to the legacy
  `/Script/OnlineSubsystemSteam.SteamNetDriver` (Steam P2P networking, also
  relayed by Steam, `BaseEngine.ini` already defines its section) with
  `bInitServerOnClient=true`. session.md already names this alternative. W2–W5
  do not depend on which driver won.
- Done when: a packaged Development build launched with Steam running shows the
  Shift+Tab overlay, and a second PC (second Steam account) reaches the host's
  truck pad through SteamSockets with no port forwarding — **twice in one
  process** (host → 나가기 → host again, UE-239936 check).

### W2 — Session layer (C++): host / invite / join code / leave
- New `UCL9SessionSubsystem : UGameInstanceSubsystem`
  (`Source/CL9/Session/CL9SessionSubsystem.{h,cpp}`) wrapping `IOnlineSession`:
  `HostSession()`, `JoinByCode(FString)`, `LeaveSession()`, plus delegates for
  the UI (`OnSessionCreated`, `OnJoinFailed(reason)`, `OnSessionEnded`).
  **Availability predicate** (eng voice finding 1): `IsSteamAvailable()` =
  `IOnlineSubsystem::Get(STEAM_SUBSYSTEM) != nullptr && ->IsEnabled()`. Never
  test `IOnlineSubsystem::Get()` for null — when Steam fails the engine falls
  back to the **Null** OSS (`OnlineSubsystemModule.cpp:216-219`, plugin
  `EnabledByDefault: true`), so a bare `Get()` is live in PIE, `-nosteam` and
  Steam-off, and would silently create LAN sessions. AC7 asserts that PIE and
  `-nosteam` create **no** session (log line "session: Steam unavailable —
  skipping").
- Session settings — exact tuples, because OSS Steam derives the Steam lobby
  type from them (`OnlineSessionAsyncLobbySteam.cpp:69-91 BuildLobbyType`):
  - **방 만들기 (multiplayer host):** `NumPublicConnections=4`,
    `bUsesPresence=true`, `bUseLobbiesIfAvailable=true`, `bShouldAdvertise=true`,
    `bAllowJoinViaPresence=true`, `bAllowJoinViaPresenceFriendsOnly=false`,
    `bAllowInvites=true`, `bAllowJoinInProgress=true` (flipped per phase, see
    below), `bIsLANMatch=false` → **Public** lobby: friends see "Join Game",
    invites work, and non-friends can find it by code.
  - **혼자 하기 (solo):** same but `bShouldAdvertise=false` and
    `NumPublicConnections=1` → **Private** lobby: nobody can join, no code is
    rolled; it exists only so the code path is identical (session.md). The
    GameMode also gets `bSoloRun` (set from the travel URL `?solo`) so
    `PreLogin` rejects any second connection even over a LAN `open <ip>`. With
    the Steam OSS unavailable (see the predicate above), solo skips the session
    call entirely. A unit test asserts `MakeSessionSettings(Solo)` produces
    exactly this tuple.
  Custom settings advertised on the lobby: `CL9_GAME = "cl9"` (game identity —
  mandatory while on AppID 480, which is shared with every Spacewar session in
  the world), `CL9_CODE`, `CL9_HASH` (TuningHash), `CL9_VER` (build version).
  OSS Steam hands the query settings to Steam's server-side lobby search as
  string filters (`AddRequestLobbyListStringFilter`,
  `OnlineSessionAsyncLobbySteam.cpp:838-902`) together with the result-count
  filter; per Steamworks docs the filters apply before the count cap
  (unverified here — the 0/1/>1 handling below covers the other case).
- **Join code**: 6 characters from an unambiguous alphabet (no 0/O/1/I), rolled
  by the host, shown in the lobby and copyable. Client: `FindSessions` with
  `QuerySettings.Set(SEARCH_LOBBIES, true, Equals)` — **this key is what makes
  OSS Steam search lobbies at all** (`OnlineSessionInterfaceSteam.cpp:778-789`;
  without it the query goes to the master server list, which a listen host
  never registers with, and every code would be "잘못된 코드") — plus
  `CL9_GAME == cl9 && CL9_CODE == <code>` → exactly one result → `JoinSession`
  → `ClientTravel(connect string)`. Known limit: OSS Steam hard-codes
  `k_ELobbyDistanceFilterDefault` (`OnlineSessionAsyncLobbySteam.cpp:845`),
  which per Steamworks docs returns same/nearby-region lobbies only
  (unverified here) — a friend on another continent may not find the code
  while an invite (join by lobby id) still works. The not-found string gains a
  second line: "멀리 있는 친구는 Steam 초대로 들어올 수 있습니다"; S3 records
  the behaviour.
- **Friend invite**: overlay invite (`ShowInviteUI`) and
  `OnSessionUserInviteAccepted` → `JoinSession`. Cold start (client launched
  from Steam "Join Game" while the game is closed) goes through the same
  delegate on startup; verify the 5.8 command-line path (`+connect_lobby`) in
  W1's two-PC test.
- **`CL9TuningHash`** becomes real: `UCL9DefRegistry` hashes the loaded def rows
  once at Init; host advertises it as `CL9_HASH`; client compares before
  `JoinSession` (UX-only soft reject with a message) and passes it as a
  `?hash=` travel option; host `ACL9GameMode::PreLogin` rejects on mismatch
  (authoritative).
- **Host gates in `PreLogin`** (all produce a `ErrorMessage` the client shows):
  session full, TuningHash mismatch, version mismatch, solo run, and **phase ≠
  Lobby → reject** (mid-night join is decided *No* for v1 — session.md left it
  open; see Gate item). "Full" counts `GetNumPlayers()` **plus pending
  connections** — `NetDriver->ClientConnections` entries that have no
  `PlayerController` yet — because `ACL9GameMode` derives from
  `AGameModeBase`, which has no `NumTravellingPlayers` (that field is
  `AGameMode`'s, `GameMode.h:130`), and five clients can all pass `NumPlayers <
  4` during the PreLogin → map-load window. `PostLogin` re-checks the count as
  well as the phase and returns the fifth arrival with the full string.
- `Logout` frees the slot; host quitting → `DestroySession`; client sees
  `NetworkFailure` / `TravelFailure` (handlers on `UCL9GameInstance`) →
  **`LeaveSession()` first, then** the title with "호스트와의 연결이
  끊어졌습니다" (session.md 에러 처리). The leave matters: OSS Steam only
  removes the named session on an explicit destroy
  (`OnlineSessionInterfaceSteam.cpp:576-639`, `RemoveNamedSession` at `:618`)
  or on failed create/join cleanup (`OnlineSessionAsyncLobbySteam.cpp:562,575,787`);
  a lobby-chat update on host loss just refreshes members
  (`OnlineAsyncTaskManagerSteam.cpp:189-211`), and
  Steam hands lobby ownership to a remaining member — without the leave, the
  orphaned client would keep a joinable lobby and friends would still see
  "Join Game" on them (spec review round 2, finding 1; verified in S6).
- **Version gate**: set `ProjectVersion` in `Config/DefaultGame.ini`
  (`[/Script/EngineSettings.GeneralProjectSettings]`, none today); advertised
  as `CL9_VER`, passed as `?ver=` and rejected in `PreLogin` on mismatch.
- **Console cheats for UI-less testing** (accepted expansion, P2):
  `cl9.host`, `cl9.join <code>`, `cl9.leave`, `cl9.session` (dumps code,
  players, phase, net driver) through the existing `UCL9CheatManager` — W1/W2
  are verifiable before any W3 screen exists (build-order principle 1).
- **Lobby joinability follows the phase** (spec review finding 3, verified in
  `OnlineSessionAsyncLobbySteam.cpp:447,657`: Steam lobby joinable =
  `bAllowJoinInProgress && members < max`). Host calls
  `IOnlineSession::UpdateSession` with `bAllowJoinInProgress=false` in
  `StartNight` and `true` in `EnterLobby` — **wired inside the session
  subsystem**, which (when hosting) subscribes to
  `ACL9GameState::OnNightPhaseChanged` (`CL9GameState.h:157-159`) rather than
  having the GameMode call Steam (eng voice finding 11: conventions § 2 "Steam
  is transport only" — the GameMode stays free of online types). Steam then
  refuses lobby entry during a night instead of every friend launching into a
  PreLogin reject. Whether the
  Steam client also hides the "Join Game" entry is unverified, so the same
  `UpdateSession` also writes a `CL9_PHASE` lobby key (`lobby` / `night`); the
  client checks it on the search result / invite before `JoinSession` and shows
  the night string itself. A `JoinSession` refusal that still slips through
  maps to the night string when `CL9_PHASE != lobby`, else to "연결할 수
  없습니다" (T4 table). The PreLogin phase gate stays as the authoritative
  backstop.
- ~~Rich presence "N일차 · k/4"~~ — **deferred to TODOS**: OSS Steam pushes the
  status as the `steam_display` key with a forced `#` prefix
  (`OnlinePresenceInterfaceSteam.cpp:143-151`), i.e. a localization token that
  needs a Rich Presence localization file uploaded in Steamworks — impossible on
  AppID 480. "Join Game" does not depend on it; it comes from the presence
  lobby itself.

### W3 — Screens: title → host / join → lobby → start
- New tiny `Content/Maps/Lvl_Title.umap` (Project `GameDefaultMap`). Host:
  create session → `OpenLevel(Lvl_Henhouse, listen)`; client: joins straight
  into the host's game level. **One level, one travel** — the session lobby is
  the roster panel on the truck pad (HUD layer), not a separate lobby map.
  Destinations (build-order 4g) select a *generator* per site row
  (`ECL9Generator`, "목적지 하나에 생성기 하나") inside the same level, so a
  second destination does not reintroduce level travel. `HostSession` still
  takes the level name as a parameter so the subsystem stays level-agnostic.
- UI table rows through the existing `UCL9UIManager` (ui_system.md) — **no
  `Lobby` Screen row, no `ModeSelect`** (design review decisions 2 and 7):
  `Title` (Screen, `Lvl_Title`: 혼자 하기 / 방 만들기 / 코드로 참가 / 설정 /
  종료 + a hint line "친구 게임은 Steam 친구 목록의 '게임 참가'로"), `JoinCode`
  (Popup: six cells, [붙여넣기] [참가] [취소]; its content swaps to the
  Connecting state and back, and code-path rejects land **here with the code
  preserved** and the reason inline), `Notice` (Popup, two modes: modal
  one-liner + [확인], and a 3 s auto-hide toast for "복사됨" / "Player2 님이
  들어왔습니다" / "…나갔습니다" with `UI_Confirm` from sound.md), `Roster`
  (HUD-layer row, below). Character selector (one entry) and 새 판 are dropped
  from v1 — a dropdown with one option is noise. 나가기 lives where ui.md already
  reserved it: `WBP_Pause` [로비로 나가기] → `LeaveSession` → title.
- `ACL9PlayerState`: `bIsReady`, `SlotIndex` (entry order, lowest free slot) —
  replicated. `bIsReady` is set by `Server_SetReady` on the PlayerController;
  `Server_Depart` → `ACL9GameMode::RequestDepart(PC)` (listen-host only, all
  ready, no pending offer → `StartNight`; eng E3/E4). The slot colour is read
  from the character table (`GetSlotColor()`, eng E5), not a C++ array. **No
  `SelectedCharacter` field** until a second character exists (the selector was
  dropped from v1 — it returns with the roster of characters).
- `ACL9GameState`: `FString SessionCode` (replicated, `OnRep_SessionCode`).
  **No `ECL9SessionState`** (CEO review decision 12, TASTE — gate item): the
  night phase already carries the joinable/ready axis. `Lobby` = joinable +
  roster/ready shown; `Night`/`Dawn` = not joinable. session.md's "separate
  axis" paragraph existed for a `Loading` state during seamless travel, which
  this design does not have.
- **Roster panel lives in the HUD layer and is display-only**: `WBP_Roster`
  is visible whenever the phase is `Lobby` (first truck stop and every one
  after), showing the join code, the four slots with ready marks and a status
  line. ui_system.md decides cursor/`GameAndUI` by "is the Screen layer
  non-empty", so a HUD panel cannot have clickable buttons (design review
  decision 1) — the actions are **keys**: `IA_LobbyAction` (Enter) = 준비 toggle
  for clients / 출발 for the host, `IA_CopyCode` (C) = copy the code; the panel
  prints the hints ("Enter: 준비", "Enter: 출발", "C: 코드 복사"). Gamepad
  later binds the same two actions. Screen rows such as the order pick
  (`OrderSelect`, HUD-derived) open over the panel without fighting for the
  exclusive Screen slot. Alternative considered (gate Choice 5): make 준비/출발 a
  physical `IA_Interact` (E) on the truck cab through the existing focus-prompt
  system — more in-world, same session code; two-way door.
- 출발 = the existing `StartNight()`, enabled for the host only when **every
  PlayerState is ready and the order pick is done** (`OfferedOrders` empty —
  decision 13). `cl9.night` stays unrestricted for testing. Between nights the
  same panel drives departure, so "a friend who dropped rejoins at the next
  truck stop" needs nothing extra beyond the `PreLogin` phase gate.
- Race guard: `PostLogin` re-checks the phase; a player whose `PreLogin`
  passed in `Lobby` but who arrives after `StartNight` is returned to the title
  with the night-in-progress message instead of spawning mid-night.
- **Ready rules (spec review finding 6):** `bIsReady` is reset to `false` for
  every PlayerState in `EnterLobby` (host-side, alongside `ReviveDead`); the
  host does not toggle ready — pressing 출발 is the host's ready; 혼자 하기 (host
  alone) means 출발 is enabled as soon as the order pick is done. The button is
  labelled **출발** everywhere (session.md's "게임 시작" is renamed in W5).
- **Between-night joins are fresh joins, allowed** (spec review finding 7):
  a friend who joins at a later truck stop passes the `PreLogin` phase gate,
  gets a new PlayerState (lowest free slot, full HP, empty hands) and the map
  from `OnRep_SiteMap`. This is lobby-only join (CCL8 D3), not a reconnect;
  reconnecting *into a running night* with restored state stays in TODOS.
- "혼자 하기" = host session with `bShouldAdvertise=false` (a **Private** Steam
  lobby — the W2 tuple; nobody can join, not even friends), same code path
  (session.md: no separate local mode). When
  the OSS is unavailable (Steam off, `-nosteam`) 혼자 하기 **still works**: it
  opens the level as a listen server without creating a Steam session (design
  review decision 3). The solo roster shows "혼자 · Enter: 출발" only — no code,
  no empty slots.
- **Failure routing** (design review decision 4): the `UCL9GameInstance` holds
  a `PendingNotice` (FText) that survives `OpenLevel`. Code-path failures
  (search 0/timeout, `JoinSession` result, PreLogin reject — the client is still
  on `Lvl_Title` because the travel aborts before the map loads) reopen
  `JoinCode` with the typed code kept and the reason inline. A PreLogin reject
  reaches the client as `ENetworkFailure::PendingConnectionFailure` carrying
  the host's `ErrorMessage` (`UnrealEngine.cpp:16015-16017` broadcasts it on
  `ConnectionError`); the handler passes that string through for
  `PendingConnectionFailure` / `FailureReceived` and uses the fixed table only
  for `ConnectionLost` / `ConnectionTimeout` / `OutdatedClient` /
  `OutdatedServer` (eng voice finding 4, E14). In-session
  failures (host left, PostLogin kick, connection lost) set `PendingNotice`,
  travel to the title, and the title shows the `Notice` modal. The join path
  uses its own **20 s** timer in the Connecting state (cancel = `LeaveSession`
  + abort travel); the engine's 80 s / 120 s driver timeouts stay for
  in-session drops.
- **Player-facing strings** (one `FText` table, W2/T4):
  full "방이 가득 찼습니다 (4/4)" · not found "잘못된 코드입니다" · search fail
  "검색에 실패했습니다 — 다시 시도" · version "버전이 다릅니다 — 둘 다
  업데이트하세요" · hash "게임 데이터가 호스트와 다릅니다 — 게임 파일을
  확인하세요" · night "밤이 진행 중입니다 — 다음 정차 때 들어오세요" · timeout
  "연결 시간이 초과됐습니다" · host left "호스트와의 연결이 끊어졌습니다" ·
  Steam off "Steam이 실행 중이 아닙니다 — 멀티플레이 비활성" · copied "복사됨".

### W4 — Packaging, Steam upload, real-AppID test loop
- Win64 packaging settings: cook `Lvl_Title`, `Lvl_Henhouse` (exclude test
  `Lvl_Flat`), `-nosteam` still boots. **Only Shipping builds go to Steam**
  (beta branch included): `AllowCheats` returns true in every non-Shipping
  build (`CL9GameMode.cpp:55-64`) and `Server_CL9Cheat_Validate` is
  unconditional, so a Development build on the beta branch would let any
  friend run `cl9.money` / `cl9.night` on the host (eng voice finding 8).
  Development builds stay local for the two-PC W1/W2 tests.
- SteamPipe scripts committed under `Build/Steam/` (`app_build.vdf`,
  `depot_build.vdf`; no credentials in the repo); `steamcmd` upload to a
  `beta` branch, default branch untouched until release.
- Switch 480 → own AppID: `SteamDevAppId`, Steamworks launch option, then AC8.
- VAC: **not a decision** — `bVACEnabled` is read only in the Steam
  game-server init (`SteamSharedModule.cpp:418-440`), which this plan never
  runs (`bInitServerOnClient=false`, dedicated-only handle). Leave the default;
  the gate item was dropped (eng voice finding 10).
- Steam Playtest or friend keys for 2–4 player tests on the own AppID.
- **Two-process localhost smoke** (accepted expansion, P2): `Build/Smoke/`
  script launches two `-game -nosteam -log` instances (host `?listen`, client
  `open 127.0.0.1?hash=…?ver=…` — UE travel options are `?`-separated;
  `UGameplayStatics::GrabOption` splits on `?` only (`GameplayStatics.cpp:3410-3428`),
  so an `&` would fold `ver` into the hash value and the version gate would
  reject everyone — the connect-string builder gets a unit test with
  `ParseOption`) and greps both logs for the join / roster / ready lines —
  exercises the `PreLogin`/`PostLogin` gates, ready/slot replication, 출발 and
  the client-kill case on the IpNetDriver with no Steam and no second PC. It
  does **not** touch the Steam session calls (create/find/join/invite); those
  are S1–S6 on two PCs. Optional S0 extension (eng voice finding 9, unverified):
  a dev-only `-cl9nullsession` switch lets `HostSession`/`JoinByCode` run on
  the Null OSS's LAN sessions so two local instances exercise the async
  callbacks (cancel during search, callback after `Deinitialize`); if two Null
  instances on one PC cannot discover each other, drop it. Documented as
  pie-scenarios S0.
- Cheap automation tests (P2): `FAutomationTestBase` cases for join-code
  normalization (alphabet, case, whitespace, length) and TuningHash
  determinism (same def list → same hash, one changed value → different hash).
  Pure logic, no Steam.

### W5 — Docs and memory sync
- session.md: "미구현/미결" paragraphs → implemented state; join-code alphabet;
  mid-night join = No; single-level lobby overlay; delete the self-contradicting
  "향후 구현 예정 → 랜덤 매칭" line flagged in docs/review/review-2026-09-01.md M-2.
- decisions.md entries for every Gate decision; build-order.md gains a
  "⑦ Steam 출시" table (W0–W5) and marks 1f as "PIE only — real listen connect
  is W1"; pie-scenarios.md gains S1–S6 (overlay, two-PC join, code join, cold
  invite, reject paths, host quit).

## Out-of-scope
- Dedicated servers, host migration, reconnect into a running night
  (session.md "향후 복구"), random matchmaking / public server browser (decided
  No), Epic Online Services or crossplay, consoles.
- Steam achievements, leaderboards, cloud saves, Workshop, Steam Input remap UI,
  voice chat, rich presence (needs a Steamworks localization upload on the own
  AppID — TODOS).
- The Coming Soon store page and its assets (separate item after ⑤ art; not a
  prerequisite for W1–W4).
- The save slot in the roster (the save system is being built in the
  concurrent gameplay session; the v1 roster shows no slot), additional
  playable characters, store art / trailer / pricing / marketing copy,
  localization beyond Korean.
- Seamless travel and the per-night loading-progress screen from session.md —
  not needed with the single-level design; revisit if a second destination
  (build-order 4g) needs a different level.

## Acceptance Criteria
- [ ] AC1 Packaged Development build, Steam client running: OSS log reports
      Steam initialized with AppID 480 and Shift+Tab shows the overlay.
- [ ] AC2 Two PCs, two Steam accounts, no port forwarding: host creates a
      session, the other joins via Steam friends list "Join Game"; both stand on
      the truck pad and a pickup on one screen disappears on the other.
- [ ] AC3 Same result via the 6-character join code typed on the client.
- [ ] AC4 Overlay invite accepted from a cold start (game closed) lands the
      client in the host's lobby.
- [ ] AC5 Reject paths return the client to the title with the right message:
      5th player (full), TuningHash mismatch (a DA value edited on the client),
      version mismatch, night in progress; and during a night a friend either
      sees no "Join Game" entry, **or** is refused before travel with the night
      string (the client reads the `CL9_PHASE` lobby key before `JoinSession`;
      whether the Steam client hides the entry on `SetLobbyJoinable(false)` is
      unverified — S5 records which happens).
- [ ] AC6 All ready + orders picked → host 출발 → night starts for everyone;
      host quits → clients get "호스트와의 연결이 끊어졌습니다" and land on the
      title; a friend who joins at the *next* truck stop gets a fresh slot.
- [ ] AC7 PIE 2-client and `-nosteam` still run every existing pie-scenarios.md
      case (IpNetDriver fallback) — no regression.
- [ ] AC8 `steamcmd` upload to the own AppID `beta` branch; a Steam client
      installs it, overlay + invite work on the real AppID.
- [ ] AC9 Docs synced (session.md, decisions.md, build-order.md,
      pie-scenarios.md) and the work-progress memory updated.

## Related Files / Modules
| File | Role |
|------|------|
| D:\Work\unreal\CL9\CL9.uproject | enable OnlineSubsystemSteam + SteamSockets |
| D:\Work\unreal\CL9\Source\CL9\CL9.Build.cs | OnlineSubsystem, OnlineSubsystemUtils modules |
| D:\Work\unreal\CL9\Config\DefaultEngine.ini | [OnlineSubsystem], [OnlineSubsystemSteam], NetDriverDefinitions |
| D:\Work\unreal\CL9\Source\CL9\Session\CL9SessionSubsystem.{h,cpp} | new — IOnlineSession wrapper, join code, invite delegates |
| D:\Work\unreal\CL9\Source\CL9\Data\CL9DefRegistry.{h,cpp} | TuningHash over loaded def rows |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9GameMode.{h,cpp} | PreLogin/PostLogin gates, Logout slot free, ready reset in EnterLobby, lobby joinable on phase change, 출발 → StartNight |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9GameState.{h,cpp} | SessionCode (replicated) |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9PlayerState.{h,cpp} | bIsReady, SlotIndex, GetSlotColor() |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9PlayerController.{h,cpp} | Server_SetReady / Server_Depart; IA_LobbyAction / IA_CopyCode routing |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9GameInstance.{h,cpp} | NetworkError / TravelError → title |
| D:\Work\unreal\CL9\Source\CL9\UI\CL9TitleWidget, CL9JoinCodeWidget, CL9NoticeWidget, CL9RosterWidget .{h,cpp}; D:\Work\unreal\CL9\Content\UI\WBP_Title, WBP_JoinCode, WBP_Notice, WBP_Roster, WBP_Pause, UITable.uasset | Title (Screen) / JoinCode (Popup) / Notice (Popup, modal + toast) / Roster (HUD, display-only) rows; Pause [로비로 나가기] wiring |
| D:\Work\unreal\CL9\Content\Input\Actions\IA_LobbyAction, IA_CopyCode; D:\Work\unreal\CL9\Content\Input\IMC_Gameplay.uasset | Enter = 준비/출발 by role, C = copy code (Lobby phase only) |
| D:\Work\unreal\CL9\Content\Maps\Lvl_Title.umap | new title level (GameDefaultMap) |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9CheatManager.{h,cpp}, CL9Cheats.cpp | cl9.host / cl9.join / cl9.leave / cl9.session |
| D:\Work\unreal\CL9\Config\DefaultGame.ini | ProjectVersion (CL9_VER gate) |
| D:\Work\unreal\CL9\Build\Smoke\two-process-smoke.ps1 | -nosteam localhost smoke (host + client) |
| D:\Work\unreal\CL9\Source\CL9\Tests\CL9SessionCodeTest.cpp | automation tests: code normalization, TuningHash determinism |
| D:\Work\unreal\CL9\Build\Steam\app_build.vdf, depot_build.vdf | SteamPipe upload scripts |
| D:\Work\unreal\CL9\docs\systems\session.md, docs\decisions.md, docs\plan\build-order.md, docs\testing\pie-scenarios.md | docs sync |

## Must-Preserve
- Host authority (conventions § 2): Steam is transport only; no judgment moves
  to a Steam backend. Ready / slot / character are set by Server RPC and
  replicated back — never client-trusted.
- Night lifecycle API (`StartNight` / `EndNight` / `EnterLobby`,
  `ECL9NightPhase`, `OnNightPhaseChanged`) and its subscribers (NightLighting,
  HUD) unchanged.
- Existing PIE 2-client scenarios keep working without Steam (AC7).
- No asset paths in C++; new screens go through the UI table + UIManager;
  comments in Korean (decisions.md 2026-09-06).
- MAX_PLAYERS = 4; slot = entry order (session.md 2026-09-02).
- Every new Replicated field is registered in `GetLifetimeReplicatedProps`
  (build-order ⑥ note); replicated actors call `SetNetUpdateFrequency(20)`.

## Execution Notes
- Recommended model: Claude Fable 5.1 for W1 (5.8 plugin-source verification,
  ini semantics), W2 (session subsystem, PreLogin gates, failure paths) and the
  two-PC debugging; Sonnet acceptable for W3 widget boilerplate, W4 vdf scripts
  and W5 doc edits.
- This document cannot enforce the model — the executing session's `/model`
  setting decides. If the session model is below the recommendation, surface it
  to the user and confirm before proceeding.
- Machine constraints: editor closed for full builds (CLAUDE.md); Steam client
  running for OSS init; AC2–AC6 need a second PC with a **different** Steam
  account (the user's or a friend's) — PIE cannot exercise Steam sessions.
- Rough sizes: W0 waiting-dominated (days–weeks, mostly Valve); W1 CC ~2 h +
  first two-PC test; W2 CC ~4 h; W3 CC ~4 h; W4 CC ~2 h + upload; W5 ~1 h.

## Open Questions
1. Korean legal prerequisites for selling from Korea (사업자등록·게임제작업 등록,
   GRAC rating or Steam IARC) — verify current rules before W0 store page.
2. Does a Steamworks partner account already exist / is the app fee paid? Sets
   the W0 clock.
3. Second PC + second Steam account — **W1 entry gate**, not optional: which
   machine and whose account? (Family Sharing does not give a second SteamID.)
4. ~~`bInitServerOnClient` / `bUseSteamNetworking` semantics~~ — resolved
   2026-09-06 by reading the 5.8 plugin source: `bInitServerOnClient` is not
   needed for a SteamSockets listen host (see W1).
5. Mid-night join: proposal = reject for v1 (Gate item).
6. ~~VAC~~ — not a decision: the flag only matters to the Steam game-server
   init, which this plan never runs (eng review).
7. Single-level lobby: roster panel in the HUD layer on the truck pad, no
   `ECL9SessionState`, no seamless travel / loading screen — instead of
   session.md's separate lobby flow (Gate item, TASTE).

## Fairness Constraints
- TuningHash mismatch is a hard, server-side reject at `PreLogin`; the client
  pre-check only improves the message.
- Join-code search always includes the `CL9_GAME` identity key — AppID 480 is
  shared with every Spacewar session worldwide.
- No client-trusted session state; ready / slot / character via Server RPC.

## Existing Integration Contract
- `ACL9GameMode::StartNight()` is the game start; the lobby Start button calls
  it. `EnterLobby()` between nights is untouched.
- `ACL9GameState` night fields and `OnNightPhaseChanged` consumers untouched;
  the only new GameState field is `SessionCode`.
- `UCL9UIManager::OpenWidget(FName)` Screen exclusivity; state-derived screens
  (orders) keep deriving from state; the roster panel is HUD-layer.
- Existing `cl9.*` cheats keep routing through `Server_CL9Cheat` on the Steam
  net driver; the new `cl9.host/join/leave/session` use the existing
  `bLocalOnly` route (`CL9Cheats.cpp:86`) because they run before a host exists.

---

# /autoplan — Phase 1: CEO Review (SELECTIVE EXPANSION, auto-decided)

Run 2026-09-06 via /autoplan. Every AskUserQuestion the CEO skill would fire was
auto-decided with the six principles; taste calls and user challenges are queued
for the Final Approval Gate. Codex voice: **unavailable** (usage limit until
2026-09-14, verified by a live `codex exec` probe) → `[subagent-only]`.

## System audit (pre-review)
- HEAD `ba08285`; 30 recent commits are all gameplay/UI steps (4a–4d, 5a, 5c,
  HUD); **no session or Steam commit exists**. No stashes, no TODOS.md, no
  TODO/FIXME markers in Source/ or docs/.
- Working tree is dirty (docs and many Source files modified, uncommitted — the
  UI session's ongoing work per the work-progress memory). This plan does not
  touch those files except `CL9GameMode`, `CL9GameState`, `CL9PlayerState`,
  `CL9PlayerController`, `CL9GameInstance`, which are also dirty → **W1–W3
  must start after the current UI work is committed** (merge risk, one editor
  owner per binary asset).
- Retrospective: the previous project (CCL8) died building sessions and
  generators before anything was visible (build-order.md "왜 이 계획인가").
  This plan is the first "invisible infra" step in CL9; W1's acceptance is
  deliberately visible (overlay + two PCs meet) and cheats (`cl9.host/join`)
  keep every step playable before the screens exist.
- Design doc: the CCL8 greenfield doc (2026-08-27) settles Steam listen
  server, lobby-only join, no dedicated/host migration; steam-multiplayer-intent
  A1–A5 (listen server, MAX_PLAYERS=4, lobby-only join, version gate, 20 Hz).
  This plan is consistent with all of them.
- Taste calibration — good patterns to copy: `UCL9UIManager` + UI table
  (data-driven screens), night lifecycle on `ACL9GameState` with host-only
  transitions and `OnNightPhaseChanged` (state → representation), `UCL9DefRegistry`
  alias resolution (no asset paths in C++). Anti-patterns to avoid repeating:
  1f "closed" on PIE alone; session.md carrying a self-contradicting "향후
  구현 예정" list (review-2026-09-01 M-2).
- UI scope: **YES** (title, mode select, join-code popup, connecting, roster
  panel). DX scope: **NO** — the product is a consumer game; "SDK/API" hits are
  third-party dependencies we consume, not a developer-facing surface
  (Phase 2.5 skipped, logged in the audit trail).

## Step 0A — Premise challenge
| # | Premise | Verdict |
|---|---------|---------|
| P1 | Steam is the platform (not itch/Epic first) | Accepted — settled decision (overview.md, CCL8 D5); Steam relay + friends list is the whole join story |
| P2 | Listen server + Steam relay is enough for 1–4 co-op | Accepted — genre standard (Lethal Company / Content Warning / R.E.P.O. shape); host-authority latency asymmetry already accepted in conventions § 2 |
| P3 | "No servers to operate" | Accepted with a sharper wording: no *runtime* servers. The developer still operates a build pipeline, a store page and patches, and Valve owns the relay uptime |
| P4 | Overlay appears once OSS Steam initializes | Accepted with caveats now in W1: Steam client running, launched via Steam or `steam_appid.txt`, not in PIE |
| P5 | Join code via lobby search on AppID 480 works | Accepted — needs `bShouldAdvertise` + identity key; friends-only sessions are intentionally unsearchable |
| P6 | Single-level lobby (no seamless travel) | Accepted as the plan's proposal, **TASTE** — deviates from session.md; queued for the gate (Choice 1) |
| P7 | Valve's release clock is short | **Wrong as implicitly assumed** — verified sequence is fee → 30 days → page review → Coming Soon ≥ 14 days → build review: ≥ 7 weeks floor. Fix: W0 starts now, in parallel; AC8 does not wait for the store page |
| P8 | 1f "2인 리슨 접속" is done | **Wrong** — PIE only; two processes never connected. Fix: W1 is the real 1f; build-order.md corrected in W5 |

Do-nothing cost: the game stays PIE-only; nobody outside the developer's machine
can play it; every later system (save, order economy) gets built without a real
network round trip — the exact CCL8 failure in reverse.

## Step 0B — Existing code leverage map
| Sub-problem | Existing code | Reused? |
|---|---|---|
| Joinable / not-joinable state | `ACL9GameState::Phase` (Lobby/Night/Dawn), host-only `SetPhase` | Yes — PreLogin gate reads it; no new session enum |
| Game start | `ACL9GameMode::StartNight()` (guards phase == Lobby) | Yes — 출발 calls it |
| Per-player replicated state + HUD hooks | `ACL9PlayerState` (Hp, carry, `OnRep_*` + HUD delegates) | Yes — `bIsReady`, `SlotIndex` follow the same shape |
| Server RPC pattern | `Server_AcceptOrders`, `Server_CL9Cheat` (WithValidation) | Yes — `Server_SetReady`, `Server_SelectCharacter` |
| Screens | `UCL9UIManager::OpenWidget(FName)` + `UITable` rows, `CL9UINames` | Yes — Title / ModeSelect / JoinCode / Connecting rows; roster in HUD layer |
| Def tables → hash | `UCL9DefRegistry::Init(DefList)` | Yes — hash computed there once |
| Cheats | `UCL9CheatManager`, `CL9Cheats.cpp`, `AllowCheats` override | Yes — `cl9.host/join/leave/session` |
| Transport | Engine `OnlineSubsystemSteam` 1.2 + `SteamSockets` (SDK 1.64) | Yes — nothing custom |
| Error → title | none (no `NetworkFailure` handler today) | New on `UCL9GameInstance` |
| Save "판" in lobby | none (`UCL9SaveGame` not built) | Deferred (TODOS) |
Nothing is rebuilt.

## Step 0C — Dream state
```
  CURRENT STATE                THIS PLAN                       12-MONTH IDEAL
  PIE-only 2-client tests;     Steam overlay; friend invite     Released on Steam; friends join in
  no Steam, no session code;   + join code; roster/ready on     one click; rejoin a running night;
  1f never left the editor     the truck pad; PreLogin gates;   continue a saved run from the lobby;
                        --->   beta-branch upload loop;   --->  Playtest feedback loop; Steam Deck
                               W0 paperwork clock running       verified; achievements optional
```
Delta: the plan closes the whole "play with a friend on Steam" gap and leaves
exactly two doors closed on purpose (rejoin mid-night, save/continue), both
behind a gate that already rejects cleanly.

## Step 0C-bis — Implementation alternatives
```
APPROACH A: Minimal viable — plugins + invite only
  Summary: enable OSS Steam + SteamSockets, host from a console cheat, join only via
           Steam overlay invite; no join code, no title/roster UI, no gates.
  Effort: S   Risk: Med (no reject messages → silent failures; no version gate)
  Pros: two PCs connect in a day; proves the transport
  Cons: strangers-by-code path missing; full/hash/version rejects are silent; nothing
        a player can use without the developer on Discord
  Reuses: engine plugins, CheatManager
  Completeness: 4/10

APPROACH B: The plan (session subsystem, both join paths, roster, gates, upload)
  Summary: W1–W5 as written, single-level lobby, no session enum.
  Effort: M–L   Risk: Med (Steam can only be tested on two real accounts)
  Pros: every failure has a message; testable without Steam via -nosteam smoke;
        the subsystem is the base for rejoin/save later
  Cons: ~10 files; needs a second PC + account for AC2–AC6
  Reuses: everything in 0B
  Completeness: 9/10

APPROACH C: Ideal — B + rejoin-in-progress + save/continue in the lobby
  Summary: session.md fully realized including "판" continue and mid-night rejoin.
  Effort: L–XL   Risk: High (pulls the whole save system in; spawn/state restore)
  Pros: lobby feature-complete on day one
  Cons: save system is a separate goal with its own decisions; blast radius doubles
  Completeness: 10/10 but outside this plan's blast radius

RECOMMENDATION: B — P1 (completeness) inside the blast radius; C's extra coverage
belongs to the save-system goal (P2: outside blast radius → TODOS). A vs B is
not close (4 vs 9) → not a taste decision.
```

## Step 0D — SELECTIVE EXPANSION analysis
- Complexity check: >8 files, 1 new class (`UCL9SessionSubsystem`) + 4 widgets.
  Justified — a transport/session layer touches config, GameMode, GameState,
  PlayerState, PC, GI and UI by nature; dropping `ECL9SessionState` and the
  seamless-travel/loading screen removed the avoidable parts.
- Minimum set for the stated goal: W1 + W2 (host/invite/code + gates) + roster
  panel + W4 upload. W3's title/mode screens could be deferred behind cheats,
  but AC1–AC6 are player-facing — kept.
- Expansion scan → cherry-pick ceremony auto-decided (13 proposals; table in
  `C:\Users\user\.gstack\projects\Work\ceo-plans\2026-09-06-steam-release.md`):
  **accepted** `cl9.host/join/leave/session` (P2), copy/paste code, two-process
  `-nosteam` smoke (P2), unit tests for code normalization and TuningHash
  (pulled forward from build-order ⑥, which schedules "유닛 테스트 · 2인 스모크"
  and excludes only snapshot tests), `ProjectVersion`/`CL9_VER`, drop
  `ECL9SessionState` + HUD-layer roster (TASTE → gate), 출발 gated on orders
  picked; **deferred** rich presence (needs a Steamworks localization upload —
  accepted at first, reversed after the spec review), rejoin-in-progress,
  save/continue, Steam Deck, achievements; **skipped** "다시 호스트" shortcut.
- Platform potential: session subsystem + PreLogin gates + roster = the base
  for rejoin and save/continue without touching clients.

## Step 0E — Temporal interrogation (human hours / CC minutes)
- HOUR 1 (foundations / CC ~10 min): which Steam account is logged in on the
  dev PC; Steam client must run before the editor; PIE stays on IpNetDriver
  (no `HostSession` call in PIE); editor closed for the plugin-enable build.
- HOUR 2–3 (core / CC ~40 min): `FOnlineSessionSettings` custom keys with
  `EOnlineDataAdvertisementType::ViaOnlineService`; search with
  `SEARCH_PRESENCE=true` + `CL9_GAME`/`CL9_CODE` equality; delegate handles
  cleared in the subsystem's `Deinitialize`; connect string via
  `GetResolvedConnectString`; `OnSessionUserInviteAccepted` bound in
  `Initialize` (cold start fires it after the first world).
- HOUR 4–5 (integration / CC ~40 min): `PreLogin` reads `?hash=`/`?ver=` with
  `UGameplayStatics::ParseOption`; `ErrorMessage` text must be a user-facing
  Korean string (the engine shows it verbatim); `PostLogin` phase re-check;
  `Logout` slot release; `UEngine::OnNetworkFailure` binding in the GI.
- HOUR 6+ (polish / CC ~30 min): failure text table (full / hash / version /
  night / host left / timeout / Steam off); overlay only in packaged builds;
  `steam_appid.txt` absent in Shipping; second account for AC2–AC6.

## Step 0F — Mode
SELECTIVE EXPANSION (autoplan override; also the context default for a feature
on an existing system). Approach B confirmed.

## Section 1 — Architecture
```
                         STEAM (Valve): lobby list · presence · invite · relay (SDR)
   ┌──────────────┐◄──────────────────────────────────────────────►┌──────────────┐
   │ Steam client │                                                │ Steam client │
   └──────┬───────┘ steam_api (OSS Steam, AppID 480 → own)         └──────┬───────┘
   HOST PC│                                                       CLIENT PC│
   ┌──────▼──────────────────────────────┐                ┌───────────────▼─────────────┐
   │ UCL9GameInstance                     │                │ UCL9GameInstance            │
   │  ├ UCL9DefRegistry → TuningHash      │                │  ├ UCL9DefRegistry → hash   │
   │  ├ UCL9SessionSubsystem [NEW]        │  Steam lobby   │  ├ UCL9SessionSubsystem     │
   │  │   HostSession / LeaveSession ─────┼───────────────►│  │   JoinByCode / Invite    │
   │  │   UpdateSession(joinable↔phase)   │                │  │   → ClientTravel(?hash?ver)
   │  ├ OnNetworkFailure → Title [NEW]    │                │  ├ OnNetworkFailure → Title │
   │  └ UCL9UIManager (Title/ModeSelect/  │                │  └ UCL9UIManager            │
   │       JoinCode/Connecting rows)      │                │                             │
   │ ACL9GameMode (authority)             │  SteamSockets  │                             │
   │  ├ PreLogin gates [NEW] ◄────────────┼────────────────┼── connect                   │
   │  ├ PostLogin phase re-check [NEW]    │  NetDriver     │                             │
   │  ├ StartNight (existing) ◄ 출발       │                │                             │
   │  └ Logout → slot free [NEW]          │                │                             │
   │ ACL9GameState ─── replicated ────────┼────────────────┼─► ACL9GameState (mirror)    │
   │  ├ Phase / Day / Orders (existing)   │                │    └ WBP_Roster (HUD) [NEW] │
   │  └ SessionCode [NEW]                 │                │                             │
   │ ACL9PlayerState ── replicated ───────┼────────────────┼─► ACL9PlayerState           │
   │  └ bIsReady / SlotIndex [NEW]        │◄───────────────┼── Server_SetReady (RPC up)  │
   └──────────────────────────────────────┘                └─────────────────────────────┘
```
- Coupling introduced: GameMode ↔ SessionSubsystem (Logout/Destroy), GI ↔ engine
  network-failure delegates, HUD ↔ PlayerState ready fields. All through
  existing seams (subsystem, delegates, OnRep). No new coupling between
  gameplay actors and Steam.
- State machines:
```
 HOST:  [Title] ─HostSession→ [Creating] ─ok→ [Lobby(phase)] ─StartNight→ [Night] ─EndNight→ [Dawn] ─EnterLobby→ [Lobby]
           ▲                      │fail→Title+msg      │ LeaveSession / quit → DestroySession → Title
           └───────────────────────────────────────────┘
        Invalid: StartNight outside Lobby (guarded today); HostSession while a session exists (subsystem refuses, logs)
 CLIENT:[Title] ─JoinByCode→ [Searching] ─0→ Title+"잘못된 코드" ─1→ JoinSession ─ok→ [Connecting] ─PreLogin ok→ [Lobby]
                 ─Invite→ (skip search) ─────────────────┘                    │reject→ Title+reason  │
        [Lobby/Night/Dawn] ─NetworkFailure(host left, timeout)→ Title+"호스트와의 연결이 끊어졌습니다"
        Invalid: Join while Searching/Connecting (button disabled + subsystem state check)
 JOIN GATE (PreLogin): phase==Lobby && NumPlayers<4 && hash== && ver==  else ErrorMessage
```
- Scaling: irrelevant at 4 players; 10× = 40 lobbies on AppID 480 with the
  identity key — still one filtered search. Single point of failure: the host
  PC (accepted) and Valve's relay (accepted, free).
- Security: see Section 3. Production failure per integration point: Steam
  client not running (OSS init fails → multiplayer disabled with reason); lobby
  create timeout (`OnlineAsyncTaskSteamCreateServerTimeout`) → message; relay
  connection drop → `ConnectionTimeout` (80 s default) → title.
- Rollback: `git revert` of `.uproject` + `Build.cs` + `DefaultEngine.ini`
  returns to today's IpNetDriver PIE state in one build; `-nosteam` flips the
  same at runtime.
- Beautiful version: the roster panel is the only new UI concept — everything
  else is a row in the UI table and a few replicated fields. That is the
  "clever and obvious" test passing.

## Section 2 — Error & Rescue map
```
  CODEPATH                              | WHAT CAN GO WRONG                          | ERROR CLASS / SIGNAL
  --------------------------------------|--------------------------------------------|-----------------------------
  OSS Steam init (module start)         | Steam client not running / 480 blocked     | IOnlineSubsystem::Get()==null, LogOnline error
  SessionSubsystem::HostSession         | already in session; CreateSession fails    | OnCreateSessionComplete(false)
                                        | create timeout                             | OnlineAsyncTaskSteamCreateServerTimeout
  SessionSubsystem::JoinByCode          | bad chars / wrong length                   | local validation
                                        | FindSessions fails / times out             | OnFindSessionsComplete(false)
                                        | 0 results / >1 results                     | SearchResults.Num()
                                        | JoinSession result ≠ Success               | EOnJoinSessionCompleteResult (SessionIsFull, SessionDoesNotExist, CouldNotRetrieveAddress, AlreadyInSession, UnknownError)
                                        | GetResolvedConnectString fails             | bool false
  Invite accepted (delegate)            | while hosting with players; while Night    | state check; PreLogin reject
  ClientTravel → PreLogin               | full / hash / version / phase              | ErrorMessage set → client HandleNetworkFailure(FailureReceived)
  PostLogin                             | phase changed to Night since PreLogin      | re-check → ClientReturnToMainMenuWithTextReason
  In-session                            | host quit / relay drop / Steam client dies | ENetworkFailure ConnectionLost / ConnectionTimeout
  LeaveSession / DestroySession         | DestroySession fails                       | OnDestroySessionComplete(false)
  TuningHash                            | def list missing (both sides log Error today)| hash of empty registry
  UpdateSession (joinable on phase)     | fails / times out                          | OnUpdateSessionComplete(false)

  ERROR CLASS / SIGNAL                  | RESCUED? | RESCUE ACTION                                  | USER SEES
  --------------------------------------|----------|------------------------------------------------|-----------------------------
  OSS null (Steam off)                  | Y        | subsystem reports Unavailable; IpNetDriver     | 멀티플레이 button disabled: "Steam이 실행 중이 아닙니다"
  OnCreateSessionComplete(false)        | Y        | log + stay on title                            | "세션을 만들지 못했습니다 — 다시 시도"
  create timeout                        | Y        | same as above (engine fires complete(false))   | same
  local code validation                 | Y        | inline, button disabled                        | "코드는 6자 (0·O·1·I 제외)"
  OnFindSessionsComplete(false)/timeout | Y        | log + retry button                             | "검색 실패 — 다시 시도"
  0 results                             | Y        | stay on popup                                  | "잘못된 코드입니다"
  >1 results                            | Y        | pick newest, log WARNING with both codes       | nothing (transparent)
  SessionIsFull                         | Y        | title                                          | "방이 가득 찼습니다 (4/4)"
  SessionDoesNotExist                   | Y        | title                                          | "방이 사라졌습니다 (호스트가 나갔습니다)"
  CouldNotRetrieveAddress/Unknown       | Y        | title + log result enum                        | "연결할 수 없습니다"
  AlreadyInSession                      | Y        | DestroySession then retry once                 | nothing (transparent)
  PreLogin ErrorMessage                 | Y        | engine → OnNetworkFailure(FailureReceived,msg) | the ErrorMessage itself (Korean, per reason)
  PostLogin phase race                  | Y        | return-to-menu with reason                     | "밤이 진행 중입니다 — 다음 정차 때 들어오세요"
  ConnectionLost / ConnectionTimeout    | Y        | GI handler → OpenLevel(Title)                  | "호스트와의 연결이 끊어졌습니다"
  OnDestroySessionComplete(false)       | Y        | log WARNING, continue to title                 | nothing
  hash of empty registry                | Y (exists)| Init already logs Error; hash still deterministic | (dev) log
  OnUpdateSessionComplete(false)        | Y        | log WARNING, retry once next phase change; PreLogin gate is the backstop | nothing
```
No catch-alls: each `EOnJoinSessionCompleteResult` value and each
`ENetworkFailure` value maps to its own string. GAPS: none after this table;
before the review the plan had no OSS-off path and no PostLogin race.

## Section 3 — Security & threat model
| Threat | Likelihood | Impact | Mitigated? |
|---|---|---|---|
| Arbitrary lobby data from strangers on AppID 480 (FindSessions returns any Spacewar lobby matching the filter) | Med | Low | Yes — results are compared, never displayed; identity key + code equality; >1 logged |
| Malformed `?hash=` / `?ver=` options (length, unicode) | Low | Low | Yes — `ParseOption` + fixed-length compare; anything else = reject with the generic message |
| Client sets another player's ready/character (RPC on the wrong PlayerState) | Low | Low | Yes — Server RPCs act on the caller's own PlayerState only; character alias validated against the registry |
| Client triggers 출발 | Med | Med | Yes — StartNight is host-only (existing `HasAuthority`, PC-is-host check); clients' request refused + logged like `Server_AcceptOrders` |
| Client edits a DA table (tuning) | Med | Med | Yes — TuningHash reject at PreLogin (authoritative); the host can still cheat — accepted (host authority model) |
| Cheat commands from clients | Med (non-Shipping) | Med | `AllowCheats` is true in every non-Shipping build → **only Shipping builds are uploaded to Steam**; Development builds never leave the two test PCs (eng review) |
| Secrets | — | — | None in repo: `steamcmd` login is interactive; vdf files carry AppID/depot IDs only |
| New dependency | Low | Low | Epic-maintained plugins + Steamworks SDK 1.64 bundled with the engine |
| PII | Low | Low | Steam display names in the roster (already the PlayerState name); nothing stored |
| VAC | — | — | Off (decision 6, gate item): no public sessions, nothing for strangers to reach |
No High findings.

## Section 4 — Data flow & interaction edge cases
```
 JOIN CODE (host)  roll 6 chars ──► Settings.Set(CL9_CODE) ──► CreateSession ──► GameState.SessionCode ──► WBP_Roster
                   RNG: never nil │ fixed length            │ fail → title msg  │ OnRep on clients      │ copy button
 JOIN CODE (client) typed ──► normalize(upper, trim, alphabet) ──► FindSessions(CL9_GAME==cl9 && CL9_CODE==X) ──► Join ──► ClientTravel
                   nil/empty→disabled │ bad char→inline error       │ timeout/0/>1 → table above                    │ reject → msg
 READY (client)    click ──► Server_SetReady(bool) ──► PlayerState.bIsReady (rep) ──► OnRep → roster on all ──► host 출발 enabled?
                   double click → idempotent │ not in Lobby → refused+log │ owner-only │ 4/4 all true && OfferedOrders.Num()==0
```
| Interaction | Edge case | Handled | How |
|---|---|---|---|
| 호스트 button | double click | Y | subsystem state != Idle → ignore |
| 호스트 button | Steam off | Y | button disabled with reason |
| Code entry | lowercase, spaces, 0/O confusion | Y | normalize; alphabet excludes 0/O/1/I |
| Code entry | paste 7+ chars | Y | truncate to 6, validate |
| Join | click while search in flight | Y | button disabled by state |
| Invite | accepted while hosting with players | Y | ignored + log (v1); accepted when alone → leave then join |
| Invite | accepted during Night | Y | PreLogin reject message |
| Invite | cold start | Y | delegate fires after first world; title shows "연결 중…" |
| Roster | 47-char Steam name | Y | text clipped, tooltip full |
| Roster | player leaves mid-lobby | Y | Logout frees slot; OnRep removes row; ready recount |
| 출발 | pressed while a client is still connecting | Y | PostLogin phase re-check returns that client |
| 출발 | orders not picked | Y | disabled (decision 13) |
| Connecting | host quits during client load | Y | NetworkFailure → title |
| Any | Alt+F4 on client | Y | Logout on host; slot freed |
| Any | Steam client killed mid-session | Y | SteamSockets connection dies → ConnectionLost → title |
| Title | second launch while a session runs (Steam blocks) | N/A | Steam allows one instance per account |

## Section 5 — Code quality
- Fits existing patterns: GameInstance subsystem (like `UCL9UIManager`), Server
  RPC + OnRep (like orders), UI table rows, Korean comments, no asset paths.
- DRY: reuse `CL9UINames` for new rows; reuse `Server_CL9Cheat` routing for
  `cl9.host/join` (they run on the local machine, not the host — note: these
  cheats are **local**, not routed through the host; document in the cheat
  table). Failure strings live in one `FText` table (localization-ready), not
  scattered literals.
- Naming: `HostSession`, `JoinByCode`, `LeaveSession`, `SessionCode`,
  `bIsReady`, `SlotIndex` — verbs for actions, nouns for state.
- Over-engineering check: no session enum, no abstraction over `IOnlineSession`
  beyond one subsystem, no retry framework (one retry only for AlreadyInSession).
- Under-engineering check: async OSS callbacks after world change → bind with
  `TWeakObjectPtr`/subsystem lifetime and clear handles in `Deinitialize`;
  `PreLogin` must not trust `NumPlayers` alone during travel (use
  `GetNumPlayers() + NumTravellingPlayers`).
- Complexity: `PreLogin` has 4 gates in sequence (<5 branches); fine.
- Finding (auto-decided): the plan said cheats route through the host — wrong
  for `cl9.host/join/leave` (they act locally before any host exists). Plan
  text updated above.

## Section 6 — Test review
```
 NEW UX FLOWS: title→host, title→join by code, invite (warm/cold), roster ready/출발, every reject message, host-left return
 NEW DATA FLOWS: session settings ↔ lobby, code search, ready/slot replication, hash/version options, lobby joinable ↔ phase
 NEW CODEPATHS: SessionSubsystem (6 async completions), PreLogin ×4 gates, PostLogin re-check, Logout slot free, GI network failure (+LeaveSession), cheats ×4
 NEW ASYNC: CreateSession, FindSessions, JoinSession, UpdateSession, DestroySession, invite delegate
 NEW INTEGRATIONS: Steam client/OSS, SteamSockets, steamcmd upload
 NEW ERROR PATHS: Section 2 table (18 rows)
```
- The project excludes snapshot tests by decision and schedules unit tests +
  a 2-player smoke for ⑥ (build-order.md "⑥ 마무리 정리"); this plan pulls the
  two cheap pieces forward because they guard code written now. Coverage is
  manual scenarios plus those two:
  - Unit (FAutomationTestBase): code normalization/alphabet; TuningHash
    determinism and sensitivity. Deterministic, no Steam.
  - Integration (two-process `-nosteam` smoke script): host+client on
    localhost, greps logs for PreLogin accept, roster row, ready true, 출발 →
    Night on both. Runs on one PC, no Steam. Covers gates and replication.
  - System/E2E (manual, two PCs, two accounts — pie-scenarios S1–S6): overlay,
    invite warm/cold, code join, full/hash/version/night rejects, host quit,
    host→title→host again (UE-239936 check).
- 2 a.m. Friday test: the smoke script green + S2 (friend joins via friends
  list) on the own AppID. Hostile QA: 5th player via invite during 출발; code
  with `0O1I`; kill Steam client mid-night. Chaos: pull the host's network cable
  → clients return to title within the 80 s timeout with the message.
- Flakiness: all Steam tests depend on Valve + two accounts → manual only, by
  design. Pyramid: 2 unit, 1 integration, 6 manual E2E — inverted but honest
  for a networking layer whose dependency cannot be mocked cheaply.

## Section 7 — Performance
- FindSessions with the identity+code filter returns ≤ 1 lobby; lobby data is
  4 short strings. `UpdateSession` runs on phase change only (twice a night).
- TuningHash: one pass over def rows at Init (hundreds of rows, microseconds).
- Roster updates via OnRep, no tick polling. SteamSockets defaults
  (`NetServerMaxTickRate=30`) sit above the project's 20 Hz actor policy — no
  change needed. No findings.

## Section 8 — Observability
- `LogCL9` lines at every session transition with SteamID, code, phase,
  reason (Creating/Hosting/Searching/Joining/Left/Failed:<enum>); every
  PreLogin reject logs the client's hash/version against the host's.
- `cl9.session` dumps code, players (slot/ready/name), phase, net driver class,
  OSS name — the one command to paste when a friend says "안 들어가져요".
- pie-scenarios gets a "log lines to look for" column (`LogOnline`,
  `LogNet`, `LogSockets` verbosity `Verbose` for Steam issues).
- No telemetry backend (none planned; consistent with "no servers"): player
  logs come from `Saved/Logs`. Runbook rows: Steam off → tell player to start
  Steam; version mismatch → both update; night in progress → wait for the
  truck stop; UE-239936 symptom (`SteamSockets: ... listen`) → restart game.
- Joy-to-operate: `cl9.session` prints the net driver class and OSS name, so a
  wrong config is one command away (no extra dev-build UI — spec review
  round 2 dropped the duplicate).

## Section 9 — Deployment & rollout
- No database, no migration. Rollout order: W1 config commit (buildable with
  Steam off) → W2/W3 → W4 `beta` branch on the own AppID → default branch only
  at release. Steam keeps prior builds: rollback = "set previous build live"
  in Steamworks (minutes) or `git revert` of the three config files for the
  code path.
- Mixed-version window (friend on old build, host on new) is closed by the
  `CL9_VER` gate with a message telling both to update.
- Feature flag: `-nosteam` at runtime; plugins can be disabled in the
  `.uproject` for a full retreat.
- Post-deploy check (first 5 minutes): launch from the Steam library → overlay
  → `cl9.session` shows Steam net driver → host → friend "Join Game" → roster
  shows 2/4 → 출발 → both in Night. Smoke script green before every upload.

## Section 10 — Long-term trajectory
- Debt: manual Steam testing forever (dependency cannot be mocked); docs in
  three places (session.md, build-order, pie-scenarios) must be synced in W5.
- Path dependency: OSS v1. Epic ships no `OnlineServices` (v2) Steam
  implementation in 5.8 (engine listing: EOS/Null/OSSAdapter only), so v1 is
  the supported path today; the subsystem wrapper keeps every OSS call in one
  class → migration later touches one file. Reversibility **3/5**.
- Knowledge concentration: after W5 a new engineer reads session.md and the
  roster/panel note and can find every piece. 1-year question: obvious.
- Phase 2/3: rejoin-in-progress (open the PreLogin phase gate + spawn at
  truck), save/continue (lobby "판" slot), Playtest program — all sit on the
  accepted scope without redesign.

## Section 11 — Design & UX (UI scope detected; Phase 2 runs the deep review)
```
 [Title] ── 혼자 하기 ──► host(friends-only, no code) ──► [Truck pad + Roster(1/4, 출발)]
    │ ── 멀티플레이 ──► [ModeSelect] ── 호스트 ──► [Connecting: 세션 만드는 중] ──► [Truck pad + Roster(code, k/4)]
    │                      │ ── 코드 참가 ──► [JoinCode popup] ──► [Connecting] ──► [Truck pad + Roster]
    │                      │ ── 친구 참가 ──► Steam overlay (friends list) ──► same as invite
    │ ── 설정 / 종료
    │ ◄── every reject / host-left ──── [Title + message banner]
```
(Phase 1 sketch — superseded by Phase 2: no ModeSelect (decision 29), code-path
rejects stay in the JoinCode cells, in-session failures use the Notice popup.)
| Feature | Loading | Empty | Error | Success | Partial |
|---|---|---|---|---|---|
| Title | Steam init spinner (≤1 s) | Steam off → 멀티플레이 disabled + reason | — | buttons live | — |
| JoinCode | 검색 중… | empty input → button disabled | 잘못된 코드 / 검색 실패 / 방 가득 / 버전 | 연결 중… | — |
| Connecting | spinner + cancel | — | reject reason → title | roster appears | timeout 80 s → title |
| Roster | — | 1/4 alone: "친구를 초대하세요 · 코드 ABC123" | player left row fades | all ready → 출발 enabled | some not ready → 출발 disabled with "k/4 준비" |
- Hierarchy: the join code is the biggest text on the host's roster; the
  friend's first glance answers "what do I tell my friend".
- Slop risk: generic title menu. Mitigation for Phase 2: the title is the
  truck at night with the menu on its side panel — same visual language as the
  game; no separate art needed.
- Accessibility: keyboard/gamepad focus order on Title/ModeSelect/JoinCode
  (CommonUI focus cvars already enabled in DefaultGame.ini); code text large
  and monospaced.

## NOT in scope (deferred, with rationale)
- Rejoin into a running night — needs spawn/state restore; PreLogin gate keeps
  it closed cleanly (TODOS P2).
- Save / continue-run in the lobby — belongs to the save system goal (TODOS P2).
- Seamless travel + loading progress screen — no per-night travel exists.
- Dedicated servers / host migration / random matchmaking / public list —
  decided No (session.md, CCL8 D3/D5).
- Steam Deck pass, achievements, leaderboards, cloud saves, Workshop, voice,
  Steam Input UI — post-launch (TODOS P3/P4).
- Store art, trailer, pricing, marketing copy, localization beyond Korean.

## What already exists (reused)
See Step 0B: night phase as the joinable axis, `StartNight`, PlayerState
OnRep pattern, Server RPC pattern, UIManager rows, DefRegistry, CheatManager,
engine Steam plugins. New: session subsystem, PreLogin/PostLogin/Logout gates,
GI network-failure handlers, roster/title widgets, upload scripts.

## Dream state delta
After this plan: a friend joins from the Steam friends list or a code; every
failure has a Korean message; the beta branch loop works on the own AppID.
Left for the 12-month ideal: rejoin mid-night, save/continue, Playtest
feedback loop, Steam Deck, release-day store work.

## Failure Modes Registry
```
  CODEPATH                    | FAILURE MODE                     | RESCUED? | TEST?          | USER SEES?                 | LOGGED?
  ----------------------------|----------------------------------|----------|----------------|----------------------------|--------
  OSS init                    | Steam client off                 | Y        | S1 (manual)    | button disabled + reason   | Y
  HostSession                 | create fails / timeout           | Y        | S1             | message on title           | Y
  JoinByCode                  | bad input                        | Y        | unit           | inline error               | Y
  JoinByCode                  | search fail / timeout / 0 / >1   | Y        | S3             | message                    | Y
  JoinSession                 | full / gone / addr / unknown     | Y        | S5             | message per enum           | Y
  Invite                      | during Night / while hosting     | Y        | S4, S5         | message / ignored          | Y
  PreLogin                    | full / hash / version / phase    | Y        | smoke + S5     | ErrorMessage text          | Y
  PostLogin                   | phase race                       | Y        | S5 (출발 while joining) | message              | Y
  In-session                  | host quit / relay drop / Steam dies | Y     | S6             | 연결 끊김 message           | Y
  DestroySession              | fails                            | Y        | S6             | nothing                    | Y
  TuningHash                  | def list missing                 | Y (exists)| unit          | (dev) log                  | Y
  UpdateSession (joinable)    | fails                            | Y        | S5 (Join Game absent) | nothing (gate backstop) | Y
  GI failure handler          | LeaveSession skipped → orphan lobby | Y     | S6             | friends see stale Join Game | Y
  SteamSockets re-host        | UE-239936 stale socket info      | ?        | S2 twice       | "세션을 만들지 못했습니다"   | Y
```
CRITICAL GAPS: 0 (the UE-239936 row is a known-unknown with a test and a
visible message, not a silent failure).

## TODOS.md
Written to `D:\Work\unreal\CL9\TODOS.md` (new file, gstack format): rejoin
mid-night (P2), save/continue in lobby (P2), Steam Deck pass (P3),
achievements/leaderboards (P4).

## Implementation Tasks (CEO phase)
- [ ] **T1 (P1, human: ~1h / CC: ~10min)** — W0 — Start Steamworks onboarding today (fee → 30-day clock); create the app; note the AppID in the plan
  - Surfaced by: Step 0A P7 — verified release sequence has a ≥ 7-week floor
  - Files: docs/goal/steam-release.md (AppID), Config/DefaultEngine.ini (later)
  - Verify: Steamworks app page exists; 30-day countdown visible
- [ ] **T2 (P1, human: ~2h / CC: ~15min)** — W1 — Add the OSS-off path: multiplayer disabled with reason when `IOnlineSubsystem::Get()` is null
  - Surfaced by: Section 2 — first row was a GAP before review
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp, WBP_Title
  - Verify: launch with `-nosteam` → 멀티플레이 button disabled, reason shown
- [ ] **T3 (P1, human: ~1h / CC: ~10min)** — W2 — `PostLogin` phase re-check + return-to-menu reason (race between PreLogin and 출발)
  - Surfaced by: Section 4 — 출발 pressed while a client is still connecting
  - Files: Source/CL9/GameMode/CL9GameMode.cpp
  - Verify: smoke script variant: client joins during StartNight → returned with message
- [ ] **T4 (P1, human: ~1h / CC: ~10min)** — W2 — Failure-string table (`FText`) mapping every `EOnJoinSessionCompleteResult` / `ENetworkFailure` / PreLogin reason
  - Surfaced by: Section 2 — no catch-alls rule
  - Files: Source/CL9/Session/CL9SessionSubsystem.h
  - Verify: each S5/S6 scenario shows its own string
- [ ] **T5 (P2, human: ~3h / CC: ~20min)** — W4 — Two-process `-nosteam` smoke script + pie-scenarios S0
  - Surfaced by: Section 6 — only automatable integration test for the gates
  - Files: Build/Smoke/two-process-smoke.ps1, docs/testing/pie-scenarios.md
  - Verify: script exits 0 with both logs showing Night
- [ ] **T6 (P2, human: ~2h / CC: ~15min)** — W2 — `cl9.host` / `cl9.join <code>` / `cl9.leave` / `cl9.session` cheats (local, not host-routed)
  - Surfaced by: Step 0D decision 3; Section 5 routing correction
  - Files: Source/CL9/GameMode/CL9Cheats.cpp, CL9CheatManager.h
  - Verify: W1 AC2 reachable with no W3 UI
- [ ] **T7 (P2, human: ~2h / CC: ~15min)** — W1 — Repeat AC2 twice in one process (host → 나가기 → host) for UE-239936
  - Surfaced by: Landscape check — known SteamSockets listen-socket bug
  - Files: docs/testing/pie-scenarios.md (S2b)
  - Verify: second host succeeds; if not, apply the title-tick workaround and re-verify
- [ ] **T8 (P2, human: ~2h / CC: ~15min)** — W4 — Automation tests: code normalization, TuningHash determinism
  - Surfaced by: Section 6
  - Files: Source/CL9/Tests/CL9SessionCodeTest.cpp
  - Verify: `Automation RunTests CL9` green
- [ ] **T9 (P2, human: ~1h / CC: ~10min)** — W3 — Ensure current UI-session work is committed before W1–W3 touch GameMode/GameState/PlayerState/PC/GI
  - Surfaced by: System audit — dirty working tree on the same files
  - Files: (git) — Verify: `git status` clean on Source/CL9/GameMode before starting W1
- [ ] **T10 (P1, human: ~1h / CC: ~10min)** — W2 — Lobby joinable flag follows the phase (`UpdateSession` with `bAllowJoinInProgress` false in `StartNight`, true in `EnterLobby`)
  - Surfaced by: Spec review finding 3 — Steam lobby joinable = `bAllowJoinInProgress && members < max` (`OnlineSessionAsyncLobbySteam.cpp:447,657`)
  - Files: Source/CL9/GameMode/CL9GameMode.cpp, Source/CL9/Session/CL9SessionSubsystem.cpp
  - Verify: AC5 — no "Join Game" on the host during a night; returns at the truck stop
- [ ] **T11 (P3, human: ~30min / CC: ~5min)** — W5 — Remove session.md "향후 구현 예정 → 랜덤 매칭" contradiction; sync build-order 1f note; rename "게임 시작" → 출발
  - Surfaced by: System audit — review-2026-09-01 M-2; spec review finding 6
  - Files: docs/systems/session.md, docs/plan/build-order.md
  - Verify: grep "랜덤 매칭" finds only the "만들지 않는다" line
- [ ] **T12 (P1, human: ~1h / CC: ~5min)** — W1 — Secure the second PC + second Steam account before W1 starts (entry gate); W1 time-boxed to two sessions with the legacy `SteamNetDriver` fallback
  - Surfaced by: CEO voice finding 1 — 1f closed on `058563a` (PlayerStart placement) and PIE
  - Files: docs/goal/steam-release.md (Open Question 3 answer)
  - Verify: AC2 run on two physical machines, logged with both SteamIDs
- [ ] **T13 (P2, human: ~1h / CC: ~10min)** — W3 — Ready reset in `EnterLobby`; host needs no ready; 혼자 하기 = 출발 enabled once the order pick is done
  - Surfaced by: Spec review finding 6
  - Files: Source/CL9/GameMode/CL9GameMode.cpp, Source/CL9/GameMode/CL9PlayerState.cpp
  - Verify: smoke script — second night: client ready false again until pressed
- [ ] **T14 (P2, human: ~30min / CC: ~5min)** — W1 — Confirm PIE stays on IpNetDriver after the `[/Script/Engine.GameEngine]`-only `NetDriverDefinitions` change (AC7) before anything else in W1
  - Surfaced by: CEO voice finding 2 — the fallback was asserted, not verified
  - Files: Config/DefaultEngine.ini, docs/testing/pie-scenarios.md
  - Verify: PIE 2-client A3 passes; log shows `IpNetDriver` in PIE and `SteamSocketsNetDriver` in `-game`

## Dual voices (Step 0.5)

**CODEX SAYS (CEO — strategy challenge):** `[codex-unavailable]` — live probe
2026-09-06 returned "You've hit your usage limit … try again at Sep 14th, 2026"
(codex-cli 0.136.0). Phase tagged `[subagent-only]`.

**CLAUDE SUBAGENT (CEO — strategic independence)** — 8 findings, verbatim
substance:
1. (critical) Second PC is an open question while W1's done-condition needs
   it; 1f was closed on `058563a` "second PlayerStart facing the first" + PIE.
   → **Accepted**: W1 entry gate + time box (T12).
2. (high) "`-nosteam` is documented in pie-scenarios.md" was false; PIE
   fallback to IpNetDriver was asserted. → **Accepted**: wording fixed; the
   mechanism is now stated (`GameEngine`-only definition) and T14 verifies it
   first.
3. (high) A three-question answer became a six-item release program while ④
   4e–4i and the save system are unfinished; split W1 out as a spike; defer
   W2–W5. → **Partially accepted**: W1 is already first and now has a kill
   criterion; deferring W2–W4 is **rejected by the primary reviewer** because
   build-order.md says the session layer was due before ④ started — queued as
   **User Challenge 1** for the gate (the developer decides).
4. (high) W0 "Coming Soon now" runs ahead of OQ1/OQ2 and needs assets the plan
   excludes. → **Accepted**: W0 = registration + fee + AppID; store page is a
   separate later item.
5. (medium) "No servers" cost understated; SDR terms unverified; client drop
   loses the night. → **Accepted in part**: SDR wording now "per Steamworks
   docs, reconfirm at W0"; reconnect is a named TODO with a dependency.
6. (medium) One-level assumption vs 4g. → **Rejected with evidence**: 4g
   selects a generator per site row inside the same level (`ECL9Generator`,
   "목적지 하나에 생성기 하나"); subsystem kept level-agnostic anyway.
7. (medium) AppID 480 search cap before filters. → **Rejected with evidence**:
   OSS Steam applies string filters server-side before the result-count cap
   (`OnlineSessionAsyncLobbySteam.cpp:838-902`); 0/1/>1 results are all handled.
8. (medium) Alternatives/market: Playtest/demo/Next Fest vs release, solo-first,
   legacy net driver fallback unweighed. → **Accepted in part**: legacy driver
   is now the W1 fallback; "first artifact = Playtest/demo?" queued as **Taste
   Choice 3** at the gate; store-page marketing stays out of this plan.

**Spec review of the CEO document** (independent subagent, round 1: 5/10,
11 issues; all fixed in the plan and the CEO doc, one rejected with reasoning):
`bInitServerOnClient` not required (accepted — engine source re-read, W1
corrected); rich presence needs localization tokens (accepted — deferred);
lobby joinable flag missing (accepted — T10); stale `SessionState` wording
(accepted — rewritten); `OfferedOrders`-empty gate passes with zero orders
(**rejected**: that is the intended non-soft-locking behaviour when a day has
no order rows; documented); ready reset semantics (accepted — T13);
between-night join clarified (accepted — AC6); cheats local-only (accepted);
undefined labels (legend added); smoke script wording (accepted); Valve
numbers (accepted — dated in W0).

```
CEO DUAL VOICES — CONSENSUS TABLE:
═══════════════════════════════════════════════════════════════
  Dimension                             Claude   Codex   Consensus
  ────────────────────────────────────  ───────  ──────  ─────────
  1. Premises valid?                    PARTIAL  N/A     PARTIAL → 2 fixed (P7, P8) + 2 source re-reads
  2. Right problem to solve?            PARTIAL  N/A     PARTIAL → User Challenge 1
  3. Scope calibration correct?         NO       N/A     DISAGREE → User Challenge 1
  4. Alternatives sufficiently explored? NO      N/A     DISAGREE → Taste Choice 3 (Playtest/demo first)
  5. Competitive/market risks covered?  NO       N/A     N/A — store page deliberately out of scope
  6. 6-month trajectory sound?          PARTIAL  N/A     PARTIAL → W1 split + kill criterion adopted
═══════════════════════════════════════════════════════════════
0/6 CONFIRMED (no second model), 2 DISAGREE → gate, 1 critical finding adopted.
```

## Decision Audit Trail (Phase 1)

| # | Phase | Decision | Classification | Principle | Rationale | Rejected |
|---|-------|----------|----------------|-----------|-----------|----------|
| 1 | 0 | Plan file = docs/goal/steam-release.md, English goal template | Mechanical | P3 | Matches the previous /autoplan run (CCL8 docs/goal) and the goaldoc convention | scratchpad-only plan |
| 2 | 0 | UI scope YES, DX scope NO | Mechanical | P3 | Consumer game; "SDK/API" hits are dependencies we consume | Phase 2.5 DX review |
| 3 | 0.5 | Codex voice unavailable → subagent-only | Mechanical | P6 | Live probe: usage limit until 2026-09-14 | waiting 8 days |
| 4 | 1-0A | Premise P7 (short Valve clock) wrong → W0 starts now | Mechanical | P1 | Verified ≥ 7-week floor | — |
| 5 | 1-0A | Premise P8 (1f done) wrong → W1 is the real 1f | Mechanical | P1 | `058563a` + PIE only | — |
| 6 | 1-0C-bis | Approach B (plan) over A (invite-only) and C (+save/JIP) | Mechanical | P1/P2 | 9/10 inside blast radius; C pulls the save goal in | A (4/10), C |
| 7 | 1-0F | Mode SELECTIVE EXPANSION | Mechanical | override | autoplan CEO override; also the context default | — |
| 8 | 1-0D | Accept cheats, copy/paste, smoke script, ProjectVersion, 출발 gate | Mechanical | P2 | In blast radius, < 1 day CC each | — |
| 9 | 1-0D | Drop `ECL9SessionState`, HUD-layer roster | **Taste** | P5 | Simpler; deviates from session.md → gate Choice 1 | keep session enum |
| 10 | 1-0D | Defer JIP, save/continue, Deck, achievements → TODOS | Mechanical | P3 | Outside blast radius | build now |
| 11 | 1-0D | Skip "다시 호스트" shortcut | Mechanical | P5 | Unasked branch | — |
| 12 | 1-0D | Rich presence: ACCEPT → DEFER after spec review | Mechanical | P1 | `#token` needs a Steamworks localization upload; impossible on 480 | keep in W2 |
| 13 | 1-S2 | Add OSS-off path, PostLogin race guard, failure-string table | Mechanical | P1 | Zero silent failures | — |
| 14 | 1-S5 | New cheats are local-only, not host-routed | Mechanical | P5 | `bLocalOnly` route exists (`CL9Cheats.cpp:86`) | — |
| 15 | 1-voice | Second PC = W1 entry gate; W1 time box + legacy driver fallback | Mechanical | P1 | CEO voice finding 1/3/8 | — |
| 16 | 1-voice | W0 = registration only; store page separate | Mechanical | P3 | CEO voice finding 4 | Coming Soon now |
| 17 | 1-voice | Defer W2–W4 until 4e–4i + save? | **User Challenge** | — | build-order says the session layer is overdue; developer decides | — |
| 18 | 1-voice | First Steam artifact Playtest/demo vs release | **Taste** | — | Strategic; no code impact → gate Choice 3 | — |
| 19 | 1-voice | 4g breaks one-level? | Mechanical | P5 | Rejected with evidence (`ECL9Generator` per site row) | keep travel |
| 20 | 1-spec | `bInitServerOnClient` required? | Mechanical | P5 | Rejected my own draft after re-reading source; false | keep true |
| 21 | 1-spec | Lobby joinable flag follows phase | Mechanical | P1 | Verified engine behaviour; T10 | — |
| 22 | 1-spec | `OfferedOrders`-empty gate vs explicit accepted flag | Mechanical | P5 | Explicit flag soft-locks days with no order rows | explicit flag |
| 23 | 1-spec | Ready reset rule, single label 출발, between-night fresh join allowed | Mechanical | P1 | Unspecified semantics made explicit | — |
| 24 | 1 | Mid-night join = reject for v1 | **Taste** | P5 | session.md left it open → gate Choice 2 | allow |
| 25 | 1 | VAC off | **Taste** | P3 | No public sessions → gate Choice 4 | VAC on |
| 26 | 1 | TODOS.md created at repo root (5 items) | Mechanical | P3 | gstack format; the repo had none | docs/ location |

## Completion Summary (Phase 1)
```
  +====================================================================+
  |            MEGA PLAN REVIEW — COMPLETION SUMMARY                   |
  +====================================================================+
  | Mode selected        | SELECTIVE EXPANSION (autoplan override)      |
  | System Audit         | no session/Steam commits; dirty tree on the  |
  |                      | same GameMode files; 1f = PIE only           |
  | Step 0               | 8 premises (2 wrong, fixed); approach B;     |
  |                      | 19 scope proposals                           |
  | Section 1  (Arch)    | 0 issues (diagram + 2 state machines)        |
  | Section 2  (Errors)  | 18 error paths mapped, 0 GAPS after review   |
  | Section 3  (Security)| 10 threats, 0 High severity                  |
  | Section 4  (Data/UX) | 16 edge cases mapped, 0 unhandled            |
  | Section 5  (Quality) | 1 issue (cheat routing) — fixed              |
  | Section 6  (Tests)   | Diagram produced, 0 gaps (2 unit, 1 int, 6 E2E)|
  | Section 7  (Perf)    | 0 issues                                     |
  | Section 8  (Observ)  | 0 gaps (cl9.session + log lines specified)   |
  | Section 9  (Deploy)  | 1 risk flagged (mixed versions) — gated      |
  | Section 10 (Future)  | Reversibility: 3/5, debt items: 2            |
  | Section 11 (Design)  | 3 notes → Phase 2 deep review                |
  +--------------------------------------------------------------------+
  | NOT in scope         | written (6 items)                            |
  | What already exists  | written                                      |
  | Dream state delta    | written                                      |
  | Error/rescue registry| 14 codepaths, 0 CRITICAL GAPS                |
  | Failure modes        | 13 total, 0 CRITICAL GAPS (1 known-unknown)  |
  | TODOS.md updates     | 5 items written                              |
  | Scope proposals      | 19 proposed, 12 accepted, 5 deferred, 1 skip |
  | CEO plan             | written (~/.gstack/projects/Work/ceo-plans)  |
  | Outside voice        | ran (claude subagent) [subagent-only]        |
  | Lake Score           | 6/6 recommendations chose the complete option|
  | Diagrams produced    | 5 (architecture, 2 state machines, data flow,|
  |                      | user flow)                                   |
  | Stale diagrams found | 0 (session.md ASCII screens → W5 rewrite)    |
  | Unresolved decisions | 0 mid-run; 4 taste + 1 user challenge → gate |
  +====================================================================+
```

> **Phase 1 complete.** Codex: unavailable. Claude subagent: 8 issues (5
> accepted, 2 rejected with evidence, 1 → user challenge). Spec review: 11
> issues (10 fixed, 1 rejected with reasoning). Consensus: 0/6 confirmed
> (single model), 2 disagreements → surfaced at gate. Passing to Phase 2
> (Design — UI scope detected).

---

# /autoplan — Phase 2: Design Review (auto-decided)

Scope gate: plan mode/autoplan — auto-selected B (reviewing this plan's W3
screens). Mockups: the gstack designer binary is installed but has no OpenAI
key configured (`$D setup` prints the instructions) → **text wireframes
instead of generated mockups** (progressive enhancement, not a gate).

## Step 0 — Design scope
- **0A Initial rating: 5/10.** The plan names the screens, has a state table
  and a user-flow diagram (Section 11), and knows the join code is the hero
  element — but it says nothing about *where* the roster sits in the HUD grid,
  what a roster row looks like, where failure messages appear, the focus order,
  or sizes in the project's 1080p / 24pt scale. A 10 for this plan: one ASCII
  wireframe per screen placed in the HUD zones of ui.md, a state table with the
  actual Korean copy, the roster row anatomy, keyboard focus order, and the
  message surface decided (not "title + banner" hand-waving).
- **0B DESIGN.md: none.** No `DESIGN.md`, no `docs/designs/`. Recommend
  `/design-consultation` later; for this plan the de-facto system is
  ui_system.md + ui.md: one `WBP_Main`, HUD layer always on, exclusive Screen
  layer, Popup layer for toasts/dialogs, widgets designed at 1080p × 1.0 with
  24pt base text, global `ApplicationScale` 1.4, minimum 16pt, HUD zones
  (top: day/night clock · top-left: HP/stamina/ether · bottom-left: orders ·
  bottom-center: prompts · bottom-right: carry slots). **Right side is free.**
- **0C Existing leverage:** `WBP_Spectate` (small top-center panel: one title
  line + one guidance line — the roster copies its restraint), `WBP_Pause`
  (vertical menu list — Title/ModeSelect copy its list style), `WBP_OrderSelect`
  (runtime-generated rows — roster rows are built the same way from
  `PlayerArray`), `WBP_ScanLabel` (label pool + self-hiding animation — the
  Notice toast can reuse the fade), UI table rows + `CL9UINames`.
- **0D Focus areas:** all seven (P1).

## Pass 1 — Information architecture: 4/10 → 9/10
What the player sees first, second, third — now specified per screen:
```
 TITLE (Lvl_Title: the truck at night, lamp on, camera fixed; menu on the LEFT third)
 ┌──────────────────────────────────────────────────────────────┐
 │  양계장 습격                       (title 48pt, top-left)     │
 │                                                              │
 │  ▶ 혼자 하기                       (list 24pt, left column)   │
 │    방 만들기                       친구를 초대하거나 코드를 알려줍니다 │
 │    코드로 참가                     친구가 알려준 6자 코드           │
 │    설정                                                       │
 │    종료                                                       │
 │  친구 게임은 Steam 친구 목록의 '게임 참가'로       (16pt hint)   │
 │  Steam: 연결됨 · <닉네임>      v0.1.0     (16pt, bottom-left) │
 └──────────────────────────────────────────────────────────────┘
 1st: the game's name + the truck (identity) · 2nd: 혼자 하기 (default focus) ·
 3rd: the Steam status line — trust before the first click. No mode-select hop
 (design review decision 7): hosting is one click from the title.
 Steam off: 방 만들기 / 코드로 참가 dimmed; status line reads "Steam이 실행 중이
 아닙니다 — 멀티플레이 비활성"; 혼자 하기 still works.

 JOIN CODE (Popup row, centered, 560×260 @1.0):
 ┌───────────────────────────────┐
 │  코드 입력                     │
 │  ┌─┬─┬─┬─┬─┬─┐                 │  six cells, 40pt, auto-uppercase
 │  │K│7│M│X│4│P│                 │
 │  └─┴─┴─┴─┴─┴─┘                 │
 │  (status line — see states)    │
 │  [붙여넣기]      [참가]  [취소] │  Enter = 참가 · Esc = 취소
 └───────────────────────────────┘

 CONNECTING (same Popup, content swaps): spinner · "연결 중… 호스트: <이름>" · [취소]
 · 20 s join timer. Failure never bounces the player to a different surface: the
 panel returns to the code cells with the typed code kept, the reason inline
 and [참가] re-enabled — one place to read, one key to press again.

 ROSTER (HUD layer, top-right, 420×300 @1.0; visible only while phase == Lobby;
 DISPLAY-ONLY — actions are keys, because a HUD panel has no cursor)
 ┌ 로비 ──────────────────────────────┐
 │ 코드  K7MX4P          C: 코드 복사   │  code 40pt, the largest text on screen
 │ 1 ● 나 (호스트)                     │  slot number + slot color dot (TintColor); host has no ready mark
 │ 2 ● Player2                 준비 ✓  │
 │ 3 ● 아주긴이름아주긴이름아주…   대기  │  clip at 14 chars
 │ 4 ○ 빈 자리 — 초대하세요            │
 │ 2/4 준비 · 주문 확정 전              │  status line = WHY 출발 is not yet possible
 │ Enter: 출발 (host)  /  Enter: 준비 (client)│  key hint line
 └────────────────────────────────────┘
 1st: the code (what to tell a friend) · 2nd: who is here and ready · 3rd: the
 one key. 혼자 하기: "혼자 · Enter: 출발" only. Join/leave → 3 s toast
 ("Player2 님이 들어왔습니다") so a host facing away still knows.

 NOTICE (Popup row `Notice`, reusable): modal one line + [확인] (host left,
 PostLogin kick); toast mode auto-hides in 3 s (복사됨, joined, left).
```
Constraint worship: if the roster could show only three things — code, ready
count, 출발 — it does; names are the fourth.

## Pass 2 — Interaction state coverage: 6/10 → 9/10
| Feature | Loading | Empty | Error | Success | Partial |
|---|---|---|---|---|---|
| Title | — (OSS init happens at module start; the status is known when the title opens) | — | Steam off → 방 만들기/코드로 참가 dimmed + status reason; 혼자 하기 works | status "Steam: 연결됨 · 닉네임" | — |
| Join code | cells locked, status "검색 중…" | no input → 참가 disabled, status "친구가 알려준 6자 코드" | inline, **code kept**: "잘못된 코드입니다" / "검색에 실패했습니다 — 다시 시도" / "방이 가득 찼습니다 (4/4)" / "버전이 다릅니다 — 둘 다 업데이트하세요" / "게임 데이터가 호스트와 다릅니다 — 게임 파일을 확인하세요" / "밤이 진행 중입니다 — 다음 정차 때 들어오세요" | panel swaps to Connecting | 1–5 chars typed → 참가 disabled, no error yet |
| Connecting | spinner + "연결 중… 호스트: 이름" + [취소] | — | back to the cells with the reason inline (same panel) | panel closes, roster appears | 20 s join timer → "연결 시간이 초과됐습니다", back to the cells |
| Roster (host) | — | alone (multiplayer host): "4 ○ 빈 자리 — 초대하세요" rows + code; solo: "혼자 · Enter: 출발" | — | all ready + orders picked → hint "Enter: 출발" lights up | "k/4 준비 · 주문 확정 전" line names the blocker |
| Roster (client) | — | — | — | 준비 ✓ on own row within one replication tick | host not started → "호스트 대기 중" |
| Roster (any) | — | — | player left → row returns to "빈 자리" + toast "…나갔습니다" | player joined → row fills + toast "…들어왔습니다" (UI_Confirm) | — |
| Copy code | — | — | — | toast "복사됨" | — |
| Night start | — | — | — | roster hides on phase Night (existing lighting change is the feedback) | — |
| Host left / kicked | — | — | Notice "호스트와의 연결이 끊어졌습니다" [확인] → title | — | — |
Every empty state has a next action (초대하세요 · 코드); every error names the
cause and the recovery.

## Pass 3 — User journey & emotional arc: 5/10 → 8/10
| Step | User does | User feels | Plan specifies |
|---|---|---|---|
| 1 | Opens the game from Steam | "is this working?" | Title status line: Steam 연결됨 + nickname (trust in 1 s) |
| 2 | 멀티플레이 → 호스트로 플레이 | one obvious choice | default focus on 호스트로 플레이; sub-line says what it does |
| 3 | Lands on the truck pad, roster shows the code | "what do I tell my friend?" | code is the biggest text; [복사]; "초대하세요" hint; Steam friends list is the other path |
| 4 | Friend clicks Join Game / types code | waiting… | friend's row appears with name + slot color (relief) |
| 5 | Friend presses 준비 | anticipation | ✓ appears; status line counts k/4 |
| 6 | Host confirms orders, presses 출발 | "we're going" | night lighting change (existing) is the payoff; roster hides |
| break A | Host waits alone 2 minutes | doubt | fix: the empty-slot rows say 초대하세요; code stays visible; no spinner |
| break B | Join fails | frustration | fix: reason + one button, same panel; never a silent return |
| break C | Host quits mid-night | confusion | fix: Notice with the reason, [확인] → title |
5-second: truck + name. 5-minute: code → friend row → 출발. 5-year: the roster
is the same between every night — it becomes the ritual before departure.

## Pass 4 — AI slop risk: 6/10 → 9/10
Classifier: **APP UI** (in-game panels). Hard rejections checked: no card
grids (roster rows are rows, not cards), no busy imagery behind text (panels
sit on a dimmed scrim over the scene), no decorative shadows, one accent
(warm lamp amber on the dark scene), utility copy only ("코드 · 준비 · 출발").
Litmus (self): brand unmistakable (truck + title) YES · one visual anchor
(the code) YES · scannable by headlines YES · one job per panel YES · cards
necessary N/A · motion improves hierarchy (Notice fade reuses ScanLabel) YES
· premium without shadows YES. Typeface: the project's existing UI font (not
system-ui); the code uses the same face at 40pt with the unambiguous alphabet,
so no separate monospace face is needed.

## Pass 5 — Design system alignment: 5/10 → 8/10
No DESIGN.md (gap → `/design-consultation` recommended, TODO). Aligned with the
de-facto system: every new surface is a UI-table row (`Title`, `ModeSelect`
Screen rows in `Lvl_Title`; `JoinCode`, `Notice` Popup rows; `Roster` HUD-layer
element inside `WBP_HUD` or a HUD-layer row — decision below), 24pt base /
16pt minimum / 1080p design / `ApplicationScale` 1.4 untouched, Screen
exclusivity preserved (order pick still opens over the roster), OnRep-driven
updates (no tick polling). New vocabulary: **Notice** (one-line popup) and
**Roster** (HUD panel) — two components, both reusable.

## Pass 6 — Responsive & accessibility: 3/10 → 8/10
- Input: mouse + keyboard now; every Screen/Popup has a focus order (Title:
  혼자 하기 → 방 만들기 → 코드로 참가 → 설정 → 종료; JoinCode: cells → 참가 →
  취소), Enter/Esc bound, CommonUI focus cvars already on in DefaultGame.ini.
  The roster has **no focus** — it is HUD-layer and display-only; its two
  actions are `IA_LobbyAction` (Enter) and `IA_CopyCode` (C) in `IMC_Gameplay`,
  active only while the phase is `Lobby` (no collision with E interact, R
  pocket, F light, Tab spectate, M menu). Gamepad later binds the same two
  actions; the hint line reads the bound key from the mapping, not a literal.
- Resolution: designed at 1080p × 1.0, scaled by `ApplicationScale`; the roster
  anchors top-right with a 24px margin so 720p/1440p keep it on screen; text
  never below 16pt.
- Colorblind-safe slots: slot **number** + color dot (color alone is never the
  only signal); ready = ✓ glyph + text "준비", not color.
- Contrast: 24pt light text on the dark scrim ≥ 4.5:1; the amber accent only
  on the enabled 출발 button and the code.
- Screen readers: not applicable in-engine; text stays real text (no baked
  images) for localization later.

## Pass 7 — Unresolved design decisions (auto-decided, P5 explicit)
| Decision | Chosen | If deferred |
|---|---|---|
| Roster placement | HUD layer, top-right (the only free HUD zone) | implementer drops it center-screen over the order pick |
| Roster as `WBP_HUD` child vs own HUD-layer UI-table row | own row `Roster` (Layer=HUD) opened by the HUD when phase == Lobby, closed on Night — keeps `WBP_HUD` untouched (dirty UI session) | roster logic tangled into WBP_HUD mid-refactor |
| Roster actions: buttons vs keys | keys (`IA_LobbyAction` Enter, `IA_CopyCode` C) — a HUD panel has no cursor under ui_system.md's input rule; alternative "E on the truck cab" via the focus prompt → gate Choice 5 | unclickable buttons ship |
| Failure message surface | one reusable `Notice` Popup row (modal + toast); code-path failures return to the JoinCode cells with the code kept; `GameInstance.PendingNotice` carries in-session failures across `OpenLevel(Title)` | three different banners; message lost on level change |
| Title structure | five items, no ModeSelect; Steam friends hint line | a two-hop menu with jargon ("멀티플레이 → 호스트") |
| 혼자 하기 without Steam | works: listen level, no session call | `-nosteam` cannot start a solo run |
| Join timeout | 20 s game-side timer in Connecting; engine 80/120 s untouched | 80 s of spinner |
| Code presentation | six cells, 40pt, project UI font, unambiguous alphabet, auto-uppercase, [복사]/[붙여넣기] | a text box with a placeholder-as-label |
| Slot identity | number + TintColor dot; name clipped at 14 chars with tooltip | 47-char names break the row |
| 혼자 하기 roster | no code, no empty rows, "혼자" + 출발 | host sees "초대하세요" in a solo run |
| Title art | `Lvl_Title`: truck pad actor + lamp from the game, fixed camera, night | a black screen with a menu (slop) |
| Leave a session | `WBP_Pause` [로비로 나가기] (already reserved in ui.md) → `LeaveSession` → title | a fourth "나가기" surface |

## NOT in scope (design)
- Gamepad layout/glyphs (input mapping exists; visual glyphs later).
- Settings screen contents (settings.md — separate).
- Character selector art (single character today).
- Localization beyond Korean (FText everywhere keeps the door open).

## What already exists (design)
`WBP_Spectate` panel restraint, `WBP_Pause` list, `WBP_OrderSelect` runtime
rows, `WBP_ScanLabel` fade, the UI table / layers, `ApplicationScale` 1.4,
HUD zone map in ui.md, slot `TintColor` in plan/data/character.md.

## Design TODOS
- `/design-consultation` → DESIGN.md (font, spacing scale, accent) before ⑤
  art replaces placeholders — P3 (added to TODOS.md).

## Implementation Tasks (design phase)
- [ ] **D1 (P1, human: ~2h / CC: ~15min)** — W3 — `Notice` Popup row + widget (modal one line + [확인]; toast mode auto-hide 3 s with `UI_Confirm`); `UCL9GameInstance::PendingNotice` shown by the title on arrival
  - Surfaced by: Pass 1/2; design voice findings 4/5 — one message surface, a carrier across `OpenLevel`
  - Files: Content/UI/WBP_Notice, UITable, Source/CL9/GameMode/CL9GameInstance.{h,cpp}, Source/CL9/UI/CL9NoticeWidget.{h,cpp}
  - Verify: S6 host quit → title shows the Notice → 확인; join/leave toasts on the host
- [ ] **D2 (P1, human: ~3h / CC: ~20min)** — W3 — `Roster` HUD-layer row, display-only: code + "C: 코드 복사", 4 rows (number · TintColor dot · name clipped 14 · ready ✓ / 대기; host row without ready), status line naming the blocker, key hint line; opened by HUD on phase Lobby, closed on Night; solo variant "혼자 · Enter: 출발"
  - Surfaced by: Pass 1/2/7; design voice findings 1/3/8
  - Files: Content/UI/WBP_Roster, UITable, Source/CL9/UI/CL9RosterWidget.{h,cpp}
  - Verify: smoke script screenshots; S2 with a 47-char name; solo run shows no code
- [ ] **D3 (P2, human: ~2h / CC: ~15min)** — W3 — `JoinCode` Popup: six cells, auto-uppercase, alphabet filter, [붙여넣기], status line states, swaps to Connecting content (20 s timer, 취소) and **back with the code kept** on any code-path failure, Enter/Esc
  - Surfaced by: Pass 2/6; design voice findings 4/9
  - Files: Content/UI/WBP_JoinCode, Source/CL9/UI/CL9JoinCodeWidget.{h,cpp}
  - Verify: type lowercase + spaces → normalized; 0 results → "잘못된 코드입니다" inline, code still there; full room → "(4/4)" inline
- [ ] **D4 (P2, human: ~2h / CC: ~15min)** — W3 — `Lvl_Title` with the truck pad + lamp, fixed camera; `Title` Screen row: five items (혼자 하기 / 방 만들기 / 코드로 참가 / 설정 / 종료), Steam friends hint line, Steam status line, focus order; Steam off dims the two online items and 혼자 하기 still starts
  - Surfaced by: Pass 1/3/4; design voice findings 3/7/8
  - Files: Content/Maps/Lvl_Title.umap, Content/UI/WBP_Title, Source/CL9/UI/CL9TitleWidget.{h,cpp}
  - Verify: `-nosteam` → 혼자 하기 runs, online items dimmed with reason; keyboard-only navigation works
- [ ] **D6 (P1, human: ~1h / CC: ~10min)** — W3 — `IA_LobbyAction` (Enter: 준비 toggle / 출발 by role) and `IA_CopyCode` (C) in `IMC_Gameplay`, active only in phase Lobby; PC routes 준비 → `Server_SetReady`, 출발 → host `StartNight` path
  - Surfaced by: design voice finding 1 — HUD panels have no cursor
  - Files: Content/Input/Actions/IA_LobbyAction, IA_CopyCode, IMC_Gameplay, Source/CL9/GameMode/CL9PlayerController.{h,cpp}
  - Verify: smoke script: client Enter → ready ✓ on host; host Enter with all ready → Night
- [ ] **D7 (P2, human: ~1h / CC: ~10min)** — W3 — `WBP_Pause` [로비로 나가기] → `LeaveSession` → title (host: DestroySession; client: leave)
  - Surfaced by: Pass 7 — the leave surface ui.md already reserved
  - Files: Content/UI/WBP_Pause, Source/CL9/UI/*Pause*, Source/CL9/Session/CL9SessionSubsystem.cpp
  - Verify: S6 — client leaves → host roster row empties + toast; host leaves → clients get the Notice
- [ ] **D5 (P3, human: ~1h / CC: ~10min)** — W5 — ui.md: roster zone (top-right), the Notice/Roster/JoinCode rows, the two lobby input actions; input.md: `IA_LobbyAction`, `IA_CopyCode` rows; session.md: title without mode select
  - Surfaced by: Pass 5; design voice finding 7
  - Files: docs/systems/ui.md, docs/systems/ui_system.md, docs/systems/input.md, docs/systems/session.md
  - Verify: docs list every new row and action

## Dual voices (design)

**CODEX SAYS (design — UX challenge):** `[codex-unavailable]` (usage limit,
see Phase 1). `[subagent-only]`.

**CLAUDE SUBAGENT (design — independent review)** — read the plan *before*
the Phase 2 passes were written (only W3 + Section 11 existed). 9 findings:
1. (critical) HUD-layer roster buttons are unclickable under ui_system.md's
   input rule (cursor only when the Screen layer is non-empty). → **Accepted**:
   roster is display-only; `IA_LobbyAction` (Enter) / `IA_CopyCode` (C); truck-cab
   interaction kept as gate Choice 5.
2. (critical) W3 contradicted itself (`Lobby` Screen row with character
   selector / 새 판 vs HUD roster). → **Accepted**: rows are Title / JoinCode /
   Notice / Roster; selector and 새 판 dropped; 나가기 in `WBP_Pause`.
3. (high) 혼자 하기 roster state undefined; must work with Steam off. →
   **Accepted**: "혼자 · Enter: 출발"; solo never calls the session subsystem when
   the OSS is null.
4. (high) Rejects dump the friend on the title and lose the code; nothing
   carries a message across `OpenLevel`. → **Accepted**: code-path failures
   return to the cells with the code kept; `GameInstance.PendingNotice`.
5. (high) No join/leave feedback for a host facing away. → **Accepted**: toast
   + `UI_Confirm` (sound.md has the cue).
6. (medium) Player-facing hash/version strings unwritten. → **Accepted**:
   string table written into W3.
7. (medium) ModeSelect is a hop the user does not need. → **Accepted**: title
   has five items; Steam friends hint line replaces 친구 참가.
8. (medium) Slot color / host mark / phantom "Steam init spinner". →
   **Accepted** (slot color + number were already in the Pass 1 wireframe; the
   Title loading state is removed; host row has no ready mark).
9. (medium) 80 s spinner. → **Accepted**: 20 s game-side join timer.

```
DESIGN LITMUS SCORECARD (subagent = plan before passes; primary = after fixes):
═══════════════════════════════════════════════════════════════════════
  Dimension                        Subagent  Primary(after)  Consensus
  ───────────────────────────────  ────────  ──────────────  ─────────
  1. Information hierarchy         5         9               DISAGREE → resolved (decisions 2/7 adopted)
  2. Interaction states            4         9               DISAGREE → resolved (3/4/5/9 adopted)
  3. Journey coherence             4         8               DISAGREE → resolved (4/5 adopted)
  4. Specificity (anti-slop)       5         9               DISAGREE → resolved (6/8 adopted)
  5. Design-system alignment       3         8               DISAGREE → resolved (1 adopted: keys, not buttons)
  6. Responsive / input intention  3         8               DISAGREE → resolved (1/6 adopted)
  7. Accessibility basics          4         8               DISAGREE → resolved (8 adopted)
═══════════════════════════════════════════════════════════════════════
0/7 CONFIRMED by two voices (single model); every subagent finding was adopted
into the plan, so no design disagreement travels to the gate except Choice 5
(keys vs truck-cab interaction — a genuine taste call).
```

## Decision Audit Trail (Phase 2)

| # | Phase | Decision | Classification | Principle | Rationale | Rejected |
|---|-------|----------|----------------|-----------|-----------|----------|
| 27 | 2-0 | No mockups (designer needs an OpenAI key) → text wireframes | Mechanical | P6 | progressive enhancement, not a gate | waiting on `$D setup` |
| 28 | 2-P1 | Roster top-right, display-only, keys Enter/C | **Taste** (Choice 5) | P5 | only free HUD zone; HUD panels have no cursor; truck-cab E is the alternative | buttons on a HUD panel |
| 29 | 2-P1 | Drop ModeSelect; five-item title + friends hint | Mechanical | P5 | one hop, no jargon (Krug) | two-hop menu |
| 30 | 2-P1 | Drop one-option character selector and 새 판 from v1 | Mechanical | P5 | a dropdown with one entry is noise | keep placeholders |
| 31 | 2-P2 | Code-path failures return to the JoinCode cells with the code kept; `PendingNotice` for in-session failures | Mechanical | P1 | never lose the user's work; one message carrier | title banner |
| 32 | 2-P2 | Join/leave toast + `UI_Confirm`; "복사됨" toast | Mechanical | P1 | host may face away | silent joins |
| 33 | 2-P2 | 20 s game-side join timer | Mechanical | P3 | engine 80/120 s is for in-session drops | 80 s spinner |
| 34 | 2-P3 | 혼자 하기 works without Steam | Mechanical | P1 | `-nosteam` must boot a solo run | Steam-only solo |
| 35 | 2-P7 | 나가기 = `WBP_Pause` [로비로 나가기] | Mechanical | P4 | ui.md already reserved it | new surface |
| 36 | 2-P5 | DESIGN.md missing → `/design-consultation` TODO (P3) | Mechanical | P3 | outside this plan's blast radius | run it now |
| 37 | 1-spec2 | Unit tests + smoke pulled forward from build-order ⑥ (CEO row 20) | Mechanical | P1 | ⑥ excludes only snapshot tests | wait for ⑥ |
| 38 | 1-spec2 | `LeaveSession()` before travelling to the title on host loss (CEO row 21) | Mechanical | P1 | orphaned lobby otherwise (engine source) | travel only |
| 39 | 1-spec2 | Exact session-settings tuples: Public for 방 만들기, Private for 혼자 하기 (CEO row 22) | Mechanical | P5 | `BuildLobbyType` derives the lobby type | unspecified flags |
| 40 | 1-spec3 | `CL9_PHASE` lobby key checked before `JoinSession`; AC5 accepts "no entry OR refused with the night string" | Mechanical | P1 | Steam-client hiding of "Join Game" is unverified | assert the hide |
| 41 | 1-spec3 | Remove `SelectedCharacter` / `Server_SelectCharacter` (selector dropped in decision 30) | Mechanical | P5 | orphaned replicated field | keep for later |

```
  +====================================================================+
  |         DESIGN PLAN REVIEW — COMPLETION SUMMARY                    |
  +====================================================================+
  | System Audit         | no DESIGN.md; UI scope: 4 rows + 1 level     |
  | Step 0               | 5/10; all 7 dimensions in focus              |
  | Pass 1  (Info Arch)  | 4/10 → 9/10 after fixes                      |
  | Pass 2  (States)     | 6/10 → 9/10 after fixes                      |
  | Pass 3  (Journey)    | 5/10 → 8/10 after fixes                      |
  | Pass 4  (AI Slop)    | 6/10 → 9/10 after fixes                      |
  | Pass 5  (Design Sys) | 5/10 → 8/10 after fixes (no DESIGN.md)       |
  | Pass 6  (Responsive) | 3/10 → 8/10 after fixes                      |
  | Pass 7  (Decisions)  | 12 resolved, 1 → gate (Choice 5)             |
  +--------------------------------------------------------------------+
  | NOT in scope         | written (4 items)                            |
  | What already exists  | written                                      |
  | TODOS.md updates     | 1 item (DESIGN.md via /design-consultation)  |
  | Approved Mockups     | 0 generated (no API key), 0 approved         |
  | Decisions made       | 12 added to plan                             |
  | Decisions deferred   | 1 (gate Choice 5)                            |
  | Overall design score | 5/10 → 8/10                                  |
  +====================================================================+
```

> **Phase 2 complete.** Codex: unavailable. Claude subagent: 9 issues (9
> adopted). Consensus: 0/7 confirmed (single model), 1 taste disagreement →
> surfaced at gate (Choice 5). Phase 2.5 (DX) skipped — no developer-facing
> scope. Passing to Phase 3 (Eng — reviews the amended plan).

---

# /autoplan — Phase 3: Eng Review (auto-decided, reviews the amended plan)

Scope gate: autoplan — auto-selected B (this plan, as amended by Phases 1–2).
Design doc: CCL8 greenfield design (2026-08-27) read in Phase 0; consistent.
Codex: `[codex-unavailable]` → `[subagent-only]`.

## Step 0 — Scope challenge (with code)
1. **Existing code per sub-problem** (read, not recalled):
   - Joinable state: `ACL9GameState::Phase` + `SetPhase` (host-only),
     `ACL9GameMode::StartNight()` guards `Phase != Lobby` (`CL9GameMode.cpp:155-160`).
     Reused as-is.
   - Login hooks: `ACL9GameMode` overrides **none** of `PreLogin` / `PostLogin` /
     `Logout` today (`CL9GameMode.h` grep: no match) → three new overrides,
     nothing to refactor.
   - Server RPC pattern: `Server_AcceptOrders_Validate` returns true and
     `_Implementation` refuses non-hosts with a log
     (`CL9PlayerController.cpp:224-229`); `Server_CL9Cheat` same shape. New
     `Server_SetReady` / `Server_Depart` follow it (a `_Validate` false would
     kick the client — wrong tool for a phase check).
   - Registry: `UCL9DefRegistry::Init(const UCL9DefList*)` builds typed lists
     (`GetItems/GetOrders/GetSites/GetSegKinds…`, `CL9DefRegistry.h:78-129`).
     TuningHash = iterate every list, sort rows by (class name, Alias), serialize
     each row's script properties into a memory writer
     (`UObject::SerializeScriptProperties` via `FMemoryWriter`), hash the buffer
     with `CityHash64`, format as 16 hex chars. Deterministic across machines
     running the same build (the version gate guarantees the same layout).
   - Cheats: `RouteCL9Cheat(Command, Args, World, bLocalOnly)`
     (`CL9Cheats.cpp:68-96`) + `UCL9CheatManager::HandleCommand`
     (`CL9CheatManager.h:27`) — four new commands, `bLocalOnly=true`.
   - UI: `UCL9UIManager::OpenWidget/CloseWidget/IsOpen` + `CL9UINames`
     (`CL9UIManager.h:33-49`); HUD derives screens from state
     (`CL9HUDWidget.cpp:404-419` opens `OrderSelect` when `OfferedOrders` is
     non-empty) — the roster row is opened/closed by the same derivation on
     `Phase`.
   - Transport: engine plugins only (verified present, Steamworks 1.64).
   - Save: in progress in the concurrent session (`SaveRun/LoadRun`), untouched.
2. **Minimum set:** W1 + W2 + roster + Notice + W4 upload. Title/JoinCode are
   user-facing acceptance criteria (AC1–AC6), kept. Nothing further to defer.
3. **Complexity check:** ~14 source files + 5 widgets + 1 level + 2 input
   actions, 1 new class (`UCL9SessionSubsystem`) + 4 widget classes. Over the
   8-file smell, justified: a session layer necessarily touches config, GI,
   GameMode, GameState, PlayerState, PC and UI; Phase 1 already removed the
   avoidable parts (`ECL9SessionState`, seamless travel, Lobby screen,
   ModeSelect, character selector). No custom solution where a built-in exists
   (**[Layer 1]** OSS Steam sessions, SteamSockets, `PreLogin` `ErrorMessage`,
   `UEngine::OnNetworkFailure`).
4. **Search check:** built-ins confirmed in engine source (Phase 1/2 records):
   lobby sessions, joinable flag, delayed listeners, editor self-disable,
   `DriverClassNameFallback`. Known footguns captured: UE-239936, ini inline
   comments, `#` presence tokens, AppID 480 shared search space, `_Validate`
   semantics. **[Layer 3]** insight kept: night phase is the session axis.
5. **TODOS cross-reference:** TODOS.md (created in Phase 1) has 6 items; none
   blocks this plan; JIP and save/continue are unlocked by it.
6. **Completeness:** complete version chosen everywhere (Lake Score below).
7. **Distribution:** new artifact = a Steam build. Pipeline is in scope (W4:
   packaging config, `Build/Steam/*.vdf`, steamcmd to `beta`); target =
   Win64 only (SteamSockets plugin allow-list: Win64/Mac/Linux; Mac/Linux and
   Steam Deck are TODOS). Rollback = previous build set live in Steamworks.

## Section 1 — Architecture (eng)
```
 DEPENDENCY GRAPH (new → existing; arrows = "calls / reads")
   Config/DefaultEngine.ini ──► OnlineSubsystemSteam ──► SteamSockets ──► SteamSocketsNetDriver
                                        ▲                                       ▲
   UCL9SessionSubsystem [NEW] ──────────┘ (IOnlineSession/Identity/Friends)     │ (ClientTravel URL)
        │ delegates: OnSessionCreated / OnJoinFailed(reason) / OnSessionEnded  │
        ├──► UCL9GameInstance [+PendingNotice, +OnNetworkFailure/OnTravelFailure handlers (client-only)]
        ├──► UGameplayStatics::OpenLevel / APlayerController::ClientTravel
        └◄── ACL9GameMode [+PreLogin/PostLogin/Logout, +RequestDepart, +SetJoinable on phase] ──► StartNight/EnterLobby (existing)
                  │ writes                                     │ writes
                  ▼                                            ▼
        ACL9PlayerState [+bIsReady,+SlotIndex,+SelectedCharacter]   ACL9GameState [+SessionCode]
                  │ OnRep                                      │ OnRep
                  ▼                                            ▼
        WBP_Roster (HUD row, display-only) ◄── UCL9UIManager ◄── WBP_HUD derivation on Phase
        WBP_Title / WBP_JoinCode / WBP_Notice (rows) ──► UCL9SessionSubsystem (calls) 
        IA_LobbyAction / IA_CopyCode ──► ACL9PlayerController [+Server_SetReady, +Server_Depart, +CopyCode]
        UCL9CheatManager [+host/join/leave/session] ──► UCL9SessionSubsystem
        Build/Steam/*.vdf ──► steamcmd ──► Steamworks beta branch     Build/Smoke/*.ps1 ──► two -game processes
```
- Coupling: GameMode → GI subsystem (joinable flag) is the only new
  gameplay→online edge; it goes through one method (`SetJoinable(bool)`),
  no-op when the OSS is unavailable. Widgets never touch OSS types.
- **Production failure per integration point:** (1) Steam client dies
  mid-night → SteamSockets connection closes → clients `ConnectionLost` →
  LeaveSession + title (accounted). (2) Valve lobby service hiccup during
  `UpdateSession` → joinable flag stale → PreLogin phase gate still rejects
  (accounted, logged). (3) Host alt-tabs and the overlay opens during 출발 →
  nothing (input goes to overlay; game keeps running). (4) **Host receives
  `OnNetworkFailure` for a *client's* timeout** — the engine broadcasts
  failures on the server side too; a naïve handler would send the host to the
  title. → guard `GetNetMode() == NM_Client` (finding E1, P1, confidence 9:
  `UEngine::BroadcastNetworkFailure` is called from `UNetDriver` on both ends).
- Security: unchanged from Phase 1 §3; the client-supplied `?hash=`/`?ver=`
  are compared, never interpreted; RPCs act on the caller's own PlayerState.
- Distribution: Win64 build → steamcmd → `beta`; `Lvl_Title` must be in the
  packaging map list (it is `GameDefaultMap`, so it cooks) — verify in W4.
- Inline ASCII diagram targets: `CL9SessionSubsystem.h` (host/client state
  machine from Phase 1 §1), `CL9GameMode.cpp` above `PreLogin` (gate order:
  phase → count → hash → version), `CL9GameInstance.cpp` above the failure
  handler (client-only branch + PendingNotice flow).

## Section 2 — Code quality (eng)
| # | Finding | Sev | Conf | Decision |
|---|---|---|---|---|
| E1 | `OnNetworkFailure` fires on the host for client drops; handler must return early unless `NM_Client` | P1 | 9 | Add to W2 + smoke (kill the client process; host must stay) |
| E2 | TuningHash must not depend on `DefList` asset order or `TMap` iteration order | P1 | 8 | Sort rows by (class, Alias) before hashing; unit test "shuffled list → same hash" |
| E3 | `_Validate` returning false disconnects the client; phase/role checks belong in `_Implementation` with a log (existing `Server_AcceptOrders` pattern) | P2 | 9 | Follow the pattern; state it in W2 |
| E4 | 출발 authority: `RequestDepart(PC)` must check `PC->IsLocalController() && HasAuthority()` (the listen host's own PC), not "any PC with authority" | P2 | 8 | Explicit host check + log on refusal |
| E5 | Slot → `TintColor` mapping must come from data (character table), not a C++ array (conventions § 5) | P2 | 8 | `FindCharacter(alias)->SlotColors[SlotIndex]` or rules row; roster reads it |
| E6 | Failure strings must be `FText` (`NSLOCTEXT`) in one table, not `FString` literals scattered in widgets (localization later, settings.md) | P2 | 8 | One `CL9SessionStrings` namespace in the subsystem header |
| E7 | Async OSS callbacks may fire after the world changed; delegate handles must be cleared in `Deinitialize` and callbacks guard `IsValid(this)` | P2 | 8 | Part of the subsystem contract; smoke: 취소 during search |
| E8 | DRY: `cl9.host` and the title's 방 만들기 must call the same `HostSession()`; `cl9.join` and the JoinCode popup the same `JoinByCode()` — no duplicated flag tuples | P2 | 9 | Tuples live in one private function `MakeSessionSettings(EMode)` |
Over-engineering check: none found (no session state enum, no retry framework,
one subsystem). Under-engineering: E1/E7 were the two silent-failure candidates;
both now explicit. Existing ASCII diagrams in touched files: none in
`CL9GameMode.cpp` / `CL9PlayerState.h` today; session.md's screen sketches
become stale → W5 rewrites them (D5/T11).

## Section 3 — Test review (eng)
Framework detection: CLAUDE.md has no Testing section; no test files in the
tree (`git ls-files` finds none); UE Automation (`FAutomationTestBase`) is
available in the editor build — the plan introduces the first two tests
(T8). Manual scenarios live in docs/testing/pie-scenarios.md.
```
CODE PATHS                                                         USER FLOWS
[+] Source/CL9/Session/CL9SessionSubsystem.cpp                     [+] Host a room
  ├── HostSession(mode)                                              ├── [GAP] [→E2E S1] Steam on → code shown, overlay opens
  │   ├── [GAP]        OSS null → Unavailable (solo still works)     ├── [GAP] [→INT S0] -nosteam → listen level, no session call
  │   ├── [GAP]        already in session → refuse + log             └── [GAP] [→E2E S2b] host → 나가기 → host again (UE-239936)
  │   ├── [GAP]        OnCreateSessionComplete(false) → message    [+] Join by code
  │   └── [GAP]        ok → OpenLevel(?listen) + SessionCode set      ├── [GAP] [★unit T8] normalize: lower/space/0O1I/len 7
  ├── JoinByCode(code)                                               ├── [GAP] [→E2E S3] valid code → roster on both PCs
  │   ├── [GAP] [★unit] validate/normalize                           ├── [GAP] [→E2E S5] 5th player → "(4/4)" inline, code kept
  │   ├── [GAP]        FindSessions fail/timeout → message           ├── [GAP] [→E2E S5] hash mismatch → hash string inline
  │   ├── [GAP]        0 / 1 / >1 results                            ├── [GAP] [→E2E S5] version mismatch → version string
  │   ├── [GAP]        JoinSession result enum ×5 → string           ├── [GAP] [→E2E S5] during Night → night string; no Join Game in friends list
  │   └── [GAP]        20 s timer → cancel + string                  └── [GAP] [→E2E S3] 취소 during search → no callback crash
  ├── OnInviteAccepted                                             [+] Invite
  │   ├── [GAP]        warm (title) → JoinSession                     ├── [GAP] [→E2E S4] warm invite → roster
  │   ├── [GAP]        cold start → after first world                 └── [GAP] [→E2E S4] cold invite (game closed) → roster
  │   └── [GAP]        while hosting with players → ignore + log    [+] Lobby → night
  ├── LeaveSession / DestroySession (+ failure log)                   ├── [GAP] [→INT S0] client Enter → ready ✓ on host roster
  └── SetJoinable(bool) (+ OnUpdateSessionComplete(false) log)        ├── [GAP] [→INT S0] host Enter before all ready → refused + status line
[+] Source/CL9/GameMode/CL9GameMode.cpp                              ├── [GAP] [→INT S0] host Enter with offer pending → refused
  ├── PreLogin: phase / count(+travelling) / hash / version           ├── [GAP] [→INT S0] all ready + picked → Night on both (StartNight)
  │   └── [GAP] [→INT S0] each gate → ErrorMessage string             ├── [GAP] [→INT S0] EnterLobby → ready reset to false
  ├── PostLogin: slot assign (lowest free), phase re-check → kick      └── [GAP] [→INT S0] REGRESSION: cl9.night still starts a night
  │   └── [GAP] [→INT S0] join during StartNight window            [+] Leave / drop
  ├── Logout: slot free                                              ├── [GAP] [→INT S0] client Alt+F4 → slot freed, toast on host
  ├── RequestDepart(PC): host-only, all ready, no offer pending       ├── [GAP] [→E2E S6] host quits → clients: LeaveSession + Notice + title
  └── SetPhase paths call SetJoinable (StartNight/EnterLobby)         ├── [GAP] [→INT S0] host stays when a CLIENT times out (E1)
[+] Source/CL9/GameMode/CL9GameInstance.cpp                          └── [GAP] [→E2E S6] client → Pause → 로비로 나가기 → title
  ├── OnNetworkFailure: NM_Client only → LeaveSession → PendingNotice → title   [+] Regressions
  └── OnTravelFailure: same                                          ├── [GAP] [→INT A3] REGRESSION: PIE 2-client A3 after ini change (T14)
[+] Source/CL9/Data/CL9DefRegistry.cpp                               └── [GAP] [→E2E]    REGRESSION: all existing pie-scenarios on the Steam driver
  ├── [GAP] [★unit T8] same list → same hash; shuffled → same; one value changed → different
[+] Source/CL9/UI/* (Title/JoinCode/Notice/Roster)                 [+] UI states
  └── [GAP] [→E2E]    per-state copy from the Phase 2 table          └── [GAP] [→E2E]    47-char name clipped; solo roster; Steam-off title

COVERAGE TODAY: 0/41 paths tested (0%) — no tests exist in the project
PLANNED: unit ★★★ 2 (T8) | integration S0 smoke covers 14 | manual E2E S1–S6 covers 20 | 5 UI-state checks manual
QUALITY TARGET: ★★★ for the unit tests, ★★ for S0 (asserts on log lines), ★ for manual visual checks
GAPS AFTER PLAN: 0 untested paths; 0 REGRESSIONS uncovered (A3 + cl9.night in S0/T14)
```
- 2 a.m. Friday test: S0 green + S2 on the own AppID. Hostile QA: E1 (kill a
  client → host must not go to title), 취소 during search (E7), invite while
  hosting with 3 friends. Chaos: pull the host's cable → clients within 80 s
  get the Notice; Steam client killed → same.
- Flakiness: Steam paths are manual by nature; S0 is deterministic (localhost,
  `-nosteam`, log greps with timeouts).
- LLM/prompt changes: none.
- Test plan artifact written to
  `C:\Users\user\.gstack\projects\Work\user-master-eng-review-test-plan-20260906.md`.

## Section 4 — Performance (eng)
4 players; `PlayerArray` scans are O(4). TuningHash is one serialization pass
at Init (hundreds of rows, sub-millisecond). Lobby search filtered
server-side; `UpdateSession` twice a night. SteamSockets tick defaults exceed
the 20 Hz replication policy. Roster updates by OnRep only. No findings.

## Failure modes (eng — delta over Phase 1 registry)
```
  CODEPATH                   | FAILURE MODE                          | TEST   | HANDLED | USER SEES                 | CRITICAL?
  ---------------------------|---------------------------------------|--------|---------|---------------------------|----------
  GI OnNetworkFailure (host) | host handles a client drop as its own | S0     | E1 guard| nothing (correct)         | was CRITICAL before E1
  TuningHash                 | order-dependent hash → false rejects  | T8 unit| E2 sort | (would be) hash string    | was CRITICAL before E2
  Server_SetReady            | _Validate false kicks a client        | S0     | E3      | nothing                   | no
  RequestDepart              | client with authority? (none)         | S0     | E4      | refusal log               | no
  OSS callbacks after world  | dangling this / stale widget          | S3 취소| E7      | nothing                   | no
  Roster TintColor           | hardcoded colors drift from data      | visual | E5      | wrong color               | no
```
CRITICAL GAPS: 0 after E1/E2.

## Worktree parallelization
| Step | Modules touched | Depends on |
|---|---|---|
| W0 paperwork | — (Steamworks web) | — |
| W1 config | `CL9.uproject`, `Source/CL9/CL9.Build.cs`, `Config/` | T9 commit of the concurrent session |
| W2 session + gates | `Source/CL9/Session/`, `Source/CL9/GameMode/`, `Source/CL9/Data/` | W1 |
| W3 UI + input | `Source/CL9/UI/`, `Content/UI/`, `Content/Input/`, `Content/Maps/`, `Source/CL9/GameMode/CL9PlayerController` | W2 (PlayerState fields, subsystem API) |
| W4 smoke + tests + upload | `Build/`, `Source/CL9/Tests/`, packaging settings | W2 (smoke), W0 (upload) |
| W5 docs | `docs/` | W1–W4 |
Lanes: **A** = W1 → W2 (sequential, shared `GameMode/`). **B** = W3 (after A's
PlayerState/subsystem API lands; binary assets → one editor session). **C** =
W4 tests + smoke script (after A; no shared modules with B). **W0** runs from
day one in parallel. Execution: A, then B ∥ C, then W5. Conflict flag: B and A
both touch `GameMode/CL9PlayerController` — keep PC RPC additions in lane A.

## NOT in scope (eng) / What already exists (eng)
Unchanged from Phase 1 plus: Mac/Linux builds (plugin allow-list permits;
untested), Steam auth tickets (`OnlineAuthInterfaceSteam` — VAC off, friends
co-op), reconnect token / SteamID slot persistence (JIP TODO). Existing code
reused: everything in Step 0.

## Implementation Tasks (eng phase)
- [ ] **E1 (P1, human: ~1h / CC: ~10min)** — W2 — `UCL9GameInstance` failure handlers act only when `GetNetMode() == NM_Client`; host logs and continues
  - Surfaced by: Section 1 — production failure 4 (server-side `OnNetworkFailure` broadcast)
  - Files: Source/CL9/GameMode/CL9GameInstance.cpp
  - Verify: S0 — kill the client process; host stays in Lobby, roster row empties
- [ ] **E2 (P1, human: ~2h / CC: ~15min)** — W2 — TuningHash: rows sorted by (class, Alias), `SerializeScriptProperties` into `FMemoryWriter`, `CityHash64`, hex16; unit test: shuffled DefList → same hash
  - Surfaced by: Section 2 E2 — order-dependent hash would false-reject
  - Files: Source/CL9/Data/CL9DefRegistry.{h,cpp}, Source/CL9/Tests/CL9SessionCodeTest.cpp
  - Verify: `Automation RunTests CL9.TuningHash`
- [ ] **E3 (P2, human: ~1h / CC: ~10min)** — W2 — `Server_SetReady` / `Server_Depart`: `_Validate` true, phase/role checks in `_Implementation` with logs (pattern of `Server_AcceptOrders`)
  - Surfaced by: Section 2 E3
  - Files: Source/CL9/GameMode/CL9PlayerController.{h,cpp}
  - Verify: S0 — client Enter during Night → refused + log, no disconnect
- [ ] **E4 (P2, human: ~1h / CC: ~10min)** — W2 — `ACL9GameMode::RequestDepart(PC)`: `PC->IsLocalController()` on the server (listen host) + all ready + no pending offer → `StartNight()`; otherwise log the reason
  - Surfaced by: Section 2 E4
  - Files: Source/CL9/GameMode/CL9GameMode.{h,cpp}
  - Verify: S0 — client Enter → refused; host Enter with offer pending → refused
- [ ] **E5 (P2, human: ~1h / CC: ~10min)** — W3 — Slot → `TintColor` read from the character table (plan/data/character.md slot colors), exposed via `ACL9PlayerState::GetSlotColor()`
  - Surfaced by: Section 2 E5 — conventions § 5 (numbers from assets)
  - Files: Source/CL9/Data/CL9CharacterDef.h, Source/CL9/GameMode/CL9PlayerState.{h,cpp}, Content/UI/WBP_Roster
  - Verify: change a color in the DA → roster updates without a build
- [ ] **E6 (P2, human: ~1h / CC: ~10min)** — W2 — Failure strings as `NSLOCTEXT("CL9Session", …)` in one header; widgets read them, never literals
  - Surfaced by: Section 2 E6; Phase 2 string table
  - Files: Source/CL9/Session/CL9SessionStrings.h
  - Verify: grep for Korean literals in Source/CL9/UI finds none for session errors
- [ ] **E7 (P2, human: ~1h / CC: ~10min)** — W2 — Clear every OSS delegate handle in `Deinitialize`; callbacks bound with the subsystem as weak context; 취소 during search cancels cleanly
  - Surfaced by: Section 2 E7
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp
  - Verify: S3 — 취소 during 검색 중 → no callback after; S0 — quit during search → no crash log
- [ ] **E8 (P2, human: ~30min / CC: ~5min)** — W2 — One `MakeSessionSettings(EMode)` used by both the title and `cl9.host`; `JoinByCode` shared by the popup and `cl9.join`
  - Surfaced by: Section 2 E8 — DRY
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp, Source/CL9/GameMode/CL9Cheats.cpp
  - Verify: grep shows one place building `FOnlineSessionSettings`
- [ ] **E9 (P3, human: ~30min / CC: ~5min)** — W2 — Inline ASCII diagrams: session state machine in `CL9SessionSubsystem.h`, gate order above `PreLogin`, failure-handler flow in `CL9GameInstance.cpp` (Korean comments)
  - Surfaced by: Section 1 — diagram targets
  - Files: as named
  - Verify: diagrams present and match the plan's Phase 1 §1 state machines
- [ ] **E10 (P1, human: ~1h / CC: ~10min)** — W2 — `IsSteamAvailable()` = `IOnlineSubsystem::Get(STEAM_SUBSYSTEM)` non-null and enabled; never a bare `Get()` (Null OSS fallback); AC7 asserts no session in PIE / `-nosteam`
  - Surfaced by: Eng voice finding 1 — `OnlineSubsystemModule.cpp:216-219`, Null plugin `EnabledByDefault: true`
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp
  - Verify: `-nosteam` log shows "session: Steam unavailable — skipping"; 혼자 하기 still runs
- [ ] **E11 (P1, human: ~30min / CC: ~5min)** — W2/W4 — Travel options are `?`-separated (`?hash=X?ver=Y`); connect-string builder + `ParseOption` round-trip unit test
  - Surfaced by: Eng voice finding 2 — `GrabOption` splits on `?` only (`GameplayStatics.cpp:3410-3428`)
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp, Source/CL9/Tests/CL9SessionCodeTest.cpp, Build/Smoke/two-process-smoke.ps1
  - Verify: unit test green; S0 version gate passes with matching versions
- [ ] **E12 (P1, human: ~1h / CC: ~10min)** — W2 — "Full" gate counts `GetNumPlayers()` + pending `ClientConnections` without a PlayerController; `PostLogin` re-checks count and phase
  - Surfaced by: Eng voice finding 3 — `NumTravellingPlayers` is `AGameMode`-only; `ACL9GameMode : AGameModeBase`
  - Files: Source/CL9/GameMode/CL9GameMode.cpp
  - Verify: S0 variant — five clients connect within one second; exactly four land, the fifth gets "(4/4)"
- [ ] **E13 (P1, human: ~30min / CC: ~5min)** — W2 — `FindSessions` sets `SEARCH_LOBBIES=true`; not-found string gains the far-region hint
  - Surfaced by: Eng voice findings 5/6 — `OnlineSessionInterfaceSteam.cpp:778-789`; `k_ELobbyDistanceFilterDefault` hard-coded at `OnlineSessionAsyncLobbySteam.cpp:845`
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp, Source/CL9/Session/CL9SessionStrings.h
  - Verify: S3 finds the code; S3 note records the far-region behaviour
- [ ] **E14 (P2, human: ~1h / CC: ~10min)** — W2 — Failure handler passes the host's `ErrorString` through for `PendingConnectionFailure` / `FailureReceived`; fixed table only for Lost / Timeout / Outdated*
  - Surfaced by: Eng voice finding 4 — `UnrealEngine.cpp:16015-16017`
  - Files: Source/CL9/GameMode/CL9GameInstance.cpp
  - Verify: S5 — every PreLogin reject shows its own Korean string, not a generic one
- [ ] **E15 (P2, human: ~1h / CC: ~10min)** — W2 — Solo tuple `bShouldAdvertise=false`, `NumPublicConnections=1`; `bSoloRun` from `?solo` → `PreLogin` rejects everyone; unit test on `MakeSessionSettings(Solo)`
  - Surfaced by: Eng voice finding 7; spec review round 3 finding 1
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp, Source/CL9/GameMode/CL9GameMode.cpp, Source/CL9/Tests/CL9SessionCodeTest.cpp
  - Verify: unit test; S0 — `open` into a solo host is refused
- [ ] **E16 (P2, human: ~30min / CC: ~5min)** — W4 — Steam uploads are Shipping-only; Development builds stay on the two test PCs
  - Surfaced by: Eng voice finding 8 — `AllowCheats` true in non-Shipping (`CL9GameMode.cpp:55-64`)
  - Files: Build/Steam/app_build.vdf, docs/testing/pie-scenarios.md
  - Verify: the beta-branch build refuses `cl9.*` commands
- [ ] **E17 (P3, human: ~1h / CC: ~10min)** — W2 — Joinable flag + `CL9_PHASE` updated by the subsystem subscribing to `OnNightPhaseChanged` (host side); GameMode has no online calls
  - Surfaced by: Eng voice finding 11 — conventions § 2 "Steam is transport only"
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp
  - Verify: grep shows no `IOnlineSession` include in `CL9GameMode.cpp`; S5 still passes
- [ ] **E18 (P2, human: ~2h / CC: ~15min, optional)** — W4 — Dev-only `-cl9nullsession` switch to drive `HostSession`/`JoinByCode` over the Null OSS in S0 (drop if two local instances cannot discover each other)
  - Surfaced by: Eng voice finding 9 — async session paths are otherwise manual-only
  - Files: Source/CL9/Session/CL9SessionSubsystem.cpp, Build/Smoke/two-process-smoke.ps1
  - Verify: S0 exercises cancel-during-search and callback-after-Deinitialize without a crash

## Dual voices (eng)

**CODEX SAYS (eng — architecture challenge):** `[codex-unavailable]` (usage
limit until 2026-09-14). `[subagent-only]`.

**CLAUDE SUBAGENT (eng — independent review)** — read the plan after the
Phase 1–2 amendments (before the last spec-review-3 fixes). 11 findings, every
one checked against engine or project source:
1. (P1, 9) `IOnlineSubsystem::Get()` is never null — Null OSS fallback →
   **Accepted** (E10). This would have made 혼자 하기 create LAN sessions in PIE
   and hidden the "Steam off" state entirely.
2. (P1, 9) URL options split on `?`, not `&` → **Accepted** (E11). The smoke
   script as written would have rejected everyone on version.
3. (P1, 9) `NumTravellingPlayers` does not exist on `AGameModeBase` →
   **Accepted** (E12): pending `ClientConnections` + PostLogin count re-check.
4. (P2, 8) PreLogin rejects arrive as `PendingConnectionFailure` with the
   host string → **Accepted** (E14).
5. (P2, 9) `SEARCH_LOBBIES` key required for lobby search → **Accepted**
   (E13); the plan had `SEARCH_PRESENCE`, which would never find a code.
6. (P2, 6) Distance filter hard-coded to Default → **Accepted as a documented
   limit** (E13 string + S3 note); invites unaffected.
7. (P2, 8) Solo tuple contradiction → already fixed by spec review 3;
   `NumPublicConnections=1` + `bSoloRun` gate added (E15).
8. (P2, 8) Development beta ships host cheats → **Accepted** (E16):
   Shipping-only uploads.
9. (P2, 5) Session subsystem has no automated coverage; Null OSS LAN could
   drive it → **Accepted as an optional experiment** (E18, unverified).
10. (P3, 7) VAC gate item is a no-op → **Accepted**: gate Choice 4 removed.
11. (P3, 7) GameMode → `UpdateSession` violates "Steam is transport only" →
    **Accepted** (E17): subsystem subscribes to `OnNightPhaseChanged`.

```
ENG DUAL VOICES — CONSENSUS TABLE:
═══════════════════════════════════════════════════════════════
  Dimension                           Claude   Codex  Consensus
  ──────────────────────────────────  ───────  ─────  ─────────
  1. Architecture sound?              PARTIAL  N/A    PARTIAL → E10/E17 adopted
  2. Test coverage sufficient?        NO       N/A    DISAGREE → E11/E12/E15 unit+smoke added; E18 optional; Steam paths stay manual (accepted limit)
  3. Performance risks addressed?     YES      N/A    N/A (single voice) — no findings either side
  4. Security threats covered?        PARTIAL  N/A    PARTIAL → E16 adopted
  5. Error paths handled?             PARTIAL  N/A    PARTIAL → E10/E14 adopted
  6. Deployment risk manageable?      YES      N/A    N/A (single voice) — beta branch + revert + version gate
═══════════════════════════════════════════════════════════════
0/6 CONFIRMED by two voices (single model). Three P1 findings adopted. No
user challenge from the eng voice.
```

## Decision Audit Trail (Phase 3)

| # | Phase | Decision | Classification | Principle | Rationale | Rejected |
|---|-------|----------|----------------|-----------|-----------|----------|
| 42 | 3-0 | Complexity smell (>8 files) accepted — session layer spans the stack by nature; avoidable parts already removed | Mechanical | P2 | scope challenge never reduces in autoplan | reduce |
| 43 | 3-S1 | Failure handlers client-only (E1) | Mechanical | P1 | server-side broadcast would send the host to the title | — |
| 44 | 3-S2 | TuningHash sorted + serialized + CityHash64 (E2) | Mechanical | P5 | order-independent, one pass | property-by-property reflection walk |
| 45 | 3-S2 | RPC checks in `_Implementation` (E3), host check in `RequestDepart` (E4), data-driven slot color (E5), `NSLOCTEXT` table (E6), delegate cleanup (E7), one settings builder (E8) | Mechanical | P4/P5 | existing patterns, DRY | — |
| 46 | 3-S3 | Unit ×3 + S0 smoke + manual S1–S6; Steam paths manual by nature | Mechanical | P1 | dependency cannot be mocked cheaply; E18 optional | skip tests |
| 47 | 3-voice | Steam availability predicate = `Get(STEAM_SUBSYSTEM)` (E10) | Mechanical | P1 | Null OSS fallback verified in source | bare `Get()` |
| 48 | 3-voice | `?`-separated travel options (E11) | Mechanical | P1 | engine parser | `&` |
| 49 | 3-voice | Pending-connection count + PostLogin count re-check (E12) | Mechanical | P1 | no `NumTravellingPlayers` on `AGameModeBase` | trust `NumPlayers` |
| 50 | 3-voice | `SEARCH_LOBBIES`; far-region limit documented (E13) | Mechanical | P1 | OSS Steam search routing | master-server search |
| 51 | 3-voice | Pass PreLogin `ErrorString` through (E14) | Mechanical | P1 | `PendingConnectionFailure` carries it | per-enum only |
| 52 | 3-voice | Shipping-only Steam uploads (E16) | Mechanical | P3 | `AllowCheats` true in non-Shipping | dev beta |
| 53 | 3-voice | VAC gate item removed | Mechanical | P5 | flag unread on this path | keep a no-op decision |
| 54 | 3-voice | Joinable/`CL9_PHASE` via subsystem subscription (E17) | Mechanical | P5 | GameMode stays Steam-free (conventions § 2) | GameMode calls Steam |
| 55 | 3-voice | Null-OSS session smoke as optional experiment (E18) | Mechanical | P6 | unverified feasibility; cheap to try | mandatory |

## Completion summary (eng)
- Step 0: Scope Challenge — scope accepted as-is (complexity smell justified; nothing further deferrable)
- Architecture Review: 1 issue found (E1) + 3 from the voice (E10, E12, E17)
- Code Quality Review: 8 issues found (E2–E8, E11)
- Test Review: diagram produced, 41 paths, 0 gaps after plan (3 unit, 1 integration, 6 manual E2E); E18 optional
- Performance Review: 0 issues found
- NOT in scope: written · What already exists: written
- TODOS.md updates: 0 new from eng (6 items already collected from Phases 1–2)
- Failure modes: 0 critical gaps (2 were critical before E1/E2)
- Outside voice: ran (claude subagent) `[subagent-only]`
- Parallelization: 3 lanes — A (W1→W2) sequential, then B (W3) ∥ C (W4 tests/smoke); W0 parallel from day one
- Lake Score: 9/9 recommendations chose the complete option

> **Phase 3 complete.** Codex: unavailable. Claude subagent: 11 issues (11
> adopted, 2 as documented limits/optional). Consensus: 0/6 confirmed (single
> model), 1 disagreement (test coverage) → resolved by E11/E12/E15/E18, noted
> at the gate. Passing to Phase 4 (Final Gate).

---

# /autoplan — Cross-phase themes

- **Theme: "engine source before belief"** — flagged independently in Phase 1
  (the `bInitServerOnClient` claim reversed by the spec review) and Phase 3
  (Null OSS fallback, `SEARCH_LOBBIES`, `?`-separated options, no
  `NumTravellingPlayers` on `AGameModeBase`). High-confidence signal: every W1/W2
  step begins by reading the 5.8 plugin source for the call it is about to make;
  the plan cites file:line for each verified fact and marks the rest "unverified".
- **Theme: 혼자 하기 kept slipping** — design voice (state undefined, must work
  with Steam off), spec review 3 (lobby type contradiction), eng voice (tuple +
  no solo gate). Now pinned: Private lobby tuple, `NumPublicConnections=1`,
  `bSoloRun` PreLogin reject, unit test, "혼자 · Enter: 출발" roster.
- **Theme: Steam paths cannot be automated** — Phase 1 spec review (smoke bypasses
  the subsystem) and Phase 3 eng voice (async paths manual-only). Accepted as a
  limit with mitigations: S0 covers every gate and replication path, E18 tries the
  Null-OSS route, S1–S6 are the manual contract on two PCs.

Gate inputs: 4 taste choices (single-level lobby / mid-night join = reject /
first Steam artifact / roster keys vs truck-cab E) and 1 user challenge (defer
W2–W4 until 4e–4i + save). VAC was removed from the gate (no-op).

- [ ] **D5 (P3, human: ~1h / CC: ~10min)** — W5 — ui.md: add the roster zone (top-right) and the Notice/Roster/JoinCode rows to the HUD/UI tables
  - Surfaced by: Pass 5
  - Files: docs/systems/ui.md, docs/systems/ui_system.md
  - Verify: docs list every new row
