# 맵 조각 — `UCL9SegKindDef`

벽·바닥·소품 같은 지형 조각의 종류표. 맵 배치 툴이 이걸 읽어서 액터를 세운다.

생성기가 좌표를 뱉고, 이 표가 "그 자리에 뭘 세울지"를 정한다.

> **2026-09-08.** 모듈 조립 생성기로 갈아엎으면서 쓰이는 종류가 바뀌었다
> ([module.md](module.md)). **생성기가 지금 부르는 것은 여섯이다** —
> `floor` · `ceiling` · `barn`(실내 벽) · `wall`(야외 울타리) · `door`(문틀) · `flapIn`(비상구).
> `cage` · `plug` · `stair`는 **더 이상 안 불린다** (케이지 열·통로 마개·계단 복도가 사라졌다).
> 행을 지우진 않았다 — 소품으로 되살아날 여지가 있다.
> `door` 행은 **아직 없다.** 없으면 문 자리가 그냥 뻥 뚫린 개구부가 되고 로그가 알려 준다.
> **2026-09-11**에 야외 바닥 `yard`·디딤판 `tread`·문 옆 토막 `barnStub`/`wallStub`이 더해졌다 — 지금
> 생성기가 내는 종류와 조각 배정의 정본은 [../../systems/asset-swap.md](../../systems/asset-swap.md)다.

---

## 컬럼

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `Alias` | `FName` | `wall` · `barn` · `cage` … |
| `Piece` | `FName` | **세울 조각의 역할 이름** — 실제 메시는 조각 표가 갖는다 |
| `Placement` | enum `ECL9SegPlacement` | `Stretch` 늘리기 / `Tile` 자연 크기로 깔기 / `Grid` 판 조각을 두 평면 축으로 격자 깔기(500×500 바닥·천장·마당 타일 — 2026-09-11, [../../systems/asset-swap.md](../../systems/asset-swap.md)) |
| `ActorClass` | 소프트 참조 | 전용 BP (비우면 공용) |
| `Height` | float | 실제 3D 높이 (cm) |
| `CollisionProfile` | `FName` | 콜리전 프로파일 |
| `bBlocksSight` | bool | **★ 시야 · 소리 파동 · 카메라를 한꺼번에 결정한다** |
| `MinimapColor` | `FColor` | 미니맵 색 |

---

**`BaseZMode`는 2026-09-08에 지웠다.** 층과 경사로가 사라져 `WorldTop`("램프 꼭대기에서 던져도
못 넘게")이 가리킬 대상이 없어졌다. 이제 바닥만 기준점 아래로, 나머지는 위로 자란다.

## 종류

| Alias | 높이 (u) | 높이 (cm) | `bBlocksSight` | 무엇 |
|---|---|---|---|---|
| `wall` | 96 | 288 | ✔ | 마당 경계 |
| `barn` | 210 | 630 | ✔ | 축사 외벽 |
| `cage` | 210 | 630 | ✔ | 케이지 열 |
| `plug` | 210 | 630 | ✔ | 통로 마개 |
| `truck` | 92 | 276 | ✔ | 트럭 |
| `hay` | 56 | 168 | ✔ | 건초 |
| `trough` | 26 | 78 | **✘** | 물통 |
| `crate` | 40 | 120 | **✘** | 상자 |
| `flapIn` | 210 | 630 | ✔ | 개구멍 덮개 — 일방향. 한 판에 하나, 네 방향 어디에나 달린다 |
| `door` | — | — | ✔ | **아직 없는 행.** 모듈을 잇는 문틀 — 이 조각의 넓은 축이 곧 문 폭이다 |
| `stair` | 210 | 630 | ✔ | 계단 복도 벽 |
| `brooder` | 40 | 120 | **✘** | 보온 상자 |
| `pen` | — | 130 | **✘** | 우리 울타리 (2026-09-28) — 가슴 높이 철망. 몸은 막고 시야는 연다. 조각 역할 `penFence`, `Tile` |

`pen`의 높이 130은 조각(`SM_Pen_Fence_250_A`)의 실제 높이와 같은 값이다 — 배치가 `Tile`이라
높이 축으로 한 장이 딱 맞게 서고, 눌리거나 두 장이 쌓이지 않는다. u 단위로는 43.33이라
정수가 아니다(다른 행처럼 3의 배수가 아니다) — 이 조각은 u 격자에서 나온 치수가 아니라
캐릭터 치수(캡슐 반높이 90 < 130 < 눈높이 156)에서 나왔다 ([../../asset/map/pen_fence.md](../../asset/map/pen_fence.md),
decisions.md 2026-09-28).

---

## `bBlocksSight` 한 컬럼이 세 동작을 바꾼다

**이게 이 표에서 제일 중요한 규칙이다.**

물통(78cm)·상자(120cm)·보온상자(120cm)는 **실제로는 닭 눈높이에서 시야를 가린다.**
그런데 일부러 안 가리게 해놨다. 이건 물리가 아니라 **의도된 판정 보정**이다.

같은 목록이 세 곳에서 쓰인다:

| 쓰이는 곳 | 왜 |
|---|---|
| 시야 판정 | 낮은 물건 뒤에 숨었다고 안 보이면, 숨을 곳이 너무 많아진다 |
| 소리 파동 | 소리가 지형을 속이면 안 된다 |
| 카메라 충돌 검사 | 3인칭 시점 상황에서만 의미 — 본편은 1인칭 확정이고 관전도 1인칭 초안(systems/character.md)이라, 이 용도의 실사용처는 관전 시점 확정과 함께 재검토 (미결) |

**컬럼을 셋으로 쪼개면 안 된다.** 그러면 목록이 세 벌이 되고, "낮은 물건 하나 추가"가 세 곳을
고치는 일이 된다. 지금은 **행 하나 + 체크박스 하나**로 끝난다.

구현할 때는 전용 트레이스 채널 하나(`CL9_Occlusion`)를 시야·파동·카메라가 공유하고,
`bBlocksSight`가 false인 종류만 그 채널에서 Ignore로 둔다.
구현 상태 (2026-09-17): 배치기(`ACL9SiteMap`)가 false 종류의 조각 컴포넌트를 `ECC_Visibility`와
`CL9_Occlusion`(`ECC_GameTraceChannel2`) 둘 다 Ignore로 둔다 — 파동은 이 채널을 쓰고, 시야 판정은
아직 `ECC_Visibility`를 쓴다(채널 통일은 별건).

**종류별로 이미 나뉘어 있다 (2026-09-28 확인).** `ACL9SiteMap::Rebuild`의 `ComponentFor`는
인스턴스 묶음을 `(종류, 메시)` 쌍마다 하나씩 만들고, 그 종류의 `bBlocksSight`로 채널 응답을
정한다. 그래서 **같은 메시를 쓰는 두 종류가 시야 차단만 다르게** 설 수 있다 — `pen` 행을
`bBlocksSight=false`로 넣는 것만으로 판자 울타리(`wall`, 차단)와 철망 울타리(`pen`, 투과)가
갈린다. C++ 를 고칠 필요가 없었다(실행 `pen-fence-wire` 확인 항목 1).
콜리전은 두 종류 모두 `BlockAll`이라 130cm까지 몸과 아이템을 막는다 — 시야만 열린다.

---

## 점프로 넘을 수 있는 것

`bBlocksSight`가 false인 셋이 그대로 **점프 통과** 대상이다 — 물통·상자·보온상자.

**2026-09-28: `pen`은 여기 들어가지 않는다.** false인 종류가 곧 점프 통과 대상이라는 등식이
이 행에서 깨진다 — 우리 울타리는 "보이지만 막는" 물건이라서 false이지, 낮아서 false인 게
아니다. 다만 점프 정점이 156cm(`JumpVelocity` 780 / `JumpGravity` 1950 —
[character.md](character.md))라 130cm 울타리는 **점프로는 넘긴다.** 전시용 Lvl_Flat 우리는
그래도 되지만, 본편에서 우리 안 재료를 울타리째 넘어 집는 게 허용인지는 미결(아래 미결).

스텝 높이 검증: 점프 통과 셋(물통 78cm·상자 120cm)은 `MaxStepHeight`(언리얼 기본 45cm)보다
높아서 **걸어서는 못 넘고 점프가 필요하다** — 기본값으로 이미 성립하지만, 이식·캡슐 조정 때
스텝 높이가 78cm 아래인지 실측 확인한다. 걸어서 넘어지면 점프의 존재 이유가 사라진다.

---

## 공통 외부 지붕 (2026-10-04)

| Alias | Placement | Height(cm) | Piece | bBlocksSight |
|---|---|---|---|---|
| `roof` | Grid | 20 | `roof` | true |
| `roofFascia` | Tile | 100 | `roofFascia` | true |

두 Piece는 `henhouse` 테마의 정식 불투명 금속 모델을 쓴다. `roof`는 500×500×20cm 닫힌 판,
`roofFascia`는 500×10×100cm 닫힌 패널(X 길이/Y 두께/Z 높이)이다. 두 모델 모두 XY 중심·밑면 Z0.
높이 100은 마감 조각의 기준 높이이며, 실제 빈 띠는 segment의 `HeightOverride`로 전달한다
(0이면 기존 Kind Height). Tile 배치는 길이·높이를 맞추되 10cm 두께와 축 회전·pivot 보정을 유지한다.
기존 실내 `ceiling` 행은 유지한다. 외부 지붕의 활성 조건은 [site.md](site.md)의 `bBuildRoofs`다.

## 높이 210은 천장이 아니다

`cage`·`plug`·`barn`·`flapIn`·`stair`의 210은 **투척 차단 높이**이고 천장 높이와는 별개 값이다.

2026-09-08부터 **천장 높이는 모듈 표가 갖는다** (`UCL9ModuleDef::Height`) — 방마다 다를 수 있고,
야외 모듈은 천장을 아예 안 씌운다. 여기 `ceiling` 행의 `Height`는 그 천장 슬래브 자체의 두께다.

지금 둘 다 210이라 같아 보이지만 **우연이다.** 천장을 내리게 되더라도(예: 132)
이 표의 210을 따라 내리면 **"케이지 넘겨 던지기 없음"이 무너진다.**

`Height`를 천장 값에 묶지 말 것.

마찬가지로 26·40·56도 수레 반경·열 깊이 같은 다른 상수와 숫자가 겹치지만 무관하다.

---

## `flapIn`이 특이하다

개구멍 덮개다. **안에 있으면 벽이고 밖에 있으면 통과**하는 일방향 물건이다.
1F에만 있다. 다른 종류처럼 단순한 벽으로 다루면 안 된다.

---

## 메시를 직접 가리키지 않는다

이 표의 행은 **역할 이름**만 말하고, 그 역할이 어느 메시인지는 조각 표(`UCL9PieceDef`,
`PieceTable`)가 테마별로 갖는다. 그래서 에셋 팩을 갈아 끼울 때 이 표는 손대지 않는다 —
전문은 [../../systems/asset-swap.md](../../systems/asset-swap.md).

조각의 치수도 컬럼이 아니다. `ACL9SiteMap`이 메시 바운드에서 직접 읽는다 — 손으로 옮겨 적은
숫자는 팩이 바뀌는 순간 조용히 틀린 값이 된다.

---

## 미결

- `MinimapColor` 값이 아직 안 정해졌다
- 스텝 높이(`MaxStepHeight`)의 정확한 값 — 점프 통과 셋을 걸어서 못 넘는지 실측 확인 필요
- **`pen` 종류를 생성기가 언제 내보내나** (2026-09-28). 지금은 행만 있고 `BuildSegments`가 이
  종류를 안 낸다 — Lvl_Flat 전시 우리는 손배치다. `pen` 모듈의 둘레를 이 울타리로 바꾸려면
  "어느 변이 울타리인가"를 데이터로 정할 열이 모듈 표에 하나 필요하다 ([module.md](module.md)
  `pen` 행 각주).
- **울타리를 점프로 넘는 것을 허용하나** — 130cm < 점프 정점 156cm. 허용이면 그대로, 아니면
  울타리 높이나 점프 값 중 하나가 움직여야 한다 (밸런스·캐릭터 쪽 결정).

---

## 구현 상태 (2026-09-06, ⑤ 5a 선행)

`UCL9SegKindDef`(Source/CL9/Data) — 고유 컬럼은 `Piece`·`Placement`·`Height`·`bBlocksSight`(`BaseZMode`는 2026-09-08에 지웠다 — 위), `ActorClass`는
공통 베이스(`UCL9DefBase`) 컬럼을 그대로 쓴다 (`CollisionProfile`·`MinimapColor`는 첫 소비처와 함께). 에셋 `SegKindTable`(Content/Data).

> 아래는 2026-09-06~07 격자 미로 시절의 기록이다. **2026-09-11 이후의 종류·배정 정본은
> [../../systems/asset-swap.md](../../systems/asset-swap.md)다** — 바닥·천장·마당은 `Grid`, 디딤판 `tread`는 `Tile`,
> 생성기가 `yard`·`tread`·문 옆 토막 `barnStub`/`wallStub`을 따로 내보낸다 (decisions.md 2026-09-11).

**2026-09-07 (5b): 메시를 직접 가리키던 `Mesh` 컬럼을 역할 이름 `Piece`로 바꿨다.** 실제 메시는 조각
표(`UCL9PieceDef` / `PieceTable`)가 역할×테마로 갖는다 — 전문은
[../../systems/asset-swap.md](../../systems/asset-swap.md). 상속받은 `Mesh` 컬럼은 이제 이 표에서 안 쓰므로
전 행에서 비워 뒀다(아이템 행은 계속 쓴다).

현재 배정: `barn`·`stair` → `wall`(Tile) · `plug` → `wallHalf`(Tile) · `wall` → `fence`(Tile) ·
`flapIn` → `door` · 나머지(`cage`·`floor`·`ceiling`·`truck`·`hay`·`trough`·`crate`·`brooder`) → `box`(Stretch, 회색 상자).
**`cage`가 아직 회색 상자인 건 보유 팩에 케이지 조각이 없어서다** — 사면 조각 표에 `cage` 역할을 만들고
이 표의 `Piece`를 바꾸면 끝이다.

- **12번째 행 `floor`**(높이 30, 시야 차단): 마당·1층·지하·경사로의 지면 슬래브. 위 11종은 벽·소품이라
  바닥이 없어서 추가했다. 슬래브는 자기 층 바닥면 아래로 매달린다.
- **13번째 행 `ceiling`**(높이 30, 시야 차단, 2026-09-06): 1층 축사 천장(바닥 위 630 — `BarnCeiling` 상수, 벽 Height와
  같은 값이지만 별개)·계단 복도 천장·지하 천장(지면 높이). 건물 안이 어두워야 해서 넣었다.
- `flapIn`은 ISM이 아니라 `ACL9FlapIn` 액터다 (일방향 — systems/map.md 구현 상태).
- 생성기가 지금 내는 종류: `wall`·`barn`·`cage`·`plug`·`stair`·`flapIn`·`floor`·`ceiling`. `truck`·`hay`·`trough`·`crate`·
  `brooder` 행은 있지만 배치 규칙이 기획에 없어 아직 안 깐다.
