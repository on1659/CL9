---
name: cl9
description: Use the CL9 session workflow for role contracts, task planning and delegation, status, resume, handoff, and role-scoped commits in this repository.
---

# CL9 session workflow in Codex

Use this project skill when the user invokes `$cl9` or asks to coordinate a CL9 task through its role workflow. This skill is the Codex entry point for the repository's Claude Code `/cl9` skill.

## Invocation and source of truth

- Invoke from Codex as `$cl9`; `/cl9` is the Claude Code command.
- Keep the existing project rules in `AGENTS.md`, `CLAUDE.md`, and `docs/workflow/sessions.md` authoritative. Read only the sections needed for the requested operation.
- For detailed command behavior, role aliases, report formats, gates, and lock rules, read `.claude/skills/cl9/SKILL.md` and `docs/workflow/sessions.md`.
- The `.claude` files describe Claude Code behavior. Their hooks do not automatically run in Codex. Run the relevant repository scripts explicitly and use Codex's available collaboration tools when delegation is part of the requested workflow.
- Use `.claude/cl9-roles.json` and `render_roles.py` as the role and ownership source of truth. Do not invent file grants or role contracts.

## Operations

Interpret an explicit operation and its arguments as follows:

- `role [name]`: list roles or render the requested role contract with `.claude/hooks/render_roles.py`; present the contract and respect its owned/shared paths for work in that role.
- `status`: inspect the lock state, recent run state, and live collaborators. Use the repository status scripts and `collaboration.list_agents`; distinguish Claude hook status from what Codex actually enforces.
- `task <request>`: read the current build order and session workflow, group the requested work by role and dependency, and use the repository's run scripts for grants, dispatch, audit, and finish. Delegate role work with `collaboration.spawn_agent` when the user chose the task workflow; keep each agent within its granted files and integrate only after its report is ready.
- `resume [slug]`: find the latest eligible run or named run, read its briefs, grants, reports, handoff, and audit, then continue from the recorded state. Relaunch a role agent when the previous session is unavailable.
- `handoff`: record a concise handoff in the active run directory, include changed files, current lock/build state, blockers, and the exact resume command. Do not assume Claude's machine-specific memory path exists in Codex.
- `commit [role]`: use the role report's changed-file list, inspect the staged diff, and stage only those files. Follow the project's separate approval rule before creating a commit; never include unrelated dirty files.

## Shared Unreal Editor, build, and PIE work

Before starting, stopping, building, or driving Unreal Editor, read the lock procedure in `docs/workflow/sessions.md` and inspect the current lock. Acquire the appropriate lock with `.claude/hooks/cl9_lock.py` before acting; do not bypass a live lock or assume Claude's session hooks are active in Codex. Release only locks owned by this session after the operation completes.

Do not claim the project's Claude-only hooks, agent roster, or model routing are active in Codex unless verified. Preserve the run's report and audit requirements while adapting tool calls to this environment.
