"""chicken_a v2(귀엽게 톤) 1단계 — 형태와 렌더만 만든다. 리그·클립·UE 임포트는 다음 실행에서 한다.

    py -3 Tools/Blender/run_headless.py build_chicken_a_v2.py --blend ArtSource/monster/chicken_a/chicken_a.blend

실물 chicken_a.blend 를 열어 그 파일의 카메라·조명·월드·바닥으로 '전'(실물 메시, 레스트 자세)과 '후'(v2) 렌더를 찍는다.
실물 파일은 저장하지 않는다. v2 씬만 ArtSource/monster/chicken_a/v2/chicken_a_v2.blend 로 따로 쓴다.
v2 형상은 build_chicken_a.py 의 치수를 그대로 옮긴 뒤 머리 비율·부리·볏·눈썹·색만 바꾼다. 같은 빌더로 v1 을 다시 지어
실물과 tris·치수가 같은지 먼저 확인한다(옮긴 치수가 실물과 같다는 증거).
변형안 A = 몸은 그대로 두고 머리만 키운다. B = 몸을 배 밑선 기준 0.9배로 줄이고 머리를 키운다. 눈썹은 강도 0 / 0.4 / 1.0 세 벌을 만든다.
"""
import bpy,bmesh,json,math
from pathlib import Path
from mathutils import Vector,Matrix,Euler

SRC=Path(bpy.data.filepath).resolve()
assert SRC.name=='chicken_a.blend','실물 chicken_a.blend 를 --blend 로 열어야 한다'
OUT=SRC.parent/'v2';PREV=OUT/'preview';PREV.mkdir(parents=True,exist_ok=True)
V2_BLEND=OUT/'chicken_a_v2.blend';assert V2_BLEND.resolve()!=SRC
scene=bpy.data.scenes['CL9_CHICKEN_A'];assert bpy.context.scene==scene
real=bpy.data.objects['SK_Chicken_A'];rig=bpy.data.objects['SKEL_Chicken_A']
material=real.data.materials[0]
rig.animation_data.action=None;scene.frame_set(1)  # 실물은 레스트 자세로 찍는다(v2 는 리그가 없다)

# 실물 팔레트 — build_chicken_a.py 값. v2 는 규칙 2(채도 상향)대로 강조색(볏·육수·부리·다리)의 채도·밝기를 올리고
# 몸 깃털은 색조·채도를 그대로 둔 채 밝기만 올린다(흰 닭이 누렇게 뜨지 않게).
OLD={'feather':(.64,.55,.37,1),'cream':(.9,.82,.59,1),'shadow':(.24,.20,.13,1),'red':(.42,.022,.015,1),
     'orange':(.60,.27,.032,1),'dark':(.016,.013,.011,1),'eye':(.82,.72,.45,1),'claw':(.24,.14,.045,1)}
NEW=dict(OLD,feather=(.74,.635,.43,1),red=(.80,.012,.008,1),orange=(.95,.32,.012,1),claw=(.42,.17,.02,1))

class Chicken:
    """bmesh 로 부위를 하나씩 붙인다. 부위마다 한 뼈 이름의 버텍스 그룹(가중치 1)을 달아 다음 실행의 리깅 입력으로 남긴다."""
    def __init__(s,pal):
        s.bm=bmesh.new();s.col=s.bm.loops.layers.float_color.new('Color');s.dl=s.bm.verts.layers.deform.verify()
        s.pal=pal;s.groups=[]
    def _tag(s,verts,color,bone):
        if bone not in s.groups:s.groups.append(bone)
        gi=s.groups.index(bone)
        for f in {f for v in verts for f in v.link_faces}:
            for l in f.loops:l[s.col]=s.pal[color]
        for v in verts:v[s.dl][gi]=1.0
    def ell(s,loc,scale,color,bone,seg=20,rings=12,rot=(0,0,0)):
        M=Matrix.Translation(loc)@Euler(rot).to_matrix().to_4x4()@Matrix.Diagonal((*scale,1))
        s._tag(bmesh.ops.create_uvsphere(s.bm,u_segments=seg,v_segments=rings,radius=1,matrix=M)['verts'],color,bone)
    def taper(s,a,b,r1,r2,color,bone,verts=10):
        a,b=Vector(a),Vector(b)
        M=Matrix.Translation((a+b)/2)@(b-a).to_track_quat('Z','Y').to_matrix().to_4x4()
        r=bmesh.ops.create_cone(s.bm,cap_ends=True,cap_tris=False,segments=verts,radius1=r1,radius2=r2,depth=(b-a).length,matrix=M)
        s._tag(r['verts'],color,bone)
    def object(s,name):
        me=bpy.data.meshes.new(name);s.bm.to_mesh(me);s.bm.free()
        ob=bpy.data.objects.new(name,me);scene.collection.objects.link(ob)
        for g in s.groups:ob.vertex_groups.new(name=g)
        me.materials.append(material);me.calc_loop_triangles();return ob

HC=Vector((.33,0,1.03))   # 실물 머리 중심
def build(name,pal,s_head=1.0,c_head=HC,body_k=1.0,body_pivot=(-.02,0,.28),brow_k=1.0,cute_face=False,pain=None):
    # pain = 아픈 눈 프리셋(PAINS 의 값). 주면 brow_k 대신 그 눈썹을 달고 눈·동공을 바꾼다. None 이면 이전과 똑같은 메시가 나온다.
    # body_pivot = 몸 축소 기준점. 기본값은 배 밑면 높이 — 몸을 줄여도 배 밑선이 그대로라 허벅지가 드러나지 않는다.
    # 다리(엉덩이·허벅지·정강이·발)는 축소를 받지 않는다 — 접지와 관절 위치 유지
    ch=Chicken(pal);c_head=Vector(c_head);BP=Vector(body_pivot)
    B=lambda p:tuple(BP+body_k*(Vector(p)-BP));Bs=lambda sc:tuple(body_k*x for x in sc)
    H=lambda p:tuple(c_head+s_head*(Vector(p)-HC));Hs=lambda sc:tuple(s_head*x for x in sc)
    ch.ell(B((-.06,0,.61)),Bs((.41,.31,.35)),'feather','body',24,16)
    ch.ell(B((.16,0,.68)),Bs((.27,.27,.30)),'cream','body')
    ch.ell(B((.24,0,.87)),Bs((.17,.17,.25)),'cream','neck')
    ch.ell(H((.33,0,1.03)),Hs((.19,.17,.19)),'cream','head')
    if not cute_face:
        ch.taper(H((.455,0,1.015)),H((.68,0,.974)),.092*s_head,.009*s_head,'orange','head',8)
        ch.taper(H((.44,0,.947)),H((.643,0,.959)),.060*s_head,.009*s_head,'claw','jaw',8)
    else:
        # 짧고 뭉툭한 부리 — 실물 길이의 약 절반. 끝을 둥근 마개로 막고 마개를 아래로 내려 끝이 안쪽으로 말려 들어간 듯하게
        ch.taper(H((.44,0,1.02)),H((.545,0,.988)),.085*s_head,.046*s_head,'orange','head',10)
        ch.ell(H((.545,0,.975)),Hs((.042,.046,.044)),'orange','head',12,8)
        ch.taper(H((.43,0,.95)),H((.515,0,.952)),.055*s_head,.030*s_head,'claw','jaw',10)
        ch.ell(H((.515,0,.950)),Hs((.028,.031,.026)),'claw','jaw',10,6)
    for y,side in [(-1,'r'),(1,'l')]:
        ch.ell(H((.403,y*.145,1.078)),Hs((.081,.039,.071)),'shadow','head',16,10)
        sq=pain['eye_squash'] if pain else 1.0;pk=pain['pupil_k'] if pain else 1.0  # 눈 세로 배율, 동공 배율
        ch.ell(H((.421,y*.17,1.08)),Hs((.052,.016,.047*sq)),'eye','head',16,10)
        ch.ell(H((.434,y*.184,1.08)),Hs((.023*pk,.009,.030*pk*sq)),'dark','head',12,8)
        if pain:
            # 아픈 눈썹 — 부리 쪽(안쪽) 끝을 올리고 뒤쪽(바깥) 끝을 내린다. 화난 눈(k=1, 앞 끝이 내려감)과 기울기가 반대다.
            # 가운데 높이 = 실물 눈썹 가운데(1.1255)+raise, 양끝은 가운데에서 ±tilt. 두께 배율 th, 머리 옆면 쪽으로 inset 만큼 들인다.
            # 앞 끝 x 는 front_x(실물 .471) — 올라간 앞 끝이 눈 소켓 윗면에 걸치도록 뒤로 당긴다(허공에 뜬 막대처럼 보이지 않게).
            zc=1.1255+pain['raise'];t=pain['tilt'];inset=pain['inset']
            a=(.355,y*(.176-inset),zc-t);b=(pain['front_x'],y*(.163-inset),zc+t)
            ch.taper(H(a),H(b),.024*s_head*pain['th'],.018*s_head*pain['th'],'cream','head')
            if pain['lid']>0:
                # 윗눈꺼풀 — 눈과 같은 크기의 머리색 타원을 위로 올려 눈 윗부분을 덮는다. lid = 덮는 비율(눈 높이 기준).
                # 아랫선이 아래로 볼록한 곡선이라 감기다 만 눈꺼풀처럼 보인다. 옆 두께는 눈 소켓 바깥면(y .184) 근처까지만(.020, 중심 y .168) — 더 두꺼우면 공처럼 튀어나온다.
                # 그래서 동공 윗끝이 눈꺼풀 아랫선 밖으로 살짝 비치는데, 눈꺼풀 밑으로 동공이 보이는 것처럼 읽혀 그대로 둔다.
                ch.ell(H((.423,y*.168,1.08+.094*(1-pain['lid']))),Hs((.056,.020,.047)),'cream','head',16,10)
        elif brow_k>0:
            # 눈썹 강도 k — 1.0 = 실물(눈 윗면을 덮는 낮은 쐐기). k 가 줄면 앞쪽으로 내려가는 기울기가 k 배,
            # 두께가 (0.5+0.5k) 배로 줄고, (1-k)×1.5cm 올라가 눈꺼풀을 덜 덮는다. 올라간 만큼 머리 옆면에 붙도록 (1-k)×2.5cm 안쪽으로 들인다
            # (머리 밖으로 뜬 막대처럼 보이지 않게).
            lift=(1-brow_k)*.015;inset=(1-brow_k)*.025;th=.5+.5*brow_k
            a=(.355,y*(.176-inset),1.132+lift);b=(.471,y*(.163-inset),1.132+lift-.013*brow_k)
            ch.taper(H(a),H(b),.024*s_head*th,.018*s_head*th,'cream','head')
        ch.ell(H((.412,y*.052,.866)),Hs((.047,.036,.086)),'red','jaw',12,10)
        ch.ell(B((-.08,y*.292,.64)),Bs((.28,.082,.19)),'cream','wing_'+side,rot=(0,-.3,0))
        for i in range(5):
            ch.ell(B((-.17-i*.036,y*(.306+i*.007),.60-i*.031)),Bs((.18,.044,.048)),'feather' if i%2 else 'cream',
                   'wing_tip_'+side,12,8,rot=(0,-.3+i*.04,0))
        hip=(-.075,y*.145,.47);knee=(.045,y*.145,.255);ankle=(-.055,y*.145,.065)
        ch.ell(hip,(.105,.093,.14),'feather','thigh_'+side,16,10)
        ch.taper(hip,knee,.068,.044,'feather','thigh_'+side)
        ch.taper(knee,ankle,.037,.026,'orange','shin_'+side)
        ch.ell(ankle,(.039,.033,.031),'orange','foot_'+side,12,8)
        for j in (-1,0,1):
            tip=(.15 if j==0 else .12,y*.145+j*.065,.023)
            ch.taper(ankle,tip,.025,.014,'orange','foot_'+side,8)
            ch.taper(tip,(tip[0]+.032,tip[1],.012),.015,.002,'claw','foot_'+side,8)
        ch.taper(ankle,(-.14,y*.145,.025),.022,.010,'orange','foot_'+side,8)
    if not cute_face:
        for i in range(5):
            ch.ell(H((.18+i*.064,0,1.18+(.06 if i in (1,2,3) else .018))),Hs((.054,.037,.102)),'red','comb',12,8)
    else:
        # 볏 — 로브 5→4개, 세로 반지름 .102→.068 로 낮추고 둥글게. 머리 윗면에 반쯤 묻히게 놓는다
        for i,x in enumerate((.22,.28,.34,.40)):
            top=1.03+.19*math.sqrt(max(0,1-((x-.33)/.19)**2))
            ch.ell(H((x-.01,0,top+(.032 if i in (1,2) else .018))),Hs((.052,.034,.068)),'red','comb',12,8)
    for i in range(5):
        ch.ell(B((-.44-abs(i-2)*.02,(i-2)*.064,.85+(.055 if i==2 else 0))),Bs((.11,.047,.24)),'shadow' if i%2 else 'feather',
               'tail',14,10,rot=(0,-.50+(i-2)*.07,0))
    ob=ch.object(name)
    zs=[v.co.z for v in ob.data.vertices]
    info={'triangles':len(ob.data.loop_triangles),'vertices':len(ob.data.vertices),
          'dimensions_cm':[round(x*100,2) for x in ob.dimensions],'min_z_cm':round(min(zs)*100,3),'max_z_cm':round(max(zs)*100,2),
          'head_diameter_cm':{'z':round(38*s_head,1),'y':round(34*s_head,1)},
          'body_diameter_cm':{'z':round(70*body_k,1),'y':round(62*body_k,1)},
          'head_to_body':{'z':round(38*s_head/(70*body_k),3),'y':round(34*s_head/(62*body_k),3)},
          'head_scale':s_head,'head_center_m':list(c_head),'body_scale':body_k,'brow_k':brow_k}
    return ob,info

# 아픈 눈(피격) 프리셋 후보 — 눈썹은 셋이 같고 눈 처리만 다르다. 이 파일의 본 실행은 쓰지 않고,
# build_chicken_a_v2_pain.py 가 chicken_a_v2.blend 에 메시를 더하고 비교 렌더를 찍는다(2026-09-30).
PAIN_BROW={'raise':-.005,'tilt':.025,'inset':.005,'th':.8,'front_x':.455}
PAINS={'pain_a':dict(PAIN_BROW,pupil_k=.6,eye_squash=1.0,lid=0),    # 동공만 줄인다(놀라 움츠린 눈)
       'pain_b':dict(PAIN_BROW,pupil_k=.8,eye_squash=.5,lid=0),     # 눈 전체를 세로로 찌그러뜨린다(찡그린 눈)
       'pain_c':dict(PAIN_BROW,pupil_k=.75,eye_squash=1.0,lid=.45)} # 윗눈꺼풀로 윗부분을 덮는다(반쯤 감긴 눈)

# 1) 빌더 검증 — v1 을 다시 지어 실물과 비교
real.data.calc_loop_triangles()
check,check_info=build('CHK_V1_Rebuild',OLD)
real_info={'triangles':len(real.data.loop_triangles),'vertices':len(real.data.vertices),'dimensions_cm':[round(x*100,2) for x in real.dimensions]}
print('REAL',json.dumps(real_info));print('REBUILD',json.dumps(check_info))
real_colors=sorted({tuple(round(c,3) for c in d.color) for d in real.data.color_attributes['Color'].data})
print('REAL_COLORS',real_colors)

# 2) v2 변형안 — A: 몸 그대로·머리 1.42배, B: 몸 0.9배·머리 1.30배. 머리:몸 지름비 둘 다 0.77~0.79
VARIANTS={'A':dict(s_head=1.42,c_head=(.35,0,1.02),body_k=1.0),'B':dict(s_head=1.30,c_head=(.32,0,.95),body_k=.9)}
BROWS={'brow000':0.0,'brow040':0.4,'brow100':1.0}
v2={};measure={'real':real_info,'v1_rebuild':check_info,'variants':{}}
for vk,vp in VARIANTS.items():
    for bk,bv in BROWS.items():
        name=f'SK_Chicken_A_v2{vk}_{bk}'
        ob,info=build(name,NEW,brow_k=bv,cute_face=True,**vp);v2[(vk,bk)]=ob;measure['variants'][name]=info
        print(name,json.dumps(info))

# 3) 렌더 — neutral.png 와 같은 카메라(3/4 정사영)·조명·월드·바닥. 옆모습·머리 근접 카메라를 더한다
# 실물 파일은 라이브러리 쓰기로 저장돼 씬 카메라 연결이 비어 있다 — preview_chicken_a.py 가 만든 오브젝트를 이름으로 찾는다
print('SCENE_OBJECTS',sorted(o.name for o in scene.objects))
for n in ('CL9_CHICKEN_Camera','CL9_CHICKEN_Key','CL9_CHICKEN_Fill','CL9_CHICKEN_PreviewGround'):
    if n not in scene.objects:scene.collection.objects.link(bpy.data.objects[n])
main_cam=bpy.data.objects['CL9_CHICKEN_Camera'];scene.world=scene.world or bpy.data.worlds['CL9_CHICKEN_World']
print('CAM',tuple(main_cam.location),main_cam.data.type,main_cam.data.ortho_scale,'WORLD',scene.world.name,
      'VIEW',scene.view_settings.view_transform,'ENGINE',scene.render.engine)
def ortho_cam(name,loc,target,scale):
    data=bpy.data.cameras.new(name);data.type='ORTHO';data.ortho_scale=scale
    ob=bpy.data.objects.new(name,data);scene.collection.objects.link(ob);place(ob,loc,target);return ob
def place(ob,loc,target):
    ob.location=loc;ob.rotation_euler=(Vector(target)-Vector(loc)).to_track_quat('-Z','Y').to_euler()
side_cam=ortho_cam('CL9_CHICKEN_V2_Side',(.12,-4.5,.72),(.12,0,.72),1.95)
close_cam=ortho_cam('CL9_CHICKEN_V2_Close',(0,0,0),(0,0,1),1.0)
view_dir=(Vector((0,0,.65))-main_cam.location).normalized()
everyone=[real,check,*v2.values()]
def shot(target,cam,fname):
    for ob in everyone:ob.hide_render=ob is not target
    scene.camera=cam;scene.render.filepath=str(PREV/fname);bpy.ops.render.render(write_still=True)
scene.render.image_settings.file_format='PNG';scene.render.resolution_x=scene.render.resolution_y=1000;scene.cycles.samples=32
shot(real,main_cam,'real_34.png');shot(real,side_cam,'real_side.png')
for vk in VARIANTS:
    shot(v2[(vk,'brow100')],main_cam,f'v2{vk}_34.png');shot(v2[(vk,'brow100')],side_cam,f'v2{vk}_side.png')
    head=Vector(VARIANTS[vk]['c_head'])+Vector((.06,0,.04))
    place(close_cam,head-view_dir*5,head)
    for bk in BROWS:shot(v2[(vk,bk)],close_cam,f'v2{vk}_{bk}_close.png')
scene.camera=main_cam
(OUT/'measure.json').write_text(json.dumps(measure,indent=2,ensure_ascii=False),encoding='utf-8')

# 4) v2 만 따로 저장 — 실물 파일은 건드리지 않는다. 새 씬을 libraries.write 로 쓰면 Blender 5.2.1 이 뷰 레이어 복사에서
#    죽는다(EXCEPTION_ACCESS_VIOLATION, BKE_view_layer_copy_data) — 메모리에서 실물 메시·리그·액션을 지우고 사본 저장(copy=True)으로
#    다른 경로에 쓴다. 열린 원본 경로는 바뀌지 않고 원본 파일에는 아무것도 쓰지 않는다.
for ob in (check,real,rig):bpy.data.objects.remove(ob)
for act in list(bpy.data.actions):bpy.data.actions.remove(act)
for ob in v2.values():ob.hide_render=ob.name!='SK_Chicken_A_v2A_brow100';ob.hide_viewport=ob.hide_render
scene.name='CL9_CHICKEN_A_V2';scene.camera=main_cam
bpy.ops.wm.save_as_mainfile(filepath=str(V2_BLEND),copy=True)
print('V2_BLEND',V2_BLEND)
