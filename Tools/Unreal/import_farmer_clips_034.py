"""034 처형 2인 클립(2026-09-21 execute-clips)의 농부 짝 2개를 import_farmer_clips.py 로 임포트한다 — 원격 실행(run_python.py)은 환경변수를 못 넘기므로
여기서 목록·증거 파일을 정하고 공용 스크립트를 exec 한다(관례는 import_farmer_clips_032.py).

AN_Farmer_Executed_Pig(정면 박치기 → 뒤로 넘어져 DOWN)·AN_Farmer_Executed_Raccoon(등 뒤 덮침 → 앞으로 엎어져 PRONE), 각 2.0초 = ExecuteDuration.
목적 폴더 /Game/Characters/FarmerA/Anims, 증거 docs/asset/evidence/034-farmer-executed.json. 몬스터 짝(AN_Pig_Execute·AN_Raccoon_Execute)은
reimport_creature_clips.py(CL9_SPECIES=Pig,Raccoon · CL9_CLIPS=Execute · CL9_EVIDENCE=docs/asset/evidence/034-execute-clips.json).

실행: py -3 Tools/Unreal/run_python.py Tools/Unreal/import_farmer_clips_034.py
"""
import os
from pathlib import Path
os.environ['CL9_FARMER_CLIPS'] = ','.join(['AN_Farmer_Executed_Pig', 'AN_Farmer_Executed_Raccoon'])
os.environ['CL9_FARMER_EVIDENCE'] = 'docs/asset/evidence/034-farmer-executed.json'
exec(compile(Path(__file__).with_name('import_farmer_clips.py').read_text(encoding='utf-8'), 'import_farmer_clips.py', 'exec'))
