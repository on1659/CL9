"""농부 A 넘어짐 클립 — 잡혔을 때(OnDamage → KnockdownTime 1.5초)와 죽음(넘어짐 무기한)의 표현. 2026-09-15, 사용자 "피격 애님이 부족하다".

전신 3클립(Manny 스켈레톤, 리그 root 원점·전방 −Y):
- AN_Farmer_Knockdown 0.5초 원샷 — 맞아서 상체가 젖혀지며(0~0.12) 뒤로 넘어가(0.12~0.45, 중력처럼 가속) 등으로 떨어지고 살짝 튄다(0.45~0.5).
  마지막 프레임 = Down 의 첫 프레임.
- AN_Farmer_Down 2.0초 루프 — 등을 대고 누움(골반 z 0.12, 한쪽 무릎 세움, 팔은 바닥에 벌림, 머리 옆으로). 숨쉬기(가슴 ±) — 죽음이면 ABP 가 PlayRate 0.
- AN_Farmer_GetUp 0.6초 원샷 — 상체를 일으키고(0~0.3) 무릎을 당겨 일어선다(0.3~0.6). 첫 프레임 = Down, 마지막 = 기준 자세(A 포즈).
넘어짐 자세는 전신이라 골반 위치 키가 있다(farmer_pose.key_current_pose locations=('pelvis',)). 바닥 관통은 몸 메시 최저점으로 골반을 올려 잡는다.
1인칭 2클립(카메라 로컬, 눈 (0,0,1.56)·피치 0 — farmer_pose 머리말): AN_Farmer_FPKnockdown 0.5초(두 손이 얼굴 앞으로 확 올라왔다가 화면 아래로 빠짐),
AN_Farmer_FPGetUp 0.6초(두 손바닥이 화면 아래에서 밀어 올리듯 보였다가 내려감). 누운 동안은 기존 AN_Farmer_FPArmsDown(프레임 밖).
카메라 자체의 하강·기울기는 C++(넘어짐 경과 시각으로 카메라 상대 위치를 보간)이 맡는다 — docs/systems/animation.md "넘어짐 가지".

실행: py -3 Tools/Blender/run_headless.py animate_farmer_knockdown.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export]
"""
import sys, json, hashlib, math
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from mathutils import Vector, Matrix
from farmer_pose import (ROOT, scene, rig, rest, reset, arm_ik, world_base, align_palm, aim, curl, key_current_pose, new_action, action_fcurves,
                         export_action, render_front, render_fp, fp_pose_report, FP_EYE_HEIGHT_M)
from animate_farmer_swing import ease, lerp_keys

EXPORT = '--export' in sys.argv
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
FPS = 30
BODY = bpy.data.objects['SK_Farmer_A_Body']
PELVIS_REST_Z = rest['pelvis'].translation.z     # 0.959
LIE_Z = 0.12                                      # 누웠을 때 골반 높이(초안 — 바닥 관통 보정이 최종값을 정한다)

# ---------- 전신 자세 파라미터 (모두 rad / m) ----------
# pelvis_pitch: + 가 뒤로 젖힘(누우면 π/2). pelvis_z: 골반 높이. spine: 상체 추가 굽힘(+ 뒤로). head: 머리(+ 뒤로), head_yaw.
# hip_l/r, knee_l/r: 고관절 굽힘(+ 앞으로)·무릎 굽힘(+ 접힘). arms: 손목 목표(월드 m)·pole — 없으면 기준 자세.
STAND = {'pelvis_pitch': 0.0, 'pelvis_z': PELVIS_REST_Z, 'spine': 0.0, 'spine_yaw': 0.0, 'head': 0.0, 'head_yaw': 0.0,
         'hip_l': 0.0, 'knee_l': 0.0, 'hip_r': 0.0, 'knee_r': 0.0, 'arm_r': None, 'arm_l': None}
# 맞은 순간 — 상체 젖힘, 손이 앞으로 올라옴, 무릎 살짝
HIT = {**STAND, 'spine': 0.25, 'head': 0.15, 'knee_l': 0.15, 'knee_r': 0.15,
       'arm_r': {'wrist': [-0.30, -0.35, 1.30], 'pole': [-1.0, -0.2, -0.3]}, 'arm_l': {'wrist': [0.30, -0.35, 1.30], 'pole': [1.0, -0.2, -0.3]}}
# 등으로 떨어진 직후 — 팔 벌어짐, 무릎 굽음
LAND = {**STAND, 'pelvis_pitch': math.pi / 2, 'pelvis_z': LIE_Z, 'spine': -0.05, 'head': 0.10, 'hip_l': 0.35, 'knee_l': 0.5, 'hip_r': 0.2, 'knee_r': 0.3,
        'arm_r': {'wrist': [-0.55, 0.15, 0.12], 'pole': [-0.5, 0.6, 0.6]}, 'arm_l': {'wrist': [0.55, 0.15, 0.12], 'pole': [0.5, 0.6, 0.6]}}
# 누운 자세 — 왼 무릎 세움, 오른 다리 뻗음, 머리 옆으로
DOWN = {**LAND, 'spine': 0.0, 'head': 0.05, 'head_yaw': 0.35, 'hip_l': 0.9, 'knee_l': 1.5, 'hip_r': 0.1, 'knee_r': 0.15,
        'arm_r': {'wrist': [-0.62, 0.05, 0.10], 'pole': [-0.3, 0.8, 0.5]}, 'arm_l': {'wrist': [0.45, 0.30, 0.10], 'pole': [0.6, 0.6, 0.6]}}
# 상체 일으킴 — 골반 45°, 오른손으로 바닥 짚음, 왼 무릎 세움
SIT = {**STAND, 'pelvis_pitch': 0.6, 'pelvis_z': 0.16, 'spine': -0.45, 'head': -0.15, 'hip_l': 1.3, 'knee_l': 1.9, 'hip_r': 0.5, 'knee_r': 0.7,
       'arm_r': {'wrist': [-0.40, 0.30, 0.05], 'pole': [-0.8, 0.3, 0.4]}, 'arm_l': {'wrist': [0.25, -0.30, 0.55], 'pole': [0.8, -0.2, 0.2]}}
# 쭈그려 일어서는 중 — 골반 낮게, 무릎 깊이
CROUCH = {**STAND, 'pelvis_pitch': 0.15, 'pelvis_z': 0.55, 'spine': -0.35, 'head': -0.1, 'hip_l': 1.4, 'knee_l': 2.0, 'hip_r': 1.4, 'knee_r': 2.0,
          'arm_r': {'wrist': [-0.28, -0.35, 0.75], 'pole': [-0.8, 0.0, -0.4]}, 'arm_l': {'wrist': [0.28, -0.35, 0.75], 'pole': [0.8, 0.0, -0.4]}}

CLIPS = {
    'AN_Farmer_Knockdown': {'seconds': 0.5, 'loop': False, 'keys': [(0.0, STAND), (0.12, HIT), (0.45, LAND), (0.5, DOWN)], 'ease': ['out', 'in', 'out']},
    'AN_Farmer_Down': {'seconds': 2.0, 'loop': True, 'keys': [(0.0, DOWN), (2.0, DOWN)], 'ease': ['linear']},
    'AN_Farmer_GetUp': {'seconds': 0.6, 'loop': False, 'keys': [(0.0, DOWN), (0.3, SIT), (0.48, CROUCH), (0.6, STAND)], 'ease': ['inout', 'inout', 'out']},
}


def lerp_params(keys, eases, t):
    """딕셔너리 키 보간 — arm_* 는 손목·pole 을 각각 보간, 한쪽이 None 이면 다른 쪽 값"""
    for i, ((t0, a), (t1, b)) in enumerate(zip(keys, keys[1:])):
        if t0 <= t <= t1:
            u = 0.0 if t1 == t0 else (t - t0) / (t1 - t0)
            w = ease(eases[i], u)
            out = {}
            for k in a:
                if k.startswith('arm_'):
                    if a[k] is None or b[k] is None:
                        out[k] = a[k] if w < 0.5 else b[k]
                    else:
                        out[k] = {'wrist': [x + (y - x) * w for x, y in zip(a[k]['wrist'], b[k]['wrist'])],
                                  'pole': [x + (y - x) * w for x, y in zip(a[k]['pole'], b[k]['pole'])]}
                else:
                    out[k] = a[k] + (b[k] - a[k]) * w
            return out
    return dict(keys[-1][1])


def body_lowest_z():
    e = BODY.evaluated_get(bpy.context.evaluated_depsgraph_get())
    return min((e.matrix_world @ v.co).z for v in e.data.vertices)


def apply_full(p, breath=0.0):
    """전신 파라미터 → 포즈. 골반 회전·위치 → 척추·머리 → 다리 → 팔(IK). 마지막에 바닥 관통이면 골반을 올린다"""
    reset()
    pb = rig.pose.bones
    def rot(name, xyz):
        pb[name].rotation_mode = 'XYZ'; pb[name].rotation_euler = xyz
    # 축 실측(probe 2026-09-15): 척추·골반·목 뼈는 x 비틀기, y 옆 기울기, z 앞뒤(+ 뒤로). 넓적다리 +z = 앞으로 들기, 정강이 −z = 무릎 접힘
    rot('pelvis', (0.0, 0.0, p['pelvis_pitch']))
    for b in ('spine_01', 'spine_02', 'spine_03'):
        rot(b, (p['spine_yaw'] / 3, 0.0, (p['spine'] + breath) / 3))
    rot('neck_01', (p['head_yaw'] * 0.4, 0.0, p['head'] * 0.4)); rot('head', (p['head_yaw'] * 0.6, 0.0, p['head'] * 0.6))
    for side in ('l', 'r'):
        rot('thigh_' + side, (0.0, 0.0, p['hip_' + side])); rot('calf_' + side, (0.0, 0.0, -p['knee_' + side]))
    bpy.context.view_layer.update()
    mat = pb['pelvis'].matrix.copy(); mat.translation = Vector((0.0, rest['pelvis'].translation.y, p['pelvis_z'])); pb['pelvis'].matrix = mat
    bpy.context.view_layer.update()
    for side in ('r', 'l'):
        arm = p.get('arm_' + side)
        if arm:
            shoulder = world_base('upperarm_' + side).translation
            d = Vector(arm['wrist']) - shoulder
            wrist = shoulder + d.normalized() * min(d.length, 0.54)   # 골반이 돌아간 뒤 어깨 기준으로 팔 길이 안에
            arm_ik(side, wrist, arm['pole'])
            aim('hand_' + side, wrist - pb['lowerarm_' + side].head, 0.0)   # 손은 아래팔 방향 그대로(꺾지 않음)
            curl(side, 0.3)
    low = body_lowest_z()
    if low < 0.005:
        mat = pb['pelvis'].matrix.copy(); mat.translation.z += 0.005 - low; pb['pelvis'].matrix = mat
        bpy.context.view_layer.update()
    return body_lowest_z()


# ---------- 1인칭 ----------
FP_UP = {'wrist': [-0.16, -0.30, 1.42], 'pole': [-1.0, 0.0, -0.5], 'fingers': [0.0, -0.3, 0.95], 'palm': [0.2, 0.9, 0.3], 'curl': 0.2, 'thumb': 0.2}
FP_UP_L = {'wrist': [0.16, -0.30, 1.42], 'pole': [1.0, 0.0, -0.5], 'fingers': [0.0, -0.3, 0.95], 'palm': [-0.2, 0.9, 0.3], 'curl': 0.2, 'thumb': 0.2}
FP_OUT = {'wrist': [-0.30, -0.25, 1.05], 'pole': [-1.0, 0.0, -0.5], 'fingers': [0.0, -0.5, -0.85], 'palm': [0.5, 0.8, 0.0], 'curl': 0.2, 'thumb': 0.2}
FP_OUT_L = {'wrist': [0.30, -0.25, 1.05], 'pole': [1.0, 0.0, -0.5], 'fingers': [0.0, -0.5, -0.85], 'palm': [-0.5, 0.8, 0.0], 'curl': 0.2, 'thumb': 0.2}
FP_PUSH = {'wrist': [-0.18, -0.38, 1.30], 'pole': [-1.0, 0.0, -0.6], 'fingers': [0.1, -0.7, -0.7], 'palm': [0.0, -0.5, -0.85], 'curl': 0.15, 'thumb': 0.2}
FP_PUSH_L = {'wrist': [0.18, -0.38, 1.30], 'pole': [1.0, 0.0, -0.6], 'fingers': [-0.1, -0.7, -0.7], 'palm': [0.0, -0.5, -0.85], 'curl': 0.15, 'thumb': 0.2}
FP_CLIPS = {
    'AN_Farmer_FPKnockdown': {'seconds': 0.5, 'loop': False, 'ease': ['out', 'inout', 'in'],
                              'keys': [(0.0, (FP_OUT, FP_OUT_L)), (0.12, (FP_UP, FP_UP_L)), (0.3, (FP_UP, FP_UP_L)), (0.5, (FP_OUT, FP_OUT_L))]},
    'AN_Farmer_FPGetUp': {'seconds': 0.6, 'loop': False, 'ease': ['out', 'inout', 'in'],
                          'keys': [(0.0, (FP_OUT, FP_OUT_L)), (0.15, (FP_PUSH, FP_PUSH_L)), (0.4, (FP_PUSH, FP_PUSH_L)), (0.6, (FP_OUT, FP_OUT_L))]},
}


def apply_fp(spec_pair):
    reset()
    for side, spec in (('r', spec_pair[0]), ('l', spec_pair[1])):
        arm_ik(side, spec['wrist'], spec['pole'])
        align_palm(side, spec)
        curl(side, spec['curl'], spec['thumb'])


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    for name, clip in CLIPS.items():
        frames = round(clip['seconds'] * FPS) + 1
        scene.frame_start, scene.frame_end = 1, frames
        action = new_action(name) if EXPORT else None
        if rig.animation_data and not EXPORT:
            rig.animation_data.action = None
        keyed = set(); lows = []; previews = []
        for frame in range(1, frames + 1):
            t = (frame - 1) / FPS
            p = lerp_params(clip['keys'], clip['ease'], t)
            breath = 0.03 * math.sin(math.tau * t / clip['seconds']) if clip['loop'] else 0.0
            scene.frame_set(frame)
            lows.append(round(apply_full(p, breath) * 100, 2))
            if EXPORT:
                key_current_pose(keyed, frame, locations=('pelvis',))
            if frame in {1, *range(1, frames + 1, max(1, (frames - 1) // 6)), frames}:
                tag = f'{name.replace("AN_Farmer_", "").lower()}_f{frame:02d}'
                render_front(tag, PREVIEW_DIR, cam_location=(-2.4, -2.2, 1.6), look_at=(0, 0, 0.6))
                previews.append(f'{tag}_front.png')
        results[name] = {'frames': frames, 'seconds': clip['seconds'], 'lowest_z_cm': min(lows), 'preview': previews}
        if EXPORT:
            for fc in action_fcurves(action):
                for kp in fc.keyframe_points:
                    kp.interpolation = 'LINEAR'
            out = export_action(action)
            results[name].update({'fbx': str(out), 'sha256': hashlib.sha256(out.read_bytes()).hexdigest(), 'keyed_bones': len(keyed)})
    for name, clip in FP_CLIPS.items():
        frames = round(clip['seconds'] * FPS) + 1
        scene.frame_start, scene.frame_end = 1, frames
        action = new_action(name) if EXPORT else None
        if rig.animation_data and not EXPORT:
            rig.animation_data.action = None
        keyed = set(); previews = []; samples = {}
        for frame in range(1, frames + 1):
            t = (frame - 1) / FPS
            r_spec = lerp_keys([(k, v[0]) for k, v in clip['keys']], t, clip['ease'])
            l_spec = lerp_keys([(k, v[1]) for k, v in clip['keys']], t, clip['ease'])
            scene.frame_set(frame)
            apply_fp((r_spec, l_spec))
            if EXPORT:
                key_current_pose(keyed, frame)
            if frame in {1, *range(1, frames + 1, max(1, (frames - 1) // 6)), frames}:
                tag = f'{name.replace("AN_Farmer_", "").lower()}_f{frame:02d}'
                render_fp(tag, PREVIEW_DIR)
                previews.append(f'{tag}_fp.png'); samples[f'f{frame}'] = fp_pose_report('r')['uv']
        results[name] = {'frames': frames, 'seconds': clip['seconds'], 'preview': previews, 'uv_r': samples}
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
        for name, r in results.items():
            clip = CLIPS.get(name) or FP_CLIPS[name]
            entry = {'fbx_sha256': r['sha256'], 'loop': clip['loop'], 'seconds': clip['seconds'], 'fps': FPS, 'frames': r['frames']}
            if name in FP_CLIPS:
                entry.update({'space': 'camera', 'eye_height_m': FP_EYE_HEIGHT_M, 'layer': 'full rig (camera-local view model, spine identity)',
                              'view': 'first-person arms'})
            else:
                entry.update({'layer': 'full body incl. pelvis translation (replaces locomotion + foot IK while knocked down)', 'view': 'full body',
                              'pelvis_location_keys': True})
            anims[name] = entry
        manifest['knockdown_keys'] = {'stand': STAND, 'hit': HIT, 'land': LAND, 'down': DOWN, 'sit': SIT, 'crouch': CROUCH}
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    print('KNOCKDOWN RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
