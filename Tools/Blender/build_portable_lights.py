"""Original industrial flashlight and open-bottom helmet lamp; Blender MCP entry."""
import bpy,bmesh,math,json,hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/portable_lights')
assert 'CL9_PORTABLE_LIGHTS' not in bpy.data.scenes,'Inspect existing source'
for f in ('export','preview'):(ROOT/f).mkdir(parents=True,exist_ok=True)
s=bpy.data.scenes.new('CL9_PORTABLE_LIGHTS');bpy.context.window.scene=s
s.unit_settings.system='METRIC';s.unit_settings.scale_length=1
export=bpy.data.collections.new('PORTABLE_LIGHTS_EXPORT');s.collection.children.link(export)
yellow=(.54,.285,.025,1);dark=(.018,.024,.027,1);rubber=(.035,.043,.044,1)
metal=(.27,.30,.31,1);stripe=(.72,.63,.36,1);red=(.32,.035,.015,1)
bodymat=bpy.data.materials.new('M_CL9_PortableLight_Body');bodymat.use_nodes=True
bs=next(n for n in bodymat.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
vc=bodymat.node_tree.nodes.new('ShaderNodeVertexColor');vc.layer_name='Color'
bodymat.node_tree.links.new(vc.outputs['Color'],bs.inputs['Base Color']);bs.inputs['Roughness'].default_value=.72;bs.inputs['Metallic'].default_value=.12
lensmat=bpy.data.materials.new('M_CL9_PortableLight_Lens');lensmat.use_nodes=True
bs=next(n for n in lensmat.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
bs.inputs['Base Color'].default_value=(.48,.66,.67,1);bs.inputs['Roughness'].default_value=.21;bs.inputs['Metallic'].default_value=.35
parts=[]
def mesh(label,verts,faces,color,lens=False):
    data=bpy.data.meshes.new(label+'_Mesh');data.from_pydata(verts,[],faces);data.update()
    o=bpy.data.objects.new(label,data);export.objects.link(o);data.materials.append(lensmat if lens else bodymat)
    a=data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for v in a.data:v.color=color
    parts.append(o);return o
def profile(label,sections,color,center=(0,0,0),segments=20,lens=False):
    vs=[(x+center[0],r*math.cos(i*math.tau/segments)+center[1],r*math.sin(i*math.tau/segments)+center[2]) for x,r in sections for i in range(segments)]
    faces=[]
    for k in range(len(sections)-1):
        for i in range(segments):j=(i+1)%segments;faces.append((k*segments+i,k*segments+j,(k+1)*segments+j,(k+1)*segments+i))
    faces.extend([tuple(range(segments-1,-1,-1)),tuple((len(sections)-1)*segments+i for i in range(segments))])
    return mesh(label,vs,faces,color,lens)
def box(label,center,size,color,bevel=.002):
    bpy.ops.mesh.primitive_cube_add(size=1,location=center);o=bpy.context.object;o.name=label;o.scale=size
    for c in list(o.users_collection):c.objects.unlink(o)
    export.objects.link(o)
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        mod=o.modifiers.new('Edge bevel','BEVEL');mod.width=bevel;mod.segments=1
        bpy.ops.object.modifier_apply(modifier=mod.name)
    o.data.materials.append(bodymat);a=o.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for v in a.data:v.color=color
    parts.append(o);return o
def band(label,cx,rx,ry,zlo,zhi,thickness,color,n=32):
    # Closed rectangular section elliptical ring, with an actual empty center.
    vs=[]
    for shrink,z in [(0,zlo),(0,zhi),(thickness,zhi),(thickness,zlo)]:
        vs.extend((cx+(rx-shrink)*math.cos(i*math.tau/n),(ry-shrink)*math.sin(i*math.tau/n),z) for i in range(n))
    fs=[(k*n+i,k*n+(i+1)%n,((k+1)%4)*n+(i+1)%n,((k+1)%4)*n+i) for k in range(4) for i in range(n)]
    return mesh(label,vs,fs,color)
manifest={'source':'Original CL9 geometry through Blender MCP','source_blend':'portable_lights.blend','pivot':'bounds center',
          'axis':'+X lens / +Z up','rig':'N/A static objects; light toggle by UE','assets':{},
          'materials':{'M_CL9_PortableLight_Body':{'vertex_color':True,'roughness':.72,'metallic':.12},
                       'M_CL9_PortableLight_Lens':{'vertex_color':False,'base_color':[.48,.66,.67,1],'roughness':.21,'metallic':.35,'emission':0}}}
def finish(key,name,lens_position):
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    o=parts[0];bpy.context.view_layer.objects.active=o;bpy.ops.object.join();o.name=name
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    lo=Vector(tuple(min(v.co[i] for v in o.data.vertices) for i in range(3)))
    hi=Vector(tuple(max(v.co[i] for v in o.data.vertices) for i in range(3)));mid=(lo+hi)/2
    for v in o.data.vertices:v.co-=mid
    bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
    assert all(e.is_manifold for e in bm.edges),'Open surface in '+key
    bm.to_mesh(o.data);bm.free()
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.025);bpy.ops.object.mode_set(mode='OBJECT')
    bpy.context.view_layer.update();o.data.calc_loop_triangles()
    f=ROOT/'export'/(name+'.fbx')
    bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',
        global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
    manifest['assets'][key]={'object':name,'dimensions_cm':[v*100 for v in o.dimensions],
        'triangles':len(o.data.loop_triangles),'vertices':len(o.data.vertices),'uv_layers':len(o.data.uv_layers),
        'materials':[m.name for m in o.data.materials],'lens_center_cm':list((Vector(lens_position)-mid)*100),
        'fbx_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'closed_surfaces':True}
    parts.clear()
# Flashlight: broad head, scalloped rubber grip, two protective rings and top thumb switch.
profile('FlashlightGrip',[(-.125,.028),(-.113,.032),(.020,.032),(.032,.028)],rubber)
for i in range(5):
    x=-.10+i*.022;profile('GripRib',[ (x,.032),(x+.005,.035),(x+.011,.035),(x+.013,.032)],dark,segments=16)
profile('RearCap',[(-.135,.028),(-.131,.035),(-.116,.035),(-.111,.031)],yellow)
profile('FlashlightHead',[(.018,.030),(.047,.049),(.095,.049),(.112,.042)],yellow)
profile('LensGuard',[(.095,.047),(.103,.052),(.122,.052),(.126,.046)],dark)
# Reflector ring has no front cap across the lens; cap is behind the lens plane.
profile('Reflector',[(.126,.043),(.127,.039)],metal)
profile('FlashlightLens',[(.127,.037),(.129,.037)],(1,1,1,1),lens=True)
box('ThumbSwitchMount',(-.023,0,.033),(.042,.027,.012),yellow)
box('ThumbSwitch',(-.023,0,.042),(.022,.018,.009),red,.003)
box('SideIDPlate',(-.060,-.033,.004),(.048,.002,.016),stripe,.001)
finish('flashlight','SM_Flashlight_A',(.129,0,0))
# Headlamp: hollow work helmet with brim, dark band, top ridge and a frontal lamp.
n=32;rings=10;vs=[]
for inside in (False,True):
    for j in range(rings):
        a=j/(rings-1)*math.pi/2*.98
        rx=.112-(.004 if inside else 0);ry=.102-(.004 if inside else 0);rz=.096-(.004 if inside else 0)
        vs.extend((-.02+rx*math.cos(a)*math.cos(i*math.tau/n),ry*math.cos(a)*math.sin(i*math.tau/n),rz*math.sin(a)) for i in range(n))
fs=[]
for layer in range(2):
    base=layer*rings*n
    for j in range(rings-1):
        for i in range(n):fs.append((base+j*n+i,base+j*n+(i+1)%n,base+(j+1)*n+(i+1)%n,base+(j+1)*n+i))
    fs.append(tuple(base+(rings-1)*n+i for i in range(n)))
for i in range(n):fs.append((i,(i+1)%n,rings*n+(i+1)%n,rings*n+i))
mesh('HelmetShell',vs,fs,yellow)
band('HelmetBrim',-.02,.132,.122,-.008,.002,.024,yellow)
band('HelmetBand',-.02,.114,.104,.004,.023,.006,dark)
box('HelmetRidge',(-.028,0,.094),(.139,.018,.013),yellow,.004)
for side in (-1,1):
    box('HelmetSideReflector',(-.015,side*.104,.031),(.067,.003,.012),stripe,.001)
    box('LampBracket',(.080,side*.036,.022),(.047,.014,.045),dark,.003)
profile('HelmetLamp',[(.068,.032),(.093,.040),(.135,.040)],yellow,center=(0,0,.025))
profile('HelmetLampRim',[(.124,.041),(.143,.042),(.150,.037)],dark,center=(0,0,.025))
profile('HelmetReflector',[(.150,.035),(.151,.032)],metal,center=(0,0,.025))
profile('HelmetLens',[(.151,.030),(.153,.030)],(1,1,1,1),center=(0,0,.025),lens=True)
box('HelmetSwitch',(.10,0,.064),(.022,.025,.010),red,.002)
finish('headlamp','SM_Headlamp_A',(.153,0,.025))
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'portable_lights.blend'),{s},fake_user=True)
print(json.dumps(manifest))
