"""Create a CL9 raccoon with original geometry, vertex colors and deform rig via MCP."""
import bpy,math,json
from mathutils import Vector
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/raccoon_a')
for folder in ('export','preview'):(ROOT/folder).mkdir(parents=True,exist_ok=True)
assert 'CL9_RACCOON_A' not in bpy.data.scenes,'Inspect existing raccoon before rebuilding'
scene=bpy.data.scenes.new('CL9_RACCOON_A');bpy.context.window.scene=scene
scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=1;scene.render.fps=30
parts=[]
palette={'fur':(.23,.25,.24,1),'light':(.66,.65,.56,1),'dark':(.027,.032,.030,1),
         'nose':(.010,.013,.012,1),'iris':(.34,.23,.075,1),'claw':(.30,.26,.19,1),'ear':(.16,.12,.105,1)}
material=bpy.data.materials.new('M_CL9_Raccoon_Vertex');material.use_nodes=True
bs=next(n for n in material.node_tree.nodes if n.type=='BSDF_PRINCIPLED');bs.inputs['Roughness'].default_value=.83
attr=material.node_tree.nodes.new('ShaderNodeVertexColor');attr.layer_name='Color'
material.node_tree.links.new(attr.outputs['Color'],bs.inputs['Base Color'])
def finish(ob,label,color,bone):
    ob.name='RA_'+label;bpy.context.view_layer.objects.active=ob
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
    ob.data.materials.clear();ob.data.materials.append(material)
    ca=ob.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for d in ca.data:d.color=palette[color]
    vg=ob.vertex_groups.new(name=bone);vg.add(list(range(len(ob.data.vertices))),1,'REPLACE')
    parts.append(ob);return ob
def ell(label,loc,scale,color,bone,segments=20,rings=12,rotation=(0,0,0)):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,radius=1,location=loc)
    ob=bpy.context.object;ob.scale=scale;ob.rotation_euler=rotation
    return finish(ob,label,color,bone)
def taper(label,a,b,r1,r2,color,bone,vertices=12):
    a,b=Vector(a),Vector(b)
    bpy.ops.mesh.primitive_cone_add(vertices=vertices,radius1=r1,radius2=r2,depth=(b-a).length,location=(a+b)/2)
    ob=bpy.context.object;ob.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler()
    return finish(ob,label,color,bone)
body=ell('Body',(-.14,0,.53),(.49,.32,.30),'fur','body',28,18)
# A continuous torso can bend subtly between the rear body and chest.
chest_group=body.vertex_groups.new(name='chest')
for v in body.data.vertices:
    world_x=v.co.x+body.location.x;t=max(0,min(1,(world_x+.08)/.35))
    body.vertex_groups['body'].add([v.index],1-t,'REPLACE');chest_group.add([v.index],t,'REPLACE')
ell('Chest',(.20,0,.55),(.23,.265,.26),'fur','chest')
ell('Throat',(.36,0,.59),(.18,.205,.20),'light','neck')
ell('Head',(.45,0,.72),(.245,.245,.225),'light','head',24,16)
ell('Crown',(.365,0,.805),(.205,.223,.15),'fur','head')
ell('Muzzle',(.644,0,.646),(.17,.133,.095),'light','head')
ell('LowerJaw',(.621,0,.596),(.15,.112,.047),'dark','jaw')
ell('Nose',(.792,0,.66),(.058,.071,.045),'nose','head',14,10)
for sign,side in [(1,'l'),(-1,'r')]:
    ell('Cheek_'+side,(.344,sign*.215,.658),(.18,.095,.117),'light','head',16,10,rotation=(0,0,sign*.28))
    ell('Mask_'+side,(.585,sign*.163,.755),(.123,.052,.084),'dark','head',20,12,rotation=(0,-.20,sign*-.57))
    ell('Eye_'+side,(.619,sign*.196,.775),(.045,.027,.035),'iris','head',16,10,rotation=(0,0,sign*-.40))
    ell('Pupil_'+side,(.641,sign*.207,.778),(.022,.012,.024),'nose','head',12,8,rotation=(0,0,sign*-.40))
    ell('Brow_'+side,(.572,sign*.179,.824),(.107,.040,.026),'light','head',16,8,rotation=(0,.03,sign*-.50))
    ell('Ear_'+side,(.338,sign*.177,.948),(.089,.062,.105),'dark','ear_'+side,16,12,rotation=(sign*.16,0,0))
    ell('EarRim_'+side,(.352,sign*.182,.956),(.073,.065,.088),'light','ear_'+side,16,10,rotation=(sign*.16,0,0))
    ell('InnerEar_'+side,(.393,sign*.179,.956),(.030,.047,.057),'ear','ear_'+side,14,10)
    # Fur cheek tips are a few large wedges, not individual hair cards.
    for i in range(2):
        taper('CheekTip_'+side+str(i),(.335,sign*.21,.68-i*.07),(.20,sign*(.31+i*.013),.63-i*.07),
              .071,.015,'light','head',8)
    for pair,x,z,parent,knee_x in [('front',.27,.48,'chest',.32),('hind',-.40,.48,'body',-.53)]:
        tag=pair+'_'+side;y=sign*(.215 if pair=='front' else .235)
        hip=(x,y,z);knee=(knee_x,y,.28);ankle=(x+.035,y,.07)
        ell('Haunch_'+tag,hip,(.12,.11,.15),'fur',tag+'_upper',18,12)
        taper('Upper_'+tag,hip,knee,.085,.051,'fur',tag+'_upper')
        taper('Lower_'+tag,knee,ankle,.050,.041,'dark',tag+'_lower')
        ell('Paw_'+tag,(ankle[0]+.039,y,.061),(.102,.073,.054),'dark',tag+'_paw',16,10)
        for j in range(4):
            toe=(ankle[0]+.102,y+(j-1.5)*.035,.037)
            ell('Finger_'+tag+str(j),toe,(.062,.024,.026),'dark',tag+'_paw',12,8)
            taper('Claw_'+tag+str(j),(toe[0]+.044,toe[1],.041),(toe[0]+.066,toe[1],.025),.012,.003,'claw',tag+'_paw',8)

# One continuous ringed tail with interpolated adjacent-bone weights.
tail_path=[Vector(p) for p in [(-.50,0,.60),(-.70,0,.47),(-.89,.025,.37),(-1.08,.055,.34),(-1.26,.085,.37)]]
verts=[];faces=[];rings=25;segments=16
for i in range(rings):
    t=i/(rings-1)*4;k=min(3,int(t));u=t-k;c=tail_path[k].lerp(tail_path[k+1],u)
    direction=(tail_path[k+1]-tail_path[k]).normalized();side=Vector((0,1,0));up=direction.cross(side).normalized()
    radius=.127*math.sin(math.pi*(.16+.84*i/(rings-1)))+.009
    for j in range(segments):
        a=j*math.tau/segments;verts.append(c+radius*(math.cos(a)*side+math.sin(a)*up))
for i in range(rings-1):
    for j in range(segments):faces.append((i*segments+j,i*segments+(j+1)%segments,(i+1)*segments+(j+1)%segments,(i+1)*segments+j))
faces.append(tuple(reversed(range(segments))));faces.append(tuple((rings-1)*segments+j for j in range(segments)))
data=bpy.data.meshes.new('RA_Tail_Mesh');data.from_pydata(verts,[],faces);data.update()
tail=bpy.data.objects.new('RA_Tail',data);scene.collection.objects.link(tail);finish(tail,'Tail','fur','tail_01')
for p in data.polygons:
    ring=min(v//segments for v in p.vertices)
    for li in p.loop_indices:data.color_attributes['Color'].data[li].color=palette['dark' if (ring//3)%2 else 'light']
for group in list(tail.vertex_groups):tail.vertex_groups.remove(group)
groups=[tail.vertex_groups.new(name=f'tail_{i+1:02d}') for i in range(4)]
for v in data.vertices:
    position=min(3,v.index//segments/(rings-1)*4);i=min(2,int(position));u=position-i
    groups[i].add([v.index],1-u,'REPLACE');groups[i+1].add([v.index],u,'REPLACE')

bpy.ops.object.select_all(action='DESELECT')
for ob in parts:ob.select_set(True)
bpy.context.view_layer.objects.active=body;bpy.ops.object.join();mesh=bpy.context.object;mesh.name='SK_Raccoon_A'
scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.normals_make_consistent(inside=False)
bpy.ops.uv.smart_project(island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
data=bpy.data.armatures.new('SKEL_Raccoon_A');rig=bpy.data.objects.new('SKEL_Raccoon_A',data);scene.collection.objects.link(rig)
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);bpy.context.view_layer.objects.active=rig;bpy.ops.object.mode_set(mode='EDIT')
bones={}
def bone(name,head,tail,parent=None):
    b=data.edit_bones.new(name);b.head=head;b.tail=tail
    if parent:b.parent=bones[parent]
    bones[name]=b
bone('root',(0,0,0),(0,0,.15));bone('body',(-.30,0,.53),(.08,0,.53),'root')
bone('chest',(.08,0,.53),(.28,0,.61),'body');bone('neck',(.28,0,.61),(.43,0,.70),'chest')
bone('head',(.43,0,.70),(.65,0,.70),'neck');bone('jaw',(.57,0,.606),(.76,0,.606),'head')
for sign,side in [(1,'l'),(-1,'r')]:
    bone('ear_'+side,(.338,sign*.177,.89),(.338,sign*.177,1.015),'head')
    for pair,x,parent,knee_x in [('front',.27,'chest',.32),('hind',-.40,'body',-.53)]:
        tag=pair+'_'+side;y=sign*(.215 if pair=='front' else .235)
        bone(tag+'_upper',(x,y,.48),(knee_x,y,.28),parent)
        bone(tag+'_lower',(knee_x,y,.28),(x+.035,y,.07),tag+'_upper')
        bone(tag+'_paw',(x+.035,y,.07),(x+.17,y,.045),tag+'_lower')
for i in range(4):bone(f'tail_{i+1:02d}',tail_path[i],tail_path[i+1],'body' if i==0 else f'tail_{i:02d}')
bpy.ops.object.mode_set(mode='OBJECT')
mesh.parent=rig;modifier=mesh.modifiers.new('CL9 Skin','ARMATURE');modifier.object=rig
for p in rig.pose.bones:p.rotation_mode='QUATERNION'
mesh.data.calc_loop_triangles()
for v in mesh.data.vertices:
    assert 1<=len([g for g in v.groups if g.weight>1e-6])<=4
    assert abs(sum(g.weight for g in v.groups)-1)<1e-5
manifest={'source':'Original CL9 raccoon built through Blender MCP','mesh':mesh.name,'skeleton':rig.name,
          'dimensions_cm':[float(v)*100 for v in mesh.dimensions],'triangles':len(mesh.data.loop_triangles),
          'vertices':len(mesh.data.vertices),'deform_bones':len(data.bones),'material_slots':len(mesh.data.materials),
          'vertex_color':'Color, linear RGBA; UE VertexColor -> BaseColor','weight_check':'sum 1; max 2 nonzero influences',
          'axis':'+X forward +Z up; root floor origin','clips':{}}
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'raccoon_a.blend'),{scene},fake_user=True)
print(json.dumps(manifest))
