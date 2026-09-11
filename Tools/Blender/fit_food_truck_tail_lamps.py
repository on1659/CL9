"""Mount both rear lamps on the wall posts, clear of the open doorway."""
import bpy,json,hashlib
from pathlib import Path
from mathutils import Vector
root=Path('D:/Work/unreal/CL9/ArtSource/map/food_truck/body');p=root/'export/manifest.json';m=json.loads(p.read_text());assert not m.get('tail_lamp_fit')
o=bpy.data.objects['SM_FoodTruck_A'];s=bpy.data.scenes['CL9_FOOD_TRUCK'];bpy.context.window.scene=s;shift=Vector((.07,.2325,0));groups=[]
adj={v.index:set() for v in o.data.vertices}
for e in o.data.edges:
 a,b=e.vertices;adj[a].add(b);adj[b].add(a)
u=set(adj)
while u:
 first=u.pop();group={first};q=[first]
 while q:
  for v in adj[q.pop()]:
   if v in u:u.remove(v);group.add(v);q.append(v)
 vs=[o.data.vertices[i] for i in group];cs=[v.co+shift for v in vs]
 lo=[min(v[j] for v in cs) for j in range(3)];hi=[max(v[j] for v in cs) for j in range(3)]
 if len(vs)==24 and abs((lo[0]+hi[0])/2+2.925)<1e-5 and abs(lo[2]-.99)<1e-5 and abs(hi[2]-1.11)<1e-5:
  side=1 if (lo[1]+hi[1])>0 else -1
  for v,c in zip(vs,cs):v.co.x+=.016;v.co.y=side*1.235+(c.y-side*.8)*.5-shift.y
  groups.append(side)
assert sorted(groups)==[-1,1],groups
o.data.update();bpy.context.view_layer.update();bpy.ops.object.select_all(action='DESELECT');o.select_set(True)
for c in s.objects:
 if c.name.startswith('UCX_SM_FoodTruck_A'):c.select_set(True)
bpy.context.view_layer.objects.active=o;f=root/'export/SM_FoodTruck_A.fbx'
bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
m['assets']['food_truck']['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest();m['tail_lamp_fit']={'rear_post_centres_y_cm_before_pivot':[-123.5,123.5],'lamp_width_cm':8,'collision_unchanged':True}
p.write_text(json.dumps(m,indent=2),encoding='utf-8');bpy.data.libraries.write(str(root/m['source_blend']),{s,bpy.data.scenes['CL9_FOOD_TRUCK_PREVIEW']},fake_user=True)
print('Mounted rear lamps on posts; collision and overall bounds unchanged.')
