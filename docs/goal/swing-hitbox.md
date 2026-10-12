# goal: swing-hitbox

**상태**: 구현 완료 2026-09-15(커밋 `d4fe2ec` — 애니메이션을 따르는 히트박스·손 자리 놓기; decisions 09-15 "휘두르기 판정은 애니메이션을 따르는 히트박스"·"놓기(G)는 손 자리 낙하"; PIE `docs/testing/pie-scenarios.md` C4). 체크박스는 실행 당시 갱신되지 않았다. 2026-09-20 전역 배율 `CL9Rules.SwingHitScale` 추가(실행 swing-hit-scale, decisions 09-20).

## One-line Goal
Make the swung item hit monsters through a real hitbox that follows the swing
animation (every monster it touches, once per swing, with damage values that can
kill), and make G drop the held item exactly where the hand is instead of at the
aim point.

## Background / Motivation
`HostSwing` currently picks the nearest monster inside a 200 cm sphere / forward
cone at the instant the left button is released and calls `OnStrike` — the swing
animation is cosmetic and nothing about "did the pan actually touch the chicken"
is checked. Every row has `HitDamage 0` (pan `Push`, meat `Hit`), so no item can
kill a monster. The user (2026-09-14) wants a hitbox on the swung item so that
monsters can be caught/killed by swinging.

G drop became "aim-point drop" on 2026-09-14 (sweep up to `DropReach` 250 cm,
cart special case). The user watched it and wants the item to fall from where it
is held instead ("그자리에서") — no aim point, no cart special case (cargo goes
in by throwing only).

## In-scope
### A. Drop in place (G)
- `Server_Drop` releases the held item at its **current world transform** (the
  actor root is on the body-mesh hand socket, so this is the hand position on
  every machine) with zero velocity — no sweep, no `DropReach`, no cart
  `GetCargoDropLocation` special case.
- Remove what this orphans: `CL9Rules.DropReach` (header + `rules.md` row +
  the value in `CL9Rules.uasset`, applied by the lead), `ACL9Cart::
  GetCargoDropLocation`, the `DropClearance` constant if unused elsewhere.
- Docs: `docs/systems/inventory.md` 놓기 row, `docs/systems/item.md` physics
  section if it mentions aim-point drop, `docs/plan/data/rules.md`,
  `docs/plan/data/item/cart.md` cargo-loading sentence ("G로 짐칸에" → throw
  only), `docs/testing/pie-scenarios.md` B1/B4, `docs/decisions.md` (record the
  reversal: aim-point drop 2026-09-14 → hand-position drop 2026-09-14, and that
  cart loading is throw-only).

### B. Swing hitbox
- **Hit window instead of instant pick.** On the host, from `SwingStartServerTime`
  while `SwingElapsed ∈ [SwingHitStart, SwingHitEnd)` (two new `CL9Rules` float
  columns, seconds after release; the ABP evaluates the clip at
  `SwingElapsed + 0.12` for `[0, 0.347)`, so the window lies inside `[0, 0.347)`),
  every host tick sweeps the item's hitbox from its previous-tick world position
  to its current one and calls `OnStrike(HitDamage, HitStun, this)` on every
  `ACL9Monster` it touches — **once per monster per swing** (a per-swing hit set,
  cleared when the swing starts). At the first tick of a swing the "previous"
  position is the current one (no sweep from a stale pose).
- **Hitbox shape = the item's mesh bounding sphere at the body hand.** Center and
  radius come from the item's static mesh bounds (`ItemDef->Mesh` asset bounds ×
  mesh scale) placed at the transform the mesh has when attached to the body
  hand (root `SphereComponent` world transform × grip relative transform). Do
  **not** read `MeshComponent`'s world transform for this: on the local owner the
  visible mesh is re-attached to the first-person arms (camera-local), which is
  not the authoritative hand. No new shape columns.
- **Bones must follow the swing on the host.** The body mesh is `OwnerNoSee` (own
  pawn) and may be off-screen (remote pawns), and `ACharacter` defaults the mesh
  to `AlwaysTickPose` (bones refresh only when rendered). On the authority set
  the body mesh's `VisibilityBasedAnimTickOption` to
  `AlwaysTickPoseAndRefreshBones` so the hand socket (and thus the hitbox)
  moves during the swing for every pawn, including the host's own.
- Remove the `SwingReach` / `SwingHalfAngleDot` constants and the overlap-and-pick
  loop. Keep `SwingRecovery` 0.6 s and everything about windup/cancel as is.
- Query: sweep by object type `WorldDynamic` (monster root sphere), ignore self,
  the held item and other pawns; only `ACL9Monster` counts (no player-vs-player).
- **Values (balance):** `HitDamage` for `frying_pan` and the three meat rows,
  `SwingHitStart` / `SwingHitEnd` (from the clip's contact frames —
  `docs/asset/character/farmer_a.md` 026/031, 30 fps 0.467 s clip starting at
  the raised pose t = 0.12), with rationale against chicken `HpMin 3 / HpMax 4`
  and the goose/raccoon/pig/sheep rows. The user left "how many hits to kill" to
  balance.
- Docs: `docs/plan/data/item/common.md` 사용 section (replace "사거리 200cm·정면
  부채꼴 … 상수" with the hitbox rule + current values), `docs/plan/data/rules.md`
  two new rows, `docs/systems/inventory.md` 사용 row, `docs/systems/monster.md`
  잡기=공격 bullet, `docs/systems/animation.md` line "판정(`HostSwing`)은 누르는
  즉시이고 애님은 따라올 뿐이다" (now the hit follows the clip),
  `docs/testing/pie-scenarios.md` C4 (+ new expectations: multi-hit, whiff on
  air/behind, kill count), `docs/decisions.md`.

## Out-of-scope
- Hitting other players (friendly fire) — monsters only.
  **Reversed 2026-10-12** (user decision 2026-10-11, [loop-rework.md](loop-rework.md) decision 4): the swing now also hits other players —
  [loop-7d-team-kill.md](loop-7d-team-kill.md). The monster sweep described here is unchanged; a second Pawn sweep was added beside it.
- Hit feedback UI (hit marker, sound), monster hurt VFX.
- Changing `SwingRecovery`, windup, cancel rules, throw, or the `Push`/`Hit`
  stun semantics on the monster side (`OnStrike` unchanged).
- New hitbox-shape columns (`HitboxRadius` etc.) — mesh bounds are enough; add a
  column only if a future item needs to differ from its mesh.
- Cart cargo loading by any means other than throwing (Shift+G).
- Cleaning up the asleep-item / damping work of run `item-drop-physics`.

## Acceptance Criteria
- [ ] G with an egg in hand: the egg falls from the hand position and settles
      within ~50 cm of the character's feet in front/right (not 2.5 m away, not
      inside the capsule, not launched). Looking at a cart and pressing G no
      longer puts the item in the cargo box; Shift+G still does.
- [ ] Swinging the pan at a chicken in front: the chicken is hit **during the
      swing** (visible ~0.1 s after release, not instantly) with `Push` +
      `HitDamage`; after the balance-set number of hits it dies and plays death
      (`cl9.hitmonster` behaviour unchanged).
- [ ] Two chickens standing side by side within the arc: both are pushed by one
      swing; a chicken hit once is not hit twice by the same swing.
- [ ] Swinging at empty air, or with the chicken behind the player, or 2 m away:
      nothing is hit (no more 200 cm cone).
- [ ] 2-player PIE: a client's swing hits the chicken the client is facing even
      when the host is looking elsewhere (bones refresh on the server).
- [ ] Meat (raw/cooked/burnt) swung at a chicken applies `Hit` stun with its
      balance-set damage.
- [ ] Build passes; `CL9Rules` has `SwingHitStart`/`SwingHitEnd`, no `DropReach`;
      `ItemTable` rows carry the balance values.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Character/CL9Character.cpp/.h` | `Server_Drop`, `HostSwing` → hit window tick, per-swing hit set, bone tick option |
| `Source/CL9/Items/CL9Item.cpp/.h` | hitbox helper (mesh bounds at body-hand transform) |
| `Source/CL9/Items/CL9Cart.cpp/.h` | remove `GetCargoDropLocation` |
| `Source/CL9/Data/CL9RulesDef.h` | remove `DropReach`, add `SwingHitStart`/`SwingHitEnd` |
| `Source/CL9/Monsters/CL9Monster.cpp/.h` | `OnStrike` unchanged — read only |
| `docs/plan/data/rules.md`, `docs/plan/data/item/common.md`, `docs/plan/data/item/cart.md` | column rows + values |
| `docs/systems/inventory.md`, `item.md`, `monster.md`, `animation.md` | mechanism text |
| `docs/testing/pie-scenarios.md` | B1, B4, C4 |
| `docs/decisions.md` | reversal + hitbox decision |
| `Content/Data/CL9Rules.uasset`, `Content/Data/ItemTable.uasset` | lead applies values |

## Must-Preserve
- `TakeDamage`/`UDamageType` stay banned; HP is a replicated int and hits are
  `OnStrike` calls (conventions § 6).
- Host-only judgment: sweeps and `OnStrike` run on the authority; clients only
  see replicated `SwingStartServerTime` and monster state.
- Windup → release → `HostSwing` flow, `SwingRecovery` 0.6 s, cancel on slot
  switch/drop/throw/eat/catch, ABP evaluation (`SwingElapsed + 0.12`, `[0, 0.347)`).
- No asset paths in C++; numbers live in table rows (`CL9Rules`, `ItemTable`).
- Throw (`Shift+G`) and cart cargo-by-throw untouched.
- `OnPlaced` teleport-physics release (2026-09-14) stays — it is what makes
  "drop where the hand is" land correctly.

## Execution Notes
- Recommended model: Claude Fable 5 for the gameplay slice (hit-window design,
  host bone-refresh trap, owner vs. body attachment) — judgment-heavy and easy
  to get subtly wrong across host/client. Sonnet acceptable for the docs slice
  (PIE scenario rewrite) and for the balance value sheet if the rationale is
  mechanical.
- This document cannot enforce the model — the executing session's `/model`
  setting decides. If the session model is below the recommendation, surface it
  to the user and confirm before proceeding.
- Trap: the held item's root sphere sits inside the capsule's reach (hand ≈ 25 cm
  front / 24 cm right of the pawn); releasing there overlaps the capsule and
  Chaos depenetration will nudge it out. That is acceptable ("그자리에서") unless
  it launches the item — if it does, the fix is a small outward nudge along
  capsule→hand, not a return to the sweep.
- Trap: PIE automation runs at 3–8 fps — the per-tick sweep (previous → current)
  is what keeps the hitbox from tunnelling through a chicken at low frame rate;
  do not replace it with a plain overlap.
- Executed through `/cl9 task` (roles gameplay / balance / docs, one build, one
  PIE pass on C4/B1/B4).

## Fairness Constraints
- Damage, stun and hit set are decided on the host only; a client cannot widen
  the hit window or hit more than once per swing.
- No RNG in the hit judgment.

## Existing Integration Contract
- `ACL9Monster::OnStrike(int32 Damage, ECL9HitStun Stun, AActor* Source)` —
  signature and semantics unchanged (damage first, death cancels stun, `Push`
  overrides with `OnShove`).
- `ACL9Item::OnPlaced(const FVector&)` — reused as the release path.
- `UCL9ItemDef::HitDamage / HitStun / UseAction` — unchanged columns; only values
  change.
- `ACL9Character::GetSwingElapsed()` — the same clock drives the ABP and the hit
  window.
