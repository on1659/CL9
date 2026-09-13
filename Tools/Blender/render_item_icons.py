"""아이템 아이콘 렌더 — 흰 실루엣 + 알파 PNG 128×128 (퀵슬롯 칸용).

    py -3 Tools/Blender/run_headless.py render_item_icons.py [-- --only egg,pot --preview-dir <폴더>]

`--preview-dir` 를 주면 같은 아이콘을 어두운 바탕(0.1 회색) 위에 합성한 확인용 PNG 도 그 폴더에 쓴다(흰 실루엣은 흰 배경에서 안 보인다).

각 아이템 폴더의 .blend 에서 대상 메시를 빈 씬에 append 하고(원본 .blend 는 열지도 저장하지도 않는다) Workbench 로 렌더한다.
  - 재질 무시: Workbench Flat 조명 + Single 색(1,1,1) → 색·흐림은 UI 가 하므로 텍스처는 항상 100% 흰색.
  - 필름 투명, 뷰 변환 Standard(흰색 1.0 이 255 로 그대로).
  - 카메라: 정면(+X)에서 -Y 쪽으로 45°, 위로 35° 올려다보는 3/4 고정 시점, 직교. 평가된 메시 정점을 카메라 평면에 투영해
    긴 변이 프레임의 88% 를 채우도록 ortho_scale 을 정하고 중심을 맞춘다(단위 무관).
  - 저장 뒤 PNG 를 다시 읽어 모든 픽셀의 RGB 를 1 로 맞춘다(가장자리 AA 픽셀과 투명 픽셀의 검정이 UI 필터링에서 어두운 테가
    되지 않게 — 알파만 실루엣을 담는다). 다시 읽어 채움 비율·흰색을 검사한다.
출력: ArtSource/item/<폴더>/icons/T_Icon_<alias>.png + 같은 폴더 icons/manifest.json(sha256·채움·카메라 — 임포트 스크립트가 sha 를 대조한다).
수레(cart)는 제외한다(brief).
"""
import bpy,json,hashlib,datetime,math,sys
from pathlib import Path
from mathutils import Vector
sys.stdout.reconfigure(encoding='utf-8',errors='replace')
ROOT=Path('D:/Work/unreal/CL9');ITEM_ROOT=ROOT/'ArtSource/item'
SIZE=128;FILL=.88;AZIMUTH_DEG=-45;ELEVATION_DEG=35
# (폴더, blend, 오브젝트, alias) — alias 는 ItemTable 행 Alias 와 같다. egg/meat 는 3상태 실루엣이 같아 raw 하나로 쓴다.
ITEMS=[('egg_states','egg_states.blend','SM_Egg_Raw_A','egg'),
       ('meat_states','meat_states.blend','SM_Meat_Raw_A','meat'),
       ('cookware','cookware.blend','SM_FryingPan_A','frying_pan'),
       ('cookware','cookware.blend','SM_Pot_A','pot'),
       ('portable_lights','portable_lights.blend','SM_Flashlight_A','flashlight'),
       ('portable_lights','portable_lights.blend','SM_Headlamp_A','headlamp'),
       ('standing_lamp_a','standing_lamp_a.blend','SM_StandingLamp_A','standing_lamp')]
argv=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
only=set(argv[argv.index('--only')+1].split(',')) if '--only' in argv else None
preview_dir=Path(argv[argv.index('--preview-dir')+1]) if '--preview-dir' in argv else None
bpy.ops.wm.read_factory_settings(use_empty=True)
scene=bpy.context.scene
scene.render.engine='BLENDER_WORKBENCH';scene.render.film_transparent=True
scene.render.resolution_x=SIZE;scene.render.resolution_y=SIZE;scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA';scene.render.image_settings.color_depth='8';scene.render.image_settings.compression=15
scene.view_settings.view_transform='Standard';scene.view_settings.look='None';scene.view_settings.exposure=0;scene.view_settings.gamma=1
sh=scene.display.shading;sh.light='FLAT';sh.color_type='SINGLE';sh.single_color=(1,1,1)
sh.show_cavity=False;sh.show_shadows=False;sh.show_object_outline=False;sh.show_specular_highlight=False;sh.show_xray=False;sh.show_backface_culling=False
scene.display.render_aa='8'
cam_data=bpy.data.cameras.new('IconCamera');cam_data.type='ORTHO';cam_data.clip_start=.001;cam_data.clip_end=1000
camera=bpy.data.objects.new('IconCamera',cam_data);scene.collection.objects.link(camera);scene.camera=camera
az=math.radians(AZIMUTH_DEG);el=math.radians(ELEVATION_DEG)
view_dir=Vector((math.cos(el)*math.cos(az),math.cos(el)*math.sin(az),math.sin(el)))  # 카메라가 놓이는 방향(초점 → 카메라)
def evaluated_world_vertices(o):
    dg=bpy.context.evaluated_depsgraph_get();ev=o.evaluated_get(dg);me=ev.to_mesh()
    pts=[ev.matrix_world@v.co for v in me.vertices];ev.to_mesh_clear();return pts
def fit_camera(o):
    pts=evaluated_world_vertices(o);assert pts,o.name
    lo=Vector([min(p[i] for p in pts) for i in range(3)]);hi=Vector([max(p[i] for p in pts) for i in range(3)])
    center=(lo+hi)/2;radius=(hi-lo).length/2
    camera.location=center+view_dir*radius*4
    camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
    bpy.context.view_layer.update()
    inv=camera.matrix_world.inverted();local=[inv@p for p in pts]
    xmin=min(p.x for p in local);xmax=max(p.x for p in local);ymin=min(p.y for p in local);ymax=max(p.y for p in local)
    extent=max(xmax-xmin,ymax-ymin);cam_data.ortho_scale=extent/FILL
    # 투영 중심을 프레임 중앙으로 — 카메라 로컬 x/y 축 방향으로 평행 이동(직교라 거리와 무관)
    m=camera.matrix_world;shift=m.to_3x3()@Vector(((xmin+xmax)/2,(ymin+ymax)/2,0));camera.location+=shift
    bpy.context.view_layer.update()
    return {'bounds_min':[round(v,5) for v in lo],'bounds_max':[round(v,5) for v in hi],'projected_extent':round(extent,5),'ortho_scale':round(cam_data.ortho_scale,5),
            'camera_location':[round(v,5) for v in camera.location],'vertices':len(pts)}
def measure(px):
    """알파>0 픽셀 수·상자와 RGB 최솟값(알파 0 픽셀 포함 — 전부 1 이어야 UI 의 이중선형 필터가 가장자리에 검은 테를 만들지 않는다).
    렌더 직후에는 가장자리 AA 픽셀 RGB 가 1 이 아니고(합성 해제 오차) 투명 픽셀은 검정이다 — 강제로 1 로 맞춘 뒤 다시 잰다."""
    n=SIZE*SIZE;opaque=0;xs=[];ys=[];min_rgb=1.0
    for i in range(n):
        min_rgb=min(min_rgb,px[i*4],px[i*4+1],px[i*4+2])
        if px[i*4+3]>0:opaque+=1;xs.append(i%SIZE);ys.append(i//SIZE)
    assert opaque>0,'아무것도 찍히지 않았다'
    w=max(xs)-min(xs)+1;h=max(ys)-min(ys)+1
    return {'opaque_pixels':opaque,'opaque_box_px':[min(xs),min(ys),max(xs),max(ys)],'fill_ratio':round(max(w,h)/SIZE,4),'min_rgb':round(min_rgb,4)}
def load_pixels(path):
    img=bpy.data.images.load(str(path),check_existing=False);assert img.size[0]==SIZE and img.size[1]==SIZE,img.size
    px=list(img.pixels);bpy.data.images.remove(img);return px
def force_white_and_measure(path):
    px=load_pixels(path);before=measure(px);n=SIZE*SIZE
    for i in range(n):px[i*4]=px[i*4+1]=px[i*4+2]=1.0
    img=bpy.data.images.new('icon',SIZE,SIZE,alpha=True);img.pixels=px;img.filepath_raw=str(path);img.file_format='PNG';img.save();bpy.data.images.remove(img)
    if preview_dir:
        preview_dir.mkdir(parents=True,exist_ok=True);dark=bpy.data.images.new('preview',SIZE,SIZE,alpha=False)
        dp=[0.0]*(n*4)
        for i in range(n):
            a=px[i*4+3];v=.1*(1-a)+a;dp[i*4]=dp[i*4+1]=dp[i*4+2]=v;dp[i*4+3]=1.0
        dark.pixels=dp;dark.filepath_raw=str(preview_dir/path.name);dark.file_format='PNG';dark.save();bpy.data.images.remove(dark)
    after=measure(load_pixels(path))  # 저장된 파일을 다시 읽어 확인 — 이 값이 납품 상태다
    assert after['min_rgb']>=.999 and after['opaque_pixels']==before['opaque_pixels'],(before,after)
    # 가장자리 AA 픽셀 1~2개 여유. 0.88×128=112.6px
    assert abs(after['fill_ratio']-FILL)<3/SIZE,after
    return {**after,'min_rgb_before_force':before['min_rgb']}
manifests={}
for folder,blend,obj_name,alias in ITEMS:
    if only and alias not in only:continue
    src=ITEM_ROOT/folder/blend;assert src.exists(),src
    with bpy.data.libraries.load(str(src),link=False) as (data_from,data_to):
        assert obj_name in data_from.objects,(obj_name,list(data_from.objects))
        data_to.objects=[obj_name]
    o=data_to.objects[0];assert o is not None and o.type=='MESH',obj_name
    assert o.parent is None,obj_name+' 은 부모가 있어 append 만으로 월드 변환을 잃는다'
    o.hide_render=False;o.hide_viewport=False;scene.collection.objects.link(o);bpy.context.view_layer.update()
    fit=fit_camera(o)
    out_dir=ITEM_ROOT/folder/'icons';out_dir.mkdir(exist_ok=True);out=out_dir/('T_Icon_'+alias+'.png')
    scene.render.filepath=str(out);bpy.ops.render.render(write_still=True,scene=scene.name)
    stats=force_white_and_measure(out)
    digest=hashlib.sha256(out.read_bytes()).hexdigest()
    entry={'file':out.name,'alias':alias,'source_blend':blend,'object':obj_name,'size':[SIZE,SIZE],'png_sha256':digest,
           'camera':{'azimuth_deg':AZIMUTH_DEG,'elevation_deg':ELEVATION_DEG,'projection':'ORTHO',**fit},'fill_target':FILL,**stats}
    manifests.setdefault(folder,{})[alias]=entry
    scene.collection.objects.unlink(o);bpy.data.objects.remove(o)
    print('ICON OK '+str(out.relative_to(ROOT)).replace('\\','/')+' fill=%.3f opaque=%d min_rgb_before_force=%.3f'%(stats['fill_ratio'],stats['opaque_pixels'],stats['min_rgb_before_force']))
for folder,entries in manifests.items():
    path=ITEM_ROOT/folder/'icons/manifest.json'
    old=json.loads(path.read_text(encoding='utf-8')) if path.exists() else {'icons':{}}
    old['icons'].update(entries)
    old.update({'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'blender':bpy.app.version_string,'renderer':'BLENDER_WORKBENCH flat/single white, film transparent, Standard view',
                'purpose':'퀵슬롯 아이콘 — 흰 실루엣+알파, 색·흐림은 UI 가 한다','ue_destination':'/Game/UI/Icons/T_Icon_<alias> (Tools/Unreal/import_item_icons.py)'})
    path.write_text(json.dumps(old,indent=2,ensure_ascii=False),encoding='utf-8')
# 증거 JSON — render 절은 여기서, ue_import 절은 Tools/Unreal/import_item_icons.py 가 채운다(있으면 render 절만 갱신)
evidence=ROOT/'docs/asset/evidence/030-item-icons.json'
report=json.loads(evidence.read_text(encoding='utf-8')) if evidence.exists() else {}
rendered={alias:{'folder':folder,**{k:v for k,v in e.items() if k!='camera'},'camera_azimuth_deg':AZIMUTH_DEG,'camera_elevation_deg':ELEVATION_DEG}
          for folder,entries in manifests.items() for alias,e in entries.items()}
report.setdefault('render',{'icons':{}})['icons'].update(rendered)
report['render'].update({'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'blender':bpy.app.version_string,'size':[SIZE,SIZE],'fill_target':FILL,
                         'spec':'Workbench Flat/Single 흰색·필름 투명·Standard 뷰·직교 3/4 위 시점, 모든 픽셀 RGB=1·알파만 실루엣','excluded':['cart']})
report.setdefault('ue_import','not run here — Tools/Unreal/import_item_icons.py 를 리드가 에디터에서 돌린다')
evidence.write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding='utf-8')
print('RENDER OK %d icons -> '%sum(len(v) for v in manifests.values())+str(evidence.relative_to(ROOT)).replace('\\','/'))
