import unreal
import json
from pathlib import Path
dest='/Game/Tests/Lvl_AssetQA01'
art='/Game/Maps/Henhouse/Props/'
editor=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name()==dest+'.Lvl_AssetQA01'
assert not any(a.get_actor_label().startswith('QA_PlayerStart') for a in actors.get_all_level_actors())
sun=next(a for a in actors.get_all_level_actors() if isinstance(a,unreal.DirectionalLight))
source=Path(unreal.Paths.project_dir(),'Tools/Unreal/build_asset_qa_level.py').read_text(encoding='utf-8')
exec(source[source.index('sun.get_component_by_class'):])
