# 참고 — 리썰컴퍼니의 던전 생성

> 2026-09-07, 사용자 요청("이런 느낌의 맵 디자인이야")으로 설치본을 분석한 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다. 우리 결정은
> [../systems/map.md](../systems/map.md) · [data/site.md](data/site.md)에 있다.

## 무엇을 쓰나

`Lethal Company_Data/Managed/DunGen.dll` — 유니티용 절차적 던전 생성기.
격자가 아니라 **타일 그래프**다. 곁들여 `DunGen.Integration.UnityNav.dll`(네비메시),
게임 쪽 `RuntimeDungeon` · `TileConnectionRule`(예: `MineshaftTunnelConnectionRule`) ·
태그(`CaveTileTag`/`RoomTileTag`) · 타일 단위 가시성 컬링(`SetTileVisibility`, `AdjacentTileDepth`).

## 생성 모델

1. **타일** — 손으로 만든 방·복도 프리팹. 크기와 모양이 제각각.
2. **출입구 소켓** — 타일에 붙은 접합점. 소켓 타입이 맞아야 붙는다(`DoorwaySocket`).
3. **DungeonFlow** — 0→1로 흐르는 선 위에 **아키타입** 구간을 늘어놓은 그래프. 구간마다 쓸 `TileSet`이 다르다.
4. **메인 패스** — 입구에서 목표까지 이어 붙인 한 줄기(`MainPathTiles`).
5. **가지** — 메인 패스에서 뻗는 곁가지(`BranchCount`·`BranchingDepth`·`BranchCapTileSets`). **끝은 막다른 길.**
6. **고리** — `DoorwayConnectionChance`로 맞닿은 타일의 남는 출입구를 확률로 이어 순환로를 만든다.
7. **배치 판정** — 붙인 뒤 충돌 검사(`OverlapThreshold`·`TilePlacementResult`), `TileRepeatMode`·`AllowImmediateRepeats`로 반복 억제.
8. **자물쇠와 열쇠** — `KeyManager`·`LockDoorway`가 메인 패스를 막고 열쇠를 가지에 둔다.
9. **주입** — `TileInjectionRule`로 반드시 나와야 하는 타일을 흐름의 특정 지점에 끼운다.

## 실측한 수치

`sharedassets3.assets`에서 DungeonFlow 5종의 직렬화 데이터를 직접 읽었다.

| Flow | 메인 패스 (타일) | 가지 수 |
|---|---|---|
| `Level1Flow` (공장) | 11~13 | 22~25 |
| `Level1Flow3Exits` | 7~9 | 22~25 |
| `Level1FlowExtraLarge` | 17~21 | 22~25 |
| `Level2Flow` (저택) | 8~11 | 22~25 |
| `Level3Flow` (광산) | 18~22 | 22~25 |

길이 값은 `ExtraLarge` > 기본 > `3Exits` 순서로 나와 교차 검증됐다. 가지 수는 5종이 전부 같아
확신도가 낮다(기본값을 안 건드린 것으로 보인다).

**★ 메인 패스가 8~22타일인데 가지가 22~25개다** — 타일 하나당 가지 2개꼴. 플레이어가 지나는
대부분이 본줄기가 아니라 **막다른 곁가지**다. 헤매는 느낌의 정체가 이것이다.

## 손으로 만든 양

- `Tile`로 끝나는 프리팹 60여 종 + `LargeRoom` · `PoolRoom` · `SmallLRoom` · `LargeFanRoom` ·
  `ComplexFactoryRoom` · `LargeStairRoom` · `SmallPillarRoom` … → **테마 3종에 걸쳐 60~80종**
- 출입구는 **방위 소켓**: `DoorwayPointN/S/E/W` + `N1/S1/E1/W1` + `NLeft/NRight/SLeft/SRight`
  → **네 방향 전부로 붙는다**
- 갈림길 전용 타일이 따로 있다: `CaveYTile` · `CloverTile` · `CaveForkStairTile A~E` · `CaveCrampedIntersectTile`
- **안 쓴 출입구는 막는 메시로 봉한다**: `CaveIntersectTileBlockerN/S/E` · `TunnelDoorwayBlocker` · `HallwayBlockerNarrowHallway`
- 수직 이동이 곳곳에 있다: `CaveForkStairTile` · `LargeStairRoom` · `CatwalkStairTile` · `DeepShaftTile`

## CL9에 무엇을 가져오나

**정정(2026-09-07)**: 처음엔 "CL9는 복도가 한 방향뿐이라 생성기를 다시 짜야 한다"고 봤는데
**틀렸다.** `FCL9FloorLayout`에 `OpenH`·`OpenV`가 둘 다 있고 Kruskal이 이미 2차원 격자 미로를
만들고 있다. 위상은 멀쩡했고, 문제는 **칸 비율**(1000 × 250 = 4:1)이라 정상적인 미로가
"줄무늬"로 그려진 것이었다.

| 배울 것 | CL9에서 하는 법 | 비용 |
|---|---|---|
| 칸이 거의 정사각이어야 미로로 읽힌다 | `ColWidth`를 통로 피치(250+120=370)에 가깝게 — 조각 모듈상 **500** | 표만, 빌드 없음 |
| 지나는 대부분이 막다른 곁가지 | 개구부 목표를 신장 트리에 가깝게(고리를 줄인다) | 표만 |
| 공간 크기가 다양해야 한다 | 칸 몇 개를 2×2·1×3으로 합쳐 **방**을 만든다 | `BuildSegments` 수정 |
| 네 방향 접합 · 손으로 만든 타일 | 타일 그래프 생성기 + 방 프리팹 60~80종 | 가장 큼. 언리얼엔 DunGen 대체품이 없다 |
| **수직 이동** | CL9는 1층↔지하를 경사로 **하나**로만 잇는다 | 별개 축, 미결 |

조각 표(`UCL9PieceDef`, 역할 × 테마 — [../systems/asset-swap.md](../systems/asset-swap.md))는
타일 그래프로 가더라도 그대로 타일셋 구조로 쓰인다. 헛일이 되지 않는다.
