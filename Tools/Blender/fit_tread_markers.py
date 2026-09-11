"""Fit the newly made marker faces inside the specified depth before first UE import."""
import bpy,json,hashlib,math
from pathlib import Path
from mathutils import Vector
root=Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_access_parts/tread')
m=json.loads((root/'export/manifest.json').read_text());assert not m.get('marker_fit')
bpy.context.window.scene=bpy.data.scenes['CL9_HENHOUSE_TREAD']
base=bpy.data.objects['SM_Henhouse_Tread_A'];hull=bpy.data.objects['UCX_SM_Henhouse_Tread_A_00']
color=base.data.color_attributes['Color'];yellow=set();body=set()
for p in base.data.polygons:
    if p.material_index==0:body.update(p.vertices)
    if all(color.data[i].color[0]>.4 for i in p.loop_indices):yellow.update(p.vertices)
assert len(yellow)==72 and len(body)==24
for o in [base,hull]:
    for v in o.data.vertices:v.co.y-=.0003
for i in yellow:base.data.vertices[i].co.y+=.0006
for i in body:
    if base.data.vertices[i].co.y<0:base.data.vertices[i].co.y+=.001
size=Vector((max(v.co[i] for v in base.data.vertices)-min(v.co[i] for v in base.data.vertices) for i in range(3)))
center=Vector(tuple((max(v.co[i] for v in base.data.vertices)+min(v.co[i] for v in base.data.vertices))/2 for i in range(3)))
for key,meta in m['assets'].items():
    name=meta['object'];o=bpy.data.objects[name];h=bpy.data.objects['UCX_'+name+'_00']
    if o!=base:
        angle=math.radians(meta['normalized_yaw_degrees']);c=math.cos(angle);s=math.sin(angle)
        for target,original in [(o,base),(h,hull)]:
            for v,w in zip(target.data.vertices,original.data.vertices):
                p=w.co-center;x=p.x/size.x;y=p.y/size.y;v.co=(x*c-y*s,x*s+y*c,p.z/size.z)
    bpy.ops.object.select_all(action='DESELECT')
    for obj in [o,h]:obj.hide_set(False);obj.select_set(True);obj.data.update()
    bpy.context.view_layer.objects.active=o;bpy.context.view_layer.update()
    f=root/'export'/(name+'.fbx')
    bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
    meta['dimensions_cm']=[v*100 for v in o.dimensions];meta['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest()
    if o==base:meta['anchors_cm']={'Top':[0,0,30]};meta['bounds_center_cm']=[0,0,15]
    h.hide_set(True)
    if o!=base:o.hide_set(True)
m['marker_fit']=True;(root/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(root/'tread.blend'),{bpy.context.scene},fake_user=True)
print('Tread marker corrected; depth cm:',m['assets']['henhouse_tread']['dimensions_cm'][1])
