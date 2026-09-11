import unreal,json
w=unreal.EditorLevelLibrary.get_pie_worlds(False)[0]
for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor):
    if a.get_actor_label().startswith('QA018_'):
        sm=a.static_mesh_component
        print(json.dumps({'label':a.get_actor_label(),'position':list(a.get_actor_location().to_tuple()),
            'velocity':list(sm.get_physics_linear_velocity().to_tuple()),'physics':sm.is_simulating_physics(),
            'collision_profile':str(sm.get_collision_profile_name()),'scale':list(a.get_actor_scale3d().to_tuple())}))
