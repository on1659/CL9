"""Original fixed food-truck shell and independent rear steps; no vehicle gameplay.
Final recipe after this initial build: fit_food_truck_surfaces.py,
fit_food_truck_kitchen_clearance.py, fit_food_truck_tail_lamps.py,
then clean_food_truck_grille.py (weld coincident bevel vertices).
Each fit has a manifest guard and must run exactly once on a new draft.
Existing delivered scenes must be opened/reused, not rebuilt over user work.
"""
import bpy,sys,math,importlib
from pathlib import Path
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
import static_meshkit
importlib.reload(static_meshkit)
from static_meshkit import StaticKit
ROOT=Path('D:/Work/unreal/CL9/ArtSource/map/food_truck')
paint='M_CL9_FoodTruck_Paint';red='M_CL9_FoodTruck_Red';steel='M_CL9_FoodTruck_Steel';dark='M_CL9_FoodTruck_Dark'
colors={paint:(.38,.33,.23,1),red:(.19,.032,.022,1),steel:(.095,.112,.12,1),dark:(.014,.023,.027,1)}
k=StaticKit('CL9_FOOD_TRUCK',ROOT/'body','food_truck.blend',
 {m:{'vertex_color':False,'base_color':c,'roughness':.48 if m==steel else .77,'metallic':.72 if m==steel else .08} for m,c in colors.items()})
hulls=[];convexes=[]
def box(name,center,size,mat=paint,bevel=.01,collision=False):
 o=k.box(name,center,size,colors[mat],mat,bevel)
 if collision:hulls.append((center,size))
 return o
def mesh(name,verts,faces,mat):
 data=bpy.data.meshes.new(name);data.from_pydata(verts,[],faces);data.update()
 o=bpy.data.objects.new(name,data);k.collection.objects.link(o);return k.part(o,name,colors[mat],mat)
def extrude_xz(name,points,y0,y1,mat):
 n=len(points);vs=[(x,y,z) for y in (y0,y1) for x,z in points]
 return mesh(name,vs,[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)],mat)
# Chassis, floor and open cargo shell.
for y in (-.72,.72):box('ChassisRail',(.03,y,.49),(5.92,.14,.18),steel,.01,True)
box('CargoFloor',(-.80,0,.60),(4.20,2.60,.08),steel,.005,True)
box('CargoRoof',(-.80,0,3.025),(4.20,2.60,.11),paint,.018,True)
box('CabinBulkhead',(1.265,0,1.805),(.07,2.60,2.33),paint,.004,True)
box('BlindSide',(-.80,-1.26,1.805),(4.20,.08,2.33),paint,.005,True)
box('BlindSideRedBand',(-.80,-1.306,1.13),(4.08,.02,.45),red,.008)
# Service side opening X[-2.15,.65], Z[1.54,2.72].
box('ServiceLower',(-.80,1.26,1.09),(4.20,.08,.90),paint,.005,True)
box('ServiceUpper',(-.80,1.26,2.845),(4.20,.08,.25),paint,.004,True)
box('ServiceRearPost',(-2.525,1.26,2.13),(.75,.08,1.18),paint,.004,True)
box('ServiceFrontPost',(.975,1.26,2.13),(.65,.08,1.18),paint,.004,True)
box('ServiceRedBand',(-.80,1.306,1.13),(4.08,.02,.45),red,.008)
box('ServingSill',(-.75,1.35,1.51),(2.90,.26,.06),steel,.004,True)
# Rear doorway at Y[-1.17,.07], floor64cm to lintel270cm.
box('RearNarrowPost',(-2.86,-1.235,1.67),(.08,.13,2.06),paint,.004,True)
box('RearWidePost',(-2.86,.685,1.67),(.08,1.23,2.06),paint,.004,True)
box('RearLintel',(-2.86,0,2.835),(.08,2.60,.27),paint,.004,True)
for y in (-1.15,.05):box('DoorJambTrim',(-2.908,y,1.67),(.025,.025,2.02),steel,.002)
box('RearDoorTopTrim',(-2.908,-.55,2.688),(.025,1.20,.025),steel,.002)
# Inner rear wheel housings leave the centre aisle open.
for y in (-1.085,1.085):box('InteriorWheelHousing',(-2.05,y,.835),(1.02,.35,.39),steel,.045,True)
# Cab volume starts above wheel crowns; opaque windows are surface inserts.
cab=[(1.31,1.01),(3.10,1.01),(3.10,1.48),(2.64,1.60),(2.28,2.58),(1.31,2.66)]
extrude_xz('CabShell',cab,-1.10,1.10,paint)
for profile in [[(1.31,1.01),(2.64,1.01),(2.64,1.60),(2.28,2.58),(1.31,2.66)],[(2.64,1.01),(3.10,1.01),(3.10,1.48),(2.64,1.60)]]:
 convexes.append([(x,y,z) for y in (-1.1,1.1) for x,z in profile])
for side in (-1,1):
 y=side*1.112
 extrude_xz('CabSideWindow',[(1.44,1.70),(2.12,1.70),(2.17,2.46),(1.44,2.51)],y-.005,y+.005,dark)
 box('CabLowerBand',(2.16,side*1.112,1.30),(1.62,.025,.30),red,.005)
 box('CabDoorHandle',(1.62,side*1.133,1.57),(.18,.025,.04),steel,.007)
 k.bar('MirrorArm',(1.56,side*1.08,2.11),(1.66,side*1.47,2.14),.022,colors[steel],steel,10)
 box('MirrorHousing',(1.68,side*1.49,2.14),(.16,.09,.27),dark,.025)
# Windshield as a thin sloped closed prism, on the front plane of the cab.
extrude_xz('Windshield',[(2.331,2.445),(2.604,1.702),(2.616,1.706),(2.343,2.449)],-.94,.94,dark)
box('WindshieldCentreBar',(2.471,0,2.083),(.02,.025,.80),paint,.002).rotation_euler.y=math.radians(20)
box('FrontBumper',(3.14,0,.75),(.16,2.28,.19),steel,.025,True)
box('RearBumper',(-2.99,.70,.52),(.18,1.08,.16),steel,.018,True)
box('FrontGrille',(3.108,0,1.19),(.022,1.09,.22),dark,.012)
for z in (1.12,1.19,1.26):box('GrilleSlat',(3.125,0,z),(.015,.98,.012),steel,.002)
for y in (-.80,.80):
 k.bar('HeadlightRim',(3.10,y,1.25),(3.15,y,1.25),.145,colors[steel],steel,20)
 k.bar('HeadlightGlass',(3.15,y,1.25),(3.165,y,1.25),.12,colors[paint],paint,20)
 box('RearLamp',(-2.925,y,1.05),(.025,.16,.12),red,.012)
# Four faceted tyres and nested hubs. Outer silhouette is round, with bevelled sidewalls.
for x in (-2.05,2.10):
 for side in (-1,1):
  cy=side*1.20;n=24;profile=[(-.18,.38),(-.145,.46),(.145,.46),(.18,.38)]
  vs=[(x+r*math.cos(i*math.tau/n),cy+d,.46+r*math.sin(i*math.tau/n)) for d,r in profile for i in range(n)]
  fs=[tuple(range(n-1,-1,-1)),tuple(3*n+i for i in range(n))]
  for j in range(3):fs.extend((j*n+i,j*n+(i+1)%n,(j+1)*n+(i+1)%n,(j+1)*n+i) for i in range(n))
  mesh('Tyre',vs,fs,dark)
  outer=cy+side*.186
  k.bar('WheelRim',(x,outer,.46),(x,outer+side*.024,.46),.285,colors[steel],steel,24)
  k.bar('WheelHub',(x,outer+side*.025,.46),(x,outer+side*.043,.46),.115,colors[paint],paint,16)
  for j in range(6):
   a=j*math.tau/6;k.bar('WheelBolt',(x+.18*math.cos(a),outer+side*.025,.46+.18*math.sin(a)),(x+.18*math.cos(a),outer+side*.04,.46+.18*math.sin(a)),.015,colors[dark],dark,8)
  convexes.append(vs)
  # Curved fixed fender above the crown, not a filled box over the tyre.
  for j in range(12):
   a=j*math.pi/12;b=(j+1)*math.pi/12
   pts=[(x+.485*math.cos(a),.46+.485*math.sin(a)),(x+.55*math.cos(a),.46+.55*math.sin(a)),(x+.55*math.cos(b),.46+.55*math.sin(b)),(x+.485*math.cos(b),.46+.485*math.sin(b))]
   extrude_xz('WheelArch',pts,side*1.11-.04,side*1.11+.04,red)
# Fixed open service canopy, high enough for a standing pawn beneath it.
awning=[(-2.25,1.30,2.82),(.75,1.30,2.82),(.75,2.0,2.72),(-2.25,2.0,2.72)]
mesh('OpenServiceCanopy',awning+[(x,y,z+.04) for x,y,z in awning],[(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)],red)
for x in (-2.17,.67):k.bar('CanopyStay',(x,1.32,2.43),(x,1.92,2.74),.018,colors[steel],steel,10)
box('RoofVent',(-1.4,0,3.16),(.64,.52,.16),steel,.035)
for y in (-.16,-.08,0,.08,.16):box('RoofVentSlot',(-1.4,y,3.245),(.44,.025,.012),dark,.003)
# Simple egg mascot on blind side, echoed by the existing henhouse pictogram language.
egg=[(-.4+.28*math.cos(i*math.tau/24)*(1-.15*math.sin(i*math.tau/24)),2.13+.37*math.sin(i*math.tau/24)) for i in range(24)]
extrude_xz('EggMascot',egg,-1.33,-1.318,red)
for x in (-.50,-.32):k.bar('MascotEye',(x,-1.335,2.19),(x,-1.343,2.19),.025,colors[dark],dark,10)
extrude_xz('MascotBeak',[(-.44,2.11),(-.29,2.11),(-.37,2.03)],-1.345,-1.336,paint)
k.finish('food_truck','SM_FoodTruck_A',None,
 {'RearThreshold':(-2.90,-.55,.64),'StepsOrigin':(-3.50,-.55,0),'ServiceSill':(-.75,1.35,1.54),
  'KitchenFootprintCentre':(-.70,.57,.64),'RearPanelMount':(-2.93,.68,.64)},
 {'rig':'N/A fixed base vehicle; opaque closed cab, fixed open cargo door/canopy','cargo_floor_height_cm':64,
  'rear_opening_width_cm':124,'rear_opening_height_cm':206,'material_budget_exception':'4 slots for painted body/red band/metal/tyres and opaque windows'},
 pivot='floor center',collision_boxes=hulls,collision_convexes=convexes)

# Independent four-step component, +X rises toward the truck's rear threshold.
st='M_CL9_TruckSteps_Steel';mark='M_CL9_TruckSteps_Mark'
k=StaticKit('CL9_FOOD_TRUCK_STEPS',ROOT/'steps','steps.blend',
 {st:{'vertex_color':False,'base_color':colors[steel],'roughness':.78,'metallic':.6},mark:{'vertex_color':False,'base_color':colors[paint],'roughness':.8,'metallic':.1}})
steps=[]
for i in range(4):
 x=-.45+i*.30;h=.16*(i+1)
 k.box('Step', (x,0,h/2),(.30,1.14,h),colors[steel],st,.003);steps.append(((x,0,h/2),(.30,1.14,h)))
 k.box('StepEdgeMark',(x-.134,0,h-.0005),(.02,1.0,.001),colors[paint],mark,0)
k.finish('food_truck_steps','SM_FoodTruck_Steps_A',None,{'TruckJoin':(.60,0,.64),'Approach':(-.60,0,0)},
 {'rig':'N/A fixed steps','rise_cm':16,'run_cm':30,'step_count':4},pivot='floor center',collision_boxes=steps)
