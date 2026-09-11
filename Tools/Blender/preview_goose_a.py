import bpy,json
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/goose_a')
exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/creature_preview.py',encoding='utf-8').read(),'creature_preview.py','exec'))
s=setup_creature_preview('CL9_GOOSE_A',ROOT,focus=(0,0,.82),camera_position=(2.6,-3.8,2.1),ortho=2.7)
r=bpy.data.objects['SKEL_Goose_A']
for name,frame in globals().get('POSES',[('Idle',1),('Sleep',1),('Honk',10),('Death',37)]):
    r.animation_data.action=bpy.data.actions['AN_Goose_'+name];s.frame_set(frame)
    s.render.filepath=str(ROOT/'preview'/('neutral.png' if name=='Idle' else name.lower()+'.png'));bpy.ops.render.render(write_still=True)
r.animation_data.action=bpy.data.actions['AN_Goose_Idle'];s.frame_set(1)
if globals().get('NIGHT',False):render_night(s,ROOT)
setup_creature_preview('CL9_GOOSE_A',ROOT,focus=(0,0,.82),camera_position=(2.6,-3.8,2.1),ortho=2.7)
manifest=json.loads((ROOT/'export/manifest.json').read_text())
bpy.data.libraries.write(str(ROOT/'goose_a.blend'),{s,*[bpy.data.actions[c['action']] for c in manifest['clips'].values()]},fake_user=True)
print('Saved goose previews and source')
