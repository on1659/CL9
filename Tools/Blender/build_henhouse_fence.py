"""Original opaque boundary fence, sized for the current 288cm wall contract."""
import sys,random,importlib
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
wood='M_CL9_Fence_Wood';metal='M_CL9_Fence_Metal'
k=StaticKit('CL9_HENHOUSE_FENCE','D:/Work/unreal/CL9/ArtSource/map/henhouse_fence','henhouse_fence.blend',
    {wood:{'vertex_color':True,'roughness':.92,'metallic':0},metal:{'vertex_color':True,'roughness':.68,'metallic':.55}})
rng=random.Random(1301)
for i in range(12):
    width=2.30/12;x=-1.15+(i+.5)*width
    v=rng.uniform(.86,1.13);color=(.22*v,.175*v,.117*v,1)
    board=k.box('VerticalPlank_'+str(i),(x,0,1.40),(width+.004,.08,2.72),color,wood,.003)
    # Broad variation on lower faces, no baked directional lighting.
    attr=board.data.color_attributes['Color']
    for poly in board.data.polygons:
        for li in poly.loop_indices:
            z=board.data.vertices[board.data.loops[li].vertex_index].co.z+board.location.z
            if z<.2:attr.data[li].color=(color[0]*.60,color[1]*.70,color[2]*.62,1)
for x in (-1.20,1.20):
    k.box('HalfEndPost',(x,0,1.44),(.10,.24,2.88),(.135,.102,.065,1),wood,0)
for z in (.48,2.22):
    k.box('BackRail',(0,.078,z),(2.30,.076,.17),(.18,.132,.077,1),wood,.006)
    for x in (-1.16,1.16):
        k.box('RailBracket',(x,-.044,z),(.10,.008,.23),(.084,.094,.09,1),metal,.002)
        for dz in (-.075,.075):k.bar('BracketBolt',(x,-.048,z+dz),(x,-.059,z+dz),.018,(.17,.175,.15,1),metal,8)
k.box('TopWeatherCap',(0,0,2.827),(2.30,.13,.106),(.185,.143,.087,1),wood,.004)
k.box('BottomRail',(0,.012,.048),(2.30,.10,.096),(.13,.115,.073,1),wood,.003)
k.finish('henhouse_fence','SM_Henhouse_Fence_250_A',None,
    {'SnapLeft':(-1.25,0,0),'SnapRight':(1.25,0,0)},
    {'rig':'N/A static architecture','tile_length_cm':250,'blocks_sight':True},
    pivot='floor center',collision_boxes=[((0,0,1.44),(2.5,.24,2.88))])
