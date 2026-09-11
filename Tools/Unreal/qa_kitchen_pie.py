"""Actual kitchen BP: real pickup/place, remove during cooking, then cook/burn naturally."""
import unreal,json,datetime,math
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label().replace('QA020_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) if a.get_actor_label().startswith('QA020_')};assert len(actors)==4
kitchen=actors['Kitchen'];egg=actors['Egg'];probe=actors['Probe']
pawn=unreal.GameplayStatics.get_player_pawn(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent);ps=pawn.player_state
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/020-kitchen-pie.json');assert not out.exists()
fixture=json.loads(out.with_name('020-kitchen-fixtures.json').read_text())
start=unreal.GameplayStatics.get_time_seconds(w)
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'level':w.get_path_name(),
   'world_count':len(worlds),'status':'running','phase':'pickup','checks':[],'egg_rows':fixture['egg_rows'],
   'scope':'Real kitchen BP/egg RPC pickup-place-removal and natural cook/burn. Teleport setup, no 2-player or final egg-art claim.'}
def save():out.write_text(json.dumps(r,indent=2),encoding='utf-8')
def hand():return str(ps.hand_item.item_alias)
def aim(target,y):
    pawn.set_actor_location(unreal.Vector(20000,y,1000+cap.get_scaled_capsule_half_height()+2),False,True)
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),target.get_actor_location()))
def ready_drop():
    pawn.set_actor_location(unreal.Vector(20000,115,1000+cap.get_scaled_capsule_half_height()+2),False,True)
    rotation=unreal.Rotator(pitch=0,yaw=-90,roll=0);pawn.set_actor_rotation(rotation,True);pc.set_control_rotation(rotation)
def capture(case):
    sm=egg.get_component_by_class(unreal.StaticMeshComponent);o,e,_=unreal.SystemLibrary.get_component_bounds(sm)
    sphere=egg.get_component_by_class(unreal.SphereComponent);parent=egg.get_attach_parent_actor()
    event={'case':case,'time':r['game_elapsed'],'sub_alias':str(egg.get_editor_property('sub_alias')),'hand':hand(),
      'phase':str(egg.get_cook_phase()),'progress':egg.get_cook_progress(),'parent':parent.get_name() if parent else None,
      'location':egg.get_actor_location().to_tuple(),'visual_bottom_z':o.z-e.z,'sphere_bottom_z':egg.get_actor_location().z-sphere.get_scaled_sphere_radius(),
      'mesh':sm.static_mesh.get_path_name()}
    r['checks'].append(event);return event
assert hand()=='None';aim(egg,225);save();next_time=start+1.5
def tick(delta):
    global next_time
    try:
        assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
        now=unreal.GameplayStatics.get_time_seconds(w);r['game_elapsed']=now-start
        assert r['game_elapsed']<120,'Cooking QA timed out in game seconds'
        if now<next_time:return
        next_time=now+.5;phase=r['phase']
        if phase=='pickup':pawn.call_method('Server_PickPlace');r['phase']='held';next_time=now+1
        elif phase=='held':
            assert hand()=='egg' and egg.get_attach_parent_actor()==pawn;capture('held');ready_drop();r['phase']='place';next_time=now+1
        elif phase=='place':
            pawn.call_method('Server_PickPlace');r['phase']='heating';next_time=now+2
        elif phase=='heating':
            e=capture('on_griddle');assert hand()=='None' and egg.is_on_heat() and kitchen.get_cooking_item()==egg and e['progress']>0
            aim(egg,115);pawn.call_method('Server_PickPlace');r['phase']='removed';next_time=now+1
        elif phase=='removed':
            e=capture('removed_while_cooking');assert hand()=='egg' and not egg.is_on_heat() and kitchen.get_cooking_item() is None
            assert e['sub_alias']=='None';ready_drop();r['phase']='replace';next_time=now+1
        elif phase=='replace':
            pawn.call_method('Server_PickPlace');r['phase']='cook_chain';r['chain_start']=now;next_time=now+1
        elif phase=='cook_chain':
            sub=str(egg.get_editor_property('sub_alias'))
            if sub!=r.get('last_sub'):
                capture('chain_'+sub);r['last_sub']=sub
            if sub=='burnt':
                assert any(x['case']=='chain_cooked' for x in r['checks'])
                sm=kitchen.get_component_by_class(unreal.StaticMeshComponent);o,e,_=unreal.SystemLibrary.get_component_bounds(sm)
                zone=next(c for c in kitchen.get_components_by_class(unreal.BoxComponent) if c.get_name()=='HeatZone');zo,ze,_=unreal.SystemLibrary.get_component_bounds(zone)
                dims=list((e*2).to_tuple());top=kitchen.get_top_location();r['checks'].append({'case':'kitchen_bounds_and_heat','dimensions_cm':dims,'bottom_z':o.z-e.z,'top':top.to_tuple(),'heat_origin':zo.to_tuple(),'heat_dimensions':(ze*2).to_tuple()})
                assert max(abs(a-b) for a,b in zip(dims,[300,120,90]))<.01 and abs(top.z-1090)<.01
                assert max(abs(a-b) for a,b in zip((ze*2).to_tuple(),[300,120,60]))<.01 and abs(zo.z-1120)<.01
                psm=probe.static_mesh_component;po,pe,_=unreal.SystemLibrary.get_component_bounds(psm)
                speed=math.sqrt(sum(v*v for v in psm.get_physics_linear_velocity().to_tuple()));gap=po.z-pe.z-1090
                r['checks'].append({'case':'griddle_probe','gap_cm':gap,'speed_cm_s':speed});assert abs(gap)<1 and speed<2
                r['status']='passed_local_place_remove_cook_burn';r['phase']='complete';save();unreal.unregister_slate_post_tick_callback(handle);return
        save()
    except Exception as error:
        r['status']='failed';r['error']=str(error);save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick);print('Kitchen QA callback registered: '+str(out))
