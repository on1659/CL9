"""Keep pot handle loops outside the vessel's usable interior."""
import bpy,json,hashlib
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/cookware');s=bpy.data.scenes['CL9_COOKWARE'];bpy.context.window.scene=s
m=json.loads((ROOT/'export/manifest.json').read_text());assert not m.get('handle_fit_revision')
o=bpy.data.objects['SM_Pot_A'];indices={i for p in o.data.polygons if p.material_index==1 for i in p.vertices}
assert len(indices)==256,len(indices)
for i in indices:
    v=o.data.vertices[i];side=1 if v.co.y>0 else -1;v.co.y=side*(.220+(abs(v.co.y)-.210)*(.045/.063))
o.data.update();bpy.context.view_layer.update()
bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.025);bpy.ops.object.mode_set(mode='OBJECT')
f=ROOT/'export'/(o.name+'.fbx');bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
m['assets']['pot']['dimensions_cm']=[v*100 for v in o.dimensions];m['assets']['pot']['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest()
m['handle_fit_revision']=1;(ROOT/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'cookware.blend'),{s},fake_user=True);print(m['assets']['pot'])
