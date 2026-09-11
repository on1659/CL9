"""Maintain wall thickness at the tapered pot base."""
import bpy,json,hashlib,math
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/cookware');s=bpy.data.scenes['CL9_COOKWARE'];bpy.context.window.scene=s
m=json.loads((ROOT/'export/manifest.json').read_text());assert not m.get('inner_wall_fit')
o=bpy.data.objects['SM_Pot_A'];count=0
for v in o.data.vertices:
    r=math.hypot(v.co.x,v.co.y)
    if abs(v.co.z+.105)<.00001 and abs(r-.170)<.00001:
        v.co.x*=.158/.170;v.co.y*=.158/.170;count+=1
assert count==40,count
o.data.update();bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.025);bpy.ops.object.mode_set(mode='OBJECT')
f=ROOT/'export'/(o.name+'.fbx');bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
m['assets']['pot']['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest();m['inner_wall_fit']=True
(ROOT/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'cookware.blend'),{s,bpy.data.scenes['CL9_COOKWARE_PREVIEW']},fake_user=True)
print('Adjusted 40 inner-wall vertices; outer dimensions unchanged')
