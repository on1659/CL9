"""에디터/빌드 락 테스트 — `py -3 -m unittest discover .claude/hooks`."""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

import cl9_lock as L

HERE = os.path.dirname(os.path.abspath(__file__))
LOCK_PY = os.path.join(HERE, "cl9_lock.py")


def spawn_child(seconds=60):
    """살아 있는 진짜 Windows 프로세스(PID 검증용) — 테스트 끝에 죽인다."""
    return subprocess.Popen(["ping", "-n", str(seconds), "127.0.0.1"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


class LockBase(unittest.TestCase):
    def setUp(self):
        self.root = tempfile.mkdtemp(prefix="cl9lock-")
        self._now = None
        self._orig_now = L.now
        L.now = lambda: self._now if self._now is not None else time.time()

    def tearDown(self):
        L.now = self._orig_now
        shutil.rmtree(self.root, ignore_errors=True)

    def read(self):
        with open(L.lock_path(self.root), "r", encoding="utf-8") as f:
            return json.load(f)


class AcquireRelease(LockBase):
    def test_acquire_then_busy_then_release(self):
        L.acquire(self.root, "A", "pie")
        with self.assertRaises(L.LockError) as cm:
            L.acquire(self.root, "B", "pie")
        msg = str(cm.exception)
        self.assertIn("세션 A", msg)
        self.assertIn("release --force", msg)
        self.assertIn(L.DOC_ANCHOR, msg)
        L.release(self.root, "A")
        L.acquire(self.root, "B", "pie")
        self.assertEqual(self.read()["exclusive"]["session_id"], "B")

    def test_owner_reacquire_idempotent(self):
        L.acquire(self.root, "A", "pie")
        since = self.read()["exclusive"]["since"]
        L.acquire(self.root, "A", "compile")
        ex = self.read()["exclusive"]
        self.assertEqual(ex["purpose"], "compile")
        self.assertEqual(ex["since"], since)

    def test_release_others_lock_refused_without_force(self):
        L.acquire(self.root, "A", "pie")
        with self.assertRaises(L.LockError):
            L.release(self.root, "B")
        L.release(self.root, "B", force=True)
        self.assertIsNone(self.read()["exclusive"])

    def test_build_requires_pid(self):
        with self.assertRaises(L.LockError) as cm:
            L.acquire(self.root, "A", "build")
        self.assertIn("--pid", str(cm.exception))
        with self.assertRaises(L.LockError):
            L.acquire(self.root, "A", "commandlet")

    def test_dead_pid_refused(self):
        with self.assertRaises(L.LockError):
            L.acquire(self.root, "A", "build", pid=4000000)

    def test_expiry_steal_when_process_dead(self):
        self._now = 1000.0
        L.acquire(self.root, "A", "pie", minutes=1)
        self._now = 1000.0 + 61
        L.acquire(self.root, "B", "pie")
        self.assertEqual(self.read()["exclusive"]["session_id"], "B")

    def test_expired_but_process_alive_keeps_lock(self):
        child = spawn_child()
        try:
            self._now = 1000.0
            L.acquire(self.root, "A", "build", minutes=1, pid=child.pid)
            self._now = 1000.0 + 3600
            with self.assertRaises(L.LockError) as cm:
                L.acquire(self.root, "B", "pie")
            self.assertIn("살아 있음", str(cm.exception))
            with self.assertRaises(L.LockError):
                L.release(self.root, "B", force=True)
        finally:
            child.kill()
            child.wait()

    def test_pid_reuse_detected_by_create_time(self):
        child = spawn_child()
        try:
            L.acquire(self.root, "A", "build", pid=child.pid)
            data = self.read()
            self.assertEqual(data["exclusive"]["proc"]["image"].lower(), "ping.exe")
            data["exclusive"]["proc"]["create_time"] -= 1  # 같은 PID, 다른 프로세스인 척
            with open(L.lock_path(self.root), "w", encoding="utf-8") as f:
                json.dump(data, f)
            self.assertFalse(L.proc_alive(data["exclusive"]["proc"]))
            data["exclusive"]["proc"]["create_time"] += 1
            self.assertTrue(L.proc_alive(data["exclusive"]["proc"]))
        finally:
            child.kill()
            child.wait()

    def test_force_after_process_died(self):
        child = spawn_child()
        L.acquire(self.root, "A", "build", pid=child.pid)
        child.kill()
        child.wait()
        L.release(self.root, "B", force=True)
        self.assertIsNone(self.read()["exclusive"])

    def test_leases_block_exclusive_until_released_or_expired(self):
        self._now = 1000.0
        L.renew_lease(self.root, "C")
        with self.assertRaises(L.LockError) as cm:
            L.acquire(self.root, "A", "pie")
        self.assertIn("에디터 임대", str(cm.exception))
        self.assertIn("세션 C", str(cm.exception))
        L.release(self.root, "C")
        L.acquire(self.root, "A", "pie")
        L.release(self.root, "A")
        L.renew_lease(self.root, "C")
        self._now = 1000.0 + 31 * 60
        L.acquire(self.root, "A", "pie")

    def test_own_lease_does_not_block_self(self):
        L.renew_lease(self.root, "A")
        L.acquire(self.root, "A", "pie")
        self.assertTrue(L.holds_exclusive(self.root, "A"))

    def test_wait_is_capped(self):
        L.acquire(self.root, "A", "pie")
        orig = L.WAIT_CAP_S
        L.WAIT_CAP_S = 1
        try:
            t0 = time.time()
            with self.assertRaises(L.LockError) as cm:
                L.acquire(self.root, "B", "pie", wait=500)
            self.assertLess(time.time() - t0, 4)
            self.assertIn("1초 기다렸지만", str(cm.exception))
        finally:
            L.WAIT_CAP_S = orig

    def test_malformed_lock_file(self):
        os.makedirs(L.runs_dir(self.root))
        with open(L.lock_path(self.root), "w") as f:
            f.write("{not json")
        with self.assertRaises(L.LockError):
            L.holder(self.root)
        with self.assertRaises(L.LockError):
            L.acquire(self.root, "A", "pie")


class Race(LockBase):
    def test_n_process_acquire_race_exactly_one_winner(self):
        n = 8
        procs = [subprocess.Popen([sys.executable, LOCK_PY, "--root", self.root, "--session", "S%d" % i, "acquire", "pie"],
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE) for i in range(n)]
        codes = []
        for p in procs:
            p.communicate()
            codes.append(p.returncode)
        self.assertEqual(codes.count(0), 1, codes)
        self.assertEqual(codes.count(1), n - 1, codes)
        winner = self.read()["exclusive"]["session_id"]
        self.assertTrue(winner.startswith("S"))


class GuardAndNonce(LockBase):
    def test_guard_off_then_expire(self):
        self._now = 1000.0
        L.guard_off(self.root, minutes=1)
        self.assertIsNotNone(L.guard_is_off(self.root))
        self._now = 1000.0 + 61
        self.assertIsNone(L.guard_is_off(self.root))
        self.assertFalse(os.path.exists(L.guard_off_path(self.root)))

    def test_guard_on(self):
        L.guard_off(self.root)
        L.guard_on(self.root)
        self.assertIsNone(L.guard_is_off(self.root))

    def test_nonce_marker(self):
        self.assertFalse(L.hook_seen_check(self.root, "abc"))
        L.hook_seen_mark(self.root, "abc")
        self.assertTrue(L.hook_seen_check(self.root, "abc"))
        self.assertFalse(L.hook_seen_check(self.root, "abc"))

    def test_hook_lock_mark(self):
        L.hook_lock_mark(self.root, "acquire", "pie")
        self.assertFalse(L.hook_lock_handled(self.root, "acquire", "build"))
        L.hook_lock_mark(self.root, "acquire", "pie")
        self.assertTrue(L.hook_lock_handled(self.root, "acquire", "pie"))
        self.assertFalse(L.hook_lock_handled(self.root, "acquire", "pie"))


class Tasklist(unittest.TestCase):
    def test_korean_locale_info_line_ignored(self):
        text = '정보: 실행 중인 작업이 없습니다.\r\n'
        self.assertEqual(L.parse_tasklist_csv(text, "UnrealEditor.exe"), [])
        text = '"UnrealEditor.exe","1234","Console","1","2,048,000 K"\r\n"UnrealEditor.exe","5678","Console","1","1 K"\r\n'
        self.assertEqual(L.parse_tasklist_csv(text, "UnrealEditor.exe"), [("UnrealEditor.exe", 1234), ("UnrealEditor.exe", 5678)])
        self.assertEqual(L.parse_tasklist_csv(text, "UnrealEditor-Cmd.exe"), [])

    def test_real_tasklist_runs(self):
        self.assertIsInstance(L.tasklist_pids(("explorer.exe",)), list)


class Cli(LockBase):
    def run_cli(self, *args):
        p = subprocess.run([sys.executable, LOCK_PY, "--root", self.root] + list(args),
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return p.returncode, p.stdout.decode("utf-8", "replace")

    def test_status_free_and_hookless_acquire(self):
        rc, out = self.run_cli("status")
        self.assertEqual(rc, 0)
        self.assertIn("free", out)
        rc, out = self.run_cli("acquire", "pie")
        self.assertEqual(rc, 1)
        self.assertIn("훅 비활성", out)

    def test_status_nonce_without_hook(self):
        rc, out = self.run_cli("status", "--nonce", "zzz")
        self.assertIn("비활성", out)
        L.hook_seen_mark(self.root, "zzz")
        rc, out = self.run_cli("status", "--nonce", "zzz")
        self.assertIn("활성(", out)

    def test_status_json(self):
        rc, out = self.run_cli("--session", "A", "acquire", "pie")
        self.assertEqual(rc, 0)
        rc, out = self.run_cli("status", "--json")
        self.assertEqual(json.loads(out)["exclusive"]["session_id"], "A")


if __name__ == "__main__":
    unittest.main()
