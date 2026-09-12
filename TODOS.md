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

## Workflow (세션 조율 — docs/goal/session-orchestration.md)

### Agent Teams migration for editor-in-the-loop roles

**What:** If the Agent Teams spike (session-orchestration I) runs on this Windows box, dispatch the roles the user steers by eye (WBP wiring, level placement, ABP graphs) as teammates instead of in-process subagents.

**Why:** Teammates are live, addressable sessions with a shared task list — the closest built-in shape to the terminals the user runs by hand today; subagents hide 120-turn C++ work until a report.

**Context:** Role files in `.claude/agents/cl9-*.md` are reusable as team members (docs: agent-teams). Enable with `CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1`; on Windows without tmux teammates are in-process, cannot run background subagents, cannot `/resume`. The lock (`.claude/hooks/cl9_lock.py`) and the hook apply unchanged. Start in `.claude/skills/cl9/SKILL.md` Dispatch phase: replace `Agent(subagent_type: cl9-<role>)` with a team spawn for `default_mode: C` roles.

**Effort:** M
**Priority:** P2
**Depends on:** session-orchestration rollout 4 (dispatcher) + spike I recorded positive

### Scripted PIE smoke per role

**What:** Drive each role's PIE section (`docs/testing/pie-scenarios.md` A–L) from the `/cl9` integration step: `cl9.*` cheats via MCP + `HighResShot 1` screenshots into `.claude/runs/<slug>/pie/`, pass/fail per section recorded in `run.json`.

**Why:** Today PIE verification is done by hand through MCP; a scripted pass makes the "one build, one PIE" step repeatable and gives the report evidence instead of prose.

**Context:** Cheats listed in `docs/systems/cheats.md`; `Tools/Unreal/qa_*_pie.py` already script single-asset PIE checks through `run_python.py`; `SlateInspector Screenshot` is empty in PIE — use `HighResShot 1` (Saved/Screenshots/WindowsEditor). Deferred from the CEO review (3–5 files, touches MCP flows; PIE by hand works).

**Effort:** M
**Priority:** P3
**Depends on:** session-orchestration rollout 4

### Session naming probe (`cl9-ui-1`)

**What:** Find out whether a Claude Code session can set the name that `ListAgents` shows for it; if yes, `/cl9 role <name>` names the session `cl9-<role>-N`.

**Why:** Session aliases were confused at least twice (cl9-5b vs cl9-2d on 09-11); role-named sessions make `ListAgents`, lock owners and SendMessage targets self-explanatory.

**Context:** Names like `cl9-2d` are auto-assigned today (session `agent-name` records). Check the docs/`/rename`-style commands; if only the lead can name subagents, fall back to writing the role into the lock `owner` label and the `ListAgents` announcement message.

**Effort:** S
**Priority:** P3
**Depends on:** None

### `Workflow` script variant of `/cl9`

**What:** A `.claude/workflows/cl9-split.js` that runs triage → parallel role agents → integration as deterministic phases with resume-by-run-id.

**Why:** Only if `/cl9` runs outgrow one lead context; the Workflow tool adds a phase view and resume, but requires an explicit opt-in phrase every run and uses the same in-process subagents.

**Context:** Deferred from the CEO review; `run.json` + `/cl9 resume` cover resume today. Revisit after five real runs if the lead's context is the recurring failure.

**Effort:** M
**Priority:** P4
**Depends on:** session-orchestration rollout 4

### Untrack `.claude/settings.local.json`

**What:** `git rm --cached .claude/settings.local.json` and add it to `.gitignore`; keep the hook and allowlist in the tracked `.claude/settings.json`.

**Why:** The file is machine-local by design but is tracked today, so per-machine permission lists leak into commits and diverge.

**Context:** Found during the DX review of session-orchestration (2026-09-12). One-line cleanup; do it in the same commit as the hook so the two settings files have a clear split (tracked = repo rules, local = machine allowlist).

**Effort:** S
**Priority:** P3
**Depends on:** None

### Friction audit of `/cl9` runs

**What:** After five real `/cl9` runs, read the `.claude/runs/*/run.json` files (wall time, denials, audit violations, build count) and prune sub-commands, questions or fences nobody used.

**Why:** The DX review scored measurement 8/10 because the data is recorded but no cadence reads it; a solo developer bypasses ceremony that does not pay for itself, and the tool rots.

**Context:** `/cl9 status` shows the last 48 h; the audit is the same data over a longer window. Compare the dispatched wall times against the serial number recorded in the dry-run F.

**Effort:** S
**Priority:** P3
**Depends on:** five recorded runs

## UI

### DESIGN.md via /design-consultation

**What:** Run `/design-consultation` to produce a DESIGN.md (typeface, spacing scale, accent color, panel treatment) before ⑤ art replaces the placeholder UI.

**Why:** The Steam-release design review (2026-09-06) had no design system to calibrate against; every new panel (Title, JoinCode, Notice, Roster) was specified against ui.md's de-facto rules (24pt base, 16pt minimum, `ApplicationScale` 1.4, HUD zones) only.

**Context:** Existing widgets: WBP_HUD, WBP_Pause, WBP_Spectate, WBP_OrderSelect, WBP_Scan/ScanLabel plus the four from steam-release. The consultation should pick one UI typeface (the code cells and the title need it), the amber accent already implied by the lamp light, and the scrim treatment for panels over the night scene.

**Effort:** S
**Priority:** P3
**Depends on:** steam-release W3 (so the consultation sees the real screens)

## Completed
