"""Part-only PIE support/clearance check with a simulated sphere, not a CL9 item."""
import unreal,json,datetime,math
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label():a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor) if a.get_actor_label().startswith('QA015_')}
assert len(actors)==3
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/015-workbench-pie.json');assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
   'world_count':len(worlds),'level':w.get_path_name(),'status':'running','checks':[],
   'scope':'Part bounds, capsule/line clearance and a simulated basic sphere on the work surface. No actual CL9 item or 2-player claim.'}
try:
    for label,expected in [('QA015_A',[180,75,90]),('QA015_B',[75,180,90])]:
        origin,extent,_=unreal.SystemLibrary.get_component_bounds(actors[label].static_mesh_component)
        dims=list((extent*2).to_tuple());assert max(abs(a-b) for a,b in zip(dims,expected))<.01
        assert abs(origin.z-extent.z-1000)<.001
        r['checks'].append({'case':label+'_bounds','dimensions_cm':dims,'bottom_z':origin.z-extent.z})
    probe=actors['QA015_DropProbe'];sm=probe.static_mesh_component
    origin,extent,_=unreal.SystemLibrary.get_component_bounds(sm)
    speed=math.sqrt(sum(v*v for v in sm.get_physics_linear_velocity().to_tuple()))
    gap=origin.z-extent.z-1090
    result={'case':'gravity_probe_on_worktop','simulating_physics':sm.is_simulating_physics(),
        'center_cm':list(origin.to_tuple()),'bottom_to_top_gap_cm':gap,'speed_cm_s':speed}
    r['checks'].append(result)
    assert result['simulating_physics'] and abs(gap)<1 and speed<2,'Gravity probe has not settled on the tabletop'
    pawn=unreal.GameplayStatics.get_player_pawn(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent)
    radius=cap.get_scaled_capsule_radius();half=cap.get_scaled_capsule_half_height()
    for name,start,end,kind,expected in [
        ('body_blocks',(20000,-150,1090),(20000,150,1090),'capsule',True),
        ('beside_clear',(20200,-150,1090),(20200,150,1090),'capsule',False),
        ('center_under_top_clear',(20000,-150,1050),(20000,150,1050),'line',False),
        ('shelf_blocks',(20000,0,1060),(20000,0,1010),'line',True)]:
        start=unreal.Vector(*start);end=unreal.Vector(*end)
        if kind=='capsule':hit=unreal.SystemLibrary.capsule_trace_single_by_profile(w,start,end,radius,half,'Pawn',False,[probe],unreal.DrawDebugTrace.NONE)
        else:hit=unreal.SystemLibrary.line_trace_single_by_profile(w,start,end,'BlockAll',False,[probe],unreal.DrawDebugTrace.NONE)
        r['checks'].append({'case':name,'blocked':hit is not None,'expected':expected});assert (hit is not None)==expected,name
    r['status']='passed_part_support_and_clearance'
except Exception as e:r['status']='failed';r['error']=str(e);raise
finally:out.write_text(json.dumps(r,indent=2),encoding='utf-8')
print(json.dumps(r))
