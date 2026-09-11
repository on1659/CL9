ASSET_ROOT='D:/Work/unreal/CL9/ArtSource/map/henhouse_yard/sign_review'
KIT_SCENE='CL9_WAYFINDING_EXIT';SOURCE_BLEND='sign_review.blend'
from pathlib import Path
(Path(ASSET_ROOT)/'preview').mkdir(parents=True,exist_ok=True)
OBJECT_NAMES=['SM_Henhouse_Sign_'+s+'_A' for s in ['Exit','Feed','Kitchen','Coop']]
PLACEMENTS={name:((i-1.5)*.73,0,.202) for i,name in enumerate(OBJECT_NAMES)}
ORTHO=3.55;FOCUS=(0,0,.20);CAMERA_POSITION=(1,-5,1.6);FLASHLIGHT_ENERGY=200
exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/preview_static_pair.py',encoding='utf-8').read(),'preview_static_pair.py','exec'))
