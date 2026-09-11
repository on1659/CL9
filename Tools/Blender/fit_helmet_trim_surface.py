"""Replace long trim faces with sampled geometry fitted to the actual faceted shell."""
import bpy,bmesh,json,hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/portable_lights');s=bpy.data.scenes['CL9_PORTABLE_LIGHTS'];bpy.context.window.scene=s
m=json.loads((ROOT/'export/manifest.json').read_text());assert m['trim_fit_revision']==1
o=bpy.data.objects['SM_Headlamp_A'];meta=m['assets']['headlamp']
mid=Vector((.153,0,.025))-Vector(meta['lens_center_cm'])/100
# Find the three disconnected trim islands by authored raw-space location and bounds.
bm=bmesh.new();bm.from_mesh(o.data);seen=set();remove=[]
for first in list(bm.verts):
    if first in seen:continue
    stack=[first];seen.add(first);group=[]
    while stack:
        v=stack.pop();group.append(v)
        for e in v.link_edges:
            other=e.other_vert(v)
            if other not in seen:seen.add(other);stack.append(other)
    pts=[v.co+mid for v in group];center=sum(pts,Vector())/len(pts)
    span=Vector(tuple(max(v[i] for v in pts)-min(v[i] for v in pts) for i in range(3)))
    ridge=abs(center.x+.028)<.003 and abs(center.y)<.001 and abs(span.x-.139)<.002
    plate=abs(center.x+.015)<.003 and abs(center.z-.031)<.002 and .07<abs(center.y)<.11 and abs(span.x-.067)<.002
    if ridge or plate:remove.append(group)
assert len(remove)==3,len(remove)
bmesh.ops.delete(bm,geom=[v for g in remove for v in g],context='VERTS');bm.to_mesh(o.data);bm.free();o.data.update()
parts=[]
def make(name,vs,fs,color):
    data=bpy.data.meshes.new(name);data.from_pydata([v-mid for v in vs],[],fs);data.update()
    ob=bpy.data.objects.new(name,data);s.collection.objects.link(ob);data.materials.append(bpy.data.materials['M_CL9_PortableLight_Body'])
    a=data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for e in a.data:e.color=color
    parts.append(ob)
def hit(raw_origin,direction):
    ok,co,normal,_=o.ray_cast(Vector(raw_origin)-mid,Vector(direction));assert ok,raw_origin
    return co+mid
# A closed rectangular rib with 17 short cross-sections avoids a chord cutting through the dome.
vs=[];steps=16
for k in range(steps+1):
    x=-.0975+.139*k/steps
    for y,offset in [(-.009,.001),(.009,.001),(.009,.009),(-.009,.009)]:
        co=hit((x,y,.3),(0,0,-1));co.z+=offset;vs.append(co)
fs=[(k*4+i,k*4+(i+1)%4,(k+1)*4+(i+1)%4,(k+1)*4+i) for k in range(steps) for i in range(4)]
fs.extend([(3,2,1,0),tuple(steps*4+i for i in range(4))]);make('HelmetRidgeFitted',vs,fs,(.54,.285,.025,1))
# Thin closed reflective patches, each subdivided in both directions and projected onto the actual shell.
for side in (-1,1):
    nx=12;nz=3;layer=(nx+1)*(nz+1);vs=[]
    for offset in (.0008,.002):
        for iz in range(nz+1):
            for ix in range(nx+1):
                x=-.0485+.067*ix/nx;z=.025+.012*iz/nz
                co=hit((x,side*.3,z),(0,-side,0));co.y+=side*offset;vs.append(co)
    fs=[]
    for l in range(2):
        for iz in range(nz):
            for ix in range(nx):
                a=l*layer+iz*(nx+1)+ix;fs.append((a,a+1,a+nx+2,a+nx+1))
    border=list(range(nx+1))+[iz*(nx+1)+nx for iz in range(1,nz+1)]+[nz*(nx+1)+ix for ix in range(nx-1,-1,-1)]+[iz*(nx+1) for iz in range(nz-1,0,-1)]
    for i,a in enumerate(border):b=border[(i+1)%len(border)];fs.append((a,b,b+layer,a+layer))
    make('HelmetReflectorFitted',vs,fs,(.72,.63,.36,1))
bpy.ops.object.select_all(action='DESELECT');o.select_set(True)
for p in parts:p.select_set(True)
bpy.context.view_layer.objects.active=o;bpy.ops.object.join()
lo=Vector(tuple(min(v.co[i] for v in o.data.vertices) for i in range(3)));hi=Vector(tuple(max(v.co[i] for v in o.data.vertices) for i in range(3)));shift=(lo+hi)/2
for v in o.data.vertices:v.co-=shift
bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));assert all(e.is_manifold for e in bm.edges);bm.to_mesh(o.data);bm.free()
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.025);bpy.ops.object.mode_set(mode='OBJECT')
bpy.context.view_layer.update();o.data.calc_loop_triangles()
f=ROOT/'export'/(o.name+'.fbx');bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
meta.update(dimensions_cm=[v*100 for v in o.dimensions],triangles=len(o.data.loop_triangles),vertices=len(o.data.vertices),lens_center_cm=list(Vector(meta['lens_center_cm'])-shift*100),fbx_sha256=hashlib.sha256(f.read_bytes()).hexdigest())
m['trim_fit_revision']=2;(ROOT/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'portable_lights.blend'),{s,bpy.data.scenes['CL9_PORTABLE_LIGHTS_PREVIEW']},fake_user=True)
print(json.dumps(meta))
