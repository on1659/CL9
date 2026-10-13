# 맵 생성 시스템

> ## 2026-09-08: 격자 미로를 버리고 **모듈 조립**으로 갈아엎었다
> 사용자 확인 — "미로 형식이라는 게 진짜 미로를 만들라는 게 아니라, **미로처럼 매번 맵이
> 달라지는 걸** 만들어 달란 거야. 가구도 배치하고." CCL8에서 넘어온 통로·케이지 열·플러그
> 격자 미로(Kruskal·`OpenH`/`OpenV`)와 층 구조(1층/지하 + 경사로)를 **지웠고**, R.E.P.O.
> 방식의 모듈 조립 생성기가 그 자리에 들어갔다.
> **설계 정본: [../plan/data/module.md](../plan/data/module.md)** ·
> 참고 분석 [../reference/repo/repo-map.md](../reference/repo/repo-map.md) ·
> [../reference/lethal/lethal-map.md](../reference/lethal/lethal-map.md)
>
> 살아남은 것: 시드 복제 구조 · 조각 표(`UCL9PieceDef`) · `ACL9SiteMap`의 타일링 배치.

> 밤마다 시드에서 만들어지는 원정지. **설계 정본은 plan이다** —
> [../plan/data/site.md](../plan/data/site.md) · [../plan/data/segkind.md](../plan/data/segkind.md) ·
> [../plan/build-order.md](../plan/build-order.md) 5a.

## 개요

- **밤은 시드에서 만들어진다.** 레이아웃을 복제하지 않고 **시드만 보내서 각자 만든다** —
  호스트가 시드를 Replicated로 공유하면 모든 클라가 같은 맵을 로컬에서 생성한다.
- **트럭은 같은 맵 안에 있다.** 대기소 ↔ 원정지 별도 레벨·로딩·순간이동이 없다.
  트럭은 격자 **밖**에 서고, 시작 모듈의 서쪽 문으로 걸어 들어간다 (R.E.P.O.도 리썰도 밖이다).
- **손으로 짓는 기하가 없다.** 마당까지 전부 모듈 격자다 — 시드가 바꾸는 것이 곧 맵 전체다.
- **목적지마다 생성기가 다르다.** `UCL9SiteDef.Generator`가 어떤 생성기를 쓸지 가리킨다 —
  목적지 하나 추가 = 맵 생성기 하나.
- **지금 목적지는 `henhouse`(양계장) 하나뿐이다.** 산림·생수처 같은 목록은 없다 — 다음은
  벌집 후보 (재료 목록부터 정해야 한다).
- **2026-10-03: `farm`(대농장)** — 양계장 모듈 세트로 1층 200모듈(5배)을 짓는 스테이지. 레벨 `Lvl_Farm`.
  같은 날 `pigfarm`(돼지 농장, 단층 100모듈, `Lvl_PigFarm`). 스테이지 만드는 법은 [stage.md](stage.md).
  레벨이 사이트 행을 고르는 방식(`UCL9SiteDef.Level`)과 깊이 구역은 [../plan/data/site.md](../plan/data/site.md).
  격자 한 변 상한을 64 → 128칸으로 올렸다(양계장은 36칸 이하라 배치가 안 바뀐다).

## 양계장 구조 (모듈 조립 생성기)

```
칸 500cm 정사각 격자 — 한 변 16~64칸을 모듈 수에서 잡는다
모듈 = 표(UCL9ModuleDef)의 행. W×H칸을 차지하고 0/90/180/270 회전한다
소켓 = 경계 칸 단위. 열린 변에 닿은 모든 칸이 문 후보이고, 안 쓴 자리는 벽으로 막힌다
한 판 1층 34~50모듈 + 위층 절반 — 사이트 BaseModuleCount 40 × 진행 SiteSizeCurve(1일 0.85 → 10일 1.25) (2026-09-15, 24에서 올림)
층은 사이트 MaxFloors까지 — 격자가 층마다 한 장이고, 계단 모듈이 두 층의 같은 칸을 함께 차지한다
위층은 아래층 실내 모듈이 받쳐 주는 자리에만 선다. 양계장 2층 · 한 층 610cm (2026-09-10)
위층 예산 = 아래층 × UpperFloorShare(양계장 0.75), 위층도 본줄기를 먼저 뻗는다 (2026-09-16)
방은 시설 조명(작업등 소품의 빛 값)이 밝히고 정전이면 꺼진다 — light.md (2026-09-16)
입구 하나(시작 모듈 서쪽) + 비상구 하나(가장 먼 막다른 방의 flapIn) — 딱 둘
```

배치는 **본줄기 → Special → 곁가지** 순이다. 본줄기는 가장 깊은 자리만 이어 붙여 멀리 뻗고,
곁가지는 무작위 자리에 붙어 막다른 방으로 끝난다 — 리썰컴퍼니의 "지나는 곳 대부분이 막다른
곁가지"를 그 비율로 흉내낸다. 이미 놓인 모듈끼리 맞닿으면 사이트 행의 `LoopChance`(양계장 0.25) 확률로 문을
뚫어 **고리(우회로)**를 만든다 (DunGen의 `DoorwayConnectionChance`). 본줄기·통로·막다른 방 비율도
사이트 행 컬럼이다(`MainPathShare` · `PassageShare` · `DeadEndShare` — 2026-09-15, 그전에는 생성기 상수).

실측(시드 200개, BaseModuleCount 24 시절): **22~24모듈 · 평균 24.0 · 막다른 방 29% · 판마다 고리 문 4.7개.**
2026-09-15에 40으로 올리고 복도 행(1×3 · 1×4)을 더했으니 다시 재야 한다.

맵 조각 종류표는 [../plan/data/segkind.md](../plan/data/segkind.md), 모듈 19종은
[../plan/data/module.md](../plan/data/module.md). **무엇으로 짓는지**(조각 표·테마·타일링
배치)는 [asset-swap.md](asset-swap.md).

## 생성 흐름

```
밤 시작 (호스트)
  ↓ 시드 결정 → GameState에 Replicated
각 머신: Generator(시드, 일자) 실행 → 모듈 배치 + 칸 점유 + 변 상태(벽/문/비상구) 산출
  ↓ BuildSegments가 종류별 2D 발자국을 뱉고, 그대로 액터 배치
    (메시는 종류별 표에서 읽는다 — 경로 하드코딩 금지)
호스트만: 몬스터·아이템 스폰 (자리는 생성기의 LevelPoint가, 개수는 site×곡선이 — raiding.md)
```

- 생성기는 결정적이어야 한다 — 같은 시드면 모든 머신에서 같은 좌표가 나온다.
- 스폰 액터(몬스터·아이템)는 호스트 전용 판정 + 표준 복제다. 지형만 시드 공유로 만든다.
- **생성기 산출 지형 액터는 `bReplicates=false`** — 호스트 생성분이 복제되면 클라에 지형이
  이중 스폰된다. 밤 종료 철거도 각 머신의 로컬 정리 패스가 담당한다 (GameState 페이즈 OnRep).

## 이동·시야 기반

- 몬스터 경로 탐색 방식은 **미결** ([monster.md](monster.md) — NavMesh vs 커스텀
  그래프). 층 봉쇄·일방향 개구멍(flapIn)은 어느 방식이든 맵이 보장해야 하는 요구사항이다.
- 어둠이 기본, 손전등 도달 320u ([light.md](light.md), 5c에서 구현).

## 맵 배치 툴 (에디터)

```
[ 시드: 873 ]  [ 배치 ]  [ 지우기 ]
```

에디터 창에 시드를 넣으면 그 시드의 맵이 레벨에 깔린다 — 밤마다 새 맵이 나오는 게임 규칙과
에디터에서 특정 맵을 열어보는 작업이 **같은 생성 코드**를 쓴다 (build-order.md 5a).

## 디버깅

```
cl9.seed <N> [site] [day]   // 특정 시드로 재생성. 0 = 짓지 않음, clear = 제거 (cheats.md)
                            // day는 한 판의 모듈 수만 바꾼다 — 10일차 맵을 눈으로 볼 때 쓴다
cl9.shownav                 // 몬스터 이동·경로 시각화
```

## 참고

- 모듈 표·조립 규칙: [../plan/data/module.md](../plan/data/module.md)
- 사이트 컬럼: [../plan/data/site.md](../plan/data/site.md)
- 조각 표·테마·팩 교체 절차: [asset-swap.md](asset-swap.md)
- 리썰컴퍼니 던전 생성 분석(참고): [../reference/lethal/lethal-map.md](../reference/lethal/lethal-map.md)
- 스폰 개수·시간 규칙: [raiding.md](raiding.md) · [balance.md](balance.md)
- 개발용 테스트 맵 관리 주의(.umap 비대): [../plan/build-order.md](../plan/build-order.md) "아직 모르는 것"

---

## 구현 상태 (⑤ 5a — 2026-09-08 모듈 조립으로 재작성)

```
Source/CL9/World/
  CL9SiteLayout.h/.cpp   순수 조립기 — FCL9SiteParams(사이트 컬럼 + 실측 치수) + 모듈 표 + 시드
                         → FCL9SiteLayout(모듈 배치 · 칸 점유 · 변 상태 · LevelPoint) →
                         BuildSegments()가 종류별 2D 발자국(FCL9SiteSeg) 목록을 뱉는다.
                         엔진 의존 없음. FRandomStream(시드)로 결정적.
  CL9SiteMap.h/.cpp      ACL9SiteMap — 발자국을 종류별 ISM으로 깐다. bReplicates=false, 트랜지언트.
                         트럭 패드를 앵커로, 지면 z=0. 에디터 월드에선 editor-only.
  CL9FlapIn.h/.cpp       ACL9FlapIn — 일방향 비상구. 이제 네 방향 어디에나 달린다.
Source/CL9/Data/
  CL9ModuleDef.h/.cpp    모듈 표 — Type·Size·Sockets(4비트)·Rarity·Height·bStairs·bOutdoor·PointPurposes.
  CL9SiteDef             맵 컬럼이 다섯으로 줄었다 — Generator·Theme·CellSize·BaseModuleCount·EntryOffset.
  CL9ProgressionDef      SiteSizeCurve 추가 — 한 판의 모듈 수 배율.
```

생성 흐름은 그대로다: `ACL9GameState::SetSiteMap(seed)` → `FCL9SiteMapKey{Seed, Build}` 복제 →
OnRep에서 각 머신이 `ACL9SiteMap::Rebuild(…, Seed, CurrentDay)`. 시드는 로비 진입 때 굴린다.

**스폰은 `LevelPoint`를 고른다** (module.md 결정 4). 배치된 칸마다 지점 하나가 나오고, 그 용도는
모듈 표의 `PointPurposes` 컬럼이 정한다 — 그래서 "항아리는 납품 방에만" 같은 규칙이 C++에 별칭으로
박히지 않는다. `PointsOfPurpose()`가 **시작 모듈에서 먼 것부터** 정렬해 돌려주므로, 몬스터는 그
목록의 앞쪽 60%에서, 재료는 전체에서 고른다. 항아리는 Jar 용도가 켜진 모듈마다 하나다.
사이트에 **깊이 구역**(`Zones`)이 있으면 구역마다 `PointsInDepthBand()`로 자기 띠의 지점만 골라 놓는다
(2026-10-03 — [../plan/data/site.md](../plan/data/site.md) 구역).
**마당 무스폰 (2026-10-12, 7b — [../goal/loop-7b-yard-no-spawn.md](../goal/loop-7b-yard-no-spawn.md))**: 실외 모듈은 `yard_gate`(0x0)·`yard_open`(**0x8 = Patrol 만**,
옛 0xB) — 마당 칸은 몬스터·재료 지점을 내지 않는다. 코드 변경 없이 모듈 표 한 칸이다(이 규칙의 주인은 표다). Patrol 은 남겨서 마당 지점이
레이아웃에 남는다 — 깊이의 분모(판 전체 최대 홉)가 바뀌지 않아 구역 띠가 그대로다. 검증: 판에 나온 몬스터·아이템 아래로 쏜 바닥이
`SM_Henhouse_YardGround_*` 인 것이 대농장·돼지 농장·양계장 여러 판에서 아이템 0(옛 값 0xB 로는 한 판에 2개) — 마당 위 몬스터는 배회(WANDER)로 걸어 들어간 것뿐이다.

**고정물 — 호스트가 세우는 복제 액터 (2026-10-12, 7c — [../goal/loop-7c-kitchen-in-building.md](../goal/loop-7c-kitchen-in-building.md))**: 소품 세트(`FCL9PropSet`)에
`Fixtures`(`FCL9FixturePlacement` — `ActorClass` 소프트 클래스·회전 전 모듈 로컬 `Local`·`Yaw`)가 있다. 소품은 모든 머신이 시드로 각자 그리는 메시라 상태를 못 가지므로,
상태가 있는 시설(조리대)은 여기에 둔다. `ACL9SiteMap::GetFixtureSpawns()` 가 모듈마다 **소품과 같은 함수(`ChoosePropSet`, 자리 좌표 해시)로 뽑은 세트**의 고정물을
월드 변환으로 돌려주고, 호스트(`ACL9GameMode::SpawnNightPopulation`)가 맵마다 **재료·몬스터보다 먼저** 세운다(그 둘은 겹치면 비켜 서는 스폰이라 조리대 속에 박히지 않는다).
세운 것은 `NightSpawned` 에 들어가 밤 개체와 같이 치워진다. 로그 `Night N: K fixture(s) placed — <클래스> in module <i> (<별칭>) at <위치> yaw <요>`.
지금 쓰는 곳은 `kitchen` 모듈의 두 세트뿐이다(작업대를 빼고 `BP_CL9Kitchen` — 세트 0 (250, 94) 요 90·남쪽 벽, 세트 1 (750, 1406) 요 −90·북쪽 벽).
**인트로 카메라**는 목적지 모듈에 항아리가 없고 고정물이 있으면 그 고정물(원점 위 130 cm)을 본다 — 같은 계산이라 조리대 액터가 아직 복제되지 않은 클라도 같은 곳을 본다
(로그 `look-at … (fixture)`, `ACL9PlayerController` 인트로 경로).

**치수는 여전히 실측이다** — 벽 두께는 `barn` 조각의 얇은 축, 문 폭은 `door` 조각의 넓은 축에서
읽는다 ([asset-swap.md](asset-swap.md)). 표에 옮겨 적은 숫자는 팩이 바뀌면 조용히 틀린 값이 된다.

### 지운 것

`FCL9HenhouseParams` · `FCL9SiteFrame`(고정 프레임 전부) · `FCL9FloorLayout`(OpenH/OpenV) ·
Kruskal `CarveFloor` · 케이지 열 · 플러그 · 층 낙차와 경사로 · `CellToWorld` · `CeilingBase` ·
`UCL9SiteDef`의 격자 컬럼 열세 개 · `UCL9SegKindDef::BaseZMode`(층이 없어져 고아가 됐다).

### 실제로 서는 것을 확인했다 (2026-09-08)

`ModuleTable`에 양계장 16종 행을 넣고 `CL9DefList`에 등록했다. `Lvl_Henhouse`에서
`cl9.seed`로 실측한 값이다.

| 명령 | 결과 |
|---|---|
| `cl9.seed 873` | 1일차 · **20모듈** · 22×22 격자 · 지점 95 · 인스턴스 329 |
| `cl9.seed 873 henhouse 10` | 10일차 · **30모듈** · 28×28 격자 · 지점 136 · 인스턴스 477 |
| `cl9.seed 4242 henhouse 5` | 5일차 · **24모듈** · 24×24 격자 · 지점 106 · 인스턴스 373 |

**일자 커브가 설계값 20 · 24 · 30을 그대로 낸다** (module.md 결정 5). 세 판 모두 목표 수를
빠짐없이 채웠다. 20모듈 판의 실제 크기는 60m × 95m다.

### Lvl_Flat 전시 우리 (2026-09-28)

몬스터 5종을 가둬 놓고 보는 테스트 우리다. 2026-09-06에 회색 큐브 20개(`StaticMeshActor_8~27`,
안쪽 5×5m · 두께 1m · 높이 0.6m)로 세웠던 것을 우리 울타리 조각
`SM_Pen_Fence_250_A`([../asset/map/pen_fence.md](../asset/map/pen_fence.md))로 바꾼다. 여기는
생성기가 아니라 **손배치**다 — `pen` seg kind를 생성기가 아직 안 내기 때문이고(위 미결),
그래서 이 좌표표가 정본이다.

- 울타리 중심선이 한 변 500cm인 정사각형이다. 조각이 250cm라 **변마다 두 장, 우리마다 여덟 장,
  다섯 우리에 40장.** 눌림 없이 1.00으로 맞는다.
- 안쪽 빈 칸은 480 × 480cm(중심선 500 − 두께 20). 옛 큐브 우리의 5×5m보다 20cm 작고, 바깥
  발자국은 520 × 520cm로 옛 700 × 700cm보다 작아져 우리 사이 통로가 280cm로 넓어진다.
- 네 모서리는 조각 끝의 반 폭 기둥(10cm)끼리 10 × 10cm 겹친다 — 새는 틈 없이 20cm 기둥
  하나가 되고, 바깥쪽 4분면만 비어 보인다. 겹침 대신 틈을 내면 아이템이 새므로 겹치는 쪽으로 둔다.
- 피벗이 바닥 중앙이라 **Z는 0**(Lvl_Flat 지면 위, 시설 인스턴스와 같은 기준), 배율 1이다.

우리 중심 (cx, 1000), cx = 1000 / 1800 / 2600 / 3400 / 4200. 한 우리의 여덟 장:

| 위치 | Yaw | 무엇 |
|---|---|---|
| (cx−125, 750) · (cx+125, 750) | 0 | 남쪽 변 |
| (cx−125, 1250) · (cx+125, 1250) | 0 | 북쪽 변 |
| (cx−250, 875) · (cx−250, 1125) | 90 | 서쪽 변 |
| (cx+250, 875) · (cx+250, 1125) | 90 | 동쪽 변 |

**130cm는 점프로 넘긴다** (정점 156cm). 전시 우리로는 그대로 두고, 본편 우리에서 넘기 허용
여부는 미결이다 ([../plan/data/segkind.md](../plan/data/segkind.md) 미결).

### Lvl_Farm — 대농장 (2026-10-03)

`Lvl_Henhouse` 복제. `Tools/Unreal/setup_stage.py Tools/Unreal/stages/farm.json apply`(헤드리스 커맨드릿 — 절차 [stage.md](stage.md))가 레벨·`MapTable` 행을 만들고, 10일차 판(250모듈 →
78칸)까지 덮게 바닥 판(`Floor`)과 `NavBounds`를 465 × 440 m로 늘린다. **레벨에 저장된 RecastNavMesh는 지운다** — 복제해 온 내비 데이터가
옛 경계의 타일 용량을 들고 와 판의 일부만 지어졌기 때문이다(decisions 2026-10-03). 런타임(Dynamic)이 새로 지어 밤 +20초 전에 끝난다.
PIE 실측(`Tools/Unreal/qa_farm_stage_pie.py`, 결과 `Saved/QA/stage_pie_Lvl_Farm.json`): 1일차 213/213모듈 · 항아리 5(납품 방 4 + 레벨에 놓인 1) · 몬스터 25(전부 내비 위)
· 아이템 ~75 · 에디터 PIE 평균 프레임 10 ms.

**`Lvl_Henhouse`의 내비 경계는 판보다 훨씬 작다**(x −1700~11000 · y −2100~2500). 같은 QA로 대조하면 몬스터 5마리 중 1마리만 내비 위에 있고
아무도 움직이지 않았다 — 고칠지는 미결.

### 아직 안 한 것

- **문 옆 벽 토막이 −75%로 눌린다.** 칸 500cm에 문 250cm를 뚫으면 양옆에 125cm 토막이 남는데,
  타일링이 500짜리 벽 조각 한 장을 거기에 우겨 넣는다(`worst run fit 0.25`). 온전한 벽은 1.00으로
  잘 맞고 문 옆 토막만 그렇다. 고치려면 문 폭을 칸에 맞추거나 토막 전용 조각이 필요하다.
- **`door` seg kind 행** — 지금은 문 자리가 문틀 없는 개구부다. 조각 표의 `door` 역할을 그대로
  쓰면 **안 된다** — 그 메시를 개구부에 채우면 콜리전이 생겨 문이 통째로 막힌다.
- **천장을 꺼 두었다** (2026-09-08 사용자: "천장없는건 일단 유지해"). `SegKindTable`의
  `ceiling` 행을 `__ceiling_off__`로 바꿔 둔 상태라 로그에 그 종류를 못 찾는다는 에러가 뜬다 —
  의도된 것이다. 맵을 위에서 내려다보며 확인한 뒤 되돌릴지 정한다.
- **소품**(module.md 결정 7, REPO의 `ModulePropSwitch`) — `PropSets` 컬럼이 설계에만 있다.
- **`pen`(우리 울타리) 종류를 생성기가 안 낸다** (2026-09-28). 조각·seg kind 행은 준비됐지만
  `WallKind()`는 `bOutdoor`만 보고 `barn`/`wall`을 고른다. 모듈 표에 둘레 종류 열을 하나 더할지가
  미결이다 ([../plan/data/module.md](../plan/data/module.md) `pen` 행 각주).

검증: 유닛 테스트 `CL9.Map.LargeSite`(200모듈 판 30시드 — 연결·겹침·비상구·1층 90% 도달, 2026-10-03) ·
`CL9.Map.DepthBand`(깊이 띠가 지점을 빠짐없이 나누고 깊은 띠일수록 홉이 크다) ·
`CL9.Map.Determinism`(같은 시드 = 같은 해시·같은 칸·같은 변) ·
`CL9.Map.Assembly`(시드 1~200에서 전 모듈 도달 · 칸 겹침 0 · 비상구 정확히 하나 · 크기 분포) ·
`CL9.Map.Segments`(모듈당 바닥 한 장 · 실내 모듈당 천장 한 장 · 넓이 0인 박스 없음).
2인 PIE에서는 양쪽 로그의 `hash %08X`가 같아야 한다.
