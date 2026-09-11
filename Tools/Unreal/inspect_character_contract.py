"""Read actual farmer_a row and export its existing mesh as a rig reference, not a new authored asset."""
import unreal,json,hashlib,datetime
from pathlib import Path
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
root=Path(unreal.Paths.project_dir());out=root/'docs/asset/evidence/024-character-contract.json';assert not out.exists()
row=unreal.load_object(None,'/Game/Data/DA_CharacterTable.DA_CharacterTable:CL9CharacterDef_0');assert row
fields={}
for key in ['alias','skeletal_mesh','skeleton','anim_bp','arms_mesh','arms_anim_bp','tint_color','capsule_radius','capsule_half_height','eye_height']:
 value=row.get_editor_property(key);fields[key]=value.get_path_name() if hasattr(value,'get_path_name') else str(value)
print(json.dumps(fields))
mesh=row.get_editor_property('skeletal_mesh');assert isinstance(mesh,unreal.SkeletalMesh)
folder=root/'ArtSource/character/_reference';folder.mkdir(parents=True,exist_ok=True);fbx=folder/'farmer_a_existing_rig.fbx';assert not fbx.exists()
task=unreal.AssetExportTask();task.object=mesh;task.filename=str(fbx);task.automated=True;task.prompt=False;task.replace_identical=False;task.options=unreal.FbxExportOption()
assert unreal.Exporter.run_asset_export_task(task) and fbx.exists()
r={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'engine':unreal.SystemLibrary.get_engine_version(),'row':row.get_path_name(),'fields':fields,'reference_fbx':str(fbx.relative_to(root)),'sha256':hashlib.sha256(fbx.read_bytes()).hexdigest(),
 'source':'Existing project Epic mannequin. Rig reference only; not CL9 original model delivery or a completed farmer model.',
 'next':'Import this actual exported rig in an isolated Blender scene; preserve bone names, hierarchy and reference transforms before authoring replacement workwear/body and first-person arms.'}
out.write_text(json.dumps(r,indent=2),encoding='utf-8');print(json.dumps(r))
