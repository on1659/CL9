import unreal,json
from pathlib import Path
rows=unreal.load_asset('/Game/Data/ModuleTable').get_editor_property('rows')
out={'module_heights_cm':sorted(set(r.get_editor_property('height') for r in rows)),
     'stairs':[{'alias':str(r.get_editor_property('alias')),'size':str(r.get_editor_property('size')),'height':r.get_editor_property('height')} for r in rows if 'Stair' in str(r.get_editor_property('type')) or 'stair' in str(r.get_editor_property('alias'))]}
Path(unreal.Paths.project_dir(),'docs/asset/evidence/017-step-contract.json').write_text(json.dumps(out,indent=2),encoding='utf-8')
print(json.dumps(out))
