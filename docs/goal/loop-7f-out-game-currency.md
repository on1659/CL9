# goal: loop-7f-out-game-currency

**Status**: done (2026-10-12 — PIE 1 player + listen 2 players verified, uncommitted). Build order ⑦ step **7f** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decision 1 (two currency layers: run money owned by the host's run, and a personal out-game currency that outlives runs, buys cosmetics only); open question 1 (where it is earned) answered here.

## One-line Goal
Each player gets a personal out-game currency, named **이더** on screen, stored on their own machine so it survives any round or run, earned by surviving a round in proportion to that round's income, and shown on the dawn screen and the title screen; the in-game run money is renamed **코인** on screen.

## Background / Motivation
- Decision 1 (2026-10-11): out-game currency is personal (Steam account), survives runs, buys cosmetics only, no balance impact. Its reason to exist is the imposter (7g).
- User answers 2026-10-12 (this run):
  - **Earning (open question 1)**: surviving players at round end earn an amount **proportional to the round's income** ("그래야 더 열심히 한다"); imposter success / imposter kill rewards (7g) must be **larger** than this.
  - **Names**: the out-game currency is called **이더**; the in-game currency (today shown as "이더") is renamed **코인** on screen. Code names (`SharedMoney`, `PeriodIncome`, logs saying "ether") stay.
  - **Where shown**: dawn screen + title screen.
- Recon: the run save and the best-day record (`CL9Records`) are host-only files; a client has no save. The out-game currency must live on each player's own machine, so the host only *tells* each player what they earned.

## In-scope
1. **Local wallet** — new `UCL9WalletSave` (`USaveGame`, version + `Ether` int32) and `UCL9WalletSubsystem` (`UGameInstanceSubsystem`, one per machine/PIE instance): loads at init, `ApplyGrant(Amount)` adds (≥ 0), saves when > 0, remembers this dawn's grant (`LastGrant`, runtime only), broadcasts a change. Slot `CL9Wallet`; in PIE, instances ≥ 1 get `CL9Wallet_PIE<n>` so a host and a client on one machine do not share a file.
2. **Grant path** — `ACL9PlayerController::Client_WalletGrant(int32)` (Client, Reliable) → the owning machine's subsystem. The host sends one grant to **every** player at each round end (0 for the dead / left behind), so `LastGrant` always means "this round". Same controller channel as `Client_OpenScreen(Dawn)` and sent before it → the dawn screen already has the number.
3. **Earning** — new `CL9Rules.WalletIncomeRate` (draft **0.01** — 1 이더 per 100 코인 earned this round; load check finite, 0..1). Round income = `PeriodIncome` now − `PeriodIncome` at round start (snapshot in `StartNight`). Granted in `EndNight` after the left-behind check and the wipe penalty, to players still alive: floor(income × rate) (basis-point integer maths like `WipePenaltyFor`). Log one line per player.
4. **Display** — dawn screen line: `이더 +N (보유 M)` when N > 0, else `보유 이더 M`; title screen line `보유 이더 M` under the game name. Both read the local subsystem.
5. **Rename on screen** — ko strings that show run money as "이더" (11 rows: HUD money/order/counter lines, scan label value, shop money/price, order card, dawn earned/settle, `Dawn.WipePenalty`) → "코인". ko column only (translations later, project rule).
6. **Cheat** `cl9.wallet [delta]` (local, any machine) — prints the wallet (slot, balance, last grant), applies a delta for tests (balance never below 0).
7. **Docs**: plan/economy.md (two names), systems/save.md (wallet file), systems/ui.md (dawn + title lines), plan/data/rules.md (cell), cheats.md, decisions.md, loop-rework open question 1, build-order 7f → done, goal README.

## Out-of-scope
- Imposter / kill rewards (7g), skin shop / my page / skins (7k), Steam cloud or Steam inventory, anti-tamper of the local file (cosmetic-only currency).
- Renaming code identifiers or English/other-language strings.
- HUD display of the wallet.

## Acceptance Criteria
- [x] `CL9Editor` builds.
- [x] PIE `Lvl_Henhouse` 1 player: earn coins in a round (`cl9.money`), end the round alive inside the truck → log grant = floor(round income × 0.01); dawn line `이더 +N (보유 M)`; wallet file written.
- [x] Dead / left-behind player at round end → grant 0, dawn line `보유 이더 M`.
- [x] Persistence: stop PIE and start again → `cl9.wallet` shows the same balance; a lost run (settlement fail / wipe) does not touch it.
- [x] Title (`Lvl_Start`) shows `보유 이더 M`.
- [x] Listen 2-player: the client's own wallet (`CL9Wallet_PIE1`) receives its grant when the round ends by the C++ night timer (not a Python-driven cheat — Client RPCs from editor Python don't leave the host).
- [x] On-screen run money now reads 코인 (HUD money line).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/GameMode/CL9Wallet.{h,cpp}` (new) | save object, subsystem, `cl9.wallet` |
| `Source/CL9/GameMode/CL9PlayerController.{h,cpp}` | `Client_WalletGrant` |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | round-income snapshot, `GrantRoundWallet` |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | `WalletIncomeRate`, `WalletIncomeFor` |
| `Source/CL9/UI/CL9DawnWidget.{h,cpp}`, `Source/CL9/UI/CL9TitleWidget.{h,cpp}` | display |
| `Content/Strings/CL9_Strings.csv` (+ generated `CL9.csv`) | renames + new keys |

## Must-Preserve
- Run save / records unchanged; settlement, wipe penalty, left-behind order in `EndNight`.
- Dawn screen open order (screen before settlement) and its existing lines.
- Other sessions' uncommitted changes in the touched files.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (networked grant path, persistence). A cheaper model is fine for the string renames and docs.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Editor / build / PIE via the lock CLI (`docs/workflow/sessions.md` "잠금"), lock commands issued alone.

## Results (2026-10-12)
- Build: `CL9Editor` OK (112 s).
- 1 player, `Lvl_Henhouse`: 1000 coins earned in the round, home in the truck → `Wallet grant: '…' came home — round income 1000 coins × 0.0100 -> 10 ether.`; dawn `이번 기간 1,000 코인` / `이더 +10 (보유 10)`; `CL9Wallet.sav` written.
- Same player left outside the truck next round (1 player → this was also a wipe): `did not survive — round income 500 coins × 0.0100 -> 0 ether.`, `Wallet: 'CL9Wallet' +0 ether this round — 10 total.`; dawn `전멸 — 코인 313 차감` / `보유 이더 10` — the wipe penalty took run money only, the wallet kept 10.
- Persistence: PIE stopped and restarted → `Wallet: 'CL9Wallet' loaded — 10 ether.`; title (`Lvl_Start`) shows `보유 이더 10`; HUD money line shows `545 코인`.
- Listen 2 players, round ended by the C++ night timer (`CL9Rules.NightLength` 35 s in memory only, restored to 245 and saved): both `came home — round income 1000 coins × 0.0100 -> 10 ether.`; client `Wallet: 'CL9Wallet_PIE1' +10 …` (0 → 10), host 10 → 20; both dawn screens show `이더 +10 (보유 …)`.
- Not tested directly: a settlement-fail game over. The wallet has no path from the run save or `EndRun`, so a lost run cannot touch it.
- Left on this machine by the tests: `Saved/SaveGames/CL9Wallet.sav` (20) and `CL9Wallet_PIE1.sav` (10). `cl9.wallet -20` / `-10` or deleting the files resets them.
- English strings still say "ether" for the run money (ko column only, project rule).
