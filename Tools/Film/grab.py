"""PIE 창 한 장 — 클라이언트 영역 전체(DPI 를 맞춰 잘리지 않게).

쓰기: py -3 Tools/Film/grab.py --title "Client 1" --out client.png [--any]
  --any 면 제목에 'NetMode' 가 없어도 된다(에디터 창 "CL9 - 언리얼 에디터" 등).
"""
import argparse, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rec import capture, find_window


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--title', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--any', action='store_true')
    a = ap.parse_args()
    hwnd, title = find_window(a.title, need_netmode=not a.any, wait=5.0)
    if hwnd is None:
        print('NO WINDOW', a.title)
        sys.exit(1)
    im = capture(hwnd)
    im.save(a.out)
    print('GRAB', a.out, im.size, title)


if __name__ == '__main__':
    main()
