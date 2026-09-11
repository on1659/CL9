"""Original hinged-origin timber exit leaf. No frame or level assembly."""
import bpy,sys,random,importlib
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
wood='M_CL9_Door_PaintedWood';metal='M_CL9_Door_Hardware'
k=StaticKit('CL9_HENHOUSE_DOOR','D:/Work/unreal/CL9/ArtSource/map/henhouse_door','henhouse_door.blend',
    {wood:{'vertex_color':True,'roughness':.86,'metallic':0},metal:{'vertex_color':True,'roughness':.62,'metallic':.45}})
W=1.3198728942871094;H=2.564242858886719
teal=(.063,.133,.118,1);edge=(.037,.082,.073,1);steel=(.061,.072,.074,1);yellow=(.47,.30,.048,1)
rng=random.Random(1401)
for i in range(7):
    width=(W-.18)/7;x=.09+(i+.5)*width;v=rng.uniform(.90,1.12)
    k.box('PaintedBoard_'+str(i),(x,0,H/2),(width+.002,.058,H-.16),tuple(c*v for c in teal[:3])+(1,),wood,.004)
for x in (.045,W-.045):k.box('DoorStile',(x,0,H/2),(.09,.072,H),edge,wood,.003)
for z in (.075,H-.075):k.box('DoorRail',(W/2,0,z),(W-.18,.072,.15),edge,wood,.004)
k.box('CenterBrace',(W/2,.041,1.00),(W-.18,.024,.13),edge,wood,.004)
for z in (.34,H-.34):
    k.box('HingeStrap',(.21,-.041,z),(.35,.014,.11),steel,metal,.004)
    k.bar('HingeBarrel',(.029,0,z-.09),(.029,0,z+.09),.028,steel,metal,12)
    for x in (.075,.32):k.bar('HingeBolt',(x,-.047,z),(x,-.060,z),.013,(.17,.18,.16,1),metal,8)
for x in (.28,W-.20):
    k.box('PushBarMount',(x,-.049,1.10),(.07,.03,.13),steel,metal,.008)
k.bar('PushBar',(.28,-.066,1.10),(W-.20,-.066,1.10),.014,yellow,metal,16)
k.box('OutsidePlate',(W-.18,.044,1.10),(.09,.016,.22),steel,metal,.006)
k.bar('OutsideFixedGrip',(W-.18,.066,1.03),(W-.18,.066,1.17),.014,steel,metal,12)
k.box('ExitSign',(W/2,-.036,1.85),(.48,.008,.22),(.035,.105,.064,1),wood,.006)
# A solid extruded direction arrow. No texture dependency and no paper-thin faces.
points=[(-.17,-.035),(.035,-.035),(.035,-.08),(.17,0),(.035,.08),(.035,.035),(-.17,.035)]
verts=[(W/2+x,y,1.85+z) for y in (-.041,-.045) for x,z in points];n=len(points)
faces=[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
data=bpy.data.meshes.new('ExitArrowMesh');data.from_pydata(verts,[],faces);data.update()
o=bpy.data.objects.new('ExitArrow',data);k.collection.objects.link(o)
k.part(o,'ExitArrow',(.66,.64,.47,1),wood)
k.finish('henhouse_door','SM_Henhouse_Door_A',None,
    {'HingeFloor':(0,0,0),'HingeUpper':(0,0,H),'PushGrip':((.28+W-.20)/2,-.066,1.10)},
    {'rig':'N/A current StaticMesh consumer has no opening animation','natural_width_cm':W*100,'current_flap_height_cm':630},
    pivot='hinge left',collision_boxes=[((W/2,0,H/2),(W,.072,H))])
