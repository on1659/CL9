"""Exercise real pan pickup/place/throw on existing game data; no synthetic attachment."""
import unreal,json,datetime
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
host=next(w for w in worlds if 'UEDPIE_0_' in w.get_path_name())
pawn=unreal.GameplayStatics.get_player_pawn(host,0);pc=unreal.GameplayStatics.get_player_controller(host,0);ps=pawn.player_state
assert str(ps.hand_item.item_alias)=='None','Fresh empty-hand test required'
def pans(w):return [a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) if a.get_class().get_name()=='BP_CL9Item_C' and str(a.get_editor_property('alias'))=='frying_pan']
if not pans(host):unreal.SystemLibrary.execute_console_command(host,'cl9.additem frying_pan',pc)
item=next(a for a in pans(host) if not a.get_attach_parent_actor())
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/010-pan-use.json')
assert not out.exists(),'Inspect prior test before repeating'
state={'status':'running','utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'world_count':len(worlds),
       'engine':unreal.SystemLibrary.get_engine_version(),'level':host.get_path_name(),'step':0,'elapsed':0,'events':[],
       'scope':'Local real pickup/place/throw RPC and mesh/attachment positions; no 2-player or rendered grip claim.'}
def save():out.write_text(json.dumps(state,indent=2),encoding='utf-8')
def aim():
    target=item.get_actor_location();half=pawn.get_component_by_class(unreal.CapsuleComponent).get_scaled_capsule_half_height()
    pawn.set_actor_location(unreal.Vector(target.x-45,target.y,half+2),False,True)
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),target))
def capture(label):
    sm=item.get_component_by_class(unreal.StaticMeshComponent);sphere=item.get_component_by_class(unreal.SphereComponent)
    origin,extent,_=unreal.SystemLibrary.get_component_bounds(sm);parent=item.get_attach_parent_actor()
    e={'event':label,'hand':str(ps.hand_item.item_alias),'mesh':sm.static_mesh.get_path_name(),
       'parent':parent.get_name() if parent else None,'location':list(item.get_actor_location().to_tuple()),
       'scale':list(sm.get_world_scale().to_tuple()),'visual_bottom_z':origin.z-extent.z,
       'sphere_bottom_z':item.get_actor_location().z-sphere.get_scaled_sphere_radius()}
    state['events'].append(e);save();assert e['mesh'].endswith('/SM_FryingPan_A.SM_FryingPan_A');return e
aim();save()
def tick(delta):
    try:
        assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
        state['elapsed']+=delta
        if state['elapsed']<(state['step']+1)*1.2:return
        step=state['step']
        if step==0:pawn.call_method('Server_PickPlace')
        elif step==1:
            e=capture('held');assert e['hand']=='frying_pan' and e['parent']==pawn.get_name()
            pc.set_control_rotation(unreal.Rotator(pitch=35,yaw=pc.get_control_rotation().yaw+180,roll=0))
        elif step==2:pawn.call_method('Server_PickPlace')
        elif step==3:
            e=capture('placed');assert e['hand']=='None' and e['parent'] is None;aim()
        elif step==4:pawn.call_method('Server_PickPlace')
        elif step==5:
            e=capture('held_again');assert e['hand']=='frying_pan' and e['parent']==pawn.get_name()
            pc.set_control_rotation(unreal.Rotator(pitch=10,yaw=45,roll=0));pawn.call_method('Server_ThrowChargeStart')
        elif step==6:pawn.call_method('Server_ThrowRelease')
        elif step==7:
            e=capture('thrown');assert e['hand']=='None' and e['parent'] is None
            old=next(x for x in state['events'] if x['event']=='held_again')['location']
            assert sum((a-b)**2 for a,b in zip(e['location'],old))>100,'No measurable throw displacement'
        elif step==9:
            e=capture('after_throw');state['status']='passed_local_pick_place_throw';save();unreal.unregister_slate_post_tick_callback(handle);return
        state['step']+=1;save()
    except Exception as error:
        state['status']='failed';state['error']=str(error);save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick)
print('Pan use callback registered; poll '+str(out))
