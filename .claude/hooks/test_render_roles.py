"""렌더러·상태 스크립트 테스트 — 소유 경로가 레지스트리 그대로인지, 도장·--check·수동 계약."""
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

import cl9_roles as R
import render_roles as RR

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))


class Render(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reg = R.load(REPO)

    def test_owned_paths_rendered_verbatim(self):
        for name in R.role_names(self.reg):
            body = RR.render_body(self.reg, name)
            role = self.reg["roles"][name]
            for p in role["owns"] + role["shared"] + role["except"]:
                self.assertIn("- `%s`" % p, body, "%s: %s" % (name, p))
            for d in role["docs"]:
                self.assertIn("- `%s`" % d, body)

    def test_stamp_matches_registry_hash(self):
        for name in R.role_names(self.reg):
            self.assertIn("sha256:%s" % R.registry_hash(REPO), RR.render_agent_file(self.reg, REPO, name))
            self.assertIn("sha256:%s" % R.registry_hash(REPO), RR.render_manual_contract(self.reg, REPO, name))

    def test_frontmatter(self):
        text = RR.render_agent_file(self.reg, REPO, "ui")
        head = text.split("---")[1]
        self.assertIn("name: cl9-ui", head)
        self.assertIn("model: opus", head)
        self.assertIn("maxTurns: 200", head)
        self.assertNotIn("Agent", head.split("tools:")[1].splitlines()[0])
        self.assertNotIn("mcp__", head)

    def test_balance_contract_sections(self):
        body = RR.render_body(self.reg, "balance")
        self.assertIn("`## 표 행`", body)
        self.assertIn("`## 검증(부등식)`", body)
        self.assertTrue(body.index("docs/plan/economy.md") < body.index("docs/plan/overview.md"))
        gp = RR.render_body(self.reg, "gameplay")
        self.assertNotIn("`## 표 행`", gp)

    def test_report_sections_in_order(self):
        body = RR.render_body(self.reg, "ui")
        idx = [body.index("`## %s`" % s) for s in RR.REPORT_SECTIONS]
        self.assertEqual(idx, sorted(idx))

    def test_manual_contract_has_lock_synopsis_and_announce(self):
        text = RR.render_manual_contract(self.reg, REPO, "ui")
        self.assertIn("cl9_lock.py acquire", text)
        self.assertIn("cl9-ui(수동): 영역 =", text)
        self.assertIn("docs/workflow/sessions.md#잠금", text)

    def test_committed_agents_match_registry(self):
        """.claude/agents/cl9-*.md 는 --write 산출물 그대로여야 한다(레지스트리가 바뀌면 --write 를 다시)."""
        ok, stale = RR.check_all(self.reg, REPO)
        self.assertTrue(ok, "레지스트리가 바뀜 — `py -3 .claude/hooks/render_roles.py --write`: %s" % stale)

    def test_write_and_check_roundtrip(self):
        tmp = tempfile.mkdtemp(prefix="cl9render-")
        try:
            os.makedirs(os.path.join(tmp, ".claude"))
            shutil.copy(os.path.join(REPO, ".claude", "cl9-roles.json"), os.path.join(tmp, ".claude", "cl9-roles.json"))
            reg = R.load(tmp)
            self.assertFalse(RR.check_all(reg, tmp)[0])
            files = RR.write_all(reg, tmp)
            self.assertEqual(len(files), 6)
            self.assertTrue(RR.check_all(reg, tmp)[0])
            with open(files[0], "a", encoding="utf-8") as f:
                f.write("\n손으로 고침\n")
            ok, stale = RR.check_all(reg, tmp)
            self.assertFalse(ok)
            self.assertEqual(stale, [files[0]])
        finally:
            shutil.rmtree(tmp, ignore_errors=True)

    def test_cli_print_and_alias(self):
        p = subprocess.run([sys.executable, os.path.join(HERE, "render_roles.py"), "--root", REPO, "--print", "밸런스"],
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertEqual(p.returncode, 0)
        out = p.stdout.decode("utf-8")
        self.assertIn("# cl9-balance", out)
        p = subprocess.run([sys.executable, os.path.join(HERE, "render_roles.py"), "--root", REPO, "--print", "없는역할"],
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertEqual(p.returncode, 1)

    def test_status_script_runs(self):
        tmp = tempfile.mkdtemp(prefix="cl9status-")
        try:
            p = subprocess.run([sys.executable, os.path.join(HERE, "cl9_status.py"), "--root", tmp],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertEqual(p.returncode, 0, p.stderr)
            self.assertIn("free", p.stdout.decode("utf-8"))
        finally:
            shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    unittest.main()
