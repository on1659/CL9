"""타격감·피격감 에셋 생성 — /Game/Effects/M_CL9_HitFlash · CS_HitShake · CS_HurtShake + ShakeTable 의 Hit·Hurt 행.

정본: docs/goal/hit-feedback.md "Data / assets". 한 번 돌리면 아래 셋을 만들고 저장하는 **멱등** 스크립트다
(이미 있으면 값만 다시 쓴다 — 에셋을 지우고 새로 만들지 않으므로 참조가 끊기지 않는다).

1. `/Game/Effects/M_CL9_HitFlash` — 오버레이(`SetOverlayMaterial`) 전용 재질. Surface·Translucent·Unlit·TwoSided,
   ScalarParameter `Opacity`(기본 0.6) → Opacity, VectorParameter `Color`(기본 (1, 0.05, 0.05)) → Emissive Color.
   깊이 테스트는 기본값 그대로 둔다(메시 위에 덧그려지되 벽 뒤로는 비치지 않는다).
2. `/Game/Effects/CS_HitShake` · `/Game/Effects/CS_HurtShake` — `/Game/Effects/CS_StaminaShake` 를 복제한 뒤
   CDO 의 흔들림 값만 덮어쓴다. CS_StaminaShake 는 `UDefaultCameraShakeBase` 를 상속한 블루프린트이고,
   그 `RootShakePattern` 은 기본 서브오브젝트인 `UPerlinNoiseCameraShakePattern` 이다 — 그래서 `oscillation_duration`
   (ULegacyCameraShake 의 것) 이 아니라 `duration` / `blend_in_time` / `blend_out_time` / `pitch.amplitude` … 를 쓴다.
3. `/Game/Data/ShakeTable` 의 `Rows` 에 `Hit`·`Hurt` 행(`FCL9ShakeInfo`) 추가. 기존 `Stamina` 행은 건드리지 않는다.

흔들림 수치는 balance 가 확정한 값이다(2026-09-15) — 바꿀 일이 생기면 에디터에서 손으로 만지지 말고
아래 HIT_SHAKE·HURT_SHAKE 상수를 고치고 다시 돌린다. 손으로 만진 값은 다음 실행에서 덮어쓰인다.
실행 로그 맨 앞에 비교 기준으로 원본 CS_StaminaShake CDO 의 실측값(Duration·Pitch/Roll/Yaw 진폭·주파수·배속)을 찍는다.

실행(리드) — 에디터를 켠 채 에디터 Cmd 바에서:
    py "D:/Work/unreal/CL9/Tools/Unreal/create_hit_feedback_assets.py"
또는 콘솔에서 run_python 경유:
    py -3 Tools/Unreal/run_python.py Tools/Unreal/create_hit_feedback_assets.py
"""
import unreal

EFFECTS = "/Game/Effects"
SOURCE_SHAKE = EFFECTS + "/CS_StaminaShake"
FLASH_PATH = EFFECTS + "/M_CL9_HitFlash"
SHAKE_TABLE = "/Game/Data/ShakeTable"

# 재질 파라미터 기본값 — 실제 세기는 게임 코드가 MID 로 덮어쓰지 않고 이 기본값을 그대로 쓴다
FLASH_OPACITY = 0.6
FLASH_COLOR = (1.0, 0.05, 0.05, 1.0)

# 흔들림 값 — balance 확정 (2026-09-15, 리드 cl9-96 전달). 회전 진폭은 도(°), 위치 진폭은 cm,
# 주파수는 펄린 노이즈의 진행 배속이다. 패턴은 복제한 그대로(Perlin) 두고 값만 바꾼다.
# 타격 = 짧고 날카로운 한 번. 블렌드 인 0 이라 맞는 순간 바로 튄다.
HIT_SHAKE = {
    "duration": 0.15,
    "blend_in_time": 0.0,
    "blend_out_time": 0.10,
    "location": {"x": (0.0, 1.0), "y": (0.0, 1.0), "z": (0.0, 1.0)},  # 타격은 회전만
    # Pitch 2.2 는 balance 권고 1.5 에 펄린 위상 손실 보정을 더한 값이다 (상한 2.5 — 넘기지 않는다)
    "rotation": {"pitch": (2.2, 20.0), "yaw": (0.5, 18.0), "roll": (1.0, 16.0)},
    "fov": (0.0, 1.0),
}
# 피격 = 묵직하게 오래. 진폭을 키우고 주파수를 낮춰 굼뜨게, 블렌드 아웃 0.30 으로 천천히 잦아든다.
HURT_SHAKE = {
    "duration": 0.50,
    "blend_in_time": 0.0,
    "blend_out_time": 0.30,
    "location": {"x": (0.0, 1.0), "y": (0.0, 1.0), "z": (0.0, 1.0)},  # 피격도 회전만 — 위치는 멀미가 난다
    # 피격은 Roll 이 주역이다 (몸이 옆으로 휘청) — 타격의 Pitch 주역과 감각이 갈린다
    "rotation": {"pitch": (2.5, 7.0), "yaw": (1.5, 6.0), "roll": (3.5, 5.0)},
    "fov": (0.0, 1.0),
}
SHAKES = [("CS_HitShake", HIT_SHAKE), ("CS_HurtShake", HURT_SHAKE)]
# 표 행 이름은 C++ `CL9ShakeNames::Hit`·`Hurt` 와 같아야 한다 (Source/CL9/Data/CL9ShakeTable.h)
TABLE_ROWS = [("Hit", "CS_HitShake"), ("Hurt", "CS_HurtShake")]

lib = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary


def log(msg):
    unreal.log("[hit-feedback] " + msg)


def find_parameter(material, expression_class, parameter_name):
    """이미 있는 파라미터 노드를 이름으로 찾는다 — 멱등 실행에서 노드가 겹겹이 쌓이지 않게."""
    for expression in mel.get_material_expressions(material):
        if isinstance(expression, expression_class) and str(expression.get_editor_property("parameter_name")) == parameter_name:
            return expression
    return None


def build_flash_material():
    if lib.does_asset_exist(FLASH_PATH):
        material = unreal.load_asset(FLASH_PATH)
        assert isinstance(material, unreal.Material), FLASH_PATH + " 가 Material 이 아니다"
        log("재질 갱신: " + FLASH_PATH)
    else:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "M_CL9_HitFlash", EFFECTS, unreal.Material, unreal.MaterialFactoryNew())
        assert material is not None, "재질 생성 실패: " + FLASH_PATH
        log("재질 생성: " + FLASH_PATH)

    # 설정을 먼저 — Opacity 입력은 반투명일 때만 살아 있다
    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_SURFACE)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("two_sided", True)
    # 오버레이가 붙는 곳은 전부 스켈레탈 메시(몬스터·플레이어 몸·1인칭 팔)다. 에디터는 첫 사용에서
    # 이 플래그를 자동으로 켜지만, 그러면 저장되지 않은 채 남아 패키징된 빌드에서 기본 재질로 떨어진다.
    mel.set_base_material_usage(material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH, True)

    opacity = find_parameter(material, unreal.MaterialExpressionScalarParameter, "Opacity")
    if opacity is None:
        opacity = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -300, 200)
        opacity.set_editor_property("parameter_name", "Opacity")
    opacity.set_editor_property("default_value", FLASH_OPACITY)
    assert mel.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY), "Opacity 연결 실패"

    color = find_parameter(material, unreal.MaterialExpressionVectorParameter, "Color")
    if color is None:
        color = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -300, 0)
        color.set_editor_property("parameter_name", "Color")
    color.set_editor_property("default_value", unreal.LinearColor(*FLASH_COLOR))
    assert mel.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR), "Emissive Color 연결 실패"

    mel.recompile_material(material)
    assert lib.save_asset(FLASH_PATH, False), "재질 저장 실패: " + FLASH_PATH
    return material


def shaker(amplitude, frequency):
    value = unreal.PerlinNoiseShaker()
    value.set_editor_property("amplitude", amplitude)
    value.set_editor_property("frequency", frequency)
    return value


def pattern_of(asset_path, name):
    """블루프린트 생성 클래스의 CDO 에서 RootShakePattern(펄린 노이즈 패턴) 인스턴스를 꺼낸다.

    CDO 는 **모듈 함수** `unreal.get_default_object(cls)` 로만 얻는다 — `unreal.Class` 에는
    `get_default_object()` 메서드가 없고, 클래스 객체에 대고 부르면 CDO 가 아니라 클래스가 그대로
    돌아와 `root_shake_pattern` 을 못 찾는다("Failed to find property ... on 'BlueprintGeneratedClass'").
    """
    generated = unreal.load_class(None, asset_path + "." + name + "_C")
    assert generated is not None, "생성 클래스를 못 찾았다: " + asset_path
    cdo = unreal.get_default_object(generated)
    assert cdo is not None, "CDO 를 못 얻었다: " + asset_path
    pattern = cdo.get_editor_property("root_shake_pattern")
    assert pattern is not None, name + " 에 RootShakePattern 이 없다"
    return cdo, pattern


def axes(pattern, names, unit):
    return ", ".join("%s %.3f%s@%.2f" % (name, pattern.get_editor_property(name).get_editor_property("amplitude"), unit,
                                         pattern.get_editor_property(name).get_editor_property("frequency"))
                     for name in names)


def describe(pattern):
    # 축별 진폭에 배속이 곱해진 것이 실제 세기다 — 배속까지 찍어야 셰이크끼리 비교가 된다
    return "길이 %.3f초 (블렌드 인 %.3f / 아웃 %.3f) · 회전 [%s] ×진폭 %.3f ×주파수 %.3f · 위치 [%s] ×진폭 %.3f ×주파수 %.3f · FOV %.3f@%.2f" % (
        pattern.get_editor_property("duration"), pattern.get_editor_property("blend_in_time"),
        pattern.get_editor_property("blend_out_time"),
        axes(pattern, ("pitch", "roll", "yaw"), "도"),
        pattern.get_editor_property("rotation_amplitude_multiplier"), pattern.get_editor_property("rotation_frequency_multiplier"),
        axes(pattern, ("x", "y", "z"), "cm"),
        pattern.get_editor_property("location_amplitude_multiplier"), pattern.get_editor_property("location_frequency_multiplier"),
        pattern.get_editor_property("fov").get_editor_property("amplitude"),
        pattern.get_editor_property("fov").get_editor_property("frequency"))


def build_shake(name, values):
    path = EFFECTS + "/" + name
    if lib.does_asset_exist(path):
        log("셰이크 갱신: " + path)
    else:
        assert lib.duplicate_asset(SOURCE_SHAKE, path) is not None, "복제 실패: %s → %s" % (SOURCE_SHAKE, path)
        log("셰이크 생성(%s 복제): %s" % (SOURCE_SHAKE, path))

    cdo, pattern = pattern_of(path, name)
    cdo.modify()
    pattern.modify()
    pattern.set_editor_property("duration", values["duration"])
    pattern.set_editor_property("blend_in_time", values["blend_in_time"])
    pattern.set_editor_property("blend_out_time", values["blend_out_time"])
    # 배속은 1 로 고정하고 축별 진폭·주파수만 쓴다 — 읽을 값이 한 군데에만 있게.
    # 이 네 줄은 필수다: 복제 원본 CS_StaminaShake 의 회전 배속이 **0** 이라(실측 2026-09-15,
    # 스태미나는 위치 z 흔들림만 쓰는 지속형) 그대로 두면 아래 회전 진폭이 전부 0 이 곱해져 죽는다.
    pattern.set_editor_property("location_amplitude_multiplier", 1.0)
    pattern.set_editor_property("location_frequency_multiplier", 1.0)
    pattern.set_editor_property("rotation_amplitude_multiplier", 1.0)
    pattern.set_editor_property("rotation_frequency_multiplier", 1.0)
    for axis, pair in values["location"].items():
        pattern.set_editor_property(axis, shaker(*pair))
    for axis, pair in values["rotation"].items():
        pattern.set_editor_property(axis, shaker(*pair))
    pattern.set_editor_property("fov", shaker(*values["fov"]))
    assert lib.save_asset(path, False), "셰이크 저장 실패: " + path
    return path


def add_table_rows():
    table = unreal.load_asset(SHAKE_TABLE)
    assert table is not None, "셰이크 표가 없다: " + SHAKE_TABLE
    current = table.get_editor_property("rows")
    rows = {}
    for key, value in current.items():
        rows[str(key)] = value
    before = sorted(rows.keys())
    for row_name, shake_name in TABLE_ROWS:
        info = rows.get(row_name)
        if info is None:
            template = rows.get("Stamina")
            # 기존 행을 복사해 FCL9ShakeInfo 인스턴스를 얻는다 — 표가 비어 있을 때만 타입 이름을 직접 쓴다
            info = template.copy() if template is not None else unreal.CL9ShakeInfo()
        # TSoftClassPtr 열에는 경로(SoftClassPath)가 아니라 **로드된 클래스 객체**를 넣는다 —
        # SoftClassPath 를 넣으면 "Cannot nativize 'SoftClassPath' as 'Class'" 로 죽는다
        shake_class = unreal.load_class(None, "%s/%s.%s_C" % (EFFECTS, shake_name, shake_name))
        assert shake_class is not None, "셰이크 클래스를 못 찾았다: " + shake_name
        info.set_editor_property("shake", shake_class)
        # StartAt 은 건드리지 않는다 — 복제 원본(Stamina 행)의 값이 그대로 남는다.
        # 원샷 행에서 StartAt 은 게이지 문턱이라 뜻이 없고 코드가 읽지 않는다(CL9ShakeTable.h 주석).
        # 파이썬이 이 구조체에서 'start_at' 을 못 찾기도 한다 — 읽지 않는 값이라 대입 자체를 뺐다.
        rows[row_name] = info
    table.modify()
    table.set_editor_property("rows", rows)
    assert lib.save_asset(SHAKE_TABLE, False), "셰이크 표 저장 실패: " + SHAKE_TABLE
    log("셰이크 표 행: %s → %s" % (before, sorted(rows.keys())))


def verify(shake_paths):
    material = unreal.load_asset(FLASH_PATH)
    log("검증 — 재질 %s: 도메인 %s · 블렌드 %s · 셰이딩 %s · 양면 %s · 스켈레탈 사용 %s · 노드 %d개" % (
        FLASH_PATH, material.get_editor_property("material_domain"), material.get_editor_property("blend_mode"),
        material.get_editor_property("shading_model"), material.get_editor_property("two_sided"),
        mel.has_material_usage(material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH),
        mel.get_num_material_expressions(material)))
    log("검증 — 재질 파라미터: Opacity %.3f · Color %s" % (
        mel.get_material_default_scalar_parameter_value(material, "Opacity"),
        mel.get_material_default_vector_parameter_value(material, "Color")))
    for path in shake_paths:
        name = path.rsplit("/", 1)[1]
        _, pattern = pattern_of(path, name)
        log("검증 — 셰이크 %s: %s" % (path, describe(pattern)))
    table = unreal.load_asset(SHAKE_TABLE)
    rows = table.get_editor_property("rows")
    log("검증 — 셰이크 표 %s: 행 %d개" % (SHAKE_TABLE, len(rows)))
    # StartAt 은 찍지 않는다 — 원샷 행에서 읽지 않는 값이고, 파이썬이 이 구조체에서 못 찾는 경우가 있다
    for key, value in rows.items():
        log("검증 —   %s → %s" % (key, value.get_editor_property("shake")))
    missing = [p for p in [FLASH_PATH] + list(shake_paths) if not lib.does_asset_exist(p)]
    assert not missing, "에셋이 없다: " + str(missing)
    names = [str(key) for key in rows.keys()]
    absent = [row_name for row_name, _ in TABLE_ROWS if row_name not in names]
    assert not absent, "표 행이 없다: " + str(absent)


def main():
    assert unreal.SystemLibrary.get_engine_version().startswith("5.8.")
    assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor(), "PIE 중에는 돌리지 않는다"
    assert lib.does_asset_exist(SOURCE_SHAKE), "복제 원본이 없다: " + SOURCE_SHAKE
    # 원본 값을 먼저 찍어 둔다 — balance 가 스태미나 < Hit < Hurt 순서를 확정하는 데 쓰는 실측값이다.
    # 이 두 줄을 그대로 보고서에 옮긴다.
    source_pattern = pattern_of(SOURCE_SHAKE, "CS_StaminaShake")[1]
    log("실측(balance 요청) 원본 %s: %s" % (SOURCE_SHAKE, describe(source_pattern)))
    log("실측(balance 요청) 요약: Duration %.3f초 · Pitch %.3f도@%.2f · Roll %.3f도@%.2f · Yaw %.3f도@%.2f (회전 배속 진폭 %.3f / 주파수 %.3f)" % (
        source_pattern.get_editor_property("duration"),
        source_pattern.get_editor_property("pitch").get_editor_property("amplitude"),
        source_pattern.get_editor_property("pitch").get_editor_property("frequency"),
        source_pattern.get_editor_property("roll").get_editor_property("amplitude"),
        source_pattern.get_editor_property("roll").get_editor_property("frequency"),
        source_pattern.get_editor_property("yaw").get_editor_property("amplitude"),
        source_pattern.get_editor_property("yaw").get_editor_property("frequency"),
        source_pattern.get_editor_property("rotation_amplitude_multiplier"),
        source_pattern.get_editor_property("rotation_frequency_multiplier")))
    build_flash_material()
    shake_paths = [build_shake(name, values) for name, values in SHAKES]
    add_table_rows()
    verify(shake_paths)
    log("완료 — 재질 1개 · 셰이크 2개 · 표 행 2개. 셰이크 수치는 balance 확정값(2026-09-15)이다.")


main()
