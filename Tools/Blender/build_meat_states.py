"""Original three-state ingredient meshes. Execute through Blender MCP."""
import bpy, bmesh, math, json, hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/meat_states')
assert 'CL9_MEAT_KIT' not in bpy.data.scenes, 'Inspect existing source instead of rebuilding'
for folder in ('export','preview'): (ROOT/folder).mkdir(parents=True,exist_ok=True)
s=bpy.data.scenes.new('CL9_MEAT_KIT');bpy.context.window.scene=s
s.unit_settings.system='METRIC';s.unit_settings.scale_length=1
manifest={'source':'Original CL9 mesh geometry authored through Blender MCP','source_blend':'meat_states.blend',
          'rig':'N/A static state swaps','axis':'+X bone direction, +Z up','pivot':'bounds center','assets':{}}
for state in ('raw','cooked','burnt'):
    parts=[]
    palette={'raw':((.43,.058,.065,1),(.88,.60,.43,1),(.79,.72,.53,1),.61),
             'cooked':((.38,.145,.026,1),(.061,.019,.007,1),(.63,.48,.28,1),.8),
             'burnt':((.013,.011,.009,1),(.115,.040,.009,1),(.22,.16,.09,1),.98)}[state]
    def ell(label,center,scale,kind,segments=24,rings=16):
        bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,radius=1,location=center)
        o=bpy.context.object;o.name='MEAT_'+state+'_'+label;o.scale=scale
        bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
        attr=o.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
        for p in o.data.polygons:
            c=sum((o.data.vertices[i].co for i in p.vertices),Vector())/len(p.vertices)
            color=palette[2] if kind=='bone' else palette[0]
            if kind=='flesh':
                if state=='raw': mark=abs(math.sin(c.x*48+c.y*8+c.z*10))<.20
                elif state=='cooked': mark=abs(math.sin(c.x*47+c.y*17))<.29
                else: mark=abs(math.sin(c.x*69+c.y*51+math.sin(c.z*79)))<.16
                if mark:color=palette[1]
            for loop in p.loop_indices:attr.data[loop].color=color
        parts.append(o);return o
    flesh=ell('Flesh',(-.11,0,0),(.205,.173,.17),'flesh',32,20)
    ell('Neck',(.065,0,0),(.125,.103,.105),'flesh')
    ell('BoneShaft',(.19,0,0),(.13,.035,.037),'bone',16,12)
    ell('BoneTipL',(.304,-.03,0),(.052,.044,.045),'bone',16,12)
    ell('BoneTipR',(.304,.03,0),(.052,.044,.045),'bone',16,12)
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    bpy.context.view_layer.objects.active=flesh;bpy.ops.object.join()
    flesh.name='SM_Meat_'+state.title()+'_A'
    # Bake location then recenter actual bounds, preserving matching states.
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    low=Vector(tuple(min(v.co[i] for v in flesh.data.vertices) for i in range(3)))
    high=Vector(tuple(max(v.co[i] for v in flesh.data.vertices) for i in range(3)))
    mid=(low+high)/2
    for v in flesh.data.vertices:v.co-=mid
    bm=bmesh.new();bm.from_mesh(flesh.data);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
    assert all(e.is_manifold for e in bm.edges),'Open mesh surface'
    bm.to_mesh(flesh.data);bm.free()
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.025);bpy.ops.object.mode_set(mode='OBJECT')
    mat=bpy.data.materials.new('M_Meat_'+state.title()+'_Vertex');mat.use_nodes=True
    bs=next(n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    vc=mat.node_tree.nodes.new('ShaderNodeVertexColor');vc.layer_name='Color'
    mat.node_tree.links.new(vc.outputs['Color'],bs.inputs['Base Color']);bs.inputs['Roughness'].default_value=palette[3]
    flesh.data.materials.clear();flesh.data.materials.append(mat)
    for p in flesh.data.polygons:p.material_index=0;p.use_smooth=False
    bpy.context.view_layer.update();flesh.data.calc_loop_triangles()
    path=ROOT/'export'/f'{flesh.name}.fbx'
    bpy.ops.export_scene.fbx(filepath=str(path),use_selection=True,object_types={'MESH'},global_scale=1,
        apply_unit_scale=True,axis_forward='X',axis_up='Z',bake_anim=False,add_leaf_bones=False)
    dims=[float(v)*100 for v in flesh.dimensions]
    manifest['assets'][state]={'object':flesh.name,'dimensions_cm':dims,'triangles':len(flesh.data.loop_triangles),
        'vertices':len(flesh.data.vertices),'uv_layers':len(flesh.data.uv_layers),'material':mat.name,
        'roughness':palette[3],'material_slots':1,'palette_linear_rgba':palette[:3],
        'fbx_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'closed_surfaces':True}
assert all(max(abs(a-b) for a,b in zip(x['dimensions_cm'],manifest['assets']['raw']['dimensions_cm']))<.0001 for x in manifest['assets'].values())
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'meat_states.blend'),{s},fake_user=True)
print(json.dumps(manifest))
