"""농부 A 먹기 클립 — 우클 길게(Dish 먹기)의 표현. Small 들기 자세(farmer_hold_spec.json 의 Small)에서 오른손이 입 앞으로
갔다가(0→0.25초) 잠깐 머물고(0.25~0.35초) 돌아오는(→0.6초) 0.6초 원샷, 전신(AN_Farmer_Eat)과 1인칭 팔(AN_Farmer_FPEat)
한 쌍. 시작·끝이 HoldSmall 과 같은 자세라 이음새가 없다. 1인칭은 카메라 로컬 클립(031) — 주먹이 화면 우하단에서 올라와 렌즈 앞
중앙(눈 아래 7cm·앞 27cm)에 서고 손가락이 입(뒤·위)을 향한다(사용자: "손으로 가까워서 먹는 느낌"). 키 자세 사이는 선형 보간이고
판정은 애님과 무관하게 즉시다.
`--export`면 액션 저장 + cm 스테이징 FBX + manifest 갱신. `--fp-only` 면 1인칭만(전신 FBX 무변경).

실행: py -3 Tools/Blender/run_headless.py animate_farmer_eat.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export --fp-only]
"""
import sys, json, hashlib
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from farmer_pose import (ROOT, scene, rig, apply_pose, key_current_pose, new_action, action_fcurves, export_action, render_front, render_fp,
                         fp_pose_report, FP_EYE_HEIGHT_M)
from animate_farmer_swing import lerp_spec

EXPORT = '--export' in sys.argv
FP_ONLY = '--fp-only' in sys.argv
VIEWS = ('fp',) if FP_ONLY else ('tp', 'fp')
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
SECONDS = 0.6
HOLD = json.loads((Path(__file__).with_name('farmer_hold_spec.json')).read_text(encoding='utf-8'))['Small']

# 입 앞 자세 — tp: 머리 뼈(0, -0.007, 1.626m)에서 앞 ~20cm(얼굴 면 앞 10cm), 입 높이, 손가락은 얼굴을 향하고 팔꿈치는 아래·바깥.
# fp: 카메라 로컬 — 손목이 눈 아래 7cm·앞 27cm·오른쪽 5cm, 손가락은 뒤·위(입)를 향하고 손바닥은 얼굴 쪽. 필드는 Small 과 같아야 lerp 가 된다.
MOUTH = {
    'tp': {'wrist': [-0.07, -0.24, 1.57], 'pole': [-0.9, -0.2, -1.0], 'fingers': [0.4, 0.85, 0.35], 'hand_roll': 0.0, 'curl': 0.6, 'thumb': 0.3},
    'fp': {'wrist': [-0.05, -0.27, 1.49], 'pole': [-0.9, -0.2, -0.8], 'fingers': [0.15, 0.55, 0.82], 'palm': [0.4, 0.8, -0.4], 'hand_roll': 0.0,
           'curl': HOLD['fp']['arm_r']['curl'], 'thumb': HOLD['fp']['arm_r']['thumb']},
}
for view in ('tp', 'fp'):
    assert set(MOUTH[view]) == set(HOLD[view]['arm_r']), ('MOUTH 필드가 Small 과 다르다', view, set(HOLD[view]['arm_r']))
KEYS = {view: [(0.00, dict(HOLD[view]['arm_r'])), (0.25, MOUTH[view]), (0.35, MOUTH[view]), (SECONDS, dict(HOLD[view]['arm_r']))] for view in ('tp', 'fp')}
SPINE = {view: HOLD[view]['spine'] for view in ('tp', 'fp')}
PREVIEW_FRAMES = (1, 5, 9)   # t = 0 / 0.133(올라가는 중) / 0.267(입 앞)


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    for view in VIEWS:
        name = 'AN_Farmer_Eat' if view == 'tp' else 'AN_Farmer_FPEat'
        end = round(SECONDS * FPS) + 1          # 19 프레임
        scene.frame_start, scene.frame_end = 1, end
        action = new_action(name) if EXPORT else None
        if rig.animation_data and not EXPORT:
            rig.animation_data.action = None
        keyed = set()
        samples = {}
        for frame in range(1, end + 1):
            t = (frame - 1) / FPS
            spec = {'spine': SPINE[view], 'arm_r': lerp_spec(KEYS[view], t)}
            scene.frame_set(frame)
            apply_pose(spec)
            if EXPORT:
                key_current_pose(keyed, frame)
            if frame in PREVIEW_FRAMES:
                render_front(f'eat_{view}_f{frame}', PREVIEW_DIR)
                if view == 'fp':
                    render_fp(f'eat_{view}_f{frame}', PREVIEW_DIR)
                    samples[f'f{frame}'] = fp_pose_report('r')
            if frame == 9:
                results[name] = {'wrist_mouth': [round(v, 4) for v in rig.pose.bones['hand_r'].head],
                                 'head_bone': [round(v, 4) for v in rig.pose.bones['head'].head], 'frames': end}
        if view == 'fp':
            results[name]['fp_samples'] = samples
        if EXPORT:
            for fc in action_fcurves(action):
                for kp in fc.keyframe_points:
                    kp.interpolation = 'LINEAR'
            out = export_action(action)
            results[name].update({'fbx': str(out), 'sha256': hashlib.sha256(out.read_bytes()).hexdigest(), 'keyed_bones': len(keyed)})
    apply_pose({'spine': {}, 'arm_r': HOLD['tp']['arm_r']})
    rig.animation_data_clear() if rig.animation_data else None
    if EXPORT:
        manifest_path = ROOT / 'manifest.json'
        manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
        anims = manifest.setdefault('animations', {})
        for k, r in results.items():
            entry = {'fbx_sha256': r['sha256'], 'loop': False, 'seconds': SECONDS, 'fps': FPS, 'pose': 'HoldSmall -> mouth (0.25~0.35s) -> HoldSmall'}
            if 'FPEat' in k:
                entry.update({'space': 'camera', 'eye_height_m': FP_EYE_HEIGHT_M, 'layer': 'full rig (camera-local view model, spine identity)',
                              'view': 'first-person arms'})
            else:
                entry.update({'layer': 'upper body from spine_01 (montage slot)', 'view': 'full body'})
            anims[k] = entry
        manifest['eat_keys'] = KEYS
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    print('EAT RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
