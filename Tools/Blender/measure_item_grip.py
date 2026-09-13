"""아이템 그립 피벗 역산 — 고기(SM_Meat_*_A)·프라이팬(SM_FryingPan_A)을 농부 A 의 Small 들기 손에 "손잡이가 손 안, 몸통이 앞·조금 위"로
들리게 하는 ItemTable `GripOffset`(cm)·`GripRotation`(deg) 을 계산해 표 행으로 찍는다. 수정은 하지 않는다(값은 리드가 표에 넣는다).

절차
1. 아이템 원본 blend 에서 메시를 읽어(append) 손잡이를 잰다 — 고기는 +X 쪽 뼈(단면 반경이 몸통의 절반 아래인 구간), 팬은 −X 쪽 손잡이.
   손잡이 끝에서 GRIP_INSET 만큼 안쪽이 "손 안에 들어오는 점"이다. 게임은 메시를 `Radius/바운드 구 반경`으로 자동 배율하므로
   (CL9Item::LoadData) 그 배율(SCALE, production-progress 2026-09-11 실측)을 곱한다.
2. 손 소켓 `HandGrip_R` 은 Blender 리그에 없다(UE 스켈레톤 소켓). 대신 기존 행이 준 두 실측 — 두 손 물건(Large) 의
   `GripRotation (1.6, 81.6, 51.8)`·`GripOffset (12.3, −5.4, 17.3)`(= 소켓 공간에서 "X=카메라 정면·Z=월드 위·위치=양손 중점",
   docs/plan/data/item/common.md) — 로 Large 1인칭 자세의 소켓 자세를 되살리고, 소켓이 hand_r 뼈에 고정돼 있으니 손 기하 프레임
   (손가락 방향·손바닥 법선) 을 매개로 Small 자세의 소켓 자세를 얻는다. 손전등 행 `(−17.8, 136.8, 69.3)`(렌즈 +X = 정면) 으로
   이 보정이 맞는지 검산한다(FLASHLIGHT CHECK — 각도 오차가 몇 도 안이면 OK).
3. 원하는 아이템 자세(정면+UP_DEG 위로 뻗고 Z 는 위)를 소켓 공간으로 되돌려 GripRotation, 손잡이 점이 손바닥 점에 오도록 GripOffset.
   대안으로 "너클 축 정합"(손가락이 감기는 축에 손잡이를 맞춘 것 — 현재 Small 자세에선 위·안쪽을 향한다) 값도 같이 찍는다.

좌표: Blender(m, 오른손, 정면 −Y) → UE 액터 공간(cm, X 정면·Y 오른쪽·Z 위) 는 (x, y, z) → (−y, −x, z)·100. UE 회전 행렬·FRotator 변환은
엔진의 FRotationMatrix / FMatrix::Rotator 공식 그대로(행 = 축).

실행: py -3 Tools/Blender/run_headless.py measure_item_grip.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --up 15 --inset 4]
결과: 표준 출력 `GRIP RESULT {...}` + docs/asset/evidence/028-item-grip.json
"""
import bpy, sys, json, math
from pathlib import Path
from mathutils import Vector, Matrix
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
sys.path.insert(0, str(Path(__file__).parent))
from farmer_pose import scene, rig, apply_pose

REPO = Path('D:/Work/unreal/CL9')
HOLD = json.loads((Path(__file__).with_name('farmer_hold_spec.json')).read_text(encoding='utf-8'))
ARGS = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else []


def arg(name, default):
    return float(ARGS[ARGS.index(name) + 1]) if name in ARGS else default


UP_DEG = arg('--up', 15.0)        # 몸통이 정면에서 위로 들리는 각
GRIP_INSET = arg('--inset', 4.0)  # 손잡이 끝에서 손바닥 점까지(cm, 월드) — 주먹 폭의 절반쯤
PALM_DEPTH = 2.5                  # 손등 뼈 선에서 손바닥 쪽으로 들어간 깊이(cm) — 쥔 손잡이 축이 지나는 곳
SCALE = {'meat': 0.89, 'frying_pan': 0.99}   # 자동 배율 = Radius / 바운드 구 반경 (production-progress.md 2026-09-11)
ITEMS = {
    'meat': {'blend': 'ArtSource/item/meat_states/meat_states.blend', 'object': 'SM_Meat_Raw_A',
             'manifest': 'ArtSource/item/meat_states/export/manifest.json', 'key': 'raw', 'handle_side': +1,
             'rows': ['meat(raw)', 'meat.cooked', 'meat.burnt'], 'body_axis': -1},
    'frying_pan': {'blend': 'ArtSource/item/cookware/cookware.blend', 'object': 'SM_FryingPan_A',
                   'manifest': 'ArtSource/item/cookware/export/manifest.json', 'key': 'frying_pan', 'handle_side': -1,
                   'rows': ['frying_pan'], 'body_axis': +1},
}
# 기존 행 실측(common.md) — 소켓 보정의 근거
LARGE_ROT, LARGE_OFF = (1.6, 81.6, 51.8), Vector((12.3, -5.4, 17.3))
FLASH_ROT = (-17.8, 136.8, 69.3)


# ---------- UE 회전 수학 (행 = 축, 행벡터 곱) ----------
def rot_matrix(pitch, yaw, roll):
    sp, cp = math.sin(math.radians(pitch)), math.cos(math.radians(pitch))
    sy, cy = math.sin(math.radians(yaw)), math.cos(math.radians(yaw))
    sr, cr = math.sin(math.radians(roll)), math.cos(math.radians(roll))
    return Matrix(((cp * cy, cp * sy, sp),
                   (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp),
                   (-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp)))


def to_rotator(m):
    x, y, z = m[0], m[1], m[2]
    pitch = math.atan2(x.z, math.sqrt(x.x * x.x + x.y * x.y))
    yaw = math.atan2(x.y, x.x)
    sy_axis = Vector((-math.sin(yaw), math.cos(yaw), 0.0))
    roll = math.atan2(z.dot(sy_axis), y.dot(sy_axis))
    return [round(math.degrees(a), 1) for a in (pitch, yaw, roll)]


def rowmul(v, m):
    return Vector((v.dot(m.col[0]), v.dot(m.col[1]), v.dot(m.col[2])))


for _r in ((1.6, 81.6, 51.8), (-17.8, 136.8, 69.3), (30.0, -120.0, -80.0)):   # 회전 행렬 ↔ FRotator 왕복 자가 검사
    _m = rot_matrix(*_r)
    assert abs(_m.determinant() - 1.0) < 1e-6 and all(abs(a - b) < 0.11 for a, b in zip(to_rotator(_m), _r)), (_r, to_rotator(_m))


def frame(x, z_hint):
    """행 = 축인 회전 행렬. x 를 첫 축으로, z_hint 를 x 에 직교화해 셋째 축으로, y = z × x (UE: x × y = z)"""
    x = x.normalized()
    z = (z_hint - x * z_hint.dot(x)).normalized()
    y = z.cross(x)
    return Matrix((x, y, z))


def to_ue(v):
    return Vector((-v.y, -v.x, v.z)) * 100.0


def to_ue_dir(v):
    return Vector((-v.y, -v.x, v.z)).normalized()


# ---------- 손 기하 프레임 ----------
def hand_frame(spec):
    apply_pose(spec)
    pb = rig.pose.bones
    head, mid = pb['hand_r'].head.copy(), pb['middle_01_r'].head.copy()
    index, pinky = pb['index_01_r'].head.copy(), pb['pinky_01_r'].head.copy()
    fingers = (mid - head).normalized()
    across = (index - pinky).normalized()          # 새끼→검지: 손가락이 감기는 축(너클 선)
    palm = across.cross(fingers).normalized()      # 오른손: 손바닥 법선 (Blender 오른손 좌표)
    center = (head + mid) * 0.5 + palm * (PALM_DEPTH / 100.0)
    return {'M': frame(to_ue_dir(fingers), to_ue_dir(palm)), 'p': to_ue(head), 'palm_point': to_ue(center),
            'across': to_ue_dir(across), 'palm': to_ue_dir(palm), 'fingers': to_ue_dir(fingers),
            'p_l': to_ue(pb['hand_l'].head.copy()) if spec.get('arm_l') else None}


large, small_fp, small_tp, flash = (hand_frame(HOLD['Large']['fp']), hand_frame(HOLD['Small']['fp']),
                                    hand_frame(HOLD['Small']['tp']), hand_frame(HOLD['Flashlight']['fp']))
# Large 1인칭 자세의 소켓: 아이템 프레임 = 액터 프레임 → M_L · M_S = I
M_S_large = rot_matrix(*LARGE_ROT).transposed()
p_mid = (large['p'] + large['p_l']) * 0.5
p_S_large = p_mid - rowmul(LARGE_OFF, M_S_large)
M_SH = M_S_large @ large['M'].transposed()               # 소켓 회전 = M_SH · 손 프레임
o_SH = rowmul(p_S_large - large['p'], large['M'].transposed())


def socket_in(pose):
    return M_SH @ pose['M'], pose['p'] + rowmul(o_SH, pose['M'])


# 검산: 손전등 자세에서 렌즈 +X 가 정면(1,0,0)을 보는가
M_S_flash, _ = socket_in(flash)
lens = rowmul(Vector((1, 0, 0)), rot_matrix(*FLASH_ROT) @ M_S_flash)
flash_err = math.degrees(math.acos(max(-1.0, min(1.0, lens.dot(Vector((1, 0, 0)))))))
print(f'FLASHLIGHT CHECK lens_dir={[round(v, 3) for v in lens]} error_deg={flash_err:.1f}')


# ---------- 아이템 손잡이 실측 ----------
def load_item(info):
    path = REPO / info['blend']
    with bpy.data.libraries.load(str(path), link=False) as (src, dst):
        assert info['object'] in src.objects, (info['object'], src.objects)
        dst.objects = [info['object']]
    obj = dst.objects[0]
    mw = obj.matrix_world
    verts = [mw @ v.co for v in obj.data.vertices]
    lo = Vector((min(v.x for v in verts), min(v.y for v in verts), min(v.z for v in verts)))
    hi = Vector((max(v.x for v in verts), max(v.y for v in verts), max(v.z for v in verts)))
    manifest = json.loads((REPO / info['manifest']).read_text(encoding='utf-8'))['assets'][info['key']]
    dims_cm = Vector(manifest['dimensions_cm'])
    unit = dims_cm.x / (hi.x - lo.x)                       # 원본 단위 → cm
    center = (lo + hi) * 0.5
    local = [(v - center) * unit for v in verts]           # 피벗 = 바운드 중심(manifest), cm
    sphere_r = max(v.length for v in local)
    # 손잡이: handle_side 쪽 끝에서 안쪽으로, 단면 반경이 최대 단면 반경의 절반 아래인 구간
    side = info['handle_side']
    bins = {}
    for v in local:
        bins.setdefault(int(math.floor(v.x)), []).append(v)
    radii = {}
    for bx, vs in bins.items():
        cy = sum(v.y for v in vs) / len(vs)
        cz = sum(v.z for v in vs) / len(vs)
        radii[bx] = (max(math.hypot(v.y - cy, v.z - cz) for v in vs), cy, cz)
    r_max = max(r for r, _, _ in radii.values())
    order = sorted(radii, reverse=(side > 0))
    handle = []
    for bx in order:
        if radii[bx][0] < 0.5 * r_max:
            handle.append(bx)
        else:
            break
    assert handle, 'handle not found'
    tip_x = max(v.x for v in local) if side > 0 else min(v.x for v in local)
    cy = sum(radii[b][1] for b in handle) / len(handle)
    cz = sum(radii[b][2] for b in handle) / len(handle)
    handle_len = len(handle)
    handle_r = max(radii[b][0] for b in handle)
    bpy.data.objects.remove(obj, do_unlink=True)
    return {'dims_cm': [round(v, 2) for v in dims_cm], 'sphere_radius_cm': round(sphere_r, 2), 'handle_tip_x_cm': round(tip_x, 2),
            'handle_len_cm': handle_len, 'handle_radius_cm': round(handle_r, 2), 'handle_axis_yz_cm': [round(cy, 2), round(cz, 2)],
            'body_max_radius_cm': round(r_max, 2)}


def solve(name, info, geo, pose, variant):
    s = SCALE[name]
    M_S, p_S = socket_in(pose)
    up = Vector((0, 0, 1))
    if variant == 'forward':
        d = Vector((math.cos(math.radians(UP_DEG)), 0.0, math.sin(math.radians(UP_DEG))))   # 몸통이 뻗는 방향(액터 공간)
    else:
        d = pose['across']                            # 너클 축: 몸통은 엄지(검지) 쪽으로 나간다
    x_item = d * info['body_axis']                    # 아이템 +X 가 액터 공간에서 향하는 곳
    M_item = frame(x_item, up)                        # 아이템 Z(위) 는 월드 위에 가깝게
    grip_local = Vector((geo['handle_tip_x_cm'] - info['handle_side'] * GRIP_INSET / s, geo['handle_axis_yz_cm'][0], geo['handle_axis_yz_cm'][1]))
    p_item = pose['palm_point'] - rowmul(grip_local * s, M_item)      # 손잡이 점이 손바닥 점에 오는 메시 원점
    M_rel = M_item @ M_S.transposed()
    offset = rowmul(p_item - p_S, M_S.transposed())
    return {'GripOffset_cm': [round(v, 1) for v in offset], 'GripRotation_deg': to_rotator(M_rel),
            'item_x_world': [round(v, 3) for v in x_item], 'grip_point_local_cm': [round(v, 1) for v in grip_local], 'scale': s}


result = {'inputs': {'up_deg': UP_DEG, 'grip_inset_cm': GRIP_INSET, 'palm_depth_cm': PALM_DEPTH, 'scale': SCALE,
                     'calibration': {'large_rotation': LARGE_ROT, 'large_offset': list(LARGE_OFF), 'flashlight_rotation': FLASH_ROT}},
          'socket_calibration': {'socket_rot_in_hand_frame': to_rotator(M_SH), 'socket_offset_in_hand_frame_cm': [round(v, 1) for v in o_SH],
                                 'flashlight_check': {'lens_dir': [round(v, 3) for v in lens], 'error_deg': round(flash_err, 1)}},
          'hand': {}, 'items': {}}
for tag, pose in (('small_fp', small_fp), ('small_tp', small_tp)):
    M_S, p_S = socket_in(pose)
    result['hand'][tag] = {'hand_r_cm': [round(v, 1) for v in pose['p']], 'palm_point_cm': [round(v, 1) for v in pose['palm_point']],
                           'fingers': [round(v, 3) for v in pose['fingers']], 'across(knuckle axis)': [round(v, 3) for v in pose['across']],
                           'palm_normal': [round(v, 3) for v in pose['palm']], 'socket_pos_cm': [round(v, 1) for v in p_S],
                           'socket_axes': {'x': [round(v, 3) for v in M_S[0]], 'y': [round(v, 3) for v in M_S[1]], 'z': [round(v, 3) for v in M_S[2]]}}
for name, info in ITEMS.items():
    geo = load_item(info)
    result['items'][name] = {'geometry': geo, 'rows': info['rows'],
                             'forward_fp': solve(name, info, geo, small_fp, 'forward'), 'forward_tp': solve(name, info, geo, small_tp, 'forward'),
                             'knuckle_fp': solve(name, info, geo, small_fp, 'knuckle')}
apply_pose({'spine': {}, 'arm_r': HOLD['Small']['tp']['arm_r']})
out = REPO / 'docs/asset/evidence/028-item-grip.json'
out.write_text(json.dumps(result, indent=2, ensure_ascii=False), encoding='utf-8')
print('GRIP RESULT ' + json.dumps(result))
for name, r in result['items'].items():
    for row in r['rows']:
        f = r['forward_fp']
        print(f"ROW ItemTable.{row}.GripOffset: (?) -> ({f['GripOffset_cm'][0]}, {f['GripOffset_cm'][1]}, {f['GripOffset_cm'][2]}) (cm)")
        print(f"ROW ItemTable.{row}.GripRotation: (?) -> ({f['GripRotation_deg'][0]}, {f['GripRotation_deg'][1]}, {f['GripRotation_deg'][2]}) (deg P,Y,R)")
