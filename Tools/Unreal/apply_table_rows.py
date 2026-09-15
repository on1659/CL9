"""보고서 '## 표 행' 절의 행 요청을 데이터 표 에셋에 적용하고, 다시 읽어 보고서와 비교한다.

정본: docs/goal/session-orchestration.md A절(balance) — 리드가 실행마다 묶어서 적용한다.
에디터 안(리드, 에디터 켠 채):  py -3 Tools/Unreal/run_python.py 로는 인자를 못 넘기니 아래 환경변수로 준다.
    CL9_ROWS_REPORT=<report 경로> CL9_ROWS_APPLY=1  →  콘솔 py "…/apply_table_rows.py"
헤드리스(에디터 닫힌 창, `acquire commandlet` 아래):
    UnrealEditor-Cmd.exe CL9.uproject -run=pythonscript -script="Tools/Unreal/apply_table_rows.py <report> [apply] [verify]"

행 문법(한 줄 하나, 보고서 '## 표 행' 절 안의 `-` 목록 또는 그냥 줄):
    <에셋>.<Alias>[/<SubAlias>].<Column>: <옛값> → <새값> (<단위>)  — 뒤에 오는 설명은 무시
    예)  ItemTable.egg.Weight: 0.5 → 0.6 (kg)   ·   CL9Rules.default.NightLength: 245 → 260 (초)   ·   MonsterTable.raccoon.ChaseSpeed: 462 → 470 (cm/s)
에셋은 /Game/Data/<에셋>. UCL9DefTable 이면 Rows 에서 Alias(+SubAlias) 로 행을 찾고, 독립 UCL9DefBase(CL9Rules 등)면 Alias 는 무시한다.
값은 현재 프로퍼티의 파이썬 타입(int/float/bool/str/Name)으로 바꾼다. 옛값이 현재 값과 다르면 경고하고(적용은 한다), 새 열·새 행은 만들지 않는다.
기본은 dry run. `apply` 면 저장. `verify` 면 적용 없이 현재 값 == 새값인지만 검사해 불일치를 찍는다(사용자가 PIE 에서 눈으로 바꾼 값 잡기).
"""
import os
import re
import sys

import unreal

DATA_DIR = "/Game/Data"
ROW_RE = re.compile(r"^\s*-?\s*`?([A-Za-z0-9_]+)\.([A-Za-z0-9_\-]+)(?:/([A-Za-z0-9_\-]+))?\.([A-Za-z0-9_]+)`?\s*:\s*(.+?)\s*(?:→|->)\s*([^\s(]+)")


def log(msg):
    unreal.log("[rows] " + msg)


def warn(msg):
    unreal.log_warning("[rows] " + msg)


def snake(name):
    s = re.sub(r"(?<=[a-z0-9])([A-Z])", r"_\1", name)
    s = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", s)
    return s.lower()


def parse_report(path):
    """'## 표 행' 절의 행 요청 목록. 절이 없으면 파일 전체에서 찾는다."""
    with open(path, "r", encoding="utf-8") as f:
        lines = f.read().splitlines()
    section, in_section = [], False
    for line in lines:
        if line.startswith("## "):
            in_section = line.strip() == "## 표 행"
            continue
        if in_section:
            section.append(line)
    rows = []
    for line in (section or lines):
        m = ROW_RE.match(line)
        if m:
            rows.append({"asset": m.group(1), "alias": m.group(2), "sub": m.group(3), "column": m.group(4),
                         "old": m.group(5).strip("` "), "new": m.group(6).strip("` "), "line": line.strip()})
    return rows


def coerce(current, text):
    if isinstance(current, bool):
        return text.strip().lower() in ("true", "1", "yes", "예", "참")
    if isinstance(current, int):
        return int(float(text))
    if isinstance(current, float):
        return float(text)
    if isinstance(current, unreal.Name):
        return unreal.Name(text)
    if isinstance(current, unreal.Text):
        return unreal.TextLibrary.conv_string_to_text(text)
    return text


def same(a, b):
    if isinstance(a, float) or isinstance(b, float):
        try:
            return abs(float(a) - float(b)) < 1e-6
        except (TypeError, ValueError):
            return False
    return str(a) == str(b)


def find_target(asset, alias, sub, def_table, def_base):
    if unreal.MathLibrary.class_is_child_of(asset.get_class(), def_table):
        for row in asset.get_editor_property("rows"):
            if row is None:
                continue
            if str(row.get_editor_property("alias")) == alias and (sub is None or str(row.get_editor_property("sub_alias")) == sub):
                return row
        return None
    if unreal.MathLibrary.class_is_child_of(asset.get_class(), def_base):
        return asset
    return None


def main():
    argv = sys.argv[1:]
    report = os.environ.get("CL9_ROWS_REPORT") or (argv[0] if argv else None)
    apply = "apply" in argv or os.environ.get("CL9_ROWS_APPLY") == "1"
    verify = "verify" in argv or os.environ.get("CL9_ROWS_VERIFY") == "1"
    if not report or not os.path.isfile(report):
        warn("보고서 경로가 없다: %r" % report)
        return
    rows = parse_report(report)
    if not rows:
        log("표 행 요청이 없다: %s" % report)
        return
    def_table = unreal.load_class(None, "/Script/CL9.CL9DefTable")
    def_base = unreal.load_class(None, "/Script/CL9.CL9DefBase")
    touched, mismatches, failures = {}, [], []
    for r in rows:
        path = "%s/%s" % (DATA_DIR, r["asset"])
        asset = unreal.load_asset(path)
        if asset is None:
            failures.append("%s — 에셋 없음 %s" % (r["line"], path))
            continue
        target = find_target(asset, r["alias"], r["sub"], def_table, def_base)
        if target is None:
            failures.append("%s — 행 없음(Alias %s%s). 새 행은 만들지 않는다" % (r["line"], r["alias"], "/" + r["sub"] if r["sub"] else ""))
            continue
        prop = snake(r["column"])
        try:
            current = target.get_editor_property(prop)
        except Exception as ex:  # noqa: BLE001
            failures.append("%s — 열 없음 %s(%s): %s. 새 열은 gameplay 에 요청" % (r["line"], r["column"], prop, ex))
            continue
        if verify:
            if not same(current, coerce(current, r["new"])):
                mismatches.append("%s — 에셋 현재 값 %r ≠ 보고서 새값 %s (눈으로 바꾼 값이면 balance 에 문서 갱신 요청)" % (r["line"], current, r["new"]))
            continue
        if not same(current, coerce(current, r["old"])):
            warn("%s — 옛값 %s 가 현재 값 %r 과 다르다(적용은 한다)" % (r["line"], r["old"], current))
        new_val = coerce(current, r["new"])
        log("%s.%s%s.%s: %r → %r%s" % (r["asset"], r["alias"], "/" + r["sub"] if r["sub"] else "", r["column"], current, new_val,
                                        "" if apply else " (dry run)"))
        if apply:
            target.set_editor_property(prop, new_val)
            touched[path] = asset
    if apply:
        for path, asset in touched.items():
            asset.modify()
            if not unreal.EditorAssetLibrary.save_loaded_asset(asset, False):
                failures.append("저장 실패: %s" % path)
        # 다시 읽어 확인
        for r in rows:
            asset = unreal.load_asset("%s/%s" % (DATA_DIR, r["asset"]))
            target = asset and find_target(asset, r["alias"], r["sub"], def_table, def_base)
            if target is None:
                continue
            try:
                current = target.get_editor_property(snake(r["column"]))
            except Exception:  # noqa: BLE001
                continue
            if not same(current, coerce(current, r["new"])):
                mismatches.append("%s — 저장 뒤 재읽기 %r ≠ %s" % (r["line"], current, r["new"]))
    for m in mismatches:
        warn("불일치: " + m)
    for f in failures:
        warn("실패: " + f)
    log("%s — 행 %d, 저장 에셋 %d, 불일치 %d, 실패 %d" % ("VERIFY" if verify else ("APPLIED" if apply else "DRY RUN"),
                                                       len(rows), len(touched), len(mismatches), len(failures)))


main()
