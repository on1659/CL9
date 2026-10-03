"""pig_a v2 눈 표정 3단(평소/추격/피격) — pig_a_v2.blend 에 '아픈 눈' 눈썹 프리셋을 더하고 비교 렌더를 만든다.

    py -3 Tools/Blender/run_headless.py build_pig_a_v2_pain.py --blend ArtSource/monster/pig_a_v2/pig_a_v2.blend [-- --dry]

--dry 는 후보 비교 시트만 렌더하고 blend 는 저장하지 않는다(후보값을 고르는 동안 쓴다). --nosave 는 3단 비교까지 렌더하고 저장하지 않는다.
평소 = 완화 눈썹(사본에 저장된 기본형), 추격 = 현재(실물) 눈썹, 피격 = 아픈 눈썹. 눈썹 두 섬의 정점만 움직이고
토폴로지·가중치·리그·액션·색은 그대로 둔다. 기본형(Basis)은 완화 눈썹 그대로 두고, 현재·아픈 눈썹은 셰이프 키(값 0)로 더한다.
눈썹 수치 정의는 build_pig_a_v2.py 와 같다(BROW_CUR·BROW_SOFT·HEAD_ELL 은 그 파일 값 복사).
"""
import bpy,bmesh,json,math,sys
import numpy as np
from pathlib import Path
from mathutils import Vector,Euler
sys.stdout.reconfigure(encoding='utf-8',errors='replace')
OUT=Path('D:/Work/unreal/CL9/ArtSource/monster/pig_a_v2');SRC=Path('D:/Work/unreal/CL9/ArtSource/monster/pig_a')
assert Path(bpy.data.filepath).resolve()==(OUT/'pig_a_v2.blend').resolve(),'pig_a_v2.blend 를 --blend 로 열어야 한다'
DRY='--dry' in sys.argv

# ── build_pig_a_v2.py 값 ──
BROW_CUR={'center':(.638,.209,.919),'scale':(.086,.035,.032),'pitch':.05,'yaw':-.35}
BROW_SOFT={'dz':.012,'scale':(.92,.85,.72),'pitch':-.10}
HEAD_ELL=((.52,0,.76),(.30,.27,.25))

# ── 아픈 눈썹 후보(확정 아님) ──
# 화난 눈썹(코 쪽 끝이 눈으로 내려옴, pitch +)의 반대 — 코 쪽(안쪽) 끝을 크게 들어 올리고 바깥 끝은 눈꼬리 쪽으로 처지게 한다.
# dz: 실물 중심 대비 높이(m), scale: 실물 대비 배율(길이, 폭, 두께), pitch: rad(음수 = 코 쪽 끝이 위로),
# bend: 안쪽 절반을 위로 휘는 양(m, 코 쪽 끝에서 최대 — 걱정스러운 아치).
PAIN={
    'A':{'dz':.012,'scale':(.92,.85,.72),'pitch':-.30,'bend':0.0},
    'B':{'dz':.006,'scale':(.95,.85,.75),'pitch':-.45,'bend':0.0},
    'C':{'dz':.004,'scale':(.95,.85,.75),'pitch':-.60,'bend':.015},
}
PICK='B'  # 후보 비교 후 고른 것 — 3단 비교 렌더와 셰이프 키에 들어간다

s=bpy.context.scene;ob=bpy.data.objects['SK_Pig_A'];me=ob.data;rig=bpy.data.objects['SKEL_Pig_A']
assert me.shape_keys is None,'이미 셰이프 키가 있다 — 이 스크립트는 한 번만 저장한다(원본 사본에서 다시 시작할 것)'
me.calc_loop_triangles();TRIS0=len(me.loop_triangles);VERTS0=len(me.vertices)
BASE=[v.co.copy() for v in me.vertices]

def ell_level(p):return sum(((p[k]-HEAD_ELL[0][k])/HEAD_ELL[1][k])**2 for k in range(3))
def params(sg,dz,mul,pitch):
    """실물 중심에서 dz 만큼 올리고, 머리 타원체의 같은 등위면을 따라 y 를 맞춘다(build_pig_a_v2.brow_params 와 같은 식)."""
    c0=Vector((BROW_CUR['center'][0],sg*BROW_CUR['center'][1],BROW_CUR['center'][2]));z=c0.z+dz
    if dz==0:y=c0.y
    else:
        ry=ell_level(c0)-((c0.x-HEAD_ELL[0][0])/HEAD_ELL[1][0])**2-((z-HEAD_ELL[0][2])/HEAD_ELL[1][2])**2
        y=sg*HEAD_ELL[1][1]*math.sqrt(max(ry,0))
    sc=Vector([BROW_CUR['scale'][i]*mul[i] for i in range(3)])
    return Vector((c0.x,y,z)),sc,Euler((0,pitch,sg*BROW_CUR['yaw']),'XYZ')
SOFT=lambda sg:params(sg,BROW_SOFT['dz'],BROW_SOFT['scale'],BROW_SOFT['pitch'])
CUR=lambda sg:params(sg,0,(1,1,1),BROW_CUR['pitch'])

# ── 눈썹 섬: 사본의 완화 눈썹 중심으로 찾는다 ──
bm=bmesh.new();bm.from_mesh(me);bm.verts.ensure_lookup_table();seen=set();islands=[]
for v in bm.verts:
    if v.index in seen:continue
    stack=[v];part=[];seen.add(v.index)
    while stack:
        x=stack.pop();part.append(x.index)
        for e in x.link_edges:
            o=e.other_vert(x)
            if o.index not in seen:seen.add(o.index);stack.append(o)
    islands.append(part)
bm.free()
def center(p):
    cs=[BASE[i] for i in p];return Vector([(min(c[k] for c in cs)+max(c[k] for c in cs))/2 for k in range(3)])
BROWS={}
for sg in (1,-1):
    c=SOFT(sg)[0];hit=[p for p in islands if (center(p)-c).length<.004]
    assert len(hit)==1,('눈썹 섬 찾기 실패',sg,len(hit));BROWS[sg]=hit[0]
# 단위 좌표(완화 변환의 역) — 이후 모든 눈썹 모양은 여기서 다시 놓는다
UNIT={}
for sg,p in BROWS.items():
    c,sc,r=SOFT(sg);Ri=r.to_matrix().inverted()
    for i in p:u=Ri@(BASE[i]-c);UNIT[i]=(sg,Vector((u.x/sc.x,u.y/sc.y,u.z/sc.z)))
def brow_co(cfg):
    """cfg: 'soft' | 'current' | PAIN 사전 하나. 전체 정점 좌표 목록을 돌려준다."""
    co=[c.copy() for c in BASE]
    for i,(sg,u) in UNIT.items():
        if cfg=='soft':c,sc,r=SOFT(sg);bend=0
        elif cfg=='current':c,sc,r=CUR(sg);bend=0
        else:c,sc,r=params(sg,cfg['dz'],cfg['scale'],cfg['pitch']);bend=cfg['bend']
        w=Vector((u.x*sc.x,u.y*sc.y,u.z*sc.z))
        if bend:w.z+=bend*max(u.x,0)**2  # +x = 코 쪽(안쪽) 끝
        co[i]=c+r.to_matrix()@w
    return co
err=max((a-b).length for a,b in zip(brow_co('soft'),BASE));print('soft roundtrip err %.2e'%err);assert err<1e-4

# ── 렌더 무대(build_pig_a_v2.py 와 같은 값) ──
def look(o,focus):o.rotation_euler=(Vector(focus)-o.location).to_track_quat('-Z','Y').to_euler()
s.render.engine='CYCLES';s.cycles.samples=24;s.render.resolution_percentage=100
s.render.image_settings.media_type='IMAGE';s.render.image_settings.file_format='PNG'
bg=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[0].default_value=(.12,.15,.18,1);bg.inputs[1].default_value=.35
MAIN=bpy.data.objects['CL9_PIG_A_PREVIEW_Camera']
MAIN.location=(2.6,-3.8,2.0);MAIN.data.type='ORTHO';MAIN.data.ortho_scale=2.55;look(MAIN,(0,0,.54))
for label,pos,power,size in [('Key',(2,-3,4),420,3),('Fill',(-1,2,2),150,2)]:
    L=bpy.data.objects['CL9_PIG_A_PREVIEW_'+label];L.location=pos;L.data.energy=power;L.data.color=(1,1,1);L.data.shape='DISK';L.data.size=size;look(L,(0,0,.54))
cd=bpy.data.cameras.new('CL9_PIG_A_V2_BrowCam');BROWCAM=bpy.data.objects.new(cd.name,cd);s.collection.objects.link(BROWCAM)
BROWCAM.location=(1.95,-1.35,1.12);cd.type='ORTHO';cd.ortho_scale=.62;look(BROWCAM,(.70,-.02,.80))
rig.animation_data.action=bpy.data.actions['AN_Pig_Idle'];s.frame_set(1)
import tempfile;TMP=Path(tempfile.gettempdir())/'cl9_pig_a_v2_pain';TMP.mkdir(exist_ok=True)  # 낱장은 저장소 밖 — 시트만 preview/ 에 남긴다
def render(name,cam,res,ortho=None):
    s.camera=cam;s.render.resolution_x,s.render.resolution_y=res
    if ortho:cam.data.ortho_scale=ortho
    path=TMP/name;assert SRC not in path.parents;s.render.filepath=str(path);bpy.ops.render.render(write_still=True);return path
def apply(co):
    for v,c in zip(me.vertices,co):v.co=c
    me.update();s.frame_set(1)
def sheet(name,rows):
    """rows: [[경로...], ...] — 칸 사이 12px, 줄 사이 12px 여백. 같은 줄의 그림은 높이가 같아야 한다."""
    gap=12;lines=[]
    for paths in rows:
        imgs=[bpy.data.images.load(str(p)) for p in paths];h=imgs[0].size[1]
        arrs=[np.array(im.pixels[:],dtype=np.float32).reshape(im.size[1],im.size[0],4) for im in imgs]
        pad=np.tile(np.array([.05,.05,.05,1],np.float32),(h,gap,1));row=[]
        for a in arrs:row+=[a,pad]
        lines.append(np.concatenate(row[:-1],axis=1))
        for im in imgs:bpy.data.images.remove(im)
    wmax=max(l.shape[1] for l in lines);stack=[]
    for l in reversed(lines):  # Blender 픽셀은 아래 줄부터 — 위 줄이 위에 오도록 뒤집어 쌓는다
        assert l.shape[1]==wmax,'줄 폭이 다르다';stack+=[l,np.tile(np.array([.05,.05,.05,1],np.float32),(gap,wmax,1))]
    out=np.concatenate(stack[:-1],axis=0);img=bpy.data.images.new(name,out.shape[1],out.shape[0],alpha=True)
    img.pixels=out.ravel();img.filepath_raw=str(OUT/'preview'/name);img.file_format='PNG';img.save();bpy.data.images.remove(img)
    print('SHEET',OUT/'preview'/name)

# 1) 후보 비교: 평소(완화) | A | B | C — 머리 근접(compare_brow.png 와 같은 카메라·900×900)
cand=[]
for key in ['soft']+list(PAIN):
    apply(brow_co('soft' if key=='soft' else PAIN[key]));cand.append(render('pain_cand_%s.png'%key,BROWCAM,(900,900),.62))
sheet('compare_pain_candidates.png',[cand])
if DRY:
    apply(BASE);print('DRY — blend 저장 안 함');sys.exit(0)

# 2) 3단 비교: 평소(완화) | 추격(현재) | 피격(아픈) — 윗줄 neutral 카메라 전신, 아랫줄 머리 근접(가로 1200 에 맞춰 세로 범위 유지)
full=[];close=[]
for key,cfg in [('idle','soft'),('chase','current'),('hit',PAIN[PICK])]:
    apply(brow_co(cfg))
    full.append(render('state_%s_full.png'%key,MAIN,(1200,900)))
    close.append(render('state_%s_close.png'%key,BROWCAM,(1200,900),.62*1200/900))
sheet('compare_three_states.png',[full,close])
if '--nosave' in sys.argv:
    apply(BASE);print('NOSAVE — blend 저장 안 함');sys.exit(0)

# ── 셰이프 키: Basis = 완화(저장돼 있던 기본형), Brow_Current·Brow_Pain 은 값 0 ──
apply(BASE)
ob.shape_key_add(name='Basis',from_mix=False)
for kname,cfg in [('Brow_Current','current'),('Brow_Pain',PAIN[PICK])]:
    kb=ob.shape_key_add(name=kname,from_mix=False);co=brow_co(cfg)
    for d,c in zip(kb.data,co):d.co=c
    kb.value=0.0
me.calc_loop_triangles();assert len(me.loop_triangles)==TRIS0 and len(me.vertices)==VERTS0
bpy.data.objects.remove(BROWCAM);bpy.data.cameras.remove(cd)
s.camera=MAIN;MAIN.data.ortho_scale=2.55;s.render.resolution_x,s.render.resolution_y=1200,900;s.render.filepath=str(OUT/'preview'/'neutral.png')
bpy.ops.wm.save_mainfile(filepath=str(OUT/'pig_a_v2.blend'))
notes=json.loads((OUT/'notes.json').read_text(encoding='utf-8'))
notes['brow_pain_candidates']=PAIN;notes['brow_pain_pick']=PICK
notes['shape_keys']={'Basis':'soft(평소)','Brow_Current':'current(추격)','Brow_Pain':'pain %s(피격)'%PICK}
notes['previews'].update({'pain_candidates':'preview\\compare_pain_candidates.png','three_states':'preview\\compare_three_states.png'})
(OUT/'notes.json').write_text(json.dumps(notes,indent=2,ensure_ascii=False),encoding='utf-8');print('SAVED',OUT/'pig_a_v2.blend')
