"""goose_a 귀여움 1단계(형태+렌더) — 원본 goose_a.blend 는 읽기만 하고, 결과는 goose_a_v2/ 에 따로 쓴다.

    py -3 Tools/Blender/run_headless.py cute_goose_a_v2.py [-- --final A|B]

레버 A: 목(보이는 구간)을 짧게 + 머리·부리 한 덩어리 확대. 레버 B: 목 길이 유지 + 머리 확대 + 목 윗부분 굵게.
공통: 부리 뭉툭(길이↓ 두께↑), 채도 상향(주황 선명, 몸은 흰~따뜻한 크림). 눈썹 다이얼 100/40/0%.
리그·클립은 손대지 않는다 — 렌더는 전부 레스트 포즈. 메시 위상(정점·삼각형 수)은 바꾸지 않는다.
"""
import bpy, sys, math
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix
sys.stdout.reconfigure(encoding='utf-8')
SRC = Path('D:/Work/unreal/CL9/ArtSource/monster/goose_a/goose_a.blend')
OUT = Path('D:/Work/unreal/CL9/ArtSource/monster/goose_a_v2')
argv = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else []
FINAL = argv[argv.index('--final') + 1] if '--final' in argv else None
(OUT / 'preview').mkdir(parents=True, exist_ok=True)

# ---- 수치(렌더 옵션 — 확정은 사용자 몫)
NECK_SHORTEN_A = 0.175   # 레버 A: 보이는 목 길이 -17.5% (브리프 15~20%)
HEAD_SCALE = 1.35        # 레버 A·B 공통: 머리+부리 한 덩어리 ×1.35 (브리프 30~40%)
NECK_THICK_B = 1.25      # 레버 B: 목 윗부분 반지름 ×1.25 (아래로 갈수록 1.0)
BILL = dict(x=0.88, z=1.35, pivot=(0.44, 0.0, 1.47))  # 부리: 길이 ×0.88, 두께 ×1.35 (부리 뿌리 기준)
BROW_ANGLE_OLD = 0.13    # 원본 눈썹 Y축 회전(앞쪽이 내려간 쐐기)
BROW_SOFT = dict(angle=-0.07, lift=0.020, thin=0.60)  # 눈썹 0% 쪽 끝값 — t 로 보간, 40% = 60% 만큼 이동
# 선형 RGB. 원본 → v2. 몸은 흰~크림 안에서만 따뜻하게, 주황은 선명하게.
PALETTE = {
    (0.72, 0.70, 0.62): (0.78, 0.74, 0.60),    # white
    (0.88, 0.84, 0.69): (0.92, 0.85, 0.64),    # cream
    (0.70, 0.24, 0.022): (0.95, 0.23, 0.005),  # orange(부리·다리·물갈퀴)
    (0.24, 0.072, 0.010): (0.34, 0.055, 0.003),  # bill_dark(부리 경계·콧구멍)
}

with bpy.data.libraries.load(str(SRC), link=False) as (src, dst):
    dst.scenes = ['CL9_GOOSE_A']
s = bpy.data.scenes['CL9_GOOSE_A']
base = bpy.data.objects['SK_Goose_A']; rig = bpy.data.objects['SKEL_Goose_A']
rig.animation_data.action = None
for p in rig.pose.bones: p.matrix_basis.identity()
assert len(base.data.vertices) == 4840

# ---- 섬(연결 성분) 분류 — 위치·색으로 찾는다
me0 = base.data
col0 = me0.color_attributes['Color']
vcol = {}
for loop in me0.loops: vcol[loop.vertex_index] = tuple(col0.data[loop.index].color)[:3]
adj = [[] for _ in me0.vertices]
for e in me0.edges:
    a, b = e.vertices; adj[a].append(b); adj[b].append(a)
remaining = set(range(len(adj))); islands = []
while remaining:
    todo = [remaining.pop()]; isl = []
    while todo:
        v = todo.pop(); isl.append(v)
        for n in adj[v]:
            if n in remaining: remaining.remove(n); todo.append(n)
    islands.append(sorted(isl))
def center(isl): return sum((me0.vertices[i].co for i in isl), Vector()) / len(isl)
def near(c, ref, tol=0.03): return abs(c[0] - ref[0]) < tol and abs(c[1] - ref[1]) < tol and abs(c[2] - ref[2]) < tol
def is_orange(i): return vcol[i][0] > 0.2 and vcol[i][2] < 0.05
neck = [i for i in islands if len(i) == 400]; assert len(neck) == 1; neck = neck[0]
head_isl = [i for i in islands if i is not neck and center(i).z > 1.38 and center(i).x > 0.2]
bill_isl = [i for i in head_isl if is_orange(i[0])]
brow_isl = [i for i in head_isl if len(i) == 86 and near(center(i), (0.485, center(i).y, 1.615)) and abs(abs(center(i).y) - 0.124) < 0.01]
assert len(head_isl) == 14 and len(bill_isl) == 5 and len(brow_isl) == 2, (len(head_isl), len(bill_isl), len(brow_isl))
SEG, RINGS = 16, 25
assert neck[0] == neck[-1] - RINGS * SEG + 1  # 링 순서가 인덱스 순서
rings = [[neck[0] + r * SEG + j for j in range(SEG)] for r in range(RINGS)]
ring_c = [sum((me0.vertices[i].co for i in ring), Vector()) / SEG for ring in rings]
exit_ring = next(r for r, c in enumerate(ring_c) if c.z >= 1.0)  # 가슴에서 목이 나오는 링

def smooth(x): x = max(0.0, min(1.0, x)); return x * x * (3 - 2 * x)

def make_variant(name, neck_shorten=0.0, head_scale=1.0, neck_thick=1.0, bill=True, brow=1.0, recolor=True):
    ob = base.copy(); ob.data = base.data.copy(); ob.name = name; ob.data.name = name + '_Mesh'
    s.collection.objects.link(ob)
    V = ob.data.vertices
    old = {i: V[i].co.copy() for i in range(len(V))}
    # 1) 부리: 부리 뿌리 기준 아핀 — 윗부리·경계·아랫부리·콧구멍이 서로 붙은 채로 같이 변한다
    if bill:
        P = Vector(BILL['pivot'])
        for isl in bill_isl:
            for i in isl:
                d = V[i].co - P; V[i].co = P + Vector((d.x * BILL['x'], d.y, d.z * BILL['z']))
    # 2) 눈썹 다이얼: t=1 원본 쐐기, 0<t<1 앞쪽을 들고 위로 올리고 얇게, t=0 제거(머리 안으로 접음)
    for isl in brow_isl:
        c = sum((V[i].co for i in isl), Vector()) / len(isl)
        if brow <= 0.0:
            for i in isl: V[i].co = Vector((0.43, 0.0, 1.53))
            continue
        u = 1.0 - brow
        rot = Matrix.Rotation((BROW_SOFT['angle'] - BROW_ANGLE_OLD) * u, 3, 'Y')
        for i in isl:
            d = rot @ (V[i].co - c); d.z *= 1.0 - (1.0 - BROW_SOFT['thin']) * u
            V[i].co = c + d + Vector((0, 0, BROW_SOFT['lift'] * u))
    # 3) 목: 가슴 출구 링 위만 출구 기준으로 줄이고, 윗부분을 굵게
    E = ring_c[exit_ring]; new_c = []
    for r, ring in enumerate(rings):
        c = ring_c[r]
        c2 = c if r <= exit_ring else E + (1.0 - neck_shorten) * (c - E)
        k = 1.0 + (neck_thick - 1.0) * smooth((r - exit_ring) / (RINGS - 1 - exit_ring))
        for i in ring: V[i].co = c2 + k * (old[i] - c)
        new_c.append(c2)
    # 4) 머리 덩어리: 목 끝 중심 기준 확대 후 목 끝 이동량만큼 따라간다
    T = ring_c[-1]; D = new_c[-1] - T
    for isl in head_isl:
        for i in isl: V[i].co = T + head_scale * (V[i].co - T) + D
    if recolor:
        ca = ob.data.color_attributes['Color']
        for d in ca.data:
            key = tuple(round(x, 3) for x in tuple(d.color)[:3])
            for k0, k1 in PALETTE.items():
                if all(abs(a - b) < 2e-3 for a, b in zip(key, k0)): d.color = (*k1, 1.0); break
    ob.data.update()
    return ob

def measure(ob):
    V = ob.data.vertices
    def dims(idx):
        cs = [V[i].co for i in idx]
        return [100 * (max(c[a] for c in cs) - min(c[a] for c in cs)) for a in range(3)]
    bill = [i for b in bill_isl for i in b]; bill_set = set(bill)
    head = [i for isl in head_isl for i in isl if i not in bill_set]
    rc = [sum((V[i].co for i in ring), Vector()) / SEG for ring in rings]
    vis = sum((rc[r + 1] - rc[r]).length for r in range(exit_ring, RINGS - 1))
    top_r = sum(((V[i].co - rc[-1]).length for i in rings[-1])) / SEG
    zmax = max(v.co.z for v in V)
    return dict(head_cm=[round(x, 1) for x in dims(head)], bill_cm=[round(x, 1) for x in dims(bill)],
                neck_visible_cm=round(100 * vis, 1), neck_top_radius_cm=round(100 * top_r, 1), height_cm=round(100 * zmax, 1))

# ---- 렌더 무대: 원본 미리보기와 같은 카메라·조명(파일에 저장된 PREVIEW 오브젝트), 레스트 포즈
cam = s.camera or bpy.data.objects['CL9_GOOSE_A_PREVIEW_Camera']; s.camera = cam
s.render.engine = 'CYCLES'; s.cycles.samples = 32
s.render.resolution_x = 1200; s.render.resolution_y = 900; s.render.resolution_percentage = 100
s.render.image_settings.file_format = 'PNG'
label_mat = bpy.data.materials.new('V2_Label'); label_mat.use_nodes = True
bs = next(n for n in label_mat.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
bs.inputs['Base Color'].default_value = (1, 1, 1, 1); bs.inputs['Emission Color'].default_value = (1, 1, 1, 1); bs.inputs['Emission Strength'].default_value = 3.0
def label(text):
    cu = bpy.data.curves.new('V2_Label', 'FONT'); cu.body = text; cu.size = 0.085
    ob = bpy.data.objects.new('V2_Label', cu); ob.data.materials.append(label_mat); s.collection.objects.link(ob)
    ob.visible_shadow = False
    # 배경 모드에서 덧붙인(append) 씬은 matrix_world 가 평가 전(단위 행렬)이라 loc/rot 로 직접 계산한다
    assert cam.parent is None
    R = cam.rotation_euler.to_matrix(); half = cam.data.ortho_scale / 2
    ob.location = cam.location + R @ Vector((-half * 0.95, half * 0.75 * 0.92 - 0.06, -2.5))
    ob.rotation_euler = cam.rotation_euler
    return ob

meshes = []
def render(ob, text, path):
    for m in meshes: m.hide_render = m is not ob
    base.hide_render = ob is not base
    lab = label(text)
    s.render.filepath = str(path); bpy.ops.render.render(write_still=True, scene=s.name)
    bpy.data.objects.remove(lab); print('rendered', path)
    return path

def stitch(paths, out):
    imgs = [bpy.data.images.load(str(p)) for p in paths]
    w, h = imgs[0].size
    arr = [np.array(im.pixels[:], dtype=np.float32).reshape(h, w, 4) for im in imgs]
    big = np.concatenate(arr, axis=1)
    o = bpy.data.images.new('stitch', width=w * len(imgs), height=h, alpha=True)
    o.pixels.foreach_set(big.ravel()); o.filepath_raw = str(out); o.file_format = 'PNG'; o.save()
    print('stitched', out)

P = OUT / 'preview'
orig = base
varA = make_variant('VAR_LeverA', neck_shorten=NECK_SHORTEN_A, head_scale=HEAD_SCALE); meshes.append(varA)
varB = make_variant('VAR_LeverB', head_scale=HEAD_SCALE, neck_thick=NECK_THICK_B); meshes.append(varB)
print('measure original', measure(orig)); print('measure A', measure(varA)); print('measure B', measure(varB))

if FINAL is None:
    a = render(orig, 'ORIGINAL (goose_a, rest pose)', P / 'v2_original.png')
    b = render(varA, 'A: neck -17.5%, head+bill x1.35', P / 'v2_leverA.png')
    c = render(varB, 'B: neck same, head x1.35, neck top x1.25', P / 'v2_leverB.png')
    stitch([a, b, c], P / 'compare_lever.png')
else:
    lever = dict(A=dict(neck_shorten=NECK_SHORTEN_A, head_scale=HEAD_SCALE), B=dict(head_scale=HEAD_SCALE, neck_thick=NECK_THICK_B))[FINAL]
    brows = []
    for t in (1.0, 0.4, 0.0):
        ob = make_variant('VAR_Brow%03d' % int(t * 100), brow=t, **lever); meshes.append(ob)
        brows.append(render(ob, 'lever %s, brow %d%%' % (FINAL, int(t * 100)), P / ('v2_brow%03d.png' % int(t * 100))))
    stitch(brows, P / 'compare_brow.png')
    final = make_variant('SK_Goose_A_v2', brow=0.4, **lever); meshes.append(final)
    print('measure final', measure(final))
    render(final, 'v2 candidate: lever %s, brow 40%%' % FINAL, P / 'neutral_v2.png')
    stitch([P / 'v2_original.png', P / 'neutral_v2.png'], P / 'compare_final.png')
    # v2 원본: 최종 후보를 SK_Goose_A 이름으로, 원본·비교 변형은 렌더 제외로 보관. 리그는 아직 새 비율에 맞추지 않았다.
    base.name = 'REF_Original_goose_a'; base.hide_render = True; base.hide_viewport = True
    for m in meshes:
        if m is not final: m.hide_render = True; m.hide_viewport = True
    final.hide_render = False; final.hide_viewport = False
    final.name = 'SK_Goose_A'; final.data.name = 'SK_Goose_A'
    s.name = 'CL9_GOOSE_A_V2'
    bpy.data.libraries.write(str(OUT / 'goose_a_v2.blend'), {s}, fake_user=True)
    print('saved', OUT / 'goose_a_v2.blend')
