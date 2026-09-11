import bpy,sys,math,random,importlib
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit,stretch_export
importlib.reload(static_meshkit);importlib.reload(stretch_export)
from static_meshkit import StaticKit
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_yard')
soil='M_CL9_Yard_Soil'
k=StaticKit('CL9_YARD_GROUND',ROOT/'ground','ground.blend',{soil:{'vertex_color':True,'roughness':.98,'metallic':0}})
n=10;side=n+1;count=side*side;verts=[];rng=random.Random(1801)
xy=[]
for j in range(side):
    for i in range(side):
        x=-2.5+i*.5;y=-2.5+j*.5
        if 0<i<n and 0<j<n:x+=rng.uniform(-.14,.14);y+=rng.uniform(-.14,.14)
        xy.append((x,y))
for z in (0,.295):verts.extend((x,y,z) for x,y in xy)
faces=[]
for j in range(n):
    for i in range(n):
        a=j*side+i;b=a+1;c=a+side+1;d=a+side
        faces.append((d,c,b,a));faces.extend([(a+count,b+count,c+count),(a+count,c+count,d+count)])
ring=list(range(side))+[j*side+n for j in range(1,side)]+[n*side+i for i in range(n-1,-1,-1)]+[j*side for j in range(n-1,0,-1)]
for a,b in zip(ring,ring[1:]+ring[:1]):faces.append((a,b,b+count,a+count))
data=bpy.data.meshes.new('YardSoilMesh');data.from_pydata(verts,[],faces);data.update()
o=bpy.data.objects.new('YardSoil',data);k.collection.objects.link(o);k.part(o,o.name,(.14,.105,.058,1),soil)
colors=o.data.color_attributes['Color']
for loop in o.data.loops:
    v=o.data.vertices[loop.vertex_index].co
    # Periodic color field: identical values at opposing tile edges.
    f=1+.10*math.cos(v.x*math.tau/5)+.065*math.sin(v.y*math.tau/5)+.045*math.sin((v.x+v.y)*math.tau/2.5)
    colors.data[loop.index].color=(.14*f,.105*f,.058*f,1)
for i in range(35):
    x=rng.uniform(-2.30,2.30);y=rng.uniform(-2.30,2.30);r=rng.uniform(.017,.037);v=rng.uniform(.8,1.2)
    k.bar('EmbeddedGravel',(x,y,.292),(x,y,.30),r,(.18*v,.173*v,.14*v,1),soil,6)
k.finish('henhouse_ground','SM_Henhouse_YardGround_500_A',None,
    {'Top':(0,0,.30),'Left':(-2.5,0,.30),'Right':(2.5,0,.30)},
    {'rig':'N/A static terrain tile','soil_surface_cm':29.5,'collision_top_cm':30,'max_soil_collision_gap_cm':.5},
    pivot='floor center',collision_boxes=[((0,0,.15),(5,5,.30))])
stretch_export.export_stretch(ROOT/'ground','henhouse_ground','SM_Henhouse_YardGround_Stretch100_A')

def icon_polygon(k,name,points,color,material,inset=0):
    verts=[(x,y+inset,z) for y in (-.020,-.024) for x,z in points];n=len(points)
    faces=[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
    mesh=bpy.data.meshes.new(name+'Mesh');mesh.from_pydata(verts,[],faces);mesh.update()
    o=bpy.data.objects.new(name,mesh);k.collection.objects.link(o);k.part(o,name,color,material)
def ellipse(cx,cz,rx,rz,angle=0):
    return [(cx+rx*math.cos(a)*math.cos(angle)-rz*math.sin(a)*math.sin(angle),
             cz+rx*math.cos(a)*math.sin(angle)+rz*math.sin(a)*math.cos(angle)) for a in [i*math.tau/24 for i in range(24)]]
white=(.70,.69,.57,1)
for label,facecolor in [('Exit',(.035,.13,.075,1)),('Feed',(.28,.18,.045,1)),('Kitchen',(.05,.10,.17,1)),('Coop',(.20,.073,.043,1))]:
    frame='M_CL9_Sign'+label+'_Frame';face='M_CL9_Sign'+label+'_Face'
    k=StaticKit('CL9_WAYFINDING_'+label.upper(),ROOT/('sign_'+label.lower()),'sign_'+label.lower()+'.blend',
        {frame:{'vertex_color':True,'roughness':.74,'metallic':.45},face:{'vertex_color':True,'roughness':.77,'metallic':0}})
    k.box('Frame',(0,.004,0),(.60,.032,.40),(.028,.037,.036,1),frame,.008)
    k.box('ColorFace',(0,-.0145,0),(.56,.009,.36),facecolor,face,.006)
    for x in (-.276,.276):
        for z in (-.176,.176):k.bar('Fastener',(x,-.012,z),(x,-.022,z),.008,(.18,.19,.16,1),frame,8)
    if label=='Exit':
        icon_polygon(k,'ExitArrow',[(-.18,-.035),(.055,-.035),(.055,-.09),(.19,0),(.055,.09),(.055,.035),(-.18,.035)],white,face)
    elif label=='Feed':
        icon_polygon(k,'GrainStem',[(-.007,-.13),(.007,-.13),(.007,.13),(-.007,.13)],white,face,.0005)
        for z in (-.055,.005,.065):
            for x in (-.042,.042):icon_polygon(k,'Grain',ellipse(x,z,.036,.018,.6 if x<0 else -.6),white,face)
        icon_polygon(k,'GrainTip',ellipse(0,.12,.018,.03),white,face)
    elif label=='Kitchen':
        icon_polygon(k,'PanBowl',ellipse(-.035,-.015,.082,.082),white,face)
        icon_polygon(k,'PanHandle',[(.02,-.015),(.19,.055),(.205,.035),(.035,-.04)],white,face,.0005)
        for x in (-.075,-.035,.005):icon_polygon(k,'Steam',[(x-.006,.09),(x+.006,.09),(x+.016,.14),(x+.004,.14)],white,face)
    else:
        points=[(.073*math.sin(a)*(1-.22*math.cos(a)),.125*math.cos(a)) for a in [i*math.tau/32 for i in range(32)]]
        icon_polygon(k,'Egg',points,white,face)
    k.finish('henhouse_sign_'+label.lower(),'SM_Henhouse_Sign_'+label+'_A',None,
        {'WallMount':(0,.02,0)},{'rig':'N/A static sign','collision_policy':'NoCollision wall decoration'},pivot='bounds center')
