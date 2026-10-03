# goal: combat-feel-pass-1

Status: DONE — 실행 `combat-feel-pass-1` (2026-09-22; C++·표·문서·소리 완료, 빌드 1회, D-8 PIE 측정 완료 — 결과는 `docs/testing/pie-scenarios.md` C4 ⑯ 표·`docs/decisions.md` 2026-09-22 항목·`.claude/runs/combat-feel-pass-1/pie_buffer.{json,log}`). 2인 PIE(클라 폰 밀착·표면음)와 귀로 듣는 소리 3종 구별은 미측정 — 사용자 플레이로. APPROVED (autogoal HEAVY, 2026-09-22 — panel-settled). Source review: `docs/review/combat-feel-claude-handoff-2026-09-21.md` (Codex).

## One-line Goal
Make "approach → hit → enemy reacts → step back → hit again" with one frying pan and one chicken feel connected: small knockback instead of a 15.6 m slide, a camera-based hit test that cannot reach through walls, recovery that matches the visible arm, and an optional one-slot input buffer — all measured before/after in PIE.

## Background / Motivation
The user feels play is "sloppy". The Codex review found: (A) every pan hit applies `PushKnockback 1560 cm` (a CCL8 constant, not a Lethal value) so a two-hit chicken slides out of reach after the first hit; (B) `HostSwing` rejects input for a constant 0.6 s while the visible arm returns at 0.347 s (ABP evaluates `[0, 0.347)` only — `SwingClipWindow`, `CL9RulesDef.cpp:12`); (C) the visible pan (camera-child FP arms) and the hit sphere (full-body hand + mesh bounds × `SwingHitScale 2.2`) use different clips and spaces; (D) the swing query ignores `WorldStatic`, so there is no occlusion and no surface feedback.

User decisions (2026-09-22): no dedicated shove action; pan hit = damage + `HitStunMs` stun + small knockback; `PushStun`/`PushKnockback` columns stay in the table but no row uses them.

## In-scope
1. Knockback split — item column `HitKnockback`, rule column `HitKnockbackTime`; pan row `HitStun` Push → Hit.
2. Camera-based hit test (replaces the mesh sweep), with per-monster occlusion trace and a surface-hit sound.
3. `SwingRecovery` as a rule column (cell 0.347) + `SwingBufferTime` one-slot buffer (cell 0.15).
4. Lead PIE measurement before/after, three cell sets (old / recovery only / recovery + buffer).
5. Docs: decisions reversals listed below, systems/plan tables, PIE scenario rows.

## Out-of-scope
- Movement feel (sprint ramp, turn weight), client-side swing prediction, host/client latency work.
- New monsters, items, systems. Lethal-style minimum wind-up gate (would reverse "wind-up time does not affect the result" — separate goal if ever).
- Deleting `PushReact` clips, `bPushStun` replication, `OnShove`, `PushStun`/`PushKnockback` columns (kept per user; reasons recorded in decisions).
- Surface-material-specific sounds, surface camera kick, surface noise events.

## Design Decisions (panel-settled — do not relitigate during implementation)

### D-1 Hit test origin and shape (replaces mesh sweep; no dual path)
- Origin = `GetActorLocation() + Z·(EyeHeight − CapsuleHalfHeight − 66)` ≈ hand height 90 cm. Direction = `GetControlRotation().Vector()` (same as the throw path, `CL9Character.cpp` ~1070). **Never read `FirstPersonCameraComponent` transform on the host** — `bUsePawnControlRotation` applies pitch only inside the local `GetCameraView`, so client-owned pawns on the listen host would swing flat. (Rejected: camera component origin — no pitch for client pawns. Rejected: eye-height origin — with a level aim the vertical gap to a chicken centre (36 cm) is ~120 cm > radius 80 + 36, so a chicken touching you is missed.)
- Sweep segment: `Origin + Dir·SwingHitRadius → Origin + Dir·SwingHitReach`, sphere radius `SwingHitRadius`. Starting the segment one radius forward keeps the sphere from extending behind the player (no `bStartPenetrating` hits on a chicken at your back).
- New rule columns: `SwingHitReach` (header 0, cell **100**), `SwingHitRadius` (header 0, cell **60**). Max reach = 100 + 60 + 36 = 196 cm, preserving the 2026-09-20 acceptance "no hit beyond 2 m" (C4 ⑦); lateral tolerance √((60+36)² − 54²) ≈ 79 cm ≈ old 89 cm (⑬ kept). (Rejected: 150/80 from the draft — 266 cm breaks ⑦. Rejected: `SwingHitSideOffset` — ±35 cm inside a 60 cm radius is meaningless; it was a Lethal value copied verbatim. Rejected: `SwingHitMode {Camera, Mesh}` — two live paths double every rule; rollback is git.)
- Delete: `SwingHitScale` column + its `Validate`, `ACL9Item::GetSwingHitSphere`, `SwingHitPrevCenter`, the authority full-body `AlwaysTickPoseAndRefreshBones` that existed only for the hand-position hit test (verify nothing else needs it before removal), and the corresponding sentences in `docs/plan/data/rules.md` and `docs/systems/character.md`.
- Header 0 for `SwingHitReach`/`SwingHitRadius` means "not configured" → `Validate` load error (there is no old path to fall back to). Cells are set by the lead.

### D-2 Hit window semantics unchanged
`[SwingHitStart, SwingHitEnd)` stays a per-tick window (interval-overlap logic from 2026-09-15 for low fps). Each tick re-derives the segment from the current control rotation. One hit per monster per swing, `SwingHitMonsters` as today. Only the doc wording changes: the window is "when the test is open", no longer "where the hand travels". (Rejected: single sweep at the first tick — drops aim changes during the window and creates a second rule.)

### D-3 Occlusion
For each monster returned by the sweep, line trace `ECC_Visibility` from Origin to the monster's sphere **centre** (not `ImpactPoint` — a start-penetrating hit gives a zero-length trace). If a Block lands first, skip that monster. Ignore self and the held item; if the blocker is an `ACL9Character`, `ACL9Item` or `ACL9Monster`, ignore it and re-trace — lift the `TraceWorld` lambda from `ACL9Monster` death launch (`CL9Monster.cpp` ~238) into a shared static helper. Teammates do not block hits (same principle as "players are never hit"). Always on — no column. (Rejected: "surface sweep closer than monster ⇒ drop monster" — an off-axis monster beyond a wall corner is not on the surface segment. Rejected: "don't add occlusion until observed" — at 196 cm reach, thin module walls (barnStub/wallStub) are reachable by arithmetic.)

### D-4 Surface hit — sound only
Same tick, a **line** trace (not a sphere sweep) `ECC_Visibility` from Origin to `Origin + Dir·SwingHitReach` with the D-3 ignore rules. If it Blocks on world geometry and no monster was struck closer this swing, stamp `SwingSurfaceHitServerTime` once per swing. This property replicates **without** `COND_OwnerOnly`; every machine plays SoundTable row `HitSurface` on the transition (same pattern as `TickStateSounds`). No camera kick, no arm hit-stop, no `HostReportNoise`. `GetSwingAnimElapsed` is untouched. (Rejected: sphere sweep for surfaces — at hand height a 20° downward aim touches the floor 30 cm ahead and the floor would out-rank the chicken. Rejected: reuse `SwingHitServerTime` — it drives arm hit-stop and is OwnerOnly, so the sound would be asymmetric. Rejected: small kick — dilutes the "first live monster struck" signal.)

### D-5 Knockback
- Item column `HitKnockback` (cm; header 0 = old behaviour; cells: `frying_pan` **120**, meat rows 0). Rule column `HitKnockbackTime` (s; cell **0.12**).
- `OnStrike` → `OnDamage(Amount, Source, Knockback)`. After entering Stun, wait `HitStopTime` (0.08 s, frozen frame must not slide), then move at `HitKnockback / HitKnockbackTime` for `HitKnockbackTime`, then stop — finishes inside the 0.3 s stun. Direction = horizontal Source→monster; if Source is null or coincident, `−ActorForward` (same fallback as `OnShove`). Before each step, the shared Visibility trace (D-3 helper) checks the step; blocked → stop in place (the `AddActorWorldOffset` sweep only blocks on WorldStatic, and map pieces use mixed profiles — 2026-09-18).
- Killing hits (HP ≤ 0 → `Die`) get no knockback; death launch replaces it — state this in docs so measurements are unambiguous.
- Pan row: `HitStun` Push → Hit. `ECL9HitStun::Push`, `OnShove`, `bPushStun`, `PushReact` clips remain, unused by any row.
(Rejected: per-species monster column — five identical cells. Rejected: speed = distance / stun time — couples to stun tuning and slides during hit-stop. Rejected: 0 cm — user chose "small knockback". Rejected: deceleration curve — invisible over 0.12 s.)

### D-6 Recovery
Rule column `SwingRecovery` (header **0.6** = old behaviour, cell **0.347**). `Validate`: `SwingRecovery ≥ SwingClipWindow (0.347)` and `SwingHitEnd < SwingRecovery`. When `HostTickSwingHit` stamps the first live hit of a swing, also `SwingReadyTime += HitStopTime` so miss and hit leave the same slack and the next `SwingStartServerTime` reset never lands mid-arm-animation. (The draft's "0.467 = clip length" was wrong: only `[0, 0.347)` of the clip plays.) (Rejected: keep cell at 0.6 until measured — cells revert without a build; measured as its own set in D-8.)

### D-7 Input buffer
Rule column `SwingBufferTime` (header **0** = off = old behaviour, cell **0.15**). In `Server_UseRelease`, if the swing is in recovery and `SwingReadyTime − Now ≤ SwingBufferTime`, store one pending swing (overwrite). On the host tick where recovery ends, re-run the `Server_UseRelease` checks (`HeldItem`, `UseAction == Swing`, `!IsIncapacitated()`) and call `HostSwing`. No separate cancel branches: `HostCancelSwingWindup()` clears the pending swing — it is already called on slot change, drop, throw, eat, knockdown and death (`CL9Character.cpp` ~1076/1133/1544/1946/2255). Never stack more than one. (Rejected: "re-evaluate held button at recovery end" — `Server_UseStart` already stamps wind-up during recovery and a release after recovery already swings, so it fixes nothing; the lost case is *release during recovery*. Rejected: CVar — no project convention; header 0 plays that role.)

### D-8 Measurement (lead, PIE automation + cheats)
Success criteria are observations, not implementation restatements:
1. Level aim, chicken touching the player: hit rate 100 % (origin-height check).
2. Chicken beyond 2 m: 0 hits (⑦); one step sideways: hit (⑬).
3. Visible arm return time − first accepted input time ≤ 1 frame, for both miss and hit.
4. 30 rapid taps: lost-input count for three cell sets — old (0.6 / buffer 0) · recovery only (0.347 / 0) · recovery + buffer (0.347 / 0.15). Buffer timing is stamped by the cheat from server time, not tick phase.
5. Knockback: travel ≤ 150 cm, chicken beside a wall never crosses it, frames of foot slide during knockback.
6. Wall that Blocks Visibility (verify the test piece's collision profile first — `Lvl_Flat` floor is a WorldDynamic profile): 0 hits through it.
7. Air / wall / monster produce three distinguishable sounds.
Conditions: window restored (`pie-throttle-3fps`), `t.MaxFPS` pinned, host vs joining client noted. Record before/after values, adopted/rejected, remaining hypotheses in the run report.

## Acceptance Criteria
- [x] `ItemTable` has `HitKnockback`; `CL9Rules` has `HitKnockbackTime`, `SwingHitReach`, `SwingHitRadius`, `SwingRecovery`, `SwingBufferTime`; `SwingHitScale` is gone from code, asset and docs.
- [x] `frying_pan` row is `Swing / 2 / Hit / HitKnockback 120`; meat rows `HitKnockback 0`; no row uses `Push`.
- [x] Host hit test uses control rotation + hand-height origin; occlusion trace per monster; surface line trace stamps `SwingSurfaceHitServerTime` (replicated to all) and `HitSurface` sound plays on every machine.
- [x] `Validate` rejects `SwingRecovery < 0.347`, `SwingHitEnd ≥ SwingRecovery`, and unset reach/radius.
- [x] `HostCancelSwingWindup()` also clears the pending buffered swing; no double swing from one release.
- [x] D-8 items 1–6 recorded (7 = sounds by ear: not measured; surface/hit/air distinguished only via `surface=` log field) — in the run report with numbers for all three cell sets; `docs/testing/pie-scenarios.md` C section updated.
- [x] `docs/decisions.md` records: 2026-09-15 "hitbox = mesh" and "animation, not a range number, decides" reversed (WorldDynamic-Overlap rationale kept); 2026-09-20 `SwingHitScale` retired; 2026-09-15 balance "pan = Push" rationale updated (two hits still, 15.6 m ping-pong gone); `SwingRecovery` constant → column; surface sound via `SwingSurfaceHitServerTime` transition; why Push machinery stays. `docs/systems/performance.md` rules: "per swing: monsters + 1 line traces; knockback: 1 trace per step".

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Character/CL9Character.{h,cpp}` | `HostSwing`, `HostTickSwingHit`, `Server_UseRelease`, `HostCancelSwingWindup`, new replicated `SwingSurfaceHitServerTime`, buffer state |
| `Source/CL9/Items/CL9Item.{h,cpp}` | remove `GetSwingHitSphere` |
| `Source/CL9/Monsters/CL9Monster.{h,cpp}` | `OnStrike`/`OnDamage` knockback, Stun tick timing, shared `TraceWorld` helper, surface/hit sounds |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | new columns, `Validate`, `SwingClipWindow` |
| `Source/CL9/Data/CL9ItemDef.{h,cpp}` | `HitKnockback` column + validation |
| `Source/CL9/Data/CL9SoundTable.h` | `HitSurface` row name |
| `Source/CL9/GameMode/CL9CheatManager.cpp`, `CL9Cheats.cpp` | measurement cheats (server-time stamped taps, knockback distance log) |
| `Content/Data/CL9Rules`, `ItemTable`, `SoundTable` (+ `S_HitSurface`) | cells (lead) |
| `docs/plan/data/rules.md`, `docs/plan/data/item/common.md`, `docs/plan/data/monster/common.md` | table columns |
| `docs/systems/character.md`, `monster.md`, `sound.md`, `performance.md`, `docs/testing/pie-scenarios.md`, `docs/decisions.md` | design + reversals |

## Must-Preserve
- Host authority: hit, knockback, buffer decisions on the host only; owner-only `SwingHitServerTime` drives arm hit-stop and camera kick exactly as today.
- Interval-overlap hit window (low-fps safety, 2026-09-15); one hit per monster per swing; players are never hit and never block.
- Hit-stop freezes monster mesh only (movement/AI keep running) — knockback simply starts after it.
- "Wind-up time does not affect the result" (tap attack allowed).
- Execute state ignores strikes/shoves as today (D4 no rescue).
- `PushReact` clips, `bPushStun`, `OnShove` compile and keep working for any future row.

## Execution Notes
- Recommended model: Claude Fable 5 for D-1/D-3/D-4/D-7 (host/client rotation semantics, replication conditions, buffer edge cases) and for the D-8 measurement design; Sonnet acceptable for the column plumbing, docs and table rows.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Implementation runs through `/cl9 task` (gameplay → balance → docs; lead applies cells, builds, runs D-8).

## Open Questions
(none — the Lethal-style minimum wind-up gate raised by the panel is out of scope by decision, not pending)
