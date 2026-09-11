import unreal
editor=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
assert editor.get_editor_world().get_path_name()=='/Game/CL9Art/Tests/Lvl_AssetQA01.Lvl_AssetQA01'
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if a.get_actor_label().startswith(('QA_BackWall','QA_Nest')):
        a.set_actor_rotation(unreal.Rotator(pitch=0,yaw=90,roll=0),False)
    if isinstance(a,unreal.DirectionalLight):
        a.set_actor_rotation(unreal.Rotator(pitch=-65,yaw=-20,roll=0),False)
editor.set_level_viewport_camera_info(unreal.Vector(70,-50,156),unreal.Rotator(pitch=-4,yaw=0,roll=0))
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('Corrected Python Rotator positional ambiguity with explicit pitch/yaw/roll')
