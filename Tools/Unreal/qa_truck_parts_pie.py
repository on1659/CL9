"""Local actual healing with exit reset, then real lobby shop-panel interaction."""
import unreal,json,datetime,math
from pathlib import Path
worlds=unreal.EditorLevelLibrary.get_pie_worlds(False);assert worlds
w=next(x for x in worlds if 'UEDPIE_0_' in x.get_path_name())
actors={a.get_actor_label().replace('QA022_',''):a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) if a.get_actor_label().startswith('QA022_')};assert len(actors)==4
pad=actors['Pad'];panel=actors['Panel'];probe=actors['Probe'];pawn=unreal.GameplayStatics.get_player_pawn(w,0)
pc=unreal.GameplayStatics.get_player_controller(w,0);ps=pawn.player_state;cap=pawn.get_component_by_class(unreal.CapsuleComponent)
zone=next(c for c in pad.get_components_by_class(unreal.BoxComponent) if c.get_name()=='HealZone')
gi=unreal.GameplayStatics.get_game_instance(w)
managers=[m for m in unreal.ObjectIterator(unreal.CL9UIManager) if m.get_outer()==gi]
assert len(managers)==1
manager=managers[0];assert not manager.is_open('Shop')
assert ps.hp==ps.max_hp and ps.max_hp>1
heal_time=unreal.load_asset('/Game/Data/DA_CL9Rules').get_editor_property('truck_pad_heal_time');assert heal_time>2
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/022-truck-pie.json');assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'world_count':len(worlds),
 'level':w.get_path_name(),'status':'running','phase':'damage','heal_time':heal_time,'checks':[],
 'scope':'Actual pad BP + damage cheat + timed enter/exit reset; real panel pickup/interact RPC opens Shop; no purchase/2-player/keyboard walking claim.'}
def save():out.write_text(json.dumps(r,indent=2),encoding='utf-8')
def move(inside):
 pawn.set_actor_location(unreal.Vector(20000 if inside else 20400,0,(1020 if inside else 1000)+cap.get_scaled_capsule_half_height()+2),False,True)
def capture(case):
 e={'case':case,'elapsed':r['elapsed'],'hp':ps.hp,'max_hp':ps.max_hp,'overlapping':zone.is_overlapping_actor(pawn),'pawn_location':pawn.get_actor_location().to_tuple()};r['checks'].append(e);return e
move(False);save();start=unreal.GameplayStatics.get_time_seconds(w);next_time=start+1
def tick(delta):
 global next_time
 try:
  assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
  now=unreal.GameplayStatics.get_time_seconds(w);r['elapsed']=now-start
  assert r['elapsed']<90,'Timed out in game seconds'
  if now<next_time:return
  next_time=now+.5;phase=r['phase']
  if phase=='damage':
   unreal.SystemLibrary.execute_console_command(w,'cl9.damage 1',pc);r['phase']='outside_wait';next_time=now+heal_time+.5
  elif phase=='outside_wait':
   e=capture('outside_not_healed');assert e['hp']==e['max_hp']-1 and not e['overlapping'];move(True);r['phase']='short_stay';next_time=now+heal_time*.6
  elif phase=='short_stay':
   e=capture('partial_stay');assert e['overlapping'] and e['hp']<e['max_hp'];move(False);r['phase']='exit';next_time=now+1
  elif phase=='exit':
   e=capture('exit');assert not e['overlapping'] and e['hp']<e['max_hp'];move(True);r['phase']='reset_check';next_time=now+heal_time*.6;r['reentered']=now
  elif phase=='reset_check':
   e=capture('reentry_timer_reset');assert e['overlapping'] and e['hp']<e['max_hp'];r['phase']='full_stay';next_time=r['reentered']+heal_time+.5
  elif phase=='full_stay':
   e=capture('full_heal');assert e['hp']==e['max_hp'] and e['overlapping'];move(False)
   pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(pawn.get_actor_location(),panel.get_actor_location()))
   r['phase']='panel_focus';next_time=now+1
  elif phase=='panel_focus':
   r['checks'].append({'case':'panel_focus','focused':pawn.get_focused_actor()==panel,'prompt':str(pawn.get_focus_prompt())})
   assert pawn.get_focused_actor()==panel;r['screen_before']=str(manager.get_open_screen())
   pawn.call_method('Server_PickPlace');r['phase']='shop';next_time=now+1
  elif phase=='shop':
   r['checks'].append({'case':'shop_open','open':manager.is_open('Shop'),'screen':str(manager.get_open_screen()),'manager':manager.get_path_name()})
   assert manager.is_open('Shop') and str(manager.get_open_screen())=='Shop'
   manager.close_widget('Shop');r['phase']='geometry';next_time=now+.5
  elif phase=='geometry':
   assert not manager.is_open('Shop')
   for label,a,expected in [('pad',pad,[400,400,20]),('panel',panel,[15,80,120])]:
    sm=a.get_component_by_class(unreal.StaticMeshComponent);o,e,_=unreal.SystemLibrary.get_component_bounds(sm)
    r['checks'].append({'case':label+'_bounds','dimensions_cm':(e*2).to_tuple(),'bottom_z':o.z-e.z,'mesh':sm.static_mesh.get_path_name()})
    assert max(abs(x-y) for x,y in zip((e*2).to_tuple(),expected))<.01 and abs(o.z-e.z-1000)<.01
   zo,ze,_=unreal.SystemLibrary.get_component_bounds(zone)
   r['checks'].append({'case':'heal_zone','origin':zo.to_tuple(),'dimensions_cm':(ze*2).to_tuple()})
   assert max(abs(x-y) for x,y in zip((ze*2).to_tuple(),[400,400,240]))<.01 and abs(zo.z-1120)<.01
   psm=probe.static_mesh_component;po,pe,_=unreal.SystemLibrary.get_component_bounds(psm)
   gap=po.z-pe.z-1020;speed=math.sqrt(sum(v*v for v in psm.get_physics_linear_velocity().to_tuple()))
   r['checks'].append({'case':'pad_probe_support','gap_cm':gap,'speed_cm_s':speed});assert abs(gap)<1 and speed<2
   r['status']='passed_local_heal_reset_and_shop_open';r['phase']='complete';save();unreal.unregister_slate_post_tick_callback(handle);return
  save()
 except Exception as error:
  r['status']='failed';r['error']=str(error);save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick);print('Truck parts QA callback registered: '+str(out))
