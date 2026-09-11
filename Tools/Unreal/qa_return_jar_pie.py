"""Real Jar BP E-equivalent RPC plus open-mouth and physics support checks."""
import unreal,json,datetime,math
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label().replace('QA019_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) if a.get_actor_label().startswith('QA019_')}
assert len(actors)==4
jar=actors['Jar'];egg=actors['Egg'];probe=actors['DropProbe']
egg_name=egg.get_name()
pawn=unreal.GameplayStatics.get_player_pawn(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0);ps=pawn.player_state
gs=unreal.GameplayStatics.get_game_state(w);cap=pawn.get_component_by_class(unreal.CapsuleComponent)
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/019-jar-pie.json');assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
   'world_count':len(worlds),'level':w.get_path_name(),'status':'running','elapsed':0,'step':0,'checks':[],
   'scope':'Actual Jar BP instance with authored mesh; local pickup/return RPC; teleport setup; physics probe, not physical item insertion or 2-player.'}
def save():out.write_text(json.dumps(r,indent=2),encoding='utf-8')
def aim(target,x):
    pawn.set_actor_location(unreal.Vector(x,0,1000+cap.get_scaled_capsule_half_height()+2),False,True)
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),target.get_actor_location()))
assert str(ps.hand_item.item_alias)=='None'
r['returned_before']=len(gs.returned_items);aim(egg,20175);save()
def tick(delta):
    try:
        assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
        r['elapsed']+=delta
        if r['elapsed']<(r['step']+1)*1.5:return
        step=r['step']
        if step==0:pawn.call_method('Server_PickPlace')
        elif step==1:
            parent=egg.get_attach_parent_actor()
            r['checks'].append({'case':'pickup','hand':str(ps.hand_item.item_alias),'parent':parent.get_name() if parent else None})
            assert str(ps.hand_item.item_alias)=='egg' and parent==pawn
            aim(jar,20110)
        elif step==2:pawn.call_method('Server_PickPlace')
        elif step==3:
            items=gs.returned_items
            r['checks'].append({'case':'return','hand':str(ps.hand_item.item_alias),'returned_count':len(items),
                'last_alias':str(items[-1].item_alias) if items else None,'last_sub_alias':str(items[-1].item_sub_alias) if items else None})
            assert str(ps.hand_item.item_alias)=='None' and len(items)==r['returned_before']+1 and str(items[-1].item_alias)=='egg'
            assert all(a.get_name()!=egg_name for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor))
        elif step==4:
            sm=jar.get_component_by_class(unreal.StaticMeshComponent);origin,extent,_=unreal.SystemLibrary.get_component_bounds(sm)
            meshpath=sm.static_mesh.get_path_name();dims=list((extent*2).to_tuple())
            r['checks'].append({'case':'jar_mesh_bounds','mesh':meshpath,'dimensions_cm':dims,'bottom_z':origin.z-extent.z,'profile':str(sm.get_collision_profile_name())})
            assert meshpath.endswith('/SM_ReturnJar_A.SM_ReturnJar_A') and abs(origin.z-extent.z-1000)<.01
            assert max(abs(a-b) for a,b in zip(dims,[94.4,99.940896,100]))<.01
            ignored=[pawn,probe]
            center=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(19998.8,0,1120),unreal.Vector(19998.8,0,1011),'Pawn',False,ignored,unreal.DrawDebugTrace.NONE)
            side=unreal.SystemLibrary.capsule_trace_single_by_profile(w,unreal.Vector(19850,0,1092),unreal.Vector(20000,0,1092),cap.get_scaled_capsule_radius(),cap.get_scaled_capsule_half_height(),'Pawn',False,ignored,unreal.DrawDebugTrace.NONE)
            r['checks'].append({'case':'mouth_and_body','mouth_blocked':center is not None,'body_blocks_capsule':side is not None})
            assert center is None and side is not None
            psm=probe.static_mesh_component;po,pe,_=unreal.SystemLibrary.get_component_bounds(psm)
            speed=math.sqrt(sum(v*v for v in psm.get_physics_linear_velocity().to_tuple()));gap=po.z-pe.z-1009
            r['checks'].append({'case':'interior_probe_support','gap_cm':gap,'speed_cm_s':speed,'location':po.to_tuple()})
            assert abs(gap)<1 and speed<2 and psm.is_simulating_physics()
            r['status']='passed_local_return_and_vessel_collision';save();unreal.unregister_slate_post_tick_callback(handle);return
        r['step']+=1;save()
    except Exception as e:
        r['status']='failed';r['error']=str(e);save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick)
print('Registered Jar QA callback: '+str(out))
