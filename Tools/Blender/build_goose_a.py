"""Original goose with continuous weighted neck and webbed feet; execute through MCP."""
import bpy,math,json
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/goose_a')
assert 'CL9_GOOSE_A' not in bpy.data.scenes,'Inspect existing goose before rebuilding'
for directory in ('export','preview'):(ROOT/directory).mkdir(parents=True,exist_ok=True)
s=bpy.data.scenes.new('CL9_GOOSE_A');bpy.context.window.scene=s
s.unit_settings.system='METRIC';s.unit_settings.scale_length=1;s.render.fps=30
parts=[];palette={'white':(.72,.70,.62,1),'cream':(.88,.84,.69,1),'gray':(.32,.34,.33,1),
                  'orange':(.70,.24,.022,1),'bill_dark':(.24,.072,.010,1),'black':(.009,.011,.009,1),'eye':(.055,.071,.064,1)}
material=bpy.data.materials.new('M_CL9_Goose_Vertex');material.use_nodes=True
bs=next(n for n in material.node_tree.nodes if n.type=='BSDF_PRINCIPLED');bs.inputs['Roughness'].default_value=.86
attr=material.node_tree.nodes.new('ShaderNodeVertexColor');attr.layer_name='Color';material.node_tree.links.new(attr.outputs['Color'],bs.inputs['Base Color'])
def finish(ob,label,color,bone):
    ob.name='GO_'+label;bpy.context.view_layer.objects.active=ob
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
    ob.data.materials.clear();ob.data.materials.append(material)
    ca=ob.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for item in ca.data:item.color=palette[color]
    group=ob.vertex_groups.new(name=bone);group.add(list(range(len(ob.data.vertices))),1,'REPLACE')
    parts.append(ob);return ob
def ell(label,loc,scale,color,bone,segments=20,rings=12,rotation=(0,0,0)):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,location=loc)
    ob=bpy.context.object;ob.scale=scale;ob.rotation_euler=rotation;return finish(ob,label,color,bone)
def taper(label,a,b,r1,r2,color,bone):
    a,b=Vector(a),Vector(b);bpy.ops.mesh.primitive_cone_add(vertices=10,radius1=r1,radius2=r2,depth=(b-a).length,location=(a+b)/2)
    ob=bpy.context.object;ob.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler();return finish(ob,label,color,bone)
ell('Body',(-.16,0,.67),(.56,.345,.35),'white','body',24,16)
ell('Breast',(.205,0,.75),(.28,.285,.31),'cream','body')
ell('Rump',(-.48,0,.73),(.26,.28,.24),'white','body')
ell('Tail',(-.66,0,.78),(.20,.18,.10),'cream','tail',16,10,rotation=(0,.30,0))
# Dense continuous neck rings, rounded transitions along an S profile.
points=[Vector(p) for p in [(.26,0,.80),(.34,0,1.03),(.30,0,1.27),(.39,0,1.49)]]
verts=[];faces=[];ring_count=25;segments=16
for i in range(ring_count):
    t=i/(ring_count-1)*3;k=min(2,int(t));u=t-k
    # Catmull-Rom interpolation keeps the outer neck silhouette continuous.
    p0=points[max(0,k-1)];p1=points[k];p2=points[k+1];p3=points[min(3,k+2)]
    c=.5*((2*p1)+(-p0+p2)*u+(2*p0-5*p1+4*p2-p3)*u*u+(-p0+3*p1-3*p2+p3)*u*u*u)
    tangent=(p2-p1).normalized();side=Vector((0,1,0));front=side.cross(tangent).normalized()
    radius=.14-.054*min(1,i/(ring_count-1)*1.6)
    for j in range(segments):
        angle=j*math.tau/segments;verts.append(c+radius*(math.cos(angle)*side+math.sin(angle)*front))
for i in range(ring_count-1):
    for j in range(segments):
        a=i*segments+j;b=i*segments+(j+1)%segments;faces.append((a,b,b+segments,a+segments))
faces.extend([tuple(reversed(range(segments))),tuple((ring_count-1)*segments+j for j in range(segments))])
data=bpy.data.meshes.new('GO_Neck_Mesh');data.from_pydata(verts,[],faces);data.update()
neck=bpy.data.objects.new('GO_Neck',data);s.collection.objects.link(neck);finish(neck,'Neck','cream','neck_01')
for group in list(neck.vertex_groups):neck.vertex_groups.remove(group)
groups=[neck.vertex_groups.new(name=n) for n in ['body','neck_01','neck_02','neck_03','head']]
for v in data.vertices:
    t=v.index//segments/(ring_count-1)*3+.5;i=min(3,int(t));w=t-i
    groups[i].add([v.index],1-w,'REPLACE');groups[i+1].add([v.index],w,'REPLACE')
ell('Head',(.435,0,1.535),(.185,.126,.15),'cream','head',24,14)
ell('BillTop',(.623,0,1.509),(.186,.095,.048),'orange','head',16,10)
ell('BillEdge',(.633,0,1.47),(.178,.094,.013),'bill_dark','head',16,8)
ell('Jaw',(.621,0,1.45),(.169,.088,.029),'orange','jaw',16,10)
for sign,side in [(1,'l'),(-1,'r')]:
    ell('EyeRim_'+side,(.49,sign*.112,1.579),(.049,.023,.047),'gray','head',14,10)
    ell('Eye_'+side,(.506,sign*.132,1.579),(.032,.012,.030),'black','head',14,10)
    ell('EyeGlint_'+side,(.515,sign*.141,1.589),(.008,.005,.009),'cream','head',8,6)
    ell('Brow_'+side,(.485,sign*.124,1.615),(.062,.02,.02),'cream','head',12,8,rotation=(0,.13,0))
    ell('Nostril_'+side,(.66,sign*.073,1.537),(.025,.013,.008),'bill_dark','head',10,6)
    ell('Wing_'+side,(-.21,sign*.322,.76),(.405,.077,.197),'cream','wing_'+side,20,12,rotation=(0,-.16,0))
    for i in range(5):
        ell('Feather_'+side+str(i),(-.32-i*.055,sign*(.342+i*.003),.692-i*.019),(.225,.041,.041),
            'gray' if i in (3,4) else 'white','wing_tip_'+side,12,8,rotation=(0,-.18,0))
    hip=(-.15,sign*.17,.48);knee=(-.01,sign*.17,.275);ankle=(-.08,sign*.17,.06)
    ell('Haunch_'+side,hip,(.12,.105,.135),'white','thigh_'+side,16,10)
    taper('Thigh_'+side,hip,knee,.065,.04,'white','thigh_'+side)
    taper('Shin_'+side,knee,ankle,.036,.027,'orange','shin_'+side)
    # Closed low-poly webbed foot with three broad toe lobes and a flat underside.
    outline=[(-.13,-.035),(-.02,-.055),(.16,-.118),(.215,-.095),(.155,-.044),(.253,-.008),(.253,.021),(.155,.054),(.206,.104),(.16,.125),(-.02,.05),(-.13,.035)]
    vs=[(x,sign*.17+y,z) for z in (.012,.048) for x,y in outline];n=len(outline)
    fs=[tuple(reversed(range(n))),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
    md=bpy.data.meshes.new('GO_Web_'+side);md.from_pydata(vs,[],fs);md.update()
    ob=bpy.data.objects.new(md.name,md);s.collection.objects.link(ob);finish(ob,'Web_'+side,'orange','foot_'+side)
    ell('Ankle_'+side,ankle,(.043,.036,.045),'orange','foot_'+side,12,8)
bpy.ops.object.select_all(action='DESELECT')
for ob in parts:ob.select_set(True)
bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();mesh=bpy.context.object;mesh.name='SK_Goose_A'
s.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.normals_make_consistent(inside=False)
bpy.ops.uv.smart_project(island_margin=.01);bpy.ops.object.mode_set(mode='OBJECT')
data=bpy.data.armatures.new('SKEL_Goose_A');rig=bpy.data.objects.new(data.name,data);s.collection.objects.link(rig)
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);bpy.context.view_layer.objects.active=rig;bpy.ops.object.mode_set(mode='EDIT')
bones={}
def bone(name,head,tail,parent=None):
    b=data.edit_bones.new(name);b.head=head;b.tail=tail
    if parent:b.parent=bones[parent]
    bones[name]=b
bone('root',(0,0,0),(0,0,.15));bone('body',(-.16,0,.58),(.15,0,.72),'root')
for i in range(3):bone(f'neck_{i+1:02d}',points[i],points[i+1],'body' if i==0 else f'neck_{i:02d}')
bone('head',points[-1],(.57,0,1.535),'neck_03');bone('jaw',(.52,0,1.465),(.77,0,1.465),'head')
bone('tail',(-.52,0,.76),(-.80,0,.84),'body')
for sign,side in [(1,'l'),(-1,'r')]:
    bone('wing_'+side,(.04,sign*.27,.84),(-.32,sign*.34,.72),'body')
    bone('wing_tip_'+side,(-.32,sign*.34,.72),(-.65,sign*.36,.61),'wing_'+side)
    bone('thigh_'+side,(-.15,sign*.17,.48),(-.01,sign*.17,.275),'body')
    bone('shin_'+side,(-.01,sign*.17,.275),(-.08,sign*.17,.06),'thigh_'+side)
    bone('foot_'+side,(-.08,sign*.17,.06),(.20,sign*.17,.03),'shin_'+side)
bpy.ops.object.mode_set(mode='OBJECT');mesh.parent=rig;mod=mesh.modifiers.new('CL9 Skin','ARMATURE');mod.object=rig
for p in rig.pose.bones:p.rotation_mode='QUATERNION'
mesh.data.calc_loop_triangles()
for v in mesh.data.vertices:
    w=[g.weight for g in v.groups if g.weight>1e-6];assert 1<=len(w)<=2 and abs(sum(w)-1)<1e-5
manifest={'source':'Original CL9 goose through Blender MCP','mesh':mesh.name,'skeleton':rig.name,
          'dimensions_cm':[x*100 for x in mesh.dimensions],'vertices':len(mesh.data.vertices),'triangles':len(mesh.data.loop_triangles),
          'deform_bones':len(data.bones),'material_slots':len(mesh.data.materials),'vertex_color':'Color, linear RGBA; UE VertexColor -> BaseColor',
          'axis':'+X forward +Z up; root floor origin','weight_check':'sum 1, max 2 influences','clips':{}}
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'goose_a.blend'),{s},fake_user=True);print(json.dumps(manifest))
