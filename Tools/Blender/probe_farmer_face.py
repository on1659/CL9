"""farmer_a 얼굴 구조 실측(읽기 전용) — 장면·재질 노드·머리 위 메시 섬·팔레트 색을 찍는다. 파일에 쓰지 않는다.

    py -3 Tools/Blender/run_headless.py probe_farmer_face.py --blend ArtSource/character/farmer_a/farmer_a.blend
"""
import bpy, sys
from mathutils import Vector
sys.stdout.reconfigure(encoding='utf-8')
print('FILE', bpy.data.filepath)
for s in bpy.data.scenes:
    print('SCENE', s.name, 'cam', s.camera.name if s.camera else None, [o.name for o in s.objects])
for o in bpy.data.objects:
    print('OBJ', o.name, o.type, tuple(round(x, 3) for x in o.location), o.parent.name if o.parent else None, [m.type for m in o.modifiers])
for m in bpy.data.materials:
    if m.node_tree:
        print('MAT', m.name, [n.type for n in m.node_tree.nodes])
        for n in m.node_tree.nodes:
            if n.type == 'MIX_RGB': print('  mix', tuple(round(x, 4) for x in n.inputs[2].default_value))
            if n.type == 'BSDF_PRINCIPLED': print('  base', tuple(round(x, 4) for x in n.inputs['Base Color'].default_value), n.inputs['Base Color'].is_linked)
            if n.type == 'TEX_IMAGE': print('  img', n.image.name, n.image.filepath, n.image.packed_file is not None, tuple(n.image.size))
body = bpy.data.objects['SK_Farmer_A_Body']
me = body.data
print('BODY', len(me.vertices), 'uv', [u.name for u in me.uv_layers], 'col', [(c.name, c.domain, c.data_type) for c in me.color_attributes], 'shape', me.shape_keys, 'mats', [m.name for m in me.materials])
print('attrs', [(a.name, a.domain, a.data_type) for a in me.attributes])
adj = [[] for _ in me.vertices]
for e in me.edges:
    a, b = e.vertices; adj[a].append(b); adj[b].append(a)
rem = set(range(len(adj))); isl = []
while rem:
    t = [rem.pop()]; g = []
    while t:
        v = t.pop(); g.append(v)
        for n in adj[v]:
            if n in rem: rem.remove(n); t.append(n)
    isl.append(sorted(g))
col = me.color_attributes['Color']
vcol = {}
for l in me.loops: vcol[l.vertex_index] = tuple(round(x, 3) for x in tuple(col.data[l.index].color)[:3])
pmat = {}
for p in me.polygons:
    for v in p.vertices: pmat[v] = me.materials[p.material_index].name
for g in isl:
    cs = [me.vertices[i].co for i in g]
    c = sum(cs, Vector()) / len(cs)
    if c.z > 1.5:
        lo = [round(min(v[a] for v in cs), 4) for a in range(3)]; hi = [round(max(v[a] for v in cs), 4) for a in range(3)]
        print('ISL', len(g), g[0], tuple(round(x, 4) for x in c), lo, hi, vcol[g[0]], pmat[g[0]], [body.vertex_groups[vg.group].name for vg in me.vertices[g[0]].groups])
pal = sorted(set(vcol.values()))
print('PALETTE', len(pal), pal)
arms = bpy.data.objects.get('SK_Farmer_A_Arms')
if arms: print('ARMS', len(arms.data.vertices), [m.name for m in arms.data.materials])
me.calc_loop_triangles(); print('TRIS', len(me.loop_triangles))
