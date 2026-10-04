export const meta = {
  name: 'giant-stage-panel',
  description: 'Read-only planning panel for docs/goal/giant-stage.md: four critical perspectives, then a cross-examination',
  phases: [
    { title: 'Panel', detail: 'four perspectives attack the draft design', model: 'opus' },
    { title: 'Cross-exam', detail: 'attack the panel consensus and settle decisions', model: 'opus' },
  ],
}

const CONTEXT = `
You are on a READ-ONLY planning panel for the CL9 project (Unreal Engine 5.8, repo root D:\\Work\\unreal\\CL9).
Do NOT edit, create or delete any file. Do NOT start builds, the editor, commandlets or PIE. Read code and docs only.

Spec under review: D:\\Work\\unreal\\CL9\\docs\\goal\\giant-stage.md (read it first).
Useful sources: D:\\Work\\unreal\\CL9\\docs\\systems\\stage.md, map.md, performance.md, monster.md, raiding.md in D:\\Work\\unreal\\CL9\\docs\\systems\\;
D:\\Work\\unreal\\CL9\\docs\\plan\\data\\site.md; D:\\Work\\unreal\\CL9\\docs\\goal\\outdoor-backdrop.md and farm-stage.md;
D:\\Work\\unreal\\CL9\\docs\\reference\\lethal\\lethal-map.md and lethal-monsters.md;
D:\\Work\\unreal\\CL9\\Source\\CL9\\World\\CL9SiteLayout.cpp/.h (generator; grid cap at line ~866), other files in Source\\CL9\\World and Source\\CL9\\Monsters;
D:\\Work\\unreal\\CL9\\Tools\\Unreal\\setup_stage.py, qa_farm_stage_pie.py, stages\\farm.json; Config\\DefaultEngine.ini (RecastNavMesh RuntimeGeneration=Dynamic).
Grep first and read only the ranges you need.

Decisions the USER has already made (2026-10-04) — do not relitigate, design around them:
1. Size: build 5x first (BaseModuleCount ~1,000 ground modules total); go to 10x (2,000) only if 5x passes its measurements.
2. Night length: 600 s (same as Lethal Company's day by the decompiled defaults). One night, not multi-trip.
3. Shape: DISTRICTS — several building clusters joined by outdoor yards/roads, not one huge building. Players cannot clear
   everything in one night; they choose which district to raid.
4. Monsters: ALL monsters stay active even when far from players (no sleeping/despawn by distance). The total count is to be
   chosen "sensibly, by comparison with Lethal Company" — not a blind 5x of the farm's ~25.
5. Target: 1-4 players, Steam listen server. Every machine generates the map from the seed.

Lead's DRAFT design (attack this):
- A giant stage = K districts (e.g. 5 x ~200 modules), each built by the EXISTING generator (CL9SiteLayout) as its own layout
  with its own sub-seed and world origin, so the 128-cell span cap and the generator's proven 200-module behaviour stay untouched.
- Districts are arranged around/along an outdoor yard; the truck pad sits in the yard; each district has its own yard_gate entrance
  facing the yard and its own delivery jars (SpecialCopies per district).
- Depth zones (Zones) are evaluated per district; districts further from the truck can be the more dangerous/valuable ones.
- Nav: dynamic navmesh over all districts + yard; open question whether tile count/build time needs navigation invokers or
  larger tiles. All monsters must be able to path while far from players (user decision 4), which constrains invoker use.
- Rendering: existing ISM-per-segment approach; per-district culling by distance/visibility; lamp count scales with districts.
- New MapTable row + level through setup_stage.py; QA through qa_farm_stage_pie.py and layout automation tests at the new size.
`

const SCHEMA = {
  type: 'object',
  properties: {
    facts: { type: 'array', items: { type: 'string' }, description: 'Verified facts from code/docs with file:line that the design must respect' },
    objections: { type: 'array', items: { type: 'object', properties: {
      target: { type: 'string' }, problem: { type: 'string' }, evidence: { type: 'string' }, alternative: { type: 'string' } },
      required: ['target', 'problem', 'alternative'] } },
    recommendations: { type: 'array', items: { type: 'string' } },
    unverified: { type: 'array', items: { type: 'string' }, description: 'Things you could not verify by reading; needs measurement' },
  },
  required: ['facts', 'objections', 'recommendations', 'unverified'],
}

const LENSES = [
  { key: 'generator', prompt: `Your lens: GENERATOR AND WORLD ASSEMBLY. How would K independent layouts actually be built, placed, replicated by seed and
consumed by the code that today assumes ONE layout per site (site map actor, spawn-point/depth computation, jars, entry module, NavBounds/floor sizing in
setup_stage.py, layout tests)? Find every single-layout assumption with file:line. Is "K layouts" really cheaper than raising the span cap with a district
mask inside one grid? Give the smallest change that yields districts joined by outdoor yards, and state what the data model (MapTable/site row columns) needs.` },
  { key: 'nav-ai', prompt: `Your lens: NAVIGATION AND MONSTER AI COST. Monsters use custom movement with NavMesh waypoints. With ~1,000 modules in districts plus
outdoor yards and ALL monsters always active, what happens to dynamic navmesh build time, tile count, memory and pathfinding cost? Evaluate: whole-bounds dynamic
build as today, per-district nav bounds volumes (no nav over empty ground), bigger tiles, navigation invokers (and whether invokers on monsters satisfy the
"always active" rule), static/prebuilt impossible because map is seeded. Check how monsters tick, perceive and replicate (file:line) and what a sensible
always-on budget is. Use docs/reference/lethal/lethal-monsters.md to propose a concrete total monster count for day 1 and the day curve by comparison with
Lethal Company (power caps / max enemy counts on its largest moons), stating the numbers you found.` },
  { key: 'render-perf', prompt: `Your lens: RENDERING, MEMORY AND FRAME BUDGET. Read docs/systems/performance.md for the canonical budget and the farm measurements.
At 5x instances (~17,000 ISM instances, ~3,000+ props, lamps by LampDensity) on a listen server host plus clients: what breaks first? Evaluate per-district
ISM components vs one component per segment kind, cull distances, lights (count, shadow casting, how lamps are created), collision/physics cost of far districts,
load/generation hitch at night start on every client, and how the acceptance thresholds should be stated (numbers taken from performance.md, not invented).
Say which checks qa_farm_stage_pie.py already makes and what it lacks for this stage.` },
  { key: 'play-simplify', prompt: `Your lens: PLAYABILITY AND SIMPLIFICATION (devil's advocate). With a 600 s night, walking ~4.6 m/s, and districts: compute concrete
walk-time budgets (truck -> gate -> deep zone -> jar/truck) and say how many districts and how big each can be for a district to be raidable in one night. Is 5 x 200
right, or more smaller districts, or unequal ones? Where do jars go, where does the truck go, how do yards avoid being dead walking time, how do items/orders scale
(see docs/plan/data/site.md, order.md) without redesigning the economy (out of scope)? Then argue for the SMALLEST implementation that satisfies the user's five
decisions, and name what in the draft is unnecessary.` },
]

phase('Panel')
const panel = (await parallel(LENSES.map(l => () =>
  agent(`${CONTEXT}\n${l.prompt}\n\nReturn objections and alternatives, not agreement. Cite file:line for every code claim.`,
    { label: `panel:${l.key}`, phase: 'Panel', schema: SCHEMA, model: 'opus' })
    .then(r => r && ({ lens: l.key, ...r }))))).filter(Boolean)

phase('Cross-exam')
const FINAL = {
  type: 'object',
  properties: {
    decisions: { type: 'array', items: { type: 'object', properties: {
      decision: { type: 'string' }, rationale: { type: 'string' }, rejected: { type: 'string' } },
      required: ['decision', 'rationale'] } },
    unresolved: { type: 'array', items: { type: 'object', properties: {
      question: { type: 'string' }, why_user_only: { type: 'string' }, options: { type: 'array', items: { type: 'string' } } },
      required: ['question', 'why_user_only'] }, description: 'Only issues that neither code, docs nor measurement can settle — the user must choose' },
    measure_first: { type: 'array', items: { type: 'string' }, description: 'Things to measure during implementation, with the fallback if the measurement fails' },
    acceptance: { type: 'array', items: { type: 'string' }, description: 'Concrete, checkable acceptance criteria with numbers and their source' },
    work_split: { type: 'array', items: { type: 'object', properties: {
      role: { type: 'string' }, work: { type: 'string' }, files: { type: 'array', items: { type: 'string' } }, order: { type: 'string' } },
      required: ['role', 'work'] }, description: 'Roles: map, gameplay, balance, docs, art, ui' },
    weak_points: { type: 'array', items: { type: 'string' } },
  },
  required: ['decisions', 'unresolved', 'measure_first', 'acceptance', 'work_split', 'weak_points'],
}
const final = await agent(`${CONTEXT}
You are the CROSS-EXAMINER. Four panelists returned the reports below. Attack their consensus: find where they contradict each other, where a claim has no
file:line evidence (verify it yourself by reading the code), and where a recommendation violates one of the user's five decisions. Then settle a single coherent
design: one decision per row of the "What breaks at this size" table in the spec, plus district count/size, truck and jar placement, and the monster total.
Prefer the smallest change that satisfies the user's decisions. Put an issue in "unresolved" ONLY if the user alone can choose it.

PANEL REPORTS (JSON):
${JSON.stringify(panel)}`, { label: 'cross-exam', phase: 'Cross-exam', schema: FINAL, model: 'opus', effort: 'high' })

return { panel, final }
