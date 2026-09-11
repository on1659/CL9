import unreal,json,datetime,math
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label().replace('QA018_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor) if a.get_actor_label().startswith('QA018_')}
assert len(actors)==7
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/018-yard-pie.json');assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
   'world_count':len(worlds),'level':w.get_path_name(),'status':'running','checks':[],
   'scope':'Two floor parts, four decoration signs and one physics probe. No full map, actual CL9 item, visual navigation or 2-player claim.'}
try:
    bounds={}
    for label,a in actors.items():
        if label=='DropProbe':continue
        origin,extent,_=unreal.SystemLibrary.get_component_bounds(a.static_mesh_component);bounds[label]=(origin,extent)
        dims=list((extent*2).to_tuple());expected=[500,500,30] if label.startswith('Ground') else [60,4.4,40]
        assert max(abs(x-y) for x,y in zip(dims,expected))<.01
        r['checks'].append({'case':label+'_bounds','dimensions_cm':dims})
    oa,ea=bounds['GroundNatural'];ob,eb=bounds['GroundNormalized'];gap=(ob.x-eb.x)-(oa.x+ea.x)
    assert abs(gap)<.001 and abs(oa.z+ea.z-ob.z-eb.z)<.001
    r['checks'].append({'case':'ground_seam','gap_cm':gap,'top_z':oa.z+ea.z})
    probe=actors['DropProbe'];sm=probe.static_mesh_component;origin,extent,_=unreal.SystemLibrary.get_component_bounds(sm)
    speed=math.sqrt(sum(x*x for x in sm.get_physics_linear_velocity().to_tuple()));gap=origin.z-extent.z-1030
    r['checks'].append({'case':'probe_support','collision_gap_cm':gap,'speed_cm_s':speed,'nominal_soil_gap_cm':gap+.5})
    assert sm.is_simulating_physics() and abs(gap)<1 and speed<2
    pawn=unreal.GameplayStatics.get_player_pawn(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent)
    for i,label in enumerate(['Exit','Feed','Kitchen','Coop']):
        sm=actors[label].static_mesh_component;profile=str(sm.get_collision_profile_name());assert profile=='NoCollision'
        x=22000+i*100
        hit=unreal.SystemLibrary.capsule_trace_single_by_profile(w,unreal.Vector(x,-120,1200),unreal.Vector(x,120,1200),cap.get_scaled_capsule_radius(),cap.get_scaled_capsule_half_height(),'Pawn',False,[probe],unreal.DrawDebugTrace.NONE)
        r['checks'].append({'case':label+'_decoration','profile':profile,'blocked':hit is not None});assert hit is None
    r['status']='passed_part_ground_support_and_sign_collision'
except Exception as e:r['status']='failed';r['error']=str(e);raise
finally:out.write_text(json.dumps(r,indent=2),encoding='utf-8')
print(json.dumps(r))
