# goal: doc-audit-closeout

## One-line Goal
Finish the documentation audit started on 2026-09-09 — verify the reference-doc claims
that were never checked, correct the one stale C++ default the audit exposed, and land
the whole thing in git so `HEAD` compiles again.

## Background / Motivation
A staleness audit ran across `docs/` and closed three mechanical checks (symbols, table
columns, internal links). Two comparisons were never done, and one defect was found but
left unfixed:

1. **Reference docs vs. our current state.** `docs/reference/` contains 158 lines that
   mention CL9. 38 of them assert something about CL9's *current* state and are therefore
   falsifiable. Only the 18 inside `takeaways.md` were checked; **20 assertions across
   `README.md`, `repo/*.md` and `lethal/*.md` were never verified.**
2. **Prose-level consistency between `docs/plan/` and `docs/systems/`.** Numeric
   cross-checking found no contradiction, but prose claims were only spot-checked.
3. **`UCL9CharacterDef::StaminaMinResume` defaults to `0.25f`** while `decisions.md:504`
   records a user decision to raise it to `1.0` ("lock until full"). The decision was
   applied to the data asset only ("코드 변경 없음"), so the header default is now the
   pre-decision value. A newly created character row would silently get the old
   behaviour.

Separately, the working tree holds two finished-but-uncommitted bodies of work, and the
UI session already committed `4913fe4`, which calls into the uncommitted buff code — so
**`HEAD` currently does not compile.**

## In-scope
- Verify all 20 unchecked current-state assertions in `docs/reference/` outside
  `takeaways.md`; annotate stale ones with dated corrections.
- Targeted prose pass between `docs/plan/` and `docs/systems/`: confirm every "정본은
  plan이다" pointer resolves to a real file, and report which systems docs carry no
  pointer at all.
- Change `UCL9CharacterDef::StaminaMinResume` from `0.25f` to `1.f`, update its comment to
  match the decision, and run a full build.
- Commit in two separate commits, each with explicit paths:
  1. the buff table work (unblocks `HEAD`),
  2. the documentation audit.

## Out-of-scope
- **Adding "정본" pointers to the systems docs that lack them.** That is a convention gap,
  not staleness; it would touch 12 files for no correctness gain.
- **Editing `docs/decisions.md`.** Its own preamble makes it append-only: past entries are
  never rewritten, reversals are recorded as new entries.
- **Map-domain docs** (`systems/map.md`, `plan/data/module.md`, `plan/data/site.md`,
  `systems/asset-swap.md`) — owned by the map session, which has already updated them.
- Committing other sessions' files. The map session's changes stay untouched.
- Any gameplay feature work (4g and the buff candidates from `takeaways.md` are separate).

## Acceptance Criteria
- [ ] Every current-state assertion in `docs/reference/` is either verified true or
      carries a dated correction; none is silently wrong.
- [ ] Corrections touch only the CL9-facing sentence — the R.E.P.O./Lethal analysis itself
      is left as written.
- [ ] Every `docs/systems/*.md` "정본" pointer resolves to an existing file.
- [ ] `StaminaMinResume` defaults to `1.f` and its comment states the lock-until-full rule.
- [ ] `CL9Editor Win64 Development` builds with exit code 0.
- [ ] `HEAD` compiles: the buff commit contains `CL9BuffDef.*`, the registry change, the
      `EatBuffs` column, `FCL9ActiveEffect::BuffAlias`, the two getters, and the three assets.
- [ ] The three mechanical scans (symbols / columns both ways / links) still come back clean.
- [ ] All touched files keep LF line endings.

## Related Files / Modules
| File | Role |
|------|------|
| `docs/reference/repo/*.md` · `lethal/*.md` · `README.md` | The 20 unverified assertions live here |
| `docs/reference/takeaways.md` | Already corrected (sections 1 and 5) — do not redo |
| `Source/CL9/Data/CL9CharacterDef.h` | `StaminaMinResume` default and comment |
| `docs/decisions.md` | Authority for the 1.0 decision — read only, never edit |
| `Content/Data/BuffTable.uasset` · `CL9DefList` · `ItemTable` | Buff assets to commit |

## Must-Preserve
- `docs/decisions.md` stays append-only.
- The map session's uncommitted files stay out of every commit — always `git add <path>`,
  never `-a` or `-A`.
- Changing the C++ default must not change any data asset. `CharacterTable` already
  carries the intended value; the header default only affects newly created rows.
- Line endings stay LF. Writing docs from Python must pass `newline=''` — the default
  translates LF to CRLF on Windows and rewrites whole files.

## Execution Notes
- Recommended model: the strongest current Claude model for judging which reference-doc
  sentences are genuinely stale versus deliberately historical — that call needs the
  decision log's intent, not string matching. A cheaper model is acceptable for the
  mechanical rescans and the line-ending check.
- This document cannot enforce the model — the executing session's `/model` setting
  decides. If the session model is below the recommendation, surface it to the user and
  confirm before proceeding.

## Existing Integration Contract
- Three sessions share one editor and one working tree. Before any build: notify peers,
  confirm zero dirty packages, `taskkill` the editor, wait 5 seconds, then run the build
  as a detached process. Editor off, the build takes about 45 seconds.
- `seg kind 'ceiling' has no usable row` is an intended error (the ceiling debug value is
  deliberately off per user instruction). Do not treat it as a regression.
