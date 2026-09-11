"""One rear-quarter compatibility view, using copies of three separate deliverable parts."""
import bpy,sys,math
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview
root=Path('D:/Work/unreal/CL9/ArtSource/map/food_truck/body')
name='CL9_FOOD_TRUCK_FIT_PREVIEW';s=bpy.data.scenes.get(name) or bpy.data.scenes.new(name)
for original,pos,rot in [('SM_FoodTruck_A',(0,0,.002),0),('SM_FoodTruck_Steps_A',(-3.57,-.7825,.002),0),('SM_Kitchen_A',(-.47,.3375,.642),-90)]:
 key=name+'_'+original;o=bpy.data.objects.get(key)
 if o is None:o=bpy.data.objects[original].copy();o.name=key;s.collection.objects.link(o)
 o.location=pos;o.rotation_euler.z=math.radians(rot)
s=setup_creature_preview(name,root,focus=(-.5,0,1.5),camera_position=(-8,6,5.2),ortho=9.5)
s.render.filepath=str(root/'preview/rear_fit.png');bpy.ops.render.render(write_still=True)
bpy.context.window.scene=bpy.data.scenes['CL9_FOOD_TRUCK'];print('Saved rear compatibility view; source assets remain separate.')
