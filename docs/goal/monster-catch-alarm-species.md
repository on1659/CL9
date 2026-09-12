# goal: monster-catch-alarm-species

## One-line Goal
Finish build-order ③ overnight: 3e (catch + knockdown + load spill), 3f (pack/kill
alarms + revenge), 3g (goose + raccoon rows on the existing frame), unattended —
the user tests in the morning.

## Background / Motivation
3a~3d are done and verified (chicken wanders, senses, chases, gives up, walks with
a real AnimBP). The remaining ③ steps are fully specified in docs/systems/monster.md,
docs/systems/character.md and docs/plan/data/monster/*.md; the user asked for an
autonomous run with per-step user verification skipped ("개발하고 확인 스킵하고
다음 스텝", 2026-09-03).

## In-scope
- **3e — catch**: `ICL9Damageable` interface (host-only `OnDamage(Amount, Source)`);
  player HP on `ACL9PlayerState` (rolled from CharacterDef HpMin..HpMax);
  chicken catch inside `CatchRadius` → `OnDamage(CatchDamage)` + knockdown +
  spill of hand+pocket (draft: everything), `KillCooldown` reload between catches;
  monster `Catch` state (brief pause, anim hook); monster-side `OnDamage`
  (HP, `HitStunMs` stun, death) + `cl9.hitmonster` cheat to exercise it.
- **3f — alarms**: pack alarm on first alert (`PackAlarmRadius`, `ChaseCooldown`
  respected); goose alarmist path (`bAlertsOthers` + `AlertWindup` +
  `AlertCooldown`); kill alarm on death (`KillAlarmRadius`) → `RageTime` hunt
  mode, spread via `RevengeRadius`/`RevengeTime`; death drops `DropItems`.
- **3g — species**: Asleep/WakeTelegraph support (goose sleeps, `WakeRadius` walk
  wake, `HearRunRadius` run wake, `WakeSightRadius` hold + `WakeRetryTime`,
  `SleepTime` re-sleep); goose row (values from goose.md, SK_Chicken stand-in
  mesh/ABP); raccoon row (draft values chosen here, Fox stand-in mesh + ABP_Fox);
  monster gimmick layer already runs through `UCL9DefBase::Gimmicks` — verified by
  the rows loading.
- Two builds (DebugGame + Development editor targets), per-step commits, doc
  updates (raccoon.md draft values, character.md KnockdownTime, decisions.md).

## Out-of-scope
- 4a spectate/truck-pad heal (player death here = stay down; spectate is ④).
- Frying-pan push (equipment, ④) — `PushStun`/`PushKnockback` stay unread.
- Zone check on the kill alarm (flat map has no zones — map ⑤).
- Blackout senses (`SightDark`) — blackout arrives in ④.
- Goose "must not block chokepoints" spawn predicate (map generator, ⑤).

## Design Decisions (defaults chosen, documented as drafts)
- Knockdown blocks movement via the CMC: replicated `KnockdownEndServerTime` on
  the character, `GetMaxSpeed() == 0` until it passes (same replicated-read path
  as CartMult). Docs left "CMC mode vs input block" open — this is the lighter one.
- `KnockdownTime` is a new CharacterDef column (draft 1.5s) — principle 3 says
  every number lives in an asset.
- Catch pause is a presentation constant (0.8s) in the monster cpp, like the
  wander dwell constants — no table column until a species needs a different one.
- Spill uses the throw path (`OnThrown`) with small random velocities so loads
  scatter with real physics; pocket items respawn as actors at chest height.
- Raccoon draft values (raccoon.md updated): ChaseSpeed 462 (< sustained 506 —
  A1 holds with CatchRadius > 0), otherwise a slightly faster, wider-leashed
  chicken. Values are placeholders to prove "new species = new row".
- Goose/raccoon meshes are stand-ins (SK_Chicken / AVP Fox) per principle 2.

## Acceptance Criteria
- [ ] Chicken catches inside 84cm: player HP drops, falls (cannot move ~1.5s),
      hand+pocket items scatter on the floor; next catch only after 9s.
- [ ] `cl9.hitmonster` kills a chicken → DropItems (none configured yet — no spawn),
      monsters within 2100cm rage for 9s and hunt.
- [ ] First alert wakes pack members within 600cm into Investigate.
- [ ] Goose row: sleeps, wakes when walked at ≤450cm or run at ≤1260cm, honks
      after 0.55s and alerts, chases at 528 but never catches (CatchRadius 0).
- [ ] Raccoon row spawns with Fox mesh and walks/runs with ABP_Fox.
- [ ] Both editor targets build clean; registry load checks pass for all 3 rows.

## Related Files / Modules
| File | Role |
|------|------|
| Source/CL9/Core/CL9Damageable.h | new — the one damage interface |
| Source/CL9/Monsters/CL9Monster.h/.cpp | catch, stun/death, alarms, sleep states |
| Source/CL9/Character/CL9Character.h/.cpp | OnDamage, knockdown, spill |
| Source/CL9/Character/CL9CharacterMovement.cpp | speed 0 while knocked down |
| Source/CL9/GameMode/CL9PlayerState.h/.cpp | replicated Hp |
| Source/CL9/Data/CL9CharacterDef.h | KnockdownTime column |
| Source/CL9/GameMode/CL9CheatManager.h/.cpp | cl9.hitmonster |
| Content/Data/MonsterTable | goose + raccoon rows (editor/MCP) |
| Content/Monsters/ABP_Fox | raccoon stand-in locomotion (editor/MCP) |
| docs/plan/data/monster/raccoon.md | draft values recorded |
| docs/plan/data/character.md, docs/decisions.md | KnockdownTime, choices log |

## Must-Preserve
- Host-only rulings (`HasAuthority` gates); clients render replicated state.
- No `TakeDamage`/`UDamageType`; HP is plain replicated int32.
- Ruling state on PlayerState, never on the character (CMC-predicted state
  excepted); every replicated field registered in `GetLifetimeReplicatedProps`.
- No asset paths in C++; every number from a table row.
- Existing chicken 3d behavior (verified tonight) must not regress.

## Execution Notes
- Recommended model: Claude Fable 5 for the state-machine and replication work
  (judgment-heavy, easy to desync); mechanical row entry may run on a cheaper
  model. This document cannot enforce the model — the executing session's
  `/model` decides; if below the recommendation, surface it and confirm.
- This session runs unattended by explicit instruction: per-step user checks are
  skipped, compile + a PIE crash-smoke replace them; the user tests tomorrow.
