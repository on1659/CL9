"""Four temporary component fixtures in the existing isolated art QA map."""
import unreal,json
from pathlib import Path
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert '/Game/CL9Art/Tests/Lvl_AssetQA01.' in world.get_path_name()
assert not unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before=sorted(a.get_name() for a in sub.get_all_level_actors())
prefix=globals().get('PREFIX','QA013_')
assert not any(a.get_actor_label().startswith(prefix) for a in sub.get_all_level_actors())
mesh=unreal.load_asset(globals().get('MESH_PATH','/Game/CL9Art/Maps/HenhouseFence/SM_Henhouse_Fence_250_A'))
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence',globals().get('FIXTURE_EVIDENCE','013-fence-fixtures.json'));assert not out.exists()
data={'before_actor_names':before,'fixtures':[],'cleaned_up':False}
out.write_text(json.dumps(data,indent=2),encoding='utf-8')
for name,pos,yaw in globals().get('FIXTURES',[('A',(20000,0,1000),0),('B',(20250,0,1000),0),
                     ('C',(21000,0,1000),90),('D',(22000,0,1000),270)]):
    a=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos),unreal.Rotator(pitch=0,yaw=yaw,roll=0))
    a.set_actor_label(prefix+name);sm=a.static_mesh_component
    chosen=unreal.load_asset(MESH_PATHS[name]) if name in globals().get('MESH_PATHS',{}) else mesh
    sm.set_static_mesh(chosen);sm.set_collision_profile_name(globals().get('FIXTURE_COLLISION_PROFILES',{}).get(name,'BlockAll'))
    scale=globals().get('FIXTURE_SCALES',{}).get(name,(1,1,1));a.set_actor_scale3d(unreal.Vector(*scale))
    data['fixtures'].append({'label':a.get_actor_label(),'name':a.get_name(),'position':pos,'yaw':yaw,'scale':scale,'mesh':chosen.get_path_name()})
    out.write_text(json.dumps(data,indent=2),encoding='utf-8')
print(json.dumps(data['fixtures']))
