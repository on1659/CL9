"""Evaluate imported animation and compare feet to Blender's actual baked pose samples."""
import unreal,json,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();folder='/Game/Monsters/ChickenA'
mesh=unreal.load_asset(folder+'/SK_Chicken_A');a=unreal.load_asset(folder+'/AN_Chicken_Walk')
expected=json.loads((root/'ArtSource/monster/chicken_a/export/blender_animation_qa.json').read_text())['Walk']
options=unreal.AnimPoseEvaluationOptions();options.optional_skeletal_mesh=mesh;options.extract_root_motion=False
records=[]
for sample in expected['samples']:
    t=(sample['frame']-1)/30
    pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(a,t,options);values={}
    for name in ('SKEL_Chicken_A','root','body','foot_l','foot_r'):
        transform=unreal.AnimPoseExtensions.get_bone_pose(pose,name,unreal.AnimPoseSpaces.WORLD)
        p=transform.translation;scale=transform.scale3d
        values[name]={'position_cm':[p.x,p.y,p.z],'scale':[scale.x,scale.y,scale.z]}
    records.append({'frame':sample['frame'],'bones':values})
report={'engine':unreal.SystemLibrary.get_engine_version(),'animation':a.get_path_name(),
        'duration_seconds':a.get_editor_property('sequence_length'),'has_vertex_colors':mesh.has_vertex_colors(),
        'material':mesh.get_editor_property('materials')[0].material_interface.get_path_name(),
        'source_sha256':hashlib.sha256((root/'ArtSource/monster/chicken_a/export/AN_Chicken_Walk.fbx').read_bytes()).hexdigest(),
        'samples':records}
(root/'docs/asset/evidence/004-chicken-walk-pose.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in report.items() if k!='samples'}));print(json.dumps(records[0]));print(json.dumps(records[6]))
