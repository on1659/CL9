"""농부 A 던지기 클립 — Shift+G 차징(ThrowChargeStartTime)과 놓기(던짐)의 표현. 2026-09-15, 사용자 "애님이 부족하다".

전신 2클립(상체 레이어 — Hold 체인과 같은 spine_01 레이어에 얹는다):
- AN_Farmer_ThrowWindup 1.0초 루프 — 오른손을 오른쪽 어깨 뒤로 당겨 든 자세(손목 ±1cm 숨쉬기), 왼손은 앞으로 조준하듯. 차징 동안 돈다.
- AN_Farmer_Throw 0.333초(11프레임) 원샷 — 당긴 손이 앞·위로 뻗어 나가며(0~0.15, 판정은 놓는 순간이라 t=0 에 이미 물건이 손을 떠났다) 팔로스루 뒤
  기준 자세(A 포즈)로 내려온다(0.15~0.333). 던지면 손이 비어 HoldPose=None 이므로 끝 자세가 identity 여야 레이어 가중치가 빠질 때 튀지 않는다.
1인칭 2클립(카메라 로컬·피치 0, farmer_pose 머리말): AN_Farmer_FPThrowWindup 1.0초 루프(주먹이 오른쪽 아래 구석에서 뒤로 당겨져 반쯤 보임),
AN_Farmer_FPThrow 0.333초(주먹이 화면 중앙 위로 뻗으며 손이 펴지고, 아래로 빠져 FPArmsDown(프레임 밖)으로).

실행: py -3 Tools/Blender/run_headless.py animate_farmer_throw.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export]
"""
import sys, json, hashlib, math
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from mathutils import Vector
from farmer_pose import (ROOT, scene, rig, reset, apply_pose, world_base, key_current_pose, new_action, action_fcurves, export_action,
                         render_front, render_fp, fp_pose_report, FP_EYE_HEIGHT_M)
from animate_farmer_swing import ease, lerp_keys, clamp_reach

EXPORT = '--export' in sys.argv
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
HOLD = json.loads((Path(__file__).with_name('farmer_hold_spec.json')).read_text(encoding='utf-8'))['Small']
SMALL_TP, SMALL_FP = dict(HOLD['tp']['arm_r']), dict(HOLD['fp']['arm_r'])

# ---------- 전신 (리그 공간 m, 오른쪽 −X·앞 −Y·위 +Z, 어깨 (−0.19, 0.03, 1.44)) ----------
COCK_TP = {'wrist': [-0.34, 0.16, 1.58], 'pole': [-1.0, 0.4, -0.2], 'fingers': [-0.2, 0.6, 0.77], 'curl': 0.6, 'thumb': 0.3}     # 어깨 뒤로 당김
RELEASE_TP = {'wrist': [-0.20, -0.48, 1.55], 'pole': [-0.9, -0.2, 0.3], 'fingers': [-0.1, -0.85, 0.5], 'curl': 0.1, 'thumb': 0.1}  # 앞·위로 뻗음, 손 펴짐
FOLLOW_TP = {'wrist': [-0.05, -0.45, 1.20], 'pole': [-0.6, -0.3, -0.7], 'fingers': [0.2, -0.8, -0.55], 'curl': 0.1, 'thumb': 0.1}
REST_TP = None   # identity(A 포즈) — apply_pose 에 arm_r 을 안 준다
AIM_L = {'wrist': [0.22, -0.40, 1.30], 'pole': [0.9, -0.1, -0.4], 'fingers': [0.1, -0.95, 0.2], 'curl': 0.3, 'thumb': 0.2}       # 왼손 앞으로 조준
TORSO_COCK = {'spine_01': [0.0, 0.0, 0.0], 'spine_02': [0.0, 0.0, 0.0], 'spine_03': [0.0, 0.02, 0.05]}
KEYS_TP = [(0.0, COCK_TP), (0.15, RELEASE_TP), (0.25, FOLLOW_TP), (10 / 30, None)]
TORSO_TP = [(0.0, {'yaw': 0.30, 'pitch': -0.08}), (0.15, {'yaw': -0.25, 'pitch': 0.25}), (0.25, {'yaw': -0.25, 'pitch': 0.25}), (10 / 30, {'yaw': 0.0, 'pitch': 0.0})]
EASE_TP = ['in', 'out3', 'inout']

# ---------- 1인칭 (카메라 로컬, 눈 (0,0,1.56)) ----------
COCK_FP = {**SMALL_FP, 'wrist': [-0.22, -0.28, 1.47], 'pole': [-1.0, 0.3, -0.4], 'fingers': [0.6, 0.2, 0.75], 'palm': [0.6, -0.5, 0.6]}   # 오른쪽 가장자리에 주먹만(손목은 프레임 밖)
RELEASE_FP = {**SMALL_FP, 'wrist': [-0.06, -0.50, 1.56], 'pole': [-0.8, -0.3, 0.1], 'fingers': [0.05, -0.9, 0.44], 'palm': [0.3, -0.2, 0.93], 'curl': 0.05, 'thumb': 0.1}
FOLLOW_FP = {**SMALL_FP, 'wrist': [0.0, -0.44, 1.36], 'pole': [-0.6, -0.4, -0.6], 'fingers': [0.1, -0.85, -0.5], 'palm': [0.4, 0.0, -0.9], 'curl': 0.1, 'thumb': 0.1}
DOWN_FP = {**SMALL_FP, 'wrist': [-0.26, -0.20, 1.05], 'pole': [-1.0, 0.0, -0.4], 'fingers': [0.0, -0.4, -0.9], 'palm': [0.8, 0.3, -0.2], 'curl': 0.3, 'thumb': 0.2}  # 프레임 밖 아래
KEYS_FP = [(0.0, COCK_FP), (0.15, RELEASE_FP), (0.25, FOLLOW_FP), (10 / 30, DOWN_FP)]
EASE_FP = ['in', 'out3', 'inout']
WINDUP_SECONDS, THROW_SECONDS, BREATH_CM = 1.0, 10 / 30, 1.0   # 던지기 = 11프레임 0.333초(30fps 정수 프레임)


def torso_spec(p):
    from animate_farmer_swing import YAW_RIGHT
    spine = {b: [YAW_RIGHT * p['yaw'] / 3, 0.0, -p['pitch'] / 3] for b in ('spine_01', 'spine_02', 'spine_03')}
    spine['neck_01'] = [-YAW_RIGHT * p['yaw'] * 0.3, 0.0, p['pitch'] * 0.3]
    spine['head'] = [-YAW_RIGHT * p['yaw'] * 0.4, 0.0, p['pitch'] * 0.4]
    return spine


def tp_pose(t):
    torso = lerp_keys(TORSO_TP, t, EASE_TP)
    spec = {'spine': torso_spec(torso)}
    # 오른팔: 마지막 구간(팔로스루 → identity)은 손목 목표를 A 포즈 손목으로 보간
    for i, ((t0, a), (t1, b)) in enumerate(zip(KEYS_TP, KEYS_TP[1:])):
        if t0 <= t <= t1:
            u = 0.0 if t1 == t0 else (t - t0) / (t1 - t0)
            w = ease(EASE_TP[i], u)
            if b is None:
                reset(); rest_wrist = list(rig.pose.bones['hand_r'].head)
                b = {**a, 'wrist': rest_wrist, 'pole': [-0.3, 0.6, -0.8], 'fingers': [-0.3, -0.5, -0.8]}
            spec['arm_r'] = {k: ([x + (y - x) * w for x, y in zip(a[k], b[k])] if isinstance(a[k], list) else a[k] + (b[k] - a[k]) * w) for k in a}
            break
    # 왼팔: 조준 자세에서 identity 로 (0.15~0.35)
    wl = 0.0 if t < 0.15 else ease('inout', min(1.0, (t - 0.15) / 0.2))
    if wl < 1.0:
        reset(); rest_l = list(rig.pose.bones['hand_l'].head)
        spec['arm_l'] = {**AIM_L, 'wrist': [x + (y - x) * wl for x, y in zip(AIM_L['wrist'], rest_l)],
                         'pole': [x + (y - x) * wl for x, y in zip(AIM_L['pole'], [0.3, 0.6, -0.8])]}
    return clamp_reach(spec)


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    jobs = [
        ('AN_Farmer_ThrowWindup', 'tp', WINDUP_SECONDS, True),
        ('AN_Farmer_Throw', 'tp', THROW_SECONDS, False),
        ('AN_Farmer_FPThrowWindup', 'fp', WINDUP_SECONDS, True),
        ('AN_Farmer_FPThrow', 'fp', THROW_SECONDS, False),
    ]
    for name, view, seconds, loop in jobs:
        frames = round(seconds * FPS) + 1
        scene.frame_start, scene.frame_end = 1, frames
        action = new_action(name) if EXPORT else None
        if rig.animation_data and not EXPORT:
            rig.animation_data.action = None
        keyed = set(); previews = []; samples = {}
        for frame in range(1, frames + 1):
            t = (frame - 1) / FPS
            u = t / seconds
            if loop:
                dz = BREATH_CM * 0.01 * math.sin(math.tau * u)
                if view == 'tp':
                    spec = {'spine': torso_spec(TORSO_TP[0][1]), 'arm_r': {**COCK_TP, 'wrist': [COCK_TP['wrist'][0], COCK_TP['wrist'][1], COCK_TP['wrist'][2] + dz]},
                            'arm_l': AIM_L}
                else:
                    spec = {'spine': {}, 'arm_r': {**COCK_FP, 'wrist': [COCK_FP['wrist'][0], COCK_FP['wrist'][1], COCK_FP['wrist'][2] + dz]}}
            else:
                spec = tp_pose(t) if view == 'tp' else {'spine': {}, 'arm_r': lerp_keys(KEYS_FP, t, EASE_FP)}
            scene.frame_set(frame)
            apply_pose(spec)
            if EXPORT:
                key_current_pose(keyed, frame)
            if frame in {1, *range(1, frames + 1, max(1, (frames - 1) // 5)), frames}:
                tag = f'{name.replace("AN_Farmer_", "").lower()}_f{frame:02d}'
                if view == 'tp':
                    render_front(tag, PREVIEW_DIR, cam_location=(-2.2, -1.9, 1.5)); previews.append(f'{tag}_front.png')
                else:
                    render_fp(tag, PREVIEW_DIR); previews.append(f'{tag}_fp.png'); samples[f'f{frame}'] = fp_pose_report('r')['uv']
        results[name] = {'frames': frames, 'seconds': seconds, 'preview': previews}
        if view == 'fp':
            results[name]['uv_r'] = samples
        if EXPORT:
            for fc in action_fcurves(action):
                for kp in fc.keyframe_points:
                    kp.interpolation = 'LINEAR'
            out = export_action(action)
            results[name].update({'fbx': str(out), 'sha256': hashlib.sha256(out.read_bytes()).hexdigest(), 'keyed_bones': len(keyed)})
    reset()
    rig.animation_data_clear() if rig.animation_data else None
    if EXPORT:
        manifest_path = ROOT / 'manifest.json'
        manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
        anims = manifest.setdefault('animations', {})
        for name, view, seconds, loop in jobs:
            r = results[name]
            entry = {'fbx_sha256': r['sha256'], 'loop': loop, 'seconds': seconds, 'fps': FPS, 'frames': r['frames']}
            if view == 'fp':
                entry.update({'space': 'camera', 'eye_height_m': FP_EYE_HEIGHT_M, 'layer': 'full rig (camera-local view model, spine identity)',
                              'view': 'first-person arms'})
            else:
                entry.update({'layer': 'upper body from spine_01 (montage slot)', 'view': 'full body', 'ends_at_identity': not loop})
            anims[name] = entry
        manifest['throw_keys'] = {'tp': KEYS_TP, 'torso': TORSO_TP, 'fp': KEYS_FP, 'ease': EASE_TP}
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    print('THROW RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
