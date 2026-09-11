"""Move rear axle back 42cm and shorten inner housings at rear wall; keep truck bounds."""
import bpy,json,hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/food_truck/body')
path=ROOT/'export/manifest.json';m=json.loads(path.read_text());assert m.get('surface_fit') and not m.get('kitchen_clearance_fit')
o=bpy.data.objects['SM_FoodTruck_A'];scene=bpy.data.scenes['CL9_FOOD_TRUCK'];bpy.context.window.scene=scene
shift=Vector((.07,.2325,0))
adj={v.index:set() for v in o.data.vertices}
for e in o.data.edges:
 a,b=e.vertices;adj[a].add(b);adj[b].add(a)
unseen=set(adj);changed=0;housings=0
while unseen:
 first=unseen.pop();group={first};queue=[first]
 while queue:
  for v in adj[queue.pop()]:
   if v in unseen:unseen.remove(v);group.add(v);queue.append(v)
 vs=[o.data.vertices[i] for i in group];coords=[v.co+shift for v in vs]
 lo=[min(v[j] for v in coords) for j in range(3)];hi=[max(v[j] for v in coords) for j in range(3)]
 if lo[0]>-2.601 and hi[0]<-1.499 and hi[2]<1.031:
  housing=abs(hi[2]-1.03)<1e-5
  for v,c in zip(vs,coords):
   v.co.x=( -2.82+(c.x+2.56)*(.86/1.02) if housing else c.x-.42)-shift.x
  changed+=1;housings+=int(housing)
assert changed==44 and housings==2,(changed,housings)
colliders=0
for c in scene.objects:
 if not c.name.startswith('UCX_SM_FoodTruck_A'):continue
 coords=[c.matrix_world@v.co+shift for v in c.data.vertices]
 lo=[min(v[j] for v in coords) for j in range(3)];hi=[max(v[j] for v in coords) for j in range(3)]
 if lo[0]>-2.601 and hi[0]<-1.499 and hi[2]<1.031:
  inv=c.matrix_world.inverted();housing=abs(hi[2]-1.03)<1e-5
  for v,p in zip(c.data.vertices,coords):
   p.x=-2.82+(p.x+2.56)*(.86/1.02) if housing else p.x-.42
   v.co=inv@(p-shift)
  c.data.update();colliders+=1
assert colliders==4,colliders
o.data.update();bpy.context.view_layer.update()
meta=m['assets']['food_truck'];meta['anchors_cm']['KitchenFootprintCentre'][0]+=30
meta['anchors_cm']['AisleCentre']=[-47,-70.25,64]
m['kitchen_clearance_fit']={'rear_axle_shift_cm':-42,'inner_housing_x_cm_before_pivot':[-282,-196],'kitchen_shift_x_cm':30,'housing_to_kitchen_footprint_gap_cm':6,'aisle_width_beside_kitchen_footprint_cm':119}
bpy.ops.object.select_all(action='DESELECT');o.select_set(True)
for c in scene.objects:
 if c.name.startswith('UCX_SM_FoodTruck_A'):c.select_set(True)
bpy.context.view_layer.objects.active=o;f=ROOT/'export/SM_FoodTruck_A.fbx'
bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
meta['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest();path.write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/m['source_blend']),{scene,bpy.data.scenes['CL9_FOOD_TRUCK_PREVIEW']},fake_user=True)
print('Moved visual islands',changed,'colliders',colliders,meta['fbx_sha256'])
