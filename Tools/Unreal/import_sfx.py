"""임시 효과음 임포트 — ArtSource/audio/sfx/*.wav → /Game/Sounds/SFX/**/S_* (USoundWave) + 발소리 SoundCue.

정본: docs/goal/game-feel.md §1 · docs/systems/sound.md(에셋 위치·사운드 클래스). WAV 는 Tools/Audio/synth_sfx.py 가 만든다.
**멱등** 스크립트다 — 에셋이 있으면 제자리 재임포트(replace_existing), 없으면 임포트로 생성한다.
`create_asset` 을 기존 경로에 부르면 "오버라이드" 모달로 에디터가 멈추므로, 있음/없음을 먼저 보고 갈린다.

만드는 것:
  1. USoundWave 9개 — 아래 WAVES 표. 전부 `sound_class_object = SC_Effects`(있어야 한다 — 없으면 멈춘다),
     `concurrency_set = {SCC_Effects4}`.
  2. USoundConcurrency `/Game/Sounds/Master/SCC_Effects4` — max_count 4(같은 소리 4 보이스까지). 없으면 생성, 있으면 값만 다시 쓴다.
  3. USoundCue `/Game/Sounds/SFX/Character/S_Footstep` — 발소리 3개를 Random 노드로 묶는다.
     SoundCueFactoryNew.initial_sound_waves 에 웨이브를 여러 개 주면 팩토리가 Random 노드 + WavePlayer 3개를 만들어 잇는다
     (Engine/Source/Editor/AudioEditor/Private/Factories/SoundCueFactoryNew.cpp). 파이썬에 노드 API 가 따로 없어서 이 길뿐이다 —
     이미 있으면 노드를 다시 만들지 않고 클래스·Concurrency 만 다시 쓴다. 노드가 안 생기면 빈 큐를 지우고 FOOTSTEP CUE FAILED 를 찍는다
     (그때는 SoundTable 이 S_Footstep_1/2/3 웨이브 행을 직접 가리킨다 — 보고서 참조).
  SoundTable 행 연결은 이 스크립트가 하지 않는다 — 리드가 에디터에서 한다(매핑 표는 맨 끝 출력).

실행(리드) — 에디터를 켠 채 에디터 Cmd 바에서:
    py "D:/Work/unreal/CL9/Tools/Unreal/import_sfx.py"
또는 콘솔에서:
    py -3 Tools/Unreal/run_python.py Tools/Unreal/import_sfx.py
"""
import sys

import unreal

try:
    sys.stdout.reconfigure(encoding="utf-8")
except (AttributeError, ValueError):
    pass

lib = unreal.EditorAssetLibrary
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

PROJECT_DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()).replace("\\", "/").rstrip("/")
SOURCE_DIR = PROJECT_DIR + "/ArtSource/audio/sfx"

SOUND_CLASS_PATH = "/Game/Sounds/Master/SC_Effects"
CONCURRENCY_DIR = "/Game/Sounds/Master"
CONCURRENCY_NAME = "SCC_Effects4"
CONCURRENCY_PATH = CONCURRENCY_DIR + "/" + CONCURRENCY_NAME
CONCURRENCY_MAX_COUNT = 4

# (WAV 파일, 목적 폴더, 에셋 이름) — 폴더 구분은 docs/systems/sound.md "에셋 위치"(Carry/Monster/Character)
WAVES = [
    ("pickup.wav", "/Game/Sounds/SFX/Carry", "S_Pickup"),
    ("place.wav", "/Game/Sounds/SFX/Carry", "S_Place"),
    ("swing_whoosh.wav", "/Game/Sounds/SFX/Carry", "S_SwingWhoosh"),
    ("hit_pan.wav", "/Game/Sounds/SFX/Monster", "S_HitPan"),
    ("monster_death.wav", "/Game/Sounds/SFX/Monster", "S_MonsterDeath"),
    ("hurt.wav", "/Game/Sounds/SFX/Character", "S_Hurt"),
    ("footstep_1.wav", "/Game/Sounds/SFX/Character", "S_Footstep_1"),
    ("footstep_2.wav", "/Game/Sounds/SFX/Character", "S_Footstep_2"),
    ("footstep_3.wav", "/Game/Sounds/SFX/Character", "S_Footstep_3"),
]
FOOTSTEP_CUE_DIR = "/Game/Sounds/SFX/Character"
FOOTSTEP_CUE_NAME = "S_Footstep"
FOOTSTEP_CUE_PATH = FOOTSTEP_CUE_DIR + "/" + FOOTSTEP_CUE_NAME
FOOTSTEP_WAVE_PATHS = ["/Game/Sounds/SFX/Character/S_Footstep_%d" % i for i in (1, 2, 3)]

# SoundTable 행 키(C++ CL9SoundNames) ↔ 에셋 — 리드가 표에 넣을 매핑. 발소리는 큐가 만들어지면 큐, 아니면 웨이브 1번
TABLE_ROWS = [
    ("Pickup", "/Game/Sounds/SFX/Carry/S_Pickup"),
    ("Place", "/Game/Sounds/SFX/Carry/S_Place"),
    ("Swing", "/Game/Sounds/SFX/Carry/S_SwingWhoosh"),
    ("Hit", "/Game/Sounds/SFX/Monster/S_HitPan"),
    ("Hurt", "/Game/Sounds/SFX/Character/S_Hurt"),
    ("MonsterDeath", "/Game/Sounds/SFX/Monster/S_MonsterDeath"),
]


def log(msg):
    unreal.log("[import-sfx] " + msg)


def ensure_concurrency():
    """SCC_Effects4 — 없으면 생성, 있으면 max_count 만 다시 쓴다."""
    if lib.does_asset_exist(CONCURRENCY_PATH):
        concurrency = unreal.load_asset(CONCURRENCY_PATH)
        assert isinstance(concurrency, unreal.SoundConcurrency), CONCURRENCY_PATH + " 가 SoundConcurrency 가 아니다"
        log("Concurrency 갱신: " + CONCURRENCY_PATH)
    else:
        concurrency = asset_tools.create_asset(CONCURRENCY_NAME, CONCURRENCY_DIR, unreal.SoundConcurrency,
                                               unreal.SoundConcurrencyFactory())
        assert concurrency is not None, "Concurrency 생성 실패: " + CONCURRENCY_PATH
        log("Concurrency 생성: " + CONCURRENCY_PATH)
    # USoundConcurrency 의 설정 구조체 프로퍼티는 `Concurrency`(FSoundConcurrencySettings) 다 — 파이썬 이름 `concurrency`
    # (Engine/Classes/Sound/SoundConcurrency.h 216~222행). `concurrency_settings` 가 아니다.
    settings = concurrency.get_editor_property("concurrency")
    settings.set_editor_property("max_count", CONCURRENCY_MAX_COUNT)
    concurrency.modify()
    concurrency.set_editor_property("concurrency", settings)
    assert lib.save_asset(CONCURRENCY_PATH, False), "Concurrency 저장 실패: " + CONCURRENCY_PATH
    log("검증 — %s: max_count %d" % (CONCURRENCY_PATH, concurrency.get_editor_property("concurrency").get_editor_property("max_count")))
    return concurrency


def apply_sound_settings(sound, sound_class, concurrency):
    """모든 사운드 에셋은 SC_* 클래스 하나를 가져야 설정 슬라이더가 닿는다(sound.md) + 동시 재생 4."""
    sound.modify()
    sound.set_editor_property("sound_class_object", sound_class)
    concurrency_set = unreal.Set(unreal.SoundConcurrency)
    concurrency_set.add(concurrency)
    sound.set_editor_property("concurrency_set", concurrency_set)
    sound.set_editor_property("override_concurrency", False)


def import_wave(file_name, folder, name):
    path = folder + "/" + name
    source = SOURCE_DIR + "/" + file_name
    assert unreal.Paths.file_exists(source), "WAV 가 없다(먼저 py -3 Tools/Audio/synth_sfx.py): " + source
    exists = lib.does_asset_exist(path)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source)
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", False)
    task.set_editor_property("replace_existing", exists)
    task.set_editor_property("replace_existing_settings", False)
    asset_tools.import_asset_tasks([task])
    sound = unreal.load_asset(path)
    assert isinstance(sound, unreal.SoundWave), "임포트 실패(%s): %s" % (path, list(task.get_editor_property("imported_object_paths")))
    log(("재임포트: " if exists else "임포트: ") + path + "  <-  " + file_name)
    return path, sound


def ensure_footstep_cue(sound_class, concurrency):
    """발소리 3개 → Random 노드 하나로 묶은 SoundCue. 성공하면 경로, 실패하면 None."""
    if lib.does_asset_exist(FOOTSTEP_CUE_PATH):
        cue = unreal.load_asset(FOOTSTEP_CUE_PATH)
        assert isinstance(cue, unreal.SoundCue), FOOTSTEP_CUE_PATH + " 가 SoundCue 가 아니다"
        log("발소리 큐 갱신(노드는 그대로): " + FOOTSTEP_CUE_PATH)
        created = False
    else:
        waves = [unreal.load_asset(p) for p in FOOTSTEP_WAVE_PATHS]
        assert all(isinstance(w, unreal.SoundWave) for w in waves), "발소리 웨이브가 없다: " + str(FOOTSTEP_WAVE_PATHS)
        factory = unreal.SoundCueFactoryNew()
        try:
            factory.set_editor_property("initial_sound_waves", waves)
        except Exception as error:  # 파이썬이 TArray<TWeakObjectPtr> 를 못 받는 경우 — 빈 큐를 만들지 않는다
            log("발소리 큐 생성 불가(initial_sound_waves 대입 실패): %s" % error)
            return None
        cue = asset_tools.create_asset(FOOTSTEP_CUE_NAME, FOOTSTEP_CUE_DIR, unreal.SoundCue, factory)
        assert cue is not None, "발소리 큐 생성 실패: " + FOOTSTEP_CUE_PATH
        log("발소리 큐 생성: " + FOOTSTEP_CUE_PATH)
        created = True

    first = cue.get_editor_property("first_node")
    if first is None or not isinstance(first, unreal.SoundNodeRandom):
        log("발소리 큐 첫 노드가 Random 이 아니다: %s" % first)
        if created:
            lib.delete_asset(FOOTSTEP_CUE_PATH)
            log("빈 큐를 지웠다: " + FOOTSTEP_CUE_PATH)
        return None
    children = list(first.get_editor_property("child_nodes"))
    linked = []
    for child in children:
        if isinstance(child, unreal.SoundNodeWavePlayer):
            wave = child.get_editor_property("sound_wave_asset_ptr")   # TSoftObjectPtr — 파이썬은 로드된 객체로 준다
            linked.append(str(wave.get_path_name()) if hasattr(wave, "get_path_name") else str(wave))
    if len(linked) != len(FOOTSTEP_WAVE_PATHS):
        log("발소리 큐 자식 노드가 %d개(기대 %d): %s" % (len(linked), len(FOOTSTEP_WAVE_PATHS), linked))
        if created:
            lib.delete_asset(FOOTSTEP_CUE_PATH)
            log("불완전한 큐를 지웠다: " + FOOTSTEP_CUE_PATH)
        return None

    apply_sound_settings(cue, sound_class, concurrency)
    assert lib.save_asset(FOOTSTEP_CUE_PATH, False), "발소리 큐 저장 실패: " + FOOTSTEP_CUE_PATH
    log("발소리 큐 Random 자식: " + ", ".join(linked))
    return FOOTSTEP_CUE_PATH


def verify(paths, sound_class, concurrency):
    for path in paths:
        sound = unreal.load_asset(path)
        assert sound is not None, "에셋이 없다: " + path
        assert sound.get_editor_property("sound_class_object") == sound_class, path + " 의 SoundClass 가 SC_Effects 가 아니다"
        members = list(sound.get_editor_property("concurrency_set"))
        assert concurrency in members, path + " 의 ConcurrencySet 에 SCC_Effects4 가 없다"
        duration = sound.get_editor_property("duration") if isinstance(sound, unreal.SoundWave) else -1.0
        log("검증 — %s: 클래스 %s · Concurrency %d개 · 길이 %.3f초" % (
            path, sound.get_editor_property("sound_class_object").get_name(), len(members), duration))


def main():
    assert unreal.SystemLibrary.get_engine_version().startswith("5.8.")
    assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor(), "PIE 중에는 돌리지 않는다"
    assert lib.does_asset_exist(SOUND_CLASS_PATH), "사운드 클래스가 없다: " + SOUND_CLASS_PATH
    sound_class = unreal.load_asset(SOUND_CLASS_PATH)
    assert isinstance(sound_class, unreal.SoundClass), SOUND_CLASS_PATH + " 가 SoundClass 가 아니다"

    concurrency = ensure_concurrency()
    created = [CONCURRENCY_PATH]
    for file_name, folder, name in WAVES:
        path, sound = import_wave(file_name, folder, name)
        apply_sound_settings(sound, sound_class, concurrency)
        assert lib.save_asset(path, False), "저장 실패: " + path
        created.append(path)

    cue_path = ensure_footstep_cue(sound_class, concurrency)
    if cue_path:
        created.append(cue_path)
    verify(created[1:], sound_class, concurrency)

    print("CREATED ASSETS:")
    for path in created:
        print("  " + path)
    print("SOUNDTABLE ROWS (lead applies):")
    rows = list(TABLE_ROWS) + [("Footstep", cue_path if cue_path else FOOTSTEP_WAVE_PATHS[0])]
    for row, path in rows:
        print("  %-13s -> %s" % (row, path))
    if cue_path:
        print("FOOTSTEP CUE OK: " + cue_path)
    else:
        print("FOOTSTEP CUE FAILED - use S_Footstep_1/2/3 waves directly (see report)")
    log("완료 — 웨이브 %d개 · Concurrency 1개 · 큐 %d개" % (len(WAVES), 1 if cue_path else 0))


main()
