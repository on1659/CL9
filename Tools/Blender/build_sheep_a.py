"""Original wool-covered sheep with a dark long face, lateral ears and split hooves."""
import bpy,sys,math
sys.path.insert(0,'D:/Work/unreal/CL9/Tools/Blender')
from creature_meshkit import MeshKit
k=MeshKit('Sheep',{'wool':(.64,.60,.46,1),'light':(.82,.77,.59,1),'shade':(.39,.38,.29,1),
                  'skin':(.043,.050,.041,1),'inner':(.16,.11,.085,1),'hoof':(.014,.016,.013,1),
                  'eye':(.50,.39,.17,1),'black':(.005,.006,.004,1)})
body=k.ell('FleeceBody',(-.19,0,.73),(.57,.36,.36),'wool','body',24,16);k.chest_weights(body)
# Large low-poly wool lobes, restricted to the outer silhouette and belly boundary.
index=0
for x,rx in [(-.57,.21),(-.32,.235),(-.05,.24),(.20,.21)]:
    for angle in [-.90,-.20,.50,1.20,1.90,2.60,3.30,4.00]:
        y=.276*math.cos(angle);z=.73+.286*math.sin(angle)
        tuft=k.ell('Wool'+str(index),(x,y,z),(rx,.145,.158),'light' if index%4==0 else 'wool','body',12,8,rotation=(.1*math.sin(index),0,.09*math.cos(index)))
        k.chest_weights(tuft);index+=1
k.ell('Chest',(.255,0,.71),(.22,.27,.28),'wool','chest',20,12)
k.ell('Neck',(.36,0,.83),(.19,.19,.23),'skin','neck')
k.ell('Head',(.48,0,.91),(.19,.16,.245),'skin','head',20,14,rotation=(0,-.20,0))
k.ell('Muzzle',(.64,0,.757),(.173,.139,.12),'skin','head',18,12,rotation=(0,.23,0))
k.ell('Nose',(.773,0,.737),(.039,.112,.068),'hoof','head',14,10)
k.ell('Jaw',(.65,0,.689),(.157,.118,.04),'skin','jaw',16,10)
for i,(x,y,z) in enumerate([(.40,-.07,1.091),(.39,.07,1.095),(.52,-.045,1.083),(.52,.055,1.08),(.34,0,1.063)]):
    k.ell('Forelock'+str(i),(x,y,z),(.105,.099,.081),'light','head',12,8)
for sign,side in [(1,'l'),(-1,'r')]:
    k.ell('Eye_'+side,(.593,sign*.126,.926),(.060,.030,.043),'eye','head',14,10,rotation=(0,0,-sign*.35))
    k.ell('Pupil_'+side,(.611,sign*.15,.926),(.035,.010,.014),'black','head',12,8)
    k.ell('Brow_'+side,(.56,sign*.14,.968),(.071,.036,.029),'skin','head',14,8)
    k.ell('Ear_'+side,(.396,sign*.264,1.00),(.11,.192,.056),'skin','ear_'+side,16,10,rotation=(sign*.20,0,sign*.12))
    k.ell('InnerEar_'+side,(.41,sign*.278,1.035),(.074,.143,.018),'inner','ear_'+side,14,8,rotation=(sign*.20,0,sign*.12))
    for pair,x,knee_x in [('front',.27,.32),('hind',-.40,-.53)]:
        tag=pair+'_'+side;y=sign*(.215 if pair=='front' else .235);hip=(x,y,.48);knee=(knee_x,y,.28);ankle=(x+.035,y,.07)
        k.ell('Haunch_'+tag,hip,(.137,.124,.147),'wool',tag+'_upper',16,10)
        k.taper('Upper_'+tag,hip,knee,.068,.042,'skin',tag+'_upper')
        k.taper('Lower_'+tag,knee,ankle,.042,.033,'skin',tag+'_lower')
        for j in (-1,1):
            k.ell('Hoof_'+tag+str(j),(ankle[0]+.024,y+j*.031,.053),(.067,.028,.041),'hoof',tag+'_paw',12,8)
for i in range(4):
    k.ell('Tail'+str(i),(-.73-.017*i,0,.76-.032*i),(.087-.01*i,.068-.006*i,.071),'wool',f'tail_{i+1:02d}',12,8)
bones=[('root',(0,0,0),(0,0,.15),None),('body',(-.30,0,.53),(.08,0,.53),'root'),
       ('chest',(.08,0,.53),(.28,0,.61),'body'),('neck',(.28,0,.61),(.40,0,.85),'chest'),
       ('head',(.40,0,.85),(.61,0,.88),'neck'),('jaw',(.56,0,.71),(.76,0,.71),'head')]
for sign,side in [(1,'l'),(-1,'r')]:
    bones.append(('ear_'+side,(.40,sign*.13,1.0),(.39,sign*.40,1.03),'head'))
    for pair,x,parent,knee_x in [('front',.27,'chest',.32),('hind',-.40,'body',-.53)]:
        tag=pair+'_'+side;y=sign*(.215 if pair=='front' else .235)
        bones.extend([(tag+'_upper',(x,y,.48),(knee_x,y,.28),parent),(tag+'_lower',(knee_x,y,.28),(x+.035,y,.07),tag+'_upper'),
                      (tag+'_paw',(x+.035,y,.07),(x+.17,y,.045),tag+'_lower')])
for i in range(4):bones.append((f'tail_{i+1:02d}',(-.70-.02*i,0,.80-.04*i),(-.72-.02*i,0,.76-.04*i),'body' if i==0 else f'tail_{i:02d}'))
k.finish_rig(bones)
