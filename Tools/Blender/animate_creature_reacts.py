"""동물 5종 반응 클립 재작업 — Catch(공격)·HitReact(피격)·PushReact(밀침, 신규). 2026-09-15, 사용자 "피격·공격 애님이 부족하다".

원본 8클립을 만든 animate_<종>_a.py 는 건드리지 않는다(로코모션·Death 는 그대로). 이 스크립트는 종 blend 를 열어 세 클립만 다시 굽고,
그 FBX·manifest `clips` 항목·미리보기만 갱신한 뒤 blend 를 전 액션과 함께 다시 저장한다. 축·헬퍼(relative/segment/leg/limb)는 종 원본
스크립트의 것을 그대로 옮겨 수치 관행이 같다(새 +X 전방, 몸 위치는 body 뼈 이동, 다리는 발 고정 IK).

시간 규칙(docs/systems/animation.md 규칙 2 — 판정은 코드, 애님은 뒤따른다):
- HitReact 0.3초 = HitStunMs 300. 맞는 순간 확 튀고(첫 0.05초) 나머지 동안 천천히 돌아온다 — 옛 sin 펄스 하나는 맞는 느낌이 없었다.
- PushReact 0.8초 = PushStun 0.8. 코드가 이 시간 동안 액터를 뒤로 미끄러뜨리므로, 몸이 젖혀지며 붕 떴다가(0~0.15) 발을 허우적거리며
  미끄러지고(0.15~0.55) 휘청이며 선다(0.55~0.8). 전용 클립이 없던 0.3초 HitReact 뒤 0.5초를 선 자세로 밀리던 것을 대체한다.
- Catch 0.8초 = 코드의 CatchPauseSeconds. 판정(OnDamage)은 상태 진입 순간이라 예비동작을 보여 줄 틈이 없다 — 첫 0.1초에 곧장 덮치고
  (0.1~0.3 유지), 종별 2차 동작(닭 연속 쪼기·돼지/양 머리 흔들기·너구리 반대 앞발 한 번 더) 뒤 0.8초까지 원래 자세로 돌아온다.
  거위는 잡지 않는 종(CatchRadius 0)이라 Catch 를 만들지 않는다.

실행: py -3 Tools/Blender/run_headless.py animate_creature_reacts.py --blend ArtSource/monster/<종>_a/<종>_a.blend -- --species Chicken [--preview-only]
  --species: Chicken | Goose | Pig | Sheep | Raccoon.  --preview-only: FBX·manifest·blend 를 쓰지 않고 미리보기만.
"""
import bpy, math, json, sys, hashlib
from pathlib import Path
from mathutils import Vector, Matrix, Euler

argv = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else []
SPECIES = argv[argv.index('--species') + 1] if '--species' in argv else globals().get('SPECIES', 'Chicken')
PREVIEW_ONLY = '--preview-only' in argv
FAMILY = {'Chicken': 'chicken', 'Goose': 'goose', 'Pig': 'quad', 'Sheep': 'quad', 'Raccoon': 'quad'}[SPECIES]
ROOT = Path('D:/Work/unreal/CL9/ArtSource/monster') / (SPECIES.lower() + '_a')
s = bpy.data.scenes['CL9_' + SPECIES.upper() + '_A']
bpy.context.window.scene = s
r = bpy.data.objects['SKEL_' + SPECIES + '_A']
m = bpy.data.objects['SK_' + SPECIES + '_A']
r.animation_data_create()
rest = {b.name: b.matrix_local.copy() for b in r.data.bones}
FPS = 30
CLIPS = {'HitReact': 0.3, 'PushReact': 0.8}
if SPECIES != 'Goose':
    CLIPS['Catch'] = 0.8
PREVIEW_DIR = ROOT / 'preview'
PREVIEW_DIR.mkdir(exist_ok=True)


# ---------- 시간 곡선 ----------
def smooth(x):
    x = min(1.0, max(0.0, x))
    return x * x * (3 - 2 * x)


def ease_out(x):
    x = min(1.0, max(0.0, x))
    return 1 - (1 - x) ** 3


def rise_hold_fall(u, up, hold, down):
    """0→1 (0~up, 급하게), 1 유지 (~hold), 1→0 (hold~down, 부드럽게), 그 뒤 0"""
    if u < up:
        return ease_out(u / up)
    if u < hold:
        return 1.0
    return 1.0 - smooth((u - hold) / max(1e-6, down - hold))


def bump(u, a, b):
    """a~b 사이에서 sin² 혹 하나"""
    if u <= a or u >= b:
        return 0.0
    return math.sin(math.pi * (u - a) / (b - a)) ** 2


def snap(u, peak=0.15):
    """맞는 순간: peak 까지 직선으로 1, 그 뒤 smoothstep 으로 0"""
    if u < peak:
        return u / peak
    return 1.0 - smooth((u - peak) / (1 - peak))


def wobble(u, a, b, cycles, decay=True):
    """a~b 동안 cycles 번 흔들리는 사인(끝으로 갈수록 잦아든다)"""
    if u <= a or u >= b:
        return 0.0
    x = (u - a) / (b - a)
    return math.sin(math.tau * cycles * x) * ((1 - x) if decay else 1.0)


# ---------- 리그 헬퍼 (종 원본 스크립트와 동일) ----------
if FAMILY == 'chicken':
    def transform_at(head, rotation, rest_rotation):
        return Matrix.Translation(head) @ Euler(rotation, 'XYZ').to_matrix().to_4x4() @ rest_rotation.to_matrix().to_4x4()

    def relative(name, rotation=(0, 0, 0)):
        pb = r.pose.bones[name]
        base = pb.parent.matrix @ rest[pb.parent.name].inverted() @ rest[name] if pb.parent else rest[name]
        pb.matrix = base @ Euler(rotation, 'XYZ').to_matrix().to_4x4()
        bpy.context.view_layer.update()
else:
    def relative(name, rotation=(0, 0, 0)):
        p = r.pose.bones[name]
        base = p.parent.matrix @ rest[p.parent.name].inverted() @ rest[name] if p.parent else rest[name]
        q = rest[name].to_quaternion().to_matrix().to_4x4()
        p.matrix = base @ q.inverted() @ Euler(rotation, 'XYZ').to_matrix().to_4x4() @ q
        bpy.context.view_layer.update()


def segment(name, head, tail):
    b = r.data.bones[name]
    q = (b.tail_local - b.head_local).rotation_difference(Vector(tail) - Vector(head)) @ rest[name].to_quaternion()
    r.pose.bones[name].matrix = Matrix.Translation(head) @ q.to_matrix().to_4x4()
    bpy.context.view_layer.update()


def bird_leg(side, foot, upper='thigh_', lower='shin_'):
    hip = r.pose.bones['body'].matrix @ rest['body'].inverted() @ r.data.bones[upper + side].head_local
    l1 = r.data.bones[upper + side].length
    l2 = r.data.bones[lower + side].length
    d = (foot - hip).length
    if d > l1 + l2 - 1e-4:   # 몸이 붕 떠서 발이 안 닿으면 닿는 데까지만 — 나머지는 다리를 편 채 뜬다
        foot = hip + (foot - hip).normalized() * (l1 + l2 - 1e-4)
        d = (foot - hip).length
    axis = (foot - hip).normalized()
    a = (l1 * l1 - l2 * l2 + d * d) / (2 * d)
    perp = (Vector((1, 0, 0)) - axis * axis.x).normalized()
    knee = hip + axis * a + perp * math.sqrt(max(0, l1 * l1 - a * a))
    segment(upper + side, hip, knee)
    segment(lower + side, knee, foot)
    r.pose.bones['foot_' + side].matrix = Matrix.Translation(foot) @ rest['foot_' + side].to_quaternion().to_matrix().to_4x4()
    bpy.context.view_layer.update()


def limb(tag, paw):
    upper, lower, foot = tag + '_upper', tag + '_lower', tag + '_paw'
    parent = r.pose.bones[upper].parent
    hip = parent.matrix @ rest[parent.name].inverted() @ r.data.bones[upper].head_local
    l1 = r.data.bones[upper].length
    l2 = r.data.bones[lower].length
    dist = (paw - hip).length
    if dist > l1 + l2 - 1e-4:
        paw = hip + (paw - hip).normalized() * (l1 + l2 - 1e-4)
        dist = (paw - hip).length
    direction = (paw - hip).normalized()
    a = (l1 * l1 - l2 * l2 + dist * dist) / (2 * dist)
    preferred = Vector((1 if tag.startswith('front') else -1, 0, 0))
    bend = (preferred - direction * direction.dot(preferred)).normalized()
    knee = hip + direction * a + bend * math.sqrt(max(0, l1 * l1 - a * a))
    segment(upper, hip, knee)
    segment(lower, knee, paw)
    r.pose.bones[foot].matrix = Matrix.Translation(paw) @ rest[foot].to_quaternion().to_matrix().to_4x4()
    bpy.context.view_layer.update()


def lowest_z():
    e = m.evaluated_get(bpy.context.evaluated_depsgraph_get())
    return min((e.matrix_world @ v.co).z for v in e.data.vertices)


# ---------- 종별 자세 ----------
def pose_chicken(name, u, phase):
    """닭: body 위치/회전, neck·head·jaw·comb·tail·wing, 발 고정 IK. 기본 = 원본 Idle 의 정지 자세"""
    body_pos = Vector((-.06, 0, .55)); body_rot = [0, 0, 0]; neck_rot = [0, 0, 0]; head_rot = [0, 0, 0]; wing = 0; jaw = 0
    feet = {'l': Vector((-.055, .145, .065)), 'r': Vector((-.055, -.145, .065))}
    if name == 'HitReact':
        k = snap(u, 0.15)
        body_pos.x -= .10 * k; body_pos.z -= .03 * k
        body_rot = [.16 * k, -.28 * k, .0]                  # 옆으로 기우뚱 + 머리 쪽이 들림
        neck_rot = [0, -.25 * k, .15 * k]
        head_rot = [0, -.45 * k, .25 * wobble(u, .1, 1, 2)]   # 젖혀진 머리가 두어 번 흔들린다
        wing = 1.3 * k; jaw = .35 * k
    elif name == 'PushReact':
        lift = rise_hold_fall(u, .12, .18, .55)             # 젖혀져 붕 뜸
        low = bump(u, .35, .95)                             # 착지해 웅크림
        env = bump(u, .10, .70)                             # 허우적거림
        body_pos.x -= .16 * lift; body_pos.z += .10 * lift - .06 * low
        body_rot = [.10 * lift, -.62 * lift + .18 * low, .06 * wobble(u, .55, 1, 1.5)]
        neck_rot = [0, -.35 * lift + .10 * low, 0]
        head_rot = [0, -.30 * lift + .20 * low, .30 * wobble(u, .5, 1, 2)]
        wing = 1.1 * lift + 1.0 * env * abs(math.sin(math.tau * 3 * u)); jaw = .4 * lift + .15 * low
        for sign, side in ((1, 'l'), (-1, 'r')):
            kick = env * math.sin(math.tau * 4 * u + (0 if side == 'l' else math.pi))
            feet[side] = Vector((-.055 + .09 * kick, sign * .145, .065 + .06 * max(0, kick) * env))
    elif name == 'Catch':
        lunge = rise_hold_fall(u, .10, .30, .80)
        peck = abs(math.sin(math.pi * (u - .16) / .13)) * bump(u, .16, .55) if .16 < u < .55 else 0   # 연속 쪼기 3번
        body_pos.x += .20 * lunge; body_pos.z -= .04 * lunge
        body_rot = [0, .15 * lunge, 0]
        neck_rot = [0, .12 * lunge, 0]
        head_rot = [0, .30 * lunge + .30 * peck, 0]
        wing = .5 * lunge; jaw = .32 * lunge + .25 * peck
        feet = {'l': Vector((-.02, .145, .065)), 'r': Vector((-.02, -.145, .065))}   # 앞으로 내디딘 채 버틴다
    r.pose.bones['root'].matrix = rest['root']
    r.pose.bones['body'].matrix = transform_at(body_pos, body_rot, rest['body'].to_quaternion())
    bpy.context.view_layer.update()
    relative('neck', neck_rot); relative('head', head_rot); relative('jaw', (-jaw, 0, 0))
    relative('comb', (.04 * math.sin(phase), 0, 0)); relative('tail', (0, .06 * math.sin(phase), 0))
    for sign, side in [(1, 'l'), (-1, 'r')]:
        relative('wing_' + side, (sign * wing, 0, sign * .1 * wing)); relative('wing_tip_' + side, (sign * .3 * wing, 0, 0))
        bird_leg(side, feet[side])


def pose_goose(name, u, phase):
    pos = Vector((-.16, 0, .58)); rot = [0, 0, 0]; neck = [0, 0, 0]; yaw = 0; head = 0; jaw = 0; wing = 0
    feet = {'l': Vector((-.08, .17, .06)), 'r': Vector((-.08, -.17, .06))}
    if name == 'HitReact':
        k = snap(u, 0.15)
        pos.x -= .09 * k; pos.z -= .03 * k
        rot = [.10 * k, -.12 * k, 0]
        neck = [-.45 * k, .25 * k, -.35 * k]            # 목이 S 자로 움츠러든다
        head = -.15 * k; yaw = .3 * wobble(u, .1, 1, 2)
        wing = .8 * k; jaw = .45 * k
    elif name == 'PushReact':
        lift = rise_hold_fall(u, .12, .18, .55); low = bump(u, .35, .95); env = bump(u, .10, .70)
        pos.x -= .16 * lift; pos.z += .10 * lift - .06 * low
        rot = [.08 * lift, -.50 * lift + .15 * low, .06 * wobble(u, .55, 1, 1.5)]
        neck = [-.35 * lift + .15 * low, .30 * lift, -.30 * lift + .10 * low]
        head = -.10 * lift + .15 * low; yaw = .3 * wobble(u, .5, 1, 2)
        wing = .85 * lift + .7 * env * abs(math.sin(math.tau * 3 * u)); jaw = .45 * lift + .1 * low
        for sign, side in ((1, 'l'), (-1, 'r')):
            kick = env * math.sin(math.tau * 4 * u + (0 if side == 'l' else math.pi))
            feet[side] = Vector((-.08 + .10 * kick, sign * .17, .06 + .06 * max(0, kick) * env))
    r.pose.bones['root'].matrix = rest['root']
    r.pose.bones['body'].matrix = Matrix.Translation(pos) @ Euler(rot, 'XYZ').to_matrix().to_4x4() @ rest['body'].to_quaternion().to_matrix().to_4x4()
    bpy.context.view_layer.update()
    for i, pitch in enumerate(neck):
        relative(f'neck_{i + 1:02d}', (0, pitch, yaw * .25 if i == 1 else 0))
    relative('head', (0, head, yaw * .75)); relative('jaw', (0, jaw, 0)); relative('tail', (0, .02 * math.sin(phase), 0))
    for side in ('l', 'r'):
        if 'eye_' + side in r.pose.bones:
            relative('eye_' + side); r.pose.bones['eye_' + side].scale.y = 1
    bpy.context.view_layer.update()
    for sign, side in [(1, 'l'), (-1, 'r')]:
        relative('wing_' + side, (-sign * wing, 0, 0)); relative('wing_tip_' + side, (0, .1 * wing, 0))
        bird_leg(side, feet[side])


def pose_quad(name, u, phase):
    pos = Vector((-.30, 0, .53)); rotation = [0, 0, 0]; chest = [0, 0, 0]; neck = [0, 0, 0]; head = [0, 0, 0]; jaw = 0
    ear = 0; tail_up = 0
    paw_offset = {}; raised = {}   # tag → (x, y, z) 발 위치 오프셋 / tag → (upper, lower) 들어 올린 앞발 회전
    swipe = SPECIES == 'Raccoon'
    if name == 'HitReact':
        k = snap(u, 0.15)
        pos.x -= .07 * k; pos.z -= .03 * k
        rotation = [.14 * k, -.12 * k, 0]
        chest = [0, -.10 * k, 0]; neck = [0, -.18 * k, .20 * k]
        head = [0, -.30 * k, .30 * k + .25 * wobble(u, .1, 1, 2)]
        jaw = .30 * k; ear = -.5 * k; tail_up = .4 * k
    elif name == 'PushReact':
        lift = rise_hold_fall(u, .12, .18, .55); low = bump(u, .35, .95); env = bump(u, .10, .70)
        pos.x -= .16 * lift; pos.z += .10 * lift - .07 * low
        rotation = [.10 * lift, -.42 * lift + .16 * low, .06 * wobble(u, .55, 1, 1.5)]
        chest = [0, -.10 * lift, 0]; neck = [0, -.20 * lift + .10 * low, 0]
        head = [0, -.25 * lift + .18 * low, .35 * wobble(u, .5, 1, 2)]
        jaw = .35 * lift + .1 * low; ear = -.6 * lift; tail_up = .5 * lift
        for tag, ph in (('front_l', 0), ('front_r', math.pi), ('hind_l', math.pi), ('hind_r', 0)):
            kick = env * math.sin(math.tau * 4 * u + ph)
            paw_offset[tag] = (.09 * kick, 0, .07 * max(0, kick) * env)
    elif name == 'Catch' and swipe:
        rear = rise_hold_fall(u, .10, .45, .80)                     # 뒷발로 서듯 앞이 들린다
        swipe_a = bump(u, .05, .30); swipe_b = bump(u, .28, .55)    # 오른 앞발, 그다음 왼 앞발
        pos.x += .10 * rear; pos.z += .08 * rear
        rotation = [0, -.40 * rear, 0]
        chest = [0, -.10 * rear, 0]; neck = [0, .20 * rear, 0]; head = [0, .30 * rear, .12 * (swipe_a - swipe_b)]
        jaw = .45 * rear; ear = -.4 * rear; tail_up = .3 * rear
        raised['front_r'] = (-.9 * rear + 1.3 * swipe_a, .8 * rear - 1.0 * swipe_a)   # 접었던 앞발을 앞으로 뻗어 내리친다
        raised['front_l'] = (-.9 * rear + 1.3 * swipe_b, .8 * rear - 1.0 * swipe_b)
    elif name == 'Catch':
        lunge = rise_hold_fall(u, .10, .30, .80)
        toss = wobble(u, .15, .55, 1.5)                             # 머리를 좌우로 흔들며 밀어붙인다
        pos.x += .20 * lunge; pos.z -= .03 * lunge
        rotation = [0, .08 * lunge, 0]
        chest = [0, .08 * lunge, 0]; neck = [0, .10 * lunge, .10 * toss]; head = [.15 * toss, .12 * lunge, .25 * toss]   # 코를 앞으로 들이민다(땅을 파는 게 아니라)
        jaw = .12 * lunge; ear = -.3 * lunge; tail_up = .2 * lunge
        for tag in ('front_l', 'front_r'):
            paw_offset[tag] = (.05 * lunge, 0, 0)
    r.pose.bones['root'].matrix = rest['root']
    r.pose.bones['body'].matrix = Matrix.Translation(pos) @ Euler(rotation, 'XYZ').to_matrix().to_4x4() @ rest['body'].to_quaternion().to_matrix().to_4x4()
    bpy.context.view_layer.update()
    relative('chest', chest); relative('neck', neck); relative('head', head); relative('jaw', (0, jaw, 0))
    for sign, side in [(1, 'l'), (-1, 'r')]:
        relative('ear_' + side, (sign * .03 * math.sin(phase) + sign * .0, ear, 0))
    for i in range(4):
        relative(f'tail_{i + 1:02d}', (0, .015 * math.sin(phase + i * .5) - tail_up * .25, .05 * math.sin(phase + i * .4)))
    for pair in ('front', 'hind'):
        for sign, side in [(1, 'l'), (-1, 'r')]:
            tag = pair + '_' + side
            if tag in raised:
                up, lo = raised[tag]
                relative(tag + '_upper', (0, up, 0)); relative(tag + '_lower', (0, lo, 0)); relative(tag + '_paw')
                continue
            base = r.data.bones[tag + '_paw'].head_local.copy()
            off = paw_offset.get(tag, (0, 0, 0))
            limb(tag, Vector((base.x + off[0], base.y + off[1], base.z + off[2])))


POSE = {'chicken': pose_chicken, 'goose': pose_goose, 'quad': pose_quad}[FAMILY]


# ---------- 미리보기 (Workbench, 종 원본 카메라 구도) ----------
CAM = {'Chicken': ((2.8, -3.5, 2.0), (0, 0, .65), 1.95), 'Goose': ((2.6, -3.8, 2.1), (0, 0, .82), 2.7),
       'Raccoon': ((2.3, -3.8, 2), (-.1, 0, .5), 2.5), 'Pig': ((2.3, -3.8, 2), (-.1, 0, .5), 2.5), 'Sheep': ((2.3, -3.8, 2), (-.1, 0, .5), 2.5)}[SPECIES]


def setup_preview():
    s.render.engine = 'BLENDER_WORKBENCH'
    s.display.shading.light = 'STUDIO'; s.display.shading.color_type = 'MATERIAL'
    s.render.resolution_x, s.render.resolution_y = 600, 600; s.render.resolution_percentage = 100
    s.render.image_settings.file_format = 'PNG'
    name = 'CL9_REACT_Camera'
    cam = bpy.data.objects.get(name)
    if cam is None:
        cam = bpy.data.objects.new(name, bpy.data.cameras.new(name)); s.collection.objects.link(cam)
    cam.location = CAM[0]; cam.data.type = 'ORTHO'; cam.data.ortho_scale = CAM[2]
    cam.rotation_euler = (Vector(CAM[1]) - Vector(cam.location)).to_track_quat('-Z', 'Y').to_euler()
    s.camera = cam
    return cam


def teardown_preview(cam):
    bpy.data.objects.remove(cam, do_unlink=True)


# ---------- 본문 ----------
def main():
    s.render.fps, s.render.fps_base = FPS, 1.0
    cam = setup_preview()
    manifest = json.loads((ROOT / 'export/manifest.json').read_text())
    result = {}
    for name, duration in CLIPS.items():
        action_name = f'AN_{SPECIES}_{name}'
        old = bpy.data.actions.get(action_name)
        if old is not None:
            bpy.data.actions.remove(old)
        action = bpy.data.actions.new(action_name); action.use_fake_user = True; r.animation_data.action = action
        end = round(duration * FPS) + 1
        s.frame_start, s.frame_end = 1, end
        lows = []; preview = []
        for frame in range(1, end + 1):
            s.frame_set(frame)
            u = (frame - 1) / (end - 1); phase = math.tau * u
            for p in r.pose.bones:
                p.matrix_basis = Matrix.Identity(4)
            POSE(name, u, phase)
            bpy.context.view_layer.update()
            low = lowest_z()
            if low < .005:   # 바닥 관통 — 몸을 그만큼 올린다(Death 보정과 같은 방식)
                mat = r.pose.bones['body'].matrix.copy(); mat.translation.z += .005 - low; r.pose.bones['body'].matrix = mat
                bpy.context.view_layer.update(); low = lowest_z()
            lows.append(round(low * 100, 2))
            for p in r.pose.bones:
                for channel in ('location', 'rotation_quaternion', 'scale'):
                    p.keyframe_insert(data_path=channel, frame=frame, group=p.name)
            if frame in {1, *range(2, end + 1, max(1, (end - 1) // 7))}:
                path = PREVIEW_DIR / f'react_{name.lower()}_f{frame:02d}.png'
                s.render.filepath = str(path); bpy.ops.render.render(write_still=True)
                preview.append(f'f{frame}_t{(frame - 1) / FPS:.2f}={path.name}')
        info = {'frames': end, 'seconds': duration, 'lowest_z_cm': min(lows), 'preview': preview}
        if not PREVIEW_ONLY:
            s.frame_set(1)
            bpy.ops.object.select_all(action='DESELECT'); r.select_set(True); bpy.context.view_layer.objects.active = r
            out = ROOT / 'export' / f'{action.name}.fbx'
            bpy.ops.export_scene.fbx(filepath=str(out), use_selection=True, object_types={'ARMATURE'},
                                     global_scale=1, apply_unit_scale=True, axis_forward='X', axis_up='Z', add_leaf_bones=False,
                                     use_armature_deform_only=True, bake_anim=True, bake_anim_use_all_actions=False,
                                     bake_anim_use_nla_strips=False, bake_anim_simplify_factor=0, bake_anim_force_startend_keying=True)
            manifest['clips'][name] = {'action': action.name, 'duration_seconds': duration, 'frames': [1, end], 'fps': FPS, 'loop': False,
                                       'reference_speed_cm_s': 0, 'root_motion': False, 'stance_samples': [],
                                       'sha256': hashlib.sha256(out.read_bytes()).hexdigest(), 'reworked': '2026-09-15 animate_creature_reacts.py'}
            info['fbx'] = str(out); info['sha256'] = manifest['clips'][name]['sha256']
        result[name] = info
    r.animation_data.action = bpy.data.actions[f'AN_{SPECIES}_Idle']; s.frame_start, s.frame_end = 1, 61; s.frame_set(1)
    for p in r.pose.bones:
        p.matrix_basis = Matrix.Identity(4)
    teardown_preview(cam)
    s.render.engine = 'CYCLES'
    if not PREVIEW_ONLY:
        (ROOT / 'export/manifest.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
        actions = {a for a in bpy.data.actions if a.name.startswith(f'AN_{SPECIES}_')}
        bpy.data.libraries.write(str(ROOT / (SPECIES.lower() + '_a.blend')), {s, *actions}, fake_user=True)
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    print('REACT RESULT ' + json.dumps({'species': SPECIES, 'clips': result}))


main()
