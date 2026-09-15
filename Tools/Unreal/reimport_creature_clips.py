"""다시 만든 동물 반응 클립(Catch·HitReact·PushReact — Tools/Blender/animate_creature_reacts.py)을 종 스켈레톤에 재임포트한다.

import_creature_exports.py 의 import_file 은 기존 에셋의 CL9SourceSHA256 이 소스와 다르면 assert 로 막는다(원본 보호). 이 스크립트는
manifest `clips` 항목에 `reworked` 표시가 있는 클립만 골라 replace_existing 으로 덮어쓰고 sha 태그를 갱신한다 — 없는 클립(PushReact)은 새로 만든다.
로코모션·Death 등 표시 없는 클립은 건드리지 않는다. 임포트 뒤 길이가 manifest duration_seconds 와 0.001초 넘게 다르면 실패.

실행(에디터 파이썬): py -3 Tools/Unreal/run_python.py Tools/Unreal/reimport_creature_clips.py
  종 목록은 환경변수 CL9_SPECIES(쉼표, 기본 Chicken,Goose,Raccoon,Pig,Sheep), 증거 JSON 은 docs/asset/evidence/032-creature-react-clips.json.
"""
import unreal, json, hashlib, datetime, os
from pathlib import Path

root = Path(unreal.Paths.project_dir()).resolve()
at = unreal.AssetToolsHelpers.get_asset_tools()
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
species_list = [s.strip() for s in os.environ.get('CL9_SPECIES', 'Chicken,Goose,Raccoon,Pig,Sheep').split(',') if s.strip()]
flag = 'Interchange.FeatureFlags.Import.FBX'
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
old_flag = unreal.SystemLibrary.get_console_variable_int_value(flag)
report = {'engine': unreal.SystemLibrary.get_engine_version(), 'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'creatures': []}


def import_clip(source, destination, name, skeleton, fps):
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    path = destination + '/' + name
    task = unreal.AssetImportTask()
    task.filename = str(source); task.destination_path = destination; task.destination_name = name
    task.automated = True; task.save = True; task.replace_existing = True
    opt = unreal.FbxImportUI()
    opt.import_mesh = False; opt.import_as_skeletal = True; opt.import_animations = True
    opt.import_materials = False; opt.import_textures = False; opt.create_physics_asset = False
    opt.automated_import_should_detect_type = False
    opt.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    opt.skeleton = skeleton
    data = opt.anim_sequence_import_data
    data.convert_scene = True; data.convert_scene_unit = True; data.force_front_x_axis = True; data.import_uniform_scale = 1.0
    data.set_editor_property('use_default_sample_rate', fps == 30)
    data.set_editor_property('custom_sample_rate', fps)
    task.options = opt
    at.import_asset_tasks([task])
    asset = unreal.load_asset(path)
    assert asset is not None, list(task.imported_object_paths)
    unreal.EditorAssetLibrary.set_metadata_tag(asset, 'CL9SourceSHA256', digest)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset, digest


try:
    unreal.SystemLibrary.execute_console_command(world, flag + ' 0')
    for species in species_list:
        source = root / 'ArtSource/monster' / f'{species.lower()}_a/export'
        manifest = json.loads((source / 'manifest.json').read_text())
        folder = f'/Game/Monsters/{species}A'
        mesh = unreal.load_asset(f'{folder}/{manifest["mesh"]}')
        assert isinstance(mesh, unreal.SkeletalMesh), species
        skeleton = mesh.get_editor_property('skeleton')
        record = {'species': species, 'skeleton': skeleton.get_path_name(), 'clips': []}
        for name, clip in manifest['clips'].items():
            if not clip.get('reworked'):
                continue
            existed = unreal.EditorAssetLibrary.does_asset_exist(f'{folder}/{clip["action"]}')
            animation, digest = import_clip(source / (clip['action'] + '.fbx'), folder, clip['action'], skeleton, clip['fps'])
            assert isinstance(animation, unreal.AnimSequence)
            length = animation.get_editor_property('sequence_length')
            assert abs(length - clip['duration_seconds']) < .001, (name, length)
            assert animation.get_editor_property('skeleton') == skeleton
            record['clips'].append({'name': name, 'path': animation.get_path_name(), 'duration_seconds': length, 'sha256': digest,
                                    'replaced': existed})
        report['creatures'].append(record)
finally:
    unreal.SystemLibrary.execute_console_command(world, f'{flag} {old_flag}')

evidence = root / 'docs/asset/evidence/032-creature-react-clips.json'
evidence.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('REIMPORT RESULT ' + json.dumps({r['species']: [(c['name'], round(c['duration_seconds'], 3), c['replaced']) for c in r['clips']] for r in report['creatures']}))
