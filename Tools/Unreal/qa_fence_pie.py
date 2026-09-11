"""Real PIE collision queries, not a claim of walking input or full map generation."""
import unreal,json,datetime
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor)
fixture={a.get_actor_label():a for a in actors if a.get_actor_label().startswith('QA013_')};assert len(fixture)==4
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/013-fence-pie.json');assert not out.exists()
report={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
        'world_count':len(worlds),'level':w.get_path_name(),'checks':[],'status':'running',
        'scope':'PIE bounds, seam, simple line trace and current player capsule sweep. No walking-input, 2-player or performance claim.'}
def save():out.write_text(json.dumps(report,indent=2),encoding='utf-8')
save()
try:
    for label,a in fixture.items():
        sm=a.static_mesh_component;origin,extent,r=unreal.SystemLibrary.get_component_bounds(sm)
        dims=list((extent*2).to_tuple());yaw=round(a.get_actor_rotation().yaw)%360
        expected=[250,24,288] if yaw==0 else [24,250,288]
        assert max(abs(x-y) for x,y in zip(dims,expected))<.01
        report['checks'].append({'case':label+'_bounds','dimensions_cm':dims,'bottom_z':origin.z-extent.z})
    a=fixture['QA013_A'];b=fixture['QA013_B']
    oa,ea,_=unreal.SystemLibrary.get_component_bounds(a.static_mesh_component)
    ob,eb,_=unreal.SystemLibrary.get_component_bounds(b.static_mesh_component)
    gap=(ob.x-eb.x)-(oa.x+ea.x);assert abs(gap)<.001
    report['checks'].append({'case':'two_tile_seam','gap_cm':gap,'total_length_cm':ob.x+eb.x-oa.x+ea.x})
    pawn=unreal.GameplayStatics.get_player_pawn(w,0);assert pawn
    capsule=pawn.get_component_by_class(unreal.CapsuleComponent)
    radius=capsule.get_scaled_capsule_radius();half=capsule.get_scaled_capsule_half_height()
    report['capsule_cm']={'radius':radius,'half_height':half}
    for label,start,end,expected in [
        ('front',(20000,-200,1100),(20000,200,1100),True),
        ('seam',(20125,-200,1100),(20125,200,1100),True),
        ('rotated90',(20800,0,1100),(21200,0,1100),True),
        ('rotated270',(21800,0,1100),(22200,0,1100),True),
        ('clear_end',(20600,-200,1100),(20600,200,1100),False)]:
        start=unreal.Vector(*start);end=unreal.Vector(*end)
        line=unreal.SystemLibrary.line_trace_single_by_profile(w,start,end,'BlockAll',False,[],unreal.DrawDebugTrace.NONE)
        swept=unreal.SystemLibrary.capsule_trace_single_by_profile(w,start,end,radius,half,'Pawn',False,[],unreal.DrawDebugTrace.NONE)
        result={'case':label,'line_blocked':line is not None,'capsule_blocked':swept is not None,'expected_blocked':expected}
        report['checks'].append(result);save()
        assert result['line_blocked']==expected and result['capsule_blocked']==expected,label
    report['status']='passed_local_collision_and_seam'
except Exception as error:
    report['status']='failed';report['error']=str(error);raise
finally:save()
print(json.dumps(report))
