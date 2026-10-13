# 조명 시스템 (손전등·어둠·정전)

> 어두운 게임을 위한 조명 시스템 — 손전등·발광 아이템·정전과 렌더 세팅.
> **광원 개체는 전부 아이템/장비 데이터**이고, 이 문서는 아이템이 아닌 부분(정전·렌더 세팅·
> 판정 연동)을 담당한다. 조명 값은 전역 원칙이 아니라 **이 시스템에서 세팅한다.**

## 설계 방향 (이 시스템의 세팅 — 전역 원칙 아님)

- 어둠이 게임의 기본 톤이다 — 손전등 도달 1800cm 밖은 캄캄하게 세팅한다 (2026-09-16 800cm에서
  올림 — "바로 앞만 비춘다"는 피드백. 방은 시설 조명(작업등)이 밝히고 그 밖이 어둠이다).
- **렌더와 판정은 분리다.** 몬스터 시야는 화면 밝기가 아니라 값으로 판정한다
  (`SightIn` 330 → 정전 시 `SightDark` 111 — [monster.md](monster.md)). 조명은 표현이다.
- 정전이 오면 손전등도 꺼진다 (plan/loop.md).

## 광원 목록

| 광원 | 값 | 출처 |
|---|---|---|
| 헤드램프 (`headlamp`) | **머리에 쓰는 장비** (CarrySlot `Head`) — 쓰면 손이 빈 채 시선 방향 스포트라이트, F 토글(F는 헤드램프 전용). **정전에 안 꺼진다**(내 장비지 시설이 아니다 — 2026-09-05) | [../plan/data/item/headlamp.md](../plan/data/item/headlamp.md) |
| 손전등 (`flashlight`) | **손에 드는 아이템** — 활성 칸에 있으면(= 손에 들면) 지향성 스포트라이트 1800cm·반각 26°(2026-09-16 초안), 다른 칸으로 전환하면 실물이 사라져 꺼짐(퀵슬롯 — 활성 칸만 실물, [inventory.md](inventory.md)) | [../plan/data/item/flashlight.md](../plan/data/item/flashlight.md) |
| ~~발광 아이템 (계란)~~ | **폐기 (2026-09-11)** — 빛은 손전등·헤드램프에서만 나온다. `egg` 행 `bEmitsLight` off. Glow 코드 경로·컬럼은 남아 있으나 쓰는 행이 없다 (decisions.md) | [../plan/data/item/egg.md](../plan/data/item/egg.md) |
| 시설 조명 | **구현됨 (2026-09-16)** — 모듈 표 소품(`FCL9PropPlacement`)의 `LightIntensity`(cd, 0이면 빛 없음)·`LightRadius`·`LightColor`·`LightOffset`. 배치기(`ACL9SiteMap::PlaceProps`)가 그 소품 인스턴스마다 그림자 없는 포인트 라이트를 붙이고, 정전이면 `Tick`이 끈다. 양계장은 작업등(`SM_Henhouse_WorkLamp_A`)이 이 값을 든다(200cd 초안 → **10cd**, 09-17 PIE: 200은 밤 실내를 로비 밝기로 만들었다). **개수**는 사이트 행 `LampDensity`(0~1)로 조절한다 — 등 몸체는 다 서고 그 비율만 켜진다(양계장 0.5, 2026-09-18). 2026-09-17 컬럼 셋 추가: `LightScattering`(안개 후광), `LightFlicker`(진폭 0~1, 세기가 1±진폭×펄린 노이즈)·`LightFlickerSpeed`(Hz) — 배치기 `TickFlicker`가 켜진 램프만 흔든다, 위상은 자리에서 나와 시드가 같으면 같다. 마당 모듈의 모닥불(`SM_Campfire_A`)도 같은 컬럼(12cd·700cm·주황·산란 4·깜박임 0.35/5Hz)이고 정전이면 같이 꺼진다(사용자 결정). 불꽃은 메시의 발광 슬롯(`M_CL9_Campfire_Flame` — 재질 파라미터 `FlameStrength` 기본 2.0, Blender 미리보기는 3; `Tools/Unreal/import_campfire.py`) | ModuleTable PropSets |
| 스탠드 램프 (`standing_lamp`) | 놓으면 주변을 밝힘 — 조명 세팅 검토 후 값 확정 | equipment.md 미결 |

**발광 균형이 좁다** (plan/data/item/common.md): 발광이 세지면 손전등의 역할이 무의미해지고,
약하면 "들고 있으면 들킨다"가 사라진다. 값 조정은 이 문장을 기준으로.

## 손전등 (아이템)

- **손에 드는 물건이다** (2026-09-01 결정 — [../plan/data/item/flashlight.md](../plan/data/item/flashlight.md)).
  활성 칸이면 빛나고, 다른 칸으로 전환하면 실물이 없어져 꺼진다(퀵슬롯 규칙 — [inventory.md](inventory.md)) —
  계란을 손에 들려면 어둠을 감수하거나 팀이 분업한다(헤드램프가 이 마찰의 해법).
- 켜짐/꺼짐은 들고 있을 때 좌클 `IA_Use`(행 `UseAction`=`ToggleLight`) 토글 ([input.md](input.md), 2026-09-12) — 상태는 아이템 액터에
  Replicated, 다른 플레이어 화면에서도 그 사람의 빛이 보인다.
- 어둠 속 동료 식별은 손전등 + 슬롯 색(`TintColor`)이 담당한다.
- 배터리·연료 시스템을 만들지 않는다 — 손전등의 카운터는 정전이다.

**구현** (5c → 2026-09-11 클래스 분리): 빛을 내느냐는 **클래스**가 정한다 — `ACL9Item`(빛 없음)
→ `ACL9LightItem`(라이트 컴포넌트·`bLightOn` 복제·들면 켜짐, 점광원) → `ACL9FlashItem`(스포트라이트,
F 토글, 정전 규칙). 행의 `ActorClass`가 `BP_CL9FlashItem`을 가리키면 손전등이 되고, 광원 수치
(`LightRadius`·`LightIntensity`·`LightConeAngle`)만 행에서 읽는다. F(`IA_Flashlight`)→
`Server_Flashlight`→`ACL9FlashItem::ToggleLight`→`bLightOn` 복제(각 머신 OnRep로 빔 표시).
들면 flashlight는 off로 시작, 놓기·던지기·품 넣기는 off(`OnAcquired`/`OnReleased` 훅), 정전이면
flashlight 강제 off. 광원은 `MeshComponent`의 자식으로 렌즈 앞(+X 끝)에 붙어 소유자 화면에서 메시가
1인칭 팔 소켓으로 옮겨 가면 빛도 따라간다(2026-09-13). 빔 방향 = 메시 +X; 소유자 화면은 팔이
카메라 자식이라 빔이 시선을 그대로 따르고, 남의 화면은 상체가 시선 피치를 따른다(전신 ABP
`AimPitch`) — 어느 쪽이든 아래를 보면 바닥이 비춰진다.

**헤드램프** (2026-09-05): 같은 아이템 광원 경로에 `CarrySlot::Head`가 얹힌다. E로 줍는 대신
`HostWear` → `ACL9Item::OnWorn`(HoldState `Worn`): 1인칭 카메라 컴포넌트에 부착(착용자 시선을
그대로 따름 — 타인 화면은 yaw만), 착용자에겐 메시가 안 보인다(OwnerNoSee), 충돌 없음.
`Server_Flashlight`(F)는 **쓴 램프만** 토글하고, 손 손전등은 `Server_UseStart`(좌클 누름)가 토글한다(2026-09-12). 정전 강제 소등(`ACL9FlashItem`의
ToggleLight·ShouldLightBeVisible·Tick)은 `IsWorn()`이면 건너뛴다. 미러는 `PlayerState.HeadItem`.

## 정전 (Blackout)

판정은 호스트, 연출은 각 클라:

```
호스트: 밤 시작 `StartNightClock` → GameState.BlackoutServerTime = 밤 시작 서버시각 + DarkBase(185)  (Replicated)
        `bNoBlackout`(progression 일자 덮어쓰기, `IsBlackoutDisabled`)이면 0 = 그 밤은 정전 없음
        반납마다 `RecordReturn`이 DarkAccel(4)씩 앞당김, 하한 = 밤 시작 + DarkMin(125)
  ↓ 각 머신, 매 틱: IsBlackout() = GetServerWorldTimeSeconds ≥ BlackoutServerTime  (OnRep·bool 없음 — 시각에서 파생)
시설 조명 소등 (`ACL9SiteMap::Tick` → SetLampsOn) + 손전등 빔 강제 off (착용 헤드램프 예외) + 정전 연출 (미정)
호스트: 몬스터 감각을 SightDark로 전환 (monster.md — 열만 있고 아직 소비처 없음)
```

- 정전 예고 연출(조명 깜빡임 등)은 미정 ([warning.md](warning.md)).
- 정전 후 광원은 헤드램프와 스탠드 램프뿐이다(발광 계란은 2026-09-11 폐기) — "소리로 길을 찾는"
  설계는 본편이 아니라 별도 목적지의 몫이다 (plan/loop.md 귀환 절).

**구현 상태** (밤 수명주기 dc407d3 · session.md): 위 흐름 그대로다 — 복제되는 것은 `ACL9GameState.BlackoutServerTime`
하나이고 각 머신이 매 틱 `IsBlackout()`으로 파생한다(OnRep 트리거 없음 — 조명 `NightLighting`·`ACL9SiteMap`·손전등이
그렇게 읽는다). 손전등 소등(아래 손전등 절 — `IsBlackout`이면 빔 강제 off)·시설 조명 소등(광원 목록 표, 2026-09-16)·
`bNoBlackout`(`UCL9ProgressionDef::IsBlackoutDisabled` → `StartNightClock`) 구현됨. 테스트는 `cl9.blackout` 치트로
정전을 즉시 트리거. 정전 예고 연출과 `SightDark` 전환(`UCL9MonsterDef::SightDark` 열만 있고 소비처 없음)은 아직 미구현.

## 마당을 건너는 길 (2026-10-12, 7j — [../goal/loop-7j-round-start.md](../goal/loop-7j-round-start.md))

- **안개**: 전원을 켜기 전(첫 어둠) `ACL9NightLighting::SelectTarget` 이 사이트 밤 안개 밀도에 `CL9Rules.PrePowerFogScale`(초안 2)을 곱한다 — 켜면 판의 행으로 돌아간다(실측 0.10 → 0.05).
- **문 위 작업등**: `ACL9YardTension`(머신마다 로컬, 복제 없음)이 판마다 생성된 배치에서 마당(실외 모듈·격자 밖)과 실내 모듈 사이의 문(1층 `Door` 변)을 찾아
  문 바깥 위 `YardLampHeight` 에 `YardLampMesh` 와 점광(`YardLampIntensity` cd, 그림자 없음)을 건다. 판 동안만 켜지고 끊기듯 깜박인다(노이즈가 −0.25 아래면 거의 꺼짐).
  마당 모듈에는 등 기둥이 없다(모닥불뿐) — 문마다 거는 쪽이 배치에 상관없이 맞는다.
- **보이기만 하는 기척**: 전원 전, 로컬 플레이어가 마당에서 문을 바라보면(거리 600 ~ `SilhouetteRange`, 시선 내적 ≥ 0.9) 문마다 판에 한 번 어두운 실루엣(`Silhouette*` — 몸·걷기 애님·검은 재질)이
  문 안쪽 260 cm 를 문 폭 방향으로 700 cm 건넌다. 뒤에서 비추는 빛(`SilhouetteBackLight`)이 윤곽을 만든다. **창문이 키트에 없어 문간이다**(사용자 답의 "창문"에 가장 가까운 자리).
- **울음**: 전원 전 마당에 있는 동안 `YardCryInterval*` 초마다 가까운 문 안쪽 7~13 m 에서 소리 행 `YardCry`(S_MonsterNear) — 로컬, 판정 없음. 도착 카메라 동안은 울음·실루엣을 시작하지 않는다.

## 렌더 세팅 (어둠 톤 유지)

**구현: `ACL9NightLighting`** (5c 앞당김, 커밋 7dd22c3 · `Source/CL9/World/`). 레벨에 1개
놓이는 로컬 표현 액터로, `OnNightPhaseChanged`를 구독해 밤 페이즈에 맞춰 씬을 낮춘다:

- BeginPlay에 맵의 태양(DirectionalLight)·SkyLight·ExponentialHeightFog를 찾아 authored 값을
  베이스라인으로 잡고, 페이즈에 맞는 **사이트 행의 조명 세팅**으로 보간한다(`TransitionSpeed`).
  태양·하늘은 배율(×색조), 안개는 절대값이다 — 컬럼은 [../plan/data/site.md](../plan/data/site.md)
  조명 절. **페이즈별 값은 액터 프로퍼티가 아니라 표다** (2026-09-12 — 어둠은 사이트의 성격이라서.
  액터에 남은 것은 `ExposureLock`·`TransitionSpeed`뿐). 표를 고치는 데 리빌드는 필요 없다.
- 자동 노출(Eye Adaptation)은 **min=max 밝기 고정**(`ExposureLock`)으로 잠근다 — 어두운 곳에서
  화면이 저절로 밝아지지 않게. 액터가 소유한 `UPostProcessComponent`(bUnbound)에서 세팅하므로
  맵에 별도 PostProcessVolume이 필요 없다.
- 판정 아님 — 몬스터 시야는 화면 밝기가 아니라 값으로 판정한다(`SightIn`→`SightDark`, monster.md).

**"어두운데 보인다"는 밝기가 아니라 검정의 색이다** (2026-09-12, 리썰컴퍼니 방식). 축사 안이
완전 검정으로 나와서 사용자가 "조금은 보이게" 요청했고, 전역 밝기(노출·태양)를 올리는 대신
두 가지로 풀었다 — 정전 때 둘 다 다시 꺼지므로 "불빛 밖은 안 보인다"는 압박은 그대로다.

| 수단 | 값이 사는 곳 | 효과 |
|---|---|---|
| 약한 **청색 앰비언트** — `SkyScale` 10 + 차가운 `SkyColor` + `SkyLeak` 1 | 사이트 행 `NightLighting` | 벽·바닥이 회청색 실루엣으로 드러난다. 대비가 낮아 형태만 보이고 디테일은 안 보인다 |
| **볼류메트릭 안개(리썰식, 2026-09-17 조명 기본)** — `FogDensity` 밤·정전 0.05(옛 0.01은 너무 옅어 빔이 안 보였다) + 검정 `FogColor`, 광원마다 `LightScattering`(손전등·헤드램프 8, 작업등 1, 모닥불 4) | 사이트 행 + 레벨의 `ExponentialHeightFog`(볼류메트릭 켬) + 아이템 표·소품 빛 컬럼 | 손전등 빔이 공기 중에 원뿔로 보이고 광원 둘레가 뿌옇다. 안개 색이 검정이라 안개 자체는 화면을 밝히지 않는다 — 어둠은 유지. 참고 영상(CCL8)의 순검정·안개 없음 대신 리썰 쪽을 택했다(사용자 결정). PIE 후보 A(0.05/8/1) 채택, B(0.1/4/2)는 작업등 후광이 복도를 밝혔다 |
| 정전 — `SunScale` 0 · `SkyScale` 0 · `SkyLeak` 0, 안개는 검정 | 사이트 행 `BlackoutLighting` | 앰비언트가 꺼져 다시 캄캄 (PIE 실측: 완전 검정) |
| 보이는 하늘 — `SkyLuminance` 밤 0.15 · 정전 0.05 (2026-09-16 초안) | 사이트 행 각 페이즈 → `SkyAtmosphere.SkyLuminanceFactor` 배율 | 태양을 5%로 낮춰도 대기는 낮 색으로 남아 문밖·마당이 하얗게 탔다("스카이박스가 너무 밝다"). 태양과 따로 누른다 |
| **밤 하늘 구 재질** — `SkyMaterial` = `M_CL9_NightSky`(별 있는 남색, Unlit) · `bClouds` 끔 · `SkyScattering` 0.1 (2026-09-18) | 사이트 행 각 페이즈 → `ACL9NightLighting::ApplyDiscrete` 가 레벨 `SM_SkySphere` 재질을 바꾸고 `VolumetricCloud` 를 숨긴다, 하늘빛의 안개 산란은 보간 | 대기 하늘을 아무리 눌러도 낮 색이라 아예 재질을 바꾼다. 구름은 태양이 0.3 이어도 하얗게 남아 끈다. 하늘빛(SkyScale 5)이 볼류메트릭 안개를 밝혀 마당이 뿌옇던 것은 산란 0.1 로 |
| 방 안 — 시설 조명(작업등, 위 광원 목록) | ModuleTable PropSets | 천장을 덮은 방은 루멘이 하늘빛을 막아 `SkyLeak`이 전부였다 — 방은 작업등이 밝히고 정전이면 꺼진다. **UE5 루멘에는 유니티식 라이트 프로브가 없다** — 그 자리를 `SkyLeak`(이미 1)과 시설 조명이 맡는다 |

두 가지 실측 함정이 값의 모양을 정했다 (site.md 조명 절에 자세히): **루멘은 천장 덮인 방에
하늘빛을 한 점도 안 넣는다** — SkyLight `Cast Shadows`를 꺼도 마찬가지 — 그래서 `SkyLeak`
(Lumen Skylight Leaking)이 실내 앰비언트의 전부다. 그리고 **SkyLight가 실시간 캡처라 하늘이
태양을 따라 어두워진다** — 밤 `SkyScale`이 10인 건 그 어두운 하늘 위에 곱하는 값이라서다.

`ExposureLock`(1.0)은 건드리지 않는다 — 노출을 낮추면 손전등 빔과 정전까지 같이 밝아져 톤이
무너진다. 09-07에 같은 이유로 밤 배율을 0.8/1.2까지 올렸다가 원복한 이력이 있다(전역 밝기로
풀면 정전이 의미를 잃는다). 축사 통로마다 시설 조명(작업등)을 놓고 정전 때 끄는 구조는 2026-09-16에
구현됐다(`ACL9SiteMap::LampComponents`, 위 표) — 방은 작업등이 밝히고, 앰비언트·안개는 그 위에 깔리는
바닥 톤이다. 작업등이 들어오면서 밤 `SkyScale`은 10에서 내릴 수 있다(초안 5 — PIE로 맞춘다).

- 감마 슬라이더 범위는 이 세팅과 함께 정한다 ([settings.md](settings.md)).

## 바깥 모습 — 어두운 저녁 하늘 + 흙바닥 (2026-10-03)

사용자: "바깥이 그냥 테스트존에 밝은 스카이박스다, 어두운 걸로" → **지금 있는 레벨 전부 + 앞으로 만들 스테이지 전부**.
맵 격자 밖(트럭 마당·판 둘레)은 엔진 템플릿 그대로였다 — 하늘 구 `M_SimpleSkyDome`(밝은 낮 하늘)과 바닥 판의 격자 재질. 밤 페이즈는
사이트 행이 하늘을 `M_CL9_NightSky`로 바꿔 왔지만 **로비(밤 전)와 에디터 화면에는 낮 하늘이 그대로** 보였다.

| 무엇 | 에셋 | 어디에 |
|---|---|---|
| 저녁 하늘 | `/Game/Maps/Outdoor/MI_CL9_DuskSky` — `M_CL9_NightSky`의 인스턴스(지평선 남보라 0.040·0.022·0.035, 천정 0.003·0.005·0.014, 별 밀도 0.97·밝기 0.35) | ① 각 레벨 하늘 구의 기본 재질(에디터 화면·`SkyMaterial`이 빈 페이즈) ② 모든 사이트 행의 로비 `SkyMaterial` |
| 흙바닥 | `/Game/Maps/Outdoor/M_CL9_GroundDark` — 월드 좌표 노이즈 두 겹(8 m·1.5 m)으로 얼룩진 어두운 흙(`DirtDark`·`DirtLight` 파라미터), 거칠기 1. UV를 안 써서 바닥 판을 얼마나 늘려도 무늬가 안 늘어난다 | 각 레벨의 바닥 판(`Floor` 라벨 또는 `SM_Template_Map_Floor`) |
| 로비 조명 | `SunScale` 0.35 → 0.2 · `SkyLuminance` 1 → 0.3 · `bClouds` 끔 | 모든 사이트 행 로비 페이즈 ([../plan/data/site.md](../plan/data/site.md) 조명) |

- 만드는 곳은 `Tools/Unreal/outdoor_look.py` 하나다. `apply_outdoor_look.py`가 레벨 4개(`Lvl_Flat`·`Lvl_Henhouse`·`Lvl_Farm`·`Lvl_PigFarm`)와
  `MapTable` 전 행에 입히고, **새 스테이지는 `setup_stage.py`가 같은 함수를 불러 자동으로 입는다**([stage.md](stage.md)).
- 손으로 레벨을 새로 만들었으면 `apply_outdoor_look.py`의 `LEVELS`에 넣고 다시 돌린다.
- 밤·정전 페이즈 값은 안 건드렸다. 에디터 화면(PIE 밖)의 태양·하늘빛은 레벨 라이트 원래 값이라 여전히 낮만큼 밝다 — 그 값을 내리면
  밤 페이즈 배율(양계장 PIE로 맞춘 값)이 전부 어긋나서 그대로 둔다.
- 확인: `Tools/Unreal/qa_outdoor_shot.py`(PIE 로비에서 두 방향 스크린샷 → `Saved/Screenshots/WindowsEditor/`).

## 네트워크 정리

| 것 | 방식 |
|---|---|
| 정전 시각·발동 | 호스트 판정 → GameState Replicated → OnRep 연출 |
| 손전등 on/off | Server RPC → Replicated → 전 클라 표현 |
| 발광 아이템 | 운반 상태(품 여부)에서 파생 — 별도 복제 없음 |
| 몬스터 시야 전환 | 호스트 전용 (감각 판정 — 렌더와 무관) |

## 구현 시점

build-order **5c** (조명·손전등·정전)가 본 구현이지만, 정전 **판정**(시계·반납 가속)은
②(항아리 반납)와 함께 먼저 들어온다 — 연출 없이 값만 먼저 돈다.

## 참고

- 정전 시계 값: [../plan/data/rules.md](../plan/data/rules.md) "밤"
- 몬스터 시야: [monster.md](monster.md) · [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 정전 연출: [sequence.md](sequence.md)
