"""goose_a_v2.blend 읽기 전용 점검 — 오브젝트 목록, 눈썹·머리 섬 치수, 카메라. 파일은 저장하지 않는다.

    py -3 Tools/Blender/run_headless.py inspect_goose_a_v2.py --blend ArtSource/monster/goose_a_v2/goose_a_v2.blend
"""
import bpy, sys
import numpy as np
from mathutils import Vector
sys.stdout.reconfigure(encoding='utf-8')

s = bpy.data.scenes['CL9_GOOSE_A_V2']
print('scenes', [x.name for x in bpy.data.scenes])
for ob in s.objects:
    print('OB', ob.name, ob.type, 'render' if not ob.hide_render else 'hidden', ob.data.name if ob.data else '')
print('camera', s.camera and s.camera.name, s.camera and tuple(s.camera.location), s.camera and tuple(s.camera.rotation_euler),
      s.camera and s.camera.data.type, s.camera and s.camera.data.ortho_scale)
for k, v in s.items(): print('scene prop', k, v)

ob = s.objects['SK_Goose_A']; me = ob.data
adj = [[] for _ in me.vertices]
for e in me.edges:
    a, b = e.vertices; adj[a].append(b); adj[b].append(a)
remaining = set(range(len(adj))); islands = []
while remaining:
    todo = [remaining.pop()]; isl = []
    while todo:
        v = todo.pop(); isl.append(v)
        for n in adj[v]:
            if n in remaining: remaining.remove(n); todo.append(n)
    islands.append(sorted(isl))
def arr(isl): return np.array([tuple(me.vertices[i].co) for i in isl])
for isl in islands:
    a = arr(isl); c = a.mean(0)
    if c[2] > 1.3 and c[0] > 0.1:
        print('ISL n=%d first=%d c=(%.3f,%.3f,%.3f) min=(%.3f,%.3f,%.3f) max=(%.3f,%.3f,%.3f)' % (len(isl), isl[0], *c, *a.min(0), *a.max(0)))
        if len(isl) == 86:
            u, sv, vt = np.linalg.svd(a - c)
            print('   PCA axes', np.round(vt, 3).tolist(), 'sv', np.round(sv, 3).tolist())
