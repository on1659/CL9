"""미리보기 PNG 여러 장을 한 장의 격자 시트로 묶는다 — 애니메이션 프레임을 한눈에 보는 용도.

    py -3 Tools/Blender/contact_sheet.py <out.png> <열 수> <타일 폭> <label=path> [<label=path> ...]

타일은 폭에 맞춰 비율 유지 축소, 왼쪽 위에 라벨(프레임 번호·시각)을 쓴다 — 한글 라벨은 맑은 고딕(없으면 기본 글꼴). Blender 밖(py -3, Pillow)에서 돈다.
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont


def _font():
    for f in ('C:/Windows/Fonts/malgun.ttf', 'C:/Windows/Fonts/arial.ttf'):
        try:
            return ImageFont.truetype(f, 18)
        except OSError:
            continue
    return ImageFont.load_default()


def main(argv):
    out, cols, width = Path(argv[0]), int(argv[1]), int(argv[2])
    items = [a.split('=', 1) for a in argv[3:]]
    tiles = []
    for label, path in items:
        im = Image.open(path).convert('RGB')
        h = round(im.height * width / im.width)
        im = im.resize((width, h), Image.LANCZOS)
        ImageDraw.Draw(im).text((6, 4), label, fill=(255, 220, 80), font=_font())
        tiles.append(im)
    rows = (len(tiles) + cols - 1) // cols
    th = max(t.height for t in tiles)
    sheet = Image.new('RGB', (cols * width, rows * th), (30, 30, 30))
    for i, t in enumerate(tiles):
        sheet.paste(t, ((i % cols) * width, (i // cols) * th))
    out.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(out)
    print('sheet', out, sheet.size)


if __name__ == '__main__':
    main(sys.argv[1:])
