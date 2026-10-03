"""sheep_a "귀엽게" 1단계 시안 — 형태+렌더만. 리그·클립·FBX 는 만들지 않는다.

    py -3 Tools/Blender/run_headless.py build_sheep_a_v2.py

원본 build_sheep_a.py 의 부위 좌표를 그대로 옮기고, 얼굴 배율(S)·주둥이 길이(m)·눈썹 강도(t)·귀 길이(e)·털 뭉치 크기(w)·팔레트만
매개변수로 바꾼 변형 여러 개를 한 장면에 만든다. 모든 매개변수가 1 이고 팔레트가 원본이면 원본과 같은 형상이다(삼각형 수로 검사).
기존 ArtSource/monster/sheep_a/ 의 blend·export 는 읽지도 쓰지도 않는다. 산출물은 ArtSource/monster/sheep_a_v2/ 에만 쓴다.
"""
import bpy,sys,math,json
import numpy as np
from pathlib import Path
from mathutils import Vector,Euler
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_meshkit import MeshKit

OUT=Path('D:/Work/unreal/CL9/ArtSource/monster/sheep_a_v2')
TMP=OUT/'preview/_columns'
REF=Path('D:/Work/unreal/CL9/docs/reference/image/design/2.png')
for d in (OUT/'preview',TMP):d.mkdir(parents=True,exist_ok=True)

# 원본 팔레트(build_sheep_a.py 그대로)와 v2 팔레트(design-concept 규칙 2 — 털은 흰색~따뜻한 크림, 얼굴 회색은 대비를 더 또렷하게).
PAL_ORIG={'wool':(.64,.60,.46,1),'light':(.82,.77,.59,1),'shade':(.39,.38,.29,1),
          'skin':(.043,.050,.041,1),'inner':(.16,.11,.085,1),'hoof':(.014,.016,.013,1),
          'eye':(.50,.39,.17,1),'black':(.005,.006,.004,1)}
PAL_V2={'wool':(.84,.81,.70,1),'light':(.94,.92,.85,1),'shade':(.55,.52,.44,1),
        'skin':(.024,.027,.031,1),'inner':(.42,.14,.13,1),'hoof':(.008,.009,.010,1),
        'eye':(.66,.47,.10,1),'black':(.003,.003,.004,1)}
PIVOT=Vector((.36,0,.83))  # 얼굴 확대 기준점 = 원본 Neck 중심
MUZZLE_TILT=.23

# 변형 목록 — S 얼굴 배율, m 주둥이 길이(1=원본), t 눈썹 강도(1=현재, 0.5=완화), e 귀 길이, w 털 뭉치 구체 크기.
VARIANTS={
    'orig':      dict(S=1.00,m=1.00,t=1.0,e=1.00,w=1.00,pal='orig',label='original'),
    's120':      dict(S=1.20,m=.72,t=1.0,e=.88,w=1.00,pal='v2',label='face x1.20'),
    's128':      dict(S=1.28,m=.72,t=1.0,e=.88,w=1.00,pal='v2',label='face x1.28'),
    's135':      dict(S=1.35,m=.72,t=1.0,e=.88,w=1.00,pal='v2',label='face x1.35'),
    's128_mild': dict(S=1.28,m=.72,t=0.5,e=.88,w=1.00,pal='v2',label='x1.28 brow soft'),
    's128_long': dict(S=1.28,m=1.00,t=1.0,e=.88,w=1.00,pal='v2',label='x1.28 muzzle orig'),
    's128_wool': dict(S=1.28,m=.72,t=1.0,e=.88,w=1.12,pal='v2',label='x1.28 wool x1.12'),
}
# 아픈 눈 프리셋(피격 HitReact) — 눈썹 강도 t 를 대신한다. 화난 눈(t=1, 눈썹이 눈 윗절반을 덮어 노려봄)과 반대로 안쪽(콧등 쪽, +X) 끝을
# 크게 들어 찡그린 인상을 만든다. fwd 눈썹을 눈 위로 당기는 거리·lift 올리는 높이(m, 얼굴 배율 적용 전, 음수는 내림),
# tilt 안쪽 끝을 드는 기울기(rad), squint 눈·눈동자의 세로 배율.
PAIN={
    'a': dict(fwd=.000,lift=.024,tilt=.55,squint=1.00),
    'b': dict(fwd=.020,lift=-.006,tilt=.55,squint=.55),
    'c': dict(fwd=.020,lift=-.012,tilt=.60,squint=.22),
}
for pk,label in [('a','pain A brow up'),('b','pain B + squint'),('c','pain C shut')]:
    VARIANTS['s128_pain_'+pk]=dict(S=1.28,m=.72,t=0.5,e=.88,w=1.00,pal='v2',pain=pk,label=label)
PAIN_PICK='b'  # 후보 비교 렌더를 보고 고른 아픈 눈 — 3단 비교 렌더의 피격 칸에 쓴다

def new_kit(key,palette,material):
    # MeshKit.__init__ 은 창 장면을 바꾸고 sheep_a/ 폴더를 만지므로 쓰지 않는다 — 부위 생성 메서드(ell·taper·finish)만 빌린다.
    k=MeshKit.__new__(MeshKit);k.species='SheepV2_'+key;k.palette=palette;k.material=material;k.parts=[];return k

def build(k,S,m,t,e,w,pain=None):
    drop=(S-1)*.15  # 얼굴이 커진 만큼 머리를 털 뭉치 쪽으로 낮춘다(정수리가 털 위로 솟지 않게)
    def fx(loc,size):
        l=PIVOT+(Vector(loc)-PIVOT)*S;l.z-=drop
        return tuple(l),tuple(v*S for v in size)
    groups={'face':[],'body':[]}
    body=k.ell('FleeceBody',(-.19,0,.73),(.57,.36,.36),'wool','body',24,16);k.chest_weights(body);groups['body'].append(body)
    index=0
    for x,rx in [(-.57,.21),(-.32,.235),(-.05,.24),(.20,.21)]:
        for angle in [-.90,-.20,.50,1.20,1.90,2.60,3.30,4.00]:
            y=.276*math.cos(angle);z=.73+.286*math.sin(angle)
            tuft=k.ell('Wool'+str(index),(x,y,z),(rx*w,.145*w,.158*w),'light' if index%4==0 else 'wool','body',12,8,rotation=(.1*math.sin(index),0,.09*math.cos(index)))
            k.chest_weights(tuft);groups['body'].append(tuft);index+=1
    groups['body'].append(k.ell('Chest',(.255,0,.71),(.22,.27,.28),'wool','chest',20,12))
    # 목은 얼굴 배율의 절반만 키운다(기준점이 목 중심이라 위치는 그대로).
    n=1+(S-1)/2;k.ell('Neck',(.36,0,.83),(.19*n,.19*n,.23*n),'skin','neck')
    l,s=fx((.48,0,.91),(.19,.16,.245));groups['face'].append(k.ell('Head',l,s,'skin','head',20,14,rotation=(0,-.20,0)))
    # 주둥이 — m<1 이면 짧고 둥글게: 길이를 줄이고 폭·높이를 조금 키운다. 코·턱은 주둥이 끝을 따라간다(m=1 이면 원본 좌표).
    mu_x=.48+.16*m;mu_s=(.173*m,.139*(1+.35*(1-m)),.12*(1+.45*(1-m)))
    tip_shift=(mu_x+mu_s[0]*math.cos(MUZZLE_TILT))-(.64+.173*math.cos(MUZZLE_TILT))
    l,s=fx((mu_x,0,.757),mu_s);groups['face'].append(k.ell('Muzzle',l,s,'skin','head',18,12,rotation=(0,MUZZLE_TILT,0)))
    l,s=fx((.773+tip_shift,0,.737),(.039,.112*(1-.15*(1-m)),.068));groups['face'].append(k.ell('Nose',l,s,'hoof','head',14,10))
    l,s=fx((.65+(mu_x-.64),0,.689),(.157*m,.118,.04));groups['face'].append(k.ell('Jaw',l,s,'skin','jaw',16,10))
    for i,(x,y,z) in enumerate([(.40,-.07,1.091),(.39,.07,1.095),(.52,-.045,1.083),(.52,.055,1.08),(.34,0,1.063)]):
        l,s=fx((x,y,z),(.105,.099,.081));k.ell('Forelock'+str(i),l,s,'light','head',12,8)
    for sign,side in [(1,'l'),(-1,'r')]:
        q=PAIN[pain]['squint'] if pain else 1.0
        l,s=fx((.593,sign*.126,.926),(.060,.030,.043*q));k.ell('Eye_'+side,l,s,'eye','head',14,10,rotation=(0,0,-sign*.35))
        l,s=fx((.611,sign*.15,.926),(.035,.010,.014*q));k.ell('Pupil_'+side,l,s,'black','head',12,8)
        if pain:
            # 아픈 눈 — 완화 눈썹(t=0.5) 두께로, 위로 올리고 안쪽 끝을 크게 든다(바깥 끝은 눈꼬리 쪽으로 처진다).
            p=PAIN[pain]
            l,s=fx((.56+p['fwd'],sign*.14,.968+p['lift']),(.071*.95,.036,.029*.75))
            k.ell('Brow_'+side,l,s,'skin','head',14,8,rotation=(0,-p['tilt'],0))
        else:
            # 눈썹 — t=1 현재(두꺼운 눈꺼풀이 눈 윗절반을 덮는다). t 가 줄면 위로 올리고 얇게 하며 앞(콧등 쪽) 끝을 들어 순한 기울기로.
            l,s=fx((.56,sign*.14,.968+(1-t)*.020),(.071*(.9+.1*t),.036,.029*(.5+.5*t)))
            k.ell('Brow_'+side,l,s,'skin','head',14,8,rotation=(0,-.28*(1-t),0))
        # 귀 — 뿌리를 고정하고 긴 축(로컬 Y)만 e 배로 줄인다.
        rot=Euler((sign*.20,0,sign*.12))
        root=Vector((.396,sign*.264,1.00))-Vector((0,sign*.192,0))@rot.to_matrix().transposed()
        def ear(loc,size):
            v=(Vector(loc)-root);loc2=root+v*e
            return fx(tuple(loc2),(size[0],size[1]*e,size[2]))
        l,s=ear((.396,sign*.264,1.00),(.11,.192,.056));k.ell('Ear_'+side,l,s,'skin','ear_'+side,16,10,rotation=(sign*.20,0,sign*.12))
        l,s=ear((.41,sign*.278,1.035),(.074,.143,.018));k.ell('InnerEar_'+side,l,s,'inner','ear_'+side,14,8,rotation=(sign*.20,0,sign*.12))
        for pair,x,knee_x in [('front',.27,.32),('hind',-.40,-.53)]:
            tag=pair+'_'+side;y=sign*(.215 if pair=='front' else .235);hip=(x,y,.48);knee=(knee_x,y,.28);ankle=(x+.035,y,.07)
            k.ell('Haunch_'+tag,hip,(.137*w,.124*w,.147*w),'wool',tag+'_upper',16,10)
            k.taper('Upper_'+tag,hip,knee,.068,.042,'skin',tag+'_upper')
            k.taper('Lower_'+tag,knee,ankle,.042,.033,'skin',tag+'_lower')
            for j in (-1,1):
                k.ell('Hoof_'+tag+str(j),(ankle[0]+.024,y+j*.031,.053),(.067,.028,.041),'hoof',tag+'_paw',12,8)
    for i in range(4):
        k.ell('Tail'+str(i),(-.73-.017*i,0,.76-.032*i),(.087-.01*i,.068-.006*i,.071),'wool',f'tail_{i+1:02d}',12,8)
    return groups

def bbox(obs):
    pts=[ob.matrix_world@v.co for ob in obs for v in ob.data.vertices]
    lo=Vector((min(p.x for p in pts),min(p.y for p in pts),min(p.z for p in pts)))
    hi=Vector((max(p.x for p in pts),max(p.y for p in pts),max(p.z for p in pts)))
    return lo,hi

# --- 장면 준비: 시작 파일의 물체를 치우고, 새 장면과 같은 색 관리 기본값을 쓴다 -------------------------------------
scene=bpy.context.scene;scene.name='CL9_SHEEP_A_V2'
for ob in list(bpy.data.objects):bpy.data.objects.remove(ob,do_unlink=True)
probe=bpy.data.scenes.new('_probe');scene.view_settings.view_transform=probe.view_settings.view_transform
scene.view_settings.look=probe.view_settings.look;bpy.data.scenes.remove(probe)
scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=1;scene.render.fps=30
material=bpy.data.materials.new('M_CL9_SheepV2_Vertex');material.use_nodes=True
bs=next(n for n in material.node_tree.nodes if n.type=='BSDF_PRINCIPLED');bs.inputs['Roughness'].default_value=.86
attr=material.node_tree.nodes.new('ShaderNodeVertexColor');attr.layer_name='Color';material.node_tree.links.new(attr.outputs['Color'],bs.inputs['Base Color'])

meshes={};measure={}
for key,p in VARIANTS.items():
    col=bpy.data.collections.new('V_'+key);scene.collection.children.link(col)
    lc=bpy.context.view_layer.layer_collection.children[col.name];bpy.context.view_layer.active_layer_collection=lc
    k=new_kit(key,PAL_ORIG if p['pal']=='orig' else PAL_V2,material)
    g=build(k,p['S'],p['m'],p['t'],p['e'],p['w'],p.get('pain'))
    fl,fh=bbox(g['face']);bl,bh=bbox(g['body'])
    bpy.ops.object.select_all(action='DESELECT')
    for ob in k.parts:ob.select_set(True)
    bpy.context.view_layer.objects.active=k.parts[0];bpy.ops.object.join();mesh=bpy.context.object;mesh.name='SM_SheepV2_'+key
    scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    mesh.data.calc_loop_triangles();meshes[key]=mesh
    face=fh-fl;body=bh-bl
    measure[key]={**{a:p[a] for a in ('S','m','t','e','w','pal')},**({'pain':PAIN[p['pain']]} if 'pain' in p else {}),
        'face_bbox_cm':[round(v*100,1) for v in face],'body_bbox_cm':[round(v*100,1) for v in body],
        'face_to_body_height':round(face.z/body.z,3),'face_to_body_length':round(face.x/body.x,3),
        'face_to_body_area_sqrt':round(math.sqrt(face.x*face.z/(body.x*body.z)),3),
        'face_top_z_cm':round(fh.z*100,1),'body_top_z_cm':round(bh.z*100,1),
        'dimensions_cm':[round(v*100,1) for v in mesh.dimensions],'triangles':len(mesh.data.loop_triangles)}
base=measure['orig']
for key,m in measure.items():
    m['height_ratio_vs_orig']=round(m['face_to_body_height']/base['face_to_body_height'],3)
    m['area_sqrt_ratio_vs_orig']=round(m['face_to_body_area_sqrt']/base['face_to_body_area_sqrt'],3)
    print('MEASURE',key,json.dumps(m))
assert base['triangles']==14776,'원본 재현 실패 — 삼각형 수 %d != 14776'%base['triangles']

# --- 렌더 무대 (creature_preview.py 와 같은 조명·배경·바닥, 창 없이) ---------------------------------------------------
scene.render.engine='CYCLES';scene.cycles.samples=24;scene.render.resolution_percentage=100
scene.render.image_settings.media_type='IMAGE';scene.render.image_settings.file_format='PNG'
scene.world=bpy.data.worlds.new('SheepV2_World');scene.world.use_nodes=True
bg=next(n for n in scene.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[0].default_value=(.12,.15,.18,1);bg.inputs[1].default_value=.35
stage=bpy.data.collections.new('Stage');scene.collection.children.link(stage)
def look(o,focus):o.rotation_euler=(Vector(focus)-o.location).to_track_quat('-Z','Y').to_euler()
for label,pos,power,size in [('Key',(2,-3,4),420,3),('Fill',(-1,2,2),150,2)]:
    ld=bpy.data.lights.new('SheepV2_'+label,'AREA');lo=bpy.data.objects.new(ld.name,ld);stage.objects.link(lo)
    lo.location=pos;ld.energy=power;ld.shape='DISK';ld.size=size;look(lo,(0,0,.55))
fm=bpy.data.meshes.new('SheepV2_Floor');fm.from_pydata([(-100,-100,0),(100,-100,0),(100,100,0),(-100,100,0)],[],[(0,1,2,3)])
floor=bpy.data.objects.new('SheepV2_Floor',fm);stage.objects.link(floor)
fmat=bpy.data.materials.new('SheepV2_FloorMat');fmat.use_nodes=True
fb=next(n for n in fmat.node_tree.nodes if n.type=='BSDF_PRINCIPLED');fb.inputs['Base Color'].default_value=(.075,.086,.09,1);fb.inputs['Roughness'].default_value=.9
fm.materials.append(fmat)
cd=bpy.data.cameras.new('SheepV2_Camera');cam=bpy.data.objects.new(cd.name,cd);stage.objects.link(cam);cd.type='ORTHO';scene.camera=cam
tmat=bpy.data.materials.new('SheepV2_Label');tmat.use_nodes=True
tb=next(n for n in tmat.node_tree.nodes if n.type=='BSDF_PRINCIPLED');tb.inputs['Base Color'].default_value=(0,0,0,1)
tb.inputs['Emission Color'].default_value=(.9,.9,.9,1);tb.inputs['Emission Strength'].default_value=1.0
tc=bpy.data.curves.new('SheepV2_LabelText','FONT');tc.align_x='CENTER';tc.materials.append(tmat)
label=bpy.data.objects.new('SheepV2_Label',tc);stage.objects.link(label)

def shoot(key,path,cam_pos,focus,ortho,res,text):
    for kk,ob in meshes.items():ob.hide_render=(kk!=key);ob.location=(0,0,0)
    scene.render.resolution_x,scene.render.resolution_y=res
    cam.location=cam_pos;cd.ortho_scale=ortho;look(cam,focus);bpy.context.view_layer.update()
    mw=cam.matrix_world;up=Vector(mw.col[1][:3]);back=Vector(mw.col[2][:3])
    h=ortho*res[1]/res[0]
    label.hide_render=not text;tc.body=text or '';tc.size=ortho*.045
    label.location=Vector(focus)-up*(h/2-ortho*.05)+back*1.5;label.rotation_euler=cam.rotation_euler
    scene.render.filepath=str(path);bpy.ops.render.render(write_still=True);return path

def load_px(path):
    img=bpy.data.images.load(str(path));a=np.empty(len(img.pixels),dtype=np.float32);img.pixels.foreach_get(a)
    a=a.reshape(img.size[1],img.size[0],4);bpy.data.images.remove(img);return a
def save_px(a,path):
    h,w=a.shape[:2];img=bpy.data.images.new(path.stem,w,h,alpha=False);img.pixels.foreach_set(a.ravel())
    img.filepath_raw=str(path);img.file_format='PNG';img.save();bpy.data.images.remove(img)
def strip(keys,path,labels=None,**view):
    labels=labels or [VARIANTS[k]['label'] for k in keys]
    cols=[load_px(shoot(k,TMP/(path.stem+'_'+k+'.png'),text=t,**view)) for k,t in zip(keys,labels)]
    save_px(np.concatenate(cols,axis=1),path);print('WROTE',path)

STD=dict(cam_pos=(2.6,-3.8,2.0),focus=(0,0,.54),ortho=2.55,res=(1000,750))          # neutral.png 와 같은 3/4 카메라
FACE=dict(cam_pos=(.66+1.7,-1.35,.92+.45),focus=(.66,0,.92),ortho=1.2,res=(900,750))  # 얼굴 근접(오른쪽 앞 3/4)
REFV=dict(cam_pos=(3.3,2.9,1.55),focus=(.05,0,.58),ortho=2.3,res=(900,900))           # 2.png 와 같은 쪽(왼쪽 앞 3/4)

P=OUT/'preview'
# `-- --pain` 이면 아픈 눈 렌더 두 장만 새로 찍는다(앞 실행의 렌더는 그대로 둔다). blend·measurements 는 항상 다시 쓴다.
if '--pain' not in sys.argv:
    strip(['orig','s120','s128','s135'],P/'compare_face_scale.png',**STD)
    strip(['orig','s128','s128_mild'],P/'compare_brow.png',**FACE)
    strip(['s128_long','s128','s128_wool'],P/'compare_options.png',**STD)
    shoot('s128_mild',P/'neutral.png',text=None,**{**STD,'res':(1200,900)})
    # 레퍼런스 나란히 — 2.png 를 렌더 높이에 맞춰 늘려 왼쪽에 붙인다.
    v=load_px(shoot('s128_mild',TMP/'ref_v2.png',text=None,**REFV))
    ref=bpy.data.images.load(str(REF));ref.scale(round(ref.size[0]*v.shape[0]/ref.size[1]),v.shape[0])
    r=np.empty(len(ref.pixels),dtype=np.float32);ref.pixels.foreach_get(r);r=r.reshape(ref.size[1],ref.size[0],4);r[...,3]=1;bpy.data.images.remove(ref)
    save_px(np.concatenate([r,v],axis=1),P/'compare_reference.png');print('WROTE compare_reference')
# 아픈 눈 — 후보 비교(얼굴 근접, 맨 왼쪽은 대비용 화난 눈)와 게임 상태 3단(평소 t=0.5 | 추격 t=1.0 | 피격 아픈 눈, neutral.png 카메라).
strip(['s128','s128_pain_a','s128_pain_b','s128_pain_c'],P/'compare_pain_candidates.png',
      labels=['angry t=1.0 (ref)','pain A brow up','pain B + squint','pain C shut'],**FACE)
strip(['s128_mild','s128','s128_pain_'+PAIN_PICK],P/'compare_three_states.png',
      labels=['idle: brow t=0.5','chase: brow t=1.0','hit: pain '+PAIN_PICK.upper()],**STD)

# --- 저장: 변형들을 옆으로 늘어놓은 상태로 blend 에 남긴다(원점 기준 좌표는 위 측정값) ----------------------------------
for i,key in enumerate(VARIANTS):meshes[key].location=(0,-2.0*i,0);meshes[key].hide_render=False
label.hide_render=True
(OUT/'measurements.json').write_text(json.dumps({'pivot':list(PIVOT),'variants':measure},indent=2),encoding='utf-8')
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'sheep_a_v2.blend'))
print('SAVED sheep_a_v2')
