"""Import reference probe with an isolated Skeleton and compare every reference bone."""
import unreal,json,math,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir());source=root/'ArtSource/character/_reference'/globals().get('SOURCE_FBX','rig_roundtrip.fbx')
folder='/Game/Tests/FarmerRigRoundtrip';name=globals().get('PROBE_NAME','SK_RigProbe');path=folder+'/'+name;assert not unreal.EditorAssetLibrary.does_asset_exist(path)
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
t=unreal.AssetImportTask();t.filename=str(source);t.destination_path=folder;t.destination_name=name;t.automated=True;t.save=True;t.replace_existing=False
o=unreal.FbxImportUI();o.import_mesh=True;o.import_as_skeletal=True;o.import_animations=False;o.import_materials=False;o.import_textures=False;o.create_physics_asset=False;o.automated_import_should_detect_type=False;o.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH
d=o.skeletal_mesh_import_data;d.convert_scene=True;d.convert_scene_unit=True;d.force_front_x_axis=False;d.import_uniform_scale=1;t.options=o
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);mesh=unreal.load_asset(path);assert isinstance(mesh,unreal.SkeletalMesh),list(t.imported_object_paths)
mod=unreal.SkeletonModifier();assert mod.set_skeletal_mesh(mesh)
expected=json.loads((root/'ArtSource/character/_reference/ue-reference-pose.json').read_text())['bones'];names=list(map(str,mod.get_all_bone_names()));rows=[]
for b in expected:
 n=b['name']
 if n not in names:rows.append({'name':n,'missing':True});continue
 p=mod.get_bone_transform(n,True);e=b['component'];pos=math.dist(p.translation.to_tuple(),e['translation']);q=p.rotation;dot=abs(sum(a*c for a,c in zip([q.x,q.y,q.z,q.w],e['rotation_xyzw'])));angle=math.degrees(2*math.acos(min(1,dot)))
 rows.append({'name':n,'parent':str(mesh.get_bone_parent(n)),'parent_matches':str(mesh.get_bone_parent(n))==b['parent'],'position_error_cm':pos,'rotation_error_deg':angle,'scale':p.scale3d.to_tuple()})
bound=mesh.get_imported_bounds();r={'path':path,'sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'bone_count':len(names),'extra_bones':sorted(set(names)-{b['name'] for b in expected}),'dimensions_cm':(bound.box_extent*2).to_tuple(),'bones':rows}
r['passed']=not r['extra_bones'] and all(not b.get('missing') and b['parent_matches'] and b['position_error_cm']<.01 and b['rotation_error_deg']<.1 and max(abs(x-1) for x in b['scale'])<.0001 for b in rows)
(root/'docs/asset/evidence'/globals().get('EVIDENCE','024-rig-roundtrip.json')).write_text(json.dumps(r,indent=2),encoding='utf-8');print(json.dumps({k:v for k,v in r.items() if k!='bones'}));print('Worst bones',sorted(rows,key=lambda x:x.get('position_error_cm',999),reverse=True)[:3])
