"""Neutral, night, flashlight and helmet underside views from preserved sources."""
import bpy,sys,math
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview,render_night
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/portable_lights')
assert 'CL9_PORTABLE_LIGHTS_PREVIEW' not in bpy.data.scenes
p=bpy.data.scenes.new('CL9_PORTABLE_LIGHTS_PREVIEW')
copies=[]
for i,name in enumerate(('SM_Flashlight_A','SM_Headlamp_A')):
    src=bpy.data.objects[name];o=src.copy();o.name='VIEW_'+name;o.location=(0,(i-.5)*.40,src.dimensions.z/2+.002)
    p.collection.objects.link(o);copies.append(o)
p=setup_creature_preview(p.name,ROOT,focus=(0,0,.06),camera_position=(1.3,-1.6,1.35),ortho=.99)
bpy.data.libraries.write(str(ROOT/'portable_lights.blend'),{bpy.data.scenes['CL9_PORTABLE_LIGHTS'],p},fake_user=True)
bpy.ops.render.render(write_still=True)
render_night(p,ROOT)
for ob in p.objects:
    if ob.type=='LIGHT':ob.data.energy=0
bg=next(n for n in p.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.004
data=bpy.data.lights.new('CL9_PORTABLE_LIGHTS_PREVIEW_Spot','SPOT');data.energy=55;data.spot_size=.8;data.spot_blend=.35;data.shadow_soft_size=.02
ob=bpy.data.objects.new(data.name,data);p.collection.objects.link(ob);ob.location=p.camera.location;ob.rotation_euler=p.camera.rotation_euler
p.render.filepath=str(ROOT/'preview/flashlight.png');bpy.ops.render.render(write_still=True)
ob.data.energy=0
p=setup_creature_preview(p.name,ROOT,focus=(0,0,.06),camera_position=(1.3,-1.6,1.35),ortho=.99)
copies[1].rotation_euler.y=math.pi;copies[1].location.z=.075
p.render.filepath=str(ROOT/'preview/helmet_underside.png');bpy.ops.render.render(write_still=True)
copies[1].rotation_euler.y=0;copies[1].location.z=bpy.data.objects['SM_Headlamp_A'].dimensions.z/2+.002
bpy.context.window.scene=bpy.data.scenes['CL9_PORTABLE_LIGHTS']
print('Saved four preview images')
