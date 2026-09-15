"""범위 가드 훅 테스트 — `py -3 -m unittest discover .claude/hooks`.

임시 저장소(실제 레지스트리 복사)에서 훅을 함수로 부르고, JSON 모양·stdin 오류·지연은 실제 프로세스로 돈다.
`fixtures/*.json` 이 있으면(재시작 뒤 캡처한 2.1.201 실제 stdin) 그것도 돌린다.
"""
import glob
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

import cl9_lock as L
import cl9_roles as R
import cl9_scope_guard as G

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
GUARD_PY = os.path.join(HERE, "cl9_scope_guard.py")
FIXTURES = os.path.join(HERE, "fixtures")


def decision(out):
    return out.get("hookSpecificOutput", {}).get("permissionDecision") if out else "allow"


def reason(out):
    return out.get("hookSpecificOutput", {}).get("permissionDecisionReason", "")


class GuardBase(unittest.TestCase):
    def setUp(self):
        self.root = tempfile.mkdtemp(prefix="cl9guard-")
        os.makedirs(os.path.join(self.root, ".claude", "hooks"))
        shutil.copy(os.path.join(REPO, ".claude", "cl9-roles.json"), os.path.join(self.root, ".claude", "cl9-roles.json"))
        for rel in ("Source/CL9/Items/CL9Item.cpp", "Source/CL9/UI/CL9Widget.h", "docs/plan/data/item/egg.md",
                    "docs/plan/economy.md", "docs/systems/item.md", "docs/decisions.md", "Tools/Loc/sync_strings.py",
                    "Tools/Blender/build_cart_a.py", "Content/Data/ItemTable.uasset", ".claude/hooks/cl9_lock.py"):
            p = os.path.join(self.root, rel)
            os.makedirs(os.path.dirname(p), exist_ok=True)
            with open(p, "w") as f:
                f.write("x")
        self._orig_now = L.now
        self._now = None
        L.now = lambda: self._now if self._now is not None else time.time()

    def tearDown(self):
        L.now = self._orig_now
        shutil.rmtree(self.root, ignore_errors=True)

    def make_run(self, slug, roles, state="DISPATCHED", grants=None, grants_raw=None):
        d = os.path.join(self.root, ".claude", "runs", slug)
        os.makedirs(d, exist_ok=True)
        with open(os.path.join(d, "run.json"), "w", encoding="utf-8") as f:
            json.dump({"slug": slug, "state": state, "roles": roles}, f)
        if grants_raw is not None:
            with open(os.path.join(d, "grants.json"), "w", encoding="utf-8") as f:
                f.write(grants_raw)
        elif grants is not None:
            with open(os.path.join(d, "grants.json"), "w", encoding="utf-8") as f:
                json.dump(grants, f)
        return d

    def call(self, tool, tool_input, agent_type=None, session_id="SESS-A", cwd=None):
        data = {"session_id": session_id, "hook_event_name": "PreToolUse", "tool_name": tool,
                "tool_input": tool_input, "cwd": cwd or self.root, "permission_mode": "bypassPermissions"}
        if agent_type:
            data["agent_type"] = agent_type
            data["agent_id"] = "agent-1"
        return G.run(data)

    def edit(self, rel, agent_type=None, tool="Edit", **kw):
        return self.call(tool, {"file_path": os.path.join(self.root, rel), "old_string": "a", "new_string": "b"},
                         agent_type=agent_type, **kw)

    def read_log(self, run_dir, name):
        p = os.path.join(run_dir, name)
        if not os.path.exists(p):
            return ""
        with open(p, "r", encoding="utf-8") as f:
            return f.read()


class RoleEdits(GuardBase):
    def test_owned_path_allowed(self):
        self.make_run("r1", ["ui"])
        self.assertEqual(self.edit("Source/CL9/UI/CL9Widget.h", "cl9-ui"), {})

    def test_foreign_path_denied_with_owner_and_anchor(self):
        d = self.make_run("r1", ["ui"])
        out = self.edit("Source/CL9/Items/CL9Item.cpp", "cl9-ui")
        self.assertEqual(decision(out), "deny")
        self.assertIn("gameplay 영역", reason(out))
        self.assertIn("다른 역할에 요청", reason(out))
        self.assertIn(G.DENY_ANCHOR, reason(out))
        self.assertIn("cl9item.cpp", self.read_log(d, "denied.log"))

    def test_granted_path_allowed_and_logged(self):
        d = self.make_run("r1", ["ui"], grants={"cl9-ui": ["Source/CL9/Items/CL9Item.cpp"]})
        self.assertEqual(self.edit("Source/CL9/Items/CL9Item.cpp", "cl9-ui"), {})
        self.assertIn("(grant)", self.read_log(d, "shared-edits.log"))

    def test_malformed_grants_is_empty_and_logged(self):
        d = self.make_run("r1", ["ui"], grants_raw="{oops")
        out = self.edit("Source/CL9/Items/CL9Item.cpp", "cl9-ui")
        self.assertEqual(decision(out), "deny")
        self.assertEqual(self.edit("Source/CL9/UI/CL9Widget.h", "cl9-ui"), {})
        errors = self.read_log(os.path.join(self.root, ".claude", "runs"), "guard-errors.log")
        self.assertIn("grants.json 손상(r1)", errors)
        self.assertIn("(grant)", "(grant)")  # 형식 확인용 자리표시
        self.assertEqual(self.read_log(d, "shared-edits.log"), "")

    def test_shared_path_allowed_and_logged(self):
        d = self.make_run("r1", ["ui"])
        self.assertEqual(self.edit("Source/CL9/Items/CL9Item.cpp".replace("Items/CL9Item.cpp", "Character/CL9Character.cpp"), "cl9-ui"), {})
        self.assertIn("(shared)", self.read_log(d, "shared-edits.log"))

    def test_lead_passes_everywhere(self):
        self.assertEqual(self.edit("Source/CL9/Items/CL9Item.cpp"), {})
        self.assertEqual(self.edit("docs/decisions.md"), {})
        self.assertEqual(self.edit("Content/Data/ItemTable.uasset"), {})

    def test_generic_subagent_is_lead_class(self):
        self.assertEqual(self.edit("Source/CL9/Items/CL9Item.cpp", agent_type="general-purpose"), {})

    def test_unknown_role_denied(self):
        out = self.edit("Source/CL9/UI/CL9Widget.h", "cl9-nope")
        self.assertEqual(decision(out), "deny")
        self.assertIn("알 수 없는 역할", reason(out))

    def test_no_run_denied_with_distinct_reason(self):
        out = self.edit("Source/CL9/UI/CL9Widget.h", "cl9-ui")
        self.assertEqual(decision(out), "deny")
        self.assertIn("실행 상태 없음", reason(out))
        self.assertNotIn("영역", reason(out))

    def test_terminal_state_denied(self):
        self.make_run("r1", ["ui"], state="REPORTED")
        out = self.edit("Source/CL9/UI/CL9Widget.h", "cl9-ui")
        self.assertEqual(decision(out), "deny")
        self.assertIn("REPORTED", reason(out))
        self.make_run("r2", ["ui"], state="ABORTED")
        self.assertIn("ABORTED", reason(self.edit("Source/CL9/UI/CL9Widget.h", "cl9-ui")))

    def test_latest_live_run_wins_over_terminal(self):
        self.make_run("old", ["ui"], state="REPORTED")
        time.sleep(0.05)
        self.make_run("new", ["ui"])
        self.assertEqual(self.edit("Source/CL9/UI/CL9Widget.h", "cl9-ui"), {})

    def test_append_only_and_lead_only_reasons(self):
        self.make_run("r1", ["docs", "art"])
        out = self.edit("docs/decisions.md", "cl9-docs")
        self.assertIn("append-only", reason(out))
        self.assertIn("decisions.md에 남길 결정", reason(out))
        out = self.edit("Content/Data/ItemTable.uasset", "cl9-art")
        self.assertIn("리드 전용", reason(out))
        self.assertIn("표 행·에셋", reason(out))

    def test_report_path_allowed_for_own_role_only(self):
        d = self.make_run("r1", ["ui", "gameplay"])
        self.assertEqual(self.edit(os.path.relpath(os.path.join(d, "report-ui.md"), self.root), "cl9-ui", tool="Write"), {})
        out = self.edit(os.path.relpath(os.path.join(d, "report-gameplay.md"), self.root), "cl9-ui", tool="Write")
        self.assertEqual(decision(out), "deny")
        out = self.edit(os.path.relpath(os.path.join(d, "run.json"), self.root), "cl9-ui")
        self.assertEqual(decision(out), "deny")

    def test_touch_file_written(self):
        d = self.make_run("r1", ["ui"])
        self.edit("Source/CL9/UI/CL9Widget.h", "cl9-ui")
        self.assertTrue(os.path.exists(os.path.join(d, "touch-ui")))

    def test_grant_gated_gameplay_vs_balance(self):
        self.make_run("r1", ["gameplay", "balance"], grants={"cl9-balance": ["docs/plan/data/item/egg.md"]})
        out = self.edit("docs/plan/data/item/egg.md", "cl9-gameplay")
        self.assertEqual(decision(out), "deny")
        self.assertIn("balance 것", reason(out))
        self.assertEqual(self.edit("docs/plan/data/item/egg.md", "cl9-balance"), {})
        self.assertEqual(self.edit("docs/plan/economy.md", "cl9-balance"), {})
        self.assertEqual(decision(self.edit("Source/CL9/Items/CL9Item.cpp", "cl9-balance")), "deny")

    def test_grant_gated_without_any_grant(self):
        self.make_run("r1", ["gameplay"])
        out = self.edit("docs/plan/data/item/egg.md", "cl9-gameplay")
        self.assertEqual(decision(out), "deny")
        self.assertIn("grants.json 에 없다", reason(out))

    def test_multiedit_and_notebook_inputs(self):
        self.make_run("r1", ["ui"])
        out = self.call("MultiEdit", {"file_path": os.path.join(self.root, "Source/CL9/Items/CL9Item.cpp"), "edits": []}, "cl9-ui")
        self.assertEqual(decision(out), "deny")
        out = self.call("NotebookEdit", {"notebook_path": os.path.join(self.root, "docs/x.ipynb"), "new_source": ""}, "cl9-ui")
        self.assertEqual(decision(out), "deny")
        out = self.call("Write", {"file_path": os.path.join(self.root, "Source/CL9/UI/New.h"), "content": "x"}, "cl9-ui")
        self.assertEqual(out, {})

    def test_paths_outside_repo_and_normalized(self):
        self.make_run("r1", ["ui"])
        out = self.call("Edit", {"file_path": os.path.join(self.root, "..", "other.md")}, "cl9-ui")
        self.assertIn("저장소 밖", reason(out))
        out = self.call("Edit", {"file_path": "C:\\Windows\\x.txt"}, "cl9-ui")
        self.assertIn("저장소 밖", reason(out))
        dotdot = os.path.join(self.root, "docs", "..", "Source", "CL9", "UI", "CL9Widget.h")
        self.assertEqual(self.call("Edit", {"file_path": dotdot}, "cl9-ui"), {})
        upper = os.path.join(self.root.upper(), "SOURCE", "CL9", "UI", "CL9WIDGET.H")
        self.assertEqual(self.call("Edit", {"file_path": upper}, "cl9-ui"), {})
        relative = "Source/CL9/UI/CL9Widget.h"
        self.assertEqual(self.call("Edit", {"file_path": relative}, "cl9-ui"), {})

    def test_junction_resolves_to_real_path(self):
        self.make_run("r1", ["ui"])
        link = os.path.join(self.root, "J")
        target = os.path.join(self.root, "Source", "CL9", "Items")
        rc = subprocess.run(["cmd", "/c", "mklink", "/J", link, target], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
        if rc != 0:
            self.skipTest("mklink /J 불가")
        out = self.call("Edit", {"file_path": os.path.join(link, "CL9Item.cpp")}, "cl9-ui")
        self.assertEqual(decision(out), "deny")
        self.assertIn("gameplay 영역", reason(out))


class RoleShell(GuardBase):
    ALLOWED = [
        "py -3 Tools/Loc/sync_strings.py",
        "py -3 Tools/Loc/sync_strings.py --build",
        "py -3 .claude/hooks/cl9_lock.py status",
        "python -m unittest discover .claude/hooks",
        "py -3 -m unittest discover .claude/hooks",
        "blender -b Tools/Blender/build_cart_a.py",
        r'"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" -b Tools/Blender/build_cart_a.py',
        "py -3 Tools/Build/compile_check.py",
        "py -3 Tools/Build/compile_check.py --timeout-min 10",
        "git status",
        "git diff -- Source/CL9/UI",
        "git log --oneline -5",
        "git ls-files Source",
        "git grep -n HoldPose -- Source",
        "git blame docs/systems/ui.md",
        "ls Source/CL9/UI",
        "dir",
        "cat docs/systems/ui.md",
        "type docs\\systems\\ui.md",
        "head -20 docs/systems/ui.md",
        "tail -n 5 docs/decisions.md",
        "grep -rn Build.bat docs/",
        "grep Build.bat docs/",
        "findstr /s /n HoldPose Source\\*.h",
        "find Source -name \"*.h\"",
        "wc -l docs/systems/ui.md",
        "tasklist",
        "Get-ChildItem Source -Recurse",
        "Get-Content docs/systems/ui.md",
        "Select-String -Path docs/systems/ui.md -Pattern HoldPose",
        "rg -n HoldPose Source",
        "echo hi",
        "pwd",
    ]
    DENIED = [
        "git restore .",
        "git switch -c x",
        "git checkout -- Source",
        "git reset --hard",
        "git commit -m x",
        "git add -A",
        "git stash",
        "rm -rf Source",
        "rmdir /s /q Source",
        "Remove-Item -Recurse Source",
        "Set-Content docs/x.md hi",
        "python -c \"open('x','w')\"",
        "py -3 -c \"print(1)\"",
        "py -3 Tools/../Config/x.py",
        "py -3 docs/x.py",
        "py -3 .claude/hooks/cl9_lock.py acquire build --pid 1",
        "py -3 .claude/hooks/cl9_lock.py release --force",
        "py -3 .claude/hooks/cl9_lock.py guard off",
        "python",
        "python -m http.server",
        "blender Tools/Blender/build_cart_a.py",
        "blender -b docs/x.py",
        "taskkill /IM UnrealEditor.exe /F",
        "Stop-Process -Name UnrealEditor",
        "D:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Build\\BatchFiles\\Build.bat CL9Editor Win64 Development",
        "UnrealEditor-Cmd.exe CL9.uproject -run=pythonscript",
        "find Source -name \"*.h\" -delete",
        "find Source -name \"*.h\" -exec rm {} ;",
        "curl http://x",
        "npm install",
        "start build.cmd",
        "cmd /c build.cmd",
        "",
    ]
    COMPOUND = [
        "git status; git reset --hard",
        "git status && rm -rf Source",
        "cat docs/x.md | tee docs/y.md",
        "echo hi > docs/x.md",
        "cat docs/x.md >> docs/y.md",
        "echo `rm x`",
        "echo $(rm x)",
        "git status\ngit reset --hard",
        "py -3 .claude/hooks/cl9_lock.py status; git reset --hard",
        "cat < docs/x.md",
    ]

    def setUp(self):
        super().setUp()
        self.make_run("r1", ["ui", "art"])

    def sh(self, cmd, tool="Bash", role="cl9-ui"):
        return self.call(tool, {"command": cmd}, role)

    def test_allowed_corpus(self):
        for cmd in self.ALLOWED:
            self.assertEqual(self.sh(cmd), {}, cmd)
            self.assertEqual(self.sh(cmd, tool="PowerShell"), {}, "PowerShell " + cmd)

    def test_denied_corpus(self):
        for cmd in self.DENIED:
            out = self.sh(cmd)
            self.assertEqual(decision(out), "deny", cmd)
            self.assertIn(G.DENY_ANCHOR, reason(out), cmd)

    def test_compound_corpus(self):
        for cmd in self.COMPOUND:
            out = self.sh(cmd)
            self.assertEqual(decision(out), "deny", cmd)
            self.assertIn("복합 명령", reason(out), cmd)

    def test_denials_logged(self):
        self.sh("git reset --hard")
        self.assertIn("git reset --hard", self.read_log(os.path.join(self.root, ".claude", "runs", "r1"), "denied.log"))

    def test_mcp_denied_for_roles(self):
        out = self.call("mcp__unreal-mcp__call_tool", {"toolset_name": "PIE", "tool_name": "StartPIE"}, "cl9-ui")
        self.assertEqual(decision(out), "deny")
        self.assertIn("PIE/StartPIE", reason(out))
        out = self.call("mcp__unreal-mcp__call_tool", {"toolset_name": "Asset", "tool_name": "save_assets"}, "cl9-ui")
        self.assertEqual(decision(out), "deny")
        out = self.call("mcp__blender-mcp__execute_blender_code", {"code": "x"}, "cl9-art")
        self.assertEqual(decision(out), "deny")
        self.assertIn("blender -b", reason(out))


class LeadLock(GuardBase):
    def sh(self, cmd, session="SESS-A", tool="Bash"):
        return self.call(tool, {"command": cmd}, session_id=session)

    def test_destructive_without_lock_denied_even_when_free(self):
        for cmd in ("taskkill /IM UnrealEditor.exe /F", "taskkill /F /IM unrealeditor.exe",
                    "Stop-Process -Name UnrealEditor -Force",
                    "\"D:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Build\\BatchFiles\\Build.bat\" CL9Editor Win64 Development",
                    "cmd /c D:\\UE\\Engine\\Build\\BatchFiles\\Build.bat CL9 Win64 Development",
                    "& 'D:\\UE\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe' D:\\x\\CL9.uproject -run=pythonscript",
                    "git status && taskkill /IM UnrealEditor.exe /F"):
            out = self.sh(cmd)
            self.assertEqual(decision(out), "deny", cmd)
            self.assertIn("acquire build --pid", reason(out), cmd)
            self.assertIn(L.DOC_ANCHOR, reason(out))

    def test_regex_false_positive_corpus_not_destructive(self):
        for cmd in ("grep Build.bat docs/", "grep -rn UnrealEditor-Cmd docs/", "git log --oneline -- Build.bat",
                    "cat docs/x.md", "rg taskkill .claude/hooks", "echo taskkill UnrealEditor",
                    "findstr Build.bat GenerateProjectFiles.bat", "tasklist /FI \"IMAGENAME eq UnrealEditor.exe\""):
            self.assertEqual(self.sh(cmd), {}, cmd)

    def test_destructive_with_lock_allowed_and_other_holder_named(self):
        child = subprocess.Popen(["ping", "-n", "60", "127.0.0.1"], stdout=subprocess.DEVNULL)
        try:
            self.assertEqual(self.sh("py -3 .claude/hooks/cl9_lock.py acquire build --pid %d" % child.pid), {})
            self.assertTrue(L.holds_exclusive(self.root, "SESS-A"))
            self.assertTrue(L.hook_lock_handled(self.root, "acquire", "build"))
            self.assertEqual(self.sh("taskkill /IM UnrealEditor.exe /F"), {})
            out = self.sh("taskkill /IM UnrealEditor.exe /F", session="SESS-B")
            self.assertEqual(decision(out), "deny")
            self.assertIn("세션 SESS-A", reason(out))
            self.assertIn("살아 있음", reason(out))
            self.assertIn("release --force", reason(out))
            out = self.sh("py -3 .claude/hooks/cl9_lock.py acquire pie", session="SESS-B")
            self.assertEqual(decision(out), "deny")
            self.assertIn("잠금 획득 실패", reason(out))
            out = self.sh("py -3 .claude/hooks/cl9_lock.py release --force", session="SESS-B")
            self.assertEqual(decision(out), "deny")
            self.assertEqual(self.sh("py -3 .claude/hooks/cl9_lock.py release"), {})
            self.assertFalse(L.holds_exclusive(self.root, "SESS-A"))
            self.assertTrue(L.hook_lock_handled(self.root, "release"))
        finally:
            child.kill()
            child.wait()

    def test_lock_command_with_session_flag_not_mediated(self):
        self.assertEqual(self.sh("py -3 .claude/hooks/cl9_lock.py --session X acquire pie"), {})
        self.assertFalse(L.holds_exclusive(self.root, "SESS-A"))

    def test_status_nonce_marks_hook_seen(self):
        self.assertEqual(self.sh("py -3 .claude/hooks/cl9_lock.py status --nonce n0nce1"), {})
        self.assertTrue(L.hook_seen_check(self.root, "n0nce1"))

    def test_pie_polarity_and_lease_and_save(self):
        out = self.call("mcp__unreal-mcp__call_tool", {"toolset_name": "PIE", "tool_name": "StartPIE"})
        self.assertEqual(decision(out), "deny")
        self.assertIn("StartPIE", reason(out))
        self.assertEqual(self.call("mcp__unreal-mcp__call_tool", {"toolset_name": "Asset", "tool_name": "ListAssets"}), {})
        _, leases = L.holder(self.root)
        self.assertEqual([l["session_id"] for l in leases], ["SESS-A"])
        out = self.sh("py -3 .claude/hooks/cl9_lock.py acquire pie", session="SESS-B")
        self.assertEqual(decision(out), "deny")
        self.assertIn("세션 SESS-A", reason(out))
        self.assertEqual(self.call("mcp__unreal-mcp__call_tool", {"toolset_name": "Asset", "tool_name": "SaveAssets"}, session_id="SESS-B"), {})
        self.assertEqual(self.sh("py -3 .claude/hooks/cl9_lock.py acquire pie"), {})
        self.assertEqual(self.call("mcp__unreal-mcp__call_tool", {"toolset_name": "PIE", "tool_name": "StartPIE"}), {})
        out = self.call("mcp__unreal-mcp__call_tool", {"toolset_name": "PIE", "tool_name": "StopPIE"}, session_id="SESS-B")
        self.assertEqual(decision(out), "deny")

    def test_blender_mcp_allowed_for_lead(self):
        self.assertEqual(self.call("mcp__blender-mcp__get_scene_info", {}), {})

    def test_malformed_lock_surfaces_to_lead_as_ask_and_role_as_deny(self):
        os.makedirs(L.runs_dir(self.root), exist_ok=True)
        with open(L.lock_path(self.root), "w") as f:
            f.write("{broken")
        with self.assertRaises(L.LockError):
            self.call("mcp__unreal-mcp__call_tool", {"toolset_name": "Asset", "tool_name": "ListAssets"})
        p = subprocess.run([sys.executable, GUARD_PY], input=json.dumps({
            "session_id": "S", "tool_name": "Bash", "tool_input": {"command": "taskkill /IM UnrealEditor.exe /F"}, "cwd": self.root}).encode("utf-8"),
            stdout=subprocess.PIPE)
        out = json.loads(p.stdout.decode("utf-8"))
        self.assertEqual(decision(out), "deny", "파괴 동사는 검사 실패 시 막는다")
        self.assertIn("가드 훅 오류", reason(out))
        self.assertIn("LockError", reason(out))
        self.assertIn("LockError", self.read_log(L.runs_dir(self.root), "guard-errors.log"))
        p = subprocess.run([sys.executable, GUARD_PY], input=json.dumps({
            "session_id": "S", "tool_name": "Bash", "tool_input": {"command": "git status"}, "cwd": self.root}).encode("utf-8"),
            stdout=subprocess.PIPE)
        out = json.loads(p.stdout.decode("utf-8"))
        self.assertEqual(decision(out), "allow", "그 밖의 호출은 allow + additionalContext")
        self.assertIn("가드 훅 오류", out["hookSpecificOutput"]["additionalContext"])

    def test_guard_off_allows_everything_until_expiry(self):
        self._now = 1000.0
        L.guard_off(self.root, minutes=1)
        self.assertEqual(self.sh("taskkill /IM UnrealEditor.exe /F"), {})
        self.make_run("r1", ["ui"])
        self.assertEqual(self.edit("Source/CL9/Items/CL9Item.cpp", "cl9-ui"), {})
        self._now = 1000.0 + 61
        self.assertEqual(decision(self.sh("taskkill /IM UnrealEditor.exe /F")), "deny")
        self.assertEqual(decision(self.edit("Source/CL9/Items/CL9Item.cpp", "cl9-ui")), "deny")

    def test_cwd_below_root_still_finds_registry(self):
        sub = os.path.join(self.root, "Source", "CL9")
        self.assertEqual(decision(self.sh("taskkill /IM UnrealEditor.exe /F")), "deny")
        out = self.call("Bash", {"command": "taskkill /IM UnrealEditor.exe /F"}, cwd=sub)
        self.assertEqual(decision(out), "deny")


class Process(GuardBase):
    """실제 프로세스로: JSON 모양, stdin 오류 극성, 지연 예산."""

    def run_hook(self, data, raw=None):
        payload = raw if raw is not None else json.dumps(data, ensure_ascii=False).encode("utf-8")
        t0 = time.perf_counter()
        p = subprocess.run(["py", "-3", GUARD_PY], input=payload, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=self.root)
        elapsed = (time.perf_counter() - t0) * 1000
        return p.returncode, p.stdout.decode("utf-8"), elapsed

    def test_deny_json_shape_exact(self):
        self.make_run("r1", ["ui"])
        rc, out, _ = self.run_hook({"session_id": "S", "agent_type": "cl9-ui", "tool_name": "Edit",
                                    "tool_input": {"file_path": os.path.join(self.root, "Source/CL9/Items/CL9Item.cpp")}, "cwd": self.root})
        self.assertEqual(rc, 0)
        self.assertTrue(out.startswith("{") and out.endswith("}"))
        obj = json.loads(out)
        self.assertEqual(set(obj.keys()), {"hookSpecificOutput"})
        self.assertEqual(set(obj["hookSpecificOutput"].keys()), {"hookEventName", "permissionDecision", "permissionDecisionReason"})
        self.assertEqual(obj["hookSpecificOutput"]["hookEventName"], "PreToolUse")
        self.assertEqual(obj["hookSpecificOutput"]["permissionDecision"], "deny")
        self.assertTrue(out.isascii(), "stdout 은 ASCII 이스케이프(cp949 콘솔 안전)")

    def test_allow_is_empty_object(self):
        rc, out, _ = self.run_hook({"session_id": "S", "tool_name": "Edit", "tool_input": {"file_path": "x"}, "cwd": self.root})
        self.assertEqual((rc, out), (0, "{}"))

    def test_malformed_stdin_polarity(self):
        rc, out, _ = self.run_hook(None, raw=b"not json")
        self.assertEqual(rc, 0)
        self.assertEqual(decision(json.loads(out)), "allow")
        self.assertIn("가드 훅 오류", json.loads(out)["hookSpecificOutput"]["additionalContext"])
        self.make_run("r1", ["ui"])
        rc, out, _ = self.run_hook(None, raw=json.dumps({"agent_type": "cl9-ui", "tool_name": "Edit", "tool_input": "notadict", "cwd": self.root}).encode())
        self.assertEqual(decision(json.loads(out)), "deny")
        self.assertIn("가드 훅 오류", reason(json.loads(out)))

    def test_korean_in_stdin_survives(self):
        self.make_run("r1", ["ui"])
        rc, out, _ = self.run_hook({"session_id": "S", "agent_type": "cl9-ui", "tool_name": "Bash",
                                    "tool_input": {"command": "git commit -m \"한글 메시지\""}, "cwd": self.root})
        self.assertEqual(decision(json.loads(out)), "deny")

    def test_latency_budget(self):
        self.make_run("r1", ["ui"])
        data = {"session_id": "S", "agent_type": "cl9-ui", "tool_name": "Edit",
                "tool_input": {"file_path": os.path.join(self.root, "Source/CL9/UI/CL9Widget.h")}, "cwd": self.root}
        self.run_hook(data)  # 워밍업(pyc)
        samples = sorted(self.run_hook(data)[2] for _ in range(5))
        # 예산: 계획은 100 ms 였으나 이 머신의 `py -3 -c pass` 바닥이 ~80 ms(런처+인터프리터) — 훅 자체는 ~30 ms.
        # 2026-09-13 실측 중앙값 ~115 ms → 150 ms 를 예산으로 둔다 (docs/goal/session-orchestration.md 프로브 기록).
        self.assertLess(samples[len(samples) // 2], 150.0, "중앙값 %.1f ms (샘플 %s)" % (samples[len(samples) // 2], ["%.0f" % s for s in samples]))


class Fixtures(unittest.TestCase):
    """재시작 뒤 캡처한 실제 2.1.201 stdin: fixtures/<name>.json = {"stdin": {...}, "expect": "allow|deny|ask", "root_override": true}."""

    def test_fixtures(self):
        paths = sorted(glob.glob(os.path.join(FIXTURES, "*.json")))
        if not paths:
            self.skipTest("fixtures 없음 — 세션 재시작 뒤 프로브가 채운다")
        for p in paths:
            with open(p, "r", encoding="utf-8") as f:
                fx = json.load(f)
            stdin = fx["stdin"]
            for key in ("session_id", "tool_name", "tool_input", "cwd"):
                self.assertIn(key, stdin, "%s 에 %s 없음" % (os.path.basename(p), key))
            if stdin.get("agent_type", "").startswith("cl9-"):
                self.assertIn("agent_id", stdin)
            if fx.get("expect"):
                out = G.run(dict(stdin, cwd=REPO if fx.get("root_override") else stdin["cwd"]))
                self.assertEqual(decision(out), fx["expect"], os.path.basename(p))


class Settings(unittest.TestCase):
    """settings.json 의 훅 명령은 절대 경로($CLAUDE_PROJECT_DIR) — 상대 경로면 cwd 가 하위 폴더인 세션의 셸이 전부 막힌다."""

    def test_hook_command_is_absolute(self):
        with open(os.path.join(REPO, ".claude", "settings.json"), "r", encoding="utf-8") as f:
            settings = json.load(f)
        cmds = [h["command"] for m in settings["hooks"]["PreToolUse"] for h in m["hooks"]]
        guard = [c for c in cmds if "cl9_scope_guard.py" in c]
        self.assertEqual(len(guard), 1)
        self.assertTrue(guard[0].startswith('py -3 "$CLAUDE_PROJECT_DIR/'), guard[0])

    def test_relative_script_path_would_block(self):
        """재현: 파이썬이 스크립트를 못 열면 종료 코드 2 = Claude Code 의 차단 코드."""
        p = subprocess.run(["py", "-3", ".claude/hooks/cl9_scope_guard.py"], input=b"{}", cwd=tempfile.gettempdir(),
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertEqual(p.returncode, 2)


if __name__ == "__main__":
    unittest.main()
