import unreal
folder='/Game/CL9Art/Monsters/ChickenA';mat=unreal.load_asset(folder+'/M_CL9_Chicken_Vertex')
node=unreal.MaterialEditingLibrary.get_material_property_input_node(mat,unreal.MaterialProperty.MP_BASE_COLOR)
print('Before BaseColor input',node)
if node is None:
    unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
    node=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionVertexColor)
    assert unreal.MaterialEditingLibrary.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
    rough=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.86)
    assert unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
mat.set_editor_property('used_with_skeletal_mesh',True)
unreal.MaterialEditingLibrary.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat)
print('After BaseColor input',unreal.MaterialEditingLibrary.get_material_property_input_node(mat,unreal.MaterialProperty.MP_BASE_COLOR))
