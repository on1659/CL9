"""Minimal part-only PIE: hinge pivot and capsule clearance, no flap gameplay claim."""
import unreal,json,datetime
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
fixtures={a.get_actor_label():a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor) if a.get_actor_label().startswith('QA014_')}
assert len(fixtures)==2
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/014-door-pie.json');assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
   'world_count':len(worlds),'level':w.get_path_name(),'status':'running','checks':[],
   'scope':'Two static part fixtures. Manually placed 90 degree pose is a hinge/clearance reference, not a gameplay animation.'}
try:
    pawn=unreal.GameplayStatics.get_player_pawn(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent)
    radius=cap.get_scaled_capsule_radius();half=cap.get_scaled_capsule_half_height()
    r['capsule_cm']={'radius':radius,'half_height':half}
    for label,base,blocked in [('QA014_Closed',20000,True),('QA014_OpenReference',21000,False)]:
        a=fixtures[label];origin,extent,_=unreal.SystemLibrary.get_component_bounds(a.static_mesh_component)
        dims=list((extent*2).to_tuple());expected=[131.987289,16,256.424286] if blocked else [16,131.987289,256.424286]
        assert max(abs(x-y) for x,y in zip(dims,expected))<.01
        assert abs(origin.z-extent.z-1000)<.001
        start=unreal.Vector(base+66,-200,1100);end=unreal.Vector(base+66,200,1100)
        hit=unreal.SystemLibrary.capsule_trace_single_by_profile(w,start,end,radius,half,'Pawn',False,[],unreal.DrawDebugTrace.NONE)
        result={'label':label,'dimensions_cm':dims,'bottom_z':origin.z-extent.z,'pivot_cm':list(a.get_actor_location().to_tuple()),'capsule_blocked':hit is not None}
        r['checks'].append(result);assert result['capsule_blocked']==blocked
    r['status']='passed_part_pivot_and_clearance'
    r['consumer_dependency']={'current_flap_height_cm':630,'source_height_cm':256.424286,
        'predicted_vertical_scale':630/256.424286,'predicted_push_grip_height_cm':110*630/256.424286,
        'rotation_animation':'No current consumer; CL9FlapIn only changes movement ignore.',
        'actual_flap_gameplay':'Not tested. Lvl_Flat editor has zero CL9FlapIn actors; no full map generated.'}
except Exception as e:r['status']='failed';r['error']=str(e);raise
finally:out.write_text(json.dumps(r,indent=2),encoding='utf-8')
print(json.dumps(r))
