"""Audit current exported actions, render repaired death pose and real locomotion movies."""
import bpy,json,hashlib
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/chicken_a')
s=bpy.data.scenes['CL9_CHICKEN_A'];bpy.context.window.scene=s
r=bpy.data.objects['SKEL_Chicken_A'];m=bpy.data.objects['SK_Chicken_A']
manifest=json.loads((ROOT/'export/manifest.json').read_text());audit={}
for name,clip in manifest['clips'].items():
    r.animation_data.action=bpy.data.actions[clip['action']];samples=[];first=None;last=None
    for f in range(clip['frames'][0],clip['frames'][1]+1):
        s.frame_set(f);bpy.context.view_layer.update()
        matrices={p.name:p.matrix.copy() for p in r.pose.bones}
        if first is None:first=matrices
        last=matrices;e=m.evaluated_get(bpy.context.evaluated_depsgraph_get())
        samples.append({'frame':f,'min_z_cm':min(v.co.z for v in e.data.vertices)*100,
                        'root_displacement_cm':r.pose.bones['root'].head.length*100,
                        'foot_l_cm':list(r.pose.bones['foot_l'].head*100),
                        'foot_r_cm':list(r.pose.bones['foot_r'].head*100)})
    error=max(abs(first[n][i][j]-last[n][i][j]) for n in first for i in range(4) for j in range(4))
    assert max(x['root_displacement_cm'] for x in samples)<.001
    assert min(x['min_z_cm'] for x in samples)>-.1
    if clip['loop']:assert error<.001
    speed_error=0
    if name in ('Walk','Run'):
        for side in ('l','r'):
            previous=None
            for point in (p for p in clip['stance_samples'] if p['side']==side):
                actual=samples[point['frame']-1]['foot_'+side+'_cm']
                assert abs(actual[2]-point['z_cm'])<.01
                if previous and point['frame']==previous[0]+1 and actual[0]<previous[1][0]:
                    measured=(previous[1][0]-actual[0])*30
                    speed_error=max(speed_error,abs(measured-clip['reference_speed_cm_s']))
                previous=(point['frame'],actual)
        assert speed_error<.01
    audit[name]={'loop':clip['loop'],'loop_matrix_max_error':error,'stance_speed_max_error_cm_s':speed_error,
                 'fbx_sha256':hashlib.sha256((ROOT/'export'/f"{clip['action']}.fbx").read_bytes()).hexdigest(),'samples':samples}
(ROOT/'export/blender_animation_qa.json').write_text(json.dumps(audit,indent=2),encoding='utf-8')
bg=next(n for n in s.world.node_tree.nodes if n.type=='BACKGROUND');bg.inputs[1].default_value=.35
for name,power in [('Key',420),('Fill',150)]:
    light=bpy.data.lights['CL9_CHICKEN_'+name];light.energy=power;light.color=(1,1,1)
r.animation_data.action=bpy.data.actions['AN_Chicken_Death'];s.frame_set(37)
s.render.image_settings.media_type='IMAGE';s.render.image_settings.file_format='PNG';s.render.filepath=str(ROOT/'preview/death.png');bpy.ops.render.render(write_still=True)
s.render.resolution_x=512;s.render.resolution_y=512;s.cycles.samples=8
s.render.image_settings.media_type='VIDEO';s.render.image_settings.file_format='FFMPEG';s.render.ffmpeg.format='MPEG4';s.render.ffmpeg.codec='H264'
s.render.ffmpeg.constant_rate_factor='HIGH'
for name,end in [('Walk',24),('Run',15)]:
    r.animation_data.action=bpy.data.actions['AN_Chicken_'+name];s.frame_start=1;s.frame_end=end;s.render.fps=30
    s.render.filepath=str(ROOT/'preview'/f'{name.lower()}_cycle.mp4');bpy.ops.render.render(animation=True)
s.render.image_settings.media_type='IMAGE';s.render.image_settings.file_format='PNG';s.render.resolution_x=1000;s.render.resolution_y=1000;s.cycles.samples=32
r.animation_data.action=bpy.data.actions['AN_Chicken_Idle'];s.frame_start=1;s.frame_end=61;s.frame_set(1)
bpy.data.libraries.write(str(ROOT/'chicken_a.blend'),{s,*[bpy.data.actions[c['action']] for c in manifest['clips'].values()]},fake_user=True)
print('Ground/root/loop/stance speed audit passed in Blender; UE remains unverified')
