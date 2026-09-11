"""Add two eye squash bones for readable sleep/death, preserving original connected islands."""
import bpy,json
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/goose_a');s=bpy.data.scenes['CL9_GOOSE_A'];bpy.context.window.scene=s
r=bpy.data.objects['SKEL_Goose_A'];m=bpy.data.objects['SK_Goose_A']
assert 'eye_l' not in r.data.bones
bpy.data.libraries.write(str(ROOT/'goose_a_before_eye_controls.blend'),{s,*[a for a in bpy.data.actions if a.name.startswith('AN_Goose_')]},fake_user=True)
r.animation_data.action=None
for p in r.pose.bones:p.matrix_basis.identity()
bpy.context.view_layer.update();bpy.ops.object.select_all(action='DESELECT');r.select_set(True);bpy.context.view_layer.objects.active=r
bpy.ops.object.mode_set(mode='EDIT')
for sign,side in [(1,'l'),(-1,'r')]:
    b=r.data.edit_bones.new('eye_'+side);b.head=(.5,sign*.131,1.579);b.tail=(.5,sign*.131,1.619);b.parent=r.data.edit_bones['head']
bpy.ops.object.mode_set(mode='OBJECT')
adj=[[] for v in m.data.vertices]
for edge in m.data.edges:
    a,b=edge.vertices;adj[a].append(b);adj[b].append(a)
remaining=set(range(len(adj)));selected={'l':[],'r':[]}
while remaining:
    todo=[remaining.pop()];island=[]
    while todo:
        v=todo.pop();island.append(v)
        for n in adj[v]:
            if n in remaining:remaining.remove(n);todo.append(n)
    center=sum((m.data.vertices[i].co for i in island),Vector())/len(island)
    if .47<center.x<.53 and .108<abs(center.y)<.15 and 1.565<center.z<1.605:
        selected['l' if center.y>0 else 'r'].extend(island)
for side,indices in selected.items():
    assert len(indices)>100,(side,len(indices))
    m.vertex_groups['head'].remove(indices);group=m.vertex_groups.new(name='eye_'+side);group.add(indices,1,'REPLACE')
    r.pose.bones['eye_'+side].rotation_mode='QUATERNION'
for action in list(bpy.data.actions):
    if action.name.startswith('AN_Goose_'):bpy.data.actions.remove(action)
manifest=json.loads((ROOT/'export/manifest.json').read_text());manifest['deform_bones']=len(r.data.bones);manifest['clips']={}
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
print('Eye controls assigned vertices', {s:len(v) for s,v in selected.items()})
