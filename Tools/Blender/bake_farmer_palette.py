"""Store authored flat colours in a tiny shared palette/UV1; keep UV0 unwrap and vertex colour source."""
import bpy,json,struct,zlib,math
from pathlib import Path
root=Path('D:/Work/unreal/CL9/ArtSource/character/farmer_a');p=root/'manifest.json';m=json.loads(p.read_text());assert not m.get('palette_bake')
objects=[bpy.data.objects[n] for n in ['SK_Farmer_A_Body','SK_Farmer_A_Arms']]
def key(color):return tuple(round(max(0,min(1,float(c))),6) for c in color[:3])
colors=sorted({key(v.color) for o in objects for v in o.data.color_attributes['Color'].data});assert len(colors)<=32
index={c:i for i,c in enumerate(colors)};width=256;height=8
def srgb(v):return round(255*(12.92*v if v<=.0031308 else 1.055*v**(1/2.4)-.055))
pixels=bytes(channel for x in range(width) for channel in [*[srgb(v) for v in colors[min(x//8,len(colors)-1)]],255])
def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data)&0xffffffff)
png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',width,height,8,6,0,0,0))+chunk(b'IDAT',zlib.compress((b'\0'+pixels)*height))+chunk(b'IEND',b'')
folder=root/'textures';folder.mkdir(exist_ok=True);f=folder/'T_Farmer_Palette.png';f.write_bytes(png)
image=bpy.data.images.load(str(f),check_existing=True);image.colorspace_settings.name='sRGB';image.pack()
for o in objects:
 uv=o.data.uv_layers.new(name='PaletteUV')
 for i,v in enumerate(o.data.color_attributes['Color'].data):uv.data[i].uv=((index[key(v.color)]+.5)/32,.5)
for name in ['M_CL9_Farmer_Cloth','M_CL9_Farmer_Detail']:
 mat=bpy.data.materials[name];tree=mat.node_tree;vc=next(n for n in tree.nodes if n.type=='VERTEX_COLOR');tex=tree.nodes.new('ShaderNodeTexImage');tex.image=image;tex.interpolation='Closest';uv=tree.nodes.new('ShaderNodeUVMap');uv.uv_map='PaletteUV';tree.links.new(uv.outputs['UV'],tex.inputs['Vector'])
 for link in list(vc.outputs['Color'].links):tree.links.new(tex.outputs['Color'],link.to_socket)
m['palette_bake']={'texture':'textures/T_Farmer_Palette.png','size':[width,height],'swatches_linear_rgb':colors,'uv_channel':1,'kept_unwrap_channel':0,'reason':'Authored vertex colours present in FBX but actual UE shading returned white; explicit palette texture makes colours reproducible.'}
p.write_text(json.dumps(m,indent=2),encoding='utf-8');print('Created shared palette:',len(colors),'colours/UV1; UV0 and source colours preserved.')
