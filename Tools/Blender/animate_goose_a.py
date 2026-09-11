"""Bake goose locomotion, neck alarm and sleeping poses without gameplay notifies."""
import bpy,math,json
from pathlib import Path
from mathutils import Vector,Matrix,Euler
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster/goose_a');s=bpy.data.scenes['CL9_GOOSE_A'];bpy.context.window.scene=s
r=bpy.data.objects['SKEL_Goose_A'];m=bpy.data.objects['SK_Goose_A'];r.animation_data_create()
assert not any(a.name.startswith('AN_Goose_') for a in bpy.data.actions),'Inspect existing goose actions first'
rest={b.name:b.matrix_local.copy() for b in r.data.bones}
spec={'Idle':(2,True,0),'Walk':(.8,True,126),'Run':(.5,True,528),'LookAround':(2,True,0),
      'HitReact':(.3,False,0),'WakeTelegraph':(.8,False,0),'Death':(1.2,False,0),
      'Sleep':(2,True,0),'AlertWindup':(.55,False,0),'Honk':(.6,False,0)}
def relative(name,rotation=(0,0,0)):
    p=r.pose.bones[name];base=p.parent.matrix @ rest[p.parent.name].inverted() @ rest[name] if p.parent else rest[name]
    q=rest[name].to_quaternion().to_matrix().to_4x4()
    p.matrix=base @ q.inverted() @ Euler(rotation,'XYZ').to_matrix().to_4x4() @ q
    bpy.context.view_layer.update()
def segment(name,head,tail):
    bone=r.data.bones[name];q=(bone.tail_local-bone.head_local).rotation_difference(Vector(tail)-Vector(head)) @ rest[name].to_quaternion()
    r.pose.bones[name].matrix=Matrix.Translation(head) @ q.to_matrix().to_4x4();bpy.context.view_layer.update()
def leg(side,target):
    upper='thigh_'+side;lower='shin_'+side;foot='foot_'+side
    hip=r.pose.bones['body'].matrix @ rest['body'].inverted() @ r.data.bones[upper].head_local
    l1=r.data.bones[upper].length;l2=r.data.bones[lower].length;distance=(target-hip).length
    assert abs(l1-l2)<distance<l1+l2+.0001,(side,distance,l1+l2)
    axis=(target-hip).normalized();a=(l1*l1-l2*l2+distance*distance)/(2*distance)
    bend=(Vector((1,0,0))-axis*axis.x).normalized();knee=hip+axis*a+bend*math.sqrt(max(0,l1*l1-a*a))
    segment(upper,hip,knee);segment(lower,knee,target)
    r.pose.bones[foot].matrix=Matrix.Translation(target) @ rest[foot].to_quaternion().to_matrix().to_4x4();bpy.context.view_layer.update()
manifest=json.loads((ROOT/'export/manifest.json').read_text())
for name,(duration,loop,speed) in spec.items():
    fps=60 if name=='AlertWindup' else 30;s.render.fps=fps;end=round(duration*fps)+1;s.frame_start=1;s.frame_end=end
    action=bpy.data.actions.new('AN_Goose_'+name);action.use_fake_user=True;r.animation_data.action=action;contacts=[]
    for frame in range(1,end+1):
        s.frame_set(frame);u=(frame-1)/(end-1);phase=math.tau*u;pulse=math.sin(math.pi*u)**2
        for p in r.pose.bones:p.matrix_basis=Matrix.Identity(4)
        pos=Vector((-.16,0,.58));rot=(0,0,0);neck=[0,0,0];yaw=0;head=0;jaw=0;wing=0
        sleep=0
        if name in ('Idle','LookAround'):
            pos.z+=.003*math.sin(phase);neck=[.015*math.sin(phase),-.012*math.sin(phase),0]
            if name=='LookAround':yaw=.55*math.sin(phase)
        elif name in ('Walk','Run'):
            run=name=='Run';pos.z+=(-.145 if run else -.055)+(.012 if run else .006)*math.sin(phase*2)
            rot=(.025*math.sin(phase),.1 if run else 0,0)
            neck=[.45 if run else .06,-.25 if run else -.04,.05*math.sin(phase*2)];head=-.17 if run else 0;wing=.24 if run else .03
        elif name=='HitReact':
            pos.x-=.05*pulse;pos.z-=.04*pulse;neck=[-.20*pulse,.1*pulse,-.2*pulse];wing=.3*pulse
        elif name=='Sleep':sleep=1;pos.z-=.18;pos.z+=.004*math.sin(phase)
        elif name=='WakeTelegraph':
            sleep=(1-u)**2;pos.z-=.18*sleep;wing=.68*pulse;jaw=.18*pulse
        elif name=='AlertWindup':
            t=u*u*(3-2*u);neck=[-.15*t,.07*t,-.06*t];head=-.10*t;wing=.13*t
        elif name=='Honk':
            recovery=1-u;neck=[-.15*recovery+.62*pulse,.07*recovery-.36*pulse,-.06*recovery+.1*pulse]
            head=-.1*recovery-.22*pulse;jaw=.65*pulse;wing=.13*recovery+.7*pulse
        elif name=='Death':
            t=min(1,u/.75);ease=t*t*(3-2*t);pos.z-=.25*ease;rot=(1.50*ease,0,0)
            neck=[.35*ease,.32*ease,-.2*ease];head=.3*ease;wing=-.12*ease
        if sleep:
            neck=[-1.15*sleep,-.7*sleep,1.4*sleep];head=.4*sleep;yaw=.12*sleep
        r.pose.bones['root'].matrix=rest['root']
        r.pose.bones['body'].matrix=Matrix.Translation(pos) @ Euler(rot,'XYZ').to_matrix().to_4x4() @ rest['body'].to_quaternion().to_matrix().to_4x4()
        bpy.context.view_layer.update()
        for i,pitch in enumerate(neck):relative(f'neck_{i+1:02d}',(0,pitch,yaw*.25 if i==1 else 0))
        relative('head',(0,head,yaw*.75));relative('jaw',(0,jaw,0));relative('tail',(0,.02*math.sin(phase) if name!='Death' else 0,0))
        for side in ('l','r'):
            if 'eye_'+side in r.pose.bones:
                relative('eye_'+side)
                r.pose.bones['eye_'+side].scale.y=1-.94*(ease if name=='Death' else sleep)
        bpy.context.view_layer.update()
        for sign,side in [(1,'l'),(-1,'r')]:
            relative('wing_'+side,(-sign*wing,0,0));relative('wing_tip_'+side,(0,.1*wing,0))
            if name in ('Walk','Run'):
                duty=.25 if name=='Run' else .5;p=(u+(0 if side=='l' else .5))%1;travel=speed/100*duration*duty
                if p<duty:x=travel*(.5-p/duty);lift=0;stance=True
                else:
                    q=(p-duty)/(1-duty);x=-travel*.5+travel*(.5-.5*math.cos(math.pi*q));lift=(.14 if name=='Run' else .09)*math.sin(math.pi*q);stance=False
                target=Vector((-.11+x,sign*.17,.06+lift));leg(side,target)
                if stance:contacts.append({'frame':frame,'side':side,'x_cm':target.x*100,'z_cm':target.z*100})
            elif name=='Death':
                relative('thigh_'+side,(0,.5*ease,0));relative('shin_'+side);relative('foot_'+side)
            else:leg(side,Vector((-.08,sign*.17,.06)))
        if name=='Death':
            evaluated=m.evaluated_get(bpy.context.evaluated_depsgraph_get());lowest=min(v.co.z for v in evaluated.data.vertices)
            if lowest<.012:
                mat=r.pose.bones['body'].matrix.copy();mat.translation.z+=.012-lowest;r.pose.bones['body'].matrix=mat;bpy.context.view_layer.update()
        for p in r.pose.bones:
            for channel in ('location','rotation_quaternion','scale'):p.keyframe_insert(data_path=channel,frame=frame,group=p.name)
    s.frame_set(1);bpy.ops.object.select_all(action='DESELECT');r.select_set(True);bpy.context.view_layer.objects.active=r
    bpy.ops.export_scene.fbx(filepath=str(ROOT/'export'/f'{action.name}.fbx'),use_selection=True,object_types={'ARMATURE'},
        global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',add_leaf_bones=False,use_armature_deform_only=True,
        bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0)
    manifest['clips'][name]={'action':action.name,'duration_seconds':duration,'frames':[1,end],'fps':fps,'loop':loop,
                             'reference_speed_cm_s':speed,'root_motion':False,'stance_samples':contacts}
r.animation_data.action=None
for p in r.pose.bones:p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update();bpy.ops.object.select_all(action='DESELECT');r.select_set(True);m.select_set(True)
bpy.ops.export_scene.fbx(filepath=str(ROOT/'export/SK_Goose_A.fbx'),use_selection=True,object_types={'MESH','ARMATURE'},
    global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',add_leaf_bones=False,use_armature_deform_only=True,bake_anim=False)
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
r.animation_data.action=bpy.data.actions['AN_Goose_Idle'];s.render.fps=30;s.frame_start=1;s.frame_end=61;s.frame_set(1)
bpy.data.libraries.write(str(ROOT/'goose_a.blend'),{s,*[bpy.data.actions[c['action']] for c in manifest['clips'].values()]},fake_user=True)
print('Saved goose mesh and 10 clips')
