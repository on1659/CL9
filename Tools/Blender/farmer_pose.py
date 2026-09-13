"""농부 A 리그 자세 헬퍼 — animate_farmer_hold.py의 두 뼈 IK·손가락 굽힘·액션/FBX 내보내기를 다른 클립
스크립트(animate_farmer_swing.py)가 같이 쓰도록 떼어 놓은 것. 원본 리그 RIG_Farmer_A(m 단위)를 전제한다.
"""
import bpy, math
from pathlib import Path
from mathutils import Vector, Matrix, Quaternion

ROOT = Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a')

scene = bpy.data.scenes['CL9_FARMER_A']
bpy.context.window.scene = scene
rig = bpy.data.objects['RIG_Farmer_A']
rest = {b.name: b.matrix_local.copy() for b in rig.data.bones}
child_of = {}
for b in rig.data.bones:
    if b.parent is not None and b.parent.name not in child_of:
        child_of[b.parent.name] = b.name
FIRST_CHILD = {'upperarm_r': 'lowerarm_r', 'lowerarm_r': 'hand_r', 'hand_r': 'middle_metacarpal_r',
               'upperarm_l': 'lowerarm_l', 'lowerarm_l': 'hand_l', 'hand_l': 'middle_metacarpal_l'}


def reset():
    for p in rig.pose.bones:
        p.matrix_basis = Matrix.Identity(4)
    bpy.context.view_layer.update()


def world_base(name):
    pb = rig.pose.bones[name]
    if pb.parent is None:
        return rest[name].copy()
    return pb.parent.matrix @ rest[pb.parent.name].inverted() @ rest[name]


def aim(name, target_dir, roll=0.0):
    base = world_base(name)
    ref_child = FIRST_CHILD.get(name, child_of.get(name))
    v_rest = rest[ref_child].translation - rest[name].translation
    v_cur = base.to_3x3() @ (rest[name].to_3x3().inverted() @ v_rest)
    q = v_cur.rotation_difference(Vector(target_dir).normalized())
    q = Quaternion(Vector(target_dir).normalized(), roll) @ q
    rig.pose.bones[name].matrix = Matrix.Translation(base.translation) @ (q.to_matrix() @ base.to_3x3()).to_4x4()
    bpy.context.view_layer.update()


def arm_ik(side, wrist, pole, roll_upper=0.0, roll_lower=0.0):
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


def apply_pose(spec, breath=0.0):
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
        aim('hand_' + side, arm['fingers'], arm.get('hand_roll', 0.0))
        curl(side, arm.get('curl', 0.0), arm.get('thumb'), arm.get('spread', 0.0))


def key_current_pose(action_bones, frame):
    """지금 포즈에서 기준 포즈와 다른 뼈(또는 이미 키가 있는 뼈)에 회전 키를 찍는다"""
    for pb in rig.pose.bones:
        if pb.matrix_basis != Matrix.Identity(4) or pb.name in action_bones:
            action_bones.add(pb.name)
            pb.rotation_mode = 'QUATERNION'
            pb.rotation_quaternion = pb.matrix_basis.to_quaternion()
            pb.keyframe_insert('rotation_quaternion', frame=frame)


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
    if hasattr(action, 'fcurves'):
        return list(action.fcurves)
    curves = []
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                curves.extend(bag.fcurves)
    return curves


def export_action(action):
    """cm 스테이징 리그로 애니메이션 전용 FBX를 낸다 (animate_farmer_hold.py와 같은 옵션)"""
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
    copy.animation_data.action = action
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
    if occupied is not None:
        occupied.name = 'root'
    bpy.context.window.scene = scene
    return out


def render_front(tag, preview_dir):
    preview = bpy.data.scenes['CL9_FARMER_A_PREVIEW']
    if rig.name not in preview.objects:
        preview.collection.objects.link(rig)
    body = bpy.data.objects['SK_Farmer_A_Body']
    if body.name not in preview.objects:
        preview.collection.objects.link(body)
    for o in preview.objects:
        if o.name == 'PREVIEW_Farmer_A':
            o.hide_render = True
    bpy.context.window.scene = preview
    # 액션이 붙어 있으면 렌더가 미리보기 장면의 프레임으로 리그를 다시 평가한다 — 저작 장면과 같은 프레임으로 맞춘다
    preview.frame_set(scene.frame_current)
    preview.render.engine = 'BLENDER_WORKBENCH'
    preview.render.resolution_x, preview.render.resolution_y = 960, 720
    preview.display.shading.light = 'STUDIO'
    preview.display.shading.color_type = 'MATERIAL'
    cam = preview.camera
    cam.data.type = 'PERSP'
    cam.data.lens_unit = 'MILLIMETERS'
    cam.data.lens = 50
    cam.location = (1.6, -2.4, 1.5)
    cam.rotation_euler = (Vector((0, 0, 1.15)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    preview.render.filepath = str(Path(preview_dir) / f'{tag}_front.png')
    bpy.ops.render.render(write_still=True)
    bpy.context.window.scene = scene


def render_fp(tag, preview_dir):
    """1인칭 미리보기 — 눈 높이(1.56m)에서 정면(-Y)을 10도 내려다보는 FOV 70 카메라. 게임처럼 팔 메시(SK_Farmer_A_Arms)만
    그린다 — 몸 메시를 두면 턱·가슴이 손을 가려 빈 화면이 나온다(025 의 `_fp_fp.png` 가 빈 원인). 팔 메시가 없으면 몸으로 대신"""
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
    body.hide_render = arms is not None
    if arms is not None:
        arms.hide_render = False
    bpy.context.window.scene = preview
    # 액션이 붙어 있으면 렌더가 미리보기 장면의 프레임으로 리그를 다시 평가한다 — 저작 장면과 같은 프레임으로 맞춘다
    preview.frame_set(scene.frame_current)
    preview.render.engine = 'BLENDER_WORKBENCH'
    preview.render.resolution_x, preview.render.resolution_y = 960, 720
    preview.display.shading.light = 'STUDIO'
    preview.display.shading.color_type = 'MATERIAL'
    cam = preview.camera
    cam.data.type = 'PERSP'
    cam.data.lens_unit = 'FOV'
    cam.data.angle = math.radians(70)
    cam.data.clip_start = 0.22
    cam.location = (0.0, 0.0, 1.56)
    cam.rotation_euler = (Vector((0.0, -2.0, 1.56 - 2.0 * math.tan(math.radians(10)))) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    preview.render.filepath = str(Path(preview_dir) / f'{tag}_fp.png')
    bpy.ops.render.render(write_still=True)
    cam.data.lens_unit = 'MILLIMETERS'
    cam.data.clip_start = 0.1
    body.hide_render = False
    if arms is not None:
        arms.hide_render = True  # 정면 미리보기(render_front)에는 팔 메시가 몸과 겹치지 않게
    bpy.context.window.scene = scene
