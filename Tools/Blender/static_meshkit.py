"""Reusable original-geometry helpers for CL9 static props."""
import bpy,bmesh,json,hashlib
from pathlib import Path
from mathutils import Vector

class StaticKit:
    def __init__(self,scene_name,root,source_blend,materials):
        assert scene_name not in bpy.data.scenes,'Inspect existing source before rebuilding'
        self.root=Path(root);self.source_blend=source_blend;self.specs=materials;self.parts=[];self.mats={}
        for f in ('export','preview'):(self.root/f).mkdir(parents=True,exist_ok=True)
        self.scene=bpy.data.scenes.new(scene_name);bpy.context.window.scene=self.scene
        self.scene.unit_settings.system='METRIC';self.scene.unit_settings.scale_length=1
        self.collection=bpy.data.collections.new(scene_name+'_EXPORT');self.scene.collection.children.link(self.collection)
        for name,meta in materials.items():
            mat=bpy.data.materials.new(name);mat.use_nodes=True
            bs=next(n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
            if meta['vertex_color']:
                color=mat.node_tree.nodes.new('ShaderNodeVertexColor');color.layer_name='Color';mat.node_tree.links.new(color.outputs['Color'],bs.inputs['Base Color'])
            else:bs.inputs['Base Color'].default_value=meta['base_color']
            bs.inputs['Roughness'].default_value=meta['roughness'];bs.inputs['Metallic'].default_value=meta['metallic']
            if 'emission_color' in meta:
                bs.inputs['Emission Color'].default_value=meta['emission_color'];bs.inputs['Emission Strength'].default_value=meta.get('emission_default',0)
            self.mats[name]=mat
    def part(self,o,name,color,material):
        o.name=name
        for c in list(o.users_collection):c.objects.unlink(o)
        self.collection.objects.link(o);o.data.materials.append(self.mats[material])
        attr=o.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
        for v in attr.data:v.color=color
        self.parts.append(o);return o
    def box(self,name,center,size,color,material,bevel=.004):
        bpy.ops.mesh.primitive_cube_add(size=1,location=center);o=bpy.context.object;o.scale=size
        bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
        if bevel:
            mod=o.modifiers.new('Edges','BEVEL');mod.width=bevel;mod.segments=1;bpy.ops.object.modifier_apply(modifier=mod.name)
        return self.part(o,name,color,material)
    def bar(self,name,a,b,r,color,material,n=16):
        delta=Vector(b)-Vector(a)
        bpy.ops.mesh.primitive_cylinder_add(vertices=n,radius=r,depth=delta.length,location=(Vector(a)+Vector(b))/2)
        o=bpy.context.object;o.rotation_euler=delta.to_track_quat('Z','Y').to_euler()
        bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
        return self.part(o,name,color,material)
    def finish(self,key,name,runtime_radius=None,anchors=None,extra=None,pivot='bounds center',collision_boxes=None,collision_convexes=None):
        bpy.ops.object.select_all(action='DESELECT')
        for o in self.parts:o.select_set(True)
        o=self.parts[0];bpy.context.view_layer.objects.active=o;bpy.ops.object.join();o.name=name
        bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
        lo=Vector(tuple(min(v.co[i] for v in o.data.vertices) for i in range(3)))
        hi=Vector(tuple(max(v.co[i] for v in o.data.vertices) for i in range(3)));mid=(lo+hi)/2
        assert pivot in ('bounds center','floor center','hinge left')
        if pivot in ('floor center','hinge left'):mid.z=lo.z
        if pivot=='hinge left':mid.x=lo.x
        for v in o.data.vertices:v.co-=mid
        bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));assert all(e.is_manifold for e in bm.edges)
        bm.to_mesh(o.data);bm.free()
        bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.018);bpy.ops.object.mode_set(mode='OBJECT')
        bpy.context.view_layer.update();o.data.calc_loop_triangles()
        collisions=[]
        if collision_boxes or collision_convexes:
            coll=bpy.data.collections.new(self.scene.name+'_COLLISION');self.scene.collection.children.link(coll)
            for index,(center,size) in enumerate(collision_boxes or []):
                bpy.ops.mesh.primitive_cube_add(size=1,location=Vector(center)-mid);h=bpy.context.object
                h.name='UCX_'+name+'_'+str(index).zfill(2);h.scale=size
                bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
                for c in list(h.users_collection):c.objects.unlink(h)
                coll.objects.link(h);collisions.append(h)
            for points in collision_convexes or []:
                bm=bmesh.new()
                for point in points:bm.verts.new(Vector(point)-mid)
                bmesh.ops.convex_hull(bm,input=list(bm.verts),use_existing_faces=False)
                unused=[v for v in bm.verts if not v.link_faces]
                if unused:bmesh.ops.delete(bm,geom=unused,context='VERTS')
                bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
                assert all(e.is_manifold for e in bm.edges)
                mesh=bpy.data.meshes.new(name+'_Convex');bm.to_mesh(mesh);bm.free()
                h=bpy.data.objects.new('UCX_'+name+'_'+str(len(collisions)).zfill(2),mesh)
                coll.objects.link(h);collisions.append(h)
        bpy.ops.object.select_all(action='DESELECT');o.select_set(True)
        for h in collisions:h.select_set(True)
        bpy.context.view_layer.objects.active=o
        f=self.root/'export'/(name+'.fbx');bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
        for h in collisions:h.hide_render=True;h.hide_set(True)
        meta={'object':name,'dimensions_cm':[v*100 for v in o.dimensions],'triangles':len(o.data.loop_triangles),'vertices':len(o.data.vertices),
              'uv_layers':len(o.data.uv_layers),'materials':[m.name for m in o.data.materials],'runtime_radius_cm':runtime_radius,
              'closed_surfaces':True,'fbx_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'anchors_cm':{k:list((Vector(v)-mid)*100) for k,v in (anchors or {}).items()}}
        meta.update(extra or {})
        meta['bounds_center_cm']=list(((lo+hi)/2-mid)*100)
        meta['collision_hulls']=len(collisions)
        refs=bpy.data.collections.new(self.scene.name+'_REF');self.scene.collection.children.link(refs)
        for keyname,v in meta['anchors_cm'].items():
            e=bpy.data.objects.new('REF_'+name+'_'+keyname,None);refs.objects.link(e);e.location=Vector(v)/100;e.empty_display_size=.06
        manifest={'source':'Original CL9 geometry through Blender MCP','source_blend':self.source_blend,'pivot':pivot,'axis':'+X forward / +Z up','materials':self.specs,'assets':{key:meta}}
        (self.root/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
        bpy.data.libraries.write(str(self.root/self.source_blend),{self.scene},fake_user=True)
        print(json.dumps(meta));return o
