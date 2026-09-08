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

## 양계장 구조 (모듈 조립 생성기)

```
칸 500cm 정사각 격자 — 한 변 16~64칸을 모듈 수에서 잡는다
모듈 = 표(UCL9ModuleDef)의 행. W×H칸을 차지하고 0/90/180/270 회전한다
소켓 = 경계 칸 단위. 열린 변에 닿은 모든 칸이 문 후보이고, 안 쓴 자리는 벽으로 막힌다
한 판 20~30모듈 — 사이트 BaseModuleCount 24 × 진행 SiteSizeCurve(1일 0.85 → 10일 1.25)
층이 없다 — 높이차는 모듈 안의 계단으로 내고 이웃으로 전파되지 않는다
입구 하나(시작 모듈 서쪽) + 비상구 하나(가장 먼 막다른 방의 flapIn) — 딱 둘
```

배치는 **본줄기 → Special → 곁가지** 순이다. 본줄기는 가장 깊은 자리만 이어 붙여 멀리 뻗고,
곁가지는 무작위 자리에 붙어 막다른 방으로 끝난다 — 리썰컴퍼니의 "지나는 곳 대부분이 막다른
곁가지"를 그 비율로 흉내낸다. 이미 놓인 모듈끼리 맞닿으면 25% 확률로 문을 뚫어 **고리(우회로)**
를 만든다 (DunGen의 `DoorwayConnectionChance`).

실측(시드 200개): **22~24모듈 · 평균 24.0 · 막다른 방 29% · 판마다 고리 문 4.7개.**

맵 조각 종류표는 [../plan/data/segkind.md](../plan/data/segkind.md), 모듈 16종은
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

**치수는 여전히 실측이다** — 벽 두께는 `barn` 조각의 얇은 축, 문 폭은 `door` 조각의 넓은 축에서
읽는다 ([asset-swap.md](asset-swap.md)). 표에 옮겨 적은 숫자는 팩이 바뀌면 조용히 틀린 값이 된다.

### 지운 것

`FCL9HenhouseParams` · `FCL9SiteFrame`(고정 프레임 전부) · `FCL9FloorLayout`(OpenH/OpenV) ·
Kruskal `CarveFloor` · 케이지 열 · 플러그 · 층 낙차와 경사로 · `CellToWorld` · `CeilingBase` ·
`UCL9SiteDef`의 격자 컬럼 열세 개 · `UCL9SegKindDef::BaseZMode`(층이 없어져 고아가 됐다).

### 실제로 서는 것을 확인했다 (2026-09-08)

`DA_ModuleTable`에 양계장 16종 행을 넣고 `DA_CL9DefList`에 등록했다. `Lvl_Henhouse`에서
`cl9.seed`로 실측한 값이다.

| 명령 | 결과 |
|---|---|
| `cl9.seed 873` | 1일차 · **20모듈** · 22×22 격자 · 지점 95 · 인스턴스 329 |
| `cl9.seed 873 henhouse 10` | 10일차 · **30모듈** · 28×28 격자 · 지점 136 · 인스턴스 477 |
| `cl9.seed 4242 henhouse 5` | 5일차 · **24모듈** · 24×24 격자 · 지점 106 · 인스턴스 373 |

**일자 커브가 설계값 20 · 24 · 30을 그대로 낸다** (module.md 결정 5). 세 판 모두 목표 수를
빠짐없이 채웠다. 20모듈 판의 실제 크기는 60m × 95m다.

### 아직 안 한 것

- **문 옆 벽 토막이 −75%로 눌린다.** 칸 500cm에 문 250cm를 뚫으면 양옆에 125cm 토막이 남는데,
  타일링이 500짜리 벽 조각 한 장을 거기에 우겨 넣는다(`worst run fit 0.25`). 온전한 벽은 1.00으로
  잘 맞고 문 옆 토막만 그렇다. 고치려면 문 폭을 칸에 맞추거나 토막 전용 조각이 필요하다.
- **`door` seg kind 행** — 지금은 문 자리가 문틀 없는 개구부다. 조각 표의 `door` 역할을 그대로
  쓰면 **안 된다** — 그 메시를 개구부에 채우면 콜리전이 생겨 문이 통째로 막힌다.
- **천장을 꺼 두었다** (2026-09-08 사용자: "천장없는건 일단 유지해"). `DA_SegKindTable`의
  `ceiling` 행을 `__ceiling_off__`로 바꿔 둔 상태라 로그에 그 종류를 못 찾는다는 에러가 뜬다 —
  의도된 것이다. 맵을 위에서 내려다보며 확인한 뒤 되돌릴지 정한다.
- **소품**(module.md 결정 7, REPO의 `ModulePropSwitch`) — `PropSets` 컬럼이 설계에만 있다.

검증: 유닛 테스트 `CL9.Map.Determinism`(같은 시드 = 같은 해시·같은 칸·같은 변) ·
`CL9.Map.Assembly`(시드 1~200에서 전 모듈 도달 · 칸 겹침 0 · 비상구 정확히 하나 · 크기 분포) ·
`CL9.Map.Segments`(모듈당 바닥 한 장 · 실내 모듈당 천장 한 장 · 넓이 0인 박스 없음).
2인 PIE에서는 양쪽 로그의 `hash %08X`가 같아야 한다.
