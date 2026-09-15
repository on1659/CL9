"""032 애니메이션 보강 — 3인칭 육안 검증용 스크린샷(호스트 PIE 하나). qa_anim_pass_pie.py 가 수치를 재는 것과 달리 이 스크립트는 **화면을 찍는다**.

시점: `cl9.additem egg` 로 스폰한 계란 액터를 농부 앞 260cm·높이 130cm 에 세우고(물리 끔) 뷰 타깃으로 삼는다 — 뷰 타깃이 소유 폰이 아니면
OwnerNoSee 전신이 보인다(관전과 같은 원리). 그 다음 차례로 ① `cl9.damage 1` 넘어짐(0.35·0.8·1.3초) ② 프라이팬을 들고 Server_UseStart/UseRelease
휘두르기(뗀 뒤 0.1·0.2·0.35초) ③ 계란을 들고 던지기 대기·던지기 ④ `cl9.addmonster chicken` 을 농부 앞 90cm 로 옮겨 놓고 팬으로 밀침(PushReact,
뗀 뒤 0.4·0.7초) ⑤ 잡힘(Catch)·죽음(`cl9.hitmonster 999`) 을 `HighResShot 1` 로 Saved/Screenshots/WindowsEditor 에 남긴다. 결과 목록은
docs/asset/evidence/032-anim-visual.json (찍은 시각·상태). 판정은 건드리지 않는다.

실행: PIE 시작 3초 뒤 Output Log Cmd 에 `py "D:/Work/unreal/CL9/Tools/Unreal/qa_anim_pass_visual.py"`.
"""
import unreal, json, datetime, math
from pathlib import Path

w = next(x for x in unreal.EditorLevelLibrary.get_pie_worlds(False) if 'UEDPIE_0_' in x.get_path_name())
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
pc = unreal.GameplayStatics.get_player_controller(w, 0)
body = pawn.mesh
half = pawn.get_component_by_class(unreal.CapsuleComponent).get_scaled_capsule_half_height()
out = Path(unreal.Paths.project_dir(), 'docs/asset/evidence/032-anim-visual.json')
R = {'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'status': 'running', 'shots': [], 'log': []}


def save():
    out.write_text(json.dumps(R, indent=2, ensure_ascii=False), encoding='utf-8')


def gt():
    return unreal.GameplayStatics.get_time_seconds(w)


def cmd(c):
    unreal.SystemLibrary.execute_console_command(w, c, pc)


def all_items():
    return [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor) if a.get_class().get_name() == 'BP_CL9Item_C']


def free_items(alias):
    return [a for a in all_items() if str(a.get_editor_property('alias')) == alias and not a.get_attach_parent_actor()]


def held_alias():
    for a in all_items():
        if a.get_attach_parent_actor() == pawn:
            return str(a.get_editor_property('alias'))
    return 'None'


def monsters():
    return [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor) if a.get_class().get_name() == 'BP_CL9Monster_C']


def floor_z():
    return pawn.get_actor_location().z - half


def pelvis_h():
    return round(body.get_socket_location('pelvis').z - floor_z(), 1)


def shot(tag, extra=None):
    if S.get('cam'):
        place_camera()   # 물리 아이템이라 던진 팬·휘두르기에 밀릴 수 있어 찍기 직전마다 다시 세운다
    cmd('HighResShot 1')
    e = {'t': round(gt(), 2), 'tag': tag, 'pelvis_cm': pelvis_h(), 'held': held_alias()}
    if extra:
        e.update(extra)
    R['shots'].append(e); save()


def aim_at(loc):
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location() + unreal.Vector(0, 0, half), loc))


def place_camera():
    """계란 액터를 카메라 자리에 세우고 뷰 타깃으로 — 농부의 오른쪽 앞 45° 에서 몸 전체가 보이게"""
    cam = S['cam']
    sm = cam.get_component_by_class(unreal.StaticMeshComponent)
    sm.set_simulate_physics(False)
    fwd = pawn.get_actor_forward_vector(); right = pawn.get_actor_right_vector()
    base = pawn.get_actor_location()
    pos = base + fwd * 240 + right * 200 + unreal.Vector(0, 0, 70)
    look = unreal.MathLibrary.find_look_at_rotation(pos, base + unreal.Vector(0, 0, -30))
    cam.set_actor_location_and_rotation(pos, look, False, True)
    pc.set_view_target_with_blend(cam, 0.0)


S = {'phase': 'setup', 't0': gt(), 'step': 0}
cmd('slomo 0.15')   # 자동 PIE 는 1~3fps 라 게임 시간을 늦춰야 원샷 클립의 중간 프레임이 렌더된다
cmd('cl9.additem egg')


def tick(delta):
    try:
        t = gt() - S['t0']
        ph = S['phase']
        if ph == 'setup':
            if t > 0.5:
                eggs = free_items('egg')
                if not eggs:
                    raise RuntimeError('camera egg 스폰 안 됨')
                S['cam'] = eggs[0]
                pc.set_control_rotation(unreal.Rotator(0, pawn.get_actor_rotation().yaw, 0))
                place_camera()
                S['phase'] = 'settle'; S['t0'] = gt()
        elif ph == 'settle':
            if t > 0.7:
                shot('00_stand')
                cmd('cl9.damage 1')
                S['phase'] = 'knockdown'; S['t0'] = gt(); S['step'] = 0
        elif ph == 'knockdown':
            marks = [0.3, 0.75, 1.25]
            if S['step'] < len(marks) and t >= marks[S['step']]:
                shot('01_knockdown_%d' % S['step'], {'elapsed': round(t, 2), 'knocked': bool(pawn.call_method('IsKnockedDown'))}); S['step'] += 1
            if t > 2.0:
                cmd('cl9.additem frying_pan')
                S['phase'] = 'pan'; S['t0'] = gt(); S['step'] = 0
        elif ph == 'pan':
            if S['step'] == 0 and t > 0.4:
                pans = free_items('frying_pan')
                if not pans:
                    raise RuntimeError('pan 스폰 안 됨')
                loc = pans[0].get_actor_location()
                pawn.set_actor_location(unreal.Vector(loc.x - 60, loc.y, floor_z() + half + 1), False, True)
                aim_at(loc); S['step'] = 1; S['t_step'] = gt()
            elif S['step'] == 1 and gt() - S['t_step'] > 0.3:
                pawn.call_method('Server_PickPlace'); S['step'] = 2; S['t_step'] = gt()
            elif S['step'] == 2 and gt() - S['t_step'] > 0.8:
                pc.set_control_rotation(unreal.Rotator(0, pawn.get_actor_rotation().yaw, 0))
                place_camera()
                S['step'] = 3; S['t_step'] = gt()
            elif S['step'] == 3 and gt() - S['t_step'] > 0.5:
                shot('02_hold_pan')
                pawn.call_method('Server_UseStart'); S['step'] = 4; S['t_step'] = gt()
            elif S['step'] == 4 and gt() - S['t_step'] > 0.6:
                shot('03_swing_windup')
                pawn.call_method('Server_UseRelease'); S['step'] = 5; S['t_step'] = gt(); S['marks'] = [0.08, 0.18, 0.32]; S['mi'] = 0
            elif S['step'] == 5:
                e = gt() - S['t_step']
                if S['mi'] < len(S['marks']) and e >= S['marks'][S['mi']]:
                    shot('04_swing_%d' % S['mi'], {'elapsed': round(e, 2)}); S['mi'] += 1
                if e > 0.8:
                    S['phase'] = 'throw'; S['t0'] = gt(); S['step'] = 0
        elif ph == 'throw':
            if S['step'] == 0:
                pawn.call_method('Server_ThrowChargeStart'); S['step'] = 1; S['t_step'] = gt()
            elif S['step'] == 1 and gt() - S['t_step'] > 0.6:
                shot('05_throw_windup')
                pawn.call_method('Server_ThrowRelease'); S['step'] = 2; S['t_step'] = gt(); S['marks'] = [0.08, 0.2]; S['mi'] = 0
            elif S['step'] == 2:
                e = gt() - S['t_step']
                if S['mi'] < len(S['marks']) and e >= S['marks'][S['mi']]:
                    shot('06_throw_%d' % S['mi'], {'elapsed': round(e, 2)}); S['mi'] += 1
                if e > 0.6:
                    cmd('cl9.additem frying_pan')
                    S['phase'] = 'chicken'; S['t0'] = gt(); S['step'] = 0
        elif ph == 'chicken':
            if S['step'] == 0 and t > 0.4:
                pans = free_items('frying_pan')
                loc = pans[0].get_actor_location()
                pawn.set_actor_location(unreal.Vector(loc.x - 60, loc.y, floor_z() + half + 1), False, True)
                aim_at(loc); S['step'] = 1; S['t_step'] = gt()
            elif S['step'] == 1 and gt() - S['t_step'] > 0.3:
                pawn.call_method('Server_PickPlace'); S['step'] = 2; S['t_step'] = gt()
            elif S['step'] == 2 and gt() - S['t_step'] > 0.8:
                pc.set_control_rotation(unreal.Rotator(0, pawn.get_actor_rotation().yaw, 0))
                place_camera()
                cmd('cl9.addmonster chicken'); S['step'] = 3; S['t_step'] = gt()
            elif S['step'] == 3 and gt() - S['t_step'] > 0.5:
                ms = monsters()
                if not ms:
                    raise RuntimeError('닭 스폰 안 됨')
                m = ms[-1]; S['m'] = m
                fwd = pawn.get_actor_forward_vector()
                m.set_actor_location(pawn.get_actor_location() + fwd * 95 + unreal.Vector(0, 0, -half + 36), False, True)
                m.set_actor_rotation(unreal.Rotator(0, pawn.get_actor_rotation().yaw + 180, 0), False)
                aim_at(m.get_actor_location())
                shot('07_chicken_front', {'state': str(m.call_method('GetState'))})
                pawn.call_method('Server_UseStart'); S['step'] = 4; S['t_step'] = gt()
            elif S['step'] == 4 and gt() - S['t_step'] > 0.4:
                m = S['m']
                fwd = pawn.get_actor_forward_vector()
                m.set_actor_location(pawn.get_actor_location() + fwd * 95 + unreal.Vector(0, 0, -half + 36), False, True)
                pawn.call_method('Server_UseRelease'); S['step'] = 5; S['t_step'] = gt(); S['marks'] = [0.2, 0.45, 0.7]; S['mi'] = 0
            elif S['step'] == 5:
                m = S['m']; e = gt() - S['t_step']
                if S['mi'] < len(S['marks']) and e >= S['marks'][S['mi']]:
                    shot('08_push_%d' % S['mi'], {'elapsed': round(e, 2), 'state': str(m.call_method('GetState')), 'push': bool(m.call_method('IsPushStun')),
                                                   'state_elapsed': round(float(m.call_method('GetStateElapsed')), 2)}); S['mi'] += 1
                if e > 1.2:
                    S['step'] = 6; S['t_step'] = gt()
            elif S['step'] == 6:
                m = S['m']; e = gt() - S['t_step']
                st = str(m.call_method('GetState'))
                if st.endswith('Catch') or st.endswith('CATCH: 6>'):
                    if not S.get('catch_shot'):
                        shot('09_catch', {'state': st, 'state_elapsed': round(float(m.call_method('GetStateElapsed')), 2)}); S['catch_shot'] = True
                if e > 2.5 or S.get('catch_shot'):
                    if e > 2.5 or gt() - S['t_step'] > 0.5:
                        cmd('cl9.hitmonster 999'); S['step'] = 7; S['t_step'] = gt(); S['marks'] = [0.4, 1.0]; S['mi'] = 0
            elif S['step'] == 7:
                m = S['m']; e = gt() - S['t_step']
                if S['mi'] < len(S['marks']) and e >= S['marks'][S['mi']]:
                    aim_at(m.get_actor_location())
                    shot('10_death_%d' % S['mi'], {'elapsed': round(e, 2), 'state': str(m.call_method('GetState'))}); S['mi'] += 1
                if e > 1.4:
                    pc.set_view_target_with_blend(pawn, 0.0)
                    cmd('slomo 1')
                    R['status'] = 'done'; save()
                    unreal.unregister_slate_post_tick_callback(handle)
                    unreal.log_warning('ANIM VISUAL DONE ' + json.dumps([s['tag'] for s in R['shots']]))
                    return
        save()
    except Exception as e:
        R['status'] = 'failed'; R['error'] = str(e); R['phase'] = S['phase']; save()
        try:
            pc.set_view_target_with_blend(pawn, 0.0); cmd('slomo 1')
        except Exception:
            pass
        unreal.log_warning('ANIM VISUAL FAILED ' + S['phase'] + ': ' + str(e))
        unreal.unregister_slate_post_tick_callback(handle)


save()
handle = unreal.register_slate_post_tick_callback(tick)
unreal.log_warning('ANIM VISUAL QA registered')
