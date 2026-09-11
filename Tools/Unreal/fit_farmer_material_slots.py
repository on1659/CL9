import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());p=root/'docs/asset/evidence/024-farmer-imports.json';r=json.loads(p.read_text())
for row in r['assets']:
 mesh=unreal.load_asset(row['path']);assert unreal.EditorAssetLibrary.get_metadata_tag(mesh,'CL9SourceSHA256')==row['sha256'];slots=mesh.get_editor_property('materials')
 for i,slot in enumerate(slots):
  mat=unreal.load_asset('/Game/CL9Art/Characters/FarmerA/'+str(slot.material_slot_name));assert mat
  slot.material_interface=mat;slots[i]=slot
 mesh.set_editor_property('materials',slots);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 row['materials']=[slot.material_interface.get_path_name() for slot in mesh.materials];assert all('/Game/CL9Art/Characters/FarmerA/' in x for x in row['materials'])
r['material_slot_fit']='Assigned struct copies back to array indices and saved only the two owned meshes.';p.write_text(json.dumps(r,indent=2),encoding='utf-8');print('Verified both mesh material arrays use authored materials.')
