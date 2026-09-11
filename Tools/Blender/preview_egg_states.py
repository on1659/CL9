"""Project fracture ribbons onto the actual low-poly shell and render comparisons."""
import bpy, json, math
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/egg_states')
kit=bpy.data.scenes['CL9_EGG01_KIT'];bpy.context.window.scene=kit
raw=bpy.data.objects['SM_Egg_Raw_A']
for name in ('SM_Egg_Cracked_A','SM_Egg_Burnt_A'):
    ob=bpy.data.objects[name]
    verts=set(v for p in ob.data.polygons if p.material_index==1 for v in p.vertices)
    for i in verts:
        v=ob.data.vertices[i]; radial=Vector((v.co.x,v.co.y,0)).normalized()
        hit,co,n,face=raw.ray_cast(radial*.4+Vector((0,0,v.co.z)),-radial)
        assert hit
        v.co=co+n*.0006
    ob.data.update()
preview=bpy.data.scenes.get('CL9_EGG01_PREVIEW') or bpy.data.scenes.new('CL9_EGG01_PREVIEW')
assert not preview.objects, 'Preview already built; inspect before rebuilding'
bpy.context.window.scene=preview
preview.render.engine='CYCLES';preview.cycles.samples=32
preview.render.resolution_x=1600;preview.render.resolution_y=660;preview.render.resolution_percentage=100
preview.world=bpy.data.worlds.new('CL9_EGG01_World');preview.world.use_nodes=True
background=next(n for n in preview.world.node_tree.nodes if n.type=='BACKGROUND')
background.inputs[0].default_value=(.1,.12,.15,1)
background.inputs[1].default_value=.4
keys=['Raw','Cooked','Burnt','Cracked','Broken']
for i,key in enumerate(keys):
    src=bpy.data.objects[f'SM_Egg_{key}_A'];ob=src.copy();ob.name='VIEW_'+key
    ob.location=(0,(i-2)*.52,.2);preview.collection.objects.link(ob)
    font=bpy.data.curves.new('Label_'+key,'FONT');font.body=key.upper();font.align_x='CENTER';font.size=.045
    label=bpy.data.objects.new('Label_'+key,font);preview.collection.objects.link(label)
    label.location=(.29,(i-2)*.52,-.06);label.rotation_euler=(math.pi/2,0,math.pi/2)

def look(o,target):o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
camera_data=bpy.data.cameras.new('CL9_EGG01_Camera');camera=bpy.data.objects.new('CL9_EGG01_Camera',camera_data)
preview.collection.objects.link(camera);camera.location=(3.2,-.12,1.1);look(camera,(0,0,.13))
camera_data.type='ORTHO';camera_data.ortho_scale=2.85;preview.camera=camera
for name,pos,power,size in [('Key',(2,-1,2.7),180,3),('Fill',(1,2,1.2),70,2)]:
    data=bpy.data.lights.new('CL9_EGG01_'+name,'AREA');data.energy=power;data.shape='DISK';data.size=size
    ob=bpy.data.objects.new(data.name,data);preview.collection.objects.link(ob);ob.location=pos;look(ob,(0,0,.2))
preview.render.filepath=str(ROOT/'preview/neutral.png')
bpy.data.libraries.write(str(ROOT/'egg_states.blend'),{kit,preview},fake_user=True)
bpy.ops.render.render(write_still=True)
# Keep source manifest current after projecting surface detail.
manifest=json.loads((ROOT/'export/manifest.json').read_text())
bpy.context.window.scene=kit
for key,rec in manifest['assets'].items():
    ob=bpy.data.objects[rec['object']]
    rec['dimensions_cm']=[v*100 for v in ob.dimensions]
    bpy.ops.object.select_all(action='DESELECT');ob.select_set(True);bpy.context.view_layer.objects.active=ob
    bpy.ops.export_scene.fbx(filepath=str(ROOT/'export'/f'{ob.name}.fbx'),use_selection=True,object_types={'MESH'},
        global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',bake_anim=False,add_leaf_bones=False)
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.context.window.scene=preview
print('Saved egg comparison:',preview.render.filepath)
