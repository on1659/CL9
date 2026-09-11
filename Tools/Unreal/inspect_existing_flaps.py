import unreal,json
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
flaps=[a for a in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.Actor) if a.get_class().get_name()=='CL9FlapIn']
print(json.dumps({'level':world.get_path_name(),'dirty_maps':[p.get_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()],
    'flap_count':len(flaps),'sample':[{'path':a.get_path_name(),'position':list(a.get_actor_location().to_tuple())} for a in flaps[:2]]}))
