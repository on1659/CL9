"""퀵슬롯 아이템 아이콘 임포트 — ArtSource/item/<폴더>/icons/T_Icon_<alias>.png → /Game/UI/Icons/T_Icon_<alias> (Texture2D).

각 폴더의 `icons/manifest.json`(Tools/Blender/render_item_icons.py 가 쓴다) 을 읽어 PNG 의 sha256 이 manifest `png_sha256` 과 같을 때만
임포트한다. 텍스처 설정: TextureGroup UI · NoMipmaps · sRGB · NeverStream · 압축 UserInterface2D(TC_EDITOR_ICON — 알파 가장자리를 뭉개지 않는다)
— 선례 Tools/Unreal/import_farmer_palette.py. 같은 이름의 텍스처가 이미 있고 메타 `CL9SourceSHA256` 이 다르면 재임포트(replace),
같으면 설정만 다시 확인한다. 표 행(`ItemTable.<alias>.Icon`)은 여기서 바꾸지 않는다 — 리드가 보고서의 표 행 요청대로 적용한다.
증거: docs/asset/evidence/030-item-icons.json 의 `ue_import` 절(`render` 절은 렌더 스크립트가 썼다).

실행(리드): `py -3 Tools/Unreal/run_python.py Tools/Unreal/import_item_icons.py`
"""
import unreal,json,hashlib,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();item_root=root/'ArtSource/item';folder='/Game/UI/Icons'
evidence=root/'docs/asset/evidence/030-item-icons.json'
assert unreal.SystemLibrary.get_engine_version().startswith('5.8.')
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
at=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.EditorAssetLibrary
report={'engine':unreal.SystemLibrary.get_engine_version(),'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'folder':folder,'icons':[]}
manifests=sorted(item_root.glob('*/icons/manifest.json'))
assert manifests,'icons/manifest.json 이 없다 — render_item_icons.py 를 먼저 돌린다'
for mpath in manifests:
    m=json.loads(mpath.read_text(encoding='utf-8'))
    for alias,meta in m['icons'].items():
        png=mpath.parent/meta['file'];digest=hashlib.sha256(png.read_bytes()).hexdigest()
        assert digest==meta['png_sha256'],(str(png),'manifest 의 sha256 과 PNG 가 다르다 — render_item_icons.py 를 다시 돌린다')
        name='T_Icon_'+alias;path=folder+'/'+name;assert meta['file']==name+'.png',(meta['file'],name)
        exists=lib.does_asset_exist(path);reimport=False
        if exists:
            texture=unreal.load_asset(path);oldhash=lib.get_metadata_tag(texture,'CL9SourceSHA256')
            assert oldhash,path+' 은 이 스크립트가 만든 텍스처가 아니다(CL9SourceSHA256 없음)'
            reimport=oldhash!=digest
        if not exists or reimport:
            t=unreal.AssetImportTask();t.filename=str(png);t.destination_path=folder;t.destination_name=name
            t.automated=True;t.save=False;t.replace_existing=reimport;t.replace_existing_settings=reimport
            at.import_asset_tasks([t]);texture=unreal.load_asset(path);assert isinstance(texture,unreal.Texture2D),path
            lib.set_metadata_tag(texture,'CL9SourceSHA256',digest);lib.set_metadata_tag(texture,'CL9Owner','ItemIcons')
        texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
        texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        texture.set_editor_property('srgb',True);texture.set_editor_property('never_stream',True)
        lib.save_loaded_asset(texture)
        w=texture.blueprint_get_size_x();h=texture.blueprint_get_size_y();assert [w,h]==meta['size'],(w,h,meta['size'])
        report['icons'].append({'alias':alias,'path':texture.get_path_name(),'source':str(png.relative_to(root)).replace('\\','/'),'source_sha256':digest,
            'size':[w,h],'reimported':reimport,'existed':exists,
            'lod_group':str(texture.get_editor_property('lod_group')),'mip_gen_settings':str(texture.get_editor_property('mip_gen_settings')),
            'compression_settings':str(texture.get_editor_property('compression_settings')),'srgb':texture.get_editor_property('srgb'),
            'never_stream':texture.get_editor_property('never_stream'),'fill_ratio':meta['fill_ratio'],'opaque_pixels':meta['opaque_pixels']})
report['table_rows']='ItemTable.<alias>[.<sub>].Icon 은 리드가 보고서 표 행대로 적용 — 이 스크립트는 텍스처만 만든다'
full=json.loads(evidence.read_text(encoding='utf-8')) if evidence.exists() else {}
full['ue_import']=report  # render 절은 render_item_icons.py 가 썼다 — 그대로 둔다
evidence.write_text(json.dumps(full,indent=2,ensure_ascii=False),encoding='utf-8')
print('ICONS OK %d -> %s'%(len(report['icons']),folder));print(json.dumps([(i['alias'],i['path'],i['reimported']) for i in report['icons']],ensure_ascii=False))
