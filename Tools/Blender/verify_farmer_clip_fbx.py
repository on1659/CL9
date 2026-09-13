"""농부 A 클립 FBX 검증 — export/<clip>.fbx 를 빈 장면에 다시 읽어 fps·프레임 범위·길이(초)와, 루프 클립이면 첫/끝 프레임의
손 자세 차이를 찍는다. UE 임포트 길이(number_of_sampled_frames / 초) 예측용. 검증할 클립 이름은 `--` 뒤 인자, 없으면 이번 4개
(029 fp-arms-flashlight).

실행: py -3 Tools/Blender/run_headless.py verify_farmer_clip_fbx.py [-- AN_Farmer_SwingWindup ...]
"""
import bpy, sys, json, hashlib
from pathlib import Path
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
ROOT = Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a')
DEFAULT = ['AN_Farmer_FPArmsDown', 'AN_Farmer_FPRun', 'AN_Farmer_FPHoldFlashlight', 'AN_Farmer_HoldFlashlight']
names = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv and len(sys.argv) > sys.argv.index('--') + 1 else DEFAULT
manifest = json.loads((ROOT / 'manifest.json').read_text(encoding='utf-8'))['animations']
out = {}
for name in names:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    f = ROOT / 'export' / (name + '.fbx')
    bpy.ops.import_scene.fbx(filepath=str(f), use_anim=True, ignore_leaf_bones=True, automatic_bone_orientation=False)
    scene = bpy.context.scene
    rig = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
    action = rig.animation_data.action
    fr = action.frame_range
    frames = int(round(fr[1] - fr[0])) + 1
    seconds = (frames - 1) / scene.render.fps
    info = {'fps': scene.render.fps, 'frame_range': [fr[0], fr[1]], 'frames': frames, 'seconds': round(seconds, 4),
            'manifest_seconds': manifest.get(name, {}).get('seconds'), 'sha_ok': hashlib.sha256(f.read_bytes()).hexdigest() == manifest.get(name, {}).get('fbx_sha256'),
            'bones': len(rig.data.bones)}
    if manifest.get(name, {}).get('loop'):
        scene.frame_set(int(fr[0]))
        a = rig.pose.bones['hand_r'].matrix.copy()
        scene.frame_set(int(fr[1]))
        b = rig.pose.bones['hand_r'].matrix.copy()
        info['loop_seam_cm'] = round((a.translation - b.translation).length, 4)
    out[name] = info
print('VERIFY ' + json.dumps(out))
