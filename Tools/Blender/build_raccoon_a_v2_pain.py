"""raccoon_a v2 "아픈 눈" 프리셋 — 실행 eyes-pain-raccoon.

    py -3 Tools/Blender/run_headless.py build_raccoon_a_v2_pain.py --blend ArtSource/monster/raccoon_a_v2/raccoon_a_v2.blend
    py -3 Tools/Blender/run_headless.py build_raccoon_a_v2_pain.py --blend ArtSource/monster/raccoon_a_v2/raccoon_a_v2.blend -- --pick B --save

인자 없이 돌리면 후보 A·B·C 를 만들어 근접 비교 렌더(compare_pain_candidates.png)만 찍는다(저장 없음).
--pick X 는 고른 후보로 3단 비교 렌더(compare_three_states.png — 평소 눈썹0% | 추격 눈썹100% | 피격 아픈 눈)를 찍고,
--save 가 붙으면 그 후보를 메시 데이터 SK_Raccoon_A_v2_Pain(fake user)으로 raccoon_a_v2.blend 에 더한다.
기존 메시 데이터(Brow100/50/0)와 오브젝트 기본(Brow50)은 그대로 둔다. 렌더용 쌍둥이 오브젝트는 저장 뒤에 만든다.

아픈 눈 = 눈썹 100%(실물) 복사본에서
  - 눈썹: 원래 기울기(안쪽 끝이 내려간 화난 사선)를 지우고, 안쪽 끝이 위로 가는 기울기 TILT(rad)를 준다. 회전 중심은 바깥 끝
    (바깥 끝 높이는 LIFT 만큼만 바뀐다) — 바깥 끝이 마스크 안으로 떨어지지 않게. 두께는 THICK 배. 정점은 머리 중심에서의 거리를
    원래대로 되돌려 머리 표면에 붙인다.
  - 눈(홍채·동공): SQUINT 배로 세로를 눌러 찡그린 눈. 누르는 중심은 눈 중심보다 SQUINT_PIVOT(눈 높이 비율)만큼 아래 —
    윗눈꺼풀이 내려온 모양. PUPIL 배로 동공을 더 줄인다. EYE_TILT(rad)는 눌린 눈을 눈썹 축 방향으로 기울여 안쪽 끝을 올린다.
  - DARK=True 면 눈썹을 마스크와 같은 검은색으로 칠한다(흰 이마 위에서 선으로 읽히게).
"""
import bpy,math,sys
from collections import defaultdict
from pathlib import Path
from mathutils import Vector
sys.stdout.reconfigure(encoding='utf-8',errors='replace')

OUT=Path('D:/Work/unreal/CL9/ArtSource/monster/raccoon_a_v2')
BLEND=OUT/'raccoon_a_v2.blend'
assert Path(bpy.data.filepath).resolve()==BLEND.resolve(),'raccoon_a_v2.blend 를 --blend 로 열어야 한다'
argv=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
PICK=argv[argv.index('--pick')+1] if '--pick' in argv else None
SAVE='--save' in argv
assert not SAVE or PICK,'--save 는 --pick 과 함께'

CANDIDATES={
    # 흰 눈썹(색 그대로) 안쪽 끝 올림 + 찡그린 눈
    'A':dict(tilt=.6,lift=.006,thick=.75,squint=.55,pupil=.8,dark=False,eye_tilt=0),
    # 검은 눈썹을 마스크 위 흰 이마에 올림 + 찡그린 눈
    'B':dict(tilt=.6,lift=.02,thick=.6,squint=.55,pupil=.8,dark=True,eye_tilt=0),
    # B + 눌린 눈도 안쪽 끝이 올라가게 기울임(눈썹과 같은 방향의 사선)
    'C':dict(tilt=.6,lift=.02,thick=.6,squint=.55,pupil=.8,dark=True,eye_tilt=.3),
}
SQUINT_PIVOT=.25
LIGHT=(.80,.79,.72);IRIS=(.62,.33,.035);PUPIL_COL=(.008,.009,.010);DARK_COL=(.012,.013,.015)

s=bpy.data.scenes['CL9_RACCOON_A'];bpy.context.window.scene=s
rig=bpy.data.objects['SKEL_Raccoon_A'];m=bpy.data.objects['SK_Raccoon_A']
base=bpy.data.meshes['SK_Raccoon_A_v2_Brow100'];brow0=bpy.data.meshes['SK_Raccoon_A_v2_Brow0']
assert m.data.name=='SK_Raccoon_A_v2_Brow50'

# --- 섬 ---
parent=list(range(len(base.vertices)))
def find(a):
    while parent[a]!=a:parent[a]=parent[parent[a]];a=parent[a]
    return a
for e in base.edges:
    a,b=find(e.vertices[0]),find(e.vertices[1])
    if a!=b:parent[a]=b
isl=defaultdict(list)
for v in base.vertices:isl[find(v.index)].append(v.index)
isl=list(isl.values())
ca=base.color_attributes['Color']
first_loop={}
for p in base.polygons:
    for vi,li in zip(p.vertices,p.loop_indices):first_loop.setdefault(vi,li)
def col(ids):return tuple(round(x,3) for x in ca.data[first_loop[ids[0]]].color[:3])
def cen(me,ids):return sum((me.vertices[i].co for i in ids),Vector())/len(ids)
def pick_island(c,color):
    best=min((ids for ids in isl if col(ids)==color),key=lambda ids:(cen(base,ids)-Vector(c)).length)
    d=(cen(base,best)-Vector(c)).length;assert d<.01,(c,d);return best
# probe_raccoon_a_v2_eyes.py 실측 중심(눈썹 100% 데이터)
part={}
for side,sg in (('l',1),('r',-1)):
    part['Brow_'+side]=pick_island((.593,sg*.206,.843),LIGHT)
    part['Iris_'+side]=pick_island((.647,sg*.225,.786),IRIS)
    part['Pupil_'+side]=pick_island((.673,sg*.238,.790),PUPIL_COL)
    part['Mask_'+side]=pick_island((.608,sg*.187,.763),DARK_COL)
HC=cen(base,pick_island((.453,0,.723),LIGHT))  # 머리 코어 구
print('[head centre]',tuple(round(x,3) for x in HC))

def brow_axis(me,ids,sg):
    """눈썹 긴 축(수평 투영) — 안쪽(정중선 쪽) 끝이 + 가 되게."""
    c=cen(me,ids);sxx=sxy=syy=0
    for i in ids:
        d=me.vertices[i].co-c;sxx+=d.x*d.x;sxy+=d.x*d.y;syy+=d.y*d.y
    a=.5*math.atan2(2*sxy,sxx-syy);e=Vector((math.cos(a),math.sin(a),0))
    if e.y*(-sg)<0:e=-e
    return c,e

def make(name,p):
    me=base.copy();me.name=name
    report={}
    for side,sg in (('l',1),('r',-1)):
        ids=part['Brow_'+side];c,e=brow_axis(me,ids,sg)
        ts=[(me.vertices[i].co-c).dot(e) for i in ids]
        slope=sum(t*(me.vertices[i].co.z-c.z) for t,i in zip(ts,ids))/sum(t*t for t in ts)
        tmin=min(ts);r0={i:(me.vertices[i].co-HC).length for i in ids}
        for t,i in zip(ts,ids):
            v=me.vertices[i].co
            flat=v.z-c.z-slope*t  # 원래 기울기 제거, 중심선 기준 높이
            v.z=c.z+slope*tmin+flat*p['thick']+p['lift']+math.tan(p['tilt'])*(t-tmin)
            me.vertices[i].co=HC+(me.vertices[i].co-HC).normalized()*r0[i]
        report['brow_'+side]=dict(orig_slope_rad=round(math.atan(slope),3),len_cm=round((max(ts)-tmin)*100,1),
                                  axis_deg=round(math.degrees(math.atan2(e.y,e.x)),1))
        # 눈 — 홍채 전체 높이 기준으로 누른다(동공도 같은 중심)
        iris=part['Iris_'+side];pup=part['Pupil_'+side]
        zs=[me.vertices[i].co.z for i in iris];zc=(max(zs)+min(zs))/2-SQUINT_PIVOT*(max(zs)-min(zs))/2
        pc=cen(me,pup)
        for i in pup:
            v=me.vertices[i].co;me.vertices[i].co=pc+(v-pc)*p['pupil']
        ic=cen(me,iris)
        for i in iris+pup:
            v=me.vertices[i].co;v.z=zc+(v.z-zc)*p['squint']+math.tan(p['eye_tilt'])*(v-ic).dot(e)
        if p['dark']:
            ids_set=set(ids)
            for poly in me.polygons:
                if poly.vertices[0] in ids_set:
                    for li in poly.loop_indices:me.color_attributes['Color'].data[li].color=(*DARK_COL,1)
        # 마스크와의 관계: 눈썹 정점 중 마스크 위쪽 경계보다 낮은 것의 비율(같은 방위에서 비교)
        mask=part['Mask_'+side];mtop=max(me.vertices[i].co.z for i in mask)
        report['brow_'+side]['min_z']=round(min(me.vertices[i].co.z for i in ids),3)
        report['brow_'+side]['max_z']=round(max(me.vertices[i].co.z for i in ids),3)
        report['brow_'+side]['mask_top_z']=round(mtop,3)
    me.update();print('[pain]',name,p,report)
    return me

cands={k:make('SK_Raccoon_A_v2_Pain'+('' if k==PICK else '_cand'+k),p) for k,p in CANDIDATES.items() if PICK is None or k==PICK}

if SAVE:
    pain=cands[PICK];pain.use_fake_user=True
    bpy.ops.wm.save_mainfile(filepath=str(BLEND))
    print('[saved]',BLEND,[(d.name,d.use_fake_user) for d in bpy.data.meshes])

# --- 렌더 ---
exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/creature_preview.py',encoding='utf-8').read(),'creature_preview.py','exec'))
FOCUS=(-.1,0,.5);CAM=(2.3,-3.8,2)  # build_raccoon_a_v2.py neutral.png 와 같은 값
setup_creature_preview('CL9_RACCOON_A',OUT,focus=FOCUS,camera_position=CAM,ortho=2.5)
rig.animation_data.action=bpy.data.actions['AN_Raccoon_Idle']
cam=bpy.data.objects['CL9_RACCOON_A_PREVIEW_Camera']
def twin(data,offset):
    r=rig.copy();s.collection.objects.link(r);r.location=offset
    o=m.copy();o.data=data;s.collection.objects.link(o);o.parent=r;o.modifiers['CL9 Skin'].object=r;o.hide_render=False
    return r,o
def right_of(c,f):
    v=Vector(f)-Vector(c);v.z=0;v.normalize();return Vector((v.y,-v.x,0))
m.hide_render=True
if PICK is None:
    # 후보 A | B | C — 얼굴 쪽 3/4 근접(compare_brow_100_50_0.png 와 같은 방향, 더 가깝게)
    FF=(.55,0,.88);FC=(FF[0]+2.2,FF[1]-.6,FF[2]+.3);rt=right_of(FC,FF)
    for k,off in zip(CANDIDATES,(-1.1,0,1.1)):twin(cands[k],off*rt)
    cam.location=FC;cam.rotation_euler=(Vector(FF)-Vector(FC)).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=3.4
    s.render.resolution_x=2100;s.render.resolution_y=600
    s.frame_set(1);s.render.filepath=str(OUT/'preview/compare_pain_candidates.png');bpy.ops.render.render(write_still=True)
else:
    # 평소(눈썹 0%) | 추격(눈썹 100%) | 피격(아픈 눈) — neutral.png 와 같은 카메라 방향·조명, 폭만 넓힌다
    rt=right_of(CAM,FOCUS)
    for data,off in ((brow0,-2.0),(base,0),(cands[PICK],2.0)):twin(data,off*rt)
    cam.data.ortho_scale=6.6;s.render.resolution_x=3000;s.render.resolution_y=1100
    s.frame_set(1);s.render.filepath=str(OUT/'preview/compare_three_states.png');bpy.ops.render.render(write_still=True)
print('[done]')
