# goal: monster-individual-spread

**Status**: implementation (2026-10-21). User: "몬스터들이 좀 랜덤성도 있어야 될 것 같음 — 너무 두 마리가 똑같이 한 마리처럼 움직이게 되네" → proposal accepted ("랜덤성 진행").
Follows [chicken-thumper-chase.md](chicken-thumper-chase.md).

## One-line Goal
Each monster rolls its own speed, notice time, approach side and repath rhythm once at spawn, so two chickens never move as one — all from table columns.

## In-scope
- New `UCL9MonsterDef` Pattern columns (0 = everyone identical, old behaviour): `IndividualSpeedSpread` (≤0.5), `NoticeScaleMin` (≤1), `ChaseFlankOffset` + `ChaseFlankDistance` (offset needs distance), `ChaseRepathJitter` (≤0.9). Load checks enforce the bounds.
- Host rolls them with HP in `LoadData` (conventions §4): speed scale `1±spread` applied in `MoveAlongPath` (all moves); notice scale `[min,1]` multiplies the detect rate; flank side ∈ {-1,0,+1}.
- Chase repath goal = target + perpendicular × side × offset × clamp(dist/FlankDistance, 0, 1); falls back to the target when that point has no path. Repath interval 0.5 s × (1 ± jitter).
- Chicken row: 0.1 / 0.25 / 400 / 1500 / 0.3.

## Out-of-scope
- Windup range from the original proposal — the chicken no longer winds up (`PounceWindup` 0 since the Thumper pass); add a column when a windup species needs it.
- Coordinated pincer (assigning opposite sides to a pair) — random sides already split most pairs; revisit after play.
- Stalk/flee repath rhythm.

## Acceptance Criteria
- [x] Build OK (broker 2026-10-21 13:03); chicken row saved with the values above (data panel); no load-check error.
- [x] `Lvl_Flat` PIE (`Tools/Unreal/test_monster_spread_20261021.py`, 4 trials, two chickens 15 m ahead): every spawn logs its own roll (`rolls speed x0.90–1.10, notice x0.26–0.99, flank side -1/0/1`); 4/4 trials differ — chickens 10 m apart notice 0.13–0.24 s apart, drift to opposite sides (up to ±250 cm in 2.5 s) and end 5.8–8.8 m apart.
- [x] Other species unchanged (columns 0).

## Finding — pack alarm re-syncs neighbours
With the pair 1.5 m apart, both always started on the same frame: `PackAlarmRadius` 600 (chicken) pulls every chicken within 6 m into the chase the moment one notices. Speed and flank still pull them apart afterwards (end gap 0.9–2.2 m), but their start is shared. Left unchanged — it is the existing call/alarm design; lowering it is a user decision.

## Must-Preserve
- Host-only rolls; nothing new replicated (clients see positions).
- Rows with the columns at 0 behave exactly as before (scale 1, side 0, interval 0.5).

## Execution Notes
- Recommended model: Claude Opus 5.5 or above for AI changes; a cheaper model is fine for docs/values.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
