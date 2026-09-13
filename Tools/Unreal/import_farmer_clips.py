"""농부 A 애니메이션 클립을 manifest 기반으로 Manny 스켈레톤에 임포트한다 — import_farmer_hold.py/import_farmer_swing.py 의 공용판.

클립 이름 목록은 (1) 명령 인자 `-- <name> ...` (2) 환경변수 CL9_FARMER_CLIPS(쉼표 구분) (3) 기본 = 이번 4클립 순으로 고른다.
각 이름은 `ArtSource/character/farmer_a/manifest.json` `animations` 에 있어야 하고, `export/<name>.fbx` 의 sha256 이 manifest 와
같아야 임포트한다. 레거시 FBX 경로(`Interchange.FeatureFlags.Import.FBX 0`, 끝나면 복구), 목적 폴더 CL9_FARMER_ANIMS
(기본 /Game/Characters/FarmerA/Anims). 임포트 뒤 길이가 manifest `seconds` 와 0.05초 넘게 다르면 경고(FBX fps 불일치 신호 —
farmer_pose.export_action 이 스테이징 장면 fps 를 맞춘 뒤로는 30fps 그대로 들어온다). 증거 JSON 은 CL9_FARMER_EVIDENCE
(기본 docs/asset/evidence/029-farmer-fparms.json — 이미 있으면 `ue_import` 절만 갱신).

실행(리드): `py -3 Tools/Unreal/run_python.py Tools/Unreal/import_farmer_clips.py` — 원격 실행은 인자·환경변수를 못 넘기므로 기본
4클립(029: FPArmsDown·FPRun + 재출력한 Flashlight Hold 쌍)이 들어간다. 다른 목록은 에디터 Output Log 의 Python 콘솔에서
`import os; os.environ['CL9_FARMER_CLIPS']='AN_Farmer_Swing,AN_Farmer_FPSwing'` 를 먼저 치고
`exec(open(r'D:/Work/unreal/CL9/Tools/Unreal/import_farmer_clips.py', encoding='utf-8').read())`.
"""
import unreal, json, hashlib, os, sys
from pathlib import Path

DEFAULT_CLIPS = ['AN_Farmer_FPArmsDown', 'AN_Farmer_FPRun', 'AN_Farmer_FPHoldFlashlight', 'AN_Farmer_HoldFlashlight']
root = Path(unreal.Paths.project_dir())
src = root / 'ArtSource/character/farmer_a/export'
manifest = json.loads((root / 'ArtSource/character/farmer_a/manifest.json').read_text(encoding='utf-8'))
folder = os.environ.get('CL9_FARMER_ANIMS', '/Game/Characters/FarmerA/Anims')
evidence = root / os.environ.get('CL9_FARMER_EVIDENCE', 'docs/asset/evidence/029-farmer-fparms.json')
if '--' in sys.argv and len(sys.argv) > sys.argv.index('--') + 1:
    names = sys.argv[sys.argv.index('--') + 1:]
elif os.environ.get('CL9_FARMER_CLIPS'):
    names = [n.strip() for n in os.environ['CL9_FARMER_CLIPS'].split(',') if n.strip()]
else:
    names = DEFAULT_CLIPS
missing = [n for n in names if n not in manifest['animations']]
assert not missing, ('manifest.animations 에 없는 클립', missing)
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
sk = unreal.load_asset('/Game/Characters/Mannequins/Meshes/SK_Mannequin')
flag = 'Interchange.FeatureFlags.Import.FBX'
old = unreal.SystemLibrary.get_console_variable_int_value(flag)
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
report = {'engine': unreal.SystemLibrary.get_engine_version(), 'skeleton': sk.get_path_name(), 'folder': folder, 'clips': []}
try:
    unreal.SystemLibrary.execute_console_command(w, flag + ' 0')
    tasks = []
    for name in names:
        meta = manifest['animations'][name]
        f = src / (name + '.fbx')
        digest = hashlib.sha256(f.read_bytes()).hexdigest()
        assert digest == meta['fbx_sha256'], ('FBX sha256 가 manifest 와 다르다 — Blender 스크립트를 --export 로 다시 돌려라', name)
        task = unreal.AssetImportTask()
        task.filename = str(f); task.destination_path = folder; task.destination_name = name
        task.automated = True; task.save = True; task.replace_existing = True
        opt = unreal.FbxImportUI()
        opt.import_mesh = False; opt.import_as_skeletal = True; opt.import_animations = True
        opt.import_materials = False; opt.import_textures = False; opt.automated_import_should_detect_type = False
        opt.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION; opt.skeleton = sk
        data = opt.anim_sequence_import_data
        data.convert_scene = True; data.convert_scene_unit = True; data.force_front_x_axis = False; data.import_uniform_scale = 1.0
        task.options = opt
        tasks.append((name, task, digest, meta))
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t for _, t, _, _ in tasks])
    for name, task, digest, meta in tasks:
        anim = unreal.load_asset(folder + '/' + name)
        assert isinstance(anim, unreal.AnimSequence), (name, list(task.imported_object_paths))
        unreal.EditorAssetLibrary.set_metadata_tag(anim, 'CL9SourceSHA256', digest)
        unreal.EditorAssetLibrary.save_loaded_asset(anim)
        seconds = anim.get_editor_property('sequence_length')
        entry = {'name': name, 'path': anim.get_path_name(), 'seconds': seconds, 'frames': anim.get_editor_property('number_of_sampled_frames'),
                 'sha256': digest, 'manifest_seconds': meta.get('seconds'), 'loop': meta.get('loop')}
        if meta.get('seconds') is not None and abs(seconds - meta['seconds']) > 0.05:
            entry['warning'] = 'length differs from manifest — FBX fps mismatch?'
            unreal.log_warning(f'{name}: imported {seconds:.3f}s but manifest says {meta["seconds"]}s')
        report['clips'].append(entry)
finally:
    unreal.SystemLibrary.execute_console_command(w, flag + ' ' + str(old))
if evidence.exists():
    existing = json.loads(evidence.read_text(encoding='utf-8'))
    existing['ue_import'] = report
    evidence.write_text(json.dumps(existing, indent=2, ensure_ascii=False), encoding='utf-8')
else:
    evidence.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding='utf-8')
unreal.log_warning('CLIPS_IMPORT_DONE ' + json.dumps(report))
