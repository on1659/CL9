# goal: wave-landing-ping

## One-line Goal
Turn the landing wave (`land` row) into a deliberate "ping": it spreads until it meets a wall (up to a 12 m cap), passes through
people, monsters and items while its floor spokes ride up and over them, and it is gated by a 2-second table-driven cooldown.

## Background / Motivation
User (2026-10-16): the sound wave should travel until it meets a wall — but only for the player, and only on landing (a jump is
an occasional, deliberate act; running every 0.24 s would light the whole room continuously and replace the flashlight).
Revealing monster positions and silhouettes in the dark is intended. "Sound travels far" is acknowledged as a balance concern.
The landing row has no ruling (hearing) radius today (`docs/plan/data/wave.md` — `land` uses `FallbackRadius` 378), so this
changes presentation only; monster hearing is untouched.

## In-scope
- New `FCL9WaveInfo` row fields (table-driven, `Content/Data/WaveTable`):
  - `bUntilWall` — when on (and not cooling down) the ring radius becomes `UntilWallRadius` instead of the resolved radius; every
    spoke runs until its ray meets a wall or the cap. Brightness falloff and wall-stamp strength use that cap.
  - `UntilWallRadius` (cm) — the cap where no wall is met. `land` = 1200.
  - `UntilWallCooldown` (s) — after an until-wall ping by the same instigator on the same row, further events within this time
    play as the ordinary ring (resolved radius). `land` = 2.0.
  - `bPassThroughBodies` — occlusion rays skip people (`ACL9Character`) and `ACL9ActorBase` actors whose kind is Item or Monster
    (carts included) and continue to the wall behind; floor spoke dots (and crest dots) ride over whatever is at the spot.
  - `RideHeight` (cm) — the highest surface a riding dot climbs onto (probe start height above the source floor). `land` = 250.
- Riding: when the front passes a spoke sample, probe straight down (object types WorldStatic, WorldDynamic, Pawn, PhysicsBody,
  ignoring the instigator's pawn) from `RideHeight` to below the source floor; the dot stands on the highest hit. When the height
  jumps between consecutive samples of a spoke by more than one `Dots.RingSpacing`, fill the face with a vertical dotted line at
  the same spacing that rises like a wall stamp (`StampRiseScale`). Crest (ripple) dots use the height last sampled on their nearest
  spoke.
- Load check: `bUntilWall` / `bPassThroughBodies` on a monster row (`monster_prox`, `monster_call`, `monster_pounce`) → Error log
  and the flags are ignored at runtime (the row itself stays enabled). `bUntilWall` with `UntilWallRadius` ≤ 0 → Error, ignored.
- Table values: `land` row `bUntilWall` true, `UntilWallRadius` 1200, `UntilWallCooldown` 2.0, `bPassThroughBodies` true, `RideHeight` 250.
- Docs: `docs/systems/wave.md`, `docs/plan/data/wave.md`, `docs/plan/wave.md` (rule exception for landing), `docs/decisions.md`.
- Visual check: `/Game/Maps/Test/Lvl_WaveTest` filmed with the bot (`Tools/Film/shots/`), video sent to the user.

### Silhouette reveal (added 2026-10-16 after the dark test)
User: "the point is whether you can make out the object's shape in the dark — the wave must reveal the object's silhouette".
The dark film showed riding spokes alone draw only two or three lines across a person (spokes are 6° apart: 21 cm at 2 m), so the
shape did not read. Added:
- Row field `bRevealBodies` (player rows only) and table struct `Reveal` (`Material`, `Spacing`, `DotRadius`, `Push`, `Brightness`,
  `Hold`, `Fade`). When the ping starts, every body (same set as pass-through) inside the radius and not behind a wall gets a **shell**:
  a copy of each visible mesh component attached to it (skeletal copies follow the original pose via LeaderPose) wearing a dynamic
  instance of `/Game/Effects/M_CL9_WaveReveal` — unlit additive world-space dot grid, visible only inside the wavefront radius
  (`FrontRadius`, updated each tick) so the body lights up as the front sweeps through it, pushed 0.5 cm out along normals. After the
  front passes the body it holds `Hold` s, fades over `Fade` s, and the shells are destroyed.
- Why shells, not the overlay-material slot: the overlay slot is already shared by hurt flash, aim highlight, cook overlay and monster
  hit flash, each of which sets/clears it — a ping would fight them. Shells never touch the original materials or overlay.
- `land` = `bRevealBodies` true; `Reveal` = M_CL9_WaveReveal, Spacing 3.5, DotRadius 0.22, Push 0.5, Brightness 3.0, Hold 0.6, Fade 0.8
  (first take at 4 / 0.18 / 1.0 / 0.4 read too faint next to the spokes). Dark film `wave_ping_dark_20261016e`: a standing person reads as a
  dotted human figure (head, shoulders, arms, legs), the cart as a dotted box.
- Material is built by `Tools/Unreal/create_wave_reveal_material.py` (idempotent).

## Out-of-scope
- Monster hearing of landings (a hearing radius for `land`) — a monster-design decision, separate.
- Any other row using until-wall or pass-through (table can switch them on for player rows later; not done here).
- Making pawns/monsters ignore `CL9_Occlusion` globally (09-16 follow-up) — pass-through here is per row, inside the wave trace.
- Changing replicated noise events (`Multicast_Noise` signature) — the cooldown is decided on each machine from the events it receives.

## Acceptance Criteria
- [x] A landing wave in `Lvl_WaveTest` spreads past 3.9 m and stops at the walls (or 12 m), with wall stamps on walls beyond the old radius. (film 2026-10-16 `wave_ping_test_20261016b` — stamps on the 4.3 m wall)
- [x] Landing again within 2 s draws the ordinary 378 cm ring; after 2 s the next landing pings again. The time is a table value. (log: `(ping) radius 1200` then 0.68 s later `radius 378`)
- [~] A chicken / item standing in the path does not cut the spoke: the dots climb its front, run over its top and come back down, and
      the wall behind it still gets its stamp. (verified on the cart — an Item; monsters take the same path, their root sphere is
      `WorldDynamic` which the probe queries, but a monster was not filmed standing still)
- [x] Other rows (`run`, monster sonar …) behave exactly as before. (flags off on every other row; code paths gated on the flags)
- [ ] Load check logs an Error and ignores the flags when they are set on a monster row. (implemented; not exercised — no monster row has the flags)
- [x] Editor build via the broker succeeds; video sent with `SendUserFile`. (compiled in broker build #139 at 13:16 — my own request #138 failed on another session's `CL9LookDef`)

## Related Files / Modules
| File | Role |
|------|------|
| `Source/CL9/Data/CL9WaveTable.h/.cpp` | new row fields, load check |
| `Source/CL9/FX/CL9WaveRenderer.h/.cpp` | until-wall radius + cooldown in `PlayWave`, pass-through occlusion, ride-over probes in `AdvanceWave` |
| `Content/Data/WaveTable` | `land` row values |
| `Tools/Film/shots/wave_wall_test.json`, `Tools/Unreal/setup_wave_test_level.py` | test level + filming |

## Must-Preserve
- Monster hearing / noise judgment unchanged; `Multicast_Noise` unchanged.
- Rows without the new flags render exactly as before (radius = ruling radius, occlusion stops at bodies, floor dots at floor height).
- Dot pool / `NS_CL9_WaveDots` contract (`User.PoolSize`, `User.DotPositions/Colors/Sizes`).
- Wall stamp shape (spoke continuation, `StampRiseScale`, `StampLife`) from 2026-10-16.

## Execution Notes
- Recommended model: Claude Opus 5.5 or above for the ride-over geometry and look (judgment); a cheaper model is fine for field plumbing and docs.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the
  recommendation, surface it to the user and confirm before proceeding.
