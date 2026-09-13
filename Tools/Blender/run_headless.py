"""Blender 헤드리스 실행기.

    py -3 Tools/Blender/run_headless.py <script.py> [--blend <file.blend>] [-- <스크립트 인자>...]

`blender -b Tools/Blender/<script>.py` 꼴은 Blender 가 .py 를 blend 파일로 열려다 실패하고, 배경 모드는 로드 실패 시
남은 인자 처리를 멈춰서(creator_args: G.is_break) 스크립트가 절대 돌지 않는다. 올바른 형식은
`blender -b [file.blend] --python <script.py>` 이며 이 실행기가 그 형식으로 호출한다. MCP 는 쓰지 않는다.
스크립트 경로는 Tools/Blender 기준 상대 경로나 절대 경로, blend 는 저장소 기준 상대 경로나 절대 경로.
`--` 뒤의 인자는 그대로 Blender 에 넘어가 스크립트의 sys.argv 에 실린다(예: `-- --export`).
"""
import subprocess,sys
from pathlib import Path
BLENDER=Path('C:/Program Files/Blender Foundation/Blender 5.2/blender.exe')
ROOT=Path(__file__).resolve().parents[2]
def main(argv):
    sys.stdout.reconfigure(encoding='utf-8',errors='replace')
    if not argv or argv[0].startswith('-'):
        print(__doc__);return 2
    script=Path(argv[0])
    if not script.is_absolute():script=(ROOT/'Tools/Blender'/script) if not script.exists() else script.resolve()
    assert script.suffix=='.py' and script.exists(),'스크립트가 없다: '+str(script)
    args=[str(BLENDER),'-b']
    rest=argv[1:]
    if len(rest)>=2 and rest[0]=='--blend':
        blend=Path(rest[1]);blend=blend if blend.is_absolute() else ROOT/blend
        assert blend.exists(),'blend 파일이 없다: '+str(blend)
        args.append(str(blend));rest=rest[2:]
    args+=['--python',str(script)]
    if rest and rest[0]=='--':
        args+=rest  # '--' 포함 — Blender 는 '--' 뒤를 해석하지 않고 sys.argv 에 그대로 남긴다
    print('[run_headless] '+' '.join(args),flush=True)
    return subprocess.run(args,cwd=str(ROOT)).returncode
if __name__=='__main__':sys.exit(main(sys.argv[1:]))
