"""모든 레벨·모든 사이트 행에 어두운 바깥 모습을 입힌다 (outdoor_look.py — docs/systems/light.md "바깥 모습").

헤드리스(에디터 닫힌 창, `acquire commandlet` 아래):
    UnrealEditor-Cmd.exe CL9.uproject -run=pythonscript -script="D:/Work/unreal/CL9/Tools/Unreal/apply_outdoor_look.py"
새 스테이지는 setup_stage.py 가 같은 함수를 불러 자동으로 입는다 — 이 스크립트는 손으로 만든 레벨까지 한꺼번에 맞출 때 쓴다.
"""
import sys
from pathlib import Path

import unreal

sys.path.insert(0, str(Path(__file__).resolve().parent))
import outdoor_look  # noqa: E402

LEVELS = ["/Game/Maps/Lvl_Flat", "/Game/Maps/Lvl_Henhouse", "/Game/Maps/Lvl_Farm", "/Game/Maps/Lvl_PigFarm"]
TABLE = "/Game/Data/MapTable"


def log(msg):
    unreal.log_warning("[outdoor] " + msg)


sky, ground = outdoor_look.ensure_materials()
log("materials ready: %s, %s" % (sky.get_path_name(), ground.get_path_name()))

table = unreal.load_asset(TABLE)
rows = list(table.get_editor_property("rows"))
for row in rows:
    if isinstance(row, unreal.CL9SiteDef):
        outdoor_look.apply_to_row(row, sky)
        log("row %s lobby -> dusk" % row.get_editor_property("alias"))
table.set_editor_property("rows", rows)
table.modify()
log("MapTable saved: %s" % unreal.EditorAssetLibrary.save_loaded_asset(table, False))

for level in LEVELS:
    if not unreal.EditorAssetLibrary.does_asset_exist(level):
        log("%s missing — skipped" % level)
        continue
    unreal.EditorLoadingAndSavingUtils.load_map(level)
    n = outdoor_look.apply_to_open_level(sky, ground)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    log("%s: %d actors changed, saved %s" % (level, n, unreal.EditorLoadingAndSavingUtils.save_map(world, level)))
    del world

log("done")
