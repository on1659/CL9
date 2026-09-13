"""농부 A 휘두르기 클립 — 좌클 Swing(프라이팬·고기)의 표현. Small 들기 자세에서 오른팔을 치켜들었다가 정면 아래로 내려치고
되돌아오는 **0.467초(30fps 15프레임 = 14 간격)** 원샷, 전신(AN_Farmer_Swing)과 1인칭 팔(AN_Farmer_FPSwing) 한 쌍.
키 자세 사이는 선형 보간이고 판정은 애님과 무관하게 누르는 즉시다(docs/systems/input.md 사용).

- tp 키는 026 그대로(전신 HoldSmall 과 같은 시작·끝). 026 의 0.45초 저작이 옛 스테이징 24fps 로 UE 에 0.583초로 들어갔던 것을
  30fps 로 다시 낸다 — UE 길이 0.467, 대기→휘두르기 오프셋 0.12·평가 범위 0.347(031).
- fp 키는 시작·끝을 farmer_hold_spec.json `Small.fp` 에서 읽고(Small 이 바뀌면 자동 재키잉), 치켜든 자세(WINDUP_FP, t=0.12 —
  화면 안 우상단)와 내려친 자세(IMPACT_FP, t=0.25 — 화면 아래 중앙)는 카메라 로컬·피치 0 프레이밍으로 여기 적는다(farmer_pose 의
  1인칭 카메라 모델, 손바닥은 palm 으로 정렬). 공격 대기 클립(animate_farmer_windup.py)이 KEYS·SPINE·lerp_spec 을 import 한다 —
  본문은 __main__ 일 때만 돈다.
`--export`면 액션 저장 + cm 스테이징 FBX + manifest 갱신(전신·1인칭 둘 다).

실행: py -3 Tools/Blender/run_headless.py animate_farmer_swing.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export]
"""
import sys, json, hashlib
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from farmer_pose import (ROOT, scene, rig, apply_pose, key_current_pose, new_action, action_fcurves, export_action, render_front, render_fp,
                         fp_pose_report, FP_EYE_HEIGHT_M)

EXPORT = '--export' in sys.argv
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
FRAMES = 15
SECONDS = (FRAMES - 1) / FPS          # 0.4667
WINDUP_TIME, IMPACT_TIME = 0.12, 0.25
HOLD = json.loads((Path(__file__).with_name('farmer_hold_spec.json')).read_text(encoding='utf-8'))['Small']
SMALL_FP = dict(HOLD['fp']['arm_r'])
# 1인칭 치켜든 자세 — 주먹이 화면 우상단 안(손목 눈 위 1cm·앞 34cm·오른쪽 17cm → 손목 uv ≈ (0.86, 0.47), 너클 ≈ (0.82, 0.25)),
# 팔꿈치 바깥·뒤로. 내려친 자세 — 주먹이 화면 아래 중앙 가장자리(손목 눈 아래 14cm·앞 40cm), 팔꿈치 앞·아래.
# 키 이름은 SMALL_FP 와 같아야 lerp_spec 이 보간한다.
WINDUP_FP = {'wrist': [-0.17, -0.34, 1.57], 'pole': [-1.0, 0.2, -0.5], 'fingers': [-0.25, -0.55, 0.8], 'palm': [0.9, 0.35, 0.2],
             'hand_roll': 0.0, 'curl': SMALL_FP['curl'], 'thumb': SMALL_FP['thumb']}
IMPACT_FP = {'wrist': [-0.05, -0.40, 1.42], 'pole': [-0.5, -0.8, -0.8], 'fingers': [0.3, -0.85, -0.45], 'palm': [0.9, 0.0, 0.4],
             'hand_roll': 0.0, 'curl': SMALL_FP['curl'], 'thumb': SMALL_FP['thumb']}
assert set(WINDUP_FP) == set(SMALL_FP) == set(IMPACT_FP), ('fp 키 자세의 필드가 Small.fp 와 다르다', set(SMALL_FP))

# (시각, 오른팔 스펙) — 시작/끝은 Hold Small 과 같은 자세라 블렌드 이음새가 없다.
KEYS = {
    'tp': [
        (0.00, {'wrist': [-0.17, -0.30, 1.20], 'pole': [-0.6, 0.0, -1.0], 'fingers': [0.0, -1.0, 0.3], 'curl': 0.6, 'thumb': 0.3}),
        (WINDUP_TIME, {'wrist': [-0.30, -0.18, 1.50], 'pole': [-1.0, 0.3, -0.4], 'fingers': [-0.3, -0.6, 0.8], 'curl': 0.6, 'thumb': 0.3}),
        (IMPACT_TIME, {'wrist': [-0.05, -0.33, 1.08], 'pole': [-0.6, -0.8, -0.8], 'fingers': [0.3, -0.9, -0.4], 'curl': 0.6, 'thumb': 0.3}),
        (SECONDS, {'wrist': [-0.17, -0.30, 1.20], 'pole': [-0.6, 0.0, -1.0], 'fingers': [0.0, -1.0, 0.3], 'curl': 0.6, 'thumb': 0.3}),
    ],
    'fp': [
        (0.00, SMALL_FP),
        (WINDUP_TIME, WINDUP_FP),
        (IMPACT_TIME, IMPACT_FP),
        (SECONDS, SMALL_FP),
    ],
}
SPINE = {'tp': {'spine_03': [0.04, 0.0, 0.0]}, 'fp': {}}
PREVIEW_FRAMES = (1, 5, 9)   # t = 0 / 0.133(치켜든 자세 직후) / 0.267(내려친 자세 직후)


def lerp_spec(keys, t):
    for (t0, a), (t1, b) in zip(keys, keys[1:]):
        if t0 <= t <= t1:
            u = 0.0 if t1 == t0 else (t - t0) / (t1 - t0)
            out = {}
            for k in a:
                if isinstance(a[k], list):
                    out[k] = [x + (y - x) * u for x, y in zip(a[k], b[k])]
                else:
                    out[k] = a[k] + (b[k] - a[k]) * u
            return out
    return dict(keys[-1][1])


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    for view in ('tp', 'fp'):
        name = 'AN_Farmer_Swing' if view == 'tp' else 'AN_Farmer_FPSwing'
        scene.frame_start, scene.frame_end = 1, FRAMES
        action = new_action(name) if EXPORT else None
        if rig.animation_data and not EXPORT:
            rig.animation_data.action = None
        keyed = set()
        samples = {}
        for frame in range(1, FRAMES + 1):
            t = (frame - 1) / FPS
            spec = {'spine': SPINE[view], 'arm_r': lerp_spec(KEYS[view], t)}
            scene.frame_set(frame)
            apply_pose(spec)
            if EXPORT:
                key_current_pose(keyed, frame)
            if frame in PREVIEW_FRAMES:
                render_front(f'swing_{view}_f{frame}', PREVIEW_DIR)
                if view == 'fp':
                    render_fp(f'swing_{view}_f{frame}', PREVIEW_DIR)
                    samples[f'f{frame}'] = fp_pose_report('r')
        results[name] = {'wrist_impact': [round(v, 3) for v in KEYS[view][2][1]['wrist']], 'frames': FRAMES, 'seconds': round(SECONDS, 4)}
        if view == 'fp':
            results[name]['fp_samples'] = samples
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
                     'windup_time': WINDUP_TIME, 'impact_time': IMPACT_TIME}
            if 'FPSwing' in k:
                entry.update({'space': 'camera', 'eye_height_m': FP_EYE_HEIGHT_M, 'layer': 'full rig (camera-local view model, spine identity)',
                              'view': 'first-person arms', 'pose': 'HoldSmall(fp) -> windup(upper right, in frame) -> impact(bottom centre) -> HoldSmall(fp)'})
            else:
                entry.update({'layer': 'upper body from spine_01 (montage slot)', 'view': 'full body'})
            anims[k] = entry
        manifest['swing_keys'] = KEYS
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    print('SWING RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
