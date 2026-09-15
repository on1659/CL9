"""농부 A 휘두르기 클립 — 좌클 Swing(프라이팬·고기)의 표현. Small 들기 자세에서 오른손을 오른쪽 어깨 위로 치켜들었다가
정면 아래로 내려치고 되돌아오는 **0.467초(30fps 15프레임 = 14 간격)** 원샷, 전신(AN_Farmer_Swing)과 1인칭 팔(AN_Farmer_FPSwing) 한 쌍.
판정은 애님과 무관하게 누르는 즉시다(docs/systems/input.md 사용).

2026-09-15 재작업(사용자 "무기 공격이 어색하다"): 옛 4키 직선 보간을 버리고
- 키 6개(시작 → 치켜듦 0.12 → 타격 직전 0.18 → 타격 0.25 → 팔로스루 0.31 → 복귀) 사이를 **구간별 완급**(치켜듦까지 완만, 타격까지 가속,
  타격 뒤 급감속, 복귀 완만)으로 잇는다 — 호 궤적은 중간 키가 만든다.
- 치켜든 자세는 옆으로 벌린 팔이 아니라 **오른쪽 어깨 위·뒤**(팔꿈치 바깥·위), 타격은 허리 옆이 아니라 **몸 앞으로 뻗은** 자세.
- 전신은 몸통이 같이 돈다 — 치켜들 때 오른쪽으로 비틀며 살짝 젖히고, 내려칠 때 왼쪽으로 비틀며 앞으로 숙인다(척추 3마디 분배, 머리는
  반대로 돌려 정면 유지), 왼팔은 균형 잡느라 뒤로 흔든다. 시작·끝은 전신 HoldSmall 스펙(`farmer_hold_spec.json Small.tp`)을 그대로 읽어
  들기 자세와 이음새가 없다(옛 026 키는 HoldSmall 과 11cm 어긋나 가지 전환 때 튀었다).
- 1인칭도 같은 구조(카메라 로컬·피치 0, farmer_pose 머리말) — 치켜든 주먹은 화면 우상단 모서리 안쪽, 타격은 화면 아래 중앙을 지나 왼쪽
  아래로 빠졌다가 Small 로 돌아온다. 손바닥 방향(`palm`)이 구간마다 바뀌어 팬 면이 돌아간다.
ABP 상수(대기→휘두르기 오프셋 0.12·평가 범위 0.347)는 그대로다 — 공격 대기 클립(animate_farmer_windup.py)이 KEYS·SPINE·WINDUP_TIME 을
import 하므로 그 이름·형식(KEYS[view] = [(t, 오른팔 스펙)…], SPINE[view] = 치켜든 시각의 척추 스펙)은 유지한다. 본문은 __main__ 일 때만 돈다.
`--export`면 액션 저장 + cm 스테이징 FBX + manifest 갱신(전신·1인칭 둘 다). `--fp-only` 면 1인칭만.

실행: py -3 Tools/Blender/run_headless.py animate_farmer_swing.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export [--fp-only]]
"""
import sys, json, hashlib
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from mathutils import Vector
from farmer_pose import (ROOT, scene, rig, reset, apply_pose, world_base, key_current_pose, new_action, action_fcurves, export_action, render_front,
                         render_fp, fp_pose_report, FP_EYE_HEIGHT_M)

EXPORT = '--export' in sys.argv
FP_ONLY = '--fp-only' in sys.argv
VIEWS = ('fp',) if FP_ONLY else ('tp', 'fp')
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
FRAMES = 15
SECONDS = (FRAMES - 1) / FPS          # 0.4667
WINDUP_TIME, MID_TIME, IMPACT_TIME, FOLLOW_TIME = 0.12, 0.18, 0.25, 0.31
HOLD = json.loads((Path(__file__).with_name('farmer_hold_spec.json')).read_text(encoding='utf-8'))['Small']
SMALL_TP = dict(HOLD['tp']['arm_r'])
SMALL_FP = dict(HOLD['fp']['arm_r'])

# ---------- 오른팔 키 (시각, 스펙) — 좌표는 리그 공간 m (오른쪽 −X·앞 −Y·위 +Z), 어깨는 (−0.19, 0.03, 1.44), 팔 길이 0.55 ----------
# 전신: 치켜듦 = 손목이 오른쪽 어깨 위·뒤(팬이 머리 옆에서 뒤로 눕는다), 타격 직전 = 머리 높이 앞, 타격 = 가슴 앞으로 뻗음, 팔로스루 = 몸 앞 아래.
KEYS = {
    'tp': [
        (0.00, SMALL_TP),
        (WINDUP_TIME, {'wrist': [-0.30, 0.10, 1.80], 'pole': [-1.0, 0.2, -0.3], 'fingers': [-0.1, 0.5, 0.86], 'curl': 0.6, 'thumb': 0.3}),
        (MID_TIME, {'wrist': [-0.32, -0.25, 1.72], 'pole': [-1.0, -0.1, 0.1], 'fingers': [-0.1, -0.35, 0.93], 'curl': 0.6, 'thumb': 0.3}),
        (IMPACT_TIME, {'wrist': [-0.15, -0.50, 1.05], 'pole': [-0.8, -0.1, -0.6], 'fingers': [0.1, -0.8, -0.6], 'curl': 0.6, 'thumb': 0.3}),
        (FOLLOW_TIME, {'wrist': [-0.05, -0.44, 0.92], 'pole': [-0.6, -0.1, -0.8], 'fingers': [0.3, -0.7, -0.65], 'curl': 0.6, 'thumb': 0.3}),
        (SECONDS, SMALL_TP),
    ],
    # 1인칭(카메라 로컬): 눈 (0,0,1.56). 치켜든 주먹은 우상단 모서리 안쪽(손목이 눈 위 6cm·앞 31cm·오른쪽 20cm), 타격은 화면 아래 중앙 가장자리,
    # 팔로스루는 중앙 왼쪽 아래(프레임 밖으로 살짝). palm 은 손바닥이 향하는 월드 방향 — 치켜들 땐 왼쪽·앞, 내려칠 땐 왼쪽·아래.
    'fp': [
        (0.00, SMALL_FP),
        (WINDUP_TIME, {**SMALL_FP, 'wrist': [-0.17, -0.33, 1.55], 'pole': [-1.0, 0.3, 0.2], 'fingers': [0.1, 0.3, 0.95], 'palm': [0.85, -0.45, 0.25]}),
        (MID_TIME, {**SMALL_FP, 'wrist': [-0.15, -0.44, 1.56], 'pole': [-1.0, -0.2, 0.5], 'fingers': [-0.05, -0.55, 0.83], 'palm': [0.9, -0.35, -0.1]}),
        (IMPACT_TIME, {**SMALL_FP, 'wrist': [-0.04, -0.42, 1.40], 'pole': [-0.6, -0.7, -0.6], 'fingers': [0.1, -0.7, -0.7], 'palm': [0.75, 0.15, -0.65]}),
        (FOLLOW_TIME, {**SMALL_FP, 'wrist': [0.02, -0.40, 1.40], 'pole': [-0.4, -0.6, -0.8], 'fingers': [0.3, -0.8, -0.5], 'palm': [0.7, 0.25, -0.65]}),
        (SECONDS, SMALL_FP),
    ],
}
# 구간별 완급 — KEYS 의 구간 순서와 같다(5구간). 치켜듦까지 완만, 타격까지 가속(in), 타격 순간 최고 속도(linear), 그 뒤 급감속(out3), 복귀 완만.
EASE = ['inout', 'in', 'linear', 'out3', 'inout']

# ---------- 전신 몸통 키 — yaw(+ = 오른쪽으로 비틀기), pitch(+ = 앞으로 숙임), roll(+ = 왼쪽으로 기울기), head_pitch(+ = 고개 듦), arm_l_back(+ = 왼팔 뒤로) ----------
TORSO = {
    'tp': [
        (0.00, {'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0, 'head_pitch': 0.0, 'head_yaw': 0.0, 'arm_l_back': 0.0, 'arm_l_bend': 0.0}),
        (WINDUP_TIME, {'yaw': 0.35, 'pitch': -0.12, 'roll': 0.05, 'head_pitch': 0.08, 'head_yaw': -0.15, 'arm_l_back': -0.10, 'arm_l_bend': 0.15}),
        (MID_TIME, {'yaw': 0.12, 'pitch': 0.02, 'roll': 0.02, 'head_pitch': 0.02, 'head_yaw': -0.05, 'arm_l_back': 0.05, 'arm_l_bend': 0.20}),
        (IMPACT_TIME, {'yaw': -0.30, 'pitch': 0.35, 'roll': -0.05, 'head_pitch': 0.18, 'head_yaw': 0.12, 'arm_l_back': 0.30, 'arm_l_bend': 0.25}),
        (FOLLOW_TIME, {'yaw': -0.34, 'pitch': 0.38, 'roll': -0.06, 'head_pitch': 0.20, 'head_yaw': 0.14, 'arm_l_back': 0.34, 'arm_l_bend': 0.25}),
        (SECONDS, {'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0, 'head_pitch': 0.0, 'head_yaw': 0.0, 'arm_l_back': 0.0, 'arm_l_bend': 0.0}),
    ],
    'fp': [],
}
SPINE_BONES = ('spine_01', 'spine_02', 'spine_03')   # 몸통 회전을 셋에 나눠 싣는다


def _measure_axes():
    """Manny 척추 뼈의 국부 축 — 실측: x 는 뼈 축 둘레 비틀기(yaw), y 는 옆으로 기울기(+ 가 왼쪽), z 는 앞뒤 기울기(+ 가 뒤로 젖힘).
    yaw 의 부호는 오른쪽 쇄골이 뒤(+Y)로 가는 쪽을 '오른쪽으로 비틀기' 로 정한다"""
    reset()
    base = rig.pose.bones['clavicle_r'].head.copy()
    apply_pose({'spine': {'spine_03': [0.3, 0.0, 0.0]}})
    moved = rig.pose.bones['clavicle_r'].head.copy()
    reset()
    return 1.0 if (moved - base).y > 0 else -1.0


YAW_RIGHT = _measure_axes()


def torso_spec(view, p):
    """몸통 파라미터 → apply_pose 의 'spine' 사전(뼈 → [x, y, z] rad)"""
    if view != 'tp' or not p:
        return dict(HOLD['tp']['spine'])
    n = len(SPINE_BONES)
    spine = {b: [YAW_RIGHT * p['yaw'] / n, p['roll'] / n, -p['pitch'] / n] for b in SPINE_BONES}
    spine['neck_01'] = [YAW_RIGHT * p['head_yaw'] * 0.4, 0.0, p['head_pitch'] * 0.4]
    spine['head'] = [YAW_RIGHT * p['head_yaw'] * 0.6, 0.0, p['head_pitch'] * 0.6]
    spine['upperarm_l'] = [0.0, 0.0, p['arm_l_back']]
    spine['lowerarm_l'] = [0.0, 0.0, -p['arm_l_bend']]
    return spine


def ease(kind, u):
    if kind == 'in':
        return u * u
    if kind == 'out':
        return 1.0 - (1.0 - u) ** 2
    if kind == 'out3':
        return 1.0 - (1.0 - u) ** 3
    if kind == 'inout':
        return u * u * (3.0 - 2.0 * u)
    return u


def _lerp_dict(a, b, w):
    """두 키 사전을 가중치 w 로 섞는다 — 한쪽에만 있는 필드는 그쪽 값 그대로(예: hand_roll 이 없는 키)"""
    out = {}
    for k in set(a) | set(b):
        if k not in a or k not in b:
            out[k] = a.get(k, b.get(k))
        elif isinstance(a[k], list):
            out[k] = [x + (y - x) * w for x, y in zip(a[k], b[k])]
        elif isinstance(a[k], (int, float)):
            out[k] = a[k] + (b[k] - a[k]) * w
        else:
            out[k] = a[k]
    return out


def lerp_keys(keys, t, eases=None):
    """(시각, 사전) 키 목록을 시각 t 에서 보간 — 구간별 완급(eases[i])을 적용"""
    for i, ((t0, a), (t1, b)) in enumerate(zip(keys, keys[1:])):
        if t0 <= t <= t1:
            u = 0.0 if t1 == t0 else (t - t0) / (t1 - t0)
            w = ease(eases[i] if eases else 'linear', u)
            return _lerp_dict(a, b, w)
    return dict(keys[-1][1])


def lerp_spec(keys, t):
    """옛 이름 호환(직선 보간)"""
    return lerp_keys(keys, t)


REACH_M = 0.54   # 위팔 0.278 + 아래팔 0.273 = 0.551 — 완전히 펴지기 직전까지만


def clamp_reach(spec):
    """몸통을 먼저 적용한 뒤 어깨에서 손목까지가 REACH_M 을 넘으면 그 방향으로 당긴다 — 타격 키가 뻗은 자세라 몸통이 덜 숙여진 중간
    프레임에서 팔 길이를 넘을 수 있다(arm_ik 의 assert 를 피한다)"""
    apply_pose({'spine': spec['spine']})
    for side in ('r', 'l'):
        arm = spec.get('arm_' + side)
        if not arm:
            continue
        shoulder = world_base('upperarm_' + side).translation
        d = Vector(arm['wrist']) - shoulder
        if d.length > REACH_M:
            arm = dict(arm)
            arm['wrist'] = list(shoulder + d.normalized() * REACH_M)
            spec['arm_' + side] = arm
    return spec


def pose_at(view, t):
    arm = lerp_keys(KEYS[view], t, EASE)
    torso = lerp_keys(TORSO[view], t, EASE) if TORSO[view] else None
    return clamp_reach({'spine': torso_spec(view, torso), 'arm_r': arm})


# 공격 대기 클립이 쓰는 '치켜든 시각의 척추' — 전신은 몸통 비틀림 포함, 1인칭은 identity
SPINE = {view: pose_at(view, WINDUP_TIME)['spine'] for view in ('tp', 'fp')}
PREVIEW_FRAMES = tuple(range(1, FRAMES + 1))   # 전 프레임 — 시트(contact_sheet.py)·GIF 용. 전신은 f5(치켜듦)·f9(타격)만 정면(왼쪽 앞) 각도도 찍는다
FRONT_FRAMES = (5, 9)
SIDE_CAM = (-2.2, -1.9, 1.5)   # 오른쪽 앞 3/4 — 휘두르는 팔이 카메라 쪽


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    for view in VIEWS:
        name = 'AN_Farmer_Swing' if view == 'tp' else 'AN_Farmer_FPSwing'
        scene.frame_start, scene.frame_end = 1, FRAMES
        action = new_action(name) if EXPORT else None
        if rig.animation_data and not EXPORT:
            rig.animation_data.action = None
        keyed = set()
        samples = {}
        wrist_path = []
        for frame in range(1, FRAMES + 1):
            t = (frame - 1) / FPS
            spec = pose_at(view, t)
            scene.frame_set(frame)
            apply_pose(spec)
            wrist_path.append([round(v, 3) for v in rig.pose.bones['hand_r'].head])
            if view == 'tp' and frame in (5, 9):
                pbn = rig.pose.bones
                samples[f'f{frame}_shoulder_twist_cm'] = round((pbn['upperarm_r'].head.y - pbn['upperarm_l'].head.y) * 100, 1)  # + = 오른 어깨가 뒤
            if EXPORT:
                key_current_pose(keyed, frame)
            if frame in PREVIEW_FRAMES:
                if view == 'tp':
                    render_front(f'swing_{view}_f{frame}', PREVIEW_DIR, cam_location=SIDE_CAM)
                    if frame in FRONT_FRAMES:
                        render_front(f'swing_{view}_f{frame}L', PREVIEW_DIR)
                else:
                    render_fp(f'swing_{view}_f{frame}', PREVIEW_DIR)
                    samples[f'f{frame}'] = fp_pose_report('r')
        # 손목 경로의 프레임별 이동 거리(cm) — 완급이 의도대로인지(타격 직전 최대)
        speeds = [round((Vector(b) - Vector(a)).length * 100, 1) for a, b in zip(wrist_path, wrist_path[1:])]
        results[name] = {'wrist_impact': [round(v, 3) for v in KEYS[view][3][1]['wrist']], 'frames': FRAMES, 'seconds': round(SECONDS, 4),
                         'wrist_step_cm': speeds}
        if samples:
            results[name]['fp_samples' if view == 'fp' else 'tp_samples'] = samples
        if EXPORT:
            for fc in action_fcurves(action):
                for kp in fc.keyframe_points:
                    kp.interpolation = 'LINEAR'
            out = export_action(action)
            results[name].update({'fbx': str(out), 'sha256': hashlib.sha256(out.read_bytes()).hexdigest(), 'keyed_bones': len(keyed)})
    apply_pose({'spine': {}, 'arm_r': KEYS['tp'][0][1]})
    rig.animation_data_clear() if rig.animation_data else None
    if EXPORT:
        manifest_path = ROOT / 'manifest.json'
        manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
        anims = manifest.setdefault('animations', {})
        for k, r in results.items():
            entry = {'fbx_sha256': r['sha256'], 'loop': False, 'seconds': round(SECONDS, 4), 'fps': FPS, 'frames': FRAMES,
                     'windup_time': WINDUP_TIME, 'impact_time': IMPACT_TIME, 'ease': EASE}
            if 'FPSwing' in k:
                entry.update({'space': 'camera', 'eye_height_m': FP_EYE_HEIGHT_M, 'layer': 'full rig (camera-local view model, spine identity)',
                              'view': 'first-person arms',
                              'pose': 'HoldSmall(fp) -> windup(upper right corner) -> mid(centre right, high) -> impact(bottom centre) -> follow(bottom left) -> HoldSmall(fp)'})
            else:
                entry.update({'layer': 'upper body from spine_01 (montage slot)', 'view': 'full body',
                              'pose': 'HoldSmall(tp) -> windup(over right shoulder, torso twisted right) -> impact(extended forward, torso twisted left, leaning) -> HoldSmall(tp)'})
            anims[k] = entry
        manifest['swing_keys'] = {'arm_r': KEYS, 'torso': TORSO, 'ease': EASE}
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    print('SWING RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
