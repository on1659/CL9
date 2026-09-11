"""Original timber-and-steel utility bench, with separate collision supports."""
import sys,importlib,random
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
wood='M_CL9_Workbench_Wood';metal='M_CL9_Workbench_Frame'
k=StaticKit('CL9_HENHOUSE_WORKBENCH','D:/Work/unreal/CL9/ArtSource/map/henhouse_workbench','henhouse_workbench.blend',
    {wood:{'vertex_color':True,'roughness':.85,'metallic':0},metal:{'vertex_color':True,'roughness':.7,'metallic':.45}})
frame=(.035,.08,.063,1);edge=(.021,.043,.037,1);bolt=(.23,.245,.20,1)
rng=random.Random(1501);collision=[]
for i in range(5):
    y=-.375+(i+.5)*.15;v=rng.uniform(.87,1.12)
    k.box('TopPlank_'+str(i),(0,y,.8625),(1.64,.15,.075),(.30*v,.205*v,.105*v,1),wood,.006)
for x in (-.86,.86):k.box('EndBreadboard',(x,0,.8625),(.08,.75,.075),(.22,.145,.072,1),wood,.004)
collision.append(((0,0,.8625),(1.8,.75,.075)))
for x in (-.755,.755):
    for y in (-.265,.265):
        k.box('SquareLeg',(x,y,.4125),(.075,.075,.825),frame,metal,.004)
        k.box('FootPlate',(x,y,.012),(.105,.105,.024),edge,metal,.003)
        collision.append(((x,y,.4125),(.105,.105,.825)))
for y in (-.265,.265):
    k.box('LongApron',(0,y,.766),(1.51,.052,.118),frame,metal,.003)
    collision.append(((0,y,.766),(1.51,.052,.118)))
    for x in (-.65,.65):
        k.bar('ApronBolt',(x,y-.029,.773),(x,y-.040,.773),.014,bolt,metal,8)
for x in (-.755,.755):
    k.box('SideApron',(x,0,.766),(.052,.53,.118),frame,metal,.003)
    collision.append(((x,0,.766),(.052,.53,.118)))
for i in range(4):
    y=-.25+(i+.5)*.125;v=rng.uniform(.86,1.06)
    k.box('ShelfPlank',(0,y,.23),(1.45,.125,.04),(.22*v,.147*v,.074*v,1),wood,.003)
collision.append(((0,0,.23),(1.45,.50,.04)))
for y in (-.245,.245):
    k.box('ShelfSteelSupport',(0,y,.195),(1.51,.04,.03),frame,metal,.002)
    collision.append(((0,y,.195),(1.51,.04,.03)))
k.finish('henhouse_workbench','SM_Henhouse_Workbench_A',None,
    {'WorkSurface':(0,0,.9),'WorkLeft':(-.5,0,.9),'WorkRight':(.5,0,.9),'Shelf':(0,0,.25)},
    {'rig':'N/A static furniture','work_surface_height_cm':90,'shelf_height_cm':25},
    pivot='floor center',collision_boxes=collision)
