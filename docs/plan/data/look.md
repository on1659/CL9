# 룩 표 (LookTable)

> 게임이 어떻게 **보이는가** 의 값이 전부 여기 있다 — 화면 필터, 조명 프로필, 램프 종류, HUD 밝기, 구조면 디테일.
> 판정 값은 하나도 없다(렌더와 판정은 분리 — [../../systems/light.md](../../systems/light.md)). 2026-10-16 사용자 "이 모든거 그냥 하나로
> 퉁쳐서 테이블로 관리" — 그전에는 `CL9Rules` Look* 열, 사이트 행의 조명 페이즈 셋, 모듈 소품·아이템 행의 빛 열, 퀵슬롯 C++ 기본값,
> 재질 인스턴스 16개에 흩어져 있었다. 명세 [../../goal/look-table.md](../../goal/look-table.md).

애셋 `/Game/Data/LookTable` — 클래스 `UCL9LookDef`(행 하나짜리 def, `CL9Rules` 처럼 폴더에 두면 레지스트리가 자동 등록, `GetLook()`).

## 실시간 모드 (에디터)

PIE 를 켜 둔 채 이 애셋을 열어 디테일 패널에서 값을 바꾸면 **화면이 바로 따라온다.** 바꾸는 순간 표의 판(`Revision`)이 하나 오르고,
이미 틱을 도는 쪽이 자기가 본 판과 비교해 다시 적용한다(정수 비교 하나 — 평소엔 공짜):

| 바꾼 값 | 누가 다시 적용하나 | 언제 |
|---|---|---|
| 조명 프로필의 페이즈 값 | `ACL9NightLighting` — 원래 매 틱 표를 읽는다 | 다음 틱(보간 속도대로) |
| 화면 단계화·그레인·후처리 재질·레트로 값·색 보정 | `UCL9LookModifier` — 매 프레임 | 다음 프레임 |
| 안개 모양 | `ACL9NightLighting` — 판을 비교 | 다음 틱 |
| 화면 해상도·AA | 로컬 `ACL9PlayerController::PlayerTick` → `ApplyLook` | 다음 틱 |
| 램프 종류(세기·거리·색·산란·원뿔·깜박임) | `ACL9SiteMap`(시설 조명) · `ACL9LightItem`(손전등·헤드램프) · `ACL9Character`(주머니 빛) · `ACL9YardTension`(마당 문등) | 다음 틱 |
| 프로필의 `LampSurviveShare` | `ACL9SiteMap` — 램프마다 적어 둔 굴림 값을 새 비율로 다시 나눈다 | 다음 틱 |
| 퀵슬롯 색 | `UCL9HUDWidget::NativeTick` → 칸 다시 그림 | 다음 틱 |
| HUD 글자·스태미나·먼지 덮개 | `UCL9HUDWidget::NativeTick` → `ApplyLook` | 다음 틱 |
| 구조면 가족·`bSurfaceDetail` | 표가 재질 인스턴스에 직접 밀어 넣는다(놓을 때 — 슬라이더 드래그 중엔 안 함) | 즉시. **재질 인스턴스도 저장해야 남는다(Save All)** |

- 다음 판부터 먹는 것: 램프 종류의 **포인트 ↔ 스포트 전환**(원뿔 0 ↔ >0 — 컴포넌트 종류가 바뀐다), 사이트 행의 `LampDensity`·
  `LightingProfile` 이름(배치·선택은 맵을 만들 때).
- 브라우저에서 고칠 때는 데이터 패널의 `/look` 페이지([../../systems/data-panel.md](../../systems/data-panel.md)) — 값을 넣고 `ApplyNow`·`ApplyBrightness` 를 부른다.
- 파이썬으로 바꿀 때는 편집 이벤트가 안 나므로 `tune_look.py`(값을 넣고 `ApplyNow` 를 부른다)를 쓴다.
- 출시 빌드에는 실시간 모드가 없다(판이 늘 0).

## 1. 화면 (Screen) — 레트로 룩

켜고 끄기는 플레이어 설정 `bRetroLook`(설정 화면 "화면"). 값은 [../../systems/light.md](../../systems/light.md) "룩 개편".

| 열 | 값 | 뜻 |
|---|---|---|
| `MaxResolution` | 520 | 내부 렌더 세로 해상도 상한(`r.ScreenPercentage.MaxResolution`, 16:9 기준) — 리썰 860×520. 창 크기와 무관, 최근접 필터로 키운다 |
| `AntiAliasing` | 1 | 0 없음 · 1 FXAA (TSR 은 픽셀을 시간 누적으로 지운다) |
| `PosterizeBands` · `PosterizeMix` | 16 · 0.5 | 톤매핑 뒤 밝기를 n 단계로 자르고 원래 색과 섞는다(색조 유지). 12 · 0.7 은 빛 둘레에 동심원 띠 |
| `Grain` | 0.15 | 톤매퍼 필름 그레인 |
| `PosterizeMaterial` | `M_CL9_PP_Retro` | 레트로 후처리 재질(After Tonemapping) — 아래 값이 모두 이 재질의 파라미터로 간다. 비면 그레인만. 2026-10-21 전에는 `M_CL9_PP_Posterize`(Bands·Mix 만) |
| `PixelSize` | 1 | 픽셀화 — 한 칸이 내부 렌더 픽셀 몇 개인가. 1 이면 끔 |
| `PixelateFromDepth` | 0 | 이 거리(cm)보다 먼 것만 픽셀화 — 0 이면 화면 전체 |
| `ColorSteps` | 0 | 채널마다 색 단계 수 — 0 끔, 2 이상이면 R·G·B 각각 자르고 디더가 경계를 섞는다 |
| `DitherPattern` · `DitherStrength` | 2 · 1 | 디더 무늬(0 없음 · 1 베이어 2×2 · 2 베이어 4×4 · 3 베이어 8×8 · 4 백색 잡음)와 세기. 색 단계·팔레트가 꺼져 있으면 효과 없음 |
| `Palette` · `PaletteMix` | `T_Pal_Dusk16` · 0 | 1×N 팔레트 띠(N ≤ 64, `/Game/Effects/Palettes`) — 화면 색을 가장 가까운 팔레트 색으로. 섞기 0 이면 안 씀 |

처리 순서: 픽셀화 → 밝기 단계화 → 색 단계(+디더) → 팔레트(+디더). 기법은 VenediktVad "UE5 Pixelization/Dithering PostProcess"(itch.io)의
기능 목록을 우리 재질로 다시 구현한 것이다(그 팩의 애셋은 쓰지 않는다, 2026-10-21 — [../../goal/screen-debugger.md](../../goal/screen-debugger.md)).

### 1-1. 색 보정 (Grade) — 레트로 룩과 상관없이 늘

`UCL9LookModifier` 가 건다. 기본값이 엔진 기본값이라 손대지 않은 표는 화면을 바꾸지 않는다.

| 열 | 기본 | 뜻 (후처리 설정) |
|---|---|---|
| `Saturation` · `Contrast` · `Gamma` · `Gain` | 1 · 1 · 1 · 1 | 채도·대비·감마·게인 (`ColorSaturation`·`ColorContrast`·`ColorGamma`·`ColorGain`) |
| `Temperature` · `Tint` | 6500 · 0 | 색온도(켈빈)·색조 (`WhiteTemp`·`WhiteTint`) |
| `ShadowTint` · `HighlightTint` | 흰색 · 흰색 | 그림자·하이라이트에 곱하는 색 (`ColorGainShadows`·`ColorGainHighlights`) |
| `Vignette` | 0.4 | 가장자리 어둡게 (`VignetteIntensity`) |
| `Bloom` | 0.675 | 밝은 것 번짐 (`BloomIntensity`) |
| `ChromaticAberration` | 0 | 색수차 (`SceneFringeIntensity`) |
| `Sharpen` | 0 | 샤픈 |
| `MotionBlur` | 0.5 | 모션 블러 (`MotionBlurAmount`) |

### 1-2. 안개 모양 (Fog) — 음수 = 레벨 값 그대로

`ACL9NightLighting` 이 표 판(Revision)이 바뀔 때 레벨의 ExponentialHeightFog 에 넣는다. 밀도·색은 조명 프로필 페이즈 몫이다.

| 열 | 기본 | 뜻 |
|---|---|---|
| `FogHeightFalloff` | -1 | 높이 감쇠 — 클수록 안개가 바닥에 깔린다 |
| `FogStartDistance` | -1 | 안개 시작 거리(cm) |
| `FogMaxOpacity` | -1 | 최대 불투명도(0~1) |
| `VolumetricFog` | -1 | 볼류메트릭 안개 -1 레벨 값 · 0 끔 · 1 켬 |
| `VolumetricFogExtinction` · `VolumetricFogDistance` | -1 · -1 | 볼류메트릭 짙기·거리(cm) |

### 게임 안에서 고치기 — 디버그 패널 "화면" 칸

Tab 디버그 화면을 열면 오른쪽에 이 표의 화면·색 보정·조명(지금 사이트 프로필의 페이즈)·안개 모양 값이 슬라이더로 뜬다 — 바꾸는 즉시
화면에 보인다. [처음 값으로]는 패널을 연 순간의 값으로, [표 저장]은 이 애셋을 디스크에 쓴다(에디터만) — [../../systems/ui.md](../../systems/ui.md) 디버그 패널.
맨 위 [프리셋: 맵 그대로]를 켜면 이 표가 거는 것을 모두 걷어 맵에 배치된 조명·안개·후처리 그대로 보인다(2026-10-21). 표 값은 바뀌지 않는다.

## 2. 조명 프로필 (Profiles)

사이트 행은 `LightingProfile` 이름만 갖는다(비면 첫 프로필). 프로필 하나 = `Lobby` · `Night` · `Blackout` 페이즈 셋 + `LampSurviveShare`.
**몇 개의 등이 서나**(`LampDensity`)는 건물마다 달라 사이트 행에 남았다(양계장 0.5 · 대농장 0.35 · 돼지 농장 0.2).

| 프로필(`Name`) | 쓰는 사이트 | 메모 |
|---|---|---|
| `default` | henhouse · farm · pigfarm · henhouse_review | 2026-10-15 룩 개편 값 |
| `flat` | flat (테스트 레벨) | 옛 값 그대로(밤 SkyScale 10 · 안개 0.01) |

페이즈 한 벌(`FCL9LightingPhase`) — 태양·하늘은 레벨 라이트에 **곱하는 배율**, 안개는 절대값:

| 필드 | 뜻 | default 로비 / 판 / 정전 |
|---|---|---|
| `SunScale` · `SunColor` | 태양 배율·색조 | 0.2 흰색 / 0.05 달빛 청색 / 0 |
| `SkyScale` · `SkyColor` | 하늘빛 배율·색조("검정의 색") | 0.5 / 5 차가운 청색 / 0 |
| `SkyLeak` | 닫힌 실내로 새는 하늘빛(Lumen Skylight Leaking, 히트거리/1000cm 로 깎임) | 0.5 / 0.5 / 0 |
| `FogDensity` · `FogColor` | 안개 밀도(값/1000 per cm — 거리감의 전부)·색 | 0.0436 / 0.5 검정 / 0.5 검정 |
| `FogSkyAmbient` | 하늘 앰비언트가 안개에 더하는 비율 — 0 이면 검은 안개가 정말 검다 | 1 / 0 / 0 |
| `SkyLuminance` | 보이는 하늘 밝기 배율 | 0.3 / 0.15 / 0.05 |
| `SkyMaterial` · `bClouds` | 하늘 구 재질·구름 | `MI_CL9_DuskSky` 끔 / `M_CL9_NightSky` 끔 / 같음 |
| `SkyScattering` | 하늘빛이 볼류메트릭 안개를 밝히는 세기 | 1 / 0 / 0.1 |
| `ExposureBias` | 노출 보정(EV) — 화면 전체를 2^EV 배로. 노출이 고정이라 이것이 곧 화면 밝기. 좁은 방에서도 먹는 유일한 앰비언트 손잡이 | 0 / 밝기 손잡이가 정함 / 0 |
| `LampSurviveShare`(프로필) | 전원 인트로 뒤 살아남는 실내 램프 비율 | 0.3 |

전원 전 첫 어둠은 `Night` 에 `SkyLeak` 0 · `FogDensity × CL9Rules.PrePowerFogScale`(코드 — light.md 전원 단계).

### 밝기 (2026-10-16 — 사용자 "플래시 없으면 너무 어두워, 전체 밝기 하나로 다른 것들을 세팅", "게임안에서도 나오게")

손잡이 둘이 **같은 축**을 쓴다. 축의 두 끝은 표의 `NightDark`(0)와 `NightBright`(1) — 판 조명 한 벌씩이다.
축의 주된 손잡이는 **노출 보정 `ExposureBias`** 다 — 하늘빛 누수는 좁은 복도·방에서 거리로 깎여 사라져(2026-10-16 PIE: 하늘빛 40·누수 1 로도 복도가
검정), 손전등 없이 보이게 하는 것은 노출뿐이다(+4 EV 에서 복도 벽·부엌 조리대가 실루엣으로 읽힌다). 끝점(2026-10-16 PIE 로 맞춤): `NightDark` 누수 0.3 · 하늘빛 3 · 안개 0.6 · EV 0,
`NightBright` 누수 1 · 하늘빛 150 · 안개 0.12 · 하늘 색 (0.85, 0.88, 1) · EV +3. 노출만 올리면(+4.5) 등 켜진 방이 하얗게 떠서, 밝은 끝은 노출을 줄이고
하늘빛 앰비언트를 크게 했다. 노출은 램프·손전등 빛도 같이 밝힌다 — 정전은 EV 0 이라 그대로 캄캄하다.

| 손잡이 | 값 | 화면 (스위치 방 복도) |
|---|---|---|
| 프로필 `default`·`flat` 의 `Brightness` | **0.6** (누수 0.72 · 하늘빛 91 · 안개 0.31 · EV +1.8) | 복도 벽이 실루엣으로 읽히고 여전히 어둡다 |
| 설정 밝기 20% | 축 위 0.3 | 캄캄 — 손전등 필요 |
| 설정 밝기 80% | 축 위 0.9 | 손전등 없이 복도·부엌이 읽힌다 |

`flat` 프로필도 같은 축을 탄다(2026-10-16부터 테스트 레벨의 옛 밤 값 — 하늘빛 10·안개 0.01 — 은 덮였다).

| 손잡이 | 누가 | 하는 일 |
|---|---|---|
| 프로필의 `Brightness` (0~1) | 만드는 사람(표) | 돌리면 그 프로필의 `Night` 숫자 필드(태양·하늘빛·누수·안개·하늘 밝기·산란·색)가 끝점 사이 그 지점으로 **다시 써진다**. 끝점을 고치면 모든 프로필에 다시. 하늘 재질·구름은 그대로 |
| 설정 "화면 → 밝기" (0~100%, 기본 50%) | 플레이어 | 50% 는 표에 저장된 `Night` 그대로. 올리고 내리면 같은 축 위에서 그만큼 판 조명을 민다(`ShiftNight` — 저장된 값에 차이만 더하므로 손으로 고친 값도 지킨다). 자기 화면에만, `GameUserSettings.ini` 에 저장 |

- 두 손잡이 모두 **조명 값만 바꾸고 다른 일은 하지 않는다**(사용자 "그걸로 딴거 뭐하면안돼"). 게임 코드가 `Brightness` 를 읽는 곳은 플레이어
  슬라이더의 출발점 하나뿐이다.
- 정전(`Blackout`)과 로비는 두 손잡이가 건드리지 않는다 — 정전은 일부러 캄캄한 장면이다. 전원 전 첫 어둠은 `Night` 에서 파생되므로 같이 밝아진다.
- 파이썬으로 끝점·손잡이를 넣을 때는 `Tools/Unreal/set_look_brightness.py`(편집 이벤트가 안 나므로 `ApplyBrightness` 를 부른다).

## 3. 램프 종류 (LampKinds)

소품 배치(`ModuleTable` `PropSets[].Props[].LampKind`, 자리는 배치의 `LightOffset`) · 아이템 행(`ItemTable` `LampKind`) · 마당 문등
(`YardDoor`, 자리는 `CL9Rules.YardLampHeight`)이 이름으로 고른다.

| 종류 `Name` | 세기 cd `Intensity` | 거리 cm `Radius` | 색 `Color` | 산란 `Scattering` | 원뿔 `ConeAngle` | 감쇠 `FalloffExponent` | 깜박임 `Flicker` / `FlickerSpeed` | 쓰는 곳 |
|---|---|---|---|---|---|---|---|---|
| `WorkLamp` | 10 | 900 | 전구색 | 1 | 60 (아래 향한 스포트·그림자) | 0 | 0 | 모듈 작업등 36 자리 |
| `Campfire` | 12 | 700 | 주황 | 4 | 0 (포인트) | 0 | 0.35 / 5 Hz | 마당 모닥불 |
| `Flashlight` | 4 (밝기 배율) | 2600 | 흰색 | 3 | 34 | 2 | — | 손전등 |
| `Headlamp` | 4 (밝기 배율) | 2600 | 흰색 | 3 | 34 | 2 | — | 헤드램프 |
| `PocketLight` | 4 (밝기 배율) | 2600 | 흰색 | 3 | 34 | 2 | — | **주머니 빛** — 비활성 퀵슬롯 칸의 켜진 손전등이 머리에서 앞으로 비추는 빛 (`ACL9Character::PocketLight`, 2026-10-17 — [../../systems/light.md](../../systems/light.md) 손전등). `Flashlight` 복사로 시작, 손에 든 것과 차이를 두려면 세기·거리를 줄인다 |
| `YardDoor` | 10 | 900 | 전구색 | 1 | 0 | 0 | 코드(끊기는 노이즈) | 마당 → 건물 문 위 |

감쇠 `FalloffExponent`: 0 = 물리 감쇠(거리 제곱에 반비례, 세기는 칸델라). 0 보다 크면 스포트만 그 지수로 `Radius` 까지 완만하게 줄어 먼 곳까지 비추고, 세기는 칸델라가 아니라 밝기 배율이다 — 손전등 셋은 2026-10-20 플레이테스트 4번("너무 가까운 것만 비춘다")에서 16000 cd · 1800 · 26° · 물리 감쇠 → 4 · 2600 · 34° · 지수 2 로 바꿨다(봇 영상으로 확인, 값은 초안).

원뿔 0 = 포인트 라이트(그림자 없음), >0 = 스포트(그림자, 볼류메트릭 그림자는 끔). 표에 없는 이름을 고르면 빛이 없고 경고가 남는다.
(배열 이름은 `UCL9LookDef::LampKinds`, 행 구조체는 `FCL9LampKind` — 2026-10-16 표기.)

## 4. HUD

| 열 | 값 | 뜻 |
|---|---|---|
| `QuickSlotActive` · `QuickSlotInactive` | 흰색 70% · 25% | 퀵슬롯 칸 테두리·아이콘·이름 밝기 — 흰색만(2026-09-13). "먼지 낀 유리" 룩(아래)에 맞추려면 뼈색 (0.80, 0.76, 0.71) 70% · 25% 로 — 표 값이라 에디터에서 |
| `HudText` | 뼈색 (0.80, 0.76, 0.71) 60% | **"먼지 낀 유리" 룩(2026-10-22 결정, 시안 데이터 패널 `/uilook` T 절)** — 늘 떠 있는 HUD 글자(시계·HP·코인·주문 힌트·머리·창구·아이템 라벨·버프). `UCL9HUDWidget::ApplyLook` 이 NativeConstruct 와 표 변경 때 건다. WBP 의 글자색은 미리보기일 뿐 |
| `HudTextBright` | 같은 뼈색 82% | 지금 읽어야 하는 글 — 프롬프트·첫 줍기 힌트·장소 알림·주문서 카드 |
| `HudWarn` | 탁한 주황 (0.80, 0.59, 0.31) 80% | 밤 만료 경고 글자. 밝은 노랑·흰색이 아니다 |
| `HudStamina` | 뼈색 45% | 스태미나 바 채우기(탈진색은 WBP 값 그대로) |
| `HudDustOpacity` | 1 | 먼지 덮개(`WBP_HUD` 의 `DustOverlay` 이미지 — 다른 HUD 요소 **위**)의 불투명도, 0 이면 접는다 |

## 5. 구조면 (Surface)

`bSurfaceDetail`(전체 켬/끔 — 재질 static switch, 끄면 비용 0) + 가족 셋(`UCL9LookDef::SurfaceFamilies`, 행 구조체 `FCL9SurfaceFamily`). 가족 값은 그 가족의 재질 인스턴스(`M_CL9_Structure` 자식)에
밀어 넣는다 — 인스턴스의 색·`RoughnessBase` 는 인스턴스 자신의 것.

| 가족 `Name` | 노멀 · 거칠기 텍스처 `Normal`·`Roughness` | 타일 cm `Tiling` | 노멀 세기 `NormalStrength` | 거칠기 min–max `RoughnessMin`·`RoughnessMax` | 인스턴스 `Materials` |
|---|---|---|---|---|---|
| `wood` | `T_WoodGrain_N` · `T_WoodGrain_ORM` | 150 | 0.5 | 0.75–0.95 | 벽 500/250/125 나무·짙은 목재, 천장, 계단 면, 문, 기둥·대들보 나무 (11) |
| `concrete` | — · `T_Grunge_M` | 220 | 0 | 0.8–0.98 | 바닥, 계단 콘크리트 (2) |
| `metal` | `T_PaintedMetal_N` · `T_PaintedMetal_ORM` | 120 | 0.4 | 0.45–0.8 | 문 철물, 기둥·대들보 금속 (3) |

## 도구

| 스크립트 | 언제 |
|---|---|
| `Tools/Unreal/migrate_look_table.py` | 한 번 — 옛 열에서 이 표를 만들었다(2026-10-16) |
| `Tools/DataPanel/data_panel.py` (페이지 `/look`) | 브라우저에서 이 표 전부를 보고 고치기 — PIE 중 바로 반영, 저장 버튼 ([../../systems/data-panel.md](../../systems/data-panel.md)) |
| `Tools/Unreal/tune_look.py` | PIE 중 파이썬으로 값 바꾸기(`profile:` · `lamp:` · `surface:` 키, `--save` 는 json 의 `"save": true`) |
| `Tools/Unreal/create_look_materials.py` | 후처리 재질·구조 부모 재질·인스턴스를 처음 만들 때 |
| `Tools/Unreal/create_retro_material.py` | 레트로 후처리 재질 `M_CL9_PP_Retro` 와 팔레트 `T_Pal_*` 를 짓고 이 표가 그것을 가리키게 한다 — 팔레트를 더할 때 `PALETTES` 에 한 줄 |
| `Tools/Unreal/apply_outdoor_look.py` | 모든 프로필의 로비 하늘을 저녁 하늘로 |
