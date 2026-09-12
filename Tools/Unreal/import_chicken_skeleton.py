"""First representative skeletal import; never overwrite unrelated assets or levels."""
import unreal,json,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
assert unreal.SystemLibrary.get_engine_version().startswith('5.8.')
destination='/Game/Monsters/ChickenA'
source=root/'ArtSource/monster/chicken_a/export/SK_Chicken_A.fbx'
path=destination+'/SK_Chicken_A'
assert not unreal.EditorAssetLibrary.does_asset_exist(path),'Existing mesh: inspect before reimport'
task=unreal.AssetImportTask();task.filename=str(source);task.destination_path=destination
task.destination_name='SK_Chicken_A';task.automated=True;task.save=True;task.replace_existing=False
options=unreal.FbxImportUI();options.import_mesh=True;options.import_as_skeletal=True
options.import_materials=False;options.import_textures=False;options.import_animations=False
options.create_physics_asset=False;options.automated_import_should_detect_type=False
options.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH
data=options.skeletal_mesh_import_data
data.convert_scene=True;data.convert_scene_unit=True;data.force_front_x_axis=True;data.import_uniform_scale=1.0
data.vertex_color_import_option=unreal.VertexColorImportOption.REPLACE
task.options=options
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(path)
assert isinstance(mesh,unreal.SkeletalMesh),list(task.imported_object_paths)
unreal.EditorAssetLibrary.set_metadata_tag(mesh,'CL9SourceSHA256',hashlib.sha256(source.read_bytes()).hexdigest())
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
report={'engine':unreal.SystemLibrary.get_engine_version(),'mesh':mesh.get_path_name(),
        'skeleton':mesh.get_editor_property('skeleton').get_path_name(),'imported':list(task.imported_object_paths),
        'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
        'options':{'convert_scene':True,'convert_scene_unit':True,'force_front_x_axis':True,'scale':1}}
(root/'docs/asset/evidence/004-chicken-first-import.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report))
