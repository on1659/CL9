# goal: game-feel

## One-line Goal
Make swinging, hitting, killing and picking up *feel* like something happened: every one of
those moments gets a sound, a hit stops time for a blink, a kill throws the body and its loot,
a pick-up has a hand motion and a slot pulse, and a struck monster shows how much is left.

## Background / Motivation
User (2026-09-18): "실제로 플레이해보면 타격감이나 아이템 파밍하는 기분, 몬스터 때려잡는
기분이 부족해." Diagnosis from code and docs (lead, same day):

1. **Zero sounds.** No `PlaySound*` call exists in `Source/CL9`; `Content/Sounds` holds only the
   mix and six sound classes. Every event in `docs/systems/sound.md` is a name only.
2. **A hit never stops.** The first-person swing clip follows `SwingElapsed` regardless of
   contact, so a hit and a whiff look identical. The `Hit` camera shake measured 0.66° in PIE —
   below the threshold where a kick is felt.
3. **Thin visual signal.** A struck monster gets a 0.15 s red overlay and a 0.3 s `HitReact`;
   no particles, no body displacement for `Hit` (only `Push` knocks back).
4. **A kill has no reward moment.** `Dead` plays a lie-down clip and `DropItems` appear
   silently on the floor.
5. **Pick-up is a teleport.** `E` attaches the item instantly; no arm clip, no slot reaction.
6. **No progress feel across the 3–4 hits** a chicken takes (HpMin 3 / HpMax 4, pan damage 1).

The judgment rules (hitbox, stun, knockdown) are correct and stay untouched — this goal adds
the presentation layers around them.

## In-scope

### 1. Synthesized SFX (7 events) and a sound table
- `Tools/Audio/synth_sfx.py` (numpy, 44.1 kHz, 16-bit mono) writes `ArtSource/audio/sfx/*.wav`:
  `pickup`, `place`, `swing_whoosh`, `hit_pan` (metallic clang), `hurt` (dull body thud),
  `monster_death` (squawk-like burst + thud), `footstep` (soft crunch; 2–3 variants).
  Scripted, deterministic, idempotent — re-running regenerates the same files.
- `Tools/Unreal/import_sfx.py` (editor Python, idempotent — `load_asset` before create) imports
  them to `/Game/Sounds/SFX/<Carry|Monster|Character>/S_*`, sets `SoundClass = SC_Effects`,
  and writes a `Concurrency` limit of 4 voices per sound.
- New data asset `UCL9SoundTable` (`Source/CL9/Data/CL9SoundTable.h`, same shape as
  `UCL9ShakeTable`): `TMap<FName, TSoftObjectPtr<USoundBase>>`; row names are C++ constants in
  `CL9SoundNames` (`Pickup`, `Place`, `Swing`, `Hit`, `Hurt`, `MonsterDeath`, `Footstep`).
  `UCL9GameInstance::SoundTableAsset` is assigned in `BP_CL9GameInstance` by the lead (no
  asset path in C++ — conventions § 5). A missing row is silent, never an error.
- Playback is presentation-only and keyed on **already replicated state clocks**; no new RPC:
  - `Swing` — on every machine when `SwingStartServerTime` changes (attached to the pawn).
  - `Hit` — on every machine when a monster enters `Stun` or `Dead` (same trigger as the red
    flash) at the monster's location.
  - `Hurt` — on every machine when `KnockdownStartServerTime` changes, at the pawn.
  - `MonsterDeath` — on every machine on `Dead` entry.
  - `Pickup` / `Place` — on every machine when the new `PickupServerTime` (see §4) changes /
    when an item leaves a hand (`OnPlaced`/`OnThrown` on the item, which already run everywhere).
  - `Footstep` — pawn and monster: a distance accumulator on the mesh owner (walk stride
    `CharacterTable`-driven if a column exists, else a fixed 120 cm run / 90 cm walk); plays a
    random variant at the feet. Not an anim notify (clips are authored without notifies).
- Volume/pitch: fixed per row in the asset; pitch randomised ±8 % in code for `Footstep` only.

### 2. Hit-stop, stronger kick, hit particle
- **Hit-stop (per-actor, no global time dilation):** on `Stun`/`Dead` entry the struck monster's
  `SkeletalMeshComponent` holds its animation for `CL9Rules.HitStopTime` (draft 0.08 s) on every
  machine (`bPauseAnims` toggled by the same elapsed check that drives the flash). On the
  attacker's machine the first-person arms freeze for the same duration: `GetSwingElapsed()`
  returns a value clamped so the `FPSwing` evaluator holds its frame while
  `SwingHitServerTime + HitStopTime > now` (owner-only, the clock is `COND_OwnerOnly`).
  Movement, AI and hitbox sweep are unaffected. Rejected: `SetGlobalTimeDilation` (affects all
  players on the listen server); `CustomTimeDilation` on the pawn (breaks movement prediction).
- **Camera kick:** `ShakeTable` row `Hit` gets a stronger asset — pitch amplitude 6°, plus FOV
  oscillation −3° over 0.12 s (balance decides final numbers; PIE-measured peak must reach
  ≥ 2° pitch). `Hurt` unchanged.
- **Hit particle:** `NS_CL9_HitPuff` (Niagara, CPU sprites, 12–20 short-lived feathers/dust)
  spawned on `Stun`/`Dead` entry at the monster mesh's `spine`/root socket, on every machine,
  by the same code path as the flash. `Dead` entry spawns it with a larger `User.Scale` (3×).
  Built by the lead through the editor (MCP Niagara pitfalls in memory apply); C++ references
  it through `CL9Rules.HitPuffSystem` (`TSoftObjectPtr<UNiagaraSystem>`), not a path.

### 3. Death: thrown body and lobbed loot
- On `Dead` entry the host launches the monster away from the strike source: horizontal
  direction = source→monster, speed `CL9Rules.DeathLaunchSpeed` (draft 420 cm/s) with an
  upward component `DeathLaunchUp` (draft 260 cm/s). `ACL9Monster` is a custom pawn (not a
  Character), so the host integrates the flight itself the way the shove knockback does
  (`AddActorWorldOffset` sweep per tick, plus gravity on the vertical part; a blocking hit or
  the floor ends it) — replication is the actor transform as today. The existing death clip
  plays while the body flies and lands. Rejected: ragdoll — `SK_Chicken_A` has no physics
  asset and none of the monster skeletal meshes do; `LaunchCharacter` — not a Character.
- `DropItems` spawn at the monster's location and are `OnThrown` with a random lob
  (`DeathLootSpeed` draft 220 cm/s, elevation 45–70°, random yaw) so they scatter and roll
  (existing item physics/sleep rules apply).
- `HitPuff` at 3× on death (see §2), `MonsterDeath` sound (§1).

### 4. Pick-up: arm clip and quick-slot pulse
- Rules unchanged: `E` attaches the item instantly to `HandGrip_R`.
- New replicated clock `ACL9Character::PickupServerTime` (host stamps in `HostPickUp`) and
  getter `GetPickupElapsed()` (−1 when none), filled into `ABP_FP_View` like the other clocks.
- New camera-space clip `AN_Farmer_FPPickup` (0.35 s one-shot, 30 fps): the right hand starts
  below the frame, comes up into the `FPHoldSmall` pose and settles with a small overshoot.
  Authored by `Tools/Blender/animate_farmer_pickup.py` in the same camera-local convention as
  the other FP clips (manifest entry `space: "camera"`), exported to `ArtSource/character/...`
  and imported by `Tools/Unreal/import_farmer_clips.py` (list extended). ABP branch: priority
  between `EatElapsed` and `ThrowElapsed`: `PickupElapsed ∈ [0, 0.35)` → evaluator
  `AN_Farmer_FPPickup` (`ExplicitTime = PickupElapsed`). Third-person body: no new clip.
- `UCL9QuickSlotWidget` gets `Pulse()` — scales the slot to 1.25 and back over 0.2 s
  (`SetRenderScale` driven from the widget tick, no UMG animation asset). `UCL9HUDWidget`
  calls it on the slot whose item def changed from empty to filled.

### 5. Monster HP pips
- Widget component on `ACL9Monster` (`UWidgetComponent`, screen space, `WBP_MonsterHp`, above
  the mesh bounds top). Shows `Hp` as filled/empty dots (● ○) up to `HpMax` of the row. Visible
  only while `GetStateElapsed() < CL9Rules.HpPipsTime` (draft 2.0 s) after `Stun` entry; hidden
  otherwise and while `Dead`. `Hp` is already replicated — no new replication.
- User note: they may later collapse this widget in UMG to hide it entirely; the C++ visibility
  logic must not fight a designer-set `Collapsed` (only toggle between `HitTestInvisible` and
  `Hidden` on a child, or respect a `bEnabled` flag on the widget class default).

### 6. Data columns (`docs/plan/data/rules.md`, `CL9RulesDef`)
`HitStopTime` (s), `HpPipsTime` (s), `DeathLaunchSpeed` (cm/s), `DeathLaunchUp` (cm/s),
`DeathLootSpeed` (cm/s), `HitPuffSystem` (soft object ref). Balance supplies draft values and
rationale; no new inequality.

## Out-of-scope
- Real (recorded/licensed) sound assets, music, ambience, VO, UI sounds — the synthesized
  files are placeholders to be swapped later; the table makes the swap a row edit.
- Ragdoll physics assets for monsters.
- Anim-notify based footsteps; monster footstep variants per species.
- Changing any judgment rule: hitbox window, stun times, HP values, knockdown, drop tables.
- Third-person pick-up animation for other players.

## Acceptance Criteria
- [ ] `py -3 Tools/Audio/synth_sfx.py` writes 9 WAV files (7 events, footstep ×3) under
      `ArtSource/audio/sfx/`; running twice yields identical bytes.
- [ ] After `import_sfx.py`, `/Game/Sounds/SFX/**/S_*` exist with `SoundClass = SC_Effects`,
      and `SoundTable` has 7 rows; `BP_CL9GameInstance.SoundTableAsset` set.
- [ ] PIE C4: a connecting pan swing plays `Hit`, the chicken mesh freezes ≥ 0.08 s while the
      arms hold their frame, a puff spawns at the chicken, camera pitch peak ≥ 2°. A whiff plays
      only `Swing` and nothing else.
- [ ] PIE C3: being caught plays `Hurt` at the pawn on both machines (2-player PIE).
- [ ] PIE C5: the killing hit launches the chicken away from the player (it lands ≥ 100 cm
      from where it stood), its loot scatters (≥ 2 items ≥ 30 cm apart when the row drops ≥ 2),
      `MonsterDeath` plays, the 3× puff shows.
- [ ] PIE B1: `E` plays `Pickup`, the arm clip runs 0.35 s from below the frame into hold
      pose, the filled quick-slot pulses once. `G` plays `Place`.
- [ ] PIE C4: after a non-lethal hit the pips above the chicken show `Hp` dots for ~2 s and hide;
      an unstruck chicken shows nothing.
- [ ] Walking and running play footsteps at stride intervals; standing plays none.
- [ ] Load check: a missing sound row or missing `HitPuffSystem` logs nothing and does not
      crash; `check_inequalities.py` output unchanged.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Monsters/CL9Monster.{h,cpp}` | Stun/Dead entry hooks: pause anims, puff, sounds, death launch, loot lob, HP widget |
| `Source/CL9/Character/CL9Character.{h,cpp}` | `PickupServerTime`, `GetPickupElapsed`, swing clamp for hit-stop, Swing/Hurt/Pickup sounds, footstep accumulator |
| `Source/CL9/Data/CL9SoundTable.h` (new) | Sound rows by name |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | Six new columns |
| `Source/CL9/GameMode/CL9GameInstance.{h,cpp}` | `SoundTableAsset` slot + registry access |
| `Source/CL9/Items/CL9Item.cpp` | `Place` sound in `OnPlaced`/`OnThrown` |
| `Source/CL9/UI/CL9QuickSlotWidget.{h,cpp}`, `CL9HUDWidget.{h,cpp}` | `Pulse()` and its trigger |
| `Source/CL9/UI/CL9MonsterHpWidget.{h,cpp}` (new) | Pips widget class for `WBP_MonsterHp` |
| `Tools/Audio/synth_sfx.py` (new), `Tools/Unreal/import_sfx.py` (new) | Sound generation and import |
| `Tools/Blender/animate_farmer_pickup.py` (new), `Tools/Unreal/import_farmer_clips.py` | Pick-up arm clip |
| `Content/Effects/NS_CL9_HitPuff`, `Content/Effects/CS_HitShake`, `Content/Data/SoundTable`, `Content/Data/CL9Rules`, `Content/UI/WBP_MonsterHp`, `Content/Characters/FarmerA/ABP_FP_View`, `Content/GameMode/BP_CL9GameInstance` | Lead-applied assets |
| `docs/systems/sound.md`, `monster.md`, `character.md`, `ui.md`, `animation.md`, `docs/plan/data/rules.md`, `docs/testing/pie-scenarios.md` | Docs |

## Must-Preserve
- Hitbox window (`SwingHitStart/End`), stun/knockdown timings, HP and drop tables — untouched.
- Sounds and particles never run on the server-only path; all presentation keys on replicated
  clocks/state (conventions § 9). No new Server RPC.
- `KnockdownStartServerTime`, `SwingHitServerTime`, `StateStartServerTime` names and semantics.
- C++ holds no asset paths; new assets enter through `CL9Rules`, `SoundTable`, `ShakeTable`,
  `BP_CL9GameInstance`.
- `ABP_FP_View` branch priorities for knockdown/eat/throw/swing stay; pick-up slots in below eat.
- Other sessions' uncommitted work in `CL9Character`, `CL9Monster`, `CL9GameInstance`,
  `CL9ItemDef.h`, `CL9Item.cpp` (wave-impl, map fog) must be committed or hunk-separated
  before this goal's edits land in the same files.

## Execution Notes
- Recommended model: Claude Fable 5 for the hit-stop/clock clamp design and the death-launch
  interaction with the state machine (judgment-heavy, touches replication); Sonnet acceptable
  for the WAV synthesis script, import script, and docs.
- This document cannot enforce the model — the executing session's `/model` setting decides.
  If the session model is below the recommendation, surface it to the user and confirm before
  proceeding.

## Fairness Constraints
- All effects are cosmetic; host judgment is unchanged. Death launch is host-integrated
  movement (replicated as the actor transform), never a client-side impulse.

## Existing Integration Contract
- `ACL9Monster::OnStrike(int32 Damage, ECL9HitStun Stun, AActor* Source)` — `Source` is the
  launch direction origin for death.
- `ACL9GameMode::SpawnItem(FName, FName, const FTransform&, int32)` + `ACL9Item::OnThrown(
  const FVector& Origin, const FVector& LaunchVelocity)` — loot lob.
- `UCL9ShakeTable` rows `Hit`/`Hurt`; `FCL9ShakeInfo.StartAt` meaningless for one-shots.
- `UCL9HUDWidget::QuickSlots` / `QuickSlotDefs` — the change detector for the pulse.
