import unreal,json,datetime
from pathlib import Path
w=unreal.EditorLevelLibrary.get_pie_worlds(False)[0];pawn=unreal.GameplayStatics.get_player_pawn(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0)
body=pawn.mesh;arms=next(c for c in pawn.get_components_by_class(unreal.SkeletalMeshComponent) if c.get_name()=='FirstPersonMesh');body.set_skeletal_mesh(unreal.load_asset('/Game/CL9Art/Characters/FarmerA/SK_Farmer_A_Body'));arms.set_skeletal_mesh(unreal.load_asset('/Game/CL9Art/Characters/FarmerA/SK_Farmer_A_Arms'))
body.set_editor_property('override_materials',[]);arms.set_editor_property('override_materials',[])
move=pawn.get_component_by_class(unreal.CharacterMovementComponent);move.stop_movement_immediately();pawn.set_actor_location(unreal.Vector(20000,0,1092),False,True);pc.set_control_rotation(unreal.Rotator(pitch=0,yaw=0,roll=0))
cam=next(a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.SceneCapture2D) if a.get_actor_label()=='QA024_Capture');capture=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
rt=unreal.RenderingLibrary.create_render_target2d(w,768,1024,unreal.TextureRenderTargetFormat.RTF_RGBA8);capture.texture_target=rt;capture.capture_source=unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR
root=Path(unreal.Paths.project_dir());out=root/'docs/asset/evidence/024-farmer-pie.json';assert not out.exists()
r={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'level':w.get_path_name(),'status':'running','samples':[],'scope':'Actual CL9 pawn with authored body/arms in one PIE world; existing Unarmed/FP Copy AnimBP. No permanent data change, grip/2-player claim.'}
start=unreal.GameplayStatics.get_time_seconds(w);next_time=start+1;phase=0
def save():out.write_text(json.dumps(r,indent=2),encoding='utf-8')
save()
def tick(delta):
 global phase,next_time
 try:
  now=unreal.GameplayStatics.get_time_seconds(w);assert now-start<20
  if phase==1:pawn.add_movement_input(unreal.Vector(0,1,0),1,False)
  if now<next_time:return
  sample={'phase':phase,'time':now-start,'location':pawn.get_actor_location().to_tuple(),'body_anim':body.get_anim_instance().get_class().get_path_name() if body.get_anim_instance() else None,'arms_anim':arms.get_anim_instance().get_class().get_path_name() if arms.get_anim_instance() else None,'body_mesh':body.skeletal_mesh_asset.get_path_name(),'arms_mesh':arms.skeletal_mesh_asset.get_path_name(),'hand_l':body.get_socket_location('hand_l').to_tuple(),'foot_l':body.get_socket_location('foot_l').to_tuple()};r['samples'].append(sample)
  if phase==0:
   capture.capture_scene();phase=1;next_time=now+.5
  elif phase==1:
   unreal.RenderingLibrary.export_render_target(w,rt,str(root/'docs/asset/evidence'),'024-farmer-idle.png');move.stop_movement_immediately();phase=2;next_time=now+.5
  elif phase==2:
   assert r['samples'][1]['location'][1]>r['samples'][0]['location'][1]+30
   assert sample['body_anim'] and sample['arms_anim'];r['status']='passed_local_pawn_meshes_existing_anim_and_move';save();unreal.unregister_slate_post_tick_callback(handle);return
  save()
 except Exception as e:r['status']='failed';r['error']=str(e);move.stop_movement_immediately();save();unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick);print('Registered actual pawn body/arms movement/capture QA.')
