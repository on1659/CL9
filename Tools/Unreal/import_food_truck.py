for part in ['body','steps']:
 SOURCE_REL='ArtSource/map/food_truck/'+part+'/export';DESTINATION='/Game/Maps/FoodTruck/'+part.title();EVIDENCE='023-'+part+'-imports.json'
 exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_portable_lights.py',encoding='utf-8').read(),'import_portable_lights.py','exec'))
