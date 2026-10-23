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
| 시설 조명 | **구현됨 (2026-09-16)** — 2026-10-16부터 빛 값은 룩 표 램프 종류(`LampKind`, [../plan/data/look.md](../plan/data/look.md)), 아래는 옮기기 전 기록. 모듈 표 소품(`FCL9PropPlacement`)의 `LightIntensity`(cd, 0이면 빛 없음)·`LightRadius`·`LightColor`·`LightOffset`. 배치기(`ACL9SiteMap::PlaceProps`)가 그 소품 인스턴스마다 그림자 없는 포인트 라이트를 붙이고, 정전이면 `Tick`이 끈다. 실내 모듈의 램프는 판 시작의 전원 단계도 따른다(아래 [전원 단계](#전원-단계--판-시작의-첫-어둠-2026-10-06) 절, 2026-10-06). 양계장은 작업등(`SM_Henhouse_WorkLamp_A`)이 이 값을 든다(200cd 초안 → **10cd**, 09-17 PIE: 200은 밤 실내를 로비 밝기로 만들었다). **개수**는 사이트 행 `LampDensity`(0~1)로 조절한다 — 등 몸체는 다 서고 그 비율만 켜진다(양계장 0.5, 2026-09-18). 2026-09-17 컬럼 셋 추가: `LightScattering`(안개 후광), `LightFlicker`(진폭 0~1, 세기가 1±진폭×펄린 노이즈)·`LightFlickerSpeed`(Hz) — 배치기 `TickFlicker`가 켜진 램프만 흔든다, 위상은 자리에서 나와 시드가 같으면 같다. 마당 모듈의 모닥불(`SM_Campfire_A`)도 같은 컬럼(12cd·700cm·주황·산란 4·깜박임 0.35/5Hz)이고 정전이면 같이 꺼진다(사용자 결정). 불꽃은 메시의 발광 슬롯(`M_CL9_Campfire_Flame` — 재질 파라미터 `FlameStrength` 기본 2.0, Blender 미리보기는 3; `Tools/Unreal/import_campfire.py`) | ModuleTable PropSets |
| 스탠드 램프 (`standing_lamp`, `ACL9LampItem` — 내려놓아도 꺼지지 않는 `ACL9LightItem` 파생) | 놓으면 주변을 밝힘 — 조명 세팅 검토 후 값 확정 | equipment.md 미결 |

**발광 균형이 좁다** (plan/data/item/common.md): 발광이 세지면 손전등의 역할이 무의미해지고,
약하면 "들고 있으면 들킨다"가 사라진다. 값 조정은 이 문장을 기준으로.

## 손전등 (아이템)

- **손에 드는 물건이다** (2026-09-01 결정 — [../plan/data/item/flashlight.md](../plan/data/item/flashlight.md)).
- **스위치는 토글만 바꾼다 — 켠 것은 켜진 채, 끈 것은 꺼진 채, 들고 있는지와 무관하다** (2026-10-17 사용자 "무전기랑 유사" —
  [../decisions.md](../decisions.md)). 2026-09의 "다른 칸으로 전환하면 실물이 없어져 꺼진다"(빛과 짐을 동시에 못 든다는 빛 경제)를
  번복했다. 켠 채 다른 칸으로 가거나 다른 것을 주우면 손전등은 비활성 칸 데이터(`FCL9CarriedItem.bLightOn`)로 "켜진 채" 남고,
  그동안 빛은 캐릭터의 **주머니 빛**이 그린다 — 머리(쓴 헤드램프와 같은 카메라 자식 자리)에서 시선 방향 스포트, 값은 룩 표 램프 종류
  `PocketLight`([../plan/data/look.md](../plan/data/look.md) 3). 다시 꺼내면 저장된 스위치대로 켜진 채 나오고, 켠 채 놓거나 던지거나
  죽어 흘려도 바닥에서 계속 비춘다(리썰과 같다). 끄려면 그 칸으로 가서 좌클.
- 켜짐/꺼짐은 들고 있을 때 좌클 `IA_Use`(행 `UseAction`=`ToggleLight`) 토글 ([input.md](input.md), 2026-09-12) — 상태는 아이템 액터에
  Replicated, 다른 플레이어 화면에서도 그 사람의 빛이 보인다. 주머니 빛의 켜짐은 모든 머신이 복제된 `PlayerState.Slots` 에서 파생한다 — 새 RPC 없음.
- 어둠 속 동료 식별은 손전등 + 슬롯 색(`TintColor`)이 담당한다.
- 배터리·연료 시스템을 만들지 않는다 — 손전등의 카운터는 정전이다.

**구현** (5c → 2026-09-11 클래스 분리): 빛을 내느냐는 **클래스**가 정한다 — `ACL9Item`(빛 없음)
→ `ACL9LightItem`(라이트 컴포넌트·`bLightOn` 복제·들면 켜짐, 점광원) → `ACL9FlashItem`(스포트라이트,
F 토글, 정전 규칙). 행의 `ActorClass`가 `BP_CL9FlashItem`을 가리키면 손전등이 되고, 광원 수치
(세기·거리·원뿔·산란·색)는 행의 `LampKind` 가 고른 룩 표 램프 종류에서 읽는다(2026-10-16 — [../plan/data/look.md](../plan/data/look.md)). F(`IA_Flashlight`)→
`Server_Flashlight`→`ACL9FlashItem::ToggleLight`→`bLightOn` 복제(각 머신 OnRep로 빔 표시).
`OnAcquired`/`OnReleased` 는 스위치를 건드리지 않는다(2026-10-17 — 옛 "들면 off, 놓으면 off" 폐기); 칸에 넣을 때 `HostStowHeld` 가
`bLightOn` 을 칸에 적고, `HostSelectSlot`·죽음 흘림이 `HostSetLightOn` 으로 되돌린다. 주머니 빛은 `ACL9Character::PocketLight`
(`UpdatePocketLight`, 매 틱 모든 머신 — 비활성 칸 중 켜진 것이 있으면 보인다). 정전이면 flashlight·주머니 빛 모두 강제 off(호스트는
칸의 스위치도 끈다). 광원은 `MeshComponent`의 자식으로 렌즈 앞(+X 끝)에 붙어 소유자 화면에서 메시가
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
시설 조명 소등 (`ACL9SiteMap::Tick` → `SetLampGroup`, 실내·실외 무리 모두) + 스위치 불빛 off + 손전등 빔 강제 off (착용 헤드램프 예외) + 정전 연출 (미정)
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

## 전원 단계 — 판 시작의 첫 어둠 (2026-10-06)

판이 열리면 건물 안이 캄캄하고 첫 실내 방의 전원 스위치 하나만 빛난다. 누가 켜면 실내 램프가 모두 켜졌다가 깜빡이며
죽는다 — 죽은 뒤는 첫 어둠보다 밝다(판의 행 그대로). 명세 [../goal/round-start-truck-intro.md](../goal/round-start-truck-intro.md)
K6~K8, 스위치 칸은 [map.md](map.md) "전원 인트로".

```
단계 ECL9PowerStage — 복제하지 않는다. 각 머신이 ACL9GameState::GetPowerStage() 로 파생
  None      판(Night)이 아니거나 이 월드에 스위치가 없다 (주 사이트 맵 HasPowerSwitch false — Lvl_Flat)
  Dark      판이 열렸고 아직 아무도 안 켰다 (PowerOnServerTime == 0)
  LightsOn  켠 순간 T 부터 IntroLeadIn + IntroFlyTime 동안
  Flicker   그 뒤 IntroFlickerTime 동안
  Dead      그 뒤 판 끝까지
정전(IsBlackout)은 단계와 따로다 — 모든 단계를 덮는다
```

| 무엇 | None | Dark | LightsOn | Flicker | Dead | 정전 |
|---|---|---|---|---|---|---|
| 실내 램프 (소품의 모듈이 `!bOutdoor`) | 켜짐 | 꺼짐 | 켜짐 | 깜빡임 | **꺼짐 — 생존 램프(`LampSurviveShare`)만 켜짐** (2026-10-15 룩 개편) | 꺼짐 |
| 실외 램프 (마당 모닥불) | 켜짐 | 켜짐 | 켜짐 | 켜짐 | 켜짐 | 꺼짐 |
| 스위치 불빛 (`ACL9PowerSwitch::SetGlow`) | 꺼짐 | 켜짐 | 꺼짐 | 꺼짐 | 꺼짐 | 꺼짐 |
| `SkyLeak` (`ACL9NightLighting`) | 페이즈의 행 | **0** — 판의 행에서 이것만 | 판의 행 | 판의 행 | 판의 행 | 정전 행 |

- **무리** — `ACL9SiteMap`이 램프마다 무리(실내·실외)를 `LampComponents`와 나란한 `LampGroups`에 들고, 틱마다 두 무리의
  상태와 스위치 불빛을 **한 번씩** 계산해 바뀔 때만 램프를 건드린다([performance.md](performance.md) 틱 — `SetVisibility` 매
  프레임 금지). 노이즈 깜빡임(`TickFlicker`)은 자기 무리가 켜져 있을 때만 돈다. 로비·새벽(None)은 예전처럼 정전까지 켜짐.
- **깜빡임** — 박자 = `floor((now − T) × 6)`. 홀수 박자는 늘 꺼짐이고, 짝수 박자는 Flicker 구간이 지난 비율만큼 켜질 확률이
  1 → 0 으로 떨어진다(박자 번호의 해시와 비교 — 모든 머신이 같은 순간에 같은 박자). 켜짐이 짝수 박자에만 오므로 **큰 번쩍임은
  어느 1초에도 3번 이하**다. 광과민성 상한이라 6 Hz 는 표 값이 아니라 코드 상수(`IntroFlickerBeatsPerSecond`)이고, 구간의
  시작·길이는 단계를 파생하는 것과 같은 규칙 행 값(`IntroLeadIn`·`IntroFlyTime`·`IntroFlickerTime`)이다.
- **`SkyLeak` 스냅** — Dark 로 들어가거나 나오는 가장자리에서 `SkyLeak`만 보간 없이 바로 맞춘다. 판이 열리는 순간 실내가 한 번에
  캄캄해지고, 스위치를 켜는 순간 한 번에 돌아온다(서서히 밝아지면 "불이 들어왔다"가 안 읽힌다). 늦게 들어온 사람은
  `TryBindGameState`가 전부 스냅한다. 태양·하늘·안개는 예전처럼 `TransitionSpeed`로 보간한다.
- **구역** — 단계는 월드에 하나라, 구역 사이트도 모든 구역의 실내 램프가 주 사이트 맵(구역 0)의 스위치 하나를 따른다.
- 표현만이다 — 몬스터 시야 판정은 여전히 값이다.

## 마당을 건너는 길 (2026-10-12, 7j — [../goal/loop-7j-round-start.md](../goal/loop-7j-round-start.md))

- **안개**: 전원을 켜기 전(첫 어둠) `ACL9NightLighting::SelectTarget` 이 사이트 밤 안개 밀도에 `CL9Rules.PrePowerFogScale`(초안 2)을 곱한다 — 켜면 판의 행으로 돌아간다(실측 0.10 → 0.05).
- **실외 안개** (2026-10-16): 판(정전 포함)에서 이 머신의 카메라가 입구면(트럭 패드 공간 X = 사이트 `EntryOffset`)보다 트럭 쪽이면 `ACL9NightLighting::OutdoorWeight` 비율만큼 안개 밀도에 `CL9Rules.OutdoorFogScale`(초안 0.06)을 곱한다 — 입구면 앞 `OutdoorFogBlend`(600 cm) 동안 섞고, 보간은 기존 `TransitionSpeed` 를 탄다. 안개 액터가 하나라 영역이 아니라 카메라 위치로 가른다. 실외 지형은 `Tools/Unreal/setup_outdoor.py`(랜드스케이프, 명세 `Tools/Unreal/outdoor/<site>.json`; 랜드스케이프 생성 자체는 C++ `UCL9LandscapeTools::CreateLandscape`, `Source/CL9/World/CL9LandscapeTools.h`).
- **문 위 작업등**: `ACL9YardTension`(머신마다 로컬, 복제 없음)이 판마다 생성된 배치에서 마당(실외 모듈·격자 밖)과 실내 모듈 사이의 문(1층 `Door` 변)을 찾아
  문 바깥 위 `YardLampHeight` 에 `YardLampMesh` 와 점광(세기·산란은 룩 표 램프 종류 `YardDoor` 의 `FCL9LampKind::Intensity`·`Scattering` — 2026-10-16 정정: 옛 셀 `YardLampIntensity` 는 없다, `CL9YardTension.h` `YardDoorLampKind`; 그림자 없음)을 건다. 판 동안만 켜지고 끊기듯 깜박인다(노이즈가 −0.25 아래면 거의 꺼짐).
  마당 모듈에는 등 기둥이 없다(모닥불뿐) — 문마다 거는 쪽이 배치에 상관없이 맞는다.
- **보이기만 하는 기척**: 전원 전, 로컬 플레이어가 마당에서 문을 바라보면(거리 600 ~ `SilhouetteRange`, 시선 내적 ≥ 0.9) 문마다 판에 한 번 어두운 실루엣(`Silhouette*` — 몸·걷기 애님·검은 재질)이
  문 안쪽 260 cm 를 문 폭 방향으로 700 cm 건넌다. 뒤에서 비추는 빛(`SilhouetteBackLight`)이 윤곽을 만든다. **창문이 키트에 없어 문간이다**(사용자 답의 "창문"에 가장 가까운 자리).
- **울음**: 전원 전 마당에 있는 동안 `YardCryInterval*` 초마다 가까운 문 안쪽 7~13 m 에서 소리 행 `YardCry`(S_MonsterNear) — 로컬, 판정 없음. 도착 카메라 동안은 울음·실루엣을 시작하지 않는다.

## 렌더 세팅 (어둠 톤 유지)

> **2026-10-15 룩 개편** — 아래 표의 앰비언트(`SkyLeak` 1)·안개(0.05, 남색) 값은 바뀌었다. 지금 값과 그 이유는 아래 [룩 개편](#룩-개편--검정--빛-웅덩이-레트로-화면-2026-10-15) 절이 정본이다.

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
| 정전 — `SunScale` 0 · `SkyScale` 0 · `SkyLeak` 0, 안개는 검정 | 사이트 행 `BlackoutLighting` → 2026-10-16 정정: 룩 표 조명 프로필의 `Blackout` 페이즈(`FCL9LightingPhase`, `CL9LookDef.h`) | 앰비언트가 꺼져 다시 캄캄 (PIE 실측: 완전 검정) |
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

## 룩 개편 — 검정 + 빛 웅덩이, 레트로 화면 (2026-10-15)

> 2026-10-16: 이 절의 값(화면·조명 프로필·램프·퀵슬롯·구조면)은 전부 룩 표 한 곳으로 옮겼다 — [../plan/data/look.md](../plan/data/look.md).
> PIE 를 켠 채 그 표를 고치면 바로 보인다(실시간 모드).

명세 [../goal/look-rework.md](../goal/look-rework.md). 사용자가 봇 영상(`map_wave_20261015g`)을 리썰과 비교해 "화면이 못하다"고 한
데서 시작했다. 실측 원인은 (1) `SkyLeak` 1 의 고른 청색 앰비언트가 실내를 한 색조로 채우고, (2) 작업등이 그림자를 안 드리워 벽에 형태가
없고, (3) 안개 0.05 는 20 m 에서도 투과율 0.9 라 먼 복도까지 다 보이고, (4) 전원 인트로 뒤 램프가 전부 죽어 판 내내 빛 웅덩이가 없고,
(5) 검정 안개가 하늘 앰비언트 기여로 푸르게 물들었다는 것. 사용자 결정 여섯(검정+빛 웅덩이 · 리썰식 저해상도+단계화 · 구조면 텍스처 ·
복도 소품/퀵슬롯/봇 헤드램프 · 천장 확인(있었다 — 바닥 120 칸에 천장 115) · Dead 뒤 램프 일부 생존)을 패널 세 관점 + 교차 심문으로 다듬었다.

**어둠의 장치 = 작은 평탄 누수 × 검은 안개 투과율.** 루멘 하늘빛 누수는 카메라 거리와 무관하게 모든 면에 같은 양이 더해진다
(`LumenTracingCommon.ush`), 거리감은 안개 투과율만 만든다. 그래서 밤 `SkyLeak` 1 → **0.5**(잔여 — 0 은 안 된다: 규칙 "손전등 밖은
실루엣만", 동료 슬롯 색 식별, 몬스터 예고 자세, 그리고 Dead 가 첫 어둠보다 밝아야 한다는 K7. PIE 실측 2026-10-15: 누수는 `saturate(히트거리/1000cm)` 로
깎여 좁은 복도에서는 0.15 도 1.0 도 거의 검정이고 — 복도의 빛은 램프와 헤드램프가 전부다 — 0.5 는 큰 방·문가에 약한 잔광을 남긴다), `FogDensity` 0.05 → **0.5**(밀도는
`FogDensity/1000` per cm; 1.0 은 마당이 첫 어둠 ×2 로 회색 안개 벽이 됐다), `FogColor` 검정, 새 컬럼 **`FogSkyAmbient`** 0(`SkyAtmosphereAmbientContributionColorScale`
— 이것이 1 이면 안개 색을 검정으로 둬도 먼 안개가 하늘색이 된다), 밤 `SkyScattering` 0. 판정은 같은 프레임의 비율로 한다: 누수만
받은 20 m 벽은 단계화 최하 단계 아래, 헤드램프가 비춘 18 m 벽은 읽힘, 램프 웅덩이는 누수 벽의 10 배 이상. 실측 컷은 `Saved/Look/*.png`
(스위치 방: 작업등 웅덩이·나무결·검정으로 사라지는 복도·멀리 등의 후광).

**램프.** (2026-10-16 정정: 아래 소품 컬럼 `LightConeAngle`·`SourceRadius` 는 룩 표 램프 종류 `FCL9LampKind::ConeAngle` 로 옮겼다 — 동작 설명은 그대로 유효하다.) 소품 컬럼 **`LightConeAngle`** — 0 이면 전처럼 그림자 없는 포인트(모닥불), 0 보다 크면 소품 로컬 −Z(아래)를 향하는 스포트
반각이고 **그림자를 드리운다**(VSM 한 면, `SourceRadius` 0, 볼류메트릭 그림자는 끔 — 그림자 램프마다 안개 주입 패스가 하나씩 더 돈다).
작업등 29 자리 전부 60. 거리로 그림자를 켜고 끄지 않는다(`SetCastShadows` 토글은 라이트 프록시를 다시 만들어 VSM 페이지를 버리고,
그림자 없는 램프는 벽을 뚫고 루멘에 번진다) — 예산을 넘으면 `MaxDrawDistance` 로 라이트째 페이드한다. 사이트 컬럼 **`LampSurviveShare`**
(양계장 0.3): 켜진 실내 램프 가운데 시드로 고른 몫이 무리 `IndoorSurvivor` 가 되어 Dead 단계에도 정전 전까지 켜져 있다 — 같은 시드면
모든 머신에서 같은 램프. 깜빡이다 죽는 연출은 그대로다. [performance.md](performance.md) 광원 규칙을 함께 고쳤다. 실측(2026-10-15, 스위치 방, 레트로 켬): 60 fps 캡, GPU 그래픽 3.6 ms + 컴퓨트 2.3 ms, ShadowDepths 0.22 ms, VolumetricFog 0.18 ms — 예산 안.

**레트로 화면 (설정 `bRetroLook`, 기본 켬).** 저해상도 몫은 콘솔 변수 — `r.ScreenPercentage.MaxResolution` = `LookTable.MaxResolution`(2026-10-16 정정: 옛 `CL9Rules.LookMaxResolution` 등 `Look*` 셀은 룩 표 `UCL9LookDef` 로 옮겨 접두사가 빠졌다)
(520, 리썰 860×520; 창 크기와 무관) · `r.Upscale.Quality` 0(최근접) · `r.AntiAliasingMethod` = `AntiAliasing`(0 없음 / 1 FXAA — TSR 은
픽셀을 시간 누적으로 지운다). `UCL9GameUserSettings::ApplyLook` 이 처음 덮기 전 값을 적어 두고 `ECVF_SetByGameSetting` 으로 덮으며,
토글 끔·로컬 PlayerController EndPlay 에서 되돌린다(전역 변수라 PIE 가 끝나면 에디터 뷰포트까지 저해상도가 된다 — PIE 에선 에디터의
상한 덮어쓰기 때문에 `r.ScreenPercentage` 100 도 같이 박는다). 후처리 몫은 `UCL9LookModifier`(`ACL9CameraManager` DefaultModifiers) —
톤매퍼 그레인 `Grain` 과 `M_CL9_PP_Posterize`(After Tonemapping, 밝기를 `PosterizeBands` 단계로 자르고 `PosterizeMix` 로
원래 색과 섞음, 색조 유지 — 16 단계 · 0.5: 12 단계 · 0.7 은 빛 둘레에 동심원 띠가 강했다)를 WeightedBlendable 로 건다. 카메라 매니저에 두는 이유: 타이틀·로비·도착 카메라·데스캠·관전까지 폰이
없는 화면에도 걸린다(1인칭 카메라의 PostProcessSettings 는 폰과 함께 사라진다). HUD 는 업스케일 뒤에 그려져 선명하다. 콘솔 변수는
프로세스에 하나라 한 프로세스 PIE 의 클라들은 호스트 값을 같이 본다.

**구조면 텍스처.** `M_CL9_Structure`(Structure/Look) — 정점색 × `BaseColor` 파라미터(`UseVertexColor` 스위치: 정점색 조각은 켬, 벽·바닥처럼
상수색 조각은 끄고 그 색을 인스턴스에), static switch **`SurfaceDetail`** 이 켜지면 월드 정렬 트라이플래너 디테일 노멀(접선 공간 끔) +
거칠기(`Tiling`·`NormalStrength`·`RoughnessMin/Max`), 끄면 정점 노멀 + `RoughnessBase`. 구조 재질 16 개마다 `MI_CL9_Struct_*` 인스턴스
(가족: 나무 `T_WoodGrain_N/ORM` 150 cm 세기 0.5 · 콘크리트 노멀 없이 `T_Grunge_M` 거칠기만 · 금속 `T_PaintedMetal_N/ORM`) 를 만들어
구조 메시 9 개의 슬롯에 넣었다(`Tools/Unreal/create_look_materials.py`). 소품이 공유하는 `M_CL9_HH01_*` 원본은 그대로다(들고 다니는
소품에 월드 정렬 무늬가 미끄러진다). 되돌리기 = `SurfaceDetail` 끄기(비용 0). 판정: 빛 웅덩이 안 벽 한 면이 단계화 두 단계를 넘어
얼룩지면 노멀 세기를 내리거나 거칠기만 남긴다. design-concept.md 규칙 3 완화는 사용자 결정(decisions.md 2026-10-15).

**복도 소품·퀵슬롯·영상.** 복도 계열 소품 세트는 [../plan/data/module.md](../plan/data/module.md) 복도 항목, 퀵슬롯 밝기 70 % / 25 % 는
[ui.md](ui.md), 봇은 손전등 대신 헤드램프(재료를 주우면 손전등 칸이 바뀌어 꺼지고 정전에도 꺼진다)를 쓴다(cl9film SKILL).

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
| 전원 단계·스위치 불빛 | 호스트가 켠 시각(`PowerOnServerTime`)만 복제 → 각 머신이 단계를 파생해 램프·불빛·`SkyLeak`를 로컬로 (Multicast 없음) |
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
