"""Bake original in-place chicken actions with explicit two-bone foot placement."""
import bpy,math,json
from mathutils import Vector,Matrix,Euler
from pathlib import Path
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/chicken_a')
scene=bpy.data.scenes['CL9_CHICKEN_A'];bpy.context.window.scene=scene
rig=bpy.data.objects['SKEL_Chicken_A'];mesh=bpy.data.objects['SK_Chicken_A']
rig.animation_data_create()
assert not any(a.name.startswith('AN_Chicken_') for a in bpy.data.actions), 'Actions exist; inspect first'
rest={b.name:b.matrix_local.copy() for b in rig.data.bones}
spec={'Idle':(2,True,0),'Walk':(.8,True,126),'Run':(.5,True,396),'LookAround':(2,True,0),
      'HitReact':(.3,False,0),'WakeTelegraph':(.8,False,0),'Catch':(.6,False,0),'Death':(1.2,False,0)}
def transform_at(head,rotation,rest_rotation):
    return Matrix.Translation(head) @ Euler(rotation,'XYZ').to_matrix().to_4x4() @ rest_rotation.to_matrix().to_4x4()
def relative(name,rotation=(0,0,0)):
    pb=rig.pose.bones[name];rb=rig.data.bones[name]
    base=pb.parent.matrix @ rest[pb.parent.name].inverted() @ rest[name] if pb.parent else rest[name]
    pb.matrix=base @ Euler(rotation,'XYZ').to_matrix().to_4x4()
    bpy.context.view_layer.update()
def segment(name,head,tail):
    b=rig.data.bones[name]
    q=(b.tail_local-b.head_local).rotation_difference(Vector(tail)-Vector(head)) @ rest[name].to_quaternion()
    rig.pose.bones[name].matrix=Matrix.Translation(head) @ q.to_matrix().to_4x4()
    bpy.context.view_layer.update()
def leg(side,foot):
    hip=rig.pose.bones['body'].matrix @ rest['body'].inverted() @ rig.data.bones['thigh_'+side].head_local
    l1=rig.data.bones['thigh_'+side].length;l2=rig.data.bones['shin_'+side].length
    d=(foot-hip).length
    assert d < l1+l2+.0001, f'Unreachable foot {side}: {d} > {l1+l2}'
    axis=(foot-hip).normalized();a=(l1*l1-l2*l2+d*d)/(2*d)
    perp=(Vector((1,0,0))-axis*axis.x).normalized()
    knee=hip+axis*a+perp*math.sqrt(max(0,l1*l1-a*a))
    segment('thigh_'+side,hip,knee);segment('shin_'+side,knee,foot)
    rig.pose.bones['foot_'+side].matrix=Matrix.Translation(foot) @ rest['foot_'+side].to_quaternion().to_matrix().to_4x4()
    bpy.context.view_layer.update()
    assert (rig.pose.bones['foot_'+side].head-foot).length < .0001, 'Foot placement did not evaluate'

manifest=json.loads((ROOT/'export/manifest.json').read_text())
for name,(duration,loop,speed) in spec.items():
    action=bpy.data.actions.new('AN_Chicken_'+name);action.use_fake_user=True;rig.animation_data.action=action
    end=round(duration*30)+1;scene.frame_start=1;scene.frame_end=end
    contact=[]
    for frame in range(1,end+1):
        scene.frame_set(frame);u=(frame-1)/(end-1);phase=math.tau*u
        for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
        body_pos=Vector((-.06,0,.55));body_rot=(0,0,0);neck_rot=(0,0,0);head_rot=(0,0,0);wing=0;jaw=0
        if name in ('Idle','LookAround'):
            body_pos.z+=.005*math.sin(phase)
            neck_rot=(.025*math.sin(phase),0,0)
            if name=='LookAround':head_rot=(0,.12*math.sin(phase),.60*math.sin(phase))
        elif name in ('Walk','Run'):
            run=name=='Run';body_pos.z+=(-.105 if run else -.055)+(.012 if run else .007)*math.sin(phase*2)
            body_rot=(.025*math.sin(phase),.18 if run else .035,0)
            head_rot=(0,-.10*math.sin(phase*2),0);wing=.23 if run else .025
        elif name=='HitReact':
            pulse=math.sin(math.pi*u);body_pos.x-=.065*pulse;body_pos.z-=.035*pulse
            body_rot=(0,-.20*pulse,.10*pulse);head_rot=(0,-.34*pulse,0);wing=.3*pulse
        elif name=='WakeTelegraph':
            pulse=math.sin(math.pi*u);body_pos.z+=.065*pulse;body_rot=(0,-.13*pulse,0)
            wing=.85*pulse*(.75+.25*math.sin(phase*3));jaw=.30*pulse
        elif name=='Catch':
            pulse=math.sin(math.pi*u)**2;body_pos.x+=.15*pulse;body_pos.z-=.055*pulse
            body_rot=(0,.20*pulse,0);head_rot=(0,.40*pulse,0);wing=.45*pulse;jaw=.32*pulse
        elif name=='Death':
            fall=min(1,u/.75);ease=fall*fall*(3-2*fall)
            body_pos.z-=.19*ease;body_pos.y-=.06*ease;body_rot=(1.46*ease,.1*ease,0)
            head_rot=(0,.32*ease,-.2*ease);wing=-.15*ease
        rig.pose.bones['root'].matrix=rest['root']
        rig.pose.bones['body'].matrix=transform_at(body_pos,body_rot,rest['body'].to_quaternion())
        bpy.context.view_layer.update()
        relative('neck',neck_rot);relative('head',head_rot);relative('jaw',(-jaw,0,0))
        relative('comb',(.04*math.sin(phase)* (0 if name=='Death' else 1),0,0));relative('tail',(0,.06*math.sin(phase),0))
        for sign,side in [(1,'l'),(-1,'r')]:
            relative('wing_'+side,(sign*wing,0,sign*.1*wing));relative('wing_tip_'+side,(sign*.3*wing,0,0))
            if name in ('Walk','Run'):
                duty=.30 if name=='Run' else .5;p=(u+(0 if side=='l' else .5))%1
                travel=speed/100*duration*duty
                if p<duty:
                    x=travel*(.5-p/duty);z=.065;stance=True
                else:
                    swing=(p-duty)/(1-duty);x=-travel*.5+travel*(.5-.5*math.cos(math.pi*swing))
                    z=.065+(.16 if name=='Run' else .085)*math.sin(math.pi*swing);stance=False
                foot=Vector((-.075+x,sign*.145,z));leg(side,foot)
                if stance:contact.append({'frame':frame,'side':side,'z_cm':z*100,'x_cm':foot.x*100})
            elif name=='Death':
                relative('thigh_'+side,(0,.35*u,0));relative('shin_'+side);relative('foot_'+side)
            elif name in ('Idle','LookAround','HitReact','Catch'):
                leg(side,Vector((-.055,sign*.145,.065)))
            else:
                relative('thigh_'+side);relative('shin_'+side);relative('foot_'+side)
        bpy.context.view_layer.update()
        if name=='Death':
            evaluated=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get())
            lowest=min((evaluated.matrix_world@v.co).z for v in evaluated.data.vertices)
            if lowest<.0088:
                body_matrix=rig.pose.bones['body'].matrix.copy()
                body_matrix.translation.z+=.0088-lowest
                rig.pose.bones['body'].matrix=body_matrix
                bpy.context.view_layer.update()
        for p in rig.pose.bones:
            p.keyframe_insert(data_path='location',frame=frame,group=p.name)
            p.keyframe_insert(data_path='rotation_quaternion',frame=frame,group=p.name)
            p.keyframe_insert(data_path='scale',frame=frame,group=p.name)
    # Explicit single-action FBX; only deform armature, no NLA/all-actions duplication.
    scene.frame_set(1)
    bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);bpy.context.view_layer.objects.active=rig
    bpy.ops.export_scene.fbx(filepath=str(ROOT/'export'/f'{action.name}.fbx'),use_selection=True,object_types={'ARMATURE'},
        global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',add_leaf_bones=False,use_armature_deform_only=True,
        bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0,
        bake_anim_force_startend_keying=True)
    manifest['clips'][name]={'action':action.name,'duration_seconds':duration,'frames':[1,end],'fps':30,
                             'loop':loop,'reference_speed_cm_s':speed,'root_motion':False,'stance_samples':contact}
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update()
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);mesh.select_set(True);bpy.context.view_layer.objects.active=rig
bpy.ops.export_scene.fbx(filepath=str(ROOT/'export/SK_Chicken_A.fbx'),use_selection=True,object_types={'MESH','ARMATURE'},
    global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',add_leaf_bones=False,use_armature_deform_only=True,bake_anim=False)
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
rig.animation_data.action=bpy.data.actions['AN_Chicken_Idle'];scene.frame_start=1;scene.frame_end=61;scene.frame_set(1)
bpy.data.libraries.write(str(ROOT/'chicken_a.blend'),{scene,*[bpy.data.actions['AN_Chicken_'+k] for k in spec]},fake_user=True)
print('Exported chicken rig and actions:',list(manifest['clips']))
