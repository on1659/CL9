"""Separate centered 100cm export for CL9's existing Size/100 placement contract."""
import bpy,json,hashlib,math
from pathlib import Path
from mathutils import Vector

def export_stretch(root,source_key,name,yaw=0):
    root=Path(root);m=json.loads((root/'export/manifest.json').read_text());source=m['assets'][source_key]
    assert name not in bpy.data.objects,'Inspect existing variant before exporting again'
    original=bpy.data.objects[source['object']]
    lo=Vector(tuple(min(v.co[i] for v in original.data.vertices) for i in range(3)))
    hi=Vector(tuple(max(v.co[i] for v in original.data.vertices) for i in range(3)));center=(lo+hi)/2;size=hi-lo
    assert all(v>0 for v in size)
    collection=bpy.data.collections.new(name+'_EXPORT');bpy.context.scene.collection.children.link(collection)
    originals=[original]+[o for o in bpy.context.scene.objects if o.name.startswith('UCX_'+original.name+'_')]
    copies=[];c=math.cos(yaw);s=math.sin(yaw)
    for index,o in enumerate(originals):
        copy=o.copy();copy.data=o.data.copy();copy.name=name if index==0 else 'UCX_'+name+'_'+str(index-1).zfill(2)
        collection.objects.link(copy);copy.hide_render=False;copy.hide_set(False)
        for v in copy.data.vertices:
            p=v.co-center;x=p.x/size.x;y=p.y/size.y;v.co=(x*c-y*s,x*s+y*c,p.z/size.z)
        copies.append(copy)
    bpy.ops.object.select_all(action='DESELECT')
    for o in copies:o.select_set(True)
    bpy.context.view_layer.objects.active=copies[0];bpy.context.view_layer.update()
    f=root/'export'/(name+'.fbx')
    bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
    meta=dict(source);meta.update({'object':name,'dimensions_cm':[v*100 for v in copies[0].dimensions],
        'bounds_center_cm':[0,0,0],'anchors_cm':{},'pivot_override':'bounds center','normalized_yaw_degrees':math.degrees(yaw),
        'fbx_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'consumer':'Centered 100cm bounds; restore with placement Size/100'})
    m['assets'][source_key+'_stretch'+str(round(math.degrees(yaw)))]=meta
    (root/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
    for o in copies:o.hide_render=True;o.hide_set(True)
    bpy.data.libraries.write(str(root/m['source_blend']),{bpy.context.scene},fake_user=True)
    print('Saved '+name)
