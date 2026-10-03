"""raccoon_a "귀엽게" 1단계(형태+렌더) — 실행 monster-cute-raccoon.

    py -3 Tools/Blender/run_headless.py build_raccoon_a_v2.py --blend ArtSource/monster/raccoon_a/raccoon_a.blend

원본 raccoon_a.blend 를 열어 메모리에서만 고치고, 결과는 ArtSource/monster/raccoon_a_v2/ 에만 쓴다.
원본 blend·export/ 는 저장하지 않는다(save_mainfile 호출 없음, 쓰기 경로 assert).
리그(뼈 위치)·액션은 건드리지 않는다 — 2단계(리그·클립) 몫. 머리를 키웠으므로 귀 뼈는 귀 메시보다 낮게 남는다.

바꾸는 것(brief 1~7):
  1 머리 부위(head·jaw·ear 가중 섬) 전체를 head 뼈 머리점 기준 HEAD_SCALE 배 — 몸통·다리·꼬리는 그대로.
  2 주둥이(Muzzle·LowerJaw) X 를 SNOUT_X 배로 줄이고 단면을 SNOUT_YZ 배로 — 끝이 뭉툭해진다. 코는 모양 그대로 따라 들어간다.
  3 귀 껍질 Z 를 EAR_Z 배(바닥 기준), Y 를 EAR_Y 배 — 끝이 더 둥글다.
  4 발톱 끝 고리를 뿌리 고리 반경의 CLAW_TIP 배로 벌리고(뭉툭) 섬 전체를 CLAW_SCALE 배(뿌리 기준).
  5 마스크·줄무늬 꼬리 형태는 손대지 않는다(색은 7의 팔레트만).
  6 눈썹 3단계 — 100%(현재), 50%(들어 올리고 안쪽 끝을 올려 기울이고 얇게), 0%(없음). 저장 기본은 50%, 나머지는 메시 데이터로 보관.
  7 팔레트 채도·명암 대비 상향(PALETTE_V2).
"""
import bpy,json,math,sys
from collections import defaultdict
from pathlib import Path
from mathutils import Vector
sys.stdout.reconfigure(encoding='utf-8',errors='replace')

SRC=Path('D:/Work/unreal/CL9/ArtSource/monster/raccoon_a/raccoon_a.blend')
OUT=Path('D:/Work/unreal/CL9/ArtSource/monster/raccoon_a_v2')
BLEND_OUT=OUT/'raccoon_a_v2.blend'
assert Path(bpy.data.filepath).resolve()==SRC.resolve(),'원본 raccoon_a.blend 를 --blend 로 열어야 한다'
assert BLEND_OUT.resolve()!=SRC.resolve()
(OUT/'preview').mkdir(parents=True,exist_ok=True)

HEAD_PIVOT=Vector((.43,0,.70))  # head 뼈 머리점 — 머리가 여기서 돈다
HEAD_SCALE=1.15
SNOUT_X=.72;SNOUT_YZ=1.08
EAR_Z=.86;EAR_Y=1.08
CLAW_TIP=.8;CLAW_SCALE=.7
BROW_50=dict(lift=.018,tilt=.15,thick=.6,sink=.01)  # m, rad(안쪽 끝 올림), Z 두께 배율, 머리 중심 쪽으로 묻기(m — 들어 올린 눈썹이 뜨지 않게)
# 원본 팔레트(build_raccoon_a.py, refine_raccoon_ears.py) → v2. 회색 동물 정체성은 유지하고 대비·채도만 올린다.
PALETTE_V2={
    (.23,.25,.24):(.20,.21,.225),    # fur — 한 단계 어둡게(흰 얼굴과 대비), 푸른 기는 아주 약하게만
    (.66,.65,.56):(.80,.79,.72),     # light — 더 깨끗한 흰색
    (.027,.032,.030):(.012,.013,.015),  # dark — 마스크·꼬리 고리·다리를 더 검게
    (.010,.013,.012):(.008,.009,.010),  # nose
    (.34,.23,.075):(.62,.33,.035),   # iris — 선명한 호박색
    (.30,.26,.19):(.46,.42,.34),     # claw — 밝은 상아색(작아진 발톱이 형태로 읽히게)
    (.16,.12,.105):(.36,.14,.12),    # 귀 안쪽 — 분홍기 도는 갈색
}

s=bpy.data.scenes['CL9_RACCOON_A'];bpy.context.window.scene=s
rig=bpy.data.objects['SKEL_Raccoon_A'];m=bpy.data.objects['SK_Raccoon_A'];me=m.data
assert all(abs(m.matrix_world[r][c]-(r==c))<1e-6 for r in range(4) for c in range(4)),'메시 좌표 = 월드 좌표 전제'
orig_data=me.copy();orig_data.name='SK_Raccoon_A_original_compare'

# --- 섬(원래 프리미티브 하나 = 연결 성분 하나) ---
parent=list(range(len(me.vertices)))
def find(a):
    while parent[a]!=a:parent[a]=parent[parent[a]];a=parent[a]
    return a
for e in me.edges:
    a,b=find(e.vertices[0]),find(e.vertices[1])
    if a!=b:parent[a]=b
islands=defaultdict(list)
for v in me.vertices:islands[find(v.index)].append(v.index)
islands=list(islands.values())
names={g.index:g.name for g in m.vertex_groups}
def dominant(i):return names[max(me.vertices[i].groups,key=lambda g:g.weight).group]
def centroid(ids):return sum((me.vertices[i].co for i in ids),Vector())/len(ids)
def bounds(ids):
    lo=Vector((9,9,9));hi=Vector((-9,-9,-9))
    for i in ids:
        c=me.vertices[i].co
        for k in range(3):lo[k]=min(lo[k],c[k]);hi[k]=max(hi[k],c[k])
    return lo,hi
HEAD_GROUPS={'head','jaw','ear_l','ear_r'}
head_islands=[ids for ids in islands if dominant(ids[0]) in HEAD_GROUPS]
# 이름 붙일 섬 — build_raccoon_a.py 의 중심 좌표
named={'Body':(-.14,0,.53),'Head':(.45,0,.72),'Muzzle':(.644,0,.646),'LowerJaw':(.621,0,.596),'Nose':(.792,0,.66),
       'Brow_l':(.572,.179,.824),'Brow_r':(.572,-.179,.824),'Ear_l':(.338,.177,.948),'Ear_r':(.338,-.177,.948)}
part={}
for key,c in named.items():
    best=min(islands,key=lambda ids:(centroid(ids)-Vector(c)).length)
    d=(centroid(best)-Vector(c)).length;assert d<.01,(key,d)
    part[key]=best
ca=me.color_attributes['Color']
def island_color(ids):
    vs=set(ids)
    for p in me.polygons:
        if p.vertices[0] in vs:return tuple(round(x,3) for x in ca.data[p.loop_indices[0]].color[:3])
claw_islands=[ids for ids in islands if island_color(ids)==(.30,.26,.19)]
assert len(claw_islands)==16,len(claw_islands)

def ratio(tag):
    hl,hh=bounds(part['Head']);bl,bh=bounds(part['Body'])
    hy,hz=hh.y-hl.y,hh.z-hl.z;by,bz=bh.y-bl.y,bh.z-bl.z
    al,ah=bounds([i for ids in head_islands for i in ids])
    r={'head_core_cm':[round(hy*100,1),round(hz*100,1)],'body_cm':[round(by*100,1),round(bz*100,1)],
       'ratio_width':round(hy/by,3),'ratio_height':round(hz/bz,3),'ratio_mean':round((hy+hz)/(by+bz),3),
       'head_silhouette_cm':[round((ah-al)[k]*100,1) for k in range(3)]}
    tl,th=bounds(range(len(me.vertices)));r['total_dims_cm']=[round((th-tl)[k]*100,1) for k in range(3)]
    print('[measure]',tag,json.dumps(r));return r
before=ratio('before')

# 2 주둥이 — 뒤끝(x0) 기준으로 X 를 줄인다
x0=bounds(part['Muzzle'])[0].x
for key in ('Muzzle','LowerJaw'):
    c=centroid(part[key])
    for i in part[key]:
        v=me.vertices[i].co;v.x=x0+(v.x-x0)*SNOUT_X;v.y=c.y+(v.y-c.y)*SNOUT_YZ;v.z=c.z+(v.z-c.z)*SNOUT_YZ
nc=centroid(part['Nose']);shift=(x0+(nc.x-x0)*SNOUT_X)-nc.x
for i in part['Nose']:me.vertices[i].co.x+=shift
# 3 귀 — 바닥 기준으로 낮추고 옆으로 약간 넓힌다
for key in ('Ear_l','Ear_r'):
    lo,hi=bounds(part[key]);c=centroid(part[key])
    for i in part[key]:
        v=me.vertices[i].co;v.z=lo.z+(v.z-lo.z)*EAR_Z;v.y=c.y+(v.y-c.y)*EAR_Y
# 4 발톱 — 끝 고리를 벌리고 뿌리 기준으로 줄인다
for ids in claw_islands:
    c=centroid(ids)
    # 축 = 가장 먼 두 고리 방향: 중심에서 가장 먼 정점 쪽이 아니라 PCA 대신 x 가 큰 쪽이 끝(발톱은 +X 로 뻗는다)
    xs=sorted(ids,key=lambda i:me.vertices[i].co.x);half=len(ids)//2
    base=[i for i in xs[:half]];tip=[i for i in xs[half:]]
    bc=centroid(base);tc=centroid(tip)
    br=sum((me.vertices[i].co-bc).length for i in base)/len(base)
    tr=max(sum((me.vertices[i].co-tc).length for i in tip)/len(tip),1e-5)
    for i in tip:
        v=me.vertices[i].co;me.vertices[i].co=tc+(v-tc)*(br*CLAW_TIP/tr)
    for i in ids:
        v=me.vertices[i].co;me.vertices[i].co=bc+(v-bc)*CLAW_SCALE
# 1 머리 전체 확대(주둥이·귀 수정 뒤, 같은 비율로 따라간다)
for ids in head_islands:
    for i in ids:
        v=me.vertices[i].co;me.vertices[i].co=HEAD_PIVOT+(v-HEAD_PIVOT)*HEAD_SCALE
# 7 팔레트
remap={tuple(round(x,3) for x in k):v for k,v in PALETTE_V2.items()}
missing=set()
for d in ca.data:
    k=tuple(round(x,3) for x in d.color[:3])
    if k in remap:d.color=(*remap[k],1)
    else:missing.add(k)
assert not missing,missing
me.update()
after=ratio('after (brow 100%)')

# 6 눈썹 3단계 — 지금 me = 100%. 50%·0% 는 복사본에서 만든다
brow100=me.copy();brow100.name='SK_Raccoon_A_v2_Brow100'
brow0=me.copy();brow0.name='SK_Raccoon_A_v2_Brow0'
hc=centroid(part['Head'])
for key,sign in (('Brow_l',1),('Brow_r',-1)):
    ids=part[key];c=centroid(ids);d=Vector((math.cos(.5),-sign*math.sin(.5),0))  # 앞(=안쪽) 끝 방향
    sink=(hc-c).normalized()*BROW_50['sink']
    for i in ids:
        v=me.vertices[i].co;t=(v-c).dot(d)
        v.z=c.z+(v.z-c.z)*BROW_50['thick']+BROW_50['lift']+math.tan(BROW_50['tilt'])*t
        me.vertices[i].co=me.vertices[i].co+sink
me.update();me.name='SK_Raccoon_A_v2_Brow50'
import bmesh
bm=bmesh.new();bm.from_mesh(brow0)
bm.verts.ensure_lookup_table()
bmesh.ops.delete(bm,geom=[bm.verts[i] for i in part['Brow_l']+part['Brow_r']],context='VERTS')
bm.to_mesh(brow0);bm.free();brow0.update()
for d in (me,brow100,brow0):d.calc_loop_triangles();print('[tris]',d.name,len(d.loop_triangles))

# --- 저장(v2 폴더에만) ---
for d in (brow100,brow0):d.use_fake_user=True
actions=[a for a in bpy.data.actions if a.name.startswith('AN_Raccoon_')]
bpy.data.libraries.write(str(BLEND_OUT),{s,me,brow100,brow0,*actions},fake_user=True)
print('[saved]',BLEND_OUT)

# --- 렌더 ---
exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/creature_preview.py',encoding='utf-8').read(),'creature_preview.py','exec'))
FOCUS=(-.1,0,.5);CAM=(2.3,-3.8,2)
setup_creature_preview('CL9_RACCOON_A',OUT,focus=FOCUS,camera_position=CAM,ortho=2.5)
rig.animation_data.action=bpy.data.actions['AN_Raccoon_Idle'];s.frame_set(1)
s.render.filepath=str(OUT/'preview/neutral.png');bpy.ops.render.render(write_still=True)

def twin(data,offset):
    r=rig.copy();s.collection.objects.link(r);r.location=offset
    o=m.copy();o.data=data;s.collection.objects.link(o);o.parent=r;o.modifiers['CL9 Skin'].object=r
    return r,o
def right_of(cam,focus):
    v=Vector(focus)-Vector(cam);v.z=0;v.normalize();return Vector((v.y,-v.x,0))
cam=bpy.data.objects['CL9_RACCOON_A_PREVIEW_Camera']
# (a) 원본(왼쪽) vs v2 눈썹 50%(오른쪽), 원본과 같은 카메라 방향
rt=right_of(CAM,FOCUS);r0,o0=twin(orig_data,-.95*rt);rig.location=.95*rt
cam.data.ortho_scale=4.4;s.render.resolution_x=1600;s.render.resolution_y=800
s.frame_set(1);s.render.filepath=str(OUT/'preview/compare_original_v2.png');bpy.ops.render.render(write_still=True)
for o in (o0,r0):bpy.data.objects.remove(o)
# (b) 눈썹 100%/50%/0% — 얼굴 쪽 3/4
FF=(.55,0,.74);FC=(FF[0]+2.2,FF[1]-.6,FF[2]+.3)
rt=right_of(FC,FF);r1,o1=twin(brow100,-1.1*rt);r2,o2=twin(brow0,1.1*rt);rig.location=(0,0,0)
cam.location=FC;cam.rotation_euler=(Vector(FF)-Vector(FC)).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=3.5
s.render.resolution_x=1800;s.render.resolution_y=800
s.frame_set(1);s.render.filepath=str(OUT/'preview/compare_brow_100_50_0.png');bpy.ops.render.render(write_still=True)
print('[measure-summary]',json.dumps({'before':before,'after':after,'head_scale':HEAD_SCALE}))
