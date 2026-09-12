"""마스터 문자열 시트 → 런타임 String Table CSV + 언어별 번역(.po/.locres) 동기화 (docs/systems/localization.md).

사람이 고치는 파일은 Content/Strings/CL9_Strings.csv 하나다:
    No,Key,DevNotes,ko,en,ja,zh-Hans
이 스크립트가 시트를 검증하고(키·번호 중복, ko 빈칸, 언어별 {n} 불일치, 코드의 LOCTABLE 키가 시트에 없음) 거기서
  1) Content/Strings/CL9.csv (Key,SourceString=ko,DevNotes) — 엔진이 String Table로 읽는 파일 (생성물, 손대지 말 것)
  2) 언어별 Content/Localization/CL9/<culture>/CL9.po 의 msgstr — 시트의 해당 열 (빈 칸은 비워 둬 ko로 폴백)
을 만들고, 언리얼 커맨드릿(수집 → 내보내기 → 가져오기 → 컴파일)을 돌려 .locres까지 갱신한다.

사용:
    python Tools/Loc/sync_strings.py            # 1)만 — 시트 검증 + CL9.csv 재생성 (1초)
    python Tools/Loc/sync_strings.py --build    # 1) + 커맨드릿까지 (약 3분, 에디터 켠 채로 가능)
"""
import csv
import io
import os
import re
import glob
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SHEET = os.path.join(ROOT, "Content", "Strings", "CL9_Strings.csv")
TABLE_CSV = os.path.join(ROOT, "Content", "Strings", "CL9.csv")
LOC_DIR = os.path.join(ROOT, "Content", "Localization", "CL9")
UPROJECT = os.path.join(ROOT, "CL9.uproject")
UE_CMD = r"D:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
NATIVE = "ko"
FIXED_COLUMNS = ["No", "Key", "DevNotes"]
ARG_RE = re.compile(r"\{\d+\}")
SOURCE_DIR = os.path.join(ROOT, "Source", "CL9")
LOCTABLE_RE = re.compile(r'LOCTABLE\(\s*"CL9"\s*,\s*"([^"]*)"\s*\)')


def read_sheet():
    with io.open(SHEET, "r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    header = rows[0]
    if header[: len(FIXED_COLUMNS)] != FIXED_COLUMNS:
        sys.exit("CL9_Strings.csv header must start with %s (got %s)" % (FIXED_COLUMNS, header))
    cultures = header[len(FIXED_COLUMNS):]
    if NATIVE not in cultures:
        sys.exit("CL9_Strings.csv needs a '%s' column" % NATIVE)
    entries = []
    for line_no, r in enumerate(rows[1:], start=2):
        if not r or not any(c.strip() for c in r):
            continue
        r = r + [""] * (len(header) - len(r))
        entries.append({"line": line_no, "no": r[0].strip(), "key": r[1].strip(), "notes": r[2],
                        "text": {c: r[i + len(FIXED_COLUMNS)] for i, c in enumerate(cultures)}})
    return cultures, entries


def validate(cultures, entries):
    errors = []
    seen_key, seen_no = {}, {}
    for e in entries:
        if not e["key"] or not re.fullmatch(r"[A-Za-z0-9_]+(\.[A-Za-z0-9_]+)+", e["key"]):
            errors.append("line %d: bad key '%s' (expected Area.Name or Kind.alias.Name)" % (e["line"], e["key"]))
        if e["key"] in seen_key:
            errors.append("line %d: duplicate key '%s' (also line %d)" % (e["line"], e["key"], seen_key[e["key"]]))
        seen_key[e["key"]] = e["line"]
        if not e["no"].isdigit():
            errors.append("line %d: No must be a number, got '%s'" % (e["line"], e["no"]))
        elif e["no"] in seen_no:
            errors.append("line %d: duplicate No %s (also line %d)" % (e["line"], e["no"], seen_no[e["no"]]))
        seen_no[e["no"]] = e["line"]
        native = e["text"][NATIVE]
        if not native:
            errors.append("line %d: %s — '%s' column is empty" % (e["line"], e["key"], NATIVE))
        native_args = sorted(ARG_RE.findall(native))
        for c in cultures:
            t = e["text"][c]
            if t and sorted(ARG_RE.findall(t)) != native_args:
                errors.append("line %d: %s — %s has %s but %s has %s" % (e["line"], e["key"], c, ARG_RE.findall(t), NATIVE, native_args))
    if errors:
        sys.exit("CL9_Strings.csv has %d problem(s):\n  " % len(errors) + "\n  ".join(errors))


def check_code_keys(entries):
    """코드가 LOCTABLE("CL9", "키")로 부르는 키가 전부 시트에 있는지 — 오타는 빌드를 통과하고 화면에 키 문자열로 새기 때문."""
    keys = {e["key"] for e in entries}
    missing = []
    for path in glob.glob(os.path.join(SOURCE_DIR, "**", "*.cpp"), recursive=True) + glob.glob(os.path.join(SOURCE_DIR, "**", "*.h"), recursive=True):
        text = io.open(path, encoding="utf-8").read()
        for line_no, line in enumerate(text.splitlines(), start=1):
            for key in LOCTABLE_RE.findall(line):
                if key not in keys:
                    missing.append("%s:%d: LOCTABLE key '%s' is not in CL9_Strings.csv" % (os.path.relpath(path, ROOT), line_no, key))
    if missing:
        sys.exit("%d code key(s) missing from the sheet:\n  " % len(missing) + "\n  ".join(missing))


def write_table(entries):
    with io.open(TABLE_CSV, "w", encoding="utf-8-sig", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["Key", "SourceString", "DevNotes"])
        for e in entries:
            w.writerow([e["key"], e["text"][NATIVE], e["notes"]])
    print("wrote %s (%d keys)" % (os.path.relpath(TABLE_CSV, ROOT), len(entries)))


def run_commandlet(configs):
    args = [UE_CMD, UPROJECT, "-run=GatherText", "-config=" + ";".join(configs), "-unattended", "-nopause", "-nosplash"]
    print("running", " ".join(os.path.basename(c) for c in configs), "...")
    proc = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out = proc.stdout.decode("utf-8", "replace")
    bad = [l for l in out.splitlines() if ("Error:" in l or "Warning:" in l)
           and not any(s in l for s in ("aqProf", "Vtune", "WinPix", "GameFeature", "HttpListener", "LoadErrors", "LogInit: Display", "ModelContextProtocol", "DisablePythonForCommandlet"))]
    for l in bad:
        print("  ", l.strip()[:200])
    if proc.returncode not in (0, 1):
        sys.exit("commandlet failed (exit %d)" % proc.returncode)


def po_escape(s):
    return s.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")


def fill_po(culture, entries):
    path = os.path.join(LOC_DIR, culture, "CL9.po")
    if not os.path.exists(path):
        print("  no PO for", culture, "— skipped")
        return
    text = io.open(path, encoding="utf-8").read()
    by_key = {e["key"]: e["text"][culture] for e in entries}
    blocks = text.split("\n\n")
    filled = 0
    for i, b in enumerate(blocks):
        m = re.search(r'^msgctxt "CL9,(.+)"$', b, re.M)
        if not m:
            continue
        value = by_key.get(m.group(1), "")
        if culture == NATIVE:
            value = ""  # 원문 언어의 msgstr은 비워 둔다 — SourceString이 곧 표시 문자열
        blocks[i] = re.sub(r'^msgstr ".*"$', 'msgstr "%s"' % po_escape(value), b, count=1, flags=re.M)
        filled += bool(value)
    io.open(path, "w", encoding="utf-8", newline="\n").write("\n\n".join(blocks))
    print("  %s: %d translated" % (culture, filled))


def main():
    cultures, entries = read_sheet()
    validate(cultures, entries)
    check_code_keys(entries)
    write_table(entries)
    if "--build" not in sys.argv:
        return
    cfg = lambda n: os.path.join(ROOT, "Config", "Localization", "CL9_%s.ini" % n)
    run_commandlet([cfg("Gather"), cfg("Export")])
    for c in cultures:
        fill_po(c, entries)
    run_commandlet([cfg("Import"), cfg("Compile")])
    conflicts = os.path.join(LOC_DIR, "CL9_Conflicts.txt")
    if os.path.exists(conflicts) and os.path.getsize(conflicts) > 0:
        print("CONFLICTS:\n" + io.open(conflicts, encoding="utf-8").read())
    print("done — Content/Localization/CL9/<culture>/CL9.locres updated for", ", ".join(cultures))


if __name__ == "__main__":
    main()
