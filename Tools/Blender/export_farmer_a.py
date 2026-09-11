"""Validate original body/arms and export disposable cm staging copies with the original root name."""
import bpy,bmesh,json,hashlib,math
from pathlib import Path
from mathutils import Matrix
root=Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a');p=root/'manifest.json';m=json.loads(p.read_text());folder=root/'export';folder.mkdir(exist_ok=True)
src=bpy.data.objects['RIG_Farmer_A'];assert src.parent is None and max(abs(v-1) for v in src.scale)<1e-6
stage=bpy.data.scenes.get('CL9_FARMER_A_EXPORT')
if stage:
 assert all(o.name in ['EXPORT_Farmer_A_Rig','EXPORT_SK_Farmer_A_Body','EXPORT_SK_Farmer_A_Arms'] for o in stage.objects)
 for o in list(stage.objects):bpy.data.objects.remove(o,do_unlink=True)
else:stage=bpy.data.scenes.new('CL9_FARMER_A_EXPORT')
stage.unit_settings.system='METRIC';stage.unit_settings.scale_length=.01;bpy.context.window.scene=stage
occupied=bpy.data.objects.get('root');assert occupied is None or occupied in bpy.data.scenes['CL9_FARMER_RIG_ROUNDTRIP'].objects[:]
if occupied:occupied.name='REF024_ReservedRoot'
rig=src.copy();rig.data=src.data.copy();rig.name='root';rig.data.transform(Matrix.Scale(100,4));stage.collection.objects.link(rig)
try:
 for name in ['SK_Farmer_A_Body','SK_Farmer_A_Arms']:
  original=bpy.data.objects[name];bm=bmesh.new();bm.from_mesh(original.data)
  assert all(e.is_manifold for e in bm.edges),name
  assert all(f.calc_area()>1e-12 for f in bm.faces),name
  bm.free();assert all(abs(sum(g.weight for g in v.groups)-1)<1e-4 for v in original.data.vertices)
  assert all(g.name in src.data.bones for g in original.vertex_groups)
  o=original.copy();o.data=original.data.copy();o.name='EXPORT_'+name;stage.collection.objects.link(o);o.parent=rig;o.hide_render=False;o.hide_set(False);o.data.transform(Matrix.Scale(100,4))
  for mod in o.modifiers:
   if mod.type=='ARMATURE':mod.object=rig
  bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);o.select_set(True);bpy.context.view_layer.objects.active=rig
  f=folder/(name+'.fbx')
  bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH','ARMATURE'},axis_forward='-Y',axis_up='Z',global_scale=1,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',add_leaf_bones=False,use_armature_deform_only=False,armature_nodetype='ROOT',bake_anim=False,use_custom_props=False)
  original.data.calc_loop_triangles();meta=m['assets'][name];meta.update({'triangles':len(original.data.loop_triangles),'vertices':len(original.data.vertices),'uv_layers':len(original.data.uv_layers),'dimensions_cm':[100*v for v in original.dimensions],'materials':[s.name for s in original.data.materials],'fbx_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'weights_normalized':True,'manifold_islands':True,'max_influences':max(len(v.groups) for v in original.data.vertices)})
  o.hide_set(True)
finally:
 rig.name='EXPORT_Farmer_A_Rig'
 if occupied:occupied.name='root'
m['export_settings']={'staging_unit_m':.01,'source_unit_m':1,'forward':'-Y','up':'Z','scale_options':'FBX_SCALE_ALL','root_node':'root','bone_count_in_ue_expected':89,'ue_force_front_x_axis':False}
p.write_text(json.dumps(m,indent=2),encoding='utf-8');(folder/'manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.context.window.scene=bpy.data.scenes['CL9_FARMER_A'];bpy.data.libraries.write(str(root/'farmer_a.blend'),{bpy.context.scene,bpy.data.scenes['CL9_FARMER_A_PREVIEW']},fake_user=True);print(json.dumps(m['assets']))
