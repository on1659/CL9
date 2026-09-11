import unreal,json
from pathlib import Path
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
root=Path(unreal.Paths.project_dir(),'docs/asset/evidence')
p=root/'023-food-truck-pie.json';old=root/'023-food-truck-pie-before-clearance-fit.json';assert not old.exists()
old.write_text(p.read_text(),encoding='utf-8');p.unlink()
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 if a.get_actor_label()=='QA023_Kitchen':a.set_actor_location(unreal.Vector(19953,-33.75,1064),False,False)
p=root/'023-food-truck-fixtures.json';r=json.loads(p.read_text())
for row in r['fixtures']:
 if row['label']=='QA023_Kitchen':row['position']=[19953,-33.75,1064]
p.write_text(json.dumps(r,indent=2),encoding='utf-8');print('Updated only owned kitchen QA position; preserved initial failed evidence.')
