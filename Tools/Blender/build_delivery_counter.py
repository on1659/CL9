"""Original square receiving counter with an explicit package/check pictogram."""
import bpy,sys,importlib
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
cream='M_CL9_Counter_Enamel';steel='M_CL9_Counter_Steel';blue='M_CL9_Counter_Blue'
colors={cream:(.36,.32,.23,1),steel:(.043,.053,.056,1),blue:(.044,.105,.155,1)}
k=StaticKit('CL9_DELIVERY_COUNTER','D:/Work/unreal/CL9/ArtSource/map/delivery_counter','delivery_counter.blend',
 {m:{'vertex_color':False,'base_color':c,'roughness':.76,'metallic':.6 if m==steel else .12} for m,c in colors.items()})
coll=[]
# The recessed-looking receiving pad and rim share a flat support plane at 100cm.
k.box('ReceivingPad',(0,0,.9775),(.78,.78,.045),colors[steel],steel,.006)
for x in (-.445,.445):k.box('TopEdgeX',(x,0,.9675),(.11,1,.065),colors[cream],cream,.008)
for y in (-.445,.445):k.box('TopEdgeY',(0,y,.9675),(.78,.11,.065),colors[cream],cream,.008)
coll.append(((0,0,.9675),(1,1,.065)))
k.box('CabinetCore',(0,0,.555),(.86,.88,.76),colors[cream],cream,.012)
coll.append(((0,0,.555),(.86,.88,.76)))
for x in (-.35,.35):
 for y in (-.36,.36):
  k.bar('Leg',(x,y,.012),(x,y,.20),.04,colors[steel],steel,12)
  k.bar('Foot',(x,y,0),(x,y,.025),.057,colors[steel],steel,12)
  coll.append(((x,y,.10),(.11,.11,.20)))
k.box('BlueFrontPanel',(.44,0,.56),(.035,.82,.69),colors[blue],blue,.009)
for y in (-.31,.31):
 for z in (.27,.85):k.bar('PanelFastener',(.459,y,z),(.466,y,z),.012,colors[steel],steel,8)
# Extruded package outline and check; +X is the service side.
def icon(name,pts,mat=cream,x=.460):
 n=len(pts);vs=[(xx,y,z) for xx in (x,x+.005) for y,z in pts]
 fs=[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(vs,[],fs);mesh.update()
 ob=bpy.data.objects.new(name,mesh);k.collection.objects.link(ob);k.part(ob,name,colors[mat],mat)
icon('Parcel',[(-.21,.49),(.08,.49),(.08,.72),(-.21,.72)])
icon('ParcelTape',[(-.09,.717),(-.055,.717),(-.055,.63),(-.072,.65),(-.09,.63)],blue,.466)
icon('DeliveryCheck',[(.03,.447),(.092,.397),(.235,.558),(.206,.581),(.088,.449),(.052,.477)])
# Fixed side document plate and abstract ruled slip.
for y in (-.448,.448):
 k.box('DocumentBoard',(-.02,y,.64),(.41,.025,.37),colors[steel],steel,.008)
 k.box('DocumentSheet',(-.02,y*1.035,.633),(.32,.007,.29),colors[cream],cream,.003)
 for z in (.55,.59,.63,.67):k.box('PrintedRule',(-.02,y*1.045,z),(.21,.004,.009),colors[blue],blue,.001)
 k.box('DocumentClip',(-.02,y*1.05,.785),(.12,.012,.045),colors[steel],steel,.003)
for y in (-.25,.25):k.bar('RearHandleMount',(-.432,y,.75),(-.467,y,.75),.012,colors[steel],steel,10)
k.bar('RearFixedHandle',(-.467,-.25,.75),(-.467,.25,.75),.016,colors[steel],steel,12)
k.finish('delivery_counter','SM_DeliveryCounter_A',None,{'ReceivingTop':(0,0,1),'ServiceFront':(.5,0,.55)},
 {'rig':'N/A fixed counter; stock is data, no moving drawer','top_height_cm':100},pivot='floor center',collision_boxes=coll)
