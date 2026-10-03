"""M_CL9_NightSky — 밤 하늘 구 재질 (2026-09-18, "스카이박스가 밝다"). 에디터 파이썬으로 만든다(멱등: 있으면 다시 짠다).
    Unlit. 발광 = 지평선 남색 → 천정 검정 그라데이션(정점 법선 Z) + 별(고주파 노이즈를 잘라 낸 점들).
    레벨의 하늘 구(엔진 SM_SkySphere, M_SimpleSkyDome)에 사이트 표 페이즈 `SkyMaterial` 로 씌운다 — ACL9NightLighting.
    값: Horizon·Zenith·StarDensity·StarBrightness 스칼라/벡터 파라미터라 MI 로 사이트마다 바꿀 수 있다.
"""
import unreal
FOLDER='/Game/Maps/Sky';NAME='M_CL9_NightSky';PATH=FOLDER+'/'+NAME
ML=unreal.MaterialEditingLibrary;at=unreal.AssetToolsHelpers.get_asset_tools()
if not unreal.EditorAssetLibrary.does_directory_exist(FOLDER):unreal.EditorAssetLibrary.make_directory(FOLDER)
mat=unreal.load_asset(PATH)
if mat is None:mat=at.create_asset(NAME,FOLDER,unreal.Material,unreal.MaterialFactoryNew())
ML.delete_all_material_expressions(mat)
mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property('two_sided',True)
def node(cls,x,y,**props):
    e=ML.create_material_expression(mat,cls,x,y)
    for k,v in props.items():e.set_editor_property(k,v)
    return e
# 방향: 카메라에서 픽셀로 가는 벡터의 Z — 하늘 구는 카메라를 감싸므로 위를 볼수록 1
cam=node(unreal.MaterialExpressionCameraVectorWS,-1300,0)
z=node(unreal.MaterialExpressionComponentMask,-1150,0,r=False,g=False,b=True,a=False);assert ML.connect_material_expressions(cam,'',z,'')
absz=node(unreal.MaterialExpressionAbs,-1000,0);assert ML.connect_material_expressions(z,'',absz,'')
# 지평선 근처를 조금 더 오래 남기려고 제곱근 대신 0.6 제곱
gpow=node(unreal.MaterialExpressionPower,-850,0,const_exponent=0.6);assert ML.connect_material_expressions(absz,'',gpow,'Base')
horizon=node(unreal.MaterialExpressionVectorParameter,-850,150,parameter_name='Horizon',default_value=unreal.LinearColor(0.006,0.009,0.026,1.0))
zenith=node(unreal.MaterialExpressionVectorParameter,-850,300,parameter_name='Zenith',default_value=unreal.LinearColor(0.001,0.0015,0.005,1.0))
grad=node(unreal.MaterialExpressionLinearInterpolate,-600,120);assert ML.connect_material_expressions(horizon,'',grad,'A') and ML.connect_material_expressions(zenith,'',grad,'B') and ML.connect_material_expressions(gpow,'',grad,'Alpha')
# 별 — 방향 벡터를 크게 키워 노이즈에 넣고 상위 몇 % 만 남긴다
scale=node(unreal.MaterialExpressionMultiply,-1150,400,const_b=900.0);assert ML.connect_material_expressions(cam,'',scale,'A')
noise=node(unreal.MaterialExpressionNoise,-1000,400,scale=1.0,quality=1,levels=1,output_min=0.0,output_max=1.0,noise_function=unreal.NoiseFunction.NOISEFUNCTION_VALUE_ALU)
assert ML.connect_material_expressions(scale,'',noise,'')
density=node(unreal.MaterialExpressionScalarParameter,-1000,560,parameter_name='StarDensity',default_value=0.93)
cut=node(unreal.MaterialExpressionSubtract,-850,440);assert ML.connect_material_expressions(noise,'',cut,'A') and ML.connect_material_expressions(density,'',cut,'B')
sat=node(unreal.MaterialExpressionSaturate,-700,440);assert ML.connect_material_expressions(cut,'',sat,'')
sharp=node(unreal.MaterialExpressionMultiply,-600,440,const_b=14.0);assert ML.connect_material_expressions(sat,'',sharp,'A')
sat2=node(unreal.MaterialExpressionSaturate,-500,440);assert ML.connect_material_expressions(sharp,'',sat2,'')
# 지평선 가까이는 별을 지운다(안개·구조물이 있는 곳) — 방향 Z 로 가중
starmask=node(unreal.MaterialExpressionMultiply,-400,440);assert ML.connect_material_expressions(sat2,'',starmask,'A') and ML.connect_material_expressions(absz,'',starmask,'B')
bright=node(unreal.MaterialExpressionScalarParameter,-400,560,parameter_name='StarBrightness',default_value=0.9)
stars=node(unreal.MaterialExpressionMultiply,-250,440);assert ML.connect_material_expressions(starmask,'',stars,'A') and ML.connect_material_expressions(bright,'',stars,'B')
out=node(unreal.MaterialExpressionAdd,-100,200);assert ML.connect_material_expressions(grad,'',out,'A') and ML.connect_material_expressions(stars,'',out,'B')
assert ML.connect_material_property(out,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
ML.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
print('NIGHT SKY MATERIAL '+mat.get_path_name())
