"""Minimal component compatibility checks; no room or full map generation."""
import unreal,json,datetime
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label().replace('QA016_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor) if a.get_actor_label().startswith('QA016_')}
assert len(actors)==6
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/016-structure-pie.json');assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
   'world_count':len(worlds),'level':w.get_path_name(),'status':'running','checks':[],
   'scope':'Six isolated part fixtures. Current 100-unit Stretch scale applied to a compatible export. No actual module generation/2-player/performance claim.'}
try:
    bounds={}
    for name,dims,z in [('BeamA',[500,28,32],1000),('BeamB',[500,28,32],1000),('Beam90',[28,500,32],1000),
                        ('Pillar',[28,28,400],1000),('CeilingNatural',[500,500,30],1600),('CeilingStretch',[500,500,30],1600)]:
        origin,extent,_=unreal.SystemLibrary.get_component_bounds(actors[name].static_mesh_component);bounds[name]=(origin,extent)
        actual=list((extent*2).to_tuple());bottom=origin.z-extent.z
        assert max(abs(a-b) for a,b in zip(actual,dims))<.01 and abs(bottom-z)<.01,name
        r['checks'].append({'case':name+'_bounds','dimensions_cm':actual,'bottom_z':bottom})
    for first,second in [('BeamA','BeamB'),('CeilingNatural','CeilingStretch')]:
        oa,ea=bounds[first];ob,eb=bounds[second];gap=(ob.x-eb.x)-(oa.x+ea.x)
        assert abs(gap)<.001
        r['checks'].append({'case':first+'_'+second+'_seam','gap_cm':gap})
    pawn=unreal.GameplayStatics.get_player_pawn(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent)
    radius=cap.get_scaled_capsule_radius();half=cap.get_scaled_capsule_half_height()
    for name,start,end,kind,expected in [
        ('beam_cross',(20000,-100,1016),(20000,100,1016),'line',True),
        ('pillar_cross',(21800,0,1200),(22200,0,1200),'capsule',True),
        ('ceiling_seam',(24250,0,1700),(24250,0,1500),'line',True),
        ('ceiling_below_clear',(23500,0,1450),(25000,0,1450),'capsule',False),
        ('ceiling_above_blocks',(24500,0,1450),(24500,0,1590),'capsule',True)]:
        start=unreal.Vector(*start);end=unreal.Vector(*end)
        if kind=='capsule':hit=unreal.SystemLibrary.capsule_trace_single_by_profile(w,start,end,radius,half,'Pawn',False,[],unreal.DrawDebugTrace.NONE)
        else:hit=unreal.SystemLibrary.line_trace_single_by_profile(w,start,end,'BlockAll',False,[],unreal.DrawDebugTrace.NONE)
        r['checks'].append({'case':name,'blocked':hit is not None,'expected':expected});assert (hit is not None)==expected,name
    r['status']='passed_part_seams_stretch_and_collision'
except Exception as e:r['status']='failed';r['error']=str(e);raise
finally:out.write_text(json.dumps(r,indent=2),encoding='utf-8')
print(json.dumps(r))
