# TODOS

## Session / Steam

### Rejoin into a running night (JIP)

**What:** Let a player who dropped (or a late friend) join while the night phase is `Night`, spawning in the truck zone with state restored from GameState replication.

**Why:** Today a drop means waiting for the next truck stop; session.md's "중간 참가" question is still open. Closing it well needs spawn rules and screen restoration, not just opening the gate.

**Context:** After `docs/goal/steam-release.md` ships, `ACL9GameMode::PreLogin` rejects joins while the phase is not `Lobby`. Opening this means: remove that gate for reconnecting SteamIDs (or all), spawn at the truck pad, derive the screen from the phase (ui_system.md), skip missed sequence multicasts (sequence.md). Decision first (allow at all? only reconnects?), then code. Start in `CL9GameMode.cpp` PreLogin/PostLogin and `docs/systems/session.md` 중간 참가.

**Effort:** M
**Priority:** P2
**Depends on:** steam-release W2 (session layer, PreLogin gates)

### Save / continue a run ("판") from the lobby

**What:** The roster panel shows the host's current run (day, money) with 새 판 / 이어하기, backed by `UCL9SaveGame`.

**Why:** session.md's lobby design shows the run slot; the plan ships "새 판" only because the save system (save.md) does not exist yet.

**Context:** The save system is being built in the gameplay session (2026-09-06: `Source/CL9/GameMode/CL9SaveGame.h`, `ACL9GameMode::SaveRun/LoadRun`, uncommitted). Once it is committed, the roster panel reads the slot (day, money) and the host picks 새 판 / 이어하기 before 출발; clients see the host's run info via GameState replication (`CurrentDay`, money already replicated). Solo and multiplayer share the same slot.

**Effort:** M
**Priority:** P2
**Depends on:** the save-system commit; steam-release W3 (roster panel)

### Rich presence status in the Steam friends list

**What:** Show "N일차 · k/4" next to the host's name in friends' lists via `IOnlinePresence::SetPresence`.

**Why:** Makes "Join Game" self-explanatory; the plan deferred it because OSS Steam sends the status as a `steam_display` localization token (`OnlinePresenceInterfaceSteam.cpp:143-151`) that needs a Rich Presence localization file uploaded in Steamworks — not possible on the shared dev AppID 480.

**Context:** After the own AppID exists (steam-release W0/W4): upload a `richpresenceloc_korean.vdf` with a `#Status_Day` token using `%day%` / `%count%` properties, then call `SetPresence` with the token and set the properties on phase / player-count change. Representation only; no gameplay dependency.

**Effort:** S
**Priority:** P3
**Depends on:** steam-release W4 (own AppID)

### Steam Deck / Proton pass

**What:** Run the packaged build on Steam Deck (or Proton on Linux) and fix input/overlay/performance issues.

**Why:** A meaningful share of co-op indie players are on Deck; Valve's "Verified" badge affects visibility.

**Context:** Needs hardware. Check: Steam overlay via Proton, SteamSockets relay, gamepad focus on Title/JoinCode (CommonUI focus cvars are on), 30 fps floor on the generated map.

**Effort:** M
**Priority:** P3
**Depends on:** steam-release AC8 (build on the own AppID)

### Achievements / leaderboards

**What:** Steam achievements (first night survived, day 7, etc.) via `IOnlineAchievements`; optional best-day leaderboard from `UCL9RecordSave`.

**Why:** Post-launch retention hooks; Steam surfaces them on the store page.

**Context:** OSS Steam implements both interfaces; definitions live in Steamworks. Keep them out of the host-authority loop (representation only). No code until the loop is complete and the game is live.

**Effort:** M
**Priority:** P4
**Depends on:** release

## UI

### DESIGN.md via /design-consultation

**What:** Run `/design-consultation` to produce a DESIGN.md (typeface, spacing scale, accent color, panel treatment) before ⑤ art replaces the placeholder UI.

**Why:** The Steam-release design review (2026-09-06) had no design system to calibrate against; every new panel (Title, JoinCode, Notice, Roster) was specified against ui.md's de-facto rules (24pt base, 16pt minimum, `ApplicationScale` 1.4, HUD zones) only.

**Context:** Existing widgets: WBP_HUD, WBP_Pause, WBP_Spectate, WBP_OrderSelect, WBP_Scan/ScanLabel plus the four from steam-release. The consultation should pick one UI typeface (the code cells and the title need it), the amber accent already implied by the lamp light, and the scrim treatment for panels over the night scene.

**Effort:** S
**Priority:** P3
**Depends on:** steam-release W3 (so the consultation sees the real screens)

## Completed
