# 조명 시스템 (손전등·어둠·정전)

> 어두운 게임을 위한 조명 시스템 — 손전등·발광 아이템·정전과 렌더 세팅.
> **광원 개체는 전부 아이템/장비 데이터**이고, 이 문서는 아이템이 아닌 부분(정전·렌더 세팅·
> 판정 연동)을 담당한다. 조명 값은 전역 원칙이 아니라 **이 시스템에서 세팅한다.**

## 설계 방향 (이 시스템의 세팅 — 전역 원칙 아님)

- 어둠이 게임의 기본 톤이다 — 손전등 도달 320u(960cm) 밖은 캄캄하게 세팅한다.
- **렌더와 판정은 분리다.** 몬스터 시야는 화면 밝기가 아니라 값으로 판정한다
  (`SightIn` 330 → 정전 시 `SightDark` 111 — [monster.md](monster.md)). 조명은 표현이다.
- 정전이 오면 손전등도 꺼진다 (plan/loop.md).

## 광원 목록

| 광원 | 값 | 출처 |
|---|---|---|
| 헤드램프 (`headlamp`) | **머리에 쓰는 장비** (CarrySlot `Head`) — 쓰면 손이 빈 채 시선 방향 스포트라이트, F 토글(F는 헤드램프 전용). **정전에 안 꺼진다**(내 장비지 시설이 아니다 — 2026-09-05) | [../plan/data/item/headlamp.md](../plan/data/item/headlamp.md) |
| 손전등 (`flashlight`) | **손에 드는 아이템** — 들면 지향성 스포트라이트 320u(960cm), 품에 넣으면 꺼짐 | [../plan/data/item/flashlight.md](../plan/data/item/flashlight.md) |
| ~~발광 아이템 (계란)~~ | **폐기 (2026-09-11)** — 빛은 손전등·헤드램프에서만 나온다. `egg` 행 `bEmitsLight` off. Glow 코드 경로·컬럼은 남아 있으나 쓰는 행이 없다 (decisions.md) | [../plan/data/item/egg.md](../plan/data/item/egg.md) |
| 시설 조명 | 목적지 고정 조명 — 정전 시 소등 | site (연출 미정) |
| 스탠드 램프 (`standing_lamp`) | 놓으면 주변을 밝힘 — 조명 세팅 검토 후 값 확정 | equipment.md 미결 |

**발광 균형이 좁다** (plan/data/item/common.md): 발광이 세지면 손전등의 역할이 무의미해지고,
약하면 "들고 있으면 들킨다"가 사라진다. 값 조정은 이 문장을 기준으로.

## 손전등 (아이템)

- **손에 드는 물건이다** (2026-09-01 결정 — [../plan/data/item/flashlight.md](../plan/data/item/flashlight.md)).
  들면 빛나고, 품에 넣으면 꺼진다 — 계란을 들려면 어둠을 감수하거나 팀이 분업한다.
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
1인칭 팔 소켓으로 옮겨 가면 빛도 따라간다(2026-09-13). 빔 방향 = 메시 +X; 상체가 시선 피치를
따르므로(ABP `AimPitch`) 아래를 보면 바닥이 비춰진다.

**헤드램프** (2026-09-05): 같은 아이템 광원 경로에 `CarrySlot::Head`가 얹힌다. E로 줍는 대신
`HostWear` → `ACL9Item::OnWorn`(HoldState `Worn`): 1인칭 카메라 컴포넌트에 부착(착용자 시선을
그대로 따름 — 타인 화면은 yaw만), 착용자에겐 메시가 안 보인다(OwnerNoSee), 충돌 없음.
`Server_Flashlight`(F)는 **쓴 램프만** 토글하고, 손 손전등은 `Server_UseStart`(좌클 누름)가 토글한다(2026-09-12). 정전 강제 소등(`ACL9FlashItem`의
ToggleLight·ShouldLightBeVisible·Tick)은 `IsWorn()`이면 건너뛴다. 미러는 `PlayerState.HeadItem`.

## 정전 (Blackout)

판정은 호스트, 연출은 각 클라:

```
호스트: 정전 시각 = DarkBase(185) − 반납 횟수 × DarkAccel(4), 하한 DarkMin(125)
  → GameState.BlackoutAtTime (Replicated — 반납 때마다 갱신.
     시각 기준은 GetServerWorldTimeSeconds — 클라가 그대로 카운트다운 가능)
  ↓ 시각 도달 (호스트 판정)
GameState.bBlackout = true (Replicated)
  ↓ OnRep (각 클라, 렌더 전용)
시설 조명 소등 + 손전등 소등 + 정전 연출 (sequence.md "blackout")
호스트: 몬스터 감각을 SightDark로 전환 (monster.md)
```

- `bNoBlackout` (progression.md 1일차 덮어쓰기) — 그 밤은 정전 없음.
- 정전 예고 연출(조명 깜빡임 등)은 미정 ([warning.md](warning.md)).
- 정전 후 광원은 헤드램프와 스탠드 램프뿐이다(발광 계란은 2026-09-11 폐기) — "소리로 길을 찾는"
  설계는 본편이 아니라 별도 목적지의 몫이다 (plan/loop.md 귀환 절).

**구현 상태** (밤 수명주기 dc407d3 · session.md): 정전 시각은 `bBlackout` 복제 bool + OnRep이
아니라 `ACL9GameState.BlackoutServerTime`(복제) 하나로 두고, 각 머신이 서버시간과 비교해
`IsBlackout()`으로 파생한다 (반납 때 `RecordReturn`이 앞당김, DarkMin 하한). OnRep 트리거 대신
표현 측이 매 틱 `IsBlackout()`을 읽는다 — 조명(NightLighting)과 손전등이 그렇게 한다. **손전등
소등 구현됨**(아래 손전등 절 — `IsBlackout`이면 빔 강제 off, 발광 아이템은 유지). 테스트는
`cl9.blackout` 치트로 정전을 즉시 트리거. `bNoBlackout`·시설 조명 소등(평지엔 시설 조명 없음,
⑤)·정전 예고 연출·`SightDark` 전환은 아직 미구현.

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
| 옅은 **안개** — `FogDensity` 0.01 + 짙은 남색 `FogColor` | 사이트 행 + 레벨의 `ExponentialHeightFog`(볼류메트릭 켬) | 손전등 빔이 공기 중에 보이고, 먼 것은 뿌옇게 사라진다 |
| 정전 — `SunScale` 0 · `SkyScale` 0 · `SkyLeak` 0, 안개는 검정 | 사이트 행 `BlackoutLighting` | 앰비언트가 꺼져 다시 캄캄 (PIE 실측: 완전 검정) |

두 가지 실측 함정이 값의 모양을 정했다 (site.md 조명 절에 자세히): **루멘은 천장 덮인 방에
하늘빛을 한 점도 안 넣는다** — SkyLight `Cast Shadows`를 꺼도 마찬가지 — 그래서 `SkyLeak`
(Lumen Skylight Leaking)이 실내 앰비언트의 전부다. 그리고 **SkyLight가 실시간 캡처라 하늘이
태양을 따라 어두워진다** — 밤 `SkyScale`이 10인 건 그 어두운 하늘 위에 곱하는 값이라서다.

`ExposureLock`(1.0)은 건드리지 않는다 — 노출을 낮추면 손전등 빔과 정전까지 같이 밝아져 톤이
무너진다. 09-07에 같은 이유로 밤 배율을 0.8/1.2까지 올렸다가 원복한 이력이 있다(전역 밝기로
풀면 정전이 의미를 잃는다). 축사 통로마다 시설 조명(작업등)을 놓고 정전 때 끄는 구조는 여전히
미구현이며, 앰비언트·안개는 그 위에 깔리는 바닥 톤이다.

- 감마 슬라이더 범위는 이 세팅과 함께 정한다 ([settings.md](settings.md)).

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
