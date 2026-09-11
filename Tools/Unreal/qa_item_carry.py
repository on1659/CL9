"""Exercise real carry RPCs in live two-player PIE; setup teleport is recorded.

Does not synthesize inventory/attachment, and re-finds actors after pocket respawn.
"""
import json
from datetime import datetime, timezone
from pathlib import Path
import unreal

worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
assert len(worlds) == 2, 'Requires two-player PIE'
host = next(w for w in worlds if 'UEDPIE_0_' in w.get_path_name())
pawn = next(a for a in unreal.GameplayStatics.get_all_actors_of_class(host, unreal.Character)
            if a.get_controller() and a.get_controller().is_local_controller())
controller = pawn.get_controller()
ps = pawn.player_state
assert not ps.pocket_items and str(ps.hand_item.item_alias) == 'None', 'Start with empty inventory'
def eggs(world):
    return [a for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor)
            if a.get_class().get_name() == 'BP_CL9Item_C' and str(a.get_editor_property('alias')) == 'egg']
item = next(a for a in eggs(host) if str(a.get_editor_property('sub_alias')) == 'None' and not a.get_attach_parent_actor())
original = pawn.get_actor_location().to_tuple()
loc = item.get_actor_location()
half = pawn.get_capsule_component().get_scaled_capsule_half_height()
pawn.set_actor_location(unreal.Vector(loc.x-70,loc.y,loc.z-21+half+2),False,True)
unreal.SystemLibrary.execute_console_command(host,'cl9.pick 1 2',controller)
controller.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),loc))
stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
output = Path(unreal.Paths.project_dir(),'docs/asset/evidence',f'001-carry-{stamp}.json')
state = {'status':'running','engine':unreal.SystemLibrary.get_engine_version(),'utc':stamp,
         'setup':{'original_pawn_location':original,'test_pawn_location':pawn.get_actor_location().to_tuple(),
                  'item_location':loc.to_tuple(),'input':'real Server_PickPlace/Swap/Throw RPCs; no synthetic inventory'},
         'elapsed':0,'step':0,'events':[]}

def save(): output.write_text(json.dumps(state,indent=2),encoding='utf-8')
def capture(label):
    event={'event':label,'elapsed':state['elapsed'],'worlds':[]}
    for w in worlds:
        rows=[]
        for a in eggs(w):
            sm=a.get_component_by_class(unreal.StaticMeshComponent)
            parent=a.get_attach_parent_actor()
            origin,extent,radius=unreal.SystemLibrary.get_component_bounds(sm)
            rows.append({'name':a.get_name(),'sub_alias':str(a.get_editor_property('sub_alias')),
                         'mesh':sm.static_mesh.get_path_name(),'attached':bool(parent),
                         'parent_player_id':parent.player_state.player_id if parent else None,
                         'location':a.get_actor_location().to_tuple(),'scale':sm.get_world_scale().to_tuple(),
                         'bottom_z':origin.z-extent.z,'velocity':a.get_velocity().to_tuple()})
        players=[]
        for p in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PlayerState):
            if hasattr(p,'pocket_items'):
                players.append({'id':p.player_id,'hand':str(p.hand_item.item_alias),
                                'pocket':[str(v.item_alias) for v in p.pocket_items]})
        event['worlds'].append({'world':w.get_path_name(),'items':rows,'players':players})
    state['events'].append(event);save()
    return event

def expect_inventory(event, hand, pocket):
    for w in event['worlds']:
        p=next(p for p in w['players'] if p['id']==ps.player_id)
        assert p['hand']==hand and p['pocket']==pocket, f"Inventory mismatch: {w['world']} {p}"
        attached=[a for a in w['items'] if a['parent_player_id']==ps.player_id]
        assert len(attached)==(1 if hand=='egg' else 0), f"Attachment mismatch: {w['world']}"
        for a in attached:
            assert a['mesh'].endswith('SM_Egg_Raw_A.SM_Egg_Raw_A'), 'Wrong respawn mesh'

def tick(delta):
    try:
        assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor(), 'PIE stopped'
        state['elapsed']+=delta
        t=state['elapsed'];s=state['step']
        if s==0 and t>1:
            controller.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),item.get_actor_location()))
            capture('before_pickup');pawn.call_method('Server_PickPlace');state['step']=1
        elif s==1 and t>3:
            expect_inventory(capture('held'), 'egg', []);pawn.call_method('Server_Swap');state['step']=2
        elif s==2 and t>5:
            expect_inventory(capture('pocketed'), 'None', ['egg']);pawn.call_method('Server_Swap');state['step']=3
        elif s==3 and t>7:
            expect_inventory(capture('unpacked'), 'egg', [])
            controller.set_control_rotation(unreal.Rotator(pitch=0,yaw=0,roll=0))
            pawn.call_method('Server_ThrowChargeStart');state['step']=4
        elif s==4 and t>7.25:
            pawn.call_method('Server_ThrowRelease');state['step']=5
        elif s==5 and t>8:
            expect_inventory(capture('thrown'), 'None', []);state['step']=6
        elif s==6 and t>11:
            capture('settled_after_throw');state['status']='passed_carry_replication'
            save();unreal.unregister_slate_post_tick_callback(handle)
    except Exception as error:
        state['status']='failed';state['error']=str(error);save()
        unreal.unregister_slate_post_tick_callback(handle)
save()
handle=unreal.register_slate_post_tick_callback(tick)
print('Carry test registered:',str(output))
