"""녹화 프레임 + 촬영 타임라인 → 자막을 얹은 1280x720 프레임 묶음.

쓰기: py -3 Tools/Film/compose.py --frames <rec_dir> --timeline <timeline.jsonl> --out <out_dir>
        [--fps 20] [--crop auto|x0,y0,x1,y1] [--title "제목"] [--subtitle "부제"]

타임라인(봇이 쓴다, 한 줄에 JSON 하나):
  {"t": <벽시계 초>, "kind": "S", "lines": ["자막 1줄", "자막 2줄"]}   — 여기서부터 다음 표까지 제 속도
  {"t": <벽시계 초>, "kind": "F", "speed": 4, "lines": [...]}          — 다음 표까지 speed 배속(자막 끝에 "(N배속)"을 붙인다)
  {"t": <벽시계 초>, "kind": "E"}                                      — 끝
  {"kind": "STILL", "image": "<png>", "seconds": 3.5, "lines": [...]}  — 끝에 붙이는 정지 컷(순서대로)
화면: 게임 화면을 1152x648 로 줄여 위에 두고 아래 72px 띠에 자막(맑은 고딕 굵게 21). 자막 한 줄이 1240px 를 넘으면 멈춘다.
"""
import argparse, bisect, json, os, shutil
from PIL import Image, ImageDraw, ImageFont

FONT = 'C:/Windows/Fonts/malgunbd.ttf'
GAME_W, GAME_H = 1152, 648


def auto_crop(size):
    # UE PIE 창(Slate 제목 줄 포함) — 좌우 4, 위 44, 아래 6 px 를 걷으면 게임 화면이다(2026-10-13 실측, 1288x770·2312x1346 둘 다)
    w, h = size
    return (4, 44, w - 4, h - 6)


def fit(im, w, h):
    s = min(w / im.width, h / im.height)
    return im.resize((round(im.width * s), round(im.height * s)), Image.LANCZOS)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--frames', required=True)
    ap.add_argument('--timeline', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--fps', type=int, default=20)
    ap.add_argument('--crop', default='auto')
    ap.add_argument('--title', default='')
    ap.add_argument('--subtitle', default='')
    a = ap.parse_args()

    cap = ImageFont.truetype(FONT, 21)
    big = ImageFont.truetype(FONT, 40)
    mid = ImageFont.truetype(FONT, 24)
    meta = json.load(open(os.path.join(a.frames, 'meta.json'), encoding='utf-8'))
    times, start = meta['frames'], meta['start']
    entries = [json.loads(l) for l in open(a.timeline, encoding='utf-8') if l.strip()]
    marks = [e for e in entries if e.get('kind') in ('S', 'F', 'E')]
    stills = [e for e in entries if e.get('kind') == 'STILL']
    crop = auto_crop(meta['size']) if a.crop == 'auto' else tuple(int(v) for v in a.crop.split(','))

    shutil.rmtree(a.out, ignore_errors=True)
    os.makedirs(a.out)
    n = [0]

    def save(im):
        im.save(os.path.join(a.out, 'o%05d.jpg' % n[0]), quality=92)
        n[0] += 1

    def captioned(game, lines):
        canvas = Image.new('RGB', (1280, 720))
        canvas.paste(game, ((1280 - game.width) // 2, (GAME_H - game.height) // 2))
        d = ImageDraw.Draw(canvas)
        ys = [672] if len(lines) == 1 else [656, 688]
        for line, y in zip(lines[:2], ys):
            w = d.textlength(line, font=cap)
            assert w <= 1240, ('caption too wide', w, line)
            d.text(((1280 - w) / 2, y - 14), line, font=cap, fill=(255, 235, 120))
        return canvas

    if a.title:
        card = Image.new('RGB', (1280, 720))
        d = ImageDraw.Draw(card)
        for text, font, y in ((a.title, big, 300), (a.subtitle, mid, 370)):
            if text:
                d.text(((1280 - d.textlength(text, font=font)) / 2, y), text, font=font, fill=(255, 235, 120))
        for _ in range(int(2.5 * a.fps)):
            save(card)

    cache = {}
    for i, m in enumerate(marks):
        if m['kind'] == 'E' or i + 1 >= len(marks):
            break
        # 첫 장면은 녹화 처음부터 — 봇이 첫 표를 적기 전(트럭 도착 등)도 담는다
        t0 = 0.0 if i == 0 else max(0.0, m['t'] - start)
        t1 = min(times[-1], marks[i + 1]['t'] - start)
        speed = float(m.get('speed', 4)) if m['kind'] == 'F' else 1.0
        lines = list(m.get('lines', []))
        if m['kind'] == 'F' and lines:
            lines[-1] = lines[-1] + ' (%g배속)' % speed
        steps = int(max(0.0, t1 - t0) / speed * a.fps)
        for k in range(steps):
            j = max(0, bisect.bisect_right(times, t0 + k * speed / a.fps) - 1)
            if j not in cache:
                im = Image.open(os.path.join(a.frames, 'f%05d.jpg' % j)).convert('RGB').crop(crop)
                cache.clear()
                cache[j] = captioned(fit(im, GAME_W, GAME_H), lines)
            save(cache[j])

    for s in stills:
        im = Image.open(s['image']).convert('RGB')
        if s.get('crop') == 'auto':
            im = im.crop(auto_crop(im.size))
        frame = captioned(fit(im, GAME_W, GAME_H), s.get('lines', []))
        for _ in range(int(float(s.get('seconds', 3.0)) * a.fps)):
            save(frame)
    print('COMPOSED', n[0], 'frames', round(n[0] / a.fps, 2), 's')


if __name__ == '__main__':
    main()
