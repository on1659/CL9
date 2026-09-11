"""Reusable render stage for CL9-owned creature scenes. Does not edit source geometry."""
import bpy
from pathlib import Path
from mathutils import Vector

def setup_creature_preview(scene_name,asset_directory,focus=(0,0,.55),camera_position=(2.8,-3.5,2),ortho=2.6):
    s=bpy.data.scenes[scene_name];bpy.context.window.scene=s
    s.render.engine='CYCLES';s.cycles.samples=24
    s.render.resolution_x=1200;s.render.resolution_y=900;s.render.resolution_percentage=100
    s.render.image_settings.media_type='IMAGE';s.render.image_settings.file_format='PNG'
    prefix=scene_name+'_PREVIEW_'
    s.world=bpy.data.worlds.get(prefix+'World') or bpy.data.worlds.new(prefix+'World');s.world.use_nodes=True
    bg=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND')
    bg.inputs[0].default_value=(.12,.15,.18,1);bg.inputs[1].default_value=.35
    def look(o):o.rotation_euler=(Vector(focus)-o.location).to_track_quat('-Z','Y').to_euler()
    camera=bpy.data.objects.get(prefix+'Camera')
    if camera is None:
        data=bpy.data.cameras.new(prefix+'Camera');camera=bpy.data.objects.new(data.name,data);s.collection.objects.link(camera)
    camera.location=camera_position;camera.data.type='ORTHO';camera.data.ortho_scale=ortho;look(camera);s.camera=camera
    for label,pos,power,size in [('Key',(2,-3,4),420,3),('Fill',(-1,2,2),150,2)]:
        ob=bpy.data.objects.get(prefix+label)
        if ob is None:
            data=bpy.data.lights.new(prefix+label,'AREA');ob=bpy.data.objects.new(data.name,data);s.collection.objects.link(ob)
        ob.location=pos;ob.data.energy=power;ob.data.color=(1,1,1);ob.data.shape='DISK';ob.data.size=size;look(ob)
    floor=bpy.data.objects.get(prefix+'Floor')
    if floor is None:
        bpy.ops.mesh.primitive_plane_add(size=200);floor=bpy.context.object;floor.name=prefix+'Floor'
        mat=bpy.data.materials.new(prefix+'FloorMat');mat.use_nodes=True
        bs=next(n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
        bs.inputs['Base Color'].default_value=(.075,.086,.09,1);bs.inputs['Roughness'].default_value=.9
        floor.data.materials.append(mat)
    s.render.filepath=str(Path(asset_directory)/'preview/neutral.png')
    return s

def render_night(s,asset_directory):
    bg=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.025
    for ob in s.objects:
        if ob.type=='LIGHT' and ob.name.startswith(s.name+'_PREVIEW_'):
            ob.data.energy*=.2;ob.data.color=(.65,.78,1)
    s.render.filepath=str(Path(asset_directory)/'preview/night.png');bpy.ops.render.render(write_still=True)
