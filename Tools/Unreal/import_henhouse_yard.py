for part in ['ground','sign_exit','sign_feed','sign_kitchen','sign_coop']:
    SOURCE_REL='ArtSource/map/henhouse_yard/'+part+'/export'
    DESTINATION='/Game/CL9Art/Maps/HenhouseYard/'+part.title().replace('_','')
    EVIDENCE='018-'+part+'-imports.json'
    exec(compile(open('D:/Work/unreal/CL9/Tools/Unreal/import_portable_lights.py',encoding='utf-8').read(),'import_portable_lights.py','exec'))
