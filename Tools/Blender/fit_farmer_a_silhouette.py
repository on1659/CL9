"""Refine first draft shoulder/hip joins and broad five-finger glove palms."""
import bpy,json
from pathlib import Path
from mathutils import Vector
import math
root=Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a');p=root/'manifest.json';m=json.loads(p.read_text());assert not m.get('silhouette_fit')
rig=bpy.data.objects['RIG_Farmer_A']
def h(n):return rig.data.bones[n].head_local.copy()
def islands(o):
 adj={v.index:set() for v in o.data.vertices}
 for e in o.data.edges:a,b=e.vertices;adj[a].add(b);adj[b].add(a)
 u=set(adj)
 while u:
  a=u.pop();q=[a];group={a}
  while q:
   for b in adj[q.pop()]:
    if b in u:u.remove(b);q.append(b);group.add(b)
  yield [o.data.vertices[i] for i in sorted(group)]
counts={}
for name in ['SK_Farmer_A_Body','SK_Farmer_A_Arms']:
 o=bpy.data.objects[name];changed=[]
 for vs in islands(o):
  zmin=min(v.co.z for v in vs);zmax=max(v.co.z for v in vs);cx=sum(v.co.x for v in vs)/len(vs);side='l' if cx>0 else 'r'
  if len(vs)==160 and abs(zmin-.86)<1e-5 and abs(zmax-1.475)<1e-5:
   for v in vs:
    if abs(v.co.z-1.43)<1e-5:v.co.x*=.205/.18;v.co.y=-.015+(v.co.y+.015)*.135/.12;v.co.z=1.465
    elif abs(v.co.z-1.475)<1e-5:v.co.x*=.075/.073;v.co.z=1.515
   changed.append('raised_shoulder_neckline')
  elif len(vs)==128 and zmax>1.4 and zmin>1.0:
   c=h('upperarm_'+side).lerp(h('lowerarm_'+side),-.08)
   for v in vs[:16]:v.co=c+(v.co-c)*.35
   changed.append('rounded_sleeve_cap')
  elif len(vs)==112 and zmin<.3 and zmax<1.1:
   c=h('thigh_'+side)+Vector((0,0,.02))
   for v in vs[:16]:v.co=c+(v.co-c)*(.074/.108)
   changed.append('tucked_hip_join')
  elif len(vs)==48 and .8<zmin<1.1 and zmax<1.2:
   wrist=h('hand_'+side);mid=h('middle_01_'+side);u=(h('index_01_'+side)-h('pinky_01_'+side)).normalized();direction=(mid-wrist).normalized();vaxis=direction.cross(u).normalized();u=vaxis.cross(direction).normalized()
   centers=[wrist,wrist.lerp(mid,.36),wrist.lerp(mid,.8),mid];radii=[.028,.041,.047,.047]
   for j in range(4):
    for i in range(12):
     a=math.tau*i/12;vs[j*12+i].co=centers[j]+radii[j]*(math.cos(a)*u+.5*math.sin(a)*vaxis)
   changed.append('broad_glove_palm')
 o.data.update();counts[name]=changed
assert counts['SK_Farmer_A_Body'].count('rounded_sleeve_cap')==2 and counts['SK_Farmer_A_Arms'].count('broad_glove_palm')==2,counts
mat=bpy.data.materials['M_CL9_Farmer_Cloth'];mul=next(n for n in mat.node_tree.nodes if n.type=='MIX_RGB');mul.inputs[2].default_value=(.13,.20,.08,1)
m['silhouette_fit']=counts;m['cloth_tint_default']=[.13,.20,.08,1];p.write_text(json.dumps(m,indent=2),encoding='utf-8')
bpy.context.window.scene=bpy.data.scenes['CL9_FARMER_A'];bpy.data.libraries.write(str(root/'farmer_a.blend'),{bpy.context.scene,bpy.data.scenes['CL9_FARMER_A_PREVIEW']},fake_user=True);print(json.dumps(counts))
