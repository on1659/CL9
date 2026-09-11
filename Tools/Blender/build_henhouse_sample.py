"""CL9 first modular henhouse art sample. Run in Blender through MCP.

build_kit(), build_stage(), export_kit(), save_sample() are separate steps.
Only CL9_HH01-owned objects/scenes are changed; the existing scene is preserved.
"""
import bpy
import math
import json
import random
from pathlib import Path
from mathutils import Vector

ROOT = Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_sample_01')
PREFIX = 'CL9_HH01'
for folder in ('export', 'preview'):
    (ROOT / folder).mkdir(parents=True, exist_ok=True)
rng = random.Random(910)


def scene(name):
    old = bpy.data.scenes.get(name)
    if old:
        raise RuntimeError(f'{name} already exists: inspect before rebuilding')
    s = bpy.data.scenes.new(name)
    bpy.context.window.scene = s
    s.unit_settings.system = 'METRIC'
    s.unit_settings.scale_length = 1.0
    s.render.fps = 30
    return s


def mat(name, color, rough=.8, metal=0, emission=0):
    m = bpy.data.materials.new(PREFIX + '_' + name)
    m.diffuse_color = (*color, 1)
    m.use_nodes = True
    bs = next(n for n in m.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
    bs.inputs['Base Color'].default_value = (*color, 1)
    bs.inputs['Roughness'].default_value = rough
    bs.inputs['Metallic'].default_value = metal
    if emission:
        bs.inputs['Emission Color'].default_value = (*color, 1)
        bs.inputs['Emission Strength'].default_value = emission
    return m


def finish(o, name, material):
    o.name = PREFIX + '_' + name
    o.data.materials.append(material)
    return o


def box(name, loc, size, material, bevel=.012):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    o = bpy.context.object
    o.dimensions = size
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    finish(o, name, material)
    if bevel:
        mod = o.modifiers.new('Edge highlights', 'BEVEL')
        mod.width = bevel
        mod.segments = 1
        bpy.ops.object.modifier_apply(modifier=mod.name)
    return o


def sphere(name, loc, size, material, segments=16, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, radius=1, location=loc)
    o = bpy.context.object
    o.scale = size
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    return finish(o, name, material)


def cylinder(name, loc, radius, depth, material, rtop=None):
    bpy.ops.mesh.primitive_cone_add(vertices=16, radius1=radius,
        radius2=radius if rtop is None else rtop, depth=depth, location=loc)
    return finish(bpy.context.object, name, material)


def beam(name, a, b, width, material):
    a, b = Vector(a), Vector(b)
    o = box(name, (a+b)*.5, (width, width, (b-a).length), material)
    o.rotation_euler = (b-a).to_track_quat('Z', 'Y').to_euler()
    return o


def joined(name, parts):
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:
        o.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    o = bpy.context.object
    o.name = name
    bpy.context.scene.cursor.location = (0, 0, 0)
    bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    o.data.name = name + '_Mesh'
    return o


def build_kit():
    global kit, assets, M
    kit = scene(PREFIX + '_KIT')
    M = {
        'wood': mat('Wood', (.19, .115, .066)),
        'trim': mat('DarkTimber', (.071, .048, .03)),
        'metal': mat('OxidizedSteel', (.095, .13, .12), .56, .55),
        'straw': mat('Straw', (.42, .29, .10)),
        'egg': mat('Eggshell', (.83, .74, .48), .52),
        'floor': mat('Concrete', (.17, .185, .175)),
        'sack': mat('Burlap', (.34, .265, .16)),
        'glow': mat('Bulb', (1, .48, .12), .32, emission=4),
        'paint': mat('FadedTeal', (.055, .19, .17)),
        'red': mat('SignalRed', (.36, .045, .021)),
    }
    assets = {}
    p = [box('wall_core', (0,0,2), (5,.22,4), M['wood'])]
    for x in [-2.4+i*.2 for i in range(25)]:
        p.append(box('board', (x,-.126,2), (.18,.07,3.98), M['wood'], .008))
    for x in (-2.4, 0, 2.4):
        p.append(box('stud', (x,0,2), (.2,.4,4), M['trim']))
    for z in (.13, 2.0, 3.87):
        p.append(box('rail', (0,0,z), (5,.4,.26), M['trim']))
    assets['wall'] = joined('SM_Henhouse_Wall_500_A', p)
    assets['floor'] = joined('SM_Henhouse_Floor_500_A', [box('floor',(0,0,-.1),(5,5,.2),M['floor'],0)])
    p = []
    for z in (.35, .97, 1.54):
        p.append(box('shelf',(0,0,z),(3,1,.12),M['wood']))
    p.append(box('back',(0,.45,.96),(3,.1,1.16),M['wood']))
    for x in (-1.45,-.5,.5,1.45):
        p.append(box('divider',(x,0,.96),(.1,1,1.28),M['wood']))
    for x in (-1.4,1.4):
        for y in (-.4,.4):
            p.append(box('leg',(x,y,.175),(.15,.15,.35),M['wood']))
    for z in (.45, 1.07):
        p.append(box('lip',(0,-.44,z),(3,.12,.18),M['wood']))
        for x in (-1,0,1):
            p.append(sphere('nest',(x,-.02,z-.01),(.39,.31,.07),M['straw']))
    assets['nest'] = joined('SM_Henhouse_Nest_ThreeBay_A',p)
    p = [box('crate',(0,0,.425),(1.1,.8,.85),M['wood'])]
    for x in (-.48,.48):
        p.append(box('strap',(x,0,.425),(.09,.82,.86),M['metal'],.005))
    for z in (.2,.45,.7):
        p.append(box('boardline',(0,-.405,z),(1.08,.015,.025),M['trim'],0))
    # Crate uses two slots: board grooves share the timber material.
    for o in p:
        if o.data.materials[0] == M['trim']:
            o.data.materials[0] = M['wood']
    assets['crate'] = joined('SM_Henhouse_Crate_A',p)
    p = [sphere('sack',(0,0,.41),(.3,.225,.41),M['sack']),
         cylinder('tie',(0,0,.81),.065,.12,M['wood'],.045)]
    assets['sack'] = joined('SM_Henhouse_FeedSack_A',p)
    o = sphere('egg',(0,0,0),(.15,.15,.20),M['egg'],20,12)
    for v in o.data.vertices:
        f = 1 - .16*(v.co.z/.20)
        v.co.x *= f
        v.co.y *= f
    assets['egg'] = joined('SM_Egg_Raw_A',[o])
    p = [cylinder('shade',(0,0,0),.25,.18,M['metal'],.085),
         cylinder('stem',(0,0,.14),.04,.1,M['metal']),
         sphere('bulb',(0,0,-.11),(.075,.075,.075),M['glow'])]
    assets['lamp'] = joined('SM_Henhouse_WorkLamp_A',p)
    p = [box('base',(0,0,.18),(2.4,.65,.12),M['metal']),
         box('feed',(0,0,.28),(2.25,.48,.08),M['straw'])]
    for y in (-.29,.29):
        p.append(box('side',(0,y,.32),(2.4,.07,.28),M['metal']))
    for x in (-1.16,1.16):
        p.append(box('end',(x,0,.32),(.08,.65,.28),M['metal']))
    for x in (-1,1):
        for y in (-.23,.23):
            p.append(box('foot',(x,y,.06),(.12,.12,.12),M['metal'],0))
    assets['trough'] = joined('SM_Henhouse_FeedTrough_A',p)
    print('KIT READY:', list(assets))


def instance(key, loc, yaw=0):
    src=assets[key]
    o=src.copy()
    o.data=src.data
    bpy.context.scene.collection.objects.link(o)
    o.location=loc
    o.rotation_euler=(0,0,math.radians(yaw))
    return o


def light(name, loc, color, power, size=1, target=None, kind='AREA'):
    d=bpy.data.lights.new(PREFIX+'_'+name,kind)
    d.energy=power
    d.color=color
    if kind=='AREA': d.shape='DISK'; d.size=size
    if kind=='SPOT': d.spot_size=math.radians(65); d.spot_blend=.65; d.shadow_soft_size=.05
    o=bpy.data.objects.new(d.name,d)
    bpy.context.scene.collection.objects.link(o)
    o.location=loc
    if target: o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
    return o


def label(text,loc,size=.22,material=None):
    d=bpy.data.curves.new(PREFIX+'_Sign','FONT')
    d.body=text; d.size=size; d.align_x='CENTER'; d.extrude=.001
    o=bpy.data.objects.new(PREFIX+'_Sign',d)
    bpy.context.scene.collection.objects.link(o)
    o.location=loc; o.rotation_euler=(math.pi/2,0,0)
    d.materials.append(material or M['egg'])
    return o


def camera(name,loc,target,lens=24):
    d=bpy.data.cameras.new(PREFIX+'_'+name)
    o=bpy.data.objects.new(d.name,d)
    bpy.context.scene.collection.objects.link(o)
    o.location=loc; o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
    d.lens=lens; d.clip_end=200
    return o


def build_stage():
    global stage
    stage=scene(PREFIX+'_STAGE')
    for x in (-2.5,2.5):
        for y in (2.5,7.5,12.5): instance('floor',(x,y,0))
    for y in (2.5,7.5,12.5):
        instance('wall',(-5,y,0),90)
        instance('wall',(5,y,0),-90)
    for x in (-2.5,2.5): instance('wall',(x,15,0))
    box('yard',(0,-4,-.17),(15,8,.25),M['floor'])
    box('roof',(0,7.5,4.18),(10.4,15.4,.18),M['trim'])
    for y in (0,5,10,15):
        box('crossbeam',(0,y,3.9),(10,.26,.25),M['trim'])
        for x in (-4.78,4.78):
            box('post',(x,y,2),(.24,.26,4),M['trim'])
            beam('brace',(x,y,2.9),(x-math.copysign(.9,x),y,3.85),.16,M['trim'])
    # Rear nesting bank, then sideways nesting rows along each side.
    for x in (-3.2,0,3.2):
        instance('nest',(x,14.2,0))
        for dx in (-1,0,1):
            for z in (.64,1.25): instance('egg',(x+dx,14.02,z))
    for y in (6.4,10):
        instance('nest',(-4.15,y,0),90)
        instance('nest',(4.15,y,0),-90)
        instance('trough',(-2.9,y,.0),90)
        instance('trough',(2.9,y,.0),90)
    for x,y,z,yaw in [(-3.8,1.3,0,8),(-3.5,2.25,0,-4),(-3.8,1.3,.85,-3),(3.7,2,0,-8)]:
        instance('crate',(x,y,z),yaw)
    for i in range(7):
        instance('sack',(3.7+(i%2)*.55,3.1+(i//2)*.5,0),rng.randrange(360))
    # Feed workbench: a coherent secondary landmark by the entrance.
    box('bench_top',(-3.7,3.6,1.02),(2,.85,.12),M['wood'])
    for x in (-4.5,-2.9):
        for y in (3.3,3.9): box('bench_leg',(x,y,.48),(.12,.12,.96),M['metal'])
    for x in (-4,-3.6,-3.25): instance('egg',(x,3.6,1.28))
    # Hazard stripe edge guides leave the central three metres clear.
    for x in (-1.6,1.6):
        for y in range(1,15): box('floor_mark',(x,y,.008),(.08,.48,.008),M['straw'],0)
    for y in (3,8,13):
        beam('cable',(0,y,4),(0,y,3.42),.018,M['metal'])
        instance('lamp',(0,y,3.4))
        light('WorkLamp',(0,y,3.22),(1,.60,.28),350,1.4,(0,y,.0))
    light('ColdEntrance',(-1,-2,3),(.29,.5,1),650,5,(0,6,1))
    light('RearFill',(4,13,2.9),(.27,.47,.6),130,2,(-2,10,1))
    box('entry_sign',(0,0,3.15),(3.1,.12,.55),M['paint'])
    label('HENHOUSE  /  01',(0,-.071,3.02),.25)
    for x,t in [(-3.2,'A'),(0,'B'),(3.2,'C')]:
        box('bay_sign',(x,14.8,2.2),(.55,.10,.48),M['paint'])
        label(t,(x,14.735,2.03),.34)
    label('FEED  /  KEEP DRY',(-3.7,4.58,2.2),.19)
    box('feed_placard',(-3.7,4.64,2.28),(2.3,.09,.38),M['paint'])
    stage.world=bpy.data.worlds.new(PREFIX+'_Night')
    stage.world.use_nodes=True
    next(n for n in stage.world.node_tree.nodes if n.type == 'BACKGROUND').inputs[0].default_value=(.06,.09,.15,1)
    next(n for n in stage.world.node_tree.nodes if n.type == 'BACKGROUND').inputs[1].default_value=.18
    stage.camera=camera('Eye156',(1.05,-1.5,1.56),(0,8.5,1.65),22)
    camera('Overview',(11,-9,9),(0,7,1),33)
    camera('NestDetail',(-1,10.3,1.56),(0,14.1,1),36)
    stage.render.engine='CYCLES'
    stage.cycles.samples=32
    stage.cycles.use_denoising=True
    stage.render.resolution_x=1280; stage.render.resolution_y=800; stage.render.resolution_percentage=100
    stage.view_settings.view_transform='AgX'
    stage.view_settings.exposure=.4
    stage.render.image_settings.file_format='PNG'
    for area in bpy.context.screen.areas:
        if area.type=='VIEW_3D':
            area.spaces.active.region_3d.view_perspective='CAMERA'
            area.spaces.active.shading.type='MATERIAL'
            area.spaces.active.shading.use_scene_lights=True
            area.spaces.active.shading.use_scene_world=True
    configure_preview('night')
    print('STAGE READY:',len(stage.objects),'objects')


def configure_preview(mode='night'):
    """Presentation lights only: these values are not UE gameplay light settings."""
    for o in stage.objects:
        if o.type!='LIGHT':
            continue
        if 'WorkLamp' in o.name:
            o.data.energy=(90 if o.location.y<5 else (0 if o.location.y<10 else 65)) if mode=='night' else 350
            o.data.size=.55 if mode=='night' else 1.4
        if 'ColdEntrance' in o.name: o.data.energy=95 if mode=='night' else 650
        if 'RearFill' in o.name: o.data.energy=22 if mode=='night' else 130
    stage.view_settings.exposure=-.65 if mode=='night' else .4
    stage.cycles.samples=16
    stage.render.resolution_x=960
    stage.render.resolution_y=600


def export_kit():
    bpy.context.window.scene=kit
    report={}
    for key,o in assets.items():
        bpy.ops.object.select_all(action='DESELECT')
        o.select_set(True); bpy.context.view_layer.objects.active=o
        o.data.calc_loop_triangles()
        report[key]={'object':o.name,'dimensions_cm':[round(v*100,2) for v in o.dimensions],
                     'triangles':len(o.data.loop_triangles),'materials':[m.name for m in o.data.materials],
                     'uv_layers':len(o.data.uv_layers)}
        bpy.ops.export_scene.fbx(filepath=str(ROOT/'export'/f'{o.name}.fbx'),use_selection=True,
            object_types={'MESH'},global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',
            bake_anim=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    materials={}
    for k,m in M.items():
        bs=next(n for n in m.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
        materials[m.name]={'base_color_linear':list(m.diffuse_color),
            'roughness':bs.inputs['Roughness'].default_value,'metallic':bs.inputs['Metallic'].default_value,
            'emission_strength':bs.inputs['Emission Strength'].default_value}
    (ROOT/'export'/'manifest.json').write_text(json.dumps({'assets':report,'materials':materials,
        'unit':'m','export_axis':'X forward Z up','ue_import_verified':False,
        'textures':'None: constant material values; reconstruct in UE.'},ensure_ascii=False,indent=2),encoding='utf-8')
    bpy.context.window.scene=stage
    print(json.dumps(report))


def save_sample():
    bpy.context.window.scene=stage
    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'henhouse_sample_01.blend'))
    print('SAVED',str(ROOT/'henhouse_sample_01.blend'))


def render_preview(name='night'):
    stage.render.filepath=str(ROOT/'preview'/f'{name}.png')
    bpy.ops.render.render(write_still=True)


def render_flashlight_detail():
    old_camera=stage.camera
    energies={o:o.data.energy for o in stage.objects if o.type=='LIGHT'}
    detail=bpy.data.objects[PREFIX+'_NestDetail']
    flash=None
    try:
        stage.camera=detail
        for o in energies: o.data.energy=0
        flash=light('InspectionFlashlight',detail.location,(.82,.9,1),300,
                    target=(0,14.1,1),kind='SPOT')
        render_preview('flashlight_detail')
    finally:
        if flash: bpy.data.objects.remove(flash,do_unlink=True)
        for o,e in energies.items(): o.data.energy=e
        stage.camera=old_camera
