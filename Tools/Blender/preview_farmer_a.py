import bpy,json,sys,math
from pathlib import Path
from mathutils import Matrix,Quaternion,Vector
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview,render_night
root=Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a');rig=bpy.data.objects['RIG_Farmer_A'];preview=bpy.data.scenes['CL9_FARMER_A_PREVIEW']
if rig.name not in preview.objects:preview.collection.objects.link(rig)
for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
s=setup_creature_preview(preview.name,root,focus=(0,0,.94),camera_position=(2.1,-3.1,1.8),ortho=3.05);bpy.ops.render.render(write_still=True);render_night(s,root)
for o in s.objects:
 if o.type=='LIGHT':o.data.energy=0
bg=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.004
spot=bpy.data.objects.get('FARMER_Flashlight')
if spot is None:
 data=bpy.data.lights.new('FARMER_Flashlight','SPOT');spot=bpy.data.objects.new(data.name,data);s.collection.objects.link(spot)
data=spot.data;data.energy=220;data.spot_size=.75;data.spot_blend=.4;spot.location=s.camera.location;spot.rotation_euler=s.camera.rotation_euler;s.render.filepath=str(root/'preview/flashlight.png');bpy.ops.render.render(write_still=True);data.energy=0
s=setup_creature_preview(preview.name,root,focus=(0,0,.94),camera_position=(2.1,-3.1,1.8),ortho=3.05)
ref=json.loads(Path('D:/Work/unreal/CL9/ArtSource/character/_reference/ue-reference-pose.json').read_text());refs={b['name']:b['component'] for b in ref['bones']}
samples=json.loads(Path('D:/Work/unreal/CL9/docs/asset/evidence/024-walk-pose-samples.json').read_text())['samples'];C=Matrix.Diagonal((1,-1,1,1))
def matrix(t):
 q=t['rotation_xyzw'];m=Quaternion((q[3],q[0],q[1],q[2])).to_matrix().to_4x4();m.translation=Vector(t['translation'])*.01
 return C@m@C
def depth(b):return 0 if b.parent is None else 1+depth(b.parent)
try:
 for sample in [samples[1],samples[3]]:
  for b in sorted(rig.data.bones,key=depth):
   rig.pose.bones[b.name].matrix=matrix(sample['bones']['root']).inverted()@matrix(sample['bones'][b.name])@matrix(refs[b.name]).inverted()@b.matrix_local
   bpy.context.view_layer.update()
  s.render.filepath=str(root/'preview'/('walk_'+str(sample['time_seconds']).replace('.','_')+'.png'));bpy.ops.render.render(write_still=True)
finally:
 for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
 bpy.context.view_layer.update();bpy.context.window.scene=bpy.data.scenes['CL9_FARMER_A'];bpy.data.libraries.write(str(root/'farmer_a.blend'),{bpy.context.scene,s},fake_user=True)
print('Rendered neutral/night/flashlight and two existing UE Walk pose samples; source rig restored to rest.')
