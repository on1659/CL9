"""Original hollow pan and enamel pot. Execute only through Blender MCP."""
import bpy,bmesh,math,json,hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/cookware')
assert 'CL9_COOKWARE' not in bpy.data.scenes,'Inspect existing cookware source'
for f in ('export','preview'):(ROOT/f).mkdir(parents=True,exist_ok=True)
s=bpy.data.scenes.new('CL9_COOKWARE');bpy.context.window.scene=s
s.unit_settings.system='METRIC';s.unit_settings.scale_length=1
collection=bpy.data.collections.new('COOKWARE_EXPORT');s.collection.children.link(collection)
specs={'M_CL9_Cookware_Surface':{'vertex_color':True,'roughness':.60,'metallic':.25},
       'M_CL9_Cookware_Grip':{'vertex_color':True,'roughness':.84,'metallic':.02}}
mats={}
for name,meta in specs.items():
    mat=bpy.data.materials.new(name);mat.use_nodes=True;bs=next(n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    vc=mat.node_tree.nodes.new('ShaderNodeVertexColor');vc.layer_name='Color'
    mat.node_tree.links.new(vc.outputs['Color'],bs.inputs['Base Color'])
    bs.inputs['Roughness'].default_value=meta['roughness'];bs.inputs['Metallic'].default_value=meta['metallic'];mats[name]=mat
iron=(.029,.038,.041,1);inside=(.045,.053,.053,1);edge=(.14,.16,.17,1)
grip=(.027,.024,.018,1);cream=(.68,.65,.47,1);red=(.22,.038,.022,1)
parts=[]
def mesh(name,vs,fs,color,slot='M_CL9_Cookware_Surface',facecolors=None):
    data=bpy.data.meshes.new(name+'_Mesh');data.from_pydata(vs,[],fs);data.update()
    ob=bpy.data.objects.new(name,data);collection.objects.link(ob);data.materials.append(mats[slot])
    a=data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for p in data.polygons:
        c=facecolors[p.index] if facecolors else color
        for i in p.loop_indices:a.data[i].color=c
    parts.append(ob);return ob
def vessel(name,profile,colors,n=40):
    vs=[(r*math.cos(i*math.tau/n),r*math.sin(i*math.tau/n),z) for r,z in profile for i in range(n)]
    fs=[];cs=[]
    for j in range(len(profile)-1):
        for i in range(n):fs.append((j*n+i,j*n+(i+1)%n,(j+1)*n+(i+1)%n,(j+1)*n+i));cs.append(colors[j])
    fs.extend([tuple(range(n-1,-1,-1)),tuple((len(profile)-1)*n+i for i in range(n))]);cs.extend((colors[0],colors[-1]))
    return mesh(name,vs,fs,colors[0],facecolors=cs)
def box(name,center,size,color,slot='M_CL9_Cookware_Surface',bevel=.003):
    bpy.ops.mesh.primitive_cube_add(size=1,location=center);o=bpy.context.object;o.name=name;o.scale=size
    for c in list(o.users_collection):c.objects.unlink(o)
    collection.objects.link(o);bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        mod=o.modifiers.new('Edge bevel','BEVEL');mod.width=bevel;mod.segments=1;bpy.ops.object.modifier_apply(modifier=mod.name)
    o.data.materials.append(mats[slot]);a=o.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for v in a.data:v.color=color
    parts.append(o);return o
def ring_xy(name,cx,cy,rx,ry,z,thick,depth,color,slot='M_CL9_Cookware_Surface',n=24):
    vs=[]
    for shrink,zz in [(0,z-depth/2),(0,z+depth/2),(thick,z+depth/2),(thick,z-depth/2)]:
        vs.extend((cx+(rx-shrink)*math.cos(i*math.tau/n),cy+(ry-shrink)*math.sin(i*math.tau/n),zz) for i in range(n))
    fs=[(j*n+i,j*n+(i+1)%n,((j+1)%4)*n+(i+1)%n,((j+1)%4)*n+i) for j in range(4) for i in range(n)]
    return mesh(name,vs,fs,color,slot)
manifest={'source':'Original CL9 geometry through Blender MCP','source_blend':'cookware.blend','axis':'+X forward, opening +Z',
          'pivot':'bounds center','rig':'N/A static carried props','materials':specs,'assets':{}}
def finish(key,name,runtime_radius,grip_point):
    bpy.ops.object.select_all(action='DESELECT')
    for ob in parts:ob.select_set(True)
    ob=parts[0];bpy.context.view_layer.objects.active=ob;bpy.ops.object.join();ob.name=name
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    lo=Vector(tuple(min(v.co[i] for v in ob.data.vertices) for i in range(3)));hi=Vector(tuple(max(v.co[i] for v in ob.data.vertices) for i in range(3)));mid=(lo+hi)/2
    for v in ob.data.vertices:v.co-=mid
    bm=bmesh.new();bm.from_mesh(ob.data);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));assert all(e.is_manifold for e in bm.edges)
    bm.to_mesh(ob.data);bm.free()
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.025);bpy.ops.object.mode_set(mode='OBJECT')
    bpy.context.view_layer.update();ob.data.calc_loop_triangles()
    f=ROOT/'export'/(name+'.fbx');bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
    manifest['assets'][key]={'object':name,'dimensions_cm':[v*100 for v in ob.dimensions],
        'triangles':len(ob.data.loop_triangles),'vertices':len(ob.data.vertices),'uv_layers':len(ob.data.uv_layers),
        'materials':[mat.name for mat in ob.data.materials],'runtime_radius_cm':runtime_radius,
        'grip_reference_cm':list((Vector(grip_point)-mid)*100),'closed_surfaces':True,'fbx_sha256':hashlib.sha256(f.read_bytes()).hexdigest()}
    parts.clear()
# Cross section runs from the underside, up the outside, across the rim and down the inside.
vessel('PanBowl',[(.045,-.038),(.140,-.038),(.150,-.033),(.172,.016),(.175,.023),(.173,.029),(.165,.030),(.160,.020),(.140,-.024),(.045,-.024)],
        [iron,iron,iron,iron,edge,edge,edge,inside,inside])
box('PanTang',(-.183,0,.020),(.082,.029,.017),edge)
box('PanHandle',(-.270,0,.029),(.193,.043,.025),grip,'M_CL9_Cookware_Grip',.007)
ring_xy('PanHandleEye',-.385,0,.042,.030,.029,.014,.025,grip,'M_CL9_Cookware_Grip')
box('HandleMetalCollar',(-.202,0,.029),(.015,.047,.027),edge,bevel=.002)
finish('frying_pan','SM_FryingPan_A',30,(-.288,0,.029))
vessel('PotBowl',[(.05,-.130),(.143,-.130),(.164,-.117),(.172,-.093),(.177,-.070),(.177,-.052),(.179,.101),(.184,.115),(.183,.126),(.175,.130),(.170,.124),(.158,-.105),(.145,-.114),(.05,-.114)],
        [iron,iron,red,red,red,cream,cream,edge,edge,edge,cream,cream,cream])
for side in (-1,1):
    # Horizontal open loop handles, with short mounting ears outside the vessel wall.
    for x in (-.062,.062):box('PotHandleEar',(x,side*.184,.071),(.027,.054,.030),edge,bevel=.004)
    ring_xy('PotLoopHandle',0,side*.210,.097,.063,.071,.020,.023,grip,'M_CL9_Cookware_Grip',32)
finish('pot','SM_Pot_A',None,(0,-.250,.071))
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'cookware.blend'),{s},fake_user=True)
print(json.dumps(manifest))
