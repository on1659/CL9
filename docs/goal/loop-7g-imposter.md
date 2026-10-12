# goal: loop-7g-imposter

**Status**: done (2026-10-12 — listen 2-player PIE verified, uncommitted). Build order ⑦ step **7g** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decision 3 (imposter), decision 4 (killing the imposter pays), open questions 5 and 9 answered 2026-10-12.

## One-line Goal
At round start the host secretly makes 0 or 1 player the imposter (only that player is told); at round end the host judges it — win if every other player is dead, none of them by the imposter's own hand and none by being left behind, and the imposter is alive inside the truck — pays the imposter or whoever killed them in 이더, and reveals the result to everyone on the dawn screen.

## Background / Motivation
- Decision 3 (2026-10-11): random 0~1 imposter per round. Win = everyone else dead **and** the imposter alive back at the truck when it leaves. Striking anyone directly = automatic failure; any other cause is fine. Reward is out-game currency only. A bonus, never required.
- Decision 4: whoever kills the imposter earns out-game currency; killing a non-imposter has no penalty (7d already lets swings hit players).
- User answers 2026-10-12 (this chain):
  - **Chance** (open question 5): **3 players 10%, 4 players 25%**; 1–2 players never.
  - **Amounts** (open question 5): "balance adjustable in the table, move on" — data cells with draft values.
  - **Left behind** (open question 9): **not counted** — if anyone dies by being left outside the truck, the imposter has not won.
  - **Reveal**: dawn screen shows everyone who the imposter was and how it ended.
  - Imposter rewards must be larger than the survival reward (7f answer). Draft: survival ≈ 3–10 이더/round, imposter win 30, imposter kill 20.
- Recon: `ACL9PlayerState::KilledByPlayer` (7d) already records a player killer on the host. `LeaveBehindOutsideTruck` kills everyone outside the truck at round end; the lever is host-only (`ACL9DepartLever::Interact`), so a client imposter whose host died leaves when the round clock runs out.

## In-scope
1. **Cells** (`CL9Rules`, category "Imposter"): `ImposterChanceByPlayers` (float array, index = players in the round − 1, past the end = last entry; draft `[0, 0, 0.1, 0.25]`), `ImposterWinReward` (int, draft 30), `ImposterKillReward` (int, draft 20). Load check: chances finite 0..1, rewards ≥ 0. `ImposterChanceFor(Players)` helper.
2. **Role** — `ACL9PlayerState::bImposter`, replicated **owner only** (no other machine learns it), host setter `HostSetImposter(bool)` + `OnImposterChanged` (OnRep on clients, direct broadcast on the listen host).
3. **Assignment** — `ACL9GameMode::AssignImposter()` in `StartNight` after the population spawn: players with a living pawn; roll `FMath::FRand() < ImposterChanceFor(N)`; pick one uniformly. Cheat override (below) wins. One log line (host log only).
4. **Private notice** — the imposter's HUD shows `Hud.ImposterAssigned` in the existing place-notice slot for `ImposterNoticeDuration` (HUD property, 6 s) when its own flag turns on. Nobody else sees anything.
5. **Judgement** — `ACL9GameMode::JudgeImposter()` in `EndNight` **before** the left-behind step (so "outside the truck" is still "alive outside", which can never be a win):
   - **Killed**: the imposter is dead and `KilledByPlayer` is set → that player is the killer.
   - **Won**: the imposter is alive and inside the truck, there is at least one other player, every other player is already dead, and no other player's `KilledByPlayer` is the imposter.
   - **Failed**: anything else (someone alive, struck a player, dead by a monster, outside the truck…). Log the reason.
   The flag is cleared after judging, so the same player can be told again next round.
6. **Rewards** — `GrantRoundWallet` folds them into the one grant per player per round (7f): the winning imposter gets `ImposterWinReward` **instead of** the survival reward; a failed imposter who came home gets the normal survival reward; the imposter's killer gets `ImposterKillReward` **on top** (alive or not).
7. **Reveal** — `FCL9ImposterResult` (replicated on the game state, cleared at round start): dawn line `Dawn.ImposterWon` / `Dawn.ImposterKilled` / `Dawn.ImposterFailed`. No line when the round had no imposter.
8. **Cheat** `cl9.imposter [index]` (host) — no argument prints the current imposter and any forced pick; an index into the player list makes that player the imposter now if a round is running, or forces it for the next round from the lobby; `-1` clears (no imposter now / none forced).
9. **Docs**: plan/data/rules.md (cells), systems/session.md (round end order), systems/ui.md (notice + dawn lines), systems/cheats.md, decisions.md, build-order 7g → done, goal README.

## Out-of-scope
- Kill **contribution** (monster woken by a player) and its three uses — 7h.
- Letting clients pull the lever when the host is dead; imposter-specific abilities; voice changes for the imposter; telling other players mid-round.
- Balance of the chances and rewards beyond the draft cells.

## Acceptance Criteria
- [x] `CL9Editor` builds; `CL9.Run.Imposter` (chance lookup + reward folding) passes.
- [x] Listen 2 players: `cl9.imposter 0` during a round → host log names the host; only the host's HUD shows the notice (the client's HUD does not).
- [x] Win: host = imposter, client dies by a monster (`cl9.damage 99 1`), host in the truck, round ends by the clock → `Imposter: … won`, host wallet +30 (not the survival reward), dawn on both machines shows `임포스터: <host> — 성공`.
- [x] Direct strike: host = imposter swings the client to death → round end `failed — struck … directly`.
- [x] Killed: client = imposter, host swings the client to death → `killed by '<host>'`, host wallet gets survival reward + 20, dawn shows the killer.
- [x] Left behind: host = imposter, client alive outside the truck at round end → `failed`, dawn `실패`.
- [x] 2 players without the cheat → `Imposter: none` (chance 0).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | cells, `ImposterChanceFor`, load checks |
| `Source/CL9/Data/CL9Types.h` | `ECL9ImposterOutcome`, `FCL9ImposterResult` |
| `Source/CL9/GameMode/CL9PlayerState.{h,cpp}` | `bImposter` (owner only) |
| `Source/CL9/GameMode/CL9GameState.{h,cpp}` | replicated result |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | assign, judge, rewards, cheat entry |
| `Source/CL9/GameMode/CL9CheatManager.{h,cpp}` | `cl9.imposter` |
| `Source/CL9/UI/CL9HUDWidget.{h,cpp}`, `Source/CL9/UI/CL9DawnWidget.{h,cpp}` | notice, dawn line |
| `Source/CL9/GameMode/CL9ImposterTests.cpp` (new) | automation test |
| `Content/Strings/CL9_Strings.csv` (+ `CL9.csv`) | 4 new ko rows |

## Must-Preserve
- `EndNight` order: left behind → wipe penalty → wallet → lobby/dawn; the run-over branch.
- 7f: one wallet grant per player per round (`LastGrant` = this round).
- Team kill (7d) behaviour and `KilledByPlayer` semantics; the place notice for the kitchen.
- Other sessions' uncommitted changes in the touched files.

## Fairness Constraints
- The imposter's identity never replicates to other machines before the dawn reveal (owner-only property; the round-start log line is host-local).
- Judgement is host-only, from host state.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (networked secret role, round-end ordering). A cheaper model is fine for strings and docs.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Client RPCs fired from editor Python stay on the host — end rounds with the C++ night timer when the client's dawn screen or wallet must be checked.

## Results (2026-10-12)
- Build: `CL9Editor` OK (92 s). Smart App Control blocked the first DLL (CodeIntegrity 3077/3118, `GetLastError=4551`); touching one source file to relink produced a DLL that loaded.
- `CL9.Run.Imposter` and `CL9.Run.WipePenalty` pass.
- Listen 2 players, `Lvl_Henhouse`, round clock 40 s in memory only (restored to 245 and saved):
  - Roll without the cheat: `Imposter: none this round (chance 0% for 1 player(s))` — see the first-round note below.
  - Notice: host imposter → `PlaceText` visible in the host world only (client world collapsed); client imposter → visible in the client world only. One `HUD: imposter notice` line each time.
  - **Won**: host imposter, client killed by a monster (`cl9.damage 99 1`), host in the cargo box, clock end → `Imposter: '…' won`, host `+30 … imposter won: 30 instead of 0` (wallet 20 → 50); both dawn screens `임포스터: <host> — 성공`.
  - **Direct strike**: host imposter knocks the client to 1 HP by cheat, then a frying-pan swing (`cl9.swingtaps 1`) kills them → `failed — struck '<client>' directly`; both dawn screens `— 실패`.
  - **Killed**: client imposter, host swings them to death → `was killed by '<host>'`, host `+20 for killing the imposter` (50 → 70); both dawn screens `<client> — <host>에게 처치됨`.
  - **Left behind**: host imposter, client alive outside the truck at clock end → `failed — someone else is still alive (left behind does not count)`, client left behind afterwards; the failed imposter who came home got the normal survival reward (`1000 coins × 0.01 -> 10`).
- `cl9.imposter` console registration (`CL9Cheats.cpp`) was added after this build — the tests drove the same `HandleCommand` through `Server_CL9Cheat` on the host. It compiles with the next build (7h).
- **First round of a stage**: clients join a listen host after the stage's first round has already started (`StartPlay` → `StartNight`, join = connect to the running stage), so that roll sees only the host and never assigns an imposter. Rounds started from the lobby lever include everyone. Changing when the first round rolls is left for later — noted in systems/session.md.
- Another editor session used this editor over MCP during the test (an FBX import into `/Game/Monsters/ChickenA/SessionCurrent`, then `StopPIE` + `Lvl_Flat`) — after the four rounds had finished; its unsaved assets were left untouched.
