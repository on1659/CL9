"""Run with Blender MCP against the existing sample; add authored convex collision."""
import bpy
import json
from pathlib import Path
from mathutils import Vector

root=Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_sample_01')
manifest=json.loads((root/'export/manifest.json').read_text())
previous=bpy.context.window.scene
kit=bpy.data.scenes['CL9_HH01_KIT']
bpy.context.window.scene=kit
collection=bpy.data.collections.get('CL9_HH01_COLLISION')
if collection is None:
    collection=bpy.data.collections.new('CL9_HH01_COLLISION')
    kit.collection.children.link(collection)
crate=bpy.data.objects['SM_Henhouse_Crate_A']
low=min(v.co.z for v in crate.data.vertices)
if abs(low)>1e-6:
    for v in crate.data.vertices: v.co.z-=low
    crate.data.update()
def hull(name,center,size):
    obj=bpy.data.objects.get(name)
    if obj is None:
        bpy.ops.mesh.primitive_cube_add(size=1,location=center)
        obj=bpy.context.object;obj.name=name
        obj.dimensions=size
        bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
        for coll in list(obj.users_collection): coll.objects.unlink(obj)
        collection.objects.link(obj)
        obj.display_type='WIRE';obj.hide_render=True
    return obj
for key,meta in manifest['assets'].items():
    obj=bpy.data.objects[meta['object']]
    shapes=[]
    if key=='nest':
        shapes+=[((0,0,z),(3,1,.12)) for z in (.35,.97,1.54)]
        shapes.append(((0,.45,.96),(3,.1,1.16)))
        shapes += [((x,0,.96),(.1,1,1.28)) for x in (-1.45,-.5,.5,1.45)]
        shapes += [((x,y,.175),(.15,.15,.35)) for x in (-1.4,1.4) for y in (-.4,.4)]
        shapes += [((0,-.44,z),(3,.12,.18)) for z in (.45,1.07)]
    elif key=='trough':
        shapes=[((0,0,.22),(2.4,.65,.20))]
        shapes += [((0,y,.32),(2.4,.07,.28)) for y in (-.29,.29)]
        shapes += [((x,0,.32),(.08,.65,.28)) for x in (-1.16,1.16)]
        shapes += [((x,y,.06),(.12,.12,.12)) for x in (-1,1) for y in (-.23,.23)]
    elif key not in ('egg','lamp'):
        coords=[v.co for v in obj.data.vertices]
        lo=Vector(tuple(min(v[i] for v in coords) for i in range(3)))
        hi=Vector(tuple(max(v[i] for v in coords) for i in range(3)))
        shapes=[((lo+hi)*.5,hi-lo)]
    collisions=[hull('UCX_'+obj.name+'_'+str(i).zfill(2),center,size) for i,(center,size) in enumerate(shapes)]
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    for c in collisions:c.select_set(True)
    bpy.context.view_layer.objects.active=obj
    bpy.ops.export_scene.fbx(filepath=str(root/'export'/(obj.name+'.fbx')),use_selection=True,
        object_types={'MESH'},global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',
        bake_anim=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    meta['collision_hulls']=len(collisions)
    meta['collision_reason']='Item sphere owns collision' if key=='egg' else 'Ceiling light outside walkable reach' if key=='lamp' else 'Authored convex boxes'
manifest['crate_pivot_fixed']='2026-09-11: min Z -0.5cm corrected to zero; shared stage instances updated'
(root/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.context.window.scene=previous
bpy.ops.wm.save_as_mainfile(filepath=str(root/'henhouse_sample_01.blend'))
print({k:v['collision_hulls'] for k,v in manifest['assets'].items()})
