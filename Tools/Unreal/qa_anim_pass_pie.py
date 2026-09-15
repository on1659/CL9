"""032 애니메이션 보강 PIE 검증 — 호스트 PIE 하나에서 넘어짐·던지기·몬스터 경직(타격/밀침)·잡기 가지가 실제로 켜지는지 수치로 잰다.

절차(틱 콜백, ~25초): ① `cl9.damage 1` → 1.7초 동안 전신 골반 높이·ABP KnockdownElapsed·1인칭 팔 손목 위치를 샘플(쓰러졌다 일어나는가)
② `cl9.additem egg` 줍기 → Server_ThrowChargeStart(0.6초 대기 — ThrowWindupElapsed·손 위치) → Server_ThrowRelease(0.4초 — ThrowElapsed·손 위치)
③ `cl9.addmonster chicken` → `cl9.hitmonster 1`(타격 경직: State=Stun·IsPushStun=false·StateElapsed) ④ 닭이 다가오면 프라이팬을 들고 정면에서
Server_UseStart/UseRelease(밀침: IsPushStun=true) — 그동안 닭이 잡으면 Catch 상태·StateElapsed 도 기록. 주요 순간마다 `HighResShot 1`
(Saved/Screenshots). 판정 규칙은 건드리지 않는다 — 읽기와 치트·RPC 호출뿐. 결과 docs/asset/evidence/032-anim-pie.json.

실행: 에디터 Output Log Cmd 에 `py "D:/Work/unreal/CL9/Tools/Unreal/qa_anim_pass_pie.py"` (PIE 시작 3초 뒤).
"""
import unreal, json, datetime, math
from pathlib import Path

w = next(x for x in unreal.EditorLevelLibrary.get_pie_worlds(False) if 'UEDPIE_0_' in x.get_path_name())
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
pc = unreal.GameplayStatics.get_player_controller(w, 0)
body = pawn.mesh
arms = next(c for c in pawn.get_components_by_class(unreal.SkeletalMeshComponent) if c.get_name() == 'FirstPersonMesh')
half = pawn.get_component_by_class(unreal.CapsuleComponent).get_scaled_capsule_half_height()
out = Path(unreal.Paths.project_dir(), 'docs/asset/evidence/032-anim-pie.json')
R = {'engine': unreal.SystemLibrary.get_engine_version(), 'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
     'level': w.get_path_name(), 'status': 'running', 'phases': {}, 'log': []}


def save():
    out.write_text(json.dumps(R, indent=2, ensure_ascii=False), encoding='utf-8')


def gt():
    return unreal.GameplayStatics.get_time_seconds(w)


def cmd(c):
    unreal.SystemLibrary.execute_console_command(w, c, pc)
    R['log'].append([round(gt(), 2), c])


def var(mesh, name):
    inst = mesh.get_anim_instance()
    try:
        return round(float(inst.get_editor_property(name)), 3)
    except Exception as e:
        return 'ERR ' + str(e)[:60]


def floor_z():
    return pawn.get_actor_location().z - half


def rel(mesh, socket):
    p = mesh.get_socket_location(socket)
    o = pawn.get_actor_location()
    return [round(p.x - o.x, 1), round(p.y - o.y, 1), round(p.z - floor_z(), 1)]


def body_sample():
    return {'pelvis': rel(body, 'pelvis'), 'head': rel(body, 'head'), 'hand_r': rel(body, 'hand_r'), 'foot_l': rel(body, 'foot_l'),
            'KnockdownElapsed': var(body, 'KnockdownElapsed'), 'KnockdownRemaining': var(body, 'KnockdownRemaining'),
            'ThrowElapsed': var(body, 'ThrowElapsed'), 'ThrowWindupElapsed': var(body, 'ThrowWindupElapsed'),
            'arms_hand_r': rel(arms, 'hand_r'), 'arms_KnockdownElapsed': var(arms, 'KnockdownElapsed'), 'arms_ThrowElapsed': var(arms, 'ThrowElapsed'),
            'knocked': bool(pawn.call_method('IsKnockedDown'))}


def monsters():
    return [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor) if a.get_class().get_name() == 'BP_CL9Monster_C']


def monster_sample(m):
    mesh = m.get_component_by_class(unreal.SkeletalMeshComponent)
    d = (m.get_actor_location() - pawn.get_actor_location())
    return {'state': str(m.call_method('GetState')), 'StateElapsed': round(float(m.call_method('GetStateElapsed')), 3),
            'IsPushStun': bool(m.call_method('IsPushStun')), 'abp_StateElapsed': var(mesh, 'StateElapsed'), 'abp_IsPushStun': var(mesh, 'IsPushStun'),
            'dist': round(math.sqrt(d.x * d.x + d.y * d.y), 1), 'head_z': round(mesh.get_socket_location('head').z - m.get_actor_location().z, 1)}


S = {'phase': 'knockdown', 't0': gt(), 'samples': [], 'shots': 0}
cmd('cl9.damage 1')


def shot(tag):
    cmd('HighResShot 1')
    S['shots'] += 1
    R['log'].append([round(gt(), 2), 'shot ' + tag])


def items(alias):
    return [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor)
            if a.get_class().get_name() == 'BP_CL9Item_C' and str(a.get_editor_property('alias')) == alias and not a.get_attach_parent_actor()]


def held_alias():
    for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor):
        if a.get_class().get_name() == 'BP_CL9Item_C' and a.get_attach_parent_actor() == pawn:
            return str(a.get_editor_property('alias'))
    return 'None'


def aim_at(loc):
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location() + unreal.Vector(0, 0, half), loc))


def tick(delta):
    try:
        t = gt() - S['t0']
        ph = S['phase']
        if ph == 'knockdown':
            s = body_sample(); s['t'] = round(t, 2); S['samples'].append(s)
            if 0.6 < t < 0.75 and S['shots'] == 0:
                shot('down')
            if t > 1.9:
                R['phases']['knockdown'] = S['samples']
                zs = [x['pelvis'][2] for x in S['samples']]
                R['phases']['knockdown_summary'] = {'pelvis_min_cm': min(zs), 'pelvis_end_cm': zs[-1], 'pelvis_start_cm': zs[0],
                                                    'max_KnockdownElapsed': max(x['KnockdownElapsed'] for x in S['samples'] if isinstance(x['KnockdownElapsed'], float))}
                S.update({'phase': 'throw_setup', 't0': gt(), 'samples': [], 'shots': 0})
                cmd('cl9.additem egg')
        elif ph == 'throw_setup':
            if t > 0.5:
                egg = items('egg')
                if not egg:
                    raise RuntimeError('egg 스폰 안 됨')
                loc = egg[0].get_actor_location()
                pawn.set_actor_location(unreal.Vector(loc.x - 60, loc.y, floor_z() + half + 1), False, True)
                aim_at(loc)
                S['phase'] = 'throw_pick'; S['t0'] = gt()
        elif ph == 'throw_pick':
            if 0.3 < t < 0.4 and not S.get('picked'):
                pawn.call_method('Server_PickPlace'); S['picked'] = True
            if t > 1.0:
                held = held_alias()
                R['phases']['throw_held'] = held
                if held == 'None':
                    raise RuntimeError('계란을 못 들었다')
                pawn.call_method('Server_ThrowChargeStart')
                S.update({'phase': 'throw_windup', 't0': gt(), 'samples': []})
        elif ph == 'throw_windup':
            s = body_sample(); s['t'] = round(t, 2); S['samples'].append(s)
            if 0.4 < t < 0.5 and S['shots'] == 0:
                shot('throw_windup')
            if t > 0.6:
                R['phases']['throw_windup'] = S['samples']
                pawn.call_method('Server_ThrowRelease')
                S.update({'phase': 'throw', 't0': gt(), 'samples': [], 'shots': 0})
        elif ph == 'throw':
            s = body_sample(); s['t'] = round(t, 2); S['samples'].append(s)
            if 0.1 < t < 0.2 and S['shots'] == 0:
                shot('throw')
            if t > 0.5:
                R['phases']['throw'] = S['samples']
                S.update({'phase': 'monster_spawn', 't0': gt(), 'samples': [], 'shots': 0})
                cmd('cl9.addmonster chicken')
        elif ph == 'monster_spawn':
            if t > 0.5:
                ms = monsters()
                if not ms:
                    raise RuntimeError('닭 스폰 안 됨')
                S['m'] = ms[-1]
                cmd('cl9.hitmonster 1')
                S.update({'phase': 'hit', 't0': gt(), 'samples': []})
        elif ph == 'hit':
            s = monster_sample(S['m']); s['t'] = round(t, 2); S['samples'].append(s)
            if 0.1 < t < 0.2 and S['shots'] == 0:
                aim_at(S['m'].get_actor_location()); shot('hitreact')
            if t > 0.6:
                R['phases']['hit'] = S['samples']
                cmd('cl9.additem frying_pan')
                S.update({'phase': 'pan_pick', 't0': gt(), 'samples': [], 'shots': 0, 'picked': False})
        elif ph == 'pan_pick':
            pans = items('frying_pan')
            if pans and not S['picked'] and t > 0.4:
                loc = pans[0].get_actor_location()
                pawn.set_actor_location(unreal.Vector(loc.x - 60, loc.y, floor_z() + half + 1), False, True)
                aim_at(loc)
                S['picked'] = True; S['t_pick'] = gt()
            if S['picked'] and gt() - S['t_pick'] > 0.3 and not S.get('pick_sent'):
                pawn.call_method('Server_PickPlace'); S['pick_sent'] = True
            if S.get('pick_sent') and gt() - S['t_pick'] > 1.0:
                held = held_alias()
                R['phases']['pan_held'] = held
                S.update({'phase': 'wait_chicken', 't0': gt(), 'samples': []})
        elif ph == 'wait_chicken':
            m = S['m']
            s = monster_sample(m); s['t'] = round(t, 2); s['me_knocked'] = bool(pawn.call_method('IsKnockedDown')); S['samples'].append(s)
            if s['state'].endswith('Catch') and not S.get('catch_seen'):
                S['catch_seen'] = True; shot('catch')
            if s['dist'] < 130 and not S.get('swung') and not s['me_knocked']:
                aim_at(m.get_actor_location())
                pawn.call_method('Server_UseStart'); S['swung'] = True; S['t_swing'] = gt()
            if S.get('swung') and gt() - S['t_swing'] > 0.15 and not S.get('released'):
                pawn.call_method('Server_UseRelease'); S['released'] = True; S['t_release'] = gt()
            if S.get('released') and 0.15 < gt() - S['t_release'] < 0.3 and S['shots'] == 0:
                shot('push')
            if (S.get('released') and gt() - S['t_release'] > 1.2) or t > 12:
                R['phases']['push'] = S['samples']
                pushes = [x for x in S['samples'] if x['IsPushStun'] and x['state'].endswith('Stun')]
                R['phases']['push_summary'] = {'push_stun_samples': len(pushes), 'catch_seen': bool(S.get('catch_seen')), 'swung': bool(S.get('swung'))}
                R['status'] = 'done'; save()
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.log_warning('ANIM PIE RESULT ' + json.dumps({'knockdown': R['phases'].get('knockdown_summary'), 'throw_held': R['phases'].get('throw_held'),
                                                                   'pan_held': R['phases'].get('pan_held'), 'push': R['phases'].get('push_summary'),
                                                                   'hit_states': sorted(set(x['state'] for x in R['phases'].get('hit', [])))}))
                return
        save()
    except Exception as e:
        R['status'] = 'failed'; R['error'] = str(e); R['phase'] = S['phase']; save()
        unreal.log_warning('ANIM PIE FAILED ' + S['phase'] + ': ' + str(e))
        unreal.unregister_slate_post_tick_callback(handle)


save()
handle = unreal.register_slate_post_tick_callback(tick)
unreal.log_warning('ANIM PIE QA registered')
