"""CL9 original stylized chicken mesh + deform skeleton. Execute through Blender MCP."""
import bpy,math,json
from mathutils import Vector,Matrix
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/chicken_a')
for folder in ('export','preview'): (ROOT/folder).mkdir(parents=True,exist_ok=True)
assert 'CL9_CHICKEN_A' not in bpy.data.scenes, 'Inspect existing chicken before rebuilding'
scene=bpy.data.scenes.new('CL9_CHICKEN_A');bpy.context.window.scene=scene
scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=1;scene.render.fps=30
parts=[]
palette={'feather':(.64,.55,.37,1),'cream':(.9,.82,.59,1),'shadow':(.24,.20,.13,1),
         'red':(.42,.022,.015,1),'orange':(.60,.27,.032,1),'dark':(.016,.013,.011,1),
         'eye':(.82,.72,.45,1),'claw':(.24,.14,.045,1)}
material=bpy.data.materials.new('M_CL9_Chicken_Vertex');material.use_nodes=True
bs=next(n for n in material.node_tree.nodes if n.type=='BSDF_PRINCIPLED');bs.inputs['Roughness'].default_value=.77
attr=material.node_tree.nodes.new('ShaderNodeVertexColor');attr.layer_name='Color'
material.node_tree.links.new(attr.outputs['Color'],bs.inputs['Base Color'])

def finish(ob,label,color,bone):
    ob.name='CH_'+label;bpy.context.view_layer.objects.active=ob
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    ob.data.materials.clear();ob.data.materials.append(material)
    ca=ob.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for d in ca.data:d.color=palette[color]
    vg=ob.vertex_groups.new(name=bone);vg.add(list(range(len(ob.data.vertices))),1,'REPLACE')
    parts.append(ob);return ob
def ell(label,loc,scale,color,bone,segments=20,rings=12):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,radius=1,location=loc)
    ob=bpy.context.object;ob.scale=scale;return finish(ob,label,color,bone)
def taper(label,a,b,r1,r2,color,bone,vertices=10):
    a,b=Vector(a),Vector(b)
    bpy.ops.mesh.primitive_cone_add(vertices=vertices,radius1=r1,radius2=r2,depth=(b-a).length,location=(a+b)/2)
    ob=bpy.context.object;ob.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler()
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=False)
    return finish(ob,label,color,bone)

ell('Body',(-.06,0,.61),(.41,.31,.35),'feather','body',24,16)
ell('Breast',(.16,0,.68),(.27,.27,.30),'cream','body')
ell('Neck',(.24,0,.87),(.17,.17,.25),'cream','neck')
ell('Head',(.33,0,1.03),(.19,.17,.19),'cream','head')
taper('Beak',(.455,0,1.015),(.68,0,.974),.092,.009,'orange','head',8)
taper('Jaw',(.44,0,.947),(.643,0,.959),.060,.009,'claw','jaw',8)
for y,side in [(-1,'r'),(1,'l')]:
    ell('EyeSocket_'+side,(.403,y*.145,1.078),(.081,.039,.071),'shadow','head',16,10)
    ell('Eye_'+side,(.421,y*.17,1.08),(.052,.016,.047),'eye','head',16,10)
    ell('Pupil_'+side,(.434,y*.184,1.08),(.023,.009,.030),'dark','head',12,8)
    brow=taper('Brow_'+side,(.355,y*.176,1.132),(.471,y*.163,1.119),.024,.018,'cream','head')
    ell('Wattle_'+side,(.412,y*.052,.866),(.047,.036,.086),'red','jaw',12,10)
    wing=ell('Wing_'+side,(-.08,y*.292,.64),(.28,.082,.19),'cream','wing_'+side)
    wing.rotation_euler.y=-.3
    for i in range(5):
        feather=ell('Flight_'+side+str(i),(-.17-i*.036,y*(.306+i*.007),.60-i*.031),(.18,.044,.048),
                    'feather' if i%2 else 'cream','wing_tip_'+side,12,8)
        feather.rotation_euler.y=-.3+i*.04
    hip=(-.075,y*.145,.47);knee=(.045,y*.145,.255);ankle=(-.055,y*.145,.065)
    ell('Haunch_'+side,hip,(.105,.093,.14),'feather','thigh_'+side,16,10)
    taper('Thigh_'+side,hip,knee,.068,.044,'feather','thigh_'+side)
    taper('Shin_'+side,knee,ankle,.037,.026,'orange','shin_'+side)
    ell('Ankle_'+side,ankle,(.039,.033,.031),'orange','foot_'+side,12,8)
    for j in (-1,0,1):
        tip=(.15 if j==0 else .12,y*.145+j*.065,.023)
        taper('Toe_'+side+str(j),ankle,tip,.025,.014,'orange','foot_'+side,8)
        taper('Claw_'+side+str(j),tip,(tip[0]+.032,tip[1],.012),.015,.002,'claw','foot_'+side,8)
    taper('BackToe_'+side,ankle,(-.14,y*.145,.025),.022,.010,'orange','foot_'+side,8)
for i in range(5):
    x=.18+i*.064
    ell('Comb_'+str(i),(x,0,1.18+(.06 if i in (1,2,3) else .018)),(.054,.037,.102),'red','comb',12,8)
for i in range(5):
    ob=ell('Tail_'+str(i),(-.44-abs(i-2)*.02,(i-2)*.064,.85+(.055 if i==2 else 0)),(.11,.047,.24),
           'shadow' if i%2 else 'feather','tail',14,10)
    ob.rotation_euler.y=-.50+(i-2)*.07

# Join leaves rigid anatomical pieces weighted to their own deform bones; feathers overlap
# at natural joint covers instead of stretching a single shell across the wing hinge.
bpy.ops.object.select_all(action='DESELECT')
for ob in parts:ob.select_set(True)
bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();mesh=bpy.context.object
mesh.name='SK_Chicken_A';scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.01);bpy.ops.object.mode_set(mode='OBJECT')

data=bpy.data.armatures.new('SKEL_Chicken_A');rig=bpy.data.objects.new('SKEL_Chicken_A',data);scene.collection.objects.link(rig)
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);bpy.context.view_layer.objects.active=rig;bpy.ops.object.mode_set(mode='EDIT')
bones={}
def bone(name,head,tail,parent=None):
    b=data.edit_bones.new(name);b.head=head;b.tail=tail
    if parent:b.parent=bones[parent]
    bones[name]=b
bone('root',(0,0,0),(0,0,.15))
bone('body',(-.06,0,.55),(.16,0,.70),'root')
bone('neck',(.17,0,.76),(.29,0,.98),'body')
bone('head',(.29,0,.98),(.48,0,1.03),'neck')
bone('jaw',(.43,0,.945),(.62,0,.948),'head')
bone('comb',(.31,0,1.15),(.31,0,1.32),'head')
bone('tail',(-.32,0,.71),(-.50,0,.99),'body')
for sign,side in [(1,'l'),(-1,'r')]:
    bone('wing_'+side,(.065,sign*.245,.78),(-.16,sign*.31,.59),'body')
    bone('wing_tip_'+side,(-.16,sign*.31,.59),(-.36,sign*.35,.47),'wing_'+side)
    bone('thigh_'+side,(-.075,sign*.145,.47),(.045,sign*.145,.255),'body')
    bone('shin_'+side,(.045,sign*.145,.255),(-.055,sign*.145,.065),'thigh_'+side)
    bone('foot_'+side,(-.055,sign*.145,.065),(.12,sign*.145,.025),'shin_'+side)
bpy.ops.object.mode_set(mode='OBJECT')
mesh.parent=rig;mod=mesh.modifiers.new('CL9 skin','ARMATURE');mod.object=rig
for p in rig.pose.bones:p.rotation_mode='QUATERNION'
mesh.data.calc_loop_triangles()
assert all(len(v.groups)==1 and abs(v.groups[0].weight-1)<1e-6 for v in mesh.data.vertices)
manifest={'source':'Original CL9 chicken built through Blender MCP','mesh':mesh.name,'skeleton':rig.name,
          'dimensions_cm':[float(v)*100 for v in mesh.dimensions], 'triangles':len(mesh.data.loop_triangles),
          'vertices':len(mesh.data.vertices),'deform_bones':len(data.bones),'material_slots':len(mesh.data.materials),
          'vertex_color':'Color, linear RGBA; UE VertexColor -> BaseColor','weight_check':'all vertices one influence at 1.0',
          'axis':'+X forward +Z up; root floor origin','clips':{}}
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'chicken_a.blend'),{scene},fake_user=True)
print(json.dumps(manifest))
