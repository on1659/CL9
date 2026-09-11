"""Keep metre authoring intact; use separate centimetre copies to avoid a 100x UE root scale."""
import bpy,json,hashlib
from pathlib import Path
from mathutils import Matrix
folder=Path('D:/Work/unreal/CL9/ArtSource/character/_reference');assert 'CL9_FARMER_RIG_CM_EXPORT' not in bpy.data.scenes
s=bpy.data.scenes.new('CL9_FARMER_RIG_CM_EXPORT');s.unit_settings.system='METRIC';s.unit_settings.scale_length=.01;bpy.context.window.scene=s
src=bpy.data.objects['root'];assert src in bpy.data.scenes['CL9_FARMER_RIG_ROUNDTRIP'].objects[:]
src.name='REF024_MetreRig';rig=src.copy();rig.data=src.data.copy();rig.name='root';s.collection.objects.link(rig);rig.data.transform(Matrix.Scale(100,4))
mesh=bpy.data.objects['REF024_RoundtripMesh'].copy();mesh.data=mesh.data.copy();mesh.name='REF024_CM_ProbeMesh';s.collection.objects.link(mesh);mesh.parent=rig;mesh.data.transform(Matrix.Scale(100,4))
for mod in mesh.modifiers:
 if mod.type=='ARMATURE':mod.object=rig
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);mesh.select_set(True);bpy.context.view_layer.objects.active=rig
fbx=folder/'rig_roundtrip_cm.fbx'
bpy.ops.export_scene.fbx(filepath=str(fbx),use_selection=True,object_types={'MESH','ARMATURE'},axis_forward='-Y',axis_up='Z',global_scale=1,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',add_leaf_bones=False,use_armature_deform_only=False,armature_nodetype='ROOT',bake_anim=False,use_custom_props=False)
rig.name='REF024_CM_Rig';src.name='root';bpy.context.window.scene=bpy.data.scenes['CL9_FARMER_RIG_ROUNDTRIP']
(folder/'roundtrip-cm-export.json').write_text(json.dumps({'source_unit_m':1,'staging_unit_m':.01,'source_data_unchanged':True,'sha256':hashlib.sha256(fbx.read_bytes()).hexdigest()},indent=2),encoding='utf-8');print('Exported centimetre staging probe; metre source unchanged.')
