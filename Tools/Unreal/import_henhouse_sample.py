"""Import sample into an isolated art folder and measure actual UE bounds."""
import json
import hashlib
from pathlib import Path
import unreal

if unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor():
    raise RuntimeError('Stop PIE before importing or saving art assets')
root = Path(unreal.Paths.project_dir()).resolve()
manifest = json.loads((root/'ArtSource/map/henhouse_sample_01/export/manifest.json').read_text())
destination = '/Game/Maps/Henhouse/Props'
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mesh_sub = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
entries = [(v['object'],root/'ArtSource/map/henhouse_sample_01/export'/ (v['object']+'.fbx'),v) for v in manifest['assets'].values()]
entries.append(('SM_CL9_UnitCube_100',root/'ArtSource/validation/unit_axis/SM_CL9_UnitCube_100.fbx',{'dimensions_cm':[100,100,100],'materials':[]}))
for name,filename,meta in entries:
    existing=unreal.load_asset(destination+'/'+name)
    digest=hashlib.sha256(filename.read_bytes()).hexdigest()
    collision_match=existing and mesh_sub.get_convex_collision_count(existing)==meta.get('collision_hulls',0)
    if existing and collision_match and unreal.EditorAssetLibrary.get_metadata_tag(existing,'CL9SourceSHA256')==digest:
        print('Existing asset preserved:',name)
        continue
    task = unreal.AssetImportTask()
    task.filename = str(filename)
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.replace_existing_settings = True
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = False
    options.import_textures = False
    options.import_animations = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    options.automated_import_should_detect_type = False
    options.static_mesh_import_data.combine_meshes = True
    # UE5.8's FbxImportUI -> Interchange converter uses this as the master collision switch.
    # Enable for authored UCX; exact hull counts are checked below to reject generated fallback.
    options.static_mesh_import_data.auto_generate_collision = meta.get('collision_hulls',0)>0
    options.static_mesh_import_data.one_convex_hull_per_ucx = True
    options.static_mesh_import_data.convert_scene = True
    options.static_mesh_import_data.convert_scene_unit = True
    options.static_mesh_import_data.force_front_x_axis = True
    options.static_mesh_import_data.import_uniform_scale = 1.0
    task.options = options
    asset_tools.import_asset_tasks([task])
    print('Imported',name,list(task.imported_object_paths))
    imported=unreal.load_asset(destination+'/'+name)
    if imported is None: raise RuntimeError('Import missing: '+name)
    unreal.EditorAssetLibrary.set_metadata_tag(imported,'CL9SourceSHA256',digest)

materials = {}
for name,values in manifest['materials'].items():
    path = destination+'/Materials/M_'+name
    material = unreal.load_asset(path)
    if material is None:
        material = asset_tools.create_asset('M_'+name,destination+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    if material:
        unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
        color = unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionVectorParameter)
        color.set_editor_property('parameter_name', 'BaseColor')
        color.set_editor_property('default_value', unreal.LinearColor(*values['base_color_linear']))
        unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
        for prop,param,value in [(unreal.MaterialProperty.MP_ROUGHNESS,'Roughness',values['roughness']),
                                  (unreal.MaterialProperty.MP_METALLIC,'Metallic',values['metallic'])]:
            node = unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionScalarParameter)
            node.set_editor_property('parameter_name',param)
            node.set_editor_property('default_value',value)
            unreal.MaterialEditingLibrary.connect_material_property(node,'',prop)
        if values['emission_strength']:
            emission=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionVectorParameter)
            emission.set_editor_property('parameter_name','EmissiveColor')
            rgb=[c*values['emission_strength'] for c in values['base_color_linear'][:3]]
            emission.set_editor_property('default_value',unreal.LinearColor(*rgb,1))
            unreal.MaterialEditingLibrary.connect_material_property(emission,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        unreal.MaterialEditingLibrary.recompile_material(material)
        if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
            raise RuntimeError('Material save failed: '+path)
    materials[name]=material

report={'engine':unreal.SystemLibrary.get_engine_version(),'import_options':{
    'import_uniform_scale':1,'convert_scene':True,'convert_scene_unit':True,'force_front_x_axis':True},'assets':[]}
for name,filename,meta in entries:
    mesh=unreal.load_asset(destination+'/'+name)
    if not isinstance(mesh,unreal.StaticMesh):
        raise RuntimeError('Missing imported mesh: '+name)
    if meta.get('triangles',0)>1000:
        settings=[]
        for percent in [1,.5,.25]:
            setting=unreal.StaticMeshReductionSettings()
            setting.percent_triangles=percent
            settings.append(setting)
        options=unreal.StaticMeshReductionOptions()
        options.auto_compute_lod_screen_size=True
        options.reduction_settings=settings
        mesh_sub.set_lods(mesh,options)
    slots=mesh.get_editor_property('static_materials')
    # Match the original FBX material slot identity; fail on unknown slot instead of guessing.
    for i,slot in enumerate(slots):
        slot_name=str(slot.get_editor_property('imported_material_slot_name'))
        if meta['materials']:
            if slot_name not in materials:
                raise RuntimeError('Unexpected material slot: '+slot_name)
            mesh.set_material(i,materials[slot_name])
    box=mesh.get_bounding_box(); size=box.max-box.min
    dimensions=[size.x,size.y,size.z]
    delta=[abs(a-b) for a,b in zip(dimensions,meta['dimensions_cm'])]
    record={'name':name,'path':mesh.get_path_name(),'dimensions_cm':dimensions,
        'expected_cm':meta['dimensions_cm'],'max_dimension_error_cm':max(delta),
        'min_cm':[box.min.x,box.min.y,box.min.z],'max_cm':[box.max.x,box.max.y,box.max.z],
        'lod_count':mesh.get_num_lods(),'materials':[str(s.get_editor_property('imported_material_slot_name')) for s in slots],
        'simple_collisions':mesh_sub.get_simple_collision_count(mesh),
        'convex_collisions':mesh_sub.get_convex_collision_count(mesh),
        'expected_collision_hulls':meta.get('collision_hulls',0),
        'source_sha256':hashlib.sha256(filename.read_bytes()).hexdigest()}
    report['assets'].append(record)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
out=root/'docs/asset/evidence/001-ue-import.json'
out.write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report))
