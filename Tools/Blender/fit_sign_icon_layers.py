"""Separate overlapping pictogram front planes by 0.5mm without changing sign bounds."""
import bpy,json,hashlib
from pathlib import Path
for label,width,height in [('feed',.014,.26),('kitchen',.185,.095)]:
    root=Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_yard')/('sign_'+label)
    m=json.loads((root/'export/manifest.json').read_text());assert not m.get('icon_layer_fit')
    meta=next(iter(m['assets'].values()));o=bpy.data.objects[meta['object']]
    bpy.context.window.scene=bpy.data.scenes['CL9_WAYFINDING_'+label.upper()]
    neighbors={v.index:set() for v in o.data.vertices}
    for e in o.data.edges:
        a,b=e.vertices;neighbors[a].add(b);neighbors[b].add(a)
    unseen=set(neighbors);changed=0
    while unseen:
        first=unseen.pop();island={first};stack=[first]
        while stack:
            for v in neighbors[stack.pop()]:
                if v in unseen:unseen.remove(v);island.add(v);stack.append(v)
        xs=[o.data.vertices[i].co.x for i in island];zs=[o.data.vertices[i].co.z for i in island]
        if len(island)==8 and abs(max(xs)-min(xs)-width)<.00001 and abs(max(zs)-min(zs)-height)<.00001:
            for i in island:o.data.vertices[i].co.y+=.0005
            changed+=1
    assert changed==1
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
    f=root/'export'/(o.name+'.fbx')
    bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
    meta['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest();m['icon_layer_fit']=True
    (root/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
    bpy.data.libraries.write(str(root/m['source_blend']),{bpy.context.scene},fake_user=True)
    print('Corrected pictogram plane overlap:',o.name)
