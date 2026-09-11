for part in ['wall250','wall125','tread']:
    SOURCE_REL='ArtSource/map/henhouse_access_parts/'+part+'/export'
    DESTINATION='/Game/CL9Art/Maps/HenhouseAccess/'+part.title()
    EVIDENCE='017-'+part+'-imports.json'
    exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_portable_lights.py',encoding='utf-8').read(),'import_portable_lights.py','exec'))
