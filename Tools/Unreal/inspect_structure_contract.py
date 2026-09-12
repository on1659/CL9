import unreal,json
from pathlib import Path
out={'engine':unreal.SystemLibrary.get_engine_version(),'segments':[]}
for row in unreal.load_asset('/Game/Data/SegKindTable').get_editor_property('rows'):
    if str(row.get_editor_property('alias')) in ('ceiling','floor'):
        out['segments'].append({'alias':str(row.get_editor_property('alias')),'height_cm':row.get_editor_property('height'),'piece':str(row.get_editor_property('piece'))})
Path(unreal.Paths.project_dir(),'docs/asset/evidence/016-structure-contract.json').write_text(json.dumps(out,indent=2),encoding='utf-8')
print(json.dumps(out))
