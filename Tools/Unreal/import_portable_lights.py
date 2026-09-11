"""Import a static prop manifest; defaults to portable lights, accepts explicit kit configuration."""
import unreal,json,hashlib,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();source=root/globals().get('SOURCE_REL','ArtSource/item/portable_lights/export')
assert unreal.SystemLibrary.get_engine_version().startswith('5.8.')
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
m=json.loads((source/'manifest.json').read_text());folder=globals().get('DESTINATION','/Game/CL9Art/Items/PortableLights')
evidence=root/'docs/asset/evidence'/globals().get('EVIDENCE','009-portable-lights-imports.json')
oldreport=json.loads(evidence.read_text()) if evidence.exists() else None
if globals().get('ALLOW_OWNED_REIMPORT') and oldreport:
    previous=evidence.with_name(evidence.stem+'-before-fit.json')
    if not previous.exists():previous.write_text(evidence.read_text(),encoding='utf-8')
at=unreal.AssetToolsHelpers.get_asset_tools();ms=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
materials={}
for name,meta in m['materials'].items():
    path=folder+'/'+name
    mat=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if mat is None:
        mat=at.create_asset(name,folder,unreal.Material,unreal.MaterialFactoryNew())
        if meta['vertex_color']:
            color=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionVertexColor)
        else:
            color=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant4Vector)
            color.set_editor_property('constant',unreal.LinearColor(*meta['base_color']))
        assert unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
        for key,prop in [('roughness',unreal.MaterialProperty.MP_ROUGHNESS),('metallic',unreal.MaterialProperty.MP_METALLIC)]:
            value=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant)
            value.set_editor_property('r',meta[key]);assert unreal.MaterialEditingLibrary.connect_material_property(value,'',prop)
        if 'emission_color' in meta:
            emission=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant4Vector)
            emission.set_editor_property('constant',unreal.LinearColor(*meta['emission_color']))
            strength=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionScalarParameter)
            strength.set_editor_property('parameter_name',meta['emission_parameter']);strength.set_editor_property('default_value',meta.get('emission_default',0))
            multiply=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionMultiply)
            assert unreal.MaterialEditingLibrary.connect_material_expressions(emission,'',multiply,'A')
            assert unreal.MaterialEditingLibrary.connect_material_expressions(strength,'',multiply,'B')
            assert unreal.MaterialEditingLibrary.connect_material_property(multiply,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
    materials[name]=mat
report={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'assets':[]}
for key,meta in m['assets'].items():
    name=meta['object'];path=folder+'/'+name;fbx=source/(name+'.fbx');digest=hashlib.sha256(fbx.read_bytes()).hexdigest()
    assert digest==meta['fbx_sha256']
    exists=unreal.EditorAssetLibrary.does_asset_exist(path);reimport=False
    if exists:
        mesh=unreal.load_asset(path);oldhash=unreal.EditorAssetLibrary.get_metadata_tag(mesh,'CL9SourceSHA256')
        reimport=oldhash!=digest
        if reimport:
            assert globals().get('ALLOW_OWNED_REIMPORT') and oldreport
            assert oldhash==next(r['source_sha256'] for r in oldreport['assets'] if r['id']==key),'Unrecognized previous asset'
    if not exists or reimport:
        t=unreal.AssetImportTask();t.filename=str(fbx);t.destination_path=folder;t.destination_name=name
        t.automated=True;t.save=True;t.replace_existing=reimport;t.replace_existing_settings=reimport
        o=unreal.FbxImportUI();o.import_mesh=True;o.import_as_skeletal=False;o.import_animations=False
        o.import_materials=False;o.import_textures=False;o.automated_import_should_detect_type=False
        o.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
        d=o.static_mesh_import_data;d.combine_meshes=True;d.auto_generate_collision=meta.get('collision_hulls',0)>0
        d.one_convex_hull_per_ucx=True
        d.convert_scene=True;d.convert_scene_unit=True;d.force_front_x_axis=True;d.import_uniform_scale=1
        d.vertex_color_import_option=unreal.VertexColorImportOption.REPLACE
        t.options=o;at.import_asset_tasks([t]);mesh=unreal.load_asset(path);assert isinstance(mesh,unreal.StaticMesh)
        unreal.EditorAssetLibrary.set_metadata_tag(mesh,'CL9SourceSHA256',digest)
    slots=mesh.get_editor_property('static_materials');assert len(slots)==len(meta['materials'])
    for i,slot in enumerate(slots):
        slotname=str(slot.get_editor_property('imported_material_slot_name'));assert slotname in materials
        mesh.set_material(i,materials[slotname])
    if mesh.get_num_lods()==1 or reimport:
        opts=unreal.StaticMeshReductionOptions();settings=[]
        for pct in (1,.5,.25):
            st=unreal.StaticMeshReductionSettings();st.percent_triangles=pct;settings.append(st)
        opts.reduction_settings=settings;opts.auto_compute_lod_screen_size=True;ms.set_lods(mesh,opts)
    box=mesh.get_bounding_box();dims=list((box.max-box.min).to_tuple());center=(box.max+box.min)*.5
    radius=mesh.get_bounds().sphere_radius;runtime_radius=meta.get('runtime_radius_cm',12)
    scale=runtime_radius/radius if runtime_radius is not None else None
    error=max(abs(a-b) for a,b in zip(dims,meta['dimensions_cm']))
    expected_center=meta.get('bounds_center_cm',[0,0,0]);expected_center=[v*(-1 if i==1 else 1) for i,v in enumerate(expected_center)]
    assert error<.01 and max(abs(c-e) for c,e in zip(center.to_tuple(),expected_center))<.01
    hulls=ms.get_convex_collision_count(mesh);assert hulls==meta.get('collision_hulls',0)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    report['assets'].append({'id':key,'path':mesh.get_path_name(),'dimensions_cm':dims,'max_dimension_error_cm':error,
        'source_sha256':digest,'lod_count':mesh.get_num_lods(),'convex_collision_hulls':hulls,'bounds_center_cm':list(center.to_tuple()),'materials':[mesh.get_material(i).get_path_name() for i in range(len(slots))],
        'sphere_radius_cm':radius,'runtime_radius_cm':runtime_radius,'runtime_scale':scale,
        'runtime_dimensions_cm':[v*scale for v in dims] if scale is not None else None,
        'runtime_lens_center_cm':[meta['lens_center_cm'][i]*scale*(-1 if i==1 else 1) for i in range(3)] if 'lens_center_cm' in meta and scale is not None else None,
        'predicted_sphere_bottom_gap_cm':runtime_radius-dims[2]*.5*scale if scale is not None else None,
        'actual_gameplay_qa':'not tested in import step'})
evidence.write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report))
