import unreal,json
from pathlib import Path
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert '/Game/Tests/Lvl_AssetQA01.' in world.get_path_name()
prefix=globals().get('PREFIX','QA013_')
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence',globals().get('FIXTURE_EVIDENCE','013-fence-fixtures.json'));data=json.loads(out.read_text())
assert not data['cleaned_up']
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
owned={x['name'] for x in data['fixtures']}
for a in sub.get_all_level_actors():
    if a.get_name() in owned:
        assert a.get_actor_label().startswith(prefix);assert sub.destroy_actor(a)
assert sorted(a.get_name() for a in sub.get_all_level_actors())==data['before_actor_names']
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
data['cleaned_up']=True;out.write_text(json.dumps(data,indent=2),encoding='utf-8');print('Removed owned fixtures; original actor names restored.')
