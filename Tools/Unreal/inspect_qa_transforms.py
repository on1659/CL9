import unreal,json
print('Rotator',unreal.Rotator.__doc__[:1400])
print('Camera',unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_level_viewport_camera_info())
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if a.get_actor_label().startswith(('QA_Floor','QA_Nest','QA_BackWall','QA_Player')):
        print(a.get_actor_label(),a.get_actor_location(),a.get_actor_rotation())
print('PIE settings',unreal.LevelEditorPlaySettings.__doc__[:500])
