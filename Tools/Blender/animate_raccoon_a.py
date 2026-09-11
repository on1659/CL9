"""Original quadruped actions with baked paw placement, facial poses and weighted tail."""
import bpy,math,json
from mathutils import Vector,Matrix,Euler
from pathlib import Path
SPECIES=globals().get('SPECIES','Raccoon')
ROOT=Path('D:/Work/unreal/CL9/ArtSource/monster')/(SPECIES.lower()+'_a')
s=bpy.data.scenes['CL9_'+SPECIES.upper()+'_A'];bpy.context.window.scene=s
r=bpy.data.objects['SKEL_'+SPECIES+'_A'];m=bpy.data.objects['SK_'+SPECIES+'_A'];r.animation_data_create()
assert not any(a.name.startswith('AN_'+SPECIES+'_') for a in bpy.data.actions),'Existing actions: inspect before regenerating'
rest={b.name:b.matrix_local.copy() for b in r.data.bones}
spec={'Idle':(2,True,0),'Walk':(.8,True,140),'Run':(.5,True,462),'LookAround':(2,True,0),
      'HitReact':(.3,False,0),'WakeTelegraph':(.8,False,0),'Catch':(.6,False,0),'Death':(1.2,False,0)}
if SPECIES in ('Pig','Sheep'):spec.update({'Walk':(.8,True,126),'Run':(.5,True,396)})
def relative(name,rotation=(0,0,0)):
    p=r.pose.bones[name];base=p.parent.matrix @ rest[p.parent.name].inverted() @ rest[name] if p.parent else rest[name]
    q=rest[name].to_quaternion().to_matrix().to_4x4()
    p.matrix=base @ q.inverted() @ Euler(rotation,'XYZ').to_matrix().to_4x4() @ q
    bpy.context.view_layer.update()
def segment(name,head,tail):
    b=r.data.bones[name];q=(b.tail_local-b.head_local).rotation_difference(Vector(tail)-Vector(head)) @ rest[name].to_quaternion()
    r.pose.bones[name].matrix=Matrix.Translation(head) @ q.to_matrix().to_4x4();bpy.context.view_layer.update()
def limb(tag,paw):
    upper=tag+'_upper';lower=tag+'_lower';foot=tag+'_paw';parent=r.pose.bones[upper].parent
    hip=parent.matrix @ rest[parent.name].inverted() @ r.data.bones[upper].head_local
    l1=r.data.bones[upper].length;l2=r.data.bones[lower].length;dist=(paw-hip).length
    assert abs(l1-l2)<dist<l1+l2+.0001,f'Unreachable paw {tag}: {dist}, reach {l1+l2}'
    direction=(paw-hip).normalized();a=(l1*l1-l2*l2+dist*dist)/(2*dist)
    preferred=Vector((1 if tag.startswith('front') else -1,0,0))
    bend=(preferred-direction*direction.dot(preferred)).normalized()
    knee=hip+direction*a+bend*math.sqrt(max(0,l1*l1-a*a))
    segment(upper,hip,knee);segment(lower,knee,paw)
    r.pose.bones[foot].matrix=Matrix.Translation(paw) @ rest[foot].to_quaternion().to_matrix().to_4x4()
    bpy.context.view_layer.update();assert (r.pose.bones[foot].head-paw).length<.0001
manifest=json.loads((ROOT/'export/manifest.json').read_text())
for name,(duration,loop,speed) in spec.items():
    action=bpy.data.actions.new('AN_'+SPECIES+'_'+name);action.use_fake_user=True;r.animation_data.action=action
    end=round(duration*30)+1;s.frame_start=1;s.frame_end=end;contacts=[]
    for frame in range(1,end+1):
        s.frame_set(frame);u=(frame-1)/(end-1);phase=math.tau*u;pulse=math.sin(math.pi*u)**2
        for p in r.pose.bones:p.matrix_basis=Matrix.Identity(4)
        pos=Vector((-.30,0,.53));rotation=(0,0,0);chest=(0,0,0);neck=(0,0,0);head=(0,0,0);jaw=0
        if name in ('Idle','LookAround'):
            pos.z+=.002*math.sin(phase)
            if name=='LookAround':head=(0,-.07*math.sin(phase),.5*math.sin(phase));neck=(0,0,.12*math.sin(phase))
        elif name in ('Walk','Run'):
            run=name=='Run';pos.z+=(-.145 if run else -.115)+(.014 if run else .007)*math.sin(phase*2)
            rotation=(.008*math.sin(phase),.025 if run else 0,0);chest=(0,.02*math.sin(phase*2),0)
            head=(0,-.03*math.sin(phase*2),0)
        elif name=='HitReact':
            pos.x-=.035*pulse;pos.z-=.04*pulse;rotation=(0,-.04*pulse,.06*pulse);head=(0,-.22*pulse,.12*pulse)
        elif name=='WakeTelegraph':
            pos.z-=.025*pulse;chest=(0,-.13*pulse,0);neck=(0,-.2*pulse,0);jaw=.25*pulse
        elif name=='Catch':
            pos.x+=.10*pulse;pos.z-=.085*pulse;chest=(0,.06*pulse,0);neck=(0,.20*pulse,0);jaw=.42*pulse
        elif name=='Death':
            t=min(1,u/.75);ease=t*t*(3-2*t);pos.z-=.24*ease;rotation=(1.47*ease,0,0)
            head=(0,.13*ease,-.08*ease);jaw=.16*ease
        r.pose.bones['root'].matrix=rest['root']
        r.pose.bones['body'].matrix=Matrix.Translation(pos) @ Euler(rotation,'XYZ').to_matrix().to_4x4() @ rest['body'].to_quaternion().to_matrix().to_4x4()
        bpy.context.view_layer.update();relative('chest',chest);relative('neck',neck);relative('head',head);relative('jaw',(0,jaw,0))
        for sign,side in [(1,'l'),(-1,'r')]:relative('ear_'+side,(sign*.03*math.sin(phase),0,0))
        for i in range(4):
            relative(f'tail_{i+1:02d}',(0,.015*math.sin(phase+i*.5),.05*math.sin(phase+i*.4)) if name!='Death' else (0,0,0))
        for pair in ('front','hind'):
            for sign,side in [(1,'l'),(-1,'r')]:
                tag=pair+'_'+side;base=r.data.bones[tag+'_paw'].head_local.copy()
                if name in ('Walk','Run'):
                    duty=.25 if name=='Run' else .45
                    offset=0 if tag in ('front_l','hind_r') else .5
                    p=(u+offset)%1;travel=speed/100*duration*duty
                    if p<duty:
                        x=travel*(.5-p/duty);lift=0;stance=True
                    else:
                        q=(p-duty)/(1-duty);x=-travel*.5+travel*(.5-.5*math.cos(math.pi*q))
                        lift=(.12 if name=='Run' else .07)*math.sin(math.pi*q);stance=False
                    # Center the stride below its shoulder/hip rather than ahead of it.
                    target=Vector((base.x-.035+x,base.y,base.z+lift));limb(tag,target)
                    if stance:contacts.append({'frame':frame,'limb':tag,'x_cm':target.x*100,'z_cm':target.z*100})
                elif name=='Death':
                    relative(tag+'_upper',(0,(.60 if pair=='front' else -.5)*ease,0));relative(tag+'_lower');relative(tag+'_paw')
                elif name=='WakeTelegraph' and pair=='front':
                    relative(tag+'_upper',(0,-.2*pulse,0));relative(tag+'_lower',(0,.25*pulse,0));relative(tag+'_paw')
                else:limb(tag,base)
        bpy.context.view_layer.update()
        if name=='Death':
            e=m.evaluated_get(bpy.context.evaluated_depsgraph_get());lowest=min(v.co.z for v in e.data.vertices)
            if lowest<.007:
                mat=r.pose.bones['body'].matrix.copy();mat.translation.z+=.007-lowest;r.pose.bones['body'].matrix=mat;bpy.context.view_layer.update()
        for p in r.pose.bones:
            for channel in ('location','rotation_quaternion','scale'):p.keyframe_insert(data_path=channel,frame=frame,group=p.name)
    s.frame_set(1);bpy.ops.object.select_all(action='DESELECT');r.select_set(True);bpy.context.view_layer.objects.active=r
    bpy.ops.export_scene.fbx(filepath=str(ROOT/'export'/f'{action.name}.fbx'),use_selection=True,object_types={'ARMATURE'},
        global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',add_leaf_bones=False,use_armature_deform_only=True,
        bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0)
    manifest['clips'][name]={'action':action.name,'duration_seconds':duration,'frames':[1,end],'fps':30,
        'loop':loop,'reference_speed_cm_s':speed,'root_motion':False,'stance_samples':contacts}
r.animation_data.action=None
for p in r.pose.bones:p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update();bpy.ops.object.select_all(action='DESELECT');r.select_set(True);m.select_set(True)
bpy.ops.export_scene.fbx(filepath=str(ROOT/'export'/('SK_'+SPECIES+'_A.fbx')),use_selection=True,object_types={'MESH','ARMATURE'},
    global_scale=1,apply_unit_scale=True,axis_forward='X',axis_up='Z',add_leaf_bones=False,use_armature_deform_only=True,bake_anim=False)
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
r.animation_data.action=bpy.data.actions['AN_'+SPECIES+'_Idle'];s.frame_start=1;s.frame_end=61;s.frame_set(1)
bpy.data.libraries.write(str(ROOT/(SPECIES.lower()+'_a.blend')),{s,*[bpy.data.actions['AN_'+SPECIES+'_'+k] for k in spec]},fake_user=True)
print('Saved',SPECIES,'skeleton and 8 actions')
