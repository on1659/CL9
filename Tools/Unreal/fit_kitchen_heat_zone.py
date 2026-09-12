"""Correct owned QA instance only; preserve the original Kitchen BP and maps."""
import unreal,json
from pathlib import Path
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert '/Game/Tests/Lvl_AssetQA01.' in world.get_path_name()
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/020-kitchen-fixtures.json');data=json.loads(out.read_text());assert not data['cleaned_up']
name=next(x['name'] for x in data['fixtures'] if x['label']=='QA020_Kitchen')
a=next(x for x in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors() if x.get_name()==name)
assert a.get_actor_label()=='QA020_Kitchen'
zone=next(c for c in a.get_components_by_class(unreal.BoxComponent) if c.get_name()=='HeatZone')
zone.set_absolute(False,True,True);zone.set_editor_property('relative_rotation',unreal.Rotator(pitch=0,yaw=0,roll=0))
assert zone.get_editor_property('absolute_rotation')
data['heat_zone_fit']={'absolute_rotation':True,'world_rotation':str(zone.get_world_rotation()),'reason':'BeginPlay extents are already world AABB; inherited yaw rotated them twice'}
out.write_text(json.dumps(data,indent=2),encoding='utf-8');print(json.dumps(data['heat_zone_fit']))
