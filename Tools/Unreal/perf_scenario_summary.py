"""perf_scenario.py 결과 JSON 요약 — py -3 Tools/Unreal/perf_scenario_summary.py Saved/Perf/<결과>.json: 구간 통계·스파이크(시각·ms·가까운 몬스터 거리·수)·사건."""
import json, sys
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
d = json.load(open(sys.argv[1], encoding='utf-8'))
print('==', d.get('label'), d['status'], 'viewport', d.get('viewport'), 'frames', len(d['series']))
print('%-16s %6s %6s %6s %6s %6s %5s' % ('window', 'from', 'avg', 'med', 'p95', 'max', 'n'))
for wnd in d['windows']:
    print('%-16s %6.1f %6.1f %6.1f %6.1f %6.1f %5d' % (wnd['mark'], wnd['from'], wnd['avg_ms'], wnd['median_ms'], wnd['p95_ms'], wnd['max_ms'], wnd['frames']))
sp = d['spikes']
print('spikes >= %s ms: %d' % (d.get('spike_ms', '?') if 'spike_ms' in d else '?', len(sp)))
big = sorted(sp, key=lambda x: -x[1])[:15]
for t, ms, dist, n in sorted(big):
    # 어느 구간인지
    mark = ''
    for mt, name in d['marks']:
        if mt <= t:
            mark = name
    print('  t=%6.2f  %7.1f ms  dist %6d  monsters %2d  [%s]' % (t, ms, dist, n, mark))
ev = [e for e in d['events'] if e[1] != 'kite_jump']
print('events:', len(d['events']), 'kite jumps:', sum(1 for e in d['events'] if e[1] == 'kite_jump'))
for e in ev[:12]:
    print('  ', e)
if d.get('error'):
    print('ERROR', d['error'])
