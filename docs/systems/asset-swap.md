# 에셋 팩 교체 — 조각 표와 배치 규약

> 맵을 무엇으로 짓는지를 데이터로 빼 놓은 곳. 생성기 자체는 [map.md](map.md),
> 조각 종류표는 [../plan/data/segkind.md](../plan/data/segkind.md),
> 모듈 표는 [../plan/data/module.md](../plan/data/module.md), 사이트 컬럼은
> [../plan/data/site.md](../plan/data/site.md)에 있다.

**새 에셋 팩을 사면 고치는 곳은 표 두 개다. C++은 건드리지 않는다.**

---

## 왜 이 구조인가

맵 조각 종류표가 메시를 직접 가리키면, 팩을 갈아 끼울 때 행마다 메시를 다시 찍어야 하고
같은 역할의 변형(`Wall_A_01`/`_02`/`_03`)을 섞어 쓸 수도 없다. 그래서 한 겹을 끼웠다 —
**역할(role)**. 생성기는 "여기에 벽이 선다"까지만 말하고, 그 역할이 어느 메시인지는
조각 표가 정한다.

```
DA_SegKindTable  (생성기 쪽 — 이 발자국엔 무엇이 서는가)
  barn      → Piece: wall         Placement: Tile     Height: 580
  barnStub  → Piece: wallQuarter  Placement: Tile     Height: 580
  wall      → Piece: fence        Placement: Tile     Height: 288
  wallStub  → Piece: fence        Placement: Tile     Height: 288
  floor     → Piece: floor        Placement: Grid     Height: 30
  yard      → Piece: yard         Placement: Grid     Height: 30
  tread     → Piece: tread        Placement: Tile     Height: 30
  ceiling   → Piece: ceiling      Placement: Grid     Height: 30
  flapIn    → Piece: door         (액터 ACL9FlapIn)     Height: 580
        ↓ 역할 이름으로만 가리킨다
DA_PieceTable    (에셋 쪽 — 그 역할은 어느 메시인가)   ← 팩이 바뀌면 여기만 고친다
  Alias(역할)  SubAlias(테마)   Variants                       (2026-09-11부터 자체 제작 CL9Art)
  wall         henhouse         [ SM_Henhouse_Wall_500_A ]
  wallHalf     henhouse         [ SM_Henhouse_Wall_250_A ]     ← 아직 아무 seg kind도 안 쓴다
  wallQuarter  henhouse         [ SM_Henhouse_Wall_125_A ]
  fence        henhouse         [ SM_Henhouse_Fence_250_A ]
  door         henhouse         [ SM_Henhouse_Door_A ]
  floor        henhouse         [ SM_Henhouse_Floor_500_A ]
  ceiling      henhouse         [ SM_Henhouse_Ceiling_500_A ]
  yard         henhouse         [ SM_Henhouse_YardGround_500_A ]
  tread        henhouse         [ SM_Henhouse_Tread_A ]
  box          (비움)            [ /Engine/BasicShapes/Cube ]
```

Dreamscape Farmlands 조각을 가리키던 행은 2026-09-11에 전부 자체 제작 조각으로 바꿨다 — 그 팩은
저장소에서 제외됐으므로(5da02ff) 팩을 가리키는 행은 클론에서 깨진다. 아래 "이번 팩 실측표"는
그 팩의 기록으로 남겨 둔다.

**조각의 치수는 어느 표에도 적지 않는다.** `ACL9SiteMap`이 메시 바운드에서 직접 읽는다 —
손으로 옮겨 적은 숫자는 팩이 바뀌는 순간 조용히 틀린 값이 되고, 그게 이 표를 만든 이유를
무너뜨린다. **벽 두께(`FCL9SiteParams::WallThickness`)는 `barn` 조각의 얇은 축에서,
문 폭(`DoorWidth`)은 `door` 조각의 넓은 축에서** 온다 — 둘 다 `ACL9SiteMap::MeasurePieces`가 읽는다.

---

## 배치 모드 — 세 축을 따로 다룬다

`UCL9SegKindDef.Placement`가 셋 중 하나다.

| 모드 | 무엇 | 쓰는 곳 |
|---|---|---|
| `Stretch` | 조각 하나를 발자국 크기로 늘린다. **100cm 단위 정육면체(중심 피벗)를 가정한다** | 회색 상자(`box`) |
| `Tile` | 조각을 자연 크기로 깔아 나간다 — 길이 축 한 방향 | 판자·벽돌 무늬가 있는 벽·울타리·디딤판 |
| `Grid` | 판 조각을 두 평면 축으로 격자 깔기 한다 — 축마다 `round(발자국/조각)`장, 높이는 발자국 높이 | 500×500 바닥·천장·마당 타일 (2026-09-11) |

`Grid`는 모듈 사각형이 칸의 정수배라 맞춤이 1.0으로 떨어지고, 계단실의 위층 띠(250×1000)처럼
짧은 발자국만 한 장이 눌린다 — 그건 늘 있는 일이라 `Grid` 종류는 맞춤 경고를 내지 않고 기록만
남긴다. 피벗은 Tile처럼 바운드 중심으로 보정한다(바닥 조각은 윗면 피벗, 천장·마당은 바닥 피벗인데
다 같이 맞는 이유).

`Tile`이 축마다 다르게 행동하는 것이 이 규약의 핵심이다.

| 축 | 처리 |
|---|---|
| **길이 축** (발자국의 긴 쪽) | `N = round(발자국 / 조각)` 장을 늘어놓고, 나머지는 N장에 균등 분배해 흡수 |
| **두께 축** (짧은 쪽) | **늘리지 않는다.** 자연 두께 그대로 발자국 가운데에 세운다 |
| **높이 축** | `M = round(높이 / 조각높이)` 단을 쌓고, 아래 단들은 자연 높이, 남는 높이는 맨 윗단이 받는다 |

두께를 늘리지 않는 이유: **발자국의 두께는 비워 둔 공간이지 채워야 할 부피가 아니다.**
모듈 경계의 발자국이 두껍게 잡혀도 실제로 서는 건 40cm짜리 벽 한 장이고, 그 자리가
막혀 있다는 판정은 그대로다.

윗단이 남는 높이를 받는 이유: 눈이 가는 아래쪽 비율을 지키는 편이 벽 전체를 눌러 놓는
것보다 낫다. 축사 벽 580cm에 400cm 조각이면 아래 400은 제 비율 그대로 서고 위 180만
0.45배로 눌린다. (벽 높이는 2026-09-11에 630 → 580으로 낮췄다 — 층 높이가 610(최고 방 550 +
판 30+30)이라 630짜리 벽이 2층 바닥을 20cm 뚫고 나왔다. 580 = 550 + 천장 판 30.)

**피벗은 신경 쓰지 않아도 된다.** 조각의 원점이 메시 중심이든 구석이든 격자 원점이든,
바운드 중심을 기준으로 자동 보정된다. 램프(경사 슬래브)만은 기울어져 있어 쌓을 수가 없으니
`Placement`와 무관하게 늘려서 깐다.

### 안 맞으면 알려 준다

길이 축 보정 배율이 0.85~1.15를 벗어나면 `LogCL9`에 경고가 뜬다.

```
LogCL9: Warning: Site map: 'barn' pieces are stretched +25% along their run —
        retune CellSize to this pack's module (docs/systems/asset-swap.md).
```

경고가 떠도 맵은 선다 — 다만 예쁘지 않다. 사이트 행의 `CellSize`를 그 팩의 모듈에 맞추라는 신호다.
경고가 없어도 종류마다 한 줄씩 맞춤 기록이 남으니 그걸 보고 판단하면 된다.

```
LogCL9: Site map: 'barn' tiled with a 500 x 40 x 400 piece — 128 instances, worst run fit 1.02.
```

---

## 변형 — 같은 벽이 줄줄이 서지 않게

한 역할에 메시를 여럿 넣으면 자리마다 하나가 골라진다. 고르는 값은 **그 자리의 좌표
해시**다 — 난수 스트림을 쓰면 호출 순서에 의존해서 머신마다 갈릴 수 있는데, 좌표 해시는
순서와 무관하게 모든 머신에서 같은 값이 나온다. 지형이 복제되지 않고 시드에서 각자
만들어지는 구조라(map.md) 이게 지켜지지 않으면 사람마다 다른 벽을 보게 된다.

칸 격자는 첫 변형의 크기로 잡고, 각 칸에는 그 자리에 뽑힌 변형을 그 칸에 맞춰 앉힌다 —
변형끼리 크기가 조금 어긋나도 칸 밖으로 삐져나오지 않는다.

---

## 테마 — 여러 벌의 조각 묶음

조각 표의 `SubAlias`가 테마고, 목적지 행(`UCL9SiteDef.Theme`)이 어느 테마로 지을지 고른다.
테마가 그 역할을 갖추지 않았으면 **테마 칸이 빈 공용 행**으로 떨어진다 — 회색 상자처럼
테마를 타지 않는 조각이 거기 산다.

격자 치수는 조각 모듈에 맞춰야 예쁘게 떨어지므로, 실질적으로 **테마 = 조각 묶음 + 그
모듈에 맞는 격자 숫자** 한 쌍이다. 같은 배치를 다른 테마로 내려면 목적지 행을 복제해서
`Theme`만 바꾼다.

층마다 다른 테마(1층은 목조, 지하는 콘크리트)가 필요해지면 조회가 이미 테마를 인자로
받으므로 `Theme` 컬럼을 층별로 늘리는 정도로 확장된다. 지금은 목적지당 하나다.

### 반쯤 만든 테마는 로드 때 걸린다

`UCL9DefRegistry::CheckSiteReferences`가 목적지의 테마에 대해 **생성기가 쓰는 역할이
전부 있는지** 검사한다. 빠진 게 있으면 로그에 사유가 남고 `cl9.list`에 뜬다 — 테마를
절반만 만들어 놓고 맵이 까맣게 나와서 원인을 찾는 사고를 막는다.

---

## 팩 교체 절차

**1. 조각 표에 그 팩의 테마 행을 만든다.** 역할별로 메시를 넣는다. 아래 "역할" 절이
어떤 역할이 필요한지 말해 준다. 아직 살 조각이 없는 역할은 **행을 만들지 말고**, 그 역할을
쓰던 seg kind가 `box`(회색 상자)를 가리키게 둔다 — 빈 행을 남기면 그 자리가 조용히 비어 버린다.

**2. 목적지 행의 `Theme`를 그 테마로 바꾸고, `CellSize`를 그 팩의 모듈에 맞춘다.**
조각이 400cm 모듈이면 `CellSize`를 400으로. 이 단계를 건너뛰어도 맵은 서지만 조각이 늘어난다.

**`Theme`는 두 표를 한 이름으로 가리킨다** — 조각 표와 모듈 표다. 조각 표는 테마에 없는 역할이
공용 행으로 떨어지지만 **모듈 표에는 폴백이 없으므로**, 새 테마로 맵을 지으려면 그 스테이지의
모듈 행도 통째로 한 벌 있어야 한다 ([../plan/data/module.md](../plan/data/module.md)).

**3. 확인한다.** `cl9.seed 873`으로 다시 깔고 `LogCL9`를 본다.
- `has no usable piece for role` — 1단계에서 빠뜨린 역할이 있다
- `pieces are stretched ±NN%` — 2단계의 격자 숫자가 그 팩 모듈과 안 맞는다
- 둘 다 없고 `worst run fit`이 1.00 언저리면 끝이다

---

## 역할

### 생성기가 지금 쓰는 것

| 역할 | 쓰는 seg kind | 발자국이 요구하는 것 |
|---|---|---|
| `wall` | `barn`(실내 벽) | 긴 벽 한 장. **길이가 `CellSize`의 약수여야 예쁘다** |
| `wallQuarter` | `barnStub`(문·비상구 양옆 토막) | 칸 500에 문 250이면 125짜리 토막 — 그 길이의 벽 한 장 |
| `fence` | `wall`(야외 모듈의 벽) · `wallStub`(야외 문 옆 토막) | 야외 울타리. 높이 288. **125짜리 울타리가 없어 토막은 0.50으로 눌리고 경고가 난다** — 알려진 상태 |
| `door` | `flapIn`(일방향 비상구) | 문짝 한 장. 액터(`ACL9FlapIn`)가 **폭만** 개구부에 맞추고 두께·높이는 자연값으로 둔다(2026-09-11). 개구부 위쪽은 Body 상자가 막는다 |
| `floor` | `floor`(실내 바닥) | 500×500 판. `Grid` |
| `yard` | `yard`(야외 모듈의 바닥) | 500×500 흙·자갈 판. `Grid` |
| `tread` | `tread`(계단 디딤판) | 디딤판 한 장. `Tile`이 계단 진행 방향에 맞춰 90° 돌린다 — 500 폭 통로에 264짜리 두 장(맞춤 0.95) |
| `ceiling` | `ceiling` | 500×500 판. `Grid`. 양계장은 천장을 꺼 두어(site `bBuildCeilings`) 켤 때 바로 쓰인다 |
| `box` | (없음 — 폴백) | 무늬 없는 100cm 정육면체. `Stretch` 전용 |

**`door` 조각을 문틀로 쓰면 안 된다.** 문짝 메시를 개구부에 채우면 콜리전이 생겨 문이 통째로
막힌다 — 그래서 `door` seg kind 행을 만들지 않았고, 지금 문은 문틀 없는 개구부다. 생성기도
`door` 종류가 없는 것을 에러로 세지 않는다.

**문 폭은 250이다** — `door` seg kind 행이 없어서 `MeasurePieces`가 문짝을 못 읽고 기본값 250을
쓴다. 문짝 `SM_Henhouse_Door_A`(132)와 디딤판(264 = 2×132)은 132 세계로 만들어졌고, Wall_125/250은
250 세계로 만들어졌다 — 2026-09-11 사용자 결정으로 **250을 유지**한다(문짝은 폭 1.89×, 디딤판은
두 장). `door` 행을 넣으면 문 폭이 132로, 토막이 184로 바뀌어 Wall_250(0.74)·Wall_125(1.47)
어느 것도 안 맞게 되니 넣지 말 것.

`wallHalf` 역할은 **지금 아무 seg kind도 안 쓴다.** 통로 마개(`plug`)가 사라졌기 때문이다.
문 옆 토막은 125라 `wallQuarter`가 맡는다 — `SM_Henhouse_Wall_250_A`는 행에 있지만 소비처가 없다.

`cage` 역할도 비어 있다 — 케이지 열이 사라져 seg kind 자체가 안 불린다. 모듈 안 소품으로
되살아날 여지는 남아 있다.

### 팩에 있지만 아직 안 쓰는 것

배치 규칙이 기획에 없어서 표에 넣지 않았다. 규칙이 생기면 여기서 조각을 가져온다 —
폴더를 다시 뒤질 일이 없도록 분류만 해 둔다.

| 역할 | Dreamscape Farmlands 조각 |
|---|---|
| `wallDoor` 문 달린 벽 | `SM_House_Wall_5x4_Door_A~E_01~03` |
| `wallWindow` 창 달린 벽 | `SM_House_Wall_5x4_Window_A~C_01~03` |
| `wallInterior` 실내벽 | `SM_House_Wall_Interior_*`, `SM_House_Wall_5x4_Interior_Door_*` |
| `corner` 코너 기둥 | `SM_House_Corner_01/02`, `SM_House_Corner_Beam` |
| `pillar` 기둥 | `SM_Pillar_Exterior_01/02_*`, `SM_Support_Pillar_01` |
| `floorTile` 바닥판 | `SM_House_Floor_5x4_01`, 해치 4종, 오버행 |
| `foundation` 기초 | `SM_House_Foundation_2x4/5x4*` |
| `roofTile` 지붕 | `SM_House_Roof_01_*`(30종), `SM_House_Roof_02_*`(7종) |
| `roofBeam` 서까래 | `SM_House_Roof_Beam_*`, `SM_House_Floor_Beam` |
| `stairs` 계단 | `SM_Stairs_Interior_A/B/C_*`, `SM_Stairs_Exterior_*` |
| `balcony` 발코니·테라스 | `SM_Balcony_01/02*`, `SM_Terrace_Floor_*`, `SM_Platform_Exterior_*` |
| `prop` 소품 | `SM_Fireplace_*`, `SM_Bar_*`, `SM_RoofSpike` |
| (모듈러 아님) | `SM_Barn`, `SM_Chicken_Coop`, `SM_Well`, `SM_Windmill_*` — 통짜라 격자에 못 얹는다 |

---

## 이번 팩 실측표 (Dreamscape Farmlands)

에디터에서 `StaticMeshTools.get_bounds`로 직접 잰 값이다. 코드는 이 표를 읽지 않는다 —
사람이 격자 숫자를 정할 때 보는 참고표다.

| 조각 | 폭 × 두께 × 높이 (cm) | 피벗 |
|---|---|---|
| `SM_House_Wall_5x4_A_01` | 500 × 40 × 400 | X 중앙, Y −290\~−250, Z 0\~400 |
| `SM_House_Wall_2_5x4_01` | 250 × 40 × 400 | 〃 |
| `SM_House_Wall_5x4_Door_A_01` | 500 × 43 × 400 | 〃 |
| `SM_House_Wall_5x4_Window_A_01` | 500 × 48 × 400 | 〃 |
| `SM_House_Wall_Interior_A_01` | 500 × 32 × 391 | Y +217\~249 |
| `SM_House_Floor_5x4_01` | 500 × 500 × 36 | 중앙 |
| `SM_House_Foundation_5x4` | 500 × 50 × 300 | Y −305\~−255 |
| `SM_House_Corner_01` | 40 × 40 × 400 | 코너 바깥 |
| `SM_House_Roof_01_Tiling_Base` | 308 × 501 × 410 | — |
| `SM_House_Roof_02_Tiling` | 651 × 502 × 964 | — |
| `SM_Pillar_Exterior_02_Straight` | 192 × 29 × 424 | 중앙 |
| `SM_VillageWall_TIling` / `_2` | 201 × 24 × 202 | 중앙 |
| `SM_StoneWall_01` / `_02` | 299 × 35 × 136 | 중앙 (**바운드 > 돌 몸통** — 아래 참조) |
| `SM_Door_A` | 132 × 31 × 256 | X −133\~−1 (한쪽 끝) |
| `SM_Stairs_Interior_A_01` | 170 × 316 × 275 | — |
| `SM_Barn` (통짜) | 714 × 504 × 570 | 중앙 |
| `SM_Chicken_Coop` (통짜) | 230 × 264 × 223 | 중앙 |

**모듈은 500cm**다. 그래서 사이트 행의 `CellSize`를 500으로 잡았고 — 격자 칸 하나가 조각
한 장이다 — 벽 두께 40은 조각에서 읽어 온다.

---

## 실전에서 배운 것 (2026-09-07 첫 배치)

`cl9.seed 873`으로 실제로 깔아 보고 고친 것들이다. **셋 다 코드가 아니라 조각 표만 고쳤다** —
이 구조가 노린 바로 그 지점이다.

**1. 바운드가 조각의 "이어붙일 수 있는 폭"과 다를 수 있다.** `SM_StoneWall_01`은 바운드가
299cm인데 돌 몸통은 그보다 좁고 중간에 튀어나온 갓돌이 바운드를 넓히고 있었다. 그대로 깔면
담에 세로 틈이 줄줄이 생긴다. 코드는 바운드밖에 모르므로 이건 **조각을 잘못 고른 것**이다 —
팩이 타일링용으로 만든 `SM_VillageWall_TIling`(201 × 24 × 202)으로 바꾸니 이음매 없이 붙었다.
**교훈: 이름에 `Tiling`이 붙은 조각이 있으면 그걸 먼저 써 본다.**

**2. 변형이라고 다 같은 변형이 아니다.** `SM_StoneWall_02`는 `_01`의 색만 다른 짝이 아니라
**일부러 무너뜨린 폐허 변형**이었다. 한 역할에 같이 넣었더니 멀쩡한 담 사이사이에 구멍이 뚫렸다.
**교훈: 변형은 "같은 자리에 아무거나 와도 되는 것"만 묶는다.** 손상·끝단(`_Left`/`_End`) 변형은
따로 둔다.

**3. 발자국이 조각보다 짧으면 눌린다.** 지하 입구 위를 막는 축사 벽 한 장이 통로 폭(250cm)짜리
발자국인데 `wall` 역할의 조각은 500cm라 0.50배로 눌린다 — 경고가 정확히 이걸 잡아냈다.
지금은 그대로 두었다(계단 복도 천장에 가려 거의 안 보인다). 제대로 고치려면 셋 중 하나다:
역할에 "더 짧은 역할" 폴백을 두거나, 생성기가 그 자리를 다른 종류로 내보내거나, 그냥 받아들이거나.
**기획 판단이 필요해 미결로 남긴다.**

---

## 알려진 거친 부분

- **`flapIn`은 늘려서 맞춘다.** 개구멍 하나뿐이라 타일링이 과하다. 문짝(256cm)이 개구부
  높이(630cm)로 늘어나는데, 개구멍 높이를 벽과 같이 둘지는 재검토 대상이다.
- **코너 조각을 안 쓴다.** 타일링한 벽이 모서리에서 겹칠 뿐이다. `SM_House_Corner_01`이
  그 자리를 위한 조각인데, 생성기가 "여기는 모서리"를 따로 말해 주지 않는다.
- **바닥·천장·마당은 `Grid` 타일이다** (2026-09-11). 30모듈 판에 바닥 142장·마당 15장 —
  모듈 단위 한 장씩이라 인스턴스가 수천 개가 되지 않는다. 패드에서 입구까지 8m 마당 앞뜰은
  아직 회색 바닥(Lvl_Henhouse `Floor`)이다.
- **문 옆 벽 토막은 `barnStub`/`wallStub`로 분리했다** (2026-09-11). 실내는 Wall_125가 1.00으로
  맞고, 야외 울타리 토막만 125짜리 울타리가 없어 Fence_250을 0.50으로 눌러 쓴다 — 시드마다
  `wallStub` 경고 한 줄이 남는 게 정상이다. 그 경고 말고 다른 맞춤 경고가 뜨면 그게 문제다.
- **소품 세트** — 모듈 표의 `PropSets`가 인스턴스마다 하나 뽑혀 깔린다 (module.md 결정 7,
  구현 2026-09-11). 소품마다 변 조건(`SideClosed`/`SideOpen`)이 있어 벽걸이 표식·둥지는 그
  변이 막혔을 때만 선다(REPO `ModulePropSwitch`). 몸 있는 소품이 문 자리나 스폰 지점(칸 중심
  ±130)을 가리면 `LogCL9` 경고가 난다.
