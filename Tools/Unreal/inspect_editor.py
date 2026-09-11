import json
from pathlib import Path
import unreal

sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
result = {
    'engine': unreal.SystemLibrary.get_engine_version(),
    'world': sub.get_editor_world().get_path_name(),
    'dirty_content': [p.get_path_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages()],
    'dirty_maps': [p.get_path_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()],
    'actors': [{'path':a.get_path_name(),'class':a.get_class().get_name(),'label':a.get_actor_label()}
               for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()],
}
Path(unreal.Paths.project_saved_dir(), 'asset-editor-before.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(result,ensure_ascii=False))
