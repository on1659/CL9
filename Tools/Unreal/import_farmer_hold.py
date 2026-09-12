"""농부 A Hold 클립 6개(전신 3 + 1인칭 팔 3)를 Manny 스켈레톤에 애니메이션으로 임포트한다 — 레거시 FBX 경로."""
import unreal, json, hashlib
from pathlib import Path

root = Path(unreal.Paths.project_dir())
src = root / 'ArtSource/character/farmer_a/export'
manifest = json.loads((root / 'ArtSource/character/farmer_a/manifest.json').read_text(encoding='utf-8'))
folder = '/Game/Characters/FarmerA/Anims'
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
sk = unreal.load_asset('/Game/Characters/Mannequins/Meshes/SK_Mannequin')
flag = 'Interchange.FeatureFlags.Import.FBX'
old = unreal.SystemLibrary.get_console_variable_int_value(flag)
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
report = {'engine': unreal.SystemLibrary.get_engine_version(), 'skeleton': sk.get_path_name(), 'clips': []}
try:
    unreal.SystemLibrary.execute_console_command(w, flag + ' 0')
    tasks = []
    for name, meta in manifest['animations'].items():
        f = src / (name + '.fbx')
        digest = hashlib.sha256(f.read_bytes()).hexdigest()
        assert digest == meta['fbx_sha256'], name
        task = unreal.AssetImportTask()
        task.filename = str(f)
        task.destination_path = folder
        task.destination_name = name
        task.automated = True
        task.save = True
        task.replace_existing = True
        opt = unreal.FbxImportUI()
        opt.import_mesh = False
        opt.import_as_skeletal = True
        opt.import_animations = True
        opt.import_materials = False
        opt.import_textures = False
        opt.automated_import_should_detect_type = False
        opt.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
        opt.skeleton = sk
        data = opt.anim_sequence_import_data
        data.convert_scene = True
        data.convert_scene_unit = True
        data.force_front_x_axis = False
        data.import_uniform_scale = 1.0
        task.options = opt
        tasks.append((name, task, digest))
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t for _, t, _ in tasks])
    for name, task, digest in tasks:
        anim = unreal.load_asset(folder + '/' + name)
        assert isinstance(anim, unreal.AnimSequence), (name, list(task.imported_object_paths))
        unreal.EditorAssetLibrary.set_metadata_tag(anim, 'CL9SourceSHA256', digest)
        unreal.EditorAssetLibrary.save_loaded_asset(anim)
        report['clips'].append({'name': name, 'path': anim.get_path_name(), 'seconds': anim.get_editor_property('sequence_length'),
                                'frames': anim.get_editor_property('number_of_sampled_frames'), 'sha256': digest})
finally:
    unreal.SystemLibrary.execute_console_command(w, flag + ' ' + str(old))
(root / 'docs/asset/evidence/025-farmer-hold-imports.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print(json.dumps(report))
