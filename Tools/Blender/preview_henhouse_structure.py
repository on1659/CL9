"""Parts comparison, not a room or map. Ceiling is turned to show its underside."""
import bpy,sys,math
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview,render_night
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_structure');(ROOT/'preview').mkdir(exist_ok=True)
name='CL9_STRUCTURE_REVIEW';s=bpy.data.scenes.get(name) or bpy.data.scenes.new(name)
for mesh,loc,rot in [('SM_Henhouse_Beam_500_A',(-2,-1.8,.002),(0,0,0)),
                     ('SM_Henhouse_Pillar_400_A',(-4.8,.8,.002),(0,0,0)),
                     ('SM_Henhouse_Ceiling_500_A',(3.3,1,2.502),(-math.pi/2,0,0))]:
    copy=bpy.data.objects.get(name+'_'+mesh)
    if copy is None:copy=bpy.data.objects[mesh].copy();copy.name=name+'_'+mesh;s.collection.objects.link(copy)
    copy.location=loc;copy.rotation_euler=rot
def setup():
    s=setup_creature_preview(name,ROOT,focus=(.4,0,2.1),camera_position=(10,-16,10),ortho=14)
    for o in s.objects:
        if o.type=='LIGHT':o.data.energy*=4
    return s
s=setup();bpy.ops.render.render(write_still=True);render_night(s,ROOT)
for o in s.objects:
    if o.type=='LIGHT':o.data.energy=0
bg=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.004
spot=bpy.data.objects.get(name+'_Spot')
if spot is None:
    d=bpy.data.lights.new(name+'_Spot','SPOT');spot=bpy.data.objects.new(d.name,d);s.collection.objects.link(spot)
spot.location=s.camera.location;spot.rotation_euler=s.camera.rotation_euler
spot.data.energy=3500;spot.data.spot_size=.8;spot.data.spot_blend=.4
s.render.filepath=str(ROOT/'preview/flashlight.png');bpy.ops.render.render(write_still=True)
spot.data.energy=0;setup()
bpy.data.libraries.write(str(ROOT/'structure_review.blend'),{s},fake_user=True)
bpy.context.window.scene=bpy.data.scenes['CL9_HENHOUSE_CEILING']
print('Saved three structure comparison renders and a separate review scene.')
