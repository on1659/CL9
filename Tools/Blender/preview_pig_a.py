import bpy,json
from pathlib import Path
SPECIES=globals().get('SPECIES','Pig')
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster')/(SPECIES.lower()+'_a')
exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/creature_preview.py',encoding='utf-8').read(),'creature_preview.py','exec'))
s=setup_creature_preview('CL9_'+SPECIES.upper()+'_A',ROOT,focus=(0,0,.54),camera_position=(2.6,-3.8,2.0),ortho=2.55)
r=bpy.data.objects['SKEL_'+SPECIES+'_A']
for name,frame in globals().get('POSES',[('Idle',1),('Run',5),('Catch',10),('Death',37)]):
    r.animation_data.action=bpy.data.actions['AN_'+SPECIES+'_'+name];s.frame_set(frame)
    s.render.filepath=str(ROOT/'preview'/('neutral.png' if name=='Idle' else name.lower()+'.png'));bpy.ops.render.render(write_still=True)
r.animation_data.action=bpy.data.actions['AN_'+SPECIES+'_Idle'];s.frame_set(1)
if globals().get('NIGHT',False):render_night(s,ROOT)
setup_creature_preview('CL9_'+SPECIES.upper()+'_A',ROOT,focus=(0,0,.54),camera_position=(2.6,-3.8,2.0),ortho=2.55)
manifest=json.loads((ROOT/'export/manifest.json').read_text())
bpy.data.libraries.write(str(ROOT/(SPECIES.lower()+'_a.blend')),{s,*[bpy.data.actions[c['action']] for c in manifest['clips'].values()]},fake_user=True)
print('Saved',SPECIES,'previews/source')
