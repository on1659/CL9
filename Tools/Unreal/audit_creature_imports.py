"""Compare imported animation endpoints against evaluated Blender samples, all 16 clips."""
import unreal,json,hashlib,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();report={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'creatures':[]}
for species in globals().get('SPECIES',('Chicken','Raccoon')):
    folder=f'/Game/CL9Art/Monsters/{species}A';source=root/'ArtSource/monster'/f'{species.lower()}_a/export'
    mesh=unreal.load_asset(folder+f'/SK_{species}_A');reference=json.loads((source/'blender_animation_qa.json').read_text())
    manifest=json.loads((source/'manifest.json').read_text())
    options=unreal.AnimPoseEvaluationOptions();options.optional_skeletal_mesh=mesh;options.extract_root_motion=False
    record={'species':species,'clips':[]}
    for name,clip in reference.items():
        animation=unreal.load_asset(folder+f'/AN_{species}_{name}');max_error=0;root_error=0;samples=[]
        assert hashlib.sha256((source/f'AN_{species}_{name}.fbx').read_bytes()).hexdigest()==clip['fbx_sha256']
        for sample in clip['samples']:
            pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(animation,(sample['frame']-1)/manifest['clips'][name]['fps'],options)
            origin=unreal.AnimPoseExtensions.get_bone_pose(pose,'root',unreal.AnimPoseSpaces.WORLD).translation
            root_error=max(root_error,origin.length());positions={}
            targets={f'foot_{side}':sample[f'foot_{side}_cm'] for side in ('l','r')} if 'foot_l_cm' in sample else {tag+'_paw':p for tag,p in sample['paws_cm'].items()}
            for bone,expected in targets.items():
                transform=unreal.AnimPoseExtensions.get_bone_pose(pose,bone,unreal.AnimPoseSpaces.WORLD)
                p=transform.translation;actual=[p.x,p.y,p.z];positions[bone]=actual
                max_error=max(max_error,*[abs(actual[i]-expected[i]*(-1 if i==1 else 1)) for i in range(3)])
            samples.append({'frame':sample['frame'],'feet_cm':positions})
        record['clips'].append({'name':name,'max_foot_position_error_cm':max_error,'root_max_cm':root_error,
                                'pass':max_error<.01 and root_error<.001,'samples':samples})
    report['creatures'].append(record)
(root/'docs/asset/evidence'/globals().get('EVIDENCE','004-creature-pose-comparison.json')).write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps([{'species':r['species'],'clips':[{k:v for k,v in c.items() if k!='samples'} for c in r['clips']]} for r in report['creatures']]))
assert all(c['pass'] for r in report['creatures'] for c in r['clips'])
