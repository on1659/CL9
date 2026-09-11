"""Replace sparse ribbons whose long chords intersected the shell with dense projected strips."""
import bpy,bmesh,math,json
from mathutils import Vector
from pathlib import Path
root=Path('D:/Work/unreal/CL9/ArtSource/item/egg_states')
kit=bpy.data.scenes['CL9_EGG01_KIT'];bpy.context.window.scene=kit
raw=bpy.data.objects['SM_Egg_Raw_A']
paths=[[(.155,.05),(.105,-.10),(.055,.06),(.005,-.10),(-.05,.09),(-.12,-.12)],
       [(.055,.06),(.035,.36),(.07,.62),(.035,.95)],
       [(-.05,.09),(-.07,-.38),(-.04,-.62)],
       [(.13,2.8),(.07,2.65),(.015,2.82),(-.06,2.62)]]
for name in ('SM_Egg_Cracked_A','SM_Egg_Burnt_A'):
    ob=bpy.data.objects[name];bm=bmesh.new();bm.from_mesh(ob.data)
    bmesh.ops.delete(bm,geom=[f for f in bm.faces if f.material_index==1],context='FACES')
    bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS')
    use_paths=paths if 'Cracked' in name else [[(z,a+k*math.tau/5) for z,a in paths[0]] for k in range(5)]
    for path in use_paths:
        points=[]
        for p,q in zip(path,path[1:]):
            count=max(2,math.ceil((abs(q[0]-p[0])+abs(q[1]-p[1])*.15)/.004))
            points.extend([(p[0]+(q[0]-p[0])*j/count,p[1]+(q[1]-p[1])*j/count) for j in range(count)])
        points.append(path[-1]);pairs=[]
        for z,a in points:
            pair=[]
            for sign in (-1,1):
                angle=a+sign*.023
                radial=Vector((math.cos(angle),math.sin(angle),0))
                hit,co,normal,face=raw.ray_cast(radial*.4+Vector((0,0,z)),-radial)
                assert hit;pair.append(bm.verts.new(co+normal*.0009))
            pairs.append(pair)
        for a,b in zip(pairs,pairs[1:]):
            face=bm.faces.new((a[0],a[1],b[1],b[0]));face.material_index=1
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(ob.data);bm.free();ob.data.update()
    bpy.ops.object.select_all(action='DESELECT');ob.select_set(True);bpy.context.view_layer.objects.active=ob
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.02);bpy.ops.object.mode_set(mode='OBJECT')
manifest=json.loads((root/'export/manifest.json').read_text())
for key,r in manifest['assets'].items():
    ob=bpy.data.objects[r['object']];ob.data.calc_loop_triangles()
    r['triangles']=len(ob.data.loop_triangles);r['dimensions_cm']=[x*100 for x in ob.dimensions]
    bpy.ops.object.select_all(action='DESELECT');ob.select_set(True);bpy.context.view_layer.objects.active=ob
    bpy.ops.export_scene.fbx(filepath=str(root/'export'/f'{ob.name}.fbx'),use_selection=True,object_types={'MESH'},
        global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',bake_anim=False,add_leaf_bones=False)
(root/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
preview=bpy.data.scenes['CL9_EGG01_PREVIEW'];bpy.context.window.scene=preview
preview.render.filepath=str(root/'preview/neutral.png');bpy.ops.render.render(write_still=True)
background=next(n for n in preview.world.node_tree.nodes if n.type=='BACKGROUND')
background.inputs[1].default_value=.035
for ob in preview.objects:
    if ob.type=='LIGHT':ob.data.energy*=.22;ob.data.color=(.65,.77,1)
preview.render.filepath=str(root/'preview/night.png');bpy.ops.render.render(write_still=True)
bpy.data.libraries.write(str(root/'egg_states.blend'),{kit,preview},fake_user=True)
print('Corrected continuous surface fractures, exported and rendered neutral/night')
