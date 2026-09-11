import unreal,json
from pathlib import Path
rows=[]
for path in unreal.EditorAssetLibrary.list_assets('/Game/Characters/Mannequins/Anims/Unarmed',recursive=True,include_folder=False):
 a=unreal.EditorAssetLibrary.find_asset_data(path)
 if str(a.asset_class_path.asset_name)=='AnimSequence':rows.append(path)
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/024-existing-locomotion.json');out.write_text(json.dumps(rows,indent=2),encoding='utf-8');print(json.dumps(rows))
