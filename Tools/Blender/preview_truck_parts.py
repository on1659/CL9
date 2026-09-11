for part,ortho,focus,camera,energy in [('pad',6,(0,0,.10),(4,-4,5.8),550),('panel',1.85,(0,0,.6),(3,-2,2),170)]:
 ASSET_ROOT='D:/Work/unreal/CL9/ArtSource/map/truck_parts/'+part;KIT_SCENE='CL9_TRUCK_'+part.upper();SOURCE_BLEND=part+'.blend'
 OBJECT_NAMES=['SM_Truck'+part.title()+'_A'];ORTHO=ortho;FOCUS=focus;CAMERA_POSITION=camera;FLASHLIGHT_ENERGY=energy
 exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/preview_static_pair.py',encoding='utf-8').read(),'preview_static_pair.py','exec'))
