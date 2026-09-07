# 에셋 팩 교체 — 조각 표와 배치 규약

> 맵을 무엇으로 짓는지를 데이터로 빼 놓은 곳. 생성기 자체는 [map.md](map.md),
> 조각 종류표는 [../plan/data/segkind.md](../plan/data/segkind.md),
> 격자 컬럼은 [../plan/data/site.md](../plan/data/site.md)에 있다.

**새 에셋 팩을 사면 고치는 곳은 표 두 개다. C++은 건드리지 않는다.**

---

## 왜 이 구조인가

맵 조각 종류표가 메시를 직접 가리키면, 팩을 갈아 끼울 때 행마다 메시를 다시 찍어야 하고
같은 역할의 변형(`Wall_A_01`/`_02`/`_03`)을 섞어 쓸 수도 없다. 그래서 한 겹을 끼웠다 —
**역할(role)**. 생성기는 "여기에 벽이 선다"까지만 말하고, 그 역할이 어느 메시인지는
조각 표가 정한다.

```
DA_SegKindTable  (생성기 쪽 — 이 발자국엔 무엇이 서는가)
  barn   → Piece: wall      Placement: Tile     Height: 630
  plug   → Piece: wallHalf  Placement: Tile     Height: 630
  wall   → Piece: fence     Placement: Tile     Height: 288
  cage   → Piece: box       Placement: Stretch  Height: 630
  floor  → Piece: box       Placement: Stretch  Height: 30
        ↓ 역할 이름으로만 가리킨다
DA_PieceTable    (에셋 쪽 — 그 역할은 어느 메시인가)   ← 팩이 바뀌면 여기만 고친다
  Alias(역할)  SubAlias(테마)   Variants
  wall         henhouse         [ Wall_5x4_A_01, _A_02, _A_03 ]
  wallHalf     henhouse         [ Wall_2_5x4_01, _02, _03 ]
  fence        henhouse         [ VillageWall_TIling, VillageWall_TIling_2 ]
  door         henhouse         [ Door_A, Door_C, Door_D, Door_E ]
  box          (비움)            [ /Engine/BasicShapes/Cube ]
```

**조각의 치수는 어느 표에도 적지 않는다.** `ACL9SiteMap`이 메시 바운드에서 직접 읽는다 —
손으로 옮겨 적은 숫자는 팩이 바뀌는 순간 조용히 틀린 값이 되고, 그게 이 표를 만든 이유를
무너뜨린다. 벽 두께(`FCL9SiteFrame::Wall`)까지 `barn` 조각의 실측 두께에서 온다.

---

## 배치 모드 — 세 축을 따로 다룬다

`UCL9SegKindDef.Placement`가 둘 중 하나다.

| 모드 | 무엇 | 쓰는 곳 |
|---|---|---|
| `Stretch` | 조각 하나를 발자국 크기로 늘린다 | 회색 상자, 바닥·천장처럼 늘어나도 티가 안 나는 면 |
| `Tile` | 조각을 자연 크기로 깔아 나간다 | 판자·벽돌 무늬가 있는 모듈러 조각 전부 |

`Tile`이 축마다 다르게 행동하는 것이 이 규약의 핵심이다.

| 축 | 처리 |
|---|---|
| **길이 축** (발자국의 긴 쪽) | `N = round(발자국 / 조각)` 장을 늘어놓고, 나머지는 N장에 균등 분배해 흡수 |
| **두께 축** (짧은 쪽) | **늘리지 않는다.** 자연 두께 그대로 발자국 가운데에 세운다 |
| **높이 축** | `M = round(높이 / 조각높이)` 단을 쌓고, 아래 단들은 자연 높이, 남는 높이는 맨 윗단이 받는다 |

두께를 늘리지 않는 이유: **발자국의 두께는 비워 둔 공간이지 채워야 할 부피가 아니다.**
통로 마개(`plug`)의 발자국이 180cm 두꺼워도 실제로 서는 건 40cm짜리 벽 한 장이고,
통로를 막는다는 판정은 그대로다.

윗단이 남는 높이를 받는 이유: 눈이 가는 아래쪽 비율을 지키는 편이 벽 전체를 눌러 놓는
것보다 낫다. 축사 벽 630cm에 400cm 조각이면 아래 400은 제 비율 그대로 서고 위 230만
0.58배로 눌리는데, 그 높이는 630에 있는 천장에 가려 잘 보이지 않는다.

**피벗은 신경 쓰지 않아도 된다.** 조각의 원점이 메시 중심이든 구석이든 격자 원점이든,
바운드 중심을 기준으로 자동 보정된다. 램프(경사 슬래브)만은 기울어져 있어 쌓을 수가 없으니
`Placement`와 무관하게 늘려서 깐다.

### 안 맞으면 알려 준다

길이 축 보정 배율이 0.85~1.15를 벗어나면 `LogCL9`에 경고가 뜬다.

```
LogCL9: Warning: Site map: 'barn' pieces are stretched +25% along their run —
        retune the grid columns to this pack's module (docs/systems/asset-swap.md).
```

경고가 떠도 맵은 선다 — 다만 예쁘지 않다. 격자 컬럼을 그 팩의 모듈에 맞추라는 신호다.
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

**2. 목적지 행의 `Theme`를 그 테마로 바꾸고, 격자 컬럼을 그 팩의 모듈에 맞춘다.**
조각이 400cm 모듈이면 `ColWidth`를 800(2장), `AisleWidth`를 400으로. 이 단계를 건너뛰어도
맵은 서지만 조각이 늘어난다.

**3. 확인한다.** `cl9.seed 873`으로 다시 깔고 `LogCL9`를 본다.
- `has no usable piece for role` — 1단계에서 빠뜨린 역할이 있다
- `pieces are stretched ±NN%` — 2단계의 격자 숫자가 그 팩 모듈과 안 맞는다
- 둘 다 없고 `worst run fit`이 1.00 언저리면 끝이다

---

## 역할

### 생성기가 지금 쓰는 것

| 역할 | 쓰는 seg kind | 발자국이 요구하는 것 |
|---|---|---|
| `wall` | `barn`(축사 외벽) · `stair`(계단 복도 벽) | 긴 벽 한 장. 길이가 격자 열 폭의 약수여야 예쁘다 |
| `wallHalf` | `plug`(통로 마개) · 벽 자투리 | `wall`의 절반 길이 |
| `fence` | `wall`(마당 경계) | 야외 울타리. 높이 288 |
| `door` | `flapIn`(일방향 개구멍) | 문짝 한 장. 액터(`ACL9FlapIn`)라 늘려서 맞춘다 |
| `box` | `floor` · `ceiling` · (당분간) `cage` | 무늬 없는 면. `Stretch` 전용 |

`cage`(케이지 열) 역할은 **아직 비어 있다** — Dreamscape Farmlands에 케이지 조각이 없어서
`cage` seg kind가 `box`를 가리키고 있다. 케이지 에셋을 사면 조각 표에 `cage` 역할을 만들고
seg kind의 `Piece`를 `cage`로, `Placement`를 `Tile`로 바꾸면 끝이다.

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

**모듈은 500cm**(반 칸 250)다. 그래서 격자를 `ColWidth` 1000(2장) · `AisleWidth` 250(반 장)으로
잡았고, 벽 두께 40은 조각에서 읽어 온다.

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
- **바닥·천장은 늘린 회색 면이다.** 마당 전체를 500 타일로 깔면 인스턴스가 수천 개가 되고
  야외 지형은 ⑤ 5b에서 지형으로 갈 예정이라, 무늬는 머티리얼 타일링으로 처리한다.
- **`cage`가 아직 회색 상자다.** 케이지 조각이 있는 팩을 사기 전까지는 그대로다.
- **지하 입구 위 축사 벽 한 장이 0.50배로 눌린다** (위 "실전에서 배운 것" 3번). 로그가 매번
  경고를 내므로, 이 경고가 사라질 때까지는 다른 경고를 못 알아본다는 부작용이 있다.
