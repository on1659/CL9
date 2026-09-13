"""농부 A 손 자세(Hold) 클립 — 행의 HoldPose 범주(Small/Flashlight/Large)마다 상체 루프 하나.

원본 리그 RIG_Farmer_A(m 단위)에 두 뼈 IK로 팔을 세우고 손·손가락을 돌려 자세를 만든다.
CL9_HOLD_SPEC 환경변수의 JSON(스크래치 스펙)으로 목표를 바꿔 가며 렌더로 검수하고,
`--export`가 있으면 액션을 저장하고 cm 스테이징 리그로 애니메이션 전용 FBX를 내보낸다.
`--only <범주> [<범주>...]`(`--` 뒤)면 그 범주(Small/Flashlight/Large/Idle)만 렌더·수출한다 — 다른 범주의 FBX·manifest 항목은
손대지 않는다(FBX 에 저장 시각이 박혀 값이 같아도 다시 수출하면 sha 가 바뀐다).
전신 ABP가 척추 위만 레이어로 얹으므로 다리·골반은 건드리지 않는다.

실행: py -3 Tools/Blender/run_headless.py animate_farmer_hold.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export --only Flashlight]
"""
import bpy, json, math, os, sys, hashlib
from pathlib import Path
from mathutils import Vector, Matrix, Quaternion, Euler
sys.path.insert(0, str(Path(__file__).parent))
from farmer_pose import render_fp

ROOT = Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a')
SPEC = json.loads(os.environ['CL9_HOLD_SPEC']) if os.environ.get('CL9_HOLD_SPEC') else json.loads((Path(__file__).with_name('farmer_hold_spec.json')).read_text(encoding='utf-8'))
EXPORT = '--export' in sys.argv
FULL_SPEC = SPEC
if '--only' in sys.argv:
    ONLY = [a for a in sys.argv[sys.argv.index('--only') + 1:] if not a.startswith('--')]
    assert ONLY and all(t in SPEC for t in ONLY), ('--only 뒤 범주 이름이 스펙에 없다', ONLY, list(SPEC))
    SPEC = {t: SPEC[t] for t in ONLY}
PREVIEW_DIR = Path(os.environ.get('CL9_HOLD_PREVIEW', str(ROOT / 'preview')))
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)

scene = bpy.data.scenes['CL9_FARMER_A']
bpy.context.window.scene = scene
rig = bpy.data.objects['RIG_Farmer_A']
rest = {b.name: b.matrix_local.copy() for b in rig.data.bones}
# 기준 방향: 뼈 머리에서 첫 자식 머리로 — UE 임포트 리그는 tail이 임의라 tail을 쓰면 안 된다
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
    """부모가 지금 포즈대로 있고 이 뼈만 기준 포즈일 때의 월드 행렬"""
    pb = rig.pose.bones[name]
    if pb.parent is None:
        return rest[name].copy()
    return pb.parent.matrix @ rest[pb.parent.name].inverted() @ rest[name]

def aim(name, target_dir, roll=0.0):
    """기준 방향(첫 자식 쪽)이 target_dir을 보도록 돌리고, 그 축 둘레로 roll만큼 더 돌린다"""
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
    """손가락을 국부 Z축 둘레로 amount(rad) 굽힌다(실측: Z가 굽힘 축) — 셋 마디 모두. thumb은 따로"""
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
    # 척추 — 살짝 앞으로 기울여 물건을 보는 느낌 + 숨쉬기
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

def render_previews(tag):
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
    preview.render.engine = 'BLENDER_WORKBENCH'
    preview.render.resolution_x, preview.render.resolution_y = 960, 720
    preview.display.shading.light = 'STUDIO'
    preview.display.shading.color_type = 'MATERIAL'
    cam = preview.camera
    cam.data.type = 'PERSP'
    # 3/4 정면
    cam.data.lens = 50
    cam.location = (1.6, -2.4, 1.5)
    cam.rotation_euler = (Vector((0, 0, 1.15)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    preview.render.filepath = str(PREVIEW_DIR / f'hold_{tag}_front.png')
    bpy.ops.render.render(write_still=True)
    # 1인칭 — farmer_pose.render_fp(팔 메시만, 눈높이 1.56m·10도 내려다봄·FOV 70). 예전 이 자리의 자체 카메라는 몸 메시의 턱·가슴에
    # 가려 빈 화면이 나왔다(028 에서 원인 확인)
    bpy.context.window.scene = scene
    render_fp(f'hold_{tag}', PREVIEW_DIR)
    bpy.context.window.scene = preview
    cam.data.lens_unit = 'MILLIMETERS'
    # 오른손 클로즈업 — 손바닥 방향·손가락 굽힘 확인용, 정면 왼쪽 위에서 본다
    cam.data.lens = 60
    cam.data.clip_start = 0.05
    hand = rig.pose.bones['hand_r'].head
    cam.location = (hand.x + 0.35, hand.y - 0.45, hand.z + 0.30)
    cam.rotation_euler = (Vector(hand) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    preview.render.filepath = str(PREVIEW_DIR / f'hold_{tag}_hand.png')
    bpy.ops.render.render(write_still=True)
    bpy.context.window.scene = scene

def make_action(name, spec, seconds=2.0):
    old = bpy.data.actions.get(name)
    if old is not None:
        bpy.data.actions.remove(old)
    rig.animation_data_create()
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    rig.animation_data.action = action
    end = round(seconds * 30) + 1
    scene.frame_start, scene.frame_end = 1, end
    keyed = set()
    for frame in range(1, end + 1):
        scene.frame_set(frame)
        u = (frame - 1) / (end - 1)
        apply_pose(spec, breath=0.012 * math.sin(math.tau * u))
        for pb in rig.pose.bones:
            if pb.matrix_basis != Matrix.Identity(4) or pb.name in keyed:
                keyed.add(pb.name)
                pb.rotation_mode = 'QUATERNION'
                pb.rotation_quaternion = pb.matrix_basis.to_quaternion()
                pb.keyframe_insert('rotation_quaternion', frame=frame)
    for fc in action_fcurves(action):
        for kp in fc.keyframe_points:
            kp.interpolation = 'LINEAR'
    return action, sorted(keyed)

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
    stage = bpy.data.scenes.get('CL9_FARMER_A_EXPORT') or bpy.data.scenes.new('CL9_FARMER_A_EXPORT')
    for o in list(stage.objects):
        bpy.data.objects.remove(o, do_unlink=True)
    stage.unit_settings.system = 'METRIC'
    stage.unit_settings.scale_length = 0.01
    stage.frame_start, stage.frame_end = scene.frame_start, scene.frame_end
    # FBX 는 활성(스테이징) 장면의 fps 로 굽는다 — 저작 장면(30)과 맞춘다(farmer_pose.export_action 과 같은 수정, 028)
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

results = {}
for tag, both in SPEC.items():
    # tp: 남에게 보이는 전신용(자연스러운 높이) / fp: 1인칭 팔용(카메라 안에 들어오게 눈높이까지 올림)
    for view in ('tp', 'fp'):
        spec = both[view]
        # 앞 클립의 액션이 붙어 있으면 렌더·frame_set 이 그 액션으로 리그를 다시 평가해 정적 자세를 덮는다 — 떼고 나서 자세를 준다
        if rig.animation_data:
            rig.animation_data.action = None
        apply_pose(spec)
        render_previews(tag + '_' + view)
        key = ('AN_Farmer_Hold' if view == 'tp' else 'AN_Farmer_FPHold') + tag
        results[key] = {'wrist_r': [round(v, 3) for v in rig.pose.bones['hand_r'].head]}
        if EXPORT:
            action, keyed = make_action(key, spec)
            out = export_action(action)
            results[key].update({'fbx': str(out), 'sha256': hashlib.sha256(out.read_bytes()).hexdigest(), 'keyed_bones': len(keyed)})
reset()
rig.animation_data_clear() if rig.animation_data else None
if EXPORT:
    manifest_path = ROOT / 'manifest.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    manifest.setdefault('animations', {}).update({k: {'fbx_sha256': r['sha256'], 'loop': True, 'seconds': 2.0, 'fps': scene.render.fps, 'layer': 'upper body from spine_01', 'view': 'first-person arms' if 'FPHold' in k else 'full body'} for k, r in results.items()})
    manifest['hold_spec'] = FULL_SPEC
    manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
    bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
print('HOLD RESULT ' + json.dumps(results))
