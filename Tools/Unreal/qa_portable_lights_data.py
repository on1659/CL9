"""Temporarily connect real light rows, or restore the recorded pre-test references."""
import unreal,json,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();backup=root/'docs/asset/evidence'/globals().get('BACKUP','009-light-data-backup.json')
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
assert not unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()
assert '/Game/Data/ItemTable' not in [p.get_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages()]
table=unreal.load_asset('/Game/Data/ItemTable');rows=table.get_editor_property('rows')
if globals().get('MODE')=='restore':
    b=json.loads(backup.read_text())
    for r in b['rows']:
        row=next(x for x in rows if x.get_path_name()==r['row'])
        assert row.get_editor_property('mesh').get_path_name()==r['new']
        row.set_editor_property('mesh',unreal.load_asset(r['old']))
    unreal.EditorAssetLibrary.save_loaded_asset(table);b['restored']=True;backup.write_text(json.dumps(b,indent=2),encoding='utf-8')
    print('Restored recorded item mesh references')
else:
    assert not backup.exists(),'Inspect existing test before preparing another'
    records=[]
    for alias,name in globals().get('ASSETS',[('flashlight','SM_Flashlight_A'),('headlamp','SM_Headlamp_A')]):
        row=next(x for x in rows if str(x.get_editor_property('alias'))==alias)
        old=row.get_editor_property('mesh').get_path_name();assert old.startswith('/Engine/BasicShapes/')
        records.append({'row':row.get_path_name(),'old':old,'new':globals().get('DESTINATION','/Game/CL9Art/Items/PortableLights')+'/'+name+'.'+name})
    backup.write_text(json.dumps({'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'restored':False,'rows':records},indent=2),encoding='utf-8')
    for r in records:next(x for x in rows if x.get_path_name()==r['row']).set_editor_property('mesh',unreal.load_asset(r['new']))
    unreal.EditorAssetLibrary.save_loaded_asset(table);print('Connected '+str(len(records))+' item rows for QA')
