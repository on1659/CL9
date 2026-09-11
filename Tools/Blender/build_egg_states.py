"""Create original CL9 egg state meshes alongside preserved henhouse scenes, through MCP."""
import bpy, math, json
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/egg_states')
for folder in ('export','preview'): (ROOT/folder).mkdir(parents=True,exist_ok=True)
assert 'CL9_EGG01_KIT' not in bpy.data.scenes, 'Inspect existing egg scene before rebuilding'
raw=bpy.data.objects['SM_Egg_Raw_A']
kit=bpy.data.scenes.new('CL9_EGG01_KIT')
kit.unit_settings.system='METRIC';kit.unit_settings.scale_length=1;kit.render.fps=30
bpy.context.window.scene=kit
kit.collection.objects.link(raw)
assets={'raw':raw}

def material(name,color,rough):
    m=bpy.data.materials.new('CL9_EGG01_'+name);m.diffuse_color=(*color,1);m.use_nodes=True
    bs=next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    bs.inputs['Base Color'].default_value=(*color,1);bs.inputs['Roughness'].default_value=rough
    return m
white=material('CookedWhite',(.91,.86,.67),.48)
yolk=material('Yolk',(.92,.39,.025),.39)
char=material('Charcoal',(.019,.014,.012),.98)
toast=material('BurntFissure',(.14,.054,.012),.9)
shell=material('Shell',(.83,.74,.48),.62)
inside=material('FractureInterior',(.25,.065,.009),.65)

def mesh(name,verts,faces,mats,indices=None):
    data=bpy.data.meshes.new(name+'_Mesh');data.from_pydata(verts,[],faces);data.update()
    ob=bpy.data.objects.new(name,data);kit.collection.objects.link(ob)
    for mat in mats:data.materials.append(mat)
    if indices:
        for p,i in zip(data.polygons,indices):p.material_index=i
    return ob

# Cut a generous flat face in the front of a whole peeled egg. The retained back shell
# preserves top/bottom limits; the circular yolk sits on the actual cut face, not a decal.
import bmesh
cooked=raw.copy();cooked.data=raw.data.copy();cooked.name='SM_Egg_Cooked_A';kit.collection.objects.link(cooked)
cooked.data.materials.clear();cooked.data.materials.append(white);cooked.data.materials.append(yolk)
bm=bmesh.new();bm.from_mesh(cooked.data)
cut=bmesh.ops.bisect_plane(bm,geom=list(bm.verts)+list(bm.edges)+list(bm.faces),dist=.00001,
                         plane_co=(.095,0,0),plane_no=(1,0,0),clear_outer=True,clear_inner=False)
edges=[e for e in cut['geom_cut'] if isinstance(e,bmesh.types.BMEdge) and e.is_boundary]
bmesh.ops.holes_fill(bm,edges=edges,sides=0)
bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(cooked.data);bm.free()
verts=[(.096,0,-.022)]+[(.096,.064*math.cos(i*math.tau/24),-.022+.081*math.sin(i*math.tau/24)) for i in range(24)]
disk=mesh('CL9_EGG01_YolkCut',verts,[(0,1+i,1+(i+1)%24) for i in range(24)],[yolk])

def join(target,parts):
    bpy.ops.object.select_all(action='DESELECT')
    for o in [target]+parts:o.select_set(True)
    bpy.context.view_layer.objects.active=target;bpy.ops.object.join()
    return target
join(cooked,[disk]);assets['cooked']=cooked

# Cracks are shallow ribbons following the existing egg surface with broad readable forks.
def surface(z,angle):
    # Same outline as original sample: taper toward the upper pole.
    r=.16*math.sqrt(max(0,1-(z/.2)**2))*(1-.22*z/.2)
    return Vector((r*math.cos(angle),r*math.sin(angle),z))
def ribbon(name,points,width,mat):
    vs=[]
    for z,a in points:
        for side in (-1,1):
            v=surface(z,a+side*width/2);v.x*=1.012;v.y*=1.012;vs.append(v)
    return mesh(name,vs,[(2*i,2*i+1,2*i+3,2*i+2) for i in range(len(points)-1)],[mat])

def clone(name,mat):
    o=raw.copy();o.data=raw.data.copy();o.name=name;kit.collection.objects.link(o)
    o.data.materials.clear();o.data.materials.append(mat);return o
cracked=clone('SM_Egg_Cracked_A',shell)
paths=[[(.155,.05),(.105,-.10),(.055,.06),(.005,-.10),(-.05,.09),(-.12,-.12)],
       [(.055,.06),(.035,.36),(.07,.62),(.035,.95)],
       [(-.05,.09),(-.07,-.38),(-.04,-.62)],
       [(.13,2.8),(.07,2.65),(.015,2.82),(-.06,2.62)]]
join(cracked,[ribbon('CL9_EGG01_Crack',p,.065,inside) for p in paths]);assets['cracked']=cracked
burnt=clone('SM_Egg_Burnt_A',char)
join(burnt,[ribbon('CL9_EGG01_BurntCrack',[(z,a+k*math.tau/5) for z,a in paths[0]],.085,toast) for k in range(5)])
assets['burnt']=burnt

# Open a jagged front region. Solidify gives visible shell thickness; closed back and
# base remain as a carryable ruined ingredient, not disconnected particle debris.
broken=clone('SM_Egg_Broken_A',shell)
bm=bmesh.new();bm.from_mesh(broken.data)
remove=[]
for f in bm.faces:
    c=f.calc_center_median()
    if c.x>.025 and -.11<c.z<(.13 if c.y>0 else .09): remove.append(f)
bmesh.ops.delete(bm,geom=remove,context='FACES');bm.to_mesh(broken.data);bm.free()
broken.data.materials.append(inside)
bpy.context.view_layer.objects.active=broken
solid=broken.modifiers.new('Shell thickness','SOLIDIFY');solid.thickness=.007;solid.offset=-1;solid.material_offset=1;solid.material_offset_rim=1
bpy.ops.object.modifier_apply(modifier=solid.name)
bpy.ops.mesh.primitive_uv_sphere_add(segments=20,ring_count=12,radius=1,location=(.018,0,-.055))
contents=bpy.context.object;contents.name='CL9_EGG01_BrokenContents';contents.scale=(.099,.105,.11)
bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
contents.data.materials.append(yolk)
join(broken,[contents]);assets['broken']=broken

# Unique UV unwrap for each exported mesh. Raw is preserved, including its existing UV.
for key,o in assets.items():
    if key=='raw':continue
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(island_margin=.025);bpy.ops.object.mode_set(mode='OBJECT')
    for p in o.data.polygons:p.use_smooth=False

manifest={'source':'Original CL9 procedural geometry via Blender MCP; raw reused from henhouse sample',
          'source_blend':'egg_states.blend','fps':30,'rig':'N/A static state swaps','assets':{}}
for key,o in assets.items():
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
    bpy.ops.export_scene.fbx(filepath=str(ROOT/'export'/f'{o.name}.fbx'),use_selection=True,object_types={'MESH'},
                            global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',
                            bake_anim=False,add_leaf_bones=False,use_mesh_modifiers=True)
    o.data.calc_loop_triangles()
    manifest['assets'][key]={'object':o.name,'dimensions_cm':[v*100 for v in o.dimensions],
                            'triangles':len(o.data.loop_triangles),'uv_layers':len(o.data.uv_layers),
                            'max_vertex_radius_cm':max(v.co.length for v in o.data.vertices)*100,
                            'materials':[]}
    for m in o.data.materials:
        bs=next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
        manifest['assets'][key]['materials'].append({'name':m.name,'base_color':list(bs.inputs['Base Color'].default_value),
            'roughness':bs.inputs['Roughness'].default_value,'metallic':bs.inputs['Metallic'].default_value})
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'egg_states.blend'),{kit},fake_user=True)
print(json.dumps(manifest))
