"""raccoon_a.blend 실측 — 읽기만 한다(저장 없음). 부위별(주 가중 뼈) 바운드와 색 목록을 찍는다.

    py -3 Tools/Blender/run_headless.py probe_raccoon_a_v2.py --blend ArtSource/monster/raccoon_a/raccoon_a.blend
"""
import bpy,sys
from collections import defaultdict
sys.stdout.reconfigure(encoding='utf-8',errors='replace')
print('scenes',[s.name for s in bpy.data.scenes])
m=bpy.data.objects['SK_Raccoon_A']
print('objects',[o.name for o in bpy.data.scenes['CL9_RACCOON_A'].objects])
print('materials',[x.name for x in m.data.materials],'attrs',[a.name for a in m.data.color_attributes])
names={g.index:g.name for g in m.vertex_groups}
lo=defaultdict(lambda:[9,9,9]);hi=defaultdict(lambda:[-9,-9,-9])
for v in m.data.vertices:
    g=max(v.groups,key=lambda g:g.weight);n=names[g.group]
    for i in range(3):lo[n][i]=min(lo[n][i],v.co[i]);hi[n][i]=max(hi[n][i],v.co[i])
for n in sorted(lo):print('%-16s x %.3f..%.3f y %.3f..%.3f z %.3f..%.3f'%(n,lo[n][0],hi[n][0],lo[n][1],hi[n][1],lo[n][2],hi[n][2]))
ca=m.data.color_attributes['Color'];cols=defaultdict(int)
for d in ca.data:cols[tuple(round(c,3) for c in d.color)]+=1
for c,k in sorted(cols.items(),key=lambda x:-x[1]):print('color',c,k)
print('mesh parts (loose islands) count via polygons',len(m.data.polygons),'verts',len(m.data.vertices))
print('actions',[a.name for a in bpy.data.actions])
