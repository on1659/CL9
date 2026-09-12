"""Read current piece and segment contracts; no asset mutations."""
import unreal,json
out={'engine':unreal.SystemLibrary.get_engine_version(),'tables':{}}
for table_name in ['PieceTable','SegKindTable']:
    table=unreal.load_asset('/Game/Data/'+table_name)
    rows=[]
    for row in table.get_editor_property('rows'):
        alias=str(row.get_editor_property('alias'))
        if alias not in ['fence','wall']:continue
        entry={'alias':alias,'path':row.get_path_name()}
        if table_name=='PieceTable':
            entry['sub_alias']=str(row.get_editor_property('sub_alias'));entry['variants']=[]
            for mesh in row.get_editor_property('variants'):
                if mesh:
                    bounds=mesh.get_bounding_box()
                    entry['variants'].append({'path':mesh.get_path_name(),'dimensions_cm':list((bounds.max-bounds.min).to_tuple())})
        else:
            entry['height']=row.get_editor_property('height')
        rows.append(entry)
    out['tables'][table_name]=rows
print(json.dumps(out))
