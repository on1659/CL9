"""Preview material-on appearance using actual emissive surface lighting in Cycles."""
import bpy,sys
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview,render_night
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/standing_lamp_a')
s=setup_creature_preview('CL9_STANDING_LAMP_A_PREVIEW',ROOT,focus=(0,0,.9),camera_position=(3,-4,2.6),ortho=2.8)
bpy.data.objects['CL9_STANDING_LAMP_A_PREVIEW_Spot'].data.energy=0
bs=next(n for n in bpy.data.materials['M_CL9_StandingLamp_Diffuser'].node_tree.nodes if n.type=='BSDF_PRINCIPLED')
bs.inputs['Emission Strength'].default_value=4
bg=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.025
for ob in s.objects:
    if ob.type=='LIGHT':ob.data.energy*=.2;ob.data.color=(.65,.78,1)
s.cycles.samples=48;s.render.filepath=str(ROOT/'preview/on_night.png');bpy.ops.render.render(write_still=True)
bs.inputs['Emission Strength'].default_value=0
setup_creature_preview(s.name,ROOT,focus=(0,0,.9),camera_position=(3,-4,2.6),ortho=2.8)
bpy.context.window.scene=bpy.data.scenes['CL9_STANDING_LAMP_A']
bpy.data.libraries.write(str(ROOT/'standing_lamp_a.blend'),{bpy.context.scene,s},fake_user=True)
print('Rendered emissive-on night view; restored source emission to zero')
