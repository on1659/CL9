"""역할 레지스트리 로더 — `.claude/cl9-roles.json`을 읽고 경로를 역할에 대응한다.

훅(cl9_scope_guard.py)·렌더러(render_roles.py)·테스트가 이 모듈 하나만 쓴다.
정본: docs/goal/session-orchestration.md A절.

패턴 방언: 소문자·슬래시·저장소 상대 경로에 fnmatchcase — '*'는 '/'도 건넌다,
중괄호·'**'·역슬래시·선행 '/' 금지. 폴더는 'a/b/*'로 쓴다('a/b-old/'와는 안 겹친다).
"""
import fnmatch
import json
import os
# hashlib·subprocess 는 쓰는 자리에서 늦게 임포트한다 — 훅 지연 예산 때문

REGISTRY_REL = os.path.join(".claude", "cl9-roles.json")
ROLE_LIST_KEYS = ("owns", "except", "shared", "docs", "pie")
GLOBAL_LIST_KEYS = ("append_only", "lead_only", "lead_only_except", "grant_gated")
DEFAULTS = {"model": "opus", "max_turns": 200, "default_mode": "C"}


class RegistryError(ValueError):
    """레지스트리 파일이 없거나 방언을 어겼다."""


# ---------------------------------------------------------------- 패턴

def check_pattern(pattern):
    """방언 위반이면 RegistryError. 정상이면 정규화된(소문자·슬래시) 패턴을 돌려준다."""
    if not isinstance(pattern, str) or not pattern.strip():
        raise RegistryError("빈 패턴")
    p = pattern.strip()
    for bad in ("{", "}", "**", "\\"):
        if bad in p:
            raise RegistryError("패턴 '%s'에 '%s' — 방언 위반(중괄호·**·역슬래시 금지, 대안은 풀어서 쓴다)" % (p, bad))
    if p.startswith("/") or (len(p) > 1 and p[1] == ":"):
        raise RegistryError("패턴 '%s'는 저장소 상대 경로여야 한다" % p)
    return p.lower()


def match(rel, pattern):
    """정규화된 상대 경로 rel 이 패턴에 맞는가. rel 은 to_rel() 결과여야 한다."""
    return fnmatch.fnmatchcase(rel, pattern)


def matches_any(rel, patterns):
    """맞는 첫 패턴을 돌려준다(없으면 None)."""
    for p in patterns:
        if match(rel, p):
            return p
    return None


# ---------------------------------------------------------------- 경로

def to_rel(path, root):
    """path 를 root 기준 상대 경로(소문자·슬래시)로. root 밖이면 None.

    realpath(정션·심링크·..) + normcase(드라이브 문자 대소문자) + commonpath.
    """
    if not isinstance(path, str) or not path.strip():
        return None
    try:
        root_abs = os.path.normcase(os.path.realpath(root))
        full = path if os.path.isabs(path) else os.path.join(root, path)
        full_abs = os.path.normcase(os.path.realpath(full))
        if os.path.commonpath([root_abs, full_abs]) != root_abs:
            return None
    except (ValueError, OSError):
        return None
    rel = os.path.relpath(full_abs, root_abs).replace("\\", "/")
    return rel.lower()


# ---------------------------------------------------------------- 로드

def registry_path(root):
    return os.path.join(root, REGISTRY_REL)


def registry_hash(root):
    """레지스트리 파일 바이트의 sha256 앞 16자 — 렌더된 역할 파일에 찍는 도장."""
    import hashlib
    with open(registry_path(root), "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()[:16]


def load(root):
    """레지스트리를 읽어 정규화한 dict 를 돌려준다. 문제가 있으면 RegistryError."""
    path = registry_path(root)
    try:
        with open(path, "r", encoding="utf-8") as f:
            raw = json.load(f)
    except FileNotFoundError:
        raise RegistryError("레지스트리 없음: %s" % path)
    except (OSError, ValueError) as ex:
        raise RegistryError("레지스트리 읽기 실패: %s" % ex)
    if not isinstance(raw, dict) or not isinstance(raw.get("roles"), dict) or not raw["roles"]:
        raise RegistryError("레지스트리에 roles 가 없다")

    reg = {"version": raw.get("version", 1), "defaults": dict(DEFAULTS), "roles": {}, "aliases": {}}
    reg["defaults"].update(raw.get("defaults") or {})
    for key in GLOBAL_LIST_KEYS:
        reg[key] = [check_pattern(p) for p in (raw.get(key) or [])]
    for alias, role in (raw.get("aliases") or {}).items():
        if role not in raw["roles"]:
            raise RegistryError("별칭 '%s' → 없는 역할 '%s'" % (alias, role))
        reg["aliases"][alias] = role
    for name, spec in raw["roles"].items():
        if not isinstance(spec, dict):
            raise RegistryError("역할 '%s' 항목이 객체가 아니다" % name)
        role = {"name": name, "title": spec.get("title", name), "note": spec.get("note", "")}
        for key in ROLE_LIST_KEYS:
            values = spec.get(key) or []
            if not isinstance(values, list):
                raise RegistryError("역할 '%s'의 %s 는 목록이어야 한다" % (name, key))
            role[key] = [check_pattern(p) for p in values] if key in ("owns", "except", "shared") else list(values)
        for key, default in reg["defaults"].items():
            role[key] = spec.get(key, default)
        reg["roles"][name] = role
    return reg


def role_names(reg):
    return list(reg["roles"].keys())


def resolve_role(reg, name):
    """역할 이름 또는 별칭('전투'·'cl9-ui' 형태 포함)을 역할 이름으로. 없으면 None."""
    if not name:
        return None
    n = name.strip()
    if n.startswith("cl9-"):
        n = n[4:]
    if n in reg["roles"]:
        return n
    return reg["aliases"].get(n)


# ---------------------------------------------------------------- 분류

def classify(reg, role_name, rel, grants=()):
    """역할이 rel 을 편집해도 되는가.

    (kind, pattern) — kind 는 우선순위 순서대로
      'append_only' : 리드만, 항목은 보고서로 (docs/decisions.md)
      'lead_only'   : 리드만 (Config·.claude·.uasset·.umap …)
      'owns'        : 자유 편집
      'shared'      : 편집 가능, 보고서에 적는다 (grant-gated 면 grants 에 있어야 한다)
      'grant'       : 이번 실행의 grants.json 이 허용
      'outside'     : 거부 — 소유 역할은 owners_of() 로
    """
    role = reg["roles"][role_name]
    p = matches_any(rel, reg["append_only"])
    if p:
        return "append_only", p
    p = matches_any(rel, reg["lead_only"])
    if p and not matches_any(rel, reg["lead_only_except"]):
        return "lead_only", p
    p = matches_any(rel, role["owns"])
    if p and not matches_any(rel, role["except"]):
        return "owns", p
    p = matches_any(rel, role["shared"])
    if p:
        if is_grant_gated(reg, rel) and not matches_any(rel, grants):
            return "gated", p
        return "shared", p
    p = matches_any(rel, grants)
    if p:
        return "grant", p
    return "outside", None


def is_grant_gated(reg, rel):
    return matches_any(rel, reg["grant_gated"]) is not None


def owners_of(reg, rel):
    """rel 을 owns 로 가진 역할들과 shared 로 가진 역할들."""
    owns, shared = [], []
    for name, role in reg["roles"].items():
        if matches_any(rel, role["owns"]) and not matches_any(rel, role["except"]):
            owns.append(name)
        elif matches_any(rel, role["shared"]):
            shared.append(name)
    return {"owns": owns, "shared": shared}


def owner_label(reg, rel):
    """거부 사유에 넣을 소유자 문구. 예: 'gameplay 영역' / 'ui·gameplay 공유 영역' / '어느 역할도 안 가짐'."""
    if matches_any(rel, reg["append_only"]):
        return "append-only(리드가 항목을 붙인다)"
    if matches_any(rel, reg["lead_only"]) and not matches_any(rel, reg["lead_only_except"]):
        return "리드 전용"
    o = owners_of(reg, rel)
    if o["owns"]:
        return "·".join(o["owns"]) + " 영역"
    if o["shared"]:
        return "·".join(o["shared"]) + " 공유 영역"
    return "어느 역할도 안 가짐"


# ---------------------------------------------------------------- 파일 트리 (테스트·감사)

def tree_files(root):
    """추적 파일 + 추적 안 된 비무시 파일(작업 트리가 더러워도 새 파일이 잡히게). 소문자·슬래시."""
    import subprocess
    out = subprocess.run(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
        cwd=root, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=False,
    ).stdout
    return [p.decode("utf-8", "replace").replace("\\", "/").lower() for p in out.split(b"\0") if p]


def expand(pattern, files):
    return [f for f in files if match(f, pattern)]


def all_patterns(reg):
    """(위치 설명, 패턴) 목록 — 커버리지 테스트용."""
    pairs = []
    for key in GLOBAL_LIST_KEYS:
        for p in reg[key]:
            pairs.append((key, p))
    for name, role in reg["roles"].items():
        for key in ("owns", "except", "shared"):
            for p in role[key]:
                pairs.append(("%s.%s" % (name, key), p))
    return pairs


def pairwise_shared_intersections(reg, files):
    """역할 쌍마다 두 shared 에 다 걸리는 파일 — 전부 grant_gated 여야 한다."""
    names = role_names(reg)
    result = {}
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            sa = {f for p in reg["roles"][a]["shared"] for f in expand(p, files)}
            sb = {f for p in reg["roles"][b]["shared"] for f in expand(p, files)}
            both = sorted(sa & sb)
            if both:
                result[(a, b)] = both
    return result
