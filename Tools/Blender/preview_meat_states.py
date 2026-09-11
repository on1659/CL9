"""Render three states together without changing export geometry."""
import bpy, sys
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview,render_night
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/meat_states')
assert 'CL9_MEAT_PREVIEW' not in bpy.data.scenes
p=bpy.data.scenes.new('CL9_MEAT_PREVIEW')
for i,state in enumerate(('Raw','Cooked','Burnt')):
    src=bpy.data.objects['SM_Meat_'+state+'_A'];o=src.copy();o.name='VIEW_Meat_'+state
    o.location=(0,(i-1)*.68,.175);p.collection.objects.link(o)
p=setup_creature_preview(p.name,ROOT,focus=(0,0,.10),camera_position=(2.6,-.65,2.8),ortho=2.35)
p.render.resolution_x=1400;p.render.resolution_y=800
bpy.data.libraries.write(str(ROOT/'meat_states.blend'),{bpy.data.scenes['CL9_MEAT_KIT'],p},fake_user=True)
bpy.ops.render.render(write_still=True)
render_night(p,ROOT)
# Flashlight from viewer position, subdued ambient, same exposure.
for ob in p.objects:
    if ob.type=='LIGHT':ob.data.energy=0
bg=next(n for n in p.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.004
data=bpy.data.lights.new('CL9_MEAT_PREVIEW_Flashlight','SPOT');data.energy=90;data.spot_size=1.1;data.spot_blend=.4;data.shadow_soft_size=.03
ob=bpy.data.objects.new(data.name,data);p.collection.objects.link(ob);ob.location=p.camera.location;ob.rotation_euler=p.camera.rotation_euler
p.render.filepath=str(ROOT/'preview/flashlight.png');bpy.ops.render.render(write_still=True)
bpy.context.window.scene=bpy.data.scenes['CL9_MEAT_KIT']
print('Saved neutral, night and flashlight comparisons in '+str(ROOT/'preview'))
