# 맵 모듈 — `UCL9ModuleDef` (설계, 미구현)

> **2026-09-08 결정. 아직 코드가 없다.** 이 문서가 착수 지점이다.
> 참고 분석: [../../reference/repo/repo-map.md](../../reference/repo/repo-map.md)(주 모델) ·
> [../../reference/lethal/lethal-map.md](../../reference/lethal/lethal-map.md)(층 개념)
> 조각 표: [../../systems/asset-swap.md](../../systems/asset-swap.md)

## 왜 바꾸나

사용자 확인(2026-09-08): **"미로 형식이라는 게 진짜 미로를 만들라는 게 아니라, 미로처럼
매번 맵이 달라지는 걸 만들어 달란 거야. 가구도 배치하고."**

지금 CL9에 든 격자 미로 생성기(Kruskal · `OpenH`/`OpenV` · 케이지 열 · 플러그)는 **CCL8에서
넘어온 것이고 폐기한다.** 벽으로 된 진짜 미로가 아니라, **정해진 모듈이 매번 다르게 조립되는**
맵이 목표다. 사용자: "특정 패턴이 반복되면서 맵이 만들어져야 돼."

## 결정 (2026-09-08 사용자)

1. **사이트 전체가 모듈 격자다** — 축사 안쪽만이 아니라 마당까지 전부.
2. **모듈은 프리팹이 아니라 표의 행이다** (사용자가 B안 선택). 껍데기(바닥·벽·문·천장)는
   생성기가 조각 표에서 뽑아 세운다. 손으로 만드는 방 프리팹이 없다.
3. **모듈 16종으로 시작한다** (양계장 = 1스테이지). 다음 스테이지는 별도 세트.
4. **층 개념을 버린다 — 리썰컴퍼니 방식.** 지금의 1층/지하 + 경사로 구조 대신, 높이차를
   **모듈 안의 계단·구덩이**로 낸다(리썰의 `CatwalkStairTile` · `DeepShaftTile` ·
   `LargeStairRoom`처럼). `FCL9SiteFrame`의 `FloorDrop`·`Landing`·`Ramp`·`StairX0/X1`이 사라진다.
5. **한 판 = 모듈 20~30개**, 일자가 갈수록 커진다 (REPO의 `LevelSizeMultiplier`).
6. **트럭은 밖에 고정** — REPO도 리썰도 밖이다. 시작 모듈이 입구가 되고 트럭이 거기 붙는다.
7. **소품이 개성을 만든다** — REPO의 `ModulePropSwitch`. 모듈마다 소품 세트를 여러 벌 두고
   매번 하나를 뽑는다.

## 격자

- **칸 = 500cm** (조각 한 장 — asset-swap.md의 모듈 크기와 같다)
- 모듈이 **W × H칸**을 차지한다. 회전 허용(0/90/180/270).
- 연결은 **사방 소켓**(N/S/E/W). **소켓의 단위는 경계 칸이다** — 변이 열려 있다는 말은
  그 변에 닿은 **모든 경계 칸이 문 후보**라는 뜻이고, 두 모듈은 같은 경계 칸을 양쪽에서
  공유하면서 둘 다 그 변이 열려 있을 때 이어진다. 문은 칸 중앙에 서고 폭은 `door` 조각의
  실측 폭을 쓴다.
- **안 쓴 소켓은 벽으로 막는다** (REPO `ModuleBlockObject`, 리썰의 blocker와 같은 처리).

## 표 컬럼 (초안)

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `Alias` | `FName` | `coop_hall` · `corridor` … |
| `SubAlias` | `FName` | 스테이지/테마 (`henhouse`) — 조각 표와 같은 규약 |
| `Type` | enum | `Start` · `Normal` · `Passage` · `DeadEnd` · `Special` |
| `Size` | `FIntPoint` | 격자 칸 단위 W × H |
| `Sockets` | 4비트 마스크 | 어느 변이 열리나 — N/S/E/W (회전 전 기준). 열린 변의 경계 칸이 전부 문 후보다 |
| `Rarity` | int 1~3 | 뽑힐 가중치 (REPO `ModuleRarity1/2/3`) |
| `PropSets` | 배열 | 소품 배치 세트들 — 인스턴스마다 하나를 결정적으로 뽑는다 (**미구현**) |
| `Height` | float | 천장 높이 (모듈마다 다를 수 있다) |
| `bStairs` | bool | 모듈 **안에서** 오르내리는 높이차가 있는가 — 바깥으로 전파되지 않는다 |
| `bOutdoor` | bool | 야외 모듈인가 (마당) — 천장을 안 씌우고 벽 대신 울타리를 세운다 |
| `PointPurposes` | 4비트 마스크 | 이 모듈의 칸이 어떤 스폰 지점이 되나 — `Monster`·`Loot`·`Jar`·`Patrol` |

**치수를 손으로 적지 않는다는 원칙은 그대로다** — 조각 크기·피벗은 메시 바운드에서 읽는다
(asset-swap.md). 이 표가 정하는 건 "몇 칸짜리 방이고 어디가 뚫리나"까지다. 벽 두께는
`barn` 조각의 얇은 축에서, 문 폭은 `door` 조각의 넓은 축에서 실측으로 온다.

사이트 표(`UCL9SiteDef`) 쪽에 남는 맵 컬럼은 다섯이다 — `Generator` · `Theme`(모듈 표와
조각 표를 함께 가리킨다) · `CellSize` · `BaseModuleCount` · `EntryOffset`. 옛 격자 컬럼
열세 개가 이 다섯으로 줄었다.

## 양계장 모듈 16종 (1스테이지)

REPO의 타입 비율(N 31% · DE 25% · P 19% · E 19%)에 맞췄다.

| # | Alias | Type | 크기 | 무엇 | 소품 후보 |
|---|---|---|---|---|---|
| 1 | `yard_gate` | Start | 3×2 | 마당 입구 — 트럭이 붙는다 | 울타리·문 |
| 2 | `corridor` | Passage | 1×2 | 곧은 복도 | 건초 부스러기·통 |
| 3 | `corner` | Passage | 2×2 | 꺾인 복도 | 〃 |
| 4 | `stair_pass` | Passage | 2×2 | 계단 통로 (높이차) | 사다리·널판 |
| 5 | `coop_hall` | Normal | 3×3 | 케이지 열이 늘어선 큰 방 | 케이지·건초 |
| 6 | `pen` | Normal | 3×2 | 축사 우리 | `HayStack` · `Trough_FoodPile` · `Trough_WaterPlane` |
| 7 | `feed_store` | Normal | 2×3 | 사료 창고 | `SackOpen_Grain` · `Grain_Round` · `Barrel_01~04` · `SackStack` |
| 8 | `cross` | Normal | 2×2 | 사거리 홀 | 기둥·등 |
| 9 | `yard_open` | Normal | 3×3 | 마당 (야외) | `Well` · `Scarecrow` · `VillageFence` · `TreeLog` |
| 10 | `incubator` | DeadEnd | 2×2 | 부화실 | 보온 상자(`brooder`) |
| 11 | `tool_room` | DeadEnd | 2×2 | 도구방 | `Pitchfork` · `Rake` · `Hoe` · `Shovel` · `Ladder` · `WoodcutterAxe` |
| 12 | `storage` | DeadEnd | 2×2 | 잡동사니 창고 | `Crate*` · `Barrel*` · `Firewood` |
| 13 | `office` | DeadEnd | 2×2 | 사무실 | `RectangularTable` · `Shelf_01` · `Cupboard` · `ChestBox` · `Poster` |
| 14 | `coop_small` | DeadEnd | 2×2 | 작은 닭장 | `SM_Chicken_Coop` · 건초 |
| 15 | `delivery` | Special | 2×2 | 반납 항아리 방 | 항아리 |
| 16 | `kitchen` | Special | 2×3 | 조리대 방 | 조리대 · 냄비 · 선반 |

**소품은 이미 보유하고 있다** — Dreamscape Farmlands의 `Meshes/Props/Interior`(58) ·
`Exterior`(84), 총 142종. 양계장에 필요한 게 거의 다 있다.

seg kind 표의 `hay` · `trough` · `crate` · `brooder` 행이 그동안 **"배치 규칙이 기획에 없어서"**
비어 있었는데, 그 빈칸이 이 설계를 기다린 것이다.

## 생성 알고리즘

1. **시작 모듈**을 격자 가장자리에 놓는다 (트럭이 붙는 변).
2. **본줄기**를 뻗는다 — 열린 소켓 하나를 골라, 맞는 타입(`Passage`/`Normal`)의 모듈을
   빈 칸에 놓는다. 칸 점유 검사로 겹침을 막는다.
3. **곁가지**를 쳐서 `DeadEnd`로 끝낸다. 리썰컴퍼니는 **본줄기 8~22개에 가지 22~25개** —
   지나는 곳 대부분이 막다른 곁가지다. 그 비율을 참고한다.
4. **`Special` 모듈**(납품·조리대)을 필요한 수만큼 배치.
5. **안 쓴 소켓을 벽으로 막는다.**
6. **모듈마다 소품 세트를 하나 뽑아 깐다.**

시드 결정성·무복제 구조는 그대로다 — 모든 머신이 시드로 각자 만든다
([../../systems/map.md](../../systems/map.md)).

## 버리는 것 / 남는 것

**버림**
- `CL9SiteLayout`의 미로 로직 전부 — Kruskal · `OpenH`/`OpenV` · `CarveFloor` · 케이지 열 · 플러그
- `FCL9SiteFrame`의 고정 프레임 — 축사 외곽 · 계단 복도 · 층 낙차
- `UCL9SiteDef`의 격자 컬럼들 — `AisleCount` · `ColCount` · `AisleWidth` · `ColWidth` ·
  `RowDepth` · `OpenWidth` · `PlugWidth` · `TargetOpen*` (`JarCount*`는 모듈 배치로 대체)
- `CL9.Map.Determinism` / `Connectivity` / `Segments` 테스트 — 새 생성기용으로 다시 쓴다

**남음**
- **조각 표**(`UCL9PieceDef`, 역할 × 테마) — 모듈 껍데기를 세우는 재료. 그대로 쓴다.
- **`ACL9SiteMap`의 타일링 배치** — 발자국을 조각으로 채우는 코드. 그대로 쓴다.
- seg kind 표 — 벽·바닥·천장 종류. `cage`·`plug`는 쓸모가 줄고 소품 행들이 살아난다.
- 시드 복제 구조(`FCL9SiteMapKey`) · 에디터 콘솔 `cl9.seed`

## 결정 — 미결 5개 (2026-09-08 사용자, 2차)

### 1. 소켓은 경계 칸 단위다

표는 N/S/E/W **4비트**만 들고, "이 변이 열림"은 그 변에 닿은 **모든 경계 칸이 문 후보**라는
뜻으로 읽는다. 문은 칸 중앙에 서고 폭은 `door` 조각의 실측치를 쓴다.

"변 중앙에 하나 고정"은 성립하지 않는다 — 16종의 크기가 홀·짝이 섞여 있어서(3×2 · 2×2 ·
2×3 · 3×3), 변이 짝수 칸인 모듈은 중앙이 **칸 경계에** 떨어진다. 칸 단위로 잡으면 3×3 홀은
변마다 문 후보가 3개 생기고, `corridor`(1×2)는 짧은 변 두 개만 켜서 곧은 복도가 된다.

### 2. 높이차는 전파하지 않는다 (net 0)

계단 모듈은 **자기 안에서 올라갔다 도로 내려온다.** 이웃 모듈의 바닥 높이는 건드리지 않는다.

기준은 사용자가 준 것이다 — **"나중에 에셋이 추가돼도 잘 적용될 수 있어야 한다."** 진짜 전파
방식은 새 계단 모듈을 한 행 추가할 때마다 "이 ΔZ가 고리를 만들면 높이가 모순되는가"를 따져야
하고, 그래서 배치기를 트리 구조에 묶어야 한다 — 에셋을 늘릴수록 생성기 쪽 제약이 함께
늘어난다. net 0은 새 모듈이 격자 규칙을 전혀 건드리지 않으므로 표에 행만 추가하면 끝이고,
격자 칸 점유 검사가 2D로 유지되며, **고리(우회로)를 살릴 수 있다** — REPO도 고리가 있고,
추출 게임에서 도망갈 길의 유무는 체감이 완전히 다르다.

다만 껍데기를 세우는 함수는 "이 모듈의 바닥 Z"를 인자로 받는 형태로 짠다. 언젠가 진짜 층이
필요해지면 그 인자에 0이 아닌 값을 넣는 것이 착수 지점이 된다. 지금 필드를 미리 늘리지는 않는다.

### 3. 입구 하나, 비상구 하나 — 딱 둘

- **입구**는 `yard_gate` 하나. 트럭이 여기 붙는다.
- **비상구**는 `flapIn` 하나. 시작 모듈에서 **가장 먼** 경계 접촉 `DeadEnd` 모듈의 안 쓴 바깥
  소켓에 단다. 일방향이라 되돌아 들어올 수 없다 — 리썰컴퍼니의 fire exit과 같은 장치이고,
  "들어갈 땐 길고 나올 땐 짧다"를 만든다.

개수가 고정이므로 표 컬럼을 만들지 않고 상수로 둔다.

### 4. 칸 목록 위에 `LevelPoint`를 세운다

생성 결과가 **칸 목록**을 들고 있다 — 칸마다 (사이트 로컬 좌표 · 모듈 인덱스 · 모듈 `Type` ·
시작 모듈로부터의 홉 거리). 그 위에 **용도 태그가 붙은 지점 목록**(`FCL9LevelPoint`:
`Monster` · `Loot` · `Jar` · `Patrol`)을 얹는다. 지금은 칸 목록에서 파생해 만들지만, 나중에
소품 세트가 "이 선반 위" 같은 자기 지점을 밀어 넣을 자리가 여기다 (REPO `ValuablePropSwitch`).

질의는 지금 하나뿐이다 — `PointsOfPurpose(용도)`가 **시작 모듈에서 먼 것부터** 정렬해
돌려준다. `ACL9GameMode::SpawnNight`의 세 소비처가 `CellToWorld` 대신 이걸 부르고,
몬스터는 그 목록의 앞쪽 60%에서, 재료는 전체에서 고르며, 항아리는 Jar 용도가 켜진
모듈마다 하나씩 선다. REPO의 `LevelPointGetWithinDistance` 같은 나머지 질의는 **부르는
곳이 생길 때 만든다** — 지금 만들면 쓰지도 않는 채로 굳는다.

용도를 정하는 것은 C++이 아니라 표다 — 모듈 표의 `PointPurposes` 컬럼이다. 그래서
"항아리는 납품 방에만" 같은 규칙이 코드에 별칭으로 박히지 않는다.

### 5. 한 판 20 → 24 → 30 모듈

기존 규약(사이트가 기준값 · 진행 커브가 배율)을 그대로 따른다.

- `UCL9SiteDef::BaseModuleCount = 24`
- `UCL9ProgressionDef::SiteSizeCurve` — 1일차 **0.85** → 5일차 **1.0** → 10일차 **1.25**, 이후 유지

곱하면 1일차 20 · 5일차 24 · 10일차 30이 나와 결정 5의 "한 판 20~30모듈"을 그대로 채운다.
