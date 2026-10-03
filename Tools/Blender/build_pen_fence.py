"""Open wire fence for the pen seg kind - chest high, animals stay visible through the mesh."""
import sys,random,importlib
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
wood='M_CL9_Pen_Wood';wire='M_CL9_Pen_Wire'
k=StaticKit('CL9_PEN_FENCE','D:/Work/unreal/CL9/ArtSource/map/pen_fence','pen_fence.blend',
    {wood:{'vertex_color':True,'roughness':.94,'metallic':0},wire:{'vertex_color':True,'roughness':.55,'metallic':.72}})
rng=random.Random(2903)
# 나무 기둥 - 끝 기둥은 반 폭이라 두 세그먼트가 붙으면 기둥 하나가 된다.
for x in (-1.20,1.20):
    v=rng.uniform(.92,1.08)
    k.box('HalfEndPost',(x,0,.65),(.10,.20,1.30),(.150*v,.100*v,.048*v,1),wood,.005)
k.box('MidPost',(0,0,.63),(.14,.20,1.26),(.158,.106,.052,1),wood,.005)
# 가로 보강대 - 철망을 위아래에서 문다. 바운드 안쪽(Y +-0.07)에 둔다.
for z,h in ((1.20,.14),(.09,.14)):
    k.box('Rail',(0,-.008,z),(2.40,.12,h),(.200,.140,.070,1),wood,.006)
# 철망 - 세로 철사는 칸(bay)마다 8줄, 가로 철사는 세그먼트를 관통해 7줄.
for side in (-1,1):
    for i in range(8):
        x=side*(.07+(i+.5)*.135)
        v=rng.uniform(.88,1.12)
        k.box('WireV',(x,.045,.645),(.014,.014,.97),(.055*v,.062*v,.070*v,1),wire,0)
for i in range(7):
    z=.20+i*.15
    k.box('WireH',(0,.032,z),(2.30,.012,.014),(.050,.057,.066,1),wire,0)
# 사선 버팀대 - 실루엣용. 철망보다 앞(-Y)에 놓아 격자가 가려지지 않게 한다.
for side in (-1,1):
    k.bar('Brace',(side*1.13,-.035,.18),(side*.09,-.035,1.10),.022,(.172,.118,.058,1),wood,6)
# 철망 고정 꺾쇠 - 기둥과 보강대가 만나는 6곳.
for x in (-1.16,0,1.16):
    for z in (1.16,.13):
        k.box('WireStaple',(x,.045,z),(.09,.026,.05),(.088,.096,.102,1),wire,.003)
k.finish('pen_fence','SM_Pen_Fence_250_A',None,
    {'SnapLeft':(-1.25,0,0),'SnapRight':(1.25,0,0)},
    {'rig':'N/A static architecture','tile_length_cm':250,'blocks_sight':False,'height_cm':130},
    pivot='floor center',collision_boxes=[((0,0,.65),(2.5,.20,1.30))])
