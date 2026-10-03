"""PIE 성능 탐침 — 켜져 있는 PIE 안에서 설정(CVar) 조합마다 프레임 시간을 표본화하고 ProfileGPU 를 로그에 남긴다.

실행(에디터 켜진 채, PIE 시작 뒤):  py -3 Tools/Unreal/run_python.py Tools/Unreal/perf_probe.py
절차·함정은 docs/systems/performance.md "측정" 절. 잠금(pie)은 부르는 쪽이 잡는다.

설정 파일(선택): Saved/Perf/perf_probe_cfg.json
  {
    "out": "Saved/Perf/xxx.json",      # 결과 경로(생략 = Saved/Perf/perf_probe_<시각>.json)
    "label": "밤 실내",                 # 결과에 적을 이름
    "teleport": [x, y, z] | null,       # 플레이어 폰을 옮길 자리(생략 = 그대로)
    "yaw": 0.0,                         # 컨트롤 회전 요
    "configs": ["C0_baseline", ...] | null,   # 돌릴 조합 이름(생략 = C0~C9)
    "dumpframe": ["C0_baseline"],       # 표본 뒤 `stat dumpframe -ms=0.3` 도 남길 조합
    "settle_s": 2.0, "sample_s": 5.0    # 조합마다 안정화 대기·표본 길이(초)
  }
결과 JSON 의 status 가 'done' 이면 끝. 건드린 CVar·에디터 스로틀 설정은 끝날 때 원래대로 되돌린다.
로그 표식 `[perf_probe] ProfileGPU <조합>` 뒤의 GPU 표는 perf_parse.py 가 읽는다.
"""
import unreal, json, time, statistics, datetime
from pathlib import Path

PROJECT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
PERF_DIR = PROJECT / 'Saved' / 'Perf'
PERF_DIR.mkdir(parents=True, exist_ok=True)
CFG_PATH = PERF_DIR / 'perf_probe_cfg.json'
CFG = json.loads(CFG_PATH.read_text(encoding='utf-8')) if CFG_PATH.exists() else {}
OUT = Path(CFG['out']) if CFG.get('out') else PERF_DIR / ('perf_probe_%s.json' % datetime.datetime.now().strftime('%Y%m%d-%H%M%S'))
if not OUT.is_absolute():
    OUT = PROJECT / OUT
SETTLE = float(CFG.get('settle_s', 2.0))
SAMPLE = float(CFG.get('sample_s', 5.0))

worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
assert worlds, 'PIE 월드가 없다 — StartPIE 뒤에 부른다'
w = next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())


def cmd(c):
    unreal.SystemLibrary.execute_console_command(w, c)


def cv_i(n):
    return unreal.SystemLibrary.get_console_variable_int_value(n)


def cv_f(n):
    return unreal.SystemLibrary.get_console_variable_float_value(n)


# 조합이 건드리는 CVar 전부 — 끝날 때 이 값으로 되돌린다. 새 조합이 다른 CVar 를 만지면 여기에도 넣는다.
RESTORE = {n: cv_i(n) for n in [
    'r.DynamicGlobalIlluminationMethod', 'r.ReflectionMethod', 'r.Shadow.Virtual.Enable',
    'r.Lumen.HardwareRayTracing', 'r.Lumen.Reflections.HardwareRayTracing', 'r.VolumetricFog', 'r.AntiAliasingMethod',
    'sg.GlobalIlluminationQuality', 'sg.ShadowQuality', 'sg.ReflectionQuality', 'sg.PostProcessQuality',
    'sg.EffectsQuality', 'sg.AntiAliasingQuality', 'sg.ShadingQuality', 'r.ProfileGPU.ShowUI',
]}
RESTORE_F = {n: cv_f(n) for n in ['r.ScreenPercentage']}

# 창이 포그라운드가 아니면 엔진이 3fps 로 스로틀된다 — 재는 동안만 끈다(최소화된 창은 이걸 꺼도 스로틀된다: 창을 복원할 것)
perf_settings = unreal.load_object(None, '/Script/UnrealEd.Default__EditorPerformanceSettings')
THROTTLE_ORIG = perf_settings.get_editor_property('bThrottleCPUWhenNotForeground')
perf_settings.set_editor_property('bThrottleCPUWhenNotForeground', False)

SG_MEDIUM = ['sg.GlobalIlluminationQuality 1', 'sg.ShadowQuality 1', 'sg.ReflectionQuality 1', 'sg.PostProcessQuality 1',
             'sg.EffectsQuality 1', 'sg.AntiAliasingQuality 1', 'sg.ShadingQuality 1']
SG_HIGH = [c.replace(' 1', ' 2') for c in SG_MEDIUM]
LUMEN_OFF = ['r.DynamicGlobalIlluminationMethod 0', 'r.ReflectionMethod 0']
LUMEN_SW = ['r.Lumen.HardwareRayTracing 0', 'r.Lumen.Reflections.HardwareRayTracing 0']
VSM_OFF = ['r.Shadow.Virtual.Enable 0']
ALL = [
    ('C0_baseline', []),
    ('C1_lumen_off', LUMEN_OFF),
    ('C2_vsm_off', VSM_OFF),
    ('C3_lumen_vsm_off', LUMEN_OFF + VSM_OFF),
    ('C4_lumen_software', LUMEN_SW),
    ('C5_sp50', ['r.ScreenPercentage 50']),
    ('C6_fog_off', ['r.VolumetricFog 0']),
    ('C7_sg_medium', SG_MEDIUM),
    ('C8_sg_high', SG_HIGH),
    ('C9_lumen_sw_vsm_off', LUMEN_SW + VSM_OFF),
    # D = 화면 비율 200% (뷰포트가 작을 때 1080p 급 픽셀 수를 흉내 낸다) 에서 같은 비교
    ('D0_sp200', ['r.ScreenPercentage 200']),
    ('D1_sp200_lumen_off', ['r.ScreenPercentage 200'] + LUMEN_OFF),
    ('D2_sp200_vsm_off', ['r.ScreenPercentage 200'] + VSM_OFF),
    ('D3_sp200_lumen_vsm_off', ['r.ScreenPercentage 200'] + LUMEN_OFF + VSM_OFF),
    ('D4_sp200_lumen_sw', ['r.ScreenPercentage 200'] + LUMEN_SW),
    ('D5_sp200_fog_off', ['r.ScreenPercentage 200', 'r.VolumetricFog 0']),
    ('D6_sp200_sg_medium', ['r.ScreenPercentage 200'] + SG_MEDIUM),
    ('D7_sp200_sg_high', ['r.ScreenPercentage 200'] + SG_HIGH),
    ('D8_sp200_taa', ['r.ScreenPercentage 200', 'r.AntiAliasingMethod 2']),
]
want = CFG.get('configs')
CONFIGS = [c for c in ALL if (c[0] in want if want else c[0].startswith('C'))]
DUMPFRAME = set(CFG.get('dumpframe') or [])


def restore_all():
    for n, v in RESTORE.items():
        cmd('%s %d' % (n, v))
    for n, v in RESTORE_F.items():
        cmd('%s %g' % (n, v))


def scene_counts():
    c = {}
    try:
        c['actors'] = len(unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor))
        c['monsters'] = len(unreal.GameplayStatics.get_all_actors_of_class(w, unreal.load_class(None, '/Script/CL9.CL9Monster')))
        lights = [x for x in unreal.ObjectIterator(unreal.LightComponent) if x.get_world() == w]
        c['light_components'] = len(lights)
        c['light_components_visible'] = sum(1 for x in lights if x.is_visible())
        c['light_components_shadow'] = sum(1 for x in lights if x.get_editor_property('cast_shadows'))
        c['static_mesh_components'] = sum(1 for x in unreal.ObjectIterator(unreal.StaticMeshComponent) if x.get_world() == w)
        c['skeletal_mesh_components'] = sum(1 for x in unreal.ObjectIterator(unreal.SkeletalMeshComponent) if x.get_world() == w)
        c['niagara_components'] = sum(1 for x in unreal.ObjectIterator(unreal.NiagaraComponent) if x.get_world() == w)
    except Exception as e:
        c['error'] = str(e)
    return c


pc = unreal.GameplayStatics.get_player_controller(w, 0)
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
if CFG.get('teleport'):
    t = CFG['teleport']
    pawn.set_actor_location(unreal.Vector(t[0], t[1], t[2]), False, True)
    pc.set_control_rotation(unreal.Rotator(0.0, float(CFG.get('yaw', 0.0)), 0.0))

r = {
    'status': 'running', 'label': CFG.get('label', ''), 'level': w.get_path_name(),
    'engine': unreal.SystemLibrary.get_engine_version(), 'started': datetime.datetime.now().isoformat(timespec='seconds'),
    'throttle_orig': THROTTLE_ORIG, 'pawn_location': pawn.get_actor_location().to_tuple(), 'viewport': None,
    'cvars_before': dict(RESTORE, **RESTORE_F), 'scene': scene_counts(), 'settle_s': SETTLE, 'sample_s': SAMPLE,
    'configs': [], 'phase': 'init',
}
try:
    vs = pc.get_viewport_size()
    r['viewport'] = [vs[0], vs[1]] if isinstance(vs, tuple) else str(vs)
except Exception as e:
    r['viewport'] = 'ERR ' + str(e)


def save():
    OUT.write_text(json.dumps(r, indent=2, ensure_ascii=False, default=str), encoding='utf-8')


cmd('r.ProfileGPU.ShowUI 0')
save()
state = {'idx': -1, 'phase': 'apply', 'until': 0.0, 'samples': [], 'handle': None}


def begin_config(idx):
    name, cmds = CONFIGS[idx]
    restore_all()
    for c in cmds:
        cmd(c)
    state['phase'] = 'settle'
    state['until'] = time.monotonic() + SETTLE
    state['samples'] = []
    r['phase'] = name + ':settle'
    unreal.log('[perf_probe] begin ' + name)
    save()


def finish_config(idx):
    name, cmds = CONFIGS[idx]
    ms = sorted(x * 1000.0 for x in state['samples'])
    entry = {'name': name, 'cmds': cmds, 'frames': len(ms)}
    if ms:
        entry.update({'avg_ms': statistics.mean(ms), 'median_ms': statistics.median(ms),
                      'p95_ms': ms[int(len(ms) * 0.95) - 1] if len(ms) >= 20 else ms[-1], 'max_ms': ms[-1],
                      'fps_avg': 1000.0 / statistics.mean(ms)})
    entry['sp'] = cv_f('r.ScreenPercentage')
    r['configs'].append(entry)
    unreal.log('[perf_probe] %s avg %.1f ms (%.1f fps) frames %d' % (name, entry.get('avg_ms', -1), entry.get('fps_avg', -1), len(ms)))
    save()


def finish_all():
    restore_all()
    perf_settings.set_editor_property('bThrottleCPUWhenNotForeground', THROTTLE_ORIG)
    r['status'] = 'done'
    r['phase'] = 'done'
    save()
    unreal.unregister_slate_post_tick_callback(state['handle'])
    unreal.log('[perf_probe] done -> ' + str(OUT))


def tick(delta):
    try:
        now = time.monotonic()
        if state['idx'] < 0:
            state['idx'] = 0
            begin_config(0)
            return
        if state['phase'] == 'settle':
            if now >= state['until']:
                state['phase'] = 'sample'
                state['until'] = now + SAMPLE
                r['phase'] = CONFIGS[state['idx']][0] + ':sample'
                save()
            return
        if state['phase'] == 'sample':
            state['samples'].append(unreal.GameplayStatics.get_world_delta_seconds(w))
            if now >= state['until']:
                name = CONFIGS[state['idx']][0]
                if name in DUMPFRAME:
                    unreal.log('[perf_probe] DumpFrame ' + name)
                    cmd('stat dumpframe -ms=0.3')
                unreal.log('[perf_probe] ProfileGPU ' + name)
                cmd('ProfileGPU')
                state['phase'] = 'profile'
                state['until'] = now + 1.5
            return
        if state['phase'] == 'profile':
            if now >= state['until']:
                finish_config(state['idx'])
                state['idx'] += 1
                if state['idx'] >= len(CONFIGS):
                    finish_all()
                    return
                begin_config(state['idx'])
            return
    except Exception as e:
        r['status'] = 'failed'
        r['error'] = str(e)
        try:
            restore_all()
            perf_settings.set_editor_property('bThrottleCPUWhenNotForeground', THROTTLE_ORIG)
        except Exception:
            pass
        save()
        unreal.unregister_slate_post_tick_callback(state['handle'])


state['handle'] = unreal.register_slate_post_tick_callback(tick)
print('perf_probe registered: %d configs -> %s' % (len(CONFIGS), OUT))
