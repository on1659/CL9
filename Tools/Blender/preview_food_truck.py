for part,scene,source,names,ortho,focus,camera,energy in [
 ('body','CL9_FOOD_TRUCK','food_truck.blend',['SM_FoodTruck_A'],9,(0,0,1.55),(7,8,5.7),1000),
 ('steps','CL9_FOOD_TRUCK_STEPS','steps.blend',['SM_FoodTruck_Steps_A'],2.6,(0,0,.30),(-2,-2,2),200)]:
 ASSET_ROOT='D:/Work/unreal/CL9/ArtSource/map/food_truck/'+part;KIT_SCENE=scene;SOURCE_BLEND=source;OBJECT_NAMES=names
 ORTHO=ortho;FOCUS=focus;CAMERA_POSITION=camera;FLASHLIGHT_ENERGY=energy
 exec(compile(open('D:/Work/unreal/CL9/Tools/Blender/preview_static_pair.py',encoding='utf-8').read(),'preview_static_pair.py','exec'))
