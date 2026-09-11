for part in ['beam','pillar','ceiling']:
    SOURCE_REL='ArtSource/map/henhouse_structure/'+part+'/export'
    DESTINATION='/Game/CL9Art/Maps/HenhouseStructure/'+part.title()
    EVIDENCE='016-'+part+'-imports.json'
    exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_portable_lights.py',encoding='utf-8').read(),'import_portable_lights.py','exec'))
