"""Recover the verified interrupted body-only rebuild; leave orphan draft meshes intact."""
import bpy,json
from pathlib import Path
root=Path('D:/Work/unreal/CL9/ArtSource/map/food_truck/body')
assert 'CL9_FOOD_TRUCK' not in bpy.data.scenes and 'CL9_FOOD_TRUCK_STEPS' in bpy.data.scenes
assert (root/'food_truck.blend').exists()
m=json.loads((root/'export/pre-import-draft-manifest.json').read_text())
for name in m['materials']:
 mat=bpy.data.materials[name]
 assert all(mat not in list(o.data.materials) for o in bpy.data.objects if o.type=='MESH')
 mat.name=name+'_DraftUnused'
code=Path('D:/Work/unreal/CL9/Tools/Blender/build_food_truck.py').read_text(encoding='utf-8').split('# Independent four-step component')[0]
exec(compile(code,'build_food_truck_body.py','exec'))
print('Body restored from corrected source; unrelated live objects unchanged.')
