"""밸런스 부등식 검사 — docs/plan 표를 읽어 A1/A2/A5/A6/A7 을 판정하고 A4 는 "미결" 로 찍는다.

정본: docs/plan/economy.md "지켜야 하는 부등식" + "파생값 정의", docs/plan/build-order.md 원칙 3.
C++ 로드 검사(에디터 필요)의 에디터 없는 쌍둥이 — balance 역할이 보고서 '검증(부등식)' 절에 출력을 그대로 붙인다.

  py -3 Tools/Balance/check_inequalities.py [--root D:/Work/unreal/CL9] [--json]

읽는 값(표 행의 첫 열 = 컬럼 이름, 머리글이 '값'·'기본값'인 열의 첫 숫자 — 없으면 둘째 열):
  docs/plan/data/character.md      WalkSpeed · RunSpeed · StaminaDrainTime · StaminaRegenTime · CapsuleRadius
  docs/plan/data/monster/<종>.md   ChaseSpeed · CatchRadius · SightIn (`SightIn / SightOut / SightDark | a / b / c` 합친 행도 읽는다)
                                   PounceRadius · PounceWindup · PounceLunge (잡기 예고 — 2026-09-20)
                                   Radius · RetreatSpeed · ExecuteRadius · ExecuteAnchorDistance · ExecuteBehindAngle ·
                                   ExecuteUnseenTime · StalkGazeHalfAngle · StalkRetreatRadius (처형·스토킹 — 2026-09-20 너구리)
                                   bExecuteOnCatch(true/false) · ExecuteDuration (잡히면 처형 — 2026-09-20 저녁 돼지)
  docs/plan/data/progression.md    DangerCurve 행의 "N일차 x" 중 최댓값 (시야에 곱한다 — systems/monster.md: 추격 속도에는 안 곱한다)

판정:
  A1  지친 빈손 속도 = (Run×Drain + Walk×Regen)/(Drain+Regen)  >  잡는 몬스터(CatchRadius>0)의 ChaseSpeed   (배율 없음)
  A2  질주 예금 = Run×Drain  >  잡는 몬스터의 SightIn × DangerCurve 최댓값  (표 값 기준도 같이 찍는다)
  A4  잡는 몬스터의 ChaseSpeed < 운반 질주 = Run × f(유효 무게)  —  f 가 미결이라 **판정하지 않는다**. 피연산자를 지어내지 않는다.
  A5  잡는 몬스터의 PounceRadius + Walk×PounceWindup > CatchRadius + PounceLunge  그리고  PounceRadius >= CatchRadius
      (걸어서 물러나면 헛침 — 사용자 결정 D2). PounceWindup 행이 없거나 0 이면 "Pounce 없음" 으로 통과.
      참고로 0cm 여유 = Walk×PounceWindup − (CatchRadius + PounceLunge) 도 찍는다(판정엔 안 들어간다).
  A6  처형 종(ExecuteRadius>0)의 ExecuteAnchorDistance >= Radius + CapsuleRadius  (로드 검사와 같은 식 — D1)
  A7  StalkRetreatRadius > 0 ⇒ RetreatSpeed > 0  (처형 종만 — 로드 검사와 같은 식)
  잡기 처형  bExecuteOnCatch true ⇒ ExecuteDuration > 0  (잡히면 처형하는 종(돼지)만 — 로드 검사와 같은 식, economy.md '—' 행)
  처형 회피 산수(처형 종만, 판정에 들어간다 — economy.md "지켜야 하는 부등식" 의 '—' 행):
      180 − StalkGazeHalfAngle > ExecuteBehindAngle  (보고 있으면 등 뒤가 아니다)
      ExecuteUnseenTime > 복제 지연 REPLICATION_LAG 0.1  (돌아본 순간 복제가 닿기 전에 처형이 오지 않는다 — D5)
      참고로 반응 여유 = ExecuteUnseenTime − 0.1 − 180° 회전 TURNAROUND 0.3 도 찍는다(판정엔 안 들어간다 — 회전 0.3 은 표에 없는 추정).
CatchRadius 행이 없는 종은 "판정 불가" 로 따로 찍고 어느 쪽에도 넣지 않는다(거위는 산문으로만 0 이라 여기 해당).
ChaseSpeed 행이 없는 종(양 — "닭과 동일" 산문)은 추격하지 않는 종으로 보아 건너뛴다 — Pounce 행만 있으면 그 사실을 찍는다.
ExecuteRadius 행이 없거나 0 인 종은 스토킹 없음 — A6/A7/회피 산수를 건너뛴다(돼지 — 잡기 처형만 판정).
종료 코드: 0 전부 통과(미결 제외), 1 실패 있음, 2 값을 못 읽음.
"""
import glob
import json
import os
import re
import sys

NUM = re.compile(r"-?\d+(?:\.\d+)?")
REPLICATION_LAG = 0.1   # 초 — D5 "복제 지연 ~100ms 수용" (docs/goal/execute-sequence-pig.md)
TURNAROUND = 0.3        # 초 — 마우스 180° 회전 추정 (표에 없는 값, 참고 줄에만 쓴다)


def _cells(line):
    s = line.strip()
    if not (s.startswith("|") and s.endswith("|")):
        return None
    return [c.strip() for c in s[1:-1].split("|")]


def _key(cell):
    return cell.replace("`", "").strip()


def read_rows(path):
    """{컬럼: 값 열 원문}. 표마다 머리글에서 '값'·'기본값' 열을 찾는다(없으면 둘째 열).
    합친 행(`A` / `B` / `C` | x / y / z)은 낱개로 푼다. 파일이 없으면 {}."""
    rows = {}
    value_col = 1
    try:
        with open(path, "r", encoding="utf-8") as f:
            prev = None
            for line in f:
                cells = _cells(line)
                if not cells or len(cells) < 2:
                    prev = None
                    continue
                if all(re.fullmatch(r":?-{2,}:?", c) for c in cells if c) and prev:
                    heads = [_key(h) for h in prev]
                    value_col = next((i for i, h in enumerate(heads) if h in ("값", "기본값", "현재 값")), 1)
                    prev = None
                    continue
                prev = cells
                if len(cells) <= value_col:
                    continue
                keys = [_key(k) for k in cells[0].split("/")]
                vals = [v.strip() for v in cells[value_col].split("/")] if len(keys) > 1 else [cells[value_col]]
                if len(keys) != len(vals):
                    continue
                for k, v in zip(keys, vals):
                    rows.setdefault(k, v)
    except OSError:
        pass
    return rows


def number(text):
    m = NUM.search(text or "")
    return float(m.group()) if m else None


def flag(text):
    """bool 셀 — `true`/`false`(굵게·괄호 무시). 행이 없으면 None."""
    if text is None:
        return None
    s = text.replace("*", "").strip().lower()
    return s.startswith("true")


def read_curve_max(path, name):
    try:
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                cells = _cells(line)
                if cells and _key(cells[0]) == name:
                    vals = [float(v) for v in re.findall(r"일차\s*(-?\d+(?:\.\d+)?)", line)]
                    return max(vals) if vals else None
    except OSError:
        pass
    return None


def load(root):
    plan = os.path.join(root, "docs", "plan")
    ch = read_rows(os.path.join(plan, "data", "character.md"))
    char = {k: number(ch.get(k)) for k in ("WalkSpeed", "RunSpeed", "StaminaDrainTime", "StaminaRegenTime", "CapsuleRadius")}
    monsters = {}
    for p in sorted(glob.glob(os.path.join(plan, "data", "monster", "*.md"))):
        name = os.path.splitext(os.path.basename(p))[0]
        if name == "common":
            continue
        rows = read_rows(p)
        monsters[name] = {"ChaseSpeed": number(rows.get("ChaseSpeed")),
                          "CatchRadius": number(rows.get("CatchRadius")) if "CatchRadius" in rows else None,
                          "SightIn": number(rows.get("SightIn")),
                          "PounceRadius": number(rows.get("PounceRadius")),
                          "PounceWindup": number(rows.get("PounceWindup")),
                          "PounceLunge": number(rows.get("PounceLunge")),
                          # 처형·스토킹 (2026-09-20 너구리) — 행이 없으면 None = 스토킹 없음
                          "Radius": number(rows.get("Radius")),
                          "RetreatSpeed": number(rows.get("RetreatSpeed")),
                          "ExecuteRadius": number(rows.get("ExecuteRadius")),
                          "ExecuteAnchorDistance": number(rows.get("ExecuteAnchorDistance")),
                          "ExecuteBehindAngle": number(rows.get("ExecuteBehindAngle")),
                          "ExecuteUnseenTime": number(rows.get("ExecuteUnseenTime")),
                          "StalkGazeHalfAngle": number(rows.get("StalkGazeHalfAngle")),
                          "StalkRetreatRadius": number(rows.get("StalkRetreatRadius")),
                          # 잡히면 처형 (2026-09-20 저녁 돼지) — 행이 없으면 None = 보통 잡기
                          "bExecuteOnCatch": flag(rows.get("bExecuteOnCatch")),
                          "ExecuteDuration": number(rows.get("ExecuteDuration"))}
    danger = read_curve_max(os.path.join(plan, "data", "progression.md"), "DangerCurve")
    return {"character": char, "monsters": monsters, "danger_max": danger}


def evaluate(data):
    """{"ok": bool, "readable": bool, "lines": [...], "results": {...}}."""
    c = data["character"]
    lines, results = [], {}
    missing = [k for k, v in c.items() if v is None]
    if missing or data["danger_max"] is None:
        lines.append("값을 못 읽음 — character.md %s%s" % (", ".join(missing) or "OK",
                                                        " · progression.md DangerCurve" if data["danger_max"] is None else ""))
        return {"ok": False, "readable": False, "lines": lines, "results": results}
    run, walk, drain, regen = c["RunSpeed"], c["WalkSpeed"], c["StaminaDrainTime"], c["StaminaRegenTime"]
    tired = (run * drain + walk * regen) / (drain + regen)
    deposit = run * drain
    dmax = data["danger_max"]
    lines.append("파생값: 지친 빈손 속도 = (%g×%g + %g×%g)/(%g+%g) = %.1f cm/s · 질주 예금 = %g×%g = %.0f cm · DangerCurve 최댓값 %g"
                 % (run, drain, walk, regen, drain, regen, tired, run, drain, deposit, dmax))
    ok = True
    catchers, unknown = [], []
    for name, m in data["monsters"].items():
        if m["ChaseSpeed"] is None:
            continue  # 추격하지 않는 종(행 없음)
        if m["CatchRadius"] is None:
            unknown.append(name)
        elif m["CatchRadius"] > 0:
            catchers.append(name)
    for name in unknown:
        lines.append("판정 불가: %s — CatchRadius 행이 없다(잡는 종인지 문서 표에 적어야 한다). A1/A2/A4/A5 에서 제외" % name)
    for name, m in data["monsters"].items():
        if m["ChaseSpeed"] is None and m.get("PounceWindup") is not None:
            lines.append("A5 판정 불가: %s — Pounce 행은 있으나 ChaseSpeed·CatchRadius 행이 없다(문서는 '닭과 동일' 산문). 닭 결과를 그대로 적용한다" % name)
    for name in catchers:
        m = data["monsters"][name]
        chase, sight = m["ChaseSpeed"], m["SightIn"]
        a1 = tired > chase
        results["A1." + name] = a1
        ok &= a1
        lines.append("A1 %s: 지친 빈손 %.1f > %s 추격 %g → %s (여유 %+.1f cm/s)" % ("통과" if a1 else "실패", tired, name, chase, "OK" if a1 else "FAIL", tired - chase))
        if sight is None:
            lines.append("A2 판정 불가: %s SightIn 행이 없다" % name)
            results["A2." + name] = None
            ok = False
            continue
        scaled = sight * dmax
        a2 = deposit > scaled
        results["A2." + name] = a2
        ok &= a2
        lines.append("A2 %s: 질주 예금 %.0f > %s 시야 %g×%g = %.0f (표 값 기준 %g 도 %s) → %s" % (
            "통과" if a2 else "실패", deposit, name, sight, dmax, scaled, sight, "OK" if deposit > sight else "FAIL", "OK" if a2 else "FAIL"))
        lines.append("A4 %s: 미결 — 식 없음 (운반 질주 = Run × f(유효 무게), f 미확정 — economy.md 파생값 표). 판정하지 않는다" % name)
        results["A4." + name] = None
        # A5 — 잡기 예고: 걸어서 물러나면 헛침 (economy.md 2026-09-20)
        windup = m.get("PounceWindup")
        if not windup:
            lines.append("A5 %s: Pounce 없음 (PounceWindup %s) — 옛 즉시 잡기, 통과" % (name, "행 없음" if windup is None else "0"))
            results["A5." + name] = True
            continue
        p_radius, lunge, catch_r = m.get("PounceRadius"), m.get("PounceLunge"), m["CatchRadius"]
        if p_radius is None or lunge is None:
            lines.append("A5 판정 불가: %s — PounceRadius·PounceLunge 행이 없다" % name)
            results["A5." + name] = None
            ok = False
            continue
        lhs, rhs = p_radius + walk * windup, catch_r + lunge
        a5 = lhs > rhs and p_radius >= catch_r
        results["A5." + name] = a5
        ok &= a5
        lines.append("A5 %s: %s 예고 %g + 걷기 %g×%g = %.0f > 잡기 %g + 덮침 %g = %.0f (여유 %+.0f cm) · PounceRadius %g >= CatchRadius %g %s → %s (0cm 여유 %+.0f cm)" % (
            "통과" if a5 else "실패", name, p_radius, walk, windup, lhs, catch_r, lunge, rhs, lhs - rhs,
            p_radius, catch_r, "OK" if p_radius >= catch_r else "FAIL", "OK" if a5 else "FAIL", walk * windup - rhs))
    if not catchers:
        lines.append("잡는 몬스터(CatchRadius>0)가 하나도 없다 — A1/A2 판정 대상 없음")
        ok = False
    # 잡히면 처형하는 종(bExecuteOnCatch true — 돼지): ExecuteDuration > 0 (로드 검사와 같은 식, economy.md '—' 행)
    for name, m in data["monsters"].items():
        if not m.get("bExecuteOnCatch"):
            continue
        duration = m.get("ExecuteDuration")
        catch_exec = duration is not None and duration > 0
        results["잡기처형." + name] = catch_exec
        ok &= catch_exec
        lines.append("잡기 처형 %s: %s bExecuteOnCatch true ⇒ ExecuteDuration %s > 0 → %s%s" % (
            "통과" if catch_exec else "실패", name, "행 없음" if duration is None else "%g" % duration, "OK" if catch_exec else "FAIL",
            "" if m.get("ExecuteRadius") else " (ExecuteRadius 0 — 스토킹 없음, A6/A7/회피 산수 대상 아님)"))
    # 스토커 종(ExecuteRadius > 0) — A6/A7 + 회피 산수 (economy.md 2026-09-20, 너구리)
    capsule = c["CapsuleRadius"]
    for name, m in data["monsters"].items():
        if not m.get("ExecuteRadius"):
            continue
        need = ("Radius", "ExecuteAnchorDistance", "ExecuteBehindAngle", "ExecuteUnseenTime", "StalkGazeHalfAngle", "StalkRetreatRadius")
        lack = [k for k in need if m.get(k) is None]
        if lack:
            lines.append("A6/A7 판정 불가: %s — %s 행이 없다" % (name, ", ".join(lack)))
            results["A6." + name] = results["A7." + name] = None
            ok = False
            continue
        radius, anchor = m["Radius"], m["ExecuteAnchorDistance"]
        a6 = anchor >= radius + capsule
        results["A6." + name] = a6
        ok &= a6
        lines.append("A6 %s: %s 앵커 %g >= 몸 반경 %g + 캡슐 %g = %g → %s (앵커 여유 %+.0f cm)" % (
            "통과" if a6 else "실패", name, anchor, radius, capsule, radius + capsule, "OK" if a6 else "FAIL", anchor - radius - capsule))
        retreat_r, retreat_v = m["StalkRetreatRadius"], m.get("RetreatSpeed")
        if retreat_r > 0:
            a7 = retreat_v is not None and retreat_v > 0
            lines.append("A7 %s: %s StalkRetreatRadius %g > 0 ⇒ RetreatSpeed %s > 0 → %s" % (
                "통과" if a7 else "실패", name, retreat_r, "행 없음" if retreat_v is None else "%g" % retreat_v, "OK" if a7 else "FAIL"))
        else:
            a7 = True
            lines.append("A7 통과: %s StalkRetreatRadius 0 — 후퇴 없음, RetreatSpeed 무관" % name)
        results["A7." + name] = a7
        ok &= a7
        gaze, behind, unseen = m["StalkGazeHalfAngle"], m["ExecuteBehindAngle"], m["ExecuteUnseenTime"]
        cone = 180 - gaze > behind
        results["회피.등뒤." + name] = cone
        ok &= cone
        lines.append("회피 산수 %s: %s 180 − 시선 반각 %g = %g > 등 뒤 반각 %g → %s (간격 %+.0f°)" % (
            "통과" if cone else "실패", name, gaze, 180 - gaze, behind, "OK" if cone else "FAIL", 180 - gaze - behind))
        lag = unseen > REPLICATION_LAG
        results["회피.미발견." + name] = lag
        ok &= lag
        lines.append("회피 산수 %s: %s 미발견 %g > 복제 지연 %g → %s (반응 여유 %g − %g − 회전 %g = %+.1f초 — 참고)" % (
            "통과" if lag else "실패", name, unseen, REPLICATION_LAG, "OK" if lag else "FAIL", unseen, REPLICATION_LAG, TURNAROUND, unseen - REPLICATION_LAG - TURNAROUND))
    return {"ok": ok, "readable": True, "lines": lines, "results": results}


def main(argv=None):
    import argparse
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    ap = argparse.ArgumentParser(prog="check_inequalities.py")
    ap.add_argument("--root", default=os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args(argv)
    data = load(args.root)
    res = evaluate(data)
    if args.json:
        print(json.dumps({"data": data, "result": res}, ensure_ascii=True))
    else:
        print("부등식 검사 (docs/plan 표 기준, %s)" % os.path.relpath(os.path.join(args.root, "docs", "plan"), os.getcwd()).replace("\\", "/"))
        for l in res["lines"]:
            print("  " + l)
        print("결과: " + ("전부 통과 (A4 미결 제외)" if res["ok"] else ("값을 못 읽음" if not res["readable"] else "실패 있음")))
    return 0 if res["ok"] else (2 if not res["readable"] else 1)


if __name__ == "__main__":
    sys.exit(main())
