"""chicken_a v2 — 아픈 눈(피격) 후보를 chicken_a_v2.blend 에 더하고 비교 렌더를 찍는다.

    py -3 Tools/Blender/run_headless.py build_chicken_a_v2_pain.py --blend ArtSource/monster/chicken_a/v2/chicken_a_v2.blend [-- --pick pain_x]

형상은 build_chicken_a_v2.py 의 빌더(팔레트·Chicken·build·PAINS)를 그대로 가져다 쓴다 — 그 파일은 실물 blend 를 여는 본 실행이
위에 붙어 있어 import 할 수 없으므로, '# 실물 팔레트' 부터 '# 1) 빌더 검증' 앞까지만 잘라 실행한다.
빌더가 같다는 증거로 v2B_brow000·v2B_brow100 을 다시 지어 blend 안의 메시와 정점 좌표가 같은지 먼저 확인한다.
비율안은 B(몸 0.9배), 값은 measure.json 의 B 행에서 읽는다. 기존 오브젝트(A/B × 눈썹 3단)는 건드리지 않는다.

--pick 이 없으면 후보 근접 렌더와 compare_pain_candidates 용 타일만 찍고 저장하지 않는다.
--pick 을 주면 3단(평소 k=0 | 추격 k=1.0 | 피격) 3/4 렌더를 더 찍고, 후보 메시를 더한 채 blend 를 저장한다.
시트는 Blender 밖에서 contact_sheet.py 로 묶는다.
"""
import bpy,bmesh,json,math,sys
from pathlib import Path
from mathutils import Vector,Matrix,Euler

V2=Path(bpy.data.filepath).resolve()
assert V2.name=='chicken_a_v2.blend','chicken_a_v2.blend 를 --blend 로 열어야 한다'
PREV=V2.parent/'preview'
argv=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
pick=argv[argv.index('--pick')+1] if '--pick' in argv else None
scene=bpy.context.scene
base0=bpy.data.objects['SK_Chicken_A_v2B_brow000'];base1=bpy.data.objects['SK_Chicken_A_v2B_brow100']
material=base0.data.materials[0]

# 1) 빌더 가져오기
src=(Path(__file__).resolve().parent/'build_chicken_a_v2.py').read_text(encoding='utf-8')
i0=src.index('# 실물 팔레트');i1=src.index('# 1) 빌더 검증')
ns={'bpy':bpy,'bmesh':bmesh,'math':math,'Vector':Vector,'Matrix':Matrix,'Euler':Euler,'scene':scene,'material':material}
exec(compile(src[i0:i1],'build_chicken_a_v2.py[builder]','exec'),ns)
build,NEW,PAINS=ns['build'],ns['NEW'],ns['PAINS']
assert pick is None or pick in PAINS,pick
mb=json.loads((V2.parent/'measure.json').read_text(encoding='utf-8'))['variants']['SK_Chicken_A_v2B_brow100']
VB=dict(s_head=mb['head_scale'],c_head=tuple(mb['head_center_m']),body_k=mb['body_scale'])
print('VARIANT_B',VB)

def same(a,b):
    va=[v.co for v in a.data.vertices];vb=[v.co for v in b.data.vertices]
    return len(va)==len(vb) and max((x-y).length for x,y in zip(va,vb))<1e-6
for ref,k in ((base0,0.0),(base1,1.0)):
    chk,_=build('CHK_'+ref.name,NEW,brow_k=k,cute_face=True,**VB)
    ok=same(chk,ref);print('REBUILD_MATCH',ref.name,ok);assert ok
    bpy.data.objects.remove(chk)

# 2) 아픈 눈 후보 메시 — 다시 돌리면 같은 이름을 지우고 새로 짓는다
cands={}
for pk,pv in PAINS.items():
    name=f'SK_Chicken_A_v2B_{pk}'
    if name in bpy.data.objects:bpy.data.objects.remove(bpy.data.objects[name])
    ob,info=build(name,NEW,cute_face=True,pain=pv,**VB);cands[pk]=ob
    ob['pain_preset']=json.dumps(pv)
    print(name,'tris',info['triangles'],'verts',info['vertices'],'min_z_cm',info['min_z_cm'],json.dumps(pv))

# 3) 렌더 — 카메라·조명은 blend 에 저장된 것(neutral.png 와 같은 3/4 카메라, 머리 근접 카메라)을 그대로 쓴다.
main_cam=bpy.data.objects['CL9_CHICKEN_Camera'];close_cam=bpy.data.objects['CL9_CHICKEN_V2_Close']
# 근접 카메라는 build_chicken_a_v2.py 와 같은 식으로 B 머리에 다시 맞춘다(저장 상태도 B 지만 식으로 고정)
view_dir=(Vector((0,0,.65))-main_cam.location).normalized();head=Vector(VB['c_head'])+Vector((.06,0,.04))
close_cam.location=head-view_dir*5;close_cam.rotation_euler=view_dir.to_track_quat('-Z','Y').to_euler()
meshes=[o for o in scene.objects if o.type=='MESH' and o.name.startswith('SK_Chicken_A_v2')]
state={o.name:(o.hide_render,o.hide_viewport) for o in meshes}
scene.render.image_settings.file_format='PNG';scene.render.resolution_x=scene.render.resolution_y=1000;scene.cycles.samples=32
def shot(target,cam,fname):
    for o in meshes:o.hide_render=o is not target
    scene.camera=cam;scene.render.filepath=str(PREV/fname);bpy.ops.render.render(write_still=True);print('WROTE',fname)
for pk,ob in cands.items():shot(ob,close_cam,f'v2B_{pk}_close.png')
if pick:
    shot(base0,main_cam,'v2B_brow000_34.png');shot(cands[pick],main_cam,f'v2B_{pick}_34.png')
    # 3/4 추격(k=1.0)은 build_chicken_a_v2.py 가 찍은 v2B_34.png 를 쓴다

# 4) 저장 — --pick 일 때만. 기존 오브젝트의 보이기 상태를 되돌리고 새 후보는 숨긴다.
if pick:
    for o in meshes:
        o.hide_render,o.hide_viewport=state.get(o.name,(True,True))
    for o in cands.values():o.hide_render=o.hide_viewport=True
    scene['pain_pick']=pick;scene.camera=main_cam
    bpy.ops.wm.save_mainfile()
    print('SAVED',V2,'pick',pick)
