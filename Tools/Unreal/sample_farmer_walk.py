import unreal,json,math
from pathlib import Path
root=Path(unreal.Paths.project_dir());reference=unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple');animation=unreal.load_asset('/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd')
names=[b['name'] for b in json.loads((root/'ArtSource/character/_reference/ue-reference-pose.json').read_text())['bones']]
def pose(mesh,t):
 opt=unreal.AnimPoseEvaluationOptions();opt.optional_skeletal_mesh=mesh;opt.extract_root_motion=False
 return unreal.AnimPoseExtensions.get_anim_pose_at_time(animation,t,opt)
def transform(p,n):return unreal.AnimPoseExtensions.get_bone_pose(p,n,unreal.AnimPoseSpaces.WORLD)
def encode(t):
 q=t.rotation;return {'translation':list(t.translation.to_tuple()),'rotation_xyzw':[q.x,q.y,q.z,q.w],'scale':list(t.scale3d.to_tuple())}
samples=[];errors={}
for name in ['SK_Farmer_A_Body','SK_Farmer_A_Arms']:errors[name]=0
for time in [0,.25,.5,.75,1,1.25,1.5]:
 ref=pose(reference,time);sample={'time_seconds':time,'bones':{n:encode(transform(ref,n)) for n in names}};samples.append(sample)
 for name in errors:
  mesh=unreal.load_asset('/Game/CL9Art/Characters/FarmerA/'+name);actual=pose(mesh,time)
  for n in names:errors[name]=max(errors[name],math.dist(transform(actual,n).translation.to_tuple(),sample['bones'][n]['translation']))
r={'animation':animation.get_path_name(),'duration_seconds':animation.sequence_length,'samples':samples,'max_source_vs_authored_bone_position_error_cm':errors,'pose_compatibility_passed':all(e<.01 for e in errors.values()),'scope':'Existing Walk evaluated on source/body/arms skeletal meshes. Skin deformation is a separate visual gate.'}
(root/'docs/asset/evidence/024-walk-pose-samples.json').write_text(json.dumps(r,indent=2),encoding='utf-8');print(json.dumps({k:v for k,v in r.items() if k!='samples'}))
