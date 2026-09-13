"""농부 A 1인칭 팔 빈손 클립 두 개 — 리썰컴퍼니식 뷰모델(fp-arms-flashlight, 사용자 "최초 화면에선 안 보이다가 뛸 때 손을
흔들면서 뛰는 애니메이션으로 손을 자연스럽게 보여주자").

- AN_Farmer_FPArmsDown: 두 손이 카메라 프레임 밖(아래·바깥)으로 내려간 자세 2.0초 루프. 손목 z ±0.5cm 사인 숨쉬기. 빈손 서기·걷기의
  기본(AN_Farmer_FPHoldIdle 대체). 자세는 달리기 팔 호(아래)의 θ=ARMS_DOWN_DEG 지점이라 달리기로 블렌드가 짧다.
- AN_Farmer_FPRun: 달리기 팔 흔들기 0.6초 루프(RunSpeed 654cm/s 기준 한 주기 = 좌·우 교대 한 번씩). 손목이 어깨를 축으로 반지름
  RUN_RADIUS 의 호를 따라 뒤(θmin)↔앞·위(θmax)로 사인 왕복, 왼팔은 반대 위상. 앞 극점에서 손목이 카메라(눈높이 1.56m, FOV 70,
  16:9) 프레임 아래 좌우로 들어왔다가 뒤로 가면 나간다. ABP 가 PlayRate = Speed / reference_speed_cm_s(654) 로 튼다.

전신 클립은 만들지 않는다(남이 보는 전신은 템플릿 로코모션 그대로). `--export`면 액션 저장 + cm 스테이징 FBX + manifest 갱신.

실행: py -3 Tools/Blender/run_headless.py animate_farmer_fparms.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export]
"""
import sys, json, hashlib, math
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from mathutils import Vector
from farmer_pose import ROOT, scene, rig, rest, apply_pose, aim, curl, key_current_pose, new_action, action_fcurves, export_action, render_front, render_fp

EXPORT = '--export' in sys.argv
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
RUN_SECONDS = 0.6            # 654cm/s 에서 한 주기(좌·우 팔 교대)
RUN_REFERENCE_SPEED = 654    # cm/s — docs/plan/data/character.md RunSpeed, ABP PlayRate = Speed / 654
DOWN_SECONDS = 2.0
DOWN_BREATH_CM = 0.5

# 팔 호(m): 어깨(upperarm 기준 위치)를 축으로 반지름 RUN_RADIUS, θ=0 이 수직 아래, +θ 가 앞·위. 어깨 z 1.436 이라 θ=92° 면 손목이
# 어깨 높이·앞 32cm → 카메라(1.56)에서 20° 아래 = 16:9 FOV70 화면의 세로 96% 지점(아래 가장자리, 주먹은 그 위 0.72~0.95),
# θ≤40° 면 60° 넘게 아래(프레임 밖). 화면 세로 = 0.5 + 0.5·tanθ/tan21.5°.
RUN_RADIUS = 0.35
RUN_THETA_MIN, RUN_THETA_MAX = -25.0, 92.0
RUN_X_BACK, RUN_X_FRONT = 0.28, 0.20         # 손목 바깥쪽 거리 — 뒤에서 벌어지고 앞에서 좁혀진다
RUN_FINGERS_BACK, RUN_FINGERS_FRONT = (0.0, -0.75, -0.65), (0.1, -0.8, 0.55)
RUN_CURL, RUN_THUMB = 1.1, 0.6
RUN_PALM = (1.0, 0.0, -0.6)  # 주먹 손바닥이 몸 안쪽(오른손 +x)·아래를 본다 — 엄지가 위·안쪽, 카메라엔 손등·엄지
RUN_POLE = (-0.5, 0.3, -1.0)  # 오른팔 — 팔꿈치 바깥·뒤·아래, 왼팔은 x 반전
ARMS_DOWN_DEG = 22.0
ARMS_DOWN_RADIUS = 0.50      # 어깨~손목 거리 — 팔이 거의 펴진 채(팔꿈치 ~130°) 늘어진다
ARMS_DOWN_X = 0.26
ARMS_DOWN_FINGERS = (0.0, -0.45, -0.9)
ARMS_DOWN_CURL, ARMS_DOWN_THUMB = 0.35, 0.25
ARMS_DOWN_PALM = (1.0, 0.3, 0.0)  # 늘어뜨린 손 — 손바닥이 허벅지 쪽(안쪽)
ARMS_DOWN_POLE = (-0.3, 0.6, -1.0)  # 팔꿈치가 옆이 아니라 뒤로

SHOULDER = {side: rest['upperarm_' + side].translation.copy() for side in ('r', 'l')}
SIGN = {'r': -1.0, 'l': 1.0}  # 리그 x: 오른쪽이 -x


def arm_spec(side, theta_deg, radius, x_out, fingers, curl, thumb, palm, pole, dz=0.0):
    """호 위 θ(도)의 손목 + 팔꿈치 극·손가락 방향·손바닥 방향(월드). x 성분은 왼팔에서 반전(오른손 기준으로 적는다)"""
    th = math.radians(theta_deg)
    sh = SHOULDER[side]
    s = SIGN[side]
    return {'wrist': [s * x_out, sh.y - radius * math.sin(th), sh.z - radius * math.cos(th) + dz],
            'pole': [s * -pole[0], pole[1], pole[2]],
            'fingers': [s * -fingers[0], fingers[1], fingers[2]],
            'palm': [s * -palm[0], palm[1], palm[2]],
            'hand_roll': 0.0, 'curl': curl, 'thumb': thumb}


def set_palms(spec):
    """apply_pose 뒤에 손 뼈를 손가락 축 둘레로 돌려 손바닥이 spec 의 palm 방향(월드)을 보게 한다. farmer_pose.aim 의 roll=0 은
    최소 회전이라 손이 아래를 향하면 손바닥 방향이 제멋대로다. 손바닥 방향은 축 가정 없이 실측한다 — 가운뎃손가락을 굽혔을 때
    둘째 마디가 움직이는 쪽(왼손은 뼈 축이 거울상이라 국부 축 부호를 믿을 수 없다)"""
    for side in ('r', 'l'):
        arm = spec.get('arm_' + side)
        if not arm or 'palm' not in arm:
            continue
        name = 'hand_' + side
        f = Vector(arm['fingers']).normalized()
        aim(name, f, 0.0)
        curl(side, 0.0, 0.0)
        straight = rig.pose.bones['middle_02_' + side].head.copy()
        curl(side, 1.0, 0.0)
        cur = rig.pose.bones['middle_02_' + side].head - straight
        want = Vector(arm['palm'])
        cur_p = (cur - f * cur.dot(f)).normalized()
        want_p = (want - f * want.dot(f)).normalized()
        roll = math.atan2(cur_p.cross(want_p).dot(f), cur_p.dot(want_p))
        aim(name, f, roll)
        curl(side, arm.get('curl', 0.0), arm.get('thumb'), arm.get('spread', 0.0))


def run_spec(u):
    """u∈[0,1] 한 주기. 오른팔 θ = mid + amp·sin(2πu), 왼팔은 반대 위상"""
    mid = (RUN_THETA_MIN + RUN_THETA_MAX) * 0.5
    amp = (RUN_THETA_MAX - RUN_THETA_MIN) * 0.5
    spec = {'spine': {}}
    for side, phase in (('r', 0.0), ('l', math.pi)):
        theta = mid + amp * math.sin(math.tau * u + phase)
        k = (theta - RUN_THETA_MIN) / (RUN_THETA_MAX - RUN_THETA_MIN)
        x_out = RUN_X_BACK + (RUN_X_FRONT - RUN_X_BACK) * k
        fingers = tuple(a + (b - a) * k for a, b in zip(RUN_FINGERS_BACK, RUN_FINGERS_FRONT))
        spec['arm_' + side] = arm_spec(side, theta, RUN_RADIUS, x_out, fingers, RUN_CURL, RUN_THUMB, RUN_PALM, RUN_POLE)
    return spec


def down_spec(u):
    """u∈[0,1] 한 주기. 두 팔 모두 호의 ARMS_DOWN_DEG, 손목 z ±DOWN_BREATH_CM 사인"""
    dz = DOWN_BREATH_CM * 0.01 * math.sin(math.tau * u)
    return {'spine': {}, **{'arm_' + side: arm_spec(side, ARMS_DOWN_DEG, ARMS_DOWN_RADIUS, ARMS_DOWN_X, ARMS_DOWN_FINGERS, ARMS_DOWN_CURL, ARMS_DOWN_THUMB,
                                                    ARMS_DOWN_PALM, ARMS_DOWN_POLE, dz) for side in ('r', 'l')}}


CLIPS = {
    'AN_Farmer_FPArmsDown': {'seconds': DOWN_SECONDS, 'spec': down_spec, 'preview_frames': (1, 16), 'tag': 'fparmsdown'},
    'AN_Farmer_FPRun': {'seconds': RUN_SECONDS, 'spec': run_spec, 'preview_frames': (1, 5, 10, 14), 'tag': 'fprun'},
}


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    for name, clip in CLIPS.items():
        end = round(clip['seconds'] * FPS) + 1          # 마지막 프레임 = 첫 프레임(루프)
        scene.frame_start, scene.frame_end = 1, end
        action = new_action(name) if EXPORT else None
        keyed = set()
        samples = {}
        for frame in range(1, end + 1):
            u = (frame - 1) / (end - 1)
            scene.frame_set(frame)
            spec = clip['spec'](u)
            apply_pose(spec)
            set_palms(spec)
            if EXPORT:
                key_current_pose(keyed, frame)
            if frame in clip['preview_frames']:
                render_fp(f"{clip['tag']}_f{frame}", PREVIEW_DIR)
                render_front(f"{clip['tag']}_f{frame}", PREVIEW_DIR)
                samples[f'f{frame}'] = {side: [round(v, 4) for v in rig.pose.bones['hand_' + side].head] for side in ('r', 'l')}
        results[name] = {'frames': end, 'seconds': clip['seconds'], 'wrist_samples': samples}
        if EXPORT:
            for fc in action_fcurves(action):
                for kp in fc.keyframe_points:
                    kp.interpolation = 'LINEAR'
            out = export_action(action)
            results[name].update({'fbx': str(out), 'sha256': hashlib.sha256(out.read_bytes()).hexdigest(), 'keyed_bones': len(keyed)})
    apply_pose({'spine': {}})
    rig.animation_data_clear() if rig.animation_data else None
    if EXPORT:
        manifest_path = ROOT / 'manifest.json'
        manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
        anims = manifest.setdefault('animations', {})
        anims['AN_Farmer_FPArmsDown'] = {'fbx_sha256': results['AN_Farmer_FPArmsDown']['sha256'], 'loop': True, 'seconds': DOWN_SECONDS, 'fps': FPS,
                                         'layer': 'upper body from spine_01', 'view': 'first-person arms',
                                         'pose': f'both arms down/out of frame (run arc theta={ARMS_DOWN_DEG}deg)', 'breath_cm': DOWN_BREATH_CM}
        anims['AN_Farmer_FPRun'] = {'fbx_sha256': results['AN_Farmer_FPRun']['sha256'], 'loop': True, 'seconds': RUN_SECONDS, 'fps': FPS,
                                    'reference_speed_cm_s': RUN_REFERENCE_SPEED, 'layer': 'upper body from spine_01', 'view': 'first-person arms',
                                    'pose': f'alternating arm swing on shoulder arc theta {RUN_THETA_MIN}..{RUN_THETA_MAX}deg, r={RUN_RADIUS}m'}
        manifest['fparms_keys'] = {'run': {'seconds': RUN_SECONDS, 'reference_speed_cm_s': RUN_REFERENCE_SPEED, 'radius_m': RUN_RADIUS,
                                           'theta_deg': [RUN_THETA_MIN, RUN_THETA_MAX], 'x_out_m': [RUN_X_BACK, RUN_X_FRONT],
                                           'fingers': [RUN_FINGERS_BACK, RUN_FINGERS_FRONT], 'palm': RUN_PALM, 'pole_r': RUN_POLE, 'curl': RUN_CURL,
                                           'thumb': RUN_THUMB},
                                   'down': {'seconds': DOWN_SECONDS, 'theta_deg': ARMS_DOWN_DEG, 'radius_m': ARMS_DOWN_RADIUS, 'x_out_m': ARMS_DOWN_X,
                                            'fingers': ARMS_DOWN_FINGERS, 'palm': ARMS_DOWN_PALM, 'pole_r': ARMS_DOWN_POLE, 'curl': ARMS_DOWN_CURL,
                                            'thumb': ARMS_DOWN_THUMB, 'breath_cm': DOWN_BREATH_CM},
                                   'shoulder_r_m': [round(v, 4) for v in SHOULDER['r']]}
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    print('FPARMS RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
