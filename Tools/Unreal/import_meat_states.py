"""Import only owned meat assets, measure runtime scaling, and generate three LODs."""
import unreal,json,hashlib,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.SystemLibrary.get_engine_version().startswith('5.8.')
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
source=root/'ArtSource/item/meat_states/export';manifest=json.loads((source/'manifest.json').read_text())
folder='/Game/Items/MeatStates'
at=unreal.AssetToolsHelpers.get_asset_tools();ms=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
report={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'engine':unreal.SystemLibrary.get_engine_version(),
        'scope':'Isolated asset import/LOD/bounds. Runtime data and gameplay untested.','assets':[]}
for state,meta in manifest['assets'].items():
    name=meta['object'];path=folder+'/'+name;fbx=source/(name+'.fbx')
    digest=hashlib.sha256(fbx.read_bytes()).hexdigest();assert digest==meta['fbx_sha256']
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mesh=unreal.load_asset(path)
        assert unreal.EditorAssetLibrary.get_metadata_tag(mesh,'CL9SourceSHA256')==digest,'Existing source changed'
    else:
        t=unreal.AssetImportTask();t.filename=str(fbx);t.destination_path=folder;t.destination_name=name
        t.automated=True;t.save=True;t.replace_existing=False
        o=unreal.FbxImportUI();o.import_mesh=True;o.import_as_skeletal=False;o.import_animations=False
        o.import_materials=False;o.import_textures=False;o.automated_import_should_detect_type=False
        o.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
        d=o.static_mesh_import_data;d.combine_meshes=True;d.auto_generate_collision=False
        d.convert_scene=True;d.convert_scene_unit=True;d.force_front_x_axis=True;d.import_uniform_scale=1
        d.vertex_color_import_option=unreal.VertexColorImportOption.REPLACE
        t.options=o;at.import_asset_tasks([t]);mesh=unreal.load_asset(path)
        assert isinstance(mesh,unreal.StaticMesh)
        unreal.EditorAssetLibrary.set_metadata_tag(mesh,'CL9SourceSHA256',digest)
    mpath=folder+'/'+meta['material']
    mat=unreal.load_asset(mpath) if unreal.EditorAssetLibrary.does_asset_exist(mpath) else None
    if mat is None:
        mat=at.create_asset(meta['material'],folder,unreal.Material,unreal.MaterialFactoryNew())
        vc=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionVertexColor)
        assert unreal.MaterialEditingLibrary.connect_material_property(vc,'',unreal.MaterialProperty.MP_BASE_COLOR)
        rough=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant)
        rough.set_editor_property('r',meta['roughness'])
        assert unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
        unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
    assert len(mesh.get_editor_property('static_materials'))==1;mesh.set_material(0,mat)
    if mesh.get_num_lods()==1:
        opts=unreal.StaticMeshReductionOptions();settings=[]
        for percent in (1,.5,.25):
            st=unreal.StaticMeshReductionSettings();st.percent_triangles=percent;settings.append(st)
        opts.reduction_settings=settings;opts.auto_compute_lod_screen_size=True;ms.set_lods(mesh,opts)
    box=mesh.get_bounding_box();size=box.max-box.min;center=(box.max+box.min)*.5
    dims=[size.x,size.y,size.z];radius=mesh.get_bounds().sphere_radius;scale=30/radius
    error=max(abs(a-b) for a,b in zip(dims,meta['dimensions_cm']))
    assert error<.01 and max(abs(center.x),abs(center.y),abs(center.z))<.01
    assert mesh.get_num_lods()==3 and ms.get_simple_collision_count(mesh)==0
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    report['assets'].append({'state':state,'path':mesh.get_path_name(),'dimensions_cm':dims,'dimension_error_cm':error,
        'sphere_radius_cm':radius,'radius30_runtime_scale':scale,'runtime_dimensions_cm':[v*scale for v in dims],
        'lod_count':mesh.get_num_lods(),'material':mat.get_path_name(),'source_sha256':digest,
        'predicted_sphere_floor_gap_cm':30-size.z*.5*scale,'collision':'Actor SphereComponent; mesh collision disabled'})
assert max(a['sphere_radius_cm'] for a in report['assets'])-min(a['sphere_radius_cm'] for a in report['assets'])<.001
(root/'docs/asset/evidence/008-meat-imports.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report))
