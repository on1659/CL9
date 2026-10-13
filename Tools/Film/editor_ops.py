"""/cl9film 에디터 쪽 준비·복구 — Tools/Unreal/run_python.py 로 실행한다. 매개변수는 Saved/Film/ops_params.json.

{"op": "setup", "level": "/Game/Maps/Lvl_Farm", "players": 1, "rules": {"night_length": 60}, "restore": "<run>/restore.json"}
  PIE 가 돌고 있거나 저장 안 한 맵이 있으면 멈춘다(남의 작업을 덮지 않는다). 지금 레벨·PIE 인원·바꿀 규칙 셀의 원래 값을
  restore 파일에 적고, 레벨을 열고, 인원과 셀을 바꾼다.
{"op": "restore", "restore": "<run>/restore.json"}
  셀을 원래 값으로 되돌려 CL9Rules 를 저장하고(원래 더럽지 않았을 때만), PIE 인원·레벨을 되돌린다.
결과는 LogPython 의 "CL9FILM-OPS" 줄과 restore 파일.
"""
import json
import unreal

P = json.load(open('C:/Work/CL9/Saved/Film/ops_params.json', encoding='utf-8'))
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
settings = unreal.load_object(None, '/Script/UnrealEd.Default__LevelEditorPlaySettings')
rules = unreal.load_asset('/Game/Data/CL9Rules')


def dirty_names(maps):
    pk = unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages() if maps else unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages()
    return [p.get_name() for p in pk]


if P['op'] == 'setup':
    if ues.get_game_world() is not None:
        raise SystemExit('CL9FILM-OPS ABORT PIE is running')
    dirty = dirty_names(True)
    if dirty:
        raise SystemExit('CL9FILM-OPS ABORT unsaved maps %s' % dirty)
    before = {
        'level': ues.get_editor_world().get_outermost().get_name(),
        'players': settings.get_editor_property('PlayNumberOfClients'),
        'window': [settings.get_editor_property('NewWindowWidth'), settings.get_editor_property('NewWindowHeight')],
        'rules': {k: rules.get_editor_property(k) for k in P.get('rules', {})},
        'rules_dirty': '/Game/Data/CL9Rules' in dirty_names(False),
    }
    json.dump(before, open(P['restore'], 'w', encoding='utf-8'), ensure_ascii=False, default=str)
    if before['level'] != P['level']:
        les.load_level(P['level'])
    settings.set_editor_property('PlayNumberOfClients', int(P.get('players', 1)))
    # 녹화 창은 1280x720 — 크면 PrintWindow 가 느려 프레임이 준다(2312x1346 에서 13 fps, 1288x770 에서 20 fps)
    settings.set_editor_property('NewWindowWidth', 1280)
    settings.set_editor_property('NewWindowHeight', 720)
    for k, v in P.get('rules', {}).items():
        rules.set_editor_property(k, v)
    print('CL9FILM-OPS SETUP level %s players %s rules %s (was %s)' % (P['level'], P.get('players', 1), P.get('rules', {}), before))

elif P['op'] == 'restore':
    before = json.load(open(P['restore'], encoding='utf-8'))
    if ues.get_game_world() is not None:
        raise SystemExit('CL9FILM-OPS ABORT PIE still running')
    for k, v in before['rules'].items():
        rules.set_editor_property(k, v)
    if before['rules'] and not before['rules_dirty']:
        unreal.EditorAssetLibrary.save_asset('/Game/Data/CL9Rules', True)
    settings.set_editor_property('PlayNumberOfClients', int(before['players']))
    if before.get('window'):
        settings.set_editor_property('NewWindowWidth', int(before['window'][0]))
        settings.set_editor_property('NewWindowHeight', int(before['window'][1]))
    if ues.get_editor_world().get_outermost().get_name() != before['level'] and not dirty_names(True):
        les.load_level(before['level'])
    print('CL9FILM-OPS RESTORED %s' % before)
