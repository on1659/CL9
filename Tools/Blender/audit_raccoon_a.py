"""Measure evaluated raccoon actions; persist failures as well as successes."""
import bpy,json,hashlib
from pathlib import Path
SPECIES=globals().get('SPECIES','Raccoon')
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster')/(SPECIES.lower()+'_a')
s=bpy.data.scenes['CL9_'+SPECIES.upper()+'_A'];bpy.context.window.scene=s
r=bpy.data.objects['SKEL_'+SPECIES+'_A'];m=bpy.data.objects['SK_'+SPECIES+'_A']
manifest=json.loads((ROOT/'export/manifest.json').read_text());audit={}
for name,clip in manifest['clips'].items():
    r.animation_data.action=bpy.data.actions[clip['action']];samples=[];first=None
    for frame in range(clip['frames'][0],clip['frames'][1]+1):
        s.frame_set(frame);bpy.context.view_layer.update()
        matrices={p.name:p.matrix.copy() for p in r.pose.bones}
        if first is None:first=matrices
        last=matrices;e=m.evaluated_get(bpy.context.evaluated_depsgraph_get())
        samples.append({'frame':frame,'min_z_cm':min((m.matrix_world @ v.co).z for v in e.data.vertices)*100,
                        'root_displacement_cm':r.pose.bones['root'].head.length*100,
                        'paws_cm':{tag:list(r.pose.bones[tag+'_paw'].head*100) for tag in ('front_l','front_r','hind_l','hind_r')}})
    loop_error=max(abs(first[n][i][j]-last[n][i][j]) for n in first for i in range(4) for j in range(4))
    speed_error=0;position_error=0
    for tag in ('front_l','front_r','hind_l','hind_r'):
        previous=None
        for point in (p for p in clip['stance_samples'] if p['limb']==tag):
            actual=samples[point['frame']-1]['paws_cm'][tag]
            position_error=max(position_error,abs(actual[0]-point['x_cm']),abs(actual[2]-point['z_cm']))
            if previous and point['frame']==previous[0]+1 and actual[0]<previous[1][0]:
                speed_error=max(speed_error,abs((previous[1][0]-actual[0])*30-clip['reference_speed_cm_s']))
            previous=(point['frame'],actual)
    ground=min(x['min_z_cm'] for x in samples);root=max(x['root_displacement_cm'] for x in samples)
    audit[name]={'loop':clip['loop'],'loop_matrix_max_error':loop_error,'stance_speed_max_error_cm_s':speed_error,
                 'stance_position_error_cm':position_error,'min_z_cm':ground,'root_max_cm':root,
                 'passed':ground>-.1 and root<.001 and (not clip['loop'] or loop_error<.001) and speed_error<.01 and position_error<.01,
                 'fbx_sha256':hashlib.sha256((ROOT/'export'/f"{clip['action']}.fbx").read_bytes()).hexdigest(),'samples':samples}
(ROOT/'export/blender_animation_qa.json').write_text(json.dumps(audit,indent=2),encoding='utf-8')
r.animation_data.action=bpy.data.actions['AN_'+SPECIES+'_Idle'];s.frame_set(1)
print(json.dumps({name:{k:v for k,v in a.items() if k!='samples'} for name,a in audit.items()},indent=2))
