"""Original enamel griddle station matching the current 300x120x90 work area."""
import sys,importlib,bpy,math
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
enamel='M_CL9_Kitchen_Enamel';steel='M_CL9_Kitchen_Steel';red='M_CL9_Kitchen_Red'
colors={enamel:(.36,.32,.23,1),steel:(.043,.053,.056,1),red:(.19,.036,.023,1)}
k=StaticKit('CL9_KITCHEN','D:/Work/unreal/CL9/ArtSource/map/kitchen','kitchen.blend',
 {m:{'vertex_color':False,'base_color':c,'roughness':.72 if m==enamel else .62,'metallic':.65 if m==steel else .12} for m,c in colors.items()})
collision=[]
# Full flat cook surface; surrounding enamel ledges have the exact same top plane.
k.box('IronGriddle',(0,0,.88),(1.08,2.88,.04),colors[steel],steel,.005)
for x in (-.57,.57):k.box('LongEnamelEdge',(x,0,.875),(.06,3,.05),colors[enamel],enamel,.004)
for y in (-1.47,1.47):k.box('ShortEnamelEdge',(0,y,.875),(1.08,.06,.05),colors[enamel],enamel,.004)
collision.append(((0,0,.875),(1.2,3,.05)))
k.box('CabinetBody',(0,0,.52),(1.04,2.82,.62),colors[enamel],enamel,.012)
collision.append(((0,0,.52),(1.04,2.82,.62)))
for x in (-.42,.42):
 for y in (-1.30,0,1.30):
  k.bar('SupportLeg',(x,y,.012),(x,y,.25),.045,colors[steel],steel,12)
  k.bar('RoundFoot',(x,y,0),(x,y,.025),.068,colors[steel],steel,12)
  collision.append(((x,y,.125),(.13,.13,.25)))
# Front +X: one continuous fascia and three service panels, entirely below the top.
k.box('RedFascia',(.535,0,.752),(.04,2.81,.145),colors[red],red,.007)
for y in (-.92,0,.92):
 k.box('ServicePanel',(.535,y,.446),(.035,.865,.44),colors[enamel],enamel,.006)
 for yy in (y-.18,y+.18):k.bar('HandleMount',(.551,yy,.58),(.584,yy,.58),.014,colors[steel],steel,10)
 k.bar('FixedHandle',(.584,y-.18,.58),(.584,y+.18,.58),.016,colors[steel],steel,12)
 for z in (.325,.36,.395):k.box('VentSlot',(.555,y,z),(.006,.55,.018),colors[steel],steel,.003)
# Fixed knob silhouettes and a hot-surface icon; no gameplay controls implied by animation.
for y in (-1.12,1.12):
 k.bar('ControlBezel',(.555,y,.755),(.564,y,.755),.046,colors[steel],steel,20)
 k.bar('FixedDial',(.565,y,.755),(.586,y,.755),.032,colors[steel],steel,16)
 k.box('DialIndex',(.588,y,.768),(.005,.008,.022),colors[enamel],enamel,.001)
def icon(name,points):
 n=len(points);verts=[(x,y,z) for x in (.558,.563) for y,z in points]
 faces=[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
 o=bpy.data.objects.new(name,mesh);k.collection.objects.link(o);k.part(o,name,colors[enamel],enamel)
icon('HotPlatePictogram',[(-.11,.704),(.11,.704),(.11,.72),(-.11,.72)])
for y in (-.063,0,.063):
 icon('HeatWave',[(y-.008,.733),(y+.006,.733),(y+.017,.754),(y+.004,.772),(y+.016,.792),(y+.003,.797),(y-.012,.772),(y+.002,.753)])
# Side inset protection rails remain within the tabletop footprint.
for y in (-1.43,1.43):
 k.box('SideRedInset',(0,y,.58),(.84,.025,.31),colors[red],red,.006)
 for z in (.49,.54,.59,.64):k.box('SideVent',(0,y+math.copysign(.015,y),z),(.52,.009,.018),colors[steel],steel,.002)
k.finish('kitchen','SM_Kitchen_A',None,{'CookTop':(0,0,.9),'Front':(.6,0,.5)},
 {'rig':'N/A fixed griddle; no moving lid or heat controls','top_height_cm':90,'placement_yaw_for_legacy_extent':90,
  'heat_contract':'HeatZone derives XY bounds and topZ; no geometry above flat 90cm surface'},
 pivot='floor center',collision_boxes=collision)
