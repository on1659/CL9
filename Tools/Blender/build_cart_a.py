"""물리 몸체용 농장 수레 — 짐칸 안치수 120×80cm·벽 40cm·손잡이 100cm·바퀴 4개, UCX 컨벡스 9개, 소켓 Grip/Cargo.

실행: py -3 Tools/Blender/run_headless.py build_cart_a.py   (= blender.exe -b --python Tools/Blender/build_cart_a.py)
결과: ArtSource/item/cart_a/cart_a.blend · export/SM_Cart_A.fbx · export/manifest.json
이어서 verify_cart_a_fbx.py(왕복 검증·증거 027) 와 preview_cart_a.py(렌더 3장) 를 돌린다.
좌표: 1 BU = 1m, 지면 Z=0 에서 만들고 마지막에 바운드 중심으로 옮긴다. +X = 손잡이, +Z = 위.
UE 임포트는 Y 를 뒤집는다(왼손 좌표계) — manifest 의 Y 값은 Blender 기준이고, 소켓 두 개는 Y=0 이라 영향이 없다.
"""
import bpy,bmesh,math,json,hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path('D:/Work/unreal/CL9/ArtSource/item/cart_a')
assert 'CL9_CART_A' not in bpy.data.scenes,'저장된 수레 원본이 열려 있다 — 새 파일에서 돌린다'
for f in ('export','preview'):(ROOT/f).mkdir(parents=True,exist_ok=True)
s=bpy.data.scenes.new('CL9_CART_A');bpy.context.window.scene=s;s.unit_settings.system='METRIC';s.unit_settings.scale_length=1
ex=bpy.data.collections.new('CART_EXPORT');s.collection.children.link(ex)
coll=bpy.data.collections.new('CART_COLLISION');s.collection.children.link(coll)
sock=bpy.data.collections.new('CART_SOCKET');s.collection.children.link(sock)
ref=bpy.data.collections.new('CART_REF');s.collection.children.link(ref)
materials={'M_CL9_Cart_Frame':{'vertex_color':True,'roughness':.77,'metallic':.18},
           'M_CL9_Cart_Rubber':{'vertex_color':True,'roughness':.90,'metallic':.01}}
mats={}
for name,meta in materials.items():
    m=bpy.data.materials.new(name);m.use_nodes=True;bs=next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    vc=m.node_tree.nodes.new('ShaderNodeVertexColor');vc.layer_name='Color';m.node_tree.links.new(vc.outputs['Color'],bs.inputs['Base Color'])
    bs.inputs['Roughness'].default_value=meta['roughness'];bs.inputs['Metallic'].default_value=meta['metallic'];mats[name]=m
yellow=(.39,.23,.038,1);dark=(.034,.044,.047,1);wood=(.24,.16,.077,1);rubber=(.016,.020,.022,1);metal=(.20,.23,.23,1)
parts=[]
def finishpart(o,color,slot='M_CL9_Cart_Frame'):
    for c in list(o.users_collection):c.objects.unlink(o)
    ex.objects.link(o);o.data.materials.append(mats[slot]);a=o.data.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='CORNER')
    for v in a.data:v.color=color
    parts.append(o);return o
def box(name,center,size,color,bevel=.006,slot='M_CL9_Cart_Frame'):
    bpy.ops.mesh.primitive_cube_add(size=1,location=center);o=bpy.context.object;o.name=name;o.scale=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        mod=o.modifiers.new('Edges','BEVEL');mod.width=bevel;mod.segments=1;bpy.ops.object.modifier_apply(modifier=mod.name)
    return finishpart(o,color,slot)
def bar(name,a,b,r,color,slot='M_CL9_Cart_Frame',vertices=12):
    d=Vector(b)-Vector(a);bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=d.length,location=(Vector(a)+Vector(b))/2)
    o=bpy.context.object;o.name=name;o.rotation_euler=d.to_track_quat('Z','Y').to_euler()
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=True);return finishpart(o,color,slot)
def wheel(name,x,y,z):
    profile=[(-.065,.12),(-.058,.165),(-.035,.18),(.035,.18),(.058,.165),(.065,.12)]
    n=24;vs=[(x+r*math.cos(i*math.tau/n),y+yy,z+r*math.sin(i*math.tau/n)) for yy,r in profile for i in range(n)]
    fs=[(j*n+i,j*n+(i+1)%n,(j+1)*n+(i+1)%n,(j+1)*n+i) for j in range(len(profile)-1) for i in range(n)]
    fs.extend([tuple(range(n-1,-1,-1)),tuple((len(profile)-1)*n+i for i in range(n))])
    data=bpy.data.meshes.new(name+'_Mesh');data.from_pydata(vs,[],fs);data.update();o=bpy.data.objects.new(name,data);ex.objects.link(o)
    finishpart(o,rubber,'M_CL9_Cart_Rubber')
    for side in (-1,1):
        bar('WheelHub',(x,y+side*.055,z),(x,y+side*.078,z),.095,yellow,vertices=16)
        bar('AxleCap',(x,y+side*.078,z),(x,y+side*.086,z),.035,metal,vertices=12)
    # 굵은 트레드 몇 개만 — 움직일 때 회전이 읽히면 된다.
    for i in range(8):
        a=i*math.tau/8
        p=(x+.180*math.cos(a),y,z+.180*math.sin(a))
        o=box('TireTread',p,(.050,.080,.014),rubber,.002,'M_CL9_Cart_Rubber')
        o.rotation_euler.y=math.pi/2-a
        bpy.context.view_layer.objects.active=o;bpy.ops.object.transform_apply(location=False,rotation=True,scale=False)

# ---- 치수(지면 기준, m). 짐칸 안치수 = CARGO_X × CARGO_Y, 짐칸 바닥 = DECK, 벽 위 = DECK+WALL, 손잡이 봉 = GRIP_Z
CARGO_X,CARGO_Y,WALL=1.20,.80,.40
DECK=.46;WALL_T=.03;GRIP_Z=1.00;GRIP_X=.98
WHEEL_R=.187;WHEEL_Z=.19;WHEEL_X=.42;WHEEL_Y=.40;WHEEL_HALF_W=.086
HX,HY=CARGO_X/2,CARGO_Y/2                 # 짐칸 안쪽 반폭 (0.60, 0.40)
OX,OY=HX+WALL_T,HY+WALL_T                 # 벽 바깥 면 (0.63, 0.43)

# 바퀴·차축·차대 — 바퀴는 짐칸 밑에 들어가고(윗면 0.377 < 가로대 0.385), 차축은 세로보 아래 브래킷에 매달린다.
centers=[]
for x in (-WHEEL_X,WHEEL_X):
    bar('Axle',(x,-WHEEL_Y-WHEEL_HALF_W,WHEEL_Z),(x,WHEEL_Y+WHEEL_HALF_W,WHEEL_Z),.036,dark)
    for y in (-WHEEL_Y,WHEEL_Y):wheel('CartWheel',x,y,WHEEL_Z);centers.append((x,y,WHEEL_Z))
    for y in (-.22,.22):box('AxleBracket',(x,y,.235),(.09,.07,.13),dark,.003)
for y in (-.22,.22):box('ChassisBeam',(0,y,.335),(2*OX,.060,.090),dark)
for x in (-.60,-.20,.20,.60):box('DeckCrossMember',(x,0,.4025),(.050,2*OY,.035),yellow)
# 짐칸 바닥 판재 7장 — 윗면이 DECK.
for i in range(7):
    col=tuple(v*(.91 if i%2 else 1.04) for v in wood[:3])+(1,)
    box('BedPlank',(0,(i-3)*(2*OY/7),DECK-.02),(2*OX,2*OY/7-.008,.040),col,.004)
# 바닥 테두리 강판 — 벽 아래를 두른다.
for y in (-OY-.005,OY+.005):box('BedSideEdge',(0,y,.43),(2*OX+.03,.04,.06),yellow)
for x in (-OX-.005,OX+.005):box('BedEndEdge',(x,0,.43),(.04,2*OY+.01,.06),yellow)
# 측판 — 널 3장(높이 .125, 틈 .0125)으로 DECK 에서 DECK+WALL 까지. 안쪽 면이 정확히 ±HY / ±HX.
board_h=(WALL-2*.0125)/3
for k in range(3):
    zc=DECK+board_h/2+k*(board_h+.0125)
    for y in (-HY-WALL_T/2,HY+WALL_T/2):box('SideBoard',(0,y,zc),(2*OX,WALL_T,board_h),wood,.003)
    for x in (-HX-WALL_T/2,HX+WALL_T/2):box('EndBoard',(x,0,zc),(WALL_T,2*OY,board_h),wood,.003)
# 세로 기둥·모서리 기둥 — 널 바깥에 붙는다.
for y in (-OY-.015,OY+.015):
    for x in (-.60,-.20,.20,.60):box('SideUpright',(x,y,DECK+WALL/2-.02),(.035,.03,WALL+.04),yellow,.004)
for x in (-OX-.015,OX+.015):
    for y in (-.20,.20):box('EndUpright',(x,y,DECK+WALL/2-.02),(.03,.035,WALL+.04),yellow,.004)
    for y in (-OY-.015,OY+.015):box('CornerPost',(x,y,DECK+WALL/2-.03),(.03,.03,WALL+.06),yellow,.004)
# 벽 위 캡 레일.
for y in (-OY-.005,OY+.005):box('SideCap',(0,y,DECK+WALL-.01),(2*OX+.03,.045,.02),yellow,.003)
for x in (-OX-.005,OX+.005):box('EndCap',(x,0,DECK+WALL-.01),(.045,2*OY+.01,.02),yellow,.003)
# 앞쪽(+X) 밀기 손잡이 — 짐칸 앞 벽 밖으로 뻗어 개구부를 가리지 않는다. 가로봉 중심 (GRIP_X, 0, GRIP_Z).
for y in (-.30,.30):
    box('HandleGusset',(OX+.025,y,.44),(.05,.06,.08),yellow,.003)
    bar('HandleLower',(OX+.01,y,.44),(.88,y,.72),.025,yellow)
    bar('HandleUpper',(.88,y,.72),(GRIP_X,y,GRIP_Z),.025,yellow)
bar('HandleBrace',(.76,-.30,.58),(.76,.30,.58),.02,yellow)
bar('HandleCross',(GRIP_X,-.34,GRIP_Z),(GRIP_X,.34,GRIP_Z),.030,dark)
bar('HandleGrip',(GRIP_X,-.22,GRIP_Z),(GRIP_X,.22,GRIP_Z),.037,rubber,'M_CL9_Cart_Rubber')
box('FrontIDPlate',(OX+.005,0,.66),(.010,.20,.12),dark,.003)
box('FrontReflector',(OX+.013,0,.66),(.008,.115,.026),(.60,.46,.20,1),.002)

bpy.ops.object.select_all(action='DESELECT')
for o in parts:o.select_set(True)
o=parts[0];bpy.context.view_layer.objects.active=o;bpy.ops.object.join();o.name='SM_Cart_A'
bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
lo=Vector(tuple(min(v.co[i] for v in o.data.vertices) for i in range(3)));hi=Vector(tuple(max(v.co[i] for v in o.data.vertices) for i in range(3)));mid=(lo+hi)/2
for v in o.data.vertices:v.co-=mid
bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));assert all(e.is_manifold for e in bm.edges);bm.to_mesh(o.data);bm.free()
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
bpy.context.view_layer.update();o.data.calc_loop_triangles()

# ---- UCX 컨벡스 — 렌더 메시와 같은 원점(피벗 이동을 똑같이 뺀다). 짐칸 안쪽은 비워 둔다.
hulls=[]
def hull(points):
    bm=bmesh.new()
    for p in points:bm.verts.new(Vector(p)-mid)
    bmesh.ops.convex_hull(bm,input=list(bm.verts),use_existing_faces=False)
    unused=[v for v in bm.verts if not v.link_faces]
    if unused:bmesh.ops.delete(bm,geom=unused,context='VERTS')
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));assert all(e.is_manifold for e in bm.edges)
    data=bpy.data.meshes.new('SM_Cart_A_Convex');bm.to_mesh(data);bm.free()
    h=bpy.data.objects.new('UCX_SM_Cart_A_'+str(len(hulls)).zfill(2),data);coll.objects.link(h);hulls.append(h);return h
def hullbox(center,size):
    c=Vector(center);hs=Vector(size)/2
    return hull([(c.x+sx*hs.x,c.y+sy*hs.y,c.z+sz*hs.z) for sx in (-1,1) for sy in (-1,1) for sz in (-1,1)])
ucx_desc=[]
ucx_desc.append('deck');hullbox((0,0,DECK-.04),(2*OX+.03,2*OY+.01,.08))               # 00 바닥판(윗면 = DECK)
for y in (-1,1):ucx_desc.append('side_wall');hullbox((0,y*(HY+.03),DECK+WALL/2),(2*OX+.03,.06,WALL))   # 01·02 측벽(안쪽 면 ±HY)
for x in (-1,1):ucx_desc.append('end_wall');hullbox((x*(HX+.03),0,DECK+WALL/2),(.06,2*OY+.06,WALL))    # 03·04 앞뒤 벽(안쪽 면 ±HX)
ucx_desc.append('handle_lower');hull([(OX+.03,y,z) for y in (-.34,.34) for z in (.40,.50)]+[(.88,y,z) for y in (-.34,.34) for z in (.67,.77)])   # 05
ucx_desc.append('handle_upper');hull([(.86,y,z) for y in (-.36,.36) for z in (.67,.77)]+[(x,y,z) for x in (.94,GRIP_X+.04) for y in (-.36,.36) for z in (GRIP_Z-.04,GRIP_Z+.04)])   # 06
for x in (-WHEEL_X,WHEEL_X):                                                                       # 07·08 차축+바퀴 — 16각 프리즘, 밑점이 지면
    ucx_desc.append('axle_wheels');n=16
    hull([(x+WHEEL_R*math.cos(i*math.tau/n),y,WHEEL_Z+WHEEL_R*math.sin(i*math.tau/n)) for i in range(n) for y in (-WHEEL_Y-WHEEL_HALF_W,WHEEL_Y+WHEEL_HALF_W)])

# ---- 소켓 — SOCKET_ 접두사 빈 오브젝트를 메시의 자식으로. UE 가 접두사를 떼어 Grip·Cargo 로 만든다.
grip=Vector((GRIP_X,0,GRIP_Z))-mid;cargo=Vector((0,0,DECK+WALL/2))-mid
sockets={}
for name,loc in (('Grip',grip),('Cargo',cargo)):
    e=bpy.data.objects.new('SOCKET_'+name,None);sock.objects.link(e);e.empty_display_size=.1;e.empty_display_type='ARROWS'
    e.parent=o;e.matrix_parent_inverse.identity();e.location=loc;sockets[name]=e

anchors={'CargoFloor':(0,0,DECK),'Grip':(GRIP_X,0,GRIP_Z)}
for i,c in enumerate(centers):anchors['Wheel_'+str(i)]=c
for name,c in anchors.items():
    empty=bpy.data.objects.new('REF_Cart_'+name,None);ref.objects.link(empty);empty.location=Vector(c)-mid;empty.empty_display_size=.08

bpy.ops.object.select_all(action='DESELECT');o.select_set(True)
for h in hulls:h.select_set(True)
for e in sockets.values():e.select_set(True)
bpy.context.view_layer.objects.active=o
f=ROOT/'export/SM_Cart_A.fbx'
bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH','EMPTY'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
for h in hulls:h.hide_render=True;h.hide_set(True)
cm=lambda v:[round(x*100,4) for x in v]
meta={'object':o.name,'dimensions_cm':[v*100 for v in o.dimensions],'triangles':len(o.data.loop_triangles),'vertices':len(o.data.vertices),
      'uv_layers':len(o.data.uv_layers),'materials':[m.name for m in o.data.materials],
      'closed_surfaces':True,'fbx_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),
      'bounds_center_cm':[0,0,0],'ground_z_cm':round((lo.z-mid.z)*100,4),
      'deck_height_cm':round((DECK-mid.z)*100,4),'wall_height_cm':WALL*100,'handle_height_cm':GRIP_Z*100,
      'cargo_extent_cm':[HX*100,HY*100,WALL/2*100],'cargo_center_cm':cm(cargo),'grip_cm':cm(grip),
      'cargo_clear_area_cm':[CARGO_X*100,CARGO_Y*100],
      'wheel_radius_cm':WHEEL_R*100,'wheel_rotation_axis':'Y','wheel_centers_cm':[cm(Vector(c)-mid) for c in centers],
      'ucx_count':len(hulls),'collision_hulls':len(hulls),'ucx_names':[h.name for h in hulls],'ucx_roles':ucx_desc,
      'sockets':{name:cm(e.location) for name,e in sockets.items()},
      'anchors_cm':{name:cm(Vector(c)-mid) for name,c in anchors.items()},
      'note':'물리 몸체(bMeshIsBody). UCX 9개가 심플 콜리전, 짐칸 안쪽은 비어 있다. UE 는 Y 를 뒤집는다 — 소켓 두 개는 Y=0.'}
manifest={'source':'Original CL9 farm cart geometry, Tools/Blender/build_cart_a.py (headless)','source_blend':'cart_a.blend','pivot':'bounds center',
          'axis':'+X handle / +Z up','materials':materials,'assets':{'cart':meta}}
(ROOT/'export/manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
bpy.data.libraries.write(str(ROOT/'cart_a.blend'),{s},fake_user=True)
print(json.dumps(meta))
