import unreal
print('Game worlds',unreal.EditorLevelLibrary.get_game_world())
print('PIE',unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor())
print('Dirty', [x.get_path_name() for x in unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages()])
print('Material',unreal.load_asset('/Game/CL9Art/Maps/HenhouseSample01/Materials/M_CL9_HH01_Wood'))
print('Save API',unreal.EditorLoadingAndSavingUtils.save_packages.__doc__)
print('World',unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world())
