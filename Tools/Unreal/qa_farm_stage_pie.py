"""대농장(Lvl_Farm) 한 밤을 PIE 로 돌려 본다 — 스테이지 선택·구역 스폰·몬스터 이동(내비)·항아리·프레임 (docs/goal/farm-stage.md).

에디터를 레벨과 함께 띄우면서 실행한다(사람 손 없이 끝까지 돌고 에디터를 닫는다):
    UnrealEditor.exe CL9.uproject /Game/Maps/Lvl_Farm -ExecCmds="py D:/Work/unreal/CL9/Tools/Unreal/qa_farm_stage_pie.py"
결과: Saved/QA/farm_stage_pie.json
"""
import json
import math
from pathlib import Path

import unreal

# 레벨마다 따로 남긴다 — 같은 스크립트로 Lvl_Henhouse 를 돌려 대조군으로 쓴다
_level = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()
OUT = Path(unreal.Paths.project_saved_dir()) / "QA" / ("stage_pie_%s.json" % _level)
QUIT_WHEN_DONE = True

r = {"status": "waiting_editor", "steps": [], "frames": 0, "frame_time_sum": 0.0}
state = {"t": 0.0, "phase": 0, "night_t": None, "first": None, "pie_requested": False}


def save():
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(r, indent=2, ensure_ascii=False), encoding="utf-8")


def pie_world():
    for w in unreal.EditorLevelLibrary.get_pie_worlds(False):
        if "UEDPIE_0_" in w.get_path_name():
            return w
    return None


def monsters(w):
    return unreal.GameplayStatics.get_all_actors_of_class(w, unreal.load_class(None, "/Script/CL9.CL9Monster"))


def snapshot(w):
    snap = {}
    for m in monsters(w):
        loc = m.get_actor_location()
        snap[m.get_name()] = (loc.x, loc.y, loc.z, str(m.get_state()))
    return snap


def finish(status):
    r["status"] = status
    if r["frames"]:
        r["avg_frame_ms"] = round(1000.0 * r["frame_time_sum"] / r["frames"], 1)
    save()
    unreal.unregister_slate_post_tick_callback(handle)
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if les.is_in_play_in_editor():
        les.editor_request_end_play()
    if QUIT_WHEN_DONE:
        unreal.SystemLibrary.quit_editor()


def tick(delta):
    try:
        state["t"] += delta
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if state["phase"] == 0:
            # 에디터가 자리를 잡을 시간을 준다
            if state["t"] < 20.0:
                return
            if not state["pie_requested"]:
                les.editor_request_begin_play()
                state["pie_requested"] = True
                r["steps"].append("pie requested at %.1fs" % state["t"])
                save()
                return
            w = pie_world()
            if w is None or unreal.GameplayStatics.get_player_pawn(w, 0) is None:
                if state["t"] > 200.0:
                    finish("fail: PIE never came up")
                return
            gs = unreal.GameplayStatics.get_game_state(w)
            r["site"] = str(gs.get_current_site_alias())
            r["map_seed_world"] = w.get_path_name()
            pc = unreal.GameplayStatics.get_player_controller(w, 0)
            unreal.SystemLibrary.execute_console_command(w, "cl9.night", pc)
            state["night_t"] = unreal.GameplayStatics.get_time_seconds(w)
            state["phase"] = 1
            r["steps"].append("night started at game %.1fs" % state["night_t"])
            save()
            return

        w = pie_world()
        if w is None:
            finish("fail: PIE world lost")
            return
        r["frames"] += 1
        r["frame_time_sum"] += delta
        gt = unreal.GameplayStatics.get_time_seconds(w) - state["night_t"]
        # 밤 +20·50·90·150초에 표본을 뜬다 — 동적 내비가 큰 판을 다 짓는 데 걸리는 시간까지 본다
        marks = [20.0, 50.0, 90.0]
        idx = state["phase"] - 1
        if idx < len(marks) and gt >= marks[idx]:
            snap = snapshot(w)
            on_nav = 0
            project = getattr(unreal.NavigationSystemV1, "project_point_to_navigation", None) or getattr(unreal.NavigationSystemV1, "k2_project_point_to_navigation")
            origin = state.setdefault("origin", {})
            far = state.setdefault("far", {})
            off = []
            for m in monsters(w):
                loc = m.get_actor_location()
                # 몬스터 코드와 같은 500cm 상자로 투영한다(CL9Monster NavProjectExtent) — 실패하면 None
                hit = project(w, loc, None, None, unreal.Vector(500, 500, 500))
                parts = hit if isinstance(hit, tuple) else (hit,)
                if any(isinstance(p, unreal.Vector) for p in parts):
                    on_nav += 1
                else:
                    off.append([round(loc.x), round(loc.y), round(loc.z)])
                key = m.get_name()
                origin.setdefault(key, (loc.x, loc.y))
                far[key] = max(far.get(key, 0.0), math.hypot(loc.x - origin[key][0], loc.y - origin[key][1]))
            sample = {"t": round(gt), "monsters": len(snap), "on_navmesh_500": on_nav, "off_nav_at": off,
                      "ever_moved_150": sum(1 for v in far.values() if v > 150.0)}
            building = getattr(unreal.NavigationSystemV1, "is_navigation_being_built", None)
            if building is not None:
                sample["nav_building"] = bool(building(w))
            if state["first"] is not None:
                moved = 0
                for name, (x, y, z, st) in state["first"].items():
                    if name in snap and math.hypot(snap[name][0] - x, snap[name][1] - y) > 150.0:
                        moved += 1
                sample["moved_over_150cm_since_last"] = moved
            if idx == 0:
                gs = unreal.GameplayStatics.get_game_state(w)
                r["phase"] = str(gs.get_phase())
                r["items"] = len(unreal.GameplayStatics.get_all_actors_of_class(w, unreal.load_class(None, "/Script/CL9.CL9Item")))
                r["jars"] = len(unreal.GameplayStatics.get_all_actors_of_class(w, unreal.load_class(None, "/Script/CL9.CL9Jar")))
            state["first"] = snap
            r.setdefault("samples", []).append(sample)
            state["phase"] += 1
            save()
            if state["phase"] - 1 >= len(marks):
                # 내비 지도: 10m 간격 표본 — '#' 내비 있음 · '.' 바닥은 있는데 내비 없음 · ' ' 바닥 없음(격자 밖)
                rows = []
                for gy in range(-20000, 20001, 1000):
                    line = ""
                    for gx in range(-1000, 40001, 1000):
                        p0 = unreal.Vector(gx, gy, 50)
                        hit = project(w, p0, None, None, unreal.Vector(400, 400, 200))
                        parts = hit if isinstance(hit, tuple) else (hit,)
                        if any(isinstance(p, unreal.Vector) for p in parts):
                            line += "#"
                        else:
                            tr = unreal.SystemLibrary.line_trace_single(w, unreal.Vector(gx, gy, 300), unreal.Vector(gx, gy, -300),
                                                                        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True)
                            line += "." if (tr is not None and tr.to_tuple()[0]) else " "
                    rows.append(line)
                r["nav_map"] = rows
                r["last_positions"] = {k: [round(v[0]), round(v[1]), round(v[2]), v[3]] for k, v in snap.items()}
                finish("done")
    except Exception as e:  # noqa: BLE001 — QA 스크립트는 실패 사유를 파일로 남기고 멈춘다
        r["error"] = repr(e)
        finish("error")


save()
handle = unreal.register_slate_post_tick_callback(tick)
