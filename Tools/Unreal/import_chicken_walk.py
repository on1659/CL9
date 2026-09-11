"""Import one baked action on the imported skeleton and recreate its vertex material."""
import unreal,json,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();folder='/Game/CL9Art/Monsters/ChickenA'
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
mesh=unreal.load_asset(folder+'/SK_Chicken_A');skeleton=mesh.get_editor_property('skeleton')
source=root/'ArtSource/monster/chicken_a/export/AN_Chicken_Walk.fbx'
assert not unreal.EditorAssetLibrary.does_asset_exist(folder+'/AN_Chicken_Walk'),'Inspect existing animation before reimport'
task=unreal.AssetImportTask();task.filename=str(source);task.destination_path=folder
task.destination_name='AN_Chicken_Walk';task.automated=True;task.save=True;task.replace_existing=False
opt=unreal.FbxImportUI();opt.import_mesh=False;opt.import_as_skeletal=True;opt.import_animations=True
opt.import_materials=False;opt.import_textures=False;opt.automated_import_should_detect_type=False
opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_ANIMATION;opt.skeleton=skeleton
data=opt.anim_sequence_import_data
data.convert_scene=True;data.convert_scene_unit=True;data.force_front_x_axis=True;data.import_uniform_scale=1.0
task.options=opt;at=unreal.AssetToolsHelpers.get_asset_tools();at.import_asset_tasks([task])
animation=unreal.load_asset(folder+'/AN_Chicken_Walk')
assert isinstance(animation,unreal.AnimSequence),list(task.imported_object_paths)
unreal.EditorAssetLibrary.set_metadata_tag(animation,'CL9SourceSHA256',hashlib.sha256(source.read_bytes()).hexdigest())
unreal.EditorAssetLibrary.save_loaded_asset(animation)
material=unreal.load_asset(folder+'/M_CL9_Chicken_Vertex')
if material is None:
    material=at.create_asset('M_CL9_Chicken_Vertex',folder,unreal.Material,unreal.MaterialFactoryNew())
    color=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionVertexColor)
    assert unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
    rough=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant)
    rough.set_editor_property('r',.86)
    unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    material.set_editor_property('used_with_skeletal_mesh',True)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
slots=mesh.get_editor_property('materials');assert len(slots)==1
slot=slots[0];slot.set_editor_property('material_interface',material);slots[0]=slot
mesh.set_editor_property('materials',slots)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
result={'animation':animation.get_path_name(),'duration_seconds':animation.get_editor_property('sequence_length'),
        'skeleton':animation.get_editor_property('skeleton').get_path_name(),'material':material.get_path_name(),
        'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'imported':list(task.imported_object_paths)}
(root/'docs/asset/evidence/004-chicken-walk-import.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps(result))
