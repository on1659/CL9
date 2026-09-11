"""Weld coincident bevel vertices on grille; remove eight zero-area faces without moving surfaces."""
import bpy,bmesh,json,hashlib
from pathlib import Path
root=Path('D:/Work/unreal/CL9/ArtSource/map/food_truck/body');p=root/'export/manifest.json';m=json.loads(p.read_text());assert not m.get('grille_cleanup')
o=bpy.data.objects['SM_FoodTruck_A'];s=bpy.data.scenes['CL9_FOOD_TRUCK'];bpy.context.window.scene=s
bm=bmesh.new();bm.from_mesh(o.data);bad=[f for f in bm.faces if f.calc_area()<=1e-12];assert len(bad)==8
vs=list({v for f in bad for v in f.verts});assert all(abs(v.co.x-3.038)<1e-5 for v in vs)
bmesh.ops.remove_doubles(bm,verts=vs,dist=1e-7);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
assert all(e.is_manifold for e in bm.edges);assert all(f.calc_area()>1e-12 for f in bm.faces)
bm.to_mesh(o.data);bm.free();o.data.update();o.data.calc_loop_triangles()
meta=m['assets']['food_truck'];m['grille_cleanup']={'zero_area_faces_removed':8,'previous_triangles':meta['triangles'],'surfaces_moved':False}
meta['triangles']=len(o.data.loop_triangles);meta['vertices']=len(o.data.vertices)
bpy.ops.object.select_all(action='DESELECT');o.select_set(True)
for c in s.objects:
 if c.name.startswith('UCX_SM_FoodTruck_A'):c.select_set(True)
bpy.context.view_layer.objects.active=o;f=root/'export/SM_FoodTruck_A.fbx'
bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
meta['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest();p.write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(root/m['source_blend']),{s,bpy.data.scenes['CL9_FOOD_TRUCK_PREVIEW']},fake_user=True);print(meta['triangles'],meta['vertices'],meta['fbx_sha256'])
