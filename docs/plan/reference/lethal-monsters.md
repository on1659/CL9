# 참고 — Lethal Company의 몬스터와 함정

> 2026-09-08, 설치본의 `Assembly-CSharp.dll`을 디컴파일하고 에셋 값을 읽은 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](README.md).
> CL9의 대응 문서는 [../data/monster/common.md](../data/monster/common.md) ·
> [../data/progression.md](../data/progression.md)다.

Lethal Company의 몬스터는 `EnemyAI`라는 하나의 추상 클래스를 전부 상속하고, 개별 몬스터는
이 클래스가 제공하는 **상태 번호(`currentBehaviourStateIndex`)·주기적 판단(`DoAIInterval`)·
시야 검사·소리 수신·탐색 루틴** 위에 자기 로직만 얹는 방식으로 만들어져 있다. 몬스터의 개체
수는 개체 단위가 아니라 **"파워 레벨" 예산**으로 관리되며, 실내·실외·주간이라는 세 개의 예산이
서로 완전히 분리돼 있다. 실내 몬스터는 맵에 미리 배치된 **벤트(`EnemyVent`)**에서만 나오고,
게임 내 시간이 한 시간씩 흐를 때마다 다음 배치가 미리 예약된다. 아래는 디컴파일된 C# 소스를
읽고 정리한 것이며, 수치는 별도 표시가 없으면 C# 필드 초기값(= 코드 기본값)이다. 실제 게임에서
쓰이는 값은 대부분 프리팹·ScriptableObject 에셋에 저장돼 있어서 소스만으로는 알 수 없다.

---

## 공통 뼈대

### `EnemyAI` — 모든 몬스터의 부모

`EnemyAI`는 `NetworkBehaviour`를 상속한 추상 클래스다. 개별 몬스터는 이것을 상속해
`DoAIInterval()`과 `Update()`를 오버라이드하고, 부모의 `base.DoAIInterval()`을 마지막에 호출해
목적지 반영과 위치 동기화를 맡긴다.

몬스터의 종류별 설정은 코드가 아니라 `EnemyType`이라는 ScriptableObject에 들어 있고,
`EnemyAI.enemyType` 필드가 그것을 가리킨다. 즉 **"이 몬스터가 실내인가 실외인가, 스턴이
먹히는가, 문을 얼마나 빨리 여는가"는 전부 데이터**이고, 코드는 그 데이터를 읽어 행동한다.

### 상태 기계 — `EnemyBehaviourState`

상태는 정수 인덱스 하나(`currentBehaviourStateIndex`)로 표현된다. `enemyBehaviourStates`는
`EnemyBehaviourState` 배열이고, 상태 하나는 **이름 문자열, 목소리 클립, 효과음 클립, 애니메이터
파라미터 이름과 그 값** 만 들고 있다. 즉 상태 객체 자체에는 행동이 없고, 행동은 개별 AI의
`switch (currentBehaviourStateIndex)`가 담당한다.

```csharp
public void SwitchToBehaviourState(int stateIndex)
{
    SwitchToBehaviourStateOnLocalClient(stateIndex);   // 로컬에서 즉시 반영
    SwitchToBehaviourServerRpc(stateIndex);            // 서버 경유로 나머지 클라에 전파
}
```

`SwitchToBehaviourStateOnLocalClient`는 이전 상태를 `previousBehaviourStateIndex`에 저장하고,
`PlayAudioOfCurrentState()`와 `PlayAnimationOfCurrentState()`를 부른다. 애니메이션 전환은
상태 데이터의 `IsAnimTrigger` 여부에 따라 `SetTrigger` 또는 `SetBool` 중 하나로 나간다.
상태 번호의 의미는 몬스터마다 제각각이고, 이름 문자열은 프리팹에만 있으므로 소스에서는
`case 0:`, `case 1:` 같은 번호로만 읽힌다.

### 판단 주기 — `DoAIInterval`과 `AIIntervalTime`

`Update()`는 매 프레임 돌지만 무거운 판단은 하지 않는다. 대신 `updateDestinationInterval`
타이머가 0 이하로 내려갈 때마다 `DoAIInterval()`을 한 번 부르고, 타이머를 다시 채운다.

```csharp
DoAIInterval();
updateDestinationInterval = AIIntervalTime + UnityEngine.Random.Range(-0.015f, 0.015f);
```

`AIIntervalTime`은 코드 기본값 `0.2f`다. 즉 **몬스터의 "생각"은 초당 5회 정도**이고, 여기에
±0.015초의 지터를 섞어 여러 마리가 같은 프레임에 몰려 계산하는 것을 흩뜨린다.

부모의 `DoAIInterval()`이 하는 일은 딱 두 가지다. `moveTowardsDestination`이 켜져 있으면
`destination`을 `NavMeshAgent`에 밀어넣고(직전 값과 같으면 건너뛴다), 그다음 위치를 클라이언트에
동기화한다.

### 이동 목적지 — `SetDestinationToPosition`

```csharp
public bool SetDestinationToPosition(Vector3 position, bool checkForPath = false)
```

`checkForPath`가 켜져 있으면 먼저 그 위치를 NavMesh 위로 스냅한 뒤(`GetNavMeshPosition`,
반경 1.75) `agent.CalculatePath`로 경로를 계산하고, **계산된 경로의 마지막 코너가 원하는 위치에서
1.55 이상 떨어져 있으면 실패로 보고 `false`를 돌려준다**. 즉 "닿을 수 없는 곳"을 목적지로 잡는
사고를 이 한 줄이 막는다. 성공하면 `moveTowardsDestination = true`, `movingTowardsTargetPlayer = false`로
바꾸고 `destination`을 갱신한다.

플레이어를 쫓을 때는 `SetMovingTowardsTargetPlayer(player)`로 `movingTowardsTargetPlayer`를 켜고,
`Update()`가 매 프레임 `NavigateTowardsTargetPlayer()`를 부른다. 이 함수는 0.25초마다만 NavMesh
스냅을 다시 하고, 그 사이 프레임에서는 목적지의 x·z만 플레이어 좌표로 덮어써서 계산을 아낀다.
`addPlayerVelocityToDestination`이 0보다 크면 플레이어의 진행 방향으로 그만큼 앞을 조준해서,
**도망치는 플레이어를 앞질러 막는 느낌**을 만든다(Jester는 추격 상태에서 이 값을 `1f`, Hoarding
Bug는 `2f`로 올린다).

---

## 감지

몬스터가 플레이어를 알아채는 경로는 세 가지다. 시야, 소리, 그리고 접촉이다.

### 시야 — 각도·거리·프록시미티

`EnemyAI`는 시야 검사 함수를 여러 벌 제공하고, 개별 AI가 골라 쓴다.

| 함수 | 하는 일 |
|---|---|
| `CheckLineOfSightForPlayer(width, range, proximityAwareness)` | 조건을 만족하는 **첫 번째** 플레이어를 돌려준다 |
| `CheckLineOfSightForClosestPlayer(width, range, proximityAwareness, bufferDistance)` | 조건을 만족하는 플레이어 중 **가장 가까운** 쪽 |
| `GetAllPlayersInLineOfSight(...)` / `...NonAlloc(...)` | 조건을 만족하는 **모두** |
| `CheckLineOfSightForPosition(pos, width, range, proximityAwareness, overrideEye)` | 임의의 좌표가 보이는지 |
| `CheckLineOfSight(objectsToLookFor, ...)` | 임의의 오브젝트 목록 중 보이는 것 |

기본 인자는 `width = 45f`, `range = 60`, `proximityAwareness = -1`이다. 판정의 핵심은 한 줄이다.

```csharp
if (num < (float)range
    && (Vector3.Angle(eye.forward, to) < width || (proximityAwareness != -1 && num < proximityAwareness))
    && !Physics.Linecast(eye.position, position, collidersAndRoomMaskAndDefault, QueryTriggerInteraction.Ignore)
    && !Physics.Linecast(position, eye.position, collidersAndRoomMaskAndDefault, QueryTriggerInteraction.Ignore))
```

읽어야 할 점이 네 가지 있다.

첫째, **거리 판정이 각도 판정보다 먼저다.** `range`를 넘으면 정면이어도 안 보인다.

둘째, `width`는 반각(half-angle)이다. `Vector3.Angle`은 0~180을 돌려주므로 `width = 45`는
좌우 45도씩, 총 90도 원뿔이 된다.

셋째, `proximityAwareness`가 -1이 아니면 **그 거리 안에서는 각도를 무시한다.** 등 뒤에 바짝
붙어도 알아챈다는 뜻이고, 실제로 Butler는 `CheckLineOfSightForPlayer(100f, 50, 2)`처럼
"시야 100도(반각), 사거리 50, 근접 2m"를 쓴다.

넷째, 벽 판정을 **양방향 Linecast 두 번**으로 한다. 단방향 레이캐스트는 한쪽 면만 렌더되는
콜라이더에서 뚫려 보이므로, 눈→플레이어와 플레이어→눈 둘 다 막히지 않아야 보인 것으로 친다.

안개는 시야를 강제로 줄인다. 몬스터가 실외에 있고 `enemyType.canSeeThroughFog`가 꺼져 있는데
날씨가 `Foggy`이면, `range`가 `Mathf.Clamp(range, 0, 30)`으로 잘린다.

`CheckLineOfSightForClosestPlayer`의 `bufferDistance`는 **표적 전환의 히스테리시스**다. 새로
찾은 최근접 플레이어와 기존 표적의 거리 차가 `bufferDistance`보다 작으면 아예 `null`을 돌려줘서,
두 플레이어가 비슷한 거리에 있을 때 몬스터가 좌우로 흔들리는 것을 막는다. `TargetClosestPlayer`도
같은 방식으로 `bufferDistance = 1.5f`를 기본값으로 쓴다.

### 표적 선정 — `TargetClosestPlayer`

```csharp
public bool TargetClosestPlayer(float bufferDistance = 1.5f, bool requireLineOfSight = false,
    float viewWidth = 70f, bool doGroundCast = false, bool requirePath = false,
    bool checkForMineshaftStartTile = true)
```

접속한 플레이어를 순회하면서 `PlayerIsTargetable`을 통과한 사람 중 가장 가까운 쪽을
`targetPlayer`에 넣고 `true`를 돌려준다. `doGroundCast`가 켜져 있으면 플레이어 발밑으로 5m
레이캐스트를 쏴서 **바닥 위에 서 있는지**를 확인하고(사다리·공중은 제외), `requirePath`가 켜져
있으면 `PathIsIntersectedByLineOfSight`로 실제 경로가 있는지까지 본다.

`PlayerIsTargetable`이 거는 조건은 이렇다. 플레이어가 조종 중이고, 죽지 않았고, 다른 몬스터와의
연출 중이 아니고(`inAnimationWithEnemy == null`), **`playerScript.isInsideFactory != isOutside`
— 즉 몬스터와 플레이어가 같은 공간(실내/실외)에 있어야 하고**, 늪에 빠진 정도가 0.73 미만이어야
한다. 실외 몬스터는 격납고 문이 닫혀 있으면 우주선 안의 플레이어를 노릴 수 없다.

### 소리 — `INoiseListener`와 `PlayAudibleNoise`

소리는 인터페이스 하나로 처리된다.

```csharp
public interface INoiseListener {
    void DetectNoise(Vector3 noisePosition, float noiseLoudness, int timesPlayedInOneSpot, int noiseID);
}
```

`EnemyAICollisionDetect`(몬스터의 콜라이더에 붙는 컴포넌트)가 이 인터페이스를 구현해서
`mainScript.DetectNoise(...)`로 넘긴다. `EnemyAI.DetectNoise`의 기본 구현은 비어 있고,
소리를 듣는 몬스터만 오버라이드한다.

소리를 내는 쪽은 `RoundManager.PlayAudibleNoise`를 부른다.

```csharp
public void PlayAudibleNoise(Vector3 noisePosition, float noiseRange = 10f, float noiseLoudness = 0.5f,
    int timesPlayedInSameSpot = 0, bool noiseIsInsideClosedShip = false, int noiseID = 0)
```

구현은 `Physics.OverlapSphereNonAlloc(noisePosition, noiseRange, ...)`으로 반경 안의 콜라이더를
긁고, `INoiseListener`가 붙어 있는 것마다 `DetectNoise`를 호출하는 것이다. **소리는 물리 오버랩
한 번으로 끝나는 브로드캐스트이고, 벽을 통과한다.** 벽 감쇠는 듣는 쪽이 알아서 한다 —
Eyeless Dog는 `Physics.Linecast(자기 위치, 소리 위치, 256)`이 막히면 소리 크기를 절반으로 깎는다.

`noiseID`는 소리의 출처를 구분하는 정수이고, 몬스터가 무시할 소리를 걸러내는 데 쓴다. 예를 들어
Eyeless Dog는 `noiseID`가 675189, 7, 546이면 무시한다.

`timesPlayedInOneSpot`은 같은 자리에서 반복된 소리의 횟수이며, Eyeless Dog는 이것이 15를 넘으면
무시한다. **같은 소리를 계속 내면 몬스터가 익숙해진다**는 규칙이 여기 들어 있다.

`ReceiveLoudNoiseBlast(position, angle)`는 폭발음·경적처럼 방향성 있는 큰 소리를 받는 통로다.
60m 안에서 각도 35도 미만이면 크기 0.7(25m 안이면 1.0)로 `DetectNoise`를 자기 자신에게 넘긴다.

### 접촉 — `EnemyAICollisionDetect`

`EnemyAICollisionDetect`는 `OnTriggerStay`에서 `Player` 태그를 만나면 `mainScript.OnCollideWithPlayer(other)`를,
`Enemy` 태그를 만나면(그리고 `canCollideWithEnemies`가 켜져 있으면) `OnCollideWithEnemy(...)`를
호출한다. `onlyCollideWhenGrounded`가 켜진 콜라이더는 상대가 땅에 붙어 있을 때만 반응한다.

이 클래스는 `IHittable`, `INoiseListener`, `IShockableWithGun`을 한꺼번에 구현해서 **몬스터가
외부 자극을 받는 단일 창구** 역할을 한다.

### 몬스터끼리의 위협 판정 — `IVisibleThreat`

몬스터가 다른 몬스터를 인식하는 별도 체계가 있다. `IVisibleThreat`를 구현한 대상은
`ThreatType`(Player, Projectile, Bees, EyelessDog, ForestGiant, Item, RadMech, BaboonHawk,
BushWolf, GiantKiwi)과 위협도·관심도·가시성을 노출한다.

```csharp
public interface IVisibleThreat {
    ThreatType type { get; }
    int GetThreatLevel(Vector3 seenByPosition);
    int GetInterestLevel();
    Transform GetThreatLookTransform();
    Transform GetThreatTransform();
    Vector3 GetThreatVelocity();
    float GetVisibility();
    int SendSpecialBehaviour(int id);
    GrabbableObject GetHeldObject();
    bool IsThreatDead();
}
```

관찰하는 쪽은 `Threat` 객체에 마지막 목격 위치, 목격 시각, 거리, **"나를 향해 얼마나 다가왔는가"
(`distanceMovedTowardsBaboon`)**, 공격당한 적이 있는가를 누적해서 판단한다. 필드 이름이
`distanceMovedTowardsBaboon`인 것에서 알 수 있듯 이 체계는 Baboon Hawk를 위해 설계돼 다른
몬스터로 확장된 것으로 보인다(추정). `BushWolfEnemy`, `RadMechAI`, `GiantKiwiAI`가
`IVisibleThreat`를 구현한다.

### 탐색 루틴 — `AISearchRoutine`

플레이어를 잃었을 때 맵을 훑는 로직은 `AISearchRoutine`이라는 직렬화 가능한 상태 객체와
`EnemyAI`의 코루틴 두 개(`CurrentSearchCoroutine`, `ChooseNextNodeInSearchRoutine`)로 되어 있다.

`StartSearch(startOfSearch, newSearch)`가 시작점을 기록하고, **시작점의 x·z를 5 단위로 반올림한
값을 시드로 `System.Random`을 새로 만든다.** 같은 자리에서 시작한 탐색은 같은 순서를 밟는다는
뜻이고, 멀티플레이에서 소유권이 옮겨가도 경로가 크게 튀지 않게 하는 장치로 읽힌다(추정).

노드 선택 규칙은 `ChooseNextNodeInSearchRoutine`에 있다. `unsearchedNodes`를 뒤에서부터 훑되
5개마다 한 프레임씩 양보하고, 각 노드에 대해

1. `PathIsIntersectedByLineOfSight(...)`가 막히면 **그 노드를 목록에서 영구 제거**한다.
2. `onlySearchNodesInLOS`가 켜져 있는데 시작점에서 노드가 안 보이면 역시 제거한다.
3. 남은 노드 중 경로 거리(`pathDistance`)가 가장 짧은 것을 고른다.
4. `randomized`가 켜져 있으면, 더 가까운 노드를 찾아도 **65% 확률로만** 채택한다.

즉 기본은 "가장 가까운 미탐색 노드로", 무작위 모드는 "대체로 가까운 쪽이되 가끔 엉뚱한 쪽으로"다.
`searchWidth = 200f`, `searchPrecision = 5f`가 코드 기본값이고, `loopSearch`가 켜져 있으면
목록이 비었을 때 처음부터 다시 시작한다.

---

## 이동·문·벤트

### 이동

이동은 전적으로 Unity의 `NavMeshAgent`다. `EnemyAI`는 `agent.speed`를 상태별로 직접 대입하고
(예: Bracken은 회피 상태에서 `agent.speed = 6f`, 분노 상태에서
`Mathf.Clamp(agent.speed + Time.deltaTime * 1.2f, 3f, 12f)`로 서서히 가속),
목적지는 `agent.SetDestination(destination)`으로 넘긴다.

`GetLayermaskForEnemySizeLimit(enemyType)`이 `EnemyType.SizeLimit`(`NoLimit` / `MediumSpaces` /
`SmallSpaces`)에 따라 NavMesh 영역 마스크를 만든다. 즉 **큰 몬스터는 좁은 통로에 아예 진입하지
못하도록 NavMesh 레이어로 통제**하며, 스폰 위치 선정에도 같은 마스크를 쓴다.

`EnemyWaterType`(`LandOnly` / `Amphibious` / `WaterOnly`)은 물 위를 다닐 수 있는지를 정한다.
재미있는 처리가 하나 있는데, **육상 실외 몬스터가 15초 동안 1m도 못 움직이면** 물에 갇혔다고
보고 `agent.areaMask`에 Water 영역을 임시로 추가해서 빠져나오게 한다. 10초 뒤 다시 끄고
NavMesh 위 가장 가까운 지점으로 순간이동시킨다. 스택 탈출용 안전장치다.

### 문

문은 `DoorLock`의 `OnTriggerStay`에서 열린다. **서버에서만** 돌고, 잠기지 않았고 아직 닫혀 있는
문에 `Enemy` 태그가 닿아 있으면 미터를 채운다.

```csharp
enemyDoorMeter += Time.deltaTime * component.mainScript.openDoorSpeedMultiplier;
if (enemyDoorMeter > 1f) { ... OpenDoorAsEnemyServerRpc(); }
```

`openDoorSpeedMultiplier`는 `Start()`에서 `enemyType.doorSpeedMultiplier`(코드 기본값 `1f`)로
초기화되고, 상태에 따라 몬스터가 스스로 바꾼다. Thumper는 순찰 중 `0.6f`, 추격 중 `1.5f`로
올리고, Maneater와 LassoMan도 같은 패턴을 쓴다. **배수 1이면 문 하나 여는 데 정확히 1초**이고,
1.5면 0.67초다. 문을 여는 동안 몬스터가 문 앞에 붙어 있어야 하므로, 이 값이 곧 "문이 얼마나
버텨주는가"가 된다.

문이 열리고 닫힐 때 `navMeshObstacle.enabled = !isOpen`으로 NavMesh 장애물이 토글된다.
즉 닫힌 문은 경로 계산 단계에서 벽이다.

### 벤트 — 실내 몬스터의 유일한 출입구

`EnemyVent`는 맵에 미리 배치된 `NetworkBehaviour`다. 필드는 `spawnTime`(이 벤트에서 몬스터가
나올 게임 내 시각), `occupied`(예약됨), `enemyType`/`enemyTypeIndex`, `floorNode`(실제로 몬스터가
놓일 위치), 그리고 오디오·애니메이터다.

흐름은 이렇다.

1. `RoundManager.PlotOutEnemiesForNextHour()`가 비어 있는 벤트를 골라 몬스터 종류와 시각을 배정한다.
2. 서버가 `SyncVentSpawnTimeClientRpc(time, enemyIndex)`로 모든 클라에 예약을 알린다.
3. 클라의 `EnemyVent.Update()`가 `spawnTime - 현재시각 < enemyType.timeToPlayAudio`가 되면
   벤트 소리를 재생하기 시작한다. 볼륨은 남은 시간에 비례해 커지고 로우패스 필터가 풀린다.
   `timeToPlayAudio`의 코드 기본값은 `15f`다 — **몬스터가 나오기 15초 전부터 소리로 예고한다.**
4. 시각이 되면 서버의 `SpawnInsideEnemiesFromVentsIfReady()`가 `SpawnEnemyFromVent(vent)`를 부르고,
   `vent.floorNode` 위치에 몬스터를 스폰한 뒤 `OpenVentClientRpc()`로 벤트 애니메이션을 튼다.

스폰 직후 몬스터는 벤트에서 기어나오는 연출 상태에 있다. `EnemyAI.Update()`가
`timeSinceSpawn < exitVentAnimationTime + 0.005f * numberOfEnemiesInScene` 동안은 AI를 돌리지
않고 위치 동기화만 한다. `exitVentAnimationTime`의 코드 기본값은 `1f`이고, 씬에 몬스터가 많을수록
연출이 길어져서 동시 스폰 부하가 흩어진다.

### 실외 몬스터의 둥지 — `EnemyAINestSpawnObject`

실외 몬스터는 벤트 대신 둥지를 쓸 수 있다. `EnemyType.nestSpawnPrefab`이 지정돼 있으면,
`EnemyAI.Start()`에서 `RoundManager.enemyNestSpawnObjects` 목록을 뒤져 자기 종류의 둥지를 찾고,
있으면 **그 둥지 위치로 자신을 순간이동시킨다**(`UseNestSpawnObject`). `requireNestObjectsToSpawn`이
켜져 있는데 둥지를 못 찾으면 그 몬스터는 즉시 자기를 죽이거나(서버) 메시를 끈다(클라).
`useMinEnemyThresholdForNest`가 꺼져 있으면 둥지는 한 마리를 배출하고 파괴된다.

---

## 피격·스턴·죽음

### 피격

`IHittable.Hit(force, hitDirection, playerWhoHit, playHitSFX, hitID)`가 진입점이다.
`EnemyAICollisionDetect`가 이것을 받아 `mainScript.HitEnemyOnLocalClient(...)`로 넘긴다.

```csharp
// 때린 클라이언트에서 먼저 로컬로 반영하고, 서버로 보낸다
if (playerWhoHit != null) HitEnemy(force, playerWhoHit, playHitSFX, hitID);
HitEnemyServerRpc(force, playerWhoHit2, playHitSFX, hitID);
```

`HitEnemyServerRpc`는 `[ServerRpc(RequireOwnership = false)]`이므로 **아무 클라이언트나 부를 수
있고**, 서버는 이를 `HitEnemyClientRpc`로 되뿌린다. `EnemyAI.HitEnemy`의 기본 구현은 효과음만
재생하고 체력을 깎지 않는다. **체력 감소는 각 몬스터가 직접 구현한다.**

```csharp
// FlowermanAI, CrawlerAI, HoarderBugAI 등 공통 패턴
enemyHP -= force;
if (enemyHP <= 0 && base.IsOwner) { KillEnemyOnOwnerClient(); }
```

`enemyHP`의 코드 기본값은 `3`이다. 실제 값은 프리팹에 저장돼 있어 소스에서는 확인할 수 없지만,
`ButlerEnemyAI`는 `Start()`에서 `enemyHP = 2`로 덮어쓰고, `SandSpiderAI`는 남은 체력에 따라
속도를 바꾼다(`switch (enemyHP)`: 4.4 / 4.58 / 5.04 — **다칠수록 빨라진다**).
`MaskedPlayerEnemy`는 맞을 때마다 40% 확률로(체력이 1이면 확정) 달리기 상태로 전환한다.

`HitFromExplosion(distance)`는 폭발 피해를 받는 별도 통로이고 기본 구현은 비어 있다.

### 스턴

```csharp
public virtual void SetEnemyStunned(bool setToStunned, float setToStunTime = 1f,
    PlayerControllerB setStunnedByPlayer = null)
```

`enemyType.canBeStunned`가 꺼져 있거나 이미 죽었으면 무시한다. 스턴이 걸리면
`stunNormalizedTimer = setToStunTime`, `postStunInvincibilityTimer = 0.5f`가 설정된다. 이
0.5초 무적은 **연속 스턴 잠금을 막는 장치**다.

타이머는 `Update()`에서 이렇게 줄어든다.

```csharp
stunNormalizedTimer -= Time.deltaTime / enemyType.stunTimeMultiplier;
```

`stunTimeMultiplier`의 코드 기본값은 `2f`다. 배수가 클수록 스턴이 **오래 간다**(나눗셈이므로).
`stunNormalizedTimer`가 0 아래로 내려간 뒤에야 `postStunInvincibilityTimer`가
`Time.deltaTime * 5f`씩 줄기 시작한다.

`IShockableWithGun`(Zap Gun)은 다른 경로를 쓴다. `ShockWithGun`은 0.25초 스턴을 걸면서
`stunnedIndefinitely++`를 한다. 이 값이 0보다 크면 `Update()`에서 타이머를 아예 감소시키지
않으므로 **총을 겨누고 있는 동안 스턴이 무한히 유지**된다. `StopShockingWithGun`이 카운터를
내리면 그때부터 타이머가 흐른다. `stunGameDifficultyMultiplier`(코드 기본값 `1f`)는
`GetDifficultyMultiplier()`로 노출되어 감전 미니게임의 난이도가 된다.

### 죽음

```csharp
public void KillEnemyOnOwnerClient(bool overrideDestroy = false)  // 소유자만
  → KillEnemyServerRpc(destroy)  [ServerRpc(RequireOwnership = false)]
  → KillEnemyClientRpc(destroy)
  → KillEnemy(destroy)           // 각 클라에서 실제 처리
```

`KillEnemy(destroy)`는 `destroy && enemyType.canBeDestroyed`이면 서버에서 `NetworkObject.Despawn()`을
호출해 흔적 없이 지운다. 그렇지 않으면 `enemyType.canDie`를 확인하고, 스캔 노드 콜라이더를 끄고,
`isEnemyDead = true`로 바꾸고, 사망 사운드와 애니메이터 트리거(`"KillEnemy"`, `"Dead"`)를 걸고,
`SubtractFromPowerLevel()`을 부른 뒤 `agent.enabled = false`로 이동을 정지시킨다. **시체는 남는다.**

`SubtractFromPowerLevel()`은 죽은 몬스터의 `PowerLevel`을 해당 예산에서 되돌려준다.
실내/실외/주간/잡초 중 어느 통계에서 빼는지는 `EnemyType`의 플래그로 갈린다. 여기서 중요한
사실 하나 — **실내 몬스터가 죽으면 `cannotSpawnMoreInsideEnemies = false`로 리셋된다.**
즉 예산이 꽉 차서 스폰이 멈췄더라도, 플레이어가 몬스터를 죽이면 다시 스폰이 열린다.

---

## 스폰과 난이도

### 세 개의 분리된 예산

몬스터 개체 수는 `EnemyType.PowerLevel`(float)의 합으로 관리되고, 상한은 레벨
(`SelectableLevel`, 즉 행성)마다 다르다. 셋은 **완전히 독립적**이다.

| 구분 | 상한 필드 (코드 기본값) | 현재값 필드 | 목록 |
|---|---|---|---|
| 실내 | `maxEnemyPowerCount = 8` | `currentEnemyPower` / `currentEnemyPowerNoDeaths` | `SelectableLevel.Enemies` |
| 실외(야간) | `maxOutsideEnemyPowerCount = 15` | `currentOutsideEnemyPower` / `...NoDeaths` | `SelectableLevel.OutsideEnemies` |
| 주간 | `maxDaytimeEnemyPowerCount = 20` | `currentDaytimeEnemyPower` / `...NoDeaths` | `SelectableLevel.DaytimeEnemies` |
| 잡초(곰팡이) | — | `currentWeedEnemyPower` | `RoundManager.WeedEnemies` |

`...NoDeaths` 쌍이 따로 있는 이유는, **후보를 고를 때는 "죽은 것을 포함해 지금까지 쓴 예산"을
기준으로 하고, 실제로 예산이 남았는지 볼 때는 "살아 있는 것만"을 기준으로** 하기 위해서다.
`SpawnRandomOutsideEnemy`는 먼저 `...NoDeaths`로 후보가 하나라도 있는지 훑고, 하나도 없으면
그때서야 살아 있는 값(`currentOutsideEnemyPower`)으로 완화해서 다시 계산한다.

여기에 **다양성 예산**이 하나 더 있다. `EnemyType.DiversityPowerLevel`은 **종류당 한 번만**
누적되며(`hasSpawnedAtLeastOne`), `maxInsideDiversityPowerCount`/`maxOutsideDiversityPowerCount`
(둘 다 코드 기본값 100)를 넘으면 새로운 종류를 더 못 들인다. 이미 나온 종류를 더 늘리는 것은
이 제한에 걸리지 않는다. **"몇 마리"와 "몇 종류"를 따로 통제한다.**

`EnemyType.MaxCount`는 종류별 개체 수 상한이고, 예산과 무관하게 걸린다.

### 실내 — 벤트 예약(`PlotOutEnemiesForNextHour`)

한 시간 단위로 **다음 배치를 미리 예약**한다. 서버에서만 돈다.

```csharp
float num = currentLevel.enemySpawnChanceThroughoutDay.Evaluate(lengthOfHours * currentHour / totalTime);
num -= 1f;
int num2 = Mathf.RoundToInt(Mathf.Clamp(
    Mathf.Lerp(num + Mathf.Abs(TimeOfDay.Instance.daysUntilDeadline - 3) / 1.6f - currentLevel.spawnProbabilityRange,
               num + currentLevel.spawnProbabilityRange,
               (float)EnemySpawnRandom.NextDouble()),
    minEnemiesToSpawn, 20f));
```

읽는 순서는 이렇다.

1. 곡선 `enemySpawnChanceThroughoutDay`를 **정규화된 시각**(0~1)에서 평가해 기준값을 얻는다.
2. 1을 뺀다.
3. 하한은 `기준 + |마감까지 남은 날 − 3| / 1.6 − spawnProbabilityRange`,
   상한은 `기준 + spawnProbabilityRange`. 이 사이를 균등 난수로 `Lerp`한다.
   `spawnProbabilityRange`의 코드 기본값은 `3f`다.
4. `[minEnemiesToSpawn, 20]`으로 자른다.
5. `enemyRushIndex != -1`(침공 이벤트)이면 `+2`.
6. 마지막으로 **비어 있는 벤트 수**로 자른다 — 벤트가 없으면 몬스터도 없다.

`|daysUntilDeadline − 3| / 1.6`이 흥미로운데, 마감까지 3일일 때 0이고 거기서 멀어질수록 커진다.
즉 **첫날과 마지막 날이 중간보다 위험**하다.

그다음 `currentEnemyPower >= currentMaxInsidePower`이면 `cannotSpawnMoreInsideEnemies = true`로
표시하고 즉시 반환한다.

배정된 각 몬스터의 스폰 시각은

```csharp
int num4 = EnemySpawnRandom.Next((int)(10f + num3),
                                 (int)(lengthOfHours * hourTimeBetweenEnemySpawnBatches + num3));
```

`num3 = lengthOfHours * currentHour`(현재 시간대의 시작 시각)이고,
`hourTimeBetweenEnemySpawnBatches`의 코드 기본값은 `2`, `lengthOfHours`는 `100f`다.
즉 **한 배치는 현재 시각 +10초부터 +200초 사이에 흩어져 나온다.** 예약 시각 목록
(`enemySpawnTimes`)은 정렬돼 보관된다.

벤트 선택은 단순 무작위다 — 비어 있는 벤트 목록에서 `EnemySpawnRandom.Next(list.Count)`로
하나 뽑고 목록에서 제거한다. **플레이어와의 거리는 전혀 고려하지 않는다.**

### 실내 — 종류 뽑기(`AssignRandomEnemyToVent`)

후보마다 가중치를 계산하고 `GetRandomWeightedIndex`로 하나 뽑는다.

```csharp
// 기본
weight = (int)(rarity * probabilityCurve.Evaluate(t));
// useNumberSpawnedFalloff가 켜져 있으면
weight = (int)(rarity * probabilityCurve.Evaluate(t)
              * numberSpawnedFalloff.Evaluate(Mathf.Clamp(numberSpawned / 10f, 0f, 1f)));
```

`t`는 `lengthOfHours * currentHour / totalTime`, 즉 하루 중 정규화된 시각이다.
`rarity`는 `SpawnableEnemyWithRarity.rarity`로 레벨마다 0~200 범위로 지정된 정수다.
`numberSpawnedFalloff` 곡선은 **X축이 "이미 나온 마릿수 ÷ 10", Y축이 확률 배수**여서,
이미 많이 나온 종류의 확률을 스스로 낮춘다.

여기에 세 개의 보정이 붙는다.

- `increasedChanceInterior`가 현재 던전 타입과 같으면 `weight = Min(weight * 1.5, weight + 50)`.
  **특정 인테리어에서 더 자주 나오는 몬스터**를 만든다.
- `increasedInsideEnemySpawnRateIndex`와 일치하면 `weight = 100`. 이 인덱스는 라운드 시작 때
  `AnomalyRandom`으로 무작위로 하나 뽑히며, **매 판마다 "오늘 유난히 많은 몬스터"가 하나 생긴다.**
- 침공 이벤트(`enemyRushIndex`)가 켜져 있으면 대상은 `weight = 100`, 나머지는 전부
  `weight * 0.075`로 깎인다.

후보 자격은 `InsideEnemyCannotBeSpawned`가 판정한다. `spawningDisabled`가 아니고,
`PowerLevel <= 남은 예산`이고, `numberSpawned < MaxCount`이고, 새 종류라면 다양성 예산도 남아
있어야 한다.

`SelectableLevel.specialEnemyRarity`(`OverrideEnemyRarity`)는 특정 몬스터의 확률을 강제로
비틀 수 있다. `percentageChance >= 1`이면 무조건 그 몬스터를 뽑고, 0과 1 사이면 나머지 전체
가중치 합 `S`에 대해 `weight = percentageChance * S / (1 - percentageChance)`로 설정해서
**정확히 그 비율이 되도록** 맞춘다.

### 실외(야간)

```csharp
float num  = lengthOfHours * currentHour;
float num2 = (int)(outsideEnemySpawnChanceThroughDay.Evaluate(num / totalTime) * 100f) / 100f;
float num3 = num2 + Mathf.Abs(daysUntilDeadline - 3) / 1.6f;
int   num4 = Mathf.Clamp(OutsideEnemySpawnRandom.Next((int)(num3 - 3f), (int)(num2 + 3f)),
                         minOutsideEnemiesToSpawn, 20);
```

호출 전에 `currentOutsideEnemyPower > currentMaxOutsidePower`이면 아무것도 하지 않는다.
스폰 위치는 `outsideAINodes`(물 전용이면 `outsideAIWaterNodes`, 육상 전용이면 `outsideAIDryNodes`)
중 하나를 무작위로 고른 뒤 `GetRandomNavMeshPositionInBoxPredictable(node, 10f, ...)`으로 반경 10
안에서 흩고, `PositionWithDenialPointsChecked(...)`로 **금지 구역(우주선 근처 등)을 피해** 밀어낸다.

`spawnInGroupsOf`가 1보다 크면 그 수만큼 반복해서 한 자리에 무리로 낸다(예산이 먼저 마르면 중단).

### 주간

```csharp
float num2 = daytimeEnemySpawnChanceThroughDay.Evaluate(num / totalTime);
int   num3 = Mathf.Clamp(DaytimeEnemySpawnRandom.Next((int)(num2 - daytimeEnemiesProbabilityRange),
                                                      (int)(num2 + daytimeEnemiesProbabilityRange)), 0, 20);
```

`daytimeEnemiesProbabilityRange`의 코드 기본값은 `10f`로 실내(3)보다 훨씬 넓다.
주간 몬스터는 `EnemyType.normalizedTimeInDayToLeave`(코드 기본값 `1f`)를 넘긴 시각에는 후보에서
빠지고, 이미 나와 있는 개체도 `CheckTimeOfDayToLeave()`가 `DaytimeEnemyLeave()`를 불러 퇴장시킨다.
**낮 몬스터는 시간이 되면 스스로 사라진다.**

### 시간 진행과 난이도 상승

`RoundManager.Update()`는 서버에서만 돌며 두 가지를 본다.

```csharp
if (isSpawningEnemies) {
    SpawnInsideEnemiesFromVentsIfReady();
    if (timeScript.hour > currentHour && currentEnemySpawnIndex >= enemySpawnTimes.Count)
        AdvanceHourAndSpawnNewBatchOfEnemies();
}
else if (timeScript.currentDayTime > 85f && !begunSpawningEnemies) { ... BeginEnemySpawning(); }
```

즉 **게임 시작 후 85(게임 내 시간 단위)가 지나야 첫 스폰 예약이 잡힌다.** 이 유예가 착륙
직후의 안전 시간을 만든다.

`AdvanceHourAndSpawnNewBatchOfEnemies()`는 `currentHour += hourTimeBetweenEnemySpawnBatches`(2)를
하고 주간·실외·잡초 스폰을 순서대로 돌린 뒤, 실내 예약을 다시 잡는다. 그 직전에 난이도 상승
조건을 검사한다.

```csharp
if (플레이어 있음 && daysUntilDeadline <= 2
    && ( (수집액/할당량 > 0.8 && normalizedTimeOfDay > 0.3)
       || 수집액/맵총액 > 0.65
       || daysPlayersSurvivedInARow >= 5 )
    && minEnemiesToSpawn == 0)
{ minEnemiesToSpawn = 1; }
```

**잘하고 있으면 게임이 압박한다.** 마감이 2일 이내인데 할당량의 80%를 채웠거나, 맵 총액의 65%를
긁었거나, 5일 연속 생존했으면 시간당 최소 1마리 스폰이 보장된다.

### 침공 이벤트

`RefreshEnemiesList()`에서 맵 시드 기반으로 1000분의 4 확률(10월 23일에는 110분의 5)로
"침공"이 발동한다. 25% 확률로 Nutcracker 침공(`currentMaxInsidePower = 20`), 아니면 Hoarding
Bug 침공(`currentMaxInsidePower = 30`)이다. 기본 상한이 8인 것을 생각하면 **예산이 2.5~4배로
뛴다.** 침공 대상의 가중치는 100으로 고정되고 나머지는 7.5%로 깎이므로 사실상 한 종류만 나온다.

### 결정론

스폰은 전부 맵 시드 기반의 `System.Random`으로 돌아간다.

| 난수 | 시드 |
|---|---|
| `EnemySpawnRandom` (실내 종류·시각·벤트) | `randomMapSeed + 40` |
| `OutsideEnemySpawnRandom` | `randomMapSeed + 41` |
| `DaytimeEnemySpawnRandom` | `randomMapSeed + 43` |
| `WeedEnemySpawnRandom` | `randomMapSeed + 42` |
| `IndoorEnemySpawnPlacementRandom` | `randomMapSeed + 30` |
| `OutsideEnemySpawnPlacementRandom` | `randomMapSeed + 31` |
| `DaytimeEnemySpawnPlacementRandom` | `randomMapSeed + 33` |
| `AnomalyRandom` | `randomMapSeed + 5` |

---

## 실측 — 몬스터 33종의 실제 값

코드 기본값이 아니라 설치본의 `EnemyType` 에셋에서 직접 읽은 값이다. **파워**가 스폰 예산에서
차감되는 비용이고 **최대**가 한 판 동시 상한이다. 소수점이 길게 나오는 값은 유니티 float 원값이다.
 (`EnemyType` 에셋 실측)

| 이름 | 파워 | 최대 | 구역 | 무리 | 죽음 | 스턴 | 스턴배율 | 문속도 | 소음배율 | 밀치기 |
|---|---|---|---|---|---|---|---|---|---|---|
| Cadaver Bloom | 3.0 | 8 | 실내 | 0 | 죽음 | 가능 | 0.25 | 1.25 | 1.0 | 0.0 |
| Crawler | 3.0 | 4 | 실내 | 0 | 죽음 | 가능 | 0.4000000059604645 | 0.30000001192092896 | 2.0 | 0.1599999964237213 |
| Flowerman | 3.0 | 1 | 실내 | 0 | 죽음 | 가능 | 0.25 | 1.25 | 1.0 | 0.1599999964237213 |
| Jester | 3.0 | 1 | 실내 | 0 | 불사 | 가능 | 0.6000000238418579 | 0.5 | 1.0 | 0.1599999964237213 |
| Butler | 2.0 | 7 | 실내 | 0 | 죽음 | 가능 | 0.6000000238418579 | 0.5 | 1.0 | 6.699999809265137 |
| Butler Bees | 2.0 | 1 | 실내 | 0 | 불사 | 가능 | 0.6000000238418579 | 0.07999999821186066 | 1.0 | 0.10000000149011612 |
| Cadaver Growths | 2.0 | 1 | 실내 | 0 | 불사 | 불가 | 0.25 | 1.25 | 1.0 | 0.0 |
| Maneater | 2.0 | 1 | 실내 | 0 | 죽음 | 가능 | 0.25 | 1.25 | 1.0 | 0.10000000149011612 |
| Girl | 2.0 | 1 | 실내 | 0 | 불사 | 불가 | 4.0 | 1.5 | 0.10000000149011612 | 0.1599999964237213 |
| Lasso | 2.0 | 2 | 실내 | 0 | 죽음 | 가능 | 4.0 | 1.5 | 1.600000023841858 | 0.1599999964237213 |
| Red pill | 2.0 | 0 | 실내 | 0 | 죽음 | 가능 | 4.0 | 1.0 | 1.0 | 0.1599999964237213 |
| Bunker Spider | 2.0 | 1 | 실내 | 0 | 죽음 | 가능 | 1.0 | 0.15000000596046448 | 1.0 | 0.1599999964237213 |
| Blob | 1.0 | 2 | 실내 | 0 | 죽음 | 가능 | 4.0 | 0.4000000059604645 | 1.2000000476837158 | 0.1599999964237213 |
| Centipede | 1.0 | 4 | 실내 | 0 | 죽음 | 가능 | 3.0 | 0.23000000417232513 | 1.0 | 0.1599999964237213 |
| Clay Surgeon | 1.0 | 1 | 실내 | 1 | 불사 | 가능 | 1.350000023841858 | 0.30000001192092896 | 1.0 | 0.1599999964237213 |
| Hoarding bug | 1.0 | 10 | 실내 | 0 | 죽음 | 가능 | 0.30000001192092896 | 1.5 | 1.0 | 0.1599999964237213 |
| Masked | 1.0 | 10 | 실내 | 0 | 죽음 | 가능 | 0.75 | 2.0 | 0.6000000238418579 | 0.1599999964237213 |
| Nutcracker | 1.0 | 10 | 실내 | 0 | 죽음 | 가능 | 0.5 | 2.0 | 1.0 | 0.1599999964237213 |
| Puffer | 1.0 | 2 | 실내 | 0 | 불사 | 가능 | 0.6000000238418579 | 0.30000001192092896 | 1.0 | 0.1599999964237213 |
| Spring | 1.0 | 5 | 실내 | 0 | 불사 | 가능 | 3.25 | 0.05999999865889549 | 0.6000000238418579 | 0.1599999964237213 |
| Stingray | 0.5 | 6 | 실내 | 0 | 죽음 | 가능 | 1.0 | 0.800000011920929 | 0.699999988079071 | 0.1599999964237213 |
| ForestGiant | 3.0 | 3 | 실외 | 0 | 죽음 | 가능 | 1.2000000476837158 | 1.0 | 1.0 | 0.1599999964237213 |
| RadMech | 3.0 | 20 | 실외 | 0 | 불사 | 가능 | 1.2000000476837158 | 1.0 | 1.0 | 0.1599999964237213 |
| MouthDog | 2.0 | 8 | 실외 | 0 | 죽음 | 가능 | 0.699999988079071 | 0.6000000238418579 | 1.0 | 0.1599999964237213 |
| Feiopar | 2.0 | 6 | 실외 | 0 | 죽음 | 가능 | 0.30000001192092896 | 1.0 | 1.0 | 0.1599999964237213 |
| Earth Leviathan | 2.0 | 3 | 실외 | 0 | 불사 | 불가 | 5.0 | 0.5 | 1.0 | 0.1599999964237213 |
| Bush Wolf | 1.0 | 1 | 실외 | 0 | 죽음 | 가능 | 0.44999998807907104 | 0.30000001192092896 | 1.0 | 0.1599999964237213 |
| Baboon hawk | 0.5 | 15 | 실외 | 2 | 죽음 | 가능 | 0.4000000059604645 | 0.10000000149011612 | 1.0 | 0.1599999964237213 |
| GiantKiwi | 4.0 | 1 | 주간(외부) | 0 | 죽음 | 가능 | 0.5 | 1.0 | 1.0 | 0.1599999964237213 |
| Docile Locust Bees | 1.0 | 5 | 주간(외부) | 0 | 죽음 | 불가 | 1.0 | 0.30000001192092896 | 1.0 | 0.1599999964237213 |
| Manticoil | 1.0 | 16 | 주간(외부) | 0 | 죽음 | 가능 | 1.0 | 0.30000001192092896 | 1.0 | 0.1599999964237213 |
| Red Locust Bees | 1.0 | 6 | 주간(외부) | 0 | 불사 | 불가 | 1.0 | 0.30000001192092896 | 1.0 | 0.1599999964237213 |
| Tulip Snake | 0.5 | 12 | 주간(외부) | 4 | 죽음 | 불가 | 1.0 | 0.30000001192092896 | 1.0 | 0.1599999964237213 |

## 종류 목록

커뮤니티 통칭은 널리 알려진 대응만 적었고, 소스에서 확인할 수 없는 것은 "(추정)"을 붙였다.
실내/실외 구분은 `EnemyType.isOutsideEnemy`·`isDaytimeEnemy` 에셋 값이 정본이라 소스만으로는
확정할 수 없으나, 코드 맥락에서 명확한 것은 그대로 적었다. 체력은 코드에 명시된 것만 적었다
(나머지는 프리팹 값).

| 클래스 | 통칭 | 위치 | 상태 개수와 뼈대 | 감지 | 공격 |
|---|---|---|---|---|---|
| `FlowermanAI` | Bracken | 실내 | 0=배회, 1=은신·회피, 2=분노 | 시야 + **플레이어가 자기를 봤는지**(`HasLineOfSightToPosition(pos, 30f)`) | 목 꺾기 즉사(`Strangulation`) |
| `CrawlerAI` | Thumper | 실내 | 0=순찰(문 배수 0.6), 1=추격(문 배수 1.5) | `CheckLineOfSightForPlayer(55f)` / 추격 중 `(65f, 80)` | 40 대미지 (`Mauling`) |
| `SpringManAI` | Coil-Head | 실내 | 0=대기·탐색, 1=추격 | **플레이어 쪽에서** `HasLineOfSightToPosition(..., 68f)` + 수평 시야 | 90 대미지 (`Strangulation`) |
| `JesterAI` | Jester | 실내 | 0=배회, 1=태엽 감기, 2=팝업·추격 | 표적이 잡히면 `beginCrankingTimer` 시작 | 즉사 (`Mauling`) |
| `BlobAI` | Hygrodere | 실내 | 소리 추적형 | `DetectNoise` 오버라이드 | 35 대미지 |
| `HoarderBugAI` | Hoarding Bug | 실내 | 0=배회, 1=아이템 수집, 2=화남·추격 | 시야 + `DetectNoise` | 30 대미지 (`Mauling`) |
| `CentipedeAI` | Snare Flea | 실내 | 천장 매복 → 얼굴 부착 | 근접·시야 | 부착 중 10 대미지/틱 (`Suffocation`) |
| `SandSpiderAI` | Bunker Spider | 실내 | 0=대기, 1=순찰, 2=추격 | 거미줄(`SandSpiderWebTrap`) 접촉 + 시야 | 90 대미지 (`Mauling`) |
| `NutcrackerEnemyAI` | Nutcracker | 실내 | 0=순찰, 1=조준·사격 | 회전하며 시야 검사, 정지 상태에서만 발사 | 산탄총 + 근접 발차기 즉사(`Kicking`) |
| `MaskedPlayerEnemy` | Masked | 실내 | 0/1/2 (걷기·달리기·추격) | 시야 + `DetectNoise` | 즉사 (`Strangulation`) |
| `DressGirlAI` | Ghost Girl | 실내 | 0=응시·순간이동, 이후 추격 | 특정 한 명(`hauntingPlayer`)만 표적 | 즉사 (`Unknown`) |
| `ButlerEnemyAI` | Butler | 실내 | 0=순찰, 1=유인·매복 | `CheckLineOfSightForPlayer(100f, 50, 2)`, 주변 인원 수 확인 | 10 대미지 찌르기 (`Stabbing`), 죽으면 벌 소환 |
| `ButlerBeesEnemyAI` | Butler Bees | 실내 | Butler 사망 시 스폰 | 근접 | 10 대미지 (`Stabbing`) |
| `ClaySurgeonAI` | Barber | 실내 | 리듬 기반 도약 | 거리 유지 후 도약 | 즉사 (`Snipping`) |
| `CaveDwellerAI` | Maneater | 실내 | 아기 → 성체 성장형, 문 배수 1.0↔1.5 | 시야 + `DetectNoise` | 즉사 (`Mauling`) |
| `LassoManAI` | Lasso Man (추정) | 실내 | 문 배수 0.6↔1.5 | 시야 | 40 대미지 (`Strangulation`) |
| `MouthDogAI` | Eyeless Dog | 실외 | 0=배회, 1=의심, 2=격노·수색, 3=돌진 | **소리 전용** — `DetectNoise`가 유일한 각성 경로 | 즉사 (`Mauling`) |
| `ForestGiantAI` | Forest Keeper | 실외 | 0=배회, 1=추격, 2=포식 | 원거리 시야 | 집어삼켜 즉사 (`Crushing`) |
| `SandWormAI` | Earth Leviathan | 실외 | 0=지하 이동, 1=분출 | 지면 진동(플레이어 이동 감지) | 즉사 (시체 없음) |
| `BaboonBirdAI` | Baboon Hawk | 실외 | 무리·위협 판정(`Threat`) 기반 | `IVisibleThreat` 체계 + 시야 | 근접 공격, 체력 3 이하면 후퇴 |
| `RedLocustBees` | Circuit Bees | 실외 | 0=벌집 주변, 1=경계, 2=공격 | 벌집 근접 침입 | 10 대미지/틱, 누적 시 즉사 (`Electrocution`) |
| `DocileLocustBeesAI` | Roaming Locusts | 실외 | 0/1 — 흩어졌다 모임 | 근접 | 없음(무해) |
| `DoublewingAI` | Manticoil | 실외·주간 | 0=지상, 1=비행 | `DetectNoise` | 없음(무해) |
| `PufferAI` | Spore Lizard | 실내 | 0=배회, 1=경계, 2=도주·포자 | `CheckLineOfSightForPlayer(45f, 20)` / `(70f, 20, 2)` | 20 대미지 (`Mauling`), 대체로 도망 |
| `FlowerSnakeEnemy` | Tulip Snake | 실외 | 무리, 플레이어에 매달림 | 근접 | 붙어서 들어올림 |
| `RadMechAI` | Old Bird | 실외 | 대기 → 기동 → 비행·포격 | `IVisibleThreat` + 시야 | 화염방사 20/틱 → 즉사 (`Burning`), 착지 압사 70/30 (`Crushing`) |
| `BushWolfEnemy` | Kidnapper Fox | 실외 | 은신 → 혀 낚아채기 | 시야, 체력 4 미만이면 은신처로 후퇴 | 끌고 가 즉사 (`Mauling`) |
| `GiantKiwiAI` | Giant Kiwi (추정) | 실외 | 둥지·알 보호 | `IVisibleThreat` + `DetectNoise` | 10 대미지 (`Stabbing`) |
| `PumaAI` | Puma (추정) | 실외 | 나무 노드(`AllTreeNodes`)를 이용한 스토킹 | `DetectNoise` + 시야, `PumaPlayerKnowledge`로 플레이어별 기억 | 7 대미지 (`Scratching`) |
| `StingrayAI` | Stingray (추정) | 실외 | 은신처(`StingrayHidingSpot`) 기반 | 시야 | — |
| `CadaverBloomAI` | Cadaver Bloom (추정) | 실내 | 시체 기반 성장 | `DetectNoise` | 30 대미지 (`Mauling`), 포획 시 즉사 (`Suffocation`) |
| `CadaverGrowthAI` | Cadaver Growth (추정) | 실내 | 위와 한 쌍 | — | — |
| `SnowmanSimpleAI` | Snowman (추정) | — | **`EnemyAI`를 상속하지 않는 `MonoBehaviour`** — 연출용 (추정) | — | — |
| `TestEnemy` | — | — | 개발용 | — | — |

`CompanyMonster`는 몬스터 AI가 아니라 `enum { GiantHand, Tentacles, Tongue }`이다 — 회사 건물의
연출 오브젝트 종류를 고르는 값이다.

`Anomaly` / `AnomalyType`은 몬스터와 별개의 체계다. `AnomalyType`은 `anomalyMaxHealth`,
`captureDifficulty`(0~1), `difficultyVariance` 곡선, `probabilityCurve`, 그리고
**`int[] spawnableEnemies`**를 들고 있어서, 이상 현상이 특정 몬스터를 배출할 수 있게 되어 있다.

### 대표적인 기믹 세 가지

**Coil-Head (`SpringManAI`)** — "보고 있으면 멈춘다"는 규칙이 정확히 코드에 있다. 매 프레임 4명의
플레이어를 순회하면서 **플레이어 쪽 시야**(`allPlayerScripts[i].HasLineOfSightToPosition(...)`)로
검사하고, 한 명이라도 보고 있으면 `stoppingMovement = true`가 되어 `agent.speed = 0`,
`SetDestinationToPosition(현재 위치)`가 걸린다. 스턴 중에도 `flag = true`로 처리되어 멈춘다.
전환에는 `stopAndGoMinimumInterval = 0.15f`의 최소 간격이 있어 깜빡임이 걸러진다. 이 검사는
**소유 클라이언트에서만** 돌고, 소유권은 표적이 바뀔 때마다 `ChangeOwnershipOfEnemy(targetPlayer.actualClientId)`로
표적 플레이어에게 넘어간다 — 즉 **보고 있는 사람의 화면에서 판정한다.**

**Bracken (`FlowermanAI`)** — 반대 방향이다. 로컬 플레이어가 30m 안에서 자기를 보면
(`HasLineOfSightToPosition(위치 + up*0.5f, 30f)`) 상태 0에서 상태 1(은신·회피)로 전환하고
소유권을 그 플레이어에게 넘긴 뒤 `agent.speed = 0`으로 굳는다. 계속 쳐다보면 `AddToAngerMeter`가
차고, 분노 미터가 넘치면 상태 2로 넘어가 속도를 3에서 12까지 천천히 올리며 달려든다. 회피 상태의
지속 시간은 `timesFoundSneaking % 3`에 따라 11초 또는 24초다 — **세 번에 한 번은 오래 숨는다.**
분노 상태에서 스턴을 맞으면 `angerMeter = 6f`로 리셋되어 분노가 연장된다.

**Jester (`JesterAI`)** — 순수한 타이머다. 상태 0에서 표적이 잡히면 `beginCrankingTimer`가 줄고,
0이 되면 상태 1(태엽)로 간다. 상태 1에서는 `popUpTimer`가 줄고 0이 되면 상태 2다. 스턴은 두
타이머를 되감거나 멈춘다(상태 0에서는 `beginCrankingTimer -= Time.deltaTime * 15f` — 오히려
**가속**된다). 상태 2에서는 `agent.speed`가 `Mathf.Clamp(agent.speed + Time.deltaTime * 1.45f, 0f, 18f)`로
최대 18까지 오른다. 실내에 플레이어가 아무도 없으면 5초 뒤 상태 0으로 돌아간다.

---

## 맵 함정

실내 함정은 `IndoorMapHazard`(`IndoorMapHazardType` 에셋 + `numberToSpawn` 곡선)로 배치되고,
`RoundManager.SpawnMapObjects()`가 처리한다. 개수는 `numberToSpawn.Evaluate(random.NextDouble())`
— **X축 0~1을 균등 난수로 찍고 Y축을 개수로 읽는다.** 즉 곡선 모양이 곧 개수 분포다.
`increasedMapHazardSpawnRateIndex`와 일치하면 개수가 2배(최대 150)가 된다.

배치는 씬에 놓인 `RandomMapObject` 스포너 중 그 프리팹을 허용하는 것을 골라
`GetRandomNavMeshPositionInBoxPredictable(스포너 위치, spawnRange, ...)`으로 흩는다.
`requireDistanceBetweenSpawns`가 켜져 있으면 같은 종류끼리 5m 이내를 금지하고,
`spawnFacingAwayFromWall` / `spawnFacingWall` / `spawnWithBackToWall`에 따라 회전을 맞춘다.
`allowInMineshaft`가 꺼진 함정은 던전 타입 4(광산)에서 제외된다.

| 함정 | 클래스 | 동작과 수치 |
|---|---|---|
| 터렛 | `Turret.cs` | 상태 4개(`TurretMode`: Detection / Charging / Firing / Berserk). 아래 상술 |
| 지뢰 | `Landmine.cs` | 밟으면(`OnTriggerEnter`) `PressMineServerRpc`, 떼면(`OnTriggerExit`) 폭발. `SpawnExplosion(pos + up, false, killRange 5.7, damageRange 6)` — **5.7m 안 즉사(`Blast`), 6m 안 50 대미지.** 폭발은 6m 안의 다른 지뢰를 연쇄 기폭한다. 카메라 흔들림은 14m 안 Big, 25m 안 Small |
| 천장 가시 | `SpikeRoofTrap.cs` | 시드 기반으로 80% 확률로 주기 작동(`slamOnIntervals`). 주기는 10% 확률로 `random(1,35)/1.3`, 10% 확률로 `random(1,4)/1.4`, 나머지 `random(1,15)/1.3`초. 출입구 근처면 최소 1.25초. 내려올 때 플레이어 즉사(`Crushing`), **몬스터도 죽인다**(`KillEnemyOnOwnerClient`) |
| 거미줄 | `SandSpiderWebTrap.cs` | Bunker Spider가 친다. 밟으면 `isMovementHindered++`, `hinderedMultiplier *= 2.5f`. 탈출 시 `*= 0.4f`로 복원. `PlayerTripWebServerRpc`로 거미에게 알린다 |
| 늪·물 | `QuicksandTrigger.cs` | `movementHinderance = 1.6f`(이동 배수), `sinkingSpeedMultiplier = 0.15f`(가라앉는 속도). 물(`isWater`)이면 가라앉지 않고 잠수 상태만 만든다. `sinkingValue`가 0.73을 넘으면 몬스터의 표적에서 제외된다 |
| 증기 밸브 | `SteamValveHazard.cs` | `valveCrackTime` 뒤에 균열, `valveBurstTime` 뒤에 파열. 파열하면 안개가 `fogSizeMultiplier`만큼 퍼져 **시야를 가린다**. 직접 피해는 없고, 플레이어가 상호작용(`SteamValveFixInteraction`)으로 잠글 수 있다 |
| 실외 위험물 | `RoundManager.SpawnOutsideHazards()` | 시드 `randomMapSeed + 2`로 별도 배치 |

### 터렛 상세

`Turret.cs`의 상태는 `enum TurretMode { Detection, Charging, Firing, Berserk }`다.

**Detection** — 회전 속도 28, 부드러운 보간. `switchRotationTimer`가 7초를 넘으면 회전 방향을
뒤집고 `SwitchRotationClientRpc`로 동기화한다. 0.25초마다 `CheckForPlayersInLineOfSight(1.35f, angleRangeCheck: true)`로
검사하고, 살아 있는 플레이어를 찾으면 Charging으로 넘어간다. 이 검사는 **서버에서만** 돈다.

**Charging** — 회전 속도 95(급격히 조준), 감지음 재생. 1.5초 뒤 시야가 유지되고 있으면 Firing으로,
끊겼으면 표적을 놓는다. 이 1.5초가 플레이어에게 주어지는 **유일한 반응 시간**이다.

**Firing** — `PlayAudibleNoise(위치, 15f, 0.9f)`로 반경 15의 큰 소리를 낸다 —
**터렛이 몬스터를 부른다.** 0.21초마다 `CheckForPlayersInLineOfSight(3f)`로 검사해서 로컬
플레이어가 맞으면 체력이 50 초과일 때 50 대미지(`Gunshots`), 이하이면 즉사시킨다. 즉
**0.42초에 체력 100이 사라진다.** 사격 레이는 30m까지 나간다.

**Berserk** — `IHittable.Hit`으로 터렛을 때리면 진입한다(단 Firing/Berserk 중에는 무시).
표적 없이 좌우로 훑으며 계속 발사하고, `berserkTimer`가 다 되면 Detection으로 복귀한다.

회전 범위는 `rotationRange = 45f`(정면 기준 좌우 45도)이고, 이 범위를 벗어난 표적은
`TurnTowardsTargetIfHasLOS`에서 시야를 잃은 것으로 처리된다. 시체(`targetingDeadPlayer`)는
조준은 하되 각도 판정을 통과하지 못한다.

---

## 사망 원인 목록

`CauseOfDeath` enum 전체다. 순서가 곧 정수 값이다.

| # | 값 | 뜻 |
|---|---|---|
| 0 | `Unknown` | 불명 (Ghost Girl) |
| 1 | `Bludgeoning` | 타격 |
| 2 | `Gravity` | 추락 (Forest Keeper가 떨어뜨림 포함) |
| 3 | `Blast` | 폭발 (지뢰) |
| 4 | `Strangulation` | 질식·목 조름 (Bracken, Coil-Head, Masked) |
| 5 | `Suffocation` | 호흡 곤란 (Snare Flea) |
| 6 | `Mauling` | 물어뜯김 (Thumper, Jester, Eyeless Dog, Bunker Spider) |
| 7 | `Gunshots` | 총상 (터렛) |
| 8 | `Crushing` | 압사 (천장 가시, Forest Keeper, Old Bird) |
| 9 | `Drowning` | 익사 |
| 10 | `Abandoned` | 버려짐 (배가 떠난 뒤 남음) |
| 11 | `Electrocution` | 감전 (Circuit Bees) |
| 12 | `Kicking` | 발차기 (Nutcracker) |
| 13 | `Burning` | 소사 (Old Bird 화염) |
| 14 | `Stabbing` | 자상 (Butler) |
| 15 | `Fan` | 팬에 갈림 |
| 16 | `Inertia` | 관성 (차량 급정거) |
| 17 | `Snipping` | 가위질 (Barber) |
| 18 | `Scratching` | 할큄 (Puma) |

---

## 네트워크

### 스폰은 서버 전용

`SpawnEnemyGameObject`는 첫 줄에서 `if (!base.IsServer) return ...;`로 즉시 빠져나온다.
클라이언트가 스폰이 필요하면 `SpawnEnemyOnServer(...)`가 `SpawnEnemyServerRpc`로 넘긴다.
실제 스폰은 `Instantiate` 후 `NetworkObject.Spawn(destroyWithScene: true)` 한 줄이고,
Netcode for GameObjects가 나머지를 복제한다. `PlotOutEnemiesForNextHour`, `SpawnEnemiesOutside`,
`SpawnDaytimeEnemiesOutside`, `RoundManager.Update()`의 스폰 루프는 전부 `IsServer` 가드가 붙어 있다.

### AI 계산은 "소유자"에서

여기가 이 게임의 특징적인 부분이다. **몬스터의 AI는 서버가 아니라 `NetworkObject`의 소유자
클라이언트에서 돈다.**

```csharp
public virtual void Update()
{
    ...
    if (!base.IsOwner)
    {
        if (currentSearch.inProgress) StopSearch(currentSearch);
        SetClientCalculatingAI(enable: false);          // agent.enabled = false
        base.transform.position = Vector3.SmoothDamp(base.transform.position, serverPosition,
                                                     ref tempVelocity, syncMovementSpeed);
        base.transform.eulerAngles = ... Mathf.LerpAngle(..., targetYRotation, 15f * Time.deltaTime) ...;
        return;
    }
    ...
}
```

비소유자는 `NavMeshAgent`를 아예 꺼버리고(`SetClientCalculatingAI(false)`), 받은
`serverPosition`을 향해 `SmoothDamp`로 보간만 한다. `syncMovementSpeed`의 코드 기본값은 `0.22f`다.

소유권은 `ChangeOwnershipOfEnemy(newOwnerClientId)`로 옮긴다. 서버면 직접
`thisNetworkObject.ChangeOwnership(...)`을 부르고, 클라이언트면 `ChangeEnemyOwnerServerRpc`로
요청한 뒤 서버가 `ChangeEnemyOwnerClientRpc`로 알린다. `currentOwnershipOnThisClient`가 같은
값이면 중복 호출을 걸러낸다.

**소유권을 표적 플레이어에게 넘기는 것이 이 게임의 기본 패턴이다.** Coil-Head는 표적이 바뀔 때,
Bracken은 로컬 플레이어에게 발각됐을 때, Eyeless Dog은 소리를 들었을 때 소유권을 옮긴다.
이렇게 하면 **"보고 있으면 멈춘다" 같은 판정이 그 사람의 화면에서 지연 없이 일어난다.**
표적이 없을 때는 0번 플레이어(=호스트)에게 돌려주는 코드가 여럿 보인다.

### 위치 동기화 — `NetworkTransform`이 아니다

`NetworkTransform` 컴포넌트를 쓰지 않고 직접 RPC를 보낸다.

```csharp
public void SyncPositionToClients()
{
    if (Vector3.Distance(serverPosition, base.transform.position) > updatePositionThreshold)
    {
        serverPosition = base.transform.position;
        UpdateEnemyPositionRpc(serverPosition);   // [Rpc(SendTo.NotMe)]
    }
    ...
}
```

`updatePositionThreshold`의 코드 기본값은 `1f`다. 즉 **1m 넘게 움직였을 때만** 위치를 보낸다.
`SyncPositionToClients()`는 `DoAIInterval()` 끝에서 불리므로 최대 초당 5회다.

회전은 따로다. `Update()`에서 y축 회전이 직전 값과 6도 넘게 차이 나면 `short`로 캐스팅해
보낸다 — 서버면 `UpdateEnemyRotationClientRpc`, 클라이언트면 `UpdateEnemyRotationServerRpc`다.

### 플레이어 피해는 로컬 권한

플레이어에게 피해를 주는 코드는 전부 `GameNetworkManager.Instance.localPlayerController`를
대상으로 한다. 터렛도, 지뢰도, 천장 가시도 마찬가지다.

```csharp
GameNetworkManager.Instance.localPlayerController.DamagePlayer(50, hasDamageSFX: true,
    callRPC: true, CauseOfDeath.Gunshots);
```

즉 **"내가 맞았는가"는 내 클라이언트가 판정하고**, `callRPC: true`로 그 결과를 전파한다.
몬스터가 플레이어를 죽이는 연출도 같은 구조다.

### 대표 RPC 목록

| RPC | 종류 | 용도 |
|---|---|---|
| `SwitchToBehaviourServerRpc` / `SwitchToBehaviourClientRpc` | ServerRpc(소유권 불필요) / ClientRpc | 상태 전환 전파 |
| `HitEnemyServerRpc` / `HitEnemyClientRpc` | ServerRpc(소유권 불필요) / ClientRpc | 피격 전파 |
| `KillEnemyServerRpc` / `KillEnemyClientRpc` | ServerRpc(소유권 불필요) / ClientRpc | 사망 전파 |
| `ChangeEnemyOwnerServerRpc` / `ChangeEnemyOwnerClientRpc` | ServerRpc / ClientRpc | 소유권 이전 |
| `UpdateEnemyPositionRpc` | `[Rpc(SendTo.NotMe)]` | 위치 동기화 |
| `UpdateEnemyRotationServerRpc` / `UpdateEnemyRotationClientRpc` | ServerRpc / ClientRpc | 회전 동기화 |
| `SyncAgentMaskSwimmingRpc` | `[Rpc(SendTo.NotMe)]` | 물 영역 마스크 토글 |
| `SpawnEnemyServerRpc` / `DespawnEnemyServerRpc` | ServerRpc | 스폰·디스폰 요청 |
| `SyncVentSpawnTimeClientRpc` / `OpenVentClientRpc` | ClientRpc | 벤트 예약·개방 |
| `ExplodeMineServerRpc` / `ExplodeMineClientRpc` | ServerRpc / ClientRpc | 지뢰 폭발 |
| `SwitchTargetedPlayerClientRpc` / `SetToModeClientRpc` / `EnterBerserkModeServerRpc` | — | 터렛 상태 |

---

## 수치

전부 C# 필드 초기값(코드 기본값)이다. 실제 게임 값은 프리팹·에셋에 있다.

### `EnemyAI`

| 필드 | 값 | 뜻 |
|---|---|---|
| `AIIntervalTime` | `0.2f` | `DoAIInterval` 호출 주기(초). ±0.015 지터 |
| `enemyHP` | `3` | 체력 |
| `syncMovementSpeed` | `0.22f` | 비소유자 위치 보간 `SmoothDamp` 시간 |
| `updatePositionThreshold` | `1f` | 이만큼 움직여야 위치 RPC를 보냄(m) |
| `exitVentAnimationTime` | `1f` | 벤트 탈출 연출 시간. `+0.005 × 씬 내 몬스터 수` |
| `moveTowardsDestination` | `true` | — |
| `currentOwnershipOnThisClient` | `-1` | — |
| 스턴 무적 | `0.5f` | `postStunInvincibilityTimer`, 해제 속도는 `deltaTime × 5` |
| 회전 동기화 임계 | `6f` | 도(degree) |

### `EnemyType` (ScriptableObject) 전체 필드

| 필드 | 코드 기본값 | 뜻 |
|---|---|---|
| `enemyName` | — | 이름 |
| `probabilityCurve` | — | X=정규화 시각(0~1), Y=확률 가중치 |
| `spawningDisabled` | `false` | 스폰 차단 |
| `spawnFromWeeds` | `false` | 곰팡이 양에 비례해 확률 상승 |
| `numberSpawnedFalloff` | — | X=이미 나온 수÷10, Y=`probabilityCurve`에 곱할 배수 |
| `useNumberSpawnedFalloff` | `false` | 위 곡선 사용 여부 |
| `spawnInGroupsOf` | `0` | 무리 크기(1 미만이면 1로 취급) |
| `requireNestObjectsToSpawn` | `false` | 둥지 없으면 스폰 실패 |
| `enemyPrefab` | — | 프리팹 |
| `PowerLevel` | `0` | 전역 예산에 더할 값 |
| `DiversityPowerLevel` | `0` | 다양성 예산. 종류당 한 번만 |
| `MaxCount` | `0` | 이 종류의 개체 수 상한 |
| `numberSpawned` | `0` | 런타임 카운터 |
| `nestsSpawned` | `0` | 런타임 카운터 |
| `hasSpawnedAtLeastOne` | `false` | 다양성 예산 중복 방지 |
| `isOutsideEnemy` | `false` | 실외 몬스터 |
| `isDaytimeEnemy` | `false` | 주간 몬스터 |
| `increasedChanceInterior` | `-1` | 이 던전 타입에서 확률 ×1.5(최대 +50) |
| `normalizedTimeInDayToLeave` | `1f` | 이 시각을 넘으면 퇴장(주간) |
| `stunTimeMultiplier` | `2f` | 클수록 스턴이 오래 감 |
| `doorSpeedMultiplier` | `1f` | 1이면 문 하나에 1초 |
| `stunGameDifficultyMultiplier` | `1f` | 감전 미니게임 난이도 |
| `canBeStunned` | `true` | — |
| `canDie` | `true` | — |
| `canBeDestroyed` | `true` | 디스폰 가능 |
| `destroyOnDeath` | `false` | 죽으면 시체 없이 사라짐 |
| `canSeeThroughFog` | `false` | 꺼져 있으면 안개 시 사거리 30으로 제한 |
| `disableAnimatorWhenFar` | `false` | 최적화 |
| `pushPlayerForce` | `0.16f` | 플레이어를 밀어내는 힘 |
| `pushPlayerDistance` | `1.26f` | 밀어내기 반경 |
| `SizeLimit` | `NoLimit` | `NoLimit`/`MediumSpaces`/`SmallSpaces` — NavMesh 마스크 |
| `EnemySize` | `Tiny` | `Tiny`/`Giant`/`Medium` |
| `WaterType` | `LandOnly` | `LandOnly`/`Amphibious`/`WaterOnly` |
| `timeToPlayAudio` | `15f` | 스폰 몇 초 전부터 벤트 소리 |
| `loudnessMultiplier` | `1f` | 벤트 소리 배수 |
| `overrideVentSFX` | `null` | 전용 벤트 효과음 |
| `nestSpawnPrefab` | `null` | 둥지 프리팹 |
| `nestSpawnPrefabWidth` | `2f` | 둥지 폭 |
| `nestDistanceFromShip` | `-1f` | 우주선에서 떨어뜨릴 거리 |
| `useMinEnemyThresholdForNest` | `false` | 켜면 둥지 하나를 여럿이 공유 |
| `minEnemiesToSpawnNest` | `0` | 위와 짝 |
| `hitBodySFX` / `hitEnemyVoiceSFX` / `deathSFX` / `stunSFX` | — | 효과음 |

### `SelectableLevel` (몬스터 관련만)

| 필드 | 코드 기본값 |
|---|---|
| `maxEnemyPowerCount` | `8` |
| `maxOutsideEnemyPowerCount` | `15` |
| `maxDaytimeEnemyPowerCount` | `20` |
| `maxInsideDiversityPowerCount` | `100` |
| `maxOutsideDiversityPowerCount` | `100` |
| `spawnProbabilityRange` | `3f` |
| `daytimeEnemiesProbabilityRange` | `10f` |
| `spawnEnemiesAndScrap` | `true` |
| `Enemies` / `OutsideEnemies` / `DaytimeEnemies` | `List<SpawnableEnemyWithRarity>` (rarity 0~200) |
| `specialEnemyRarity` | `OverrideEnemyRarity` (몬스터 + 0~1 확률) |
| `enemySpawnChanceThroughoutDay` / `outsideEnemySpawnChanceThroughDay` / `daytimeEnemySpawnChanceThroughDay` | `AnimationCurve` |

### `RoundManager` / `TimeOfDay`

| 필드 | 코드 기본값 | 뜻 |
|---|---|---|
| `hourTimeBetweenEnemySpawnBatches` | `2` | 배치 간격(시간 단위) |
| `minEnemiesToSpawn` | `0` | 압박 조건 충족 시 `1`로 상승 |
| `lengthOfHours` | `100f` | 한 시간의 길이 |
| `numberOfHours` | `7` | 하루 시간 수 |
| `globalTimeSpeedMultiplier` | `1f` | — |
| 스폰 시작 시각 | `currentDayTime > 85f` | 착륙 후 유예 |

### `AISearchRoutine`

| 필드 | 코드 기본값 |
|---|---|
| `searchWidth` | `200f` |
| `searchPrecision` | `5f` |
| `loopSearch` | `true` |
| `randomized` | `false` (켜면 더 가까운 노드를 65%만 채택) |
| `onlySearchNodesInLOS` | `false` |

---

## CL9 시사점

여기서는 결정을 내리지 않고, "양계장 습격"의 현재 상황과 겹쳐 보이는 지점만 적는다.

**하나. `EnemyType`에 해당하는 데이터 자산이 이 게임에서는 코드보다 먼저 온다.** Lethal Company는
"실내인가 실외인가, 스턴이 먹히는가, 문을 얼마나 빨리 여는가, 하루 중 언제 나오는가"를 전부
ScriptableObject 한 장에 몰아넣었고, C# 코드는 그것을 읽기만 한다. CL9에서 닭·거위·돼지·너구리·양의
차이를 어디에 둘 것인가 — UE에서는 `UPrimaryDataAsset` 또는 `UDataTable`이 대응 지점이 된다.

**둘. 상태가 정수 하나이고, 상태 데이터에는 행동이 없다.** `EnemyBehaviourState`가 들고 있는 것은
이름·소리·애니메이터 파라미터뿐이고, 행동은 `switch (currentBehaviourStateIndex)`가 한다. 이 구조는
"상태를 바꾸면 소리와 애니메이션이 자동으로 따라온다"를 공짜로 얻으면서도, 행동 트리나
스테이트 머신 에셋 없이 C#만으로 끝난다. CL9의 "잡혀도 바로 죽지 않는다(닭)"는 상태 전이
그래프에서 **잡힘 상태와 사망 상태 사이에 상태가 하나 더 있다**는 뜻이 된다.

**셋. 예산이 개체 수가 아니라 파워 레벨 합이라는 점.** CL9는 "일자별 몬스터 수는 progression 표로
정한다"고 되어 있는데, Lethal Company는 마릿수 대신 `PowerLevel` 합의 상한을 행성마다 두고
`MaxCount`로 종류별 상한을 따로 건다. 여기에 다양성 예산이 하나 더 있어서 "몇 마리"와 "몇 종류"를
분리해 통제한다. 소·대형 몬스터가 섞인 구성에서 마릿수만으로는 난이도가 안 맞는다는 문제를
이렇게 푼 것으로 보인다.

**넷. "몬스터 스폰 지점 개념이 아직 없다"에 대응하는 것이 `EnemyVent`다.** 벤트는 맵에 미리
놓인 오브젝트이고, 스폰은 그중 **비어 있는 것 하나를 균등 무작위로** 고르는 것이 전부다.
플레이어와의 거리도, 시야도 보지 않는다. 대신 **15초 전부터 소리로 예고**하고, 예약 시각을
클라이언트에 미리 뿌려서 모두가 같은 소리를 듣게 한다. "읽히는 위협"이라는 CL9의 방향과
정확히 같은 지점이다 — 위치를 공정하게 뽑는 대신, 나오기 전에 알려준다.

**다섯. AI 계산 주체가 서버가 아니라 소유자 클라이언트다.** CL9는 "호스트=GameMode 서버 전용
판정"으로 잡혀 있는데, Lethal Company는 스폰만 서버가 하고 **AI는 표적 플레이어의 클라이언트로
소유권을 넘겨서** 돌린다. "보고 있으면 멈춘다" 같은 즉각적 반응을 지연 없이 만들기 위한 선택으로
읽히고, 대가로 위치는 1m 임계 + `SmoothDamp` 보간이라는 거친 동기화를 감수한다. UE의 리슨 서버
모델에서 이것에 해당하는 것은 `SetOwner` + 클라이언트 권한 이동인데, 서버 전용 판정과는 정면으로
다른 방향이다.

**여섯. 플레이어 피해 판정이 전부 로컬이다.** 터렛·지뢰·몬스터 모두 `localPlayerController`에게만
피해를 주고 `callRPC: true`로 전파한다. 서버가 히트를 판정하지 않는다.

**일곱. 감지의 기본 단위가 세 개뿐이다.** 시야(각도+거리+양방향 Linecast+근접 예외), 소리
(`OverlapSphere` 브로드캐스트 + `noiseID` 필터 + 반복 횟수 감쇠), 접촉(`OnTriggerStay`). CL9의
동물들도 이 세 가지 이상이 필요할지, 아니면 더 적어도 되는지가 초기 결정 지점으로 보인다.
특히 소리 쪽의 `timesPlayedInOneSpot`("같은 자리에서 몇 번째 소리인가")은 플레이어가 소음으로
몬스터를 낚는 전략을 무디게 만드는 장치인데, 협동 추출 게임에서 반복적으로 쓰이는 패턴이다.

**여덟. "통로 중앙으로 뭉툭하게 움직인다"에 대응하는 것은 `NavSizeLimit`이다.** Lethal Company는
몬스터 크기를 NavMesh 영역 마스크로 강제해서, 큰 몬스터가 애초에 좁은 통로의 경로를 계산하지
못하게 한다. UE에서는 `UNavArea` + `FNavAgentProperties`(에이전트 반경별 별도 NavMesh)가
대응 지점이다.

**아홉. 난이도 상승이 시간이 아니라 성과에 반응한다.** `minEnemiesToSpawn = 1`로 올리는 조건이
"마감 2일 이내 + (할당량 80% 달성 / 맵 총액 65% 수거 / 5일 연속 생존)"이다. 그리고
`|마감까지 남은 날 − 3| / 1.6`이 스폰 수 하한을 밀어올려서 첫날과 마지막 날이 더 위험해진다.
일자별 표만으로 정하면 이런 반응성은 나오지 않는다.

**열. 죽인 몬스터가 예산을 돌려준다.** `SubtractFromPowerLevel()`이 `PowerLevel`을 빼고,
실내의 경우 `cannotSpawnMoreInsideEnemies = false`로 스폰을 다시 연다. 즉 **몬스터를 죽이면
다음 몬스터가 온다.** 청소가 보상이 아니라 순환이 되는 구조다.
