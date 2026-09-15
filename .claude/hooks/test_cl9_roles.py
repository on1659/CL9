"""레지스트리 로더 테스트 — `py -3 -m unittest discover .claude/hooks`."""
import json
import os
import shutil
import tempfile
import unittest

import cl9_roles as R

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))


class RegistryOnRepo(unittest.TestCase):
    """실제 레지스트리 + 실제 파일 트리."""

    @classmethod
    def setUpClass(cls):
        cls.reg = R.load(REPO)
        cls.files = R.tree_files(REPO)

    def test_six_roles(self):
        self.assertEqual(sorted(R.role_names(self.reg)), ["art", "balance", "docs", "gameplay", "map", "ui"])

    def test_every_pattern_matches_a_file(self):
        """폴더 이름이 바뀌면 조용히 울타리가 풀리는 대신 여기서 실패한다."""
        empty = [(where, p) for where, p in R.all_patterns(self.reg) if not R.expand(p, self.files)]
        self.assertEqual(empty, [])

    def test_pairwise_shared_intersections_are_grant_gated(self):
        inter = R.pairwise_shared_intersections(self.reg, self.files)
        self.assertIn(("gameplay", "balance"), inter)
        for pair, paths in inter.items():
            for f in paths:
                self.assertTrue(R.is_grant_gated(self.reg, f), "%s %s 는 grant-gated 여야" % (pair, f))

    def test_balance_canon_starts_with_economy(self):
        self.assertEqual(self.reg["roles"]["balance"]["docs"][0], "docs/plan/economy.md")

    def test_aliases(self):
        for alias, role in (("전투", "gameplay"), ("시스템", "gameplay"), ("유아이", "ui"), ("맵", "map"),
                            ("아트", "art"), ("문서", "docs"), ("밸런스", "balance"), ("수치", "balance"),
                            ("cl9-ui", "ui"), ("ui", "ui")):
            self.assertEqual(R.resolve_role(self.reg, alias), role)
        self.assertIsNone(R.resolve_role(self.reg, "없는역할"))

    def test_defaults_and_overrides(self):
        self.assertEqual(self.reg["roles"]["gameplay"]["model"], "opus")
        self.assertEqual(self.reg["roles"]["gameplay"]["max_turns"], 200)
        self.assertEqual(self.reg["roles"]["balance"]["default_mode"], "A")
        self.assertEqual(self.reg["roles"]["ui"]["default_mode"], "C")

    def test_classify_matrix(self):
        reg = self.reg
        cases = [
            ("gameplay", "source/cl9/items/cl9item.cpp", (), "owns"),
            ("ui", "source/cl9/items/cl9item.cpp", (), "outside"),
            ("ui", "source/cl9/items/cl9item.cpp", ("source/cl9/items/cl9item.cpp",), "grant"),
            ("ui", "config/localization/cl9_gather.ini", (), "owns"),
            ("ui", "config/defaultgame.ini", (), "lead_only"),
            ("gameplay", "source/cl9/data/cl9sitedef.h", (), "outside"),
            ("map", "source/cl9/data/cl9sitedef.h", (), "owns"),
            ("gameplay", "docs/plan/data/item/egg.md", (), "gated"),
            ("gameplay", "docs/plan/data/item/egg.md", ("docs/plan/data/item/egg.md",), "shared"),
            ("balance", "docs/plan/data/item/egg.md", ("docs/plan/data/item/*",), "shared"),
            ("balance", "docs/plan/loop.md", (), "shared"),
            ("balance", "source/cl9/items/cl9item.cpp", (), "outside"),
            ("art", "content/data/itemtable.uasset", (), "lead_only"),
            ("art", "content/maps/lvl_flat.umap", (), "lead_only"),
            ("docs", "docs/decisions.md", (), "append_only"),
            ("docs", "docs/systems/item.md", (), "shared"),
            ("ui", "source/cl9/ui-old/x.h", (), "outside"),
            ("gameplay", ".claude/cl9-roles.json", (), "lead_only"),
            ("art", "tools/blender/build_cart_a.py", (), "owns"),
        ]
        for role, rel, grants, expected in cases:
            kind, _ = R.classify(reg, role, rel, grants)
            self.assertEqual(kind, expected, "%s %s grants=%s" % (role, rel, grants))

    def test_owner_label(self):
        self.assertEqual(R.owner_label(self.reg, "source/cl9/items/cl9item.cpp"), "gameplay 영역")
        self.assertEqual(R.owner_label(self.reg, "source/cl9/ui-old/x.h"), "어느 역할도 안 가짐")
        self.assertIn("리드 전용", R.owner_label(self.reg, "content/data/itemtable.uasset"))
        self.assertIn("append-only", R.owner_label(self.reg, "docs/decisions.md"))

    def test_to_rel_normalization(self):
        self.assertEqual(R.to_rel(os.path.join(REPO, "Source", "CL9", "UI", "CL9Widget.h"), REPO), "source/cl9/ui/cl9widget.h")
        self.assertEqual(R.to_rel("Source/CL9/UI/../Items/CL9Item.cpp", REPO), "source/cl9/items/cl9item.cpp")
        self.assertEqual(R.to_rel(REPO.upper() + "\\docs\\decisions.md", REPO), "docs/decisions.md")
        self.assertIsNone(R.to_rel(os.path.join(REPO, "..", "CCL8", "x.md"), REPO))
        self.assertIsNone(R.to_rel("C:\\Windows\\notepad.exe", REPO))
        self.assertIsNone(R.to_rel("", REPO))

    def test_directory_prefix_does_not_bleed(self):
        self.assertTrue(R.match("source/cl9/ui/a/b.h", "source/cl9/ui/*"))
        self.assertFalse(R.match("source/cl9/ui-old/b.h", "source/cl9/ui/*"))
        self.assertTrue(R.match("content/__externalactors__/x.uasset", "*.uasset"))

    def test_hash_is_stable(self):
        self.assertEqual(R.registry_hash(REPO), R.registry_hash(REPO))
        self.assertEqual(len(R.registry_hash(REPO)), 16)


class RegistryDialect(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="cl9reg-")
        os.makedirs(os.path.join(self.tmp, ".claude"))

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _write(self, obj):
        with open(os.path.join(self.tmp, ".claude", "cl9-roles.json"), "w", encoding="utf-8") as f:
            json.dump(obj, f)

    def test_reject_braces_and_doublestar(self):
        for bad in ("docs/{a,b}.md", "Source/**/x.h", "C:\\abs\\x", "/abs/x", ""):
            self._write({"roles": {"r": {"owns": [bad]}}})
            with self.assertRaises(R.RegistryError, msg=bad):
                R.load(self.tmp)

    def test_missing_file(self):
        with self.assertRaises(R.RegistryError):
            R.load(self.tmp)

    def test_alias_to_unknown_role(self):
        self._write({"roles": {"r": {"owns": ["a/*"]}}, "aliases": {"x": "nope"}})
        with self.assertRaises(R.RegistryError):
            R.load(self.tmp)

    def test_except_carves_out(self):
        self._write({"roles": {"r": {"owns": ["a/*"], "except": ["a/keep.*"]}}})
        reg = R.load(self.tmp)
        self.assertEqual(R.classify(reg, "r", "a/x.h")[0], "owns")
        self.assertEqual(R.classify(reg, "r", "a/keep.h")[0], "outside")


if __name__ == "__main__":
    unittest.main()
