"""JPEG 프레임 묶음 → H.264 MP4.

쓰기: py -3 Tools/Film/encode.py --frames <dir> --out <file.mp4> [--fps 20]
ffmpeg 가 있으면(--ffmpeg, 환경변수 CL9_FFMPEG, 또는 아래 알려진 자리) 그것으로, 없으면 Blender 5.2 의 내장 FFmpeg 로 만든다.
"""
import argparse, os, subprocess, sys

KNOWN_FFMPEG = [
    'C:/Work/CL9/Saved/BlenderMCP/chicken_video_tools/imageio_ffmpeg/binaries/ffmpeg-win-x86_64-v7.1.exe',
]
BLENDER = 'C:/Program Files/Blender Foundation/Blender 5.2/blender.exe'
HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--frames', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--fps', type=int, default=20)
    ap.add_argument('--ffmpeg', default=os.environ.get('CL9_FFMPEG', ''))
    a = ap.parse_args()
    os.makedirs(os.path.dirname(os.path.abspath(a.out)), exist_ok=True)

    ffmpeg = next((p for p in [a.ffmpeg] + KNOWN_FFMPEG if p and os.path.exists(p)), None)
    if ffmpeg:
        cmd = [ffmpeg, '-y', '-loglevel', 'error', '-framerate', str(a.fps), '-i', os.path.join(a.frames, 'o%05d.jpg'),
               '-c:v', 'libx264', '-preset', 'veryfast', '-crf', '20', '-pix_fmt', 'yuv420p', '-movflags', '+faststart', a.out]
    elif os.path.exists(BLENDER):
        cmd = [BLENDER, '-b', '--factory-startup', '--python', os.path.join(HERE, 'blender_encode.py'), '--',
               os.path.abspath(a.frames), os.path.abspath(a.out), str(a.fps)]
    else:
        sys.exit('no ffmpeg and no Blender 5.2 — cannot encode')
    r = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='replace')
    if r.returncode != 0 or not os.path.exists(a.out):
        print(r.stdout[-2000:], r.stderr[-2000:])
        sys.exit('encode failed')
    print('ENCODED', a.out, os.path.getsize(a.out), 'bytes', 'via', 'ffmpeg' if ffmpeg else 'blender')


if __name__ == '__main__':
    main()
