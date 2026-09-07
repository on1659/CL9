# 맵 생성 시스템

> 밤마다 시드에서 만들어지는 원정지. **설계 정본은 plan이다** —
> [../plan/data/site.md](../plan/data/site.md) · [../plan/data/segkind.md](../plan/data/segkind.md) ·
> [../plan/build-order.md](../plan/build-order.md) 5a.

## 개요

- **밤은 시드에서 만들어진다.** 레이아웃을 복제하지 않고 **시드만 보내서 각자 만든다** —
  호스트가 시드를 Replicated로 공유하면 모든 클라가 같은 맵을 로컬에서 생성한다.
- **트럭은 같은 맵 안에 있다.** 대기소 ↔ 원정지 별도 레벨·로딩·순간이동이 없다.
  마당·트럭·계단·축사 외곽은 손으로 짓고, 시드는 미로 내부만 바꾼다.
- **목적지마다 생성기가 다르다.** `UCL9SiteDef.Generator`가 어떤 생성기를 쓸지 가리킨다 —
  목적지 하나 추가 = 맵 생성기 하나.
- **지금 목적지는 `henhouse`(양계장) 하나뿐이다.** 산림·생수처 같은 목록은 없다 — 다음은
  벌집 후보 (재료 목록부터 정해야 한다).

## 양계장 구조 (격자 생성기)

격자 치수는 site.md의 컬럼이 정한다 — 시드와 무관하게 고정이고, 시드가 바꾸는 건 미로
내부(케이지·마개·주방·둥지·개구멍)뿐이다.

```
1층: 통로 10 × 열 16 (통로 폭 250cm · 열 폭 500cm) — 축사 내부 8000 × 3580cm
지하: 통로 8 × 열 12 — flapIn(일방향 개구멍) · stair로 연결
개구부 목표: 1층 175 · 지하 105  ← 난이도 손잡이 (줄이면 막다른 길이 늘고 도주로가 준다)
항아리: 1층 7 · 지하 6           ← 어느 셀에서든 1500u 안에 있게 배치
```

맵 조각 11종(`truck` · `stair` · `flapIn` 등)은 [../plan/data/segkind.md](../plan/data/segkind.md).
**무엇으로 짓는지**(조각 표·테마·타일링 배치)는 [asset-swap.md](asset-swap.md).

## 생성 흐름

```
밤 시작 (호스트)
  ↓ 시드 결정 → GameState에 Replicated
각 머신: Generator(시드) 실행 → 벽·케이지·둥지·계단·항아리 좌표 산출
  ↓ 좌표대로 액터 배치 (메시는 종류별 표에서 읽는다 — 경로 하드코딩 금지)
호스트만: 몬스터·아이템 스폰 (좌표는 생성기가, 개수는 site×곡선이 — raiding.md)
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
cl9.seed <N>           // 특정 시드로 재생성 (cheats.md)
cl9.shownav            // 몬스터 이동·경로 시각화
cl9.showjars           // 항아리 배치·1500u 커버리지 확인
```

## 참고

- 격자 컬럼·개구부 값: [../plan/data/site.md](../plan/data/site.md)
- 조각 표·테마·팩 교체 절차: [asset-swap.md](asset-swap.md)
- 리썰컴퍼니 던전 생성 분석(참고): [../plan/reference-lethal-company.md](../plan/reference-lethal-company.md)
- 스폰 개수·시간 규칙: [raiding.md](raiding.md) · [balance.md](balance.md)
- 개발용 테스트 맵 관리 주의(.umap 비대): [../plan/build-order.md](../plan/build-order.md) "아직 모르는 것"

---

## 구현 상태 (⑤ 5a 선행 — 2026-09-06)

빌드 오더상 ④ 뒤지만 4c 다음에 당겨 넣었다 (사용자 결정: 새 생성기 작성 · 1층+지하 한 번에 · 마당·축사 외곽은
시드와 무관한 고정 기하). CCL8의 검증된 생성기는 이식하지 않았다.

```
Source/CL9/World/
  CL9SiteLayout.h/.cpp   순수 생성기 — FCL9HenhouseParams(표 컬럼) + 시드 → FCL9SiteLayout(층별 OpenH/OpenV
                         행렬·항아리 셀·개구멍 통로) → BuildSegments()가 종류별 2D 발자국(FCL9SiteSeg) 목록을 뱉는다.
                         엔진 의존 없음. FRandomStream(시드)로 결정적. Kruskal 신장 트리(전 셀 연결 보장) 뒤
                         TargetOpen까지 추가 개방. 항아리는 최원점 우선.
  CL9SiteMap.h/.cpp      ACL9SiteMap — 세그먼트를 종류별 ISM(회색 100cm 큐브 스케일)으로 깐다. bReplicates=false,
                         트랜지언트. 트럭 패드를 앵커로, 지면 z=0. 셀→월드 좌표 제공. 에디터 월드에선 editor-only.
  CL9FlapIn.h/.cpp       ACL9FlapIn — 일방향 개구멍. 안쪽 트리거에 닿은 캐릭터만 캡슐 move-ignore로 밀고 나간다.
Data/CL9SegKindDef       조각 종류 표 (DA_SegKindTable) — Mesh·Height·BaseZMode·bBlocksSight.
Data/CL9SiteDef          격자 컬럼 (Generator·AisleCount…·TargetOpen·JarCount) — 기본값이 site.md 초안.
```

생성 흐름(위 절)은 그대로다: `ACL9GameState::SetSiteMap(seed)` → `FCL9SiteMapKey{Seed, Build}` 복제 →
OnRep에서 각 머신이 `ACL9SiteMap::Rebuild`. **시드는 로비 진입 때 굴린다** (게임 시작·로비 복귀·`cl9.seed`) —
정차 중에도 오늘 밤 갈 미로가 서 있고, 밤 시작은 그 시드를 그대로 쓴다(2026-09-06 사용자 결정: 로비가 빈 판이면
안 된다). 스폰은 밤 시작에 생성기 셀(층·통로·열 무작위 + 통로 안 지터)에서 하고, 항아리는 `JarClass`(GameMode BP)를
항아리 셀에 스폰. 로비 복귀 때 스폰물이 철거되고 새 시드로 다음 밤 미로가 선다.
**조리대(`ACL9Kitchen`)도 나중에 여기서 나온다** — 요리하는 자리가 던전 안에 생긴다(2026-09-06 사용자 결정,
[cooking.md](cooking.md)). 지금은 `Lvl_Flat`에 손으로 놓아 두었고, 배치 규칙(층·개수·항아리처럼 커버리지를 볼지)은
소품 배치 규칙과 함께 미결이다. 시드 0(외곽만)은 `cl9.seed 0`으로만
나온다.

고정 프레임(`FCL9SiteFrame`, cm): 벽 30 · 패드→축사 서벽 1400 · 마당 서쪽 경계 −900 · 여백 600 · 지하 낙차
450 · 계단 복도 = 착지 150 + 경사로 600 + 착지 150 (기울기 0.75) · 개구멍 폭 180 · 1층 천장 630(`BarnCeiling`, 지하 천장은
지면 높이 — 2026-09-06 "건물 안 느낌"으로 추가, 계단 복도도 덮고 지하 입구 위는 벽으로 막는다). 지하는 1층 동쪽에 나란히
(패드 기준 x 10380~16380 — 2026-09-07 격자 확대 후 값), 구덩이 모양이라 지면 슬래브를 조각내 깔고 지하 바닥 슬래브는 −450에 둔다.
**생성 맵은 전용 레벨 `Content/Maps/Lvl_Henhouse`에서 쓴다** (2026-09-06 사용자 결정 — 평지 테스트 레벨과 분리).
그 레벨만 바닥 Floor_0를 z −500으로 내려 구덩이가 겹치지 않게 했고, 지면은 생성기가 깐다. `Lvl_Flat`은 원래
평지 그대로이고 월드 세팅 게임모드가 `BP_CL9GameMode_Flat`(목적지 `flat` — `Generator=None`, 서식·재료는 양계장과
같음)이라 생성기가 돌지 않고 밤 스폰은 종전 원환 스탠드인을 쓴다. 목적지 행의 `Generator`가 None이면
`ACL9SiteMap`은 지워진다 — 레벨별로 생성 여부를 표 행 하나로 고른다.

넣지 않은 것: 주방 셀(요리는 트럭에서 — CL9 루프에 축사 주방이 없다), 둥지(trays — 재료는 셀 무작위),
건초·물통·상자·보온상자 소품(좌표 규칙이 기획에 없음), 맵 배치 **창**(5a) — 대신 에디터 콘솔에서 `cl9.seed N`이
같은 코드로 레벨에 트랜지언트·editor-only 액터를 깐다(`cl9.seed clear`로 제거). 몬스터 시야 채널
`CL9_Occlusion`은 아직 없어 `bBlocksSight=false` 종류는 Visibility 채널만 무시한다.

### 5b 진행분 (2026-09-07)

**조각 표를 갈라냈다.** seg kind 행이 메시 대신 **역할 이름**(`Piece`)을 가리키고, 실제 메시는
`UCL9PieceDef`(`DA_PieceTable`)가 역할×테마로 갖는다 — 목적지 행의 `Theme`가 어느 묶음을 쓸지 고른다.
에셋 팩을 갈아 끼울 때 고치는 곳이 그 표 하나로 모인다 ([asset-swap.md](asset-swap.md)).

배치 모드 `Placement`가 붙었다. `Tile`은 조각을 자연 크기로 깔아 나가는데 **축마다 다르게 다룬다** —
길이 축은 장수로 나눠 늘어놓고, 두께 축은 늘리지 않고 발자국 가운데에 세우며, 높이 축은 단을 쌓고
남는 높이만 맨 윗단이 받는다. 조각 치수와 피벗은 표에 적지 않고 `UStaticMesh::GetBoundingBox()`에서
읽으므로, 새 팩은 경로만 갈아 끼우면 된다. 안 맞으면 `LogCL9`가 종류마다 맞춤 배율을 알려 주고
0.85~1.15를 벗어나면 경고한다. 변형은 자리 좌표 해시로 고른다 — 난수 스트림은 호출 순서에 의존해
머신마다 갈릴 수 있는데 지형은 복제되지 않고 각자 만들므로 그러면 사람마다 다른 벽을 보게 된다.
ISM 컴포넌트는 (종류, 메시)마다 하나가 됐다.

**벽 두께가 상수에서 실측값으로 바뀌었다** — `barn` 조각의 두께를 읽어 `FCL9SiteFrame::Wall`에 넣는다.
격자도 조각 모듈(500cm)에 맞춰 다시 잡았고 1층을 48칸 → 80칸으로 키웠다(site.md).

아직 안 한 것: 실제 배치 육안 확인(`cl9.seed 873`), 코너 조각, 지형·머티리얼.

검증: 유닛 테스트 `CL9.Map.Determinism`·`Connectivity`(시드 1~300 전 셀 도달·개구부 수 일치)·`Segments`,
PIE 시나리오 L절(testing/pie-scenarios.md). 2인 PIE에서는 양쪽 로그의 `hash %08X`가 같아야 한다.
