"""빌드 브로커 테스트 — UBT 없이 가짜 실행기로. `py -3 -m unittest discover Tools/Build`."""
import http.client
import json
import os
import shutil
import sys
import tempfile
import threading
import time
import unittest
import urllib.request
from http.server import ThreadingHTTPServer

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_broker as B  # noqa: E402

OK = {"ok": True, "rc": 0, "elapsed": 5, "log": "x.log", "lines": []}


class Clock:
    def __init__(self):
        self.t = 1000.0

    def __call__(self):
        return self.t


class BrokerBase(unittest.TestCase):
    def setUp(self):
        self.root = tempfile.mkdtemp(prefix="cl9broker-")
        self.clock = Clock()
        self.calls = []

    def tearDown(self):
        shutil.rmtree(self.root, ignore_errors=True)

    def runner(self, result=None):
        def run(kind, force):
            self.calls.append((kind, force))
            return result if result is not None else OK
        return run

    def make(self, runner=None, **kw):
        return B.Broker(self.root, runner or self.runner(), clock=self.clock, coalesce_s=30, idle_exit_s=900, inline=True, **kw)


class Batching(BrokerBase):
    def test_three_requests_one_build(self):
        b = self.make()
        t1 = b.submit("A", "build", "r1")
        t2 = b.submit("B", "build", "r2")
        t3 = b.submit("A", "build", "r3")  # 같은 세션·종류가 줄 서 있으면 새 티켓이 아니다
        self.assertEqual(t3["id"], t1["id"])
        b.tick()
        self.assertEqual(self.calls, [])  # 묶음 대기
        self.clock.t += 31
        b.tick()
        self.assertEqual(self.calls, [("build", False)])
        v1, v2 = b.wait(t1["id"], 0), b.wait(t2["id"], 0)
        self.assertEqual(v1["state"], "done")
        self.assertEqual(v1["result"]["batch"], v2["result"]["batch"])
        self.assertEqual([q["session"] for q in v1["result"]["requesters"]], ["A", "B"])
        self.assertEqual(v1["result"]["requesters"][0]["reason"], "r3")

    def test_other_kind_waits_for_next_batch(self):
        b = self.make()
        tb = b.submit("A", "build", "x")
        tc = b.submit("B", "compile", "y")
        self.clock.t += 31
        b.tick()
        self.assertEqual(self.calls, [("build", False)])
        self.assertEqual(b.wait(tc["id"], 0)["state"], "queued")
        b.tick()
        self.assertEqual(self.calls[1], ("compile", False))
        self.assertEqual(b.wait(tb["id"], 0)["state"], "done")

    def test_bad_kind(self):
        with self.assertRaises(ValueError):
            self.make().submit("A", "pie", "x")


class UserControl(BrokerBase):
    def test_hold_then_resume(self):
        b = self.make()
        b.submit("A", "build", "x")
        self.clock.t += 31
        b.ctl("hold")
        b.tick()
        self.assertEqual(self.calls, [])
        self.assertIn("보류", b.status()["blocked"])
        b.ctl("resume")
        b.tick()
        self.assertEqual(len(self.calls), 1)

    def test_start_in(self):
        b = self.make()
        b.submit("A", "build", "x")
        self.clock.t += 31
        b.ctl("start_in", minutes=5)
        b.tick()
        self.assertEqual(self.calls, [])
        self.clock.t += 300
        b.tick()
        self.assertEqual(len(self.calls), 1)
        self.assertIsNone(b.status()["start_at"])

    def test_now_ignores_hold_and_coalesce_then_resets(self):
        b = self.make()
        b.submit("A", "build", "x")
        b.ctl("hold")
        b.ctl("start_in", minutes=60)
        self.assertTrue(b.ctl("now")["ok"])
        b.tick()  # 시간이 안 지났어도
        self.assertEqual(self.calls, [("build", True)])
        self.assertFalse(b.status()["force"])

    def test_now_on_empty_queue_refused(self):
        self.assertFalse(self.make().ctl("now")["ok"])

    def test_cancel(self):
        b = self.make()
        t = b.submit("A", "build", "x")
        self.assertTrue(b.ctl("cancel", ticket=t["id"])["ok"])
        self.assertEqual(b.wait(t["id"], 0)["state"], "cancelled")
        self.clock.t += 31
        b.tick()
        self.assertEqual(self.calls, [])

    def test_blocked_runner_keeps_ticket_and_retries(self):
        answers = [{"blocked": "에디터 임대: 세션 abcd"}, OK]

        def run(kind, force):
            return answers.pop(0)
        b = self.make(run)
        t = b.submit("A", "build", "x")
        self.clock.t += 31
        b.tick()
        v = b.wait(t["id"], 0)
        self.assertEqual(v["state"], "queued")
        self.assertIn("에디터 임대", v["blocked"])
        b.tick()
        self.assertEqual(b.wait(t["id"], 0)["state"], "done")


class Failure(BrokerBase):
    def setUp(self):
        super().setUp()
        os.makedirs(os.path.join(self.root, ".claude"))
        with open(os.path.join(self.root, ".claude", "cl9-roles.json"), "w", encoding="utf-8") as f:
            json.dump({"roles": {"gameplay": {"owns": ["source/cl9/*"]}, "ui": {"owns": ["content/ui/*"], "shared": ["source/cl9/hud.cpp"]}}}, f)
        os.makedirs(os.path.join(self.root, "Source", "CL9"))
        self.src = os.path.join(self.root, "Source", "CL9", "Hud.cpp")
        open(self.src, "w").close()

    def fail(self, extra=()):
        lines = ["%s(12,5): error C2065: 'Foo': undeclared identifier" % self.src,
                 "%s(12,5): error C2065: 'Foo': undeclared identifier" % self.src,  # 중복은 하나로
                 "CL9.cpp.obj : error LNK2019: unresolved external symbol bar", "무관한 줄"] + list(extra)
        return {"ok": False, "rc": 6, "elapsed": 9, "log": "x.log", "lines": lines}

    def test_errors_get_owner_and_flags(self):
        b = self.make(self.runner(self.fail()))
        t = b.submit("A", "compile", "x")
        self.clock.t += 31
        b.tick()
        errs = b.wait(t["id"], 0)["result"]["errors"]
        self.assertEqual(len(errs), 2)
        e = errs[0]
        self.assertEqual((e["file"], e["line"], e["code"]), ("Source/CL9/Hud.cpp", 12, "C2065"))
        self.assertEqual(e["owns"], ["gameplay"])
        self.assertEqual(e["shared"], ["ui"])
        self.assertEqual(errs[1]["file"], "")
        self.assertEqual(errs[1]["code"], "LNK2019")

    def test_newer_than_last_ok(self):
        results = [OK, self.fail()]
        b = self.make(lambda k, f: results.pop(0))
        b.submit("A", "compile", "x")
        self.clock.t += 31
        b.tick()  # 성공 → last_ok = 지금(가짜 시계 1031 — 실제 파일 mtime 보다 한참 과거)
        b.last_ok = os.path.getmtime(self.src) - 100
        t = b.submit("A", "compile", "y")
        self.clock.t += 31
        b.tick()
        self.assertTrue(b.wait(t["id"], 0)["result"]["errors"][0]["newer_than_last_ok"])

    def test_dirty_flag(self):
        errs = B.analyze(["%s(1): error C1: m" % self.src], self.root, B.load_registry(self.root), {"source/cl9/hud.cpp"}, None)
        self.assertTrue(errs[0]["dirty"])
        errs = B.analyze(["%s(1): error C1: m" % self.src], self.root, B.load_registry(self.root), set(), None)
        self.assertFalse(errs[0]["dirty"])

    def test_same_error_twice_needs_user_then_success_clears(self):
        results = [self.fail(), self.fail(), OK]
        b = self.make(lambda k, f: results.pop(0))
        for expect in (False, True):
            t = b.submit("A", "compile", "x")
            self.clock.t += 31
            b.tick()
            self.assertEqual(b.wait(t["id"], 0)["result"]["needs_user"], expect)
        self.assertTrue(b.status()["needs_user"])
        t = b.submit("A", "compile", "x")
        self.clock.t += 31
        b.tick()
        self.assertFalse(b.status()["needs_user"])

    def test_compile_success_does_not_clear_build_streak(self):
        results = {"build": [self.fail(), self.fail()], "compile": [OK]}
        b = self.make(lambda k, f: results[k].pop(0))
        b.submit("A", "build", "x")
        self.clock.t += 31
        b.tick()
        b.submit("B", "compile", "y")  # 다른 세션의 compile 성공
        self.clock.t += 31
        b.tick()
        t = b.submit("A", "build", "다시")
        self.clock.t += 31
        b.tick()
        self.assertTrue(b.wait(t["id"], 0)["result"]["needs_user"])

    def test_different_error_resets_streak(self):
        results = [self.fail(), self.fail(["%s(3): error C9999: other" % self.src])]
        b = self.make(lambda k, f: results.pop(0))
        for _ in range(2):
            b.submit("A", "compile", "x")
            self.clock.t += 31
            b.tick()
        self.assertFalse(b.status()["needs_user"])

    def test_no_parsable_errors_gives_tail(self):
        b = self.make(self.runner({"ok": False, "rc": -1, "elapsed": 1, "log": "", "lines": ["그냥 로그 한 줄"]}))
        t = b.submit("A", "build", "x")
        self.clock.t += 31
        b.tick()
        self.assertEqual(b.wait(t["id"], 0)["result"]["tail"], ["그냥 로그 한 줄"])

    def test_runner_exception_becomes_failure(self):
        def boom(kind, force):
            raise RuntimeError("x")
        b = self.make(boom)
        t = b.submit("A", "build", "x")
        self.clock.t += 31
        b.tick()
        r = b.wait(t["id"], 0)["result"]
        self.assertFalse(r["ok"])
        self.assertIn("runner exception", r["tail"][0])


class IdleAndHistory(BrokerBase):
    def test_idle_exit(self):
        b = self.make()
        self.clock.t += 901
        b.tick()
        self.assertTrue(b.stopping)

    def test_no_idle_exit_with_queue(self):
        b = self.make()
        b.submit("A", "build", "x")
        b.ctl("hold")
        self.clock.t += 5000
        b.tick()
        self.assertFalse(b.stopping)

    def test_history_line(self):
        b = self.make()
        b.submit("A", "build", "헤더 수정")
        b.submit("B", "build", "다른 것")
        self.clock.t += 31
        b.tick()
        with open(b.history_path, encoding="utf-8") as f:
            rows = [json.loads(l) for l in f]
        self.assertEqual(len(rows), 1)
        self.assertEqual([q["reason"] for q in rows[0]["requesters"]], ["헤더 수정", "다른 것"])
        for key in ("head", "dirty", "ok", "kind", "elapsed", "at"):
            self.assertIn(key, rows[0])


class Http(BrokerBase):
    def setUp(self):
        super().setUp()
        self.broker = B.Broker(self.root, self.runner(), coalesce_s=0, idle_exit_s=900)
        self.httpd = ThreadingHTTPServer(("127.0.0.1", 0), B.Handler)
        self.httpd.broker = self.broker
        self.port = self.httpd.server_address[1]
        threading.Thread(target=self.httpd.serve_forever, daemon=True).start()
        self.alive = True

        def loop():
            while self.alive:
                self.broker.tick()
                time.sleep(0.05)
        threading.Thread(target=loop, daemon=True).start()

    def tearDown(self):
        self.alive = False
        self.httpd.shutdown()
        self.httpd.server_close()
        super().tearDown()

    def call(self, method, path, body=None):
        req = urllib.request.Request("http://127.0.0.1:%d%s" % (self.port, path), method=method,
                                     data=json.dumps(body).encode() if body is not None else None,
                                     headers={"Content-Type": "application/json"})
        with urllib.request.urlopen(req, timeout=10) as r:
            return json.loads(r.read().decode("utf-8"))

    def test_request_wait_done(self):
        v = self.call("POST", "/request", {"session": "S", "kind": "compile", "reason": "r"})
        done = self.call("GET", "/ticket/%d?wait=5" % v["id"])
        self.assertEqual(done["state"], "done")
        self.assertTrue(done["result"]["ok"])

    def test_ctl_and_status(self):
        self.assertTrue(self.call("POST", "/ctl", {"action": "hold"})["ok"])
        self.assertTrue(self.call("GET", "/status")["held"])
        self.assertFalse(self.call("POST", "/ctl", {"action": "bogus"})["ok"])

    def test_page_served(self):
        with urllib.request.urlopen("http://127.0.0.1:%d/" % self.port, timeout=5) as r:
            self.assertIn("빌드 브로커", r.read().decode("utf-8"))

    def test_foreign_host_and_non_json_rejected(self):
        c = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        c.request("GET", "/status", headers={"Host": "evil.example"})
        self.assertEqual(c.getresponse().status, 403)
        c = http.client.HTTPConnection("127.0.0.1", self.port, timeout=5)
        c.request("POST", "/ctl", body="action=stop", headers={"Content-Type": "application/x-www-form-urlencoded"})
        self.assertEqual(c.getresponse().status, 415)


if __name__ == "__main__":
    unittest.main()
