"""Drive the real pickup/wear/F RPCs; document local state without inventing attachment."""
import unreal,json,datetime
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
host=next(w for w in worlds if 'UEDPIE_0_' in w.get_path_name())
pawn=unreal.GameplayStatics.get_player_pawn(host,0);pc=unreal.GameplayStatics.get_player_controller(host,0);ps=pawn.player_state
assert str(ps.hand_item.item_alias) in ('None','flashlight') and str(ps.head_item.item_alias)=='None'
allitems=[a for a in unreal.GameplayStatics.get_all_actors_of_class(host,unreal.Actor) if a.get_class().get_name()=='BP_CL9Item_C']
lamps={alias:next(a for a in allitems if str(a.get_editor_property('alias'))==alias) for alias in ('flashlight','headlamp')}
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/009-light-use.json')
if out.exists():
    previous=json.loads(out.read_text());assert previous['status']!='running','Inspect live callback before retrying'
    out.with_name('009-light-use-attempt-'+datetime.datetime.now(datetime.timezone.utc).strftime('%H%M%S')+'.json').write_text(out.read_text(),encoding='utf-8')
state={'status':'running','utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'world_count':len(worlds),
       'engine':unreal.SystemLibrary.get_engine_version(),'level':host.get_path_name(),
       'scope':'Real Server_PickPlace and Server_Flashlight RPC state/attachment; no input-focus, rendered beam, or 2-player claim.',
       'elapsed':0,'step':0,'events':[],'original_pawn_location':list(pawn.get_actor_location().to_tuple())}
def save():out.write_text(json.dumps(state,indent=2),encoding='utf-8')
def aim(alias):
    target=lamps[alias].get_actor_location();half=pawn.get_component_by_class(unreal.CapsuleComponent).get_scaled_capsule_half_height()
    pawn.set_actor_location(unreal.Vector(target.x-45,target.y,half+2),False,True)
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),target))
def capture(label):
    e={'event':label,'hand':str(ps.hand_item.item_alias),'head':str(ps.head_item.item_alias),'lamps':{}}
    for alias,a in lamps.items():
        sm=a.get_component_by_class(unreal.StaticMeshComponent);spot=a.get_component_by_class(unreal.SpotLightComponent)
        parent=a.get_attach_parent_actor()
        e['lamps'][alias]={'mesh':sm.static_mesh.get_path_name(),'parent':parent.get_name() if parent else None,
            'relative_location':list(a.get_component_by_class(unreal.SphereComponent).get_editor_property('relative_location').to_tuple()),
            'scale':list(sm.get_world_scale().to_tuple()),
            'spot_visible':spot.is_visible(),'spot_local_location':list(spot.get_editor_property('relative_location').to_tuple())}
    state['events'].append(e);save();return e
if str(ps.hand_item.item_alias)=='flashlight':state['step']=1;state['elapsed']=1.2
else:aim('flashlight')
save()
def tick(delta):
    try:
        assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
        state['elapsed']+=delta
        if state['elapsed']<(state['step']+1)*1.2:return
        step=state['step']
        if step==0:pawn.call_method('Server_PickPlace')
        elif step==1:
            e=capture('flashlight_held_off');assert e['hand']=='flashlight' and e['lamps']['flashlight']['parent']==pawn.get_name()
            pawn.call_method('Server_Flashlight')
        elif step==2:
            e=capture('flashlight_on');assert e['lamps']['flashlight']['spot_visible'];pawn.call_method('Server_Flashlight')
        elif step==3:
            e=capture('flashlight_off');assert not e['lamps']['flashlight']['spot_visible'];aim('headlamp')
        elif step==4:pawn.call_method('Server_PickPlace')
        elif step==5:
            e=capture('headlamp_worn_off');assert e['head']=='headlamp' and e['hand']=='flashlight'
            assert e['lamps']['headlamp']['parent']==pawn.get_name();pawn.call_method('Server_Flashlight')
        elif step==6:
            e=capture('worn_light_priority_on');assert e['lamps']['headlamp']['spot_visible'] and not e['lamps']['flashlight']['spot_visible']
            pawn.call_method('Server_Flashlight')
        elif step==7:
            e=capture('worn_light_off');assert not e['lamps']['headlamp']['spot_visible']
            state['status']='passed_local_rpc_state_and_attachment';save();unreal.unregister_slate_post_tick_callback(handle);return
        state['step']+=1;save()
    except Exception as error:
        state['status']='failed';state['error']=str(error);save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick)
print('Registered light RPC QA callback; poll '+str(out))
