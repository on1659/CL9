"""Import the actual UE reference FBX; do not infer a compatible skeleton from names."""
import bpy,json,hashlib
from pathlib import Path
root=Path('D:/Work/unreal/CL9');folder=root/'ArtSource/character/_reference';fbx=folder/'farmer_a_existing_rig.fbx'
e=json.loads((root/'docs/asset/evidence/024-character-contract.json').read_text());assert hashlib.sha256(fbx.read_bytes()).hexdigest()==e['sha256']
assert 'CL9_FARMER_RIG_REFERENCE' not in bpy.data.scenes
s=bpy.data.scenes.new('CL9_FARMER_RIG_REFERENCE');s.unit_settings.system='METRIC';s.unit_settings.scale_length=1;bpy.context.window.scene=s
bpy.ops.import_scene.fbx(filepath=str(fbx),use_anim=False,automatic_bone_orientation=False,ignore_leaf_bones=False)
rigs=[o for o in s.objects if o.type=='ARMATURE'];meshes=[o for o in s.objects if o.type=='MESH'];assert len(rigs)==1 and meshes
rig=rigs[0];rig.name='REF024_Manny_Rig';bones=[]
for b in rig.data.bones:bones.append({'name':b.name,'parent':b.parent.name if b.parent else None,'matrix_local':[list(row) for row in b.matrix_local],'head':list(b.head_local),'tail':list(b.tail_local)})
rows=[]
for i,o in enumerate(meshes):
 original=o.name;o.name='REF024_Manny_Mesh_'+str(i);o.data.calc_loop_triangles()
 rows.append({'object':o.name,'imported_name':original,'triangles':len(o.data.loop_triangles),'vertices':len(o.data.vertices),'dimensions_cm':[v*100 for v in o.dimensions],'groups':len(o.vertex_groups),'materials':[m.name if m else None for m in o.data.materials]})
r={'reference_only':True,'source_sha256':e['sha256'],'scene':s.name,'rig_object':rig.name,'rig_matrix_world':[list(row) for row in rig.matrix_world],'bone_count':len(bones),'bones':bones,'meshes':rows,
 'contract':'Imported FBX reference transforms. No auto bone orientation or invented hierarchy. Must round-trip against UE before claiming animation compatibility.'}
(folder/'rig-reference.json').write_text(json.dumps(r,indent=2),encoding='utf-8');bpy.data.libraries.write(str(folder/'rig-reference.blend'),{s},fake_user=True)
print(json.dumps({'scene':s.name,'bones':len(bones),'root_bones':[b['name'] for b in bones if b['parent'] is None],'meshes':rows}))
