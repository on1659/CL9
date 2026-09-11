PREFIX='QA018_';FIXTURE_EVIDENCE='018-yard-fixtures.json'
MESH_PATH='/Game/CL9Art/Maps/HenhouseYard/Ground/SM_Henhouse_YardGround_500_A'
MESH_PATHS={'GroundNormalized':'/Game/CL9Art/Maps/HenhouseYard/Ground/SM_Henhouse_YardGround_Stretch100_A'}
FIXTURE_SCALES={'GroundNormalized':(5,5,.3)};FIXTURE_COLLISION_PROFILES={}
FIXTURES=[('GroundNatural',(20000,0,1000),0),('GroundNormalized',(20500,0,1015),0)]
for i,label in enumerate(['Exit','Feed','Kitchen','Coop']):
    MESH_PATHS[label]='/Game/CL9Art/Maps/HenhouseYard/Sign'+label+'/SM_Henhouse_Sign_'+label+'_A'
    FIXTURE_COLLISION_PROFILES[label]='NoCollision';FIXTURES.append((label,(22000+i*100,0,1200),0))
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/qa_fence_prepare.py',encoding='utf-8').read(),'qa_fence_prepare.py','exec'))
probe=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(20000,0,1150));probe.set_actor_label(PREFIX+'DropProbe')
data['fixtures'].append({'label':probe.get_actor_label(),'name':probe.get_name(),'position':[20000,0,1150],'role':'10cm radius physics probe'})
out.write_text(json.dumps(data,indent=2),encoding='utf-8')
sm=probe.static_mesh_component;sm.set_mobility(unreal.ComponentMobility.MOVABLE)
sm.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Sphere'));probe.set_actor_scale3d(unreal.Vector(.2,.2,.2))
sm.set_collision_profile_name('PhysicsActor');sm.set_enable_gravity(True);sm.set_simulate_physics(True)
