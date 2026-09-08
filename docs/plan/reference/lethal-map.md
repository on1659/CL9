# 참고 — Lethal Company의 맵 생성과 달

> 2026-09-07 시작, 2026-09-08 디컴파일과 에셋 덤프로 전면 보강.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](README.md).
> CL9의 결정은 [../data/module.md](../data/module.md) · [../data/site.md](../data/site.md) ·
> [../../systems/map.md](../../systems/map.md)에 있다.

**CL9은 이쪽이 아니라 R.E.P.O. 방식을 주 모델로 골랐다**([repo-map.md](repo-map.md)).
자유 타일 그래프는 우리가 이미 가진 격자와 맞지 않아 이식이 어렵기 때문이다. 이 문서는
"헤매는 느낌"과 "달마다 다른 값"이 어떻게 만들어지는지를 보려고 남긴다.

리썰 컴퍼니의 레벨은 세 층으로 갈라져 있다. 달(moon)마다 하나씩 있는 `SelectableLevel`
스크립터블 오브젝트가 "이 행성은 어떤 곳인가"를 데이터로 들고 있고, 실외 지형은 달마다 손으로
만든 고정 씬이며, 실내 시설만 DunGen이 매 원정마다 새로 조립한다. 호스트는 지오메트리를
동기화하지 않고 정수 시드 하나(`randomMapSeed`)와 달 번호만 뿌리며, 각 클라이언트가 자기
머신에서 같은 시드로 같은 던전을 만들어 낸다. 네트워크로 실제 스폰되는 것은 스크랩·함정·동기
프롭처럼 "서버가 소유권을 가져야 하는 것"뿐이다. 아래는 디컴파일된 C# 소스를 읽고 정리한
구조와 수치이며, 코드에서 직접 확인한 것과 추정을 구분해 적었다.

---

## 달(SelectableLevel) 데이터

`SelectableLevel.cs`는 `[CreateAssetMenu(menuName = "ScriptableObjects/Level")]`가 붙은
스크립터블 오브젝트다. 필드 전부를 목적별로 묶으면 다음과 같다. "코드 기본값"은 C# 필드
초기값이며, 실제 각 달의 에셋에서는 대부분 덮어써져 있다.

### 신원과 씬

| 필드 | 형 | 코드 기본값 | 뜻 |
|---|---|---|---|
| `planetPrefab` | GameObject | — | 우주선 창밖에 보이는 행성 모형 |
| `sceneName` | string | — | 실외 지형이 들어 있는 씬 이름 |
| `levelID` | int | — | 배열 인덱스이자 네트워크로 오가는 식별자 |
| `lockedForDemo` | bool | false | 데모판 잠금 |
| `spawnEnemiesAndScrap` | bool | true | **false면 던전 생성·스크랩·적을 통째로 건너뛴다** |
| `PlanetName` | string | — | 표시 이름 |
| `LevelDescription` | string | — | 터미널 설명문 |
| `videoReel` | VideoClip | — | 터미널 영상 |
| `riskLevel` | string | — | "S++", "S+", "A" 같은 **문자열**. 곰팡이 확률 분기에만 쓰이는 것을 확인했다 |
| `levelIconString` | string | — | 지도 아이콘 |
| `levelIncludesSnowFootprints` | bool | false | 눈 발자국 풀 생성 여부 |

### 시간

`timeToArrive`(코드 기본값 8초)는 우주선이 그 달까지 날아가는 연출 시간이다.
`OffsetFromGlobalTime`은 전역 시계에 더해지는 그 행성만의 시차이고, `DaySpeedMultiplier`
(기본값 1)는 하루가 흐르는 속도의 배율이다. `planetHasTime`(기본값 true)이 false면 자동
이륙 타이머와 "곧 떠난다" 경고가 돌지 않는다 — 회사 건물(levelID 3)이 이 경우로 보인다(추정).

### 날씨

`randomWeathers`는 `RandomWeatherWithVariables[]`이고, 각 원소는 `weatherType`
(`LevelWeatherType`), `weatherVariable`, `weatherVariable2`(둘 다 int), `weatherVariableColor`
(기본 검정)를 갖는다. 두 변수의 의미는 날씨마다 다르다 — 폭풍이면 번개 빈도, 안개면
`meanFreePath`의 하한과 상한, 홍수면 시작 수위와 상승량, 일식이면 최소 적 마릿수다.
`overrideWeather`/`overrideWeatherType`은 무작위 추첨을 무시하고 고정하며, `currentWeather`는
그날 확정된 값이 들어오는 런타임 슬롯이다.

`LevelWeatherType`은 `None = -1, DustClouds, Rainy, Stormy, Foggy, Flooded, Eclipsed`
일곱 값이다.

### 레벨 값(던전 크기와 배치)

- `factorySizeMultiplier` — 던전 크기의 핵심 입력. 아래 "던전 생성 절차"의 공식 참조.
- `dungeonFlowTypes` — `IntWithRarity[]`. `id`는 `RoundManager.dungeonFlowTypes`
  (`IndoorMapType[]`) 배열의 인덱스이고, `rarity`는 `[Range(0, 300)]` 가중치다.
  `overrideLevelAmbience`로 그 플로우가 걸렸을 때만 쓰는 앰비언스 세트를 지정할 수 있다.
- `spawnableMapObjects` — `SpawnableMapObject[]`. **정정: `SpawnMapObjects()`는 이 배열을
  읽지 않는다.** 실제 실내 함정은 아래 `indoorMapHazards`에서 나온다. `spawnableMapObjects`는
  구버전 잔재로 보인다(추정).
- `indoorMapHazards` — `IndoorMapHazard[]`. 각 원소는 `hazardType`
  (`IndoorMapHazardType` 스크립터블 오브젝트)과 `numberToSpawn`(AnimationCurve) 둘뿐이다.
- `spawnableOutsideObjects` — `SpawnableOutsideObjectWithRarity[]`. 여기서도 rarity가 아니라
  `randomAmount`라는 AnimationCurve가 개수를 정한다.
- `canSpawnMold`(기본 true), `moldSpreadIterations`, `moldStartPosition`, `moldType` —
  곰팡이/잡초 상태. 세이브에 저장되어 원정 사이에 누적된다.

### 스크랩

`spawnableScrap`은 `List<SpawnableItemWithRarity>`이고 각 원소는 `spawnableItem`(Item)과
`rarity`(`[Range(0, 100)]`)다. 개수 범위는 `minScrap`/`maxScrap`, 가치 범위는
`minTotalScrapValue`/`maxTotalScrapValue`.

> **정정할 만한 발견**: `minTotalScrapValue`와 `maxTotalScrapValue`는 `SpawnScrapInLevel()`
> 안에서 **디버그 로그에만 등장하고 배치 로직에 전혀 관여하지 않는다.** 총 가치를 목표치에
> 맞추는 루프가 이 버전에는 없다. 총액은 개수 × 아이템별 가치 추첨의 결과로 나올 뿐이다.

### 적 예산

| 필드 | 코드 기본값 | 뜻 |
|---|---|---|
| `maxEnemyPowerCount` | 8 | 실내 적 파워 예산 |
| `maxOutsideEnemyPowerCount` | 15 | 실외(야간) 적 파워 예산 |
| `maxDaytimeEnemyPowerCount` | 20 | 주간 적 파워 예산 |
| `maxInsideDiversityPowerCount` | 100 | 실내 종 다양성 상한 |
| `maxOutsideDiversityPowerCount` | 100 | 실외 종 다양성 상한 |
| `spawnProbabilityRange` | 3f | 실내 스폰 수 추첨의 폭 |
| `daytimeEnemiesProbabilityRange` | 10f | 주간 적 추첨의 폭 |

`Enemies`, `OutsideEnemies`, `DaytimeEnemies`는 각각 `List<SpawnableEnemyWithRarity>`이고,
`specialEnemyRarity`(`OverrideEnemyRarity`)로 특정 조건에서 희귀도를 덮어쓴다.
`enemySpawnChanceThroughoutDay`, `outsideEnemySpawnChanceThroughDay`,
`daytimeEnemySpawnChanceThroughDay` 세 AnimationCurve는 X축이 `normalizedTimeOfDay`,
Y축이 그 시각의 기준 스폰 수다. 곡선 값에 데드라인 압박이 더해지는 것을 확인했다:

```
num6 = 곡선값 + Mathf.Abs(daysUntilDeadline - 3) / 1.6f
스폰 수 = Mathf.Clamp(random.Next((int)(num6 - 3f), (int)(곡선값 + 3f)), minOutsideEnemiesToSpawn, 20)
```

마지막으로 `levelAmbienceClips`(`LevelAmbienceLibrary`)가 실내/실외/우주선 앰비언스와
광기(insanity) 변형 클립을 묶어 들고 있다.

### 달 목록·경로 가격·회사 건물

달 목록은 `Terminal.moonsCatalogueList`(`SelectableLevel[]`)에 있고, 터미널의 `moons` 명령이
이 배열을 훑어 이름과 괄호 안 현재 날씨를 출력한다. **경로 가격은 달 데이터가 아니라
터미널 노드에 있다** — `TerminalNode.buyRerouteToMoon`이 목적지 `levelID`이고
`TerminalNode.itemCost`가 그 경로의 크레딧 비용이다. 확정 시
`StartOfRound.ChangeLevelServerRpc(node.buyRerouteToMoon, groupCredits)`가 호출된다.
`buyRerouteToMoon == -2`는 특수 값으로 별도 처리된다.

회사 건물은 `levelID == 3`으로 하드코딩된 분기가 셋 있다. 도착 시 회사 방문 음악을 트는 것,
워키토키가 그곳에서 다르게 동작하는 것, 시체를 데스크에 올려놨을 때의 판정이다. 던전이 없는
것은 별도 플래그가 아니라 `spawnEnemiesAndScrap == false`로 처리된다(추정 — 그 달의 에셋을
직접 보지 못했으나, 인버스 텔레포터가 `spawnEnemiesAndScrap`이 false면 사용 불가로 막히는
것이 회사 건물의 알려진 동작과 일치한다).

---

## DunGen이라는 기성품

리썰컴퍼니는 생성기를 직접 쓰지 않는다. `Managed/DunGen.dll`은 유니티용 상용 절차적 던전
생성기이고, 격자가 아니라 **타일 그래프**다. 곁들여 `DunGen.Integration.UnityNav.dll`(네비메시)와
게임 쪽 `RuntimeDungeon`·`TileConnectionRule`(예: `MineshaftTunnelConnectionRule`)·
태그(`CaveTileTag`/`RoomTileTag`)·타일 단위 가시성 컬링(`SetTileVisibility`, `AdjacentTileDepth`)이 붙는다.

DunGen이 맵을 만드는 방식은 이렇다.

1. **타일** — 손으로 만든 방·복도 프리팹. 크기와 모양이 제각각이다.
2. **출입구 소켓** — 타일에 붙은 접합점. 소켓 타입이 맞아야 붙는다(`DoorwaySocket`).
3. **DungeonFlow** — 0에서 1로 흐르는 선 위에 **아키타입** 구간을 늘어놓은 그래프다. 구간마다
   쓸 `TileSet`이 다르다.
4. **메인 패스** — 입구에서 목표까지 이어 붙인 한 줄기(`MainPathTiles`).
5. **가지** — 메인 패스에서 뻗는 곁가지(`BranchCount`·`BranchingDepth`·`BranchCapTileSets`).
   **끝은 막다른 길이다.**
6. **고리** — `DoorwayConnectionChance`로 맞닿은 타일의 남는 출입구를 확률로 이어 순환로를 만든다.
7. **배치 판정** — 붙인 뒤 충돌 검사(`OverlapThreshold`·`TilePlacementResult`)를 하고,
   `TileRepeatMode`·`AllowImmediateRepeats`로 반복을 억제한다.
8. **자물쇠와 열쇠** — `KeyManager`·`LockDoorway`가 메인 패스를 막고 열쇠를 가지에 둔다.
9. **주입** — `TileInjectionRule`로 반드시 나와야 하는 타일을 흐름의 특정 지점에 끼운다.

### 실측 — DungeonFlow 5종

`sharedassets3.assets`에서 DungeonFlow 5종의 직렬화 데이터를 직접 읽었다.

| Flow | 메인 패스 (타일) | 가지 수 |
|---|---|---|
| `Level1Flow` (공장) | 11~13 | 22~25 |
| `Level1Flow3Exits` | 7~9 | 22~25 |
| `Level1FlowExtraLarge` | 17~21 | 22~25 |
| `Level2Flow` (저택) | 8~11 | 22~25 |
| `Level3Flow` (광산) | 18~22 | 22~25 |

길이 값이 `ExtraLarge` > 기본 > `3Exits` 순서로 나와 교차 검증됐다. 가지 수는 5종이 전부 같아
확신도가 낮다 — 기본값을 안 건드린 것으로 보인다.

**★ 메인 패스가 8~22타일인데 가지가 22~25개다.** 타일 하나당 가지 2개꼴이라, 플레이어가 지나는
대부분이 본줄기가 아니라 **막다른 곁가지**다. 헤매는 느낌의 정체가 이것이다.

### 손으로 만든 양

- `Tile`로 끝나는 프리팹 60여 종에 `LargeRoom`·`PoolRoom`·`SmallLRoom`·`LargeFanRoom`·
  `ComplexFactoryRoom`·`LargeStairRoom`·`SmallPillarRoom` 같은 방들이 더해져 **테마 3종에 걸쳐 60~80종**이다.
- 출입구는 **방위 소켓**이다: `DoorwayPointN/S/E/W` + `N1/S1/E1/W1` + `NLeft/NRight/SLeft/SRight`.
  네 방향 전부로 붙는다.
- 갈림길 전용 타일이 따로 있다: `CaveYTile`·`CloverTile`·`CaveForkStairTile A~E`·`CaveCrampedIntersectTile`.
- **안 쓴 출입구는 막는 메시로 봉한다**: `CaveIntersectTileBlockerN/S/E`·`TunnelDoorwayBlocker`·
  `HallwayBlockerNarrowHallway`.
- 수직 이동이 곳곳에 있다: `CaveForkStairTile`·`LargeStairRoom`·`CatwalkStairTile`·`DeepShaftTile`.
  **층 개념은 없고 계단이 타일 안에 들어 있다.**

## 실측 — 달 14곳의 실제 값

코드 기본값이 아니라 설치본의 `SelectableLevel` 에셋에서 직접 읽은 값이다. 첫 행은 이름이 없는
테스트용 프리셋(`TestAllEnemies`)이다. `7 Dine`의 스크랩 개수 200~250은 다른 달과 자릿수가 달라
이 설치본 버전의 값 그대로임을 밝혀 둔다.


| 달 | 위험도 | 스크랩 개수 | 총액 | 던전 크기배율 | 실내예산 | 실외예산 | 주간예산 | 하루속도 | 도착시간 |
|---|---|---|---|---|---|---|---|---|---|
|  | 1 | 7~14 | 80~450 | 0.0 | 8 | 15 | 20 | 1.0 | 5.0 |
| 41 Experimentation | D | 8~12 | 80~450 | 1.0 | 4 | 8 | 5 | 1.0 | 1.0 |
| 220 Assurance | C | 13~16 | 100~250 | 1.0 | 6 | 8 | 7 | 1.0 | 1.0 |
| 56 Vow | C | 12~15 | 300~700 | 1.149999976158142 | 7 | 6 | 17 | 1.0 | 2.5 |
| 71 Gordion | Safe | 5~11 | 100~250 | 0.3499999940395355 | 0 | 0 | 20 | 1.0 | 3.0 |
| 61 March | B | 13~17 | 300~700 | 1.75 | 14 | 12 | 14 | 1.0 | 2.75 |
| 20 Adamance | B | 14~17 | 300~700 | 1.1799999475479126 | 13 | 11 | 20 | 1.0 | 4.0 |
| 85 Rend | A | 18~26 | 300~700 | 1.7999999523162842 | 10 | 6 | 20 | 1.0 | 2.700000047683716 |
| 7 Dine | S | 200~250 | 250~700 | 1.7999999523162842 | 10 | 9 | 20 | 1.0 | 2.700000047683716 |
| 21 Offense | B | 14~19 | 100~250 | 1.25 | 12 | 8 | 20 | 1.0 | 2.0 |
| 8 Titan | S+ | 28~32 | 300~700 | 2.200000047683716 | 18 | 7 | 20 | 1.0 | 1.5 |
| 68 Artifice | S++ | 26~31 | 300~700 | 1.7999999523162842 | 13 | 13 | 15 | 1.0 | 4.0 |
| 44 Liquidation | S++ | 28~45 | 300~700 | 1.600000023841858 | 13 | 13 | 20 | 1.0 | 4.0 |
| 5 Embrion | S | 14~17 | 100~250 | 1.100000023841858 | 8 | 70 | 20 | 1.0 | 2.0 |

`71 Gordion`이 회사 건물이고 실내·실외 예산이 0이라 몬스터가 없다. 던전 크기 배율은
`41 Experimentation` 1.0에서 `8 Titan` 2.2까지 벌어지고, 실내 몬스터 예산은 4에서 18까지 오른다.
**위험도 등급(D~S++)은 표시용 문자열이고 판정에 쓰이지 않는다.**

## 던전 생성 절차

번호는 실행 순서다. 시작점은 서버의 `RoundManager.LoadNewLevel(randomSeed, newLevel)`.

1. **서버가 시드를 정한다.** `StartOfRound.randomMapSeed`는 보통
   `UnityEngine.Random.Range(1, 100000000)`이고, 세이브에서 이어할 때는 저장값을 읽으며,
   챌린지 파일이면 주차 번호에서 결정론적으로 만든다. 이 시점에
   `SetPlanetsWeather()`가 **모든 달의 날씨를 한꺼번에** 정해둔다.

2. **`LoadNewLevelWait` 코루틴**이 곰팡이 시작 노드를 먼저 확정한다. `moldSpreadIterations`가
   0보다 크고 `moldStartPosition`이 -1이면 시드 `+2017`로 `outsideAINodes` 중 하나를 고르되,
   우주선 착륙 지점에서 40m 이상 떨어지고 바닥 태그가 Grass/Snow/Gravel(또는 활성 Terrain)인
   곳을 찾을 때까지 배열을 한 칸씩 돌린다. 매 후보마다 30% 확률로, 또는 15회 성공 후 멈춘다.

3. **`GenerateNewLevelClientRpc(randomSeed, levelID, moldIterations, moldStartPosition,
   syncDestroyedMold[])`** — 여기서부터는 **모든 클라이언트가 각자** 실행한다. 받은 시드를
   `playersManager.randomMapSeed`에 넣고 `InitializeRandomNumberGenerators()`로
   난수기 세트를 만든다(아래 "수치"의 시드 오프셋 표). `spawnEnemiesAndScrap`이 false면
   여기서 끝난다.

4. **DunGen 준비.** 씬에서 `RuntimeDungeon`을 찾아
   `GenerateAsynchronously = true`, `MaxAsyncFrameMilliseconds = 1f`,
   `PauseBetweenRooms = 0f`로 두고 `GenerateNewFloor()`를 부른다.

5. **`GenerateNewFloor()` — DungeonFlow 추첨.** `currentLevel.dungeonFlowTypes`의 `rarity`
   배열로 `GetRandomWeightedIndex(weights, LevelRandom)`을 돌려 하나 고른다. 이 함수는
   가중치 합으로 나눈 누적 확률과 `randomSeed.NextDouble()`을 비교하는 평범한 룰렛휠이며,
   합이 0 이하면 균등 추첨으로 떨어진다. 고른 `id`가 `currentDungeonType`이 되고
   `RoundManager.dungeonFlowTypes[id].dungeonFlow`가 제너레이터에 꽂힌다. 앰비언스도
   여기서 `overrideLevelAmbience` → `levelAmbienceClips` 순으로 정해진다.

6. **크기 결정.** 시드를 고정(`ShouldRandomizeSeed = false`, `Seed = LevelRandom.Next()`)한 뒤:

   ```
   LengthMultiplier = round((factorySizeMultiplier / IndoorMapType.MapTileSize * mapSizeMultiplier) * 100) / 100
   ```

   즉 달의 크기 배율을 그 던전 타입의 타일 크기로 나눠 정규화하고, 전역
   `RoundManager.mapSizeMultiplier`(코드 기본값 1)를 곱한 뒤 소수 둘째 자리로 반올림한다.
   플로우가 지정되지 않은 예외 경로에서는 `factorySizeMultiplier * mapSizeMultiplier`만 쓴다.
   `IndoorMapType.restrictBounds`가 0이 아니면 `RestrictDungeonToBounds = true`로 두고
   `TilePlacementBounds = restrictBounds * 2 * factorySizeMultiplier`를 준다 — 던전이
   퍼질 수 있는 물리적 상자다. 컬링 깊이도 여기서
   `occlusionCuller.AdjacentTileDepth = IndoorMapType.cullingTileDepth`(코드 기본값 6)로 설정된다.

7. **DunGen이 비동기로 조립.** 프레임당 1ms만 쓰므로 여러 프레임에 걸친다. 완료 이벤트
   (`Generator_OnGenerationStatusChanged`)에서 `FinishGeneratingLevel()`로 넘어간다.
   한 프레임에 끝나는 경우도 따로 처리한다.

8. **`FinishGeneratingLevel()`** — 클라이언트마다 실행. `AINode`/`CaveNode` 태그 오브젝트를
   수집하고, `BakeDunGenNavMesh()`로 던전 게임오브젝트에 에이전트 타입 수만큼 `NavMeshSurface`를
   만들어 굽는다(하위의 다른 서피스는 비활성화). 이어서 `SpawnCaveDoorLights()`,
   `SetToCurrentLevelWeather()`, `SpawnOutsideHazards()`를 부르고, `AdjacentRoomCullingModified`
   전부에 방금 만든 `Dungeon`을 물린 뒤 `FinishedGeneratingLevelServerRpc(localClientId)`로
   서버에 완료를 보고한다.

9. **서버가 전원을 기다린다.** `LoadNewLevelWait`는
   `playersFinishedGeneratingFloor.Count >= connectedPlayers`가 될 때까지 대기한 뒤
   0.3초를 더 쉬고 `SpawnSyncedProps()`, 이어서 `GeneratedFloorPostProcessing()`
   (= `SpawnScrapInLevel()` + `SpawnMapObjects()`)을 **서버에서만** 실행한다.

10. **`FinishGeneratingNewLevelClientRpc()`** — 로딩 화면을 내리고, 조명 목록을 갱신하고,
    우주선 문 열리는 시퀀스를 시작하고, `SetLevelObjectVariables()`로 문·코드·출구 ID를
    확정하고, `RefreshEnemiesList()`와 `PredictAllOutsideEnemies()`로 그날의 적 스폰 계획을
    미리 계산한다.

### 문·코드·전원의 사후 설정

`SetLevelObjectVariables()`는 `waitForMainEntranceTeleportToSpawn` 코루틴을 돌려 메인 입구
텔레포트가 나타날 때까지 최대 15초 기다린 뒤, 그 위치를 기준으로 다음을 순서대로 한다.

- **`SetLockedDoors`** — y가 -160보다 큰 문과 `canBeLocked == false`인 문을 후보에서 뺀다
  (즉 던전은 월드에서 y = -160 아래에 생성된다). 남은 문을 **메인 입구에서 먼 순**으로
  정렬하고 확률을 1.1에서 시작해 문 하나 처리할 때마다 1.55로 나눠 간다. 결과적으로 가장 먼
  문 몇 개는 확정 잠금이고 뒤로 갈수록 급감한다. 잠금 해제 시간은
  `Clamp(LevelRandom.Next(2, 90), 2, 32)`초. 마지막에 **잠근 문 수 + 2**개의 열쇠를 실내
  AI 노드 근처(반경 8)에 서버가 네트워크 스폰한다. 광산(`currentDungeonType == 4`)에서는
  노드를 입구 가까운 순으로 정렬해 앞쪽 1/3에서만 고른다.
- **`SetSteamValveTimes`** — 증기 밸브의 75%에 파열 시각을 `Clamp(NextDouble(), 0.2, 1.0)`,
  균열 시각을 그 값 × 난수, 안개 크기를 `Clamp(NextDouble(), 0.6, 0.98)`로 준다.
- **`SetBigDoorCodes`** — `TerminalAccessibleObject`를 메인 입구에서 **가까운 순**으로 정렬해
  코드를 배정하고, 큰 문 중 처음 3개는 열어두며 그 뒤로는 22% 확률로 열어둔다.
- **`SetExitIDs`** — 실외 쪽 비상구(`entranceId == 1 && !isEntranceToBuilding`)에 메인 입구에서
  가까운 순으로 1, 2, 3…을 부여한다.
- **`SetPowerOffAtStart`** — 6초 뒤, 시드 `+3`의 난수가 0.08 미만이면 차단기를 내린 상태로
  시작한다. **즉 8% 확률로 처음부터 정전.**

### 광산(mineshaft)의 특수 처리

`currentDungeonType == 4`가 광산이다. `MineshaftTunnelConnectionRule`이 DunGen의
`DoorwayPairFinder.CustomConnectionRules`에 우선순위 10000짜리 규칙 두 개를 상태에 따라
붙였다 뗀다.

- `GenerationStatus.Branching` 단계: `DisallowMixingCavesWithTunnels`. cave 태그 타일과
  tunnel 태그 타일이 이어지려 할 때, cave 타일의 이미 연결된 문틀을 **2단계 이웃까지** 훑어
  tunnel 태그가 하나라도 있으면 `Deny`. 동굴 구역과 터널 구역이 뒤섞이지 않게 하는 규칙이다.
- `GenerationStatus.InstantiatingTiles` 단계: `AllowCaveTunnelConnections`. 두 문틀의
  `Socket`이 다르면 `Deny`, 같으면 `Allow`. **"방위 소켓" 개념이 코드로 확인된다.**

`SpawnPropOnDoorwayPair`는 cave 타일과 비-cave 타일이 만나는 문틀에 동굴 입구 프롭을 붙이고,
`RoundManager.SpawnCaveDoorLights()`는 광산에서 tunnel 타일이 cave 문틀과 만나는 지점에
같은 프롭을 붙이면서 그 타일의 `PoweredLight` 태그 조명을 전부 지운다.

`MineshaftElevatorController`는 별도의 `NetworkBehaviour`로, 위/아래/내부 포인트와 호출
쿨다운, 문 개폐, 이동 사운드, 그리고 네트워크 동기 엘리베이터 음악(`SetElevatorMusicServerRpc`)을
갖는다. 스크랩 개수도 광산에서는 **+6** 보정을 받는다.

### 컬링

`AdjacentRoomCullingModified`가 현재 플레이어가 서 있는 `Tile`에서 시작해 문으로 이어진
타일을 `AdjacentTileDepth`(= `IndoorMapType.cullingTileDepth`, 코드 기본값 6)만큼 BFS로
넓히며 그 집합만 렌더러와 라이트를 켠다. `CullBehindClosedDoors`가 켜져 있으면
`Door.ShouldCullBehind`인 문 너머로는 확장을 멈춘다. 시작 타일은
`Placement.NormalizedPathDepth == 0`인 타일이며, 관전 중 다른 플레이어가 메인 입구로
들어가면 `SetToStartTile()`로 강제 리셋한다.

---

## 스크랩 배치

`SpawnScrapInLevel()`은 **서버에서만** 실행되며 `AnomalyRandom`(시드 `+5`) 하나로 전부 굴린다.

1. **개수.** `(int)(AnomalyRandom.Next(minScrap, maxScrap) * scrapAmountMultiplier)`.
   광산이면 `+6`, 챌린지 파일이면 `+Next(10, 30)`.
   `scrapAmountMultiplier`는 `RoundManager`의 필드로 코드 기본값 1.

2. **"한 종류 도배" 변칙.** `AnomalyRandom.Next(0, 500) <= 20`(약 4.2%)이면 스크랩 목록에서
   한 종류를 골라 그것만 채운다. 단 `rarity < 5`이거나 양손 아이템이면 최대 2번까지 다시
   뽑고, 그래도 조건을 못 맞추면 60% 확률로 변칙 자체를 취소한다.

3. **종류 추첨.** 도배가 아니면 각 슬롯마다 `rarity` 가중 룰렛으로 아이템을 고른다.
   챌린지의 `increasedScrapSpawnRateIndex`에 걸린 종류는 가중치가 100으로 덮어써진다.

4. **지점 선택.** 씬(=타일 프리팹) 안에 흩어진 `RandomScrapSpawn` 컴포넌트가 전부 후보다.
   `RandomScrapSpawn`은 `spawnableItems`(`ItemGroup`), `itemSpawnRange`,
   `spawnedItemsCopyPosition`, `spawnUsed`, `spawnWithParent`만 가진 얇은 마커다.
   아이템의 `spawnPositionTypes`에 그 지점의 `ItemGroup`이 포함되고 `spawnUsed == false`인
   것만 남긴다. 이미 쓴 지점(`usedSpawns`)은 목록에서 빼되, **후보가 바닥나면 `usedSpawns`를
   비우고 같은 아이템을 다시 시도한다** — 지점 재사용은 "한 바퀴를 다 돈 뒤"에만 허용된다는
   뜻이고, 결과적으로 스크랩이 맵 전체에 고르게 퍼진다.

5. **위치.** `spawnedItemsCopyPosition`이면 그 지점(또는 `spawnWithParent`)의 좌표를 그대로
   쓰고 `spawnUsed = true`로 소진시킨다. 아니면
   `GetRandomNavMeshPositionInBoxPredictable(지점, itemSpawnRange, ..., AnomalyRandom, -8193)`
   에 아이템의 `verticalOffset`을 더한다. 이 함수는 축마다
   `(NextDouble() - 0.5) * radius`를 더한 뒤 y를 원래 높이로 되돌리고
   `NavMesh.SamplePosition`으로 스냅한다 — **구가 아니라 축정렬 상자**이며 시드 난수기를
   받으므로 재현 가능하다.

6. **가치.** `(int)(AnomalyRandom.Next(item.minValue, item.maxValue) * scrapValueMultiplier)`.
   도배 모드에서는 여기에 `Clamp(..., 50, 170)`이 걸린다. `scrapValueMultiplier`도 코드
   기본값 1이다.

7. **도배 모드 사후 보정.** 총합이 4500을 넘으면 전부 ×0.7, 하한(일반 600, 양손 아이템
   1500) 미만이면 전부 ×1.4. **일반 모드에는 총액 보정이 없다.**

8. **네트워크.** 서버가 프리팹을 `Instantiate` 후 `NetworkObject.Spawn()` 한다. 그런 뒤
   **11초를 기다렸다가** `SyncScrapValuesClientRpc(spawnedScrap[], allScrapValue[])`로 값을
   보낸다. 클라이언트는 `SetScrapValue`로 값을 적용하고, `ScrapValuesRandom`(시드 `+210`)으로
   `meshVariants`/`materialVariants` 중 하나를 골라 외형을 결정한다 — **외형 변형은 시드로
   각자 계산하고 값만 전송한다.** 마지막에 `totalScrapValueInLevel`을 갱신한다.

위험도(`riskLevel`)나 날씨가 스크랩 양·가치에 직접 곱해지는 코드는 없다. 차이는 각 달의
`minScrap`/`maxScrap`/`spawnableScrap` 데이터와 `factorySizeMultiplier`(= 던전이 커지면
`RandomScrapSpawn` 지점도 늘어난다)에서 나온다.

---

## 함정·실외 오브젝트 배치

### 실내 함정 (`SpawnMapObjects`, 서버 전용, 시드 `+587`)

`currentLevel.indoorMapHazards`를 순회한다. 광산에서 `allowInMineshaft == false`인 종류는
건너뛴다. 개수는 곡선 한 번의 평가로 끝난다:

```
int num = (int)indoorMapHazards[i].numberToSpawn.Evaluate((float)random.NextDouble());
```

X축 0~1을 균등 난수로 찍고 Y축 값을 개수로 읽는다. 즉 **곡선의 모양이 곧 개수 분포**이며,
평평하면 고정 개수, 가파르면 변동이 크다. 챌린지에서 지목된 종류는 `Min(num * 2, 150)`.

스포너는 씬의 `RandomMapObject`(필드는 `spawnablePrefabs` 목록, `randomizePosition`,
`spawnRange` 기본 10) 중 그 프리팹을 목록에 담고 있는 것들이다. 매 개체마다 스포너 하나를
랜덤으로 고르고 `GetRandomNavMeshPositionInBoxPredictable(스포너, spawnRange)`로 좌표를 잡는다.
`requireDistanceBetweenSpawns`면 같은 종류끼리 5m 이내면 취소한다.
`disallowSpawningNearEntrances` 분기는 **거리를 계산해 놓고 버리는 빈 코드**다 — 디컴파일
과정에서 조건이 최적화로 사라진 흔적일 수 있다(추정).

회전은 `spawnFacingAwayFromWall`이면 `YRotationThatFacesTheFarthestFromPosition`(6방향
레이캐스트로 가장 먼 방향, 여기에 ±15도 난수), `spawnFacingWall`이면 가장 가까운 방향,
아니면 0~359 균등이다. `spawnWithBackToWall`이면 뒤로 100m 레이캐스트해 맞은 점으로 위치를
옮기고, `spawnWithBackFlushAgainstWall`이면 법선에 맞춰 붙인다. 마지막에
`NetworkObject.Spawn(destroyWithScene: true)` 하고, 다 쓴 `RandomMapObject` 마커는
전부 `Destroy` 한다.

### 실외 오브젝트와 실외 위험 (`SpawnOutsideHazards`, 시드 `+2`)

**이 함수는 `FinishGeneratingLevel()` 안에 있어서 서버뿐 아니라 모든 클라이언트가 실행한다.**
시드가 같으므로 결과도 같다 — 나무와 바위는 네트워크로 스폰하지 않고 각자 만든다.

- 비(`Rainy`) 날씨면 진흙(quicksand) 웅덩이를 `Next(5, 15)`개, 그리고 7% 확률로는
  `Next(5, 30)`개 만든다. 위치는 임의의 `outsideAINode` + 반경 30 상자.
- `spawnableOutsideObjects` 각각에 대해 개수 = `randomAmount.Evaluate(NextDouble())`.
  챌린지 지목이면 `+12`. **그리고 20% 확률로 개수를 2배로 뻥튀기한다.**
- 각 개체의 위치 판정은 순서대로: 임의 노드 + 반경 30 상자 → 아래로 5m 레이캐스트해
  `spawnableFloorTags` 중 하나와 맞는지 검사 → `PositionEdgeCheck(objectWidth)`로 네비메시
  가장자리에서 폭만큼 안쪽으로 밀어냄 → `shipSpawnPathPoints`·`SpawnDenialPoint` 태그
  오브젝트에서 `objectWidth + 6` 이내면 취소, `ItemShipLandingNode`에서 `objectWidth + 4`
  이내면 취소 → `objectWidth > 4`인 큰 물체는 이미 놓인 다른 물체들과 폭을 비교해 겹치면 취소.
- `destroyTrees`가 켜진 물체는 자기 `objectWidth` 안의 "Tree" 태그 오브젝트를 파괴한다.
- 하나라도 배치됐거나 날씨가 `Flooded`면 `OutsideLevelNavMesh`의 `NavMeshSurface`를 다시 굽는다.

`SpawnableOutsideObject`는 스크립터블 오브젝트이며 `prefabToSpawn`,
`spawnFacingAwayFromWall`, `objectWidth`(네비메시 가장자리와의 최소 거리 겸 겹침 판정 폭),
`destroyTrees`, `spawnableFloorTags`, `rotationOffset`을 갖는다.

---

## 출입구와 문

### EntranceTeleport

**실내와 실외는 같은 씬, 같은 좌표 공간의 서로 다른 위치다.** 던전은 y = -160보다 아래에
생성되고(문 잠금 필터가 이 값을 쓴다), 이동은 순수한 좌표 텔레포트다. 로딩도 페이드도 없다.

각 `EntranceTeleport`는 `isEntranceToBuilding`(실외 쪽 문인가)과 `entranceId`를 갖는다.
짝은 런타임에 찾는다:

```
// FindExitPoint(): isEntranceToBuilding이 서로 다르고 entranceId가 같은 것을 짝으로 삼는다
if (array[i].isEntranceToBuilding != isEntranceToBuilding && array[i].entranceId == entranceId)
    exitScript = array[i];
```

`entranceId == 0`이 메인 입구이고, 비상구는 `SetExitIDs`가 메인 입구에서 가까운 순으로
1부터 매긴다. 짝을 못 찾으면 "The entrance appears to be blocked." 툴팁을 띄운다 —
**던전이 그 비상구를 쓰지 않은 경우의 폴백이다.**

텔레포트는 로컬 클라이언트가 자기 위치를 옮기고 `TeleportPlayerServerRpc` →
`TeleportPlayerClientRpc`로 남에게 알리는 구조다. 이때 `isInsideFactory`와 들고 있는 모든
아이템의 `isInFactory`가 갱신되고(폭풍 낙뢰 판정과 직결된다), `audioReverbPreset`으로 리버브가
바뀐다. 문 여닫이 애니메이션과 삐걱 소리는 양쪽 문에서 동시에 재생되며, 소리 종류는
`currentDungeonType == 1`(저택으로 추정)일 때만 나무 문 소리를 쓴다.

메인 입구는 `Update`에서 10초(이후 1초) 간격으로 **반대편 출구 지점 7.7m 안에 살아 있는
적이 있는지** 검사해 상호작용 툴팁을 "[Near activity detected!]"로 바꾼다.

### DoorLock

`maxTimeLeft`와 `lockPickTimeLeft` 모두 코드 기본값 60초. `canBeLocked`, `twinDoor`(양문),
락픽 장치가 붙는 위치 두 곳을 갖는다. 잠금 해제는 서버 왕복(`UnlockDoorServerRpc` →
`UnlockDoorClientRpc`)으로 동기화된다. 적이 트리거에 닿으면 서버가 문을 열어준다(잠기지
않았을 때만).

### TerminalAccessibleObject / BreakerBox

`TerminalAccessibleObject`는 코드로 여는 큰 문과 터렛류를 함께 다룬다. `objectCode`,
`codeAccessCooldownTimer`, `isBigDoor`, 코드 표시용 `codeMaterials`/`rows`/`columns`,
지도 레이더 표시용 `mapRadarObject`를 갖는다. 전원이 꺼지면 `OnPowerSwitch`로 반응한다.

`BreakerBox`는 `leversSwitchedOff`(코드 기본값 2)와 `isPowerOn`을 들고 스위치 애니메이터
배열을 관리한다. `SetSwitchesOff()`는 `BreakerBoxRandom.Next(2, 스위치수 - 1)`번 반복하며
매번 랜덤 스위치를 내리는데, 이미 내려간 스위치를 뽑으면 건너뛰므로 **실제로 내려가는 수는
그 값 이하**다. `leversSwitchedOff <= 0`이 되어야 전원이 들어오고, 그때 서버가
`RoundManager.SwitchPower(true)`를 호출해 `onPowerSwitch` 이벤트를 뿌린다.
`PowerSwitchable`은 그 이벤트를 받아 개별 오브젝트에 전달하는 얇은 어댑터다.

### HangarShipDoor

`doorPower`가 1.0에서 시작해 문이 열려 있는 동안 `doorPowerDuration`(코드 기본값 20)초에
걸쳐 0까지 소모되고, 닫으면 `doorPowerDuration * 0.22`(= 4.4초)에 걸쳐 회복한다. 과열되면
100%가 될 때까지 다시 못 쓴다.

### ShipTeleporter

일반 텔레포터는 대상 플레이어를 우주선으로 끌어오면서 **들고 있던 아이템을 전부 떨어뜨린다.**
인버스 텔레포터는 `CanUseInverseTeleporter()`가 `inShipPhase == false`,
`dungeonIsGenerating == false`, 그리고 **`currentLevel.spawnEnemiesAndScrap == true`**를
모두 요구한다 — 던전이 없는 달(회사 건물)에서는 쓸 수 없다는 뜻이다. 목적지는
`RoundManager.insideAINodes` 중 하나(개인별 시드 `randomMapSeed + 17 + playerClientId`)에
반경 10 상자를 더한 지점이다. `hasBeenSpawnedThisSession` 정적 플래그로 세션 중 재배치 시
쿨다운을 유지한다.

---

## 시간과 날씨

### 하루의 구조

`TimeOfDay`의 코드 기본값은 `lengthOfHours = 100f`, `numberOfHours = 7`이고
`Start()`에서 `totalTime = lengthOfHours * numberOfHours = 700`이 계산된다. 상수
`startingGlobalTime = 100`도 있다.

진짜 시계는 `globalTime`이고 달마다 다른 시각을 이 식으로 뽑는다:

```
currentDayTime = (globalTime + level.OffsetFromGlobalTime) * level.DaySpeedMultiplier % (totalTime + 1f)
normalizedTimeOfDay = currentDayTime / totalTime
hour = (int)(currentDayTime / lengthOfHours)
```

`MoveGlobalTime()`은 `globalTime`을 `Time.deltaTime * globalTimeSpeedMultiplier`만큼
`globalTimeAtEndOfDay`까지만 밀고, 같은 양을 `timeUntilDeadline`에서 뺀다. 하루가 끝나는
시각은 착륙 순간에
`globalTimeAtEndOfDay = globalTime + (totalTime - currentDayTime) / DaySpeedMultiplier`로 고정된다.

### DayMode

**정정**: 이전 세션 메모의 Dawn/Day/Dusk/Night가 아니라 `DayMode`는
`None = -1, Dawn, Noon, Sundown, Midnight`이다. `GetDayPhase(t)`의 경계는 정규화 시각 기준
**0.33 미만 Dawn / 0.33~0.63 Noon / 0.63~0.9 Sundown / 0.9 이상 Midnight**이다.
낮 단계가 올라갈 때마다 실외에 있는 플레이어에게 시계 아이콘과 음악이 바뀐다.

### 자동 이륙

`shipLeaveAutomaticallyTime`은 필드 코드 기본값이 0.996이지만 하루가 시작될 때 코드에서
**0.998로 다시 설정된다.** 정규화 시각이 0.9를 넘으면 "곧 떠난다" 경고 대사가 한 번 나가고
(`shipLeavingAlertCalled`), `shipLeaveAutomaticallyTime`에 도달하면 서버가
`SetShipToLeaveOnMidnightClientRpc()`를 보낸다. 투표로 조기 이륙을 걸면 그 값이 앞당겨진다.
착륙 쪽에도 제한이 있다 — 도착 시점의 `GetDayPhase`가 이미 `Midnight`이면 출발 레버가
"Too late on moon to land!"로 잠긴다.

데드라인은 `daysUntilDeadline = floor(timeUntilDeadline / totalTime)`,
`hoursUntilDeadline = timeUntilDeadline / lengthOfHours - daysUntilDeadline * numberOfHours`.

### 그날의 날씨를 정하는 방식

`StartOfRound.SetPlanetsWeather()`(시드 `+35`)가 **모든 달의 날씨를 한꺼번에** 정한다.

```
num2 = Clamp(planetsWeatherRandomCurve.Evaluate(NextDouble()) * num, 0, 1)
날씨가 걸리는 달 수 = Clamp((int)(num2 * (levels.Length - 3f)), 0, levels.Length)
```

여기서 `num`은 평소 1이지만, **멀티플레이(2인 이상)이고 연속 생존 일수가 3의 배수이며 2보다
클 때** `Next(15, 25) / 10`(1.5~2.4배)로 올라간다 — 잘 나가는 팀에게 날씨를 더 뿌리는
난이도 조절이다. 선택된 달은 자기 `randomWeathers` 중 하나를 균등 추첨한다. `levelID == 0`인
달은 `daysUntilDeadline == 3`일 때(= 주기 첫날) 제외된다.

원정이 시작되면 `RoundManager.SetToCurrentLevelWeather()`가 그 달의 `randomWeathers`에서
`currentWeather`와 맞는 항목을 찾아 `currentWeatherVariable`, `...2`, 색을 `TimeOfDay`로
옮긴다. 안개는 `Max(4, variable)`로 하한이 걸리고, `SetWeatherBasedOnVariables`가
`foggyWeather.parameters.meanFreePath = Next((int)var, (int)var2)`(시드 `+101`)로 짙기를 정한다.

### 날씨별 동작

- **Eclipsed** — `OnEnable`에서 `RoundManager.minEnemiesToSpawn`과
  `minOutsideEnemiesToSpawn`을 `currentWeatherVariable`로 올린다. 즉 일식은 스폰 수의
  **하한을 밀어올리는** 방식이다.
- **Flooded** — 수면 오브젝트가
  `Clamp(globalTime / 1080, 0, 100) * currentWeatherVariable2`만큼 `currentWeatherVariable`
  높이에서 위로 올라가며, 초당 0.5의 속도로 목표 높이를 따라간다. 홍수용 네비메시 영역은
  두 변수를 0.225 비율로 lerp한 높이에 한 번만 배치된다.
- **Stormy** — `BeginDay`에서 `randomThunderTime = globalTime + 7`. 매 타격 후 간격은
  `Clamp(Next(-5, 110) * 0.25, 0.6, 110) / Clamp(currentWeatherVariable, 1, 100)`
  이므로 `weatherVariable`이 곧 폭풍 강도다. 금속 아이템 표적 선정은 4초마다 갱신되고,
  **공장 안·우주선 안·버려진 상태가 아닌** 금속 아이템 중 플레이어에게 가장 가까운 것을
  고른다(20% 확률로 순회를 조기 종료해 무작위성을 준다). 낙뢰까지 남은 시간은
  `Clamp(Random.Range(1, 28), 0, 20)`초이고, 10초가 남으면 정전기 파티클 경고가 뜬다.
  들고 있으면 타이머가 4~20초 범위로 연장된다.
- **Meteor** — 날씨가 아니라 그날의 이벤트다. `DecideRandomDayEvents()`(시드 `+28`)가
  `Next(0, 1000) < 7`(0.7%)일 때 `meteorShowerAtTime = Next(5, 80) / 100`으로 시각을 정하고,
  그 정규화 시각에 도달하면 서버가 유성우를 시작한다. 개수와 크기는:

  ```
  개수 = Round(Clamp(numberOfMeteorsCurve.Evaluate(t) * 200, 160, 250) / Clamp(timeOfDay * 2.5, 1, 10))
  크기 = Clamp(baseMeteorSize / (개수 * 0.1), 12, baseMeteorSize)
  ```

  늦게 시작할수록 개수가 줄고, 개수가 적으면 개체가 커진다. 착지점은 우주선 착륙 지점에서
  `크기 * 2`, 메인 입구에서 `크기 + 24` 밖의 `outsideAINode` 중에서 고른다 — **거점 근처는
  보호한다.** 크기가 18 미만이면 `Next(1, 5)`개를 골라 `Next(8, 40)`만큼 키운다.
  `heightInSky = 800`, `landingSpeed = 1.5`, `baseWarningTime = 0.15`, `skyAngularTilt = 0.5`.

`WeatherEffect`는 이름·효과 오브젝트·`lerpPosition`·`effectEnabled`·`sunAnimatorBool`만 가진
가벼운 구조체이고, `SetWeatherEffects()`가 매 프레임 켜진 효과의 오브젝트를 플레이어 위치로
따라 움직인다(또는 lerp). 태양은 `animatedSun`이 `TimeOfDay`에 자기 애니메이터와 두 라이트를
등록하고, `sunAnimator.SetFloat("timeOfDay", ...)`로 하루가 애니메이션 클립으로 재생된다.

---

## 곰팡이(재방문 변화)

달마다 잡초/곰팡이 상태가 세이브에 남아 **다시 방문할 때마다 자란다.**

- 저장 키는 `Level{levelID}Mold`(반복 횟수)와 `Level{levelID}MoldOrigin`(시작 노드 인덱스)이며
  `LoadPlanetsMoldSpreadData()`가 읽어 `SelectableLevel`에 되돌려 놓는다.
- `SetPlanetsMold()`(시드 `+32`)가 매 주기마다 모든 달을 훑는다. 이미 곰팡이가 있는 달은
  무조건 자란다 — `Clamp(15 - luckValue * 100, 0, 100)` 확률로 `Next(2, 8)`, 아니면
  `Next(2, 4)`만큼 반복 횟수가 는다. 아직 없는 달은 새로 생길 확률이 상황에 따라 다르다:
  현재 있는 달이면 0.03(할당량을 3번 이상 채웠으면 0.045), 다른 달이면 0.022, 다만
  위험도 `S++`/`S+`/`A`인 달은 0.015. 우주선 가구가 주는 `luckValue`가 확률을 낮춘다.
- **세이브스커밍 페널티**: `GameNetworkManager`가 달별 `TimesSavescumming` 카운터를 세고,
  5번째부터는 곰팡이가 없으면 `+5`, 있으면 `+1`을 강제로 더한다.
- 실제 확산은 `MoldSpreadManager.GenerateMold(startingPosition, iterations)`가 한다.
  난수 시드가 `(int)startPos.x + (int)startPos.z`라서 **시작 좌표만 같으면 어느 머신에서도
  같은 모양이 나온다.** 매 반복마다 활성 포자 최대 `maxSporesInSingleIteration = 25`개를
  처리하고, 포자마다 `Next(1, moldBranchCount = 3)`개로 분기하며, 각 가지는
  `±moldDistance = 9` 범위(최소 절댓값 2)로 뻗는다. `maxIterations = 25`가 상한이다.
- `MoldAttractionPoint` 태그 오브젝트가 있으면 42m 이상 떨어진 포자는 오프셋 부호를 그쪽으로
  치우쳐 잡초가 특정 지점을 향해 자란다.
- 플레이어가 태운 포자는 인덱스로 `PlanetMoldState.destroyedMold`(달마다 하나)에 기록되고
  `GenerateNewLevelClientRpc`의 `syncDestroyedMold[]` 인자로 전 클라이언트에 전달된다.
  다음 방문에서 그 인덱스의 가지는 자라지 않는다 — **제거가 영구적으로 반영된다.**
- 실내 잡초는 `TileWithGrowth`가 타일 단위로 관리한다. 타일 용량은 기본 40이고, 광산의 cave
  타일이나 room/tunnel 태그가 아닌 타일은 절반으로 줄인다.

---

## 네트워크

정리하면 이렇다.

- **지오메트리는 동기화하지 않는다.** 호스트는 `GenerateNewLevelClientRpc(randomSeed, levelID,
  moldIterations, moldStartPosition, syncDestroyedMold[])` 하나만 보내고, 각 클라이언트가
  DunGen을 자기 머신에서 돌린다. 던전 타일은 네트워크 오브젝트가 아니다.
- **결정론은 시드 오프셋으로 확보한다.** `InitializeRandomNumberGenerators()`가
  `randomMapSeed`에 고정 오프셋을 더한 `System.Random` 여러 개를 만들어, 용도별로 난수 소비
  순서가 어긋나도 서로 영향을 주지 않게 한다. 실외 오브젝트·날씨 효과·유성·곰팡이는 전부
  이 방식으로 각자 계산한다.
- **동기 지점은 두 번뿐이다.** 클라이언트가 던전을 다 만들면
  `FinishedGeneratingLevelServerRpc(clientId)`로 보고하고, 서버는 전원이 끝날 때까지 기다린
  뒤에야 스크랩과 함정을 스폰한다. 이는 스폰 좌표가 클라이언트마다 이미 존재하는 지오메트리를
  전제하기 때문이다.
- **서버가 실제로 네트워크 스폰하는 것**: 스크랩(`SpawnScrapInLevel`), 실내 함정
  (`SpawnMapObjects`, `Spawn(destroyWithScene: true)`), `SpawnSyncedObject` 마커가 지정한
  프롭(`SpawnSyncedProps`), 문 열쇠(`SetLockedDoors`). 이 넷만 `NetworkObject.Spawn()`을 탄다.
- **스크랩 값은 별도 RPC로 나중에 보낸다.** 서버가 스폰하고 11초 뒤
  `SyncScrapValuesClientRpc(참조 배열, 값 배열)`를 쏜다. 값 배열을 따로 보내는 이유는 스폰
  시점의 초기 동기화와 값 확정을 분리하려는 것으로 보인다(추정). 외형(메시·머티리얼) 변형은
  값 대신 시드(`randomMapSeed + 210`)로 클라이언트가 각자 고른다.
- 텔레포트·문 개폐·차단기·엘리베이터처럼 **상태가 갈리면 곤란한 것**만 개별 RPC로 왕복한다.

---

## 수치

### 시드 오프셋 (모두 `StartOfRound.randomMapSeed` 기준)

| 오프셋 | 난수기 / 용도 |
|---|---|
| +0 | `LevelRandom` (DungeonFlow 추첨, DunGen 시드, 문 잠금) |
| +2 | `SpawnOutsideHazards` |
| +3 | 시작 시 정전 판정 |
| +5 | `AnomalyRandom` (스크랩 전반) |
| +12 | 유성 생성 |
| +17 | 큰 문 코드 |
| +17+clientId | 인버스 텔레포터 목적지 |
| +20 | `BreakerBoxRandom` |
| +21, +41 | 실외 적 예측 |
| +28 | 유성우 발생 여부 |
| +30 / +31 / +33 | 실내·실외·주간 적 배치 위치 |
| +32 | 행성별 곰팡이 증가 |
| +35 | 행성별 날씨 결정 |
| +40 / +43 | 실내 적 / 주간 적 스폰 |
| +42 / +43 | 잡초 적 스폰 / 배치 |
| +101 | 날씨 변수 세부 |
| +210 | `ScrapValuesRandom` (메시·머티리얼 변형) |
| +513 | 증기 밸브 시각 |
| +587 | 실내 함정 배치 |
| +2017 | 곰팡이 시작 노드 |
| +5781 | 적 목록 갱신 |

### 핵심 공식

| 대상 | 식 |
|---|---|
| 던전 길이 배율 | `round(factorySizeMultiplier / MapTileSize * mapSizeMultiplier, 2자리)` |
| 던전 배치 상자 | `restrictBounds * 2 * factorySizeMultiplier` |
| 스크랩 개수 | `Next(minScrap, maxScrap) * scrapAmountMultiplier` (광산 +6) |
| 스크랩 가치 | `Next(item.minValue, item.maxValue) * scrapValueMultiplier` |
| 함정/실외 개수 | `curve.Evaluate(NextDouble())` (X 0~1 균등, Y = 개수) |
| 행성 시각 | `(globalTime + Offset) * DaySpeedMultiplier % (totalTime + 1)` |
| 하루 길이 | `lengthOfHours(100) * numberOfHours(7) = 700` |
| 낙뢰 간격 | `Clamp(Next(-5,110) * 0.25, 0.6, 110) / Clamp(weatherVariable, 1, 100)` |
| 홍수 수위 | `weatherVariable + Clamp(globalTime / 1080, 0, 100) * weatherVariable2` |
| 유성 개수 | `Clamp(curve * 200, 160, 250) / Clamp(timeOfDay * 2.5, 1, 10)` |
| 문 잠금 확률 | 1.1에서 시작해 문마다 ÷1.55 (먼 문부터) |
| 락픽 시간 | `Clamp(Next(2, 90), 2, 32)`초 |

### 확률 상수

| 사건 | 확률 |
|---|---|
| 시작 시 정전 | 8% |
| 스크랩 "한 종류 도배" | 약 4.2% (500 중 21) |
| 실외 오브젝트 개수 2배 | 20% |
| 비 날씨의 진흙 대량 발생 | 7% |
| 유성우 발생 | 0.7% (`Next(0,1000) < 7`) |
| 큰 문이 열린 채 시작 | 앞 3개는 확정, 이후 22% |
| 증기 밸브 활성 | 75% |

### 기타 상수

| 값 | 뜻 |
|---|---|
| `cullingTileDepth = 6` | 컬링 BFS 깊이 (코드 기본값) |
| `moldBranchCount = 3`, `moldDistance = 9`, `maxIterations = 25`, `maxSporesInSingleIteration = 25` | 곰팡이 확산 |
| `TileWithGrowth.tileCapacity = 40` | 타일당 잡초 상한 |
| `doorPowerDuration = 20`, 회복 계수 `0.22` | 격납고 문 |
| `-160f` | 던전이 생성되는 y 문턱 |
| 적 감지 반경 `7.7f` | 메인 입구 툴팁 |
| 함정 간 최소 간격 `5f` | `requireDistanceBetweenSpawns` |
| 실외 배치 반경 `30f` | AI 노드 기준 |

---

## CL9 시사점

여기부터는 결정이 아니라 관찰이다.

- **"시드만 보내고 각자 만든다"는 리슨 서버와 궁합이 좋다.** 리썰은 맵 지오메트리를 한 바이트도
  복제하지 않고, 대신 두 개의 배리어(전원 생성 완료 대기, 그 뒤 서버 단독 스폰)로 순서를
  맞춘다. CL9가 REPO 방식 모듈 조립으로 가더라도 이 골격 — 호스트가 정수 시드 하나 + 사이트
  ID를 보내고, 클라이언트가 조립하고, 완료 보고를 모은 뒤에야 아이템을 스폰하는 순서 — 은
  그대로 옮길 수 있는 형태다.
- **결정론을 "난수기 하나"가 아니라 "용도별 난수기 여럿"으로 지킨다.** 오프셋을 20개 넘게
  나눠 둔 이유는, 어느 한 시스템이 난수를 하나 더 소비해도 다른 시스템의 결과가 밀리지 않게
  하려는 것으로 보인다. UE에서 `FRandomStream`을 용도별로 나눠 갖는 것과 같은 발상이다.
- **배치를 "곡선 한 번 평가"로 끝낸다.** 실내 함정과 실외 오브젝트의 개수가 전부
  `curve.Evaluate(random01)` 한 줄이다. 최소/최대/평균 같은 파라미터를 따로 두지 않고 곡선
  모양 하나로 분포를 표현하므로, 디자이너가 에디터에서 직접 만지기 쉽다. UE의
  `FRuntimeFloatCurve`로 같은 패턴이 가능하다.
- **"지점 마커 + 반경 상자 + 네비메시 스냅"이 배치의 기본 단위다.** 리썰은 타일 프리팹 안에
  `RandomScrapSpawn`·`RandomMapObject` 같은 빈 마커를 심어 두고, 런타임에는 그 마커 주변
  상자 안에서 네비메시 위 점을 찾는다. 모듈 조립으로 가더라도 모듈 프리팹 안에 스폰 마커를
  심는 방식은 동일하게 성립한다.
- **지점 소진 규칙이 분포를 만든다.** 후보가 바닥날 때까지 지점을 재사용하지 않다가, 다 쓰면
  목록을 비우고 다시 도는 방식은 "고르게 퍼뜨리되 개수가 많으면 겹치게" 되는 자연스러운
  결과를 준다. 별도의 거리 판정 없이 분포를 얻는 값싼 기법이다.
- **총액 목표를 강제하지 않는다.** 필드는 있지만 쓰지 않고, 개수와 개별 가치의 곱으로 흘러가게
  둔다. CL9에서 재료 총량을 정확히 통제할지, 리썰처럼 변동에 맡길지는 갈림길이다.
- **어둠과 정전을 "확률"과 "차단기"로 나눠 놓았다.** 8% 확률로 처음부터 정전이고, 그 복구는
  차단기 앞에서 레버 여러 개를 올리는 물리적 행동이다. CL9의 "어둠이 기본이고 정전이 온다"는
  설정과 맞닿은 구조다.
- **밤 길이를 하나의 정규화 값으로 통일했다.** `normalizedTimeOfDay` 하나로 낮 단계, 적 스폰
  곡선, 자동 이륙, 유성우 시각, 조명 밝기를 전부 인덱싱한다. CL9의 "밤 길이가 원정 제한
  시간"도 같은 방식으로 0~1 하나에 걸어 두면 UI·AI·조명이 같은 축을 공유하게 된다.
- **출입구가 좌표 텔레포트일 뿐이라는 점.** 실내와 실외가 같은 레벨에 있고 y로만 떨어져 있어서
  로딩이 없다. CL9의 트럭이 맵 밖에 고정된다면, 트럭↔사이트 이동을 레벨 스트리밍으로 볼지
  같은 레벨 안의 좌표 이동으로 볼지가 같은 종류의 선택이 된다.
- **재방문에 상태를 남기는 값싼 방법.** 곰팡이는 "시작 좌표 + 반복 횟수 + 파괴된 인덱스 목록"
  세 값만 저장하면 전체 모양이 재현된다. 사이트마다 누적되는 변화를 주고 싶을 때, 결과가
  아니라 생성 파라미터를 저장하는 방식은 저장 용량과 동기화 비용이 모두 작다.
- **난이도 보정이 데이터가 아니라 코드 분기에 숨어 있다.** 연속 생존 일수에 따른 날씨 확률
  상승, 데드라인 압박에 따른 적 수 증가, 세이브스커밍 페널티가 전부 하드코딩된 상수다.
  읽기는 어렵지만 튜닝 파라미터가 데이터로 노출되지 않아 밸런스가 흔들리지 않는 장점도 있다.
