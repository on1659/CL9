"""실행 관리(cl9_run.py)·밸런스 검사기 테스트."""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

import cl9_lock as L
import cl9_run as RUN

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(REPO, "Tools", "Balance"))
import check_inequalities as CI  # noqa: E402


def git(root, *args):
    subprocess.run(["git"] + list(args), cwd=root, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)


class RunBase(unittest.TestCase):
    def setUp(self):
        self.root = tempfile.mkdtemp(prefix="cl9run-")
        git(self.root, "init", "-q")
        git(self.root, "config", "user.email", "t@t")
        git(self.root, "config", "user.name", "t")
        os.makedirs(os.path.join(self.root, ".claude"))
        shutil.copy(os.path.join(REPO, ".claude", "cl9-roles.json"), os.path.join(self.root, ".claude", "cl9-roles.json"))
        for rel in ("Source/CL9/UI/CL9HUDWidget.cpp", "Source/CL9/Items/CL9Item.cpp", "docs/plan/data/item/egg.md", "docs/plan/economy.md"):
            p = os.path.join(self.root, rel)
            os.makedirs(os.path.dirname(p), exist_ok=True)
            with open(p, "w", encoding="utf-8") as f:
                f.write("| 키 | 값 |\n|---|---|\n| Price | 12 |\n")
        with open(os.path.join(self.root, ".gitignore"), "w") as f:
            f.write(".claude/runs/\n")
        git(self.root, "add", "-A")
        git(self.root, "commit", "-q", "-m", "base")

    def tearDown(self):
        shutil.rmtree(self.root, ignore_errors=True)

    def cli(self, *args):
        p = subprocess.run([sys.executable, os.path.join(HERE, "cl9_run.py"), "--root", self.root] + list(args),
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return p.returncode, p.stdout.decode("utf-8", "replace")

    def grants_file(self, name, obj):
        p = os.path.join(self.root, name)
        with open(p, "w", encoding="utf-8") as f:
            json.dump(obj, f)
        return p

    def write(self, rel, text):
        p = os.path.join(self.root, rel)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "w", encoding="utf-8") as f:
            f.write(text)


class Lifecycle(RunBase):
    def test_init_dispatch_audit_finish(self):
        g = self.grants_file("g.json", {"cl9-ui": ["Source/CL9/Items/CL9Item.cpp"], "cl9-gameplay": []})
        rc, out = self.cli("init", "hud-stamina", "--task", "스태미나 바 문턱", "--roles", "유아이,gameplay", "--grants", g)
        self.assertEqual(rc, 0, out)
        run = RUN.load_run(self.root, "hud-stamina")
        self.assertEqual(run["state"], "TRIAGE")
        self.assertEqual(run["roles"], ["ui", "gameplay"])
        with open(os.path.join(RUN.run_dir(self.root, "hud-stamina"), "grants.json"), encoding="utf-8") as f:
            self.assertEqual(json.load(f), {"cl9-ui": ["source/cl9/items/cl9item.cpp"], "cl9-gameplay": []})
        rc, out = self.cli("dispatch", "hud-stamina")
        self.assertEqual(rc, 0, out)
        self.assertEqual(RUN.load_run(self.root, "hud-stamina")["state"], "DISPATCHED")
        self.assertTrue(os.path.exists(os.path.join(RUN.run_dir(self.root, "hud-stamina"), "baseline.json")))
        # ui 가 grant 받은 파일 + 자기 파일 수정, 리드 전용 파일도 하나(위반)
        self.write("Source/CL9/Items/CL9Item.cpp", "changed by ui via grant")
        self.write("Source/CL9/UI/CL9HUDWidget.cpp", "changed")
        self.write("Config/DefaultGame.ini", "x")
        rc, out = self.cli("audit", "hud-stamina")
        self.assertEqual(rc, 1, out)
        self.assertIn("위반", out)
        self.assertIn("config/defaultgame.ini", out)
        self.assertEqual(RUN.load_run(self.root, "hud-stamina")["state"], "AUDITED")
        rc, out = self.cli("finish", "hud-stamina", "--state", "REPORTED", "--wall-min", "12", "--builds", "1", "--pie", "F2 OK")
        self.assertEqual(rc, 0, out)
        run = RUN.load_run(self.root, "hud-stamina")
        self.assertEqual((run["state"], run["wall_min"], run["builds"], run["pie_result"]), ("REPORTED", 12, 1, "F2 OK"))
        rc, out = self.cli("state", "hud-stamina", "PIE")
        self.assertEqual(rc, 1)
        self.assertIn("끝난 상태", out)

    def test_duplicate_and_bad_slug_and_unknown_role(self):
        rc, _ = self.cli("init", "a-b", "--task", "t", "--roles", "ui")
        self.assertEqual(rc, 0)
        rc, out = self.cli("init", "a-b", "--task", "t", "--roles", "ui")
        self.assertEqual(rc, 1)
        rc, out = self.cli("init", "Bad Slug", "--task", "t", "--roles", "ui")
        self.assertEqual(rc, 1)
        rc, out = self.cli("init", "c-d", "--task", "t", "--roles", "없는역할")
        self.assertEqual(rc, 1)
        self.assertIn("없는 역할", out)

    def test_transitions(self):
        self.cli("init", "r1", "--task", "t", "--roles", "docs")
        self.cli("dispatch", "r1")
        rc, out = self.cli("state", "r1", "BUILDING")
        self.assertEqual(rc, 1)
        self.assertIn("전이 거부", out)
        rc, out = self.cli("state", "r1", "BLOCKED_ON_QUESTION")
        self.assertEqual(rc, 0)
        rc, out = self.cli("state", "r1", "DISPATCHED")
        self.assertEqual(rc, 0)
        self.cli("audit", "r1")
        rc, out = self.cli("state", "r1", "REVIEWED")
        self.assertEqual(rc, 0)
        rc, out = self.cli("state", "r1", "BUILDING")
        self.assertEqual(rc, 1, "잠금 없이 BUILDING 불가")
        self.assertIn("배타 잠금", out)

    def test_overlap_intra_and_cross_run(self):
        g1 = self.grants_file("g1.json", {"cl9-ui": ["Source/CL9/Items/CL9Item.cpp"], "cl9-gameplay": ["Source/CL9/Items/CL9Item.cpp"]})
        rc, out = self.cli("overlap", "--grants", g1)
        self.assertEqual(rc, 1)
        self.assertIn("둘 다", out)
        g2 = self.grants_file("g2.json", {"cl9-ui": ["Source/CL9/Items/CL9Item.cpp"]})
        self.cli("init", "first", "--task", "t", "--roles", "ui", "--grants", g2)
        self.cli("dispatch", "first")
        g3 = self.grants_file("g3.json", {"cl9-gameplay": ["Source/CL9/Items/CL9Item.cpp"]})
        rc, out = self.cli("overlap", "--grants", g3)
        self.assertEqual(rc, 1)
        self.assertIn("실행 `first`", out)
        self.cli("init", "second", "--task", "t", "--roles", "gameplay", "--grants", g3)
        rc, out = self.cli("dispatch", "second")
        self.assertEqual(rc, 1)
        self.assertIn("겹침", out)
        self.cli("finish", "first", "--state", "REPORTED")
        rc, out = self.cli("dispatch", "second")
        self.assertEqual(rc, 0, out)

    def test_validate_report(self):
        self.cli("init", "r1", "--task", "t", "--roles", "ui,balance")
        d = RUN.run_dir(self.root, "r1")
        good = "".join("## %s\n없음\n\n" % s for s in RUN.REPORT_SECTIONS)
        with open(os.path.join(d, "report-ui.md"), "w", encoding="utf-8") as f:
            f.write(good)
        rc, out = self.cli("validate-report", "r1", "ui")
        self.assertEqual(rc, 0, out)
        with open(os.path.join(d, "report-balance.md"), "w", encoding="utf-8") as f:
            f.write(good)
        rc, out = self.cli("validate-report", "r1", "balance")
        self.assertEqual(rc, 1)
        self.assertIn("표 행", out)
        self.assertIn("검증(부등식)", out)
        with open(os.path.join(d, "report-ui.md"), "w", encoding="utf-8") as f:
            f.write("## 바꾼 파일\n\n## 한 일\n" + good)
        rc, out = self.cli("validate-report", "r1", "ui")
        self.assertEqual(rc, 1)
        self.assertIn("순서", out)
        rc, out = self.cli("validate-report", "r1", "gameplay")
        self.assertEqual(rc, 1)
        self.assertIn("보고서 없음", out)

    def test_latest_and_show(self):
        self.cli("init", "old", "--task", "t", "--roles", "ui")
        self.cli("finish", "old", "--state", "REPORTED")
        self.cli("init", "new", "--task", "t", "--roles", "ui")
        rc, out = self.cli("latest", "--not-reported")
        self.assertEqual(out.split()[0], "new")
        rc, out = self.cli("show", "new")
        self.assertEqual(rc, 0)
        self.assertIn("brief 없음", out)


class Inequalities(unittest.TestCase):
    def data(self, run=654, walk=396, drain=6.0, regen=8.0, chase=396, sight=330, catch=84, danger=1.3):
        return {"character": {"WalkSpeed": walk, "RunSpeed": run, "StaminaDrainTime": drain, "StaminaRegenTime": regen},
                "monsters": {"chicken": {"ChaseSpeed": chase, "CatchRadius": catch, "SightIn": sight},
                             "goose": {"ChaseSpeed": 528, "CatchRadius": None, "SightIn": 300}},
                "danger_max": danger}

    def test_current_docs_pass(self):
        res = CI.evaluate(CI.load(REPO))
        self.assertTrue(res["readable"], res["lines"])
        self.assertTrue(res["ok"], res["lines"])
        self.assertIn("A1.chicken", res["results"])
        self.assertIn("A1.raccoon", res["results"])
        self.assertIsNone(res["results"]["A4.chicken"])

    def test_a1_fails_by_one_cm_per_s(self):
        # 지친 빈손 506.57 — 추격을 507.57 로 올리면 1 cm/s 차이로 실패
        res = CI.evaluate(self.data(chase=507.57))
        self.assertFalse(res["ok"])
        self.assertFalse(res["results"]["A1.chicken"])
        res = CI.evaluate(self.data(chase=505.57))
        self.assertTrue(res["results"]["A1.chicken"])

    def test_a2_uses_curve_max_not_base(self):
        # 예금 3924 — 시야 3200 은 표 값으론 통과하지만 ×1.3 = 4160 이라 실패
        res = CI.evaluate(self.data(sight=3200))
        self.assertFalse(res["results"]["A2.chicken"])
        self.assertIn("표 값 기준 3200 도 OK", " ".join(res["lines"]))
        res = CI.evaluate(self.data(sight=3200, danger=1.0))
        self.assertTrue(res["results"]["A2.chicken"])

    def test_a4_refused_and_goose_unknown(self):
        res = CI.evaluate(self.data())
        text = " ".join(res["lines"])
        self.assertIn("A4 chicken: 미결 — 식 없음", text)
        self.assertIsNone(res["results"]["A4.chicken"])
        self.assertIn("판정 불가: goose", text)
        self.assertNotIn("A1.goose", res["results"])

    def test_unreadable(self):
        d = self.data()
        d["character"]["RunSpeed"] = None
        res = CI.evaluate(d)
        self.assertFalse(res["readable"])

    def test_header_aware_parser(self):
        tmp = tempfile.mkdtemp(prefix="cl9ci-")
        try:
            p = os.path.join(tmp, "t.md")
            with open(p, "w", encoding="utf-8") as f:
                f.write("| 컬럼 | 뜻 | 기본값 | 구 |\n|---|---|---|---|\n| `WalkSpeed` | 걷기 | 396 | x |\n\n"
                        "| 컬럼 | 값 |\n|---|---|\n| `SightIn` / `SightOut` / `SightDark` | 390 / 260 / 130 |\n")
            rows = CI.read_rows(p)
            self.assertEqual(CI.number(rows["WalkSpeed"]), 396)
            self.assertEqual(CI.number(rows["SightOut"]), 260)
        finally:
            shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    unittest.main()
