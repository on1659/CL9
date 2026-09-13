"""SM_Cart_A.fbx 왕복 검증 — 빈 Blender 에 FBX 를 다시 읽어 치수·UCX·소켓을 manifest 와 맞춰 보고 증거 JSON 을 쓴다.

    py -3 Tools/Blender/run_headless.py verify_cart_a_fbx.py

검사: 렌더 메시 1개 + UCX_SM_Cart_A_NN 9개(각각 컨벡스·닫힘) + SOCKET_Grip/SOCKET_Cargo(메시의 자식, Y=0).
짐칸 안쪽(±60×±40, 바닥 위 40cm)에 UCX 정점이 하나도 없어야 한다(짐칸을 막지 않는다).
결과: docs/asset/evidence/027-cart-physics-rebuild.json
"""
import bpy,bmesh,json,hashlib,datetime,sys
from pathlib import Path
from mathutils import Vector
sys.stdout.reconfigure(encoding='utf-8',errors='replace')
ROOT=Path('D:/Work/unreal/CL9');ASSET=ROOT/'ArtSource/item/cart_a'
manifest=json.loads((ASSET/'export/manifest.json').read_text(encoding='utf-8'));meta=manifest['assets']['cart']
fbx=ASSET/'export/SM_Cart_A.fbx';digest=hashlib.sha256(fbx.read_bytes()).hexdigest()
assert digest==meta['fbx_sha256'],'manifest 의 sha256 과 FBX 가 다르다 — build_cart_a.py 를 다시 돌린다'
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=str(fbx),use_custom_normals=False)
objs={o.name:o for o in bpy.context.scene.objects}
mesh=objs['SM_Cart_A'];hulls=[objs[n] for n in sorted(objs) if n.startswith('UCX_SM_Cart_A_')]
sockets={n:objs[n] for n in objs if n.startswith('SOCKET_')}
assert len(hulls)==meta['ucx_count']==9,[h.name for h in hulls]
assert set(sockets)=={'SOCKET_Grip','SOCKET_Cargo'},list(sockets)
def bounds(o):
    pts=[o.matrix_world@Vector(c) for c in o.bound_box]
    return [min(p[i] for p in pts) for i in range(3)],[max(p[i] for p in pts) for i in range(3)]
lo,hi=bounds(mesh);dims=[(b-a)*100 for a,b in zip(lo,hi)];center=[(a+b)*50 for a,b in zip(lo,hi)]
dim_error=max(abs(a-b) for a,b in zip(dims,meta['dimensions_cm']));center_error=max(abs(c) for c in center)
assert dim_error<.01 and center_error<.01,(dims,center)
ex=meta['cargo_extent_cm'];deck=meta['deck_height_cm'];cargo=Vector(meta['cargo_center_cm'])/100
report_hulls=[];inside=0
for h in hulls:
    bm=bmesh.new();bm.from_mesh(h.data)
    closed=all(e.is_manifold for e in bm.edges)
    # 컨벡스 검사: 모든 정점이 모든 면의 안쪽(또는 면 위)에 있다.
    convex=True
    for f in bm.faces:
        for v in bm.verts:
            if f.normal.dot(v.co-f.verts[0].co)>1e-5:convex=False;break
        if not convex:break
    for v in bm.verts:
        p=h.matrix_world@v.co
        if abs(p.x-cargo.x)<ex[0]/100-1e-4 and abs(p.y-cargo.y)<ex[1]/100-1e-4 and deck/100+1e-4<p.z<deck/100+2*ex[2]/100-1e-4:inside+=1
    hlo,hhi=bounds(h)
    report_hulls.append({'name':h.name,'vertices':len(bm.verts),'faces':len(bm.faces),'closed':closed,'convex':convex,
                         'min_cm':[round(v*100,3) for v in hlo],'max_cm':[round(v*100,3) for v in hhi]})
    bm.free();assert closed and convex,h.name
assert inside==0,'UCX 정점 %d개가 짐칸 안쪽에 있다'%inside
report_sockets={}
for n,e in sockets.items():
    assert e.parent==mesh,n+' 은 SM_Cart_A 의 자식이어야 한다'
    loc=[round(v*100,3) for v in (e.matrix_world.translation)]
    expected=meta['sockets'][n[len('SOCKET_'):]]
    assert max(abs(a-b) for a,b in zip(loc,expected))<.01,(n,loc,expected)
    report_sockets[n[len('SOCKET_'):]]={'location_cm_blender':loc,'parent':e.parent.name,'rotation_deg':[round(v*57.29578,3) for v in e.matrix_world.to_euler()]}
# 바닥판 UCX 윗면 = 짐칸 바닥, 측벽 안쪽 면 = ±반폭 확인
deck_top=report_hulls[0]['max_cm'][2];assert abs(deck_top-deck)<.01,(deck_top,deck)
side_inner=min(abs(report_hulls[1]['max_cm'][1]),abs(report_hulls[2]['min_cm'][1]));assert abs(side_inner-ex[1])<.01,side_inner
end_inner=min(abs(report_hulls[3]['max_cm'][0]-cargo.x*100),abs(report_hulls[4]['min_cm'][0]-cargo.x*100));assert abs(end_inner-ex[0])<.01,end_inner
report={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'blender':bpy.app.version_string,'fbx':str(fbx.relative_to(ROOT)).replace('\\','/'),
        'fbx_sha256':digest,'blend_sha256':hashlib.sha256((ASSET/'cart_a.blend').read_bytes()).hexdigest(),
        'render_mesh':{'object':mesh.name,'dimensions_cm':dims,'bounds_center_cm':center,'max_dimension_error_cm':dim_error,
                       'triangles':meta['triangles'],'vertices':meta['vertices'],'materials':[m.name for m in mesh.data.materials],'uv_layers':len(mesh.data.uv_layers)},
        'cargo':{'deck_height_cm':deck,'cargo_extent_cm':ex,'cargo_center_cm':meta['cargo_center_cm'],'clear_area_cm':meta['cargo_clear_area_cm'],
                 'wall_height_cm':meta['wall_height_cm'],'ucx_vertices_inside_cargo':inside},
        'handle':{'grip_cm':meta['grip_cm'],'handle_height_cm':meta['handle_height_cm']},
        'wheels':{'radius_cm':meta['wheel_radius_cm'],'centers_cm':meta['wheel_centers_cm'],'ground_z_cm':meta['ground_z_cm']},
        'ucx':report_hulls,'sockets':report_sockets,
        'axis_note':'값은 Blender 좌표(+X 손잡이/+Z 위). UE 임포트(force_front_x_axis)는 Y 부호를 뒤집는다 — 소켓 둘은 Y=0.',
        'ue_import':'not run here — Tools/Unreal/import_cart_a.py 를 리드가 에디터에서 돌린다'}
out=ROOT/'docs/asset/evidence/027-cart-physics-rebuild.json';out.write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding='utf-8')
print('VERIFY OK '+str(out));print(json.dumps({'dims':dims,'hulls':len(hulls),'sockets':report_sockets},ensure_ascii=False))
