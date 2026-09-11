import unreal,json
from pathlib import Path
w=unreal.EditorLevelLibrary.get_pie_worlds(False)[0];pawn=unreal.GameplayStatics.get_player_pawn(w,0)
body=pawn.mesh;arms=next(c for c in pawn.get_components_by_class(unreal.SkeletalMeshComponent) if c.get_name()=='FirstPersonMesh')
for c,name in [(body,'SK_Farmer_A_Body'),(arms,'SK_Farmer_A_Arms')]:c.set_skinned_asset_and_update(unreal.load_asset('/Game/CL9Art/Characters/FarmerA/'+name));c.set_editor_property('override_materials',[])
pawn.get_component_by_class(unreal.CharacterMovementComponent).stop_movement_immediately();pawn.set_actor_location(unreal.Vector(20000,0,1092),False,True)
cam=next(a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.SceneCapture2D) if a.get_actor_label()=='QA024_Capture');capture=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
rt=unreal.RenderingLibrary.create_render_target2d(w,768,1024,unreal.TextureRenderTargetFormat.RTF_RGBA8);capture.texture_target=rt;capture.capture_source=unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR
capture.set_editor_property('capture_every_frame',True)
root=Path(unreal.Paths.project_dir());tag=globals().get('CAPTURE_TAG','lit');out=root/'docs/asset/evidence'/('024-farmer-'+tag+'-capture.json');assert not out.exists();start=unreal.GameplayStatics.get_time_seconds(w)
def tick(delta):
 try:
  now=unreal.GameplayStatics.get_time_seconds(w)
  if now-start<2:return
  unreal.RenderingLibrary.export_render_target(w,rt,str(root/'docs/asset/evidence'),'024-farmer-'+tag+'.png');capture.set_editor_property('capture_every_frame',False)
  out.write_text(json.dumps({'status':'captured_not_yet_visually_reviewed','elapsed':now-start,'world':w.get_path_name(),'body_mesh':body.skeletal_mesh_asset.get_path_name(),'arms_mesh':arms.skeletal_mesh_asset.get_path_name()},indent=2),encoding='utf-8');unreal.unregister_slate_post_tick_callback(handle)
 except Exception as e:
  out.write_text(json.dumps({'status':'failed','error':str(e)}),encoding='utf-8');unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick);print('Capturing authored pawn under two QA lights.')
