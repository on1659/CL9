# goal: loop-7d-team-kill

**Status**: done (2026-10-12) — see Results. Build order ⑦ step **7d** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — user decision 4 ("팀킬 — 몬스터 타격 도구가 플레이어도 때린다"). Reverses the
"Hitting other players (friendly fire) — monsters only" out-of-scope line of [swing-hitbox.md](swing-hitbox.md).

## One-line Goal
A swing that reaches another player damages that player through the normal damage path (knock-down, death), and a death caused by a player keeps that player as the killer.

## Background / Motivation (recon 2026-10-12)
- `ACL9Character::HostTickSwingHit` sweeps a sphere on `ECC_WorldDynamic` with only `WorldDynamic` set to Overlap, so pawns never come back; the loop then keeps `ACL9Monster` only and calls `OnStrike(HitDamage, HitStun, HitKnockback, this)`.
- Player damage has one entry, `ACL9Character::OnDamage(Amount, Source, Knockback)`: ignored while knocked down or during the power intro, otherwise `HostApplyDamage(Amount, Source, "caught by")` — HP down, knock-down (`KnockdownTime` 1.5 s) or death (drop everything, death camera, `HandlePlayerDeath`), and the replicated hurt event that turns the victim's camera toward `Source`.
- Striking items in the table today: `frying_pan` (HitDamage 2, knockback 120) and `meat` (1). There is **no shovel item** — the build-order wording "삽으로" is verified with the frying pan. Player HP is 3 (`farmer_a` HpMin = HpMax = 3), so the pan knocks a teammate down (3 → 1) and the next hit after they get up kills.
- Nothing records who killed whom beyond the log line; 7g (imposter "direct kill = fail") needs the killer on the host.

## In-scope
1. **Swing hits players.** In `HostTickSwingHit`, after the unchanged monster sweep, a second overlap sweep of the same sphere on `ECC_Pawn` picks `ACL9Character`s other than the swinger. Same rules as monsters: once per swing per victim (`SwingHitPlayers`), wall occlusion via `ACL9Monster::TraceWorld` from the hand origin to the victim, counted in the swing log (`tested` / `occluded`), first connect gives the attacker's camera kick and hit-stop like a monster hit.
2. **Damage = the item's `HitDamage`** through `OnDamage(HitDamage, Attacker, HitKnockback)` — knock-down invulnerability, intro invulnerability, drop-on-death and the hurt event all apply as they do for monsters. No new data cell (balance later — loop-rework "밸런스 수치는 나중").
3. **Death cause.** `OnDamage` logs the verb `struck by` when the source is a player character (monsters keep `caught by`). When a hit kills, the victim's `ACL9PlayerState::KilledByPlayer` (host-only) is set to the attacker's player state, and one log line names the killer player; any other death (monster, left behind, cheat) clears it. `HostRevive` clears it.
4. **Docs**: systems/character.md (swing targets, death cause), goal swing-hitbox.md (dated reversal note), decisions.md, build-order 7d → done, goal README row.

## Out-of-scope
- New striking items (shovel), damage tuning, friendly-fire toggle, throw/push hitting players.
- Imposter logic, rewards, UI for "who killed me" (7g / later).
- Monster-side behaviour and the monster sweep itself (unchanged).

## Acceptance Criteria
- [x] `CL9Editor` builds.
- [x] Listen 2-player PIE (`Lvl_Henhouse`): host holding the frying pan swings (`cl9.swingtaps 1`) at the client standing in reach → client HP 3 → 1, knocked down, log `struck by`; a swing while the client is down is ignored (invulnerable); after the client gets up a second swing kills (HP 0, death), log names the killer player and `KilledByPlayer` is set.
- [x] Client world sees its own HP drop and the death (replicated).
- [x] A swing with a wall between does not hit (occluded count).
- [x] Monster hits unchanged (one swing at a chicken still strikes it).
- [x] The host never hits itself.

## Results (2026-10-12)
- **Build**: `CL9Editor` first try (81 s).
- **Listen 2-player PIE, `Lvl_Henhouse`** (`cl9.swinglog 1`; the host's pan picked up with `Server_PickPlace`; positions set from editor Python, swings by `cl9.swingtaps` on the host — a C++ timer, so the RPC/replication path is the real one):
  - **Wall**: host 50 cm in front of `SM_Henhouse_Wall_500_A`, client 60 cm behind it → `tested=10 occluded=10`, `surface` stamped, client HP 3 unchanged.
  - **Hit 1** (client 110 cm in front, kitchen room): `'BP_CL9Character_C_1' struck by 'BP_CL9Character_C_0': -2 HP → 1`, swing `hit=127.965 … tested=1 occluded=0`, hit-stop applied (`ready` 0.08 s after `arm_return`).
  - **Hit 2** after the client got up: `-2 HP → 0 (dead — down for the night)` and `'DESKTOP-…C5E6' was killed by player 'DESKTOP-…AE61'.` Client world (`UEDPIE_1`): its own player state `hp 0 dead True`.
  - **Knocked-down invulnerability** (next round, `cl9.swingtaps 2 500`): tap 1 → 3 → 1; tap 2 → `hit by 'BP_CL9Character_C_0' while knocked down — ignored (invulnerable until getting up).`, HP stays 1.
  - **Monster regression**: chicken spawned 90 cm in front, swing → `hit=278.541 tested=1 occluded=0` (struck), then the chicken caught the host (normal AI).
  - No `struck by` line ever named the swinger as its own victim.
- Note: a hit on a knocked-down teammate still counts as a connect for the attacker (camera kick / hit-stop), like a hit on a stunned monster; only the victim's damage is ignored.
- Editor left on `Lvl_Henhouse`, no dirty packages, PIE play settings ListenServer · 2 clients.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Character/CL9Character.{h,cpp}` | `HostSwing`, `HostTickSwingHit`, `OnDamage`, `HostApplyDamage`, `HostRevive` |
| `Source/CL9/GameMode/CL9PlayerState.{h,cpp}` | `KilledByPlayer` |

## Must-Preserve
- Monster sweep, monster filtering, `OnStrike` arguments, swing timing/recovery/buffer, surface sound.
- `OnDamage` gates (knocked down, intro) and the single damage path.
- Other sessions' uncommitted changes in the touched files.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (shared combat path, replication). A cheaper model is fine for doc edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Editor / build / PIE via the lock CLI (`docs/workflow/sessions.md` "잠금"), lock commands issued alone.
