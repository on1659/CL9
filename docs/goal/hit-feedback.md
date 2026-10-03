# goal: hit-feedback

**상태**: 구현 완료 2026-09-15(커밋 `3e20783` — 빨간 오버레이·공격자 킥·피격 셰이크·비네트; decisions 09-15 "타격감·피격감은 복제된 시각에서 파생한다"; PIE `docs/testing/pie-scenarios.md` C3~C5). 체크박스는 실행 당시 갱신되지 않았다. `CS_HitShake` 는 2026-09-18 사인파 오실레이터로 번복됐다(아래 정정 표기, decisions 09-18).

## One-line Goal
Make hits *feel* like hits: a struck monster and a caught player both flash red,
the attacker's camera kicks on a connecting swing, and the caught player gets a
camera shake plus a red screen vignette — using the existing replicated state
clocks, one new overlay material, two camera-shake assets and a few table cells.

## Background / Motivation
User (2026-09-15): "맞았을 때 맞는 느낌이 하나도 안 나. 몬스터랑 사람이랑 둘 다
빨간색으로 되든 뭐든 한번 해봐. 타격감과 피격감을 만들어줘." The hitbox run
(`swing-hitbox`) made strikes land correctly, and cl9-4d wired HitReact/PushReact/
Knockdown clips, but nothing else tells the player a hit connected or that they
were caught. `docs/systems/warning.md` already plans "피격: 비틀거림 + 카메라
셰이크"; this run delivers that plus the visual flash on both sides.

## In-scope
### 타격감 — the swing connected
- **Monster red flash.** On every machine, while `State ∈ {Stun, Dead}` and
  `GetStateElapsed() < CL9Rules.HitFlashTime`, the monster's skeletal mesh wears
  `CL9Rules.HitFlashMaterial` as its overlay material (`SetOverlayMaterial`,
  the same mechanism the pick-up highlight uses); otherwise the overlay is cleared.
  Keying on state + elapsed avoids the level-placed monsters whose
  `StateStartServerTime` is still 0 (cl9-4d). No new replication on the monster.
- **Attacker camera kick.** The host records `SwingHitServerTime` (replicated,
  `COND_OwnerOnly`) when `HostTickSwingHit` strikes at least one new monster in a
  swing (once per swing). The owning client, on seeing it change (OnRep or tick
  compare), plays the shake-table row `Hit` once (`StartCameraShake(class, 1)`).
  Listen-server host: same path locally.
### 피격감 — the player was caught
- **Player red flash for everyone.** While `GetKnockdownElapsed() <
  CL9Rules.HurtFlashTime`, the body mesh (and the owner's first-person arms) wear
  the same `HitFlashMaterial` overlay; cleared afterwards. `KnockdownStart-
  ServerTime` is already replicated (cl9-4d) — reuse, do not rename.
- **Owner screen vignette.** `ACL9Character::GetHurtFlash()` returns
  `1 − KnockdownElapsed / CL9Rules.HurtVignetteTime` clamped to 0..1 (0 when not
  knocked down). `UCL9HUDWidget` binds an optional `HurtVignette` image and sets
  its render opacity from that getter every tick. The image is a full-screen
  red tint (lead adds it to `WBP_HUD`: anchored 0..1, color (1,0,0), alpha 0.45,
  hit-test invisible, drawn behind the other HUD elements).
- **Hurt camera shake.** When the owning client sees `KnockdownStartServerTime`
  change, play the shake-table row `Hurt` once.
### Data / assets
- `CL9Rules` new columns: `HitFlashTime` (s, monster flash), `HurtFlashTime` (s,
  player body flash), `HurtVignetteTime` (s, vignette fade),
  `HitFlashMaterial` (`TSoftObjectPtr<UMaterialInterface>`). Draft values by
  balance; the material reference is set by the lead in `CL9Rules.uasset`.
- `CL9ShakeNames::Hit`, `CL9ShakeNames::Hurt` (row names, C++ constants) — rows
  added to `ShakeTable.uasset` pointing at `CS_HitShake` / `CS_HurtShake`.
  `FCL9ShakeInfo.StartAt` is meaningless for one-shot rows (document it).
- Assets (art writes an editor Python script under `Tools/Unreal/`, lead runs it
  from the editor Cmd bar): `/Game/Effects/M_CL9_HitFlash` (unlit translucent
  overlay, emissive red, opacity from a scalar parameter `Opacity` default 0.6),
  `/Game/Effects/CS_HitShake` and `/Game/Effects/CS_HurtShake` (duplicates of
  `CS_StaminaShake` with oscillation values from balance: Hit = short sharp
  kick ~0.15 s, Hurt = heavier ~0.5 s), and the two `ShakeTable` rows. (2026-09-18 번복 — `CS_HitShake` 는 펄린 복제가 아니라 사인파 오실레이터 `WaveOscillatorCameraShakePattern`, Pitch −3°@4Hz·0.12초·블렌드 아웃 0.04·FOV −3°; 정본 `Tools/Unreal/create_hit_feedback_assets.py` `HIT_SHAKE`. `CS_HurtShake` 는 펄린 그대로. decisions 09-18)
- Docs: `docs/systems/monster.md` (flash rule), `docs/systems/character.md`
  (피격 절: flash·vignette·shake), `docs/systems/warning.md` (몸 감각 line),
  `docs/systems/ui.md` (HUD vignette), `docs/plan/data/rules.md` (4 rows),
  `docs/testing/pie-scenarios.md` (C3 피격감 expectations, C4 타격감
  expectations), `docs/decisions.md` (lead).

## Out-of-scope
- Sound (no SFX assets exist yet — `docs/systems/sound.md` lists planned
  `SFX_*`; add `SFX_Hit`/`SFX_Hurt` there as planned names only if docs touches
  the file, no assets).
- Hit-stop / time dilation on hit (multiplayer clock).
- Damage numbers, hit markers on the crosshair, HP-loss UI beyond the vignette
  (hearts already exist).
- Changing hit-react animations (cl9-4d's, done) or the hitbox itself.
- Per-species flash colours or per-item shake strength.

## Acceptance Criteria
- [ ] Pan strike on a chicken: the chicken turns red for `HitFlashTime` on the
      host and on a client, the attacker's camera kicks once per connecting
      swing (not on a whiff), and a whiff produces no kick.
- [ ] Two chickens hit by one swing: both flash; one camera kick.
- [ ] Killing blow: the corpse flashes once (Dead state) and stops.
- [ ] Caught by a chicken: on the victim's screen the red vignette appears and
      fades over `HurtVignetteTime`, the camera shakes once; on the other
      player's screen the victim's body flashes red for `HurtFlashTime`.
- [ ] Level-placed monsters do not flash at PIE start (state-keyed).
- [ ] Build passes; `CL9Rules` has the 4 new columns and the material reference;
      `ShakeTable` has `Hit`/`Hurt`; `WBP_HUD` has `HurtVignette`.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Monsters/CL9Monster.{h,cpp}` | flash overlay in Tick (all machines), keyed on state clock |
| `Source/CL9/Character/CL9Character.{h,cpp}` | `SwingHitServerTime` (owner-only rep), hurt flash overlay, `GetHurtFlash()`, Hit/Hurt shake triggers |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | 4 columns + Validate |
| `Source/CL9/Data/CL9ShakeTable.h` | `CL9ShakeNames::Hit/Hurt` |
| `Source/CL9/UI/CL9HUDWidget.{h,cpp}` | `HurtVignette` binding + opacity |
| `Tools/Unreal/create_hit_feedback_assets.py` | material + shakes + table rows (art) |
| `Content/Data/CL9Rules.uasset`, `Content/Data/ShakeTable.uasset`, `Content/Effects/*`, `Content/UI/WBP_HUD.uasset` | lead applies |

## Must-Preserve
- Replicated field names/semantics from cl9-4d (`StateStartServerTime`,
  `bPushStun`, `KnockdownStartServerTime`, `ThrowServerTime`,
  `ThrowChargeStartTime`) — seven ABPs read their getters.
- Pick-up highlight overlay (`HighlightOverlayMaterial`) — the hurt flash must
  restore/coexist: never clear an overlay you did not set (track which overlay
  is active on the character body).
- `TakeDamage` ban; host-only judgment; presentation derived from replicated
  clocks, no multicast RPC (conventions § 9 allows Multicast for presentation,
  but the derived-clock pattern is the project default).
- No asset paths in C++ (material via `CL9Rules`, shakes via `ShakeTable`).

## Execution Notes
- Recommended model: Claude Fable 5 for gameplay (overlay ownership vs the
  highlight, owner-only replication, listen-server host path); Sonnet acceptable
  for ui, art script, balance sheet and docs.
- This document cannot enforce the model — the executing session's `/model`
  setting decides. If the session model is below the recommendation, surface it
  to the user and confirm before proceeding.
- Trap: `SetOverlayMaterial` is one slot per mesh — the pick-up highlight already
  uses it on *item* meshes, not on characters/monsters, so no clash today; keep
  a small "current overlay reason" guard anyway.
- Trap: the listen-server host is both authority and owner — OnRep does not
  fire locally; detect changes by comparing against a locally cached value in
  Tick (works for host and clients alike).
- Executed through `/cl9 task` (roles gameplay / ui / art / balance / docs, one
  build, PIE on C3·C4).

## Fairness Constraints
- Purely presentational — no gameplay state changes; all triggers derive from
  host-authored replicated clocks.

## Existing Integration Contract
- `ACL9Monster::GetStateElapsed()`, `GetState()`, `IsPushStun()` — read only.
- `ACL9Character::GetKnockdownElapsed()` — read only.
- `HostTickSwingHit` strike loop — add the owner-only timestamp write only.
- `UCL9GameInstance::GetShakeTable()` + `CameraManager->StartCameraShake` — the
  stamina shake's existing path.
