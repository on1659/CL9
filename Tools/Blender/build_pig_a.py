"""Original pig mesh and quadruped rig. No existing animal geometry is copied."""
import bpy,math,json
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/pig_a');assert 'CL9_PIG_A' not in bpy.data.scenes
for folder in ('export','preview'):(ROOT/folder).mkdir(parents=True,exist_ok=True)
s=bpy.data.scenes.new('CL9_PIG_A');bpy.context.window.scene=s;s.unit_settings.system='METRIC';s.unit_settings.scale_length=1;s.render.fps=30
parts=[];palette={'skin':(.54,.265,.22,1),'light':(.70,.39,.32,1),'snout':(.44,.15,.13,1),'inner':(.30,.10,.10,1),
                  'hoof':(.055,.037,.028,1),'dark':(.008,.007,.005,1),'eye':(.75,.61,.43,1),'mud':(.18,.10,.05,1)}
material=bpy.data.materials.new('M_CL9_Pig_Vertex');material.use_nodes=True
bs=next(n for n in material.node_tree.nodes if n.type=='BSDF_PRINCIPLED');bs.inputs['Roughness'].default_value=.86
attr=material.node_tree.nodes.new('ShaderNodeVertexColor');attr.layer_name='Color';material.node_tree.links.new(attr.outputs['Color'],bs.inputs['Base Color'])
def finish(ob,label,color,bone):
    ob.name='PI_'+label;bpy.context.view_layer.objects.active=ob;bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
    ob.data.materials.clear();ob.data.materials.append(material);ca=ob.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for d in ca.data:d.color=palette[color]
    vg=ob.vertex_groups.new(name=bone);vg.add(list(range(len(ob.data.vertices))),1,'REPLACE');parts.append(ob);return ob
def ell(label,loc,scale,color,bone,segments=20,rings=12,rotation=(0,0,0)):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,location=loc)
    ob=bpy.context.object;ob.scale=scale;ob.rotation_euler=rotation;return finish(ob,label,color,bone)
def taper(label,a,b,r1,r2,color,bone):
    a,b=Vector(a),Vector(b);bpy.ops.mesh.primitive_cone_add(vertices=12,radius1=r1,radius2=r2,depth=(b-a).length,location=(a+b)/2)
    ob=bpy.context.object;ob.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler();return finish(ob,label,color,bone)
body=ell('Body',(-.20,0,.66),(.60,.38,.35),'skin','body',28,18);chest_group=body.vertex_groups.new(name='chest')
for v in body.data.vertices:
    t=max(0,min(1,(v.co.x+body.location.x+.05)/.38));body.vertex_groups['body'].add([v.index],1-t,'REPLACE');chest_group.add([v.index],t,'REPLACE')
ell('Chest',(.22,0,.67),(.31,.32,.31),'light','chest')
ell('Neck',(.365,0,.73),(.24,.255,.23),'skin','neck')
ell('Head',(.52,0,.76),(.30,.27,.25),'light','head',24,16)
ell('Chin',(.62,0,.61),(.22,.20,.083),'skin','jaw')
ell('SnoutBridge',(.727,0,.717),(.217,.197,.15),'skin','head')
ell('SnoutDisc',(.88,0,.716),(.065,.20,.136),'snout','head',20,12)
for sign,side in [(1,'l'),(-1,'r')]:
    ell('Nostril_'+side,(.94,sign*.075,.739),(.014,.038,.049),'dark','head',12,8)
    ell('EyeWhite_'+side,(.665,sign*.216,.875),(.065,.027,.049),'eye','head',16,10,rotation=(0,0,-sign*.35))
    ell('Pupil_'+side,(.688,sign*.239,.875),(.026,.012,.031),'dark','head',12,8)
    ell('Brow_'+side,(.638,sign*.209,.919),(.086,.035,.032),'skin','head',16,8,rotation=(0,.05,-sign*.35))
    # Thick folded ear wedge; interior is painted on faces rather than interpenetrating shells.
    vs=[(.32,sign*.15,.91),(.54,sign*.15,.94),(.53,sign*.31,1.11),(.35,sign*.36,1.07),
        (.34,sign*.18,.97),(.51,sign*.18,.99),(.49,sign*.38,1.035),(.36,sign*.41,1.01)]
    fs=[(0,1,2,3),(4,7,6,5),(0,4,5,1),(1,5,6,2),(2,6,7,3),(3,7,4,0)]
    md=bpy.data.meshes.new('PI_Ear_'+side);md.from_pydata(vs,[],fs);md.update();ear=bpy.data.objects.new(md.name,md);s.collection.objects.link(ear)
    finish(ear,'Ear_'+side,'skin','ear_'+side)
    for poly in md.polygons:
        if poly.index in (0,3):
            for li in poly.loop_indices:md.color_attributes['Color'].data[li].color=palette['inner']
    for pair,x,knee_x in [('front',.27,.32),('hind',-.40,-.53)]:
        tag=pair+'_'+side;y=sign*(.215 if pair=='front' else .235);hip=(x,y,.48);knee=(knee_x,y,.28);ankle=(x+.035,y,.07)
        ell('Haunch_'+tag,hip,(.155,.14,.17),'skin',tag+'_upper',18,12)
        taper('Upper_'+tag,hip,knee,.095,.062,'skin',tag+'_upper')
        taper('Lower_'+tag,knee,ankle,.059,.048,'skin',tag+'_lower')
        for j in (-1,1):
            bpy.ops.mesh.primitive_cube_add(size=1,location=(ankle[0]+.033,y+j*.038,.057));ob=bpy.context.object;ob.scale=(.137,.067,.092)
            bevel=ob.modifiers.new('Hoof rounded corners','BEVEL');bevel.width=.012;bevel.segments=1
            bpy.context.view_layer.objects.active=ob;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
            bpy.ops.object.modifier_apply(modifier=bevel.name);finish(ob,'Hoof_'+tag+str(j),'hoof',tag+'_paw')
# Tube along a tapered curl behind the rump; four deformation sections.
path=[];rings=33;segments=8
for i in range(rings):
    u=i/(rings-1);a=math.tau*1.25*u;radius=.095*math.sin(math.pi*min(1,u*1.4))+.018
    path.append(Vector((-.72-.19*u,radius*math.sin(a),.78+radius*(1-math.cos(a)))))
verts=[];faces=[]
for i,c in enumerate(path):
    direction=(path[min(rings-1,i+1)]-path[max(0,i-1)]).normalized();side=direction.cross(Vector((0,1,0))).normalized();up=direction.cross(side).normalized()
    radius=.031-.017*i/(rings-1)
    for j in range(segments):
        a=j*math.tau/segments;verts.append(c+radius*(math.cos(a)*side+math.sin(a)*up))
for i in range(rings-1):
    for j in range(segments):faces.append((i*segments+j,i*segments+(j+1)%segments,(i+1)*segments+(j+1)%segments,(i+1)*segments+j))
faces.extend([tuple(reversed(range(segments))),tuple((rings-1)*segments+j for j in range(segments))])
md=bpy.data.meshes.new('PI_Tail');md.from_pydata(verts,[],faces);md.update();tail=bpy.data.objects.new(md.name,md);s.collection.objects.link(tail);finish(tail,'Tail','skin','tail_01')
for group in list(tail.vertex_groups):tail.vertex_groups.remove(group)
groups=[tail.vertex_groups.new(name=f'tail_{i+1:02d}') for i in range(4)]
for v in md.vertices:
    t=min(3,v.index//segments/(rings-1)*4);i=min(2,int(t));u=t-i;groups[i].add([v.index],1-u,'REPLACE');groups[i+1].add([v.index],u,'REPLACE')
bpy.ops.object.select_all(action='DESELECT')
for ob in parts:ob.select_set(True)
bpy.context.view_layer.objects.active=body;bpy.ops.object.join();mesh=bpy.context.object;mesh.name='SK_Pig_A';s.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.normals_make_consistent(inside=False);bpy.ops.uv.smart_project(island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
data=bpy.data.armatures.new('SKEL_Pig_A');rig=bpy.data.objects.new(data.name,data);s.collection.objects.link(rig)
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);bpy.context.view_layer.objects.active=rig;bpy.ops.object.mode_set(mode='EDIT');bones={}
def bone(name,head,tail,parent=None):
    b=data.edit_bones.new(name);b.head=head;b.tail=tail
    if parent:b.parent=bones[parent]
    bones[name]=b
bone('root',(0,0,0),(0,0,.15));bone('body',(-.30,0,.53),(.08,0,.53),'root');bone('chest',(.08,0,.53),(.28,0,.61),'body')
bone('neck',(.28,0,.61),(.43,0,.70),'chest');bone('head',(.43,0,.70),(.72,0,.74),'neck');bone('jaw',(.54,0,.63),(.79,0,.63),'head')
for sign,side in [(1,'l'),(-1,'r')]:
    bone('ear_'+side,(.42,sign*.17,.94),(.43,sign*.34,1.06),'head')
    for pair,x,parent,knee_x in [('front',.27,'chest',.32),('hind',-.40,'body',-.53)]:
        tag=pair+'_'+side;y=sign*(.215 if pair=='front' else .235)
        bone(tag+'_upper',(x,y,.48),(knee_x,y,.28),parent);bone(tag+'_lower',(knee_x,y,.28),(x+.035,y,.07),tag+'_upper');bone(tag+'_paw',(x+.035,y,.07),(x+.17,y,.045),tag+'_lower')
for i in range(4):bone(f'tail_{i+1:02d}',path[i*8],path[(i+1)*8],'body' if i==0 else f'tail_{i:02d}')
bpy.ops.object.mode_set(mode='OBJECT');mesh.parent=rig;mod=mesh.modifiers.new('CL9 Skin','ARMATURE');mod.object=rig
for p in rig.pose.bones:p.rotation_mode='QUATERNION'
mesh.data.calc_loop_triangles()
for v in mesh.data.vertices:
    weights=[g.weight for g in v.groups if g.weight>1e-6];assert 1<=len(weights)<=2 and abs(sum(weights)-1)<1e-5
manifest={'source':'Original CL9 pig built through Blender MCP','mesh':mesh.name,'skeleton':rig.name,'dimensions_cm':[x*100 for x in mesh.dimensions],
          'triangles':len(mesh.data.loop_triangles),'vertices':len(mesh.data.vertices),'deform_bones':len(data.bones),'material_slots':len(mesh.data.materials),
          'vertex_color':'Color, linear RGBA; UE VertexColor -> BaseColor','axis':'+X forward +Z up; root floor origin','weight_check':'sum 1, max 2 influences','clips':{}}
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8');bpy.data.libraries.write(str(ROOT/'pig_a.blend'),{s},fake_user=True);print(json.dumps(manifest))
