"""농부 A 휘두르기 클립 — 좌클 Swing(프라이팬·고기)의 표현. Small 들기 자세에서 오른팔을 치켜들었다가
정면 아래로 내려치고 되돌아오는 0.45초 원샷, 전신(AN_Farmer_Swing)과 1인칭 팔(AN_Farmer_FPSwing) 한 쌍.
키 자세 사이는 선형 보간이고 판정은 애님과 무관하게 누르는 즉시다(docs/systems/input.md 사용).
`--export`면 액션 저장 + cm 스테이징 FBX + manifest 갱신.

실행: py -3 Tools/Blender/run_headless.py animate_farmer_swing.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export]
KEYS·SPINE·lerp_spec 은 공격 대기 클립(animate_farmer_windup.py)이 import 해 쓴다 — 본문은 __main__ 일 때만 돈다.
"""
import sys, json, hashlib
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from farmer_pose import ROOT, scene, rig, apply_pose, key_current_pose, new_action, action_fcurves, export_action, render_front

EXPORT = '--export' in sys.argv
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
SECONDS = 0.45

# (시각, 오른팔 스펙) — 시작/끝은 Hold Small과 같은 자세라 레이어 블렌드 이음새가 없다.
KEYS = {
    'tp': [
        (0.00, {'wrist': [-0.17, -0.30, 1.20], 'pole': [-0.6, 0.0, -1.0], 'fingers': [0.0, -1.0, 0.3], 'curl': 0.6, 'thumb': 0.3}),
        (0.12, {'wrist': [-0.30, -0.18, 1.50], 'pole': [-1.0, 0.3, -0.4], 'fingers': [-0.3, -0.6, 0.8], 'curl': 0.6, 'thumb': 0.3}),
        (0.25, {'wrist': [-0.05, -0.33, 1.08], 'pole': [-0.6, -0.8, -0.8], 'fingers': [0.3, -0.9, -0.4], 'curl': 0.6, 'thumb': 0.3}),
        (0.45, {'wrist': [-0.17, -0.30, 1.20], 'pole': [-0.6, 0.0, -1.0], 'fingers': [0.0, -1.0, 0.3], 'curl': 0.6, 'thumb': 0.3}),
    ],
    'fp': [
        (0.00, {'wrist': [-0.15, -0.45, 1.36], 'pole': [-1.0, -0.3, -1.0], 'fingers': [0.0, -1.0, 0.3], 'curl': 0.6, 'thumb': 0.3}),
        (0.12, {'wrist': [-0.32, -0.30, 1.58], 'pole': [-1.0, 0.2, -0.6], 'fingers': [-0.3, -0.6, 0.8], 'curl': 0.6, 'thumb': 0.3}),
        (0.25, {'wrist': [-0.02, -0.42, 1.24], 'pole': [-0.6, -0.8, -0.8], 'fingers': [0.3, -0.9, -0.4], 'curl': 0.6, 'thumb': 0.3}),
        (0.45, {'wrist': [-0.15, -0.45, 1.36], 'pole': [-1.0, -0.3, -1.0], 'fingers': [0.0, -1.0, 0.3], 'curl': 0.6, 'thumb': 0.3}),
    ],
}
SPINE = {'tp': {'spine_03': [0.04, 0.0, 0.0]}, 'fp': {}}


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
    results = {}
    for view in ('tp', 'fp'):
        name = 'AN_Farmer_Swing' if view == 'tp' else 'AN_Farmer_FPSwing'
        end = round(SECONDS * FPS) + 1
        scene.frame_start, scene.frame_end = 1, end
        action = new_action(name) if EXPORT else None
        keyed = set()
        for frame in range(1, end + 1):
            t = (frame - 1) / FPS
            spec = {'spine': SPINE[view], 'arm_r': lerp_spec(KEYS[view], t)}
            scene.frame_set(frame)
            apply_pose(spec)
            if EXPORT:
                key_current_pose(keyed, frame)
            if frame in (1, 5, 9):
                render_front(f'swing_{view}_f{frame}', PREVIEW_DIR)
        results[name] = {'wrist_impact': [round(v, 3) for v in KEYS[view][2][1]['wrist']], 'frames': end}
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
        manifest.setdefault('animations', {}).update({k: {'fbx_sha256': r['sha256'], 'loop': False, 'seconds': SECONDS, 'layer': 'upper body from spine_01 (montage slot)',
                                                           'view': 'first-person arms' if 'FPSwing' in k else 'full body'} for k, r in results.items()})
        manifest['swing_keys'] = KEYS
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    print('SWING RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
