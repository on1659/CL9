import bpy,bmesh,json,math,hashlib,datetime
from pathlib import Path
root=Path('D:/Work/unreal/CL9');rows=[]
for part,name,scene in [('body','SM_FoodTruck_A','CL9_FOOD_TRUCK'),('steps','SM_FoodTruck_Steps_A','CL9_FOOD_TRUCK_STEPS')]:
 folder=root/'ArtSource/map/food_truck'/part;m=json.loads((folder/'export/manifest.json').read_text());meta=next(iter(m['assets'].values()))
 o=bpy.data.objects[name];o.data.calc_loop_triangles();bm=bmesh.new();bm.from_mesh(o.data)
 assert all(e.is_manifold for e in bm.edges);assert all(f.calc_area()>1e-12 for f in bm.faces);bm.free()
 assert all(math.isfinite(c) for uv in o.data.uv_layers.active.data for c in uv.uv)
 dims=[100*v for v in o.dimensions];assert max(abs(a-b) for a,b in zip(dims,meta['dimensions_cm']))<.001
 assert len(o.data.loop_triangles)==meta['triangles'];assert len(o.data.vertices)==meta['vertices']
 f=folder/'export'/(name+'.fbx');digest=hashlib.sha256(f.read_bytes()).hexdigest();assert digest==meta['fbx_sha256']
 ue=json.loads((root/'docs/asset/evidence'/('023-'+part+'-imports.json')).read_text())['assets'][0];assert ue['source_sha256']==digest
 colliders=[c for c in bpy.data.scenes[scene].objects if c.name.startswith('UCX_'+name)]
 assert len(colliders)==ue['convex_collision_hulls']==meta['collision_hulls']
 for c in colliders:
  bm=bmesh.new();bm.from_mesh(c.data);assert all(e.is_manifold for e in bm.edges);bm.free()
 rows.append({'part':part,'source_sha256':digest,'dimensions_cm':dims,'triangles':len(o.data.loop_triangles),'vertices':len(o.data.vertices),'uv_layers':len(o.data.uv_layers),'materials':len(o.data.materials),'closed_manifold_islands':True,'collision_hulls':len(colliders),'ue_hash_matches':True,'preview_views':['neutral','night','flashlight']})
r={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'assets':rows,'rear_fit_view':'ArtSource/map/food_truck/body/preview/rear_fit.png',
 'post_walk_change':'Non-collision rear lamps relocated onto wall posts; coincident grille vertices welded without moving surfaces. Final bounds/hull count/import hash/manifold revalidated; movement collision unchanged.',
 'remaining':'Permanent base BP hooks, 2-player representation, in-game lighting and LOD/performance remain unverified.'}
(root/'docs/asset/evidence/023-blender-final.json').write_text(json.dumps(r,indent=2),encoding='utf-8');print(json.dumps(r))
