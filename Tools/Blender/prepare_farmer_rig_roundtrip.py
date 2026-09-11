"""Normalize imported reference into metres and export the original root as an FBX root node."""
import bpy,json,hashlib
from pathlib import Path
from mathutils import Matrix
root=Path('D:/Work/unreal/CL9/ArtSource/character/_reference')
assert 'CL9_FARMER_RIG_ROUNDTRIP' not in bpy.data.scenes and bpy.data.objects.get('root') is None
s=bpy.data.scenes.new('CL9_FARMER_RIG_ROUNDTRIP');s.unit_settings.system='METRIC';s.unit_settings.scale_length=1;bpy.context.window.scene=s
src=bpy.data.objects['REF024_Manny_Rig'];rig=src.copy();rig.data=src.data.copy();rig.name='root';s.collection.objects.link(rig)
rig.parent=None;rig.matrix_world=Matrix.Identity(4);rig.data.transform(Matrix.Scale(.01,4));rig.animation_data_clear()
src=bpy.data.objects['REF024_Manny_Mesh_1'];mesh=src.copy();mesh.data=src.data.copy();mesh.name='REF024_RoundtripMesh';s.collection.objects.link(mesh)
mesh.parent=rig;mesh.matrix_parent_inverse=Matrix.Identity(4);mesh.matrix_basis=Matrix.Identity(4);mesh.data.transform(Matrix.Scale(.01,4));mesh.animation_data_clear()
for mod in mesh.modifiers:
 if mod.type=='ARMATURE':mod.object=rig
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);mesh.select_set(True);bpy.context.view_layer.objects.active=rig
fbx=root/'rig_roundtrip.fbx'
bpy.ops.export_scene.fbx(filepath=str(fbx),use_selection=True,object_types={'MESH','ARMATURE'},axis_forward='-Y',axis_up='Z',global_scale=1,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',add_leaf_bones=False,use_armature_deform_only=False,armature_nodetype='ROOT',bake_anim=False,use_custom_props=False)
(root/'roundtrip-export.json').write_text(json.dumps({'source':'Existing Manny LOD2 as compatibility probe only','rig_object':rig.name,'bone_count_in_blender':len(rig.data.bones),'fbx_root_is_armature_object':True,'sha256':hashlib.sha256(fbx.read_bytes()).hexdigest(),'mesh_dimensions_cm':[v*100 for v in mesh.dimensions],'settings':{'forward':'-Y','up':'Z','scale_options':'FBX_SCALE_ALL','armature_nodetype':'ROOT'}},indent=2),encoding='utf-8')
bpy.data.libraries.write(str(root/'rig-roundtrip.blend'),{s},fake_user=True);print('Saved metre-normalized rig compatibility probe; original reference preserved.')
