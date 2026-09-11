"""Original low-poly ceramic return vessel; open mouth and segmented hulls."""
import bpy,math,sys,importlib
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
clay='M_CL9_ReturnJar_Clay';glaze='M_CL9_ReturnJar_Glaze';mark='M_CL9_ReturnJar_Mark'
colors={clay:(.23,.115,.056,1),glaze:(.035,.115,.075,1),mark:(.65,.59,.39,1)}
k=StaticKit('CL9_RETURN_JAR','D:/Work/unreal/CL9/ArtSource/map/return_jar','return_jar.blend',
 {m:{'vertex_color':False,'base_color':c,'roughness':.38 if m==glaze else .82,'metallic':0} for m,c in colors.items()})
profile=[(.33,0),(.36,.04),(.43,.22),(.46,.48),(.45,.62),(.40,.77),(.34,.84),(.34,.92),(.36,.96),(.35,1),
         (.30,1),(.29,.96),(.30,.92),(.30,.85),(.36,.76),(.40,.62),(.41,.48),(.38,.23),(.30,.09)]
n=32;verts=[(r*math.cos(i*math.tau/n),r*math.sin(i*math.tau/n),z) for r,z in profile for i in range(n)]
faces=[]
for j in range(len(profile)-1):
 for i in range(n):faces.append((j*n+i,j*n+(i+1)%n,(j+1)*n+(i+1)%n,(j+1)*n+i))
faces.extend([tuple(range(n-1,-1,-1)),tuple((len(profile)-1)*n+i for i in range(n))])
mesh=bpy.data.meshes.new('ReturnJarShell');mesh.from_pydata(verts,[],faces);mesh.update()
o=bpy.data.objects.new('CeramicShell',mesh);k.collection.objects.link(o);k.part(o,o.name,colors[clay],clay)
o.data.materials.append(k.mats[glaze])
for p in o.data.polygons:
 if sum(o.data.vertices[i].co.z for i in p.vertices)/len(p.vertices)>.77:p.material_index=1
# Solid loop handles stay within a one-metre footprint; they are fixed ceramic.
for side in (-1,1):
 for i in range(12):
  a=i*math.tau/12;b=(i+1)*math.tau/12
  k.bar('CeramicHandle',(0,side*(.42+.055*math.cos(a)),.74+.095*math.sin(a)),
        (0,side*(.42+.055*math.cos(b)),.74+.095*math.sin(b)),.025,colors[glaze],glaze,8)
k.box('ReturnPlaque',(.46,0,.48),(.028,.37,.30),colors[glaze],glaze,.012)
# One closed extruded arrow, downward into a receiving tray. Front normal +X.
def icon(name,points,x=.478):
 vs=[(xx,y,z) for xx in (x,x+.006) for y,z in points];count=len(points)
 fs=[tuple(range(count-1,-1,-1)),tuple(range(count,2*count))]
 fs.extend((i,(i+1)%count,(i+1)%count+count,i+count) for i in range(count))
 data=bpy.data.meshes.new(name);data.from_pydata(vs,[],fs);data.update()
 ob=bpy.data.objects.new(name,data);k.collection.objects.link(ob);k.part(ob,name,colors[mark],mark)
icon('ReturnArrow',[(-.026,.58),(.026,.58),(.026,.49),(.076,.49),(0,.415),(-.076,.49),(-.026,.49)])
icon('ReceivingTray',[(-.11,.455),(-.087,.455),(-.087,.386),(.087,.386),(.087,.455),(.11,.455),(.11,.362),(-.11,.362)])
# Convex walls retain a central opening; each hull is the convex envelope of one sector.
hulls=[]
for i in range(16):
 angles=[(i+t)*math.tau/16 for t in (0,.5,1)]
 hulls.append([(r*math.cos(a),r*math.sin(a),z) for r,z in profile for a in angles])
hulls.append([(.33*math.cos(i*math.tau/16),.33*math.sin(i*math.tau/16),z) for z in (0,.09) for i in range(16)])
k.finish('return_jar','SM_ReturnJar_A',None,{'Mouth':(0,0,1),'InteriorFloor':(0,0,.09),'InteractionFront':(.50,0,.50)},
 {'rig':'N/A fixed vessel; return consumes held item, no moving lid','mouth_clear_diameter_cm':58,
  'collision_note':'16 convex shell sectors plus base; inner curved wall conservatively approximated'},
 pivot='floor center',collision_convexes=hulls)
