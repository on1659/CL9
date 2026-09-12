import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());folder='/Game/Characters/FarmerA';at=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
assert not unreal.EditorAssetLibrary.does_asset_exist(folder+'/T_Farmer_Palette')
t=unreal.AssetImportTask();t.filename=str(root/'ArtSource/character/farmer_a/textures/T_Farmer_Palette.png');t.destination_path=folder;t.destination_name='T_Farmer_Palette';t.automated=True;t.save=True;at.import_asset_tasks([t]);texture=unreal.load_asset(folder+'/T_Farmer_Palette');assert isinstance(texture,unreal.Texture2D)
texture.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST);texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS);texture.set_editor_property('srgb',True);texture.set_editor_property('never_stream',True);unreal.EditorAssetLibrary.save_loaded_asset(texture)
for key in ['Cloth','Detail']:
 mat=unreal.load_asset(folder+'/M_CL9_Farmer_'+key);assert unreal.EditorAssetLibrary.get_metadata_tag(mat,'CL9ProductionBatch')=='024';lib.delete_all_material_expressions(mat)
 sample=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample);sample.set_editor_property('texture',texture)
 uv=lib.create_material_expression(mat,unreal.MaterialExpressionTextureCoordinate);uv.set_editor_property('coordinate_index',1);assert lib.connect_material_expressions(uv,'',sample,'UVs')
 expr=sample
 if key=='Cloth':
  tint=lib.create_material_expression(mat,unreal.MaterialExpressionVectorParameter);tint.set_editor_property('parameter_name','TintColor');tint.set_editor_property('default_value',unreal.LinearColor(.13,.20,.08,1))
  expr=lib.create_material_expression(mat,unreal.MaterialExpressionMultiply);assert lib.connect_material_expressions(sample,'RGB',expr,'A');assert lib.connect_material_expressions(tint,'',expr,'B')
 assert lib.connect_material_property(expr,'RGB' if key=='Detail' else '',unreal.MaterialProperty.MP_BASE_COLOR)
 rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.88);assert lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
 lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
e=root/'docs/asset/evidence/024-farmer-imports.json';e.with_name('024-farmer-imports-before-palette.json').write_text(e.read_text(),encoding='utf-8')
flag='Interchange.FeatureFlags.Import.FBX';old=unreal.SystemLibrary.get_console_variable_int_value(flag);w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
try:
 unreal.SystemLibrary.execute_console_command(w,flag+' 0');REIMPORT_OWNED=True
 exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_farmer_a.py',encoding='utf-8').read(),'import_farmer_a.py','exec'))
finally:unreal.SystemLibrary.execute_console_command(w,flag+' '+str(old))
print('Imported shared palette/UV1 and authored flat normals; restored Interchange flag.')
