"""Rebuild only this unimported owned body after correcting roof seams/cab hulls."""
import bpy,json
from pathlib import Path
root=Path('D:/Work/unreal/CL9/ArtSource/map/food_truck/body')
assert not bpy.app.is_job_running('RENDER')
assert not Path('D:/Work/unreal/CL9/docs/asset/evidence/023-body-imports.json').exists()
backup=root/'export/pre-import-draft-manifest.json';assert not backup.exists()
source=root/'export/manifest.json';m=json.loads(source.read_text());assert m['assets']['food_truck']['collision_hulls']==23
scene=bpy.data.scenes['CL9_FOOD_TRUCK'];objects=list(scene.objects)
assert all(o.name.startswith(('SM_FoodTruck_A','UCX_SM_FoodTruck_A_','REF_SM_FoodTruck_A_')) for o in objects)
assert all(sum(o.name in s.objects for s in bpy.data.scenes)==1 for o in objects)
collections=list(scene.collection.children)
assert all(c.name.startswith('CL9_FOOD_TRUCK_') for c in collections)
backup.write_text(source.read_text(),encoding='utf-8')
if bpy.context.window.scene==scene:bpy.context.window.scene=bpy.data.scenes['CL9_FOOD_TRUCK_STEPS']
for o in objects:
 data=o.data;bpy.data.objects.remove(o,do_unlink=True)
 if isinstance(data,bpy.types.Mesh) and data.users==0:bpy.data.meshes.remove(data)
bpy.data.scenes.remove(scene)
for c in collections:
 assert not c.objects;bpy.data.collections.remove(c)
for name in m['materials']:
 mat=bpy.data.materials.get(name)
 if mat is not None:
  assert all(mat not in list(o.data.materials) for o in bpy.data.objects if o.type=='MESH')
  mat.name=name+'_DraftUnused'
code=Path('D:/Work/unreal/CL9/Tools/Blender/build_food_truck.py').read_text(encoding='utf-8').split('# Independent four-step component')[0]
exec(compile(code,'build_food_truck_body.py','exec'))
print('Owned body refined before first UE import; steps and unrelated scenes preserved.')
