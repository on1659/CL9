"""농부 A 공격 대기 클립 — 좌클을 누르고 있는 동안 도는 루프(전신 AN_Farmer_SwingWindup / 1인칭 팔 AN_Farmer_FPSwingWindup).
자세는 휘두르기(animate_farmer_swing.py) KEYS 의 t=0.12 '치켜든 자세' 그대로이고, 1.0초 동안 손목이 위아래로 ±1cm 사인
곡선을 그리며 숨쉬듯 흔들린다. 시작·끝 프레임의 사인값이 0 이라 루프 이음새가 없고, 뗀 순간 ABP 가 AN_Farmer_Swing 을
t=0.12 부터 이어 틀면 자세가 그대로 이어진다(docs/systems/animation.md "공격 대기·먹기 가지").
1인칭은 카메라 로컬 클립(031) — 치켜든 주먹이 피치 0 에서 화면 우상단 안에 있다(swing 의 WINDUP_FP).
`--export`면 액션 저장 + cm 스테이징 FBX + manifest 갱신. `--fp-only` 면 1인칭만(전신 FBX 무변경).

실행: py -3 Tools/Blender/run_headless.py animate_farmer_windup.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export --fp-only]
"""
import sys, json, hashlib, math
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from farmer_pose import (ROOT, scene, rig, apply_pose, key_current_pose, new_action, action_fcurves, export_action, render_front, render_fp,
                         fp_pose_report, FP_EYE_HEIGHT_M)
from animate_farmer_swing import KEYS, SPINE, WINDUP_TIME

EXPORT = '--export' in sys.argv
FP_ONLY = '--fp-only' in sys.argv
VIEWS = ('fp',) if FP_ONLY else ('tp', 'fp')
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
SECONDS = 1.0
BREATH_CM = 1.0         # 손목 위아래 진폭
SPINE_BREATH = 0.006    # spine_03 앞뒤 기울기 진폭(rad) — 전신만, 미세하게
PREVIEW_FRAMES = {'tp': (1, 8, 16), 'fp': (1,)}

WINDUP = {view: next(spec for t, spec in KEYS[view] if abs(t - WINDUP_TIME) < 1e-6) for view in ('tp', 'fp')}


def breathing_spec(view, u):
    """u∈[0,1] 한 주기. 손목 z 에 ±BREATH_CM 사인, 나머지는 치켜든 자세 그대로"""
    base = WINDUP[view]
    s = math.sin(math.tau * u)
    arm = dict(base)
    arm['wrist'] = [base['wrist'][0], base['wrist'][1], base['wrist'][2] + BREATH_CM * 0.01 * s]
    return {'spine': SPINE[view], 'arm_r': arm}, SPINE_BREATH * s if view == 'tp' else 0.0


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    for view in VIEWS:
        name = 'AN_Farmer_SwingWindup' if view == 'tp' else 'AN_Farmer_FPSwingWindup'
        end = round(SECONDS * FPS) + 1          # 31 프레임 — 마지막 프레임이 첫 프레임과 같은 자세(루프)
        scene.frame_start, scene.frame_end = 1, end
        action = new_action(name) if EXPORT else None
        if rig.animation_data and not EXPORT:
            rig.animation_data.action = None
        keyed = set()
        for frame in range(1, end + 1):
            u = (frame - 1) / (end - 1)
            spec, breath = breathing_spec(view, u)
            scene.frame_set(frame)
            apply_pose(spec, breath=breath)
            if EXPORT:
                key_current_pose(keyed, frame)
            if frame in PREVIEW_FRAMES[view]:
                render_front(f'windup_{view}_f{frame}', PREVIEW_DIR)
                if view == 'fp':
                    render_fp(f'windup_{view}_f{frame}', PREVIEW_DIR)
        results[name] = {'wrist': [round(v, 3) for v in WINDUP[view]['wrist']], 'frames': end}
        if view == 'fp':
            results[name]['fp_r'] = fp_pose_report('r')
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
            entry = {'fbx_sha256': r['sha256'], 'loop': True, 'seconds': SECONDS, 'fps': FPS, 'pose': f'AN_Farmer_Swing t={WINDUP_TIME}', 'breath_cm': BREATH_CM}
            if 'FPSwing' in k:
                entry.update({'space': 'camera', 'eye_height_m': FP_EYE_HEIGHT_M, 'layer': 'full rig (camera-local view model, spine identity)',
                              'view': 'first-person arms'})
            else:
                entry.update({'layer': 'upper body from spine_01 (montage slot)', 'view': 'full body'})
            anims[k] = entry
        manifest['windup_keys'] = {'time_in_swing': WINDUP_TIME, 'breath_cm': BREATH_CM, 'spine_breath_rad': SPINE_BREATH, 'pose': WINDUP}
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    print('WINDUP RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
