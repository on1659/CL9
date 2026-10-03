"""PIE 시나리오 탐침 — 프레임 시계열 + 예약 단계(콘솔 명령·텔레포트·표식·카이팅·RPC) + 구간 통계·스파이크 목록.
실행(PIE 켜진 뒤): py -3 Tools/Unreal/run_python.py Tools/Unreal/perf_scenario.py — 절차는 docs/systems/performance.md "측정".
cfg: Saved/Perf/perf_scenario_cfg.json
  {"out": "...json", "label": "...", "duration_s": 60,
   "steps": [{"t": 2, "mark": "lobby"}, {"t": 5, "cmd": "cl9.night"}, {"t": 6, "teleport": [x,y,z], "yaw": 0},
             {"t": 10, "kite": {"min": 250, "back": 900}}, {"t": 30, "kite": null}, {"t": 31, "profilegpu": true}],
   "spike_ms": 33}
끝나면 CVar·스로틀 원복. 시계열은 [t, ms, nearest_monster_cm, n_monsters] (n_monsters 는 0.5초마다 갱신)."""
import unreal, json, time, statistics, datetime, math
from pathlib import Path

PROJECT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
PERF_DIR = PROJECT / 'Saved' / 'Perf'
PERF_DIR.mkdir(parents=True, exist_ok=True)
CFG = json.loads((PERF_DIR / 'perf_scenario_cfg.json').read_text(encoding='utf-8'))
OUT = Path(CFG.get('out') or (PERF_DIR / ('perf_scenario_%s.json' % datetime.datetime.now().strftime('%Y%m%d-%H%M%S'))))
if not OUT.is_absolute():
    OUT = PROJECT / OUT
DURATION = float(CFG.get('duration_s', 60))
SPIKE = float(CFG.get('spike_ms', 33))

worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
assert worlds, 'PIE 월드 없음'
w = next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
MonsterClass = unreal.load_class(None, '/Script/CL9.CL9Monster')


def cmd(c):
    unreal.SystemLibrary.execute_console_command(w, c)


def cv_i(n):
    return unreal.SystemLibrary.get_console_variable_int_value(n)


def cv_f(n):
    return unreal.SystemLibrary.get_console_variable_float_value(n)


RESTORE = {n: cv_i(n) for n in [
    'r.DynamicGlobalIlluminationMethod', 'r.ReflectionMethod', 'r.Shadow.Virtual.Enable', 'r.Lumen.HardwareRayTracing',
    'r.Lumen.Reflections.HardwareRayTracing', 'r.VolumetricFog', 'r.AntiAliasingMethod', 'r.RayTracing.Geometry.SkeletalMeshes',
    'r.SkinCache.Mode', 'r.ProfileGPU.ShowUI', 'fx.Niagara.Enabled' if False else 'r.ProfileGPU.ShowUI',
]}
RESTORE_F = {n: cv_f(n) for n in ['r.ScreenPercentage']}
for n in CFG.get('restore_extra', []):
    RESTORE[n] = cv_i(n)

perf_settings = unreal.load_object(None, '/Script/UnrealEd.Default__EditorPerformanceSettings')
THROTTLE_ORIG = perf_settings.get_editor_property('bThrottleCPUWhenNotForeground')
perf_settings.set_editor_property('bThrottleCPUWhenNotForeground', False)

pc = unreal.GameplayStatics.get_player_controller(w, 0)
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)

r = {'status': 'running', 'label': CFG.get('label', ''), 'level': w.get_path_name(), 'started': datetime.datetime.now().isoformat(timespec='seconds'),
     'duration_s': DURATION, 'steps': CFG.get('steps', []), 'marks': [], 'series': [], 'events': [], 'windows': [], 'spikes': [], 'cvars_before': dict(RESTORE)}
try:
    vs = pc.get_viewport_size()
    r['viewport'] = [vs[0], vs[1]] if isinstance(vs, tuple) else str(vs)
except Exception:
    r['viewport'] = None


def save():
    OUT.write_text(json.dumps(r, indent=None, ensure_ascii=False, default=str), encoding='utf-8')


def restore_all():
    for n, v in RESTORE.items():
        cmd('%s %d' % (n, v))
    for n, v in RESTORE_F.items():
        cmd('%s %g' % (n, v))


steps = sorted(CFG.get('steps', []), key=lambda s: s['t'])
state = {'t0': None, 'next_step': 0, 'kite': None, 'handle': None, 'last_count': 0.0, 'n_monsters': 0, 'monsters': [], 'last_kite': 0.0}
cmd('r.ProfileGPU.ShowUI 0')
save()


def monsters():
    return unreal.GameplayStatics.get_all_actors_of_class(w, MonsterClass)


def nearest():
    p = pawn.get_actor_location()
    best = None
    for m in state['monsters']:
        try:
            d = (m.get_actor_location() - p).length()
        except Exception:
            continue
        if best is None or d < best[0]:
            best = (d, m)
    return best


def do_step(s, now):
    if 'mark' in s:
        r['marks'].append([now, s['mark']])
        unreal.log('[perf_scenario] mark ' + s['mark'])
    if 'cmd' in s:
        cmd(s['cmd'])
        r['events'].append([now, 'cmd', s['cmd']])
        unreal.log('[perf_scenario] cmd ' + s['cmd'])
    if 'teleport' in s:
        t = s['teleport']
        pawn.set_actor_location(unreal.Vector(t[0], t[1], t[2]), False, True)
        if 'yaw' in s:
            pc.set_control_rotation(unreal.Rotator(0.0, float(s['yaw']), 0.0))
        r['events'].append([now, 'teleport', t])
    if 'kite' in s:
        state['kite'] = s['kite']
        r['events'].append([now, 'kite', s['kite']])
    if s.get('profilegpu'):
        unreal.log('[perf_scenario] ProfileGPU ' + str(s.get('name', r['marks'][-1][1] if r['marks'] else 'x')))
        cmd('ProfileGPU')
    if s.get('dumpframe'):
        unreal.log('[perf_scenario] DumpFrame ' + str(s.get('name', '')))
        cmd('stat dumpframe -ms=0.3')
    if 'look_at_nearest' in s:
        nb = nearest()
        if nb:
            rot = unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(), nb[1].get_actor_location())
            pc.set_control_rotation(unreal.Rotator(0.0, rot.yaw, 0.0))
    if 'look_at_item' in s:
        # 가장 가까운 놓인 아이템을 본다(줍기용) — 피치까지 맞춘다
        ItemClass = unreal.load_class(None, '/Script/CL9.CL9Item')
        p = pawn.get_actor_location()
        best = None
        for it in unreal.GameplayStatics.get_all_actors_of_class(w, ItemClass):
            d = (it.get_actor_location() - p).length()
            if best is None or d < best[0]:
                best = (d, it)
        if best:
            rot = unreal.MathLibrary.find_look_at_rotation(p + unreal.Vector(0, 0, 60), best[1].get_actor_location())
            pc.set_control_rotation(rot)
            r['events'].append([now, 'look_at_item', best[1].get_name(), round(best[0])])
    if 'rpc' in s:
        pawn.call_method(s['rpc'])
        r['events'].append([now, 'rpc', s['rpc']])


def finish(status='done'):
    try:
        restore_all()
        perf_settings.set_editor_property('bThrottleCPUWhenNotForeground', THROTTLE_ORIG)
    except Exception:
        pass
    # 구간 통계 (표식 사이)
    marks = r['marks'] + [[r['series'][-1][0] if r['series'] else DURATION, 'end']]
    for i in range(len(marks) - 1):
        a, b = marks[i][0], marks[i + 1][0]
        ms = sorted(x[1] for x in r['series'] if a <= x[0] < b)
        if len(ms) >= 5:
            r['windows'].append({'mark': marks[i][1], 'from': a, 'to': b, 'frames': len(ms), 'avg_ms': statistics.mean(ms), 'median_ms': statistics.median(ms),
                                 'p95_ms': ms[int(len(ms) * 0.95) - 1], 'max_ms': ms[-1], 'fps_avg': 1000.0 / statistics.mean(ms)})
    r['spikes'] = [x for x in r['series'] if x[1] >= SPIKE]
    r['status'] = status
    save()
    unreal.unregister_slate_post_tick_callback(state['handle'])
    unreal.log('[perf_scenario] %s -> %s' % (status, OUT))


def tick(delta):
    try:
        now_m = time.monotonic()
        if state['t0'] is None:
            state['t0'] = now_m
            state['monsters'] = monsters()
            return
        now = now_m - state['t0']
        # 단계 실행
        while state['next_step'] < len(steps) and steps[state['next_step']]['t'] <= now:
            do_step(steps[state['next_step']], now)
            state['next_step'] += 1
        # 0.5초마다 몬스터 목록 갱신
        if now - state['last_count'] >= 0.5:
            state['last_count'] = now
            state['monsters'] = monsters()
            state['n_monsters'] = len(state['monsters'])
        nb = nearest()
        dist = nb[0] if nb else -1.0
        r['series'].append([round(now, 3), round(unreal.GameplayStatics.get_world_delta_seconds(w) * 1000.0, 2), round(dist), state['n_monsters']])
        # 카이팅 — 가장 가까운 몬스터가 min 안이면 그 반대쪽으로 back 만큼 순간이동(잡히지 않고 접근을 계속 받는다)
        k = state['kite']
        if k and nb and dist < k.get('min', 250) and now - state['last_kite'] > 0.3:
            p = pawn.get_actor_location()
            m = nb[1].get_actor_location()
            away = unreal.Vector(p.x - m.x, p.y - m.y, 0.0)
            if away.length() < 1.0:
                away = unreal.Vector(1.0, 0.0, 0.0)
            away = away / away.length()
            back = float(k.get('back', 900))
            dest = unreal.Vector(p.x + away.x * back, p.y + away.y * back, p.z)
            pawn.set_actor_location(dest, False, True)
            state['last_kite'] = now
            r['events'].append([round(now, 2), 'kite_jump', round(dist)])
        if len(r['series']) % 300 == 0:
            save()
        if now >= DURATION:
            finish('done')
    except Exception as e:
        r['error'] = str(e)
        finish('failed')


state['handle'] = unreal.register_slate_post_tick_callback(tick)
print('perf_scenario registered: %d steps, %.0fs -> %s' % (len(steps), DURATION, OUT))
