"""Three independent structural pieces plus a legacy Stretch-compatible ceiling export."""
import bpy,sys,importlib,json,hashlib
from pathlib import Path
from mathutils import Vector
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/henhouse_structure')
wood='M_CL9_Structure_Wood';metal='M_CL9_Structure_Metal'
# Names are unique per kit to keep reimports and original sources independent.
def kit(label,with_metal=True):
    global wood,metal
    wood='M_CL9_'+label.title()+'_Wood';metal='M_CL9_'+label.title()+'_Metal'
    specs={wood:{'vertex_color':True,'roughness':.91,'metallic':0}}
    if with_metal:specs[metal]={'vertex_color':True,'roughness':.69,'metallic':.55}
    return StaticKit('CL9_HENHOUSE_'+label.upper(),ROOT/label,label+'.blend',specs)
timber=(.19,.125,.064,1);steel=(.047,.056,.053,1);bolt=(.20,.215,.17,1)
k=kit('beam')
k.box('TimberBeam',(0,0,.16),(5,.24,.28),timber,wood,.003)
for x in (-2.32,2.32):
    for y in (-.133,.133):
        k.box('IronBandSide',(x,y,.16),(.14,.014,.32),steel,metal,.002)
        k.bar('BandBolt',(x,y,.16),(x,y+(.004 if y>0 else -.004),.16),.018,bolt,metal,8)
    for z in (.007,.313):k.box('IronBandCap',(x,0,z),(.14,.28,.014),steel,metal,.002)
k.finish('henhouse_beam','SM_Henhouse_Beam_500_A',None,{'EndLeft':(-2.5,0,0),'EndRight':(2.5,0,0)},
    {'rig':'N/A static structure'},pivot='floor center',collision_boxes=[((0,0,.16),(5,.28,.32))])
k=kit('pillar')
k.box('TimberPillar',(0,0,2),(.24,.24,3.96),(.17,.111,.056,1),wood,.003)
for z in (.03,3.97):k.box('EndPlate',(0,0,z),(.28,.28,.06),steel,metal,.002)
for z in (.135,3.865):
    for y in (-.127,.127):
        k.box('SocketSideY',(0,y,z),(.268,.014,.15),steel,metal,.002)
        k.bar('SocketBoltY',(0,y,z),(0,y+(.005 if y>0 else -.005),z),.016,bolt,metal,8)
    for x in (-.127,.127):k.box('SocketSideX',(x,0,z),(.014,.24,.15),steel,metal,.002)
k.finish('henhouse_pillar','SM_Henhouse_Pillar_400_A',None,{'Base':(0,0,0),'Top':(0,0,4)},
    {'rig':'N/A static structure'},pivot='floor center',collision_boxes=[((0,0,2),(.28,.28,4))])
k=kit('ceiling',False)
for i in range(10):
    v=1+(i%3-1)*.055
    k.box('CeilingBoard',(0,-2.25+i*.5,.04),(5,.5,.08),(.35*v,.32*v,.245*v,1),wood,.003)
for y in (-2.43,-1.25,0,1.25,2.43):k.box('CeilingJoist',(0,y,.17),(5,.14,.18),timber,wood,.003)
for x in (-2.43,2.43):k.box('CeilingEdge',(x,0,.17),(.14,5,.18),timber,wood,.003)
k.box('CeilingBack',(0,0,.28),(5,5,.04),(.14,.113,.077,1),wood,.002)
o=k.finish('henhouse_ceiling','SM_Henhouse_Ceiling_500_A',None,
    {'EdgeLeft':(-2.5,0,0),'EdgeRight':(2.5,0,0),'EdgeFront':(0,-2.5,0),'EdgeBack':(0,2.5,0)},
    {'rig':'N/A static structure'},pivot='floor center',collision_boxes=[((0,0,.15),(5,5,.3))])
# A distinct normalized export for the current 100-unit-centered Stretch contract.
normalized=bpy.data.collections.new('CL9_HENHOUSE_CEILING_STRETCH');k.scene.collection.children.link(normalized)
copies=[]
for original in [o,bpy.data.objects['UCX_SM_Henhouse_Ceiling_500_A_00']]:
    copy=original.copy();copy.data=original.data.copy();copy.hide_render=False;copy.hide_set(False)
    copy.name='SM_Henhouse_Ceiling_Stretch100_A' if original==o else 'UCX_SM_Henhouse_Ceiling_Stretch100_A_00'
    normalized.objects.link(copy)
    for v in copy.data.vertices:v.co=Vector((v.co.x/5,v.co.y/5,(v.co.z-.15)/.3))
    copies.append(copy)
bpy.ops.object.select_all(action='DESELECT')
for copy in copies:copy.hide_set(False);copy.select_set(True)
bpy.context.view_layer.objects.active=copies[0];bpy.context.view_layer.update()
f=k.root/'export/SM_Henhouse_Ceiling_Stretch100_A.fbx'
bpy.ops.export_scene.fbx(filepath=str(f),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=True,bake_anim=False,add_leaf_bones=False)
m=json.loads((k.root/'export/manifest.json').read_text());meta=dict(m['assets']['henhouse_ceiling'])
meta.update({'object':copies[0].name,'dimensions_cm':[x*100 for x in copies[0].dimensions],
    'bounds_center_cm':[0,0,0],'anchors_cm':{},'fbx_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),
    'pivot_override':'bounds center','restore_scale':[5,5,.3],'consumer':'CL9SiteMap Stretch: scale size/100, centered pivot'})
m['assets']['henhouse_ceiling_stretch100']=meta
(k.root/'export/manifest.json').write_text(json.dumps(m,indent=2),encoding='utf-8')
for copy in copies:copy.hide_render=True;copy.hide_set(True)
bpy.data.libraries.write(str(k.root/k.source_blend),{k.scene},fake_user=True)
print('Saved normalized ceiling export with restore scale (5,5,0.3).')
