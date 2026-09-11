import unreal,json
from pathlib import Path
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/024-farmer-fixtures.json');r=json.loads(out.read_text());assert not r['cleaned_up']
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any(a.get_actor_label().startswith('QA024_Light') for a in sub.get_all_level_actors())
for label,rot,intensity in [('Key',(-35,140,0),30000),('Fill',(-30,-45,0),10000)]:
 a=sub.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(20000,0,1450),unreal.Rotator(pitch=rot[0],yaw=rot[1],roll=rot[2]));a.set_actor_label('QA024_Light'+label)
 r['fixtures'].append({'label':a.get_actor_label(),'name':a.get_name(),'class':a.get_class().get_path_name()});out.write_text(json.dumps(r,indent=2),encoding='utf-8')
 a.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(intensity)
print('Added two tracked QA lights for render validation.')
