# 참고 — R.E.P.O.의 맵 생성

> 2026-09-08, 사용자 요청("리포랑 리썰 참고해서 조사해 줘")으로 설치본을 분석한 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](../README.md), CL9의 결정은
> [../data/module.md](../../plan/data/module.md)에 있다.

**CL9이 따라가기로 한 모델이 이것이다.** 리썰컴퍼니보다 우리 구조에 가깝다 — 격자 기반이기 때문이다.
`DunGen` 같은 기성품이 없고 `Assembly-CSharp.dll`의 자체 `LevelGenerator`다.

R.E.P.O.의 레벨은 `LevelGenerator`가 정사각 격자 위에서 랜덤 워크로 타일을 켜고, 켜진 타일마다 15 m 정사각 모듈 프리팹을 하나씩 놓는 방식으로 만들어진다. 생성은 마스터 클라이언트(호스트)만 수행하고, 시드를 공유하는 대신 모든 모듈·연결물·귀중품·몬스터를 Photon 룸 오브젝트로 네트워크 인스턴스화해서 결과를 그대로 복제한다. 모듈은 모두 같은 크기이고, 높이 차 없이 Y축 회전만 허용되며, 본체(Normal) 모듈은 사방이 전부 열릴 수 있고 쓰지 않는 변은 차단물 프리팹으로 막는다. 소품 교체는 확률이 아니라 "그 변이 연결됐는가", "그 자리에 귀중품이 뽑혔는가", "트랩이 켜졌는가"에 따라 결정적으로 갈린다. 몬스터·경로·방 판정은 모듈 안에 손으로 놓은 `LevelPoint` 웨이포인트와 `RoomVolume` 박스가 담당하고, 미니맵도 모듈 안에 손으로 놓은 바닥·벽 마커를 4 m 단위 층으로 나누어 그린다. 아래는 모두 디컴파일 C# 소스에서 직접 확인한 내용이며, 추정은 문장에 "(추정)"을 붙였다.

## 생성 알고리즘

`LevelGenerator.LevelState` 열거형은 진행 단계를 그대로 보여준다: `Start`, `Load`, `Tiles`, `StartRoom`, `ConnectObjects`, `ModuleGeneration`, `BlockObjects`, `ModuleSpawnLocal`, `ModuleSpawnRemote`, `LevelPoint`, `Item`, `Valuable`, `PlayerSetup`, `PlayerSpawn`, `Enemy`, `Done`. `Generate()` 코루틴이 이 순서로 하위 코루틴을 부르고, 각 단계는 `waitingForSubCoroutine` 플래그로 직렬화된다.

1. **시드와 준비.** 별도 시드는 없다. `UnityEngine.Random`을 그대로 쓰고, 마스터 클라이언트(또는 싱글플레이)만 `TileGeneration` 이하를 실행한다("시드 공유" 추측은 **정정**). 멀티플레이면 `AllPlayersReady`가 될 때까지 `Load` 상태로 대기한 뒤 `Level = RunManager.instance.levelCurrent`를 잡고 `ModuleAmount = Level.ModuleAmount`로 시작한다.

2. **격자 크기(`TileGeneration`).** `LevelWidth`·`LevelHeight`는 생성기 컴포넌트에 직렬화된 값(코드 기본값 3×3, 실제 에셋 값은 코드로 알 수 없음)이며 `Mathf.Max(2, Mathf.CeilToInt(값 * DebugLevelSize))`로만 보정된다. `DebugLevelSize`는 `DebugComputerCheck.LevelSizeMultiplier`에서만 설정되는 **디버그 노브**다(이전 세션의 `LevelSizeMultiplier`는 게임 곡선이 아니라 디버그 값 — **정정**). 플레이어 수는 격자 크기에도 모듈 수에도 전혀 들어가지 않는다(**정정**).

3. **모듈 수 공식.** `Level.ModuleAmount`(코드 기본값 6)가 4보다 크면 아래 공식으로 덮어쓴다. 그 이하(상점·로비 프리셋으로 추정)는 에셋 값을 그대로 쓴다.
   ```
   ModuleAmount = Mathf.Min(5 + levelsCompleted, 10);
   if (levelsCompleted >= 10) ModuleAmount += Mathf.Min(levelsCompleted - 9, 5);
   ModuleAmount = Mathf.CeilToInt(ModuleAmount * DebugLevelSize);
   ```
   따라서 랜덤 워크 모듈 수는 5(첫 판)에서 10(6판째)까지 오르고, 11판째부터 다시 15까지 오른 뒤 고정된다. 부속 모듈 수는 `DeadEndAmount = Mathf.CeilToInt(ModuleAmount / 3)`인데 `ModuleAmount`가 int라 정수 나눗셈이 먼저 일어나므로 실제로는 `floor(ModuleAmount / 3)`이다. `ExtractionAmount`는 15 이상 4, 10 이상 3, 8 이상 2, 6 이상 1, 그 미만 0이다. `SpecialAmount = Level.ModulesSpecialAmount`. 상점 레벨(`SemiFunc.IsLevelShop`)은 `DeadEndAmount = 1`로 강제된다. 이 부속 모듈들은 `ModuleAmount`에 **더해지는** 것이라 실제 총 모듈 수는 워크 수 + 막다른 + 추출 + 특수다.

4. **랜덤 워크.** 시작 타일은 `LevelGrid[LevelWidth / 2, 0]`이고 `first = true`다. 커서를 그 타일에 두고 `moduleAmount`가 0이 될 때까지 상하좌우 중 하나를 뽑아 이동한다. `y == 1`이면 `Random.Range(0, 3)`으로 아래(y-1) 방향을 빼서 한 번 2행으로 올라가면 다시 1행(시작 행)으로 내려오지 못하게 한다. 격자 밖이면 재추첨한다. 이미 켜진 타일을 밟는 것은 허용되며 그때는 카운트를 줄이지 않는다 — 이것이 가지치기와 되돌아감을 만든다. 실패 재시도는 없다. 격자가 꽉 차면 이 루프는 끝나지 않으므로 에셋의 격자가 워크 수를 충분히 담는 크기여야 한다.

5. **부속 타일 후보와 배치.** 워크가 끝나면 "꺼져 있으면서 켜진 이웃이 정확히 1개인 타일"을 모두 `possibleExtractionTiles`에 모은다. 추출 타일은 farthest-point sampling으로 고른다. 기준 목록 `_extractionTiles`에 가상 타일 `(LevelWidth / 2, -1)` — 즉 격자 바로 아래의 트럭·시작방 위치 — 를 먼저 넣고, 후보 중 "기준 목록과의 최소 거리가 가장 큰" 타일을 하나씩 뽑아 `SetExtractionTile(Module.Type.Extraction, …)`으로 켠다. 그래서 첫 추출 지점은 트럭에서 가장 먼 곳, 다음 추출 지점은 트럭과 앞선 추출 지점 모두에서 가장 먼 곳에 놓인다. `SetExtractionTile`은 타일을 켜고 후보에서 빼며, **그 타일에 인접한 후보들도 전부 제거**해 막다른 방끼리 붙거나 부속 방이 두 개의 이웃을 갖는 일을 막는다. 특수(Special) 타일과 막다른(DeadEnd) 타일은 같은 후보 목록에서 무작위로 뽑는다. 후보가 바닥나면 남은 수는 그냥 버린다.

6. **시작방(`StartRoomGeneration`).** `Level.StartRooms`를 셔플해 첫 번째를 원점 `(0,0,0)`, 회전 없음으로 `PhotonNetwork.InstantiateRoomObject`한다. 시작방은 격자 바깥에 있고, 첫 모듈은 z = 7.5에 놓이므로 첫 모듈의 아래 변(z = 0)이 시작방 정면과 맞닿는다. 트럭은 시작방 프리팹 안에 있다(`RoomVolume.Truck`, `TruckSafetySpawnPoint`, `LevelPoint.Truck`가 여기서 온다 — 추정).

7. **연결물(`GenerateConnectObjects`).** 모듈보다 먼저 실행된다. 켜진 타일마다 `connections`(켜진 이웃 수)를 세고, 위 이웃과 공유하는 변 중앙에 회전 0°, 오른쪽 이웃과 공유하는 변 중앙에 회전 90°로 `Level.ConnectObject` 프리팹을 하나씩 놓는다. 시작 타일의 아래 변에도 회전 0°로 하나 놓아 시작방과 잇는다. 왼쪽·아래 분기는 순회 순서상 이미 이웃이 놓은 뒤라 시작 타일을 빼고는 실행되지 않는다. 변 중앙 좌표는 타일 중심 ± `ModuleWidth * TileSize / 2` = ±7.5 m다.

8. **모듈 배치(`ModuleGeneration`).** 각 타입별 3개 난이도 리스트(`ModulesNormal1..3` 등)를 셔플한 가방으로 만들고, 가중치 `ModuleRarity1..3 = DifficultyCurve1..3.Evaluate(SemiFunc.RunGetDifficultyMultiplier1())`를 계산한다. `RunGetDifficultyMultiplier1() = Mathf.Clamp01(levelsCompleted / 9f)`다. 격자를 x, y 순으로 훑으며 켜진 타일마다:
   - 위치는 `((x - LevelWidth / 2) * 15, 0, y * 15 + 7.5)`.
   - **Extraction·DeadEnd·Special**은 유일한 켜진 이웃을 향해 회전한다: 아래 이웃 0°, 왼쪽 90°, 위 180°, 오른쪽 -90°. 즉 이 모듈들은 로컬 -Z(아래) 변이 유일한 입구다.
   - **Passage**는 `PassageAmount < Level.PassageMaxAmount`(코드 기본값 2)일 때만 나온다. 위가 켜져 있고 (아래가 켜져 있거나 `first`)이며 좌우가 꺼져 있으면 세로 통로로 0° 또는 180°를 50%로 고르고, `first`가 아니면서 좌우가 켜져 있고 상하가 꺼져 있으면 가로 통로로 90° 또는 -90°를 50%로 고른다. 통로는 정확히 마주보는 두 변만 여는 일자형이다.
   - 그 밖의 모든 타일은 **Normal**이고 회전은 `ModuleRotations` 4개 중 무작위다. 어떤 회전이든 성립해야 하므로 Normal 모듈은 사방이 전부 열릴 수 있어야 한다.
   - `SpawnModule`이 프리팹을 룸 오브젝트로 인스턴스화하고 `GridX/GridY`를 적은 뒤 `ModuleConnectionSet(top, bottom, right, left, first)`를 부른다. `bottom`은 아래 이웃이 있거나 `first`면 true다.

9. **희귀도 추첨(`PickModule`).** 이전 세션이 "희귀도 1/2/3"이라 부른 것은 실제로는 **난이도 티어 리스트 3개에 대한 가중치**다(**정정**). 가중치 배열 `[R1, R2, R3]`에서 빈 리스트는 0으로 만든다. 세 리스트의 "가방을 다 돈 횟수"(`loops`)가 서로 다르면 가장 적게 돈 리스트만 남기고 나머지 가중치를 0으로 만든다(균등 소진). 남은 각 티어에 대해 `Random.Range(0f, 가중치)`를 뽑아 가장 큰 값이 나온 티어를 택하고, 그 티어의 셔플 가방에서 순서대로 하나 꺼낸다. 가방이 비면 재셔플하고 `loops`를 올린다. Special은 리스트가 하나뿐이라 같은 함수를 세 인자 모두 같은 리스트로 부른다.

10. **차단물(`GenerateBlockObjects`).** **Normal 타일만** 대상이다. 켜진 이웃이 없는 변 중앙에 `Level.BlockObject`를 놓는다(위 180°, 오른쪽 -90°, 아래 0° — 단 시작 타일의 아래 변은 제외, 왼쪽 90°). Passage·DeadEnd·Extraction·Special은 프리팹 자체가 필요한 변만 열려 있어서 차단물이 필요 없다.

11. **스폰 확인과 환경 셋업.** `ModulesSpawned < ModuleAmount - 1`인 동안 `ModuleSpawnLocal`로 대기하고(부속 모듈까지 세면 이 조건은 느슨한 하한이다), `ModulesReadyRPC`(AllBuffered)로 각 클라이언트가 준비를 알리면 `ModuleSpawnRemote`에서 `PhotonNetwork.CurrentRoom.PlayerCount`만큼 모일 때까지 기다린다. 그 뒤 모든 클라이언트가 `EnvironmentDirector.Setup()`, `PostProcessing.Setup()`, `LevelMusic.Setup()`, `ConstantMusic.Setup()`을 부른다.

12. **마스터 후반 단계.** `LevelPathPoints.Count == 0`인 동안 `LevelPoint` 상태로 기다린다(모듈 안 `LevelPoint`들이 `Start`에서 스스로 등록한다). 이어서 `ItemSetup`(상점·아이템 초기화, 아레나 제외) → `ValuableDirector.SetupHost()` 완료 대기(`Valuable`) → `NavMeshSetup()` → `GameDirector.PlayerList`가 찰 때까지 대기(`PlayerSetup`) → `PlayerSpawn()` → 모든 플레이어가 `PlayerSpawnedRPC`를 보낼 때까지 대기(`PlayerSpawn`) → `Level.HasEnemies`면 `EnemySetup()`과 `EnemyReadyRPC` 집계(`Enemy`) → `Done` → `GenerateDone` RPC(AllBuffered)에서 `SemiFunc.OnLevelGenDone()`, `GameDirector.SetStart()`, `Generated = true`. 마지막에 `TruckSafetySpawnPoint.instance`가 없으면 에러를 찍는다.

13. **네브메시.** `NavMeshSetupRPC`는 AllBuffered로 보내지만 실행은 각 클라이언트가 자기 `NavMeshSurface`에 대해 `RemoveData()` → `BuildNavMesh()`를 하는 로컬 런타임 빌드다. 디버그 빌드에서는 `NavMeshValidator.SafetyCheck()`가 `NavMesh.CalculateTriangulation()` 결과를 정점 용접·간선 인접으로 섬(island)으로 나누고, 가장 큰 섬과 경로가 이어지지 않는 섬을 `GetRoomVolumeAtPosition`으로 모듈 이름·로컬 좌표와 함께 에러로 보고한다. `NavMeshBox`는 기즈모만 그리는 저작용 마커다.

14. **플레이어 스폰(`PlayerSpawn`).** `FindObjectsOfType<SpawnPoint>()`를 셔플해 플레이어마다 하나씩 배정하고 부족하면 순환한다. `SpawnPoint.debug`가 켜진 점이 하나라도 있으면 그것들만 쓴다. 스폰 뒤 `PlayerDeathHeadPrefab`·`PlayerTumblePrefab`을 플레이어마다 하나씩 네트워크 인스턴스화한다. `SpawnPoint`는 시작방 프리팹 안에 있다(추정 — 생성기가 따로 놓지 않는다).

15. **몬스터 스폰(`EnemySetup`).** `RoomVolume.Truck`인 볼륨을 찾고 `LevelPathPoints` 중 그 볼륨에서 가장 먼 `LevelPoint`를 `furthestPoint`로 잡는다. `EnemyDirector.AmountSetup()`이 난이도 곡선으로 `totalAmount`를 정하면 그 수만큼 `GetEnemy()`로 세트를 꺼내 **전부 `furthestPoint` 위치에** `InstantiateRoomObject`하고 `EnemyTeleported`로 옮긴다. 이어 `EnemyDirector.FirstSpawnPointAdd`가 트럭이 아닌 `LevelPoint` 중 `LevelPathTruck`에서 가장 먼 점을 적마다 겹치지 않게 `firstSpawnPoint`로 배정한다(이미 배정된 점은 거리 0으로 취급). 즉 몬스터 전용 스폰 액터는 없고, 웨이포인트 그래프가 스폰 지점을 겸한다.

## 모듈 데이터

`Module`(MonoBehaviour)은 프리팹 루트에 붙는다. `Module.Type`은 `Normal`, `Passage`, `DeadEnd`, `Extraction`, `Special` 다섯 가지다. 런타임 필드는 `ConnectingTop/Right/Bottom/Left`, `SetupDone`, `First`, `GridX`, `GridY`, `Explored`, `StartRoom`(자식에 `StartRoom` 컴포넌트가 있으면 true)이고, `moduleOwner`는 제작자 표시다. 나머지 public bool 21개는 저작 QA 체크리스트다: `wallsInside`, `wallsMap`, `levelPointsEntrance`, `levelPointsWaypoints`, `levelPointsRoomVolume`, `levelPointsNavmesh`, `levelPointsConnected`, `lightsMax`, `lightsPrefab`, `roomVolumeDoors`, `roomVolumeHeight`, `roomVolumeSpace`, `navmeshConnected`, `navmeshPitfalls`, `valuablesAllTypes`, `valuablesMaxed`, `valuablesSwitch`, `valuablesSwitchNavmesh`, `valuablesTest`, `ModulePropSwitchSetup`, `ModulePropSwitchNavmesh`. 하나라도 꺼져 있으면 `ModuleConnectionSetRPC`에서 "Module not checked off" 경고를 찍는다. 이 목록 자체가 REPO 모듈 하나가 갖춰야 할 요소의 명세다: 내부 벽, 미니맵 마커, 입구·웨이포인트·방 볼륨·네브메시에 걸친 `LevelPoint`, 조명 상한, 문이 있는 방 볼륨, 높이·여유 공간, 연결된 네브메시, 낙하 구멍 검증, 모든 크기의 귀중품 볼륨, 귀중품 스위치, 소품 스위치.

`Module.OnDrawGizmos`는 소켓 위치를 알려준다. 모듈 경계는 `ModuleWidth * TileSize` = 15 m 정사각이고, **각 변의 정중앙**에 ±1 m(폭 2 m) 표식을 그린다. `GenerateConnectObjects`·`GenerateBlockObjects`도 변의 중앙점에만 프리팹을 놓는다. 즉 소켓은 변당 하나, 변 중앙이다.

`ModuleConnectionSet`은 멀티플레이면 `ModuleConnectionSetRPC`를 `RpcTarget.All`로 보내고, 수신 측은 네 방향 플래그와 `First`를 저장한 뒤 자식의 모든 `ModulePropSwitch`에 `Module`을 넣고 `Setup()`을 부르며 `LevelGenerator.ModulesSpawned++`한다.

`Level`(ScriptableObject, 메뉴 "Level/Level Preset")의 필드 전부: `NarrativeName`, `NarrativeNameLocalized`, `ModuleAmount`(코드 기본값 6), `PassageMaxAmount`(2), `HasEnemies`(true), `ConnectObject`, `BlockObject`, `StartRooms`(리스트), `LoadingGraphic01..03`, 난이도 1·2·3별 `ModulesNormalN`·`ModulesPassageN`·`ModulesDeadEndN`·`ModulesExtractionN`, `ModulesSpecialAmount`, `ModulesSpecial`, `ValuablePresets`(`LevelValuables` 리스트), `MusicPreset`, `ConstantMusicPreset`, `AmbiencePresets`(`LevelAmbience` 리스트), 안개 `FogColor`(검정)·`FogStartDistance`(0)·`FogEndDistance`(15), 환경광 `AmbientColor`·`AmbientColorAdaptation`, 색 `ColorTemperature`·`ColorFilter`, 블룸 `BloomIntensity`(10)·`BloomThreshold`(0.9), `LensDistortionIntensity`(30), 비네트 `VignetteColor`·`VignetteIntensity`(0.4)·`VignetteSmoothness`(0.7), `DiscordIcon`. 즉 테마 하나가 모듈 리스트 13개 + 귀중품 프리셋 + 음악·앰비언스 + 후처리를 한 에셋에 담는다.

`LevelValuables`는 크기별 프리팹 리스트 7개(`tiny`, `small`, `medium`, `big`, `wide`, `tall`, `veryTall`)다. `LevelAmbience`는 `loopClip`·`loopVolume`과 `breakers`(간헐음 `LevelAmbienceBreaker`: `sound`, `volume`)다.

부속 컴포넌트는 매우 얇다. `ModuleBlockObject`는 `Start`에서 부모를 `LevelParent`로 옮길 뿐이다. `ModuleConnectObject`는 `ModuleConnecting`·`MasterSetup` 두 bool을 `OnPhotonSerializeView`로 동기화하고 `MasterSetup` 뒤 부모를 옮긴다. `StartRoom`은 부모를 옮기는 마커, `TruckSafetySpawnPoint`는 싱글턴 마커(정찰 점수의 거리 기준점), `SpawnPoint`는 `debug` bool 하나와 화살표 기즈모다. `LevelMaterialSwitch`는 `switches`(`Level`→`Material`) 목록에서 현재 레벨과 맞는 머티리얼을 `MeshRenderer`에 끼운다.

## 손으로 만든 양 — 모듈 이름 통계

에셋 파일에서 `Module - ...` 이름을 전부 뽑아 집계했다. 디버그용 20개(`Module - Debug 이름`)를
빼면 **모듈 311개**가 나온다.

| 테마 | 계 | N 본체 | DE 막다른 | P 통로 | E 특수 | S 시작 |
|---|---|---|---|---|---|---|
| Manor | 80 | 29 | 20 | 15 | 16 | — |
| Wizard | 72 | 23 | 18 | 16 | 15 | — |
| Arctic | 71 | 25 | 15 | 16 | 15 | — |
| Museum | 69 | 23 | 14 | 18 | 14 | — |
| Shop | 19 | 3 | 15 | — | — | 1 |
| **계** | **311** | **103** | **82** | **65** | **60** | **1** |

명명 규칙이 그대로 설계도다: `Module - {테마} - {타입} - {티어} - {이름}`.
네 번째 칸은 **난이도 티어 1·2·3**이고 개수는 114 / 95 / 83이다. 상점 모듈 19개만 이 칸이
없다. 이전 세션이 이 칸을 "희귀도"로 읽었던 것은 **정정**이다 — `PickModule`이 쓰는 것은
`DifficultyCurve1..3`로 평가하는 티어 가중치이지 고정 희귀도가 아니다.

타입 비율은 스테이지 테마 넷에서 대략 **N 34% · DE 24% · P 22% · E 20%**로 고르다. 즉 테마
하나를 만들 때 본체를 가장 많이, 나머지 셋을 비슷하게 그린다. 이름을 보면 N은
`Big kitchen`·`Cross Corridors`·`Master Bedroom` 같은 큰 방, DE는 `Small Bedroom`·`Wardrobe`
같은 작은 곁방, P는 `Tight Corridor`·`Stair Passage` 같은 이음, E는 `Basement Extract`처럼
추출 지점이 들어가는 방이다.

## 소품 교체

`ModulePropSwitch`는 "연결 여부에 따른 교체"다. 필드는 `ConnectionSide`(`Top`/`Right`/`Bot`/`Left`), `ConnectedParent`, `NotConnectedParent`다. `Setup()`은 모듈의 Y 회전을 90°씩 세며 `ConnectionSide`를 한 칸씩 돌려 월드 기준 방향으로 바꾼 뒤, 그 변의 `Module.ConnectingX`가 true면 `ConnectedParent`만, 아니면 `NotConnectedParent`만 켠다. **확률은 없다** — 같은 모듈이 매번 다르게 보이는 이유는 소켓 연결 여부가 판마다 다르기 때문이다("매번 바뀐다"는 관찰의 원인을 **정정**). 네브메시 재구성도 따로 없다. 소품 스위치는 `ModuleConnectionSetRPC` 시점에 끝나고 네브메시는 그보다 뒤인 `NavMeshSetup`에서 한 번만 빌드되므로 결과가 자연히 반영된다.

`ValuablePropSwitch`는 "귀중품 자리 교체"다. `ValuableParent`/`PropParent`를 갖고, `Awake`에서 일단 Valuable 쪽을 켜 두어 그 안의 `ValuableVolume`이 스캔되게 한다. `ValuableDirector`가 귀중품을 다 놓은 뒤 `VolumesAndSwitchSetupRPC`(All)에서 모든 `ValuableVolume.Setup()`과 `ValuablePropSwitch.Setup()`을 부르고, 자식 중 활성 `ValuableObject` 또는 `CosmeticWorldObject`가 있으면 Valuable 쪽을 유지하고 없으면 `PropParent`로 바꾼다. 즉 귀중품이 뽑히지 않은 자리는 장식 소품으로 메워진다. 스위치 안에 스위치를 두면 에러다.

`TrapPropSwitch`는 `TrapParent`/`PropParent`를 둘 다 켜 두었다가 `TrapDirector.TrapListUpdated` 뒤 자식 `Trap`이 활성이면 소품을 끈다.

## 스폰 지점(LevelPoint)

`LevelPoint`는 모듈 프리팹 안에 손으로 놓는 웨이포인트다. 필드는 `ModuleConnect`(모듈 입구 점), `Truck`, `Room`(`RoomVolume`), `ConnectedPoints`(같은 모듈 안 이웃 점, 양방향으로 손으로 연결), `inStartRoom`이다. `Start`에서 `LevelGenerator.LevelPathPoints`에 스스로 등록하고 `Truck`이면 `LevelPathTruck`이 된다. `ModuleConnect` 점은 `Generated` 뒤 `ModuleConnectSetup`에서, 15 m 이내에 있고 서로 마주보며(`forward` 내적 ≤ -0.8) 상대가 자기 정면(내적 > 0.8)에 있는 다른 `ModuleConnect` 점을 `ConnectedPoints`에 추가한다 — 이것이 모듈 간 그래프 연결이다. `NavMeshCheck`는 점이 네브메시 위인지(`SamplePosition` 0.5 m), `Room`이 있는지, 연결이 양방향인지 검사해 에러를 찍는다. `RoomVolume.Setup`이 자기 박스·메시 콜라이더로 `Physics.OverlapBox`("Other" 레이어)를 돌려 안에 든 `LevelPoint.Room`을 채운다.

`SemiFunc`의 선택 함수 규칙은 아래 표와 같다. 모두 `LevelPathPoints` 전체를 훑는 O(n) 검색이다.

| 함수 | 규칙 |
|---|---|
| `LevelPointGet(pos, min, max)` | 트럭 방이 아닌 점 중 거리 [min, max]에서 무작위 |
| `LevelPointGetWithinDistance(pos, min, max)` | 거리 [min, max]인 점 목록(트럭 제외 없음), 없으면 null |
| `LevelPointsGetPlayerDistance(_, min, max, startRoomOnly)` | 트럭 방 제외, 가장 가까운 플레이어와의 거리 > min 이고 어떤 플레이어와는 < max 인 점 목록 |
| `LevelPointGetPlayerDistance(...)` | 위 목록에서 무작위 하나 |
| `LevelPointGetFurthestFromPlayer(_, min)` | 트럭 방 제외, 모든 플레이어에서 min보다 멀면서 가장 먼 플레이어 거리가 최대인 점 |
| `LevelPointsGetInPlayerRooms()` | 플레이어들의 `RoomVolumeCheck.CurrentRooms`와 같은 `Room`을 가진 점들 |
| `LevelPointInTargetRoomGet(check, min, max)` | 대상의 현재 방 안 점 중 거리 조건 만족하는 것에서 무작위 |
| `LevelPointsGetClosestToPlayer()` / `...LocalPlayer()` | 어느 플레이어에든 가장 가까운 점 |
| `LevelPointsGetAllCloseToPlayers()` | 플레이어마다 가장 가까운 점 하나씩 |
| `LevelPointsGetVisiblePointsBehindPlayers(eye, max, dot)` | 플레이어 뒤쪽(내적 ≤ dot)이고 시선 차단 없는 점 |
| `LevelPointsGetInStartRoom()` | `inStartRoom` 점 |
| `LevelPointListPurgeObstructed(list, capsule)` | 캡슐 자리에 플레이어·물리 오브젝트가 겹치는 점 제거 |

`FindLevelPointInCone`은 `SemiFunc`가 아니라 `EnemyTricycle`의 private 메서드로, 진행 방향 원뿔(65°, 15~35 m) 안의 점을 찾는다. `Enemy.GetLevelPointAhead`는 목표에 가장 가까운 점의 `ConnectedPoints` 중 진행 방향과 내적이 큰 이웃을 고른다.

용도별로 보면 **몬스터**는 초기 배치를 트럭에서 가장 먼 점에서 하고, 로밍은 `EnemyStateRoaming`이 `PlayerFarMove`가 아니면 `LevelPathPoints` 전체에서 무작위, 맞으면 `LevelPointGetFurthestFromPlayer(pos, 5)`를 고른다. 각 적 클래스는 재등장 위치에 `LevelPointGetPlayerDistance(pos, 30, 50)` 같은 거리 밴드를 쓴다(Tick은 8~50, Hunter·Spinny는 25~50, ValuableThrower는 30~60). `EnemyParent.GetRoomVolume`은 적의 현재 방을 `RoomVolume` 레이어 OverlapBox로 얻는다. **귀중품**은 `LevelPoint`를 쓰지 않고 모듈 안에 손으로 놓은 `ValuableVolume`(`Tiny`, `Small`, `Medium`, `Big`, `Wide`, `Tall`, `VeryTall`)에 놓인다. `ValuableDirector.SetupHost`는 난이도 곡선으로 크기별 상한과 `totalMaxValue`를 정하고, 모든 `ValuableVolume`을 크기별로 셔플해 순서대로 채우며, 누적 가치가 상한을 넘으면 멈춘다. **플레이어**는 `SpawnPoint`를 쓴다. `MapBacktrack`은 `Room.Truck`인 `LevelPoint`를 트럭 목적지로 삼는다.

## 방 볼륨

`RoomVolume`은 모듈 안의 방 하나를 감싸는 박스(또는 메시) 콜라이더 묶음이다. 필드는 `Truck`, `Extraction`, `Color`, `ReverbPreset`, `RoomAmbience`(볼륨), `RoomAmbienceOverride`(`LevelAmbience`), `RoomAmbientLight`, `RoomFog`, `RoomTransitionSpeedMultiplier`(1), `Module`, `MapModule`, `Explored`다. `Awake`에서 자식에 중첩된 `RoomVolume`을 제거하고, `Setup`에서 안에 든 `LevelPoint.Room`을 채운 뒤, 트럭·추출·시작방이 아니면 콜라이더마다 `Map.Instance.AddRoomVolume`으로 미니맵의 미탐색 덩어리를 만든다. 상점·아레나에서는 그 덩어리를 즉시 숨긴다. `SetExplored()`는 처음 한 번만 true를 돌려주고 `MapModule.Hide()`를 부른다.

`RoomVolumeCheck`는 플레이어 아바타와 귀중품에 붙는다. `Generated` 뒤 플레이어는 0.1초, 그 밖은 0.5초 간격으로 `RoomVolume` 레이어에 `OverlapBox`를 돌려 `CurrentRooms`, `inTruck`, `inExtractionPoint`를 갱신한다. 플레이어가 새 방을 밟아 `SetExplored`가 true를 돌려주면 `ProgressionManager.roundPointsScouting += 100 * (트럭 안전 지점까지의 거리 / 20)`으로 정찰 점수를 준다. 디버그 빌드에서 방 밖에 있으면 `SemiFunc.GetModuleAtPosition`으로 모듈 이름과 로컬 좌표를 에러로 찍는다. 방 볼륨은 이렇게 "탐색 여부·정찰 점수·트럭/추출 안 판정·적의 플레이어 방 판정·오디오·안개"의 단위다.

`EnvironmentDirector`는 매 프레임 로컬 플레이어의 `CurrentRooms`를 본다. 방이 정확히 하나면 그 방의 `RoomAmbientLight`·`RoomFog`를, 여럿이면 하나라도 null인 경우 null을 목표로 잡고, `Level` 기본값과 사이를 `(0.05 + 카메라 속도 * 0.2) * RoomTransitionSpeedMultiplier` 속도로 보간한다. 안개 끝 거리는 곧 `MainCamera.farClipPlane = FogEndDistance + 1`이 되어 사실상 거리 컬링을 맡는다. 손전등이 꺼져 있으면 `AmbientColor`→`AmbientColorAdaptation`으로 암순응한다. `AmbienceLoop`·`AmbienceBreakers`·`ReverbDirector`도 `CurrentRooms[0]`의 `RoomAmbience`·`RoomAmbienceOverride`·`ReverbPreset`을 읽는다.

`LightManager`는 방 단위가 아니다. "Prop Lights"·"Prop Emission" 태그 오브젝트를 모아 두고, 대상이 `checkDistance` 이상 움직이거나 20° 이상 돌 때 0.5초 간격으로 `GraphicsManager.lightDistance * lightRangeMultiplier` 거리 기준으로 페이드 온오프한다. 대상 뒤쪽(내적 ≤ -0.25)이면 0.8배 거리에서 끈다. 모듈이나 방을 통째로 비활성화하는 컬링·오클루전 코드는 없다(`CullingGroup`·`Occlusion` grep 결과 없음).

## 미니맵

`Map`은 월드를 `Scale = 0.1`로 축소해 그리는 3D 씬 미니맵이다. 층은 `LayerHeight = 4f`로 `GetLayerParent(y) = floor((y + 0.1) / 4)`가 필요할 때마다 `MapLayer` 오브젝트를 만든다. `Update`에서 플레이어 층은 y = 0, 한 층 아래·위는 `GetLayerPosition(2)`·`(3)` 오프셋, 나머지는 y = -5로 내려 숨긴다. 즉 **층 개념이 있고 4 m 단위**다. `OverLayerParent`는 층과 무관하게 항상 위에 그리는 부모다.

그리는 요소는 모두 모듈 프리팹 안에 손으로 놓은 마커다. `DirtFinderMapFloor`(`FloorType` 15종: 1x1·1x0.5·1x0.25와 대각·곡선·역곡선, 트럭 바닥·벽, Used, Inactive)와 `DirtFinderMapWall`(`WallType` 20종: 벽·문·막힌 문·테마별 문·계단)이 `Generated` 뒤 `Map.AddFloor/AddWall`로 해당 층에 복제되고, `DirtFinderMapDoor`는 실제 문의 힌지 회전을 따라간다. 미탐색 방은 `RoomVolume.Setup`이 만든 `MapModule` 아래의 덩어리(`RoomVolume` 프리팹 + `RoomVolumeOutline`)로 표시되고, 플레이어가 그 방을 밟으면 `MapModule.Hide()`가 스케일 곡선 애니메이션 뒤 파괴한다. 덩어리는 `OverLayerParent` 아래에 있고 바닥·벽 마커는 층 아래에 있으므로, 덩어리가 사라지면서 그 자리의 마커가 드러나는 방식이다(추정). `MapModule.Update`는 덩어리를 플레이어 방향으로 돌린다. 발견 단위는 **모듈이 아니라 `RoomVolume`(방)**이다.

그 위에 `MapCustom`(임의 오브젝트, `sprite`·`color`, 다른 층이면 알파 0.3)·`MapValuable`(귀중품, `Medium` 이하는 작은 스프라이트)·`MapBacktrackPoint`(경로 점)가 올라간다. `Map.AddEnemy`·`EnemyPositionSet`은 빈 함수라 적은 표시되지 않는다. 상점·아레나·로비에서는 모든 추가가 무시된다. 표시는 `MapToolController`가 `InputKey.Map` 홀드 또는 토글 시(달리는 중에는 불가) `Map.Instance.ActiveSet(true)`로 켠다. `MapBacktrack`은 맵이 켜져 있을 때 플레이어의 `LastNavmeshPosition`에서 현재 목표(활성 추출 지점, 모두 끝났으면 트럭)까지 `NavMesh.CalculatePath`를 계산해 `spacing` 간격으로 점을 찍고, 같은 층이면서 10 m 이내면 생략한다. 추출 지점의 순서는 생성기가 정하지 않는다. 플레이어가 어느 추출 지점의 버튼을 누르든 `RoundDirector.ExtractionPointActivateRPC`가 그것을 `extractionPointCurrent`로 잡고 나머지를 잠근다.

## 레벨 종류와 순서

`RunManager`는 `Level` 에셋 참조를 종류별로 나눠 가진다: 단일 `levelLobby`(트럭 안), `levelLobbyMenu`, `levelMainMenu`, `levelRecording`, `levelSplashScreen`, `levelTutorial`, 그리고 리스트 `levelShop`, `levelArena`, `levels`(실제 스테이지 테마들). `SemiFunc.RunIsLevel()`은 상점·로비·메뉴·스플래시·튜토리얼·아레나가 아닐 때만 true다. 상점·로비도 같은 `LevelGenerator`가 만든다(상점은 `DeadEndAmount = 1`, `ModuleAmount`는 에셋 값 그대로 — `ModuleAmount ≤ 4`로 추정).

`ChangeLevel(_completedLevel, _levelFailed, type)`의 기본 흐름은 이렇다. 메인 메뉴·로비 메뉴에서는 `levelLobby`로, 로비에서는 `SetRunLevel()`(직전 스테이지와 다른 것이 나올 때까지 `levels`에서 무작위)로, 스테이지를 완료하면 `levelsCompleted++`·`LevelSuccessful()` 뒤 `levelShop`에서 무작위로, 상점에서는 다시 로비로 간다. 실패(전멸)하면 아레나로 가고(멀티면 `levelArena` 무작위, 싱글이면 `[0]`), 아레나에서도 실패하면 `ResetProgress()`로 `levelsCompleted = 0`이 되며 게임오버다. `levelsMax = 10`은 코드 기본값이고 생성기에서는 쓰이지 않는다. 문(moon) 단계는 `CalculateMoonLevel = (levelsCompleted + 1) / 5`다.

`levelsCompleted`는 세 개의 정규화 난이도로 바뀌어 생성 전반에 들어간다: `RunGetDifficultyMultiplier1 = clamp01(levelsCompleted / 9)`, `...2 = clamp01((levelsCompleted - 9) / 10)`, `...3 = clamp01((levelsCompleted - 19) / 10)`. 1은 모듈 티어 가중치 곡선, 적 수량 곡선(`amountCurve1_1..3_1`), 귀중품 수량·총액 곡선, 목표 금액 곡선(`haulGoalCurve1`)에 쓰이고, 2·3은 10판째·20판째 이후의 두 번째 곡선 세트로 넘어간다. 모듈 개수 자체는 곡선이 아니라 위의 정수 공식이다.

## 네트워크

시드는 공유하지 않는다. 마스터가 격자를 계산한 뒤 시작방·모듈·연결물·차단물·귀중품·적을 전부 `PhotonNetwork.InstantiateRoomObject`로 만들므로 룸 소유 오브젝트로서 모든 클라이언트에 같은 위치·회전으로 복제된다. 모듈 프리팹 안의 소품·`LevelPoint`·`RoomVolume`·미니맵 마커는 프리팹의 일부라 별도 동기화가 필요 없다. 소켓 연결 플래그만 `ModuleConnectionSetRPC`(`RpcTarget.All`)로 보내고, 각 클라이언트가 그 플래그로 `ModulePropSwitch.Setup()`을 로컬 실행한다. 귀중품 스위치도 `VolumesAndSwitchSetupRPC`(All)로 각자 실행한다. 네브메시는 각자 빌드한다. 단계 게이트는 카운트 RPC다: `ModulesReadyRPC`, `PlayerSpawnedRPC`, `EnemyReadyRPC`는 보낸 `Player`를 목록에 넣어 인원수와 비교하고, `ModuleAmountRPC`·`EnemySpawnTargetRPC`·`NavMeshSetupRPC`·`ItemSetup`·`GenerateDone`은 `AllBuffered`다. 모든 수신 RPC는 `SemiFunc.MasterOnlyRPC(info)`로 보낸 이가 마스터인지 확인한다. 레벨 선택은 `RunManagerPUN.UpdateLevelRPC(levelName, levelsCompleted, gameOver)`(`OthersBuffered`)로 **에셋 이름 문자열**을 보내고, 수신 측이 `levelShop`→`levelArena`→고정 레벨→`levels` 순으로 이름을 찾아 `levelCurrent`를 맞춘다. 씬 전환은 마스터가 `PhotonNetwork.LoadLevel("Reload")`로 한다. 클라이언트는 `Generated`가 될 때까지 `Generate()` 안에서 그냥 기다린다.

## 수치

| 항목 | 값 | 출처 |
|---|---|---|
| `TileSize` | 5 m | `LevelGenerator` static |
| `ModuleWidth` / `ModuleHeight` | 3 / 1 (타일 단위) → 모듈 한 변 15 m | `LevelGenerator` static |
| `LevelWidth` / `LevelHeight` | 코드 기본값 3 / 3, 실제 에셋 값 미확인 | 직렬화 필드 |
| 모듈 위치 | `((x - W/2) * 15, 0, y * 15 + 7.5)` | `ModuleGeneration` |
| 시작 타일 | `(W/2, 0)`, 시작방은 원점, 트럭 가상 타일 `(W/2, -1)` | `TileGeneration` |
| 워크 모듈 수 | `min(5 + n, 10)`; n ≥ 10이면 `+ min(n - 9, 5)` (n = `levelsCompleted`) → 5~15 | `TileGeneration` |
| 막다른 수 | `floor(ModuleAmount / 3)` (정수 나눗셈), 상점은 1 | `TileGeneration` |
| 추출 지점 수 | ≥15: 4, ≥10: 3, ≥8: 2, ≥6: 1, 그 외 0 | `TileGeneration` |
| 특수 수 | `Level.ModulesSpecialAmount` | 에셋 |
| 통로 최대 | `Level.PassageMaxAmount` 코드 기본값 2, 방향 뒤집기 50% | `ModuleGeneration` |
| 회전 후보 | 0°, 90°, 180°, 270° (Normal은 무작위, 부속은 이웃 방향) | `ModuleRotations` |
| 티어 가중치 | `DifficultyCurve1..3.Evaluate(clamp01(n / 9))` | `PickModule` |
| 난이도 정규화 | D1 = n/9, D2 = (n-9)/10, D3 = (n-19)/10, 각 clamp01 | `SemiFunc` |
| 문 단계 | `(n + 1) / 5` | `RunManager` |
| 소켓 표식 | 변 중앙 ±1 m | `Module.OnDrawGizmos` |
| `LevelPoint` 자동 연결 | 15 m 이내, 마주봄 내적 ≤ -0.8, 정면 내적 > 0.8 | `ModuleConnectSetup` |
| 네브메시 점 검증 | `SamplePosition` 반경 0.5 m | `LevelPoint.NavMeshCheck` |
| 방 검사 주기 | 플레이어 0.1초, 그 외 0.5초 | `RoomVolumeCheck` |
| 정찰 점수 | `100 * (트럭 안전 지점 거리 / 20)` per 새 방 | `RoomVolumeCheck` |
| 방 전환 속도 | `(0.05 + 카메라 속도 * 0.2) * RoomTransitionSpeedMultiplier` | `EnvironmentDirector` |
| 안개 기본 | 0 ~ 15 m, `farClipPlane = FogEnd + 1` | `Level`, `EnvironmentDirector` |
| 미니맵 축척 / 층 높이 | 0.1 / 4 m | `Map` |
| 미니맵 길 안내 생략 | 같은 층이면서 10 m 이내 | `MapBacktrack` |
| 조명 컬링 | `lightDistance * lightRangeMultiplier`, 뒤쪽은 0.8배 | `LightManager` |
| 적 초기 스폰 | 트럭에서 가장 먼 `LevelPoint` 한 곳에 전부 | `EnemySetup` |
| 적 재등장 밴드(대표) | 플레이어에서 30~50 m | 각 `Enemy*` |
| 귀중품 가치 환산 | `dollarValueCurrent * 0.001`을 `totalMaxValue`와 비교 | `ValuableDirector` |
| 목표 금액 | `haulGoalMax * 0.7 * haulGoalCurve` | `RoundDirector` |

## CL9 시사점

결정을 내리지 않고 관찰만 적는다. CL9의 미결 사항과 짝지어 본다.

- **소켓을 변의 어디에 둘지.** REPO는 변당 소켓 하나, 변의 정중앙이다. 기즈모 표식이 ±1 m이고 `ConnectObject`·`BlockObject`가 정확히 변 중앙 좌표에 스폰된다. 모듈이 정사각이고 4회전이 모두 허용되므로 중앙 이외의 위치는 회전 시 짝이 안 맞는다. CL9의 칸 500 cm 격자에서 모듈 한 변이 몇 칸이든, "변 중앙 한 곳"이면 회전 대칭이 자동으로 성립한다는 점이 관찰된다.
- **높이차 전파.** REPO에는 높이차 전파가 없다. 모든 모듈은 y = 0에 놓이고 회전은 Y축뿐이다. 높이는 모듈 내부(계단·발코니)에서만 생기고, 미니맵이 4 m 단위 층으로 그것을 표현한다. CL9의 "층 개념 없이 높이는 모듈 안 계단" 결정과 같은 구조다.
- **입구 개수.** 타입이 곧 입구 개수다. Normal은 4방 모두 열릴 수 있어야 하고(회전이 무작위이므로) 쓰지 않는 변만 `BlockObject`로 막는다. Passage는 마주보는 2변, DeadEnd·Extraction·Special은 아래(-Z) 1변이다. 열림·닫힘의 시각적 차이는 모듈 안 `ModulePropSwitch`가 결정적으로 처리한다. 입구 수를 모듈마다 다르게 두는 대신 "타입 5개 × 사방 개폐"로 단순화한 셈이다.
- **몬스터 스폰 지점.** REPO는 몬스터 전용 스폰 액터를 두지 않는다. 모듈 안의 `LevelPoint` 웨이포인트 그래프가 스폰(트럭에서 가장 먼 점)·로밍·재등장(플레이어와의 거리 밴드)·방 판정을 전부 겸한다. 각 점이 `RoomVolume`에 귀속되어 "플레이어가 있는 방의 점"을 바로 고를 수 있다. CL9에 `LevelPoint` 해당물이 없다는 미결에 대해, REPO의 답은 "모듈 표 행에 웨이포인트 목록을 두고 모듈 경계에서 마주보는 점끼리 런타임에 잇는다"이다.
- **일자별 크기 곡선.** REPO는 곡선이 아니라 정수 공식이다: 워크 모듈 5 + 완료 수, 상한 10, 11판째부터 15까지. 부속으로 막다른 floor(n/3), 추출 0~4, 특수 고정 수가 더해져 총 모듈은 대략 7~25다. CL9의 "한 판 20~30모듈"은 REPO 최대치 부근에 해당한다. 플레이어 수는 크기에 반영되지 않는다.
- **추출 지점 위치.** 생성기가 farthest-point sampling으로 트럭과 서로에게서 가장 먼 후보 타일에 놓고, 활성화 순서는 플레이어가 고른다. 트럭이 격자 밖 고정 위치라는 점은 CL9 결정과 같다.
- **실패 처리.** 조립 실패라는 개념이 없다. 모든 Normal 모듈이 4방 호환이므로 어떤 배치도 성립하고, 재시도 코드가 없다. 대신 격자가 워크 수를 못 담으면 무한 루프가 되므로 격자 크기가 에셋에서 충분히 크게 잡혀 있어야 한다.
- **모듈 선택.** 타입별로 난이도 티어 3개의 셔플 가방을 두고, 티어 가중치 추첨 + "가장 덜 소진된 가방 우선" 규칙으로 같은 모듈의 반복을 늦춘다. CL9의 "모듈은 표의 행"이라면 티어 열 하나와 가방 인덱스만으로 같은 규칙을 만들 수 있다.
- **소품 교체.** 세 종류가 모두 확률 없이 상태에서 결정된다(소켓 연결, 귀중품 유무, 트랩 유무). 판마다 달라 보이는 개성은 소켓 연결 상태의 조합에서 나온다.
- **방 볼륨의 역할 범위.** 방 박스 하나가 앰비언스·리버브·안개·환경광·탐색 점수·미니맵 발견·적의 방 판정·트럭/추출 안 판정을 모두 맡는다. 렌더 컬링은 안개 거리(`farClipPlane`)와 조명 거리 페이드로만 하고, 모듈 단위 컬링은 없다.
- **네트워크.** 시드 공유 대신 호스트가 결과를 네트워크 인스턴스로 뿌리고, 프리팹 내부 상태는 플래그 RPC 하나로 각자 재현한다. CL9의 리슨 서버(호스트 판정)와 같은 권한 구조이고, 모듈이 표의 행이라면 "행 ID + 격자 좌표 + 회전 + 4방 플래그"만 복제하면 같은 효과가 난다는 점이 관찰된다.
