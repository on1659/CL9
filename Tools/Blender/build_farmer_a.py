"""Original workwear, boots, face and five-finger gloves on the proven Manny reference rig.
Final recipe: this initial build -> fit_farmer_a_silhouette.py -> bake_farmer_palette.py
-> export_farmer_a.py. Reuse existing delivered source rather than rerunning creation.
"""
import bpy,bmesh,math,json
from pathlib import Path
from mathutils import Vector,Matrix
ROOT=Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a');ROOT.mkdir(parents=True,exist_ok=True)
assert 'CL9_FARMER_A' not in bpy.data.scenes
s=bpy.data.scenes.new('CL9_FARMER_A');s.unit_settings.system='METRIC';s.unit_settings.scale_length=1;s.render.fps=30;bpy.context.window.scene=s
rig=bpy.data.objects['root'].copy();rig.data=rig.data.copy();rig.name='RIG_Farmer_A';s.collection.objects.link(rig);rig.animation_data_clear()
parts=[];arm_parts=[];materials={}
for key,col,vc in [('Cloth',(.22,.31,.135,1),True),('Detail',(1,1,1,1),True),('Skin',(.48,.30,.19,1),False)]:
 m=bpy.data.materials.new('M_CL9_Farmer_'+key);m.use_nodes=True;bs=next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED');bs.inputs['Base Color'].default_value=col;bs.inputs['Roughness'].default_value=.88
 if vc:
  v=m.node_tree.nodes.new('ShaderNodeVertexColor');v.layer_name='Color'
  mul=m.node_tree.nodes.new('ShaderNodeMixRGB');mul.blend_type='MULTIPLY';mul.inputs[0].default_value=1;mul.inputs[2].default_value=col;m.node_tree.links.new(v.outputs['Color'],mul.inputs[1]);m.node_tree.links.new(mul.outputs[0],bs.inputs['Base Color'])
 materials[key]=m
def head(n):return rig.data.bones[n].head_local.copy()
def weight_mix(a,b,t):return {a:1-t,b:t} if a!=b else {a:1}
def torso_weights(p):
 names=['pelvis','spine_01','spine_02','spine_03','spine_04','spine_05','neck_01'];z=p.z
 if z<head(names[0]).z:return {names[0]:1}
 for a,b in zip(names,names[1:]):
  if z<=head(b).z:return weight_mix(a,b,max(0,min(1,(z-head(a).z)/(head(b).z-head(a).z))))
 return {'spine_05':1}
def make(name,verts,faces,mat='Cloth',color=(1,1,1,1),weights=None,arm=False):
 data=bpy.data.meshes.new(name);data.from_pydata(verts,[],faces);data.update();o=bpy.data.objects.new(name,data);s.collection.objects.link(o);data.materials.append(materials[mat])
 bm=bmesh.new();bm.from_mesh(data);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(data);bm.free()
 c=data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
 for v in c.data:v.color=color
 for v in data.vertices:
  ws=weights(v.co) if callable(weights) else weights or {'pelvis':1};total=sum(ws.values())
  for n,w in ws.items():
   if w>1e-5:(o.vertex_groups.get(n) or o.vertex_groups.new(name=n)).add([v.index],w/total,'REPLACE')
 parts.append(o)
 if arm:arm_parts.append(o)
 return o
def loft(name,rings,mat='Cloth',color=(1,1,1,1),weights=None,n=16,power=.85,arm=False):
 vs=[]
 for center,rx,ry in rings:
  for i in range(n):
   a=math.tau*i/n;c=math.cos(a);t=math.sin(a);vs.append(Vector(center)+Vector((rx*math.copysign(abs(c)**power,c),ry*math.copysign(abs(t)**power,t),0)))
 fs=[tuple(reversed(range(n))),tuple((len(rings)-1)*n+i for i in range(n))]
 for j in range(len(rings)-1):fs.extend((j*n+i,j*n+(i+1)%n,(j+1)*n+(i+1)%n,(j+1)*n+i) for i in range(n))
 return make(name,vs,fs,mat,color,weights,arm)
def tube(name,points,radii,mat='Cloth',color=(1,1,1,1),weights=None,n=12,arm=False,flatten=1):
 ps=list(map(Vector,points));vs=[]
 for j,p in enumerate(ps):
  tangent=(ps[min(j+1,len(ps)-1)]-ps[max(j-1,0)]).normalized();reference=Vector((0,-1,0))
  if abs(tangent.dot(reference))>.94:reference=Vector((1,0,0))
  u=tangent.cross(reference).normalized();v=tangent.cross(u).normalized()
  for i in range(n):a=math.tau*i/n;vs.append(p+radii[j]*(math.cos(a)*u+flatten*math.sin(a)*v))
 fs=[tuple(reversed(range(n))),tuple((len(ps)-1)*n+i for i in range(n))]
 for j in range(len(ps)-1):fs.extend((j*n+i,j*n+(i+1)%n,(j+1)*n+(i+1)%n,(j+1)*n+i) for i in range(n))
 return make(name,vs,fs,mat,color,weights,arm)
def box(name,center,size,mat='Detail',color=(.08,.065,.042,1),weights=None,arm=False,bevel=.004):
 bpy.ops.mesh.primitive_cube_add(size=1,location=center);o=bpy.context.object;o.name=name;o.dimensions=size;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
 if bevel:
  mod=o.modifiers.new('Edge','BEVEL');mod.width=min(bevel,min(size)*.3);mod.segments=1;bpy.ops.object.modifier_apply(modifier=mod.name)
 vs=[o.matrix_world@v.co for v in o.data.vertices];fs=[tuple(p.vertices) for p in o.data.polygons];bpy.data.objects.remove(o,do_unlink=True)
 return make(name,vs,fs,mat,color,weights,arm)
# Garment silhouette: rounded rectangular hips, tapered waist, broad shoulders.
loft('CoverallTorso',[((0,-.015,z),rx,ry) for z,rx,ry in [(.86,.17,.105),(.93,.205,.13),(1.025,.195,.13),(1.105,.165,.12),(1.24,.195,.14),(1.35,.22,.145),(1.43,.18,.12),(1.475,.073,.063)]],weights=torso_weights,n=20)
loft('Neck',[((0,0,z),r,r*.85) for z,r in [(1.45,.056),(1.51,.058),(1.61,.048)]],mat='Skin',weights={'neck_01':.5,'neck_02':.5},n=12)
loft('WorkBelt',[((0,-.015,z),.198,.135) for z in [1.015,1.045]],mat='Detail',color=(.13,.075,.032,1),weights=torso_weights,n=20)
box('BeltBuckle',(0,-.157,1.030),(.065,.012,.042),'Detail',(.45,.40,.25,1),torso_weights)
box('BibPocket',(0,-.168,1.275),(.18,.025,.15),'Cloth',(.88,.9,.85,1),torso_weights,bevel=.01)
box('PocketLip',(0,-.184,1.343),(.17,.012,.016),'Detail',(.13,.15,.085,1),torso_weights)
for side in [-1,1]:
 box('BibStrap',(side*.115,-.157,1.385),(.052,.022,.155),'Cloth',(.78,.8,.76,1),torso_weights)
 box('StrapButton',(side*.115,-.173,1.337),(.026,.014,.027),'Detail',(.45,.40,.25,1),torso_weights)
box('BackIdentityPanel',(0,.142,1.328),(.245,.014,.12),'Cloth',(1.25,1.25,1.25,1),torso_weights)
dark=(.045,.055,.048,1);glove=(.22,.16,.075,1)
for side,sign in [('l',1),('r',-1)]:
 hip=head('thigh_'+side);knee=head('calf_'+side);ankle=head('foot_'+side)
 def legw(p,side=side,knee=knee):
  t=max(0,min(1,(knee.z+.06-p.z)/.12));return weight_mix('thigh_'+side,'calf_'+side,t)
 pts=[hip+Vector((0,0,.02)),hip.lerp(knee,.2),hip.lerp(knee,.65),knee+Vector((0,0,.055)),knee,knee.lerp(ankle,.3),knee.lerp(ankle,.7)]
 tube('Trouser_'+side,pts,[.108,.109,.094,.09,.087,.084,.072],weights=legw,n=16)
 # Boots have a broad toe, sloped instep and fitted shaft, with level soles.
 x=ankle.x;y=ankle.y
 loft('Boot_'+side,[((x,y-.065,.025),.089,.158),((x,y-.065,.065),.093,.158),((x,y-.09,.115),.09,.135),((x,y-.026,.175),.075,.08),((x,y,.255),.075,.067),((x,y,.275),.08,.073)],mat='Detail',color=dark,weights={'foot_'+side:.7,'calf_'+side:.3},n=16,power=.65)
 loft('BootSole_'+side,[((x,y-.065,.008),.091,.16),((x,y-.065,.035),.091,.16)],mat='Detail',color=(.023,.028,.023,1),weights={'foot_'+side:1},n=16,power=.65)
 shoulder=head('upperarm_'+side);elbow=head('lowerarm_'+side);wrist=head('hand_'+side)
 def armw(p,side=side,elbow=elbow,shoulder=shoulder,wrist=wrist):
  d=(wrist-elbow).normalized();along=(p-elbow).dot(d);t=max(0,min(1,(along+.045)/.09));return weight_mix('upperarm_'+side,'lowerarm_'+side,t)
 pts=[shoulder.lerp(elbow,-.08),shoulder.lerp(elbow,.15),shoulder.lerp(elbow,.60),shoulder.lerp(elbow,.9),elbow,elbow.lerp(wrist,.25),elbow.lerp(wrist,.70),elbow.lerp(wrist,.91)]
 tube('WorkSleeve_'+side,pts,[.10,.097,.082,.08,.078,.073,.067,.062],weights=armw,n=16,arm=True)
 tube('Cuff_'+side,[elbow.lerp(wrist,.85),elbow.lerp(wrist,.96)],[.064,.064],mat='Detail',color=(.12,.14,.065,1),weights={'lowerarm_'+side:1},n=16,arm=True)
 mid=head('middle_01_'+side);pinky=head('pinky_01_'+side);idx=head('index_01_'+side)
 # Hand width follows the actual metacarpals, with separately articulated fingers.
 tube('GlovePalm_'+side,[wrist,wrist.lerp(mid,.36),wrist.lerp(mid,.8),mid],[.032,.045,.043,.033],mat='Detail',color=glove,weights={'hand_'+side:1},n=12,arm=True,flatten=.64)
 for finger in ['thumb','index','middle','ring','pinky']:
  names=[finger+'_'+str(j).zfill(2)+'_'+side for j in [1,2,3]];ps=[head(n) for n in names];ps.append(ps[-1]+(ps[-1]-ps[-2])*.72)
  def fingerw(p,names=names,ps=ps):
   ds=[(p-q).length for q in ps[:3]];j=min(range(3),key=lambda i:ds[i]);return {names[j]:1}
  radius=.011 if finger in ['thumb','middle'] else .0095
  tube('Glove_'+finger+'_'+side,ps,[radius,radius*.97,radius*.87,radius*.66],mat='Detail',color=glove,weights=fingerw,n=8,arm=True,flatten=.88)
 # Patch follows upper arm for visible slot colour in side views.
 patch=shoulder.lerp(elbow,.30)+Vector((0,-.087,0))
 box('SleevePatch_'+side,patch,(.052,.012,.065),'Cloth',(1.2,1.2,1.2,1),{'upperarm_'+side:1},True)
# Original stylized face, ears and short-brim cap, all weighted to the head bone.
hw={'head':1}
loft('Face',[((0,y,z),rx,ry) for z,y,rx,ry in [(1.58,-.023,.052,.044),(1.62,-.024,.075,.060),(1.69,-.005,.087,.079),(1.75,0,.085,.081),(1.79,.007,.069,.065),(1.805,.012,.040,.04)]],mat='Skin',weights=hw,n=16,power=.85)
for sign in [-1,1]:
 loft('Ear',[((sign*.089,.005,z),r,r*.65) for z,r in [(1.66,.016),(1.687,.02),(1.711,.012)]],mat='Skin',weights=hw,n=8)
 box('Eye',(sign*.032,-.084,1.723),(.019,.009,.013),'Detail',(.014,.018,.014,1),hw,bevel=.003)
 box('Brow',(sign*.032,-.083,1.746),(.041,.012,.011),'Detail',(.075,.042,.023,1),hw,bevel=.002)
box('Nose',(0,-.096,1.699),(.031,.037,.037),'Skin',weights=hw,bevel=.011)
box('Moustache',(0,-.086,1.666),(.072,.017,.019),'Detail',(.10,.054,.026,1),hw,bevel=.004)
box('LowerLip',(0,-.081,1.647),(.035,.01,.007),'Detail',(.27,.105,.063,1),hw,bevel=.002)
loft('CapCrown',[((0,.012,z),rx,ry) for z,rx,ry in [(1.76,.093,.087),(1.80,.092,.084),(1.835,.077,.07),(1.852,.04,.04)]],weights=hw,n=16)
loft('CapBand',[((0,.012,z),.094,.088) for z in [1.757,1.777]],mat='Detail',color=(.13,.15,.072,1),weights=hw,n=16)
box('CapBill',(0,-.095,1.775),(.185,.125,.015),'Cloth',(.72,.76,.65,1),hw,bevel=.006)
box('CapBadge',(0,-.078,1.809),(.040,.014,.022),'Detail',(.71,.63,.40,1),hw,bevel=.003)
# Two mesh deliveries share original sleeve/glove geometry, not mannequin surface geometry.
arm_copies=[]
for o in arm_parts:
 c=o.copy();c.data=o.data.copy();s.collection.objects.link(c);arm_copies.append(c)
def finish(objs,name):
 bpy.ops.object.select_all(action='DESELECT')
 for o in objs:o.select_set(True)
 bpy.context.view_layer.objects.active=objs[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
 bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.018);bpy.ops.object.mode_set(mode='OBJECT')
 mod=o.modifiers.new('MannyDeform','ARMATURE');mod.object=rig;o.parent=rig;o.data.calc_loop_triangles()
 return o
body=finish(parts,'SK_Farmer_A_Body');arms=finish(arm_copies,'SK_Farmer_A_Arms');arms.hide_render=True;arms.hide_set(True)
meta={'source':'Original CL9 geometry via Blender MCP; existing Epic Manny bones/reference transforms reused','source_blend':'farmer_a.blend','scene':s.name,'rig':rig.name,'authoring_units_m':1,'bone_count_without_root_object':len(rig.data.bones),'materials':{k:m.name for k,m in materials.items()},'assets':{}}
for o in [body,arms]:meta['assets'][o.name]={'triangles':len(o.data.loop_triangles),'vertices':len(o.data.vertices),'dimensions_cm':[100*v for v in o.dimensions],'uv_layers':len(o.data.uv_layers),'vertex_groups':len(o.vertex_groups)}
(ROOT/'manifest.json').write_text(json.dumps(meta,indent=2),encoding='utf-8');bpy.data.libraries.write(str(ROOT/'farmer_a.blend'),{s},fake_user=True);print(json.dumps(meta))
