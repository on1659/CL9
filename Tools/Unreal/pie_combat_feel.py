"""combat-feel-pass-1 D-8 PIE 자동 측정 — 호스트 PIE(Lvl_Flat)에서 프라이팬·닭으로 판정·넉백·회복·예약·표면을 잰다.
실행(PIE 켜진 뒤 3초): py -3 Tools/Unreal/run_python.py Tools/Unreal/pie_combat_feel.py
cfg: Saved/Perf/combat_feel_cfg.json  {"label": "old|recovery|buffer", "out": "...json", "taps_only": false}
결과: 로그(LogCL9 `cl9.swing:`·`cl9.swingtaps:`·`cl9.knockback:`)를 리드가 파싱한다 — 이 스크립트는 단계 표식 `[lead-pie] MARK <이름>`·
액터 위치 샘플·요약 json 을 남긴다. 판정 규칙은 건드리지 않는다 — 읽기·치트·RPC 호출뿐. 벽은 PIE 전에 에디터 레벨에 놓은 BlockAll 큐브 고정물 `LeadWall`(리드 scratchpad wall_fixture.py, 저장 안 함)을 쓴다.
절차(docs/testing/pie-scenarios.md C4 ⑭~⑲): taps(⑯·⑰) → 밀착 10회(⑱) → 2m 밖·옆 비껴(⑦·⑬) → 넉백·벽 옆(⑲) → 벽 너머(⑭)·표면(⑮) → 죽이는 타격(C5)."""
import unreal, json, datetime, math
from pathlib import Path

PROJECT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
CFG_PATH = PROJECT / 'Saved' / 'Perf' / 'combat_feel_cfg.json'
CFG = json.loads(CFG_PATH.read_text(encoding='utf-8')) if CFG_PATH.is_file() else {}
LABEL = CFG.get('label', 'run')
OUT = Path(CFG.get('out') or (PROJECT / 'Saved' / 'Perf' / ('combat_feel_%s.json' % LABEL)))
TAPS_ONLY = bool(CFG.get('taps_only', False))

w = next(x for x in unreal.EditorLevelLibrary.get_pie_worlds(False) if 'UEDPIE_0_' in x.get_path_name())
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
pc = unreal.GameplayStatics.get_player_controller(w, 0)
half = pawn.get_component_by_class(unreal.CapsuleComponent).get_scaled_capsule_half_height()
MonsterClass = unreal.load_class(None, '/Script/CL9.CL9Monster')
CUBE = unreal.load_asset('/Engine/BasicShapes/Cube')
rules = unreal.load_asset('/Game/Data/CL9Rules')

R = {'label': LABEL, 'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'level': w.get_path_name(), 'status': 'running',
     'rules': {k: float(rules.get_editor_property(k)) for k in ['swing_hit_reach', 'swing_hit_radius', 'swing_recovery', 'swing_buffer_time', 'hit_knockback_time', 'hit_stop_time', 'swing_hit_start', 'swing_hit_end']},
     'tests': {}, 'log': []}


def save():
    OUT.write_text(json.dumps(R, indent=1, ensure_ascii=False, default=str), encoding='utf-8')


def gt():
    return unreal.GameplayStatics.get_time_seconds(w)


def cmd(c):
    unreal.SystemLibrary.execute_console_command(w, c, pc)
    R['log'].append([round(gt(), 2), c])


def mark(name):
    unreal.log_warning('[lead-pie] MARK %s t=%.3f' % (name, gt()))
    R['log'].append([round(gt(), 2), 'MARK ' + name])


def floor_z():
    return pawn.get_actor_location().z - half


def fwd():
    f = pc.get_control_rotation().get_forward_vector()
    f.z = 0
    return f.normal()


def right():
    r = pc.get_control_rotation().get_right_vector()
    r.z = 0
    return r.normal()


def aim(yaw, pitch=0.0):
    pc.set_control_rotation(unreal.Rotator(pitch, yaw, 0))


def monsters():
    return [m for m in unreal.GameplayStatics.get_all_actors_of_class(w, MonsterClass)]


def kill_all_monsters():
    for m in monsters():
        m.destroy_actor()


def place_monster(m, dist, side=0.0):
    p = pawn.get_actor_location()
    loc = p + fwd() * dist + right() * side
    loc.z = floor_z() + 36 + 2
    m.set_actor_location(loc, False, True)
    return loc


def find_wall():
    """에디터 레벨에 미리 놓은 BlockAll 큐브 'LeadWall'(scratchpad wall_fixture.py) — PIE 월드에 복사돼 있다"""
    for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.StaticMeshActor):
        if a.get_actor_label() == 'LeadWall':
            return a
    raise RuntimeError('LeadWall 없음 — wall_fixture.py 로 먼저 놓는다')


def stand_before_wall(dist):
    """벽 앞면(−X 쪽)에서 dist 떨어진 자리에 서서 벽(+X)을 본다 — 벽 두께 20 이라 앞면은 중심 −10"""
    wall = find_wall()
    wl = wall.get_actor_location()
    p = unreal.Vector(wl.x - 10 - dist, wl.y, floor_z() + half + 1)
    pawn.set_actor_location(p, False, True)
    aim(0.0)
    C['origin'] = pawn.get_actor_location()
    C['wall'] = wall
    return wall


def stand_open():
    """고정물 없는 빈 평지(원래 자리)"""
    pawn.set_actor_location(C['home'], False, True)
    aim(BASE_YAW)
    C['origin'] = pawn.get_actor_location()


def items(alias):
    return [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor)
            if a.get_class().get_name() == 'BP_CL9Item_C' and str(a.get_editor_property('alias')) == alias and not a.get_attach_parent_actor()]


def held_alias():
    for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor):
        if a.get_class().get_name() == 'BP_CL9Item_C' and a.get_attach_parent_actor() == pawn:
            return str(a.get_editor_property('alias'))
    return 'None'


def msample(m):
    d = m.get_actor_location() - pawn.get_actor_location()
    return {'state': str(m.call_method('GetState')).split('.')[-1], 'dist': round(math.sqrt(d.x * d.x + d.y * d.y), 1),
            'loc': [round(v, 1) for v in m.get_actor_location().to_tuple()]}


# ---- 순차 실행기: (대기초, 함수) 목록 -------------------------------------------------------------------------
Q = []
S = {'i': 0, 'due': 0.0, 'handle': None, 'ctx': {}}


def step(wait, fn=None, name=None):
    Q.append((wait, fn, name))


def swing(gap=0.15):
    """누르고 gap 초 뒤 뗀다 — 두 단계로 큐에 넣는다"""
    step(0.0, lambda: pawn.call_method('Server_UseStart'))
    step(gap, lambda: pawn.call_method('Server_UseRelease'))


def tick(_dt):
    try:
        now = gt()
        if now < S['due']:
            return
        if S['i'] >= len(Q):
            finish('done')
            return
        wait, fn, name = Q[S['i']]
        S['i'] += 1
        if fn is not None:
            fn()
        S['due'] = now + wait
    except Exception as e:  # noqa: BLE001
        R['error'] = repr(e)
        finish('error')


def finish(status):
    R['status'] = status
    R['held'] = held_alias()
    save()
    mark('END ' + status)
    unreal.unregister_slate_post_tick_callback(S['handle'])
    unreal.log_warning('[lead-pie] RESULT %s -> %s' % (status, OUT))


# ---- 시나리오 -----------------------------------------------------------------------------------------------
BASE_YAW = 0.0
C = S['ctx']


def setup():
    mark('SETUP ' + LABEL)
    cmd('t.MaxFPS 60')
    cmd('cl9.swinglog 1')
    cmd('cl9.knockbacklog 1')
    kill_all_monsters()
    aim(BASE_YAW)
    cmd('cl9.additem frying_pan')


def pick_pan():
    pans = items('frying_pan')
    if not pans:
        raise RuntimeError('frying_pan 없음')
    loc = pans[0].get_actor_location()
    pawn.set_actor_location(unreal.Vector(loc.x - 60, loc.y, floor_z() + half + 1), False, True)
    pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location() + unreal.Vector(0, 0, half), loc))


def pick_send():
    pawn.call_method('Server_PickPlace')


def pick_check():
    R['held'] = held_alias()
    if R['held'] != 'frying_pan':
        raise RuntimeError('팬을 못 들었다: ' + R['held'])
    aim(BASE_YAW)
    C['origin'] = pawn.get_actor_location()
    C['home'] = pawn.get_actor_location()


def taps_begin():
    mark('TAPS')
    aim(BASE_YAW)  # 허공 — 몬스터 없음
    cmd('cl9.swingtaps 30 100')


def new_chicken(dist, side=0.0, freeze=True):
    def f():
        kill_all_monsters()
        cmd('cl9.addmonster chicken 1')
    return f


def move_chicken(dist, side=0.0, freeze=True):
    def f():
        ms = monsters()
        if not ms:
            raise RuntimeError('닭 없음')
        m = ms[-1]
        m.set_actor_tick_enabled(not freeze)
        place_monster(m, dist, side)
        aim(BASE_YAW)
        C['m'] = m
        C['before'] = msample(m)
    return f


def record(name, extra=None):
    def f():
        m = C.get('m')
        s = msample(m) if m is not None and unreal.SystemLibrary.is_valid(m) else None
        R['tests'].setdefault(name, []).append({'before': C.get('before'), 'after': s, 'extra': extra() if extra else None, 't': round(gt(), 3)})
        save()
    return f


# 0. 준비
step(0.5, setup)
step(0.6, pick_pan)
step(0.5, pick_send)
step(0.8, pick_check)
# 1. ⑯ ⑰ 연타 — 허공
step(6.0, taps_begin)
step(0.3, lambda: mark('TAPS_END'))
if not TAPS_ONLY:
    # 2. ⑱ 밀착 명중 10회 — 닭 중심을 캡슐 반경(≈39)+닭 반경 36 ≈ 76cm 앞, 수평 조준. 매번 새 닭(두 대면 죽는다)
    for i in range(10):
        step(0.0, lambda i=i: mark('TOUCH %d' % i))
        step(0.4, new_chicken(76))
        step(0.3, move_chicken(76))
        swing()
        step(0.6, record('touch'))
    # 3. ⑦ 2m 밖 3회 (중심 거리 230 > 196) · ⑬ 옆 비껴 (전방 80, 옆 70 → 맞음 / 옆 110 → 안 맞음)
    for i in range(3):
        step(0.0, lambda i=i: mark('FAR %d' % i))
        step(0.4, new_chicken(230))
        step(0.3, move_chicken(230))
        swing()
        step(0.6, record('far'))
    for side, tag in [(70, 'SIDE_IN'), (110, 'SIDE_OUT')]:
        for i in range(3):
            step(0.0, lambda i=i, tag=tag: mark('%s %d' % (tag, i)))
            step(0.4, new_chicken(80, side))
            step(0.3, move_chicken(80, side))
            swing()
            step(0.6, record(tag.lower()))
    # 4. ⑲ 넉백 — 틱 켠 닭, 평지 (첫 타는 살아 있다)
    step(0.0, lambda: mark('KNOCK_OPEN'))
    step(0.4, new_chicken(80))
    step(0.3, move_chicken(80, 0, freeze=False))
    swing()
    step(0.8, record('knock_open'))
    # 벽 옆 — 닭 80cm 앞, 벽 140cm 앞(닭 뒤 64cm — 120cm 넉백이 벽에 막혀야 한다)
    step(0.0, lambda: mark('KNOCK_WALL'))
    step(0.2, lambda: stand_before_wall(140))
    step(0.4, new_chicken(80))
    step(0.3, move_chicken(80, 0, freeze=False))
    swing()
    step(0.8, record('knock_wall', lambda: {'wall_face_x': round(C['wall'].get_actor_location().x - 10, 1), 'origin_x': round(C['origin'].x, 1)}))
    # 5. ⑭ 벽 너머 5회 — 벽 90cm 앞(두께 20), 닭 150cm 앞(원점에서 196 안)
    step(0.0, lambda: mark('OCCLUDE'))
    step(0.2, lambda: stand_before_wall(90))
    for i in range(5):
        step(0.0, lambda i=i: mark('OCCLUDE %d' % i))
        step(0.4, new_chicken(150))
        step(0.3, move_chicken(150))
        swing()
        step(0.6, record('occlude'))
    # ⑮ 벽만(닭 없음) 2회 → surface=t
    step(0.0, lambda: mark('SURFACE_WALL'))
    step(0.3, kill_all_monsters)
    for i in range(2):
        swing()
        step(0.6, record('surface_wall'))
    # 허공 2회 → surface=-
    step(0.0, lambda: mark('SURFACE_AIR'))
    step(0.2, stand_open)
    for i in range(2):
        swing()
        step(0.6, record('surface_air'))
    # 6. C5 죽이는 타격 — 같은 닭을 두 번(첫 타 넉백 줄, 둘째 타 줄 없음·Dead)
    step(0.0, lambda: mark('KILL'))
    step(0.4, new_chicken(76))
    step(0.3, move_chicken(76, 0, freeze=False))
    swing()
    step(0.7, record('kill_1'))
    step(0.0, move_chicken(76, 0, freeze=False))
    swing()
    step(0.8, record('kill_2'))
    step(0.2, kill_all_monsters)

S['due'] = gt()
S['handle'] = unreal.register_slate_post_tick_callback(tick)
save()
unreal.log_warning('[lead-pie] START %s steps=%d' % (LABEL, len(Q)))
