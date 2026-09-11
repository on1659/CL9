PREFIX='QA022_';FIXTURE_EVIDENCE='022-truck-fixtures.json';MESH_PATH='/Engine/BasicShapes/Cube'
MESH_PATHS={};FIXTURE_SCALES={'Floor':(10,10,.3)};FIXTURE_COLLISION_PROFILES={};FIXTURES=[('Floor',(20000,0,985),0)]
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/qa_fence_prepare.py',encoding='utf-8').read(),'qa_fence_prepare.py','exec'))
def track(a,label):
 a.set_actor_label(PREFIX+label);data['fixtures'].append({'label':a.get_actor_label(),'name':a.get_name(),'position':a.get_actor_location().to_tuple(),'class':a.get_class().get_path_name()})
 out.write_text(json.dumps(data,indent=2),encoding='utf-8')
for kind,pos in [('Pad',(20000,0,1000)),('Panel',(20320,0,1000))]:
 a=sub.spawn_actor_from_class(unreal.load_class(None,'/Game/Items/BP_CL9Truck'+kind+'.BP_CL9Truck'+kind+'_C'),unreal.Vector(*pos));track(a,kind)
 sm=a.get_component_by_class(unreal.StaticMeshComponent);sm.set_static_mesh(unreal.load_asset('/Game/CL9Art/Maps/TruckParts/'+kind+'/SM_Truck'+kind+'_A'))
 sm.set_editor_property('override_materials',[]);sm.set_collision_profile_name('BlockAllDynamic');a.set_actor_scale3d(unreal.Vector(1,1,1))
probe=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(20100,0,1070));track(probe,'Probe')
sm=probe.static_mesh_component;sm.set_mobility(unreal.ComponentMobility.MOVABLE)
sm.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Sphere'));probe.set_actor_scale3d(unreal.Vector(.2,.2,.2))
sm.set_collision_profile_name('PhysicsActor');sm.set_simulate_physics(True);sm.set_enable_gravity(True)
print('Prepared owned pad/panel/physics probe on one support floor.')
