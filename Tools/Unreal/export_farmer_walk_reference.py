import unreal,json,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir());f=root/'ArtSource/character/_reference/existing_walk.fbx';assert not f.exists()
a=unreal.load_asset('/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd');assert isinstance(a,unreal.AnimSequence)
t=unreal.AssetExportTask();t.object=a;t.filename=str(f);t.automated=True;t.prompt=False;t.replace_identical=False;t.options=unreal.FbxExportOption()
assert unreal.Exporter.run_asset_export_task(t) and f.exists()
r={'animation':a.get_path_name(),'duration_seconds':a.sequence_length,'sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'reference_only':True}
(root/'ArtSource/character/_reference/existing_walk.json').write_text(json.dumps(r,indent=2),encoding='utf-8');print(json.dumps(r))
