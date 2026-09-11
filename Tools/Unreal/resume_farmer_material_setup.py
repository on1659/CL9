"""Resume the one partially-created owned material after a Python property accessor error."""
import unreal
path='/Game/CL9Art/Characters/FarmerA/M_CL9_Farmer_Cloth';mat=unreal.load_asset(path);assert isinstance(mat,unreal.Material)
assert not unreal.EditorAssetLibrary.get_metadata_tag(mat,'CL9ProductionBatch')
unreal.EditorAssetLibrary.set_metadata_tag(mat,'CL9ProductionBatch','024')
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_farmer_a.py',encoding='utf-8').read(),'import_farmer_a.py','exec'))
