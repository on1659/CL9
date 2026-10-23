# goal: burnt-bait

**Status**: spec + implementation (2026-10-17) — decision F of [../review/combat-handoff-2026-10-17.md](../review/combat-handoff-2026-10-17.md) §3
("두 번째 도구는 미끼부터"). Closes the long-standing note "탄 요리는 미끼가 된다" ([../systems/cooking.md](../systems/cooking.md)) and the
`decoy` noise row that had no code path ([../reference/combat-gap.md](../reference/combat-gap.md) §1.1).

## One-line Goal
A burnt dish lying on the floor is bait: a monster that is not hunting and passes within `DecoyRadius` walks to it and spends `DecoyEatTime`
eating it — ignoring players meanwhile — and the dish is gone; landing it also rings the `decoy` wave.

## Background / Motivation
- combat-gap §1.1: CL9's only verb against a monster is "hit". Lethal and R.E.P.O. give "stun and pass", "throw and topple", "lure". The user
  chose the lure first because the design (burnt food = bait, `decoy` noise row, `DecoyRadius` 330) and the asset (burnt egg model) already
  exist — only the code path is missing.
- Valheim ([../reference/valheim/valheim-monsters.md](../reference/valheim/valheim-monsters.md)) shows the general pattern: an interest
  target (sound, item) that an idle AI walks to and lingers at, abandoned once a real target appears — here the eating window deliberately
  wins over sight so that the bait buys time.

## In-scope
- **Bait = burnt row item that is free in the world** (`ACL9Item::IsBurntRow() && IsFree()`). No new item, no new column on the item row.
- **Noise**: when a burnt item comes to rest on the host (`bAtRest` turns true in the item's physics step), report `ECL9NoiseSource::Decoy`
  once at its location — the existing `decoy` wave row (radius `DecoyRadius`) draws the ring. One ring per landing.
- **Lure (host, `ACL9Monster::SenseTick`)**: when no player is sensed this tick, the monster is in `Wander`/`Patrol`/`Return`/`Investigate`,
  `ChaseCooldownRemaining <= 0` and it has no bait yet, the nearest bait within `CL9Rules.DecoyRadius` becomes `BaitItem`; the monster enters
  `Investigate` with `LastSensedLocation` = bait location (same door as a pack alarm).
- **Eat (host, `Investigate` tick)**: while `BaitItem` is valid and still bait, the sight→hunt transition is skipped; within
  `CL9Rules.DecoyEatRadius` of it the monster stands and counts `DecoyEatTime`; at zero the item is destroyed and the monster enters `Return`.
  If the bait is picked up or destroyed meanwhile, `BaitItem` clears and the ordinary investigate search continues. Any other state entry
  (Stun, Chase from alarm, Dead …) clears `BaitItem`.
- `CL9Rules` columns (Category `Sound`, draft): `DecoyEatTime` 5.0 s · `DecoyEatRadius` 90 cm. `DecoyRadius` 330 stays the lure radius.
- Docs: [../plan/data/rules.md](../plan/data/rules.md) 소리, [../systems/monster.md](../systems/monster.md) 상태 기계 (Investigate 미끼),
  [../systems/cooking.md](../systems/cooking.md), [../plan/data/equipment.md](../plan/data/equipment.md) 추가 품목, `effects/rules.csv`, decisions.

## Out-of-scope
- Goose sleep via decoy (the other "2건" — goose row untouched).
- A hunting monster (Chase/Pounce/Stalk) being pulled off a player by bait — bait is for monsters that are not yet on you.
- Species differences (which species take bait, how long) — one global pair of values; species columns come with axis A/E.
- Eating animation / sound — the monster stands in its idle pose (the `MonsterTable` has no eat clip); a `MonsterEat` sound row can be added later.
- Burnt food in the cart cargo box or on the counter (not `IsFree` on the ground … actually `IsFree` is true in the cargo box — accepted: a cart full
  of burnt food lures monsters; that is a consequence the user can judge in play).

## Acceptance Criteria
- [ ] Throw a burnt egg: a `decoy` ring (330) appears where it lands; a wandering chicken within 330 cm walks to it, stands ~5 s, the egg
      disappears, the chicken returns home. Filmed with `/cl9film`.
- [ ] During the 5 s the chicken does not start chasing a player who walks in front of it (sight ignored while eating); a hit still stuns it.
- [ ] A raw egg or a cooked egg on the floor lures nothing.
- [ ] Picking the burnt egg up before the chicken reaches it ends the lure (the chicken searches the spot and leaves).
- [ ] `DecoyEatTime` 0 → the item is eaten on arrival; `DecoyRadius` 0 → no lure (ring only if the wave row has a fallback — it does not; both off).
- [ ] Compile via the broker.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9RulesDef.h` | `DecoyEatTime`, `DecoyEatRadius` |
| `Source/CL9/Items/CL9Item.cpp` | decoy noise on rest |
| `Source/CL9/Monsters/CL9Monster.h/.cpp` | `BaitItem`, lure in `SenseTick`, eat in `Investigate`, clear in `EnterState` |

## Must-Preserve
- Sense pipeline for players (sight/hearing/gauge) unchanged when no bait is around.
- `Investigate` search (hops, dwell) unchanged when `BaitItem` is null.
- Burnt row semantics (`BurnResult`, value 0, smoke) unchanged.
