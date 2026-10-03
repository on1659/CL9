"""pig_a 귀여움 1단계(형태+렌더) — 실물 blend 를 읽어 메시만 고친 사본을 pig_a_v2/ 에 만든다.

    py -3 Tools/Blender/run_headless.py build_pig_a_v2.py --blend ArtSource/monster/pig_a/pig_a.blend

실물 `pig_a/pig_a.blend`·`export/`·`preview/` 에는 아무것도 쓰지 않는다(열기만 하고 사본은 copy=True 로 다른 경로에 저장).
토폴로지·정점 수·가중치·리그·액션은 그대로 두고 정점 위치와 정점 색만 바꾼다 — tris 8,404 유지.
바꾸는 것: 채도(분홍/살구 4색), 눈썹(현재/완화 2단계 렌더, 사본은 완화), 발굽 모따기 폭. 귀·코·꼬리는 건드리지 않는다.
머리 크기는 그대로 두고(실측 71%), 참고용으로 머리 묶음 +8% 렌더 1장만 남긴다.
"""
import bpy,bmesh,colorsys,json,math,sys
import numpy as np
from pathlib import Path
from mathutils import Vector,Euler,Matrix
sys.stdout.reconfigure(encoding='utf-8',errors='replace')
SRC=Path('D:/Work/unreal/CL9/ArtSource/monster/pig_a')
OUT=Path('D:/Work/unreal/CL9/ArtSource/monster/pig_a_v2');(OUT/'preview').mkdir(parents=True,exist_ok=True)
assert Path(bpy.data.filepath).resolve()==(SRC/'pig_a.blend').resolve(),'실물 pig_a.blend 를 --blend 로 열어야 한다'

# ── 조정값(확정 아님 — 렌더 옵션) ──
SAT_GAIN=1.40            # sRGB HSV 채도 배율, 분홍/살구 4색에만
HOOF_CHAMFER=(.012,.020) # 발굽 모따기 폭 m: 실물 → v2
HEAD_OPTION=1.08         # 참고 렌더용 머리 묶음 배율(사본에는 넣지 않음)
BROW_SOFT={'dz':.012,'scale':(.92,.85,.72),'pitch':-.10}  # 완화 끝점: 위로, 얇게, 앞(코 쪽) 끝을 들어 올림
BROW_CUR={'center':(.638,.209,.919),'scale':(.086,.035,.032),'pitch':.05,'yaw':-.35}  # build_pig_a.py 값
HEAD_ELL=((.52,0,.76),(.30,.27,.25))  # 머리 타원체 중심·반지름(build_pig_a.py)

s=bpy.context.scene;ob=bpy.data.objects['SK_Pig_A'];me=ob.data;rig=bpy.data.objects['SKEL_Pig_A']
me.calc_loop_triangles();TRIS0=len(me.loop_triangles);VERTS0=len(me.vertices)

# ── 섬 분류 ──
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
REST=[v.co.copy() for v in me.vertices]
def bounds(part,co=None):
    co=co or REST;cs=[co[i] for i in part]
    lo=Vector([min(c[k] for c in cs) for k in range(3)]);hi=Vector([max(c[k] for c in cs) for k in range(3)]);return lo,hi
def find(center,size=None,tol=.004):
    hit=[]
    for p in islands:
        lo,hi=bounds(p)
        if ((lo+hi)/2-Vector(center)).length<tol and (size is None or ((hi-lo)-Vector(size)).length<tol):hit.append(p)
    assert len(hit)==1,('섬 찾기 실패',center,len(hit));return hit[0]
BODY=find((-.20,0,.66),(1.20,.76,.70));HEAD=find((.52,0,.76),(.60,.54,.50))
HEAD_UNIT=[HEAD,find((.62,0,.61)),find((.727,0,.717)),find((.88,0,.716))]  # 머리+턱+코다리+코판
for sg in (1,-1):
    HEAD_UNIT+=[find((.94,sg*.075,.739)),find((.665,sg*.216,.875)),find((.688,sg*.239,.875)),find((.638,sg*.209,.919)),find((.43,sg*.28,1.01))]
BROWS={sg:find((.638,sg*.209,.919),(.161,.088,.064)) for sg in (1,-1)}
HOOFS=[p for p in islands if len(p)==24 and abs((bounds(p)[0].z+bounds(p)[1].z)/2-.057)<.003]
assert len(HOOFS)==8,len(HOOFS)  # 네 다리 × 갈라진 발굽 2

# ── 실측: 머리:몸 비 ──
bl,bh=bounds(BODY);hl,hh=bounds(HEAD);bd=bh-bl;hd=hh-hl
ul=Vector([min(bounds(p)[0][k] for p in HEAD_UNIT) for k in range(3)]);uh=Vector([max(bounds(p)[1][k] for p in HEAD_UNIT) for k in range(3)]);ud=uh-ul
measure={'body_cm':[round(x*100,1) for x in bd],'head_cm':[round(x*100,1) for x in hd],'head_unit_cm':[round(x*100,1) for x in ud],
         'head_over_body_height':round(hd.z/bd.z,3),'head_over_body_width':round(hd.y/bd.y,3),'head_over_body_length':round(hd.x/bd.x,3),
         'head_unit_over_body_height':round(ud.z/bd.z,3),'head_unit_over_body_width':round(ud.y/bd.y,3)}
print('MEASURE',json.dumps(measure))

# ── 렌더 무대(creature_preview.py 와 같은 값, 출력 경로만 pig_a_v2) ──
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
def render(name,cam,res):
    s.camera=cam;s.render.resolution_x,s.render.resolution_y=res
    path=OUT/'preview'/name;assert SRC not in path.parents;s.render.filepath=str(path);bpy.ops.render.render(write_still=True);return path
def apply(co):
    for v,c in zip(me.vertices,co):v.co=c
    me.update();s.frame_set(1)

# 1) 실물 그대로
shots={'orig':render('original_neutral.png',MAIN,(1200,900)),'orig_brow':render('original_brow.png',BROWCAM,(900,900))}

# ── 채도 ──
def lin2s(c):return c*12.92 if c<=.0031308 else 1.055*c**(1/2.4)-.055
def s2lin(c):return c/12.92 if c<=.04045 else ((c+.055)/1.055)**2.4
TARGET={'skin':(.54,.265,.22),'light':(.70,.39,.32),'snout':(.44,.15,.13),'inner':(.30,.10,.10)}
remap={}
for k,c in TARGET.items():
    h,sat,val=colorsys.rgb_to_hsv(*[lin2s(x) for x in c]);n=tuple(round(s2lin(x),4) for x in colorsys.hsv_to_rgb(h,min(1,sat*SAT_GAIN),val))
    remap[tuple(round(x,3) for x in c)]=n;print('COLOR',k,c,'sat %.3f -> %.3f'%(sat,min(1,sat*SAT_GAIN)),n)
col=me.color_attributes['Color'];changed=0
for d in col.data:
    key=tuple(round(x,3) for x in d.color[:3])
    if key in remap:d.color=(*remap[key],1);changed+=1
print('recolored corners',changed)

# ── 발굽 모따기: 각 정점은 한 축만 면 끝(±h), 나머지 두 축은 h-w 로 들어와 있다 → h-w2 로 ──
co=[c.copy() for c in REST];w0,w1=HOOF_CHAMFER
for p in HOOFS:
    lo,hi=bounds(p);c=(lo+hi)/2;h=(hi-lo)/2
    for i in p:
        d=REST[i]-c;full=[k for k in range(3) if abs(abs(d[k])-h[k])<1e-4];inset=[k for k in range(3) if abs(abs(d[k])-(h[k]-w0))<1e-4]
        assert len(full)==1 and len(inset)==2,('발굽 정점 구조가 예상과 다르다',i,d,h)
        for k in inset:co[i][k]=c[k]+math.copysign(h[k]-w1,d[k])

# ── 눈썹: 섬 정점을 단위구로 되돌렸다가 새 중심·크기·회전으로 다시 놓는다 ──
def ell_level(p):return sum(((p[k]-HEAD_ELL[0][k])/HEAD_ELL[1][k])**2 for k in range(3))
def brow_params(sg,k):
    c0=Vector((BROW_CUR['center'][0],sg*BROW_CUR['center'][1],BROW_CUR['center'][2]));f=1-k
    z=c0.z+BROW_SOFT['dz']*f;lvl=ell_level(c0)  # 올린 만큼 머리 표면을 따라 안쪽(y)으로 붙인다
    ry=(lvl-((c0.x-HEAD_ELL[0][0])/HEAD_ELL[1][0])**2-((z-HEAD_ELL[0][2])/HEAD_ELL[1][2])**2)
    y=sg*HEAD_ELL[1][1]*math.sqrt(max(ry,0)) if f>0 else c0.y
    sc=Vector([BROW_CUR['scale'][i]*(1+(BROW_SOFT['scale'][i]-1)*f) for i in range(3)])
    pitch=BROW_CUR['pitch']+(BROW_SOFT['pitch']-BROW_CUR['pitch'])*f
    return Vector((c0.x,y,z)),sc,Euler((0,pitch,sg*BROW_CUR['yaw']),'XYZ')
def set_brow(co,k):
    for sg,p in BROWS.items():
        c0,s0,r0=brow_params(sg,1);c1,s1,r1=brow_params(sg,k);R0=r0.to_matrix();R1=r1.to_matrix()
        for i in p:
            u=R0.inverted()@(REST[i]-c0);u=Vector((u.x/s0.x,u.y/s0.y,u.z/s0.z))
            co[i]=c1+R1@Vector((u.x*s1.x,u.y*s1.y,u.z*s1.z))
    return co
# 되돌림 검산: k=1 은 실물과 같아야 한다
chk=set_brow([c.copy() for c in REST],1);err=max((chk[i]-REST[i]).length for p in BROWS.values() for i in p);print('brow roundtrip err %.2e'%err);assert err<1e-4

# 2) 눈썹 2단계(채도·발굽 반영 상태)
BROW_LEVELS={'current':1.0,'soft':0.0}
for name,k in BROW_LEVELS.items():
    apply(set_brow([c.copy() for c in co],k));shots['brow_'+name]=render('v2_brow_%s.png'%name,BROWCAM,(900,900))
# 3) v2 전신(완화 눈썹) + 참고: 머리 묶음 +8%
v2=set_brow([c.copy() for c in co],BROW_LEVELS['soft']);apply(v2);shots['v2']=render('v2_neutral.png',MAIN,(1200,900))
pivot=Vector((.43,0,.70))  # head 뼈 머리
big=[c.copy() for c in v2]
for p in HEAD_UNIT:
    for i in p:big[i]=pivot+(v2[i]-pivot)*HEAD_OPTION
apply(big);shots['v2_head']=render('v2_head108_neutral.png',MAIN,(1200,900))
apply(v2)

# ── 비교 시트(가로로 붙임, 사이 여백 12px) ──
def sheet(name,keys):
    imgs=[bpy.data.images.load(str(shots[k])) for k in keys];h=imgs[0].size[1];gap=12
    arrs=[np.array(im.pixels[:],dtype=np.float32).reshape(im.size[1],im.size[0],4) for im in imgs]
    pad=np.tile(np.array([.05,.05,.05,1],np.float32),(h,gap,1));row=[]
    for a in arrs:row+=[a,pad]
    out=np.concatenate(row[:-1],axis=1);img=bpy.data.images.new(name,out.shape[1],out.shape[0],alpha=True)
    img.pixels=out.ravel();img.filepath_raw=str(OUT/'preview'/name);img.file_format='PNG';img.save()
    for im in imgs+[img]:bpy.data.images.remove(im)
sheet('compare_neutral.png',['orig','v2','v2_head'])
sheet('compare_brow.png',['orig_brow','brow_current','brow_soft'])

# ── 사본 저장(카메라·렌더 경로 정리 후) ──
me.calc_loop_triangles();assert len(me.loop_triangles)==TRIS0 and len(me.vertices)==VERTS0
bpy.data.objects.remove(BROWCAM);s.camera=MAIN;s.render.resolution_x,s.render.resolution_y=1200,900;s.render.filepath=str(OUT/'preview'/'neutral.png')
dst=OUT/'pig_a_v2.blend';bpy.ops.wm.save_as_mainfile(filepath=str(dst),copy=True)
notes={'source':'pig_a.blend 사본 — build_pig_a_v2.py','measure_original':measure,'tris':TRIS0,'vertices':VERTS0,
       'sat_gain_srgb_hsv':SAT_GAIN,'colors_linear':{k:remap[tuple(round(x,3) for x in c)] for k,c in TARGET.items()},
       'hoof_chamfer_m':HOOF_CHAMFER,'brow_saved':'soft','brow_soft':BROW_SOFT,'head_scale_saved':1.0,'head_option_rendered':HEAD_OPTION,
       'previews':{k:str(v.relative_to(OUT)) for k,v in shots.items()}}
(OUT/'notes.json').write_text(json.dumps(notes,indent=2,ensure_ascii=False),encoding='utf-8');print('SAVED',dst)
