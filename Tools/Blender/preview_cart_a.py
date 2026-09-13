"""수레 미리보기 3장(중립·야간·손전등) — cart_a.blend 를 열고 돌린다.

    py -3 Tools/Blender/run_headless.py preview_cart_a.py --blend ArtSource/item/cart_a/cart_a.blend
"""
ASSET_ROOT='D:/Work/unreal/CL9/ArtSource/item/cart_a'
KIT_SCENE='CL9_CART_A'
SOURCE_BLEND='cart_a.blend'
OBJECT_NAMES=('SM_Cart_A',)
ORTHO=3.0
FOCUS=(0,0,.52)
CAMERA_POSITION=(2.7,-3.2,2.7)
exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/preview_static_pair.py',encoding='utf-8').read(),'preview_static_pair.py','exec'))
