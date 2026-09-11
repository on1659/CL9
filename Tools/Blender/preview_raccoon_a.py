"""Render representative evaluated poses and neutral/night views."""
import bpy,json
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/raccoon_a')
exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/creature_preview.py',encoding='utf-8').read(),'creature_preview.py','exec'))
s=setup_creature_preview('CL9_RACCOON_A',ROOT,focus=(-.1,0,.5),camera_position=(2.3,-3.8,2),ortho=2.5)
r=bpy.data.objects['SKEL_Raccoon_A']
# Pass POSES to render a bounded subset without repeating earlier renders.
for name,frame in globals().get('POSES',[('Idle',1),('Walk',7),('Run',5),('LookAround',16),('Catch',10),('Death',37),('WakeTelegraph',13),('HitReact',5)]):
    r.animation_data.action=bpy.data.actions['AN_Raccoon_'+name];s.frame_set(frame)
    s.render.filepath=str(ROOT/'preview'/('neutral.png' if name=='Idle' else name.lower()+'.png'))
    bpy.ops.render.render(write_still=True)
r.animation_data.action=bpy.data.actions['AN_Raccoon_Idle'];s.frame_set(1)
if globals().get('NIGHT',False):render_night(s,ROOT)
setup_creature_preview('CL9_RACCOON_A',ROOT,focus=(-.1,0,.5),camera_position=(2.3,-3.8,2),ortho=2.5)
manifest=json.loads((ROOT/'export/manifest.json').read_text())
bpy.data.libraries.write(str(ROOT/'raccoon_a.blend'),{s,*[bpy.data.actions[c['action']] for c in manifest['clips'].values()]},fake_user=True)
print('Saved raccoon poses and source')
