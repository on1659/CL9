"""농부 A 먹기 클립 — 우클 길게(Dish 먹기)의 표현. Small 들기 자세(farmer_hold_spec.json 의 Small)에서 오른손이 입 앞으로
갔다가(0→0.25초) 잠깐 머물고(0.25~0.35초) 돌아오는(→0.6초) 0.6초 원샷, 전신(AN_Farmer_Eat)과 1인칭 팔(AN_Farmer_FPEat)
한 쌍. 시작·끝이 HoldSmall 과 같은 자세라 레이어 이음새가 없다. 1인칭은 손이 화면 아래 중앙에서 올라와 렌즈 앞까지
오는 느낌(사용자: "손으로 가까워서 먹는 느낌"). 키 자세 사이는 선형 보간이고 판정은 애님과 무관하게 즉시다.
`--export`면 액션 저장 + cm 스테이징 FBX + manifest 갱신.

실행: py -3 Tools/Blender/run_headless.py animate_farmer_eat.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export]
"""
import sys, json, hashlib
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from farmer_pose import ROOT, scene, rig, apply_pose, key_current_pose, new_action, action_fcurves, export_action, render_front, render_fp
from animate_farmer_swing import lerp_spec

EXPORT = '--export' in sys.argv
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
SECONDS = 0.6
HOLD = json.loads((Path(__file__).with_name('farmer_hold_spec.json')).read_text(encoding='utf-8'))['Small']

# 입 앞 자세 — 머리 뼈(0, -0.007, 1.626m)에서 앞 ~20cm(얼굴 면 앞 10cm), 입 높이. 손가락은 입(얼굴)을 향하고 팔꿈치는
# 아래·바깥으로 내려간다. fp 는 게임 카메라(눈높이 ~1.56m) 기준으로 손이 화면 아래 중앙에서 올라와 렌즈 바로 앞에 선다.
MOUTH = {
    'tp': {'wrist': [-0.07, -0.24, 1.57], 'pole': [-0.9, -0.2, -1.0], 'fingers': [0.4, 0.85, 0.35], 'hand_roll': 0.0, 'curl': 0.6, 'thumb': 0.3},
    'fp': {'wrist': [-0.05, -0.27, 1.50], 'pole': [-1.0, -0.3, -1.0], 'fingers': [0.3, 0.9, 0.4], 'hand_roll': 0.0, 'curl': 0.6, 'thumb': 0.3},
}
KEYS = {view: [(0.00, dict(HOLD[view]['arm_r'])), (0.25, MOUTH[view]), (0.35, MOUTH[view]), (SECONDS, dict(HOLD[view]['arm_r']))] for view in ('tp', 'fp')}
SPINE = {view: HOLD[view]['spine'] for view in ('tp', 'fp')}


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    for view in ('tp', 'fp'):
        name = 'AN_Farmer_Eat' if view == 'tp' else 'AN_Farmer_FPEat'
        end = round(SECONDS * FPS) + 1          # 19 프레임
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
                render_front(f'eat_{view}_f{frame}', PREVIEW_DIR)
                if view == 'fp':
                    render_fp(f'eat_{view}_f{frame}', PREVIEW_DIR)
            if frame == 9:
                results[name] = {'wrist_mouth': [round(v, 4) for v in rig.pose.bones['hand_r'].head],
                                 'head_bone': [round(v, 4) for v in rig.pose.bones['head'].head], 'frames': end}
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
        manifest.setdefault('animations', {}).update({k: {'fbx_sha256': r['sha256'], 'loop': False, 'seconds': SECONDS, 'fps': FPS,
                                                           'layer': 'upper body from spine_01 (montage slot)',
                                                           'view': 'first-person arms' if 'FPEat' in k else 'full body',
                                                           'pose': 'HoldSmall -> mouth (0.25~0.35s) -> HoldSmall'} for k, r in results.items()})
        manifest['eat_keys'] = KEYS
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    print('EAT RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
