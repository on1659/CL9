PREFIX='QA019_';FIXTURE_EVIDENCE='019-jar-fixtures.json'
MESH_PATH='/Game/Maps/Henhouse/Yard/Ground/SM_Henhouse_YardGround_500_A'
MESH_PATHS={};FIXTURE_SCALES={};FIXTURE_COLLISION_PROFILES={}
FIXTURES=[('SupportFloor',(20000,0,970),0)]
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/qa_fence_prepare.py',encoding='utf-8').read(),'qa_fence_prepare.py','exec'))
def track(a,label):
    a.set_actor_label(PREFIX+label)
    data['fixtures'].append({'label':a.get_actor_label(),'name':a.get_name(),'position':a.get_actor_location().to_tuple(),'class':a.get_class().get_path_name()})
    out.write_text(json.dumps(data,indent=2),encoding='utf-8')
jar=sub.spawn_actor_from_class(unreal.load_class(None,'/Game/Items/BP_CL9Jar.BP_CL9Jar_C'),unreal.Vector(20000,0,1000))
track(jar,'Jar')
sm=jar.get_component_by_class(unreal.StaticMeshComponent)
sm.set_static_mesh(unreal.load_asset('/Game/Items/ReturnJar/SM_ReturnJar_A'));sm.set_editor_property('override_materials',[])
sm.set_collision_profile_name('BlockAllDynamic');jar.set_actor_scale3d(unreal.Vector(1,1,1))
egg=sub.spawn_actor_from_class(unreal.load_class(None,'/Game/Items/BP_CL9Item.BP_CL9Item_C'),unreal.Vector(20120,0,1035))
track(egg,'Egg');egg.set_editor_property('alias','egg');egg.set_editor_property('sub_alias','None')
probe=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(19998.8,0,1140));track(probe,'DropProbe')
sm=probe.static_mesh_component;sm.set_mobility(unreal.ComponentMobility.MOVABLE)
sm.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Sphere'));probe.set_actor_scale3d(unreal.Vector(.2,.2,.2))
sm.set_collision_profile_name('PhysicsActor');sm.set_enable_gravity(True);sm.set_simulate_physics(True)
print('Prepared one actual Jar BP, one real egg, one floor and one probe.')
