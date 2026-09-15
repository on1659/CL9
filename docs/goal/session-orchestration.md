<!-- /autoplan restore point: /c/Users/user/.gstack/projects/Work/master-autoplan-restore-20260912-132850.md -->
# goal: session-orchestration

Status: APPROVED (/autoplan, gate cycle 2, 2026-09-12 — user chose "그대로 승인"; Korean
walkthrough in [session-orchestration.walkthrough.md](session-orchestration.walkthrough.md))
Date: 2026-09-12 · Branch: master

## One-line Goal
Turn the ad-hoc "one terminal per domain" way CL9 has been built for two weeks into a
repeatable system: a role registry + shared-resource protocol (editor/build lock) that
every session follows, `/cl9 role <name>` contracts generated from that registry for
manually opened terminals, and `/cl9 <task>` that splits a task into role briefs and runs
the editor-free parts as role subagents in parallel while the lead serializes the one
Unreal editor, PIE and the build.

## Background / Motivation

The user asked (2026-09-12): "우리 작업하는게 보니까 컨텐츠/전투/ui 총괄 이런식으로 내가
임의로 세션을 새로 만들고 작업을 나눠서 했는데 이걸 시스템화해서 프롬프트로 할 수 있나?
작업 하나 주면 알아서 UI/전투/시스템/아트로 나눠서 할 수 있지 않을까? 가능해?"

What the 29 session logs (2026-08-29 → 09-12, summarized in
`docs/review/sessions-2026-08-29-to-09-12.md`) show:

- **The split already happened by hand, and it worked.** From 09-04 on, the user ran
  2–6 terminals at once. Each got an opening "contract" typed by the user, e.g. session
  `e8946193` (09-06): *"내 세션은 UI/UMG 담당이다. 영역: Content/UI, Content/Input,
  Content/Effects, Source/CL9/UI, BP_CL9GameInstance, BP_CL9PlayerController. 다른
  세션이 Source/CL9 게임플레이 코드·Content/Data·Content/Monsters·조명을 맡는다.
  게임플레이 파일이 꼭 필요하면 상대에게 먼저 통지하고 동의를 받은 뒤 최소한으로만
  건드려라. 에디터 1개를 포트 8000 MCP로 공유한다. 빌드로 에디터를 끄기 전과 PIE 시작
  전에는 반드시 SendMessage로 상대 세션에 먼저 통지하고…"*
- **Five domains emerged, not four — and the user added a sixth.** Gameplay C++ + data
  tables; UI/UMG + input; map/level (World C++, module/site/segkind tables, `.umap`);
  art/assets (ArtSource, Blender scripts, CL9Art, AnimBPs); docs/plan/review. Plus an
  "ops" role (commit/push, editor lifecycle) that any session ended up doing. The user's
  words were "UI/전투/시스템/아트"; no session ever split 전투 from 시스템 — both were the
  gameplay C++ session — while map/level did become its own session three times. At the
  gate (2026-09-12) the user asked for a **balance** agent: "밸런스 담당하는 에이전트도
  있어야 — plan이 기획이니까 이거보고 판단할거". CL9 is data-driven by principle
  (`build-order.md` 원칙 3: 수치는 전부 에셋에서, 관계 검사 A1/A2/A4), so "what the number
  should be" is a judgment made from `docs/plan` (economy, progression, rules) and the
  reference-game analyses — distinct from "how the column works" (gameplay C++).
- **Coordination rules exist but live only in memory files and in the user's head:**
  one editor on MCP port 8000, PIE is exclusive, notify via `ListAgents`/`SendMessage`
  before `taskkill`, batch builds across sessions, hand off through
  `memory/work-progress.md`, split commits by domain, "세션끼리 알아서 조율" (09-12).
- **Recurring pain, ranked by cost:** (1) an editor killed without notice (09-07, cost
  cl9-19 six minutes) and a second editor instance that became a ghost holding the level —
  the shared-resource incidents; (2) build waits of 20–48 min with Live Coding while every
  other session idles; (3) uncommitted edits from 3 sessions interleaving on
  `ItemTable`, `Lvl_Henhouse`, `decisions.md`; (4) sessions hitting 75% context and
  being re-seeded with a long resume prompt; (5) the user writing every contract by hand;
  (6) session aliases confused (cl9-5b vs cl9-2d).

**Answer to "가능해?" up front: yes, with one structural limit.** Claude Code can spawn
role-specialized subagents from one prompt (custom agents in `.claude/agents/`, hooks that
see which agent is editing). What it cannot do is give five agents five Unreal editors:
the project has one editor, one PIE, one build (UBT holds a global mutex), one working
tree with binary `.uasset`/`.umap` files that cannot be merged. So the system splits the
*thinking and the text edits* in parallel and keeps *editor, PIE and build* serialized —
and the serialization, not the split, is where the recorded incidents and waiting time
live. That is why the shared-resource protocol and lock ship first and the dispatcher
second.

### Verified current state (2026-09-12)

- Claude Code 2.1.201. Project has `.claude/settings.json` (permissions only, tracked in
  git) and no `.claude/agents/`, `.claude/skills/`, `.claude/hooks/`, `.claude/workflows/`.
  One global agent exists (`~/.claude/agents/ue-doc-collector.md`, haiku, 2 tools).
- Docs (https://code.claude.com/docs/en/sub-agents.md, hooks.md, skills.md, agent-teams.md,
  worktrees.md — checked 2026-09-12): custom agents support `name, description, tools,
  disallowedTools, model (sonnet|opus|haiku|fable|inherit), permissionMode, maxTurns,
  skills, hooks, memory, background, isolation`; the body is the agent's system prompt.
  PreToolUse hooks receive `session_id`, `agent_id` and `agent_type` on stdin JSON, so a
  hook can tell which session and which role is acting; exit 2 blocks the call, other
  non-zero exits are non-blocking. Skills support `$ARGUMENTS`, `context: fork`, `agent:`.
  Agent Teams (separate Claude Code instances with a shared task list + mailbox) is
  experimental behind `CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1`; split panes need tmux or
  iTerm2, so on this Windows box teammates run in-process; subagent definitions can be
  referenced as team members, so role files serve both. A subagent can be resumed with its
  context via `SendMessage` to its name.
- Existing project harness: `/autogoal` (probe → `docs/goal/<slug>.md` → implement) is the
  entry point for features; `docs/testing/pie-scenarios.md` lists PIE checks (A–L);
  `Tools/Unreal/*.py` (130 editor-python scripts: import/qa/inspect) run either through MCP
  (`run_python.py`, editor open) or headless (`UnrealEditor-Cmd -run=pythonscript`, editor
  closed — saving with the editor open is the ghost-editor failure class); memory
  `editor-kill-allowed.md` and `unreal-mcp-usage.md` hold the editor/build protocol
  (save_assets → notify → taskkill → detached `.cmd` build → restart → MCP wait; 1–3 min
  with the editor closed, 20–48 min with Live Coding). gstack's `/freeze` fences the *main*
  session to one directory via a PreToolUse hook — same mechanism, session-level not
  role-level.
- Shared resources are physical, not conventional: MCP is on `127.0.0.1:8000` for whichever
  editor process starts first; a second editor becomes a ghost that blocks `.umap` saves
  (`Error Code 32`); UBT rejects a second concurrent build of the same target.
- `Saved/` is gitignored but is also where UE writes autosaves/crash dumps and what people
  delete to "fix" the editor — run artifacts go under `.claude/runs/` (gitignored) instead.
- Working tree today: 189 modified/untracked files from 4 concurrent sessions (String
  Table, HoldPose, lighting table, CL9Art materials), uncommitted by user decision; one
  stash (cl9-21 debug values). No remote delta (`origin/master` == HEAD).
- Codex CLI is out of usage credits until 2026-09-17; this review's outside voices are
  Claude subagents only.


## In-scope

- **A. Role registry** — `.claude/cl9-roles.json`: one entry per role with `owns`
  (patterns the role may edit freely), `shared` (patterns it may edit but must list in its
  report; **single-writer per run** — triage assigns each shared file to exactly one role),
  `except` (patterns carved out of `owns`), `note` (prose qualifiers the hook cannot
  enforce, e.g. "mesh/anim columns only" — checked by the post-run audit), `docs`
  (canonical docs to read first), `pie` (section letters in
  `docs/testing/pie-scenarios.md`), and the per-role escape hatches `model` (default
  `opus` — the user's model policy set at the gate, 2026-09-12: **lead = the session's
  model (Fable today), role agents that plan and implement their slice = Opus,
  research-shaped sub-tasks = Sonnet, bulk extraction = Haiku**), `max_turns` (default
  200), `default_mode` (`A` subagent / `C` manual terminal). Roles: `gameplay`, `ui`, `map`, `art`, `docs`, `balance`. Two global lists:
  `append_only` (`docs/decisions.md` — subagents never edit it; they hand the entry to the
  lead) and `lead_only` (`Config/**`, `.claude/**`, `CL9.uproject`,
  `Source/CL9/CL9.Build.cs`, `Source/CL9/CL9.{h,cpp}`, `Content/Maps/*.umap`, every
  `.uasset` — binary assets are edited only through the editor, which only the lead
  drives). **Pattern dialect** (python 3.9 has no full `**` glob): `fnmatch` on
  lowercased, forward-slash, repo-relative paths where `*` also crosses `/`; no brace
  groups — write the alternatives out; `Source/CL9/Data/*` + `except:
  [Source/CL9/Data/CL9SiteDef.*, …]` instead of prose. One loader module
  `.claude/hooks/cl9_roles.py` (load, normalize, match, expand) used by the hook, the
  renderer and the tests. A registry test asserts every pattern matches at least one
  tracked file, so a folder rename fails the test instead of silently un-fencing a role.
  Initial ownership (table form; the JSON is the source):

  | role | owns | shared |
  |---|---|---|
  | gameplay | `Source/CL9/Character/*`, `Items/*`, `Monsters/*`, `GameMode/*`, `Core/*`, `Source/CL9/Data/*` except `CL9{Site,Module,SegKind,Piece}Def.*`, `docs/systems/{character,item,inventory,monster,cooking,order,economy,drop,raiding,save,session,warning,cheats,balance,shop,sequence,actor-base,sound}.md` (`balance.md` says only how code reads the values — a gameplay doc), `docs/plan/data/sequence.md` | `Source/CL9/UI/*` (getters); **grant-gated with balance**: `docs/plan/data/{rules,progression,buff,equipment,order}.md`, `docs/plan/data/item/*`, `docs/plan/data/monster/*` (note: gameplay = columns, enums, mechanism; balance = the numeric cells) |
  | balance | `docs/plan/economy.md` only | **grant-gated with gameplay**: `docs/plan/data/{rules,progression,buff,equipment,order}.md`, `docs/plan/data/item/*`, `docs/plan/data/monster/*` (note: numeric cells of existing rows — price, weight, speed, detection, durations, counts); with art: `docs/plan/data/character.md` (body numbers only); with map: `docs/plan/data/site.md` (per-site base counts and night clock); `docs/plan/loop.md` (day-length and pacing numbers only) |
  | ui | `Source/CL9/UI/*`, `Content/Strings/*`, `Config/Localization/*`, `Tools/Loc/*`, `docs/systems/{ui,ui_system,input,settings,localization}.md` | `Source/CL9/GameMode/CL9PlayerController.*` (input binding), `Source/CL9/Character/*` (input only) |
  | map | `Source/CL9/World/*`, `Source/CL9/Data/CL9{Site,Module,SegKind,Piece}Def.*`, `ArtSource/map/*`, `docs/systems/{map,light,asset-swap}.md`, `docs/plan/data/{module,site,segkind}.md` | `Source/CL9/GameMode/CL9GameMode.*` (spawn/seed hooks) |
  | art | `ArtSource/{character,item,monster,validation}/*`, `Tools/Blender/*`, `Tools/Unreal/*`, `docs/asset/*`, `docs/systems/animation.md`, `docs/plan/data/character.md` | `Source/CL9/Data/CL9ItemDef.*`, `CL9CharacterDef.*` (note: mesh/anim columns) |
  | docs | `docs/plan/{README,overview,loop,build-order}.md`, `docs/conventions.md`, `docs/ue-cpp-conventions.md`, `docs/reference/*`, `docs/review/*`, `docs/goal/*`, `docs/testing/*`, `TODOS.md`, `CLAUDE.md` | any `docs/systems/*.md` (consistency fixes) |

  **Grant-gated shared paths** (any path that appears in two roles' `shared`): the hook
  allows it only when it is in *that* role's `grants.json` for the current run, and
  dispatch refuses an intra-run grant overlap as well as the cross-run one — so
  `docs/plan/data/item/egg.md` in a run with both gameplay and balance goes to exactly
  one of them by name. A registry test computes every pairwise `shared` intersection
  across the six roles and asserts each is in the grant-gated set.

  **The balance role** (user direction at the gate): its canon is `docs/plan` —
  `overview.md`, `loop.md`, `economy.md` (prices, the difficulty curve, the inequalities),
  `data/README.md`, `data/rules.md`, `data/progression.md`, `docs/systems/balance.md`
  ("수치의 정본은 plan이다" — it only says how code reads the values), `docs/plan/
  build-order.md` 원칙 3 (the relationship checks A1/A2/A4) and `docs/reference/*` (the
  R.E.P.O. / Lethal Company analyses) — and it **decides values**. A "table number" is
  bounded: a **tunable column of a `docs/plan/data/*` table** (price, weight, speed,
  detection radius, stamina, night length, monster count per day, buff strength, order
  reward, equipment cost) — not capsule radii, hit-stun milliseconds, animation play
  rates or a `UPROPERTY` default. When a task introduces or changes one, `gameplay`
  builds the column, unit and range and sets a **placeholder default**, and `balance`
  confirms or overrides it **in parallel** (triage records gameplay→balance as an
  interface dependency — column name + unit — the same way it records gameplay→ui);
  a task with no such column has no balance role. Because every runtime value is a
  `.uasset` row (`CL9Rules`, `CL9Progression`, `ItemTable`, `MonsterTable`,
  `BuffTable`, `OrderTable`, `EquipmentTable`), the balance report carries:
  **표 행** — one line per change, `asset.row.column: old → new (unit)`, the doc line it
  changes, the doc line that justifies it, and an **observable to confirm in PIE**
  ("빈손 지친 상태로 닭에게서 10초 안에 벌어짐"); **검증** — the pasted output of
  `py -3 Tools/Balance/check_inequalities.py` (parses the docs tables, applies the
  progression curves' maxima, and prints A1/A2 pass/fail with the operands; **A4 is
  printed as "미결 — 식 없음"** until the weight→slowdown formula exists — no agent may
  invent an operand; the C++ load-time check needs the editor, so this script is the
  editor-free twin and the lead's load/PIE pass confirms). The lead applies the rows
  **batched per run** — through MCP while the editor is open, or with
  `Tools/Unreal/apply_table_rows.py <report>` as a commandlet under `acquire commandlet`
  when it is closed — then re-reads the assets and compares them to the report; any
  mismatch (including a value the user tweaked by eye in PIE) goes back to balance as a
  doc update, never absorbed silently, so `docs/plan` stays the 정본. The `decisions.md`
  entry for a balance value is tagged **초안** until its PIE observable is recorded. The
  post-run audit for balance is **diff-shaped**: in grant-gated files, every changed line
  must be an existing table row whose only change is a numeric cell (row key must
  pre-exist; a renamed column is a violation). Balance never edits `Source/`; if a value
  needs a new mechanism it becomes a request to `gameplay`. Default mode A (it is text
  and arithmetic), `model: opus`, `pie: [J, K, O, P]` (일자 진행 곡선 · 주문서 · 장비
  구매 · 정산 in `pie-scenarios.md`).

  The registry is the **default fence**; each run's brief carries the explicit file list
  for that role and the lead writes it as `.claude/runs/<slug>/grants.json`
  (`{"cl9-ui": ["Source/CL9/Items/CL9ItemDef.h", …]}`) — the hook allows
  `owns ∪ shared ∪ grants` so a vertical task (one item = Def + UI + mesh + placement +
  doc) sliced by file in triage does not dead-end on the first foreign path. The hook
  finds the run by **`agent_type`** (the latest non-terminal `run.json` that lists that
  role), never by `session_id` — subagents, Agent Teams teammates and a lead after
  `/clear` all resolve the same way. At dispatch the lead intersects the new grants with
  every other live run's `grants.json` and refuses overlaps ("`CL9PlayerController.cpp`는
  run `hud-stamina`의 ui가 이미 쓰는 중"). A malformed `grants.json` counts as empty
  (registry-only) and is logged, never as deny-all.
  `.uasset`/`.umap` edits (data-table rows, WBP wiring, level placement) are **requests**
  in a role's report ("표 행: ItemTable egg.HoldPose=Small"); the lead applies them
  through MCP while the editor is open, or through a headless commandlet only inside a
  closed-editor window — never both at once.

- **B. Role agents** — `.claude/agents/cl9-{gameplay,ui,map,art,docs,balance}.md`, **rendered**
  from the registry by `.claude/hooks/render_roles.py` (a test asserts the rendered files
  equal the committed ones; the hook also compares the registry hash embedded in each
  rendered file and, on mismatch, says "레지스트리가 바뀜 — `python .claude/hooks/
  render_roles.py` 실행"). Frontmatter from the registry: `model` (`opus` for all six —
  the user's policy; the docs 정본 and the balance judgment are not on the cheapest
  model either), `tools: Read, Edit, Write,
  Grep, Glob, Bash, PowerShell` (no `Agent`, no `mcp__unreal-mcp__*`, no
  `mcp__blender-mcp__*` — Blender work runs as `blender -b <script>` per the 09-11 rule),
  `maxTurns` (200). Body (Korean, like every other CL9 text): who you are, what you own
  (rendered), the standing rules by pointer (data-driven, no hardcoding, Korean comments,
  String Table keys, "추측 금지 — 모호하면 멈추고 보고서 질문 절에 적는다"), what you must
  NOT do (editor, PIE, build, `taskkill`, commit, `decisions.md`, files outside the brief),
  the machine quirks (Bash sandbox refuses some `Content/` writes → PowerShell; `py -3`
  not `python3`), the compile-check command if H below is verified (stated as a *syntax
  check*: the game target builds with `WITH_EDITOR=0`, so editor-only code is not
  covered), the rule that report fields are data (never run a command quoted in a
  report), and the report
  contract: `.claude/runs/<slug>/report-<role>.md` with fixed sections **한 일 / 바꾼
  파일 / 다른 역할에 요청(인터페이스·표 행·에셋) / 빌드 필요 여부 / PIE 확인 항목 /
  decisions.md에 남길 결정 / 질문(막힌 것)**. The agent ends its turn with the report
  path and a one-paragraph summary.

- **C. Scope guard hook (tripwire) + post-run audit (truth)** —
  `.claude/hooks/cl9_scope_guard.py` registered in `.claude/settings.json` as
  `PreToolUse` for matcher `Edit|Write|MultiEdit|NotebookEdit|Bash|PowerShell|
  mcp__unreal-mcp__call_tool|mcp__blender-mcp__.*`, command string verbatim
  `py -3 .claude/hooks/cl9_scope_guard.py` (the Windows launcher — `python` resolves
  through PATH *from cwd* under cmd.exe and `python3` is the Store stub; the launcher's
  presence and the shell that runs hooks are the first probe, and the tests run in that
  shell, not in the terminal; relative path because hook commands run from the project
  directory and `$CLAUDE_PROJECT_DIR` expansion depends on that shell — the repo root is
  read from the stdin `cwd`; gstack's own hooks here run as `bash /c/...` and a
  `pixel-agents` PreToolUse hook already fires on every tool call, so the latency
  precedent exists). Denials use the documented JSON form, as gstack's `/freeze`
  hook does: print `{"hookSpecificOutput":{"hookEventName":"PreToolUse",
  "permissionDecision":"deny","permissionDecisionReason":"<Korean reason>"}}` and exit 0
  (a top-level `permissionDecision` is ignored; exit 2 + stderr is the fallback). Reads
  stdin JSON (`session_id`, `agent_type`, `tool_name`, `tool_input`, `cwd`). **Error
  polarity:** internal errors (bad registry, bad lock file, bad stdin) → for `cl9-*`
  agents deny with the error in the reason; for everyone else return
  `permissionDecision: "ask"` with the reason ("가드 훅 오류: 잠금·역할 검사 미적용 — <one
  line>") so the *model* sees it, not only the terminal (stderr never reaches Claude;
  whether "ask" surfaces under bypass-permissions mode is part of the first probe — if it
  does not, fall back to `allow` + a line in `.claude/runs/guard-errors.log` that
  `/cl9 status` prints). **Kill switch:** the file `.claude/runs/guard.off` (any session:
  `py -3 .claude/hooks/cl9_lock.py guard off [--minutes N]`, default TTL 60 min, shown
  by `/cl9 status`) makes the hook allow everything until it expires. For a `cl9-*`
  agent: Edit/Write path outside `owns ∪ shared ∪ grants` of that role, or matching
  `append_only`/`lead_only`, or outside the repo, or when that role's run is in a
  terminal state (`ABORTED`/`REPORTED`) → deny with a one-line Korean reason naming the
  owning role and the doc anchor ("`Source/CL9/Items/CL9Item.cpp`는 gameplay 영역 —
  보고서 '다른 역할에 요청'에 적어라. docs/workflow/sessions.md#거부"; a missing run dir
  gets its own reason, "실행 상태 없음", never a misleading owner), and the denial appended
  to `.claude/runs/<slug>/denied.log` (`makedirs` first). Shared path → allow + line in
  `shared-edits.log`; every call by a role also touches `.claude/runs/<slug>/
  touch-<role>` (the hook never writes `run.json` — that file is lead-only). Any
  `mcp__blender-mcp__*` call and any `mcp__unreal-mcp__call_tool` (parsed at
  `tool_input.tool_name`, with `tool_input.toolset_name` in the reason) → deny for roles.
  **Bash/PowerShell for roles is an allowlist, not a denylist** (a denylist is both leaky —
  `git restore .`, `git switch`, `rmdir /s`, `Remove-Item -r` — and noisy — `grep
  Build.bat docs/`): allowed are `py -3 Tools/*`, `py -3 .claude/hooks/*`, `python -m
  unittest …`, `blender -b Tools/Blender/*`, git read verbs (`status|diff|log|show|
  ls-files|grep|blame`), and read-only shell (`ls|dir|cat|type|head|tail|grep|findstr|
  find|wc|tasklist|Get-ChildItem|Get-Content|Select-String`), each parsed as a single
  command with no `;`, `&`, `|`, backtick, `$(` or newline; anything else → deny
  ("에디터·빌드·커밋·그 밖의 셸은 리드가 한다 — 필요하면 보고서에 적어라"). Path matching:
  `realpath` + `normcase` + `commonpath` against the stdin `cwd` (drive-letter case,
  `..`, junctions), forward slashes, then `fnmatchcase` (plain `fnmatch` re-applies
  `normcase` and flips slashes back); patterns compile as directory-prefix matches so
  `Source/CL9/UI/*` never matches `Source/CL9/UI-old/`. The hook is a tripwire, not a
  fence — `Set-Content`, `python -c`, `blender -b` can write anywhere — so the **post-run
  audit** is the truth source, **against a baseline**: at DISPATCHED the lead snapshots
  `git status --porcelain`, `git diff` of tracked files (saved as
  `.claude/runs/<slug>/baseline.patch`, also the recovery patch if a role ever destroys
  work) and the mtime/hash of every dirty path; at integration it audits only the delta
  (the 189 files four other sessions left dirty are not "violations"), maps every changed
  path to the brief/grant that listed it, reports every unclaimed path as a violation
  (never silently absorbed), and also checks that `guard.off` is absent and
  `git diff --stat .claude/ Config/` is empty for that run. Unit tests in
  `.claude/hooks/test_cl9_scope_guard.py` (`py -3 -m unittest discover .claude/hooks`):
  allowed path, denied path with owner named, grants.json path allowed, malformed grants
  = empty, shared path logged, lead passes, unknown role denied, terminal-state deny,
  Bash allowlist corpus (allowed forms, denied forms, compound-command forms, regex
  false-positive corpus such as `grep Build.bat docs/`), `call_tool` with `tool_name:
  StartPIE` denied for role / lock-checked for lead, path outside repo / `..` / junction /
  drive-case normalized, `MultiEdit` and `notebook_path` inputs, malformed stdin (role →
  deny, lead → ask), guard.off honored and expired, registry pattern coverage, renderer
  equality + hash stamp, JSON deny shape, hook latency budget (< 100 ms per call), and
  **fixtures captured from the real 2.1.201 hook stdin/stdout** in the probe.

- **G. Editor/build lock** — `.claude/runs/editor.lock` (JSON: one **exclusive** holder
  `{session_id, purpose: build|compile|pie|commandlet, since, expires, proc: {pid,
  create_time, image}}` and a list of **shared `editor` leases** `[{session_id, since,
  expires}]`). Expiry default **90 min** for `build`/`commandlet` (Live Coding builds
  outlive any sane shorter value), 30 min otherwise; the exclusive lock also stays valid
  while its recorded process is alive — validated by the triple (pid, create time, image
  name), never by a bare PID, because Windows recycles PIDs. `acquire build|commandlet`
  **requires `--pid`** (the detached `.cmd`'s own process, `Start-Process -PassThru`).
  Synopsis, single source for docs and messages:
  ```
  py -3 .claude/hooks/cl9_lock.py acquire <build|compile|pie|commandlet|editor> [--minutes N] [--pid P] [--wait S]
  py -3 .claude/hooks/cl9_lock.py release [--force]
  py -3 .claude/hooks/cl9_lock.py status
  py -3 .claude/hooks/cl9_lock.py guard off [--minutes N] | on
  ```
  **Atomicity:** acquire creates the file with `O_CREAT|O_EXCL`; on `EEXIST` it reads,
  validates (expiry, process triple), and only then replaces via temp file +
  `os.replace`, then re-reads and confirms its own `session_id` is the holder — two
  simultaneous acquires cannot both believe they won. `--wait` is capped at 90 s (a
  Bash tool call dies at 120 s and would leave a lock nobody knows about); beyond that
  the command returns "busy" and the caller retries. **Owner identity:** a script run
  from Bash does not know its session; the hook does. The hook does **not** rewrite
  commands (`updatedInput` + allow would let `…acquire build; git reset --hard` ride
  through): when it sees exactly `py -3 .claude/hooks/cl9_lock.py acquire|release …` as a
  single simple command (no `;&|`, backtick, `$(`, newline) it performs the acquire or
  release itself from stdin, stamping the exact `session_id`, and lets the command print
  `status`. **Polarity:** a destructive editor/build verb — Bash/PowerShell matching
  `taskkill.*UnrealEditor|Stop-Process.*Unreal|Build\.bat|UnrealEditor-Cmd`, or
  `mcp__unreal-mcp__call_tool` with `tool_name` in `StartPIE|StopPIE` — is allowed only
  when **this** session holds the exclusive lock (not merely "nobody else does": a
  session that drives the editor over MCP without ever locking would otherwise get its
  editor killed — the 09-07 incident). An exclusive `acquire` waits for outstanding
  `editor` leases held by *other* sessions to be released or to expire; `editor` leases
  are automatic — the hook renews one for a session on every `mcp__unreal-mcp__call_tool`
  it makes, so any restarted session that touches the editor is protected without
  typing anything, and `/cl9 role` takes one on adopt. `save_assets` is **never** denied
  (a peer told "the editor goes down" must be able to save). Denials carry the holder,
  purpose, expiry, process state and the one-line fix ("끝났으면 상대가 `cl9_lock.py
  release`, 죽었으면 `release --force` — 살아 있는 빌드(PID 1234)가 있으면 force도
  거부된다. docs/workflow/sessions.md#잠금"). `acquire commandlet` first checks
  `tasklist /FO CSV /NH` (locale-independent: count rows, do not parse the Korean
  "정보:" line) for **both** `UnrealEditor.exe` and `UnrealEditor-Cmd.exe` and refuses
  while either is alive, naming the PID ("에디터가 떠 있다(PID 1234) — 커맨드릿은 에디터를
  닫은 창에서만"). `release --force` refuses while the recorded process is alive. The
  `SendMessage` courtesy stays (the lock says *no*, the message says *why and when*).
  A human closing the editor by hand or a PowerShell window outside the harness is out
  of the hook's reach — that stays the checklist in `sessions.md`. **Hooks are
  snapshotted at session start:** the four sessions running today do not load the hook
  until restarted — rollout step 1 says so, and `/cl9 status` reports whether the hook is
  active in *this* session by a nonce: `status` runs `py -3 .claude/hooks/cl9_lock.py
  status --nonce <random>`; the hook, seeing that command, writes the nonce to
  `.claude/runs/hook-seen/<nonce>`; the command then checks for it (no mtime races).

- **H. Compile check that does not require closing the editor (probe, then adopt)** — the
  critical path of every run is build → editor restart → PIE. Verify on this machine
  whether `Build.bat CL9 Win64 Development -Project=...` (the *game* target: separate
  binaries and intermediates from the loaded `UnrealEditor-CL9.dll`) compiles the same
  sources while the editor process stays open, and how long it takes. If yes: it becomes
  the `cl9-*` agents' allowed compile check (the hook allows exactly that command,
  serialized through the lock with `purpose: compile` — UBT's global mutex serializes it
  against any other UBT run anyway), so agents return C++ that at least *compiles as a
  syntax check* — the game target builds with `WITH_EDITOR=0`, so editor-only code paths
  are not proven — and the lead's editor build is one pass more often, not always. If no (UBT mutex or
  WITH_EDITOR-only code makes it useless): record the result in this doc and keep the
  one-build-after-all-reports flow. Same probe session also checks whether background
  subagents stall on permission prompts for `blender -b` and `python -m unittest`
  (the likeliest "silent agent" cause) and adds the needed allowlist entries.

- **D. Dispatcher skill** — `.claude/skills/cl9/SKILL.md` (`/cl9`, `user-invocable`).
  Synopsis (single source; sub-command names English like every Claude Code command,
  every message Korean; Korean aliases accepted: `작업`, `역할`, `상태`, `이어서`, `인계`,
  `커밋`, and role aliases `전투|시스템→gameplay`, `유아이→ui`, `맵→map`, `아트→art`,
  `문서→docs`, `밸런스|수치→balance`):
  ```
  /cl9                                  usage (Korean)
  /cl9 task <설명> [--mode A|C] [--roles ui,gameplay]
  /cl9 <설명>                            same as task, when the first word is not a sub-command
  /cl9 role [<name>]                    print + adopt a contract; no name → list roles
  /cl9 status                           peers · lock · hook active? · recent runs (flags, denials, per-role elapsed)
  /cl9 resume [<slug>]                  default: latest run not REPORTED
  /cl9 handoff
  /cl9 commit [<role>]                  default: the role this session adopted
  ```
  Parsing rule: a first token is a sub-command only if it is a keyword **and** the
  remaining tokens fit that keyword's arity (`status`/`handoff` 0, `role`/`resume`/
  `commit` 0–1); otherwise the whole string is a task — so `/cl9 status 표시 버그 고쳐`
  is a task. `task` and `--` are the explicit escapes.
  1. `/cl9 <task>` —
     **Intake** (no dispatch yet): restate the task; read `docs/plan/build-order.md`
     status, `memory/work-progress.md`, `ListAgents` (other live sessions and what they
     own), `cl9_lock.py status`; ask **every** ambiguity now, one `AskUserQuestion` each
     (the user's standing rule: never guess intent) — subagents get no mid-run questions
     except a hard blocker. If the task is feature-shaped (new system/mechanic), hand it
     to `/autogoal` first and dispatch from the resulting `docs/goal/<slug>.md`.
     **Triage**: produce the split table — role → goal, **explicit file list** (owned +
     assigned shared files, single writer each; written to `grants.json`), interface it
     needs from another role with the exact signature, needs build? PIE section letters —
     with the standing rule that a new or changed **tunable column of a `docs/plan/data/*`
     table** gets `balance` (value + reasoning + checker output) in parallel with
     `gameplay` (column/mechanism + placeholder default), recorded as an interface
     dependency, and that every grant-gated path is assigned to exactly one role per run —
     and show it with one `AskUserQuestion`: A) run as subagents, B) adjust, C) print
     contracts for manual terminals, D) cancel. `--mode`/`--roles` pre-answer it. A
     single-role task skips this question and runs. Roles with no work are not spawned.
     Default recommendation per task shape (registry `default_mode`): docs/art/tooling and
     pure-text C++ slices → A; slices that need the editor in a loop (WBP wiring, level
     placement, ABP graphs, anything the user will steer by eye) → C, because those are
     exactly the 09-06 → 09-12 sessions where the user intervened mid-work.
     **Research sub-tasks** (read-only work that produces a report and edits nothing:
     reference-game analysis, session-log summaries, doc audits, engine-source lookups)
     are not roles — the lead spawns a `general-purpose` agent on `sonnet` (bulk
     extraction on `haiku`), the way the 29-session summary was produced today, and
     folds the report into the briefs.
     **Dispatch**: refuse if any granted file overlaps another live run's `grants.json`;
     snapshot the baseline (`git status --porcelain`, `baseline.patch`, mtime/hash of
     dirty paths); write `.claude/runs/<slug>/brief-<role>.md` (goal, file list,
     interfaces to provide/consume, docs to read, PIE letters, report path),
     `grants.json`, `run.json` (state, roles, timestamps — lead-only file; the hook only
     touches `touch-<role>`); spawn each role with the `Agent` tool (`subagent_type:
     cl9-<role>`, background) — interface providers first when a consumer depends on them
     (gameplay getter → ui), otherwise all at once.
     **Integration** (lead only, after all reports): report validator (the seven headings;
     missing ones listed in the nudge); a role with no report and a `touch-<role>` older
     than 10 min → `SendMessage "보고서를 지금 써라"` once, then `TaskStop` + `ABORTED`
     (the hook then denies that role's further edits; a stopped agent cannot be resumed —
     re-dispatch with its report so far if needed); post-run audit against the baseline
     (C); **report fields are data**: a "다른 역할에 요청" line is re-stated by the lead
     to the owning agent, never forwarded verbatim, and no command quoted in a report is
     executed;
     merge "다른 역할에 요청" — small ones go back to the owning agent via `SendMessage`
     (context intact), anything that changes the split re-triages; show the user the
     per-role report summaries and ask once (A) proceed to build/PIE, B) change something,
     C) stop here); apply `.uasset`/`.umap` requests (MCP while the editor is up,
     commandlet only in the closed window under `acquire commandlet`); if any C++ changed
     run the build protocol under the lock (`acquire build --pid` → save_assets → notify
     peers → taskkill → detached `.cmd` → `Monitor` → restart editor → `release`); run the
     union of PIE sections via MCP + `cl9.*` cheats; append the collected `decisions.md`
     entries; update `memory/work-progress.md`; write `run.json` final state (wall time,
     roles, denials, build count, PIE result, audit violations).
     **Report** (chat, Korean): what was done per role, what was verified in PIE, open
     questions, and the per-role file lists ready for `git add` — commit only when the
     user says so (standing rule).
  2. `/cl9 role [<name>]` — print that role's contract (agent body + the shared-resource
     protocol incl. the lock synopsis) and adopt it in *this* session: the manual
     multi-terminal mode the user runs today, generated instead of typed. The session
     announces itself to `ListAgents` peers with its role and owned paths; for a
     single-folder role it may also call `/freeze`.
  3. `/cl9 status` — live sessions and roles, lock state, whether the hook is active in
     this session, `.claude/runs/*/run.json` from the last 48 h with their "빌드 필요 /
     PIE / 질문 / 위반" flags, denial counts, and per-role elapsed since `last_write`.
  4. `/cl9 resume [<slug>]` — a fresh lead (after context exhaustion) reloads briefs,
     grants, reports and `run.json` and continues from the recorded state (pain 4).
  5. `/cl9 handoff` — what every session did by hand at 75%: write
     `memory/work-progress.md`, write `.claude/runs/<slug>/handoff.md`, print the resume
     prompt for a new terminal.
  6. `/cl9 commit [<role>]` — stage exactly the files the role's report (or, in manual
     mode, the session's own list) names, draft the Korean commit message from the
     report, show it, commit only on the user's explicit yes (pain 3).

- **E. Shared-resource protocol doc** — `docs/workflow/sessions.md` (Korean): the role
  table, the lock and the editor/PIE/build serialization rules (moved out of memory into
  the repo; `memory/editor-kill-allowed.md` and `unreal-mcp-usage.md` reduced to
  pointers), the brief/report formats **with one copy-paste example of each**, the lock
  synopsis and copy-paste blocks (acquire/release/force, restart-after-settings-change,
  the resume prompt), the `#거부` and `#잠금` anchors every denial links to, how `/cl9`
  and manual sessions coexist (a `/cl9` lead is one more `ListAgents` peer under the same
  lock), the commit convention (one commit per role, file lists from the reports), the
  "verified on Claude Code 2.1.201" note, and the runbooks: ghost editor, build failure
  routed back to the owning role, silent subagent, stale lock, hook inactive (restart
  the session). `CLAUDE.md` gets a one-line pointer. `.claude/settings.json` also gains
  explicit allowlist entries for each helper (`Bash(py -3 .claude/hooks/cl9_lock.py:*)`,
  `Bash(py -3 .claude/hooks/render_roles.py:*)`, and the PowerShell twins — a mid-pattern
  glob in a permission rule is unverified, so scripts are listed one by one) so
  `status`/`lock` never cost a permission prompt.

- **F. Dry-run acceptance task** — before real use, run `/cl9` on a two-role task that
  already exists in the tree (e.g. "HUD 스태미나 바 색 문턱을 CL9Rules 컬럼으로": gameplay
  adds the column + getter, ui reads it) and record in this doc: split table produced,
  hook blocked one deliberate out-of-scope edit and allowed one granted foreign path,
  post-run audit clean, lock held during the build, build count, PIE section passed,
  per-role file lists correct, and **wall time vs the same task done serially in one
  session** (if the number is not better, say so — the dispatcher then stays a
  convenience, not a speed claim).

- **I. Agent Teams spike (bounded, 30 min)** — with the role files in place, set
  `CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1`, spawn `cl9-ui` + `cl9-gameplay` as teammates
  on the same dry-run task, and record — first of all the raw hook stdin a teammate's
  tool call produces (its own `session_id`? an `agent_type`?), before any lock test —
  then: does it run on this Windows box, can the user see and steer each teammate, do
  teammates coordinate the lock themselves. Outcome decides whether D's dispatch phase
  uses Teams for editor-in-the-loop roles.

## Design

```
  user ──/cl9 "<task>"──▶ LEAD (this session)
                            │ intake: every question now → AskUserQuestion
                            │ triage: split table (file lists!) → AskUserQuestion (A/B/C/D)
                            │ briefs + grants.json + run.json → .claude/runs/<slug>/
                            ├──Agent(cl9-gameplay)──┐  parallel, editor-free
                            ├──Agent(cl9-ui)────────┤  text edits only
                            ├──Agent(cl9-map)───────┤  hook = tripwire (owns∪shared∪grants)
                            ├──Agent(cl9-art)───────┤  (blender -b allowed)
                            ├──Agent(cl9-docs)──────┤  (compile check if H ok)
                            └──Agent(cl9-balance)───┘  docs/plan is its canon; decides values;
                                      │               emits table-row requests + inequality check
                                      │ report-<role>.md
                            ◀─────────┘
                            │ integration (serialized, lead only):
                            │   report validator · post-run audit (git status vs grants)
                            │   requests → SendMessage(agent) / re-triage
                            │   user sees report summaries → AskUserQuestion
                            │   .uasset/.umap requests → MCP (editor up) | commandlet (editor down)
                            │   C++ changed? → lock(build,pid) → notify → build → restart → release
                            │   PIE sections → decisions.md → work-progress → run.json
                            ▼
                          report + per-role file lists (/cl9 commit <role> on request)

  manual mode:  terminal N ──/cl9 role ui──▶ same contract, same lock, same rules;
                peers coordinate via ListAgents/SendMessage + editor.lock.

  editor.lock:  any session ── cl9_lock.py acquire build ──▶ hook stamps session_id ──▶
                .claude/runs/editor.lock; hook denies taskkill/Build.bat/StartPIE/
                save_assets to everyone else until release, expiry (60 min) or PID death.
```

State machine of one `/cl9` run (`run.json.state`): `INTAKE → TRIAGE → DISPATCHED →
(BLOCKED_ON_QUESTION ⇄ DISPATCHED)* → AUDITED → REVIEWED → BUILDING? → PIE? → REPORTED`,
plus `ABORTED` from any state (user `TaskStop`, or a role fails to produce a report after
one `SendMessage` nudge). Invalid transitions: DISPATCHED → BUILDING (skips AUDITED and
REVIEWED — the lead never builds unreviewed subagent output); BUILDING without the lock.
A subagent that hits an ambiguity does not guess: it writes the question, ends its turn;
the lead asks the user and resumes that agent by name.

Why subagents for dispatch and not Agent Teams (yet): teams give each role a live session
the user can watch and talk to — closest to today's terminals — but the feature is
experimental, in-process on Windows, cannot run background subagents, cannot `/resume`.
The registry, role files, lock, hook and protocol doc are identical under both; the spike
(I) decides the dispatch layer with evidence instead of four lines of prose.

Why not `Workflow`: it needs an explicit opt-in phrase every run and its agents are the
same in-process subagents; its one distinct benefit, resume-by-run-id, is covered by
`run.json` + `/cl9 resume`. Reconsider if runs outgrow a single lead context.

Why not git worktrees (even for text-only roles): `Content/` is binary and editor-locked;
`Source/` edits must be built together in one tree; a docs-only worktree adds a merge step
for conflicts that never occurred in 29 sessions. Paths and per-run file lists, not
branches, are the isolation unit — and the post-run audit, not the hook, is the proof.

## NOT in scope
- Running the game or editor from a subagent; multiple editors; PIE in parallel.
- Auto-commit or auto-push — `/cl9 commit <role>` commits only on the user's explicit yes
  (standing rule; today's tree has four sessions' worth of deliberate uncommitted work).
- A `Workflow` script variant (TODO), a scripted PIE smoke harness driving `cl9.*` cheats
  per role with screenshots (TODO — the integration step runs PIE via MCP by hand), a
  naming scheme for sessions (`cl9-ui-1`) — whether a session can set its own
  `ListAgents` name is unverified (TODO probe), GUI dashboards.
- Rewriting existing memory files beyond reducing them to pointers at
  `docs/workflow/sessions.md`.
- Changing `/autogoal`; `/cl9` calls it when a task is feature-shaped.
- Column-level enforcement in the hook (art's "mesh/anim columns only") — the `note`
  field, the post-run audit and the report cover it.
- Untracking `.claude/settings.local.json` (it is tracked today, contrary to its purpose)
  — a separate one-line cleanup; noted in TODOS.
- Intercepting a human who closes the editor or runs PowerShell outside Claude Code.

## Open Questions (for the final gate)
1. Default dispatch mode for C++ slices: the outside voice recommends manual terminals
   (mode C) as the default for anything the user steers by eye, subagents (mode A) for
   docs/art/tooling and pure-text C++. The user's stated direction was "알아서 나눠서" —
   this doc keeps A available for everything (`--mode A`) and recommends C only where the
   logs show the user intervening mid-work. Your call.
2. Ship order: outside voice says lock + protocol + hook + `/cl9 role` first and the
   dispatcher only after H (compile check) or I (Agent Teams) proves out. This doc keeps
   the dispatcher in scope but orders rollout that way.
3. Role taxonomy: you said UI/전투/시스템/아트 and, at the gate, asked for a balance
   agent; this doc now has gameplay/ui/map/art/docs/**balance** (전투 and 시스템 both live
   in gameplay C++, map is separate; `전투`/`시스템` are accepted aliases for gameplay,
   `밸런스`/`수치` for balance). A registry edit if still wrong.
4. The hook and allowlist entries go in the tracked `.claude/settings.json` (recommended —
   the lock and hook are the repo's working rules). `settings.local.json` being tracked
   is a separate cleanup (TODO).

## Success criteria (verified by the implementer, then by the user)
0. First probe, recorded in this doc: which shell runs hook commands on this box and
   whether `py -3` resolves there; the exact stdin a subagent's Edit produces (captured as
   a test fixture); whether `permissionDecision: "ask"` surfaces under bypass mode;
   whether `agent_type` is present on 2.1.201.
1. `py -3 -m unittest discover .claude/hooks` → all cases pass (scope guard incl. grants,
   grant-gated shared paths (gameplay + balance in one run, `egg.md` granted to balance →
   gameplay denied naming balance), allowlist corpus, terminal-state deny, normalization
   and JSON deny shape; lock incl. N-process acquire race, process-triple liveness and
   PID reuse, owner re-acquire idempotence, `--force` vs live process, `--wait` cap,
   Korean-locale `tasklist`; registry coverage + pairwise `shared` intersections all
   grant-gated; renderer equality; latency budget; balance diff-shape audit (a renamed
   column is a violation); `Tools/Balance/check_inequalities.py` fixtures — A1 failing by
   1 cm/s, curve-max vs base, A4 refused).
2. Hook probe inside Claude Code (after restarting the session): a `cl9-ui` subagent
   asked to edit `Source/CL9/Items/CL9Item.cpp` is refused with the owner named and the
   doc anchor; the same path listed in `grants.json` is allowed; the lead editing the
   same file is not refused; without holding the lock, this session's `taskkill /IM
   UnrealEditor.exe` is refused (polarity: the killer must hold it); with another session
   holding it, the refusal names the holder + expiry + process state; a peer's
   `save_assets` is never refused; a malformed `editor.lock` makes the lead's next tool
   call surface the reason to the model, not pass silently; `/cl9 status` says the hook
   is active (nonce).
3. `/cl9 role ui` prints a contract whose owned paths equal the registry entry (rendered,
   not retyped); `/cl9` alone prints usage; `/cl9 status 표시 버그 고쳐` is treated as a
   task; `/cl9 role balance` prints a contract whose canon list starts with
   `docs/plan/economy.md` and whose report template carries the "표 행" and "검증(부등식)"
   sections.
4. H recorded: game-target compile with the editor open — works or not, and its time;
   background-subagent permission stalls checked.
5. The dry-run task (F) completes with the post-run audit clean **against the baseline
   taken with the tree already dirty**, one build under the lock (`--pid` set, a second
   session refused meanwhile, `release --force` refused while the build lives), one PIE
   pass, per-role file lists that `git add` cleanly by role, and a recorded wall time vs
   serial (recorded, not gated — one sample with model variance). The stamina-bar
   threshold has no home in `docs/plan/data/rules.md` (only the shake `StartAt`), so in
   F `balance` must take the **질문 path** (report a question, not invent a value) — that
   is the test. A second, single-number dry-run **F2** exercises the happy path: change
   one A1 operand (e.g. `raccoon.ChaseSpeed`) → balance sets it, the checker output shows
   A1/A2 and "A4 미결", the lead applies the row, re-reads the asset, and the PIE
   observable is recorded.
6. `/cl9 status` and `/cl9 resume` (no argument) reproduce the dry-run's state from disk.
7. I recorded: Agent Teams spike outcome on this box.

## Rollout
1. Probe (success criterion 0) → registry + loader + lock (atomic, polarity, leases,
   process triple) + hook (allowlist, baseline audit helpers) + tests + `.gitignore`
   (`.claude/runs/`) + explicit settings allowlist — **then restart every open Claude
   Code session**: hooks are snapshotted at session start, so until a session restarts
   it neither enforces nor obeys the lock (`/cl9 status` tells you which). Behavior
   change for restarted sessions: killing the editor, building or starting PIE requires
   holding the lock (`acquire` first — one command); a session that merely drives the
   editor over MCP is protected automatically by its `editor` lease.
2. `docs/workflow/sessions.md` + `CLAUDE.md` pointer + memory pointers + `/cl9 role`,
   `status`, `handoff`, `commit` (the parts every session benefits from today).
3. H probe (compile check + background-permission check) — record result, wire into
   agents if positive.
4. Rendered role agents + `/cl9 <task>` dispatch/integration + `resume`; dry-runs F and
   F2 (balance question path, then balance happy path); record results in this doc.
5. I spike (Agent Teams) — record; if positive, switch dispatch for editor-in-the-loop
   roles.
6. Use on the next real multi-domain task; capture friction into TODOS.md.

## Implementation record

### Rollout 1 — probe + registry + loader + lock + hook + audit + tests (2026-09-13, session cl9 lead)

**Built** (all under `.claude/`, 86 unit tests green via `py -3 -m unittest discover .claude/hooks`,
one skipped = real-stdin fixtures pending the restart):
- `.claude/cl9-roles.json` — six roles as in A; `lead_only_except: [Config/Localization/*]` added
  because A gives ui `Config/Localization/*` while `Config/*` is lead-only (precedence: append_only >
  lead_only(-except) > owns(-except) > shared(grant-gated needs a grant) > grants > outside).
  Two paths the table did not place: `docs/plan/settings.md` → ui, `docs/plan/data/README.md` → docs.
- `.claude/hooks/cl9_roles.py` — loader/matcher (fnmatchcase, `*` crosses `/`, braces/`**` rejected),
  `to_rel` = realpath + normcase + commonpath, coverage/intersection helpers.
- `.claude/hooks/cl9_lock.py` — G as specified: O_EXCL create → mkdir-mutex read-modify-write →
  tmp + `os.replace` → re-read confirm (8-process race test: exactly one winner); process triple
  via `OpenProcess/GetProcessTimes/QueryFullProcessImageName` (+ STILL_ACTIVE); `--pid` required for
  build/commandlet; leases 30 min, exclusive 90/30 min; `--wait` capped 90 s; `release --force`
  refused while the triple is alive; commandlet refuses while `UnrealEditor.exe`/`UnrealEditor-Cmd.exe`
  are in `tasklist /FO CSV /NH` (Korean "정보:" line ignored by CSV-row parsing); `guard off|on`
  (TTL 60 min); `status --nonce` marker; CLI without `--session` = hook-mediated — the hook
  performs acquire/release from stdin `session_id` and leaves `hook-seen/last-lock-cmd.json`, which
  the CLI checks (5 s) to say "훅이 처리함" vs "훅 비활성 — 재시작".
- `.claude/hooks/cl9_scope_guard.py` — C as specified; extras: a role may write only its own
  `.claude/runs/<slug>/report-<role>.md` inside `.claude/`; read-only list also has `rg`, `echo`,
  `pwd`, `git rev-parse`; `find` with `-delete/-exec…` denied; roles may run `cl9_lock.py status`
  only; `save`-named unreal tools are never denied; destructive-verb detection is per shell segment
  and skips read-only programs (so `grep Build.bat docs/` passes); root is found by walking up from
  stdin `cwd` to the registry (survives a lead that `cd`'d).
- `.claude/hooks/cl9_audit.py` — baseline snapshot (porcelain, `baseline.patch`, mtime/sha1 of dirty
  paths, copies of grant-gated docs) + delta audit (unclaimed = violation, reverted foreign dirt =
  violation, `.claude/`/`Config/` delta and `guard.off` = violation) + `balance_diff_shape`
  (numeric cell of an existing row only; renamed column / row key / new row / prose = violation).
- `.gitignore` gets `.claude/runs/`. `settings.json` is **not** touched yet (user: after cl9-5b's
  commits) — the exact merge is in `.claude/hooks/settings-merge.json`.

**Probe 0 (success criterion 0), what could be verified without a restart:**
- `py -3` → `C:\Windows\py.exe` → Python 3.9.13 (3.12 entry is the Store stub). Works from the Bash
  tool (process chain python.exe ← py.exe ← bash.exe ← claude.exe).
- **cp949 locale trap**: Python's pipe encoding is cp949 here — a hook printing Korean/`—` crashes
  with UnicodeEncodeError and stdin with Korean mis-decodes. Rule for every hook/CLI: read
  `sys.stdin.buffer` as UTF-8, write stdout JSON with `ensure_ascii=True`, `reconfigure(utf-8)` for
  human output. Tests cover Korean in stdin and ASCII-only stdout.
- **Agent definitions load with a delay on 2.1.201**: a new `.claude/agents/cl9-probe.md` was
  "not found" for the rest of the turn in which it was created (also when copied to
  `~/.claude/agents/`, and frontmatter `hooks:` added to an already-loaded agent were not
  re-read on spawn) — it became available at the next user turn, ~30 min later. Rule: create
  role files, then expect them from the next turn on; a restart is the sure path.
- **Post-restart probe ran in-session (2026-09-13 00:23, subagent `cl9-probe`, 7 hook calls
  captured to `.claude/hooks/fixtures/`)**:
  - Hook shell = **Git Bash** (process chain python.exe ← py.exe ← bash.exe ← bash.exe ←
    claude.exe; `SHELL=C:\Program Files\Git\bin\bash.exe`, `MSYSTEM=MINGW64`), cwd = the
    project root, `CLAUDE_PROJECT_DIR=D:/Work/unreal/CL9` (forward slashes), plus
    `CLAUDE_CODE_SESSION_ID`, `CLAUDE_PID`, `CLAUDE_EFFORT`, messaging socket/token in env.
    `py -3` resolves there. Hook body time: 434 ms cold (first pyc), ~73 ms warm.
  - stdin has `session_id` (**the parent session's id** — lock ownership by session works for
    subagents as planned), `agent_id`, `agent_type: "cl9-probe"`, `permission_mode:
    "bypassPermissions"`, `hook_event_name`, `tool_name`, `tool_input` (Write: file_path,
    content · Edit: file_path, old_string, new_string, replace_all · Bash: command,
    description), `cwd` with backslashes.
  - `permissionDecision: "deny"` → the agent receives the reason verbatim as the tool error.
  - `additionalContext` on an allow **reaches the model** as a system-reminder
    ("PreToolUse:Bash hook additional context: …"); `systemMessage` does **not** (it is UI-only).
  - `"ask"` under bypassPermissions **does surface a prompt** and blocked the agent for 16 min
    until it was answered (denied). So `ask` is a stall, not a message channel. **Decision:**
    lead internal-error polarity = `deny` for destructive verbs / StartPIE / StopPIE /
    `cl9_lock.py` commands (the guard could not verify, so it blocks), `allow` +
    `additionalContext` (the model sees the reason) + `guard-errors.log` for everything else.
    Implemented in `lead_internal_error()`, tests updated.
  - Under bypass mode every tool result carries a system-reminder telling the agent to prefer
    Bash heredocs/sed over Write/Edit. A `cl9-*` role would then hit the shell allowlist (no
    `>`/`sed -i`) and be denied — the rendered role bodies (rollout 4) must say explicitly
    "파일 수정은 Edit/Write 도구로 — Bash 는 읽기 전용".
- Docs (hooks.md, 2026-09-13): `ask` forces the prompt even in bypassPermissions when interactive
  and degrades to allow when non-interactive; exit 0 + JSON is the decision channel, stderr never
  reaches the model; Windows default hook shell is Git Bash when present, PowerShell otherwise,
  `"shell"` selectable; `$CLAUDE_PROJECT_DIR` is the session's project root. Whether settings-file
  hooks are file-watched on 2.1.201 is unverified — the rollout keeps the restart.
- **Latency**: `py -3 -c pass` alone is ~80 ms on this box (launcher + interpreter + site with a
  pywin32 .pth); the hook end-to-end is median ~115 ms via `py -3`, ~95 ms via the absolute
  `python.exe`, ~79 ms with `-S`. The 100 ms budget is not reachable through the launcher; the
  test budget is 150 ms. Switch the command to the absolute interpreter path if the 20 ms matter.
- Tooling traps for the implementer: the Bash tool sends commands in cp949 (Korean literals in
  heredocs corrupt) and unescapes `\\` in heredocs — write scripts with the Write tool; MSYS
  converts `/FI`-style flags and `$$` is an MSYS pid, so tests spawn real Windows PIDs from Python.

**Rollout 1 closed (2026-09-13 01:10)**: cl9-5b reported its commits done (HEAD 04fb84f) → hook +
allowlist merged into the tracked `.claude/settings.json` → restart notice sent to the four live peers
(cl9-37, cl9-2d, cl9-13, cl9-73; cl9-5b had already exited). Verified: the settings-file hook is
**not** picked up by a running session on 2.1.201 (`status --nonce` still "비활성" after the edit) —
the restart is required, as the rollout says.

### Rollout 2 — protocol doc + `/cl9 role|status|handoff|commit` (2026-09-13, same session)

- `docs/workflow/sessions.md` (Korean): component table, role summary table (registry is the source),
  `#잠금` (lock rules, copy-paste build/PIE/commandlet blocks, "when blocked"), `#거부` (every deny
  reason → what to do), brief + report examples, `/cl9` vs manual coexistence, commit convention,
  handoff + resume prompt, runbooks (ghost editor, build failure → owning role, silent subagent,
  stale lock, hook inactive, post-settings-change notice). `CLAUDE.md` gets a table row;
  `memory/editor-kill-allowed.md` reduced to a pointer, `unreal-mcp-usage.md` gets a pointer line
  (its MCP API quirks stay — they are not protocol).
- `.claude/hooks/render_roles.py`: `--list`, `--print <role|alias>` (manual contract = agent body +
  shared-resource block + announce line), `--write` (six `.claude/agents/cl9-*.md`, **not run yet** —
  rollout 4), `--check` (rendered == committed, registry sha256 stamp). The body is the B-section
  contract: owns/except/shared/grant-gated rendered from the registry, docs, must-NOT list, machine
  quirks (incl. "파일 수정은 Edit/Write 도구로" against the bypass-mode Bash nudge), the seven report
  headings, balance extras (표 행 · 검증(부등식), A4 미결).
- `.claude/hooks/cl9_status.py`: lock/guard + runs of the last 48 h (state, roles, reports present,
  denial count, flags, minutes since `touch-<role>`), guard-errors tail. Added to the allowlist.
- `.claude/skills/cl9/SKILL.md`: usage, parsing rule (keyword + arity, else task), `role` (print
  verbatim → adopt → `acquire editor` → announce to peers → manual run dir), `status` (nonce via
  Bash first, then script, then ListAgents), `handoff`, `commit` (report file list only, user yes
  only). `task`/`resume` print "롤아웃 4에서" and stop.
- Tests: `test_render_roles.py` (paths verbatim, stamp, frontmatter without Agent/MCP tools, balance
  sections, report order, write/check roundtrip, alias CLI, status script) → **95 tests green**.

**Incident, 2026-09-13 01:35 (cl9-2d report)** — the hook command was the relative
`py -3 .claude/hooks/cl9_scope_guard.py`; hooks run in the session shell's cwd, and cl9-2d's cwd was
`Saved/Screenshots/WindowsEditor`, so Python exited with code 2 ("can't open file") — and **exit 2 is
Claude Code's blocking code**, so every Bash/PowerShell call in that session was refused, including
`cd`. Fix: `py -3 "$CLAUDE_PROJECT_DIR/.claude/hooks/cl9_scope_guard.py"` (hook shell is Git Bash,
variable verified as `D:/Work/unreal/CL9`); reproduced from `Saved/` and covered by
`test_cl9_scope_guard.Settings` (command must be absolute; relative script path → rc 2). Lesson
recorded in the hook docstring: any hook launcher failure is a hard block, so the command line must
not depend on cwd. Observation: cl9-2d's session picked up the corrected command **without a
restart** — settings-file hooks are file-watched on 2.1.201 once a session has the hook loaded;
this lead session (started before the hook existed) still reports 비활성, so a session without the
hook does need the restart.

### Rollout 3 — H probe: compile check with the editor open (2026-09-13 01:59–02:33, editor PID 10872 up throughout)

**Verdict: positive, adopted.** `Build.bat CL9 Win64 Development -Project=… -WaitMutex` (the *game*
target) compiles the same `Source/CL9` while `UnrealEditor.exe` stays open and MCP port 8000 keeps
listening; it writes `Binaries/Win64/CL9.exe` (322 MB) + `Intermediate/Build/Win64/x64/CL9/` and never
touches `UnrealEditor-CL9.dll`. Measured on this 16 GB box with the editor committing ~8 GB and the
user's game/browser using ~3 GB more (free RAM 0.7–2 GB):

| run | what | wall time |
|---|---|---|
| 1 | first game-target build (SharedPCH + module + link, 12 actions) | 10 min 02 s |
| 2 | no change | 1 min 25 s |
| 3 | one unity-member `.cpp` touched → whole `Module.CL9.cpp` recompiles | **20 min** — `cl.exe` sat at 1.7 s CPU for 9 min (paging), finished within 1 min once ~5 GB freed |
| 4 | no change, via `Tools/Build/compile_check.py`, memory freed | 16 s |

So the mechanism is sound and the cost is **memory, not the editor**: the same stall hit cl9-73's
editor-target build at 01:24 (cl.exe 1.8 s CPU in 12 min, killed and retried, 20 min total). With
≥ 3 GB free an incremental check is 1–3 min; below that it can crawl for tens of minutes. Adopted as
follows:
- `Tools/Build/compile_check.py` — takes the `compile` lock itself (session id from
  `CLAUDE_CODE_SESSION_ID`, verified present in the Bash tool env), runs the game target in the
  foreground (call with tool timeout 600000), kills the chain after `--timeout-min` (default 20),
  prints a memory warning under 3 GB free, ends with `COMPILE OK|FAIL <s>` + up to 40 error lines,
  logs to `.claude/runs/compile/`. Passes the role shell allowlist (`py -3 Tools/*.py`); added to the
  settings allowlist. Not unit-tested (needs the engine) — validated end to end (run 4).
- Role contracts (renderer) now say: after any C++ change run the check, paste the result line into
  '빌드 필요 여부', and that it is a *syntax check* (`WITH_EDITOR=0`); the editor-target build stays
  with the lead. `docs/workflow/sessions.md#잠금` gained a "컴파일 검사" block.
- Blender is not on PATH here: contracts and the allowlist corpus use
  `"C:/Program Files/Blender Foundation/Blender 5.2/blender.exe" -b Tools/Blender/<script>.py`.
- Background-subagent permission stalls (the second half of H): a background `general-purpose`
  agent on sonnet ran `py -3 -m unittest discover .claude/hooks` (97 tests, 18 s), `blender.exe -b
  --version` and `py -3 Tools/Loc/sync_strings.py` under bypassPermissions with **no prompt and no
  stall**. (Side effect to remember: `sync_strings.py` has no `--help` and runs the sync — harmless,
  CL9.csv unchanged.)
- Protocol observations from the same window, folded into `sessions.md`: cl9-73 took `acquire
  compile` (30 min, no pid) for an editor-target build and the lock expired mid-build at 01:48 —
  builds must use `acquire build --pid`; my own probe used `compile --pid` so liveness kept it.
  Peers did use the lock unprompted (cl9-73 compile lock, a cl9-13/other editor lease), i.e. the
  rollout-1 behavior change is in effect for restarted sessions.

### Rollout 4 — dispatcher built, dry-runs pending (2026-09-13 02:40–03:20)

- `.claude/agents/cl9-{gameplay,balance,ui,map,art,docs}.md` written by `render_roles.py --write`
  (registry sha256 `22d5a0e9c7be498b`); `test_render_roles.test_committed_agents_match_registry`
  fails if the registry changes without a re-render.
- `.claude/hooks/cl9_run.py` — the lead's deterministic steps: `init` (slug + roles + grants.json,
  TRIAGE), `overlap` (intra-run + cross-run grant intersections against every non-terminal run),
  `dispatch` (refuses on overlap, snapshots the baseline, DISPATCHED), `state` (transition table;
  `BUILDING` requires this session to hold the exclusive lock via `CLAUDE_CODE_SESSION_ID`),
  `validate-report` (seven headings, +2 for balance, order checked), `audit` (delta vs baseline +
  balance diff-shape → `audit.json`, flags.violations), `finish`, `show`, `latest --not-reported`.
- `Tools/Balance/check_inequalities.py` — header-aware markdown table reader (`값`/`기본값`
  column, `A / B / C | x / y / z` rows), A1 on unscaled ChaseSpeed, A2 on SightIn × DangerCurve
  max (base also printed), A4 printed as `미결 — 식 없음`, monsters without a `CatchRadius` row
  reported as 판정 불가 (goose). Current docs: 506.6 > 396/462, 3924 > 429/507 — all pass.
- `Tools/Unreal/apply_table_rows.py` — parses `Asset.Row[/Sub].Column: old → new (unit)` from the
  report's `## 표 행`, resolves `/Game/Data/<Asset>` (DefTable rows by Alias/SubAlias or a
  standalone Def), coerces to the property's type, warns on old-value mismatch, never creates
  rows/columns; `apply` saves and re-reads, `verify` compares assets to the report. Editor-side
  via console `py` with `CL9_ROWS_REPORT`/`CL9_ROWS_APPLY` env, or headless under `acquire
  commandlet`. Parser unit-checked; editor path not yet exercised.
- `/cl9 task` and `/cl9 resume` in `SKILL.md`: intake → triage table → A/B/C/D → dispatch
  (grants → overlap → init → briefs → dispatch → Agent per role, providers first) → integration
  (validate → audit → requests re-stated → question loop → A/B/C → table rows → build under lock →
  PIE → decisions → work-progress → finish) → report. `task` refuses to start when this session's
  hook is inactive (the subagents' fence would be unenforced).
- 109 unit tests green (`py -3 -m unittest discover .claude/hooks`).

**Model policy addendum (user, 2026-09-13 03:40)**: planning (triage table) and review (report/audit
judgment) run on **Fable** — inline when the lead session is Fable, otherwise as one foreground
`general-purpose` subagent with `model: fable`; never both. Mechanical phases (lock, build wait, PIE,
request relay) stay on the lead's model, so an Opus lead is the cheap default and Fable is spent only
where the split can go wrong. Also added `--unattended` (밤샘): all questions and the full multi-run
plan up front, one approval, then no prompts — mid-run questions defer that slice into
`questions.md`, build failure gets one round-trip then ABORTED, PIE modal gets one WM_CLOSE retry,
3 h/run and 8 h total caps, never commits, ends with `morning.md`. Multi-item tasks are split by the
lead into roles and dependency-ordered runs — the user is never asked to pre-split.

**Not done — needs a restarted session**: success criterion 2 (in-session hook checks), dry-run F
("HUD 스태미나 바 색 문턱을 CL9Rules 컬럼으로": gameplay + ui + balance question path) and F2 (one A1
operand, e.g. `raccoon.ChaseSpeed`, balance happy path with `apply_table_rows.py` and the PIE
observable), the wall-time-vs-serial number, and spike I (Agent Teams). This lead session was
started before the hook existed, so it cannot run F itself.

---

## Review Record — /autoplan 2026-09-12

Pipeline: CEO (Phase 1) → DX (Phase 2.5, developer-facing scope detected: skill, agents,
hooks, CLI sub-commands) → Eng (Phase 3). Design phase skipped — no UI scope (0 view
terms in the plan). Codex CLI: usage limit until 2026-09-17 → every phase's outside voice
is `[subagent-only]`.

### Phase 1 — CEO review (mode: SELECTIVE EXPANSION, auto-selected: enhancement of an
existing workflow)

**Pre-review system audit.** 189 dirty paths from 4 live sessions (Items 14, Data 11,
World 8, UI 6, Content/Data 12, docs/systems 12, ArtSource 32); one stash of debug values;
no TODO/FIXME markers in Source/docs/Tools; `origin/master == HEAD`. Design doc found
(`user-master-design-20260827-163548.md`) is the 08-27 Steam design — unrelated, not used.
TODOS.md: five Steam/session items and one UI DESIGN.md item; none touch this plan, but
"Rejoin into a running night" and "Save/continue" will each be multi-role tasks — first
real customers of `/cl9`. Retrospective: the map rewrite (commits 81d1971 → b166eb9)
re-partitioned folders once already; that is the registry-drift case the coverage test
guards. Taste calibration — well-designed: `docs/plan/build-order.md` (principles before
steps, every step has a "직접 해보는 것"), `Tools/Loc/sync_strings.py` (one master sheet
→ generated artifacts, validation built in), `memory/editor-kill-allowed.md` (a protocol
written from incidents, with dates). Anti-patterns: rules living only in memory files;
`docs/decisions.md` appended by four sessions at once; CRLF accidents from mixed tools.
Landscape (WebSearch, Aside not installed): [Layer 1] orchestrator/worker with a lease on
the single mutable resource is the standard shape; [Layer 2] Claude Code docs and 2026
write-ups agree on three primitives — subagents (in-session), Agent Teams (cross-session,
experimental, role files reusable as team members), background agents — and on "define a
custom agent when you keep spawning the same worker; omitting `tools` grants all tools";
[Layer 3] for this repo the scarce resource is not agent time but editor/build time, so
the lock is the part with real leverage. No eureka beyond that.

**0A Premise challenge.**
- P1 "one prompt → automatic split" (user-stated): valid; ambiguous between auto-*run*
  and auto-*plan*; the plan delivers both (subagents + generated contracts). Surfaced at
  the gate (open question 1).
- P2 "one editor / PIE / build is physical": verified (port 8000, ghost editor, UBT mutex).
- P3 "parallel text edits on one tree are safe": true only for disjoint files → amended:
  single-writer per shared file, explicit file lists per brief, post-run audit.
- P4 "subagents cannot ask the user": true → all questions in intake; mid-run only blockers.
- P5 "the user wants less visibility": *not* stated and contradicted by the logs (constant
  mid-work steering) → amended: report review gate before build; mode C default for
  editor-in-the-loop slices; surfaced at the gate.
- P6 "five roles": derived from evidence, differs from the user's four words → gate item 3.
- P7 "path ownership is the isolation unit": valid for the fence, insufficient for
  vertical tasks → file lists per run; registry = default.
- P8 "hooks see `agent_type` on 2.1.201": documented, unverified → first implementation
  step is the probe; fallback is prompt-only fence + post-run audit (which exists anyway).
- Do nothing: the two shared-resource incidents recur; contracts stay hand-typed. Real.

**0B Existing code leverage.** Intake/probe → `/autogoal` Phase 1. Editor/build protocol →
`memory/editor-kill-allowed.md` + `unreal-mcp-usage.md` (moved, not rewritten). PIE
checks → `docs/testing/pie-scenarios.md` A–L. Asset requests → `Tools/Unreal/*.py`
(`run_python.py` via MCP; commandlet when closed). Peer coordination → built-in
`ListAgents`/`SendMessage`. Path fencing → gstack `/freeze` (session-level; same hook
mechanism, so `/cl9 role` in manual mode can simply call `/freeze` for single-folder
roles). Agent file format → `~/.claude/agents/ue-doc-collector.md`. Nothing is rebuilt.

**0C Dream state.**
```
  CURRENT                         THIS PLAN                          12-MONTH IDEAL
  2–6 terminals, contracts typed  registry + lock + hook + protocol  roles as team members (Teams
  by hand, rules in memory files, doc; /cl9 role contracts; /cl9     stable); editor lease honored by
  2 shared-resource incidents,    dispatcher for editor-free work;   every session; compile check
  20–48 min build waits, 75%      one build under lock; run.json +   without editor; scripted PIE
  context re-seeds                resume/handoff                     smoke per role; commit-by-role
```
Delta after this plan: incidents structurally prevented (lock), contracts generated,
resume/handoff scripted, dispatch available; still manual: PIE by hand, build wall time.

**0C-bis Implementation alternatives.**
```
APPROACH A: Contracts + protocol only (registry, /cl9 role, sessions.md, lock)
  Effort S · Risk Low · Completeness 6/10
  Pros: zero new runtime behavior for agents; fixes pains 1,5 · Cons: no auto-split, no
  hook, no resume · Reuses: memory protocol, /freeze
APPROACH B (chosen): A + tripwire hook + post-run audit + role subagents + /cl9 dispatch/
  resume/handoff/commit + compile-check probe + Teams spike
  Effort M · Risk Med · Completeness 9/10
  Pros: covers all six pains; layers ship in value order; dispatch layer swappable ·
  Cons: ~15 small files; hook adds ~50 ms per tool call; dispatcher value unproven until F
APPROACH C: Agent Teams as the dispatch layer from day one
  Effort M–L · Risk High (experimental, Windows in-process, no /resume) · Completeness 7/10
  Pros: closest to today's live terminals; shared task list · Cons: unverified on this box;
  cannot run background subagents · Reuses: same role files
RECOMMENDATION: B — completeness (P1) with A as its first two rollout steps and C as a
bounded spike (I) before committing the dispatch layer.
```
Taste decision (close approaches): B vs C for the dispatch layer → surfaced at the gate
as open question 2 together with the outside voice's "dispatcher later" verdict.

**0D Selective expansion.** Complexity check: ~15 files, 0 new C++ classes — above the
8-file smell, but the minimum for the mechanism is A's 5 files and every added file is a
test, a renderer or a one-purpose script; nothing is an abstraction. Expansion scan
(candidates, decided under P2 "in blast radius and < 1 day CC"):
| # | proposal | effort | decision |
|---|---|---|---|
| 1 | editor/build lock file (G) | S | ACCEPTED — outside voice: critical omission; targets the two recorded incidents |
| 2 | render agent bodies from registry + equality test | S | ACCEPTED (P4 DRY; registry drift regret) |
| 3 | registry glob coverage test | S | ACCEPTED (map-rewrite drift case) |
| 4 | `/cl9 resume <slug>` + `run.json` | S | ACCEPTED (lead context exhaustion = SPOF) |
| 5 | `/cl9 handoff` | S | ACCEPTED (done by hand in ≥4 sessions) |
| 6 | `/cl9 commit <role>` (explicit yes only) | S | ACCEPTED (pain 3) |
| 7 | compile check without editor (H) | S probe / M adopt | ACCEPTED as probe (outside voice: the 10x lever) |
| 8 | Agent Teams spike (I) | S | ACCEPTED (bounded; decides dispatch layer with evidence) |
| 9 | post-run `git status` audit vs briefs | S | ACCEPTED (hook is a tripwire) |
| 10 | deny `rm -rf` / `Remove-Item -Recurse` / `git clean` for role agents | S | ACCEPTED |
| 11 | scripted PIE smoke per role (cheats + HighResShot) | M | DEFERRED → TODOS (3–5 files, touches MCP flows; PIE by hand works today) |
| 12 | session naming (`cl9-ui-1`) | S probe | DEFERRED → TODOS (unverified whether a session can set its `ListAgents` name) |
| 13 | Workflow-script variant | M | DEFERRED → TODOS |
| 14 | `docs` role on `sonnet` | — | REJECTED (outside voice: 정본 on the cheapest model is the wrong economy) |
Delight (30-min touches, in plan): Korean denial messages naming the owner; `shared-edits.log`;
`/cl9 status` showing peers + lock; report summaries shown before build; wall-time line in F.

**0E Temporal interrogation.** HOUR 1: probe `agent_type` in hook stdin on 2.1.201 and
which shell runs hook commands on Windows (`python` on PATH via cmd vs Git Bash) — decide
the `command:` string; JSON registry schema. HOUR 2–3: renderer output format; SKILL.md
phase text; brief template with interface signatures; how the lead addresses a resumed
agent (by `name` = `cl9-<role>`; one per role per run). HOUR 4–5: lock semantics for
sessions that never heard of it (deny + explanation only when someone else holds it);
commandlet-only-when-closed sequencing; Bash sandbox PermissionError on some Content
paths → PowerShell. HOUR 6+: tests, dry-run F, H probe timing, I spike, docs, memory
pointers, TODOS.

**0F Mode.** SELECTIVE EXPANSION confirmed; approach B.

**CEO dual voices.**

CODEX SAYS (CEO — strategy challenge): `[codex-unavailable]` — "You've hit your usage
limit … try again at Sep 17th, 2026" (exit 1, 8 s). Tagged `[subagent-only]`.

CLAUDE SUBAGENT (CEO — strategic independence) — 15 findings, verdict: "build the lock,
the protocol doc, the tripwire hook and the contract generator now; the auto-dispatcher
is the smallest gain at the largest cost and should wait for Agent Teams or a real
compile-without-editor path." Disposition:
| # | finding (severity) | disposition |
|---|---|---|
| 1 | dispatcher fixes the cheapest pain; Amdahl on the build critical path (critical) | ACCEPTED: rollout reordered (A-parts first), wall-time metric added to F; dispatcher kept in scope → USER CHALLENGE at gate (scope order/cut) |
| 2 | agents write C++ blind; compile check without editor is the lever (critical) | ACCEPTED as H (probe then adopt) |
| 3 | never-guess + no-ask agents = user becomes scheduler (high) | ACCEPTED: all questions in intake; mid-run blockers only |
| 4 | path ownership contradicted by shared files (high) | ACCEPTED: file lists per brief, single-writer shared files, registry = default |
| 5 | parallel edits safe only when disjoint (high) | ACCEPTED (same amendment + lock for builders) |
| 6 | hook is a tripwire not a fence (high) | ACCEPTED: post-run audit is the truth source; tests reworded |
| 7 | commandlet while editor open = ghost-editor class (high) | ACCEPTED: MCP when open, commandlet only when closed |
| 8 | "less visibility" assumed (medium) | ACCEPTED partly: report-review gate + mode C default for editor-in-the-loop slices; the default itself → gate question 1 |
| 9 | `Saved/` is unsafe for run artifacts (medium) | ACCEPTED: `.claude/runs/` gitignored |
| 10 | lock file omitted (critical) | ACCEPTED as G, applied to every session |
| 11 | Agent Teams dismissed in four lines (high) | ACCEPTED as bounded spike I; dispatch layer decided by evidence → TASTE at gate |
| 12 | Workflow dismissed (medium) | REJECTED: its distinct benefit (resume) is covered by run.json + `/cl9 resume`; opt-in phrase per run is a real cost |
| 13 | worktree for text-only roles (medium) | REJECTED with rationale in Design (docs conflicts never occurred; Source must build in one tree) |
| 14 | reimplementation of Teams (high) | ACCEPTED in ordering: registry/hook/protocol first (survive either way) |
| 15 | lease + queue over path fencing (medium) | ACCEPTED: lock (G) is the lease; fencing stays for edits |

```
CEO DUAL VOICES — CONSENSUS TABLE:
═══════════════════════════════════════════════════════════════
  Dimension                           Claude  Codex  Consensus
  ──────────────────────────────────── ─────── ─────── ─────────
  1. Premises valid?                   partly   N/A    N/A (2 premises amended)
  2. Right problem to solve?           partly   N/A    N/A (lock/protocol yes; dispatcher = user challenge)
  3. Scope calibration correct?        no       N/A    N/A (reordered; dispatcher kept → gate)
  4. Alternatives sufficiently explored? no     N/A    N/A (Teams spike added)
  5. Competitive/market risks covered? partly   N/A    N/A (Teams obsolescence → survive-either-way order)
  6. 6-month trajectory sound?         partly   N/A    N/A (drift tests, sonnet rejected)
═══════════════════════════════════════════════════════════════
0/6 CONFIRMED (single voice — nothing can be confirmed); critical findings 1, 2, 10 flagged
regardless and absorbed.
```

**Section 1 — Architecture.** Dependency graph:
```
  cl9-roles.json ──▶ cl9_roles.py ──┬──▶ cl9_scope_guard.py ◀── settings.json PreToolUse
                                    ├──▶ render_roles.py ──▶ .claude/agents/cl9-*.md
                                    └──▶ tests
  cl9_lock.py ──▶ .claude/runs/editor.lock ◀── cl9_scope_guard.py (everyone)
  skills/cl9/SKILL.md ──▶ Agent(cl9-*) ──▶ .claude/runs/<slug>/{brief,report}-*.md, run.json
                     ──▶ MCP / Build.bat / Tools/Unreal (lead only, under lock)
  docs/workflow/sessions.md ◀── CLAUDE.md, memory pointers
```
Data flow — task text → intake → triage table → briefs → reports → audit → integration:
happy path above; nil (empty task) → intake asks; empty (triage finds no role) → "not a
`/cl9` task" and stop; error (a subagent aborts / no report) → one `SendMessage` nudge →
`ABORTED` for that role, user told, others continue. State machine in Design. Coupling:
the registry schema is shared by three consumers → one loader module (finding 1.1,
accepted). SPOF: the lead's context → `run.json` + `resume` (1.2, accepted). Scaling:
5 roles is the ceiling by design (one editor); 10× tasks/day only stresses the lock queue,
which is FIFO by `since`. Security architecture: hook and lock are local scripts run as
the user; MCP calls denied to role agents. Production failure: two sessions both believe
they hold the lock after a clock skew → lock is a single file with `session_id`; the hook
compares the caller's `session_id` from stdin, so ownership is exact, not time-based;
expiry only frees. Rollback: delete `.claude/{agents,skills,hooks,cl9-roles.json,runs}`
and the `hooks` block in settings.json — zero runtime footprint; `git revert` of the
commit. Beautiful-architecture check: one JSON, one hook, one skill, files on disk any
human can read — a new engineer sees the whole mechanism in `docs/workflow/sessions.md`.
Findings: 1.1 loader module (ACCEPTED, P4); 1.2 resume (ACCEPTED, P1); 1.3 `Saved/` →
`.claude/runs/` (ACCEPTED from voice #9).

**Section 2 — Error & Rescue map.** See registry below. Rule check: no catch-all — the
hook's outer `try` is deliberate fail-open for non-role callers and fail-closed for role
callers, both with a message; every rescued error is user-visible in the tool result or
in `denied.log`. LLM-specific: a role agent returning no report / a malformed report
(missing sections) → lead validates the seven headers and nudges once; a hallucinated
file path in "바꾼 파일" → post-run audit compares to `git status`, mismatch reported.

**Section 3 — Security & threat model.** New surface: a PreToolUse hook executing python
on every matched call in every session of this repo (local code, tracked in git — same
trust as `Tools/`); role agents with `Bash`/`PowerShell` under bypass permissions can
still write anywhere (tripwire + audit, accepted residual for a solo repo); denied
verbs cover destructive git and recursive deletes (finding 3.1, ACCEPTED). Prompt
injection via repo docs: unchanged from today. Secrets: none. Dependencies: python 3.9
stdlib only. Audit trail: `denied.log`, `shared-edits.log`, `run.json`. Threats: (a) a
role agent bypasses the hook with `python -c` — likelihood Med, impact Low, mitigated by
audit; (b) stale lock blocks a build — likelihood Med, impact Low, mitigated by expiry
and `--force` with the owner named; (c) a session ignorant of the lock kills the editor
via a tool the matcher misses (e.g. `Stop-Process`) — likelihood Low, impact Med →
finding 3.2: add `Stop-Process.*Unreal` to the lock verbs (ACCEPTED).

**Section 4 — Data flow & interaction edge cases.**
```
  INPUT(task) ─▶ VALIDATION(intake) ─▶ TRANSFORM(triage) ─▶ PERSIST(briefs) ─▶ OUTPUT(reports)
   [empty]→ask   [ambiguous]→ask       [0 roles]→stop        [.claude/runs   [missing]→nudge
   [feature-     [conflicts with       [1 role]→no AUQ,       missing]→mkdir  [malformed]→
    shaped]→     live peer's files]→    run directly          [slug dup]→      validate 7
    /autogoal    ask                    [>5 roles]→impossible  suffix -2        sections
```
Interaction edge cases: user interrupts mid-dispatch → agents keep running; `/cl9 status`
shows them; `TaskStop` documented. Two roles edit one shared header concurrently → made
impossible by single-writer assignment; if it happens anyway (manual + agent), the Edit
tool's exact-match fails and the agent re-reads (4.1, no change). Same slug twice →
timestamp suffix. Lock expires mid-build → the builder re-acquires; the hook only checks
at verb time, so a running detached build is never killed by the lock (4.2, documented).
Peer session that never acquires the lock kills the editor → the hook denies only *their*
`taskkill` when *someone else* holds the lock; a peer holding nothing is free — by design
(no lock = today's behavior). `ListAgents` empty → skip peer notify.

**Section 5 — Code quality.** DRY: one loader; agents rendered. Naming: `cl9-<role>`,
`/cl9`, `cl9_scope_guard.py`, `cl9_lock.py`, `render_roles.py`. Over-engineering check:
`pie` letters in the registry are optional strings — fine; `run.json` state machine has 9
states — keep, each has a distinct next action. Under-engineering: Windows path case and
backslashes (5.1, ACCEPTED: normalize + test); `maxTurns` unstated → 200 (5.2, ACCEPTED,
P5 explicit). Cyclomatic: `guard.main` split into `classify_path()`, `check_command()`,
`check_lock()` (5.3, ACCEPTED). No further issues.

**Section 6 — Test review.** Diagram:
```
  NEW UX FLOWS:      /cl9 <task> (intake→triage→dispatch→integration), /cl9 role, status,
                     resume, handoff, commit
  NEW DATA FLOWS:    registry→agents (render), registry→hook, briefs→agents→reports→audit,
                     lock file→hook
  NEW CODEPATHS:     guard: lead/role split, owns/shared/append_only/lead_only/outside,
                     verb regex, lock check, fail-open/closed; lock: acquire/release/
                     status/expiry/force; renderer; audit (git status vs briefs)
  NEW ASYNC WORK:    background role agents; detached build (existing)
  NEW INTEGRATIONS:  Agent tool spawn/resume, MCP (lead), Build.bat (lead), Teams spike
  NEW ERROR PATHS:   see registry (10 rows)
```
Coverage: hook + lock + registry + renderer → unit (★★★ target: happy, denied, edge,
malformed); skill flows → the dry-run F [→E2E] and a negative dry-run (role asked to do
out-of-scope work must put it in 요청, not edit) [→EVAL — agent bodies and SKILL.md are
prompts]; H and I are recorded probes. Friday-2am test: another session holds the lock
and this one runs the build protocol → must be refused with owner+expiry (in success
criteria 2). Hostile QA: `Write` with a path like `..\\..\\Source\\CL9\\Items\\x.cpp`
from `cl9-ui` → normalized and denied (test). Chaos: delete `editor.lock` mid-build →
nothing breaks (hook only gates verbs). Flakiness: F depends on editor state — run with
the editor closed first. Load: hook cost ≈ python startup per matched call (~50 ms) in
every session — measured in F; if > 100 ms, narrow the matcher to `Edit|Write|Bash|
PowerShell|mcp__unreal-mcp__call_tool` (6.1, ACCEPTED as a measurement). No gaps left
untested except column-level art scope (accepted, audit).

**Section 7 — Performance.** Hook latency (above). Token cost: five parallel `inherit`
agents each reading their docs — briefs point to sections, not files; idle roles not
spawned; `maxTurns` 200. Build wall time unchanged (one batch build) unless H proves out.
Lock queue: FIFO by `since`, a waiting `acquire` polls every 5 s with a timeout. No N+1,
no DB.

**Section 8 — Observability.** Logs: `.claude/runs/<slug>/{run.json, brief-*, report-*,
shared-edits.log, denied.log, handoff.md}`; `editor.lock`. Metrics in `run.json`: wall
time, roles, denials, build count, PIE result, audit violations. Dashboard: `/cl9
status`. Three weeks later: `run.json` + reports reconstruct a run. Runbooks in
`sessions.md`: ghost editor, build failure → owning role, silent agent, stale lock. Joy to
operate: `status` shows who holds the editor and why — the question every session asked
by SendMessage.

**Section 9 — Deployment & rollout.** No migrations. Feature flag = presence of the
files; rollback = delete. Risk window: the hook lands in tracked `settings.json` → the 4
live sessions start running it on every matched call immediately; fail-open for them, and
the only new refusal is a lock-held `taskkill`/build — which is the intended behavior.
Post-deploy check (first 5 min): one `Edit` in the lead with no delay/error; unit tests
green; `cl9_lock.py status` prints "free". Rollout order in the plan (1→6) puts the
zero-behavior-change parts first.

**Section 10 — Long-term trajectory.** Debt: prompt text in agent bodies (kept short,
rules by pointer); registry maintenance (tested). Path dependency: none — every piece is
deletable. Knowledge: `docs/workflow/sessions.md`. Reversibility 5/5. Ecosystem fit:
aligned with Claude Code's direction (role files → team members). What comes after: Teams
as dispatch (I), scripted PIE smoke (TODO), commit-by-role. Retrospective on cherry-picks:
all ten accepted items are S-effort and in radius; the two deferred (scripted PIE,
naming) are not load-bearing for the accepted set.

**Section 11 — Design & UX.** SKIPPED (no UI scope; `/cl9` output is terminal text and
files).

**Error & Rescue Registry.**
```
  METHOD/CODEPATH               | WHAT CAN GO WRONG                     | CLASS / SIGNAL
  ------------------------------|---------------------------------------|----------------------------
  cl9_scope_guard (stdin parse) | malformed / missing JSON              | ValueError → exit 0 lead / exit 2 role
  cl9_scope_guard (registry)    | cl9-roles.json missing or invalid     | RegistryError → exit 2 role, msg
  cl9_scope_guard (path)        | path outside repo / unknown role      | exit 2 with owner or "unknown role"
  cl9_scope_guard (lock)        | lock file corrupt                     | treated as free + warning line
  cl9_lock acquire              | held by other, not expired            | wait/poll up to timeout → "held by X until T"
  cl9_lock release              | not owner                             | refused unless --force (owner named)
  /cl9 dispatch (Agent tool)    | spawn fails                           | ToolError → offer mode C
  role agent                    | no report / malformed report          | validate 7 sections → nudge once → ABORTED
  integration build             | UBT error                             | log parsed → owning role via SendMessage → rebuild
  integration MCP               | modal dialog hangs MCP                | existing WM_CLOSE recipe; timeout → user
  commandlet apply              | editor still open                     | refused by sequence check (lock purpose)

  CLASS                         | RESCUED? | RESCUE ACTION                     | USER SEES
  ------------------------------|----------|-----------------------------------|---------------------------
  malformed stdin (lead)        | Y        | allow                             | nothing
  malformed stdin (role)        | Y        | deny + message                    | agent reports "hook error"
  RegistryError                 | Y        | deny role edits                   | "레지스트리 깨짐: <path>"
  lock held                     | Y        | wait then refuse with owner       | "cl9-ui-lead가 빌드 중, 14:05까지"
  spawn failure                 | Y        | fall back to contracts            | AskUserQuestion
  missing report                | Y        | nudge → ABORTED role              | report in chat
  UBT error                     | Y        | route to owner, rebuild           | build log excerpt
  MCP hang                      | Y        | WM_CLOSE / timeout                | "에디터 대화상자 닫음" or ask
  commandlet with editor open   | Y        | refuse                            | "에디터 닫힌 창에서만"
```
No GAP rows.

**Failure Modes Registry.**
```
  CODEPATH              | FAILURE MODE                  | RESCUED? | TEST? | USER SEES?      | LOGGED?
  ----------------------|-------------------------------|----------|-------|-----------------|--------
  guard path check      | role edits foreign file       | Y        | Y     | denial msg      | denied.log
  guard verbs           | role runs taskkill/build      | Y        | Y     | denial msg      | denied.log
  guard bypass          | Set-Content/python -c writes  | N (by    | Y(F)  | audit report    | run.json
                        |                               | design)  |       |                 |
  lock                  | stale lock after crash        | Y(expiry)| Y     | owner+expiry    | lock file
  lock                  | peer never acquires           | partial  | Y     | (today's rules) | —
  dispatch              | agent silent                  | Y        | Y(F)  | ABORTED notice  | run.json
  integration           | build fails                   | Y        | manual| log excerpt     | build log
  integration           | ghost editor                  | Y        | manual| runbook         | —
  resume                | run.json corrupt              | Y        | Y     | "재개 불가: …"   | —
```
No row is RESCUED=N ∧ TEST=N ∧ silent → 0 CRITICAL GAPS. "peer never acquires" is the
deliberate compatibility mode for sessions that predate the lock.

**What already exists** (reused, not rebuilt): `/autogoal` intake; memory build/editor
protocol; `pie-scenarios.md`; `Tools/Unreal/*.py`; `ListAgents`/`SendMessage`;
`/freeze` hook mechanism; agent file format; `.gitignore` conventions.

**NOT in scope** — see the section above (plus: column-level enforcement, Workflow
variant, scripted PIE smoke, session naming, GUI).

**Dream state delta** — after this plan: incidents structurally prevented, contracts
generated, resume/handoff scripted, dispatch available for editor-free work; still
manual: PIE by hand, one build per run, Teams pending the spike.

**TODOS.md updates proposed** (written by the Eng phase): scripted PIE smoke per role;
session naming probe; Workflow-script variant; Agent Teams migration if spike I passes.

**Implementation tasks (CEO phase)** — written to
`~/.gstack/projects/Work/tasks-ceo-review-<ts>.jsonl`; aggregated at the gate.

```
  +====================================================================+
  |            MEGA PLAN REVIEW — COMPLETION SUMMARY (CEO)             |
  +====================================================================+
  | Mode selected        | SELECTIVE EXPANSION (auto)                  |
  | System Audit         | 189 dirty paths / 4 sessions; 1 stash; 0 TODO markers |
  | Step 0               | 8 premises (2 amended, 2 → gate); approach B |
  | Section 1  (Arch)    | 3 issues found (all accepted)               |
  | Section 2  (Errors)  | 11 error paths mapped, 0 GAPS               |
  | Section 3  (Security)| 2 issues found, 0 High severity             |
  | Section 4  (Data/UX) | 9 edge cases mapped, 0 unhandled            |
  | Section 5  (Quality) | 3 issues found                              |
  | Section 6  (Tests)   | Diagram produced, 1 measurement gap (hook latency) |
  | Section 7  (Perf)    | 0 issues (1 measurement)                    |
  | Section 8  (Observ)  | 0 gaps (run.json added earlier)             |
  | Section 9  (Deploy)  | 1 risk flagged (live sessions), mitigated   |
  | Section 10 (Future)  | Reversibility: 5/5, debt items: 2           |
  | Section 11 (Design)  | SKIPPED (no UI scope)                       |
  +--------------------------------------------------------------------+
  | NOT in scope         | written (6 items)                           |
  | What already exists  | written                                     |
  | Dream state delta    | written                                     |
  | Error/rescue registry| 11 codepaths, 0 CRITICAL GAPS               |
  | Failure modes        | 9 total, 0 CRITICAL GAPS                    |
  | TODOS.md updates     | 4 items proposed                            |
  | Scope proposals      | 14 proposed, 10 accepted, 3 deferred, 1 rejected |
  | CEO plan             | written (ceo-plans/2026-09-12-session-orchestration.md) |
  | Outside voice        | ran (claude subagent only; codex usage limit) |
  | Lake Score           | 10/10 recommendations chose the complete option |
  | Diagrams produced    | 5 (architecture, data flow, state machine, dream, alternatives) |
  | Stale diagrams found | 0 (no ASCII diagrams in touched files)      |
  | Unresolved decisions | 0 mid-run (4 queued for the gate)           |
  +====================================================================+
```

> **Phase 1 complete.** Codex: unavailable. Claude subagent: 15 issues (12 accepted,
> 2 rejected with rationale, 1 partly). Consensus: 0/6 confirmed (single voice); 3
> critical findings absorbed; 2 items → gate (dispatch mode default, ship order) plus 2
> premise items (role taxonomy, settings file). Passing to Phase 2.5 (DX).

### Phase 2.5 — DX review (mode: DX POLISH, auto-selected: enhancement of an existing
workflow; product type: Claude Code Skill + two CLI scripts; no prior DX reviews)

**0A Developer persona** (inferred from the logs, auto-decided P6 — the product has one
developer):
```
TARGET DEVELOPER PERSONA
========================
Who:       the CL9 author — solo UE 5.8 game developer driving Claude Code in 2–6 terminals
Context:   opens a terminal, wants a role or a task running within a minute; types Korean;
           bypass-permissions mode; four sessions already open when this lands
Tolerance: one wrong step and the denial is read as a bug; two prompts before "왜 물어봐"
Expects:   the contract it used to type by hand, the lock to Just Work, Korean messages
```

**0B Empathy narrative.** I open a new terminal in `D:\Work\unreal\CL9` and type
`/cl9 role ui`. It prints the UI contract in Korean — the same paths I used to type by
hand, plus three lock commands I have not seen before. I start editing `WBP_HUD`; later I
want to build, run `taskkill`, and get "cl9-2d 세션이 빌드 중(14:05까지) — 끝나면 상대가
release" — that is the message I used to wait for over SendMessage. In the lead terminal
I type `/cl9 HUD 스태미나 바 색 문턱을 표로`; it asks me one question (which table), shows
a two-row split, I press A; two agents run; the summaries come back; I press A again; it
takes the lock, tells the others, kills the editor, builds two minutes, restarts, runs PIE
section C and lists the files per role. Where I would get lost: the first time the hook
refuses *my* taskkill in a session I did not restart (nothing happens — the old session
never loaded the hook), and finding the reports (`.claude/runs/<slug>/`).

**0C Competitive benchmark** (Aside not installed; WebSearch used for the landscape,
reference tiers for TTHW):
```
COMPETITIVE DX BENCHMARK
=========================
Tool                        | TTHW        | Notable DX choice                    | Source
gstack /freeze              | < 1 min     | one command, one hook, one state file | skill source
Claude Code Agent Teams     | ~2 min      | env var + prompt; shared task list    | docs (agent-teams)
plain custom agent (.md)    | ~2 min      | one file, no framework                | docs (sub-agents)
/cl9 role (this plan)       | ~1 min*     | contract generated from registry      | this plan
/cl9 <task> (this plan)     | ~3 min to split; 45–90 min to first verified run (build-dominated)
* after the one-time restart of the session so the hook loads
```
Target tier auto-decided: Champion for `role`/`status` (< 2 min), Competitive for the
first dispatched run to the split table (2–5 min); the build wall time is not a DX
number this plan can move except through H.

**0D Magical moment.** The first refused `taskkill` that names the owner and the expiry —
the incident of 09-07 not happening — and `/cl9 status` answering "who has the editor and
why" without a SendMessage round-trip. Delivery vehicle: copy-paste demo command
(`/cl9 status`, `/cl9 role ui`) — lowest effort (P5).

**0E Mode.** DX POLISH.

**0F Developer journey trace.**
```
STAGE           | DEVELOPER DOES                         | FRICTION POINTS                                    | STATUS
----------------|----------------------------------------|----------------------------------------------------|--------
1. Discover     | reads CLAUDE.md pointer → sessions.md  | CLAUDE.md is terse by rule; one line is enough     | ok
2. Install      | nothing — files are in the repo        | hooks load only at session start (voice #1);       | fixed (rollout 1 +
                |                                        | python vs python3 (voice #2); permission prompts   |  status marker;
                |                                        | for the helper scripts (voice #3)                  |  verbatim cmd; allowlist)
3. Hello World  | /cl9 status · /cl9 role ui             | /cl9 with no args; keyword vs task ambiguity (#7)  | fixed (usage; arity rule)
4. Real Usage   | /cl9 <task> → questions → split → run  | too many questions for small tasks; brief-granted  | fixed (single-role skips
                |                                        | file denied by registry (#5); lock owner unknown   |  AUQ; grants.json;
                |                                        | for a Bash-run acquire (#6)                        |  session_id via hook)
5. Debug        | reads a denial; /cl9 status            | no doc anchor, denied.log never surfaced (#16);    | fixed (anchors; counts;
                |                                        | lead-side hook errors silent (#11); MCP deny       |  exit 1 + stderr;
                |                                        | unparseable (#12); silent agent undefined (#14)    |  call_tool parse; 10-min nudge)
6. Upgrade      | edits registry → render → tests        | Claude Code field renames; registry drift          | ok (probe; coverage +
                |                                        |                                                    |  hash check; version note)
```

**0G First-time developer roleplay.**
```
FIRST-TIME DEVELOPER REPORT
============================
Persona: the CL9 author, an already-open session
T+0:00  types /cl9 → usage in Korean. Fine.
T+0:30  /cl9 role ui → contract printed; notices "잠금" commands; skims.
T+1:00  another terminal holds the lock; tries taskkill → passes (!) because this session
        was open before the hook landed. Confused: "잠금이 왜 안 걸려?" → /cl9 status says
        "이 세션은 훅 미적용 — 세션을 다시 열어라". Restarts.
T+2:00  /cl9 HUD 스태미나 … → one question → split table → A.
T+3:00  reports arrive; summaries shown; A → build under lock → PIE → file lists.
T+6:00  succeeded. Asks where the reports are → .claude/runs/<slug>/ named in the report.
```
All confusion points are addressed in the amended plan (auto-decided "fix all", P1).

**DX dual voices.**

CODEX SAYS (DX — developer experience challenge): `[codex-unavailable]` (usage limit).

CLAUDE SUBAGENT (DX — independent review) — 19 findings, TTHW estimate "role: 4 steps
≈ 5 min; first dispatched run 45–90 min, build-dominated", score 4/10 with findings 1, 5,
6 making "the first real run fail". Disposition (all accepted unless noted):
| # | finding (severity) | disposition |
|---|---|---|
| 1 | hooks snapshotted at session start → live sessions unprotected while the lead believes otherwise (critical) | ACCEPTED: rollout step 1 restart; hook-seen marker; `/cl9 status` reports hook active |
| 2 | interpreter unspecified; `python3` is the Store stub (high) | ACCEPTED: verbatim `python .claude/hooks/…`; startup self-test in the suite |
| 3 | helper scripts cost permission prompts (medium) | ACCEPTED: allowlist entries in the same commit |
| 4 | first mistake: un-restarted session reads denials as bugs | ACCEPTED via #1 |
| 5 | brief-granted file denied by registry-only hook (critical) | ACCEPTED: `grants.json` ∪ registry; `current/<session_id>` slug resolution |
| 6 | lock owner identity for a Bash-run acquire (critical) | ACCEPTED: hook rewrites `--session` via `updatedInput` (probe) or performs the acquire itself |
| 7 | positional parsing ambiguity (high) | ACCEPTED: keyword + arity rule; `task`/`--` escapes; usage on no args |
| 8 | inconsistent grammar (medium) | ACCEPTED: one synopsis block, purposes `build|compile|pie|commandlet` |
| 9 | defaults + Korean aliases (medium) | ACCEPTED |
| 10 | registry syntax is prose; 3.9 has no `**` (high) | ACCEPTED: fnmatch dialect, `except`/`note` keys, no braces |
| 11 | lead-side internal errors swallowed (high) | ACCEPTED: exit 1 + stderr |
| 12 | MCP denial cannot work as written (high) | ACCEPTED: `mcp__unreal-mcp__call_tool` + `tool_input.tool_name`; unit case |
| 13 | 20-min expiry < build time (high) | ACCEPTED: 60 min + PID liveness |
| 14 | no message for commandlet-with-editor / malformed report / stale render / silent agent (high) | ACCEPTED: tasklist check, validator, hash check, 10-min nudge + per-role `last_write` |
| 15 | background permission stalls unprobed (medium) | ACCEPTED: folded into H |
| 16 | denials lack doc anchor; denied.log unsurfaced (medium) | ACCEPTED |
| 17 | sessions.md lacks copy-paste blocks (medium) | ACCEPTED |
| 18 | no escape hatches / kill switch (high) | ACCEPTED: registry `model`/`max_turns`/`default_mode`, `--mode`/`--roles`, `guard.off` file switch (env var rejected — file works from any session on Windows) |
| 19 | `settings.local.json` is tracked; question 4 moot (—) | ACCEPTED partly: hook in tracked settings.json; untracking is a separate TODO |

```
DX DUAL VOICES — CONSENSUS TABLE:
═══════════════════════════════════════════════════════════════
  Dimension                           Claude  Codex  Consensus
  ──────────────────────────────────── ─────── ─────── ─────────
  1. Getting started < 5 min?          no→yes*  N/A    N/A (fixed: restart note, allowlist)
  2. API/CLI naming guessable?         partly   N/A    N/A (fixed: synopsis, arity rule)
  3. Error messages actionable?        no→yes*  N/A    N/A (fixed: anchors, exit 1, parse)
  4. Docs findable & complete?         partly   N/A    N/A (fixed: copy-paste blocks)
  5. Upgrade path safe?                yes      N/A    N/A
  6. Dev environment friction-free?    partly   N/A    N/A (fixed: python, prompts; probe)
═══════════════════════════════════════════════════════════════
0/6 CONFIRMED (single voice); * = after the amendments above. Critical findings 1, 5, 6
flagged regardless and absorbed into the plan body.
```

**Pass 1 — Getting started: 5/10 → 9/10.** It was a 5 because a user in an un-restarted
session would see no lock at all (voice #1) and the helper scripts would prompt three
times (#3); a 10 for THIS product is "restart, type one command, see the contract".
Fixed: rollout restart note + `status` marker, allowlist, verbatim interpreter, usage
on no args. Remaining gap: the one-time restart itself (unavoidable — hooks load at
start). Stripe test: `/cl9 role ui` in one terminal session without leaving it — yes.

**Pass 2 — API/CLI design: 6/10 → 9/10.** Verbs are consistent (`role/status/resume/
handoff/commit`), but `/cl9 status 표시…` was ambiguous (#7) and the lock grammar was
inconsistent (#8); defaults were missing (#9). Fixed: arity rule + `task`/`--` escapes,
one synopsis, defaults (`resume` latest, `commit` adopted role, `role` lists), Korean
aliases. Remaining: `--mode`/`--roles` are the only flags; progressive disclosure holds
(one command for a simple task). One-example test: after seeing `/cl9 role ui` once, the
user can guess `/cl9 role map` — yes.

**Pass 3 — Error messages: 4/10 → 9/10.** Denials named the owner but not the doc; MCP
denial was unimplementable as written; lead-side errors were silent; silent agents had no
timeout. Fixed: every denial = what + why + fix + `sessions.md#anchor` (Tier 1/Elm shape,
Korean), `call_tool` parsing, exit 1 + stderr for lead errors, validator listing missing
headings, `acquire commandlet` naming the editor PID, 10-min nudge. Three traced paths:
(a) role edits foreign file → "`…CL9Item.cpp`는 gameplay 영역 — 보고서 '다른 역할에
요청'에 적어라. sessions.md#거부"; (b) lead taskkill under lock → "cl9-2d(빌드)가
14:05까지 잠금 — 끝나면 상대가 release, 죽었으면 release --force. sessions.md#잠금";
(c) commandlet with editor open → "에디터가 떠 있다(PID 1234) — 커맨드릿은 에디터를 닫은
창에서만". Debug mode: `denied.log`, `shared-edits.log`, `status` counts.

**Pass 4 — Documentation: 6/10 → 8/10.** `sessions.md` existed as a list of rules; now
carries copy-paste brief/report/lock/resume blocks and the anchors denials link to.
Remaining gap (accepted): no interactive playground — the dry-run F is the tutorial.

**Pass 5 — Upgrade & migration: 8/10.** Registry edits are tested (coverage + renderer
equality + hash check); Claude Code field renames are guarded by the probe and the
"verified on 2.1.201" note; everything is deletable (reversibility 5/5). No codemod
needed — there is no prior version.

**Pass 6 — Developer environment: 6/10 → 8/10.** Windows-only by nature; `python` 3.9
user install named verbatim; Bash-sandbox `Content/` writes → PowerShell noted in agent
bodies; permission stalls for background subagents probed in H. Non-interactive: the
hook and lock scripts are plain stdin/argv programs, unit-testable without Claude Code.

**Pass 7 — Community & ecosystem: 6/10.** Public GitHub repo; no plugin surface and none
needed for one developer; the pattern (registry + lock + role agents) is documented well
enough to lift into the next project. Not raised further (P3).

**Pass 8 — Measurement: 7/10 → 8/10.** `run.json` records wall time, roles, denials,
build count, PIE result, audit violations; F records wall time vs serial; `status`
shows denial counts. Friction audit cadence: TODO "after five runs, read the run.json
files and prune what nobody used".

**Claude Code Skill DX checklist.** AskUserQuestion design — one issue per call,
re-grounded ✓. State storage — per-project `.claude/runs/`, append-only logs ✓.
Progressive consent — one report-review gate per run, single-role tasks skip the split
question ✓. Auto-upgrade — n/a (repo files) ✓. Skill composition — calls `/autogoal`,
may call `/freeze` in manual mode ✓. Error recovery — `resume`, partial reports kept ✓.
Session continuity — `handoff`, `work-progress` ✓. Bounded autonomy — no commit, no
editor for role agents, lock for everyone, `guard.off` kill switch ✓.

```
DX SCORECARD (after amendments)
================================
  Dimension                 | Before | After | Gap to 10
  Getting started           |   5    |   9   | one-time session restart
  API/CLI design            |   6    |   9   | only two flags — fine
  Error messages            |   4    |   9   | column-level scope not diagnosable (audit only)
  Documentation             |   6    |   8   | no playground; F is the tutorial
  Upgrade path              |   8    |   8   | Claude Code field renames (probe guards)
  Dev environment           |   6    |   8   | Windows-only; permission stalls probed in H
  Community                 |   6    |   6   | n/a for one developer
  Measurement               |   7    |   8   | no cadence for friction audits (TODO)
  OVERALL                   |   6    |   8   |
TTHW: role/status ~1 min after restart (Champion); first dispatched run to split ~3 min
(Competitive); first verified run 45–90 min (build-dominated; H is the lever).
```

**DX implementation checklist** (absorbed into the plan body): restart note + hook-seen
marker; verbatim `python` command; allowlist entries; usage + arity parsing; synopsis;
defaults + Korean aliases; `grants.json` + `current/<session_id>`; lock owner via
`updatedInput`/hook-performed acquire; 60-min + PID; commandlet tasklist check; report
validator; hash check; 10-min nudge; exit 1 + stderr; `call_tool` parse; doc anchors;
denial counts; copy-paste blocks; registry escape fields; `--mode`/`--roles`; `guard.off`.

**Implementation tasks (DX phase)** — written to
`~/.gstack/projects/Work/tasks-devex-review-<ts>.jsonl`; aggregated at the gate.

> **Phase 2.5 complete.** DX overall: 6/10 → 8/10 after amendments. TTHW: ~5 min → ~1
> min (role) after the one-time restart; first dispatched split ~3 min. Codex:
> unavailable. Claude subagent: 19 issues (18 accepted, 1 partly). Consensus: 0/6
> confirmed (single voice); 3 critical findings absorbed. Passing to Phase 3 (Eng Review —
> the required gate reviews the final amended plan).

### Phase 3 — Eng review (FULL REVIEW; reviews the plan as amended by Phases 1 and 2.5)

**Step 0 — Scope challenge.** Existing code read for this phase: `.claude/settings.json`
(permissions only; global `~/.claude/settings.json` already runs a `pixel-agents` node
hook on every PreToolUse/PostToolUse and gstack's `bash /c/…` Stop hook — the hook
mechanism and its per-call latency are already in production here); gstack
`freeze/bin/check-freeze.sh` (deny-tier PreToolUse hook: reads stdin, prints
`hookSpecificOutput.permissionDecision=deny`, fails **closed** with a backstop trap —
the shape the scope guard copies); `Tools/Unreal/run_python.py` (remote execution into
the one running editor, fails if not exactly one `CL9` node — the "one editor" fact in
code), `Tools/Unreal/mcp_client.py` (raw JSON-RPC to port 8000), `Tools/Loc/sync_strings.py`
(module docstring in Korean, `ROOT` from `__file__`, `sys.exit` with a message — the
house style for python tools); memory `editor-kill-allowed.md` (the protocol the lock
encodes). Sub-problem → existing code: intake → `/autogoal`; fence → `/freeze` shape;
editor scripts → `Tools/Unreal`; peer messaging → built-in; PIE checks →
`pie-scenarios.md`; nothing rebuilt. Minimum set: registry + lock + hook + `sessions.md`
+ `/cl9 role` (rollout 1–2) — the dispatcher is the increment on top, ordered last.
Complexity check: ~16 files, 0 new C++ classes, 3 new python scripts + 1 loader module
(2+ new "services" → smell triggered; resolution: each script is one responsibility with
a unit test and could be deleted independently — held, per P2 never-reduce). Search
check [Layer 1]: file lock with owner + expiry + steal is the standard advisory-lock
pattern; [Layer 2] Claude Code hooks doc: PreToolUse `updatedInput` exists, hooks
snapshot at start; [Layer 3] the PID-liveness extension is the eureka for this repo —
Live Coding builds outlive any sane expiry. TODOS cross-reference: none block; six new
TODOs written (Workflow section). Completeness: complete version chosen everywhere
(tests, audit, kill switch, resume). Distribution: repo files, no artifact pipeline
needed.

**Eng dual voices.**

CODEX SAYS (eng — architecture challenge): `[codex-unavailable]` (usage limit).

CLAUDE SUBAGENT (eng — independent review):
29 findings, verdict: "registry/renderer/role split are sound; the lock (non-atomic,
opt-in polarity, save_assets denial, bare PID) and the audit (no baseline) as written
would recreate the incidents they exist to prevent — fix 1–3, 9, 11 before rollout
step 1." Disposition (confidence as given by the reviewer):
| # | finding (severity, confidence) | disposition |
|---|---|---|
| 1 | lock acquire not atomic (critical, 9) | ACCEPTED: `O_CREAT|O_EXCL` → validate → tmp + `os.replace` → re-read confirm |
| 2 | lock polarity opt-in — an MCP-driving session that never locked gets killed (high, 8) | ACCEPTED: killer must hold the exclusive lock; shared `editor` leases renewed automatically by the hook on every `call_tool` |
| 3 | denying peers' `save_assets` is backwards (high, 8) | ACCEPTED: `save_assets` never denied |
| 4 | bare-PID liveness; Windows recycles PIDs (high, 7) | ACCEPTED: process triple (pid, create time, image); `--force` refuses while alive |
| 5 | slug via `current/<session_id>` rests on an unprobed claim (high, 7) | ACCEPTED: resolve by `agent_type` → latest non-terminal run; `session_id` only for lock ownership |
| 6 | hook writing `run.json` clobbers the lead (medium, 8) | ACCEPTED: hook touches `touch-<role>`; `run.json` lead-only |
| 7 | exit 1 + stderr invisible to the model; `guard.off` never expires (medium, 8) | ACCEPTED: `permissionDecision: "ask"` (probe under bypass; fallback log + status); `guard.off` TTL 60 min shown by status |
| 8 | `updatedInput` + allow lets `…; git reset --hard` ride through (high, 7) | ACCEPTED: `updatedInput` dropped; hook performs acquire/release itself on an exactly-parsed simple command |
| 9 | audit vs 189 pre-existing dirty files; concurrent leads; duplicate grants (critical, 9) | ACCEPTED: baseline snapshot at DISPATCHED (porcelain + `baseline.patch` + mtime/hash), delta audit, grant-overlap refusal |
| 10 | ABORTED role keeps editing (medium, 8) | ACCEPTED: terminal-state deny + `TaskStop` |
| 11 | Bash denylist leaky and noisy — `git restore .` unrecoverable (critical, 8) | ACCEPTED: allowlist for roles, single simple commands only; `baseline.patch` as recovery |
| 12 | `fnmatch` re-applies `normcase`; `..`/junctions/drive case; `MultiEdit`/`notebook_path` missing (medium, 8) | ACCEPTED: `realpath`+`normcase`+`commonpath`, `fnmatchcase`, matcher + inputs extended |
| 13 | Korean-locale `tasklist` text (medium, 7) | ACCEPTED: `/FO CSV /NH` row count |
| 14 | `--wait` vs the 120 s Bash tool timeout (medium, 7) | ACCEPTED: cap 90 s, "busy" return |
| 15 | `.claude/runs/` deleted / malformed grants → deny-all (medium, 7) | ACCEPTED: `makedirs`, distinct reason, malformed = empty + log |
| 16 | missing tests: race, PID reuse, idempotence, force vs live, terminal deny, overlap, dirty baseline, compound commands, regex corpus, latency, Korean tasklist, real fixtures (—) | ACCEPTED: all added to the suite |
| 17 | hook-active marker by mtime is flaky; wall-time is one sample (—) | ACCEPTED: nonce; recorded not gated |
| 18 | `acquire build` without `--pid` expires mid Live-Coding build (high) | ACCEPTED: `--pid` required for build/commandlet; 90 min default |
| 19 | game-target compile is a syntax check (`WITH_EDITOR=0`); UBT mutex still serializes (medium) | ACCEPTED: stated in H and in agent bodies |
| 20 | `python` resolves from cwd under cmd.exe (low-medium, 6) | ACCEPTED: `py -3` launcher, verified in the probe |
| 21 | audit blind spots: `guard.off`, edits to `.claude/`/`Config/` via `python -c` (medium, 8) | ACCEPTED: audit checks both |
| 22 | reports are an injection channel (medium, 7) | ACCEPTED: report fields are data; lead re-states requests; lead writes commit messages |
| 23 | role agents inherit bypass; regex is the only barrier (—) | PARTLY: `permissionMode` left inherited — a prompting mode would stall background agents (the "silent agent" cause); the allowlist (#11) + audit are the barrier, stated in the plan |
| 24 | `updatedInput` semantics unverified (8) | ACCEPTED by dropping it (#8) |
| 25 | hook shell on Windows / PATH order (—) | ACCEPTED: probe in the hook shell (success criterion 0) |
| 26 | `session_id` after `/clear`/`/resume` (—) | ACCEPTED via #5; `--force` path documented |
| 27 | `UnrealEditor-Cmd.exe` missed by the commandlet guard (—) | ACCEPTED: both image names |
| 28 | Teams spike must first capture a teammate's hook stdin (—) | ACCEPTED |
| 29 | mid-pattern permission glob unverified (—) | ACCEPTED: scripts listed explicitly |

```
ENG DUAL VOICES — CONSENSUS TABLE:
═══════════════════════════════════════════════════════════════
  Dimension                           Claude  Codex  Consensus
  ──────────────────────────────────── ─────── ─────── ─────────
  1. Architecture sound?               partly   N/A    N/A (lock/audit redesigned: atomic, polarity, leases, baseline)
  2. Test coverage sufficient?          no→yes*  N/A    N/A (12 test classes added)
  3. Performance risks addressed?       yes      N/A    N/A (latency budget test added)
  4. Security threats covered?          partly   N/A    N/A (allowlist, report-as-data, audit blind spots)
  5. Error paths handled?               partly   N/A    N/A (ask channel, distinct reasons, terminal deny)
  6. Deployment risk manageable?        partly   N/A    N/A (probe first, restart note, --pid required)
═══════════════════════════════════════════════════════════════
0/6 CONFIRMED (single voice); critical findings flagged regardless and absorbed.
```

**Section 1 — Architecture.** Dependency graph (final):
```
  cl9-roles.json ──▶ cl9_roles.py ──┬──▶ cl9_scope_guard.py ◀── settings.json PreToolUse (every session)
        ▲                            ├──▶ render_roles.py ──▶ .claude/agents/cl9-*.md (hash-stamped)
        │ escape fields              └──▶ tests
  cl9_lock.py ──▶ .claude/runs/editor.lock ◀── cl9_scope_guard.py (stamps session_id via updatedInput,
                  {session_id, purpose, since, expires, pid}      denies editor/build verbs to others)
  skills/cl9/SKILL.md ──▶ .claude/runs/<slug>/{brief-*,grants.json,run.json,report-*,logs}
                     ──▶ .claude/runs/current/<session_id> (slug for the hook)
                     ──▶ Agent(cl9-*) · SendMessage · MCP · Build.bat · Tools/Unreal (lead only, under lock)
  docs/workflow/sessions.md ◀── CLAUDE.md, memory pointers, every denial's anchor
```
State ownership: registry (git), run dir (lead of that run), lock (whichever session
holds it; the hook is the only writer of `session_id`), agents (renderer). Coupling: the
registry schema is consumed by hook/renderer/skill through one loader; the lock format
by hook and lock script — same module (`cl9_lock.py` exposes `read_lock()`). Bottleneck:
the hook runs on every matched call in every session — ~50 ms python start; the matcher
excludes Read/Grep/Glob so the read-heavy majority of calls is untouched (1.1, ACCEPTED:
measure in F, narrow further if > 100 ms). SPOF: the lead's context → `run.json` +
`resume`. Security architecture: local scripts as the user; role agents denied MCP and
editor verbs; `updatedInput` rewrites only the two lock commands (allowlisted exact
prefix). Production failure per integration point: Agent spawn fails → mode C fallback;
`SendMessage` to a finished agent → resumes it (documented) — but a `TaskStop`ped agent
cannot be resumed → lead re-dispatches with the report so far (1.2, ACCEPTED,
documented in runbooks); MCP hang → existing WM_CLOSE recipe; build failure → owning role
via `SendMessage`, rebuild under the same lock; lock file corrupt → treated free +
stderr warning to every session (1.3, ACCEPTED). Distribution: none. Diagram-worthy
code comments: `cl9_scope_guard.py` (decision tree lead/role → path/verb/lock),
`cl9_lock.py` (state: free → held(pid alive) → expired/released), `SKILL.md` (run state
machine) — all three carry the ASCII diagram from this doc (1.4, ACCEPTED).

**Section 2 — Code quality.** House style from `Tools/Loc/sync_strings.py`: Korean
module docstring with usage lines, `ROOT` from `__file__`, `sys.exit("<message>")` on
user errors, stdlib only — the three new scripts follow it (2.1, ACCEPTED). DRY: one
loader; one lock module; renderer is the only writer of agent bodies; messages built
from one `MESSAGES` table so `sessions.md` and the hook never drift on wording (2.2,
ACCEPTED). Error handling pattern: every deny path returns through one
`deny(reason, anchor)` function that also appends `denied.log`; lead-side errors through
one `warn_and_allow()` that exits 1 (2.3, ACCEPTED). Missing edge cases called out:
`tool_input.file_path` absent (non-file Edit variants) → allow; `command` absent → allow;
`cwd` outside the repo (a subagent with `isolation: worktree`) → deny for roles (2.4,
ACCEPTED, tests added). Over-engineering: `run.json` nine states — kept (each has a next
action); `note` field — prose only, never parsed (fine). Under-engineering: fnmatch
`*` crossing `/` makes `Source/CL9/UI/*` match `Source/CL9/UI/x/y.cpp` as intended but
also `Source/CL9/UI-old/…` if such a folder appears → patterns end with `/` boundaries in
the loader (`Source/CL9/UI/*` compiled as prefix `source/cl9/ui/`) (2.5, ACCEPTED).
Technical debt hotspot: prompt text in agent bodies — rules by pointer, hash-stamped.
No existing ASCII diagrams in touched files.

**Section 3 — Test review.** Framework: no CLAUDE.md testing section; repo has UE
automation tests (`CL9SiteLayoutTests.cpp`, `CL9ReplicationCheck.cpp`) for C++ and
nothing for python → `python -m unittest discover .claude/hooks` is the new suite
(stdlib, no dependency). Coverage diagram (paths traced from the plan):
```
CODE PATHS                                                  USER FLOWS
[+] .claude/hooks/cl9_scope_guard.py                        [+] Manual session
  ├── main(): parse stdin                                     ├── [GAP→F]  /cl9 → usage
  │   ├── [★★★ unit] malformed → role deny / lead exit 1      ├── [GAP→F]  /cl9 role ui = registry
  │   └── [★★★ unit] guard.off → allow + stderr               ├── [GAP→F]  taskkill refused under lock
  ├── classify_path(role, path)                               └── [GAP→F]  /cl9 status: hook active?
  │   ├── [★★★ unit] owns / shared(log) / grants / except   [+] Dispatched run
  │   ├── [★★★ unit] append_only / lead_only / outside repo   ├── [GAP→F]  intake question(s)
  │   └── [★★★ unit] backslash, case, unicode                 ├── [GAP→F]  split table → A/B/C/D
  ├── check_command(cmd)                                      ├── [GAP→F]  single-role skips AUQ
  │   ├── [★★★ unit] taskkill/Build/git/rm/Stop-Process       ├── [GAP→F]  report validator + nudge
  │   └── [★★★ unit] lock-verb + other holder → deny          ├── [GAP→F]  post-run audit report
  ├── check_mcp(tool_input.tool_name)                         ├── [GAP→F]  build under lock (pid)
  │   └── [★★★ unit] role deny / lead lock-check              ├── [GAP→F]  PIE section pass
  └── rewrite_lock_cmd → updatedInput                         └── [GAP→F]  per-role file lists
      ├── [★★ unit]  adds --session                         [+] Recovery
      └── [GAP→probe] host honors updatedInput (2.1.201)      ├── [GAP→F]  /cl9 resume (no arg)
[+] .claude/hooks/cl9_lock.py                                 ├── [GAP→F]  /cl9 handoff
  ├── [★★★ unit] acquire free / held(wait,timeout) / --force  └── [GAP→F]  /cl9 commit ui (yes only)
  ├── [★★★ unit] release owner / non-owner                  [+] Prompts (agent bodies, SKILL.md)
  ├── [★★★ unit] expiry / pid alive extends / corrupt file    ├── [GAP→EVAL] role puts foreign work in 요청
  └── [★★★ unit] acquire commandlet with editor alive          └── [GAP→EVAL] role never asks mid-run
[+] .claude/hooks/cl9_roles.py                              [+] Probes
  ├── [★★★ unit] load, dialect reject (braces, **), except    ├── [GAP→probe H] game-target compile, editor open
  └── [★★★ unit] coverage: every pattern matches ≥1 file      ├── [GAP→probe H] background permission stalls
[+] .claude/hooks/render_roles.py                             └── [GAP→probe I] Agent Teams on Windows
  └── [★★★ unit] rendered == committed; hash stamp

COVERAGE (planned): unit 22/22 code paths  |  user flows 0/17 until F  |  evals 0/2 until F  |  probes 0/3
QUALITY: ★★★:20 ★★:2  |  GAPS: 22 → all mapped to F (E2E dry-run), H, I
```
Regression rule: nothing existing changes behavior except that a `taskkill`/build under
someone else's lock is refused — covered by the F step "other session refused meanwhile".
Friday-2am test: lock held by a crashed session with a dead PID and a 60-min expiry not
yet reached → `status` shows dead PID, `release --force` works (unit + F). Hostile QA:
`Write` to `..\\..\\Source\\CL9\\Items\\x.cpp` from `cl9-ui` → normalized, denied (unit);
`python -c "open('Source/CL9/Items/x.cpp','w')"` from `cl9-ui` → passes the hook
(tripwire) and is caught by the post-run audit (F). Chaos: delete `editor.lock` mid-build
→ nothing breaks; delete `.claude/runs/` → hook allows registry-only, `status` reports.
Flaky by construction: F depends on editor state (run with the editor closed first) and on
the 4 live sessions (restart them first); H depends on machine load (record the number,
do not assert it). LLM/prompt changes: agent bodies and SKILL.md are prompts → the two
[→EVAL] rows run as part of F with a deliberately mis-scoped brief. Test plan artifact:
`~/.gstack/projects/Work/user-master-eng-review-test-plan-20260912.md`.

**Section 4 — Performance.** Hook: python start per matched call (~50 ms; Read/Grep/Glob
excluded by the matcher; measured in F; threshold 100 ms). Lock polling: 5 s, bounded by
`--wait`. Registry: <100 patterns, compiled once per call. Token cost: five `inherit`
agents with briefs pointing at doc sections; idle roles not spawned; `max_turns` 200.
Build wall time unchanged unless H. No caching needed; no N+1.

**Worktree parallelization strategy** (for implementing this plan):
| Step | Modules touched | Depends on |
|---|---|---|
| S1 registry + loader + tests | `.claude/cl9-roles.json`, `.claude/hooks/` | — |
| S2 lock script + tests | `.claude/hooks/` | — |
| S3 scope guard hook + settings + tests | `.claude/hooks/`, `.claude/settings.json` | S1, S2 |
| S4 renderer + agents | `.claude/hooks/`, `.claude/agents/` | S1 |
| S5 sessions.md + CLAUDE.md + memory pointers | `docs/workflow/`, `CLAUDE.md`, memory | S2 (lock synopsis) |
| S6 /cl9 skill | `.claude/skills/cl9/` | S1–S5 |
| S7 probe H | none (records into docs/goal) | S2 |
| S8 dry-run F, spike I | none | S6 |
Lanes: A: S1 → S3 → S4 (shared `.claude/hooks/`); B: S2 → S5 (independent of A until
S3); C: S7 (independent, needs the lock). Launch A and B in parallel (same tree — these
are text files with no binary assets; no worktree needed), then S6, then S8. Conflict
flag: A and B both touch `.claude/hooks/` — one module dir; keep them sequential in one
session or coordinate file names (`cl9_lock.py` vs `cl9_scope_guard.py`/`cl9_roles.py`).

**Failure modes (final).**
```
  CODEPATH                | FAILURE MODE                          | TEST? | HANDLED? | USER SEES
  ------------------------|---------------------------------------|-------|----------|----------------------------
  guard path check        | role edits foreign file               | Y     | Y        | Korean deny + anchor
  guard grants            | slug unresolvable (no current marker) | Y     | Y        | registry-only fence; status
  guard verbs             | role runs taskkill/build/git          | Y     | Y        | deny
  guard lock              | lead verb under other's lock          | Y     | Y        | owner + expiry + fix
  guard internal          | bad registry/lock/stdin (lead)        | Y     | Y        | stderr warning (exit 1)
  guard internal          | same (role)                           | Y     | Y        | deny with error
  guard bypass            | python -c / Set-Content writes        | F     | audit    | audit violations in report
  lock                    | stale after crash                     | Y     | Y        | expiry / dead PID / --force
  lock                    | updatedInput unsupported on 2.1.201   | probe | fallback | hook performs acquire itself
  lock                    | peer session never restarted          | F     | partial  | status: 훅 미적용 (that peer)
  dispatch                | agent silent > 10 min                 | F     | Y        | nudge → ABORTED notice
  dispatch                | TaskStop'd agent cannot resume        | —     | Y        | re-dispatch with report
  integration             | build fails                           | manual| Y        | log excerpt → owning role
  integration             | ghost editor                          | manual| Y        | runbook
  integration             | commandlet with editor open           | Y     | Y        | refused with PID
  resume                  | run.json corrupt                      | Y     | Y        | "재개 불가: …"
```
No row is untested ∧ unhandled ∧ silent → **0 critical gaps**.

**What already exists / NOT in scope** — unchanged from Phase 1 (see above and the plan
body).

**TODOS.md updates** — written (auto, all phases): Agent Teams migration (P2), scripted
PIE smoke (P3), session naming probe (P3), Workflow variant (P4), untrack
`settings.local.json` (P3), friction audit of `/cl9` runs (P3).

**Implementation tasks (Eng phase)** — written to
`~/.gstack/projects/Work/tasks-eng-review-<ts>.jsonl`; aggregated at the gate.

```
  +====================================================================+
  |              ENG PLAN REVIEW — COMPLETION SUMMARY                  |
  +====================================================================+
  | Scope challenge      | complexity smell (3 scripts) held, P2       |
  | Section 1 (Arch)     | 4 issues found (all accepted)               |
  | Section 2 (Quality)  | 5 issues found (all accepted)               |
  | Section 3 (Tests)    | diagram produced; 22 unit paths, 17 flows→F, 2 evals→F, 3 probes |
  | Section 4 (Perf)     | 0 issues (1 measurement)                    |
  | Outside voice        | claude subagent only (codex usage limit)    |
  | NOT in scope         | written (8 items)                           |
  | What already exists  | written                                     |
  | Failure modes        | 16 total, 0 CRITICAL GAPS                   |
  | Test plan artifact   | written                                     |
  | Worktree strategy    | 3 lanes, 1 conflict flag                    |
  | TODOS.md updates     | 6 items written                             |
  | Diagrams             | 4 (dependency, coverage, failure modes, lanes) |
  | Unresolved decisions | 0 mid-run (4 gate items)                    |
  +====================================================================+
```

> **Phase 3 complete.** Codex: unavailable. Claude subagent: 29 issues
> (28 accepted, 1 partly). Consensus: 0/6 confirmed (single voice); 3 critical
> findings absorbed. Passing to Phase 4 (Final Gate).

### Cross-phase themes

- **Theme: the shared resource is the product; the dispatcher is the wrapper** — CEO
  voice #1/#10/#14, DX voice #1/#6, Eng voice #1–#3/#9. Three independent reviewers,
  three phases, same conclusion: the lock, its polarity and the audit baseline carry the
  value; the split is a convenience. High-confidence signal → rollout order 1–2 before 4.
- **Theme: the hook is a tripwire, the audit is the truth** — CEO voice #6, Eng voice
  #9/#11/#21. Both phases refused to let a regex be the safety story → allowlist +
  baseline delta audit + `baseline.patch`.
- **Theme: probe the host before building on it** — DX voice #1/#2, Eng voice #5/#7/#8/
  #20/#24/#25/#28. Hooks snapshot at start, `python` vs `py -3`, `updatedInput`, "ask"
  under bypass, `agent_type`, teammate stdin: every one became success criterion 0.
- **Theme: the user steers by eye** — CEO voice #8, DX narrative/TTHW (build-dominated).
  Two phases; the third (Eng) did not touch it → carried to the gate as open question 1,
  not auto-decided.

<!-- AUTONOMOUS DECISION LOG -->
### Decision Audit Trail

| # | Phase | Decision | Classification | Principle | Rationale | Rejected |
|---|---|---|---|---|---|---|
| 1 | CEO | Mode = SELECTIVE EXPANSION | Mechanical | P6 | enhancement of an existing workflow | HOLD, EXPANSION |
| 2 | CEO | Approach B (registry+lock+hook+dispatcher, layered) | Taste → gate Q2 | P1 | completeness; A is B's first two steps | A alone, C (Teams first) |
| 3 | CEO | Accept expansions 1–10 (lock, renderer, coverage test, resume, handoff, commit, H probe, I spike, audit, deny verbs) | Mechanical | P2 | each S-effort, in blast radius | — |
| 4 | CEO | Defer 11–13 (scripted PIE, naming, Workflow) to TODOS | Mechanical | P3 | 3–5 files / unverified / not load-bearing | building now |
| 5 | CEO | Reject `docs` on sonnet | Mechanical | P1 | 정본 on cheapest model (voice #14) | sonnet |
| 6 | CEO | Dispatcher stays in scope; rollout reordered | User challenge → gate Q2 | P1/P2 | user asked for it; voice says wait | cutting D |
| 7 | CEO | Mode C default for editor-in-the-loop slices | User challenge → gate Q1 | P5 | logs show mid-work steering | A everywhere |
| 8 | CEO | Reject Workflow variant now | Mechanical | P3 | resume covered by run.json | Workflow |
| 9 | CEO | Reject worktrees for text roles | Mechanical | P5 | no conflicts in 29 sessions; single build tree | worktree |
| 10 | CEO | `.claude/runs/` not `Saved/` | Mechanical | P5 | Saved/ is deleted/written by UE | Saved/ |
| 11 | CEO §1 | Loader module; `/cl9 resume`; run.json | Mechanical | P4/P1 | one schema, SPOF | — |
| 12 | CEO §3 | Deny destructive verbs; later allowlist (Eng) | Mechanical | P1 | security | — |
| 13 | CEO §5 | maxTurns 200; normalize paths; split guard functions | Mechanical | P5 | explicit | unbounded |
| 14 | CEO §6 | Hook latency measured in F, threshold 100 ms | Mechanical | P6 | measure, don't guess | narrowing matcher now |
| 15 | CEO | CEO plan spec review: 2 iterations, 8 fixed | Mechanical | P1 | reviewer issues all actionable | — |
| 16 | DX | Persona = the CL9 author; DX POLISH; magical moment = refused taskkill / status | Mechanical | P6 | one developer; enhancement | EXPANSION/TRIAGE |
| 17 | DX | Accept 18 of 19 voice findings (restart note, py launcher, allowlist, parsing rule, synopsis, defaults, aliases, dialect, exit/ask channel, MCP parse, 60→90 min, validator, nudge, anchors, copy-paste blocks, escape hatches, kill switch) | Mechanical | P1/P5 | each concrete, S effort | — |
| 18 | DX | Kill switch as a file (`guard.off`), not env var | Mechanical | P5 | works from any session on Windows | env var |
| 19 | DX | settings.local.json untracking → TODO | Mechanical | P3 | separate cleanup | in-scope |
| 20 | Eng | Complexity smell (3 scripts) held | Mechanical | P2 | never reduce; each deletable | reduction |
| 21 | Eng | Accept 28 of 29 voice findings (atomic lock, polarity + leases, save_assets never denied, process triple, agent_type run resolution, touch files, ask channel, drop updatedInput, baseline audit, terminal deny, allowlist, fnmatchcase/realpath, Korean tasklist, wait cap, runs-dir loss, tests, nonce, --pid, syntax-check note, py -3, audit blind spots, report-as-data, both image names, teammate stdin, explicit allowlist) | Mechanical | P1/P5 | each concrete, verified against the plan text | — |
| 22 | Eng | permissionMode for role agents left inherited | Mechanical | P5 | prompting mode stalls background agents; allowlist + audit are the barrier | acceptEdits/default |
| 23 | Eng §1 | ASCII diagrams in the three scripts + SKILL.md | Mechanical | P1 | eng preference | — |
| 24 | Eng §2 | House style from sync_strings.py; MESSAGES table; deny()/warn() funcs; prefix-boundary patterns | Mechanical | P4/P5 | DRY, explicit | — |
| 25 | Eng §3 | All 22 unit paths ★★★; 17 flows + 2 evals → F; 3 probes | Mechanical | P1 | complete | deferring tests |
| 26 | Eng | TODOS.md: 6 items written | Mechanical | P7 | everything deferred is written down | — |
| 27 | All | Codex unavailable → `[subagent-only]` every phase; consensus never CONFIRMED | Mechanical | P6 | usage limit until 09-17 | waiting |

### Pre-gate verification

- Phase 1: premise challenge (8 named) ✓ · sections 1–10 with findings or "examined,
  nothing flagged" ✓ · Error & Rescue Registry ✓ · Failure Modes Registry ✓ · NOT in
  scope ✓ · What already exists ✓ · Dream state delta ✓ · Completion Summary ✓ · dual
  voices (subagent; codex unavailable) ✓ · consensus table ✓ · CEO plan persisted +
  spec-reviewed ✓ · tasks JSONL ✓
- Phase 2 (Design): skipped — no UI scope ✓
- Phase 2.5: 8 dimensions scored ✓ · journey map ✓ · empathy narrative ✓ · TTHW with
  target ✓ · DX implementation checklist ✓ · dual voices ✓ · consensus table ✓ · tasks
  JSONL ✓
- Phase 3: scope challenge with code read ✓ · architecture diagram ✓ · test diagram ✓ ·
  test plan artifact on disk ✓ (`~/.gstack/projects/Work/user-master-eng-review-test-plan-20260912.md`)
  · NOT in scope ✓ · What already exists ✓ · failure modes with critical-gap check ✓ ·
  Completion Summary ✓ · dual voices ✓ · consensus table ✓ · TODOS.md updated ✓ · tasks
  JSONL ✓ · worktree strategy ✓
- Cross-phase themes ✓ · Decision Audit Trail (27 rows) ✓

### Revision 1 — gate cycle 2 (2026-09-12): the `balance` role

**User direction at the gate:** "밸런스 담당하는 에이전트도 있어야 되지 않을까? plan이
기획이니까 이거보고 판단할거." Classified as a plan revision (option C/D) → scope (CEO 1B)
and architecture/tests (Eng) re-run as one focused delta review on the amended plan;
Codex still unavailable → `[subagent-only]`.

**Amendment:** sixth role `balance` — canon `docs/plan` (economy, progression, rules,
`build-order.md` 원칙 3 inequalities, `docs/reference/*`), decides tunable table values,
never edits `Source/`, emits batched `.uasset` row requests with a PIE observable each,
pastes the output of `Tools/Balance/check_inequalities.py`, and is grant-gated with
gameplay on the numeric design docs.

**Delta review (Claude subagent, CEO + Eng):** 10 findings, verdict "not ready as first
specified — fix 1, 2, 6, 8 and add the checker (10)". Disposition:
| # | finding (severity, confidence) | disposition |
|---|---|---|
| 1 | `docs/systems/balance.md` and the five data docs are column/mechanism docs, not value docs (high, 8) | ACCEPTED: balance owns `economy.md` only; data docs grant-gated with gameplay; `balance.md` → gameplay |
| 2 | A4 has no formula yet — an agent would invent an operand (high, 9) | ACCEPTED: checker prints "A4 미결 — 식 없음"; test asserts refusal |
| 3 | docs-only values are consistent but untested (high, 8) | ACCEPTED: `old → new (unit)` + doc lines + PIE observable per row; `decisions.md` entry tagged 초안 until observed |
| 4 | canon drifts when the lead tweaks by eye in PIE (medium, 7) | ACCEPTED: re-read assets after applying rows; mismatches routed back to balance as doc updates |
| 5 | every balance run ends in the serialized editor (medium, 6) | ACCEPTED: rows batched per run; `Tools/Unreal/apply_table_rows.py` commandlet path when the editor is closed |
| 6 | doubly-shared paths defeat single-writer (critical, 9) | ACCEPTED: grant-gated shared paths, intra-run overlap refusal, pairwise-intersection registry test, hook test |
| 7 | seven `note` fences checked by nobody (medium, 8) | ACCEPTED: diff-shape audit for balance (numeric cell of an existing row only) |
| 8 | "any number → balance" is unbounded and serializes every task (high, 8) | ACCEPTED: bounded to tunable columns of `docs/plan/data/*` tables; gameplay placeholder + balance in parallel; interface dependency |
| 9 | dry-run F does not exercise balance (medium, 8) | ACCEPTED: F = question path; F2 = single A1 operand happy path |
| 10 | mental arithmetic ignores `DangerCurve`; disagrees with the C++ check (high, 8) | ACCEPTED: `Tools/Balance/check_inequalities.py` (docs-table parser, curve maxima, A4 refused) + fixtures |

Audit-trail rows 28–30 appended. Tasks appended to the CEO and Eng JSONL of this run
(same `run_id`): CEO T11 (balance role registry + agent + docs), Eng T9 (grant-gated
shared paths + tests), T10 (`check_inequalities.py` + fixtures), T11
(`apply_table_rows.py` + asset re-read compare), T12 (diff-shape audit + F2).

| # | Phase | Decision | Classification | Principle | Rationale | Rejected |
|---|---|---|---|---|---|---|
| 28 | Gate/CEO | Add `balance` as the sixth role | User direction | — | user: plan is the 기획, values are judged from it | five roles |
| 29 | Rev1 | Accept all 10 delta findings (ownership, A4 refusal, observables, drift check, batching, grant gating, diff-shape audit, bounded rule, F2, checker) | Mechanical | P1/P5 | each grounded in a quoted doc line; S–M effort, in radius | — |
| 30 | Rev1 | Re-run scope = focused delta review (CEO 1B + Eng) instead of full phases | Mechanical | P3 | one-role addition; full re-run disproportionate; findings recorded at the same depth | full re-run |
| 31 | Gate | Model policy: lead = session model, role agents = `opus`, research sub-tasks = `sonnet`, bulk = `haiku` | User direction | — | user at the gate ("리드는 페이블, 작업은 오푸스, 자료조사는 소넷"); registry `model` field already existed; consistent with row 5 | `inherit` everywhere |
| 32 | Gate | APPROVED as-is (option A) at gate cycle 2 — Q1 mode C default, Q2 lock-first order, Q3 six roles, Q4 tracked settings.json | User decision | — | user chose "그대로 승인" after the walkthrough and model policy | B/C/D |

## GSTACK REVIEW REPORT

| Review | Trigger | Why | Runs | Status | Findings |
|--------|---------|-----|------|--------|----------|
| CEO Review | `/plan-ceo-review` | Scope & strategy | 1 + revision 1 (via /autoplan, 2026-09-12) | PENDING GATE (cycle 2) | 15 proposals, 11 accepted (incl. `balance` role by user direction), 3 deferred, 1 rejected; 0 critical gaps; 15 + 5 outside-voice findings absorbed |
| Codex Review | `/codex review` | Independent 2nd opinion | 0 | UNAVAILABLE | Codex usage limit until 2026-09-17; every phase ran `[subagent-only]` |
| Eng Review | `/plan-eng-review` | Architecture & tests (required) | 1 + revision 1 (via /autoplan, 2026-09-12) | PENDING GATE (cycle 2) | 29 + 5 outside-voice findings (33 accepted, 1 partly), 14 section findings, 0 critical gaps, 22 unit paths + F/F2/H/I |
| Design Review | `/plan-design-review` | UI/UX gaps | 0 | SKIPPED | no UI scope |
| DX Review | `/plan-devex-review` | Developer experience gaps | 1 (via /autoplan, 2026-09-12) | PENDING GATE | score 6/10 → 8/10 after amendments; TTHW ~5 min → ~1 min (role); 19 findings (18 accepted) |

Prior entries in the review log (2026-08-27, commit 41e72ce, a different plan) are older
than 7 days and excluded.

- **CROSS-MODEL:** Codex unavailable in all phases; the three outside voices were fresh-context
  Claude subagents (same model family). Their overlap with each other is the cross-phase
  theme list above — treat it as a strong same-family signal, not cross-model consensus.
- **VERDICT:** CEO + DX + ENG CLEARED — approved as-is at gate cycle 2 (2026-09-12): Q1
  mode C default for editor-in-the-loop slices (`--mode A` available), Q2 lock + protocol
  + hook + `/cl9 role` first, Q3 six roles incl. `balance`, Q4 hook in the tracked
  `.claude/settings.json`; model policy lead = session model / roles = Opus / research =
  Sonnet. Ready to implement from rollout step 1.

NO UNRESOLVED DECISIONS
