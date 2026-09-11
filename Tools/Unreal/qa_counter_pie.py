"""Real delivery and empty-hand sale, with independent surface/body collision checks."""
import unreal,json,datetime,math
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label().replace('QA021_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) if a.get_actor_label().startswith('QA021_')};assert len(actors)==4
counter=actors['Counter'];egg=actors['Egg'];egg_name=egg.get_name();probe=actors['DropProbe']
pawn=unreal.GameplayStatics.get_player_pawn(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent);ps=pawn.player_state
gs=unreal.GameplayStatics.get_game_state(w)
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/021-counter-pie.json');assert not out.exists()
def hand():return str(ps.hand_item.item_alias)
assert hand()=='None'
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'world_count':len(worlds),
   'level':w.get_path_name(),'status':'running','step':0,'checks':[],'money_before':gs.get_shared_money(),'income_before':gs.get_period_income(),
   'scope':'Real Counter BP/local pickup-delivery-sale RPC; observable prompt/focus, actor removal and income; protected Stock is not read; no order fulfilment/cart/2-player/performance claim.'}
def save():out.write_text(json.dumps(r,indent=2),encoding='utf-8')
def aim(target,x):
    pawn.set_actor_location(unreal.Vector(x,0,1000+cap.get_scaled_capsule_half_height()+2),False,True)
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),target.get_actor_location()))
aim(egg,20175);save();start=unreal.GameplayStatics.get_time_seconds(w)
def tick(delta):
    try:
        assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
        r['elapsed']=unreal.GameplayStatics.get_time_seconds(w)-start
        if r['elapsed']<(r['step']+1)*1.5:return
        step=r['step']
        if step==0:pawn.call_method('Server_PickPlace')
        elif step==1:
            parent=egg.get_attach_parent_actor();r['checks'].append({'case':'pickup','hand':hand(),'parent':parent.get_name() if parent else None})
            assert hand()=='egg' and parent==pawn;r['delivered_value']=egg.get_value();assert r['delivered_value']>0;aim(counter,20110)
        elif step==2:pawn.call_method('Server_PickPlace')
        elif step==3:
            r['checks'].append({'case':'delivery','hand':hand(),'prompt':str(pawn.get_focus_prompt()),'counter_focused':pawn.get_focused_actor()==counter})
            assert hand()=='None' and pawn.get_focused_actor()==counter and str(pawn.get_focus_prompt())
            assert all(a.get_name()!=egg_name for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor))
        elif step==4:pawn.call_method('Server_PickPlace')
        elif step==5:
            money=gs.get_shared_money();income=gs.get_period_income()
            r['checks'].append({'case':'empty_hand_sale','prompt':str(pawn.get_focus_prompt()),'counter_focused':pawn.get_focused_actor()==counter,'money_after':money,'income_after':income})
            assert pawn.get_focused_actor()!=counter and money>r['money_before'] and income-r['income_before']==money-r['money_before']
            r['money_after_sale']=money
        elif step==6:
            pawn.call_method('Server_PickPlace')
        elif step==7:
            assert gs.get_shared_money()==r['money_after_sale'];r['checks'].append({'case':'second_empty_interaction','money_unchanged':True})
            sm=counter.get_component_by_class(unreal.StaticMeshComponent);o,e,_=unreal.SystemLibrary.get_component_bounds(sm)
            r['checks'].append({'case':'counter_bounds','dimensions_cm':(e*2).to_tuple(),'bottom_z':o.z-e.z,'mesh':sm.static_mesh.get_path_name()})
            assert sm.static_mesh.get_path_name().endswith('/SM_DeliveryCounter_A.SM_DeliveryCounter_A') and abs(o.z-e.z-1000)<.01
            assert max(abs(v-100) for v in (e*2).to_tuple())<.01
            hit=unreal.SystemLibrary.capsule_trace_single_by_profile(w,unreal.Vector(19850,0,1092),unreal.Vector(20000,0,1092),cap.get_scaled_capsule_radius(),cap.get_scaled_capsule_half_height(),'Pawn',False,[pawn,probe],unreal.DrawDebugTrace.NONE)
            psm=probe.static_mesh_component;po,pe,_=unreal.SystemLibrary.get_component_bounds(psm)
            gap=po.z-pe.z-1100;speed=math.sqrt(sum(v*v for v in psm.get_physics_linear_velocity().to_tuple()))
            r['checks'].append({'case':'body_and_surface','body_blocks_capsule':hit is not None,'probe_gap_cm':gap,'probe_speed_cm_s':speed})
            assert hit is not None and abs(gap)<1 and speed<2
            r['status']='passed_local_delivery_sale_and_collision';save();unreal.unregister_slate_post_tick_callback(handle);return
        r['step']+=1;save()
    except Exception as error:
        r['status']='failed';r['error']=str(error);save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick);print('Counter QA callback registered: '+str(out))
