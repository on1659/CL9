"""Reusable non-destructive neutral/night/spot comparison stage for authored props."""
import bpy,sys
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview,render_night
ROOT=Path(ASSET_ROOT);kit=bpy.data.scenes[KIT_SCENE]
prefix=KIT_SCENE+'_PREVIEW';p=bpy.data.scenes.get(prefix) or bpy.data.scenes.new(prefix)
spacing=globals().get('SPACING',.68);ortho=globals().get('ORTHO',1.5)
focus=globals().get('FOCUS',(0,0,.09));camera=globals().get('CAMERA_POSITION',(2,-2.3,2.6))
for i,name in enumerate(OBJECT_NAMES):
    original=bpy.data.objects[name];copy_name=prefix+'_'+name
    o=bpy.data.objects.get(copy_name)
    if o is None:o=original.copy();o.name=copy_name;p.collection.objects.link(o)
    bottom=min(v[2] for v in original.bound_box)
    o.location=(0,(i-(len(OBJECT_NAMES)-1)/2)*spacing,-bottom+.002)
    if name in globals().get('PLACEMENTS',{}):o.location=PLACEMENTS[name]
p=setup_creature_preview(p.name,ROOT,focus=focus,camera_position=camera,ortho=ortho)
spot=bpy.data.objects.get(prefix+'_Spot')
if spot:spot.data.energy=0
bpy.data.libraries.write(str(ROOT/SOURCE_BLEND),{kit,p},fake_user=True)
bpy.ops.render.render(write_still=True);render_night(p,ROOT)
for o in p.objects:
    if o.type=='LIGHT':o.data.energy=0
bg=next(n for n in p.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.004
if spot is None:
    data=bpy.data.lights.new(prefix+'_Spot','SPOT');spot=bpy.data.objects.new(data.name,data);p.collection.objects.link(spot)
spot.data.energy=globals().get('FLASHLIGHT_ENERGY',110);spot.data.color=(1,1,1);spot.data.spot_size=.8;spot.data.spot_blend=.4;spot.data.shadow_soft_size=.02
spot.location=p.camera.location;spot.rotation_euler=p.camera.rotation_euler
p.render.filepath=str(ROOT/'preview/flashlight.png');bpy.ops.render.render(write_still=True)
spot.data.energy=0
setup_creature_preview(p.name,ROOT,focus=focus,camera_position=camera,ortho=ortho)
bpy.context.window.scene=kit;bpy.data.libraries.write(str(ROOT/SOURCE_BLEND),{kit,p},fake_user=True)
print('Saved three comparison renders at '+str(ROOT/'preview'))
