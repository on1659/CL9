"""Fit trim to the authored shell; retain source meshes, UVs and palette."""
import bpy,json,hashlib,math
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/portable_lights')
s=bpy.data.scenes['CL9_PORTABLE_LIGHTS'];bpy.context.window.scene=s
m=json.loads((ROOT/'export/manifest.json').read_text());assert not m.get('trim_fit_revision')
bpy.data.libraries.write(str(ROOT/'portable_lights_before_trim_fit.blend'),{s},fake_user=True)
def islands(o):
    adj=[[] for v in o.data.vertices]
    for e in o.data.edges:a,b=e.vertices;adj[a].append(b);adj[b].append(a)
    seen=set();result=[]
    for start in range(len(adj)):
        if start in seen:continue
        stack=[start];seen.add(start);group=[]
        while stack:
            v=stack.pop();group.append(v)
            for other in adj[v]:
                if other not in seen:seen.add(other);stack.append(other)
        result.append(group)
    return result
fixed=[]
for key,meta in m['assets'].items():
    o=bpy.data.objects[meta['object']]
    # Previous canonical raw-space bounds center, obtained from authored extremities.
    oldmid=Vector((-.003,0,0)) if key=='flashlight' else Vector((.0005,0,.04175))
    for group in islands(o):
        points=[o.data.vertices[i].co+oldmid for i in group]
        center=sum(points,Vector())/len(points)
        kind=None
        if key=='flashlight' and (center-Vector((-.060,-.033,.004))).length<.003:kind='grip_plate'
        elif key=='headlamp':
            if (center-Vector((-.028,0,.094))).length<.003:kind='ridge'
            elif abs(center.x+.015)<.004 and abs(abs(center.y)-.104)<.004 and abs(center.z-.031)<.003:kind='reflector'
            elif abs(center.x-.080)<.004 and abs(abs(center.y)-.036)<.004 and abs(center.z-.022)<.003:kind='bracket'
            elif abs(center.x-(.068+.093+.135)/3)<.002 and abs(center.y)<.001 and abs(center.z-.025)<.002:kind='lamp_body'
        if not kind:continue
        fixed.append(key+':'+kind)
        for i,v in zip(group,points):
            if kind=='grip_plate':v.y-=.0035
            elif kind=='ridge':
                shell=.096*math.sqrt(max(0,1-((v.x+.02)/.112)**2-(v.y/.102)**2))
                v.z=shell+.001+(v.z-.0875)
            elif kind=='reflector':
                side=1 if v.y>0 else -1
                shell=.102*math.sqrt(max(0,1-((v.x+.02)/.112)**2-(v.z/.096)**2))
                v.y=side*(shell+.001+abs(v.y)-.1025)
            elif kind=='bracket':v.x=.085+(v.x-.0565)/.047*.034
            elif kind=='lamp_body' and abs(v.x-.068)<.0001:v.x=.091
            o.data.vertices[i].co=v-oldmid
    lo=Vector(tuple(min(v.co[i] for v in o.data.vertices) for i in range(3)))
    hi=Vector(tuple(max(v.co[i] for v in o.data.vertices) for i in range(3)));shift=(lo+hi)/2
    for v in o.data.vertices:v.co-=shift
    meta['lens_center_cm']=list(Vector(meta['lens_center_cm'])-shift*100)
    o.data.update();bpy.context.view_layer.update()
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.normals_make_consistent(inside=False);bpy.ops.uv.smart_project(island_margin=.025);bpy.ops.object.mode_set(mode='OBJECT')
    f=ROOT/'export'/(o.name+'.fbx')
    bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',
        global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
    meta['dimensions_cm']=[v*100 for v in o.dimensions];meta['fbx_sha256']=hashlib.sha256(f.read_bytes()).hexdigest()
assert len(fixed)==7,fixed
m['trim_fit_revision']=1;m['trim_fixes']=fixed
(ROOT/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'portable_lights.blend'),{s,bpy.data.scenes['CL9_PORTABLE_LIGHTS_PREVIEW']},fake_user=True)
print(json.dumps({'fixed':fixed,'dimensions':{k:v['dimensions_cm'] for k,v in m['assets'].items()}}))
