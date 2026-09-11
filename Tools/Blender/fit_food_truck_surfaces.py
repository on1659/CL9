"""Fit existing authored islands; preserve UVs, bounds, anchors and UCX hulls."""
import bpy,json,hashlib
from pathlib import Path
from mathutils import Vector,Matrix
from math import radians
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/food_truck')

def islands(o):
    adj={v.index:set() for v in o.data.vertices}
    for e in o.data.edges:
        a,b=e.vertices;adj[a].add(b);adj[b].add(a)
    unseen=set(adj)
    while unseen:
        first=unseen.pop();group={first};queue=[first]
        while queue:
            for v in adj[queue.pop()]:
                if v in unseen:unseen.remove(v);group.add(v);queue.append(v)
        yield [o.data.vertices[i] for i in group]

for part,name,scene in [('body','SM_FoodTruck_A','CL9_FOOD_TRUCK'),('steps','SM_FoodTruck_Steps_A','CL9_FOOD_TRUCK_STEPS')]:
    root=ROOT/part;path=root/'export/manifest.json';m=json.loads(path.read_text())
    assert not m.get('surface_fit'),part
    o=bpy.data.objects[name];bpy.context.window.scene=bpy.data.scenes[scene]
    changed=[]
    for vs in islands(o):
        shift=Vector((.07,.2325,0)) if part=='body' else Vector((0,0,0))
        coords=[v.co+shift for v in vs]
        lo=[min(v[j] for v in coords) for j in range(3)];hi=[max(v[j] for v in coords) for j in range(3)]
        if part=='body':
            # Side panels butt into the bulkhead and rear posts. Avoid overlapping outer corner planes.
            if len(vs)==24 and abs(hi[1]-lo[1]-.08)<1e-5 and abs(abs((hi[1]+lo[1])/2)-1.26)<1e-5:
                target_lo=-2.82 if abs(lo[0]+2.9)<1e-5 else lo[0]
                target_hi=1.23 if abs(hi[0]-1.3)<1e-5 else hi[0]
                for v,c in zip(vs,coords):v.co.x=target_lo+(c.x-lo[0])*(target_hi-target_lo)/(hi[0]-lo[0])-shift.x
                changed.append('side_corner')
            elif len(vs)==24 and abs(hi[1]-.0125)<1e-5 and abs(lo[1]+.0125)<1e-5 and 1.70<lo[2]<1.71 and 2.46<hi[2]<2.47:
                center=Vector((2.471,0,2.083))-shift;rot=Matrix.Rotation(radians(-40),3,'Y')
                for v in vs:v.co=center+rot@(v.co-center)
                changed.append('windshield_bar')
        elif len(vs)==24 and abs(hi[1]-lo[1]-1.14)<1e-5:
            # Preserve the overall step height at the marker; place the steel tread 1mm below it.
            for v in vs:v.co.z*= (hi[2]-.001)/hi[2]
            changed.append('steel_tread')
    assert changed.count('side_corner')==5 and changed.count('windshield_bar')==1 if part=='body' else changed.count('steel_tread')==4,changed
    o.data.update();bpy.context.view_layer.update()
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True)
    for c in bpy.context.scene.objects:
        if c.name.startswith('UCX_'+name):c.select_set(True)
    bpy.context.view_layer.objects.active=o
    f=root/'export'/(name+'.fbx')
    bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
    meta=next(iter(m['assets'].values()));meta['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest()
    m['surface_fit']={'changes':changed,'collision_unchanged':True,'steel_step_collision_gap_cm':.1 if part=='steps' else 0}
    path.write_text(json.dumps(m,indent=2),encoding='utf-8')
    scenes={bpy.context.scene};preview=bpy.data.scenes.get(scene+'_PREVIEW')
    if preview:scenes.add(preview)
    bpy.data.libraries.write(str(root/m['source_blend']),scenes,fake_user=True)
    print(part,changed,meta['fbx_sha256'])
