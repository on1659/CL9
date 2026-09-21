"""다시 만든 동물 반응 클립(Catch·HitReact·PushReact·닭 PounceWindup·돼지/너구리 Execute — Tools/Blender/animate_creature_reacts.py)을 종 스켈레톤에 재임포트한다.

import_creature_exports.py 의 import_file 은 기존 에셋의 CL9SourceSHA256 이 소스와 다르면 assert 로 막는다(원본 보호). 이 스크립트는
manifest `clips` 항목에 `reworked` 표시가 있는 클립만 골라 덮어쓰고 sha 태그를 갱신한다 — 없는 클립(PushReact·PounceWindup·Execute)은
새로 만든다. 로코모션·Death 등 표시 없는 클립은 건드리지 않는다. 임포트 뒤 길이가 manifest duration_seconds 와 0.001초 넘게 다르면 실패.

**Interchange 로만 넣는다(2026-09-21)** — 옛 판은 `Interchange.FeatureFlags.Import.FBX` 를 끄고 레거시 FBX 경로(`force_front_x_axis`)로 넣었는데,
그렇게 들어간 클립은 런타임에서 **요가 −90° 돌아** 있었다(PIE 측정: 로코모션(원본 Interchange 임포트)은 머리 +X, 반응 클립은 −Y — 09-15 5종
Catch·HitReact·PushReact, 09-20 닭 PounceWindup, 09-21 Execute 전부). Blender·FBX 는 정상이라 임포트 경로 차이다. 기존 에셋이 있으면
Interchange 는 replace 가 아니라 다른 에셋을 만들므로 **임시 폴더(`<종>A/ReimportTmp`)에 임포트 → `consolidate_assets` 로 참조(ABP 평가기 등)를
새 에셋으로 교체 → 옛 자리의 리디렉터 삭제 → 원래 이름으로 rename → 저장**. 없으면 원래 자리에 바로 임포트. 반쯤 끝난 실행(임시 에셋 + 리디렉터)은
다음 실행이 이어받는다(`resumed`).

실행(에디터 파이썬): py -3 Tools/Unreal/run_python.py Tools/Unreal/reimport_creature_clips.py
  종 목록은 환경변수 CL9_SPECIES(쉼표, 기본 Chicken,Goose,Raccoon,Pig,Sheep), 증거 JSON 은 환경변수 CL9_EVIDENCE(저장소 상대 경로, 기본
  docs/asset/evidence/032-creature-react-clips.json — 새 클립 묶음은 새 번호로: 2026-09-21 Execute 는 034-execute-clips.json).
  클립 목록은 환경변수 CL9_CLIPS(쉼표, manifest 키 — 예 PounceWindup; 기본은 reworked 표시 전부). 클립 하나만 새로 넣을 때 바뀌지 않은
  Catch·HitReact·PushReact 에셋까지 다시 저장하지 않으려고 둔다(2026-09-20 닭 PounceWindup: `CL9_SPECIES=Chicken CL9_CLIPS=PounceWindup`;
  2026-09-21 처형: `CL9_SPECIES=Pig,Raccoon CL9_CLIPS=Execute CL9_EVIDENCE=docs/asset/evidence/034-execute-clips.json`).
  run_python.py 는 이미 떠 있는 에디터 프로세스 안에서 스크립트를 돌리므로 셸의 환경변수가 건너가지 않는다 — 에디터 파이썬(콘솔·MCP)에서
  먼저 `import os; os.environ['CL9_SPECIES'] = 'Chicken'; os.environ['CL9_CLIPS'] = 'PounceWindup'` 을 실행해 두고 돌린다.
"""
import unreal, json, hashlib, datetime, os
from pathlib import Path

root = Path(unreal.Paths.project_dir()).resolve()
at = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
species_list = [s.strip() for s in os.environ.get('CL9_SPECIES', 'Chicken,Goose,Raccoon,Pig,Sheep').split(',') if s.strip()]
clip_filter = {c.strip() for c in os.environ.get('CL9_CLIPS', '').split(',') if c.strip()}   # 비어 있으면 reworked 전부
report = {'engine': unreal.SystemLibrary.get_engine_version(), 'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
          'import_path': 'Interchange (legacy FBX path yaws clips -90 deg at runtime — 2026-09-21)', 'creatures': []}


def asset_class(path):
    return str(EAL.find_asset_data(path).asset_class_path.asset_name) if EAL.does_asset_exist(path) else None


def import_interchange(source, destination, name, skeleton):
    """Interchange 애니메이션 임포트(플래그 그대로) — 새 AnimSequence 를 돌려준다"""
    task = unreal.AssetImportTask()
    task.filename = str(source); task.destination_path = destination; task.destination_name = name
    task.automated = True; task.save = False; task.replace_existing = True
    opt = unreal.FbxImportUI()
    opt.import_mesh = False; opt.import_as_skeletal = True; opt.import_animations = True
    opt.import_materials = False; opt.import_textures = False; opt.create_physics_asset = False
    opt.automated_import_should_detect_type = False
    opt.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    opt.skeleton = skeleton
    task.options = opt
    at.import_asset_tasks([task])
    asset = unreal.load_asset(destination + '/' + name)
    assert isinstance(asset, unreal.AnimSequence), (name, list(task.imported_object_paths))
    return asset


def finish(path, source):
    """sha 메타태그 + 저장"""
    asset = unreal.load_asset(path)
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    EAL.set_metadata_tag(asset, 'CL9SourceSHA256', digest)
    EAL.save_loaded_asset(asset, only_if_is_dirty=False)
    return asset, digest


def import_clip(source, folder, name, skeleton, duration):
    """기존 에셋이 있으면 임시 폴더 임포트 → consolidate → 리디렉터 삭제 → rename(ABP 참조 유지); 없으면 바로. (에셋, sha, replaced, how)"""
    final = f'{folder}/{name}'; tmp_folder = f'{folder}/ReimportTmp'; tmp = f'{tmp_folder}/{name}'
    if asset_class(tmp) == 'AnimSequence' and asset_class(final) == 'ObjectRedirector':   # 지난 실행이 rename 전에 끊겼다
        assert EAL.delete_asset(final), ('redirector delete failed', final)
        assert EAL.rename_asset(tmp, final), ('rename failed', tmp)
        asset, digest = finish(final, source)
        return asset, digest, True, 'resumed'
    existed = EAL.does_asset_exist(final)
    new = import_interchange(source, tmp_folder if existed else folder, name, skeleton)
    assert new.get_editor_property('skeleton') == skeleton
    length = new.get_editor_property('sequence_length')
    assert abs(length - duration) < .001, (name, length)
    if existed:
        old = unreal.load_asset(final)
        assert EAL.consolidate_assets(new, [old]), ('consolidate failed', final)
        if EAL.does_asset_exist(final):   # consolidate 가 옛 자리에 리디렉터를 남긴다 — 지우고 이름을 돌려놓는다(참조는 이미 새 에셋)
            assert EAL.delete_asset(final), ('redirector delete failed', final)
        assert EAL.rename_asset(tmp, final), ('rename failed', tmp)
    asset, digest = finish(final, source)
    return asset, digest, existed, 'consolidated' if existed else 'new'


for species in species_list:
    source = root / 'ArtSource/monster' / f'{species.lower()}_a/export'
    manifest = json.loads((source / 'manifest.json').read_text())
    folder = f'/Game/Monsters/{species}A'
    mesh = unreal.load_asset(f'{folder}/{manifest["mesh"]}')
    assert isinstance(mesh, unreal.SkeletalMesh), species
    skeleton = mesh.get_editor_property('skeleton')
    record = {'species': species, 'skeleton': skeleton.get_path_name(), 'clips': []}
    if clip_filter:
        missing = clip_filter - set(manifest['clips'])
        assert not missing, (species, 'manifest 에 없는 클립', sorted(missing))
    for name, clip in manifest['clips'].items():
        if not clip.get('reworked') or (clip_filter and name not in clip_filter):
            continue
        animation, digest, existed, how = import_clip(source / (clip['action'] + '.fbx'), folder, clip['action'], skeleton, clip['duration_seconds'])
        length = animation.get_editor_property('sequence_length')
        assert abs(length - clip['duration_seconds']) < .001, (name, length)
        assert animation.get_editor_property('skeleton') == skeleton
        record['clips'].append({'name': name, 'path': animation.get_path_name(), 'duration_seconds': length, 'sha256': digest,
                                'replaced': existed, 'how': how,
                                'import_data': type(animation.get_editor_property('asset_import_data')).__name__})
    if EAL.does_directory_exist(f'{folder}/ReimportTmp'):
        EAL.delete_directory(f'{folder}/ReimportTmp')
    report['creatures'].append(record)

evidence = root / os.environ.get('CL9_EVIDENCE', 'docs/asset/evidence/032-creature-react-clips.json')
evidence.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('REIMPORT RESULT ' + json.dumps({r['species']: [(c['name'], round(c['duration_seconds'], 3), c['how']) for c in r['clips']] for r in report['creatures']}))
