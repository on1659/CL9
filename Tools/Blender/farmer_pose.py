"""농부 A 리그 자세 헬퍼 — animate_farmer_*.py 가 같이 쓰는 두 뼈 IK·손가락 굽힘·손바닥 정렬·액션/FBX 내보내기·미리보기.
원본 리그 RIG_Farmer_A(m 단위)를 전제한다.

1인칭 카메라 모델(031 fp-viewmodel): 팔 메시 SK_Farmer_A_Arms 는 게임에서 카메라 컴포넌트의 자식(상대 (0,0,−EyeHeight)·yaw −90)이라
클립 공간 = 리그 root 원점, 눈 (0,0,FP_EYE_HEIGHT_M)에서 −Y(UE +X)를 보는 공간이다. 피치·요는 카메라가 통째로 돌리므로 1인칭 클립은
**피치 0 프레이밍**으로만 저작하고, 몸 포즈 복사·척추 레이어가 없으니 클립 하나가 전 뼈를 소유한다(root·pelvis·spine 은 identity).
화면 위치는 `fp_screen_uv` 로 실측하고 `render_fp` 가 같은 모델(1280×720·가로 FOV 70·피치 0)로 그린다. FirstPersonScale 0.6 은 카메라
원점 기준 닮음 변환이라 화면상 위치·크기가 그대로다 — 근접 클립면만 5cm/0.6 로 환산한다.
"""
import bpy, math
from pathlib import Path
from mathutils import Vector, Matrix, Quaternion

ROOT = Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a')

# ---------- 1인칭 카메라 모델 (게임과 동일: CharacterTable EyeHeight 156, FirstPersonFieldOfView 70(가로), 16:9, NearClipPlane 5cm, FirstPersonScale 0.6) ----------
FP_EYE_HEIGHT_M = 1.56
FP_FOV_H_DEG = 70.0
FP_RESOLUTION = (1280, 720)
FP_NEAR_CLIP_M = 0.05 / 0.6
FP_FORWARD, FP_RIGHT, FP_UP = Vector((0.0, -1.0, 0.0)), Vector((-1.0, 0.0, 0.0)), Vector((0.0, 0.0, 1.0))


def fp_camera_basis(pitch_deg=0.0):
    """카메라 (전방, 오른쪽, 위) 단위 벡터 — 피치는 UE 부호(+ 가 위를 봄), 오른쪽 축 둘레 회전"""
    q = Quaternion(FP_RIGHT, math.radians(pitch_deg))
    return q @ FP_FORWARD, FP_RIGHT, q @ FP_UP


def fp_screen_uv(point, pitch_deg=0.0, inside_only=True):
    """리그 공간 점(m) → 화면 (u, v) — 왼쪽 위 (0,0)·오른쪽 아래 (1,1). 카메라 뒤(깊이 ≤ 0)면 None, inside_only 면 프레임 밖도 None"""
    fwd, right, up = fp_camera_basis(pitch_deg)
    rel = Vector(point) - Vector((0.0, 0.0, FP_EYE_HEIGHT_M))
    depth = rel.dot(fwd)
    if depth <= 1e-6:
        return None
    tan_h = math.tan(math.radians(FP_FOV_H_DEG) * 0.5)
    tan_v = tan_h * FP_RESOLUTION[1] / FP_RESOLUTION[0]
    u = 0.5 + 0.5 * (rel.dot(right) / depth) / tan_h
    v = 0.5 - 0.5 * (rel.dot(up) / depth) / tan_v
    if inside_only and not (0.0 <= u <= 1.0 and 0.0 <= v <= 1.0):
        return None
    return (round(u, 3), round(v, 3))


scene = bpy.data.scenes['CL9_FARMER_A']
bpy.context.window.scene = scene
rig = bpy.data.objects['RIG_Farmer_A']
rest = {b.name: b.matrix_local.copy() for b in rig.data.bones}
child_of = {}
for b in rig.data.bones:
    if b.parent is not None and b.parent.name not in child_of:
        child_of[b.parent.name] = b.name
# 기준 방향: 뼈 머리에서 첫 자식 머리로 — UE 임포트 리그는 tail 이 임의라 tail 을 쓰면 안 된다
FIRST_CHILD = {'upperarm_r': 'lowerarm_r', 'lowerarm_r': 'hand_r', 'hand_r': 'middle_metacarpal_r',
               'upperarm_l': 'lowerarm_l', 'lowerarm_l': 'hand_l', 'hand_l': 'middle_metacarpal_l'}


def reset():
    for p in rig.pose.bones:
        p.matrix_basis = Matrix.Identity(4)
    bpy.context.view_layer.update()


def world_base(name):
    """부모가 지금 포즈대로 있고 이 뼈만 기준 포즈일 때의 월드 행렬"""
    pb = rig.pose.bones[name]
    if pb.parent is None:
        return rest[name].copy()
    return pb.parent.matrix @ rest[pb.parent.name].inverted() @ rest[name]


def aim(name, target_dir, roll=0.0):
    """기준 방향(첫 자식 쪽)이 target_dir 을 보도록 돌리고, 그 축 둘레로 roll 만큼 더 돌린다"""
    base = world_base(name)
    ref_child = FIRST_CHILD.get(name, child_of.get(name))
    v_rest = rest[ref_child].translation - rest[name].translation
    v_cur = base.to_3x3() @ (rest[name].to_3x3().inverted() @ v_rest)
    q = v_cur.rotation_difference(Vector(target_dir).normalized())
    q = Quaternion(Vector(target_dir).normalized(), roll) @ q
    rig.pose.bones[name].matrix = Matrix.Translation(base.translation) @ (q.to_matrix() @ base.to_3x3()).to_4x4()
    bpy.context.view_layer.update()


def arm_ik(side, wrist, pole, roll_upper=0.0, roll_lower=0.0):
    """어깨는 지금 자리, 손목을 wrist(월드 m)에, 팔꿈치는 pole 방향으로 — 두 뼈 IK"""
    up, lo = 'upperarm_' + side, 'lowerarm_' + side
    shoulder = world_base(up).translation
    l1 = (rest[lo].translation - rest[up].translation).length
    l2 = (rest['hand_' + side].translation - rest[lo].translation).length
    wrist = Vector(wrist)
    d = (wrist - shoulder).length
    assert d < l1 + l2 - 1e-3, f'unreachable wrist {side}: {d:.3f} > {l1 + l2:.3f}'
    axis = (wrist - shoulder).normalized()
    a = (l1 * l1 - l2 * l2 + d * d) / (2 * d)
    perp = Vector(pole) - axis * Vector(pole).dot(axis)
    perp.normalize()
    elbow = shoulder + axis * a + perp * math.sqrt(max(0.0, l1 * l1 - a * a))
    aim(up, elbow - shoulder, roll_upper)
    aim(lo, wrist - elbow, roll_lower)
    got = rig.pose.bones['hand_' + side].head
    assert (got - wrist).length < 2e-3, f'wrist off by {(got - wrist).length:.4f}'


def curl(side, amount, thumb=None, spread=0.0):
    """손가락을 국부 Z축 둘레로 amount(rad) 굽힌다(실측: Z 가 굽힘 축) — 셋 마디 모두. thumb 은 따로"""
    for finger in ('index', 'middle', 'ring', 'pinky'):
        for seg in ('01', '02', '03'):
            pb = rig.pose.bones[f'{finger}_{seg}_{side}']
            pb.rotation_mode = 'XYZ'
            pb.rotation_euler = (spread if seg == '01' else 0.0, 0.0, amount * (1.0 if seg != '01' else 0.85))
    t = amount * 0.5 if thumb is None else thumb
    for seg in ('01', '02', '03'):
        pb = rig.pose.bones[f'thumb_{seg}_{side}']
        pb.rotation_mode = 'XYZ'
        pb.rotation_euler = (0.0, 0.0, t * (0.6 if seg == '01' else 1.0))
    bpy.context.view_layer.update()


def align_palm(side, arm):
    """손 뼈를 손가락 축 둘레로 돌려 손바닥이 arm['palm'](월드 방향)을 보게 한다. aim 의 roll=0 은 최소 회전이라 손이 어디를 향하느냐에
    따라 손바닥 방향이 제멋대로다. 손바닥 방향은 축 가정 없이 실측한다 — 가운뎃손가락을 굽혔을 때 둘째 마디가 움직이는 쪽(왼손은 뼈 축이
    거울상이라 국부 축 부호를 믿을 수 없다)"""
    name = 'hand_' + side
    f = Vector(arm['fingers']).normalized()
    aim(name, f, 0.0)
    curl(side, 0.0, 0.0)
    straight = rig.pose.bones['middle_02_' + side].head.copy()
    curl(side, 1.0, 0.0)
    cur = rig.pose.bones['middle_02_' + side].head - straight
    want = Vector(arm['palm'])
    cur_p = (cur - f * cur.dot(f)).normalized()
    want_p = (want - f * want.dot(f)).normalized()
    roll = math.atan2(cur_p.cross(want_p).dot(f), cur_p.dot(want_p))
    aim(name, f, roll + arm.get('hand_roll', 0.0))


def apply_pose(spec, breath=0.0):
    """spec = {'spine': {뼈: [x,y,z] rad}, 'arm_r'/'arm_l': {wrist, pole, fingers, [palm], [hand_roll], curl, [thumb], [spread]}}.
    palm 이 있으면 손바닥 방향을 실측 정렬하고(align_palm), 없으면 aim 의 최소 회전 + hand_roll"""
    reset()
    for name, rot in spec.get('spine', {}).items():
        pb = rig.pose.bones[name]
        pb.rotation_mode = 'XYZ'
        pb.rotation_euler = (rot[0] + breath, rot[1], rot[2])
    bpy.context.view_layer.update()
    for side in ('r', 'l'):
        arm = spec.get('arm_' + side)
        if not arm:
            continue
        arm_ik(side, arm['wrist'], arm['pole'], arm.get('roll_upper', 0.0), arm.get('roll_lower', 0.0))
        if arm.get('palm'):
            align_palm(side, arm)
        else:
            aim('hand_' + side, arm['fingers'], arm.get('hand_roll', 0.0))
        curl(side, arm.get('curl', 0.0), arm.get('thumb'), arm.get('spread', 0.0))


def breathe(spec, u, cm=0.5):
    """루프 위상 u∈[0,1] 에서 두 손목 z 를 ±cm 사인으로 흔든 spec 사본 — 카메라 로컬 클립의 미세 숨쉬기(척추를 안 쓰니 손목으로).
    u=0·1 에서 0 이라 루프 이음새가 없다"""
    dz = cm * 0.01 * math.sin(math.tau * u)
    out = {'spine': spec.get('spine', {})}
    for side in ('r', 'l'):
        arm = spec.get('arm_' + side)
        if arm:
            out['arm_' + side] = {**arm, 'wrist': [arm['wrist'][0], arm['wrist'][1], arm['wrist'][2] + dz]}
    return out


def fp_pose_report(side='r', pitch_deg=0.0):
    """지금 자세의 1인칭 실측 — 팔꿈치 안쪽 각, 손목 꺾임(아래팔 대 손등 방향), 손목·너클·손가락 끝의 화면 uv(프레임 밖이면 None)"""
    pb = rig.pose.bones
    sh, el, wr = pb['upperarm_' + side].head, pb['lowerarm_' + side].head, pb['hand_' + side].head
    kn, tip = pb['middle_01_' + side].head, pb['middle_03_' + side].head
    elbow_deg = math.degrees((sh - el).angle(wr - el))
    wrist_deg = math.degrees((wr - el).angle(kn - wr))
    return {'wrist_m': [round(v, 3) for v in wr], 'elbow_deg': round(elbow_deg, 1), 'wrist_flex_deg': round(wrist_deg, 1),
            'depth_m': round(-wr.y, 3), 'below_eye_m': round(FP_EYE_HEIGHT_M - wr.z, 3),
            'uv': {'wrist': fp_screen_uv(wr, pitch_deg), 'knuckle': fp_screen_uv(kn, pitch_deg), 'fingertip': fp_screen_uv(tip, pitch_deg),
                   'elbow': fp_screen_uv(el, pitch_deg)}}


def key_current_pose(action_bones, frame, locations=()):
    """지금 포즈에서 기준 포즈와 다른 뼈(또는 이미 키가 있는 뼈)에 회전 키를 찍는다. locations 에 든 뼈(예: 넘어짐의 pelvis)는 위치 키도 찍는다"""
    for pb in rig.pose.bones:
        if pb.matrix_basis != Matrix.Identity(4) or pb.name in action_bones:
            action_bones.add(pb.name)
            pb.rotation_mode = 'QUATERNION'
            pb.rotation_quaternion = pb.matrix_basis.to_quaternion()
            pb.keyframe_insert('rotation_quaternion', frame=frame)
        if pb.name in locations:
            pb.location = pb.matrix_basis.to_translation()
            pb.keyframe_insert('location', frame=frame)


def new_action(name):
    old = bpy.data.actions.get(name)
    if old is not None:
        bpy.data.actions.remove(old)
    rig.animation_data_create()
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    rig.animation_data.action = action
    return action


def action_fcurves(action):
    """Blender 5.x 슬롯 액션(layers/strips/channelbags)과 옛 fcurves 둘 다 지원"""
    if hasattr(action, 'fcurves'):
        return list(action.fcurves)
    curves = []
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                curves.extend(bag.fcurves)
    return curves


def export_action(action):
    """cm 스테이징 리그로 애니메이션 전용 FBX를 낸다 — 전 뼈를 굽는다(bake_anim_use_all_bones). 키가 없는 뼈는 identity 로 들어가므로
    카메라 로컬 클립의 root·pelvis·spine 도 이 한 파일이 소유한다"""
    stage = bpy.data.scenes.get('CL9_FARMER_A_EXPORT') or bpy.data.scenes.new('CL9_FARMER_A_EXPORT')
    for o in list(stage.objects):
        bpy.data.objects.remove(o, do_unlink=True)
    stage.unit_settings.system = 'METRIC'
    stage.unit_settings.scale_length = 0.01
    stage.frame_start, stage.frame_end = scene.frame_start, scene.frame_end
    # FBX 는 활성(스테이징) 장면의 fps 로 굽는다 — 새 장면 기본값 24 를 그대로 두면 30fps 로 저작한 프레임이 24fps 로
    # 읽혀 길이가 1.25배 늘어난다(AN_Farmer_Swing 이 0.45초 저작 → UE 0.583초로 들어온 원인, 2026-09-13 확인).
    stage.render.fps, stage.render.fps_base = scene.render.fps, scene.render.fps_base
    bpy.context.window.scene = stage
    occupied = bpy.data.objects.get('root')
    if occupied is not None:
        occupied.name = 'REF_ReservedRoot'
    copy = rig.copy()
    copy.data = rig.data.copy()
    copy.name = 'root'
    copy.data.transform(Matrix.Scale(100, 4))
    stage.collection.objects.link(copy)
    copy.animation_data_create()
    # 뼈 위치 키(넘어짐의 pelvis)는 m 단위 국부 이동이라 100배 리그에 그대로 얹으면 100배 작게 나간다(2026-09-15 실측: 78cm 가 0.78cm) —
    # 위치 커브만 100배로 환산한 임시 액션으로 굽는다. 회전 키뿐인 클립은 그대로(sha 불변).
    export_act = action
    if any(fc.data_path.endswith('].location') for fc in action_fcurves(action)):
        export_act = action.copy()
        export_act.name = action.name + '_cm'
        for fc in action_fcurves(export_act):
            if fc.data_path.endswith('].location'):
                for kp in fc.keyframe_points:
                    kp.co[1] *= 100.0
                    kp.handle_left[1] *= 100.0
                    kp.handle_right[1] *= 100.0
    copy.animation_data.action = export_act
    bpy.ops.object.select_all(action='DESELECT')
    copy.select_set(True)
    bpy.context.view_layer.objects.active = copy
    out = ROOT / 'export' / (action.name + '.fbx')
    bpy.ops.export_scene.fbx(filepath=str(out), use_selection=True, object_types={'ARMATURE'}, axis_forward='-Y', axis_up='Z',
                             global_scale=1, apply_unit_scale=True, apply_scale_options='FBX_SCALE_ALL', add_leaf_bones=False,
                             use_armature_deform_only=False, armature_nodetype='ROOT', bake_anim=True, bake_anim_use_all_bones=True,
                             bake_anim_use_nla_strips=False, bake_anim_use_all_actions=False, bake_anim_force_startend_keying=True,
                             bake_anim_step=1.0, bake_anim_simplify_factor=0.0, use_custom_props=False)
    bpy.data.objects.remove(copy, do_unlink=True)
    if export_act is not action:
        bpy.data.actions.remove(export_act)
    if occupied is not None:
        occupied.name = 'root'
    bpy.context.window.scene = scene
    return out


def _preview_scene():
    preview = bpy.data.scenes['CL9_FARMER_A_PREVIEW']
    if rig.name not in preview.objects:
        preview.collection.objects.link(rig)
    body = bpy.data.objects['SK_Farmer_A_Body']
    arms = bpy.data.objects.get('SK_Farmer_A_Arms')
    for o in (body, arms):
        if o is not None and o.name not in preview.objects:
            preview.collection.objects.link(o)
    for o in preview.objects:
        if o.name == 'PREVIEW_Farmer_A':
            o.hide_render = True
    preview.render.engine = 'BLENDER_WORKBENCH'
    preview.display.shading.light = 'STUDIO'
    preview.display.shading.color_type = 'MATERIAL'
    return preview, body, arms


def render_front(tag, preview_dir, cam_location=(1.6, -2.4, 1.5), look_at=(0, 0, 1.15)):
    """3/4 정면 전신 미리보기(960×720, 50mm) — 팔 메시는 몸과 겹치므로 숨긴다. cam_location 으로 다른 각도(예: 오른쪽 앞)도 찍는다"""
    preview, body, arms = _preview_scene()
    body.hide_render = False
    if arms is not None:
        arms.hide_render = True
    bpy.context.window.scene = preview
    # 액션이 붙어 있으면 렌더가 미리보기 장면의 프레임으로 리그를 다시 평가한다 — 저작 장면과 같은 프레임으로 맞춘다
    preview.frame_set(scene.frame_current)
    preview.render.resolution_x, preview.render.resolution_y = 960, 720
    preview.render.resolution_percentage = 100
    cam = preview.camera
    cam.data.type = 'PERSP'
    cam.data.sensor_fit = 'AUTO'
    cam.data.lens_unit = 'MILLIMETERS'
    cam.data.lens = 50
    cam.data.clip_start = 0.1
    cam.location = cam_location
    cam.rotation_euler = (Vector(look_at) - Vector(cam.location)).to_track_quat('-Z', 'Y').to_euler()
    preview.render.filepath = str(Path(preview_dir) / f'{tag}_front.png')
    bpy.ops.render.render(write_still=True)
    bpy.context.window.scene = scene


def render_fp(tag, preview_dir, pitch_deg=0.0):
    """1인칭 미리보기 — 게임과 동일한 카메라(눈 (0,0,1.56)·−Y 전방·가로 FOV 70·1280×720·피치 0 기본·근접 8.3cm). 게임처럼 팔 메시
    (SK_Farmer_A_Arms)만 그린다 — 몸 메시를 두면 턱·가슴이 손을 가려 빈 화면이 나온다(025 의 `_fp_fp.png` 가 빈 원인). 팔 메시가 없으면
    몸으로 대신. pitch_deg 는 검산용(UE 부호, + 위) — 클립 저작·게이트는 0 으로만 본다"""
    preview, body, arms = _preview_scene()
    body.hide_render = arms is not None
    if arms is not None:
        arms.hide_render = False
    bpy.context.window.scene = preview
    preview.frame_set(scene.frame_current)
    preview.render.resolution_x, preview.render.resolution_y = FP_RESOLUTION
    preview.render.resolution_percentage = 100
    cam = preview.camera
    cam.data.type = 'PERSP'
    cam.data.sensor_fit = 'HORIZONTAL'
    cam.data.lens_unit = 'FOV'
    cam.data.angle = math.radians(FP_FOV_H_DEG)
    cam.data.shift_x = cam.data.shift_y = 0.0
    cam.data.clip_start = FP_NEAR_CLIP_M
    cam.location = (0.0, 0.0, FP_EYE_HEIGHT_M)
    fwd, _, _ = fp_camera_basis(pitch_deg)
    cam.rotation_euler = fwd.to_track_quat('-Z', 'Y').to_euler()
    preview.render.filepath = str(Path(preview_dir) / f'{tag}_fp.png')
    bpy.ops.render.render(write_still=True)
    cam.data.sensor_fit = 'AUTO'
    cam.data.lens_unit = 'MILLIMETERS'
    cam.data.clip_start = 0.1
    body.hide_render = False
    if arms is not None:
        arms.hide_render = True  # 정면 미리보기(render_front)에는 팔 메시가 몸과 겹치지 않게
    bpy.context.window.scene = scene
