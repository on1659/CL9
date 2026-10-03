"""종별 정점색 머티리얼 `M_CL9_<종>_Vertex` 를 만들어 `SK_<종>_A` 슬롯에 붙인다 (2026-09-20, 리드).

닭은 `fix_chicken_material.py` 로 이미 있고, 거위·너구리·돼지·양은 임포트 때 슬롯 이름만 남고 에셋이 없어 `WorldGridMaterial` 로 보였다.
구성은 닭과 같다 — VertexColor → BaseColor, Roughness 0.86, UsedWithSkeletalMesh. 멱등: 있으면 노드만 확인하고 다시 붙인다.
실행(에디터 켠 채): py -3 Tools/Unreal/run_python.py Tools/Unreal/create_creature_materials.py
"""
import unreal

SPECIES = ['Chicken', 'Goose', 'Raccoon', 'Pig', 'Sheep']
MEL = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()
report = []
for sp in SPECIES:
    folder = '/Game/Monsters/%sA' % sp
    name = 'M_CL9_%s_Vertex' % sp
    path = '%s/%s' % (folder, name)
    mat = unreal.load_asset(path)
    created = False
    if mat is None:
        mat = tools.create_asset(name, folder, unreal.Material, unreal.MaterialFactoryNew())
        created = True
    if MEL.get_material_property_input_node(mat, unreal.MaterialProperty.MP_BASE_COLOR) is None:
        MEL.delete_all_material_expressions(mat)
        vc = MEL.create_material_expression(mat, unreal.MaterialExpressionVertexColor, -300, 0)
        assert MEL.connect_material_property(vc, '', unreal.MaterialProperty.MP_BASE_COLOR)
        rough = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -300, 200)
        rough.set_editor_property('r', .86)
        assert MEL.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)
    mat.set_editor_property('used_with_skeletal_mesh', True)
    MEL.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat, only_if_is_dirty=False)

    mesh = unreal.load_asset('%s/SK_%s_A' % (folder, sp))
    assert mesh is not None, sp
    mats = list(mesh.get_editor_property('materials'))
    changed = False
    for i, slot in enumerate(mats):
        cur = slot.get_editor_property('material_interface')
        if cur is None or cur.get_path_name() != mat.get_path_name():
            slot.set_editor_property('material_interface', mat)
            mats[i] = slot
            changed = True
    if changed:
        mesh.modify()
        mesh.set_editor_property('materials', mats)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    report.append((sp, 'created' if created else 'existed', 'assigned' if changed else 'kept', len(mats)))
unreal.log('CREATURE MATERIALS %s' % report)
