import unreal,json,datetime
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label().replace('QA017_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor) if a.get_actor_label().startswith('QA017_')}
assert len(actors)==6
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/017-access-pie.json');assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
    'world_count':len(worlds),'level':w.get_path_name(),'status':'running','checks':[],
    'scope':'Isolated wall interfaces and tread direction variants. No complete staircase, actual stair walking, 2-player or module selection claim.'}
try:
    bounds={};width=263.9745788574219;depth=850/31
    for name,dims in [('Wall500',[500,40,400]),('Wall250',[250,40,400]),('Wall125',[125,40,400]),
                       ('TreadNatural',[width,depth,30]),('TreadY',[width,depth,30]),('TreadX',[depth,width,30])]:
        origin,extent,_=unreal.SystemLibrary.get_component_bounds(actors[name].static_mesh_component);bounds[name]=(origin,extent)
        actual=list((extent*2).to_tuple());bottom=origin.z-extent.z
        assert max(abs(a-b) for a,b in zip(actual,dims))<.01 and abs(bottom-1000)<.01,name
        r['checks'].append({'case':name+'_bounds','dimensions_cm':actual,'bottom_z':bottom})
    for first,second in [('Wall500','Wall250'),('Wall250','Wall125')]:
        oa,ea=bounds[first];ob,eb=bounds[second];gap=(ob.x-eb.x)-(oa.x+ea.x);assert abs(gap)<.001
        r['checks'].append({'case':first+'_'+second+'_seam','gap_cm':gap})
    pawn=unreal.GameplayStatics.get_player_pawn(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent)
    radius=cap.get_scaled_capsule_radius();half=cap.get_scaled_capsule_half_height()
    for x in [20250,20500]:
        hit=unreal.SystemLibrary.capsule_trace_single_by_profile(w,unreal.Vector(x,-150,1120),unreal.Vector(x,150,1120),radius,half,'Pawn',False,[],unreal.DrawDebugTrace.NONE)
        r['checks'].append({'case':'wall_seam_capsule','x':x,'blocked':hit is not None});assert hit is not None
    for label,x in [('TreadNatural',22000),('TreadY',22500),('TreadX',23000)]:
        hit=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(x,0,1100),unreal.Vector(x,0,990),'BlockAll',False,[],unreal.DrawDebugTrace.NONE)
        r['checks'].append({'case':label+'_support','blocked':hit is not None});assert hit is not None
    r['status']='passed_part_seams_and_directional_tread_fit'
except Exception as e:r['status']='failed';r['error']=str(e);raise
finally:out.write_text(json.dumps(r,indent=2),encoding='utf-8')
print(json.dumps(r))
