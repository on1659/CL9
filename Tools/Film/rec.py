"""PIE 창을 PrintWindow(PW_CLIENTONLY | PW_RENDERFULLCONTENT)로 JPEG 프레임에 찍는다 — 다른 창이 덮어도 그 창 화면만 담긴다.
실시간 녹화다(벽시계 기준, 이 머신에서 약 17~20 fps) — 프레임 단위로 멈춰 찍는 캡처와 구분한다(docs/asset/workflow.md).

쓰기: py -3 Tools/Film/rec.py --out <frames_dir> --stop <stop_file> [--max 600] [--title "Server 0"] [--fps 20]
  제목에 'NetMode' 와 --title 이 든 창을 찾는다(호스트 = "Server 0", 참가자 = "Client 1", 혼자 = "Standalone").
  stop 파일이 생기거나 --max 초가 지나면 멈추고 <frames_dir>/meta.json 에 프레임 시각을 남긴다.
"""
import argparse, ctypes, ctypes.wintypes as wt, json, os, sys, time
from PIL import Image

u = ctypes.windll.user32
g = ctypes.windll.gdi32
try:
    ctypes.windll.shcore.SetProcessDpiAwareness(2)
except Exception:
    u.SetProcessDPIAware()


class BIH(ctypes.Structure):
    _fields_ = [('biSize', wt.DWORD), ('biWidth', ctypes.c_long), ('biHeight', ctypes.c_long), ('biPlanes', wt.WORD),
                ('biBitCount', wt.WORD), ('biCompression', wt.DWORD), ('biSizeImage', wt.DWORD), ('biXPelsPerMeter', ctypes.c_long),
                ('biYPelsPerMeter', ctypes.c_long), ('biClrUsed', wt.DWORD), ('biClrImportant', wt.DWORD)]


def find_window(want, need_netmode=True, wait=90.0):
    """제목에 want(와 'NetMode')가 든 보이는 창 — 여럿이면 제목 순 첫째"""
    P = ctypes.WINFUNCTYPE(ctypes.c_bool, wt.HWND, wt.LPARAM)
    deadline = time.time() + wait
    while True:
        found = []

        def cb(h, l):
            n = u.GetWindowTextLengthW(h)
            if u.IsWindowVisible(h) and n > 0:
                b = ctypes.create_unicode_buffer(n + 1)
                u.GetWindowTextW(h, b, n + 1)
                if want in b.value and (not need_netmode or 'NetMode' in b.value):
                    found.append((h, b.value))
            return True
        u.EnumWindows(P(cb), 0)
        if found or time.time() > deadline:
            return sorted(found, key=lambda x: x[1])[0] if found else (None, None)
        time.sleep(0.5)


def capture(hwnd):
    """창 클라이언트 영역 한 장(PIL Image)"""
    rc = wt.RECT()
    u.GetClientRect(hwnd, ctypes.byref(rc))
    W, H = rc.right, rc.bottom
    hdc = u.GetWindowDC(hwnd)
    mdc = g.CreateCompatibleDC(hdc)
    bmp = g.CreateCompatibleBitmap(hdc, W, H)
    g.SelectObject(mdc, bmp)
    try:
        u.PrintWindow(hwnd, mdc, 3)
        bih = BIH()
        bih.biSize = ctypes.sizeof(BIH); bih.biWidth = W; bih.biHeight = -H; bih.biPlanes = 1; bih.biBitCount = 32
        buf = ctypes.create_string_buffer(W * H * 4)
        g.GetDIBits(mdc, bmp, 0, H, buf, ctypes.byref(bih), 0)
        return Image.frombuffer('RGB', (W, H), buf, 'raw', 'BGRX', 0, 1).copy()
    finally:
        g.DeleteObject(bmp); g.DeleteDC(mdc); u.ReleaseDC(hwnd, hdc)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--out', required=True)
    ap.add_argument('--stop', required=True)
    ap.add_argument('--max', type=float, default=600.0)
    ap.add_argument('--title', default='Server 0')
    ap.add_argument('--fps', type=float, default=20.0)
    a = ap.parse_args()

    os.makedirs(a.out, exist_ok=True)
    hwnd, title = find_window(a.title)
    if hwnd is None:
        print('NO PIE WINDOW', a.title)
        sys.exit(1)
    meta = {'title': title, 'frames': [], 'start': time.time(), 'method': 'real-time PrintWindow'}
    i = 0
    t0 = meta['start']
    try:
        while time.time() - t0 < a.max and not os.path.exists(a.stop):
            t = time.time()
            im = capture(hwnd)
            im.save(os.path.join(a.out, 'f%05d.jpg' % i), quality=90)
            meta['frames'].append(round(t - t0, 3))
            meta['size'] = list(im.size)
            i += 1
            dt = time.time() - t
            if dt < 1 / a.fps:
                time.sleep(1 / a.fps - dt)
    finally:
        meta['count'] = i
        meta['duration'] = round(time.time() - t0, 2)
        json.dump(meta, open(os.path.join(a.out, 'meta.json'), 'w', encoding='utf-8'), ensure_ascii=False)
        print('frames', i, 'duration', meta['duration'], 'fps', round(i / max(meta['duration'], 0.01), 1), 'size', meta.get('size'))


if __name__ == '__main__':
    main()
