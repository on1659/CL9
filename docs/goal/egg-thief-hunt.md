# goal: egg-thief-hunt

## One-line Goal
Monsters hear players from far away (Lethal Company radii, through walls) and walk to the sound. A chicken whose egg is picked up hunts the thief from any distance by a winding route, and switches aggro to any player it sees on the way.

## Background / Motivation
User, 2026-10-21: "소리를 듣고도 멀리서 따라오게 해야 돼. 닭은 자기 소유의 달걀이 몇 개 있는데, 그걸 누군가 먹으면 얼마나 멀리 떨어져 있든 바로 걔를 타게팅해서 찾으러 간다. 최단 경로로는 가지 않아. 하지만 가는 도중에 다른 유저를 만난다면 그 유저한테 어그로가 다시 가."
Before this change, walking was heard at 2 m and running at 3.9 m, so a monster one room away never reacted. Lethal Company: walk 17 m, sprint 22 m, shovel hit 17 m, all through walls (`docs/reference/lethal/lethal-control-feel.md` 6.3).

Answers given by the user (AskUserQuestion):
- "Eats" means **picks up** the egg.
- The owner is **random**, not the nearest chicken.
- Hearing radii are the **same as Lethal**.

## In-scope
- Rules table: `WalkNoiseRadius` 1700, `RunNoiseRadius` 2200, `HitNoiseRadius` 1700.
- Wave table rows WALK, RUN and HIT switch to a fixed radius (200, 390, 600) so the on-screen rings keep their old size.
- While a monster investigates a heard sound and still hears the player, it re-paths to the newest heard spot every chase-repath interval.
- Monster columns: `OwnedItemAlias`, `ThiefHuntSpeed`, `ThiefDetourStep`, `ThiefDetourRadius`. The chicken gets `egg`, 400, 1000 and 600.
- Night-placed loot is marked as site loot. The first player pickup of a site item reports it. A random living monster whose `OwnedItemAlias` matches then hunts the picker.
- Hunting behaviour:
  - The hunt is an Investigate sub-mode. Each leg aims at a random navigable point within `ThiefDetourRadius` of a spot `ThiefDetourStep` toward the thief. Inside `ThiefDetourRadius` the monster goes straight.
  - Sight of any player takes priority and starts a normal chase, which moves the aggro.
  - When that chase ends (Wander/Return), the hunt resumes.
  - The hunt ends when the monster catches the thief or the thief dies. An asleep owner wakes first (telegraph).
  - Bait and light lures are ignored while a thief is known.

## Out-of-scope
- Wall attenuation of sound, and habituation to repeated sound (Lethal Eyeless Dog). Not asked for.
- Showing ownership to players (egg markings, nests).
- Pre-assigning ownership at round start. The owner is rolled at pickup. Players see the same thing because the roll is uniform over living chickens.

## Acceptance Criteria
- [ ] PIE Lvl_Flat: a chicken facing away 12 m from the player stays in Wander while the player stands still.
- [ ] Same setup: the chicken leaves Wander within 1.5 s once the player walks.
- [ ] A chicken 20 m away leaves Wander within 1.5 s when the player picks up a site egg.
- [ ] That chicken reaches the thief: closest distance < 2 m, or Catch.
- [ ] Its route deviates more than 1.5 m from the straight line.
- [ ] Two-player PIE: with the client standing midway, the chicken chases the client. It gets within 2 m of the client and stays at least 3 m farther from the thief.
- [ ] Build passes. Monster table load checks pass.

## Related Files / Modules
| File | Role |
|------|------|
| Source/CL9/Monsters/CL9Monster.cpp | hunt, hearing re-path, loot report |
| Source/CL9/Data/CL9MonsterDef.h/.cpp | new columns and load checks |
| Source/CL9/Items/CL9Item.h | site-loot flag |
| Source/CL9/GameMode/CL9GameMode.cpp | marks night-placed loot |
| Source/CL9/Character/CL9Character.cpp | reports first pickup |
| Tools/Unreal/test_egg_thief_20261021.py | PIE acceptance test |
| docs/plan/data/monster/chicken.md, common.md, rules.md | plan docs |

## Must-Preserve
- The existing sense gauge, chase, catch and flee rules.
- Wave ring sizes.
- Host-only AI. Nothing new is replicated.

## Execution Notes
- Recommended model: Claude Opus 5.5 or better for the AI state changes. A cheaper model is fine for the docs and CSV rows.
- This document cannot enforce the model. The executing session's `/model` setting decides. If the session model is below the recommendation, tell the user and confirm before proceeding.
