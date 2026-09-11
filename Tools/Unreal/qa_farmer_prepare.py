PREFIX='QA024_';FIXTURE_EVIDENCE='024-farmer-fixtures.json';MESH_PATH='/Engine/BasicShapes/Cube';MESH_PATHS={};FIXTURE_SCALES={'Floor':(14,10,.3)};FIXTURE_COLLISION_PROFILES={};FIXTURES=[('Floor',(20000,0,985),0)]
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/qa_fence_prepare.py',encoding='utf-8').read(),'qa_fence_prepare.py','exec'))
a=sub.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector(20400,-350,1240));a.set_actor_label('QA024_Capture')
a.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(a.get_actor_location(),unreal.Vector(20000,0,1095)),False)
data['fixtures'].append({'label':a.get_actor_label(),'name':a.get_name(),'class':a.get_class().get_path_name()});out.write_text(json.dumps(data,indent=2),encoding='utf-8')
c=a.get_component_by_class(unreal.SceneCaptureComponent2D);c.set_editor_property('capture_every_frame',False);c.set_editor_property('capture_on_movement',False);c.set_editor_property('fov_angle',29)
print('Prepared one support floor and one QA capture camera.')
