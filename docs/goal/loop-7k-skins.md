# goal: loop-7k-skins

**Status**: done — structure with stand-ins (2026-10-12, listen 2-player PIE verified, uncommitted). **2026-10-16 정정**: 코드·`SkinTable` 은 커밋 a5af9f2(2026-10-13)에 들어갔다. Real skin models pending. Build order ⑦ step **7k** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decision 1 (out-game currency buys cosmetics only), open questions 2 and 3 answered 2026-10-12.

## One-line Goal
Players spend their 이더 on cosmetic skins at a wardrobe in the company (shop + my page in one screen); ownership and the worn skin live in each player's own wallet file, and the worn skin is sent to the host and replicated so everyone sees it — built now with colour-swapped farmer stand-ins so real skin models only add table rows later.

## Background / Motivation
- Decision 1 (2026-10-11): out-game currency is personal, survives runs, buys cosmetics only (no balance impact).
- User answers 2026-10-12: **structure now with stand-in skins** ("skin table, shop, my page and applying, with two or three colour-swapped farmers; whole-body model replacement; when models arrive only rows are added"); **shop and my page live in the company** (open question 2).
- Recon (2026-10-12):
  - Defs load from table assets (`UCL9DefTable` rows, `CL9DefList`) through `UCL9DefRegistry`.
  - The farmer body (`SK_Farmer_A_Body`, slots Cloth/Skin/Detail) and arms (Cloth/Detail) use `M_CL9_Farmer_Cloth`, which already exposes a `TintColor` vector parameter.
  - Nothing applies a per-player tint today.
  - The owner never sees their own third-person body, except in the death cam.
  - The wallet (7f) saves only `Ether`.
  - Self-built C++ screens exist (title, game over).

## Design Decisions
- **`UCL9SkinDef`** (new def, `SkinTable`):
  - `BodyMesh` — whole-body skeletal mesh; empty = the character row's body.
  - `TintSlot` / `TintParameter` / `Tint` — stand-in cloth colour; alpha 0 = none.
  - `Price` — 이더.
  - `bDefault` — owned by everyone from the start.
  - Stand-in rows: `farmer_default` (default), `farmer_red` 20, `farmer_blue` 30, `farmer_gold` 60. Prices are drafts.
- **Ownership is local**: `UCL9WalletSave` v2 adds `OwnedSkins` and `EquippedSkin`. The wallet subsystem gains `OwnsSkin`, `BuySkin` (spend + own + save) and `EquipSkin`. The host cannot verify a client's ownership — accepted, cosmetics only (as for the 7f wallet).
- **Applying**:
  - `ACL9Character::SkinAlias` is replicated with OnRep. The host sets it through `ACL9PlayerController::Server_SetSkin`, which only checks that the row exists.
  - Each machine re-applies the character row's body (or the skin's), clears material overrides, then tints the `TintSlot` with a dynamic material instance on body and arms.
  - The local player sends their worn skin when their pawn is possessed (`OnPossess` on the host, `AcknowledgePossession` on clients) and again when they change it.
- **Screen** `UCL9SkinShopWidget` (UI row `SkinShop`, self-built tree like the title):
  - Shows the 이더 balance and one button per skin: "name — N 이더" to buy, "보유" to wear, "입는 중".
  - Clicking buys if needed, then wears.
  - Opened by a new company fixture `ACL9SkinCloset` (prompt `[E] 옷장`, anyone, own screen).
- **Cheat** `cl9.skin [alias]` (local): lists skins (owned / worn / price), or buys if needed and wears.

## Out-of-scope
- Real skin models (rows later), a 3D preview of yourself (the owner never sees their own body), a title-screen entry point, Steam inventory or payments.

## Acceptance Criteria
- [x] `CL9Editor` builds; `SkinTable` loads 4 rows (no broken rows).
- [x] `cl9.skin farmer_red` with enough 이더 → wallet −20, owned + worn, saved; the host's character shows the red tint on the client machine.
- [x] A client whose wallet file already wears a skin → on PIE start the host receives it through possession (real RPC path) and every machine shows it.
- [x] Company: the wardrobe opens the screen (balance + 4 rows).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9SkinDef.{h,cpp}` (new), `Source/CL9/Data/CL9DefRegistry.{h,cpp}` | def + registry |
| `Source/CL9/GameMode/CL9Wallet.{h,cpp}` | ownership / worn skin |
| `Source/CL9/Character/CL9Character.{h,cpp}` | replicated skin, apply |
| `Source/CL9/GameMode/CL9PlayerController.{h,cpp}` | `Server_SetSkin`, send on possession |
| `Source/CL9/UI/CL9SkinShopWidget.{h,cpp}` (new) | wardrobe screen |
| `Source/CL9/Items/CL9SkinCloset.{h,cpp}` (new) | company fixture |
| `Source/CL9/GameMode/CL9Cheats.cpp`, `CL9CheatManager.{h,cpp}` | `cl9.skin` |
| `Content/Data/SkinTable.uasset` (new), `Content/UI/UITable.uasset`, `Content/Maps/Lvl_Company.umap`, strings | data |

## Must-Preserve
- Character row visuals and animation; wallet balance semantics (7f); other sessions' uncommitted changes.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above. A cheaper model is fine for strings/data.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.

## Results (2026-10-12)
- Builds: 4 — UHT rejected `HandleCloseClicked` (the parent `UCL9Widget` already declares it); two shadowed names; then `cl9.skin` moved from the cheat manager to a local console command next to `cl9.wallet` (client controllers have no cheat manager, so the client-side test did nothing).
- Data: `SkinTable` (4 rows, names from String Table keys `Skin.<alias>.Name`), `CL9DefList` re-saved (now lists `SkinTable`), UI row `SkinShop` → `/Script/CL9.CL9SkinShopWidget` (added through MCP keeping the existing PascalCase keys), `ACL9SkinCloset` placed in `Lvl_Company`; fixture names moved to keys `Fixture.destination_board.Name` / `Fixture.skin_closet.Name`. **The farmer's material slot is named `M_CL9_Farmer_Cloth`, not `Cloth`** — set per row (`TintSlot`).
- Listen 2 players (PIE):
  - Host: `cl9.skin` lists 4 skins; `cl9.skin farmer_red` → `bought skin 'farmer_red' for 20 ether — 120 left`; later `farmer_gold` → host body gold (0.85, 0.62, 0.12) in **both** worlds (dynamic material on the cloth slot).
  - Client: `cl9.skin farmer_blue` on the client world → `CL9Wallet_PIE1 bought … 30 left`, saved; PIE restarted → `Wallet: 'CL9Wallet_PIE1' loaded — 30 ether, 1 skin(s) owned, wearing 'farmer_blue'` → on possession `Skin: 'BP_CL9PlayerController_C_1' wears 'farmer_blue'` (real client → host RPC) → client body blue (0.08, 0.2, 0.75) in both worlds.
  - Company: host presses the wardrobe → `Skin closet: … opens the wardrobe` → screen with `옷장 — 스킨 상점·마이페이지`, `보유 이더 60`, rows `기본 작업복 — 보유`, `빨간 작업복 — 보유`, `파란 작업복 — 30 이더`, `금빛 작업복 — 입는 중` (disabled), `닫기` (screenshot).
- Test wallets now hold: host `CL9Wallet` 60 이더 (red, gold owned, gold worn), client `CL9Wallet_PIE1` 30 (blue).
