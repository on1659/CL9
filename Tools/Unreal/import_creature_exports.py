"""Import authored creature files with proven options; protect existing unrelated assets."""
import unreal,json,hashlib,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();at=unreal.AssetToolsHelpers.get_asset_tools()
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
assert unreal.SystemLibrary.get_engine_version().startswith('5.8.')
report={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'creatures':[]}
def import_file(source,destination,name,skeleton=None,fps=30):
    digest=hashlib.sha256(source.read_bytes()).hexdigest();path=destination+'/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing=unreal.load_asset(path)
        assert unreal.EditorAssetLibrary.get_metadata_tag(existing,'CL9SourceSHA256')==digest,'Changed source/existing asset: '+path
        return existing
    task=unreal.AssetImportTask();task.filename=str(source);task.destination_path=destination
    task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=False
    opt=unreal.FbxImportUI();opt.import_mesh=skeleton is None;opt.import_as_skeletal=True
    opt.import_animations=skeleton is not None;opt.import_materials=False;opt.import_textures=False
    opt.create_physics_asset=False;opt.automated_import_should_detect_type=False
    opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_ANIMATION if skeleton else unreal.FBXImportType.FBXIT_SKELETAL_MESH
    if skeleton:opt.skeleton=skeleton
    data=opt.anim_sequence_import_data if skeleton else opt.skeletal_mesh_import_data
    data.convert_scene=True;data.convert_scene_unit=True;data.force_front_x_axis=True;data.import_uniform_scale=1.0
    if skeleton:
        data.set_editor_property('use_default_sample_rate',fps==30)
        data.set_editor_property('custom_sample_rate',fps)
    else:data.vertex_color_import_option=unreal.VertexColorImportOption.REPLACE
    task.options=opt;at.import_asset_tasks([task]);asset=unreal.load_asset(path)
    assert asset is not None,list(task.imported_object_paths)
    unreal.EditorAssetLibrary.set_metadata_tag(asset,'CL9SourceSHA256',digest)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset
for species in globals().get('SPECIES',('Chicken','Raccoon')):
    source=root/'ArtSource/monster'/f'{species.lower()}_a/export'
    manifest=json.loads((source/'manifest.json').read_text());folder=f'/Game/Monsters/{species}A'
    mesh=import_file(source/(manifest['mesh']+'.fbx'),folder,manifest['mesh']);assert isinstance(mesh,unreal.SkeletalMesh)
    skeleton=mesh.get_editor_property('skeleton');material_path=folder+f'/M_CL9_{species}_Vertex'
    material=unreal.load_asset(material_path) if unreal.EditorAssetLibrary.does_asset_exist(material_path) else None
    if material is None:
        material=at.create_asset(f'M_CL9_{species}_Vertex',folder,unreal.Material,unreal.MaterialFactoryNew())
        color=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionVertexColor)
        assert unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
        rough=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.86)
        assert unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
        material.set_editor_property('used_with_skeletal_mesh',True)
        unreal.MaterialEditingLibrary.recompile_material(material);unreal.EditorAssetLibrary.save_loaded_asset(material)
    slots=mesh.get_editor_property('materials');assert len(slots)==1
    slot=slots[0];slot.material_interface=material;slots[0]=slot;mesh.set_editor_property('materials',slots)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    bound=mesh.get_imported_bounds();extent=bound.box_extent;dimensions=[extent.x*2,extent.y*2,extent.z*2]
    record={'species':species,'mesh':mesh.get_path_name(),'skeleton':skeleton.get_path_name(),'material':material.get_path_name(),
            'has_vertex_colors':mesh.has_vertex_colors(),'dimensions_cm':dimensions,
            'dimension_error_cm':max(abs(a-b) for a,b in zip(dimensions,manifest['dimensions_cm'])),'clips':[]}
    assert record['dimension_error_cm']<.01 and record['has_vertex_colors']
    for name,clip in manifest['clips'].items():
        animation=import_file(source/(clip['action']+'.fbx'),folder,clip['action'],skeleton,clip['fps'])
        assert isinstance(animation,unreal.AnimSequence)
        length=animation.get_editor_property('sequence_length')
        assert abs(length-clip['duration_seconds'])<.001,(name,length)
        assert animation.get_editor_property('skeleton')==skeleton
        record['clips'].append({'name':name,'path':animation.get_path_name(),'duration_seconds':length,
                                'source_sha256':unreal.EditorAssetLibrary.get_metadata_tag(animation,'CL9SourceSHA256')})
    report['creatures'].append(record)
    (root/'docs/asset/evidence'/globals().get('EVIDENCE','004-creature-imports.json')).write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report))
