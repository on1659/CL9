import bpy,sys
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview,render_night
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_access_parts');(ROOT/'preview').mkdir(exist_ok=True)
name='CL9_ACCESS_PARTS_REVIEW';s=bpy.data.scenes.get(name) or bpy.data.scenes.new(name)
for mesh,loc in [('SM_Henhouse_Wall_250_A',(-1.75,1,.002)),('SM_Henhouse_Wall_125_A',(1.1,1,.002)),('SM_Henhouse_Tread_A',(0,-1.1,.002))]:
    o=bpy.data.objects.get(name+'_'+mesh)
    if o is None:o=bpy.data.objects[mesh].copy();o.name=name+'_'+mesh;s.collection.objects.link(o)
    o.location=loc
def setup():
    s=setup_creature_preview(name,ROOT,focus=(-.4,0,1.65),camera_position=(7,-12,7),ortho=8.4)
    for o in s.objects:
        if o.type=='LIGHT':o.data.energy*=3
    return s
s=setup();bpy.ops.render.render(write_still=True);render_night(s,ROOT)
for o in s.objects:
    if o.type=='LIGHT':o.data.energy=0
bg=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.004
spot=bpy.data.objects.get(name+'_Spot')
if spot is None:
    d=bpy.data.lights.new(name+'_Spot','SPOT');spot=bpy.data.objects.new(d.name,d);s.collection.objects.link(spot)
spot.location=s.camera.location;spot.rotation_euler=s.camera.rotation_euler
spot.data.energy=2200;spot.data.spot_size=.8;spot.data.spot_blend=.4
s.render.filepath=str(ROOT/'preview/flashlight.png');bpy.ops.render.render(write_still=True)
spot.data.energy=0;setup();bpy.data.libraries.write(str(ROOT/'access_review.blend'),{s},fake_user=True)
bpy.context.window.scene=bpy.data.scenes['CL9_HENHOUSE_TREAD']
print('Saved three access part review images.')
