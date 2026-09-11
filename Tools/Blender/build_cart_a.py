"""Original farm cart with open load bed, coarse rails and four wheels."""
import bpy,bmesh,math,json,hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/cart_a')
assert 'CL9_CART_A' not in bpy.data.scenes,'Inspect saved cart source'
for f in ('export','preview'):(ROOT/f).mkdir(parents=True,exist_ok=True)
s=bpy.data.scenes.new('CL9_CART_A');bpy.context.window.scene=s;s.unit_settings.system='METRIC';s.unit_settings.scale_length=1
ex=bpy.data.collections.new('CART_EXPORT');s.collection.children.link(ex)
ref=bpy.data.collections.new('CART_REF');s.collection.children.link(ref)
materials={'M_CL9_Cart_Frame':{'vertex_color':True,'roughness':.77,'metallic':.18},
           'M_CL9_Cart_Rubber':{'vertex_color':True,'roughness':.90,'metallic':.01}}
mats={}
for name,meta in materials.items():
    m=bpy.data.materials.new(name);m.use_nodes=True;bs=next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    vc=m.node_tree.nodes.new('ShaderNodeVertexColor');vc.layer_name='Color';m.node_tree.links.new(vc.outputs['Color'],bs.inputs['Base Color'])
    bs.inputs['Roughness'].default_value=meta['roughness'];bs.inputs['Metallic'].default_value=meta['metallic'];mats[name]=m
yellow=(.39,.23,.038,1);dark=(.034,.044,.047,1);wood=(.24,.16,.077,1);rubber=(.016,.020,.022,1);metal=(.20,.23,.23,1)
parts=[]
def finishpart(o,color,slot='M_CL9_Cart_Frame'):
    for c in list(o.users_collection):c.objects.unlink(o)
    ex.objects.link(o);o.data.materials.append(mats[slot]);a=o.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for v in a.data:v.color=color
    parts.append(o);return o
def box(name,center,size,color,bevel=.006,slot='M_CL9_Cart_Frame'):
    bpy.ops.mesh.primitive_cube_add(size=1,location=center);o=bpy.context.object;o.name=name;o.scale=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        mod=o.modifiers.new('Edges','BEVEL');mod.width=bevel;mod.segments=1;bpy.ops.object.modifier_apply(modifier=mod.name)
    return finishpart(o,color,slot)
def bar(name,a,b,r,color,slot='M_CL9_Cart_Frame',vertices=12):
    d=Vector(b)-Vector(a);bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=d.length,location=(Vector(a)+Vector(b))/2)
    o=bpy.context.object;o.name=name;o.rotation_euler=d.to_track_quat('Z','Y').to_euler()
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=True);return finishpart(o,color,slot)
def wheel(name,x,y,z):
    profile=[(-.065,.12),(-.058,.165),(-.035,.18),(.035,.18),(.058,.165),(.065,.12)]
    n=24;vs=[(x+r*math.cos(i*math.tau/n),y+yy,z+r*math.sin(i*math.tau/n)) for yy,r in profile for i in range(n)]
    fs=[(j*n+i,j*n+(i+1)%n,(j+1)*n+(i+1)%n,(j+1)*n+i) for j in range(len(profile)-1) for i in range(n)]
    fs.extend([tuple(range(n-1,-1,-1)),tuple((len(profile)-1)*n+i for i in range(n))])
    data=bpy.data.meshes.new(name+'_Mesh');data.from_pydata(vs,[],fs);data.update();o=bpy.data.objects.new(name,data);ex.objects.link(o)
    finishpart(o,rubber,'M_CL9_Cart_Rubber')
    for side in (-1,1):
        bar('WheelHub',(x,y+side*.055,z),(x,y+side*.078,z),.095,yellow,vertices=16)
        bar('AxleCap',(x,y+side*.078,z),(x,y+side*.086,z),.035,metal,vertices=12)
    # A few broad molded ribs read in motion without creating dense tread geometry.
    for i in range(8):
        a=i*math.tau/8
        p=(x+.180*math.cos(a),y,z+.180*math.sin(a))
        o=box('TireTread',p,(.050,.080,.014),rubber,.002,'M_CL9_Cart_Rubber')
        o.rotation_euler.y=math.pi/2-a
        bpy.context.view_layer.objects.active=o;bpy.ops.object.transform_apply(location=False,rotation=True,scale=False)
# Wheels and chassis: bottom at approximately Z0, load deck at Z0.43.
centers=[]
for x in (-.40,.40):
    bar('Axle',(x,-.57,.19),(x,.57,.19),.036,dark)
    for y in (-.49,.49):wheel('CartWheel',x,y,.19);centers.append((x,y,.19))
for y in (-.32,.32):box('ChassisBeam',(0,y,.29),(1.12,.060,.105),dark)
for x in (-.51,0,.51):box('DeckCrossMember',(x,0,.35),(.055,.80,.075),yellow)
for i in range(6):
    col=tuple(v*(.91 if i%2 else 1.04) for v in wood[:3])+(1,)
    box('BedPlank',(0,(i-2.5)*.119,.409),(1.05,.111,.042),col,.004)
for y in (-.405,.405):
    box('BedSideEdge',(0,y,.420),(1.16,.045,.080),yellow)
    for x in (-.555,-.18,.18,.555):box('SideUpright',(x,y,.685),(.036,.036,.51),yellow,.004)
    for z in (.59,.76,.94):box('SideRail',(0,y,z),(1.15,.032,.032),yellow,.004)
for x in (-.555,.555):
    box('BedEndEdge',(x,0,.420),(.04,.81,.080),yellow)
    for y in (-.135,.135):box('EndUpright',(x,y,.685),(.03,.03,.51),yellow,.004)
    for z in (.59,.76,.94):box('EndRail',(x,0,z),(.03,.80,.032),yellow,.004)
# Front angled push/pull handle with rubber cross grip; leaves the cargo opening clear.
for y in (-.28,.28):
    bar('HandleLower',(.54,y,.39),(.84,y,.66),.025,yellow)
    bar('HandleUpper',(.84,y,.66),(.91,y,1.02),.025,yellow)
bar('HandleCross',(.91,-.30,1.02),(.91,.30,1.02),.030,dark)
bar('HandleGrip',(.91,-.21,1.02),(.91,.21,1.02),.037,rubber,'M_CL9_Cart_Rubber')
box('FrontIDPlate',(.579,0,.70),(.009,.20,.12),dark,.003)
box('FrontReflector',(.586,0,.70),(.008,.115,.026),(.60,.46,.20,1),.002)
bpy.ops.object.select_all(action='DESELECT')
for o in parts:o.select_set(True)
o=parts[0];bpy.context.view_layer.objects.active=o;bpy.ops.object.join();o.name='SM_Cart_A'
bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
lo=Vector(tuple(min(v.co[i] for v in o.data.vertices) for i in range(3)));hi=Vector(tuple(max(v.co[i] for v in o.data.vertices) for i in range(3)));mid=(lo+hi)/2
for v in o.data.vertices:v.co-=mid
bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));assert all(e.is_manifold for e in bm.edges);bm.to_mesh(o.data);bm.free()
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
bpy.context.view_layer.update();o.data.calc_loop_triangles()
anchors={'CargoFloor':(0,0,.430),'TowGrip':(.91,0,1.02)}
for i,c in enumerate(centers):anchors['Wheel_'+str(i)]=c
for name,c in anchors.items():
    empty=bpy.data.objects.new('REF_Cart_'+name,None);ref.objects.link(empty);empty.location=Vector(c)-mid;empty.empty_display_size=.08
f=ROOT/'export/SM_Cart_A.fbx'
bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
meta={'object':o.name,'dimensions_cm':[v*100 for v in o.dimensions],'triangles':len(o.data.loop_triangles),'vertices':len(o.data.vertices),
      'uv_layers':len(o.data.uv_layers),'materials':[m.name for m in o.data.materials],'runtime_radius_cm':60,
      'closed_surfaces':True,'fbx_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),
      'anchors_cm':{name:list((Vector(c)-mid)*100) for name,c in anchors.items()},
      'cargo_clear_area_cm':[102,70],'wheel_radius_cm':18.7,'wheel_rotation_axis':'Y',
      'animation':'Current UE consumer is StaticMesh; wheel rotation and steering have no consumer.'}
manifest={'source':'Original CL9 farm cart geometry through Blender MCP','source_blend':'cart_a.blend','pivot':'bounds center',
          'axis':'+X handle / +Z up','materials':materials,'assets':{'cart':meta}}
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'cart_a.blend'),{s},fake_user=True)
print(json.dumps(meta))
