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
| 손전등 (`flashlight`) | **손에 드는 아이템** — 들면 지향성 스포트라이트 320u(960cm), 품에 넣으면 꺼짐 | [../plan/data/item/flashlight.md](../plan/data/item/flashlight.md) |
| 발광 아이템 (계란) | `LightRadius` 192cm. 품에 넣으면 꺼짐(`bLightLostInPocket`). 정전 때 소지자가 `CarrierSeenRadius` 276cm에서 몬스터에게 보인다 | [../plan/data/item/egg.md](../plan/data/item/egg.md) |
| 시설 조명 | 목적지 고정 조명 — 정전 시 소등 | site (연출 미정) |
| 스탠드 램프 (`standing_lamp`) | 놓으면 주변을 밝힘 — 조명 세팅 검토 후 값 확정 | equipment.md 미결 |

**발광 균형이 좁다** (plan/data/item/common.md): 발광이 세지면 손전등의 역할이 무의미해지고,
약하면 "들고 있으면 들킨다"가 사라진다. 값 조정은 이 문장을 기준으로.

## 손전등 (아이템)

- **손에 드는 물건이다** (2026-09-01 결정 — [../plan/data/item/flashlight.md](../plan/data/item/flashlight.md)).
  들면 빛나고, 품에 넣으면 꺼진다 — 계란을 들려면 어둠을 감수하거나 팀이 분업한다.
- 켜짐/꺼짐은 들고 있을 때 `IA_Flashlight` 토글 ([input.md](input.md)) — 상태는 아이템 액터에
  Replicated, 다른 플레이어 화면에서도 그 사람의 빛이 보인다.
- 어둠 속 동료 식별은 손전등 + 슬롯 색(`TintColor`)이 담당한다.
- 배터리·연료 시스템을 만들지 않는다 — 손전등의 카운터는 정전이다.

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
- 정전 후 광원은 발광 아이템과 스탠드 램프뿐이다 — "소리로 길을 찾는" 설계는 본편이 아니라
  별도 목적지의 몫이다 (plan/loop.md 귀환 절).

**구현 상태** (밤 수명주기 dc407d3 · session.md): 정전 시각은 `bBlackout` 복제 bool + OnRep이
아니라 `ACL9GameState.BlackoutServerTime`(복제) 하나로 두고, 각 머신이 서버시간과 비교해
`IsBlackout()`으로 파생한다 (반납 때 `RecordReturn`이 앞당김, DarkMin 하한). OnRep 트리거 대신
표현 측이 매 틱 `IsBlackout()`을 읽는다 — 조명이 이미 그렇게 한다(아래). `bNoBlackout`·시설
조명 소등·손전등 소등·정전 예고 연출·`SightDark` 전환은 아직 미구현(남은 5c·③ 감각 연동).

## 렌더 세팅 (어둠 톤 유지)

**구현: `ACL9NightLighting`** (5c 앞당김, 커밋 7dd22c3 · `Source/CL9/World/`). 레벨에 1개
놓이는 로컬 표현 액터로, `OnNightPhaseChanged`를 구독해 밤 페이즈에 맞춰 씬을 낮춘다:

- BeginPlay에 맵의 태양(DirectionalLight)·SkyLight authored 강도를 베이스라인으로 잡고,
  페이즈별 **배율**을 곱한다 (로비 0.35·밤 0.05·정전 0 초안 — 값은 액터 프로퍼티라 에디터에서
  튜닝, 리빌드 불필요). 밤엔 태양이 달빛 색으로도 전이. `TransitionSpeed`로 이징.
- 자동 노출(Eye Adaptation)은 **min=max 밝기 고정**(`ExposureLock`)으로 잠근다 — 어두운 곳에서
  화면이 저절로 밝아지지 않게. 액터가 소유한 `UPostProcessComponent`(bUnbound)에서 세팅하므로
  맵에 별도 PostProcessVolume이 필요 없다.
- 판정 아님 — 몬스터 시야는 화면 밝기가 아니라 값으로 판정한다(`SightIn`→`SightDark`, monster.md).
  ⑤ 맵 스포너가 생기면 이 액터를 정식 배치하고 배율 값을 재사용한다.

- Lumen GI·앰비언트·포스트프로세스 최소 밝기는 어둠 톤을 해치지 않는 값으로 세팅한다 (미결).
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
