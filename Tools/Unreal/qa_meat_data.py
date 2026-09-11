"""Temporary real-row connection for meat QA; MODE=restore reverses only this test's edits."""
import unreal,json,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();backup=root/'docs/asset/evidence/008-meat-data-backup.json'
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
table=unreal.load_asset('/Game/Data/DA_ItemTable');rows=table.get_editor_property('rows')
dirty=[p.get_name() for p in unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages()]
assert '/Game/Data/DA_ItemTable' not in dirty,'Preserve user unsaved table'
if globals().get('MODE')=='restore':
    records=json.loads(backup.read_text())['rows']
    for r in records:
        row=next(x for x in rows if x.get_path_name()==r['row'])
        current=row.get_editor_property('mesh').get_path_name()
        assert current==r['new'],'Table changed after test'
        row.set_editor_property('mesh',unreal.load_asset(r['old']))
    unreal.EditorAssetLibrary.save_loaded_asset(table)
    print('Restored three original meat mesh references')
else:
    assert not backup.exists(),'Existing test backup: inspect before preparing again'
    assert not unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages(),'Preserve unsaved map'
    records=[]
    for row in rows:
        if str(row.get_editor_property('alias'))!='meat':continue
        sub=str(row.get_editor_property('sub_alias'));state={'None':'Raw','cooked':'Cooked','burnt':'Burnt'}[sub]
        old=row.get_editor_property('mesh').get_path_name()
        path='/Game/CL9Art/Items/MeatStates/SM_Meat_'+state+'_A.SM_Meat_'+state+'_A'
        assert old.startswith('/Engine/BasicShapes/'),'Unexpected current art; inspect before replacing'
        records.append({'row':row.get_path_name(),'old':old,'new':path})
    assert len(records)==3
    backup.write_text(json.dumps({'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'rows':records},indent=2),encoding='utf-8')
    for r in records:
        row=next(x for x in rows if x.get_path_name()==r['row']);row.set_editor_property('mesh',unreal.load_asset(r['new']))
    unreal.EditorAssetLibrary.save_loaded_asset(table)
    print('Connected three meat rows temporarily; restore after test')
