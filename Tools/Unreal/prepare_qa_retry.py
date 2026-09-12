import unreal
editor=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not editor.is_in_play_in_editor()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name()=='/Game/Tests/Lvl_AssetQA01.Lvl_AssetQA01'
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    label=a.get_actor_label()
    if label=='QA_Egg_0':a.set_actor_location(unreal.Vector(200,0,24),False,False)
    elif label=='QA_Egg_1':a.set_actor_location(unreal.Vector(1350,100,75),False,False)
    elif label=='QA_PlayerStart_0':a.set_actor_location(unreal.Vector(110,0,100),False,False)
    elif label=='QA_PlayerStart_1':a.set_actor_location(unreal.Vector(110,170,100),False,False)
assert editor.save_current_level()
print('Retry prepared; first egg in real PickRadius, second inside nest bay')
