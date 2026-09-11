import unreal
mesh=unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple')
sk=mesh.skeleton
for obj in [mesh,sk,unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem),unreal.AnimPoseExtensions]:
 print(str(type(obj)),[n for n in dir(obj) if any(k in n for k in ['bone','ref_pose','reference','skeleton','pose'])])
for name in ['SkeletonModifier','SkeletalMeshEditorSubsystem','AnimationLibrary','AnimationBlueprintLibrary']:
 if hasattr(unreal,name):
  cls=getattr(unreal,name);print(name,[n for n in dir(cls) if any(k in n for k in ['bone','ref_pose','reference','skeleton','pose'])])
