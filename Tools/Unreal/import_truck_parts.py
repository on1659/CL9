for part in ['pad','panel']:
 SOURCE_REL='ArtSource/map/truck_parts/'+part+'/export';DESTINATION='/Game/Items/Truck'+part.title();EVIDENCE='022-'+part+'-imports.json'
 exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_portable_lights.py',encoding='utf-8').read(),'import_portable_lights.py','exec'))
