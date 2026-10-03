"""우리 철망 울타리 미리보기 3장(중립·야간·손전등) — pen_fence.blend 를 열고 돌린다.

    py -3 Tools/Blender/run_headless.py preview_pen_fence.py --blend ArtSource/map/pen_fence/pen_fence.blend
"""
ASSET_ROOT='D:/Work/unreal/CL9/ArtSource/map/pen_fence'
KIT_SCENE='CL9_PEN_FENCE'
SOURCE_BLEND='pen_fence.blend'
OBJECT_NAMES=('SM_Pen_Fence_250_A',)
ORTHO=3.3
FOCUS=(0,0,.62)
CAMERA_POSITION=(3,-4.4,2.0)
FLASHLIGHT_ENERGY=450
exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/preview_static_pair.py',encoding='utf-8').read(),'preview_static_pair.py','exec'))
