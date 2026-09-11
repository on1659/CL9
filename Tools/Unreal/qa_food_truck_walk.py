"""Drive the real pawn's movement component up/down the four steps and through the aisle."""
import unreal,json,datetime
from pathlib import Path
w=unreal.EditorLevelLibrary.get_pie_worlds(False)[0]
pawn=unreal.GameplayStatics.get_player_pawn(w,0);cap=pawn.get_component_by_class(unreal.CapsuleComponent)
move=pawn.get_component_by_class(unreal.CharacterMovementComponent);pc=unreal.GameplayStatics.get_player_controller(w,0)
out=Path(unreal.Paths.project_dir(),'docs/asset/evidence/023-food-truck-walk.json');assert not out.exists()
move.stop_movement_immediately();pawn.set_actor_location(unreal.Vector(19520,70.25,1092),False,True)
pc.set_control_rotation(unreal.Rotator(pitch=0,yaw=0,roll=0))
start=unreal.GameplayStatics.get_time_seconds(w);last_sample=-1
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'level':w.get_path_name(),'status':'running','phase':'enter','samples':[],
 'scope':'One real pawn, setup teleport only, AddMovementInput through CharacterMovement up steps/aisle and back down. No keyboard/camera/2-player claim.'}
def save():out.write_text(json.dumps(r,indent=2),encoding='utf-8')
save()
def tick(delta):
 global last_sample
 try:
  assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
  t=unreal.GameplayStatics.get_time_seconds(w)-start;p=pawn.get_actor_location();bottom=p.z-cap.get_scaled_capsule_half_height()
  if t-last_sample>.12:r['samples'].append({'t':t,'phase':r['phase'],'location':p.to_tuple(),'feet_z':bottom});last_sample=t;save()
  assert t<20,'Movement did not complete within 20 game seconds'
  assert abs(p.y-70.25)<5 and bottom>998,'Pawn left aisle or fell through support'
  if t<.5:return
  if r['phase']=='enter' and p.x>=20025:
   assert abs(bottom-1064)<4,'Pawn did not stand on cargo floor'
   r['phase']='exit';r['entered_at_seconds']=t;move.stop_movement_immediately()
  if r['phase']=='exit' and p.x<=19520:
   assert abs(bottom-1000)<4,'Pawn did not return to ground'
   move.stop_movement_immediately();r['status']='passed_real_pawn_up_aisle_down';r['elapsed_seconds']=t;save();unreal.unregister_slate_post_tick_callback(handle);return
  pawn.add_movement_input(unreal.Vector(1 if r['phase']=='enter' else -1,0,0),1,False)
 except Exception as e:
  move.stop_movement_immediately();r['status']='failed';r['error']=str(e);save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick);print('Registered food-truck walk callback '+str(handle))
