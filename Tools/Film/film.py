"""/cl9film 진행기 — 샷 목록 하나로 게임플레이 영상을 찍는다(준비 → PIE → 녹화 → 봇 → 정지 컷 → 정리 → 자막 → 인코딩).

쓰기: py -3 Tools/Film/film.py --shots Tools/Film/shots/round_farm.json [--name 이름] [--timeout 900] [--keep]
  **먼저 잠금**: py -3 .claude/hooks/cl9_lock.py acquire pie (단독 명령) — 끝나면 release. 이 스크립트는 잠금을 잡지 않는다.
  결과: Saved/Video/<이름>.mp4 와 Saved/Film/<이름>/(frames·timeline.jsonl·status.json·sheet.jpg). 실패하면 정리까지는 하고 0 이 아닌 코드로 끝난다.

샷 목록(json): level, players(1|2), rules({셀: 값} — 촬영 동안만), title, subtitle,
  steps(봇 동작 — Tools/Film/bot.py, .claude/skills/cl9film/SKILL.md "봇 동작"),
  stills([{"window": "Client 1", "seconds": 3, "lines": ["…"]}] — 봇이 끝난 뒤 그 창을 한 장 찍어 영상 끝에 붙인다).
"""
import argparse, bisect, json, os, re, subprocess, sys, time
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
import mcp  # noqa: E402

RUN_PY = os.path.join(ROOT, 'Tools', 'Unreal', 'run_python.py')
FILM = os.path.join(ROOT, 'Saved', 'Film')
ENV = dict(os.environ, PYTHONIOENCODING='utf-8')


def run_in_editor(script, params_file=None, params=None):
    if params_file:
        json.dump(params, open(params_file, 'w', encoding='utf-8'), ensure_ascii=False)
    r = subprocess.run(['py', '-3', RUN_PY, script], capture_output=True, text=True, encoding='utf-8', errors='replace', env=ENV)
    out = r.stdout + r.stderr
    # run_python 은 JSON 으로 돌려준다 — 글 안의 줄바꿈은 \r\n 두 글자로 들어 있다
    for line in re.split(r'(?:\\r)?\\n|\n', out):
        if 'CL9FILM' in line:
            print('  ', line.strip().strip('"')[:200])
    if 'ABORT' in out or '"success": false' in out:
        raise SystemExit('editor step failed: %s' % os.path.basename(script))
    return out


def contact_sheet(run, frames, start):
    times = json.load(open(os.path.join(frames, 'meta.json'), encoding='utf-8'))['frames']
    marks = [json.loads(l) for l in open(os.path.join(run, 'timeline.jsonl'), encoding='utf-8') if l.strip()]
    marks = [m for m in marks if m.get('kind') in ('S', 'F')][:16]
    if not marks or not times:
        return
    W, H = 480, 270
    sheet = Image.new('RGB', (W * 4, H * ((len(marks) + 3) // 4)))
    for k, m in enumerate(marks):
        j = min(bisect.bisect_left(times, max(0.0, m['t'] - start + 2.0)), len(times) - 1)
        im = Image.open(os.path.join(frames, 'f%05d.jpg' % j)).resize((W, H))
        ImageDraw.Draw(im).text((6, H - 16), '%d %s %.1fs' % (k, m['kind'], m['t'] - start), fill=(255, 255, 0))
        sheet.paste(im, ((k % 4) * W, (k // 4) * H))
    sheet.save(os.path.join(run, 'sheet.jpg'), quality=85)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--shots', required=True)
    ap.add_argument('--name', default='')
    ap.add_argument('--timeout', type=float, default=900.0)
    ap.add_argument('--keep', action='store_true', help='프레임 묶음을 남긴다(기본: 영상을 만든 뒤 지운다)')
    a = ap.parse_args()

    shots = json.load(open(a.shots, encoding='utf-8'))
    name = a.name or '%s_%s' % (shots.get('name', 'film'), time.strftime('%Y%m%d_%H%M'))
    run = os.path.join(FILM, name)
    frames = os.path.join(run, 'frames')
    os.makedirs(frames, exist_ok=True)
    for f in ('timeline.jsonl', 'status.json', 'stop'):
        if os.path.exists(os.path.join(run, f)):
            os.remove(os.path.join(run, f))
    restore = os.path.join(run, 'restore.json')
    ops = os.path.join(FILM, 'ops_params.json')
    players = int(shots.get('players', 1))

    print('1/7 setup', shots.get('level'), 'players', players)
    run_in_editor(os.path.join(HERE, 'editor_ops.py'), ops, {'op': 'setup', 'level': shots['level'], 'players': players,
                                                             'rules': shots.get('rules', {}), 'restore': restore})
    error = None
    rec = None
    try:
        print('2/7 StartPIE')
        mcp.call('EditorToolset.EditorAppToolset', 'StartPIE', {'options': {'playMode': 'PlayMode_InEditorFloating'}})
        print('3/7 recorder')
        rec = subprocess.Popen(['py', '-3', os.path.join(HERE, 'rec.py'), '--out', frames, '--stop', os.path.join(run, 'stop'),
                                '--max', str(a.timeout + 60), '--title', 'Server 0' if players > 1 else 'NetMode'],
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, env=ENV)
        time.sleep(3.0)
        print('4/7 bot')
        json.dump({'shots': os.path.abspath(a.shots), 'status': os.path.join(run, 'status.json'),
                   'timeline': os.path.join(run, 'timeline.jsonl')}, open(os.path.join(FILM, 'bot_params.json'), 'w', encoding='utf-8'))
        run_in_editor(os.path.join(HERE, 'bot.py'))
        t0, last = time.time(), -1
        while True:
            time.sleep(2.0)
            try:
                st = json.load(open(os.path.join(run, 'status.json'), encoding='utf-8'))
            except Exception:
                continue
            if st['idx'] != last:
                last = st['idx']
                print('   step %d/%d %s' % (st['idx'], st['total'], st['log'][-1] if st['log'] else ''))
            if st['done']:
                error = st['error']
                break
            if time.time() - t0 > a.timeout:
                error = 'timeout'
                break
        print('5/7 stills', 'bot error: ' + error if error else 'bot done')
        tl = open(os.path.join(run, 'timeline.jsonl'), 'a', encoding='utf-8')
        for k, s in enumerate(shots.get('stills', [])):
            png = os.path.join(run, 'still%d.png' % k)
            r = subprocess.run(['py', '-3', os.path.join(HERE, 'grab.py'), '--title', s['window'], '--out', png],
                               capture_output=True, text=True, env=ENV)
            if os.path.exists(png):
                tl.write(json.dumps({'kind': 'STILL', 'image': png, 'crop': 'auto', 'seconds': s.get('seconds', 3),
                                     'lines': s.get('lines', [])}, ensure_ascii=False) + '\n')
        tl.close()
    finally:
        open(os.path.join(run, 'stop'), 'w').close()
        if rec:
            print('  ', (rec.communicate(timeout=60)[0] or '').strip()[-200:])
        print('6/7 StopPIE + restore')
        try:
            mcp.call('EditorToolset.EditorAppToolset', 'StopPIE', {})
        except Exception as e:
            print('   StopPIE failed', e)
        time.sleep(3.0)
        run_in_editor(os.path.join(HERE, 'editor_ops.py'), ops, {'op': 'restore', 'restore': restore})

    start = json.load(open(os.path.join(frames, 'meta.json'), encoding='utf-8'))['start']
    contact_sheet(run, frames, start)
    if error:
        print('FAILED', error, '— see', os.path.join(run, 'status.json'), 'and sheet.jpg')
        sys.exit(2)
    print('7/7 compose + encode')
    out_frames = os.path.join(run, 'out')
    subprocess.run(['py', '-3', os.path.join(HERE, 'compose.py'), '--frames', frames, '--timeline', os.path.join(run, 'timeline.jsonl'),
                    '--out', out_frames, '--title', shots.get('title', ''), '--subtitle', shots.get('subtitle', '')], check=True, env=ENV)
    video = os.path.join(ROOT, 'Saved', 'Video', name + '.mp4')
    subprocess.run(['py', '-3', os.path.join(HERE, 'encode.py'), '--frames', out_frames, '--out', video], check=True, env=ENV)
    if not a.keep:
        import shutil
        shutil.rmtree(frames, ignore_errors=True)
        shutil.rmtree(out_frames, ignore_errors=True)
    print('VIDEO', video)
    print('SHEET', os.path.join(run, 'sheet.jpg'))


if __name__ == '__main__':
    main()
