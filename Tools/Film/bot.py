"""/cl9film 게임플레이 봇 — 에디터 안에서(Tools/Unreal/run_python.py) PIE 호스트 캐릭터를 샷 목록대로 움직인다.

사람 조작과 같은 길만 쓴다:
- 이동은 매 프레임 이동 입력(AddMovementInput — 스틱을 민 것과 같다), 길은 내비메시 경로(FindPathToLocationSynchronously).
- 시선은 컨트롤 회전을 부드럽게 돌린다. 걸을 때는 가는 쪽을 본다.
- 줍기·상호작용(E)=Server_PickPlace, 놓기·조리(G)=Server_Drop, 휘두르기=Server_UseStart/Release — 입력이 부르는 그 함수다.
- 치트(cl9.*)·순간이동은 샷 목록에 적힌 것만 하고, 자막에 밝히는 것은 샷 목록을 쓰는 쪽의 몫이다(.claude/skills/cl9film/SKILL.md).

매개변수: Saved/Film/bot_params.json — {"shots": <샷 목록 json>, "status": <상태 json>, "timeline": <타임라인 jsonl>}
샷 목록 "steps" 의 한 줄 = {"do": <동작>, ...}. 동작과 인자는 SKILL.md "봇 동작" 표가 정본이다.
진행은 상태 파일({"idx", "done", "error", "log"})과 LogPython 의 "CL9FILM" 줄로 알린다. 다시 실행하면 앞의 봇을 멈추고 새로 돈다.
"""
import json, math, time
import unreal

PARAMS = 'C:/Work/CL9/Saved/Film/bot_params.json'
p = json.load(open(PARAMS, encoding='utf-8'))
shots = json.load(open(p['shots'], encoding='utf-8'))
STATUS, TIMELINE = p['status'], p['timeline']

old = getattr(unreal, '_cl9film_handle', None)
if old is not None:
    try:
        unreal.unregister_slate_post_tick_callback(old)
    except Exception:
        pass
    unreal._cl9film_handle = None

W = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
PC = unreal.GameplayStatics.get_player_controller(W, 0)
PAWN = unreal.GameplayStatics.get_player_pawn(W, 0)
RULES = unreal.load_asset('/Game/Data/CL9Rules')
NS = unreal.NavigationSystemV1
STATE = {'log': [], 'last': None}


def say(msg):
    STATE['log'].append(msg)
    print('CL9FILM', msg)


def write_status(idx, done=False, error=None):
    json.dump({'idx': idx, 'total': len(shots['steps']), 'done': done, 'error': error, 'log': STATE['log'][-40:]},
              open(STATUS, 'w', encoding='utf-8'), ensure_ascii=False)


def mark(kind, lines=None, speed=None):
    e = {'t': time.time(), 'kind': kind, 'lines': lines or []}
    if speed:
        e['speed'] = speed
    with open(TIMELINE, 'a', encoding='utf-8') as f:
        f.write(json.dumps(e, ensure_ascii=False) + '\n')


def cls(name):
    return unreal.load_class(None, '/Script/CL9.' + name)


def dist2d(a, b):
    return math.hypot(a.x - b.x, a.y - b.y)


def resolve(t):
    """대상 → (액터 또는 None, 위치). t: {"class": "CL9Kitchen"} | {"item": "egg", "sub": "cooked"} | {"kitchen_item": true}
    | {"truck_inside": true} | {"loc": [x, y, z]} | {"last": true} — 액터는 폰에서 가장 가까운 것"""
    here = PAWN.get_actor_location()
    if 'last' in t:
        a = STATE['last']
        return a, a.get_actor_location()
    if 'loc' in t:
        return None, unreal.Vector(*t['loc'])
    if 'truck_inside' in t:
        truck = unreal.GameplayStatics.get_all_actors_of_class(W, cls('CL9Truck'))[0]
        c = truck.get_actor_transform().transform_location(RULES.get_editor_property('truck_interior_center'))
        return None, unreal.Vector(c.x, c.y, c.z)
    if 'class' in t:
        found = list(unreal.GameplayStatics.get_all_actors_of_class(W, cls(t['class'])))
    elif 'item' in t:
        def sub_ok(i):
            if 'sub' not in t:
                return True
            sub = str(i.get_editor_property('sub_alias'))
            return sub == t['sub'] or (t['sub'] == '' and sub in ('', 'None'))
        found = [i for i in unreal.GameplayStatics.get_all_actors_of_class(W, cls('CL9Item'))
                 if str(i.get_editor_property('alias')) == t['item'] and i.get_attach_parent_actor() is None and sub_ok(i)]
    elif 'kitchen_item' in t:
        found = [k.get_cooking_item() for k in unreal.GameplayStatics.get_all_actors_of_class(W, cls('CL9Kitchen'))]
        found = [i for i in found if i is not None]
    else:
        raise ValueError('unknown target %s' % t)
    if not found:
        raise RuntimeError('no target for %s' % t)
    # 걸어서 닿는 것 중 가장 가까운 것 — 직선으로 가까워도 벽 너머면 멀다. 후보는 직선 거리로 12 개까지만 경로를 잰다
    found.sort(key=lambda x: (x.get_actor_location() - here).length())
    best = None
    for x in found[:12]:
        pts, partial, length = path_info(x.get_actor_location())
        gap = dist2d(pts[-1], x.get_actor_location()) if pts else 1e9
        if pts and gap < 260.0 and (best is None or length < best[0]):
            best = (length, x)
    a = best[1] if best else found[0]
    STATE['last'] = a
    return a, a.get_actor_location()


def path_info(goal):
    """(경로 점들 — 첫 점 = 지금 자리 제외, 부분 경로인가, 길이). 내비메시가 없으면 빈 목록"""
    here = PAWN.get_actor_location()
    path = NS.find_path_to_location_synchronously(W, here, goal)
    pts = list(path.get_editor_property('path_points')) if path else []
    length = sum(dist2d(pts[i], pts[i + 1]) for i in range(len(pts) - 1)) if len(pts) > 1 else 1e9
    return pts[1:] if len(pts) > 1 else pts, (path.is_partial() if path else True), length


def turn_toward(rot, yaw, pitch, rate, dt):
    """컨트롤 회전을 (yaw, pitch) 쪽으로 rate(도/초)만큼"""
    dy = (yaw - rot.yaw + 180.0) % 360.0 - 180.0
    dp = pitch - rot.pitch
    step = rate * dt
    ny = rot.yaw + max(-step, min(step, dy))
    np_ = rot.pitch + max(-step, min(step, dp))
    PC.set_control_rotation(unreal.Rotator(roll=0.0, pitch=np_, yaw=ny))
    return abs(dy) < 1.0 and abs(dp) < 1.0


def look_angles(target_loc):
    r = unreal.MathLibrary.find_look_at_rotation(PAWN.get_actor_location(), target_loc)
    return r.yaw, max(-80.0, min(80.0, r.pitch))


# -- 동작 — 제너레이터: 매 프레임 dt 를 받는다 --

def act_wait(s, seconds=1.0):
    t = 0.0
    while t < seconds:
        t += yield


def act_mark(s, kind='S', lines=None, speed=None):
    mark(kind, lines, speed)
    if False:
        yield


def act_cheat(s, cmd):
    unreal.SystemLibrary.execute_console_command(W, cmd, PC)
    say('cheat ' + cmd)
    if False:
        yield


def act_wait_move(s, timeout=30.0):
    """조작 잠금(도착·전원 인트로)이 풀릴 때까지 — 앞으로 밀어 보고 속도가 붙으면 풀린 것"""
    t = 0.0
    fwd = PAWN.get_actor_forward_vector()
    while t < timeout:
        PAWN.add_movement_input(fwd, 0.15, True)
        if PAWN.get_velocity().length() > 5.0:
            return
        t += yield
    raise RuntimeError('still locked after %ss' % timeout)


def act_walk(s, to, within=60.0, run=False, timeout=40.0, pitch=-8.0):
    """내비 경로를 따라 걷고(run 이면 달리기), 경로 끝에서는 대상 쪽으로 몇 걸음 다가가 within 안에 서거나 더 못 가면 멈춘다
    — 조리대처럼 몸체 안이 목표면 경로가 몸체 앞에서 끝난다(부분 경로)"""
    a, goal = resolve(to)
    pts, partial, length = path_info(goal)
    if not pts:
        # 목표가 내비메시 밖(트럭 옆 마당 등) — 목표에서 가장 가까운 내비 지점까지 경로로 가고 거기서 곧장 걷는다.
        # 그 지점도 없으면 곧장 걷고, 벽에 막히면 아래 막힘 검사가 실패로 알린다
        near = None
        try:
            near = NS.project_point_to_navigation(W, goal, None, None, unreal.Vector(900.0, 900.0, 400.0))
        except Exception:
            near = None
        if isinstance(near, unreal.Vector):
            pts = (path_info(near)[0] or []) + [goal]
            say('goal off navmesh — path to nearest nav point, then straight')
        else:
            pts = [goal]
            say('no nav path to %s — walking straight' % to)
    unreal.CL9FeelDebug.set_run_held(PAWN, bool(run))
    t = 0.0
    check_t, check_loc, replans = 0.0, PAWN.get_actor_location(), 0
    approach_still = 0.0
    try:
        while True:
            here = PAWN.get_actor_location()
            if dist2d(here, goal) <= within:
                return
            while len(pts) > 1 and dist2d(here, pts[0]) < 70.0:
                pts.pop(0)
            approaching = len(pts) == 1 and dist2d(here, pts[0]) < 45.0
            wp = goal if approaching else pts[0]
            dx, dy = wp.x - here.x, wp.y - here.y
            n = max(1.0, math.hypot(dx, dy))
            PAWN.add_movement_input(unreal.Vector(dx / n, dy / n, 0.0), 0.6 if approaching else 1.0, True)
            dt = yield
            turn_toward(PC.get_control_rotation(), math.degrees(math.atan2(dy, dx)), pitch, 240.0, dt)
            t += dt
            moved = (PAWN.get_actor_location() - here).length()
            if approaching:
                # 몸체에 막히면 거기가 닿을 수 있는 가장 가까운 자리다
                approach_still = approach_still + dt if moved < 2.0 else 0.0
                if approach_still > 0.4:
                    return
                continue
            check_t += dt
            if check_t > 1.5:
                if (PAWN.get_actor_location() - check_loc).length() < 25.0:
                    replans += 1
                    if replans > 3:
                        raise RuntimeError('stuck walking to %s' % to)
                    pts, partial, length = path_info(goal)
                    pts = pts or [goal]
                    say('replan %d' % replans)
                check_t, check_loc = 0.0, PAWN.get_actor_location()
            if t > timeout:
                raise RuntimeError('walk timeout to %s' % to)
    finally:
        unreal.CL9FeelDebug.set_run_held(PAWN, False)


def act_face(s, to=None, seconds=0.6):
    a, loc = resolve(to or {'last': True})
    yaw, pitch = look_angles(loc)
    rot = PC.get_control_rotation()
    dyaw = abs((yaw - rot.yaw + 180.0) % 360.0 - 180.0)
    rate = max(60.0, max(dyaw, abs(pitch - rot.pitch)) / max(0.1, seconds))
    t = 0.0
    while t < seconds + 1.0:
        dt = yield
        t += dt
        if turn_toward(PC.get_control_rotation(), yaw, pitch, rate, dt):
            return


def act_press(s, expect=None, times=1, gap=0.6):
    """E — Server_PickPlace. expect 가 있으면 지금 조준 대상의 클래스 이름에 그 글자가 들어 있어야 누른다"""
    for k in range(times):
        if expect:
            f = PAWN.get_focused_actor()
            name = f.get_class().get_name() if f else 'None'
            if expect not in name:
                raise RuntimeError('focus is %s, expected %s' % (name, expect))
        PAWN.call_method('Server_PickPlace')
        say('press E')
        t = 0.0
        while t < gap:
            t += yield


def act_drop(s):
    PAWN.call_method('Server_Drop')
    say('press G')
    if False:
        yield


def act_swing(s, hold=0.2):
    PAWN.call_method('Server_UseStart')
    t = 0.0
    while t < hold:
        t += yield
    PAWN.call_method('Server_UseRelease')


def act_slot(s, n=0):
    PAWN.call_method('Server_SelectSlot', (int(n),))
    if False:
        yield


def act_wait_cooked(s, timeout=40.0):
    """가장 가까운 조리대 위의 것이 cooked 가 될 때까지"""
    t = 0.0
    while t < timeout:
        items = [k.get_cooking_item() for k in unreal.GameplayStatics.get_all_actors_of_class(W, cls('CL9Kitchen'))]
        if any(i is not None and str(i.get_editor_property('sub_alias')) == 'cooked' for i in items):
            return
        t += yield
    raise RuntimeError('not cooked in %ss' % timeout)


def act_teleport(s, to, offset=None, who='host'):
    """순간이동 — who 가 'client' 면 첫 참가자 캐릭터(호스트 세계에서 옮기면 복제로 따라간다). 영상에 쓰면 자막에 밝힌다"""
    a, loc = resolve(to)
    o = offset or [0, 0, 92]
    pawn = PAWN
    if who == 'client':
        gs = unreal.GameplayStatics.get_game_state(W)
        others = [ps.get_pawn() for ps in gs.player_array if ps.get_pawn() and ps.get_pawn() != PAWN]
        if not others:
            raise RuntimeError('no client pawn')
        pawn = others[0]
    pawn.set_actor_location(unreal.Vector(loc.x + o[0], loc.y + o[1], loc.z + o[2]), False, True)
    say('teleport ' + who)
    if False:
        yield


def act_rebind(s, timeout=90.0):
    """레벨을 건넌 뒤(회사·다른 사이트) 새 PIE 세계의 호스트 컨트롤러·캐릭터를 다시 잡는다"""
    global W, PC, PAWN
    old = W.get_name() if W else ''
    t = 0.0
    while t < timeout:
        w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        pawn = unreal.GameplayStatics.get_player_pawn(w, 0) if w else None
        if w is not None and pawn is not None and (w.get_name() != old or not W.is_valid()):
            W, PC, PAWN = w, unreal.GameplayStatics.get_player_controller(w, 0), pawn
            say('rebind ' + w.get_name())
            return
        t += yield
    raise RuntimeError('no new world after %ss' % timeout)


ACTIONS = {'wait': act_wait, 'mark': act_mark, 'cheat': act_cheat, 'wait_move': act_wait_move, 'walk': act_walk,
           'face': act_face, 'press': act_press, 'drop': act_drop, 'swing': act_swing, 'slot': act_slot,
           'wait_cooked': act_wait_cooked, 'teleport': act_teleport, 'rebind': act_rebind}

steps = shots['steps']
RUN = {'idx': -1, 'gen': None}


def start_step(i):
    RUN['idx'] = i
    write_status(i)
    if i >= len(steps):
        return
    s = dict(steps[i])
    do = s.pop('do')
    s.pop('note', None)
    say('step %d %s' % (i, do))
    RUN['gen'] = ACTIONS[do](s, **s)


def stop(error=None):
    h = getattr(unreal, '_cl9film_handle', None)
    if h is not None:
        unreal.unregister_slate_post_tick_callback(h)
        unreal._cl9film_handle = None
    write_status(RUN['idx'], done=True, error=error)
    say('DONE' if error is None else 'FAILED ' + error)


def tick(dt):
    try:
        # 레벨을 건너는 동안 PIE 세계가 잠깐 비는 것은 참는다 — 8초 넘게 없으면 PIE 가 끝난 것
        if unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world() is None:
            RUN['no_world'] = RUN.get('no_world', 0.0) + dt
            if RUN['no_world'] > 8.0:
                stop('PIE ended')
            return
        RUN['no_world'] = 0.0
        while True:
            if RUN['idx'] >= len(steps):
                stop()
                return
            try:
                RUN['gen'].send(None if not RUN.get('primed') else min(dt, 0.1))
                RUN['primed'] = True
                return
            except StopIteration:
                RUN['primed'] = False
                start_step(RUN['idx'] + 1)
    except Exception as e:
        stop('step %d: %s' % (RUN['idx'], e))


open(TIMELINE, 'a', encoding='utf-8').close()
start_step(0)
RUN['primed'] = False
unreal._cl9film_handle = unreal.register_slate_post_tick_callback(tick)
say('bot started — %d step(s)' % len(steps))
