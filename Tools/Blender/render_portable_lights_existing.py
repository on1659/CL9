import bpy,sys,math
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview,render_night
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/portable_lights')
p=setup_creature_preview('CL9_PORTABLE_LIGHTS_PREVIEW',ROOT,focus=(0,0,.06),camera_position=(1.3,-1.6,1.35),ortho=.99)
spot=bpy.data.objects['CL9_PORTABLE_LIGHTS_PREVIEW_Spot'];spot.data.energy=0
for name in ('SM_Flashlight_A','SM_Headlamp_A'):
    ob=bpy.data.objects['VIEW_'+name];ob.rotation_euler=(0,0,0);ob.location.z=bpy.data.objects[name].dimensions.z/2+.002
bpy.ops.render.render(write_still=True);render_night(p,ROOT)
for ob in p.objects:
    if ob.type=='LIGHT':ob.data.energy=0
bg=next(n for n in p.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.004
spot.data.energy=55;spot.data.color=(1,1,1)
p.render.filepath=str(ROOT/'preview/flashlight.png');bpy.ops.render.render(write_still=True)
spot.data.energy=0
p=setup_creature_preview(p.name,ROOT,focus=(0,0,.06),camera_position=(1.3,-1.6,1.35),ortho=.99)
helmet=bpy.data.objects['VIEW_SM_Headlamp_A'];helmet.rotation_euler.y=math.pi;helmet.location.z=.075
p.render.filepath=str(ROOT/'preview/helmet_underside.png');bpy.ops.render.render(write_still=True)
helmet.rotation_euler.y=0;helmet.location.z=bpy.data.objects['SM_Headlamp_A'].dimensions.z/2+.002
bpy.context.window.scene=bpy.data.scenes['CL9_PORTABLE_LIGHTS']
bpy.data.libraries.write(str(ROOT/'portable_lights.blend'),{bpy.context.scene,p},fake_user=True)
print('Updated four previews; neutral source lighting restored')
