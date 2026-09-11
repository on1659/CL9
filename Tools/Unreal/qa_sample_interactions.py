"""Real PIE server interaction path + host/client observations. No game-state fabrication."""
import json
from pathlib import Path
import unreal

worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
assert len(worlds)==2,'Expected host and client PIE worlds'
host=next(w for w in worlds if 'UEDPIE_0_' in w.get_path_name())
pawn=next(a for a in unreal.GameplayStatics.get_all_actors_of_class(host,unreal.Character)
          if a.get_controller() and a.get_controller().is_local_controller())
controller=pawn.get_controller()
item=next(a for a in unreal.GameplayStatics.get_all_actors_of_class(host,unreal.Actor)
          if a.get_name()=='BP_CL9Item_C_0')
camera=pawn.get_component_by_class(unreal.CameraComponent)
controller.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),item.get_actor_location()))
state={'elapsed':0,'step':0,'events':[],'status':'running'}
output=Path(unreal.Paths.project_dir(),'docs/asset/evidence/001-pie-interactions-retry.json')
def capture(label):
    result={'event':label,'game_seconds':state['elapsed'],'worlds':[]}
    for w in worlds:
        rows=[]
        for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor):
            if a.get_class().get_name()=='BP_CL9Item_C':
                sm=a.get_component_by_class(unreal.StaticMeshComponent)
                parent=a.get_attach_parent_actor()
                rows.append({'name':a.get_name(),'alias':str(a.get_editor_property('alias')),
                    'sub_alias':str(a.get_editor_property('sub_alias')),'mesh':sm.static_mesh.get_path_name(),
                    'attached':bool(parent),'location':a.get_actor_location().to_tuple(),
                    'mesh_scale':sm.get_world_scale().to_tuple()})
        result['worlds'].append({'world':w.get_path_name(),'items':rows})
    state['events'].append(result)
    output.write_text(json.dumps(state,indent=2),encoding='utf-8')
def tick(delta):
    try:
        if not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor():
            raise RuntimeError('PIE stopped during interaction test')
        state['elapsed']+=delta
        t=state['elapsed'];s=state['step']
        if s==0 and t>1:
            controller.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),item.get_actor_location()))
            capture('before_pickup');pawn.call_method('Server_PickPlace');state['step']=1
        elif s==1 and t>2:
            capture('after_pickup')
            assert item.get_attach_parent_actor()==pawn,'Pickup failed: inspect focus/range before proceeding'
            pawn.call_method('Server_Swap');state['step']=2
        elif s==2 and t>3.5:
            capture('after_pocket');pawn.call_method('Server_Swap');state['step']=3
        elif s==3 and t>5:
            capture('after_unpocket')
            controller.set_control_rotation(unreal.Rotator(pitch=0,yaw=0,roll=0))
            pawn.call_method('Server_ThrowChargeStart');state['step']=4
        elif s==4 and t>5.25:
            pawn.call_method('Server_ThrowRelease');state['step']=5
        elif s==5 and t>7:
            capture('after_throw');state['status']='observed'
            output.write_text(json.dumps(state,indent=2),encoding='utf-8')
            unreal.unregister_slate_post_tick_callback(handle)
    except Exception as error:
        state['status']='failed';state['error']=str(error)
        output.write_text(json.dumps(state,indent=2),encoding='utf-8')
        unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick)
print('Registered interaction observation callback',handle)
