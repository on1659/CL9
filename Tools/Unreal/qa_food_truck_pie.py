"""Four isolated fixtures: truck/steps/kitchen footprint/support floor."""
import unreal,json,datetime
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label().replace('QA023_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor) if a.get_actor_label().startswith('QA023_')};assert len(actors)==4
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/023-food-truck-pie.json');assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'level':w.get_path_name(),'world_count':len(worlds),'checks':[],
 'scope':'Isolated truck parts and existing kitchen footprint. Capsule sweeps, not keyboard walking, whole-map layout, 2-player or performance validation.'}
pawn=unreal.GameplayStatics.get_player_pawn(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent)
radius=cap.get_scaled_capsule_radius();half=cap.get_scaled_capsule_half_height();r['capsule_cm']={'radius':radius,'half_height':half}
def check(name,value,expected=True,**extra):r['checks'].append(dict(case=name,passed=value==expected,**extra))
def support_z(x,y,top,bottom):
 # This engine exposes HitResult as opaque; locate the first blocking height to 0.002cm.
 def blocked(z):return unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(x,y,top),unreal.Vector(x,y,z),'BlockAll',False,[pawn],unreal.DrawDebugTrace.NONE) is not None
 if not blocked(bottom):return None
 low=bottom;high=top
 for _ in range(18):
  mid=(low+high)/2
  if blocked(mid):low=mid
  else:high=mid
 return (low+high)/2
for name,dims in [('Body',(630,353.5,325.1)),('Steps',(120,114,64)),('Kitchen',(300,120,90))]:
 origin,extent,_=unreal.SystemLibrary.get_component_bounds(actors[name].static_mesh_component)
 actual=(extent*2).to_tuple();check(name+'_bounds',max(abs(a-b) for a,b in zip(actual,dims))<.01,dimensions_cm=actual)
for i in range(4):
 x=19643-45+i*30;z=support_z(x,78.25,1200,990)
 check('step_'+str(i+1),z is not None and abs(z-(1016+i*16))<.01,support_z=z)
for name,start,end,expected in [
 ('rear_entry',(19660,70.25,1064+half+2),(19740,70.25,1064+half+2),False),
 ('aisle_with_kitchen',(19740,70.25,1064+half+2),(20040,70.25,1064+half+2),False),
 ('blind_wall',(20000,80,1190),(20000,200,1190),True),
 ('cab_bulkhead',(20050,78.25,1190),(20160,78.25,1190),True),
 ('ceiling',(19900,78.25,1200),(19900,78.25,1350),True)]:
 hit=unreal.SystemLibrary.capsule_trace_single_by_profile(w,unreal.Vector(*start),unreal.Vector(*end),radius,half,'Pawn',False,[pawn],unreal.DrawDebugTrace.NONE)
 check(name,hit is not None,expected,blocked=hit is not None)
for name,x in [('top_step',19690),('cargo_floor',19715)]:
 z=support_z(x,78.25,1150,1000);check(name+'_threshold_height',z is not None and abs(z-1064)<.01,support_z=z)
r['status']='passed_part_checks' if all(c['passed'] for c in r['checks']) else 'failed_part_checks'
out.write_text(json.dumps(r,indent=2),encoding='utf-8');print(json.dumps(r))
