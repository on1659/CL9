import sys,math
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from static_meshkit import StaticKit
frame='M_CL9_StandingLamp_Frame';diffuser='M_CL9_StandingLamp_Diffuser'
spec={frame:{'vertex_color':True,'roughness':.72,'metallic':.20},
      diffuser:{'vertex_color':False,'base_color':[.50,.58,.52,1],'roughness':.4,'metallic':.05,
                'emission_color':[1,.68,.30,1],'emission_parameter':'LightStrength','emission_default':0,'preview_on_strength':4}}
k=StaticKit('CL9_STANDING_LAMP_A','D:/Work/unreal/CL9/ArtSource/item/standing_lamp_a','standing_lamp_a.blend',spec)
yellow=(.39,.23,.038,1);dark=(.026,.034,.036,1);steel=(.26,.30,.31,1);red=(.34,.035,.019,1)
for i in range(3):
    a=(i*120+90)*math.pi/180;x=.43*math.cos(a);y=.43*math.sin(a)
    k.bar('TripodLeg',(.03*math.cos(a),.03*math.sin(a),.34),(x,y,.045),.026,yellow,frame,12)
    o=k.box('TripodFoot',(x,y,.025),(.11,.075,.05),dark,frame,.008);o.rotation_euler.z=a
    k.bar('LegBrace',(0,0,.16),(.23*math.cos(a),.23*math.sin(a),.20),.012,steel,frame,10)
k.bar('TripodHub',(0,0,.12),(0,0,.37),.060,dark,frame,20)
k.bar('LowerPost',(0,0,.33),(0,0,.99),.032,yellow,frame,20)
k.bar('UpperPost',(0,0,.96),(0,0,1.245),.024,steel,frame,20)
k.bar('PostCollar',(0,0,.945),(0,0,1.005),.042,dark,frame,20)
k.box('ClampKnob',(.050,0,.976),(.047,.055,.039),dark,frame,.005)
k.box('PowerHousing',(.065,0,1.075),(.095,.14,.19),yellow,frame,.009)
k.box('PowerSwitch',(.116,0,1.11),(.014,.050,.041),red,frame,.003)
k.bar('LampBase',(0,0,1.235),(0,0,1.295),.130,yellow,frame,24)
k.bar('Diffuser',(0,0,1.292),(0,0,1.574),.094,(1,1,1,1),diffuser,32)
k.bar('TopCap',(0,0,1.573),(0,0,1.618),.135,yellow,frame,24)
for i in range(8):
    a=i*math.tau/8;x=.116*math.cos(a);y=.116*math.sin(a)
    k.bar('GuardBar',(x,y,1.274),(x,y,1.595),.009,dark,frame,10)
for y in (-.075,.075):k.bar('CarryHandleUpright',(0,y,1.607),(0,y,1.773),.018,dark,frame,12)
k.bar('CarryGrip',(0,-.075,1.773),(0,.075,1.773),.023,dark,frame,16)
k.finish('standing_lamp','SM_StandingLamp_A',None,{'Floor':(0,0,0),'LightOrigin':(0,0,1.433),'CarryGrip':(0,0,1.773)},
         {'rig':'N/A static prop','states':{'off':{'LightStrength':0},'on_preview':{'LightStrength':4}}})
