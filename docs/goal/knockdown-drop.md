# goal: knockdown-drop

**Status**: spec + implementation (2026-10-17) — decision B of [../review/combat-handoff-2026-10-17.md](../review/combat-handoff-2026-10-17.md) §3.
Reverses 2026-09-15 "짐은 죽을 때만 흘린다" ([../decisions.md](../decisions.md) 2026-10-17 전투·조작감 결정).

## One-line Goal
When a player is knocked down (HP remains), 0–2 of their quick-slot items fall at their feet — the count is rolled from three
table weights, two being the rare outcome — so that getting caught costs cargo, not only HP and time.

## Background / Motivation
- [../reference/combat-gap.md](../reference/combat-gap.md) §1.4: in R.E.P.O. 22 of 28 monsters never touch HP — the threat is loss of
  control and cargo; Lethal targets heavy carriers and a death loses everything. In CL9 a knockdown left all four slots intact, so the
  extraction game's real stake (cargo) was not wagered in combat at all.
- User (2026-10-17, question-box answer to axis B): "넘어지면 랜덤으로 아이템 떨구기 0~2개. 2개는 낮음."
- Content Warning ([../reference/contentwarning/contentwarning-monsters.md](../reference/contentwarning/contentwarning-monsters.md)) makes
  "drop what you hold" a monster verb (Upscream-like forced release); here it is a property of every knockdown instead, which keeps the
  monster rows untouched.

## In-scope
- `CL9Rules` columns (Category `Carry`, all draft): `KnockdownDropWeight0` 40 · `KnockdownDropWeight1` 45 · `KnockdownDropWeight2` 15.
  Relative weights of dropping 0, 1 or 2 items on a knockdown. All three 0 → no drop (old behaviour). Values live in
  [../plan/data/rules.md](../plan/data/rules.md) 던지기와 운반.
- Host: `ACL9Character::HostDropOnKnockdown()` called from `HostApplyDamage` on the HP-remains branch (after `HostCancelCarryActions`):
  roll the count, pick that many **distinct filled quick-slot slots at random** (the active slot counts; the head item does not), and
  spill each exactly as death does — the held actor is released and `Spill`ed, a stowed slot is re-spawned from its saved data
  (alias · sub-alias · value · light switch) and `Spill`ed, the slot is cleared, the active slot number stays. Fewer filled slots than the
  roll → drop what there is.
- `NotifyCarryChanged()` once when anything dropped; the knockdown itself (timers, hurt event, invulnerability) is unchanged.
- Docs: [../systems/inventory.md](../systems/inventory.md) 잡힘 row, [../systems/character.md](../systems/character.md) 피격·넘어짐,
  [../plan/data/rules.md](../plan/data/rules.md), `Tools/DataPanel/effects/rules.csv`, decisions.

## Out-of-scope
- Head item (headlamp) — stays on the head (worn, not a quick slot).
- Death — still `HostDropEverything` (all slots).
- Weighting the roll by what is held (heavy cargo, lit flashlight) or by monster species — species rows are untouched (decision axis A/E pending).
- Any new replication — the spill uses the existing item actors; clients see items appear as they always did.
- Picking up while knocked down (still blocked by the knockdown rule).

## Acceptance Criteria
- [ ] With default weights, 100 simulated knockdowns of a 4-slot carrier drop 0 / 1 / 2 items in roughly 40 / 45 / 15 proportion (log count).
- [ ] A knockdown with the flashlight lit in slot 2 and the egg held: whichever falls lands at the feet; a dropped lit flashlight keeps shining
      ([../systems/light.md](../systems/light.md)); the active slot number does not change; the other slots are intact after getting up.
- [ ] Weights 0/0/0 → no item falls (old behaviour).
- [ ] Death still drops everything; the head item never falls on a knockdown.
- [ ] Compile via the broker; filmed with `/cl9film` (chicken catch with a full quick bar).

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.h` | three weight columns |
| `Source/CL9/Character/CL9Character.h/.cpp` | `HostDropOnKnockdown`, call in `HostApplyDamage` |
| `Content/Data/CL9Rules` | cells (code defaults apply until edited) |

## Must-Preserve
- Knockdown invulnerability, timers and the hurt event exactly as before.
- `HostDropEverything` on death; `Spill` physics (sideways offset, `DeathLootSpeed`-style launch is **not** used here — `Spill` is the
  existing feet-drop).
- Active slot number stays across the knockdown (inventory.md 잡힘).
