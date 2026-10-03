SOURCE_REL='ArtSource/map/campfire/export'
DESTINATION='/Game/Maps/Henhouse/Props'
EVIDENCE='033-campfire-imports.json'
ALLOW_OWNED_REIMPORT=True
exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_portable_lights.py',encoding='utf-8').read(),'import_portable_lights.py','exec'))

# 불꽃 재질 — 공용 임포트가 만든 "상수색 × 세기" 를 버리고 다시 짠다 (2026-09-18):
#   Unlit, 발광 = 정점색(밑동 노랑 → 끝 검붉음) × FlameStrength, 월드 포지션 오프셋 = 정점 알파(높이 비율) 로 가중한 흔들림.
#   정점 색·알파는 Tools/Blender/build_campfire.py 의 tongue() 이 굽는다. 램프 깜박임(소품 빛 컬럼)과 합쳐 불이 산다.
ML=unreal.MaterialEditingLibrary
flame=unreal.load_asset(DESTINATION+'/M_CL9_Campfire_Flame')
ML.delete_all_material_expressions(flame)
flame.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
flame.set_editor_property('two_sided',True)
def node(cls,x,y,**props):
    e=ML.create_material_expression(flame,cls,x,y)
    for k,v in props.items():e.set_editor_property(k,v)
    return e
vc=node(unreal.MaterialExpressionVertexColor,-700,0)
strength=node(unreal.MaterialExpressionScalarParameter,-700,200,parameter_name='FlameStrength',default_value=2.0)
emis=node(unreal.MaterialExpressionMultiply,-450,80)
assert ML.connect_material_expressions(vc,'',emis,'A') and ML.connect_material_expressions(strength,'',emis,'B')
assert ML.connect_material_property(emis,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
# 흔들림: sin(time*6 + z*0.08) 을 X 로, sin(time*4.3 + x*0.05 + 1.7) 을 Y 로, 끝(알파 1)에서 4cm
t=node(unreal.MaterialExpressionTime,-1100,400)
wp=node(unreal.MaterialExpressionWorldPosition,-1100,520)
mz=node(unreal.MaterialExpressionComponentMask,-950,520,r=False,g=False,b=True,a=False)
mx=node(unreal.MaterialExpressionComponentMask,-950,620,r=True,g=False,b=False,a=False)
assert ML.connect_material_expressions(wp,'',mz,'') and ML.connect_material_expressions(wp,'',mx,'')
def sway(speed,posmask,posscale,phase,y):
    ts=node(unreal.MaterialExpressionMultiply,-800,y,const_b=speed);assert ML.connect_material_expressions(t,'',ts,'A')
    ps=node(unreal.MaterialExpressionMultiply,-800,y+80,const_b=posscale);assert ML.connect_material_expressions(posmask,'',ps,'A')
    a1=node(unreal.MaterialExpressionAdd,-650,y+40);assert ML.connect_material_expressions(ts,'',a1,'A') and ML.connect_material_expressions(ps,'',a1,'B')
    a2=node(unreal.MaterialExpressionAdd,-550,y+40,const_b=phase);assert ML.connect_material_expressions(a1,'',a2,'A')
    s=node(unreal.MaterialExpressionSine,-450,y+40,period=6.2831853);assert ML.connect_material_expressions(a2,'',s,'')
    return s
sx=sway(6.0,mz,0.08,0.0,400);sy=sway(4.3,mx,0.05,1.7,600)
zero=node(unreal.MaterialExpressionConstant,-450,760,r=0.0)
vec=node(unreal.MaterialExpressionAppendVector,-300,450);assert ML.connect_material_expressions(sx,'',vec,'A') and ML.connect_material_expressions(sy,'',vec,'B')
vec3=node(unreal.MaterialExpressionAppendVector,-200,500);assert ML.connect_material_expressions(vec,'',vec3,'A') and ML.connect_material_expressions(zero,'',vec3,'B')
# 정점색 노드의 '' 출력은 RGB 라 알파 마스크가 컴파일되지 않는다 — 'A' 출력을 바로 쓴다
amp=node(unreal.MaterialExpressionMultiply,-300,300,const_b=4.0);assert ML.connect_material_expressions(vc,'A',amp,'A')
wpo=node(unreal.MaterialExpressionMultiply,-100,480);assert ML.connect_material_expressions(vec3,'',wpo,'A') and ML.connect_material_expressions(amp,'',wpo,'B')
assert ML.connect_material_property(wpo,'',unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
ML.recompile_material(flame);unreal.EditorAssetLibrary.save_loaded_asset(flame)
# 옛 MI(세기 3 오버라이드)는 필요 없다 — 재질 기본값이 2 다
mesh=unreal.load_asset(DESTINATION+'/SM_Campfire_A')
for i,slot in enumerate(mesh.get_editor_property('static_materials')):
    if str(slot.get_editor_property('imported_material_slot_name'))=='M_CL9_Campfire_Flame':mesh.set_material(i,flame)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
mi=DESTINATION+'/MI_Campfire_Flame'
if unreal.EditorAssetLibrary.does_asset_exist(mi):unreal.EditorAssetLibrary.delete_asset(mi)
print('FLAME MATERIAL REBUILT')
