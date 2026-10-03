"""스테이지 하나를 세운다 — 명세 JSON 하나 → MapTable 행 + 레벨 (가이드: docs/systems/stage.md).

헤드리스(에디터 닫힌 창, `acquire commandlet` 아래):
    UnrealEditor-Cmd.exe CL9.uproject -run=pythonscript -script="Tools/Unreal/setup_stage.py Tools/Unreal/stages/<alias>.json [inspect|apply]"
에디터 안(콘솔): py "D:/Work/unreal/CL9/Tools/Unreal/setup_stage.py" "D:/Work/unreal/CL9/Tools/Unreal/stages/<alias>.json" apply

inspect(기본) = 지금 상태만 찍는다. apply = 레벨이 없으면 원본 레벨을 복제하고, 행을 만들거나 덮어써 저장한 뒤,
레벨의 바닥 판·NavBounds 를 10일차 격자까지 덮게 늘리고 레벨에 저장된 RecastNavMesh 를 지워 저장한다.
다시 돌려도 같은 결과다 — 명세 파일이 정본이고, 행은 매번 명세대로 다시 쓴다.

명세 JSON (예: Tools/Unreal/stages/farm.json):
    alias         행 이름(FName). MapTable 에 없으면 새 행을 만든다
    name_key      표시 이름의 String Table 'CL9' 키 — CL9_Strings.csv 에 먼저 넣어야 한다(없으면 중단)
    level         이 스테이지의 레벨 경로(/Game/Maps/Lvl_X)
    source_level  레벨이 없을 때 복제할 원본(트럭 패드·푸드트럭·주방·카운터·조명이 따라온다)
    inherit_from  명세에 안 적은 열을 물려받을 행(조명·생성기·테마 …) — INHERIT 목록만 물려받는다
    columns       UCL9SiteDef 열 → 값 (파이썬 이름 = snake_case, bool 은 b 를 뗀다: bBuildCeilings → build_ceilings)
    monster_spawns / item_spawns   사이트 전체 몫 [[alias, min, max], …] (구역과 별도로 예전 규칙으로 놓인다)
    zones         [{name, depth:[min,max], monsters:[[alias,min,max]…], items:[[alias,min,max]…]}, …]
"""
import json
import math
import sys
from pathlib import Path

import unreal

# 바깥 모습(어두운 하늘·흙바닥)은 모든 스테이지 공통 — 같은 함수를 apply_outdoor_look.py 도 쓴다
sys.path.insert(0, str(Path(__file__).resolve().parent))
import outdoor_look  # noqa: E402

TABLE = "/Game/Data/MapTable"

# inherit_from 행에서 물려받는 열 — 명세 columns 가 같은 열을 적으면 그 값이 이긴다
INHERIT = ["generator", "theme", "cell_size", "entry_offset", "build_ceilings", "lobby_lighting", "night_lighting",
           "blackout_lighting", "base_danger", "unlock_day", "value_rate", "max_floors", "stair_count_min",
           "stair_count_max", "upper_floor_share", "lamp_density", "loop_chance", "main_path_share", "passage_share",
           "dead_end_share", "special_copies", "night_length", "dark_base", "dark_min"]

# 진행 표 SiteSizeCurve 의 10일차 배율 — 레벨 바닥·내비 경계는 가장 큰 판까지 덮어야 한다 (site.md 크기는 일자가 정한다)
LARGEST_DAY_SCALE = 1.25

# 격자 끝까지 덮는 여유 (cm) — 트럭 마당과 격자 바깥 벽
MARGIN = 2500.0

TAG = "[stage]"


def log(msg):
    # 커맨드릿 stdout 에는 Display 이상만 나온다 — 경고로 찍어야 로그에 남는다
    unreal.log_warning("%s %s" % (TAG, msg))


def fail(msg):
    unreal.log_error("%s %s" % (TAG, msg))
    raise SystemExit(1)


def entries(rows):
    out = []
    for alias, lo, hi in rows:
        e = unreal.CL9SpawnEntry()
        e.set_editor_property("alias", alias)
        e.set_editor_property("min_count", lo)
        e.set_editor_property("max_count", hi)
        out.append(e)
    return out


def find_row(rows, alias):
    for r in rows:
        if str(r.get_editor_property("alias")) == alias:
            return r
    return None


def grid_extent(cell, modules, entry_offset):
    """생성기(CL9SiteLayout::Generate)와 같은 식 — 한 변 칸 수와 사이트 좌표의 격자 사각형."""
    span = max(16, min(128, int(math.ceil(math.sqrt(modules * 6.0))) * 2))
    x0, x1 = entry_offset, entry_offset + span * cell
    y0, y1 = -span * cell * 0.5, span * cell * 0.5
    return span, x0, x1, y0, y1


def name_text(key):
    text = unreal.TextLibrary.text_from_string_table("CL9", key)
    if not str(text) or "MISSING" in str(text).upper():
        fail("String Table 'CL9' has no key '%s' — add it to Content/Strings/CL9_Strings.csv and run Tools/Loc/sync_strings.py" % key)
    return text


def setup_row(spec, apply):
    table = unreal.load_asset(TABLE)
    rows = list(table.get_editor_property("rows"))
    base = find_row(rows, spec["inherit_from"])
    if base is None:
        fail("inherit_from row '%s' missing" % spec["inherit_from"])
    row = find_row(rows, spec["alias"])
    log("row '%s' %s" % (spec["alias"], "exists" if row else "missing"))
    if not apply:
        return row or base
    text = name_text(spec["name_key"])
    if row is None:
        row = unreal.new_object(type(base), outer=table)
        rows.append(row)
    row.set_editor_property("alias", spec["alias"])
    # 이름은 String Table 'CL9' 키여야 행이 레지스트리를 통과한다 (docs/systems/localization.md)
    row.set_editor_property("display_name", text)
    for name in INHERIT:
        row.set_editor_property(name, base.get_editor_property(name))
    # 로비 하늘은 물려받은 값과 무관하게 공통 저녁 하늘로 (outdoor_look.py)
    outdoor_look.apply_to_row(row, outdoor_look.ensure_materials()[0])
    for name, value in spec.get("columns", {}).items():
        row.set_editor_property(name, value)
    row.set_editor_property("monster_spawns", entries(spec.get("monster_spawns", [])))
    row.set_editor_property("item_spawns", entries(spec.get("item_spawns", [])))
    zones = []
    for zs in spec.get("zones", []):
        z = unreal.CL9SiteZone()
        z.set_editor_property("name", zs["name"])
        z.set_editor_property("depth_min", zs["depth"][0])
        z.set_editor_property("depth_max", zs["depth"][1])
        z.set_editor_property("monster_spawns", entries(zs.get("monsters", [])))
        z.set_editor_property("item_spawns", entries(zs.get("items", [])))
        zones.append(z)
    row.set_editor_property("zones", zones)
    # TSoftObjectPtr<UWorld> 열은 파이썬에서 로드된 월드 에셋으로만 받는다(SoftObjectPath 는 거부)
    row.set_editor_property("level", unreal.load_asset(spec["level"]))
    table.set_editor_property("rows", rows)
    table.modify()
    ok = unreal.EditorAssetLibrary.save_loaded_asset(table, False)
    log("MapTable saved: %s (rows %d)" % (ok, len(rows)))
    return row


def describe(actor):
    origin, extent = actor.get_actor_bounds(False)
    return "%s [%s] center=(%.0f,%.0f,%.0f) half=(%.0f,%.0f,%.0f)" % (
        actor.get_actor_label(), actor.get_class().get_name(), origin.x, origin.y, origin.z, extent.x, extent.y, extent.z)


def fit_level(spec, apply, row):
    level = spec["level"]
    exists = unreal.EditorAssetLibrary.does_asset_exist(level)
    log("%s %s" % (level, "exists" if exists else "missing"))
    unreal.EditorLoadingAndSavingUtils.load_map(level if exists else spec["source_level"])
    actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()

    modules = row.get_editor_property("base_module_count") * LARGEST_DAY_SCALE
    span, x0, x1, y0, y1 = grid_extent(row.get_editor_property("cell_size"), modules, row.get_editor_property("entry_offset"))
    log("largest grid: %d cells a side -> site x %.0f..%.0f, y %.0f..%.0f" % (span, x0, x1, y0, y1))

    actors = actor_sys.get_all_level_actors()
    pad = next((a for a in actors if "TruckPad" in a.get_class().get_name()), None)
    anchor = pad.get_actor_location() if pad else unreal.Vector(0, 0, 0)
    log("truck pad at (%.0f, %.0f, %.0f)" % (anchor.x, anchor.y, anchor.z))

    for a in actors:
        if a.get_class().get_name() == "RecastNavMesh":
            log("  saved navmesh %s (tile %.0f)" % (a.get_name(), a.get_editor_property("tile_size_uu")))
            if apply:
                # 복제해 온 내비 데이터는 원본 레벨의 경계로 타일 용량이 잡혀 있어 런타임에 새 경계 일부만 지어진다
                # (2026-10-03 farm PIE 실측: 몬스터 11/26 이 내비 밖). 지우면 런타임(Dynamic)이 지금 경계로 새로 짓는다.
                actor_sys.destroy_actor(a)
                log("  -> removed (rebuilt at runtime from the current bounds)")

    # 위에서 지운 액터가 섞이지 않게 목록을 다시 받는다
    for a in actor_sys.get_all_level_actors():
        cls = a.get_class().get_name()
        label = a.get_actor_label()
        if not (cls == "NavMeshBoundsVolume" or label.startswith("Floor") or "Ground" in label):
            continue
        log("  " + describe(a))
        if not apply:
            continue
        # 격자는 트럭 패드 기준 사이트 좌표다 (ACL9SiteMap 이 패드를 앵커로 깐다)
        cx = anchor.x + (x0 + x1) * 0.5 - MARGIN * 0.5
        cy = anchor.y + (y0 + y1) * 0.5
        half_x = (x1 - x0 + MARGIN * 3) * 0.5
        half_y = (y1 - y0 + MARGIN * 2) * 0.5
        origin, extent = a.get_actor_bounds(False)
        scale = a.get_actor_scale3d()
        loc = a.get_actor_location()
        # 지금 스케일에서의 반폭으로 스케일 1당 반폭을 구해 새 스케일을 잡는다 — 줄이지는 않는다(원본이 더 크면 그대로)
        unit_x = extent.x / max(abs(scale.x), 1e-3)
        unit_y = extent.y / max(abs(scale.y), 1e-3)
        new_scale = unreal.Vector(max(scale.x, half_x / max(unit_x, 1e-3)), max(scale.y, half_y / max(unit_y, 1e-3)), scale.z)
        if cls == "NavMeshBoundsVolume":
            new_loc = unreal.Vector(cx, cy, loc.z)
        else:
            new_loc = unreal.Vector(cx - (origin.x - loc.x) * new_scale.x / max(scale.x, 1e-3),
                                    cy - (origin.y - loc.y) * new_scale.y / max(scale.y, 1e-3), loc.z)
        a.set_actor_location(new_loc, False, False)
        a.set_actor_scale3d(new_scale)
        log("  -> " + describe(a))

    if apply:
        sky, ground = outdoor_look.ensure_materials()
        log("outdoor look: %d actors" % outdoor_look.apply_to_open_level(sky, ground))
        ok = unreal.EditorLoadingAndSavingUtils.save_map(world, level)
        log("%s saved: %s" % (level, ok))


def main():
    args = sys.argv[1:]
    spec_path = next((a for a in args if a.lower().endswith(".json")), None)
    if spec_path is None:
        fail("usage: setup_stage.py <stages/alias.json> [inspect|apply]")
    path = Path(spec_path)
    if not path.is_absolute():
        path = Path(unreal.Paths.project_dir()) / path
    spec = json.loads(path.read_text(encoding="utf-8"))
    global TAG
    TAG = "[stage %s]" % spec["alias"]
    apply = "apply" in [a.lower() for a in args]

    # 레벨을 먼저 만든다 — 행의 Level 열이 그 에셋을 가리켜야 한다
    if apply and not unreal.EditorAssetLibrary.does_asset_exist(spec["level"]):
        if unreal.EditorAssetLibrary.duplicate_asset(spec["source_level"], spec["level"]) is None:
            fail("duplicate of %s failed" % spec["source_level"])
        log("duplicated %s -> %s" % (spec["source_level"], spec["level"]))
    row = setup_row(spec, apply)
    fit_level(spec, apply, row)
    log("done (%s)" % ("apply" if apply else "inspect"))


main()
