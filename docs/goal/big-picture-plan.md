# goal: big-picture-plan

**Status**: spec + delivered as planning documents (2026-10-20). Track: HEAVY (mechanism design), panel replaced by the existing draft/integration rounds plus one cross-examination pass (see Results).

## One-line Goal
Write the "first session" play scenario (three friends, henhouse, rounds 1–3 plus the first company visit) in the player's own eyes, then derive from it the ordered list of what must be built — one big-picture document that the 15k lines of `docs/plan/draft/` never produced.

## Background / Motivation
- The autonomous planning loop (2026-10-18) produced 23 monster cards, 7 destinations, tools, dishes, round timeline, controls, UI, sound and art drafts (`C:\Work\CL9\docs\plan\draft\`, four integration rounds). The user cannot hold that in one view: "큰 그림을 그리자 … 시나리오를 먼저 써. 그리고 그 시나리오를 실현하기 위해서 뭘 만들어야 되는지 먼저 구상해보자" (2026-10-20).
- Decisions already settled that the plan must respect: first production batch = henhouse + 10 species (decision #48), one core behaviour per species (2026-10-20, `docs/plan/data/monster/common.md` "핵심 행동"), all five house owners get a monster form (#49), hidden-role screen name "머큐리" (temporary, #46), rat = swarm herding (#45), no notes for maskless species (#52).
- Today's probe answer: held meat stays a club — make it read as one (not: tools only / weapon set).

## In-scope
- `C:\Work\CL9\docs\plan\scenario.md` — first-session scenario in Korean: title → lobby → round 1 (darkness, eggs, chickens) → round 2 (sound, goose, yard wakes) → round 3 (watching, raccoon, hidden role) → company. Each beat marks what exists today and what is missing (M-numbers).
- `C:\Work\CL9\docs\plan\roadmap.md` — the big picture in Korean: pillars, monster design frame (card, rule types, "ten verbs"), first 10 species and later 13, three new candidate patterns, combat readability decision, item and dish lists, phases 1–4 with build bundles (M1…), assumptions table for the open A-questions, remaining user decisions, success criteria.
- Index pointers in `C:\Work\CL9\docs\plan\README.md` and a one-line pointer at the head of `build-order.md`.

## Out-of-scope
- Any code, table asset, or art change. No new decisions are made for the user; defaults are marked as assumptions.
- Rewriting the drafts or the 14 A-questions; they are referenced, not duplicated.

## Acceptance Criteria
- [x] Scenario covers three rounds plus company with concrete current values (245 s round, blackout 125–185 s, egg 10–15, fried egg 19–25, quota 600 on day 3) and names the player-facing lesson of each round.
- [x] Every scenario beat that the game cannot do today points to an M-number in the roadmap.
- [x] Roadmap phase 1 is exactly the #48 batch (henhouse + 10 species) and lists an exit condition ("is it fun" play test).
- [x] Monster section follows "one core behaviour per species" and gives each of the 10 species a distinct player verb.
- [x] Assumptions table lists each default taken on an open question and where to flip it.
- [x] Cross-examination pass found no contradiction with `docs/decisions.md` / `monster/common.md` that remains unfixed.

## Related Files / Modules
| File | Role |
|------|------|
| `C:\Work\CL9\docs\plan\scenario.md` | new — first-session scenario |
| `C:\Work\CL9\docs\plan\roadmap.md` | new — big picture and build bundles |
| `C:\Work\CL9\docs\plan\README.md` | index rows |
| `C:\Work\CL9\docs\plan\build-order.md` | pointer line to roadmap |
| `C:\Work\CL9\docs\plan\draft\_integration-4.md` | source of A1–A14 and recommendations |
| `C:\Work\CL9\docs\plan\data\monster\common.md` | "핵심 행동" table (2026-10-20) — binding |
| `C:\Work\CL9\docs\decisions.md` | #45–#70 — binding |

## Must-Preserve
- `build-order.md` stays the canonical work-order document; the roadmap groups work and points back, it does not replace step numbering.
- Terms: "플레이어" (not 농부), "판" (not 밤), "예고" (not 텔), "몰래 역할" with "머큐리" only as the temporary screen name.
- Values stay in tables; the documents quote table/column names, never propose hard-coded numbers.

## Execution Notes
- Recommended model: Claude Fable 5.1 for the scenario and roadmap (judgment-heavy synthesis across 15k lines of drafts and 70 decisions). Sonnet acceptable for the index edits.
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.

## Results (2026-10-20)
- Written: `scenario.md`, `roadmap.md`, index rows, build-order pointer.
- Cross-examination (read-only opus subagent against decisions #45–#70, `monster/common.md`, `site.md`, `rules.md`, `session.md`): 8 confirmed + 4 to-verify findings, all resolved — egg count 17–21; current `MidSpawns` already mixes pig/sheep/raccoon from day 1 so the scenario's day ladder is M14's job, not current state; model-before-rule order per the 2026-10-20 decision (M12 now depends on M13); goose "cries when hit" was a draft side-rule and was removed; decision numbers #49/#50 fixed; spectating exists (only camera choice is missing); one starting flashlight exists in the level; gate goose and locking other destinations moved from "consequence of #67" to explicit assumptions → 미결 6·7; A13 read narrowly. List in roadmap "근거와 한계".
- Remaining user decisions: roadmap §7 (7 items). No code or asset was touched.
