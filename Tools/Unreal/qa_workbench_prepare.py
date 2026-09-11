PREFIX='QA015_';FIXTURE_EVIDENCE='015-workbench-fixtures.json'
MESH_PATH='/Game/CL9Art/Maps/HenhouseWorkbench/SM_Henhouse_Workbench_A'
FIXTURES=[('A',(20000,0,1000),0),('B',(21000,0,1000),90)]
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/qa_fence_prepare.py',encoding='utf-8').read(),'qa_fence_prepare.py','exec'))
probe=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(20000,0,1150))
probe.set_actor_label(PREFIX+'DropProbe')
data['fixtures'].append({'label':probe.get_actor_label(),'name':probe.get_name(),'position':[20000,0,1150],'yaw':0,'role':'10cm radius physics probe'})
out.write_text(json.dumps(data,indent=2),encoding='utf-8')
sm=probe.static_mesh_component;sm.set_mobility(unreal.ComponentMobility.MOVABLE)
sm.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Sphere'));probe.set_actor_scale3d(unreal.Vector(.2,.2,.2))
sm.set_collision_profile_name('PhysicsActor');sm.set_enable_gravity(True);sm.set_simulate_physics(True)
print('Added gravity probe; no CL9 item data changes.')
