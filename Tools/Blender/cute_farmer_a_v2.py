"""farmer_a 귀여움 1단계(얼굴·눈·채도 + 렌더) — 실물 farmer_a.blend 는 열어서 읽기만 하고, 결과는 farmer_a_v2/ 에 사본으로 쓴다.

    py -3 Tools/Blender/run_headless.py cute_farmer_a_v2.py --blend ArtSource/character/farmer_a/farmer_a.blend

바꾸는 것: 얼굴 로프트 모서리(초타원 지수 .85 → 1.0, 턱 끝 링 약간 넓게), 코(작고 둥글게), 콧수염(모서리만 둥글게),
눈(검은 상자 → 흰자·동공·하이라이트 타원체), 눈썹(상자 → 짧은 타원체), 볼 홍조, 채도(작업복 기본 틴트·팔레트).
바꾸지 않는 것: 신장·몸통·팔다리·머리 크기, 모자·장갑·장화 형태, 리그·클립, 재질 슬롯 3개. 렌더는 전부 레스트 포즈.
눈 2단: Basis = 평소, 셰이프 키 Eye_Hurt_A(찡그림: 눈 세로 0.5배) / Eye_Hurt_B(작은 동공 0.55배) = 피격 후보 둘.
수치는 전부 렌더 옵션이다 — 확정은 사용자 몫. 실물 파일에는 저장하지 않는다(save_as_mainfile copy=True 로 다른 경로에만 쓴다).
"""
import bpy, bmesh, sys, math, json, struct, zlib
import numpy as np
from pathlib import Path
from mathutils import Vector
from mathutils.bvhtree import BVHTree
sys.stdout.reconfigure(encoding='utf-8')
sys.path.insert(0, 'D:/Work/unreal/CL9/Tools/Blender')
from creature_preview import setup_creature_preview

SRC = Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a/farmer_a.blend')
OUT = Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a_v2')
PREV = OUT / 'preview'
PREV.mkdir(parents=True, exist_ok=True); (OUT / 'textures').mkdir(exist_ok=True)
assert Path(bpy.data.filepath) == SRC, bpy.data.filepath

# ---- 수치(렌더 옵션)
FACE_RINGS = [(1.58, -.023, .052, .044), (1.62, -.024, .075, .060), (1.69, -.005, .087, .079),
              (1.75, 0, .085, .081), (1.79, .007, .069, .065), (1.805, .012, .040, .04)]  # build_farmer_a.py 원본 로프트
FACE_POWER_OLD, FACE_POWER_NEW = .85, 1.0   # 초타원 지수: .85 는 모서리가 각진 사각형 쪽, 1.0 은 타원
CHIN_RING_NEW = (.058, .048)                 # 턱 끝 링 반지름 (.052,.044) → 뾰족한 턱을 둥글게
NOSE = dict(spherify=.6, scale=(.9, .8, .85), pivot_y=-.080)   # 코: 상자 → 둥근 단추, 얼굴 쪽 뒷면 기준 축소
MOUSTACHE_SPHERIFY = .45                     # 콧수염: 크기 유지, 모서리만 둥글게
EYE_ANCHOR = (.036, 1.716)                   # (|x|, z) — 원본 눈 (.032, 1.723) 보다 조금 벌리고 낮춤
BROW_ANCHOR = (.036, 1.746)
BLUSH_ANCHOR = (.057, 1.688)
# 부품: (a, b, c) = 얼굴 표면 국부 좌표(a = 캐릭터 왼쪽(+X), b = 위, c = 얼굴 밖). 반지름 단위 m.
SCLERA = dict(center=(0, 0, .001), half=(.0155, .0185, .006), seg=(10, 6))
PUPIL = dict(center=(0, -.0008, .0048), half=(.0118, .0148, .0042), seg=(10, 6))
SHINE = dict(center=(.0042, .0058, .0092), half=(.0036, .0036, .0014), seg=(6, 4))  # 두 눈 모두 +X 쪽 위(같은 광원)
BROW = dict(center=(0, 0, .0022), half=(.0135, .0036, .003), seg=(8, 4))
BLUSH = dict(center=(0, 0, .0006), half=(.012, .0075, .0024), seg=(8, 4))
HURT_BROW = dict(tilt=.38, lift=.003, inset=.0015)  # 안쪽 끝이 올라가는 걱정 쐐기(rad), 위로, 안쪽으로
HURT = {'Eye_Hurt_A': dict(eye_squash=.5, eye_tilt=.12, pupil_k=1.0, pupil_squash=.5, shine=True),    # 찡그린 눈(흰자·동공 세로 0.5배)
        'Eye_Hurt_B': dict(eye_squash=.8, eye_tilt=.10, pupil_k=.55, pupil_squash=1.0, shine=False)}  # 작아진 동공(0.55배), 하이라이트 숨김
# 색(선형 RGB)
C_SCLERA, C_PUPIL, C_SHINE, C_BLUSH = (.86, .84, .78), (.018, .015, .013), (1.0, 1.0, 1.0), (.62, .22, .20)
C_BROW = None  # None = 원본 눈썹색에 채도 규칙 적용
TINT_OLD, TINT_NEW = (.13, .20, .08), (.07, .25, .04)   # 작업복 기본 틴트(Cloth 재질 곱) — 더 선명한 풀색
GREENS = {(.12, .14, .065), (.13, .15, .072), (.13, .15, .085)}   # 소매 끝·모자 띠·주머니 입구: 작업복 녹색과 같이 올린다
K_GREEN, K_OTHER, K_SKIN = 1.5, 1.2, 1.15   # 휘도 유지 채도 배율 — 녹색 강하게, 장화·장갑·띠 등은 살짝
SKIN_OLD = (.48, .30, .19)

def luma(c): return .2126 * c[0] + .7152 * c[1] + .0722 * c[2]
def saturate(c, k):
    L = luma(c); return tuple(max(.0005, L + k * (x - L)) for x in c)
def key3(c): return tuple(round(float(x), 3) for x in c[:3])

# ---- 실물 준비: 레스트 포즈
s_main = bpy.data.scenes['CL9_FARMER_A']; s_prev = bpy.data.scenes['CL9_FARMER_A_PREVIEW']
rig = bpy.data.objects['RIG_Farmer_A']
if rig.animation_data: rig.animation_data.action = None
for p in rig.pose.bones: p.matrix_basis.identity()
real = bpy.data.objects['SK_Farmer_A_Body']; arms = bpy.data.objects['SK_Farmer_A_Arms']
assert len(real.data.vertices) == 2148
def tris(ob): ob.data.calc_loop_triangles(); return len(ob.data.loop_triangles)
def dims_cm(ob):
    co = np.array([v.co for v in ob.data.vertices]); return [round(float(x) * 100, 2) for x in co.max(0) - co.min(0)]
measure = {'real': dict(triangles=tris(real), vertices=len(real.data.vertices), dimensions_cm=dims_cm(real))}

def islands(me):
    adj = [[] for _ in me.vertices]
    for e in me.edges: a, b = e.vertices; adj[a].append(b); adj[b].append(a)
    rem = set(range(len(adj))); out = []
    while rem:
        t = [rem.pop()]; g = []
        while t:
            v = t.pop(); g.append(v)
            for n in adj[v]:
                if n in rem: rem.remove(n); t.append(n)
        out.append(sorted(g))
    return out
def centroid(me, g): return sum((me.vertices[i].co for i in g), Vector()) / len(g)
def find(isls, me, c, n, tol=.003):
    hit = [g for g in isls if len(g) == n and (centroid(me, g) - Vector(c)).length < tol]
    assert len(hit) == 1, (c, n, len(hit)); return hit[0]

# ---- 1) v2 몸 메시 = 실물 사본
v2 = real.copy(); v2.data = real.data.copy(); v2.name = 'V2_Body'; v2.data.name = 'V2_Body'
for sc in (s_main, s_prev): sc.collection.objects.link(v2)
me = v2.data; V = me.vertices
isl = islands(me)
face = find(isl, me, (0, -.0055, 1.7058), 96)
nose = find(isl, me, (0, -.096, 1.699), 24)
must = find(isl, me, (0, -.086, 1.666), 24)
old_eyes = [find(isl, me, (sx * .032, -.084, 1.723), 24) for sx in (-1, 1)]
old_brows = [find(isl, me, (sx * .032, -.083, 1.746), 24) for sx in (-1, 1)]

# 얼굴 로프트: 원본 공식으로 되짚어 확인한 뒤 지수만 바꿔 다시 놓는다(위상 그대로)
def loft_pt(ring, i, power, rxy=None):
    z, y, rx, ry = FACE_RINGS[ring]
    if rxy: rx, ry = rxy
    a = math.tau * i / 16; c = math.cos(a); t = math.sin(a)
    return Vector((rx * math.copysign(abs(c) ** power, c), y + ry * math.copysign(abs(t) ** power, t), z))
for j, vi in enumerate(face):
    assert (V[vi].co - loft_pt(j // 16, j % 16, FACE_POWER_OLD)).length < 1e-4, (j, tuple(V[vi].co))
for j, vi in enumerate(face):
    V[vi].co = loft_pt(j // 16, j % 16, FACE_POWER_NEW, CHIN_RING_NEW if j // 16 == 0 else None)

def spherify(g, t):
    cs = [V[i].co.copy() for i in g]; lo = Vector([min(c[a] for c in cs) for a in range(3)]); hi = Vector([max(c[a] for c in cs) for a in range(3)])
    c0 = (lo + hi) / 2; h = (hi - lo) / 2
    for i, co in zip(g, cs):
        d = Vector(((co - c0)[a] / h[a] for a in range(3))); n = d.normalized()
        d = d.lerp(n, t); V[i].co = c0 + Vector((d[a] * h[a] for a in range(3)))
    return c0
nc = spherify(nose, NOSE['spherify'])
piv = Vector((0, NOSE['pivot_y'], nc.z))
for i in nose:
    d = V[i].co - piv; V[i].co = piv + Vector((d[a] * NOSE['scale'][a] for a in range(3)))
spherify(must, MOUSTACHE_SPHERIFY)
me.update()

# ---- 2) 채도 — 몸·팔 정점색(재질 곱 이전 값), 작업복 틴트, 피부
def recolor(mesh):
    ca = mesh.color_attributes['Color']; changed = {}
    for d in ca.data:
        c = tuple(d.color); k0 = key3(c)
        if max(k0) - min(k0) < 1e-6: continue  # 무채색(흰·회색)은 그대로 — 틴트가 색을 준다
        k = K_GREEN if k0 in GREENS else K_OTHER
        n = saturate(c[:3], k); d.color = (*n, c[3]); changed[k0] = key3(n)
    return changed
color_map = recolor(me); color_map.update(recolor(arms.data))
brow_color = C_BROW or saturate((.075, .042, .023), K_OTHER)
skin_new = saturate(SKIN_OLD, K_SKIN)

# ---- 3) 눈·눈썹·홍조 부품 — 옛 눈·눈썹 섬을 지우고 타원체로 새로 만든다(머리 뼈 가중치 1, Detail 재질)
face_bvh = BVHTree.FromPolygons([V[i].co.copy() for i in face],
                                [[face.index(v) for v in p.vertices] for p in me.polygons if p.vertices[0] in set(face)])
def surf(x, z):
    loc, nrm, _, _ = face_bvh.ray_cast(Vector((x, -.5, z)), Vector((0, 1, 0))); assert loc is not None, (x, z); return loc
def frame(x, z, d=.008):
    p = surf(x, z); tx = surf(x + d, z) - surf(x - d, z); tz = surf(x, z + d) - surf(x, z - d)
    f = tz.cross(tx).normalized()
    if f.y > 0: f = -f
    u = (Vector((0, 0, 1)) - f * f.z).normalized(); r = u.cross(f).normalized()
    return p, r, u, f

def rot2(a, b, phi): c, s = math.cos(phi), math.sin(phi); return a * c - b * s, a * s + b * c
def place(spec, unit, fr, center=None, half=None, phi=0.0, cphi=0.0):
    """unit = 단위 구 정점. 부품 중심 둘레로 phi 회전, 중심 자체는 앵커 둘레로 cphi 회전."""
    P, r, u, f = fr; ca, cb, cc = center or spec['center']; ha, hb, hc = half or spec['half']
    a, b = rot2(unit.x * ha, unit.y * hb, phi); ca, cb = rot2(ca, cb, cphi)
    return P + r * (ca + a) + u * (cb + b) + f * (cc + unit.z * hc)

def hurt_eye(spec, name, h, sign):
    """피격 상태의 (center, half, phi, cphi). 기울기 부호: 안쪽 끝이 올라가는 쪽(sign=+1 이면 -)."""
    ca, cb, cc = spec['center']; ha, hb, hc = spec['half']; phi = -sign * h['eye_tilt']
    if name == 'sclera': return (ca, cb, cc), (ha, hb * h['eye_squash'], hc), phi, phi
    if name == 'pupil':
        k = h['pupil_k']; return (ca, cb * h['eye_squash'], cc), (ha * k, hb * k * h['pupil_squash'], hc), phi, phi
    if h['shine']: return (ca, cb * h['eye_squash'], cc), (ha, hb * .6, hc), phi, phi
    pc = PUPIL['center']; return (pc[0], pc[1], pc[2] - .001), (ha * .2, hb * .2, hc * .2), phi, phi  # 동공 속에 숨긴다

bm = bmesh.new(); bm.from_mesh(me)
bm.verts.ensure_lookup_table()
bmesh.ops.delete(bm, geom=[bm.verts[i] for g in old_eyes + old_brows for i in g], context='VERTS')
col_l = bm.loops.layers.float_color['Color']; uv0 = bm.loops.layers.uv[me.uv_layers[0].name]
dl = bm.verts.layers.deform.verify(); head_i = v2.vertex_groups['head'].index
DETAIL = [m.name for m in me.materials].index('M_CL9_Farmer_Detail')
pieces = []  # (bm 정점 목록, 단위 좌표 목록, {키: 좌표 목록})
for sign in (1, -1):
    fe = frame(sign * EYE_ANCHOR[0], EYE_ANCHOR[1]); fb = frame(sign * BROW_ANCHOR[0], BROW_ANCHOR[1]); fc = frame(sign * BLUSH_ANCHOR[0], BLUSH_ANCHOR[1])
    for name, spec, fr, color in [('sclera', SCLERA, fe, C_SCLERA), ('pupil', PUPIL, fe, C_PUPIL), ('shine', SHINE, fe, C_SHINE),
                                  ('brow', BROW, fb, brow_color), ('blush', BLUSH, fc, C_BLUSH)]:
        res = bmesh.ops.create_uvsphere(bm, u_segments=spec['seg'][0], v_segments=spec['seg'][1], radius=1.0, calc_uvs=True)
        vs = res['verts']; units = [v.co.copy() for v in vs]
        fs = {f for v in vs for f in v.link_faces}
        for v, un in zip(vs, units):
            v.co = place(spec, un, fr); v[dl].clear(); v[dl][head_i] = 1.0
        for f in fs:
            f.material_index = DETAIL; f.smooth = False
            for l in f.loops: l[col_l] = (*color, 1.0)
        keys = {}
        for kname, h in HURT.items():
            if name in ('sclera', 'pupil', 'shine'):
                c, hf, phi, cphi = hurt_eye(spec, name, h, sign); keys[kname] = [place(spec, un, fr, c, hf, phi, cphi) for un in units]
            elif name == 'brow':
                ca, cb, cc = spec['center']; c = (ca - sign * HURT_BROW['inset'], cb + HURT_BROW['lift'], cc)
                keys[kname] = [place(spec, un, fr, c, None, -sign * HURT_BROW['tilt']) for un in units]
            else:
                keys[kname] = [v.co.copy() for v in vs]
        pieces.append((vs, keys))
bm.verts.index_update()
piece_idx = [([v.index for v in vs], keys) for vs, keys in pieces]
bm.to_mesh(me); bm.free(); me.update()

# 셰이프 키: Basis = 평소, 피격 후보 2개(부품 정점만 움직인다)
v2.shape_key_add(name='Basis', from_mix=False)
for kname in HURT:
    kb = v2.shape_key_add(name=kname, from_mix=False)
    for idx, keys in piece_idx:
        for i, co in zip(idx, keys[kname]): kb.data[i].co = co
    kb.value = 0.0

# ---- 4) 팔레트 v2 — 몸·팔의 정점색을 새 팔레트 PNG(bake_farmer_palette.py 와 같은 규격 256×8·32칸)로 굽고 UV1 을 다시 잇는다
def pal_key(c): return tuple(round(max(0, min(1, float(x))), 6) for x in c[:3])
colors = sorted({pal_key(d.color) for o in (v2, arms) for d in o.data.color_attributes['Color'].data}); assert len(colors) <= 32, len(colors)
index = {c: i for i, c in enumerate(colors)}; W, H = 256, 8
def srgb(v): return round(255 * (12.92 * v if v <= .0031308 else 1.055 * v ** (1 / 2.4) - .055))
px = bytes(ch for x in range(W) for ch in [*[srgb(v) for v in colors[min(x // 8, len(colors) - 1)]], 255])
def chunk(kind, data): return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff)
png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', W, H, 8, 6, 0, 0, 0)) + chunk(b'IDAT', zlib.compress((b'\0' + px) * H)) + chunk(b'IEND', b'')
pal_path = OUT / 'textures/T_Farmer_Palette_v2.png'; pal_path.write_bytes(png)
for o in (v2, arms):
    uv = o.data.uv_layers['PaletteUV']
    for i, d in enumerate(o.data.color_attributes['Color'].data): uv.data[i].uv = ((index[pal_key(d.color)] + .5) / 32, .5)

v2.data.calc_loop_triangles()
measure['v2'] = dict(triangles=tris(v2), vertices=len(v2.data.vertices), dimensions_cm=dims_cm(v2),
                     materials=[m.name for m in v2.data.materials], shape_keys=[k.name for k in v2.data.shape_keys.key_blocks])
measure['arms'] = dict(triangles=tris(arms), vertices=len(arms.data.vertices), note='형태 그대로, 정점색 채도만')
measure['options'] = dict(face_power=[FACE_POWER_OLD, FACE_POWER_NEW], chin_ring_m=[[.052, .044], list(CHIN_RING_NEW)], nose=NOSE,
                          moustache_spherify=MOUSTACHE_SPHERIFY, eye_anchor=EYE_ANCHOR, brow_anchor=BROW_ANCHOR, blush_anchor=BLUSH_ANCHOR,
                          sclera=SCLERA, pupil=PUPIL, shine=SHINE, brow=BROW, blush=BLUSH, hurt_brow=HURT_BROW, hurt=HURT,
                          colors_linear=dict(sclera=C_SCLERA, pupil=C_PUPIL, shine=C_SHINE, blush=C_BLUSH, brow=key3(brow_color)),
                          cloth_tint=[TINT_OLD, TINT_NEW], skin=[SKIN_OLD, key3(skin_new)], k=dict(green=K_GREEN, other=K_OTHER, skin=K_SKIN),
                          vertex_color_map={str(k): v for k, v in sorted(color_map.items())}, palette_swatches=len(colors))
print('MEASURE', json.dumps({k: measure[k] for k in ('real', 'v2', 'arms')}, ensure_ascii=False))

# ---- 5) 렌더 — 전신은 preview/neutral.png 와 같은 무대(setup_creature_preview 같은 인자), 얼굴 근접은 원근 카메라 + 얼굴 보조광
s = setup_creature_preview(s_prev.name, OUT, focus=(0, 0, .94), camera_position=(2.1, -3.1, 1.8), ortho=3.05)
s.cycles.samples = 48
full_cam = s.camera
for o in s.objects:
    if o.name == 'FARMER_Flashlight': o.data.energy = 0
label_mat = bpy.data.materials.new('V2_Label'); label_mat.use_nodes = True
bs = next(n for n in label_mat.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
bs.inputs['Base Color'].default_value = (1, 1, 1, 1); bs.inputs['Emission Color'].default_value = (1, 1, 1, 1); bs.inputs['Emission Strength'].default_value = 3.0
def look(ob, loc, target): ob.location = loc; ob.rotation_euler = (Vector(target) - Vector(loc)).to_track_quat('-Z', 'Y').to_euler()
fdata = bpy.data.cameras.new('V2_FaceCam'); fdata.lens = 85; face_cam = bpy.data.objects.new('V2_FaceCam', fdata); s.collection.objects.link(face_cam)
look(face_cam, (.17, -.62, 1.72), (0, -.07, 1.712))
ldata = bpy.data.lights.new('V2_FaceFill', 'AREA'); ldata.energy = 12; ldata.size = .6; ldata.color = (1, .97, .92)
face_fill = bpy.data.objects.new('V2_FaceFill', ldata); s.collection.objects.link(face_fill); look(face_fill, (.45, -1.1, 1.85), (0, -.05, 1.71))
def label(text, cam):
    cu = bpy.data.curves.new('V2_Label', 'FONT'); cu.body = text
    ob = bpy.data.objects.new('V2_Label', cu); ob.data.materials.append(label_mat); s.collection.objects.link(ob); ob.visible_shadow = False
    ob.parent = cam
    if cam.data.type == 'ORTHO':
        half = cam.data.ortho_scale / 2; cu.size = .075; ob.location = (-half * .95, half * .75 * .9, -2.0)
    else:
        d = .3; half = d * 18 / cam.data.lens; cu.size = half * .075; ob.location = (-half * .95, half * .85, -d)
    return ob
everyone = [o for o in s.objects if o.type == 'MESH' and o.name != 'CL9_FARMER_A_PREVIEW_PREVIEW_Floor']
def shot(target, cam, text, fname, res, fill=False, key=None):
    for o in everyone: o.hide_render = o is not target
    face_fill.hide_render = not fill
    if target.data.shape_keys:
        for kb in target.data.shape_keys.key_blocks[1:]: kb.value = 1.0 if kb.name == key else 0.0
    s.camera = cam; s.render.resolution_x, s.render.resolution_y = res
    lab = label(text, cam); s.render.filepath = str(PREV / fname); bpy.ops.render.render(write_still=True, scene=s.name)
    bpy.data.objects.remove(lab); print('rendered', fname); return PREV / fname
def stitch(paths, out):
    imgs = [bpy.data.images.load(str(p)) for p in paths]; w, h = imgs[0].size
    big = np.concatenate([np.array(im.pixels[:], dtype=np.float32).reshape(h, w, 4) for im in imgs], axis=1)
    o = bpy.data.images.new('stitch_' + out.stem, width=w * len(imgs), height=h, alpha=True)
    o.pixels.foreach_set(big.ravel()); o.filepath_raw = str(out); o.file_format = 'PNG'; o.save()
    for im in imgs: bpy.data.images.remove(im)
    bpy.data.images.remove(o); print('stitched', out)

FULL, CLOSE = (1200, 900), (800, 800)
cloth = bpy.data.materials['M_CL9_Farmer_Cloth']; skin = bpy.data.materials['M_CL9_Farmer_Skin']
mix = next(n for n in cloth.node_tree.nodes if n.type == 'MIX_RGB')
skin_bs = next(n for n in skin.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
assert key3(mix.inputs[2].default_value) == key3(TINT_OLD) and key3(skin_bs.inputs['Base Color'].default_value) == key3(SKIN_OLD)
old_img = next(n for n in cloth.node_tree.nodes if n.type == 'TEX_IMAGE').image
# 실물(재질·팔레트 원본 상태)
a_full = shot(real, full_cam, 'ORIGINAL farmer_a (rest)', 'full_original.png', FULL)
a_face = shot(real, face_cam, 'ORIGINAL', 'face_original.png', CLOSE, fill=True)
# v2 재질: 틴트·피부·팔레트 이미지 교체(메모리 안에서만 — 실물 파일에는 쓰지 않는다)
mix.inputs[2].default_value = (*TINT_NEW, 1); skin_bs.inputs['Base Color'].default_value = (*skin_new, 1)
new_img = bpy.data.images.load(str(pal_path)); new_img.name = 'T_Farmer_Palette_v2'; new_img.colorspace_settings.name = 'sRGB'; new_img.pack()
for mname in ('M_CL9_Farmer_Cloth', 'M_CL9_Farmer_Detail'):
    for n in bpy.data.materials[mname].node_tree.nodes:
        if n.type == 'TEX_IMAGE': n.image = new_img
b_full = shot(v2, full_cam, 'v2: face/eyes/saturation (rest)', 'full_v2.png', FULL)
stitch([a_full, b_full], PREV / 'compare_full.png')
b_face = shot(v2, face_cam, 'v2 NORMAL', 'face_v2_normal.png', CLOSE, fill=True)
c_face = shot(v2, face_cam, 'v2 HURT A (squint)', 'face_v2_hurtA.png', CLOSE, fill=True, key='Eye_Hurt_A')
d_face = shot(v2, face_cam, 'v2 HURT B (small pupil)', 'face_v2_hurtB.png', CLOSE, fill=True, key='Eye_Hurt_B')
stitch([a_face, b_face, c_face, d_face], PREV / 'compare_eyes.png')
(OUT / 'measure.json').write_text(json.dumps(measure, indent=2, ensure_ascii=False), encoding='utf-8')

# ---- 6) v2 사본 저장 — 실물 메시는 REF_ 로 이름을 바꿔 숨기고, v2 를 SK_Farmer_A_Body 이름으로
for kb in v2.data.shape_keys.key_blocks[1:]: kb.value = 0.0
for o in list(s.objects):
    if o.name.startswith('V2_Label'): bpy.data.objects.remove(o)
real.name = 'REF_Original_SK_Farmer_A_Body'; real.data.name = 'REF_Original_SK_Farmer_A_Body'
real.hide_render = True; real.hide_viewport = True
v2.name = 'SK_Farmer_A_Body'; v2.data.name = 'SK_Farmer_A_Body'; v2.hide_render = False; v2.hide_viewport = False
arms.hide_render = True
bpy.data.images.remove(old_img)
s.camera = full_cam
dst = OUT / 'farmer_a_v2.blend'
bpy.ops.wm.save_as_mainfile(filepath=str(dst), copy=True)
assert Path(bpy.data.filepath) == SRC
print('saved', dst)
