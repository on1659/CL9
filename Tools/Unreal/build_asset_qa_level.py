"""Create the isolated sample test room with the real CL9 game mode and item data."""
import json
from pathlib import Path
import unreal

dest='/Game/Tests/Lvl_AssetQA01'
art='/Game/Maps/Henhouse/Props/'
editor=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if editor.is_in_play_in_editor():raise RuntimeError('PIE must be stopped')
if unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages():raise RuntimeError('Preserve unsaved map before creating QA level')
if unreal.load_asset(dest):raise RuntimeError('QA level already exists; inspect and edit, do not replace')
assert editor.new_level(dest)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
world.get_world_settings().set_editor_property('default_game_mode',unreal.load_class(None,'/Game/GameMode/BP_CL9GameMode_Flat.BP_CL9GameMode_Flat_C'))

def place(name,mesh,location,yaw=0):
    a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*location),unreal.Rotator(pitch=0,yaw=yaw,roll=0))
    a.set_actor_label(name)
    a.static_mesh_component.set_static_mesh(unreal.load_asset(art+mesh))
    a.static_mesh_component.set_collision_profile_name('BlockAll')
    return a
for x in (250,750,1250):
    for y in (-250,250):place('QA_Floor','SM_Henhouse_Floor_500_A',(x,y,0))
    for y in (-500,500):place('QA_Wall','SM_Henhouse_Wall_500_A',(x,y,0))
for y in (-250,250):place('QA_BackWall','SM_Henhouse_Wall_500_A',(1500,y,0),90)
place('QA_Nest','SM_Henhouse_Nest_ThreeBay_A',(1350,0,0),90)
place('QA_Crate','SM_Henhouse_Crate_A',(420,350,0))
place('QA_Sack','SM_Henhouse_FeedSack_A',(560,350,0))
place('QA_Trough','SM_Henhouse_FeedTrough_A',(950,-360,0))
for x in (300,800,1300):
    place('QA_Lamp','SM_Henhouse_WorkLamp_A',(x,0,370))
    lamp=actors.spawn_actor_from_class(unreal.PointLight,unreal.Vector(x,0,340))
    lamp.set_actor_label('QA_WarmLight')
    lamp.point_light_component.set_intensity(2200)
    lamp.point_light_component.set_attenuation_radius(800)
    lamp.point_light_component.set_light_color(unreal.LinearColor(1,.66,.35,1))
    lamp.point_light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
sun=actors.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,1000),unreal.Rotator(pitch=-65,yaw=-20,roll=0))
sun.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(.4)
sun.get_component_by_class(unreal.DirectionalLightComponent).set_mobility(unreal.ComponentMobility.MOVABLE)
for i,y in enumerate((-65,65)):
    start=actors.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector(110,y,100))
    start.set_actor_label('QA_PlayerStart_'+str(i))
eggdef=unreal.load_asset('/Game/Data/ItemTable').get_editor_property('rows')[0]
assert str(eggdef.get_editor_property('alias'))=='egg'
assert str(eggdef.get_editor_property('sub_alias'))=='None'
old_mesh=str(eggdef.get_editor_property('mesh'))
eggdef.set_editor_property('mesh',unreal.load_asset(art+'SM_Egg_Raw_A'))
assert unreal.EditorAssetLibrary.save_asset('/Game/Data/ItemTable')
for i,location in enumerate([(260,0,24),(1300,-100,67)]):
    egg=actors.spawn_actor_from_class(unreal.load_class(None,'/Game/Items/BP_CL9Item.BP_CL9Item_C'),unreal.Vector(*location))
    egg.set_actor_label('QA_Egg_'+str(i))
    egg.set_editor_property('alias','egg')
    egg.set_editor_property('sub_alias','None')
assert editor.save_current_level()
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(unreal.Vector(70,-50,156),unreal.Rotator(pitch=-4,yaw=0,roll=0))
result={'level':dest,'old_egg_mesh':old_mesh,'new_egg_mesh':art+'SM_Egg_Raw_A','data_row':'ItemTable.rows[0]',
        'actors':len(actors.get_all_level_actors()),'engine':unreal.SystemLibrary.get_engine_version()}
Path(unreal.Paths.project_dir(),'docs/asset/evidence/001-qa-level.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(result)
