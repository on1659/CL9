"""밸런스 부등식 검사 — docs/plan 표를 읽어 A1/A2 를 판정하고 A4 는 "미결" 로 찍는다.

정본: docs/plan/economy.md "지켜야 하는 부등식" + "파생값 정의", docs/plan/build-order.md 원칙 3.
C++ 로드 검사(에디터 필요)의 에디터 없는 쌍둥이 — balance 역할이 보고서 '검증(부등식)' 절에 출력을 그대로 붙인다.

  py -3 Tools/Balance/check_inequalities.py [--root D:/Work/unreal/CL9] [--json]

읽는 값(표 행의 첫 열 = 컬럼 이름, 머리글이 '값'·'기본값'인 열의 첫 숫자 — 없으면 둘째 열):
  docs/plan/data/character.md      WalkSpeed · RunSpeed · StaminaDrainTime · StaminaRegenTime
  docs/plan/data/monster/<종>.md   ChaseSpeed · CatchRadius · SightIn (`SightIn / SightOut / SightDark | a / b / c` 합친 행도 읽는다)
  docs/plan/data/progression.md    DangerCurve 행의 "N일차 x" 중 최댓값 (시야에 곱한다 — systems/monster.md: 추격 속도에는 안 곱한다)

판정:
  A1  지친 빈손 속도 = (Run×Drain + Walk×Regen)/(Drain+Regen)  >  잡는 몬스터(CatchRadius>0)의 ChaseSpeed   (배율 없음)
  A2  질주 예금 = Run×Drain  >  잡는 몬스터의 SightIn × DangerCurve 최댓값  (표 값 기준도 같이 찍는다)
  A4  잡는 몬스터의 ChaseSpeed < 운반 질주 = Run × f(유효 무게)  —  f 가 미결이라 **판정하지 않는다**. 피연산자를 지어내지 않는다.
CatchRadius 행이 없는 종은 "판정 불가" 로 따로 찍고 어느 쪽에도 넣지 않는다(거위는 산문으로만 0 이라 여기 해당).
종료 코드: 0 전부 통과(미결 제외), 1 실패 있음, 2 값을 못 읽음.
"""
import glob
import json
import os
import re
import sys

NUM = re.compile(r"-?\d+(?:\.\d+)?")


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
    char = {k: number(ch.get(k)) for k in ("WalkSpeed", "RunSpeed", "StaminaDrainTime", "StaminaRegenTime")}
    monsters = {}
    for p in sorted(glob.glob(os.path.join(plan, "data", "monster", "*.md"))):
        name = os.path.splitext(os.path.basename(p))[0]
        if name == "common":
            continue
        rows = read_rows(p)
        monsters[name] = {"ChaseSpeed": number(rows.get("ChaseSpeed")),
                          "CatchRadius": number(rows.get("CatchRadius")) if "CatchRadius" in rows else None,
                          "SightIn": number(rows.get("SightIn"))}
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
        lines.append("판정 불가: %s — CatchRadius 행이 없다(잡는 종인지 문서 표에 적어야 한다). A1/A2/A4 에서 제외" % name)
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
    if not catchers:
        lines.append("잡는 몬스터(CatchRadius>0)가 하나도 없다 — A1/A2 판정 대상 없음")
        ok = False
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
