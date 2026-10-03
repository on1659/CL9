"""로비(밤 전) 바깥 모습을 PIE 에서 찍는다 — 하늘·바닥이 어두운지 눈으로 확인 (outdoor_look.py).

    UnrealEditor.exe CL9.uproject /Game/Maps/Lvl_X -ExecCmds="py D:/Work/unreal/CL9/Tools/Unreal/qa_outdoor_shot.py"
결과: Saved/Screenshots/WindowsEditor/HighresScreenshot*.png (두 장 — 트럭 쪽 지평선, 맵 반대쪽 바깥). 끝나면 에디터를 닫는다.
"""
import unreal

state = {"t": 0.0, "step": 0, "requested": False}


def pie_world():
    for w in unreal.EditorLevelLibrary.get_pie_worlds(False):
        if "UEDPIE_0_" in w.get_path_name():
            return w
    return None


def aim(w, yaw, pitch):
    pc = unreal.GameplayStatics.get_player_controller(w, 0)
    r = unreal.Rotator()
    r.pitch = pitch
    r.yaw = yaw
    pc.set_control_rotation(r)
    return pc


def tick(delta):
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    state["t"] += delta
    if state["step"] == 0:
        if state["t"] > 20.0 and not state["requested"]:
            les.editor_request_begin_play()
            state["requested"] = True
            state["t"] = 0.0
            state["step"] = 1
        return
    w = pie_world()
    if w is None:
        if state["t"] > 120.0:
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.quit_editor()
        return
    pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
    if pawn is None:
        return
    # 로비 조명이 보간으로 자리 잡을 시간을 준다
    if state["step"] == 1 and state["t"] > 12.0:
        pc = aim(w, 180.0, 8.0)  # 트럭 뒤 — 맵 반대쪽 바깥과 지평선
        unreal.SystemLibrary.execute_console_command(w, "HighResShot 1280x720", pc)
        state["step"] = 2
        state["t"] = 0.0
    elif state["step"] == 2 and state["t"] > 4.0:
        pc = aim(w, 90.0, 4.0)  # 옆 — 바닥 판과 맵 외벽
        unreal.SystemLibrary.execute_console_command(w, "HighResShot 1280x720", pc)
        state["step"] = 3
        state["t"] = 0.0
    elif state["step"] == 3 and state["t"] > 4.0:
        unreal.unregister_slate_post_tick_callback(handle)
        les.editor_request_end_play()
        unreal.SystemLibrary.quit_editor()


handle = unreal.register_slate_post_tick_callback(tick)
