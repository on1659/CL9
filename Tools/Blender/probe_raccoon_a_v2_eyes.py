"""raccoon_a_v2.blend 눈 주변 실측 — 읽기만 한다(저장 없음). 머리 쪽 섬을 색·중심·바운드로 찍는다.

    py -3 Tools/Blender/run_headless.py probe_raccoon_a_v2_eyes.py --blend ArtSource/monster/raccoon_a_v2/raccoon_a_v2.blend
"""
import bpy,sys
from collections import defaultdict
from mathutils import Vector
sys.stdout.reconfigure(encoding='utf-8',errors='replace')
print('meshes',[(d.name,d.users,d.use_fake_user) for d in bpy.data.meshes])
print('objects',[(o.name,o.type,o.data.name if o.data else None) for o in bpy.data.objects])
print('scenes',[s.name for s in bpy.data.scenes],'worlds',[w.name for w in bpy.data.worlds])
m=bpy.data.objects['SK_Raccoon_A']
print('modifiers',[(x.name,x.type) for x in m.modifiers],'parent',m.parent and m.parent.name)
for me in (bpy.data.meshes['SK_Raccoon_A_v2_Brow100'],):
    parent=list(range(len(me.vertices)))
    def find(a):
        while parent[a]!=a:parent[a]=parent[parent[a]];a=parent[a]
        return a
    for e in me.edges:
        a,b=find(e.vertices[0]),find(e.vertices[1])
        if a!=b:parent[a]=b
    isl=defaultdict(list)
    for v in me.vertices:isl[find(v.index)].append(v.index)
    ca=me.color_attributes['Color'];col={}
    for p in me.polygons:
        r=find(p.vertices[0])
        if r not in col:col[r]=tuple(round(x,3) for x in ca.data[p.loop_indices[0]].color[:3])
    names={g.index:g.name for g in m.vertex_groups}
    for r,ids in sorted(isl.items(),key=lambda kv:-len(kv[1])):
        vs=[me.vertices[i].co for i in ids];c=sum(vs,Vector())/len(vs)
        if c.x<.3 or c.z<.55:continue  # 머리 앞쪽만
        lo=[min(v[k] for v in vs) for k in range(3)];hi=[max(v[k] for v in vs) for k in range(3)]
        g=names[max(me.vertices[ids[0]].groups,key=lambda g:g.weight).group]
        print('island n=%4d grp=%-6s col=%s c=(%.3f,%.3f,%.3f) x %.3f..%.3f y %.3f..%.3f z %.3f..%.3f'%(len(ids),g,col.get(r),c.x,c.y,c.z,lo[0],hi[0],lo[1],hi[1],lo[2],hi[2]))
