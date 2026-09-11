import unreal
from pathlib import Path
m=unreal.load_asset('/Game/CL9Art/Characters/FarmerA/SK_Farmer_A_Body');d=m.get_editor_property('asset_import_data')
print('Import data class',d.get_class().get_name())
if isinstance(d,unreal.FbxSkeletalMeshImportData):
 for n in ['vertex_color_import_option','vertex_override_color']:print(n,d.get_editor_property(n))
f=Path(unreal.Paths.project_dir(),'ArtSource/character/_reference/farmer_ue_color_probe.fbx');assert not f.exists()
t=unreal.AssetExportTask();t.object=m;t.filename=str(f);t.automated=True;t.prompt=False;t.options=unreal.FbxExportOption();assert unreal.Exporter.run_asset_export_task(t)
print('Exported imported mesh for vertex-colour inspection.')
