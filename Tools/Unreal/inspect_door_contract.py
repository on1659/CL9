import unreal,json
from pathlib import Path
out={'engine':unreal.SystemLibrary.get_engine_version(),'tables':{}}
for table_name,wanted in [('DA_PieceTable','door'),('DA_SegKindTable','flapIn')]:
    table=unreal.load_asset('/Game/Data/'+table_name);rows=[]
    for row in table.get_editor_property('rows'):
        if str(row.get_editor_property('alias'))!=wanted:continue
        entry={'path':row.get_path_name(),'alias':wanted}
        if wanted=='door':
            entry['variants']=[]
            for mesh in row.get_editor_property('variants'):
                b=mesh.get_bounding_box()
                entry['variants'].append({'path':mesh.get_path_name(),'dimensions_cm':list((b.max-b.min).to_tuple())})
        else:entry['height']=row.get_editor_property('height')
        rows.append(entry)
    out['tables'][table_name]=rows
Path(unreal.Paths.project_dir(),'docs/asset/evidence/014-door-contract.json').write_text(json.dumps(out,indent=2),encoding='utf-8')
print(json.dumps(out))
