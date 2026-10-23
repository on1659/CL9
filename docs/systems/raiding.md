# 원정(습격) 시스템

> 목적지에 침입해 재료를 회수하고 탈출하는, 게임의 심장.
> **설계 정본은 plan이다** — [../plan/loop.md](../plan/loop.md) ·
> [../plan/data/site.md](../plan/data/site.md) · [../plan/data/item/common.md](../plan/data/item/common.md) ·
> [../plan/data/rules.md](../plan/data/rules.md).

## 개요

- **침입 · 회수 · 탈출.** 재료의 주인이 곧 위협이다.
- **재료는 월드에 놓인 물건이다.** 채집 노드·상호작용 게이지가 아니라, 바닥과 케이지에
  실제로 놓인 계란을 손으로 줍는다.
- **트럭은 같은 맵 안에 있다.** 원정지로 "이동"하는 별도 레벨·로딩이 없다 ([map.md](map.md)).
- **밤은 시드에서 만들어진다.** 레이아웃을 복제하지 않고 시드만 보내서 각자 만든다.

## 재료 회수 — 물리 아이템

채집이 아니라 **운반**이다. 상세 조작은 [inventory.md](inventory.md)(운반 시스템).

- **줍기**: `PickRadius`(120cm) 안의 아이템을 줍는다. 걸으면서 주우면 조용하고
  (`PickupQuietRadius` 150), 뛰다가 주우면 시끄럽다 (`PickupLoudRadius` 390).
- **놓기 / 던지기**: 던지기는 차징이다 (`ThrowMinSpeed` 600 → `ThrowMaxSpeed` 1680,
  만충 0.85초). 던지기 원점에 점프 높이를 더하지 않는다.
- **깨짐**: 계란은 충격 속도로 깨진다 — `CrackSpeed` < `CrackedBreakSpeed` < `BreakSpeed`
  순서를 로드 검사가 확인한다. 깨짐은 의도된 마찰이다 — 전력질주가 위험해야 한다.
- **항아리 반납**: 어느 셀에서든 항아리가 1500u 안에 있다 — "도망칠 곳은 항상 있다"의
  기계적 보증. **반납 1회가 정전 시계를 `DarkAccel`(4초)만큼 앞당긴다.**

## 아이템 스폰 (밤 시작 시, 호스트)

목적지의 `ItemSpawns`(기준값) × `ItemCountCurve`(일자 배율)로 개수를 정하고,
시드가 위치를 정한다. 양계장 초안: 계란 16~20 + 고기 8~12.

```cpp
// GameMode — 밤 시작 시 1회
if (!HasAuthority()) return;
for (const FCL9SpawnEntry& Entry : Site->ItemSpawns) {
  int32 Count = RollCount(Entry, Progression->ItemCountCurve, Day);
  // 시드 기반 생성기가 뱉은 좌표에 ACL9Item 스폰 + SetAlias(Entry.Alias)
}
```

## 몬스터 스폰 (밤 시작 시, 호스트)

**시간 경과 확률 스폰이 아니다.** 밤 시작 시 마릿수를 굴려서 배치하고, 그 마릿수로 밤을 치른다.

- 개수: `MonsterSpawns`(기준값, 닭 8~12) × `MonsterCountCurve`(일자 배율)
- `ThreatActMinRadius`(420cm) 안에서는 위협이 깨어나거나 스폰되지 않는다
- 깨어남에는 예고가 있다 — `WakeTelegraph` 0.8초 퍼덕임, `WakeSightRadius` 안에 시선이
  있으면 기상 보류 ([../plan/data/monster/common.md](../plan/data/monster/common.md))
- 리스폰 없음 — 사살하면 그 밤에는 줄어든다 (사살 경보 `KillAlarmRadius`가 대가다)

**구현 (4b, 2026-09-05)**: 위 두 스폰 모두 `ACL9GameMode::SpawnNightPopulation` — 목적지 행
(`MapTable` henhouse)의 기준값 × `CL9Progression` 곡선. 좌표는 생성기가 없어 평지
스탠드인(트럭 패드 앵커 원환, 몬스터 `CL9Rules.FlatMonsterMinDistance`~`FlatMonsterMaxDistance` 4000~7000cm(2026-10-21, 옛 900~2400) · 재료 400~1800cm, `ThreatActMinRadius` 안 회피).
밤 스폰분은 로비 복귀 때 철거된다(들고 나온 것은 남는다). 상세 [balance.md](balance.md).

**스테이지와 구역 (2026-10-03)**: 게임 모드는 `StartPlay`에서 지금 레벨을 `Level` 열에 가진 사이트 행을 고른다
(없으면 `DefaultSiteAlias`). 사이트에 깊이 구역(`Zones`)이 있으면 구역마다 자기 깊이 띠의 지점에 자기 몬스터·재료를
놓는다 — 로그 `Night N zone <이름> (…)`. 밤 시계(`NightLength`·`DarkBase`·`DarkMin`)도 사이트가 덮어쓸 수 있다.
정본 [../plan/data/site.md](../plan/data/site.md).

**구역 사이트와 나눠 스폰 (2026-10-04, [../goal/giant-stage.md](../goal/giant-stage.md))**: 스폰은 지어진 맵마다 돈다
(`ACL9SiteMap::FindAll`, 구역 순) — 맵마다 자기 지점, 자기 구역 행(`GetZones()`, 구역 행이 비면 사이트 `Zones`), 자기 항아리
중복 제거 집합. 사이트 열(`MonsterSpawns`·`ItemSpawns`)도 맵마다 더해진다. 단일 사이트는 맵이 하나라 예전과 같고, 로그에만
구역이면 `Night N district i …` 가 붙는다. 재료·항아리는 밤 시작에 바로 놓고, **몬스터는 밤 시작에 자리·방향까지 정해 큐에 넣은 뒤
다음 프레임부터 `CL9Rules.MonsterSpawnPerFrame` 마리씩 스폰한다** — 로그 `… ×P queued (…)` 와 큐가 빈 뒤 `Night N: S of Q queued monsters
spawned, F per frame, T s.` 실제 프레임 번호로 상한을 지키며, 큐가 있는 동안에만 GameMode 틱이 켜진다. 상한은 성공 수가 아니라
큐 소비/스폰 시도 수다. 기다리는 동안 플레이어가 후보 위치의 조용한 반경 안으로 들어오면 후보를 버린다(다시 추첨하지 않는다).
위치·방향은 FIFO 순서로 보존하고 충돌 처리는 기존 `AdjustIfPossibleButAlwaysSpawn`을 쓴다. 아이템·항아리가 먼저 존재하므로 충돌 보정 결과와
몬스터 개체 초기화의 전역 랜덤 소비 순서는 일괄 스폰 때와 달라질 수 있다. 지점·깊이·개수 분포 규칙은 유지하며 동일 시드의 완전히 같은 개체 배치를 보장하지 않는다.
밤 시작은 호스트에서 구역 짓기(`ACL9SiteMap::IsBuildPending`)나 내비 빌드(더러운 영역 포함)가 진행 중이면
다음 프레임으로 미뤄지고(여전히 로비), 기다린 시간이 `StartNight: … done after T s of waiting` 로 남는다.
중복 시작 요청은 대기 하나를 공유한다. 로비 복귀·새벽·새 판·로드·월드 종료는 시작 대기와 남은 큐를 취소하며, 큐 도중 맵 시드나 일차가
바뀌면 이전 위치는 버린다. 규칙 행이 사라지거나 상한이 1 미만이면 남은 큐를 취소하고 오류를 기록한다.

## 소리 규칙

몬스터가 듣는 소음 반경은 `CL9Rules`의 값이다 ([../plan/data/rules.md](../plan/data/rules.md)).

- **걷기는 어그로 0이다** — 걸으면 안 들킨다는 게 기본 안전 수단이다 (예외: 거위 `WakeRadius`).
- 달리기 390 / 조용한 줍기 150 / 시끄러운 줍기 390 / 밀치기 255 / 미끼 330 (cm)

## 탈출과 밤 종료

- 밤 시계는 `NightLength`(245초) 하나 — 만료는 새벽 실패다 ([balance.md](balance.md))
- 정전이 오면 손전등도 죽는다 — 정전 시계는 반납이 앞당긴다
- 트럭 패드가 유일한 회복 채널이다. 죽으면 그 순간부터 **데스캠**(자기 시체를 3인칭으로 보는 폰의
  `DeathCamera`, 2026-09-20 밤)이고, `SpectateDelay`(초안 3초) 뒤 동료 관전으로 넘어간다 — 혼자면 데스캠이
  밤 끝까지 이어진다. 전원이 죽으면 전멸 — 밤이 그 자리에서 끝난다 (4a, [character.md](character.md) 죽음·데스캠 절)

## 네트워크 동기화

- **스폰·깨짐·잡힘 판정**: 호스트 전용 (`if (!HasAuthority()) return;`)
- **맵 레이아웃**: 시드만 Replicated — 각 클라가 같은 시드로 같은 맵을 만든다
- **아이템·몬스터 상태**: 각 액터의 Replicated 변수 (conventions.md § 4)
- **줍기/던지기 입력**: Server RPC

## 참고

- 맵 생성·항아리 배치: [map.md](map.md)
- 손전등·정전: [light.md](light.md)
- 운반(퀵슬롯·수레): [inventory.md](inventory.md)
- 몬스터 AI (공용 상태 기계): [monster.md](monster.md) — 값은 [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 드롭: [drop.md](drop.md)
