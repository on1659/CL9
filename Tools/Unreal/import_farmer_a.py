"""Import authored meshes onto the existing Manny Skeleton, without updating its reference pose."""
import unreal,json,hashlib,math
from pathlib import Path
root=Path(unreal.Paths.project_dir());src=root/'ArtSource/character/farmer_a/export';manifest=json.loads((src/'manifest.json').read_text());folder='/Game/CL9Art/Characters/FarmerA'
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
sk=unreal.load_asset('/Game/Characters/Mannequins/Meshes/SK_Mannequin');at=unreal.AssetToolsHelpers.get_asset_tools()
def skeleton_signature():
 p=sk.get_reference_pose();rows=[]
 for n in unreal.AnimPoseExtensions.get_bone_names(p):
  t=unreal.AnimPoseExtensions.get_bone_pose(p,n,unreal.AnimPoseSpaces.LOCAL);q=t.rotation
  rows.append([str(n),list(t.translation.to_tuple()),[q.x,q.y,q.z,q.w],list(t.scale3d.to_tuple())])
 return hashlib.sha256(json.dumps(rows).encode()).hexdigest()
before=skeleton_signature();refs=json.loads((root/'ArtSource/character/_reference/ue-reference-pose.json').read_text())['bones']
old_report_path=root/'docs/asset/evidence/024-farmer-imports.json';old_report=json.loads(old_report_path.read_text()) if old_report_path.exists() else None
made={};report={'engine':unreal.SystemLibrary.get_engine_version(),'skeleton':sk.get_path_name(),'reference_signature_before':before,'assets':[]}
for key in ['Cloth','Detail','Skin']:
 name='M_CL9_Farmer_'+key;path=folder+'/'+name
 if unreal.EditorAssetLibrary.does_asset_exist(path):
  mat=unreal.load_asset(path);assert unreal.EditorAssetLibrary.get_metadata_tag(mat,'CL9ProductionBatch')=='024'
  if globals().get('REIMPORT_OWNED'):made[name]=mat;continue
  unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
 else:
  mat=at.create_asset(name,folder,unreal.Material,unreal.MaterialFactoryNew());unreal.EditorAssetLibrary.set_metadata_tag(mat,'CL9ProductionBatch','024')
 mat.set_editor_property('used_with_skeletal_mesh',True)
 lib=unreal.MaterialEditingLibrary
 if key=='Skin':
  expr=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);expr.set_editor_property('constant',unreal.LinearColor(.48,.30,.19,1))
 elif key=='Detail':expr=lib.create_material_expression(mat,unreal.MaterialExpressionVertexColor)
 else:
  vc=lib.create_material_expression(mat,unreal.MaterialExpressionVertexColor);tint=lib.create_material_expression(mat,unreal.MaterialExpressionVectorParameter);tint.set_editor_property('parameter_name','TintColor');tint.set_editor_property('default_value',unreal.LinearColor(.13,.20,.08,1))
  expr=lib.create_material_expression(mat,unreal.MaterialExpressionMultiply);assert lib.connect_material_expressions(vc,'',expr,'A');assert lib.connect_material_expressions(tint,'',expr,'B')
 assert lib.connect_material_property(expr,'',unreal.MaterialProperty.MP_BASE_COLOR)
 rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.88);assert lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
 lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat);made[name]=mat
for name,meta in manifest['assets'].items():
 path=folder+'/'+name;exists=unreal.EditorAssetLibrary.does_asset_exist(path)
 f=src/(name+'.fbx');digest=hashlib.sha256(f.read_bytes()).hexdigest();assert digest==meta['fbx_sha256']
 if exists:
  assert globals().get('REIMPORT_OWNED') and old_report
  previous=next(x for x in old_report['assets'] if x['name']==name)
  assert unreal.EditorAssetLibrary.get_metadata_tag(unreal.load_asset(path),'CL9SourceSHA256')==previous['sha256']
 t=unreal.AssetImportTask();t.filename=str(f);t.destination_path=folder;t.destination_name=name;t.automated=True;t.save=True;t.replace_existing=exists;t.replace_existing_settings=True;t.factory=unreal.FbxFactory()
 o=unreal.FbxImportUI();o.skeleton=sk;o.import_mesh=True;o.import_as_skeletal=True;o.import_animations=False;o.import_materials=False;o.import_textures=False;o.create_physics_asset=False;o.automated_import_should_detect_type=False;o.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH
 d=o.skeletal_mesh_import_data;d.convert_scene=True;d.convert_scene_unit=True;d.force_front_x_axis=False;d.import_uniform_scale=1;d.vertex_color_import_option=unreal.VertexColorImportOption.REPLACE;d.set_editor_property('update_skeleton_reference_pose',False);d.set_editor_property('use_t0_as_ref_pose',False);t.options=o
 d.set_editor_property('normal_import_method',unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
 at.import_asset_tasks([t]);mesh=unreal.load_asset(path);assert isinstance(mesh,unreal.SkeletalMesh),list(t.imported_object_paths);assert mesh.skeleton==sk
 slots=mesh.materials
 for i,slot in enumerate(slots):slot.material_interface=made[str(slot.material_slot_name)];slots[i]=slot
 mesh.set_editor_property('materials',slots);assert all(slot.material_interface==made[str(slot.material_slot_name)] for slot in mesh.materials)
 unreal.EditorAssetLibrary.set_metadata_tag(mesh,'CL9SourceSHA256',digest);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 mod=unreal.SkeletonModifier();assert mod.set_skeletal_mesh(mesh);names=list(map(str,mod.get_all_bone_names()));assert len(names)==89
 max_pos=max_rot=max_scale=0
 for b in refs:
  n=b['name'];assert n in names and str(mesh.get_bone_parent(n))==b['parent'];p=mod.get_bone_transform(n,True);q=p.rotation;e=b['component']
  max_pos=max(max_pos,math.dist(p.translation.to_tuple(),e['translation']));dot=abs(sum(a*c for a,c in zip([q.x,q.y,q.z,q.w],e['rotation_xyzw'])));max_rot=max(max_rot,math.degrees(2*math.acos(min(1,dot))));max_scale=max(max_scale,max(abs(x-1) for x in p.scale3d.to_tuple()))
 dims=(mesh.get_imported_bounds().box_extent*2).to_tuple();err=max(abs(a-b) for a,b in zip(dims,meta['dimensions_cm']))
 r={'name':name,'path':path,'sha256':digest,'dimensions_cm':dims,'dimension_error_cm':err,'bone_count':len(names),'max_reference_position_error_cm':max_pos,'max_reference_rotation_error_deg':max_rot,'max_bone_scale_error':max_scale,'materials':[slot.material_interface.get_path_name() for slot in mesh.materials],'vertex_colors':mesh.has_vertex_colors()}
 assert err<.01 and max_pos<.01 and max_rot<.1 and max_scale<.0001
 report['assets'].append(r)
assert before==skeleton_signature(),'Existing Skeleton reference pose changed'
report['reference_signature_after']=skeleton_signature();report['existing_skeleton_unchanged']=True
(root/'docs/asset/evidence/024-farmer-imports.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report))
