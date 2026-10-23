# goal: execute-sequence-pig

**상태**: Run 1 착수 2026-09-20(실행 execute-sequence-pig — 역할 gameplay·balance·docs), 같은 날 저녁 종 재배치(실행 execute-species-swap),
Run 2 착수 2026-09-21(실행 execute-clips — 역할 art·gameplay·balance·docs, 2인 클립 4개). **2026-10-16 정정**: Run 2 클립 4개(`AN_Pig_Execute`·`AN_Raccoon_Execute`·`AN_Farmer_Executed_Pig`·`AN_Farmer_Executed_Raccoon`)는 커밋 7e00227(09-21)·a2c4834(09-22)에, 처형 C++(`ExecuteRadius`·`bExecuteOnCatch` 등)은 4a151e7(09-21)에 들어 있다. 남은 것은 2인 PIE 검증(수용 기준 마지막 항목)뿐이다. 수용 기준 체크박스는 실행 당시 갱신되지 않았다.

**2026-09-20 밤 데스캠 도입(실행 death-camera)** — D6 의 "뷰 타깃을 죽은 폰으로 되돌리지 않는다"를 **번복**한다: 즉사 순간 몬스터 카메라에서 폰의 `DeathCamera` 컴포넌트(액터 공간 오프셋 `CL9Rules.DeathCameraOffset`에서 시체를 바라봄, `DeathCameraBlend`)로 블렌드하고 `SpectateDelay` 뒤 관전으로 간다. 데스캠은 넘어짐 1인칭 카메라와 별도 컴포넌트라 D6 이 피하려던 "시체 속에 박히는 넘어짐 카메라" 문제가 없다. 몬스터가 시체 옆에 `SpectateDelay` 까지 머무는 것·처형 중 카메라(`ExecuteCamera*`)·그 밖의 D6 은 그대로. PIE 는 `docs/testing/pie-scenarios.md` H2·C-처형-돼지 ①·C-처형-너구리 ④(2026-09-21 정정 — 옛 표기 "C-처형 ③" 은 없는 절 번호였다).

## 2026-09-20 종 재배치(사용자 결정)

Run 1 구현 직후 사용자가 종을 바꿨다 — 아래 본문(D1~D9)의 "pig" 는 **기제**를 가리키며, 어느 종이 어느 기제를 쓰는지는 이 절이 정본이다.
기제는 종에 묶이지 않고 **표 값으로만** 갈린다(`UCL9MonsterDef::IsExecutioner()` = `ExecuteRadius > 0 || bExecuteOnCatch`).

- **스토커(Bracken 식) = 너구리(`raccoon`)**. D1~D9 그대로 — 등지면 다가오고 보면 멈추고, 등 뒤 미발견에서 앵커로 **순간 이동**해 처형.
  확정값(사용자): `StalkSpeed` **450**(걷기 396 보다 빨라 걸어도 따라붙고, 달리기 654 로만 뗀다) · `StalkStareAnger` **6**(흘깃은 안전) ·
  `SleepTime` **0**(안 잔다). 나머지 열은 balance(`docs/plan/data/monster/raccoon.md`). ABP 는 `ABP_Raccoon_A` Execute 가지(`ABP_Pig_A` 사양과
  같고 평가기만 `AN_Raccoon_Catch` — `docs/systems/animation.md`). `bExecuteOnCatch` false.
- **잡히면 무조건 처형 = 돼지(`pig`)**. 새 열 `bExecuteOnCatch`(bool) true — 스토킹 없이 닭처럼 `Chase` → `Pounce` → 잡는 순간(즉시 잡기·덮침
  성공, `HostCatch` 맨 앞)이 곧 `Execute` 진입이다. **순간 이동 없음**(`HostBeginExecute(Target, Anchor, bTeleport=false)` — 닿은 자리에서 요만 맞춘다,
  앵커 스윕 없음 → 벽 옆에서도 된다). D1 의 앵커·스윕·`ExecuteAnchorDistance` 검사는 돼지에 **적용되지 않고**, D2·D3·D4(무구조)·D6(카메라)·
  D7(ABP — `ABP_Pig_A` 는 그대로, Catch 대신 Execute 상태에서 같은 클립)·D9 는 그대로다. 스토킹 열 9개는 0(`GetHuntState()` = `Chase`),
  남는 열은 `ExecuteDuration`·카메라 셋. 로드 검사 추가: `bExecuteOnCatch ⇒ ExecuteDuration > 0`. 롤백 = `bExecuteOnCatch false`(보통 잡기).
  추격 열(`ChaseSpeed`·Pounce)은 "잡히면 즉사" 기준으로 balance 가 다시 정한다(`pig.md`).
- 두 기제가 한 종에 겹치는 것(`ExecuteRadius > 0` 이면서 `bExecuteOnCatch`)은 코드가 막지 않지만 의도가 아니다 — 값으로 피한다.
- 파일 이름(`execute-sequence-pig`)·Run 2 클립 이름(`AN_Pig_Execute`)은 그대로 둔다 — Run 2 착수 때 종별로 다시 정한다(너구리 2인 클립이
  필요하고, 돼지는 잡기 자리에서 이어지는 클립이라 앵커 표가 다르다).

## One-line Goal
Give the pig a species-only instant-kill "execution": it stalks a player who is not looking at it, and when it reaches the player's back unseen it plays an execution sequence (victim camera on the pig, input locked, guaranteed death) that teammates can witness but not interrupt.

## Background / Motivation
User (2026-09-19/20): "Some monsters should have an instant-kill move like Lethal Company's Bracken — it grabs you from behind and the last thing you see is it dragging you away." Today there is no instant-kill concept (damage is a pure HP pool, death = infinite knockdown), no monster camera, and the pig is a placeholder species with copied chicken values. User decisions (fixed, do not relitigate): species = **pig**; trigger = **behind + unseen (Bracken-style) including the stalking AI** (retreats/stops when watched, approaches when not); **no rescue** once the execution starts; camera = **socket camera on the monster**, an explicit exception to the first-person-only decision (same category as spectating).

Track: **HEAVY** — new mechanism (two monster states, victim incapacitation, monster camera, two-rig animation). A 3-perspective panel + cross-examination was run; results are in Design Decisions.

## In-scope
Two runs (each = one build + one PIE). Run 1 is the whole system with reused clips; run 2 only adds the two-rig animation.

**Run 1 — system (MVP clips)**
- Monster states `Stalk` and `Execute` (appended to `ECL9MonsterState`, every switch gets a case), pig-only via table columns.
- Stalk AI: target lock, per-target unseen/stared counters, "watched" test (any living player, 2D yaw cone + LOS + radius), stop/retreat when watched, approach the target's back when not, anger → normal `Chase` after `StalkStareAnger`, `Break` on the usual give-up rules.
- Execute: entry checks (unseen time, distance, behind-cone, LOS, target not knocked down / not already executed / alive, anchor sweep clear, kill cooldown), pig teleports to the anchor behind the victim, victim incapacitated (speed 0, rotation locked, server RPC gate), dedicated `HostKill` at `ExecuteDuration`, pig holds position with camera until the victim's spectate begins, then `Break`.
- Monster `UCameraComponent` (inactive by default) + server `SetViewTarget(Monster)` for the victim only; HUD/shake/focus gated while executed.
- Table columns (`UCL9MonsterDef` "Execute" category + `CL9Rules.ExecuteCameraBlend`) with load checks; balance values for the pig (including "heavy, slow" species redefinition: `ChaseSpeed`, `CatchRadius`, `Radius`, `HpMax`, pounce columns).
- ABP branches: `ABP_Pig_A` (Stalk = walk locomotion; Execute = `AN_Pig_Catch` evaluator held on its last frame), `ABP_Unarmed` (Executed branch: idle pose frozen for `ExecuteDuration`, then the existing knockdown/down chain after the kill).
- Docs: monster.md state machine, character.md death section, common.md columns, pig.md values, decisions (camera exception, no rescue, strikes do not interrupt), pie-scenarios "C-Execute" section, cheats (`cl9.execute` optional — see Acceptance).

**Run 2 — two-rig animation**
- `AN_Pig_Execute` (Blender `animate_creature_reacts.py`-style, pig blend) and `AN_Farmer_Executed` (`farmer_pose.py`-style, farmer blend), each authored in its **own** blend file against a shared **actor-space anchor table** (distance, yaw, timing); `ExecuteDuration` raised to the clip length; ABP branches switched to the new clips; Executed branch held until respawn with its last frame clamped (pelvis XY = 0, DOWN key).

## Out-of-scope
- Darkness/flashlight modelling in the "watched" test (a player looking into the dark still counts as watching) — recorded, revisit after PIE.
- Rescue/interrupt of a running execution (user decision).
- Execution for any species other than the pig; per-species camera BP assets (camera offsets live in the monster table).
- Ragdoll, Sequencer/LevelSequence, dismemberment, blood.
- Changes to the generic Catch/Pounce rules beyond excluding executed victims.

## Design Decisions (panel-settled — do not relitigate during implementation)
- **D1 Move the pig, not the victim.** On Execute entry the host teleports the pig (`TeleportPhysics`) to the anchor = victim location − victim forward × `ExecuteAnchorDistance`, yaw facing the victim, after a sphere sweep (pig `Radius`, WorldStatic|WorldDynamic, ignoring characters/items/monsters **and the victim's `PullingCart`**) proves the anchor is free; if blocked, no execution this tick (keep stalking). Victim capsule, collision and location are untouched. Pig mesh smoothing (`SmoothedLocation/Rotation`) is frozen during Execute. Load check: `ExecuteAnchorDistance ≥ Radius + 39` (capsule radius). (Rejected: snapping the victim capsule with NoCollision — breaks rule 3 "capsule is the reference", fights client prediction, clips through walls.)
- **D2 Victim state = `ExecuteStartServerTime` (replicated to everyone, ≥ 0 while executed) + `ExecutedBy` (TObjectPtr<ACL9Monster>, replicated).** `GetMaxSpeed()` returns 0 and look input is ignored while `IsExecuted()`; visibility/input/camera side effects live in one `ApplyExecuted()` called from `OnRep` and from the host path (same pattern as `HostSetSpectating`). Server RPC gate becomes `IsIncapacitated()` = knocked down ∨ executed (swing, throw, pickup, slot, eat, use).
- **D3 Kill = dedicated `HostKill(AActor* Source)`** that bypasses the knockdown-invulnerability gate and then follows the existing death path (HP 0, `HostDropEverything`, `KnockdownEnd = ∞`, `HurtEvent`, `HandlePlayerDeath`). Monster target selection, `bCanCatch`, Pounce entry **and the Pounce lunge re-check** exclude targets with `ExecutedBy != null` (otherwise a chicken could knock the victim down and the knockdown gate would "save" them).
- **D4 Stalk target lock and counters are per target;** switching target resets `UnseenTime`/`StaredTime`. Knocked-down target → pig stops and waits. Rage (`ReceiveKillAlarm`) and pack alarm turn Stalk into Chase. `ACL9Monster::OnDamage` (where Stun is entered) and `OnShove` get an `Execute` exemption: damage applies, **no Stun, no knockback, no state change** (no rescue). If the pig dies mid-execution the victim is killed immediately (`HostKill`) and its view target is handed straight to spectating (see D6). (Rejected: releasing the victim when the pig dies — contradicts "no rescue".)
- **D5 "Watched" = any living, non-executed player whose 2D aim yaw (`GetBaseAimRotation`) is within `StalkGazeHalfAngle` of the direction to the pig, with line of sight, within `StalkGazeRadius`.** Dead pawns are excluded (a corpse must not freeze the pig). Draft half-angle 55° = world FOV 90/2 + 10 margin. Replication lag (~100 ms) is accepted and documented. (Rejected: 3D dot ≥ 0.75 as in `IsHeldByGaze` — looking at the floor would count as not watching; 41° is narrower than the screen.)
- **D6 Camera: `UCameraComponent` on `ACL9Monster` (inactive by default), activated on `OnRep_State == Execute` and on the host; the server calls `SetViewTarget(Monster)` for the victim's controller (same path as spectating).** After the kill the pig **stays in Execute** (idle over the corpse, camera held) until the victim's spectate begins (`HandlePlayerDeath` → `SpectateDelay` → `BeginSpectating` overrides the view target) or at most `ExecuteDuration + Rules.SpectateDelay`, then `Break`. The view target is never returned to the dead pawn (the knockdown camera would sit inside the now-visible body). Shakes, focus/prompts, HUD hurt vignette/arc are gated by `IsExecuted()`. Camera socket/offset/rotation are monster-table columns (species-specific); blend time is a Rules column. (Rejected: `UCameraModifier` POV-only — view target stays the pawn and `UCL9HurtLookModifier` would stack; separate camera actor — extra replication/lifetime.)
- **D7 Two runs.** Run 1 ships the system with reused clips; run 2 authors the two-rig clips in **separate** blend files sharing an actor-space anchor table (distance/yaw/timing) and verifies alignment in PIE screenshots. ABP priority is fixed from run 1: the Executed branch sits **above** the knockdown branch; in run 1 its condition is `IsExecuted() ∧ GetExecuteElapsed() < ExecuteDuration` (then the kill's knockdown chain shows), in run 2 the condition becomes `IsExecuted()` (held until respawn, last frame clamped). **피해자 요는 처형 진입 때 서버가 처형자 쪽으로 돌린다(2026-09-21 사용자 결정)** — 돼지는 정면에서 박히는 자세, 너구리는 등 뒤에서 덮이는 자세를 그대로 가정해 클립을 짤 수 있다(등을 보이고 잡힌 경우도 포함). (Rejected: authoring both rigs in one appended scene — per-rig export axes/scenes/saves differ, so an in-Blender alignment preview would be false.)
- **D8 Columns.** `UCL9MonsterDef` "Execute": `ExecuteRadius` (cm, 0 = species has no execution) · `ExecuteBehindAngle` (deg, half-angle of the cone behind the victim) · `ExecuteUnseenTime` (s) · `ExecuteDuration` (s) · `ExecuteAnchorDistance` (cm) · `ExecuteCameraSocket` (FName) · `ExecuteCameraOffset` (FVector) · `ExecuteCameraRotation` (FRotator) · `StalkSpeed` (cm/s) · `StalkGazeHalfAngle` (deg) · `StalkGazeRadius` (cm) · `StalkStareAnger` (s) · `StalkRetreatRadius` (cm; retreat uses existing `RetreatSpeed`). `UCL9RulesDef`: `ExecuteCameraBlend` (s). Load checks: all finite, non-negative; `ExecuteRadius > 0 ⇒ ExecuteDuration > 0 ∧ StalkSpeed > 0 ∧ ExecuteAnchorDistance ≥ Radius + 39 ∧ StalkGazeRadius > 0`; `StalkRetreatRadius > 0 ⇒ RetreatSpeed > 0`. Values are balance's (draft).
- **D9 Night end / lobby.** Before `ClearNightPopulation` destroys monsters, every executed character gets `HostEndExecuted()` (clears `ExecuteStartServerTime`/`ExecutedBy`, restores view target to its own pawn or spectate target); respawn (`ReviveDead`) clears the same fields.

## Acceptance Criteria
(2026-09-20 종 재배치 — 스토커 항목은 **너구리**, 잡히면 처형 항목은 **돼지**. PIE 절은 `docs/testing/pie-scenarios.md` C-처형-너구리·C-처형-돼지.)

**너구리(스토커, `ExecuteRadius > 0`)**
- [ ] Raccoon with `ExecuteRadius > 0`: when a player is sensed it enters `Stalk` (not `Chase`); while any living player watches it (cone + LOS + radius) it stops (and retreats inside `StalkRetreatRadius`); when unwatched it walks toward the target's back at `StalkSpeed` (450 — a walking player is caught up with, a running player pulls away).
- [ ] Staring at the raccoon for `StalkStareAnger` seconds (6 — a glance is safe, the counter decays when you look away) turns it into a normal `Chase` (pounce/catch as other species — an ordinary catch, since `bExecuteOnCatch` is false); hitting it while stalking does the same after the stun.
- [ ] Reaching the victim's back unseen (`ExecuteUnseenTime`, `ExecuteRadius`, `ExecuteBehindAngle`, LOS, anchor sweep clear) starts `Execute`: the raccoon teleports to the anchor, the victim cannot move/turn/act (server RPCs refused), the victim's camera blends (`ExecuteCameraBlend`) to the raccoon camera; teammates see both animations; after `ExecuteDuration` the victim dies regardless of HP, drops items, and the normal spectate flow starts with the raccoon camera held until then.
- [ ] Cart puller: the pulled cart does not block the anchor sweep.
- [ ] Rollback without a build: `MonsterTable.raccoon.ExecuteRadius 0` → raccoon behaves as before (chase/pounce/catch). Cell 0 on required companions → load error line.

**돼지(잡히면 처형, `bExecuteOnCatch`)**
- [ ] Pig with `bExecuteOnCatch`: no stalking — it senses, chases, telegraphs and pounces exactly like the chicken; the moment it catches (instant catch or pounce hit) it enters `Execute` **in place** (no teleport, no anchor sweep — works next to a wall), instead of `Catch` (no knockdown, no HP loss). Victim input lock, camera, guaranteed death, item drop and spectate flow are the same as the raccoon's.
- [ ] `cl9.execute` on a pig enters immediately wherever the pig stands (no sweep).
- [ ] Rollback without a build: `MonsterTable.pig.bExecuteOnCatch false` → ordinary catch (C3). `bExecuteOnCatch` with `ExecuteDuration 0` → load error line naming both columns.

**공통**
- [ ] No rescue: strikes/shoves on the executioner during Execute deal damage but do not interrupt; an executioner killed mid-execution still kills the victim and the camera goes straight to spectating.
- [ ] A second monster cannot catch/pounce an executed victim; a pounce already in flight misses an executed victim.
- [ ] Night end / respawn leaves no orphaned view target or stuck `ExecutedBy`.
- [ ] Run 2(2026-09-21, 실행 execute-clips): `AN_Pig_Execute`·`AN_Raccoon_Execute`·`AN_Farmer_Executed_Pig`·`AN_Farmer_Executed_Raccoon`(each 2.0s) play aligned in PIE (screenshots from the monster camera and a witness, saved under `.claude/runs/execute-clips/`), clip length = `ExecuteDuration`(2.0), the pig pair ends with the farmer in the DOWN pose and the raccoon pair ends with the farmer in the new PRONE pose, both held until respawn.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Monsters/CL9Monster.{h,cpp}` | `ECL9MonsterState` (+Stalk, +Execute), `HostTickAI` branches, Stalk/Execute logic, camera component, OnDamage/OnShove exemption, target exclusion, anchor sweep |
| `Source/CL9/Data/CL9MonsterDef.{h,cpp}` | Execute/Stalk columns + Validate |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | `ExecuteCameraBlend` |
| `Source/CL9/Character/CL9Character.{h,cpp}` | `ExecuteStartServerTime`/`ExecutedBy` replication, `IsExecuted`/`IsIncapacitated`/`GetExecuteElapsed`, `HostBeginExecuted`/`HostEndExecuted`/`HostKill`, `ApplyExecuted`, `GetMaxSpeed` gate, RPC gates, shake/focus gates |
| `Source/CL9/GameMode/CL9GameMode.cpp` | `HandlePlayerDeath`/`BeginSpectating` interaction, `ClearNightPopulation`/`ReviveDead` clearing |
| `Source/CL9/GameMode/CL9PlayerController.cpp` | `SetGameplayInputEnabled` reuse, `SetViewTarget` from server |
| `Source/CL9/GameMode/CL9HurtLookModifier.cpp`, `Source/CL9/UI/CL9HUDWidget.cpp` | gate by `IsExecuted()` |
| `Source/CL9/GameMode/CL9CheatManager.cpp` | optional `cl9.execute` (force the nearest pig to execute me) for PIE |
| `Content/Monsters/ABP_Pig_A`, `Content/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed` | ABP branches (lead, MCP) |
| `Content/Data/MonsterTable`, `Content/Data/CL9Rules` | cells (lead) |
| `Tools/Blender/animate_creature_reacts.py`(Execute 가지 추가, `execute_anchor.py` import), `Tools/Blender/execute_anchor.py`(신설 — 액터 공간 앵커 표 상수), `Tools/Blender/animate_farmer_executed.py`(신설), `Tools/Unreal/import_farmer_clips_034.py`(신설) | clips (Run 2 착수 2026-09-21, 실행 execute-clips) |
| `docs/systems/monster.md`, `docs/systems/character.md`, `docs/plan/data/monster/common.md`, `docs/plan/data/monster/pig.md`, `docs/plan/data/rules.md`, `docs/testing/pie-scenarios.md`, `docs/decisions.md` | docs |

## Must-Preserve
- Rule 2 (animation never enters judgement): every timing comes from table values (`ExecuteDuration`, etc.), clips follow.
- Rule 3 (capsule is the reference): the victim capsule is never moved or disabled (D1).
- Existing death → spectate → revive flow (`OnDamage` HP path, `HandlePlayerDeath`, `SpectateDelay`, `BeginSpectating`, truck-pad revive) — `HostKill` feeds into it, never replaces it.
- Knockdown invulnerability (2026-09-20) stays for ordinary damage; only `HostKill` bypasses it.
- Pounce/Catch behaviour of chicken/raccoon/sheep unchanged (only the executed-victim exclusion is added).
- Replication style: state + start-time replicated, no new multicasts; `State`/`StateStartServerTime` already reach everyone.
- No mesh/anim paths in C++; `PounceWindup 0`-style rollback via `ExecuteRadius 0`.

## Execution Notes
- Recommended model: Claude Fable 5 (top tier) for the gameplay slice (state machine, replication, kill path) and the panel-settled design interpretation; Sonnet acceptable for docs/balance/table-value mechanics and the run-2 Blender clips once the anchor table is fixed.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Implementation goes through the project harness: `/cl9 task` (roles gameplay · balance · docs for run 1; art · gameplay · docs for run 2; lead applies cells and ABP/WBP via MCP). Build 1 per run, PIE section "C-Execute".

## Fairness Constraints
- The host is authoritative for watched/behind/unseen checks (`GetBaseAimRotation` of remote pawns); clients cannot spoof by looking away locally faster than replication (~100 ms lag accepted).
- Execution outcome is decided at entry; no client input can alter it (server RPC gate).

## Existing Integration Contract
- `ECL9MonsterState` is appended, never reordered (replication/asset values).
- `ACL9Monster` replicated set gains nothing beyond `State`/`StateStartServerTime` already present; character gains `ExecuteStartServerTime`, `ExecutedBy`.
- `SetGameplayInputEnabled` semantics unchanged (local mapping contexts); spectate mapping context flow unchanged.
