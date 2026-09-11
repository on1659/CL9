import bpy,sys,importlib,math
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/truck_parts')
def newkit(kind):
 names=['M_CL9_Truck'+kind+'_'+m for m in ['Steel','Green','Mark']]
 colors=[(.045,.057,.061,1),(.042,.14,.09,1),(.64,.56,.34,1)]
 return StaticKit('CL9_TRUCK_'+kind.upper(),ROOT/kind.lower(),kind.lower()+'.blend',
  {m:{'vertex_color':False,'base_color':c,'roughness':.79,'metallic':.5 if i==0 else .1} for i,(m,c) in enumerate(zip(names,colors))}),names,colors
def meshpart(k,name,verts,faces,mat,color):
 data=bpy.data.meshes.new(name);data.from_pydata(verts,[],faces);data.update()
 o=bpy.data.objects.new(name,data);k.collection.objects.link(o);return k.part(o,name,color,mat)
def solid_xy(k,name,points,z0,z1,mat,color):
 n=len(points);verts=[(x,y,z) for z in (z0,z1) for x,y in points]
 return meshpart(k,name,verts,[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)],mat,color)
k,m,c=newkit('Pad')
def ring(r,cut):return [(-r+cut,-r),(r-cut,-r),(r,-r+cut),(r,r-cut),(r-cut,r),(-r+cut,r),(-r,r-cut),(-r,-r+cut)]
outer=ring(2,.24);inner=ring(1.70,.15)
verts=[(x,y,0) for x,y in outer]+[(x,y,.199) for x,y in inner]
body=meshpart(k,'SlopedPad',verts,[tuple(range(7,-1,-1)),tuple(range(8,16))]+[(i,(i+1)%8,(i+1)%8+8,i+8) for i in range(8)],m[0],c[0])
body.data.materials.append(k.mats[m[1]]);body.data.polygons[1].material_index=1
cross=[(-.16,-.64),(.16,-.64),(.16,-.16),(.64,-.16),(.64,.16),(.16,.16),(.16,.64),(-.16,.64),(-.16,.16),(-.64,.16),(-.64,-.16),(-.16,-.16)]
solid_xy(k,'RecoveryMark',cross,.199,.2,m[2],c[2])
for axis in (0,1):
 for side in (-1,1):
  for j in range(7):
   center=[-.90+j*.30,side*1.51,.1995];size=[.18,.065,.001]
   if axis:center[0],center[1]=center[1],center[0];size[0],size[1]=size[1],size[0]
   k.box('EdgeMarker',center,size,c[2],m[2],0)
for x in (-1.22,1.22):
 for i in range(11):k.box('GripTread',(x,-1.05+i*.21,.1995),(.20,.018,.001),c[0],m[0],0)
for x in (-1.53,1.53):
 for y in (-1.53,1.53):k.bar('DeckBolt',(x,y,.197),(x,y,.20),.025,c[0],m[0],8)
collision=[(x,y,0) for x,y in outer]+[(x,y,.20) for x,y in inner]
k.finish('truck_pad','SM_TruckPad_A',None,{'StandingTop':(0,0,.2),'HealZoneCenter':(0,0,1.2)},
 {'rig':'N/A fixed recovery platform','top_height_cm':20,'unmarked_deck_height_cm':19.9,'surface_collision_gap_cm':.1},
 pivot='floor center',collision_convexes=[collision])
k,m,c=newkit('Panel')
k.box('ProtectiveBody',(-.01,0,.6),(.13,.80,1.2),c[0],m[0],.035)
k.box('GreenFascia',(.054,0,.64),(.01,.69,1.04),c[1],m[1],.018)
k.box('DisplayBezel',(.061,0,.81),(.012,.59,.55),c[0],m[0],.016)
k.box('DisplayFace',(.068,0,.81),(.004,.52,.48),c[1],m[1],.01)
def icon(name,pts,x=.071):
 n=len(pts);vs=[(xx,y,z) for xx in (x,x+.003) for y,z in pts]
 return meshpart(k,name,vs,[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)],m[2],c[2])
icon('ShopBasket',[(-.16,.93),(-.12,.93),(-.09,.75),(.15,.75),(.18,.88),(.145,.88),(.12,.785),(-.062,.785),(-.088,.93)])
icon('BasketHandle',[(-.16,.93),(-.21,.93),(-.21,.95),(-.16,.95)])
for y in (-.055,.12):k.bar('BasketWheel',(.071,y,.70),(.074,y,.70),.025,c[2],m[2],12)
icon('EquipmentPlus',[(-.012,.85),(.012,.85),(.012,.90),(.065,.90),(.065,.924),(.012,.924),(.012,.972),(-.012,.972),(-.012,.924),(-.065,.924),(-.065,.90),(-.012,.90)])
for y in (-.20,0,.20):
 k.bar('ButtonBezel',(.059,y,.38),(.066,y,.38),.066,c[0],m[0],16)
 k.bar('FixedButton',(.066,y,.38),(.075,y,.38),.045,c[2],m[2],16)
for y in (-.28,.28):
 for z in (.16,1.10):k.bar('FaceBolt',(.06,y,z),(.074,y,z),.013,c[2],m[2],8)
k.box('LowerPullRecess',(.061,0,.18),(.011,.27,.048),c[0],m[0],.008)
k.finish('truck_panel','SM_TruckPanel_A',None,{'WallMount':(-.075,0,.6),'UsePoint':(.075,0,.60)},
 {'rig':'N/A fixed control panel; real shop opens through BP'},pivot='floor center',collision_boxes=[((0,0,.6),(.15,.8,1.2))])
