# goal: use-action-left-click

## One-line Goal
Left mouse button becomes "use the held item" (row-driven: toggle light / swing-hit / eat), throw moves to G, and the cart is pushed in front of the player instead of towed behind.

## Background / Motivation
User (2026-09-12): "우리 좌버튼으로 던지는거 없애자 이거좀 에바임", "좌버튼누르면 아이템에 따라 다르게 동작하게하자", "타격시 hp피해량은 int(0이면 없는거), 경직타입을 넣을 수 있게 해서 enum으로 세팅하게 하자", cart: "잡으면 밀고 다니자". Throwing on LMB felt wrong; LMB should be the item's primary action like most FPS. The empty-hand shove that stood in for the frying pan (`Server_Shove`, "④ 장비 전까지 대역") is now realised by the frying pan itself.

## In-scope
- Input: new `IA_Use` on LMB (Started/Completed — tap vs hold, 0.4 s local timer). `IA_Throw` rebound to **G** (Started/Completed, charging unchanged). `IA_Eat` removed (asset, `EatAction` property, `EatInput`). `IA_Flashlight` (F) now toggles the **worn headlamp only**.
- `ACL9Character::UseInput → Server_Use → HostUse()` dispatch on the held item row:
  1. **Hold** (≥0.4 s) → eat if `ItemType == Dish` (existing `HostEat`; plan rule "only Dish is edible, no separate flag"), else nothing. User 2026-09-12: "길게누르면 먹기, 짧게누르면 휘두르기" — resolves cooked meat (Dish + Swing).
  2. **Tap** → row `UseAction`: `ToggleLight` → toggle the held `ACL9FlashItem` (existing light toggle path minus the headlamp priority); `Swing` → `HostSwing()`; `None` → nothing.
  3. Empty hand → nothing (old empty-hand shove removed; the frying pan is the real shover now).
- Item row columns (`UCL9ItemDef`, category "Use"): `ECL9UseAction UseAction = None` (`None / ToggleLight / Swing`), `int32 HitDamage = 0` (0 = no HP damage), `ECL9HitStun HitStun = None` (`None / Hit` = monster `HitStunMs` / `Push` = monster `PushStun` + knockback + revenge as `OnShove`).
- `HostSwing`: reuse `Server_Shove` targeting (sphere reach 200 cm, forward cone dot 0.3, recovery 0.6 s — constants unchanged) → `ACL9Monster::OnStrike(HitDamage, HitStun, Source)`: apply HP damage first (may kill → death path, no stun); if still alive apply the stun kind. Non-lethal damage with `HitStun::Hit` stuns once (no double stun from `OnDamage`).
- DA_ItemTable values: flashlight `ToggleLight`; frying_pan `Swing`, HitDamage 0, HitStun Push (today's shove feel); meat raw/cooked/burnt `Swing`, HitDamage 0, HitStun Hit (cooked meat is a Dish: tap swings, hold eats); egg×5 / pot / standing_lamp / cart / headlamp `None`. Dish rows (egg+cooked, meat+cooked if Dish) eat via LMB by type.
- Cart: `ECL9ItemHoldState::Pulled` → `Pushed`. While pushed the cart sits **in front** of the pusher at hitch distance (capsule radius + cart radius + 20 cm), yaw = pusher yaw, moved each host tick with a sweep; if the sweep is blocked the pusher is moved back by the remaining delta so walls stop the pair. Grab/release/load/unload/CartMult/prompts unchanged.
- Docs: input.md (IA table + key layout), inventory.md 조작 table, plan/data/item/common.md (3 columns), plan/data/item/cart.md (push), plan/data/item/flashlight.md + headlamp.md + systems/light.md (F → headlamp only, LMB toggles flashlight), systems/monster.md (player response = swing by item row), decisions.md.

## Out-of-scope
- Swing / eat / toggle animations (no clips yet — HoldPose layer stays static).
- Thrown-item damage to monsters, noise from swinging, swing cooldown/reach as table columns (constants stay until asked).
- HUD hint text changes beyond what existing prompts already show.
- Player-vs-player hits.

## Acceptance Criteria
- [x] CL9Editor builds (`CL9.Map.*` not rerun — no map code touched).
- [x] PIE (Lvl_Flat): LMB with flashlight toggles it on/off; F does nothing without a headlamp (headlamp-worn case not rerun — code path unchanged).
- [x] PIE: frying pan tap → chicken STUN + knockback; meat tap → STUN, no knockback; egg tap → nothing; cooked meat tap → swing, hold ≥0.4 s → eaten. (Pot/lamp/empty hand share the `None`/null path with egg.)
- [x] PIE: G throws the held item; LMB never throws.
- [x] PIE: grab cart with E → cart holds 119 cm ahead through walking and a 60° turn, E releases it in place. Wall stop observed only indirectly (long run slid along the test walls); CartMult path unchanged.
- [x] Rows validate (no Broken rows) and docs listed above are updated.

## Related Files / Modules
| File | Role |
|------|------|
| Source/CL9/Data/CL9ItemDef.h/.cpp | new enums + 3 columns, Validate |
| Source/CL9/Data/CL9Types.h | `ECL9ItemType` (Dish) reference |
| Source/CL9/Character/CL9Character.h/.cpp | `UseAction`, `UseInput/Server_Use/HostUse/HostSwing`, remove Eat input, flashlight input → headlamp only, throw binding unchanged |
| Source/CL9/Items/CL9Item.h/.cpp | `ECL9ItemHoldState::Pulled` → `Pushed` |
| Source/CL9/Items/CL9Cart.h/.cpp | push-in-front tick, naming |
| Source/CL9/Items/CL9FlashItem.h/.cpp | toggle entry point used by HostUse |
| Source/CL9/Monsters/CL9Monster.h/.cpp | `OnStrike(Damage, Stun, Source)` composed from `OnDamage`/`OnShove` |
| Content/Input/IMC_Gameplay, Actions/IA_Use (new), IA_Throw (G), IA_Eat (delete) | key mapping |
| Content/Characters/… BP_CL9Character | `UseAction` property set, `EatAction` gone |
| Content/Data/DA_ItemTable | row values |
| docs/systems/input.md, inventory.md, light.md, monster.md, item.md; docs/plan/data/item/common.md, cart.md, flashlight.md, headlamp.md; docs/decisions.md | documentation |

## Must-Preserve
- Input is local, judgement is a Server RPC (conventions § 2); throw charge timing measured on host as today.
- `TakeDamage`/`UDamageType` stay banned — HP changes are plain function calls on the host (monster.md).
- Monster HP/stun/knockback numbers keep coming from the monster row (`HitStunMs`, `PushStun`, `PushKnockback`); item rows only choose damage amount and stun kind.
- Hands are exclusive: holding an item and pushing the cart cannot coexist; R (swap) locked while pushing.
- Headlamp behaviour on F, blackout rules (`light.md`), `bLightLostInPocket`.
- Existing `HostEat` semantics (Dish only, EatBuffs).
- Screen strings go through the String Table (`docs/systems/localization.md`) — reuse existing prompt keys; no new user-facing strings expected.

## Execution Notes
- Recommended model: the strongest current Claude model for the character/monster dispatch and cart push movement (multiplayer authority + collision sweep judgement); a cheaper model is fine for the enum/column boilerplate and doc edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Full editor build required (header changes) — coordinate with other sessions before killing the editor.

## Existing Integration Contract
- `ACL9Character::Server_Shove` targeting code becomes `HostSwing`; cheat `cl9.damage` keeps calling `OnDamage`.
- `ACL9Cart::Interact` flow (E: load if holding / toggle grab otherwise) unchanged; `Puller` field semantics = pusher.
- `ACL9CharacterMovement` reads `GetPullingCart()` for `CartMult` — accessor name kept.
