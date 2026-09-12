PREFIX='QA020_';FIXTURE_EVIDENCE='020-kitchen-fixtures.json'
MESH_PATH='/Game/Maps/Henhouse/Yard/Ground/SM_Henhouse_YardGround_500_A'
MESH_PATHS={};FIXTURE_SCALES={};FIXTURE_COLLISION_PROFILES={};FIXTURES=[('Floor',(20000,0,970),0)]
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/qa_fence_prepare.py',encoding='utf-8').read(),'qa_fence_prepare.py','exec'))
def track(a,label):
    a.set_actor_label(PREFIX+label)
    data['fixtures'].append({'label':a.get_actor_label(),'name':a.get_name(),'position':a.get_actor_location().to_tuple(),'class':a.get_class().get_path_name()})
    out.write_text(json.dumps(data,indent=2),encoding='utf-8')
kitchen=sub.spawn_actor_from_class(unreal.load_class(None,'/Game/Items/BP_CL9Kitchen.BP_CL9Kitchen_C'),unreal.Vector(20000,0,1000),unreal.Rotator(pitch=0,yaw=90,roll=0))
track(kitchen,'Kitchen');sm=kitchen.get_component_by_class(unreal.StaticMeshComponent)
sm.set_static_mesh(unreal.load_asset('/Game/Items/Kitchen/SM_Kitchen_A'));sm.set_editor_property('override_materials',[])
sm.set_collision_profile_name('BlockAllDynamic');kitchen.set_actor_scale3d(unreal.Vector(1,1,1))
zone=next(c for c in kitchen.get_components_by_class(unreal.BoxComponent) if c.get_name()=='HeatZone')
# BeginPlay supplies world AABB extents, so the zone must keep world-aligned rotation.
zone.set_absolute(False,True,True);zone.set_editor_property('relative_rotation',unreal.Rotator(pitch=0,yaw=0,roll=0))
egg=sub.spawn_actor_from_class(unreal.load_class(None,'/Game/Items/BP_CL9Item.BP_CL9Item_C'),unreal.Vector(20000,170,1035))
track(egg,'Egg');egg.set_editor_property('alias','egg');egg.set_editor_property('sub_alias','None')
probe=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(20080,0,1160));track(probe,'Probe')
sm=probe.static_mesh_component;sm.set_mobility(unreal.ComponentMobility.MOVABLE)
sm.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Sphere'));probe.set_actor_scale3d(unreal.Vector(.2,.2,.2))
sm.set_collision_profile_name('PhysicsActor');sm.set_enable_gravity(True);sm.set_simulate_physics(True)
rows=unreal.load_asset('/Game/Data/ItemTable').get_editor_property('rows')
data['egg_rows']=[{'path':row.get_path_name(),'sub_alias':str(row.get_editor_property('sub_alias')),
    'cook_time':row.get_editor_property('cook_time'),'burn_time':row.get_editor_property('burn_time'),
    'mesh':row.get_editor_property('mesh').get_path_name(),'radius':row.get_editor_property('radius')}
    for row in rows if str(row.get_editor_property('alias'))=='egg']
out.write_text(json.dumps(data,indent=2),encoding='utf-8');print(json.dumps(data['egg_rows']))
