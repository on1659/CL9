"""perf_probe.py 가 로그에 남긴 `[perf_probe] ProfileGPU <조합>` 표식 뒤의 ProfileGPU 표를 읽는다.

  py -3 Tools/Unreal/perf_parse.py <조합 이름> [--after 줄번호] [--top N] [--log Saved/Logs/CL9.log]
  py -3 Tools/Unreal/perf_parse.py --list            # 로그에 있는 표식과 줄 번호

같은 조합을 여러 번 돌렸으면 --after 로 원하는 실행의 시작 줄(--list 로 찾는다) 이후 첫 표식을 고른다.
출력: 파이프라인(Copy/Compute/Graphics)별 총합, Graphics 의 Scene 직계 자식 inclusive ms 상위, 키워드별 최대값.
"""
import re, sys
from pathlib import Path

# cp949 콘솔에서 한글 라벨이 깨지지 않게 — 로그 자체는 UTF-8 이다
try:
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
except Exception:
    pass

ROOT = Path(__file__).resolve().parents[2]
args = sys.argv[1:]


def opt(name, default):
    if name in args:
        i = args.index(name)
        v = args[i + 1]
        del args[i:i + 2]
        return v
    return default


log = Path(opt('--log', str(ROOT / 'Saved' / 'Logs' / 'CL9.log')))
after = int(opt('--after', 0))
top = int(opt('--top', 12))
list_only = '--list' in args
if list_only:
    args.remove('--list')

lines = log.read_text(encoding='utf-8', errors='replace').splitlines()
if list_only:
    for i, ln in enumerate(lines):
        if '[perf_probe] ' in ln:
            print('%7d  %s' % (i + 1, ln[ln.index('[perf_probe]'):][:100]))
    sys.exit(0)

if not args:
    print(__doc__)
    sys.exit(1)
want = args[0]
row_re = re.compile(r'┃.*?│\s*([\d.]+) ms\s*┃.*?│\s*([\d.]+) ms\s*┃(.*)$')
pipe_re = re.compile(r'(\w+) pipeline (\d+) - GPU (\d+)')

marker = None
for i, ln in enumerate(lines):
    if i >= after and '[perf_probe] ProfileGPU ' + want in ln:
        marker = i
        break
if marker is None:
    print('표식 없음: [perf_probe] ProfileGPU %s (after %d)' % (want, after))
    sys.exit(1)

pipeline = None
totals = {}
rows = []
end = marker
for i in range(marker + 1, min(len(lines), marker + 6000)):
    ln = lines[i]
    if '[perf_probe]' in ln:
        break
    m = pipe_re.search(ln)
    if m:
        pipeline = m.group(1)
        continue
    if 'LogRHI' not in ln:
        continue
    m = row_re.search(ln)
    if not m:
        continue
    excl, incl, name = float(m.group(1)), float(m.group(2)), m.group(3)
    indent = len(name) - len(name.lstrip(' '))
    # 이름 칸 끝에 표 테두리(┃)가 붙어 오고, 콘솔 인코딩에 따라 대체 문자로 깨지기도 한다
    name = name.replace('�', '').replace('┃', '').strip()
    if name == '<root>':
        totals[pipeline] = incl
    rows.append((pipeline, indent, name, excl, incl))
    end = i

print('표식 %d줄, 표 끝 %d줄' % (marker + 1, end + 1))
for p, t in totals.items():
    print('  %s 파이프라인 총합: %.3f ms' % (p, t))
scene_indent = next((indent for p, indent, name, _, _ in rows if p == 'Graphics' and name == 'Scene'), None)
if scene_indent is None:
    print('  (Graphics 의 Scene 행이 없다)')
    sys.exit(0)
kids = sorted(((name, incl) for p, indent, name, _, incl in rows if p == 'Graphics' and indent == scene_indent + 3), key=lambda x: -x[1])
print('  Graphics/Scene 직계 자식 (inclusive ms) 상위 %d:' % top)
for name, incl in kids[:top]:
    print('    %7.3f  %s' % (incl, name[:70]))
keys = ['ShadowDepths', 'VirtualShadowMap', 'BasePass', 'Lumen', 'LumenScreenProbeGather', 'DiffuseIndirectAndAO', 'Reflections',
        'VolumetricFog', 'PostProcessing', 'Translucency', 'Nanite', 'RayTracing', 'Lights', 'TSR', 'Substrate', 'Niagara', 'SlateUI']
print('  키워드별 최대 inclusive:')
for k in keys:
    best = None
    for p, _, name, _, incl in rows:
        if p == 'Graphics' and k.lower() in name.lower() and (best is None or incl > best[1]):
            best = (name, incl)
    if best:
        print('    %-24s %7.3f  %s' % (k, best[1], best[0][:60]))
