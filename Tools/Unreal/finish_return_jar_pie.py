"""Finish the same PIE after the unsupported is_valid harness call; no repeat return."""
import unreal,json,math
from pathlib import Path
root=Path(unreal.Paths.project_dir());out=root/'docs/asset/evidence/019-jar-pie.json'
r=json.loads(out.read_text());assert r['status']=='failed' and r['error']=="module 'unreal' has no attribute 'is_valid'"
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert len(worlds)==r['world_count']
w=next(x for x in worlds if x.get_path_name()==r['level'])
actors={a.get_actor_label().replace('QA019_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) if a.get_actor_label().startswith('QA019_')}
assert set(actors)=={'SupportFloor','Jar','DropProbe'}
fixture=json.loads((root/'docs/asset/evidence/019-jar-fixtures.json').read_text())
for key,a in actors.items():assert a.get_name()==next(x['name'] for x in fixture['fixtures'] if x['label']=='QA019_'+key)
jar=actors['Jar'];probe=actors['DropProbe'];pawn=unreal.GameplayStatics.get_player_pawn(w,0)
gs=unreal.GameplayStatics.get_game_state(w);items=gs.returned_items
assert str(pawn.player_state.hand_item.item_alias)=='None' and len(items)==r['returned_before']+1 and str(items[-1].item_alias)=='egg'
out.with_name('019-jar-pie-before-harness-fix.json').write_text(out.read_text(),encoding='utf-8')
r.pop('error');r['harness_recovery']='Same PIE: unsupported unreal.is_valid replaced with actual actor-list absence; successful pickup/return retained.'
try:
    sm=jar.get_component_by_class(unreal.StaticMeshComponent);origin,extent,_=unreal.SystemLibrary.get_component_bounds(sm)
    dims=list((extent*2).to_tuple());meshpath=sm.static_mesh.get_path_name()
    r['checks'].append({'case':'jar_mesh_bounds','mesh':meshpath,'dimensions_cm':dims,'bottom_z':origin.z-extent.z,'profile':str(sm.get_collision_profile_name())})
    assert meshpath.endswith('/SM_ReturnJar_A.SM_ReturnJar_A') and abs(origin.z-extent.z-1000)<.01
    assert max(abs(a-b) for a,b in zip(dims,[94.4,99.940896,100]))<.01
    cap=pawn.get_component_by_class(unreal.CapsuleComponent);ignored=[pawn,probe]
    center=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(19998.8,0,1120),unreal.Vector(19998.8,0,1011),'Pawn',False,ignored,unreal.DrawDebugTrace.NONE)
    side=unreal.SystemLibrary.capsule_trace_single_by_profile(w,unreal.Vector(19850,0,1092),unreal.Vector(20000,0,1092),cap.get_scaled_capsule_radius(),cap.get_scaled_capsule_half_height(),'Pawn',False,ignored,unreal.DrawDebugTrace.NONE)
    r['checks'].append({'case':'mouth_and_body','mouth_blocked':center is not None,'body_blocks_capsule':side is not None})
    assert center is None and side is not None
    psm=probe.static_mesh_component;po,pe,_=unreal.SystemLibrary.get_component_bounds(psm)
    speed=math.sqrt(sum(v*v for v in psm.get_physics_linear_velocity().to_tuple()));gap=po.z-pe.z-1009
    r['checks'].append({'case':'interior_probe_support','gap_cm':gap,'speed_cm_s':speed,'location':po.to_tuple()})
    assert abs(gap)<1 and speed<2 and psm.is_simulating_physics()
    r['status']='passed_local_return_and_vessel_collision'
except Exception as e:r['status']='failed';r['error']=str(e);raise
finally:out.write_text(json.dumps(r,indent=2),encoding='utf-8')
print(json.dumps(r))
