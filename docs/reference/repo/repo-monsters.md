# 참고 — R.E.P.O.의 몬스터와 함정

> 2026-09-08, 설치본의 `Assembly-CSharp.dll`을 디컴파일해 읽은 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](../README.md).
> CL9의 대응 문서는 [../data/monster/common.md](../../plan/data/monster/common.md) ·
> [../data/progression.md](../../plan/data/progression.md)다.

R.E.P.O.의 몬스터는 개체마다 완전히 다른 AI를 짜는 대신, `Enemy`라는 공통 허브에 상태 컴포넌트(`EnemyState*`)와 감각 컴포넌트(`EnemyVision`, `EnemyPlayerDistance`, `EnemyOnScreen`, `EnemyPlayerRoom`)를 붙여 조립하는 구조를 쓴다. 판정은 전부 마스터 클라이언트가 하고, 다른 클라이언트는 위치와 상태를 받아 보간만 한다. 몬스터는 항상 맵에 살아 있지 않고, `EnemyParent`가 스폰/디스폰 타이머를 돌려 주기적으로 사라졌다가 플레이어에게서 먼 `LevelPoint`에 다시 나타난다. 레벨마다 몇 마리를 어느 난이도로 뽑을지는 `EnemyDirector`가 클리어한 레벨 수를 0~1로 정규화한 값을 AnimationCurve에 넣어 결정한다.

이 문서는 디컴파일된 C# 소스를 직접 읽고 정리한 것이다. 코드에서 확인한 값은 그대로 적었고, 프리팹·ScriptableObject 인스펙터에만 있는 값은 "코드에 없음"으로 표시했다. 추측한 부분에는 "(추정)"을 붙였다.

## 공통 뼈대

### 계층 구조

몬스터 하나는 두 단계의 게임오브젝트로 이루어진다.

- `EnemyParent` — 스폰/디스폰의 주인. 레벨 생성 시 `LevelGenerator.Instance.EnemyParent` 아래로 붙는다. `EnableObject`를 켜고 끄는 방식으로 몬스터 본체를 활성/비활성한다. 난이도 등급(`Difficulty1/2/3`), 표시 이름(`enemyName`, 코드 기본값 `"Dinosaur"`), 그리고 `SpawnedTimeMin/Max`·`DespawnedTimeMin/Max`를 갖는다.
- `Enemy` — 실제 AI 허브. `PhotonView`를 갖고 `IPunObservable`을 구현한다. `Awake`에서 `GetComponent`로 옵션 컴포넌트를 전부 훑어 `HasVision`, `HasRigidbody`, `HasStateChase` 같은 플래그를 세운다. 즉 **어떤 능력을 갖느냐는 프리팹에 어떤 컴포넌트를 붙였느냐로 정해진다.**

`EnemyChecklist`는 개발용 체크리스트 컴포넌트로, 새 몬스터를 만들 때 빠뜨리기 쉬운 설정(중심 트랜스폼, 난이도, 타입, 체력 최대치, 사망 이벤트, 리지드바디 프리셋 등)을 bool 목록으로 나열해 둔 것이다. 게임 로직에는 관여하지 않는다.

### 상태 머신

상태는 **모든 몬스터가 공유하는 단일 enum** `EnemyState`다.

```
None, Spawn, Roaming, ChaseBegin, Chase, ChaseSlow, ChaseEnd,
Investigate, Sneak, Stunned, LookUnder, Despawn
```

`Enemy.CurrentState`가 유일한 진실이고, 각 `EnemyState*` 컴포넌트는 자기 `Update`에서 `Enemy.CurrentState`가 자기 상태인지 확인한 뒤에야 동작한다. 상태 전환은 그냥 `Enemy.CurrentState = EnemyState.Chase`처럼 필드에 대입하는 것이다. 별도의 전환 테이블이나 상태 오브젝트 스택이 없다.

중요한 점은, **개별 몬스터는 여기에 더해 자기만의 `State` enum을 하나 더 갖는다**는 것이다(예: `EnemyBang.State`). 즉 이중 상태 머신이다. 공통 `EnemyState`는 "지금 배회 중인가 추격 중인가"라는 디렉터 수준의 상태를 나타내고, 몬스터 자신의 enum은 "지금 공격 모션 어느 단계인가, 도화선이 몇 초 남았나" 같은 연기(演技) 수준의 상태를 나타낸다. 몬스터의 `Update`는 보통 `Enemy.CurrentState`를 읽어 자기 enum을 갱신하는 식으로 두 층을 잇는다.

공통 상태 컴포넌트를 하나씩 보면 다음과 같다.

| 컴포넌트 | 하는 일 | 코드에 있는 값 |
|---|---|---|
| `EnemyStateSpawn` | 스폰 직후 대기했다가 `Roaming`으로. `OnSpawn` UnityEvent 발화 | 대기 `WaitTimer = 2f` (하드코딩) |
| `EnemyStateRoaming` | `LevelPoint`를 골라 배회. 플레이어 근접/원거리 반응, 화면에 잡히면 방향 전환 | `Speed`/`Acceleration` 등 대부분 프리팹 값 |
| `EnemyStateInvestigate` | 지정 좌표로 이동. 소리·충격에 반응하는 창구 | `rangeMultiplier = 1f`, `PhysObjectHitMax = 3`, `OnlyEvent = true` |
| `EnemyStateSneak` | 목표 플레이어 쪽으로 조용히 접근. 시야에 걸리면 타이머 리셋 | `StateTimeMin/Max` 프리팹 값 |
| `EnemyStateChaseBegin` | 멈춰서 플레이어를 바라보고 연출을 재생한 뒤 `Chase`로 | 카메라 흔들림 `Shake(5f, 0.25f)`, `Shake(3f, 0.5f)` |
| `EnemyStateChase` | 본격 추격. 시야가 끊기면 마지막 위치로 | `VisionTime`, `VisionsToReset` 프리팹 값. 못 닿는 상태 2초면 `ChaseSlow` |
| `EnemyStateChaseSlow` | 놓친 뒤 `GetLevelPointAhead`로 진행 방향의 다음 지점을 계속 좇음 | `StateTimeMin/Max` 프리팹 값 |
| `EnemyStateChaseEnd` | 제자리에 서서 잠깐 뜸 들이고 `Roaming` | 속도 0, 가속 5 고정 |
| `EnemyStateLookUnder` | 기어들어간(crawling) 플레이어의 마지막 내비메시 좌표를 들여다봄 | `WaitTimerMin/Max`, `LookTimerMin/Max` 프리팹 값 |
| `EnemyStateStunned` | 스턴. 리지드바디 추종을 끊고 드래그를 0.05로 낮춰 널브러지게 함 | `DisableChase(0.25f)` 매 프레임 |
| `EnemyStateDespawn` | 사라지기. 막힘 카운트·추격 지속으로도 트리거됨 | `StuckDespawnCount = 10`, `DespawnAfterChaseTime = 10f` |

`EnemyStateChase`의 흐름이 이 게임 추격의 핵심이다. 시야가 살아 있으면 `TargetPlayerAvatar.LastNavmeshPosition`을 목적지로 잡는다(플레이어의 실제 위치가 아니라 **플레이어가 마지막으로 내비메시 위에 있던 좌표**다). 경로의 끝점이 플레이어에게서 0.5m 넘게 떨어져 있으면 `ChaseCanReach = false`가 되고, 그 상태가 2초 지속되면 시야 카운터를 0으로 밀고 `ChaseSlow`로 내려간다. 즉 **플레이어가 몬스터가 갈 수 없는 곳(기어들어간 틈, 올라간 선반)에 있으면 추격은 자연히 풀린다.**

## 감지

### 시야 — `EnemyVision`

시야 판정은 0.25초마다 도는 코루틴이고, 멀티플레이에서는 마스터 클라이언트만 돌린다. 코드 기본값은 다음과 같다.

| 항목 | 필드 | 코드 기본값 |
|---|---|---|
| 기본 시야 거리 | `VisionDistance` | 10 |
| 근접 자동 발각 거리(서 있을 때) | `VisionDistanceClose` | 3.5 |
| 근접 자동 발각 거리(앉았을 때) | `VisionDistanceCloseCrouch` | 2 |
| 시야 원뿔 dot — 서 있음 | `VisionDotStanding` | 0.4 |
| 시야 원뿔 dot — 앉음 | `VisionDotCrouch` | 0.6 |
| 시야 원뿔 dot — 기어감 | `VisionDotCrawl` | 0.9 |
| 발각까지 필요한 누적 횟수 — 서 있음 | `VisionsToTrigger` | 4 |
| 발각까지 필요한 누적 횟수 — 앉음 | `VisionsToTriggerCrouch` | 10 |
| 발각까지 필요한 누적 횟수 — 기어감 | `VisionsToTriggerCrawl` | 20 |
| 들고 있는 물건 감지 반경 | `PhysObjectVisionRadius` | 10 (private, `PhysObjectVisionRadiusOverride`로 덮어씀) |
| 들고 있는 물건 감지 dot | `PhysObjectVisionDot` | 0.4 |

설계가 깔끔하다. 자세(서기/앉기/기기)가 **원뿔의 좁기**와 **누적 횟수**를 동시에 바꾼다. 서 있으면 dot 0.4(넓은 원뿔)에 4회 연속이면 걸리니 0.25초 × 4 = 1초 만에 발각된다. 기어가면 dot 0.9(거의 정면만)에 20회가 필요하니 5초를 정면에 노출돼야 걸린다. 다만 **근접 거리 안(서 있으면 3.5m, 앉으면 2m)에 들어가면 각도와 무관하게 즉시 `VisionsTriggered`가 `VisionsToTrigger`로 채워져 바로 발각된다.** 넘어져 있는(`isTumbling`) 플레이어는 앉은 것으로 취급한다.

`PhysObjectVision`은 별개의 감지 채널이다. 반경 10m 안에서 **플레이어가 들고 있는 물건**을 찾아, 그 물건이 시야 원뿔(dot 0.4) 안에 있고 가려지지 않았으면 그 물건을 든 플레이어를 발각 처리한다. 플레이어 자신은 벽 뒤에 숨어 있어도 손에 든 짐이 삐져나와 있으면 걸린다는 뜻이다.

`DisableVision(time)`으로 일시 무력화할 수 있고, `StandOverride(time)`는 앉거나 기고 있어도 서 있는 것으로 취급하게 만든다.

### 소리 — `EnemyDirector.SetInvestigate`

소리는 별도의 전파 시뮬레이션이 아니라 **"이 좌표를 반경 R 안의 몬스터들이 조사하라"는 브로드캐스트 한 줄**이다.

```
EnemyDirector.SetInvestigate(Vector3 position, float radius, bool pathfindOnly = false)
```

마스터 클라이언트에서만 실행되고, `enemiesSpawned` 전부를 훑는다. 스폰돼 있지 않은 몬스터라면 반경이 15 이상일 때 `DisableDecrease(5f)`로 **재등장 타이머를 5초 앞당긴다**(큰 소음이 나면 잠들어 있던 몬스터가 더 빨리 깬다). 스폰돼 있으면 거리를 `StateInvestigate.rangeMultiplier`로 나눈 값이 반경보다 작을 때 `StateInvestigate.Set(position, pathfindOnly)`를 호출한다. 즉 `rangeMultiplier`가 큰 몬스터일수록 귀가 밝다.

코드에서 확인한 소음원과 반경은 다음과 같다.

| 소음원 | 호출 위치 | 반경 |
|---|---|---|
| 무거운 발소리 | `PlayerAvatar.Footstep` (`SoundType.Heavy`) | 5 |
| 중간 발소리 | `PlayerAvatar.Footstep` (`SoundType.Medium`) | 1 |
| 착지 | `PlayerAvatar` 착지 처리 | 10 |
| 보이스챗(말하기) | `PlayerVoiceChat` | 5 (재호출 쿨다운 1초) |
| 물건 파괴 | `PhysGrabObjectImpactDetector.DestroyObject` | 15 |
| 물건 충돌 — 대 | `PhysGrabObjectImpactDetector` | 10 |
| 물건 충돌 — 중 | 〃 | 5 |
| 물건 충돌 — 소 | 〃 | 3 |
| 물건이 바닥에 놓임/작은 접촉 | 〃 | 1 / 0.5 / 0.2 |
| 경첩(문) 여닫기 | `PhysGrabHinge` | 0.5 ~ 1, 세게 열면 `investigateRadius` |
| 총 | `ItemGun` | `investigateRadius` (프리팹 값) |
| 차량 | `ItemVehicle` | 35 |
| 함정 | `Trap` | `enemyInvestigateRange` (프리팹 값) |
| 추출 지점 작동 | `ExtractionPoint` | 20 |
| 비명 인형 | `ScreamDollValuable` | 20 |
| 음악 상자류 귀중품 | `MusicalValuableLogic` | 25 |

가벼운 발소리(`SoundType.Light`)에는 아예 호출이 없다. 즉 **앉아 걷기가 조용한 것은 별도 스텔스 수치가 아니라 발소리의 재질 등급이 낮아 investigate 호출 자체가 나가지 않기 때문**이다. 또 `PlayerAvatar.OverrideDisableEnemyInvestigate(_time)`로 특정 시간 동안 자기 소음을 무시시킬 수 있다.

### 물건 충격과 근접 — `EnemyTriggerAttack`

`EnemyTriggerAttack`는 몬스터 주위의 트리거 콜라이더로, 두 가지 일을 한다. 첫째, `Chase`나 `LookUnder` 상태에서 플레이어 트리거가 들어오면 시야 레이캐스트로 가림을 확인한 뒤 `Attack = true`를 세운다(실제 공격 실행은 몬스터별 스크립트가 이 플래그를 읽어서 한다). 둘째, **플레이어가 잡고 있는 물건**(`PhysGrabObject`/`StaticGrabObject`)이 시야 거리 안에 있고 dot 0.8 이상 정면이며 가려지지 않았으면 `StateInvestigate.Set(그 물건 위치)`를 호출한다. 물건을 든 채로 몬스터 정면에 있으면 조사하러 온다는 뜻이다.

### 방 — `EnemyPlayerRoom`, `RoomVolume`

`EnemyPlayerRoom`은 1초마다 `RoomVolumeCheck.CurrentRooms`를 플레이어들의 방 목록과 대조해 `SameAny`(아무 플레이어와 같은 방), `SameLocal`(로컬 플레이어와 같은 방)을 갱신한다. 이 값은 AI 판단보다는 **연출 범위 결정**(추격 시작 화면 흔들림을 로컬에서 재생할지)과 `EnemyDirector`의 행동량 누적에 쓰인다.

`EnemyParent.GetRoomVolume`은 1초마다 몬스터 중심에 1×1×1 `OverlapBox`를 쳐서 현재 방을 갱신한다.

### 거리와 화면

`EnemyPlayerDistance`는 0.25초마다 모든 플레이어와의 거리를 재서 `PlayerDistanceLocal`(로컬 플레이어)과 `PlayerDistanceClosest`(가장 가까운 살아 있는 플레이어)를 남긴다. 초기값은 1000f.

`EnemyOnScreen`은 0.25초마다 로컬 카메라의 프러스텀 안에 몬스터의 `points`가 들어오는지, 그리고 가려지지 않았는지(최대 12m까지 레이캐스트)를 판정해 `OnScreenLocal`/`CulledLocal`을 만들고, 멀티플레이에서는 **각 클라이언트가 자기 화면 판정을 RPC로 올려** `OnScreenPlayer`/`CulledPlayer` 딕셔너리를 채운다. 코드 기본값은 `maxDistance = 20f`, 여백 `paddingWidth/Height = 0.1f`. 이것이 "플레이어가 보고 있으면 몬스터가 다르게 행동한다"의 근거가 된다.

## 이동과 재배치

### 내비메시

`EnemyNavMeshAgent`는 유니티 `NavMeshAgent`의 얇은 래퍼다. 멀티플레이에서는 마스터 클라이언트에서만 `Agent.enabled = true`다. 각 상태 컴포넌트가 매 프레임 `UpdateAgent(Speed, Acceleration)`을 호출해 속도를 덮어쓰는 방식이라, **속도는 상태별로 프리팹에 따로 설정**된다. `Warp`, `Disable(time)`, `Stop(time)`, `OverrideAgent(speed, accel, time)` 같은 조작이 있고, `GetPoint()`는 현재 경로의 마지막 코너를 돌려줘 "목표에 닿을 수 있는가" 판정에 쓰인다.

### 목적지 선택 — `LevelPoint`

레벨 생성기가 배치한 `LevelPoint` 목록(`LevelGenerator.Instance.LevelPathPoints`)이 몬스터의 이동 그래프다. 각 `LevelPoint`는 `ConnectedPoints`로 이웃과 연결돼 있고, `Room`(어느 방인지), `Truck`(트럭 방인지), `inStartRoom` 플래그를 갖는다.

`EnemyStateRoaming.PickPath`의 규칙:

1. `SemiFunc.EnemySpawnIdlePause()`가 참이거나 이미 경로가 있으면 아무것도 하지 않는다.
2. `RoamingChangeCurrent`가 남아 있으면 **직전에 고른 `LevelPoint`를 계속 쓴다**(같은 구역을 몇 번 더 돈다). 소진되면 새로 고른다.
3. 새로 고를 때, 플레이어를 피해야 하는 상황(`PlayerFarMove`)이면 `SemiFunc.LevelPointGetFurthestFromPlayer(위치, 5f)`, 아니면 전체 `LevelPathPoints`에서 완전 무작위로 하나.
4. 고른 지점에 `Random.insideUnitSphere * Random.Range(RoamingPathRadiusMin, RoamingPathRadiusMax)`만큼 흔들림을 주고, 경로가 `PathComplete`일 때만 목적지로 확정한다.

`Enemy.GetLevelPointAhead(currentTargetPosition)`는 추격을 놓친 뒤 쓰는 함수다. 목표 좌표에 가장 가까운 `LevelPoint`를 찾고, 그 이웃들 중 **몬스터→목표 방향과 dot이 가장 큰** 이웃을 돌려준다. 즉 플레이어가 도망친 방향으로 통로를 따라 한 칸 더 밀고 들어간다. `ChaseSlow`가 이 함수를 반복 호출한다.

### 플레이어 근접 회피

`EnemyStateRoaming`에는 "플레이어에게 너무 오래 붙어 있으면 일부러 멀어진다"는 로직이 있다. `PlayerDistanceClosest <= PlayerNearDistance`인 동안 `PlayerNearTimer`가 쌓이고(멀어지면 `PlayerNearDecrease` 배속으로 감소), `PlayerNearTimeMax`를 넘으면 `PlayerFarMove`가 켜져 `PlayerFarTimeMin~Max` 동안 플레이어에게서 가장 먼 지점으로 이동한다. 배회하는 몬스터가 플레이어 주변을 계속 맴돌지 않게 하는 장치다.

`EnemyDirector`가 `forceLeave`를 세우면(아래 "행동량" 참고) `SemiFunc.EnemyForceLeave`가 참이 되어 같은 이탈 동작을 즉시 시작한다.

### 화면 안에서의 방향 전환

`EnemyStateRoaming.PlayerTurn`은 몬스터가 `RoamingOnScreenTime` 이상 누군가의 화면에 잡혀 있으면, `RoamingTurnWaitTime`만큼 멈춰 있다가 `GetLevelPointAhead(플레이어 위치)`로 목적지를 바꾼다. 그 뒤 `RoamingOnScreenCooldown` 동안은 다시 걸리지 않는다. **플레이어가 보고 있으면 그쪽으로 방향을 튼다** — 위협적으로 읽히게 만드는 연출성 이동이다.

### 스폰과 디스폰 사이클

`EnemyParent.Logic()` 코루틴이 이 사이클의 전부다.

- 게임 시작 시 일단 `Despawn()`으로 시작하고 `DespawnedTimer = Random.Range(2f, 5f)`.
- 스폰 상태라면: `SpawnedTimer`가 0 이하이고 **플레이어가 가깝지 않으면**(`!playerClose`) `Enemy.CurrentState = Despawn`. 플레이어가 가까우면 타이머가 아예 줄지 않는다. 즉 **플레이어 눈앞에서 사라지지 않는다.**
- 디스폰 상태라면: `DespawnedTimer`가 0 이하가 되면 `Spawn()`.

`Spawn()`은 `SpawnedTimer = Random.Range(SpawnedTimeMin, SpawnedTimeMax)`로 채우고 상태를 `Spawn`으로 만든 뒤 전원에게 `SpawnRPC`. `SpawnRPC`는 체력을 최대로 되돌리고(`Health.OnSpawn()`), 스턴을 풀고, 막힘 카운트를 0으로 만들고, `EnableObject`를 켠다. 그리고 **`SpawnedTimerPause(Random.Range(3f, 4f) * 60f)`** — 즉 스폰 직후 3~4분 동안은 스폰 타이머가 흐르지 않는다(막 나타난 몬스터가 금방 사라지지 않게).

`Despawn()`의 디스폰 시간 계산은 다음과 같다.

```
DespawnedTimer = Random.Range(DespawnedTimeMin, DespawnedTimeMax)
               * EnemyDirector.despawnedDecreaseMultiplier
               * EnemyDirector.despawnedTimeMultiplier;
DespawnedTimer = Mathf.Max(DespawnedTimer, 1f);
```

죽은 상태로 디스폰하면 `DespawnedTimer *= 3f`가 추가로 붙는다. 즉 **몬스터를 죽여도 영구히 사라지는 게 아니라 재등장이 3배로 늦어질 뿐이다.**

`playerClose`/`playerVeryClose`는 `PlayerCloseLogic()` 코루틴이 1초마다 갱신한다. 수평 거리(y 무시) 기준으로 **20m 이내면 `playerClose`, 6m 이내면 `playerVeryClose`**이며, 살아 있는 플레이어뿐 아니라 `GameDirector.instance.PlayerDeathSpots`(시체 위치)도 같은 기준으로 센다. `playerClose`가 참인 동안 `EnemyDirector.spawnIdlePauseTimer`가 0으로 리셋되고, `valuableSpawnTimer = 10f`가 채워진다.

### 텔레포트

`Enemy.TeleportToPoint(minDistance, maxDistance)`의 규칙:

1. `EnemyParent.firstSpawnPointUsed`가 아직 거짓이면 **첫 스폰 지점**(`firstSpawnPoint`)을 쓴다.
2. 모든 추출이 끝난 상태(`RoundDirector.allExtractionPointsCompleted`)라면 먼저 시작 방 안에서만 찾는다 — `SemiFunc.LevelPointGetPlayerDistance(pos, min, max, _startRoomOnly: true)`.
3. 실패하면 시작 방 제한 없이 `LevelPointGetPlayerDistance(pos, min, max)`.

`LevelPointsGetPlayerDistance`는 "**가장 가까운 플레이어까지 거리가 `minDistance`보다 크고, 어떤 플레이어든 `maxDistance` 안에 있는**" 지점만 모아 그중 하나를 무작위로 고른다. 트럭 방의 지점은 제외한다. 즉 "너무 가깝지도, 아주 동떨어지지도 않은" 지점을 고르는 것이다. `SemiFunc.EnemyLeavePoint`는 이 함수에 30~50m를 넣어 쓴다.

첫 스폰 지점은 `EnemyDirector.FirstSpawnPointAdd`가 정한다. 트럭이 아닌 모든 `LevelPoint` 중 **트럭에서 가장 먼 지점**을 고르되, 이미 다른 몬스터가 쓴 지점은 거리를 0으로 취급해 제외한다. 모든 지점을 다 쓰면 목록을 비우고 재사용한다. 결과적으로 **게임 시작 시 몬스터들은 트럭에서 먼 곳부터 흩어져 배치**된다.

`EnemyTeleported(position)`는 트랜스폼을 옮기고 내비메시 에이전트를 `Warp`하고 리지드바디를 `Teleport`한 뒤 전원에게 RPC를 보내며, 받는 쪽은 `TeleportedTimer = 1f`를 세워 1초간 보간을 건너뛴다.

### 디스폰 트리거

`EnemyStateDespawn`은 상태를 표현하는 것 외에 두 가지 자동 디스폰 조건을 감시한다.

- `StuckDespawnLogic` — `Enemy.StuckCount >= StuckDespawnCount`(코드 기본값 10)면 시야를 0.25초 끄고 `Despawn`. 끼여서 못 움직이는 몬스터를 회수한다.
- `ChaseDespawnLogic` — `Chase`/`ChaseSlow`/`LookUnder` 상태가 누적 `DespawnAfterChaseTime`(코드 기본값 10초)을 넘으면 `Despawn`. 추격을 벗어난 뒤 10초 안에 다시 추격 상태로 들어가지 않으면 누적이 0으로 리셋된다. **한 번 추격이 시작되면 최대 10초 뒤에는 반드시 끝난다**는 안전장치다.

## 피격·스턴·죽음

### 체력 — `EnemyHealth`

코드 기본값 `health = 100`이고 실제 값은 프리팹마다 다르다. 주요 필드는 다음과 같다.

| 필드 | 코드 기본값 | 뜻 |
|---|---|---|
| `health` | 100 | 최대 체력 |
| `deathFreezeTime` | 0.1 | 죽는 순간 물리 정지 시간 |
| `impactHurt` | false | 스스로 부딪혀서 다치는가 |
| `impactLightDamage` / `impactMediumDamage` / `impactHeavyDamage` | 0 | 충돌 등급별 자해 피해 |
| `objectHurt` | false | 던져진 물건에 다치는가 |
| `objectHurtMultiplier` | 1 | 물건 피해 배수 |
| `objectHurtStun` | true | 물건에 맞으면 스턴되는가 |
| `objectHurtStunTime` | 2 | 스턴 시간 |
| `spawnValuable` | true | 죽으면 귀중품을 떨어뜨리는가 |
| `spawnValuableMax` | 3 | 한 몬스터가 평생 떨굴 수 있는 최대 개수 |

`Hurt(damage, direction)`는 마스터 클라이언트에서만 실행되고, `damageResistance`(0~1)를 적용한 뒤 체력을 깎고, 0 이하면 `Death`로 간다. `Heal(amount)`도 있다(`EnemyTickHealAura`가 쓴다 — 추정).

`impactLightDamage` 계열은 **몬스터가 스턴 중이거나 `nonStunHurtTimer`가 남아 있을 때만** 적용된다. 즉 평소에 벽에 부딪히는 것으로는 다치지 않고, 넘어뜨린 뒤 굴려야 다친다.

### 던진 물건의 피해 공식

`PhysGrabObjectImpactDetector`가 몬스터 콜라이더와 부딪혔을 때 계산한다. 먼저 충돌의 **파괴 등급** `num`(1/2/3)이 정해진다.

```
float massClamped = Mathf.Max(rb.mass, 1f);
breakForce > 500f * massClamped → 등급 3
breakForce > 400f * massClamped → 등급 2
breakForce > 300f * massClamped → 등급 1
```

그 다음, 충돌 직전 속도의 크기가 2를 넘어야 하고(`previousPreviousVelocityRaw.magnitude > 2f`), 스윕 테스트로 실제 그 몬스터를 맞혔는지 확인한 뒤 피해를 준다.

```
int damage = Mathf.RoundToInt((float)(10 * num) * (rb.mass * 0.5f));
damage = Mathf.RoundToInt((float)damage * enemy.Health.objectHurtMultiplier);
enemy.Health.Hurt(damage, -toEnemyCenter.normalized);
```

즉 **피해 = 10 × 파괴등급 × 물건질량 × 0.5 × 배수**다. 무거운 물건일수록 등급 문턱도 같이 올라가므로, 실제로는 "빠르게, 그리고 무겁게" 던져야 한다. 맞으면 물건 쪽으로 `2 × 등급` 크기의 충격량과 토크가 몬스터에 가해져 넘어진다. 귀중품으로 때리면 귀중품도 값의 10~20%만큼 깨진다(`Tiny/Small` 20%, `Medium` 약 14%, 나머지 10%, ±5% 난수).

### 스턴

귀중품으로 때렸을 때 스턴이 걸리는지는 **몬스터의 `EnemyType`과 귀중품의 질량**으로 정해진다. `EnemyType`은 `VeryLight, Light, Medium, Heavy, VeryHeavy` 5단계다.

```
float offset = (SemiFunc.MoonLevel() >= 1) ? 0f : -1.5f;
VeryLight  : 질량 >= 0.5 + offset
Light      : 질량 >= 1.0 + offset
Medium     : 질량 >= 2.0 + offset
Heavy      : 질량 >= 3.5 + offset
VeryHeavy  : 질량 >= 5.0 + offset
→ 통과하면 StateStunned.Set(2f)
```

첫 달(`MoonLevel() == 0`)에는 문턱이 1.5만큼 낮아 스턴이 쉽고, 이후 달에서는 원래 문턱이 적용된다. 부러진 경첩(문짝)으로 때리면 `EnemyType`이 `Medium` 이하인 몬스터만 2초 스턴된다.

이 밖의 스턴 경로:

- `EnemyRigidbody.stunFromFall` — 공중에 `stunFromFallTime`(1초) 이상 떠 있으면 `StateStunned.Set(3f)`. 즉 **몬스터를 들어 올려 떨어뜨리면 3초 스턴**된다.
- `EnemyRigidbody.grabStun` — 플레이어가 몬스터를 잡고 있는 동안 매 프레임 `StateStunned.Set(0.1f)`로 스턴을 유지한다. 다만 `grabTimeMax`(코드 기본값 3초)를 넘기면 몬스터가 몸을 흔들어 손을 뿌리친다(`GrabReleaseShake`).
- `HurtCollider`의 `enemyStun` / `enemyStunTime`(코드 기본값 2초) — 함정·아이템이 주는 스턴.
- `Enemy.OverrideType(type, time)` — 일시적으로 몬스터의 무게 등급을 바꿔 스턴 난이도를 조절한다.

스턴 중에는 `EnemyStateStunned`가 매 프레임 `enemy.DisableChase(0.25f)`를 호출해 추격 시작을 막고, 리지드바디의 위치·회전 추종을 끊고 드래그를 0.05로 낮춰 물리적으로 널브러지게 만든다. 스턴이 끝나면 내비메시 에이전트를 리지드바디 위치로 `Warp`시켜 다시 붙인다.

### 죽음과 귀중품

`Hurt`에서 체력이 0이 되면 `DeathRPC` → `deathFreezeTime` 뒤 `DeathImpulseRPC` → `onDeath` UnityEvent. `onDeathStart`/`onDeath`가 나뉘어 있어 죽는 연출과 실제 소멸을 분리한다.

**귀중품 드롭은 죽는 순간이 아니라 디스폰하는 순간에 일어난다.** `EnemyParent.Despawn()`의 끝부분에서, 체력이 0이고 `spawnValuable`이 켜져 있고 `valuableSpawnTimer > 0`(= 최근 10초 안에 플레이어가 20m 안에 있었음)이고 `spawnValuableCurrent < spawnValuableMax`일 때만 생성한다. 떨어지는 물건은 난이도 등급으로 정해진다.

| `EnemyParent.difficulty` | 생성되는 프리팹 |
|---|---|
| `Difficulty1` | `AssetManager.instance.enemyValuableSmall` |
| `Difficulty2` | `AssetManager.instance.enemyValuableMedium` |
| `Difficulty3` | `AssetManager.instance.enemyValuableBig` |

`EnemyValuable`은 생성 후 5초 동안 파괴 불가(`indestructibleTimer = 5f`)이고, 그동안 프레넬 셰이더로 다르게 빛나 "아직 못 깬다"를 시각화한다. `EnemyDirector.enemyValuables` 목록은 최대 10개만 유지하고, 넘으면 가장 오래된 것을 파괴한다. 달 4 이상(`SemiFunc.MoonLevel() >= 4`)에서는 파괴될 때 폭발한다.

또한 `PhysGrabObjectImpactDetector.DestroyObject`가 몬스터에게 호출되면 `Hurt(999999, ...)`로 즉사시킨다(체력 컴포넌트가 없으면 그냥 디스폰).

### 구덩이

`EnemyPitCheck`는 `pitCheckTransforms` 각각에서 아래로 `pitCheckDistance`(코드 기본값 2m) 레이캐스트를 쏴, `Default` 레이어 바닥이 없거나 `HurtCollider.deathPit` 트리거가 걸리면 `isOverPit = true`로 만든다. `CheckPit()`을 부르면 0.2초 동안 초당 5회 검사한다. 몬스터가 구덩이로 걸어 들어가는 것을 막는 용도(추정).

## 스폰과 난이도

### 마릿수 공식

전부 `EnemyDirector.AmountSetup()`에 있다. 난이도 진행은 클리어한 레벨 수를 세 구간으로 나눈 정규화 값으로 표현된다.

```
RunGetDifficultyMultiplier1() = Clamp01(levelsCompleted / 9)
RunGetDifficultyMultiplier2() = Clamp01((levelsCompleted - 9) / 10)
RunGetDifficultyMultiplier3() = Clamp01((levelsCompleted - 19) / 10)
```

레벨 0~9는 1구간, 10~19는 2구간, 20~29는 3구간이다. 마릿수는 난이도 등급별로 두 개의 `AnimationCurve`를 가지며, 2구간에 진입했으면 `_2` 커브를, 아니면 `_1` 커브를 쓴다.

```
if (RunGetDifficultyMultiplier2() > 0) {
    amountCurve3Value = (int)amountCurve3_2.Evaluate(RunGetDifficultyMultiplier2());
    amountCurve2Value = (int)amountCurve2_2.Evaluate(RunGetDifficultyMultiplier2());
    amountCurve1Value = (int)amountCurve1_2.Evaluate(RunGetDifficultyMultiplier2());
} else {
    amountCurve3Value = (int)amountCurve3_1.Evaluate(RunGetDifficultyMultiplier1());
    ... 같은 식으로 2, 1
}
totalAmount = amountCurve1Value + amountCurve2Value + amountCurve3Value;
```

커브의 실제 키프레임은 인스펙터 값이라 코드에는 없다. **즉 "레벨 N에서 몇 마리"는 코드가 아니라 데이터가 정하고, 코드는 커브를 평가할 x 값을 정의할 뿐이다.**

디스폰 대기 시간 배수도 같은 방식이다.

```
if (RunGetDifficultyMultiplier3() > 0) despawnedTimeMultiplier = despawnTimeCurve_2.Evaluate(RunGetDifficultyMultiplier3());
else if (RunGetDifficultyMultiplier2() > 0) despawnedTimeMultiplier = despawnTimeCurve_1.Evaluate(RunGetDifficultyMultiplier2());
else despawnedTimeMultiplier = 1f;
```

그리고 `despawnedDecreaseMultiplier`는 시간이 지날수록 줄어든다. `despawnedDecreaseMinutes`분마다 `despawnedDecreasePercent`만큼 빼고 0에서 멈춘다. 즉 **한 레벨에 오래 머무를수록 몬스터가 점점 빨리 되돌아온다.**

### 어떤 몬스터를 뽑는가 — `PickEnemies`

`EnemyDirector`는 난이도 등급별 후보 목록 `enemiesDifficulty1`, `enemiesDifficulty2`, `enemiesDifficulty3`를 들고 있고, 등급 3부터 역순으로 필요한 수만큼 `PickEnemies`를 부른다.

각 호출에서 목록을 셔플한 뒤 후보를 순회하며, 다음 조건에 걸리면 건너뛴다.

- `levelsCompletedCondition`이 켜져 있고 현재 `levelsCompleted`가 `[levelsCompletedMin, levelsCompletedMax]` 밖일 때 (단 `Max`가 0이면 상한 없음)
- 플레이어의 누적 플레이 횟수(`DataDirector.Setting.RunsPlayed`)가 `EnemySetup.runsPlayed`보다 작을 때 — **신규 플레이어에게는 일부 몬스터가 아예 등장하지 않는다.**

통과한 후보마다 가중치를 굴린다.

```
float baseChance = rarityPreset ? rarityPreset.chance : 100f;
float maxRoll = Mathf.Max(1f, baseChance - 30f * runSpawnCount - 10f * levelSpawnCount);
float roll = Random.Range(0f, maxRoll);
// roll이 가장 큰 후보가 선택된다
```

`runSpawnCount`는 이번 런 전체(`RunManager.instance.enemiesSpawned`)에서 그 몬스터가 뽑힌 횟수, `levelSpawnCount`는 이번 목록(`enemyList`)에서 뽑힌 횟수다. **같은 몬스터가 반복될수록 확률 상한이 런 단위로 30, 레벨 단위로 10씩 깎여 다양성이 강제된다.**

`GetEnemy()`는 선택 목록을 순서대로 꺼내면서, 그 몬스터의 런 누적 카운트가 4 미만이면 최대 2개까지 `RunManager.instance.enemiesSpawned`에 더 추가한다. 즉 한 번 나온 몬스터는 카운트를 크게 늘려 다음 레벨에서 잘 안 나오게 만든다.

### 레벨 생성기와의 접속

`LevelGenerator.EnemySetup()` 코루틴이 실제 생성을 한다.

1. 트럭 `RoomVolume`을 찾고, `LevelPathPoints` 중 트럭에서 **가장 먼 지점** `furthestPoint`를 구한다.
2. `RunManager.instance.EnemiesSpawnedRemoveStart()`
3. `EnemyDirector.instance.AmountSetup()`을 호출해 `totalAmount`를 확정.
4. `for (int i = 0; i < totalAmount; i++) EnemySpawn(EnemyDirector.instance.GetEnemy(), furthestPoint.transform.position);`
5. `EnemiesSpawnedRemoveEnd()`, 그리고 멀티플레이면 `EnemySpawnTargetRPC`로 전원에게 목표 마릿수를 알린다.

`EnemySpawn(setup, position)`은 `EnemySetup.spawnObjects`의 프리팹을 하나씩 `PhotonNetwork.InstantiateRoomObject`(싱글이면 `Instantiate`)로 만들고, `SetupDone = true`를 세우고, `EnemyTeleported(position)`로 옮기고, `EnemiesSpawnTarget++`, 그리고 `EnemyDirector.FirstSpawnPointAdd`로 첫 스폰 지점을 배정한다. **`spawnObjects`가 리스트이므로 하나의 `EnemySetup`이 여러 마리를 묶어 낼 수 있다**(무리 몬스터).

모든 몬스터가 준비되면 각 클라이언트가 `EnemyReadyRPC`를 보내고, 전원이 준비되면 `EnemyReadyAllRPC`로 레벨 생성이 다음 단계로 넘어간다.

### `EnemySetup` (ScriptableObject) 필드 전부

```
List<PrefabRef> spawnObjects;   // 이 셋업이 만들어내는 프리팹들
bool  levelsCompletedCondition; // 레벨 수 조건을 쓸 것인가
int   levelsCompletedMin;       // [Range(0,10)]
int   levelsCompletedMax = 10;  // [Range(0,10)]
RarityPreset rarityPreset;      // .chance 를 가중치로 사용
int   runsPlayed;               // 이 값 이상 플레이해야 등장
```

메뉴 경로는 `Other/Enemy Setup`, 파일명 규약은 `Enemy - _____`다. 주목할 점: **난이도 등급은 `EnemySetup`이 아니라 `EnemyParent.difficulty`에 있고, `EnemyDirector`의 세 목록에 어느 셋업을 넣었는지가 등급을 결정한다.** 둘은 별개로 관리된다.

### 행동량 — 몬스터를 강제로 물러나게 하는 장치

`EnemyDirector.Update`에는 "몬스터가 플레이어와 같은 방에 얼마나 오래 있었나"를 누적하는 카운터가 있다.

```
// 플레이어와 같은 방에 있고 forceLeave가 아닌 스폰된 몬스터마다
Difficulty1 → 0.5, Difficulty2 → 1.0, Difficulty3 → 2.0
num += 등급값 * enemyParent.actionMultiplier;
enemyActionAmount += num * Time.deltaTime;   // 없으면 초당 0.1씩 감소
if (enemyActionAmount > 120f) { ... }
```

**누적치가 120을 넘으면** `enemyActionAmount`를 0으로 리셋하고, 플레이어에게서 가장 먼 `LevelPoint`로 `SetInvestigate(..., float.MaxValue, pathfindOnly: true)`를 쏴 **모든 몬스터를 그쪽으로 유인**하며, 스폰된 몬스터 전부에 `forceLeave = true`를 세운다. 난이도 3 몬스터 한 마리가 계속 붙어 있으면 60초 만에, 난이도 1 한 마리면 240초 만에 도달한다. **압박이 무한정 이어지지 않게 하는 리듬 장치**다.

### 시작 유예 — `spawnIdlePauseTimer`

레벨 시작 시 `EnemyDirector.Start`에서 계산한다.

```
spawnIdlePauseTimer = 60f * Random.Range(2f, 3f) * spawnIdlePauseCurve.Evaluate(RunGetDifficultyMultiplier1());
if (Random.Range(0, 100) < 20) spawnIdlePauseTimer *= Random.Range(0.1f, 0.25f);  // 20% 확률로 크게 단축
spawnIdlePauseTimer = Mathf.Max(spawnIdlePauseTimer, 5f);
```

이 타이머가 도는 동안 `SemiFunc.EnemySpawnIdlePause()`가 참이 되어 배회 상태의 몬스터가 새 경로를 잡지 않는다(제자리 대기). 단, 모든 몬스터가 첫 스폰 지점을 사용했을 때만 타이머가 흐르고, **플레이어가 20m 안에 접근하면(`playerClose`) 즉시 0으로 리셋되어 유예가 끝난다.** 기본 2~3분이지만 20% 확률로 12~45초로 줄어들어 "안전한 초반"을 믿을 수 없게 만든다.

### 추출 완료 후

`RoundDirector.instance.allExtractionPointsCompleted`가 참이 되면 규칙이 바뀐다.

- 디스폰 대기가 30초 넘게 남은 몬스터는 전부 0으로 당겨져 즉시 재등장한다.
- `extractionsDoneState`가 `StartRoom`인 10초 동안: `enemyActionAmount = 0`, `despawnedDecreaseMultiplier = 0`으로 만들고, 플레이어에게서 먼 곳의 스폰된 몬스터를 전부 즉시 디스폰시킨 뒤, 3초마다 **시작 방(트럭 쪽)의 `LevelPoint`로 반경 100 investigate**를 쏜다. 몬스터들이 탈출로에 모인다.
- 그 뒤 `PlayerRoom` 상태로 넘어가면 **플레이어가 있는 방의 `LevelPoint`로 반경 100 investigate**를 쏜다. 간격은 3초에서 시작해 2초씩 늘어 최대 30초까지 벌어진다.

즉 임무를 다 끝내면 몬스터들이 탈출 경로와 플레이어 위치로 몰려온다.

## 종류 목록

읽기 전에 알아 둘 것이 세 가지 있다.

첫째, **개별 몬스터는 거의 전부 자기만의 `State` enum을 갖는다.** 공통 `EnemyState`(12개)와는 별개이고, 개수가 10~26개로 몬스터마다 크게 다르다. `EnemyElsa`는 26개, `EnemyBirthdayBoy`는 22개다.

둘째, **체력과 이동 속도의 기본값은 대부분 코드에 없다.** 체력은 `EnemyHealth.health`(코드 기본값 100)를 프리팹에서 덮어쓰고, 이동 속도는 `NavMeshAgent.speed`를 프리팹에 두고 `EnemyNavMeshAgent.DefaultSpeed`로 읽어 쓴다. 코드에 숫자가 나오는 것은 상태별 `OverrideAgent(속도, 가속, 시간)` 호출뿐이라, 아래 표의 속도 칸은 **그 몬스터가 코드에서 강제로 걸어 두는 속도**를 적은 것이다.

셋째, **대미지도 대부분 코드에 없다.** 접촉 피해는 자식 오브젝트의 `HurtCollider`(`playerDamage` 코드 기본값 10)가 처리하고, 프리팹에서 값을 덮어쓴다. 코드에 리터럴로 박혀 있는 것은 폭발(`ParticleScriptExplosion.Spawn`)과 몇몇 직접 호출(`HurtOther`)뿐이다.

또 하나, 커뮤니티에서 부르는 이름은 소스에 없다(`EnemyParent.enemyName`도 프리팹 값이다). 널리 알려진 몇 개만 "(추정)"으로 달았고, 확신이 없는 것은 비워 두었다.

| 클래스 | 상태 수 | 감지 | 공격과 대미지 | 코드에 있는 속도 |
|---|---|---|---|---|
| `EnemyAnimal` | 8 | `EnemyVision` + `OnInvestigate`(소음) | 접촉(HurtCollider, 코드에 없음). 잡히면 `grabAggroTimer = 60f` | Investigate 4, WreakHavoc 5, Leave 6 |
| `EnemyBang` (추정: Clown) | 12 | `EnemyVision`. 감지 시 `EnemyBangDirector.TriggerNearby`로 주변 개체 연쇄 점화 | 자폭. 퓨즈 완주 시 `Spawn(size 1, playerDamage 30, enemyDamage 25, forceMulti 2)`. 도중 사망 시 `size 0.5, playerDamage 15, enemyDamage 10` | 오버라이드 없음(프리팹 기본값) |
| `EnemyBeamer` (추정: Apex Predator) | 13 | `EnemyVision` | `SemiLaser`로 원거리 조준 사격. `laserRange = 10f`. 대미지는 코드에 없음 | Seek 시 기본×2 |
| `EnemyBirthdayBoy` | 22 | `EnemyVision` + 풍선 터짐 감지(`popAggroRadius = 10f`) | 접촉(코드에 없음). 침대 밑으로 기어드는 `AttackUnder` 패턴 보유 | Leave 6, 분노 추격 4, Attack 4, 침대밑 접근 기본×0.5, LookAround 3, Seek 2.5 |
| `EnemyBombThrower` | 13 | `EnemyVision` + 레이캐스트 재확인 | 자기 머리를 키워 던진다. `EnemyBombThrowerHead.ExplodeRPC`에서 `Spawn(size 2, playerDamage 75, enemyDamage 30, forceMulti 4)` + 넘어짐 추가 피해 10 | BackAwayHead 2 |
| `EnemyBowtie` | 10 | `EnemyVision` | 비명(`Yell`). 코드에 대미지 없음. 도망칠 때 `hurtColliderLeave`를 켜 경로상의 플레이어를 친다 | Leave 5 |
| `EnemyCeilingEye` (추정: Peeper) | 6 | `EnemyVision` + `SemiFunc.PlayerVisionCheck`(5방향 레이, 20m) + `playerClose` | **락온 3초 뒤 1초마다 `HurtOther(2)`** — 코드에 대미지가 있는 드문 사례 | 이동 오버라이드 없음(천장 고정, 추정) |
| `EnemyDuck` | 21 | `EnemyVision` | 변신 후 플레이어 카메라 위치로 직접 돌진. 접촉 피해(코드에 없음) | ChaseNavmesh 10, DeTransform 0.5, 우회 접근 기본×0.5 |
| `EnemyElsa` | 26 | `EnemyVision`(작은 형태·큰 형태 각각) | 큰 형태로 변신 후 접촉(코드에 없음) | 전용 `OverrideMoveSpeed(speed, 0.2f)` 사용. TryTransform·Leave 3, 우회 1.5, 다른 지점 Leave 4, Notice 3 |
| `EnemyFloater` (추정: Mentalist) | 13 | `EnemyVision` | ChargeAttack 7초 → DelayAttack 3초 → Attack 2초. 3회 또는 30% 확률로 Leave | Sneak 1.5, Leave 1.5, 접근 2 |
| `EnemyGnome` | 13 | `EnemyVision` → `EnemyGnomeDirector.SetTarget` (개체가 아니라 **디렉터가 무리의 타깃을 관리**) | 3초짜리 Attack. 대미지 판정은 디렉터가 함 | `speedMin = 1f`, `speedMax = 2f` — 개체마다 이 범위에서 무작위로 정해진다 |
| `EnemyHeadGrabber` (추정: Headman) | 20 | `EnemyVision` + `PlayerDeathHead` 별도 탐색(`GetClosestDeathHead`) | 반복 타격(`attacksInRow`). 죽은 플레이어 머리에는 `AddForce(방향 × 15f)`. 산 플레이어는 넘어뜨리기(`TumbleOverrideTime`) | Leave 6 |
| `EnemyHeartHugger` | 17 | `EnemyVision` + 가스에 노출된 플레이어 목록(`playersInGas`) | 가스로 무력화 → 유인(`Lure`) → `bites = Random.Range(3,6)`회 물기(0.5초 간격, `biteHurtCollider`) | 속도 필드 없음(스프링 기반 이동) |
| `EnemyHidden` | 14 | `EnemyVision` | **대미지가 아니라 납치.** 플레이어를 들고 최대 10초 이동한 뒤 놓아준다 | PlayerMove 5 (가속 10) |
| `EnemyHunter` (추정: Huntsman) | 12 | `EnemyVision` + `LayerMaskGetVisionObstruct` 레이캐스트 | 원거리 사격. `hurtCollider`를 총구로 옮겨 켜는 방식. `shotsFiredMax = 4` | 오버라이드 없음(프리팹 기본값) |
| `EnemyOogly` | 12 | `EnemyVision` + `PlayerGetAllPlayerAvatarWithinRangeAndVision(20f)` | 천장에서 급강하(Dive, 5초 제한) → 4~6초 몸싸움(`WrestlePlayer`, `TumbleOverrideTime` 1.5→1.0) | 물리 힘 계수 `moveForce = 1f`. Dive는 ×0.5, 접근은 ×`Clamp01(거리/3)` |
| `EnemyRobe` | 15 | `EnemyVision` + `VisionBlocked()` 레이캐스트 | 2초짜리 Attack, 그리고 문 밑으로 팔을 뻗는 `LookUnderAttack` | Seek 3(가속 3), Attack 3(가속 10) |
| `EnemyRunner` | 18 | `EnemyVision` | 돌진. `agentSpeed = 3f`까지 가속해 부딪친다 | 돌진 3, Sneak·Leave 1, StuckAttack 3 |
| `EnemyShadow` (추정: Shadow Child) | 9 | 자체 `visionTransform` + `EnemyVision` + 근접 `HandsInPositionToSlap()` | `slapDuration = 0.3f`의 손바닥 내려치기. `hurtCollider`와 `hurtColliderWave` 두 개 | StuckAttack 2.5 (가속 30) |
| `EnemySlowMouth` (추정: Spewer) | 19 | `EnemyVision` | 붙은 뒤 `attachedTimer = Random.Range(20f, 60f)` 동안 떨어지지 않고, 5~10초마다 토해 시야를 막는다. 직접 대미지 호출은 없음 | Roam 0.8(가속 30), 부착 추적 8 |
| `EnemySlowWalker` (추정: Trudge) | 17 | `EnemyVision`. **LookUnder 상태에서 `VisionDotStanding`을 0으로 낮춰** 기어들어간 플레이어까지 잡는다 | `LookUnderAttack`에서 0.6초 간격 최대 10회. 대미지 수치는 코드에 없음 | LookUnder 계열 이동 8 |
| `EnemySpinny` | 15 + 색상 5 | `EnemyVision` + 플레이어가 룰렛을 직접 잡아 돌리는 상호작용 | 공격이 아니라 도박. 9칸 룰렛 결과에 따라 초록 `Heal(25)`, 빨강 `Hurt(50)`, 검정 체력 1로, 흰색 완전 회복, 노랑 돈주머니 | 단계별 2.5 / 4 / 6. `spinDurationSeconds = 3.8f` |
| `EnemyThinMan` | 8 | **`EnemyVision`을 안 쓴다.** `enemy.OnScreen.GetOnScreen(player)` — 화면에 보이는가로만 판단(슬렌더맨형) | `HurtOther(50, savingGrace: false)` — 코드에 대미지가 박힌 사례. 동시에 넘어뜨림 | 없음. **내비메시가 아니라 텔레포트로 이동** |
| `EnemyTick` | 14 | `EnemyVision` + 물건을 든 플레이어에게 자동 흡착(`instantSuck`) | 흡혈. `healthPerSuck = 10`, 간격 `0.8f + 자기체력 × 0.01f`, `HurtOther(min(10, 남은치))` | 4 (가속 12) |
| `EnemyTricycle` | 21 | **`EnemyVision`을 안 쓴다.** `SphereCastAll` 기반 길막힘 감지(`IsPlayerBlockingNavmeshPath`) | 벨을 울려 예고한 뒤 돌진(`AttackDive`). `hurtColliderTransform` 켜고 끄기 | 명시 상수 없음. BackToNavmesh만 3 |
| `EnemyTumbler` | 14 | `EnemyVision` + `Physics.Raycast` 시야선 확인 | 자기 몸에 `AddForce(방향 × 40f)` + `AddTorque(8f)`를 걸어 굴러 부딪친다. 맞으면 `OverrideEnemyHurt(3f)` | 4 (가속 12) |
| `EnemyUpscream` | 10 | `EnemyVision`. `idleBreakTimeMin/Max = 45f/90f` | **체력 피해가 전혀 없다.** `CameraAim`, `CameraZoom(50f)`, 비네트, `InputDisable(0.1f)`, 들고 있던 물건 강제 낙하, 카메라 글리치 — 순수 점프스케어 | GoToPlayer 5 (가속 10) |
| `EnemyValuableThrower` | 11 | `EnemyVision` + 8m 안 플레이어 시선 강제(`PlayerEyesOverride`) | 귀중품을 주워 던진다. `AddForce(방향 × min(20 × 질량, 100), Impulse)` + `PlayerHurtMultiplier(5f, 2f)`. 피해는 물리 충돌로 계산된다 | 오버라이드 없음(프리팹 기본값) |

읽고 나서 눈에 띄는 패턴이 몇 가지 있다.

**공격이 곧 체력 감소인 몬스터가 소수다.** 28종 중 코드에 직접 HP 감소가 박힌 것은 `EnemyThinMan`(50), `EnemyCeilingEye`(초당 2), `EnemyTick`(회당 10), `EnemySpinny`(룰렛 결과), 그리고 폭발형 둘(`EnemyBang` 30, `EnemyBombThrower` 75)뿐이다. 나머지는 접촉 콜라이더나 **넘어뜨리기**(`TumbleRequest` + `TumbleOverrideTime` + `OverrideEnemyHurt`)로 처리한다. `EnemyUpscream`은 아예 피해가 없고 `EnemyHidden`은 납치만 한다. **"위협 = 즉시 체력 감소"가 아니라 "위협 = 통제권 상실"로 설계돼 있다.**

**감지 방식에 세 갈래가 있다.** 대부분은 `EnemyVision`을 쓰지만, `EnemyThinMan`은 `EnemyOnScreen`만 쓰고(보고 있으면 멈춘다), `EnemyTricycle`은 스피어캐스트로 "내 진로를 막고 있는가"만 본다. 감지 컴포넌트를 갈아 끼우는 것만으로 완전히 다른 종류의 위협이 만들어진다.

**무리와 디렉터.** `EnemyGnome`은 개체가 아니라 `EnemyGnomeDirector`가 타깃을 정하고, `EnemyBang`은 `EnemyBangDirector.TriggerNearby`로 근처 개체를 연쇄 점화한다. 개체 AI 위에 소규모 디렉터를 한 겹 더 얹는 구조다.

**변신·형태 전환이 흔하다.** `EnemyDuck`(양동이를 씌우면 변신 차단), `EnemyElsa`(안 볼 때만 변신, 역시 양동이로 저지 가능), `EnemyBombThrower`(머리가 없으면 몸통만), `EnemyHeartHugger`(성장/축소). 상태 enum이 20개를 넘는 몬스터는 대부분 이 형태 전환 때문이다.

## 함정

함정은 몬스터와 별개 계통이지만 **소리로 몬스터를 부른다**는 점에서 같은 시스템에 물려 있다.

### 공통 뼈대 — `Trap.cs`

플래그 세 개로 돌아간다. `trapTriggered`(한 번 터지면 잠금) → `trapActive`(작동 중) → `trapStart`(발동 신호, RPC로 전 클라이언트에 전파).

발동 경로는 둘이다.

- **경로 A — 잡고 있는 동안 랜덤 발동.** `triggerOnTimer`가 켜진 함정은 `physGrabObject.grabbed`인 동안에만 `Random.Range(5f, 15f)` 타이머가 돌고, 만료되면 `SemiFunc.ValuableTrapActivatedDiceRoll((int)trapActivateRarityLevel)`을 굴린다. 확률은 등급 1이 1/2, 등급 2가 1/4, 등급 3 이상이 1/9, 등급 없음은 항상 실패다. 통과하면 `TrapActivateSyncRPC`로 전원에게 알린다. **즉 함정 귀중품은 들고 있을 때만 위험하고, 값비쌀수록(희귀 등급이 높을수록) 오히려 덜 터진다.**
- **경로 B — 즉시 발동.** 각 함정이 자기 조건(잡기·놓기·근접·코 누르기)에서 `TrapStart()`를 직접 부른다. 여기서도 같은 다이스롤을 거친다.

몬스터 호출은 `enemyInvestigate` 플래그가 1초(`enemyInvestigateTimerMax = 1f`) 연속 켜져 있으면 `EnemyDirector.instance.SetInvestigate(위치, enemyInvestigateRange)`를 부르는 방식이고, **기본 반경은 35**다. 앞의 소음 표와 비교하면 발소리(5)나 물건 파괴(15)보다 훨씬 크다. 함정은 이 게임에서 가장 시끄러운 물건이다.

`TrapColliderScript`는 근접 트리거 보조로, 로컬 플레이어가 들어오고 + 몬스터에게 쫓기는 중이 아니고 + `TrapDirector.instance.TrapCooldown <= 0` 이고 + 웅크리지 않았을 때만 `TrapCollision = true`를 세운다. **웅크리면 근접 함정을 피할 수 있다.**

### 배치 — `TrapDirector.cs`

씬의 모든 함정은 `TrapTypeIdentifier.Start()`에서 `TrapDirector.instance.TrapList`에 자동 등록된다. 레벨 생성이 끝나면 마스터 클라이언트에서 `UpdateTrapList()`가 돈다.

1. `TrapTypeIdentifier.trapType`(문자열)별로 묶고, **각 타입에서 무작위로 하나만 남긴 뒤 나머지는 파괴**한다(멀티플레이면 `DestroyTrap` RPC). `OnlyRemoveTrigger`가 켜진 것은 오브젝트 대신 `Trigger`만 지워 장식 프롭으로 남긴다(`TrapPropSwitch`).
2. 남은 개수가 `TrapCount`(코드 기본값 **2**)보다 많으면 초과분을 무작위로 더 지운다.

결과적으로 **레벨 하나에 실제로 작동하는 함정은 2개뿐**이고, 나머지 함정처럼 생긴 물건은 전부 껍데기다. 플레이어는 어느 것이 진짜인지 모른 채 모든 함정 프롭을 경계하게 된다. 값싼 긴장을 만드는 좋은 수법이다. `DebugAllTraps`를 켜면 이 필터링을 건너뛴다.

### 함정 목록

| 클래스 | 트리거 | 일어나는 일 | 몬스터 호출 반경 |
|---|---|---|---|
| `AnimalCrateTrap` | 잡으면 **즉시**(확률·타이머 없음) | 뒤집힌 상태에서 1초마다 4단계 충격(`TinyBump`~`BigBump`)과 카메라 흔들림, 끝에 `Spawn(size 1, playerDamage 50, enemyDamage 300)` 폭발 | 35 (충격마다) |
| `BottleTrap` | 경로 A(타이머) | 코르크가 튀며 병이 로켓처럼 날아다닌다. 잡은 손은 강제로 풀린다 | 15 |
| `ChompBookTrap` | 경로 A | 반경 10m 안의 최근접 플레이어에게 물리적으로 돌진해 `biteAmount`회 문다. 그동안 파괴 불가 | 35 |
| `ClownTrap` | **코를 누르는 근접 상호작용**(`TouchNose()`) | 경고 2회 뒤 최종 경고와 카운트다운, 그리고 `Spawn(size 1.5, playerDamage 100, enemyDamage 300)` 자폭 | 15 |
| `DoorTrap` | 자체 코드 없음(`Trap` 기본 로직만) | — | 35 |
| `FanTrap` | 잡으면 작동, 놓으면 정지 | 날개가 2초에 걸쳐 최고 회전속도 1500(deg/s)까지 올라가고 정지에 4초. `HurtCollider`의 타격력이 회전속도에 비례해 선형 증가 | 35 (작동 내내) |
| `FrogTrap` | 잡았다가 **놓으면** (카트 안이면 무시) | 0.5~0.8초 간격 무작위 점프(임펄스 + 토크) | 15 (점프마다) |
| `GrandfatherClockTrap` | 경로 A | 진자가 15초에 걸쳐 최고 속도 5까지 가속, 종을 최대 3회 울린다 | **60** — 전체 함정 중 최대 |
| `LightningTrap` | 자체 코드 없음 | — | 35 |
| `MusicBoxTrap` | 잡으면 | 뚜껑이 열리고 음악이 무작위 지점부터 재생. 놓으면 자동으로 닫힌다 | 35 (재생 중) |
| `PropaneTankTrap` | 경로 A | 불붙은 탱크가 로켓처럼 날아다니며 접촉 피해, `Explode()` 시 `Spawn(size 0.8, playerDamage 50, enemyDamage 100)` | 15 |
| `ToiletTrap` | 자체 코드 없음 | — | 35 |
| `ToyMonkeyTrap` | 경로 A | 심벌즈를 치는 원숭이. 한 사이클마다 소리와 임펄스로 몸체가 들썩인다 | 35 (작동 중) |
| `TrapGramophone` | 경로 A | 음악을 무작위 지점부터 재생, 레코드판·크랭크 회전 | 35 (재생 중) |
| `TrapRadio` | 경로 A | 0.5초 점멸 후 음악 루프, 미터 바늘 진동 | 35 (재생 중) |
| `TrapTV` | 경로 A | 정전기 0.5초 → 10초 일반 재생 → 고양이 대사 10초 × 2 → 쥐 대사 10초 × 2 순환 | 35 (작동 중) |

폭발 대미지만 코드에 리터럴로 박혀 있고, 나머지 접촉 피해는 전부 `HurtCollider` 프리팹 값이다. `enemyDamage`가 `playerDamage`보다 훨씬 큰 점(300 대 50, 300 대 100)이 눈에 띈다 — **함정은 플레이어를 죽이기보다 몬스터를 죽이는 데 훨씬 효과적**이고, 그래서 유인해서 터뜨리는 플레이가 성립한다.

## 연출

`MusicEnemyNear`는 몬스터가 가까울수록 볼륨이 올라가는 상시 루프다(`MaxDistance = 15f`, `MinDistance = 4f`, `OnScreenDistance = 10f`). 시야 차단 레이캐스트를 하므로 벽 너머의 몬스터는 덜 들리고, 몬스터마다 `Enemy.EnemyNearMusic` 플래그로 이 음악을 쓸지 정한다.

`MusicEnemySighting`은 "처음 봤을 때" 스팅어다. 1초마다 `enemiesSpawned`를 훑어 `Enemy.SightingStinger`가 켜진 몬스터가 `DistanceMax` 안에 있고 **로컬 화면에 실제로 보이며**(`OnScreen.OnScreenLocal`) 방금 텔레포트하지 않았을 때만 울린다. 이 컴포넌트는 대상 몬스터에 `EnemyPlayerDistance`와 `EnemyOnScreen`이 없으면 `Debug.LogError`를 낸다 — **연출 요구사항이 컴포넌트 요구사항을 만든다.** `MusicEnemyCatch`는 잡혔을 때 무작위 클립을 재생하고 배경음악을 10초 끊는다.

`AudioScare`는 놀람 효과음 재생기로 `PlayImpact()`·`PlaySoft()`·`PlayCustom()` 셋을 갖고, 어느 쪽이든 배경음악을 20초 중단시킨다. `EnemyStateChaseBegin`은 `Stinger`가 켜져 있으면 `CameraGlitch.Instance.PlayShort()`와 함께 이것을 부르고, 추격 대상이 나 자신이거나 같은 방이거나 화면에 보이면 `Shake(5f, 0.25f)` + `Shake(3f, 0.5f)`를 풀로, 아니면 거리 감쇠판을 쓴다. **누구를 쫓는지가 흔들림의 세기로 전달된다.**

머리 계열에서 중요한 것은 `EnemyHeadController`다. 추격 중(`Chase`/`ChaseSlow`/`LookUnder`) 8m 안의 플레이어에게 `SemiFunc.PlayerEyesOverride`를 걸어 **플레이어의 시선을 몬스터 쪽으로 강제로 돌린다.** `EnemyHeadCatchCutscene`은 물기 컷신 사운드 묶음으로 물기 시작/첫 물기/마지막 물기마다 카메라 글리치와 유리 깨지는 소리를 함께 낸다. 나머지 `EnemyHead*`(`Tilt`, `Eye`, `Hair`, `Float`, `Lean`, `Pupil`)는 순수 시각 보조다.

## 네트워크

판정 주체는 명확하다. **몬스터 AI는 전부 마스터 클라이언트에서만 돈다.**

- `Enemy.Start`에서 `MasterClient = !GameManager.Multiplayer() || PhotonNetwork.IsMasterClient`를 정한다. 거의 모든 `EnemyState*.Update`는 첫 줄에서 `if (!Enemy.MasterClient) return;`를 한다.
- `EnemyVision.Vision()` 코루틴은 마스터가 아니면 즉시 `yield break`. `EnemyNavMeshAgent.Awake`는 마스터가 아니면 `Agent.enabled = false`.
- `EnemyHealth.Hurt`/`Heal`은 `SemiFunc.IsMasterClientOrSingleplayer()` 안에서만 체력을 바꾸고, 결과를 RPC로 알린다.

동기화되는 것과 방식:

| 무엇 | 방식 | 내용 |
|---|---|---|
| 위치·상태·타깃·스턴 | `Enemy.OnPhotonSerializeView` | `transform.position`, `CurrentState`, `TargetPlayerViewID`, `Stunned` |
| 셋업 완료 | `EnemyParent.OnPhotonSerializeView` | `SetupDone` |
| `LookUnder` 진행 | `EnemyStateLookUnder.OnPhotonSerializeView` | `WaitDone` |
| 스폰/디스폰 | `SpawnRPC` / `DespawnRPC` (`RpcTarget.All`) | `EnableObject` 토글, 체력 리셋 |
| 텔레포트 | `EnemyTeleportedRPC` | 좌표 + `TeleportedTimer = 1f` |
| 피격/회복/사망 | `HurtRPC`, `HealRPC`, `DeathRPC`, `DeathImpulseRPC` | 색 플래시와 이벤트 발화 |
| 정지 | `FreezeRPC` | `FreezeTimer` |
| 조사 지시 | `EnemyStateInvestigate.SetRPC` | 좌표 + `pathfindOnly`, `onInvestigateTriggered` 이벤트 |
| 화면 판정 | `EnemyOnScreen.OnScreenPlayerUpdateRPC` | **클라이언트 → 전원** (역방향) |

클라이언트가 하는 일은 세 가지다. 첫째, `Enemy.Update`에서 받은 `PositionTarget`을 향해 `Vector3.MoveTowards`로 보간한다. 속도는 `PositionDistance / (1 / PhotonNetwork.SerializationRate)`로, 다음 패킷이 올 때까지 정확히 도착하도록 맞춘다. 둘째, 자기 카메라 기준 화면 판정을 계산해 `OnScreenPlayerUpdateRPC`로 마스터에게 올린다 — **몬스터 AI에서 클라이언트가 주도권을 갖는 유일한 정보**다. 셋째, 모든 연출(애니메이션, 사운드, 카메라 흔들림, 머티리얼 플래시)을 RPC와 UnityEvent를 받아 로컬에서 재생한다.

RPC 보안은 `SemiFunc.MasterOnlyRPC(_info)`(마스터가 보낸 것만 수락)와 `SemiFunc.OwnerOnlyRPC(_info, view)`(해당 오브젝트 소유자가 보낸 것만)로 처리한다. 모든 `[PunRPC]` 메서드가 이 검사로 시작한다.

싱글플레이(`GameManager.instance.gameMode == 0`)에서는 RPC를 보내지 않고 같은 메서드를 직접 호출하는 분기가 모든 곳에 쌍으로 들어가 있다.

## 수치

코드에서 직접 읽은 상수만 모았다. 프리팹 인스펙터 값은 제외했고, 앞의 시야 표(`EnemyVision`)와 소음 표는 중복이라 다시 싣지 않았다.

| 값 | 위치 | 의미 |
|---|---|---|
| 0.25초 | `EnemyVision`, `EnemyPlayerDistance`, `EnemyOnScreen` | 시야·거리·화면 판정 주기 |
| 1초 | `EnemyPlayerRoom`, `EnemyParent.PlayerCloseLogic` | 방·근접 판정 주기 |
| 20m / 6m | `EnemyParent.PlayerCloseLogic` | `playerClose` / `playerVeryClose` 거리 |
| 2초 | `EnemyStateSpawn.WaitTimer` | 스폰 후 배회 진입까지 |
| 2초 / 0.5m | `EnemyStateChase` | 못 닿는 상태가 이만큼 지속되면 `ChaseSlow`, 경로 끝점 허용 오차 |
| 10 | `EnemyStateDespawn.StuckDespawnCount` | 막힘 디스폰 문턱 |
| 10초 / 10초 | `DespawnAfterChaseTime` / `ChaseResetTimer` | 추격 최대 지속, 누적 리셋 유예 |
| 3 / 1m | `EnemyAttackStuckPhysObject` | 물건 공격 진입 문턱, 탐색 반경 |
| 3 | `EnemyStateRoaming.PhysObjectHitMax` | 물건 때리기 반복 한계 |
| 2~5초 | `EnemyParent.Logic` 초기값 | 최초 등장 대기 |
| 180~240초 | `EnemyParent.SpawnRPC` | 스폰 직후 타이머 일시정지 |
| ×3 / 최소 1초 | `EnemyParent.Despawn` | 죽어서 디스폰할 때 대기 배수, 대기 하한 |
| 10초 / 3 | `valuableSpawnTimer` / `spawnValuableMax` | 귀중품 드롭 유효 시간, 몬스터당 최대 개수 |
| 100 / 2초 / 0.1초 | `EnemyHealth` | 체력 기본값, 물건 피격 스턴, 사망 정지 |
| 300 / 400 / 500 | `impactLevel1/2/3` × `max(mass,1)` | 파괴 등급 문턱 |
| 20 / 80 / 150 | `impactForce` 문턱 | 경/중/강 충돌 판정 |
| 2.0 | 최소 충돌 속도 | 이보다 느리면 피해 없음 |
| 0.5초 / 3초 | `grabTimeNeeded` / `grabTimeMax` | 잡기 성립 시간, 버틸 수 있는 최대 시간 |
| 1초 → 3초 스턴 | `EnemyRigidbody.stunFromFallTime` | 공중 체공 → 낙하 스턴 |
| 10 | `EnemyRigidbody.stunResetSpeed` | 스턴 해제 시 복귀 속도 |
| 0.5 / 1 / 2 / 3.5 / 5 | 스턴 질량 문턱 | `VeryLight`~`VeryHeavy` (첫 달은 −1.5) |
| 120 | `EnemyDirector` 행동량 문턱 | 넘으면 전원 강제 이탈 |
| 0.5 / 1 / 2 | 난이도별 행동량 가중치 | `Difficulty1/2/3`. 없을 땐 초당 0.1 감소 |
| 120~180초 | `spawnIdlePauseTimer` 기본 | ×커브, 20% 확률로 0.1~0.25배, 하한 5초 |
| 5초 | `SetInvestigate`의 `DisableDecrease` | 반경 15 이상 소음이 재등장을 앞당기는 양 |
| 30~50m / 10~25m | `EnemyLeavePoint` / `EnemyRoamPoint` | 이탈·배회 목적지 거리 범위 |
| 3초 / 100 반경 | 추출 완료 후 investigate | 간격은 2초씩 증가, 최대 30초 |
| 10개 / 5초 | `enemyValuables` 상한 / `indestructibleTimer` | 넘으면 가장 오래된 것 파괴, 드롭 직후 파괴 불가 |
| 30 / 10 | `PickEnemies` 확률 감산 | 런 단위 / 레벨 단위 중복 페널티 |
| /9, /10, /10 | 난이도 정규화 분모 | 레벨 0–9, 10–19, 20–29 |
| 8m | `EnemyHeadController` | 시선 강제 사거리 |
| 15 / 4 / 10 | `MusicEnemyNear` | Max / Min / OnScreen 거리 |
| 20초 / 10초 | `AudioScare` / `MusicEnemyCatch` | 배경음악 중단 시간 |
| 2 | `TrapDirector.TrapCount` | 레벨당 실제 작동하는 함정 수 |
| 5~15초 | `Trap` 랜덤 타이머 | 잡고 있는 동안만 흐름 |
| 1/2, 1/4, 1/9 | `ValuableTrapActivatedDiceRoll` | 희귀도 1 / 2 / 3+ 발동 확률 |
| 35 / 60 / 1초 | `Trap` 소음 | 기본 반경, 괘종시계 반경, 호출까지 유지 시간 |

## CL9 시사점

관찰만 적는다. 결정은 하지 않는다.

- **상태 enum이 하나로 통일돼 있다.** REPO는 몬스터마다 상태 머신을 새로 만들지 않고 `EnemyState` 12개를 전 몬스터가 공유하며, 개성은 "자기 enum을 하나 더 갖는 두 번째 층"과 "어떤 컴포넌트를 붙였는가"로 낸다. CL9의 닭·거위·돼지·너구리·양도 같은 축을 공유할 수 있는지(배회/조사/추격/스턴/퇴장) 살펴볼 여지가 있다. UE라면 `Enemy` 허브가 하는 `GetComponent` 스캔은 `ActorComponent` 조회로, `EnemyState*`는 상태별 컴포넌트나 `StateTree`로 대응된다(추정).
- **"잡혀도 바로 죽지 않는다(닭)"에 해당하는 구조가 REPO에도 있다.** `EnemyRigidbody.grabStun`은 플레이어가 몬스터를 잡고 있는 동안 0.1초짜리 스턴을 계속 갱신해 유지하고, `grabTimeMax`(3초)를 넘기면 몬스터가 몸을 흔들어 빠져나간다. 잡기의 결과가 즉사가 아니라 "제한 시간이 있는 무력화"라는 점이 참고가 된다.
- **읽히는 위협 — 통로 중앙으로 뭉툭하게.** REPO에서 이 인상을 만드는 것은 두 가지다. 첫째, 목적지가 자유 좌표가 아니라 `LevelPoint` 그래프의 노드이고 흔들림은 반경 몇 미터뿐이라 이동 경로가 통로 중앙을 따른다. 둘째, `EnemyStateRoaming.PlayerTurn`이 "화면에 잡힌 지 N초"가 되면 멈췄다가 플레이어 쪽으로 방향을 튼다. **의도적으로 보이도록 만든 이동**이다.
- **몬스터 스폰 지점 개념이 없는 문제.** REPO의 `LevelPoint`는 모듈 조립 시 함께 배치되는 그래프 노드이고, 배회 목적지·텔레포트 목적지·추격 놓쳤을 때의 다음 칸·추출 완료 후 유인 지점까지 **네 가지 용도를 하나의 목록이 전부 감당한다**. CL9의 모듈 조립(`docs/plan/data/module.md`)에 이 성격의 노드를 모듈마다 심는 방식이 대응될 수 있다(추정). 노드가 `ConnectedPoints`로 이웃을 알고 `Room`·`Truck`·`inStartRoom` 플래그를 갖는 점도 함께 볼 만하다.
- **일자별 몬스터 수를 표로 정하는 것과 REPO 방식의 차이.** REPO는 코드에 숫자를 두지 않고 `AnimationCurve` 세 쌍(난이도 1/2/3 × 구간 1/2)에 맡기고, 코드는 `Clamp01(levelsCompleted / 9)` 같은 x 값만 정의한다. CL9도 같은 자리에 곡선을 쓴다 (2026-09-09 정정 — 이 문단은 처음에 CL9이 숫자 표를 쓴다고 적었는데 사실이 아니다). `UCL9ProgressionDef`의 일곱 축이 전부 `FRuntimeFloatCurve`이고 에디터에서 그래프로 끌어 만진다 — 보간도 그대로 있다. REPO와 다른 것은 곡선의 x가 완료 레벨 수가 아니라 **일자**이고, REPO처럼 구간마다 곡선 세트를 갈아 끼우지 않고 하나를 10일까지 쓴 뒤 마지막 키 값으로 고정한다는 점이다.
- **몬스터가 상시 존재하지 않는다.** `EnemyParent`의 스폰/디스폰 사이클 덕분에 REPO의 맵에는 실제로 활동 중인 몬스터가 항상 소수다. 플레이어가 20m 안에 있으면 디스폰 타이머가 멈춰 눈앞에서 사라지지 않고, 소음이 크면(반경 15+) 잠든 몬스터의 재등장이 5초 앞당겨진다. 마릿수를 늘리지 않고 밀도감을 조절하는 축이다.
- **압박에 자동 상한이 있다.** `enemyActionAmount`가 120을 넘으면 모든 몬스터를 플레이어에게서 가장 먼 곳으로 유인하고 강제 이탈시킨다. 난이도 3 몬스터 한 마리 기준 60초다. "쫓기다 죽는 것 말고는 방법이 없는 상태"가 무한히 이어지지 않게 하는 장치로, 협동 추출 게임의 리듬 설계로 참고할 만하다.
- **소리는 시뮬레이션이 아니라 브로드캐스트 한 줄이다.** `SetInvestigate(위치, 반경)`이 전부이고, 벽·문·거리 감쇠는 계산하지 않는다. 대신 소음원마다 반경 상수를 손으로 정해 두었다(발소리 5, 착지 10, 물건 파괴 15, 차량 35). 구현 비용 대비 체감이 좋은 구조로 보인다.
- **호스트 판정 구조가 CL9의 리슨 서버 전제와 그대로 맞는다.** REPO는 AI·시야·체력·상태 전이를 전부 마스터에서만 돌리고, 클라이언트는 위치 보간과 연출 재생만 한다. 예외가 하나 있는데 **"몬스터가 내 화면에 보이는가"는 각 클라이언트가 계산해 RPC로 올린다**는 것이다. 로컬 카메라 정보는 호스트가 알 수 없으니 필연적인 역방향이고, CL9에서도 같은 종류의 정보(각 플레이어의 시야·화면)를 쓰려면 같은 구조가 필요하다.
- **위협이 체력 감소가 아니라 통제권 상실이다.** 28종 중 코드에 HP 감소가 직접 박힌 것은 6종뿐이고, 나머지는 넘어뜨리기(`TumbleRequest` + `OverrideEnemyHurt`), 납치(`EnemyHidden`), 시야 방해(`EnemySlowMouth`), 입력 차단과 물건 낙하(`EnemyUpscream`)로 압박한다. 특히 `EnemyUpscream`은 피해가 0인데도 위협으로 성립한다 — **들고 있던 짐을 떨어뜨리게 만드는 것 자체가 추출 게임에서는 손해**이기 때문이다. CL9의 "잡혀도 바로 죽지 않는다(닭)"와 같은 방향의 설계로 보인다.
- **감지 컴포넌트를 바꾸는 것만으로 다른 종류의 위협이 된다.** 대부분은 `EnemyVision`을 쓰지만 `EnemyThinMan`은 `EnemyOnScreen`만(보고 있으면 안 움직인다), `EnemyTricycle`은 "내 진로를 막고 있는가"만 본다. 닭·거위·돼지·너구리·양을 서로 다르게 만드는 축으로 감지 방식이 후보가 될 수 있다.
- **함정은 레벨당 두 개만 진짜다.** `TrapDirector.TrapCount = 2`이고 나머지 함정 프롭은 트리거가 제거돼 껍데기로 남는다. 배치를 늘리지 않고 경계심만 늘리는 방식이다. 또 함정의 소음 반경(기본 35, 괘종시계 60)이 발소리(5)나 물건 파괴(15)보다 훨씬 크고, `enemyDamage`가 `playerDamage`의 3~6배(300 대 50/100)로 잡혀 있어 **함정은 위험이자 무기**로 양방향에서 쓰인다.
- **함정 발동 확률이 희귀도에 반비례한다.** 희귀도 1은 1/2, 3 이상은 1/9로 터진다. 비싼 물건일수록 안전하게 만들면 "값과 위험이 비례한다"는 직관이 깨지는데, REPO는 반대로 잡았다. 값비싼 것을 들고 나오는 긴장을 확률이 아니라 부피·무게로 만들겠다는 선택으로 읽힌다(추정).
- **귀중품 드롭 시점이 죽는 순간이 아니라 디스폰 순간이다.** 그리고 최근 10초 안에 플레이어가 20m 안에 있었어야 한다. 아무도 없는 곳에서 죽은 몬스터의 보상이 허공에 쌓이지 않게 하는 조건이다. CL9에서 몬스터가 재료를 떨어뜨리는 규칙을 만든다면 같은 종류의 조건이 필요해질 수 있다(추정).
