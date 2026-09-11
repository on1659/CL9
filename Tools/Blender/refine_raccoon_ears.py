"""Replace intersecting ear inserts with color regions on the original ear shell."""
import bpy,bmesh,json
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/raccoon_a')
s=bpy.data.scenes['CL9_RACCOON_A'];bpy.context.window.scene=s;m=bpy.data.objects['SK_Raccoon_A']
bm=bmesh.new();bm.from_mesh(m.data);deform=bm.verts.layers.deform.active
colors=bm.loops.layers.float_color.get('Color');assert colors
ear_ids={m.vertex_groups['ear_l'].index:1,m.vertex_groups['ear_r'].index:-1}
discard=[]
for face in bm.faces:
    groups=face.verts[0][deform]
    if not any(groups.get(i,0)>.9 for i in ear_ids):continue
    color=face.loops[0][colors]
    if color[0]>.1:discard.append(face)
bmesh.ops.delete(bm,geom=discard,context='FACES')
bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS')
for face in bm.faces:
    groups=face.verts[0][deform]
    side=next((sign for i,sign in ear_ids.items() if groups.get(i,0)>.9),None)
    if side is None:continue
    c=face.calc_center_median();rad=((c.y-side*.177)/.062)**2+((c.z-.948)/.105)**2
    if c.x>.36 and rad<.76:
        color=(.16,.12,.105,1) if rad<.28 else (.66,.65,.56,1)
        for loop in face.loops:loop[colors]=color
bm.to_mesh(m.data);bm.free();m.data.update();m.data.calc_loop_triangles()
manifest=json.loads((ROOT/'export/manifest.json').read_text())
manifest['triangles']=len(m.data.loop_triangles);manifest['vertices']=len(m.data.vertices)
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'raccoon_a.blend'),{s},fake_user=True)
s.render.filepath=str(ROOT/'preview/neutral.png');bpy.ops.render.render(write_still=True)
print('Removed intersecting ear inserts; triangles:',manifest['triangles'])
