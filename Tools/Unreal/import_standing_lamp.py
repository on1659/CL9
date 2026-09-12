SOURCE_REL='ArtSource/item/standing_lamp_a/export'
DESTINATION='/Game/Items/StandingLampA'
EVIDENCE='012-standing-lamp-imports.json'
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_portable_lights.py',encoding='utf-8').read(),'import_portable_lights.py','exec'))
# Two reviewable states for the same shader contract; no new item/gameplay row.
states=[];parent=unreal.load_asset(DESTINATION+'/M_CL9_StandingLamp_Diffuser')
for label,value in [('Off',0),('On',4)]:
    name='MI_StandingLamp_'+label;path=DESTINATION+'/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        instance=unreal.load_asset(path);assert unreal.EditorAssetLibrary.get_metadata_tag(instance,'CL9Owner')=='StandingLampA'
    else:
        instance=at.create_asset(name,DESTINATION,unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
        unreal.EditorAssetLibrary.set_metadata_tag(instance,'CL9Owner','StandingLampA')
    unreal.MaterialEditingLibrary.set_material_instance_parent(instance,parent)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(instance,'LightStrength',value)
    actual=unreal.MaterialEditingLibrary.get_material_instance_scalar_parameter_value(instance,'LightStrength');assert abs(actual-value)<.0001
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    states.append({'state':label,'path':instance.get_path_name(),'LightStrength':actual})
report['material_states']=states;report['gameplay_status']='No standing_lamp item/equipment runtime row. No gameplay QA claim.'
evidence.write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(states))
