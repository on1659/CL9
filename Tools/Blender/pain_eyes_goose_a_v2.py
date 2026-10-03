"""goose_a v2 "아픈 눈"(피격 HitReact) 눈썹 프리셋 — 후보 비교 렌더 + 평소|추격|피격 3단 비교 렌더.

    py -3 Tools/Blender/run_headless.py pain_eyes_goose_a_v2.py --blend ArtSource/monster/goose_a_v2/goose_a_v2.blend [-- --pick P3 | -- --measure]

기반은 cute_goose_a_v2.py 가 저장한 goose_a_v2.blend 다. 레버 A 메시(VAR_Brow100 = 레버 A + 원본 쐐기 눈썹)를 복사해
눈썹 섬만 옮긴다(위상 그대로). 기존 오브젝트(레버 A/B·눈썹 단계·SK_Goose_A)는 손대지 않고, 후보를 VAR_Pain_* 로 덧붙이고
씬 사용자 속성 cl9_brow_presets 에 평소/추격/피격 파라미터를 JSON 으로 남긴다. 렌더는 전부 레스트 포즈.
--pick 이 없으면 후보 비교만 렌더하고 저장하지 않는다. --pick Pn 이면 3단 비교까지 렌더하고 blend 에 저장한다.
--measure 는 실루엣·기울기 수치만 찍는다(렌더·저장 없음). 저장된 blend 에 다시 --pick 을 돌리면 VAR_Pain_* 가 중복되니
그때는 cute_goose_a_v2.py --final A 로 blend 를 먼저 다시 만든다.

눈썹 각도 부호는 cute_goose_a_v2.py 와 같다: Y축 회전 +는 앞쪽(부리 쪽 = 안쪽 끝)이 내려감(화남), -는 앞쪽이 올라감(걱정/아픔).
"""
import bpy, sys, json, math
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix, Euler
from mathutils.bvhtree import BVHTree
sys.stdout.reconfigure(encoding='utf-8')
OUT = Path('D:/Work/unreal/CL9/ArtSource/monster/goose_a_v2')
P = OUT / 'preview'
argv = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else []
PICK = argv[argv.index('--pick') + 1] if '--pick' in argv else None

BROW_ANGLE_OLD = 0.13  # 원본 쐐기 각도(cute_goose_a_v2.py 와 같은 값)
# 아픈 눈 후보. angle = 절대 Y축 각도(rad), lift = 회전 뒤 위로(m),
# embed = 표면에 붙일 때 가장 낮은 점이 표면 안쪽으로 들어가는 깊이(m), keep = 표면 법선 방향 두께 배율 — 먼 쪽 눈썹이 머리 실루엣 밖으로
# 나가지 않게 하는 두 값. dx = 붙인 뒤 앞(+)/뒤(-) 이동(m). squint = 눈(흰자·홍채·동공) 세로 배율(아래 끝 고정, 1 = 그대로).
PAIN = {
    'P1': dict(angle=-0.45, lift=0.060, embed=0.004, keep=0.60, dx=0.010, squint=1.00),
    'P2': dict(angle=-0.70, lift=0.065, embed=0.004, keep=0.60, dx=0.010, squint=1.00),
    'P3': dict(angle=-0.70, lift=0.045, embed=0.004, keep=0.60, dx=0.010, squint=0.72),
}
DESC = {'P1': 'mild worry', 'P2': 'strong worry', 'P3': 'strong worry + squint'}

s = bpy.data.scenes['CL9_GOOSE_A_V2']
rig = bpy.data.objects['SKEL_Goose_A']
if rig.animation_data: rig.animation_data.action = None
for pb in rig.pose.bones: pb.matrix_basis.identity()
ref100 = s.objects['VAR_Brow100']; ref000 = s.objects['VAR_Brow000']
assert len(ref100.data.vertices) == 4840

# ---- 섬 분류 (레버 A 좌표에서)
me0 = ref100.data
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
def center(me, isl): return sum((me.vertices[i].co for i in isl), Vector()) / len(isl)
head_isl = [i for i in islands if len(i) != 400 and center(me0, i).z > 1.30 and center(me0, i).x > 0.15]
brow_isl = [i for i in head_isl if len(i) == 86 and abs(center(me0, i).y) > 0.1]
eye_isl = [i for i in head_isl if len(i) in (128, 42) and abs(center(me0, i).y) > 0.1]
skull = max(head_isl, key=len)
neck = [i for i in islands if len(i) == 400][0]
assert len(brow_isl) == 2 and len(eye_isl) == 6 and len(skull) == 314, (len(brow_isl), len(eye_isl), len(skull))
skull_set = set(skull)
polys = [p for p in me0.polygons if p.vertices[0] in skull_set]

def skull_bvh(me):
    return BVHTree.FromPolygons([v.co.copy() for v in me.vertices], [tuple(p.vertices) for p in polys])

def surface_height(me, bvh, ch, co):
    """머리 중심에서 co 방향으로 쏜 광선이 머리 표면에 닿는 거리와 co 까지 거리의 차 = 표면 위 높이(m)."""
    d = co - ch; L = d.length; dn = d / L
    hit = bvh.ray_cast(ch, dn, 2.0)
    assert hit[0] is not None
    return L - (hit[0] - ch).length, dn, (hit[0] - ch).length

def make_pain(name, prm):
    ob = ref100.copy(); ob.data = ref100.data.copy(); ob.name = name; ob.data.name = name + '_Mesh'
    s.collection.objects.link(ob)
    V = ob.data.vertices
    ch = center(ob.data, skull); bvh = skull_bvh(ob.data)
    for isl in brow_isl:
        c = sum((V[i].co for i in isl), Vector()) / len(isl)
        rot = Matrix.Rotation(prm['angle'] - BROW_ANGLE_OLD, 3, 'Y')
        for i in isl: V[i].co = c + rot @ (V[i].co - c) + Vector((0, 0, prm['lift']))
        # 앞뒤 위치: 눈썹 중심 x 를 같은 쪽 눈 흰자 중심 x + dx 에 맞춘다
        sgn = 1.0 if c.y > 0 else -1.0
        ex = center(ob.data, next(e for e in eye_isl if len(e) == 128 and center(me0, e).y * sgn > 0 and abs(center(me0, e).y) < 0.16)).x
        c = sum((V[i].co for i in isl), Vector()) / len(isl)
        for i in isl: V[i].co.x += ex + prm['dx'] - c.x
        c = sum((V[i].co for i in isl), Vector()) / len(isl)
        # 머리 표면에 붙이기(모양 보존): 눈썹 중심의 표면 법선 n 하나로, 뒤·가운데·앞 세 구역의 "표면 위 최저 높이"를 지나는
        # 2차 곡선만큼 n 방향으로 끌어내린다(매끈하게 휘어 머리 곡면을 따른다). 그다음 n 방향 두께를 keep 배로 줄인다.
        n = surface_height(ob.data, bvh, ch, c)[1]
        info = {i: surface_height(ob.data, bvh, ch, V[i].co)[0] for i in isl}
        t = {i: V[i].co.x - c.x for i in isl}
        t0, t1 = min(t.values()), max(t.values()); w = t1 - t0
        zones = [[i for i in isl if t[i] < t0 + 0.25 * w], [i for i in isl if abs(t[i]) < 0.15 * w], [i for i in isl if t[i] > t1 - 0.25 * w]]
        ts = [sum(t[i] for i in z) / len(z) for z in zones]; hs = [min(info[i] for i in z) + prm['embed'] for z in zones]
        q = np.polyfit(ts, hs, 2)
        amin = min((V[i].co - c).dot(n) for i in isl)
        for i in isl:
            a = (V[i].co - c).dot(n) - amin
            V[i].co = V[i].co - n * float(np.polyval(q, t[i])) - n * (1.0 - prm['keep']) * a
    if prm['squint'] != 1.0:
        for side in (1, -1):
            idx = [i for isl in eye_isl if center(me0, isl).y * side > 0 for i in isl]
            zb = min(V[i].co.z for i in idx)
            for i in idx: V[i].co.z = zb + prm['squint'] * (V[i].co.z - zb)
    ob.data.update(); ob.hide_render = True; ob.hide_viewport = True
    return ob

# ---- 실루엣 점검: 기본 카메라 시점에서 먼 쪽 눈썹(+y)이 머리(눈썹 제외 머리 섬 + 목) 투영 볼록 껍질 밖으로 나가는 양
cam = bpy.data.objects['CL9_GOOSE_A_PREVIEW_Camera']
R0 = cam.rotation_euler.to_matrix()
def hull(pts):
    pts = sorted(set(map(tuple, pts)))
    def cross(o, a, b): return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])
    lo, up = [], []
    for p in pts:
        while len(lo) >= 2 and cross(lo[-2], lo[-1], p) <= 0: lo.pop()
        lo.append(p)
    for p in reversed(pts):
        while len(up) >= 2 and cross(up[-2], up[-1], p) <= 0: up.pop()
        up.append(p)
    return lo[:-1] + up[:-1]  # 반시계
def silhouette(ob):
    V = ob.data.vertices; Rt = R0.transposed()
    proj = lambda i: tuple((Rt @ V[i].co)[:2])
    brow_set = {i for b in brow_isl for i in b}
    H = hull([proj(i) for isl in head_isl for i in isl if i not in brow_set] + [proj(i) for i in neck])
    far = [i for b in brow_isl if center(me0, b).y > 0 for i in b]
    worst = -1.0; out = 0
    for i in far:
        q = proj(i); dmax = -9.0
        for k in range(len(H)):
            a, b = H[k], H[(k + 1) % len(H)]
            ex, ey = b[0] - a[0], b[1] - a[1]; L = math.hypot(ex, ey)
            dmax = max(dmax, ((q[0] - a[0]) * ey - (q[1] - a[1]) * ex) / L)  # 반시계 껍질: + = 밖
        worst = max(worst, dmax); out += dmax > 0
    # 옆 방향: 눈썹이 머리 표면 위로 솟은 최대 높이
    ch = center(ob.data, skull); bvh = skull_bvh(ob.data)
    hmax = max(surface_height(ob.data, bvh, ch, V[i].co)[0] for b in brow_isl for i in b)
    # 가까운 쪽(-y) 눈썹: 앞(부리 쪽)·뒤 끝 높이 차(+ = 앞쪽이 높음 = 걱정)와 눈 위 끝과의 간격
    nb = [i for b in brow_isl if center(me0, b).y < 0 for i in b]
    xs = [V[i].co.x for i in nb]; x0, x1 = min(xs), max(xs); w = x1 - x0
    if w < 1e-4: return dict(brow='none (folded into head)')  # 눈썹 0% 는 한 점으로 접혀 있다
    zf = max(V[i].co.z for i in nb if V[i].co.x > x1 - 0.2 * w); zb = max(V[i].co.z for i in nb if V[i].co.x < x0 + 0.2 * w)
    eye_top = max(V[i].co.z for b in eye_isl if center(me0, b).y < 0 for i in b)
    return dict(far_brow_verts_outside=out, far_brow_outside_cm=round(100 * max(worst, 0.0), 2), brow_height_above_skull_cm=round(100 * hmax, 2),
                front_minus_back_cm=round(100 * (zf - zb), 1), brow_bottom_minus_eye_top_cm=round(100 * (min(V[i].co.z for i in nb) - eye_top), 1))

# ---- 렌더 무대: 파일에 저장된 PREVIEW 카메라·조명 그대로 + 머리 근접 카메라 2대(같은 조명)
s.render.engine = 'CYCLES'; s.cycles.samples = 32
s.render.resolution_x = 1200; s.render.resolution_y = 900; s.render.resolution_percentage = 100
s.render.image_settings.file_format = 'PNG'
head_c = center(me0, skull) + Vector((0.06, 0, 0.02))  # 머리 + 부리 뿌리가 함께 들어오게
def closeup(name, rot):
    cd = bpy.data.cameras.new(name); cd.type = 'ORTHO'; cd.ortho_scale = 0.62
    ob = bpy.data.objects.new(name, cd); s.collection.objects.link(ob)
    ob.rotation_euler = rot; ob.location = head_c + rot.to_matrix() @ Vector((0, 0, 5.0))
    return ob
cam_q = closeup('TMP_Close34', cam.rotation_euler.copy())
cam_s = closeup('TMP_CloseSide', Euler((math.pi / 2, 0, 0)))  # -y 쪽(가까운 눈썹)에서 정옆

label_mat = bpy.data.materials.new('TMP_Label'); label_mat.use_nodes = True
bs = next(n for n in label_mat.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
bs.inputs['Base Color'].default_value = (1, 1, 1, 1); bs.inputs['Emission Color'].default_value = (1, 1, 1, 1); bs.inputs['Emission Strength'].default_value = 3.0
def label(text, c):
    cu = bpy.data.curves.new('TMP_Label', 'FONT'); cu.body = text; half = c.data.ortho_scale / 2; cu.size = 0.085 * half / 1.35
    ob = bpy.data.objects.new('TMP_Label', cu); ob.data.materials.append(label_mat); s.collection.objects.link(ob)
    ob.visible_shadow = False
    R = c.rotation_euler.to_matrix()
    ob.location = c.location + R @ Vector((-half * 0.95, half * 0.75 * 0.92 - 0.06 * half / 1.35, -2.5))
    ob.rotation_euler = c.rotation_euler
    return ob

meshes = [o for o in s.objects if o.type == 'MESH' and o.name != 'CL9_GOOSE_A_PREVIEW_Floor']
hidden0 = {o.name: (o.hide_render, o.hide_viewport) for o in meshes}
def render(ob, text, c, path):
    for m in meshes: m.hide_render = m is not ob
    s.camera = c
    lab = label(text, c)
    s.render.filepath = str(path); bpy.ops.render.render(write_still=True, scene=s.name)
    bpy.data.objects.remove(lab); print('rendered', path)
    return path

def stitch(rows, out):
    """rows = [[위 행 경로...], [아래 행 경로...]]"""
    def load(p):
        im = bpy.data.images.load(str(p)); w, h = im.size
        a = np.array(im.pixels[:], dtype=np.float32).reshape(h, w, 4); bpy.data.images.remove(im); return a
    big = np.concatenate([np.concatenate([load(p) for p in r], axis=1) for r in reversed(rows)], axis=0)  # 픽셀은 아래 행부터
    h, w = big.shape[:2]
    o = bpy.data.images.new('stitch', width=w, height=h, alpha=True)
    o.pixels.foreach_set(big.ravel()); o.filepath_raw = str(out); o.file_format = 'PNG'; o.save()
    bpy.data.images.remove(o); print('stitched', out)

T = Path(bpy.app.tempdir) / 'cl9_pain_goose'; T.mkdir(parents=True, exist_ok=True)  # 낱장은 임시 폴더, 합친 것만 preview/
cands = {k: make_pain('VAR_Pain_' + k, v) for k, v in PAIN.items()}
meshes += list(cands.values())
for name, ob in [('idle brow000', ref000), ('chase brow100', ref100), ('SK_Goose_A brow040', s.objects['SK_Goose_A'])] + [(k, o) for k, o in cands.items()]:
    print('silhouette', name, silhouette(ob))

if '--measure' in argv:
    print('measure only - no render, no save')
elif PICK is None:
    top = [render(ref100, 'ref: CHASE brow 100%', cam_q, T / 'c_ref_q.png')]
    bot = [render(ref100, 'ref: CHASE brow 100% (side)', cam_s, T / 'c_ref_s.png')]
    for k, ob in cands.items():
        p = PAIN[k]
        top.append(render(ob, '%s %s: angle %+.2f' % (k, DESC[k], p['angle']), cam_q, T / ('c_%s_q.png' % k)))
        bot.append(render(ob, '%s (side)' % k, cam_s, T / ('c_%s_s.png' % k)))
    stitch([top, bot], P / 'compare_pain_candidates.png')
else:
    pain = cands[PICK]
    states = [(ref000, 'IDLE / patrol: brow 0%'), (ref100, 'CHASE: brow 100%'), (pain, 'HITREACT: pain %s' % PICK)]
    top = [render(o, t, cam, T / ('s%d_full.png' % n)) for n, (o, t) in enumerate(states)]
    bot = [render(o, t + ' (close)', cam_q, T / ('s%d_close.png' % n)) for n, (o, t) in enumerate(states)]
    stitch([top, bot], P / 'compare_three_states.png')
    # 저장: 후보는 VAR_Pain_*, 고른 것은 VAR_BrowPain 으로 이름을 바꿔 둔다. 기존 오브젝트 표시 상태는 원래대로.
    s.camera = cam
    for o in (cam_q, cam_s): bpy.data.objects.remove(o)
    bpy.data.materials.remove(label_mat)
    for m in meshes:
        if m.name in hidden0: m.hide_render, m.hide_viewport = hidden0[m.name]
    for o in cands.values(): o.hide_render = True; o.hide_viewport = True
    pain.name = 'VAR_BrowPain'; pain.data.name = 'VAR_BrowPain_Mesh'
    s['cl9_brow_presets'] = json.dumps({
        'note': '비교용 임시값 - 확정은 사용자 승인 뒤. 각도는 Y축 rad, +는 앞쪽(부리 쪽)이 내려감',
        'idle': dict(object='VAR_Brow000', brow_dial=0.0),
        'chase': dict(object='VAR_Brow100', brow_dial=1.0, angle=BROW_ANGLE_OLD),
        'hitreact_pain': dict(object='VAR_BrowPain', pick=PICK, **PAIN[PICK]),
        'pain_candidates': PAIN,
    }, ensure_ascii=False)
    bpy.data.libraries.write(str(OUT / 'goose_a_v2.blend'), {s}, fake_user=True)
    print('saved', OUT / 'goose_a_v2.blend')
