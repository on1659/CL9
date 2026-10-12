# goal: loop-7a-wipe-penalty

**Status**: done (2026-10-12) — see Results. **Commit (user decision 2026-10-12)**: only the settlement-every-3 part is committed now; the wipe-penalty code/docs sit on other sessions' uncommitted base (start-lobby · round-start-truck-intro · voice) and are committed after that base lands. Build order ⑦ step **7a** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decisions 2 (settlement every 3 rounds) and 5 (a wipe is not game over), open question 4 closed at **30%** (2026-10-12).

## One-line Goal
A wipe ends only that round (dawn → lobby → everyone revived → next round) and takes 30% of the run's accumulated ether, and settlement happens every 3rd day instead of every 2nd.

## Background / Motivation
- 2026-10-05 the user asked for "if I die the game should end" (`CL9Rules.bWipeEndsRun`, intended true). 2026-10-11 the user reversed it: a wipe ends the round, not the run, and costs n% of the accumulated money; n = 30% on 2026-10-12.
- 2026-10-11 the user also moved the company settlement to every 3 rounds. `SettlementIntervalDays` and `QuotaCurve` are asset values.
- The player calls a play segment a **round (판)**; code names (`Night`, `EndNight`) stay.

## In-scope
1. **Wipe = round over.** `bWipeEndsRun` intended value becomes **false**: header default stays false, the `Content/Data/CL9Rules.uasset` cell is set to false. The true branch (game over, `EndRunAtDawn`) stays in code as a switch so the 2026-10-05 rule can come back with one cell.
2. **Wipe penalty.** New `CL9Rules` cell `WipePenaltyRate` (float 0–1, header default 0.30, load check: finite and within [0, 1]).
   - Trigger: after the round is torn down (`EndNight` → dawn → `LeaveBehindOutsideTruck`), if nobody is alive and the run did not end (`bWipeEndsRun` false). This one spot covers both a mid-round wipe (`HandlePlayerDeath` → `EndNight`) and "everyone left behind outside the truck" at round end.
   - Amount = `floor(SharedMoney × WipePenaltyRate)` (ether is an integer, round down), taken through `ACL9GameState::Deduct` (balance never below 0). Once per wipe. `PeriodIncome` is not touched — settlement judges period income.
   - One log line with before/after money and the rate.
   - The dawn screen shows one extra line `전멸 — 이더 N 차감` (also when N = 0). The value is replicated (`ACL9GameState`) and cleared when the next round starts, like the left-behind names.
3. **Settlement every 3.** `Content/Data/CL9Progression.uasset`: `SettlementIntervalDays` 2 → 3, first `QuotaCurve` key moves from day 2 to day 3 (value 800 unchanged). Header default `SettlementIntervalDays` = 3. No code change in settlement logic.
4. **Docs.** systems/session.md, systems/balance.md (wipe & settlement as implemented), plan/data/rules.md (`bWipeEndsRun` row + new `WipePenaltyRate` row), plan/data/progression.md and plan/loop.md (n% → 30%, "not applied" → applied), decisions.md (2026-10-12 entry naming the cell), build-order.md 7a → done. Keep the user's existing uncommitted corrections in rules.md / loop.md.
5. **Automation test** for the penalty arithmetic (`CL9.Run.WipePenalty`).

## Out-of-scope
- 7b yard no-spawn, 7c kitchen in building, external currency, imposter, team kill, balance curves, any other economy change.
- Re-judging a wipe on logout (unchanged — session.md "아직 없는 것").
- Where/how the dead revive (unchanged — truck pad `WakeSpot`).

## Acceptance Criteria
- [x] Build `CL9Editor` with the `C:\Program Files\Epic Games\UE_5.8` engine succeeds.
- [x] PIE 1 player, `Lvl_Henhouse`: mid-round wipe (`cl9.damage 99 0`) → lobby, player alive, log shows money before/after and 30%, dawn screen shows `전멸 — 이더 N 차감`.
- [x] Round-end wipe (`cl9.night` while outside the truck) takes the same penalty once.
- [x] Money 0 → wipe → money stays 0 (no negative), dawn line shows 0.
- [x] Setting the `bWipeEndsRun` cell true restores the old game over (run-over state); set back to false afterwards.
- [x] `cl9.day` cycling: settlement fires on days 3 and 6, not on days 2 and 4; first target 800.
- [x] Listen 2-player PIE once: after a wipe the client is alive in the lobby too.
- [x] Automation test `CL9.Run.WipePenalty` passes.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | `bWipeEndsRun` comment, new `WipePenaltyRate` + load check + `WipePenaltyFor(Money)` |
| `Source/CL9/Data/CL9ProgressionDef.h` | `SettlementIntervalDays` header default 3 |
| `Source/CL9/Data/CL9Types.h` | `FCL9WipePenalty` replicated result |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | `ApplyWipePenalty()` called from `EndNight`; clear at `StartNight` |
| `Source/CL9/GameMode/CL9GameState.{h,cpp}` | replicated `WipePenalty`, setter, OnRep, native delegate |
| `Source/CL9/UI/CL9DawnWidget.{h,cpp}` | extra result line |
| `Source/CL9/GameMode/CL9WipePenaltyTests.cpp` | arithmetic test |
| `Content/Strings/CL9_Strings.csv` (+ generated `CL9.csv`) | `Dawn.WipePenalty` (ko only) |
| `Content/Data/CL9Rules.uasset`, `Content/Data/CL9Progression.uasset` | cell values |

## Must-Preserve
- `bWipeEndsRun` true path (`FinishRunOver`, `EndRunAtDawn`, `GameState.RunOver`, game-over screen) works unchanged when the cell is true.
- Settlement judges `PeriodIncome`, never `SharedMoney`; `OnSettled` / `LastSettlement` contract unchanged.
- Left-behind flow order (`SetPhase(Dawn)` before deaths), `EnterLobby` order (dawn screen before `Settle`, save after settle).
- Other sessions' uncommitted changes in the same files are kept; only this step's lines are added.
- All numbers come from asset cells (no hard-coded 0.3 in game logic). Comments in Korean.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above for the whole step (small, but touches shared game-mode flow and replication). A cheaper model is fine for the doc edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Editor / build / PIE go through the lock CLI (`docs/workflow/sessions.md` "잠금"), lock commands issued alone.

## Results (2026-10-12)
- **Build**: `CL9Editor Win64 Development` with `C:\Program Files\Epic Games\UE_5.8`, editor closed — succeeded first try (45 s). Editor relaunched on `Lvl_Henhouse`, CL9 DLL loaded.
- **Assets** (MCP `set_properties` + `save_assets`, read back): `CL9Rules.bWipeEndsRun` true → **false**, `WipePenaltyRate` 0.30 (header default, not serialized);
  `CL9Progression.SettlementIntervalDays` **3** (the asset held no explicit value — it followed the header default), `QuotaCurve` keys (2, 800) → **(3, 800)**, then (4, 1600), (6, 2600), (10, 5200) unchanged.
- **PIE 1 player, `Lvl_Henhouse`** (editor Python via `Tools/Unreal/run_python.py`, cheats in the host world):
  1. Night 1, `cl9.money 1000` → `cl9.damage 99 0`: `Wipe penalty: 30% of 1000 ether — 300 taken. Money 1000 -> 700.` → `revived at the truck` → `Lobby — day 2`. Period income stayed 1000.
     Dawn: `이번 기간 1,000 이더` / `전멸 — 이더 300 차감`. No settlement on day 2. (The expired-order fine that follows — `Penalties: 500 ether due` — is the existing order rule.)
  2. Money 0 → night 2 wipe: `30% of 0 ether — 0 taken. Money 0 -> 0.`, dawn line `전멸 — 이더 0 차감`; same dawn: `Settlement passed on day 3: earned 1000 >= 800` (first target 800 on day 3).
  3. Night 3 ended with `cl9.night` while the player stood outside the cargo box: `Round end: 1 player(s) left behind` → one `Wipe penalty … 300 taken` → `Lobby — day 4`, no settlement on day 4. Dawn showed both the left-behind and the wipe line.
  4. `cl9.day 5` → night → end: `Lobby — day 6` → `Settlement failed on day 6: earned 1000 < 2600` (settles on day 6, target = curve key 6).
  5. `bWipeEndsRun` set true in memory → wipe: `Wipe ends the run — game over in 3.0s.` → `Run over — ended on day 1`, phase Dawn, money 1000 untouched (no penalty). Set back to false; package re-saved clean.
- **PIE listen 2 players** (`PIE_ListenServer`, 2 clients): money 1000, `cl9.damage 99 1` then `cl9.damage 99 0` → one penalty (1000 → 700), both `revived at the truck`, `Lobby — day 2`.
  Client world (`UEDPIE_1`): phase Lobby, day 2, local pawn alive, both players alive.
- **Automation**: `CL9.Run.WipePenalty` — `Result={Success}`.
- **Observed, not changed (pre-existing)**: in the 2-player run the dawn screen opened only on the host; the client had no dawn widget after the wipe.
  **Corrected 2026-10-12 (session 8dca39 investigated): a test artifact, not a game bug.** UFUNCTIONs called from editor Python run under `FEditorScriptExecutionGuard`
  (PythonScriptPlugin `PyUtil.cpp` `InvokeFunctionCall`), and under that guard `AActor::GetFunctionCallspace` returns Local — so the cheat-driven
  `EnterLobby` → `Client_OpenScreen(Dawn)` ran on the host's copy instead of reaching the client. Ending the night by the C++ expiry timer opens the dawn screen on the client
  in the same frame. The same applies to `Multicast_Settled` in Python-driven tests (replicated properties are unaffected). The spectate-close race guessed here was wrong.
  Also, a dawn screen left open across a round (cheat-driven test, nobody pressed 계속) keeps its old title/settlement text while the appended lines refresh.
