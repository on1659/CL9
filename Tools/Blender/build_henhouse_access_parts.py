import bpy,sys,math,importlib
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit,stretch_export
importlib.reload(static_meshkit);importlib.reload(stretch_export)
from static_meshkit import StaticKit
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_access_parts')
for cm in (250,125):
    w=cm/100;label='wall'+str(cm)
    wood='M_CL9_'+label+'_Wood';trim='M_CL9_'+label+'_DarkTimber'
    specs={wood:{'vertex_color':False,'base_color':[.19,.115,.066,1],'roughness':.8,'metallic':0},
        trim:{'vertex_color':False,'base_color':[.071,.048,.03,1],'roughness':.8,'metallic':0}}
    k=StaticKit('CL9_HENHOUSE_WALL_'+str(cm),ROOT/label,label+'.blend',specs)
    k.box('WallCore',(0,0,2),(w-.4,.22,3.48),(1,1,1,1),wood,.008)
    for i in range(int(w/.2)):
        k.box('Board',(-w/2+.1+i*.2,-.126,2),(.18,.07,3.98),(1,1,1,1),wood,.008)
    for x in (-w/2+.1,w/2-.1):k.box('EdgeStud',(x,0,2),(.2,.4,4),(1,1,1,1),trim,.012)
    for z in (.13,2,3.87):k.box('Rail',(0,0,z),(w-.4,.4,.26),(1,1,1,1),trim,.012)
    k.finish('henhouse_'+label,'SM_Henhouse_Wall_'+str(cm)+'_A',None,
        {'Left':(-w/2,0,0),'Right':(w/2,0,0)},{'rig':'N/A static wall'},pivot='floor center',collision_boxes=[((0,0,2),(w,.4,4))])
width=2*1.3198728942871094;depth=8.5/31;thick=.3
body='M_CL9_Tread_Concrete';steel='M_CL9_Tread_Surface'
k=StaticKit('CL9_HENHOUSE_TREAD',ROOT/'tread','tread.blend',
    {body:{'vertex_color':True,'roughness':.94,'metallic':0},steel:{'vertex_color':True,'roughness':.83,'metallic':.25}})
k.box('TreadBody',(0,.0005,.143),(width,depth-.001,.286),(.16,.171,.156,1),body,.004)
k.box('GripSurface',(0,0,.293),(width,depth,.014),(.039,.047,.044,1),steel,.001)
# Safety band lies inside the fixed top bound rather than protruding past the step.
for x in (-width/2+.12,0,width/2-.12):
    k.box('NoseMarker',(x,-depth/2+.0005,.258),(.16,.001,.035),(.48,.32,.061,1),steel,.0002)
k.finish('henhouse_tread','SM_Henhouse_Tread_A',None,{'Top':(0,0,.3)},
    {'rig':'N/A static step','calculated_tread_depth_cm':depth*100,'calculated_rise_cm':610/31},
    pivot='floor center',collision_boxes=[((0,0,.15),(width,depth,.3))])
stretch_export.export_stretch(ROOT/'tread','henhouse_tread','SM_Henhouse_Tread_Stretch100_YRun_A')
stretch_export.export_stretch(ROOT/'tread','henhouse_tread','SM_Henhouse_Tread_Stretch100_XRun_A',math.pi/2)
