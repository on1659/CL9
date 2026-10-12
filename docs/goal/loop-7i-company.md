# goal: loop-7i-company

**Status**: done (2026-10-12 — listen 2-player PIE round trip verified, uncommitted). Build order ⑦ step **7i** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decision 2 (every 3 rounds the company: settle, buy), open questions 2 and 10 answered 2026-10-12.

## One-line Goal
On a settlement day the depart lever takes everyone (truck and run) to a walkable company level; there the period is settled (a miss costs coins instead of ending the run), equipment is bought, the next destination is picked on a board, and the lever takes everyone to that site where the run continues.

## Background / Motivation
- Decision 2 (2026-10-11): "3번의 스테이지를 거치면 회사로 가서 정산하고, 아이템 사고, 다시 반복".
- User answers 2026-10-12: the company is a **walkable level** (open question 2); a missed quota is **a penalty and the run continues** instead of game over (open question 10); **buying moves to the company** — the truck keeps only the non-order sell counter; the skin shop and my page live in the company too (7k).
- Recon (2026-10-12):
  - No level travel exists. `decisions.md` once rejected a lobby level + ServerTravel because of carrying state. The user's walkable company reverses that.
  - The run save already carries day, money, period income, accepted orders and owned equipment (`SaveRun`/`LoadRun`, slot `CL9Save_1`, equipment respawned at the truck pad). Nothing else crosses a level change except the game instance.
  - Non-seamless `ServerTravel` works in PIE; seamless is off in PIE unless `net.AllowPIESeamlessTravel`.
  - Settlement runs in `EnterLobby` on `Day % SettlementIntervalDays == 0` (days 3, 6, 9… — the first period is rounds 1–2, later ones three rounds). A miss calls `StartNewRun` (back to day 1). Only `Lvl_Flat` has a truck panel (shop); stage levels have none.

## Design Decisions
- **Company is its own level** (`Lvl_Company`), named by a data cell `CL9Rules.CompanyLevel` (soft world; empty = no company, settlement stays in the lobby). The game mode recognises it by package, like `ResolveSiteAlias` does for sites — no extra game-mode class.
- **Travel = non-seamless `ServerTravel`** (relative, `?listen` outside Shipping), clients follow. The run crosses through the existing save (`SaveRun(1)` before, `LoadRun(1)` on arrival, no order re-offer) and a flag on `UCL9GameInstance` (`SetRunCarry` / `ConsumeRunCarry`, plus the site we came from as the default destination).
- **When**: in the lobby of a settlement day, `RequestDepart` (lever / `cl9.depart`) travels to the company instead of starting the round. The dawn screen says so (`Dawn.CompanyDue`). In the company, `RequestDepart` travels to the chosen site; the round there starts as usual (K1) with the carried run.
- **Settlement moves to the company** when a company level is set: arrival → `Settle()` → every player's dawn screen opens on login (the widget already reads the replicated last settlement). A miss deducts `CL9Rules.QuotaMissPenaltyRate` (draft 0.3, same maths as the wipe penalty) of the run's coins, resets the period and continues; `Dawn.Failed` / `Dawn.CloseFailed` ko text changes accordingly and a `Dawn.QuotaPenalty` line shows the amount. `StartNewRun` stays for `cl9.settle force` only.
- **Buying**: the company level holds a truck panel (`BP_CL9TruckPanel`, existing shop screen). Stage levels have no panel, so nothing to remove; `Lvl_Flat` (test level) keeps its panel.
- **Destination board** — new `ACL9DestinationBoard` (fixture + interactable): host presses to cycle through `GetHostableSites()`; replicated choice; prompt `[{0}] 목적지: 이름` (others see `목적지: 이름 — 호스트가 고른다`). Default = the site the truck came from.
- **Level content (graybox)**: floor, walls, ceiling, lights, 4 player starts, a truck pad (truck + lever spawn as on stages), the panel and the board. Art later.

## Out-of-scope
- Truck driving cinematic, in-truck settlement, reviving at a special place (round-start-truck-intro "next spec" items) — travel is a plain level change.
- Changing the first period's length (2 rounds, see recon) — noted for the user.
- Skin shop / my page (7k); voice/EOS behaviour across travel (no second machine).

## Acceptance Criteria
- [x] `CL9Editor` builds; `Lvl_Company` exists and `CL9Rules.CompanyLevel` points at it.
- [x] Listen 2 players: lobby on a settlement day → `cl9.depart` → both machines land in `Lvl_Company`; log shows the carried day/money/equipment; both see the settlement dawn screen.
- [x] Miss: period income below target → coins − 30%, dawn `미달` + penalty line, run continues (day unchanged, no reset).
- [x] Buy in the company (`cl9.buy` / panel) → equipment spawns at the truck; carried to the next site.
- [x] Board cycles the destination (host); `cl9.depart` in the company → both machines land on that site, the round starts with the carried day/money/equipment.
- [x] Non-settlement days keep the old lobby → round flow.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | `CompanyLevel`, `QuotaMissPenaltyRate` |
| `Source/CL9/GameMode/CL9GameInstance.{h,cpp}` | run carry flag |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | company detection, travel, resume, settlement penalty |
| `Source/CL9/GameMode/CL9GameState.{h,cpp}` | quota penalty replication |
| `Source/CL9/Items/CL9DestinationBoard.{h,cpp}` (new) | destination board |
| `Source/CL9/UI/CL9DawnWidget.{h,cpp}` | company-due and penalty lines |
| `Content/Maps/Lvl_Company.umap` (new), `Content/Data/CL9Rules.uasset` | level, cell |
| `Content/Strings/CL9_Strings.csv` (+ `CL9.csv`) | strings |

## Must-Preserve
- Non-settlement lobby → round flow, K1 immediate round start on stage levels, 7a–7h behaviour.
- Other sessions' uncommitted changes in the touched files.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (level travel and state carry). A cheaper model is fine for strings and docs.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.

## Results (2026-10-12)
- Builds: 3 (company code; then travel deferred one tick + restore before the map build; then the HUD-derived company screen). `Lvl_Company` made by an editor script (engine cubes, movable lights, truck pad, `BP_CL9TruckPanel`, `ACL9DestinationBoard`); `CL9Rules.CompanyLevel` set and saved.
- Listen 2 players (PIE, `Lvl_Henhouse`):
  - Settlement-day lobby (`cl9.day 3`, 300 coins) → `cl9.depart` → `Travel (settlement day -> company)` → host and client load `Lvl_Company` (`Join succeeded`); `Loaded 'CL9Save_1' — day 3, 300 ether, 2 order(s)`; `Settlement failed on day 3: earned 300 < 800 — 90 coins taken, the run goes on. Money 300 -> 210.`
  - Second run through real rounds (1000 coins): both company screens `3일차 정산 / 이번 기간 1,000 / 목표 800 코인 / 통과 — 다음 기간이 시작된다` (`HUD: company — settlement screen 'Dawn' opened` on each machine).
  - `cl9.buy 1` in the company → frying pan, 210 → 10; back to `Lvl_Henhouse` → `Load: 1 equipment respawned by the truck`, `Night 3 at 'henhouse'`, site map day 3 with the same hash on both machines.
  - Destination board pressed by the host → `'farm' chosen` → `cl9.depart` → both on `Lvl_Farm`, `Night 3 at 'farm' started`, run carried (day 3, 205 coins).
- Found and fixed while testing: (1) `ServerTravel` sends `ClientTravel` at once — from an editor-Python call that RPC never left the host, and anyway the world should not be torn down inside the lever's call stack → travel now leaves on the next tick; (2) the resumed run was restored after the site map was rolled (map sized for day 1) → restored first; (3) the company screen opened from `PostLogin` was dropped (UI manager not up yet) → the HUD derives it; (4) the board/panel sat in the middle of a 120 cm desk, beyond `PickRadius` 120 → moved to the desk's front edge.
- Not verified here: the dawn widget re-drawing its lines on the lobby phase change (added after the last test build — compiled with the 7j build), voice across travel, a real truck-travel cinematic.
- Design consequence for the user: with the quota miss no longer ending the run (and wipes only ending the round, 7a), nothing ends a run now.
