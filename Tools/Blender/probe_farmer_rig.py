"""농부 A 리그 실측 — 클립 스크립트를 쓰기 전에 장면 fps, 뼈 기준 위치, Small 들기·휘두르기 치켜든 자세의
손·머리 월드 위치, 손 뼈 축을 찍는다(animate_farmer_windup.py·animate_farmer_eat.py·measure_item_grip.py의 수치 근거).
`-- --fpcam` 이면 1인칭 미리보기 카메라 상태도 찍고 시험 렌더를 남긴다.

실행: py -3 Tools/Blender/run_headless.py probe_farmer_rig.py --blend ArtSource/character/farmer_a/farmer_a.blend [-- --fpcam]
"""
import bpy, sys, json, math
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
sys.path.insert(0, 'D:/Work/unreal/CL9/Tools/Blender')
from mathutils import Vector
from farmer_pose import ROOT, scene, rig, rest, apply_pose


def v3(v):
    return [round(x, 4) for x in v]


def axes(m):
    m3 = m.to_3x3()
    return {'x': v3(m3.col[0]), 'y': v3(m3.col[1]), 'z': v3(m3.col[2])}


out = {'fps': scene.render.fps, 'fps_base': scene.render.fps_base, 'scenes': [(s.name, s.render.fps) for s in bpy.data.scenes]}
for n in ('head', 'neck_01', 'spine_03', 'upperarm_r', 'lowerarm_r', 'hand_r', 'middle_metacarpal_r', 'middle_01_r', 'thumb_01_r', 'index_01_r', 'pinky_01_r'):
    if n in rest:
        out['rest_' + n] = v3(rest[n].translation)
out['rest_hand_r_axes'] = axes(rest['hand_r'])
out['grip_like_bones'] = [b.name for b in rig.data.bones if 'grip' in b.name.lower() or 'socket' in b.name.lower()]
out['grip_like_objects'] = [(o.name, o.parent_bone, v3(o.matrix_world.translation)) for o in bpy.data.objects if 'grip' in o.name.lower() or 'socket' in o.name.lower()]
spec = json.loads((ROOT.parent.parent.parent / 'Tools/Blender/farmer_hold_spec.json').read_text(encoding='utf-8'))
for view in ('tp', 'fp'):
    apply_pose(spec['Small'][view])
    hand = rig.pose.bones['hand_r']
    out['small_' + view] = {'hand_head': v3(hand.head), 'head_bone': v3(rig.pose.bones['head'].head), 'hand_axes': axes(hand.matrix),
                            'metacarpal': v3(rig.pose.bones['middle_metacarpal_r'].head), 'middle_01': v3(rig.pose.bones['middle_01_r'].head),
                            'thumb_01': v3(rig.pose.bones['thumb_01_r'].head), 'index_01': v3(rig.pose.bones['index_01_r'].head), 'pinky_01': v3(rig.pose.bones['pinky_01_r'].head)}
swing = json.loads((ROOT / 'manifest.json').read_text(encoding='utf-8'))['swing_keys']
for view in ('tp', 'fp'):
    apply_pose({'spine': {'spine_03': [0.04, 0.0, 0.0]} if view == 'tp' else {}, 'arm_r': swing[view][1][1]})
    out['windup_' + view] = {'hand_head': v3(rig.pose.bones['hand_r'].head), 'head_bone': v3(rig.pose.bones['head'].head)}

if '--fpcam' in sys.argv:
    preview = bpy.data.scenes['CL9_FARMER_A_PREVIEW']
    cam = preview.camera
    out['fpcam'] = {'camera': cam.name, 'parent': cam.parent.name if cam.parent else None, 'constraints': [c.type for c in cam.constraints],
                    'clip': [cam.data.clip_start, cam.data.clip_end], 'sensor_fit': cam.data.sensor_fit, 'shift': [cam.data.shift_x, cam.data.shift_y],
                    'objects': [(o.name, o.type, o.hide_render, o.hide_viewport) for o in preview.objects],
                    'body_modifiers': [(m.type, getattr(m, 'object', None) and m.object.name) for m in bpy.data.objects['SK_Farmer_A_Body'].modifiers],
                    'view_layers': [(vl.name, vl.use) for vl in preview.view_layers]}
    apply_pose(spec['Small']['fp'])
    from farmer_pose import render_fp
    render_fp('probe_small_fp', ROOT / 'preview')
    # 카메라를 머리 밖(앞 30cm)으로 빼서 같은 방향을 본다 — 근접 클립 문제인지 가른다
    if rig.name not in preview.objects:
        preview.collection.objects.link(rig)
    body = bpy.data.objects['SK_Farmer_A_Body']
    if body.name not in preview.objects:
        preview.collection.objects.link(body)
    bpy.context.window.scene = preview
    cam.data.lens_unit = 'FOV'
    cam.data.angle = math.radians(70)
    cam.data.clip_start = 0.01
    cam.location = (0.0, -0.30, 1.56)
    cam.rotation_euler = (Vector((0.0, -2.0, 1.2)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    preview.render.filepath = str(ROOT / 'preview' / 'probe_small_fp_outside.png')
    bpy.ops.render.render(write_still=True)
    bpy.context.window.scene = scene
print('PROBE ' + json.dumps(out))
