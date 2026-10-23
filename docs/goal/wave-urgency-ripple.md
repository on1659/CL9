# goal: wave-urgency-ripple

## One-line Goal
Make every noise wave read as a water ripple (bright crest band + trailing echo crests over the CCL8 floor spokes), and let its look
change with urgency — monster state, monster HP, character HP and (when it exists) the rage gauge — with every input and every effect
switchable in the wave table.

## Background / Motivation
- 2026-10-15 the wave was rebuilt the CCL8 way (C++ drops dots on a fixed pool, `NS_CL9_WaveDots` only draws) — `docs/systems/wave.md`
  "점 그림". The user then asked (same day): the spread must differ by monster/character HP and urgency (rage/광폭화), the resource should
  look like a radar or water ripple, and whether each of these is used must live in the data table.
- User answers (AskUserQuestion, 2026-10-15): inputs = **all four** (monster state, monster HP ratio, character HP ratio, rage gauge);
  effects = **all four** (color/brightness, echo count, jitter, pulse rate); style = **ripple** (bright crest band + fading echo rings,
  floor spokes stay).

## In-scope
1. **Urgency `U` in 0..1 per wave**, computed when the wave is played, as the **max** of the row's enabled inputs:
   - Monster state — table map `ECL9MonsterState → 0..1` (replicated `State`).
   - Monster HP — `1 − Hp / MaxHp` (`Hp` replicated; add replicated `MaxHp` set where `Hp` is rolled).
   - Character HP — `1 − Hp / MaxHp` of the instigating `ACL9PlayerState` (both already replicated).
   - Rage — `ACL9Monster::GetRageAlpha()` returning 0 until the rage gauge (`docs/plan/data/monster/common.md` 광폭화 게이지) is
     implemented; the wave side is complete and the gauge plugs into that one getter.
   Per-row switches decide which inputs a row reads.
2. **Ripple look (always, switchable globally)**: as the front passes a radius it drops a short-lived, larger, brighter **crest** ring
   filling the circumference at `CrestDotGap` (not the ray angles — at ray angles the crest read as extra spokes; occlusion uses the
   nearest ray) — its fading tail is the bright band. Echo fronts follow the main front at `EchoGap` and drop dimmer crests
   only (no spokes). Spokes and wall curtain from the main front are unchanged.
3. **Effects of `U`** (each switchable globally):
   - Color/brightness — color lerps (linear RGB) toward the row's `UrgentColor` by `U`, brightness × lerp(1, `BrightnessMax`, U). Per-row
     urgent color because an HSV blend from blue to red passed through pink and a hurt player's wave read like a chicken sonar.
   - Echo count — lerp(`EchoMin`, `EchoMax`, U) rounded.
   - Jitter — crest and spoke dots offset radially/tangentially by up to `U × JitterMax` cm.
   - Pulse rate — repeating rows (`RepeatInterval > 0`, i.e. `monster_prox`) repeat at `RepeatInterval ÷ lerp(1, PulseRateMax, U)`
     (host side, in `ACL9Monster::HostTickProxNoise`).
4. All values in `UCL9WaveTable` (global `Ripple`, `Urgency` structs; per-row input booleans). Load check for broken ranges.
5. Docs: `docs/systems/wave.md`, `docs/plan/data/wave.md`, `docs/decisions.md`.

## Out-of-scope
- Implementing the rage gauge itself (separate spec — only the getter hook is added).
- Changing the wave radius or speed by urgency — radius must equal the ruling radius (`docs/plan/wave.md` rule 1) and speed is one
  for all rows ("링이 멈춘 곳까지가 들리는 곳" distance reading).
- Monster wall stamps (`bWallStamp` false on monster rows — 2026-09-16 design decision, unchanged).
- A rotating radar sweep (user picked ripple).

## Acceptance Criteria
- [x] A run wave at full HP shows a bright crest band leading the spokes plus one dim echo crest. (PIE 2026-10-15, ScreenShot00020)
- [x] The same wave from a character at low HP is warmer/brighter with more echo crests and visibly jittered dots. (HP 1/3 → U 0.67, 2 echoes, ScreenShot00021)
- [~] A chicken's `monster_prox` sonar is warmer, has more echoes and repeats faster when urgent — verified through the **HP input**
  (hit chicken: U 0.25, 2 echoes, interval 1.2 → 0.97 s). The Chase/Pounce **state** input was not observed (chickens did not chase in PIE);
  it runs through the same function.
- [x] Turning any input or effect off in `WaveTable` removes exactly that variation — by construction (each is one `if` on its switch); not
  toggled one by one in PIE.
- [x] Radius and front speed are identical at `U = 0` and `U = 1` (log: radius 390 at both; speed untouched in code).
- [x] Editor build succeeds; PIE `cl9.wave` shows the above; no LogCL9 errors from the wave load check.

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9WaveTable.h/.cpp` | new `FCL9WaveRipple`, `FCL9WaveUrgency`, per-row input switches, load check |
| `Source/CL9/FX/CL9WaveRenderer.h/.cpp` | urgency computation, crest/echo fronts, jitter, color |
| `Source/CL9/Monsters/CL9Monster.h/.cpp` | replicated `MaxHp`, `GetRageAlpha()`, pulse rate in `HostTickProxNoise` |
| `Content/Data/WaveTable` | values |
| `docs/systems/wave.md`, `docs/plan/data/wave.md`, `docs/decisions.md` | docs |

## Must-Preserve
- Ring radius = ruling radius; one speed for all rows.
- Dot pool / `NS_CL9_WaveDots` contract (`User.PoolSize`, `User.DotPositions/Colors/Sizes`).
- Own waves use the row color; slot color only for others (CCL8 D16, 2026-10-15) — urgency color lerps from whichever base applies.
- Noise events / judgment untouched except the `monster_prox` repeat interval (presentation-only noise, no ruling radius).

## Execution Notes
- Recommended model: Claude Opus 5.5 or above for the urgency/ripple design and look tuning (judgment-heavy); a cheaper model is fine
  for the table field plumbing and doc edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the
  recommendation, surface it to the user and confirm before proceeding.
