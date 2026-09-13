"""농부 A 손 자세(Hold) 클립 — 행의 HoldPose 범주(Small/Flashlight/Large)마다 전신 상체 루프(tp)와 1인칭 팔 루프(fp) 한 쌍.

원본 리그 RIG_Farmer_A(m 단위)에 두 뼈 IK로 팔을 세우고 손·손가락을 돌려 자세를 만든다(farmer_pose.apply_pose —
farmer_hold_spec.json: wrist·pole·fingers·palm·curl·thumb). CL9_HOLD_SPEC 환경변수의 JSON(스크래치 스펙)으로 목표를 바꿔 가며 렌더로
검수하고, `--export`가 있으면 액션을 저장하고 cm 스테이징 리그로 애니메이션 전용 FBX를 내보낸다.
- `--only <범주> [<범주>...]`(`--` 뒤)면 그 범주만 렌더·수출한다 — 다른 범주의 FBX·manifest 항목은 손대지 않는다(FBX 에 저장 시각이
  박혀 값이 같아도 다시 수출하면 sha 가 바뀐다).
- `--fp-only` 면 1인칭 팔 클립(AN_Farmer_FPHold*)만 렌더·수출한다 — 전신 FBX 는 sha 까지 그대로다(031 fp-viewmodel: 전신은 무변경).
- tp 는 전신 ABP 가 척추 위만 레이어로 얹으므로 다리·골반은 건드리지 않고 spine_03 으로 숨쉰다. fp 는 카메라 자식 뷰모델이라
  클립이 전 뼈를 소유한다(척추 identity) — 숨쉬기는 손목 z ±FP_BREATH_CM 사인(farmer_pose.breathe). 화면 위치는 farmer_pose.fp_screen_uv
  로 실측해 `HOLD RESULT` 에 찍는다(손목·너클·손가락 끝 uv, 팔꿈치 각, 손목 꺾임).

실행: py -3 Tools/Blender/run_headless.py animate_farmer_hold.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export --fp-only --only Small]
"""
import bpy, json, math, os, sys, hashlib
from pathlib import Path
from mathutils import Vector
sys.path.insert(0, str(Path(__file__).parent))
from farmer_pose import (ROOT, scene, rig, apply_pose, breathe, key_current_pose, new_action, action_fcurves, export_action,
                         render_front, render_fp, fp_pose_report, FP_EYE_HEIGHT_M)

SPEC = json.loads(os.environ['CL9_HOLD_SPEC']) if os.environ.get('CL9_HOLD_SPEC') else json.loads((Path(__file__).with_name('farmer_hold_spec.json')).read_text(encoding='utf-8'))
EXPORT = '--export' in sys.argv
FP_ONLY = '--fp-only' in sys.argv
FULL_SPEC = SPEC
if '--only' in sys.argv:
    ONLY = [a for a in sys.argv[sys.argv.index('--only') + 1:] if not a.startswith('--')]
    assert ONLY and all(t in SPEC for t in ONLY), ('--only 뒤 범주 이름이 스펙에 없다', ONLY, list(SPEC))
    SPEC = {t: SPEC[t] for t in ONLY}
VIEWS = ('fp',) if FP_ONLY else ('tp', 'fp')
PREVIEW_DIR = Path(os.environ.get('CL9_HOLD_PREVIEW', str(ROOT / 'preview')))
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
SECONDS = 2.0
TP_BREATH_RAD = 0.012   # 전신: spine_03 앞뒤 기울기 진폭
FP_BREATH_CM = 0.5      # 1인칭: 손목 z 진폭


def render_previews(tag):
    """3/4 정면(몸) + 1인칭(팔 메시만, 게임 카메라) + 오른손 클로즈업"""
    render_front(tag, PREVIEW_DIR)
    render_fp(tag, PREVIEW_DIR)
    preview = bpy.data.scenes['CL9_FARMER_A_PREVIEW']
    bpy.context.window.scene = preview
    preview.frame_set(scene.frame_current)
    cam = preview.camera
    cam.data.lens_unit = 'MILLIMETERS'
    # 오른손 클로즈업 — 손바닥 방향·손가락 굽힘 확인용, 정면 왼쪽 위에서 본다
    cam.data.lens = 60
    cam.data.clip_start = 0.05
    hand = rig.pose.bones['hand_r'].head
    cam.location = (hand.x + 0.35, hand.y - 0.45, hand.z + 0.30)
    cam.rotation_euler = (Vector(hand) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    preview.render.filepath = str(PREVIEW_DIR / f'{tag}_hand.png')
    bpy.ops.render.render(write_still=True)
    cam.data.clip_start = 0.1
    bpy.context.window.scene = scene


def make_action(name, spec, view):
    action = new_action(name)
    end = round(SECONDS * scene.render.fps) + 1      # 마지막 프레임 = 첫 프레임(루프)
    scene.frame_start, scene.frame_end = 1, end
    keyed = set()
    for frame in range(1, end + 1):
        scene.frame_set(frame)
        u = (frame - 1) / (end - 1)
        if view == 'tp':
            apply_pose(spec, breath=TP_BREATH_RAD * math.sin(math.tau * u))
        else:
            apply_pose(breathe(spec, u, FP_BREATH_CM))
        key_current_pose(keyed, frame)
    for fc in action_fcurves(action):
        for kp in fc.keyframe_points:
            kp.interpolation = 'LINEAR'
    return action, sorted(keyed)


results = {}
for tag, both in SPEC.items():
    # tp: 남에게 보이는 전신용(자연스러운 높이) / fp: 1인칭 팔용(카메라 로컬, 피치 0 프레이밍)
    for view in VIEWS:
        spec = both[view]
        # 앞 클립의 액션이 붙어 있으면 렌더·frame_set 이 그 액션으로 리그를 다시 평가해 정적 자세를 덮는다 — 떼고 나서 자세를 준다
        if rig.animation_data:
            rig.animation_data.action = None
        apply_pose(spec)
        render_previews(f'hold_{tag}_{view}')
        key = ('AN_Farmer_Hold' if view == 'tp' else 'AN_Farmer_FPHold') + tag
        results[key] = {'wrist_r': [round(v, 3) for v in rig.pose.bones['hand_r'].head]}
        if view == 'fp':
            results[key]['fp_r'] = fp_pose_report('r')
            if spec.get('arm_l'):
                results[key]['fp_l'] = fp_pose_report('l')
        if EXPORT:
            action, keyed = make_action(key, spec, view)
            out = export_action(action)
            results[key].update({'fbx': str(out), 'sha256': hashlib.sha256(out.read_bytes()).hexdigest(), 'keyed_bones': len(keyed)})
apply_pose({'spine': {}})
rig.animation_data_clear() if rig.animation_data else None
if EXPORT:
    manifest_path = ROOT / 'manifest.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    anims = manifest.setdefault('animations', {})
    for k, r in results.items():
        entry = {'fbx_sha256': r['sha256'], 'loop': True, 'seconds': SECONDS, 'fps': scene.render.fps}
        if 'FPHold' in k:
            entry.update({'space': 'camera', 'eye_height_m': FP_EYE_HEIGHT_M, 'layer': 'full rig (camera-local view model, spine identity)',
                          'view': 'first-person arms', 'breath_cm': FP_BREATH_CM})
        else:
            entry.update({'layer': 'upper body from spine_01', 'view': 'full body', 'breath_rad': TP_BREATH_RAD})
        anims[k] = entry
    manifest['hold_spec'] = FULL_SPEC
    manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
    bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
print('HOLD RESULT ' + json.dumps(results))
