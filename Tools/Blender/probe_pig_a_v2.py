"""pig_a 실물 blend 를 읽기만 해서 구성과 머리:몸 비를 잰다. 저장하지 않는다.

    py -3 Tools/Blender/run_headless.py probe_pig_a_v2.py --blend ArtSource/monster/pig_a/pig_a.blend
"""
import bpy,bmesh,sys
from mathutils import Vector
sys.stdout.reconfigure(encoding='utf-8',errors='replace')
print('scenes',[s.name for s in bpy.data.scenes])
for ob in bpy.data.objects:print('obj',ob.name,ob.type,[m.name for m in getattr(ob.data,'materials',[]) or []])
me=bpy.data.objects['SK_Pig_A'].data
print('verts',len(me.vertices),'polys',len(me.polygons),'colors',[(c.name,c.domain,c.data_type) for c in me.color_attributes])
me.calc_loop_triangles();print('tris',len(me.loop_triangles))
# 섬(loose part)마다 중심·범위·대표색
bm=bmesh.new();bm.from_mesh(me);bm.verts.ensure_lookup_table()
seen=set();islands=[]
for v in bm.verts:
    if v.index in seen:continue
    stack=[v];part=[]
    seen.add(v.index)
    while stack:
        x=stack.pop();part.append(x.index)
        for e in x.link_edges:
            o=e.other_vert(x)
            if o.index not in seen:seen.add(o.index);stack.append(o)
    islands.append(part)
col=me.color_attributes['Color']
vcol={}
for l in me.loops:vcol.setdefault(l.vertex_index,tuple(round(c,3) for c in col.data[l.index].color))
for part in islands:
    cs=[me.vertices[i].co for i in part];lo=Vector((min(c[k] for c in cs) for k in range(3)));hi=Vector((max(c[k] for c in cs) for k in range(3)))
    print('island n=%d c=(%.3f,%.3f,%.3f) size=(%.3f,%.3f,%.3f) col=%s'%(len(part),*((lo+hi)/2),*(hi-lo),vcol.get(part[0])))
if me.materials:
    print('poly material counts',{i:sum(1 for p in me.polygons if p.material_index==i) for i in range(len(me.materials))})
