import unreal,json
from pathlib import Path
folder='/Game/Monsters/ChickenA'
mesh=unreal.load_asset(folder+'/SK_Chicken_A');a=unreal.load_asset(folder+'/AN_Chicken_Walk')
print('AnimPose methods',[(n,getattr(unreal.AnimPoseExtensions,n).__doc__) for n in dir(unreal.AnimPoseExtensions) if n in ('get_anim_pose_at_time','get_bone_pose','get_anim_pose_at_frame')])
print('AnimPoseEvaluationOptions',unreal.AnimPoseEvaluationOptions.__doc__)
print('Mesh color methods',[n for n in dir(mesh) if any(k in n for k in ('color','description','bound'))])
print('Mesh vertex colors',mesh.has_vertex_colors())
print('Slots',[(str(s.material_slot_name),s.material_interface.get_path_name() if s.material_interface else None) for s in mesh.get_editor_property('materials')])
print('Animation length',a.get_editor_property('sequence_length'))
