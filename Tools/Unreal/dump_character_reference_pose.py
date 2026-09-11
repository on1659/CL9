import unreal,json
from pathlib import Path
mesh=unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple');sk=mesh.skeleton
modifier=unreal.SkeletonModifier();assert modifier.set_skeletal_mesh(mesh)
def tr(t):
 return {'translation':list(t.translation.to_tuple()),'rotation_xyzw':[t.rotation.x,t.rotation.y,t.rotation.z,t.rotation.w],'scale':list(t.scale3d.to_tuple())}
names=modifier.get_all_bone_names();rows=[]
for n in names:rows.append({'name':str(n),'parent':str(mesh.get_bone_parent(n)),'local':tr(modifier.get_bone_transform(n,False)),'component':tr(modifier.get_bone_transform(n,True))})
r={'mesh':mesh.get_path_name(),'skeleton':sk.get_path_name(),'bone_count':len(rows),'bones':rows,'read_only':'SkeletonModifier queried; no changes or commit to source.'}
out=Path(unreal.Paths.project_dir(),'ArtSource/character/_reference/ue-reference-pose.json');assert not out.exists();out.write_text(json.dumps(r,indent=2),encoding='utf-8')
print(json.dumps({'bone_count':len(rows),'roots':[r for r in rows if r['parent']=='None'],'sample':rows[1]}))
