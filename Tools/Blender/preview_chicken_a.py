"""Render the actual weighted chicken and audit sampled action transforms."""
import bpy,math,json
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/chicken_a')
scene=bpy.data.scenes['CL9_CHICKEN_A'];bpy.context.window.scene=scene
rig=bpy.data.objects['SKEL_Chicken_A'];mesh=bpy.data.objects['SK_Chicken_A']
scene.render.engine='CYCLES';scene.cycles.samples=32
scene.render.resolution_x=1000;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
scene.world=bpy.data.worlds.get('CL9_CHICKEN_World') or bpy.data.worlds.new('CL9_CHICKEN_World');scene.world.use_nodes=True
bg=next(n for n in scene.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[0].default_value=(.12,.15,.18,1);bg.inputs[1].default_value=.35
def look(o,point):o.rotation_euler=(Vector(point)-o.location).to_track_quat('-Z','Y').to_euler()
camera_data=bpy.data.cameras.get('CL9_CHICKEN_Camera') or bpy.data.cameras.new('CL9_CHICKEN_Camera')
camera=bpy.data.objects.get('CL9_CHICKEN_Camera')
if camera is None:
    camera=bpy.data.objects.new(camera_data.name,camera_data);scene.collection.objects.link(camera)
camera.location=(2.8,-3.5,2.0);look(camera,(0,0,.65));camera_data.type='ORTHO';camera_data.ortho_scale=1.95;scene.camera=camera
for name,pos,energy,size in [('Key',(2,-3,4),420,3),('Fill',(-1,2,2),150,2)]:
    data=bpy.data.lights.get('CL9_CHICKEN_'+name) or bpy.data.lights.new('CL9_CHICKEN_'+name,'AREA')
    data.energy=energy;data.color=(1,1,1);data.shape='DISK';data.size=size
    ob=bpy.data.objects.get(data.name)
    if ob is None:ob=bpy.data.objects.new(data.name,data);scene.collection.objects.link(ob)
    ob.location=pos;look(ob,(0,0,.6))
ground=bpy.data.objects.get('CL9_CHICKEN_PreviewGround')
if ground is None:
    bpy.ops.mesh.primitive_plane_add(size=200,location=(0,0,0));ground=bpy.context.object;ground.name='CL9_CHICKEN_PreviewGround'
mat=bpy.data.materials.get('CL9_CHICKEN_Ground') or bpy.data.materials.new('CL9_CHICKEN_Ground');mat.diffuse_color=(.08,.095,.10,1);mat.use_nodes=True
bs=next(n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED');bs.inputs['Base Color'].default_value=(.08,.095,.1,1);bs.inputs['Roughness'].default_value=.9
ground.data.materials.clear();ground.data.materials.append(mat)
rig.animation_data.action=bpy.data.actions['AN_Chicken_Idle'];scene.frame_set(1)
scene.render.filepath=str(ROOT/'preview/neutral.png');bpy.ops.render.render(write_still=True)
manifest=json.loads((ROOT/'export/manifest.json').read_text());audit={}
for key,clip in manifest['clips'].items():
    rig.animation_data.action=bpy.data.actions[clip['action']]
    first=None;last=None;samples=[]
    for frame in range(clip['frames'][0],clip['frames'][1]+1):
        scene.frame_set(frame);bpy.context.view_layer.update()
        mats={p.name:p.matrix.copy() for p in rig.pose.bones}
        if first is None:first=mats
        last=mats
        evaluated=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
        coords=[evaluated.matrix_world@v.co for v in evaluated.data.vertices]
        samples.append({'frame':frame,'min_z_cm':min(v.z for v in coords)*100,
                        'root_displacement_cm':rig.pose.bones['root'].head.length*100,
                        'foot_l_cm':list(rig.pose.bones['foot_l'].head*100),
                        'foot_r_cm':list(rig.pose.bones['foot_r'].head*100)})
    error=max(abs(first[n][r][c]-last[n][r][c]) for n in first for r in range(4) for c in range(4))
    audit[key]={'loop':clip['loop'],'loop_matrix_max_error':error,'samples':samples}
    assert max(s['root_displacement_cm'] for s in samples)<.001,'Root moved'
    assert min(s['min_z_cm'] for s in samples)>-.1,'Ground penetration'
    if clip['loop']:assert error<.001,'Loop pose discontinuity'
    scene.frame_set(clip['frames'][1] if key=='Death' else max(1,round(clip['frames'][1]*.5)))
    if key in ('Catch','Death','WakeTelegraph') or not (ROOT/'preview'/f'{key.lower()}.png').exists():
        scene.render.filepath=str(ROOT/'preview'/f'{key.lower()}.png');bpy.ops.render.render(write_still=True)
(ROOT/'export/blender_animation_qa.json').write_text(json.dumps(audit,indent=2),encoding='utf-8')
rig.animation_data.action=bpy.data.actions['AN_Chicken_Idle'];scene.frame_start=1;scene.frame_end=61;scene.frame_set(1)
bg.inputs[1].default_value=.025
for ob in scene.objects:
    if ob.type=='LIGHT':ob.data.energy*=.2;ob.data.color=(.65,.78,1)
scene.render.filepath=str(ROOT/'preview/night.png');bpy.ops.render.render(write_still=True)
bpy.data.libraries.write(str(ROOT/'chicken_a.blend'),{scene,*[bpy.data.actions[c['action']] for c in manifest['clips'].values()]},fake_user=True)
print(json.dumps({k:{'loop_error':v['loop_matrix_max_error'],'lowest_z_cm':min(s['min_z_cm'] for s in v['samples'])} for k,v in audit.items()}))
