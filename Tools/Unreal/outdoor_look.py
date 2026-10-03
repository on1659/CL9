"""바깥(맵 격자 밖) 모습 — 어두운 저녁 하늘 + 어두운 흙바닥 (2026-10-03, 사용자 "바깥이 테스트존에 밝은 스카이박스다, 어둡게").

다른 스크립트가 불러 쓰는 모듈이다 — 직접 돌리는 쪽은 apply_outdoor_look.py(전 레벨·전 사이트 행), setup_stage.py(새 스테이지).
정본 설명: docs/systems/light.md "바깥 모습".

- MI_CL9_DuskSky  : M_CL9_NightSky 의 인스턴스. 지평선이 조금 남은 어두운 남보라 + 옅은 별. 레벨 하늘 구의 기본 재질이자
                    사이트 행 로비 페이즈의 SkyMaterial — 로비·에디터 화면에서도 낮 하늘이 안 보이게.
- M_CL9_GroundDark: 월드 좌표 노이즈 두 겹으로 얼룩진 어두운 흙. UV 를 안 쓰니 바닥 판을 얼마나 늘려도 무늬가 안 늘어난다.
"""
import unreal

FOLDER = "/Game/Maps/Outdoor"
NIGHT_SKY = "/Game/Maps/Sky/M_CL9_NightSky"
DUSK_SKY = FOLDER + "/MI_CL9_DuskSky"
GROUND = FOLDER + "/M_CL9_GroundDark"

# 로비 페이즈 값 — 하늘 재질 외에 태양·보이는 하늘을 함께 낮춘다(옛 값 0.35 / 1.0, 구름 켬)
LOBBY = {"sun_scale": 0.2, "sky_luminance": 0.3, "clouds": False}

ML = unreal.MaterialEditingLibrary


def _log(msg):
    unreal.log_warning("[outdoor] " + msg)


def ensure_materials():
    """재질 둘을 만들거나 다시 짠다(멱등). (하늘 MI, 바닥 재질)을 돌려준다."""
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    if not unreal.EditorAssetLibrary.does_directory_exist(FOLDER):
        unreal.EditorAssetLibrary.make_directory(FOLDER)

    parent = unreal.load_asset(NIGHT_SKY)
    if parent is None:
        raise RuntimeError("missing %s — run Tools/Unreal/create_night_sky.py first" % NIGHT_SKY)
    sky = unreal.load_asset(DUSK_SKY)
    if sky is None:
        sky = tools.create_asset(DUSK_SKY.rsplit("/", 1)[1], FOLDER, unreal.MaterialInstanceConstant,
                                 unreal.MaterialInstanceConstantFactoryNew())
    ML.set_material_instance_parent(sky, parent)
    # 밤(지평선 0.006·천정 0.001)보다 한 단계 밝은 저녁 — 지평선에 보랏빛 노을이 조금 남는다
    ML.set_material_instance_vector_parameter_value(sky, "Horizon", unreal.LinearColor(0.040, 0.022, 0.035, 1.0))
    ML.set_material_instance_vector_parameter_value(sky, "Zenith", unreal.LinearColor(0.003, 0.005, 0.014, 1.0))
    ML.set_material_instance_scalar_parameter_value(sky, "StarDensity", 0.97)
    ML.set_material_instance_scalar_parameter_value(sky, "StarBrightness", 0.35)
    unreal.EditorAssetLibrary.save_loaded_asset(sky, False)

    ground = unreal.load_asset(GROUND)
    if ground is None:
        ground = tools.create_asset(GROUND.rsplit("/", 1)[1], FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    ML.delete_all_material_expressions(ground)

    def node(cls, x, y, **props):
        e = ML.create_material_expression(ground, cls, x, y)
        for k, v in props.items():
            e.set_editor_property(k, v)
        return e

    wp = node(unreal.MaterialExpressionWorldPosition, -1200, 0)
    # 큰 얼룩(약 8 m)과 잔 얼룩(약 1.5 m) 두 겹 — 바닥 판이 465 m 로 늘어나도 무늬 크기는 그대로다
    big_s = node(unreal.MaterialExpressionMultiply, -1000, -80, const_b=1.0 / 800.0)
    small_s = node(unreal.MaterialExpressionMultiply, -1000, 120, const_b=1.0 / 150.0)
    assert ML.connect_material_expressions(wp, "", big_s, "A") and ML.connect_material_expressions(wp, "", small_s, "A")
    big = node(unreal.MaterialExpressionNoise, -820, -80, scale=1.0, quality=1, levels=3, output_min=0.0, output_max=1.0)
    small = node(unreal.MaterialExpressionNoise, -820, 120, scale=1.0, quality=1, levels=2, output_min=0.0, output_max=1.0)
    assert ML.connect_material_expressions(big_s, "", big, "") and ML.connect_material_expressions(small_s, "", small, "")
    mix = node(unreal.MaterialExpressionLinearInterpolate, -620, 20, const_alpha=0.3)
    assert ML.connect_material_expressions(big, "", mix, "A") and ML.connect_material_expressions(small, "", mix, "B")
    dark = node(unreal.MaterialExpressionVectorParameter, -620, -220, parameter_name="DirtDark",
                default_value=unreal.LinearColor(0.022, 0.017, 0.012, 1.0))
    light = node(unreal.MaterialExpressionVectorParameter, -620, 220, parameter_name="DirtLight",
                 default_value=unreal.LinearColor(0.050, 0.042, 0.028, 1.0))
    color = node(unreal.MaterialExpressionLinearInterpolate, -380, 0)
    assert ML.connect_material_expressions(dark, "", color, "A") and ML.connect_material_expressions(light, "", color, "B") \
        and ML.connect_material_expressions(mix, "", color, "Alpha")
    rough = node(unreal.MaterialExpressionConstant, -380, 200, r=1.0)
    assert ML.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    assert ML.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    ML.recompile_material(ground)
    unreal.EditorAssetLibrary.save_loaded_asset(ground, False)
    return sky, ground


def apply_to_open_level(sky, ground):
    """지금 열린 레벨의 하늘 구·바닥 판 재질을 바꾼다. 저장은 부르는 쪽이 한다. 바꾼 액터 수를 돌려준다."""
    changed = 0
    for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        comp = a.static_mesh_component
        mesh = comp.static_mesh
        if mesh is None:
            continue
        name = mesh.get_name()
        # 하늘 구는 ACL9NightLighting 과 같은 규칙으로 찾는다(메시 이름에 SkySphere) — 로비 SkyMaterial 이 비었을 때도 이 재질이 보인다
        if "SkySphere" in name:
            comp.set_material(0, sky)
            changed += 1
            _log("  sky sphere %s -> %s" % (a.get_actor_label(), sky.get_name()))
        # 바닥 판 — 템플릿 바닥 메시(엔진 SM_Template_Map_Floor)이거나 라벨이 Floor 인 것
        elif "Template_Map_Floor" in name or a.get_actor_label() == "Floor" or a.get_actor_label().startswith("Floor_"):
            for i in range(max(1, comp.get_num_materials())):
                comp.set_material(i, ground)
            changed += 1
            _log("  ground %s -> %s" % (a.get_actor_label(), ground.get_name()))
    return changed


def apply_to_row(row, sky):
    """사이트 행의 로비 페이즈를 어두운 저녁으로. 밤·정전 페이즈는 손대지 않는다."""
    lobby = row.get_editor_property("lobby_lighting")
    lobby.set_editor_property("sky_material", sky)
    for k, v in LOBBY.items():
        lobby.set_editor_property(k, v)
    row.set_editor_property("lobby_lighting", lobby)
