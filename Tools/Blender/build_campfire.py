"""모닥불 SM_Campfire_A — 돌 테두리·재·장작 네 개·저폴리 불꽃 원뿔 셋. 마당 모듈 소품(ModuleTable PropSets)으로 놓이고
빛은 소품 빛 컬럼(LightIntensity·LightRadius·LightColor·LightOffset)이 붙인다. 불꽃은 발광 재질 슬롯(FlameStrength)이다.
헤드리스: "C:/Program Files/Blender Foundation/Blender 5.2/blender.exe" -b -P Tools/Blender/build_campfire.py
"""
import bpy,sys,math,random,importlib
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
body='M_CL9_Campfire_Body';flame='M_CL9_Campfire_Flame'
k=StaticKit('CL9_CAMPFIRE_A','D:/Work/unreal/CL9/ArtSource/map/campfire','campfire.blend',
    {body:{'vertex_color':True,'roughness':.88,'metallic':0},
     flame:{'vertex_color':True,'roughness':1,'metallic':0,
            'emission_color':[1,.52,.16,1],'emission_parameter':'FlameStrength','emission_default':3,'preview_on_strength':3}})
rng=random.Random(2026)
stone=[(.30,.29,.27,1),(.24,.23,.22,1),(.34,.32,.29,1)];ash=(.09,.085,.08,1)
bark=(.23,.13,.06,1);char=(.05,.04,.035,1);ember=(.55,.14,.03,1)
yellow=(1,.85,.25,1);orange=(1,.45,.08,1);red=(.85,.18,.04,1)

# 돌 테두리 — 열 개, 반지름 0.46 m, 조금씩 다른 크기·기울기
for i in range(10):
    a=i*math.tau/10+rng.uniform(-.08,.08);r=.46+rng.uniform(-.02,.02)
    sx=rng.uniform(.17,.22);sy=rng.uniform(.13,.16);sz=rng.uniform(.12,.16)
    o=k.box('Stone',(r*math.cos(a),r*math.sin(a),sz*.42),(sx,sy,sz),rng.choice(stone),body,.012)
    o.rotation_euler=(rng.uniform(-.12,.12),rng.uniform(-.12,.12),a)
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=False)
# 재 바닥 — 낮은 원반
k.bar('AshBed',(0,0,0.0),(0,0,.035),.34,ash,body,20)
# 장작 — 다섯 개가 제각각 기울어 얽힌다(가지런한 티피가 아니라). 중심 쪽 끝은 그을음.
for i in range(5):
    a=i*math.tau/5+rng.uniform(-.25,.25);r0=rng.uniform(.30,.40);x=r0*math.cos(a);y=r0*math.sin(a)
    top=(rng.uniform(-.10,.10),rng.uniform(-.10,.10),rng.uniform(.17,.30));rad=rng.uniform(.040,.058)
    k.bar('Log',(x,y,.06),top,rad,bark,body,10)
    k.bar('LogChar',((x+top[0])/2,(y+top[1])/2,(.06+top[2])/2),top,rad+.003,char,body,10)
# 잉걸불 — 재 위 덩어리 넷 + 발광 잉걸 원반(불꽃 밑 주황 빛)
for i in range(4):
    a=i*math.tau/4+1.1
    k.box('Ember',(.14*math.cos(a),.14*math.sin(a),.06),(.09,.07,.05),ember,body,.006)
k.bar('EmberGlow',(0,0,.035),(0,0,.075),.20,(1,.42,.10,1),flame,20)

# 불꽃 — 혀 일곱 개: 높이·반지름·기울기가 제각각. 정점 색은 밑동 노랑 → 중간 주황 → 끝 검붉음,
# 알파는 높이 비율(재질의 흔들림 가중치 — 밑동은 고정, 끝은 크게 흔들린다).
def tongue(name,r,h,cx,cy,z,tilt,yaw,verts=7):
    bpy.ops.mesh.primitive_cone_add(vertices=verts,radius1=r,radius2=.008,depth=h,location=(cx,cy,z+h/2))
    o=bpy.context.object;o.rotation_euler=(tilt,rng.uniform(-.08,.08),yaw)
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
    o=k.part(o,name,yellow,flame)
    attr=o.data.color_attributes['Color'];zs=[v.co.z for v in o.data.vertices];lo=min(zs);hi=max(zs)
    for l,c in zip(o.data.loops,attr.data):
        t=(o.data.vertices[l.vertex_index].co.z-lo)/max(hi-lo,1e-6)
        if t<.45:u=t/.45;col=[yellow[j]*(1-u)+orange[j]*u for j in range(3)]
        else:u=(t-.45)/.55;col=[orange[j]*(1-u)+red[j]*u for j in range(3)]
        c.color=(col[0],col[1],col[2],t)
    return o
tongue('FlameCore',.11,.42,0,0,.16,.04,0)
tongue('FlameA',.085,.34,.07,.03,.15,-.18,.9)
tongue('FlameB',.075,.30,-.06,.06,.15,.16,2.1)
tongue('FlameC',.07,.26,.02,-.08,.14,.12,3.6)
tongue('FlameD',.055,.22,-.08,-.04,.13,-.14,4.8)
tongue('FlameE',.05,.20,.09,-.06,.13,.20,5.5)
tongue('FlameF',.045,.17,-.02,.10,.12,-.22,1.6)

# 콜리전은 돌 테두리 높이의 상자 하나 — 불꽃은 막지 않는다(시야·빛 트레이스가 넘어간다)
k.finish('campfire','SM_Campfire_A',None,{'Floor':(0,0,0),'LightOrigin':(0,0,.40)},
         {'rig':'N/A static prop','states':{'off':{'FlameStrength':0},'on_preview':{'FlameStrength':2}},'flame_material':'unlit, emissive = vertex color x FlameStrength, WPO sway by vertex alpha (Tools/Unreal/import_campfire.py)'},
         pivot='floor center',collision_boxes=[((0,0,.09),(1.06,1.06,.18))])
