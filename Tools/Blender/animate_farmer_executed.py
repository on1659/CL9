"""농부 A 처형당함 클립 — 종별 2인 클립의 농부 짝. 2026-09-21 execute-clips(사용자 "몬스터 제작 목록에 공격 모션 — 처형과 공격은 같은 모션").

전신 2클립(Manny 스켈레톤, 리그 root 원점·전방 −Y = 액터 +X), 길이 = execute_anchor.DURATION(= 행 ExecuteDuration, 초안 2.0초):
- AN_Farmer_Executed_Pig — 정면(+100)에서 박치기. 0~0.40 선 채(카메라 블렌드 중) → 0.50 손을 앞으로 올려 막으려 함(BRACE) → 0.50 접촉 → 0.62 척추 뒤로
  젖혀지고 팔이 앞으로 튐(knockdown 의 HIT) → 뒤로(−X 로 머리) 넘어져 1.10 LAND → 1.40 DOWN → 2.0 까지 DOWN 유지. 끝 = `AN_Farmer_Down` 첫 프레임과
  같은 자세(같은 DOWN 키·숨쉬기 0) — 보통 죽음 시체와 같은 그림.
- AN_Farmer_Executed_Raccoon — 등 뒤(−120)에서 덮침. 0~0.55 선 채 → 0.60 접촉 → 0.72 척추 앞으로, 팔 뒤·옆, 무릎 꺾임(HIT_FRONT) → 앞으로(+X 로 머리)
  엎어져 1.00 FALL_FRONT → 1.25 PRONE → 2.0 까지 유지. **PRONE 신설**: 골반 피치 −π/2(엎드림), 얼굴 옆으로(head_yaw), 팔 앞·옆, 골반 z 는 몸 메시
  최저점으로 바닥 관통 보정(apply_full).
농부 골반 XY 는 항상 0(apply_full 관례 — 규칙 3, 데스캠이 캡슐 중심을 본다). 마지막 프레임은 부활까지 유지된다(ABP_Unarmed Executed 가지가
`IsExecuted` 동안 평가기를 붙들고 있다 — docs/systems/animation.md). 피해자 요는 처형 진입 때 서버가 맞춰 주므로 "정면"·"등 뒤"를 그대로 가정한다.
시각·자리는 몬스터 클립(animate_creature_reacts.py Execute)과 execute_anchor.py 앵커 표를 공유한다 — 두 리그는 각자 자기 blend 에서 저작(D7).

실행: py -3 Tools/Blender/run_headless.py animate_farmer_executed.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --export]
  --export: FBX(export/AN_Farmer_Executed_*.fbx)·manifest `animations`·`executed_keys`·blend 재저장. 없으면 미리보기(preview/executed_<종>_f*_front.png)만.
"""
import sys, json, hashlib, math
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import bpy
from farmer_pose import ROOT, scene, rig, reset, key_current_pose, new_action, action_fcurves, export_action, render_front
from animate_farmer_knockdown import apply_full, lerp_params, STAND, HIT, LAND, DOWN, LIE_Z, FPS, PREVIEW_DIR
import execute_anchor

EXPORT = '--export' in sys.argv
SPECIES = ('Pig', 'Raccoon')

# ---------- 자세 키 (animate_farmer_knockdown 의 파라미터 사전 — rad / m) ----------
# 막으려 함 — 두 손을 가슴 앞으로 반쯤 올리고 살짝 뒤로 기운다(정면에서 오는 돼지를 본 순간)
BRACE = {**STAND, 'spine': 0.05, 'head': 0.05, 'knee_l': 0.10, 'knee_r': 0.10,
         'arm_r': {'wrist': [-0.28, -0.40, 1.15], 'pole': [-1.0, -0.2, -0.3]}, 'arm_l': {'wrist': [0.28, -0.40, 1.15], 'pole': [1.0, -0.2, -0.3]}}
# 등 뒤에서 덮친 순간 — 척추가 앞으로 꺾이고 팔은 뒤·옆으로 튀며 무릎이 꺾여 골반이 내려간다
HIT_FRONT = {**STAND, 'pelvis_pitch': -0.15, 'pelvis_z': 0.80, 'spine': -0.40, 'head': 0.20, 'hip_l': 0.35, 'knee_l': 0.45, 'hip_r': 0.35, 'knee_r': 0.45,
             'arm_r': {'wrist': [-0.45, 0.25, 1.05], 'pole': [-1.0, 0.3, -0.2]}, 'arm_l': {'wrist': [0.45, 0.25, 1.05], 'pole': [1.0, 0.3, -0.2]}}
# 앞으로 엎어지는 중 — 골반 −1.0rad, 팔이 앞·아래로 내려온다(짚으려는 듯 — 죽어 가므로 힘없이)
FALL_FRONT = {**STAND, 'pelvis_pitch': -1.0, 'pelvis_z': 0.45, 'spine': -0.20, 'head': 0.20, 'hip_l': 0.30, 'knee_l': 0.40, 'hip_r': 0.30, 'knee_r': 0.40,
              'arm_r': {'wrist': [-0.40, -0.55, 0.35], 'pole': [-0.6, -0.3, 0.7]}, 'arm_l': {'wrist': [0.35, -0.65, 0.30], 'pole': [0.6, -0.3, 0.7]}}
# 엎드린 시체 — 골반 피치 −π/2, 얼굴 옆으로(오른뺨이 바닥), 오른팔 옆·왼팔 앞, 다리는 거의 펴짐. 골반 z 는 초안(바닥 관통 보정이 최종값)
PRONE = {**STAND, 'pelvis_pitch': -math.pi / 2, 'pelvis_z': LIE_Z, 'spine': 0.10, 'head': 0.15, 'head_yaw': 0.80,
         'hip_l': -0.05, 'knee_l': 0.05, 'hip_r': 0.05, 'knee_r': 0.15,
         'arm_r': {'wrist': [-0.55, -0.60, 0.05], 'pole': [-0.6, -0.3, 0.7]}, 'arm_l': {'wrist': [0.30, -0.90, 0.05], 'pole': [0.5, -0.2, 0.7]}}

DUR = execute_anchor.DURATION
_fp = execute_anchor.ANCHOR['Pig']['farmer']; _fr = execute_anchor.ANCHOR['Raccoon']['farmer']
CLIPS = {
    'AN_Farmer_Executed_Pig': {'seconds': DUR, 'loop': False, 'species': 'Pig', 'end': 'DOWN',
                               'keys': [(0.0, STAND), (_fp['brace'], STAND), (_fp['hit'], BRACE), (_fp['react'], HIT), (_fp['land'], LAND), (_fp['down'], DOWN), (DUR, DOWN)],
                               'ease': ['linear', 'out', 'out', 'in', 'out', 'linear']},
    'AN_Farmer_Executed_Raccoon': {'seconds': DUR, 'loop': False, 'species': 'Raccoon', 'end': 'PRONE',
                                   'keys': [(0.0, STAND), (_fr['hit'] - 0.05, STAND), (_fr['react'], HIT_FRONT), (_fr['fall'], FALL_FRONT), (_fr['prone'], PRONE), (DUR, PRONE)],
                                   'ease': ['linear', 'out', 'in', 'out', 'linear']},
}


def main():
    scene.render.fps, scene.render.fps_base = FPS, 1.0
    results = {}
    for name, clip in CLIPS.items():
        frames = round(clip['seconds'] * FPS) + 1
        scene.frame_start, scene.frame_end = 1, frames
        action = new_action(name) if EXPORT else None
        if rig.animation_data and not EXPORT:
            rig.animation_data.action = None
        preview_frames = {round(tt * FPS) + 1 for tt in execute_anchor.preview_times(clip['species'])}
        keyed = set(); lows = []; previews = []; pelvis = {}
        for frame in range(1, frames + 1):
            t = (frame - 1) / FPS
            p = lerp_params(clip['keys'], clip['ease'], t)
            scene.frame_set(frame)
            lows.append(round(apply_full(p, 0.0) * 100, 2))
            if frame in {1, round(execute_anchor.ANCHOR[clip['species']]['contact_s'] * FPS) + 1, frames}:
                head = rig.pose.bones['head'].head; pv = rig.pose.bones['pelvis'].head
                pelvis[f'f{frame}_t{t:.2f}'] = {'pelvis_cm': [round(v * 100, 1) for v in pv], 'head_cm': [round(v * 100, 1) for v in head]}
            if EXPORT:
                key_current_pose(keyed, frame, locations=('pelvis',))
            if frame in preview_frames:
                tag = f'executed_{clip["species"].lower()}_f{frame:02d}'
                render_front(tag, PREVIEW_DIR, cam_location=(-2.9, -2.7, 1.9), look_at=(0, 0, 0.8))   # 선 자세의 머리와 누운 자세가 다 들어오게
                previews.append(f'{tag}_front.png')
        results[name] = {'frames': frames, 'seconds': clip['seconds'], 'lowest_z_cm': min(lows), 'end_pose': clip['end'], 'preview': previews,
                         'samples': pelvis}
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
            clip = CLIPS[name]
            A = execute_anchor.ANCHOR[clip['species']]
            anims[name] = {'fbx_sha256': r['sha256'], 'loop': False, 'seconds': clip['seconds'], 'fps': FPS, 'frames': r['frames'],
                           'layer': 'full body incl. pelvis translation (Executed branch — held on the last frame until respawn)', 'view': 'full body',
                           'pelvis_location_keys': True, 'end_pose': clip['end'],
                           'execute_anchor': {'victim_space': 'origin = victim capsule centre on floor, +X victim forward (rig -Y), cm',
                                              'monster_clip': f'AN_{clip["species"]}_Execute', 'monster_xy_cm': list(A['monster_xy_cm']),
                                              'monster_yaw_deg': A['monster_yaw_deg'], 'contact_s': A['contact_s'], 'farmer_times': A['farmer']}}
        manifest['executed_keys'] = {'brace': BRACE, 'hit_front': HIT_FRONT, 'fall_front': FALL_FRONT, 'prone': PRONE}
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
        bpy.ops.wm.save_mainfile(filepath=str(ROOT / 'farmer_a.blend'))
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    print('EXECUTED RESULT ' + json.dumps(results))


if __name__ == '__main__':
    main()
