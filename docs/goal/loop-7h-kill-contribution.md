# goal: loop-7h-kill-contribution

**Status**: done (2026-10-12 — listen 2-player PIE verified, uncommitted). **2026-10-16 정정**: 코드·규칙 셀은 커밋 a5af9f2(2026-10-13)에 들어갔다. Build order ⑦ step **7h** ([../plan/build-order.md](../plan/build-order.md)).
Parent concept spec: [loop-rework.md](loop-rework.md) — decision 4 ("kill contribution would be nice"), "구현 가능성 — 처치 기여", open question 6 answered 2026-10-12.

## One-line Goal
A monster remembers which player woke it and when; if it kills another player within a time window, that death is credited to the waker — shown on the dawn screen, counted as killing the imposter, and paid to an imposter per induced death.

## Background / Motivation
- Concept (2026-10-11): the imposter wins by making others die "naturally" — luring monsters. Direct kills are already recorded (`KilledByPlayer`, 7d); induced kills need "who woke this monster, when".
- User answers 2026-10-12 (open question 6): limit contribution to **the player who woke the monster**; window is a data cell (draft **30 s**); use it for **all three**: (1) it counts as killing the imposter (kill reward), (2) it is listed on the dawn screen, (3) the imposter earns extra 이더 per induced death.
- Recon: a sleeping monster wakes from (a) a living player walking within `WakeRadius` / running within `HearRunRadius` (the triggering player is known in the loop), (b) a pending alarm spot — pack alarm (`FirePackAlarm`, the caller's `SensedCharacter` is the player it is calling about) or kill alarm (`Die(Source)`, the source is the player who killed it). Noise reports (`HostReportNoise`) are visual only and wake nothing. Monster kills reach `ACL9Character::HostApplyDamage` with the monster as the source (catch → `OnDamage`, execution → `HostKill`).

## In-scope
1. **Waker memory** (`ACL9Monster`, host only): `WakerPlayer` + `WakeTime` set when the monster actually wakes (leaves Asleep for WakeTelegraph — a gaze-held retry does not count). Waker = the player behind the pending alarm if the wake came from an alarm, else the walking/running player that tripped it. Alarms carry the cause: `ReceivePackAlarm(Spot, Cause)` (the calling monster's sensed player), `ReceiveKillAlarm(Spot, Time, Cause)` (the player who killed the screaming monster, null otherwise). Log `'monster' woken by 'player' (walk|run|alarm).`
2. **Credit** — `ACL9Monster::GetContributor(Victim, Window)`: the waker if set, not the victim, and `now − WakeTime ≤ Window`. `HostApplyDamage` at death, when the source is a monster, stores it on `ACL9PlayerState::InducedByPlayer` (+ the monster's display name), host only, cleared at revive like `KilledByPlayer`. Log `'victim' was caught by a monster 'player' woke Ns ago — contribution.`
3. **Cells** (`CL9Rules`, category "Imposter"): `ContributionWindow` (s, draft 30, 0 = off), `ImposterAssistReward` (이더 per induced death, draft 10). Load check: finite ≥ 0 / ≥ 0.
4. **Imposter killed** — `JudgeImposter`: a dead imposter with no `KilledByPlayer` but an `InducedByPlayer` is **Killed** by that player (kill reward, dawn line names them).
5. **Imposter assist** — `GrantRoundWallet`: the round's imposter (won or not) gets `ImposterAssistReward` × the number of players whose `InducedByPlayer` is the imposter.
6. **Dawn list** — `FCL9Contribution {VictimName, WakerName, MonsterName}` array replicated on the game state (cleared at round start, set in `EndNight` before the wallet step); each entry is a dawn line `Dawn.Contribution`.
7. Docs: rules.md (cells), systems/monster.md (waker memory), session.md (EndNight order), ui.md (dawn line), decisions.md, build-order 7h → done, goal README.

## Out-of-scope
- Other indirect means (doors, stealing carry, leading an already-awake monster) — the user limited it to waking.
- Chained credit through several alarms beyond one hop (an alarm passes the cause it was given; a monster woken by an alarm does not pass its own waker on when it later calls).
- Balance of the window and the assist reward.

## Acceptance Criteria
- [x] `CL9Editor` builds; `cl9.imposter` is a console command now (7g follow-up).
- [x] Listen 2 players: host kills a monster (`cl9.hitmonster`) → nearby sleepers log `woken by '<host>' (alarm)`; the client then dies to one of them (`cl9.damage 99 1`, nearest monster = the woken one) → `contribution` log, both dawn screens show the contribution line.
- [x] Client = imposter, same setup → `Imposter: '<client>' was killed by '<host>'` (induced), host wallet + kill reward.
- [x] Host = imposter, same setup, host in the truck → imposter won + assist: `30 instead of …` and `+10 × 1 induced`.
- [x] A death long after the wake (window shrunk in memory) → no contribution.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Monsters/CL9Monster.{h,cpp}` | waker memory, alarm cause, `GetContributor` |
| `Source/CL9/Character/CL9Character.cpp` | credit at death, clear at revive |
| `Source/CL9/GameMode/CL9PlayerState.h` | `InducedByPlayer`, `InducedByMonsterName` |
| `Source/CL9/Data/CL9Types.h`, `Source/CL9/GameMode/CL9GameState.{h,cpp}` | `FCL9Contribution`, replicated list |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | imposter credit, assist reward, publish list |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}` | cells |
| `Source/CL9/UI/CL9DawnWidget.{h,cpp}` | dawn lines |
| `Content/Strings/CL9_Strings.csv` (+ `CL9.csv`) | `Dawn.Contribution` |

## Must-Preserve
- Monster wake rules (gaze hold, retry, alarm behaviour) — only bookkeeping is added.
- 7g judgement order and the one-grant-per-round wallet rule.
- Other sessions' uncommitted changes in the touched files.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above (host-only bookkeeping across monster AI and death paths). A cheaper model is fine for strings and docs.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.

## Results (2026-10-12)
- Build: first attempt failed (`ACL9Character::GetCL9PlayerState` is private — switched to `GetPlayerState<ACL9PlayerState>()`), second OK. The new DLL loaded (no Smart App Control block this time). `cl9.imposter` now answers on the console (`cl9.imposter: now none, next round rolled.`).
- Editor: before the build another user was working in this editor over MCP (sequencer keys, an unsaved FBX import, an unsaved `Lvl_Flat`); the user chose to close it anyway (2026-10-12 17:18).
- Listen 2 players, `Lvl_Henhouse`, round clock 60 s in memory (restored to 245, saved). Setup each time: two sleeping geese spawned in front of the host (`cl9.addmonster goose 2` — the goose row starts asleep), the host kills one (`cl9.hitmonster 99`), the client is placed next to the other and killed by it (`cl9.damage 99 1`, nearest monster = the woken goose).
  - No imposter: `'BP_CL9Monster_C_7' woken by '<host>' (alarm).` → 4 s later `'<client>' was caught by …, woken by '<host>' — contribution.` → both dawn screens `거위에게 당한 <client> — 깨운 사람 <host>`.
  - Client imposter: `Imposter: '<client>' was killed by '<host>' (woke the monster).`, host `+20 for killing the imposter` (80 → 100); both dawn screens show the imposter line and the contribution line.
  - Host imposter, host in the truck: `won`, host `1000 coins × 0.01 -> 40 ether — imposter won: 30 instead of 10 — +10 × 1 induced death(s)` (100 → 140).
  - `ContributionWindow` 0.5 s in memory, kill 3.9 s after the wake → no contribution line (restored to 30, saved).
- The test PIE was closed from the editor window once mid-run (08:23:29, the client window) and restarted from the editor; the rounds above are from the next PIE session.
