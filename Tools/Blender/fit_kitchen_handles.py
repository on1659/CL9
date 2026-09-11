"""Fit 1mm handle overshoot before first import; restore exact centred footprint."""
import bpy,json,hashlib
from pathlib import Path
root=Path('D:/Work/unreal/CL9/ArtSource/map/kitchen');m=json.loads((root/'export/manifest.json').read_text())
assert not m.get('handle_fit');meta=m['assets']['kitchen'];o=bpy.data.objects[meta['object']]
scene=bpy.data.scenes['CL9_KITCHEN'];bpy.context.window.scene=scene
shift=-meta['anchors_cm']['CookTop'][0]/100
hulls=[h for h in scene.objects if h.name.startswith('UCX_'+o.name+'_')];assert len(hulls)==8
for obj in [o]+hulls:
 for v in obj.data.vertices:v.co.x+=shift
for v in o.data.vertices:
 if .55<v.co.z<.61 and v.co.x>.55:v.co.x-=.001
for values in meta['anchors_cm'].values():values[0]+=shift*100
for e in scene.objects:
 if e.type=='EMPTY':e.location.x+=shift
bpy.ops.object.select_all(action='DESELECT')
for obj in [o]+hulls:obj.hide_set(False);obj.select_set(True);obj.data.update()
bpy.context.view_layer.objects.active=o;bpy.context.view_layer.update()
assert abs(o.dimensions.x-1.2)<.00001
f=root/'export'/(o.name+'.fbx')
bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
meta['dimensions_cm']=[v*100 for v in o.dimensions];meta['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest();m['handle_fit']=True
for h in hulls:h.hide_set(True)
(root/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(root/m['source_blend']),{scene},fake_user=True)
print(meta['dimensions_cm'])
