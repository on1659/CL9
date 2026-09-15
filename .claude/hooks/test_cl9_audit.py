"""기준선 감사 테스트 — 임시 git 저장소."""
import json
import os
import shutil
import subprocess
import tempfile
import unittest

import cl9_audit as A
import cl9_lock as L
import cl9_roles as R

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))

TABLE_OLD = """# 달걀

| 키 | 값 | 단위 |
|---|---|---|
| Price | 12 | 원 |
| Weight | 0.5 | kg |
| Note | 깨진다 | - |

설명 문단.
"""


def git(root, *args):
    subprocess.run(["git"] + list(args), cwd=root, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)


class AuditRepo(unittest.TestCase):
    def setUp(self):
        self.root = tempfile.mkdtemp(prefix="cl9audit-")
        git(self.root, "init", "-q")
        git(self.root, "config", "user.email", "t@t")
        git(self.root, "config", "user.name", "t")
        os.makedirs(os.path.join(self.root, ".claude"))
        shutil.copy(os.path.join(REPO, ".claude", "cl9-roles.json"), os.path.join(self.root, ".claude", "cl9-roles.json"))
        self.write("Source/CL9/UI/CL9Widget.h", "a")
        self.write("Source/CL9/Items/CL9Item.cpp", "a")
        self.write("docs/plan/data/item/egg.md", TABLE_OLD)
        self.write("docs/decisions.md", "d")
        self.write(".gitignore", ".claude/runs/\n")
        git(self.root, "add", "-A")
        git(self.root, "commit", "-q", "-m", "base")
        # 다른 세션이 남긴 더러운 파일(기준선 이전)
        self.write("Source/CL9/Items/CL9Item.cpp", "dirty-by-other-session")
        self.write("docs/untracked-other.md", "x")
        self.reg = R.load(self.root)
        self.run_dir = os.path.join(self.root, ".claude", "runs", "r1")
        self.base = A.snapshot(self.root, self.run_dir, self.reg)

    def tearDown(self):
        shutil.rmtree(self.root, ignore_errors=True)

    def write(self, rel, text):
        p = os.path.join(self.root, rel)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "w", encoding="utf-8") as f:
            f.write(text)

    def test_snapshot_contents(self):
        self.assertIn("source/cl9/items/cl9item.cpp", self.base["porcelain"])
        self.assertIn("docs/untracked-other.md", self.base["porcelain"])
        self.assertIn("docs/plan/data/item/egg.md", self.base["gated_files"])
        self.assertTrue(os.path.exists(os.path.join(self.run_dir, A.BASELINE_PATCH)))
        with open(os.path.join(self.run_dir, A.BASELINE_PATCH), "rb") as f:
            self.assertIn(b"dirty-by-other-session", f.read())

    def test_pre_existing_dirt_is_not_a_violation(self):
        res = A.audit(self.root, self.run_dir, self.reg, ["ui"], {})
        self.assertEqual(res["changed"], [])
        self.assertEqual(res["violations"], [])

    def test_delta_mapped_to_roles_and_unclaimed_is_violation(self):
        self.write("Source/CL9/UI/CL9Widget.h", "ui-edit")
        self.write("Source/CL9/Items/CL9Item.cpp", "dirty-by-other-session + ui touched it")
        self.write("Source/CL9/UI/New.h", "new")
        res = A.audit(self.root, self.run_dir, self.reg, ["ui"], {})
        self.assertEqual(res["claimed"], {"source/cl9/ui/cl9widget.h": "ui", "source/cl9/ui/new.h": "ui"})
        self.assertEqual([v[0] for v in res["violations"]], ["source/cl9/items/cl9item.cpp"])
        self.assertIn("gameplay 영역", res["violations"][0][1])
        res = A.audit(self.root, self.run_dir, self.reg, ["ui"], {"ui": ["source/cl9/items/cl9item.cpp"]})
        self.assertEqual(res["violations"], [])

    def test_reverting_foreign_dirt_is_a_violation(self):
        git(self.root, "checkout", "--", "Source/CL9/Items/CL9Item.cpp")
        res = A.audit(self.root, self.run_dir, self.reg, ["ui"], {})
        self.assertEqual([v[0] for v in res["violations"]], ["source/cl9/items/cl9item.cpp"])

    def test_claude_config_and_guard_off_are_violations(self):
        self.write(".claude/settings.json", "{}")
        self.write("Config/DefaultGame.ini", "x")
        L.guard_off(self.root)
        res = A.audit(self.root, self.run_dir, self.reg, ["ui"], {})
        paths = [v[0] for v in res["violations"]]
        self.assertIn(".claude/settings.json", paths)
        self.assertIn("config/defaultgame.ini", paths)
        self.assertIn(".claude/runs/guard.off", paths)
        self.assertTrue(res["guard_off"])

    def test_balance_numeric_change_passes(self):
        self.write("docs/plan/data/item/egg.md", TABLE_OLD.replace("| Price | 12 |", "| Price | 15 |").replace("| Weight | 0.5 |", "| Weight | 0.6 |"))
        changed = A.changed_since(self.root, self.base)
        self.assertEqual(changed, ["docs/plan/data/item/egg.md"])
        self.assertEqual(A.balance_audit(self.root, self.run_dir, changed), {})

    def test_balance_renamed_column_is_violation(self):
        self.write("docs/plan/data/item/egg.md", TABLE_OLD.replace("| 키 | 값 | 단위 |", "| 키 | 기본값 | 단위 |"))
        v = A.balance_audit(self.root, self.run_dir, ["docs/plan/data/item/egg.md"])
        self.assertIn("docs/plan/data/item/egg.md", v)
        self.assertTrue(any("수치 셀이 아닌 변경" in r for _, r in v["docs/plan/data/item/egg.md"]))

    def test_balance_row_key_new_row_prose_are_violations(self):
        cases = {
            "행 키": TABLE_OLD.replace("| Price | 12 |", "| Cost | 12 |"),
            "새 행": TABLE_OLD.replace("| Weight | 0.5 | kg |\n", "| Weight | 0.5 | kg |\n| Speed | 3 | m/s |\n"),
            "산문": TABLE_OLD.replace("설명 문단.", "설명 문단을 고쳤다."),
            "텍스트 셀": TABLE_OLD.replace("| Note | 깨진다 |", "| Note | 안 깨진다 |"),
        }
        for name, text in cases.items():
            self.assertTrue(A.balance_diff_shape(TABLE_OLD, text), name)
        self.assertEqual(A.balance_diff_shape(TABLE_OLD, TABLE_OLD), [])
        self.assertEqual(A.balance_diff_shape(TABLE_OLD, TABLE_OLD.replace("| 12 |", "| 12.5 |")), [])


if __name__ == "__main__":
    unittest.main()
