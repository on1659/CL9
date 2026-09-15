"""실행 후 감사 — 기준선(baseline) 대비 델타만 본다. 훅은 tripwire, 이것이 진실.

정본: docs/goal/session-orchestration.md C절·Revision 1(밸런스 diff 모양).
- snapshot(root, run_dir): DISPATCHED 때 리드가 부른다 — `git status --porcelain`, 추적 파일의 `git diff HEAD`
  (baseline.patch, 복구용), 더러운 경로의 mtime/sha1, grant-gated 문서의 사본(baseline-files/).
- audit(root, run_dir, reg, roles, grants): 델타(바뀐 경로)를 역할의 owns ∪ shared ∪ grants 에 대응한다.
  대응 안 되는 경로는 전부 위반(조용히 흡수하지 않는다). guard.off 존재, .claude/·Config/ 델타도 위반.
- balance_diff_shape(old_text, new_text): grant-gated 문서에서 balance 가 바꾼 줄은 기존 표 행의 수치 셀만이어야
  한다 — 행 키 변경·열 이름 변경·행 추가/삭제·산문 변경은 위반.
"""
import difflib
import hashlib
import json
import os
import re
import subprocess
import time

import cl9_lock as L
import cl9_roles as R

BASELINE_JSON = "baseline.json"
BASELINE_PATCH = "baseline.patch"
BASELINE_FILES = "baseline-files"
NUMERIC_CELL = re.compile(r"^\s*[-+±~]?\s*\d[\d,]*(\.\d+)?\s*(%|[A-Za-z가-힣/·]{0,12})?\s*(\([^)]*\))?\s*$")


def _git(root, *args, binary=False):
    p = subprocess.run(["git"] + list(args), cwd=root, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=False)
    return p.stdout if binary else p.stdout.decode("utf-8", "replace")


def porcelain(root):
    """{경로(소문자·슬래시): 상태코드} — 이름 바뀐 항목은 새 이름만."""
    out = _git(root, "status", "--porcelain", "-z", "--untracked-files=all")
    result = {}
    parts = out.split("\0")
    i = 0
    while i < len(parts):
        entry = parts[i]
        if not entry:
            i += 1
            continue
        code, path = entry[:2], entry[3:]
        if code[0] == "R" or code[1] == "R":
            i += 1  # 다음 항목이 원래 이름
        result[path.replace("\\", "/").lower()] = code
        i += 1
    return result


def file_hash(root, rel):
    p = os.path.join(root, rel)
    try:
        st = os.stat(p)
        h = hashlib.sha1()
        with open(p, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 16), b""):
                h.update(chunk)
        return {"mtime": st.st_mtime, "sha1": h.hexdigest()}
    except OSError:
        return None


def snapshot(root, run_dir, reg=None):
    """기준선을 run_dir 에 남긴다. 반환: baseline dict."""
    os.makedirs(run_dir, exist_ok=True)
    dirty = porcelain(root)
    with open(os.path.join(run_dir, BASELINE_PATCH), "wb") as f:
        f.write(_git(root, "diff", "HEAD", "--", ".", binary=True))
    hashes = {rel: file_hash(root, rel) for rel in dirty}
    reg = reg or R.load(root)
    files_dir = os.path.join(run_dir, BASELINE_FILES)
    kept = []
    for rel in R.tree_files(root):
        if R.is_grant_gated(reg, rel) and os.path.isfile(os.path.join(root, rel)):
            dst = os.path.join(files_dir, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            with open(os.path.join(root, rel), "rb") as src, open(dst, "wb") as out:
                out.write(src.read())
            kept.append(rel)
    base = {"time": time.time(), "head": _git(root, "rev-parse", "HEAD").strip(), "porcelain": dirty,
            "hashes": hashes, "gated_files": kept}
    with open(os.path.join(run_dir, BASELINE_JSON), "w", encoding="utf-8") as f:
        json.dump(base, f, ensure_ascii=False, indent=1)
    return base


def load_baseline(run_dir):
    with open(os.path.join(run_dir, BASELINE_JSON), "r", encoding="utf-8") as f:
        return json.load(f)


def changed_since(root, base):
    """기준선 이후 바뀐 경로 — 새로 더러워진 것, 내용이 달라진 것, 그리고 기준선에 있었는데 깨끗해진 것(복원 = 남의 작업 파괴)."""
    now = porcelain(root)
    changed = []
    for rel in sorted(set(now) | set(base["porcelain"])):
        if rel not in base["porcelain"]:
            changed.append(rel)
        elif rel not in now:
            changed.append(rel)
        else:
            before, after = base["hashes"].get(rel), file_hash(root, rel)
            if before != after:
                changed.append(rel)
    return changed


def audit(root, run_dir, reg, roles, grants):
    """{"changed", "claimed": {path: role}, "violations": [(path, 이유)], "guard_off": bool}."""
    base = load_baseline(run_dir)
    changed = changed_since(root, base)
    claimed, violations = {}, []
    for rel in changed:
        if rel.startswith(".claude/") or rel.startswith("config/"):
            violations.append((rel, ".claude/·Config/ 는 실행 중 바뀌면 안 된다(리드 전용)"))
            continue
        owner = None
        for role in roles:
            kind, _ = R.classify(reg, role, rel, grants.get(role, []))
            if kind in ("owns", "shared", "grant"):
                owner = role
                break
        if owner is None:
            violations.append((rel, "어느 역할의 brief/grant 에도 없다 — %s" % R.owner_label(reg, rel)))
        else:
            claimed[rel] = owner
    guard_off = L.guard_is_off(root) is not None or os.path.exists(L.guard_off_path(root))
    if guard_off:
        violations.append((".claude/runs/guard.off", "실행 중 가드가 꺼져 있었다"))
    return {"changed": changed, "claimed": claimed, "violations": violations, "guard_off": guard_off}


# ---------------------------------------------------------------- 밸런스 diff 모양

def _cells(line):
    s = line.strip()
    if not (s.startswith("|") and s.endswith("|")):
        return None
    return [c.strip() for c in s[1:-1].split("|")]


def _is_separator(cells):
    return all(re.fullmatch(r":?-{2,}:?", c) for c in cells if c) and any(cells)


def balance_diff_shape(old_text, new_text):
    """위반 목록 [(줄 번호, 이유)]. 비어 있으면 통과."""
    old_lines, new_lines = old_text.splitlines(), new_text.splitlines()
    old_rows = {}
    for line in old_lines:
        cells = _cells(line)
        if cells and not _is_separator(cells):
            old_rows.setdefault(cells[0], cells)
    violations = []
    sm = difflib.SequenceMatcher(None, old_lines, new_lines, autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            continue
        if tag in ("insert", "delete") or (i2 - i1) != (j2 - j1):
            for j in range(j1, j2):
                violations.append((j + 1, "행 추가/삭제 또는 줄 수 변경 — 기존 행의 수치 셀만 바꿀 수 있다: %s" % new_lines[j].strip()[:60]))
            for i in range(i1, i2):
                if j2 == j1:
                    violations.append((i + 1, "줄 삭제: %s" % old_lines[i].strip()[:60]))
            continue
        for i, j in zip(range(i1, i2), range(j1, j2)):
            oc, nc = _cells(old_lines[i]), _cells(new_lines[j])
            if oc is None or nc is None:
                violations.append((j + 1, "표 행이 아닌 줄 변경: %s" % new_lines[j].strip()[:60]))
                continue
            if _is_separator(oc) or _is_separator(nc):
                violations.append((j + 1, "표 구분선 변경"))
                continue
            if len(oc) != len(nc):
                violations.append((j + 1, "열 개수 변경"))
                continue
            if oc[0] != nc[0]:
                violations.append((j + 1, "행 키 변경: %s → %s" % (oc[0], nc[0])))
                continue
            if oc[0] not in old_rows:
                violations.append((j + 1, "기존에 없던 행: %s" % nc[0]))
                continue
            for k, (a, b) in enumerate(zip(oc, nc)):
                if a == b:
                    continue
                if k == 0 or not (NUMERIC_CELL.match(a) and NUMERIC_CELL.match(b)):
                    violations.append((j + 1, "수치 셀이 아닌 변경(열 %d): %r → %r" % (k, a, b)))
    return violations


def balance_audit(root, run_dir, changed_paths):
    """grant-gated 문서 중 바뀐 것에 diff 모양 검사 — {경로: [(줄, 이유)]}."""
    base = load_baseline(run_dir)
    result = {}
    for rel in changed_paths:
        if rel not in base.get("gated_files", []):
            continue
        old_p = os.path.join(run_dir, BASELINE_FILES, rel)
        new_p = os.path.join(root, rel)
        try:
            with open(old_p, "r", encoding="utf-8") as f:
                old = f.read()
            with open(new_p, "r", encoding="utf-8") as f:
                new = f.read()
        except OSError as ex:
            result[rel] = [(0, "읽기 실패: %s" % ex)]
            continue
        v = balance_diff_shape(old, new)
        if v:
            result[rel] = v
    return result
