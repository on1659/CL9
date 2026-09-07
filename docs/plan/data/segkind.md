# 맵 조각 — `UCL9SegKindDef`

벽·상자·케이지 같은 지형 조각의 종류표. **11종**. 맵 배치 툴이 이걸 읽어서 액터를 세운다.

생성기가 좌표를 뱉고, 이 표가 "그 자리에 뭘 세울지"를 정한다.

---

## 컬럼

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `Alias` | `FName` | `wall` · `barn` · `cage` … |
| `Piece` | `FName` | **세울 조각의 역할 이름** — 실제 메시는 조각 표가 갖는다 |
| `Placement` | enum `ECL9SegPlacement` | `Stretch` 늘리기 / `Tile` 자연 크기로 깔기 |
| `ActorClass` | 소프트 참조 | 전용 BP (비우면 공용) |
| `Height` | float | 실제 3D 높이 (cm) |
| `BaseZMode` | enum | `LocalFloor` 바닥 기준 / `WorldTop` 절대 상단 |
| `CollisionProfile` | `FName` | 콜리전 프로파일 |
| `bBlocksSight` | bool | **★ 시야 · 소리 파동 · 카메라를 한꺼번에 결정한다** |
| `MinimapColor` | `FColor` | 미니맵 색 |

---

## 11종

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
| `flapIn` | 210 | 630 | ✔ | 개구멍 덮개 — 일방향 |
| `stair` | 210 | 630 | ✔ | 계단 복도 벽 |
| `brooder` | 40 | 120 | **✘** | 보온 상자 |

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

---

## 점프로 넘을 수 있는 것

`bBlocksSight`가 false인 셋이 그대로 **점프 통과** 대상이다 — 물통·상자·보온상자.

스텝 높이 검증: 점프 통과 셋(물통 78cm·상자 120cm)은 `MaxStepHeight`(언리얼 기본 45cm)보다
높아서 **걸어서는 못 넘고 점프가 필요하다** — 기본값으로 이미 성립하지만, 이식·캡슐 조정 때
스텝 높이가 78cm 아래인지 실측 확인한다. 걸어서 넘어지면 점프의 존재 이유가 사라진다.

---

## 높이 210은 천장이 아니다

`cage`·`plug`·`barn`·`flapIn`·`stair`의 210은 **투척 차단 높이**이고
천장 높이(구 CCL8 상수 `BarnHeightU`)와는 별개 값이다.

지금 둘 다 210이라 같아 보이지만 **우연이다.** 천장을 내리게 되더라도(예: 132)
이 표의 210을 따라 내리면 **"케이지 넘겨 던지기 없음"이 무너진다.**

`Height`를 천장 값에 묶지 말 것.

마찬가지로 26·40·56도 수레 반경·열 깊이 같은 다른 상수와 숫자가 겹치지만 무관하다.

---

## `flapIn`이 특이하다

개구멍 덮개다. **안에 있으면 벽이고 밖에 있으면 통과**하는 일방향 물건이다.
1F에만 있다. 다른 종류처럼 단순한 벽으로 다루면 안 된다.

---

## `stair`의 기준면

`BaseZMode`가 `WorldTop`인 유일한 종류다. 물리 기준면이 상층 기준이라, 램프 상단에서 던진
계란이 못 넘어간다.

다른 종류는 전부 `LocalFloor`(자기가 선 바닥 기준)다.

---

## 메시를 직접 가리키지 않는다

이 표의 행은 **역할 이름**만 말하고, 그 역할이 어느 메시인지는 조각 표(`UCL9PieceDef`,
`DA_PieceTable`)가 테마별로 갖는다. 그래서 에셋 팩을 갈아 끼울 때 이 표는 손대지 않는다 —
전문은 [../../systems/asset-swap.md](../../systems/asset-swap.md).

조각의 치수도 컬럼이 아니다. `ACL9SiteMap`이 메시 바운드에서 직접 읽는다 — 손으로 옮겨 적은
숫자는 팩이 바뀌는 순간 조용히 틀린 값이 된다.

---

## 미결

- `MinimapColor` 값이 아직 안 정해졌다
- 스텝 높이(`MaxStepHeight`)의 정확한 값 — 점프 통과 셋을 걸어서 못 넘는지 실측 확인 필요

---

## 구현 상태 (2026-09-06, ⑤ 5a 선행)

`UCL9SegKindDef`(Source/CL9/Data) — 고유 컬럼은 `Piece`·`Placement`·`Height`·`BaseZMode`·`bBlocksSight`, `ActorClass`는
공통 베이스(`UCL9DefBase`) 컬럼을 그대로 쓴다 (`CollisionProfile`·`MinimapColor`는 첫 소비처와 함께). 에셋 `DA_SegKindTable`(Content/Data).

**2026-09-07 (5b): 메시를 직접 가리키던 `Mesh` 컬럼을 역할 이름 `Piece`로 바꿨다.** 실제 메시는 조각
표(`UCL9PieceDef` / `DA_PieceTable`)가 역할×테마로 갖는다 — 전문은
[../../systems/asset-swap.md](../../systems/asset-swap.md). 상속받은 `Mesh` 컬럼은 이제 이 표에서 안 쓰므로
전 행에서 비워 뒀다(아이템 행은 계속 쓴다).

현재 배정: `barn`·`stair` → `wall`(Tile) · `plug` → `wallHalf`(Tile) · `wall` → `fence`(Tile) ·
`flapIn` → `door` · 나머지(`cage`·`floor`·`ceiling`·`truck`·`hay`·`trough`·`crate`·`brooder`) → `box`(Stretch, 회색 상자).
**`cage`가 아직 회색 상자인 건 보유 팩에 케이지 조각이 없어서다** — 사면 조각 표에 `cage` 역할을 만들고
이 표의 `Piece`를 바꾸면 끝이다.

- **12번째 행 `floor`**(높이 30, 시야 차단): 마당·1층·지하·경사로의 지면 슬래브. 위 11종은 벽·소품이라
  바닥이 없어서 추가했다. 슬래브는 자기 층 바닥면 아래로 매달린다.
- `stair`의 `WorldTop`은 3D에서 "아랫층 바닥에서 시작해 윗층 바닥 + Height까지"로 해석했다 — 경사로 양옆 벽이
  구덩이 바닥부터 1층 위 630까지 한 장으로 선다.
- **13번째 행 `ceiling`**(높이 30, 시야 차단, 2026-09-06): 1층 축사 천장(바닥 위 630 — `BarnCeiling` 상수, 벽 Height와
  같은 값이지만 별개)·계단 복도 천장·지하 천장(지면 높이). 건물 안이 어두워야 해서 넣었다.
- `flapIn`은 ISM이 아니라 `ACL9FlapIn` 액터다 (일방향 — systems/map.md 구현 상태).
- 생성기가 지금 내는 종류: `wall`·`barn`·`cage`·`plug`·`stair`·`flapIn`·`floor`·`ceiling`. `truck`·`hay`·`trough`·`crate`·
  `brooder` 행은 있지만 배치 규칙이 기획에 없어 아직 안 깐다.
