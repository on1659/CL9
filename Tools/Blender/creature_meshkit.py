"""Small original mesh construction helpers; geometry and proportions belong to each asset."""
import bpy,math,json
from pathlib import Path
from mathutils import Vector
class MeshKit:
    def __init__(self,species,palette):
        self.species=species;self.root=Path('D:/Work/unreal/CL9/ArtSource/monster')/(species.lower()+'_a')
        assert 'CL9_'+species.upper()+'_A' not in bpy.data.scenes
        for folder in ('export','preview'):(self.root/folder).mkdir(parents=True,exist_ok=True)
        self.scene=bpy.data.scenes.new('CL9_'+species.upper()+'_A');bpy.context.window.scene=self.scene
        self.scene.unit_settings.system='METRIC';self.scene.unit_settings.scale_length=1;self.scene.render.fps=30
        self.palette=palette;self.parts=[]
        self.material=bpy.data.materials.new('M_CL9_'+species+'_Vertex');self.material.use_nodes=True
        bs=next(n for n in self.material.node_tree.nodes if n.type=='BSDF_PRINCIPLED');bs.inputs['Roughness'].default_value=.86
        attr=self.material.node_tree.nodes.new('ShaderNodeVertexColor');attr.layer_name='Color';self.material.node_tree.links.new(attr.outputs['Color'],bs.inputs['Base Color'])
    def finish(self,ob,label,color,bone):
        ob.name=self.species+'_'+label;bpy.context.view_layer.objects.active=ob;bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
        ob.data.materials.clear();ob.data.materials.append(self.material);ca=ob.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
        for item in ca.data:item.color=self.palette[color]
        group=ob.vertex_groups.new(name=bone);group.add(list(range(len(ob.data.vertices))),1,'REPLACE');self.parts.append(ob);return ob
    def ell(self,label,loc,scale,color,bone,segments=16,rings=10,rotation=(0,0,0)):
        bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,location=loc)
        ob=bpy.context.object;ob.scale=scale;ob.rotation_euler=rotation;return self.finish(ob,label,color,bone)
    def taper(self,label,a,b,r1,r2,color,bone,vertices=10):
        a,b=Vector(a),Vector(b);bpy.ops.mesh.primitive_cone_add(vertices=vertices,radius1=r1,radius2=r2,depth=(b-a).length,location=(a+b)/2)
        ob=bpy.context.object;ob.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler();return self.finish(ob,label,color,bone)
    def chest_weights(self,ob):
        group=ob.vertex_groups.get('chest') or ob.vertex_groups.new(name='chest')
        body=ob.vertex_groups['body']
        for v in ob.data.vertices:
            t=max(0,min(1,(v.co.x+ob.location.x+.08)/.38));body.add([v.index],1-t,'REPLACE');group.add([v.index],t,'REPLACE')
    def finish_rig(self,bone_specs):
        bpy.ops.object.select_all(action='DESELECT')
        for ob in self.parts:ob.select_set(True)
        bpy.context.view_layer.objects.active=self.parts[0];bpy.ops.object.join();mesh=bpy.context.object;mesh.name='SK_'+self.species+'_A'
        self.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
        bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.normals_make_consistent(inside=False)
        bpy.ops.uv.smart_project(island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
        data=bpy.data.armatures.new('SKEL_'+self.species+'_A');rig=bpy.data.objects.new(data.name,data);self.scene.collection.objects.link(rig)
        bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);bpy.context.view_layer.objects.active=rig;bpy.ops.object.mode_set(mode='EDIT');bones={}
        for name,head,tail,parent in bone_specs:
            b=data.edit_bones.new(name);b.head=head;b.tail=tail
            if parent:b.parent=bones[parent]
            bones[name]=b
        bpy.ops.object.mode_set(mode='OBJECT');mesh.parent=rig;mod=mesh.modifiers.new('CL9 Skin','ARMATURE');mod.object=rig
        for p in rig.pose.bones:p.rotation_mode='QUATERNION'
        mesh.data.calc_loop_triangles()
        for v in mesh.data.vertices:
            weights=[g.weight for g in v.groups if g.weight>1e-6];assert 1<=len(weights)<=2 and abs(sum(weights)-1)<1e-5
        manifest={'source':'Original CL9 '+self.species+' through Blender MCP','mesh':mesh.name,'skeleton':rig.name,
                  'dimensions_cm':[x*100 for x in mesh.dimensions],'triangles':len(mesh.data.loop_triangles),'vertices':len(mesh.data.vertices),
                  'deform_bones':len(data.bones),'material_slots':len(mesh.data.materials),'vertex_color':'Color, linear RGBA; UE VertexColor -> BaseColor',
                  'axis':'+X forward +Z up; root floor origin','weight_check':'sum 1, max 2 influences','clips':{}}
        (self.root/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
        bpy.data.libraries.write(str(self.root/(self.species.lower()+'_a.blend')),{self.scene},fake_user=True)
        print(json.dumps(manifest));return mesh,rig
