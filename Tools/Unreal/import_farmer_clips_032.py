"""032 애니메이션 보강(2026-09-15)의 농부 클립 13개를 import_farmer_clips.py 로 임포트한다 — 원격 실행(run_python.py)은 환경변수를 못 넘기므로
여기서 목록·증거 파일을 정하고 공용 스크립트를 exec 한다.

휘두르기 재작업 4(AN_Farmer_Swing·FPSwing·SwingWindup·FPSwingWindup) + 넘어짐 5(Knockdown·Down·GetUp·FPKnockdown·FPGetUp) + 던지기 4(ThrowWindup·Throw·
FPThrowWindup·FPThrow). 목적 폴더 /Game/Characters/FarmerA/Anims, 증거 docs/asset/evidence/032-farmer-anim-pass.json.

실행: py -3 Tools/Unreal/run_python.py Tools/Unreal/import_farmer_clips_032.py
"""
import os
from pathlib import Path
os.environ['CL9_FARMER_CLIPS'] = ','.join([
    'AN_Farmer_Swing', 'AN_Farmer_FPSwing', 'AN_Farmer_SwingWindup', 'AN_Farmer_FPSwingWindup',
    'AN_Farmer_Knockdown', 'AN_Farmer_Down', 'AN_Farmer_GetUp', 'AN_Farmer_FPKnockdown', 'AN_Farmer_FPGetUp',
    'AN_Farmer_ThrowWindup', 'AN_Farmer_Throw', 'AN_Farmer_FPThrowWindup', 'AN_Farmer_FPThrow'])
os.environ['CL9_FARMER_EVIDENCE'] = 'docs/asset/evidence/032-farmer-anim-pass.json'
exec(compile(Path(__file__).with_name('import_farmer_clips.py').read_text(encoding='utf-8'), 'import_farmer_clips.py', 'exec'))
