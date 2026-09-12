ALLOW_OWNED_REIMPORT=True
for part in ['sign_feed','sign_kitchen']:
    SOURCE_REL='ArtSource/map/henhouse_yard/'+part+'/export'
    DESTINATION='/Game/Maps/Henhouse/Yard/'+part.title().replace('_','')
    EVIDENCE='018-'+part+'-imports.json'
    exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_portable_lights.py',encoding='utf-8').read(),'import_portable_lights.py','exec'))
