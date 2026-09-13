"""농부 A 1인칭 뷰모델 클립 9종 일괄 수출(031 fp-viewmodel) — 값을 바꾼 뒤 이 한 명령으로 미리보기·FBX·manifest·blend 를 다시 낸다.

순서대로(모두 manifest.json 과 farmer_a.blend 를 쓰므로 병렬 금지):
  1. animate_farmer_hold.py   -- --export --fp-only      AN_Farmer_FPHoldSmall / FPHoldFlashlight / FPHoldLarge   (전신 Hold 3종 FBX 는 sha 그대로)
  2. animate_farmer_fparms.py -- --export                AN_Farmer_FPArmsDown / FPRun
  3. animate_farmer_swing.py  -- --export                AN_Farmer_FPSwing + 전신 AN_Farmer_Swing(30fps 0.467초 재수출)
  4. animate_farmer_windup.py -- --export --fp-only      AN_Farmer_FPSwingWindup
  5. animate_farmer_eat.py    -- --export --fp-only      AN_Farmer_FPEat
  6. verify_farmer_clip_fbx.py                           FBX 재읽기 — fps·길이·루프 이음새·sha
`--no-verify` 면 6 을 건너뛴다. `--only hold,fparms` 처럼 단계 이름(hold/fparms/swing/windup/eat)을 골라 돌릴 수도 있다.
각 단계의 `... RESULT {...}` 줄(화면 uv 실측 포함)은 그대로 흘려보내고, 실패한 단계에서 멈춘다.

실행: py -3 Tools/Blender/export_farmer_fp_clips.py [--no-verify] [--only hold,swing]
"""
import subprocess, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / 'Tools/Blender/run_headless.py'
BLEND = 'ArtSource/character/farmer_a/farmer_a.blend'
STEPS = [
    ('hold', ['animate_farmer_hold.py', '--blend', BLEND, '--', '--export', '--fp-only']),
    ('fparms', ['animate_farmer_fparms.py', '--blend', BLEND, '--', '--export']),
    ('swing', ['animate_farmer_swing.py', '--blend', BLEND, '--', '--export']),
    ('windup', ['animate_farmer_windup.py', '--blend', BLEND, '--', '--export', '--fp-only']),
    ('eat', ['animate_farmer_eat.py', '--blend', BLEND, '--', '--export', '--fp-only']),
]


def main(argv):
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    only = None
    if '--only' in argv:
        only = set(argv[argv.index('--only') + 1].split(','))
        unknown = only - {name for name, _ in STEPS}
        assert not unknown, ('모르는 단계 이름', unknown)
    steps = [(n, a) for n, a in STEPS if only is None or n in only]
    if '--no-verify' not in argv:
        steps.append(('verify', ['verify_farmer_clip_fbx.py']))
    for name, args in steps:
        print(f'[export_farmer_fp_clips] === {name} ===', flush=True)
        code = subprocess.run([sys.executable, str(RUNNER)] + args, cwd=str(ROOT)).returncode
        if code != 0:
            print(f'[export_farmer_fp_clips] {name} FAILED (exit {code})', flush=True)
            return code
    print('[export_farmer_fp_clips] DONE', flush=True)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
