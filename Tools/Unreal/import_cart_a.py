"""SM_Cart_A 임포트 — 물리 몸체용(UCX 1:1·소켓 Grip/Cargo·심플 콜리전). 에디터 파이썬에서 리드가 돌린다.

    자동 콜리전 생성 끔(auto_generate_collision=False), UCX 하나당 컨벡스 하나(one_convex_hull_per_ucx=True).
    manifest 의 ucx_count 와 임포트된 컨벡스 수, 소켓 Grip·Cargo 의 위치(UE 는 Y 부호가 반대)를 검사하고
    Collision Complexity 가 Simple(=UseDefault 또는 UseSimpleAndComplex, UseSimpleAsComplex/ComplexAsSimple 아님)인지 확인한다.
    소켓 회전이 0 이 아니면 0 으로 되돌린다(gameplay 는 소켓 위치만 쓴다 — 짐칸 박스 축은 메시 축).
    재임포트 조건: 기존 에셋의 CL9SourceSHA256 태그가 011 증거·이전 027 증거의 sha 이거나 비어 있을 때. 그 밖은 FORCE_REIMPORT=True.
    증거: docs/asset/evidence/027-cart-physics-imports.json
"""
import unreal,json,hashlib,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();source=root/'ArtSource/item/cart_a/export';folder='/Game/Items/CartA'
assert unreal.SystemLibrary.get_engine_version().startswith('5.8.')
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
m=json.loads((source/'manifest.json').read_text(encoding='utf-8'));meta=m['assets']['cart'];name=meta['object'];path=folder+'/'+name
fbx=source/(name+'.fbx');digest=hashlib.sha256(fbx.read_bytes()).hexdigest();assert digest==meta['fbx_sha256'],'manifest sha256 != FBX'
evidence=root/'docs/asset/evidence/027-cart-physics-imports.json'
known=set()
for old in ('011-cart-imports.json','027-cart-physics-imports.json'):
    p=evidence.with_name(old)
    if p.exists():known.update(a['source_sha256'] for a in json.loads(p.read_text(encoding='utf-8'))['assets'])
at=unreal.AssetToolsHelpers.get_asset_tools();ms=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
# 재질 — 이미 있으면 그대로 쓴다(버텍스 색 재질, 011 에서 만든 것).
materials={}
for matname,spec in m['materials'].items():
    mpath=folder+'/'+matname
    mat=unreal.load_asset(mpath) if unreal.EditorAssetLibrary.does_asset_exist(mpath) else None
    if mat is None:
        mat=at.create_asset(matname,folder,unreal.Material,unreal.MaterialFactoryNew())
        color=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionVertexColor)
        assert unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
        for key,prop in [('roughness',unreal.MaterialProperty.MP_ROUGHNESS),('metallic',unreal.MaterialProperty.MP_METALLIC)]:
            value=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant)
            value.set_editor_property('r',spec[key]);assert unreal.MaterialEditingLibrary.connect_material_property(value,'',prop)
        unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
    materials[matname]=mat
exists=unreal.EditorAssetLibrary.does_asset_exist(path);reimport=False;oldhash=None
if exists:
    mesh=unreal.load_asset(path);oldhash=str(unreal.EditorAssetLibrary.get_metadata_tag(mesh,'CL9SourceSHA256') or '')
    reimport=oldhash!=digest
    if reimport:
        assert oldhash=='' or oldhash in known or globals().get('FORCE_REIMPORT'),'모르는 SM_Cart_A(태그 %s) — FORCE_REIMPORT=True 로 덮어쓴다'%oldhash
        ms.remove_collisions(mesh)   # 옛 심플 콜리전을 비우고 UCX 만 남긴다(임포터는 기존 AggGeom 을 지우지 않는다)
if not exists or reimport:
    t=unreal.AssetImportTask();t.filename=str(fbx);t.destination_path=folder;t.destination_name=name
    t.automated=True;t.save=True;t.replace_existing=reimport;t.replace_existing_settings=reimport
    o=unreal.FbxImportUI();o.import_mesh=True;o.import_as_skeletal=False;o.import_animations=False
    o.import_materials=False;o.import_textures=False;o.automated_import_should_detect_type=False
    o.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    d=o.static_mesh_import_data;d.combine_meshes=True;d.auto_generate_collision=False;d.one_convex_hull_per_ucx=True
    d.convert_scene=True;d.convert_scene_unit=True;d.force_front_x_axis=True;d.import_uniform_scale=1
    d.vertex_color_import_option=unreal.VertexColorImportOption.REPLACE
    t.options=o;at.import_asset_tasks([t]);mesh=unreal.load_asset(path);assert isinstance(mesh,unreal.StaticMesh)
    unreal.EditorAssetLibrary.set_metadata_tag(mesh,'CL9SourceSHA256',digest)
slots=mesh.get_editor_property('static_materials');assert len(slots)==len(meta['materials'])
for i,slot in enumerate(slots):
    slotname=str(slot.get_editor_property('imported_material_slot_name'));assert slotname in materials;mesh.set_material(i,materials[slotname])
if mesh.get_num_lods()==1 or reimport:
    opts=unreal.StaticMeshReductionOptions();settings=[]
    for pct in (1,.5,.25):
        st=unreal.StaticMeshReductionSettings();st.percent_triangles=pct;settings.append(st)
    opts.reduction_settings=settings;opts.auto_compute_lod_screen_size=True;ms.set_lods(mesh,opts)
# 치수·원점
box=mesh.get_bounding_box();dims=list((box.max-box.min).to_tuple());center=(box.max+box.min)*.5
error=max(abs(a-b) for a,b in zip(dims,meta['dimensions_cm']));assert error<.01 and max(abs(c) for c in center.to_tuple())<.01,(dims,center)
# UCX 1:1
hulls=ms.get_convex_collision_count(mesh);simple=ms.get_simple_collision_count(mesh)
assert hulls==meta['ucx_count'],'컨벡스 %d / 심플(박스·구·캡슐) %d, 기대 컨벡스 %d'%(hulls,simple,meta['ucx_count'])
# Collision Complexity — 심플 콜리전을 물리에 쓴다. UseSimpleAsComplex / ComplexAsSimple 이면 UseDefault 로.
body=mesh.get_editor_property('body_setup');flag=body.get_editor_property('collision_trace_flag');flag_before=str(flag)
if flag not in (unreal.CollisionTraceFlag.CTF_USE_DEFAULT,unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AND_COMPLEX):
    body.set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_DEFAULT)
flag_after=str(body.get_editor_property('collision_trace_flag'))
# 소켓 Grip·Cargo — 위치는 manifest(Blender 좌표) 의 Y 부호를 뒤집어 비교. 회전은 0 으로 맞춘다.
sockets={}
for sname,expected in meta['sockets'].items():
    sk=mesh.find_socket(sname);assert sk is not None,'소켓 %s 가 없다 — FBX 의 SOCKET_%s 빈 오브젝트를 확인'%(sname,sname)
    loc=sk.get_editor_property('relative_location');exp=[expected[0],-expected[1],expected[2]]
    got=[loc.x,loc.y,loc.z];err=max(abs(a-b) for a,b in zip(got,exp));assert err<.01,(sname,got,exp)
    rot=sk.get_editor_property('relative_rotation');rot_before=[rot.roll,rot.pitch,rot.yaw]
    if max(abs(v) for v in rot_before)>1e-3:sk.set_editor_property('relative_rotation',unreal.Rotator(0,0,0))
    sockets[sname]={'location_cm':got,'expected_cm':exp,'error_cm':err,'rotation_before':rot_before,'scale':list(sk.get_editor_property('relative_scale').to_tuple())}
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
report={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'assets':[{
    'id':'cart','path':mesh.get_path_name(),'reimported':reimport,'previous_sha256':oldhash,'source_sha256':digest,
    'dimensions_cm':dims,'max_dimension_error_cm':error,'bounds_center_cm':list(center.to_tuple()),'lod_count':mesh.get_num_lods(),
    'convex_collision_hulls':hulls,'simple_collision_count':simple,'expected_ucx':meta['ucx_count'],
    'collision_trace_flag':{'before':flag_before,'after':flag_after},'sockets':sockets,
    'deck_height_cm':meta['deck_height_cm'],'cargo_extent_cm':meta['cargo_extent_cm'],'cargo_center_cm':meta['cargo_center_cm'],'grip_cm':meta['grip_cm'],
    'materials':[mesh.get_material(i).get_path_name() for i in range(len(slots))],
    'actual_gameplay_qa':'not tested in import step'}]}
evidence.write_text(json.dumps(report,indent=2),encoding='utf-8')
print('IMPORT OK '+json.dumps(report))
