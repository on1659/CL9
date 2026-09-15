# 참고 — Lethal Company의 게임 루프·쿼터·플레이어·멀티

> 2026-09-08, 설치본의 `Assembly-CSharp.dll`을 디컴파일해 읽은 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](../README.md).
> CL9의 대응 문서는 [../loop.md](../../plan/loop.md) · [../economy.md](../../plan/economy.md) ·
> [../data/progression.md](../../plan/data/progression.md) · [../data/rules.md](../../plan/data/rules.md)다.

디컴파일된 C# 소스를 읽고 정리한 초안이다. Lethal Company의 한 판은 "궤도의 우주선에서 레버를 당겨
달에 착륙 → 시간이 흐르는 동안 스크랩을 모음 → 레버를 당기거나 자정이 되어 이륙 → 하루 결산 →
다시 궤도"라는 고리를 반복하고, 그 고리 위에 "며칠 안에 얼마를 벌어라"는 쿼터가 얹혀 있다.
판정은 호스트(서버)가 세계를, 소유 클라이언트가 자기 캐릭터를 맡는 식으로 갈라져 있으며,
플레이어 위치조차 서버 검증 없이 소유 클라이언트가 RPC로 통보한다.
아래 수치 중 C# 필드 초기값은 모두 "코드 기본값"으로 표시했다. 프리팹·에셋에서 덮어썼을 수 있다.

## 게임 루프

핵심 클래스는 `StartOfRound`(라운드·우주선·플레이어 관리)와 `TimeOfDay`(시간·쿼터), 두 개다.
둘 다 `NetworkBehaviour`이고, 상태 전이는 거의 전부 서버가 시작해 ClientRpc로 퍼뜨린다.

전체 흐름은 다음과 같다.

```
[궤도 정박 inShipPhase=true]
  → StartMatchLever.PullLever() → StartGame()
  → (클라면 StartOfRound.StartGameServerRpc) → StartOfRound.StartGame()  ※서버 전용
  → 씬 Additive 로드 + OpenShipDoors() → RoundManager.LoadNewLevel(seed, level)
  → openingDoorsSequence(): inShipPhase=false, currentDayTimeStarted=true, shipHasLanded=true
[달 위 — 시간이 흐름]
  → (누가 레버를 당김) EndGameServerRpc → EndGameClientRpc → ShipLeave()
     또는 (자정/전멸) ShipLeaveAutomatically() → gameOverAnimation() → ShipLeave()
  → ElevatorAnimationEvents.ElevatorFullyRunning(): 밖에 있던 사람은 Abandoned 사망
  → ShipHasLeft() → unloadSceneForAllPlayers() ※서버
  → EndOfGameClientRpc(bodiesInsured, daysPlayersSurvived, connectedPlayers, scrapCollected)
[하루 결산]
  → EndOfGame(): 결과 화면 → 아이템 정리 → XP → ReviveDeadPlayers() → ApplyPenalty → PassTimeToNextDay()
  → 서버 분기:
      쿼터 달성  → TimeOfDay.SetNewProfitQuota() + AllPlayersHaveRevivedClientRpc()
      마감 초과  → FirePlayersAfterDeadlineClientRpc()  ※게임오버
      그 외      → AllPlayersHaveRevivedClientRpc()
  → SetShipReadyToLand(): inShipPhase=true, AutoSaveShipData()
[다시 궤도 정박]
```

### 출발

`StartMatchLever.StartGame()`이 다섯 가지 가드를 통과해야 출발한다. `travellingToNewLevel`이 아니고,
`inShipPhase`이며, `beganLoadingNewLevel`이 아니고, 씬이 하나뿐이고 던전 생성 중이 아니며,
접속 인원이 2명 이상(`singlePlayerEnabled`이면 1명도 허용)이어야 한다. 마지막으로
`fullyLoadedPlayers.Count >= connectedPlayersAmount + 1`, 즉 전원이 로딩을 마쳤어야 한다.
하나라도 어긋나면 `CancelStartGame()`으로 레버가 원위치로 돌아간다.

서버의 `StartOfRound.StartGame()`은 첫 출발일 때 `GameNetworkManager.LeaveLobbyAtGameStart()`를 부르고
`gameHasStarted = true`로 세운다. **이 순간부터 로비가 닫혀 늦은 참가가 영구히 막힌다.**
시드는 `ChooseNewRandomMapSeed()`가 `Random.Range(1, 100000000)`으로 뽑고, 챌린지 파일이면
주차 번호에서 결정론적으로 만든다.

### 착륙과 이륙

`openingDoorsSequence()`는 `inShipPhase = false`로 바꾸고, `currentLevel.planetHasTime`이면
`TimeOfDay.currentDayTimeStarted = true`와 `movingGlobalTimeForward = true`를 켜서 시간을 흐르게 한다.
문이 열린 뒤 약 5초에 행성 소개 애니메이션, 그로부터 10초 뒤 쿼터 HUD, 다시 4초 뒤에야
`shipHasLanded = true`가 되어 레버로 이륙할 수 있게 된다.

이륙은 `EndGameServerRpc(playerClientId)`가 받는다. 서버는 `shipHasLanded && !shipLeftAutomatically &&
(!shipIsLeaving || playerClientId == 0)`일 때만 승인한다. 즉 이미 자동 이륙이 시작됐으면 호스트(0번)만
다시 개입할 수 있다.

엘리베이터 애니메이션이 완전히 돌면 `ElevatorAnimationEvents.ElevatorFullyRunning()`이 실행되고,
여기서 로컬 플레이어가 `isInElevator`가 아니면 그 자리에서 죽는다.

```csharp
GameNetworkManager.Instance.localPlayerController.KillPlayer(
    Vector3.zero, spawnBody: false, CauseOfDeath.Abandoned);
```

`spawnBody: false`이므로 **밖에 남은 사람은 시체조차 남기지 않는다.** 회수할 시신이 없으니
사망 페널티도 최대치로 물게 된다. 안에 있던 사람은 `ForcePlayerIntoShip()`으로 좌석 위치로 옮겨진다.

### 하루 결산과 아이템 소실

`unloadSceneForAllPlayers()`(서버)가 씬을 내리고 전원 로딩을 기다린 뒤 결산 값을 만든다.
연속 생존 일수는 사망자가 0명이고 그날 찾은 스크랩 가치가 30을 넘을 때만 +1, 아니면 0으로 초기화된다.
그날 수익은 `livingPlayers != 0`일 때만 `GetValueOfAllScrap(onlyScrapCollected: true, onlyNewScrap: true)`로
계산되고, **전멸이면 무조건 0**이다.

아이템 정리는 `RoundManager.DespawnPropsAtEndOfRound()`가 한다. 디스폰 조건은
`despawnAllItems || (!isHeld && !isInShipRoom) || deactivated || (StartOfRound.Instance.allPlayersDead
&& itemProperties.isScrap)`이다. 평소에는 우주선 방 안에 있거나 손에 들고 있는 물건만 살아남는다.
전원 사망이면 우주선 안에 있던 스크랩까지 전부 사라지지만, 스크랩이 아닌 장비(손전등, 삽 등)는 남는다.
살아남은 물건은 `scrapPersistedThroughRounds = true` 표시가 붙는다.

## 하루의 시간

`TimeOfDay`는 전역 시계 `globalTime` 하나를 굴리고, 각 달은 그 시계에 오프셋과 배속을 곱해
자기 시각을 얻는다.

```csharp
public float CalculatePlanetTime(SelectableLevel level)
    => (globalTime + level.OffsetFromGlobalTime) * level.DaySpeedMultiplier % (totalTime + 1f);
```

`totalTime = lengthOfHours * numberOfHours`이고 코드 기본값은 100 × 7 = 700이다. 시각은
`hour = (int)(currentDayTime / lengthOfHours)`로, 정규화 값은 `normalizedTimeOfDay = currentDayTime / totalTime`로
쓴다. 하루가 시작할 때 `globalTimeAtEndOfDay = globalTime + (totalTime - currentDayTime) / DaySpeedMultiplier`를
미리 계산해 두고, `MoveGlobalTime()`에서 그 값으로 클램프한다. 시계가 앞으로 간 만큼
`timeUntilDeadline`에서 그대로 빼기 때문에, **달의 배속이 빠를수록 같은 실시간에 마감이 덜 소모된다.**

하루 위상은 `GetDayPhase(t)`가 정규화 시각으로 나눈다. 0.9 이상이 Midnight, 0.63 이상이 Sundown,
0.33 이상이 Noon, 그 아래가 Dawn이다. 이 값이 조명 밝기와 시계 아이콘, 그리고 광기 상승 속도에 쓰인다.

### 강제 이륙

정규화 시각이 0.9를 넘으면 `shipLeavingSoonDialogue`가 낭독되고 HUD에 조기 이륙 아이콘이 켜진다.
서버는 `normalizedTimeOfDay >= shipLeaveAutomaticallyTime`이 되는 순간 `SetShipToLeaveOnMidnightClientRpc()`를
보내 `StartOfRound.ShipLeaveAutomatically(leavingOnMidnight: true)`를 실행한다. 이 임계값은 필드 기본값이
0.996이지만 하루가 시작될 때 코드가 0.998로 다시 세팅한다. 자정 이륙이 시작되면 `gameOverAnimation()` 코루틴이
`shipHasLanded`를 기다렸다가 레버를 원위치시키고 `ShipLeave()`를 부른다. 밖에 있던 사람은 앞서 말한 대로
`Abandoned`로 죽는다.

죽은 사람은 조기 이륙에 투표할 수 있다. `SetShipLeaveEarlyServerRpc()`는 표를 세고,
필요 표수를 `connectedPlayersAmount + 1 - livingPlayers`, 즉 **죽은 사람 수**로 잡는다. 죽은 사람이
전부 투표하면 이륙 시각이 `normalizedTimeOfDay + 0.1`로 앞당겨지고, "WARNING! Please return by ..."
대사가 나온다. 산 사람은 투표에 참여하지 않는다.

우주선 적재량이 한계에 닿으면 `ShipFullCapacityMidnightClientRpc()`가 "물건을 빼기 전에는 이륙할 수 없다"고
경고한다.

### 회사(Gordion)

회사 행성은 `currentLevel.planetHasTime == false`이고 `levelID == 3`이다. 시간이 흐르지 않으므로
자정 강제 이륙도, 하루 결과 화면도, XP 정산도 일어나지 않는다(`EndOfGameClientRpc`와 `EndOfGame()` 곳곳이
`planetHasTime` 조건으로 감싸여 있다). `PassTimeToNextDay()`의 마감 차감도
`if (currentLevel.planetHasTime || TimeOfDay.Instance.daysUntilDeadline <= 0)` 안에 있어서,
**마감이 아직 남아 있는 동안 회사 방문은 날을 소모하지 않는다.** 마감 당일(`daysUntilDeadline <= 0`)에는
예외적으로 처리된다.

## 쿼터와 경제

### 쿼터 갱신 공식

`QuotaSettings`의 코드 기본값은 `startingQuota = 300`, `startingCredits = 60`, `deadlineDaysAmount = 4`,
`increaseSteepness = 4f`, `baseIncrease = 200f`, `randomizerMultiplier = 1f`이다. `randomizerCurve`는
`AnimationCurve`라 에셋 값이 소스에 없다.

`TimeOfDay.SetNewProfitQuota()`(서버 전용)의 계산은 이렇다.

```csharp
timesFulfilledQuota++;
int 초과분 = quotaFulfilled - profitQuota;
float m = Mathf.Clamp(1f + timesFulfilledQuota * (timesFulfilledQuota / increaseSteepness), 0f, 10000f);
float r = Mathf.Clamp(Random.Range(0f, 1f) - luckValue * 1.5f, 0f, 1f);
m = baseIncrease * m * (randomizerCurve.Evaluate(r) * randomizerMultiplier + 1f);
profitQuota = (int)Mathf.Clamp(profitQuota + m, 0f, 1E+09f);
```

증가 배수 `m`은 달성 횟수의 제곱에 비례한다(`n + n²/4`). `increaseSteepness`가 클수록 완만해진다.
`luckValue`는 우주선에 놓인 가구들의 `luckValue` 합(−0.5 ~ 1 클램프)이고, 이 값이 랜덤 시드를 낮은 쪽으로
밀어 증가폭을 줄인다. 어떤 가구가 놓여 있었는지는 `furniturePlacedAtQuotaStart`로 쿼터 시작 시점에 스냅샷을 뜬다.

초과 달성 보너스는 별도 공식이다.

```csharp
int overtimeBonus = 초과분 / 5 + 15 * daysUntilDeadline;
```

초과분의 5분의 1에, 남긴 날 하루당 15크레딧이 더해진다. 마감 직전에 겨우 채우는 것보다 일찍 채우는 편이
보상이 크다. 이 값은 `SyncNewProfitQuotaClientRpc(newProfitQuota, overtimeBonus, fulfilledQuota)`로
전파되고, 클라 쪽에서 `groupCredits += overtimeBonus`, `quotaFulfilled = 0`,
`timeUntilDeadline = totalTime * deadlineDaysAmount`, `companyBuyingRate = 0.3f`,
그리고 상점 가구 라인업 회전(`RotateShipDecorSelection()`)까지 한다.
(서버 쪽 코드는 `timeUntilDeadline = totalTime * 4f`로 상수가 박혀 있는데, `deadlineDaysAmount`
기본값이 4라서 결과는 같다.)

### 매입률

`SetBuyingRateForDay()`는 마감이 가까울수록 회사가 후하게 쳐주도록 만든다.

```csharp
daysUntilDeadline = (int)Mathf.Floor(timeUntilDeadline / totalTime);
if (daysUntilDeadline == 0) { companyBuyingRate = 1f; return; }
float min = 0.3f;
float step = (1f - min) / quotaVariables.deadlineDaysAmount;
companyBuyingRate = step * (deadlineDaysAmount - daysUntilDeadline) + min;
```

기본값(min 0.3, deadlineDaysAmount 4)을 대입하면 남은 날 3일 = 0.475, 2일 = 0.65, 1일 = 0.825,
마감 당일 = 1.0이다. 이 값은 `OnDayChanged()`에서 하루가 넘어갈 때마다 갱신된다.

판매는 `DepositItemsDesk`가 처리한다. 카운터 위 스크랩 값을 더한 뒤 `(int)(합계 * companyBuyingRate)`가
실수령액이고, `terminal.groupCredits += 수령액`과 함께 `TimeOfDay.quotaFulfilled += profit`,
`gameStats.scrapValueCollected += profit`이 올라간다. 서버가 계산해
`SellItemsClientRpc(itemProfit, newGroupCredits, itemsSold, buyingRate)`로 매입률까지 같이 내려보낸다.
스크랩이 아니어도 `itemId == 14`인 물건은 팔린다(추정: 시체 등 특수 품목).

### 해고와 페널티

마감이 지났는데 쿼터를 못 채웠으면 서버가 `FirePlayersAfterDeadlineClientRpc(endGameStats, abridgedVersion)`을
보낸다. 이어지는 `playersFiredGameOver()` 연출은 5초 뒤 경보와 조명 소등, 9.37초 뒤 궤도에서 문이 열리며
진공에 빨려 나가는 장면, 6초 뒤 "PLAYERS FIRED" 화면이다. 이 과정에서 `Terminal.ClearBoughtItems()`로
산 물건이 사라지고, 서버는 `GameNetworkManager.ResetSavedGameValues()`로 세이브를 초기화한 뒤
`StartOfRound.ResetShip()`을 부른다.

`ResetShip()`은 `globalTime = 100`, `profitQuota = startingQuota`, `quotaFulfilled = 0`,
`timesFulfilledQuota = 0`, `timeUntilDeadline = totalTime * deadlineDaysAmount`, `companyBuyingRate = 0.3f`,
`groupCredits = startingCredits`, 행성을 `defaultPlanet`으로 되돌리고 `randomMapSeed++`를 한다.
가구와 곰팡이 상태도 리셋된다. 즉 **해고가 유일한 진짜 게임오버**다.

사망 페널티는 결산 화면에서 `HUDManager.ApplyPenalty(playersDead, bodiesInsured)`가 매긴다.
기준은 페널티 적용 전 크레딧이고, 시신을 회수하지 못한 사망자 1명당 그 기준의 20%,
회수한 사망자 1명당 20 / 2.5 = 8%를 뺀다. 하한은 `Mathf.Min(현재 크레딧, 60)`이라
크레딧이 60 이상이면 60 밑으로는 떨어지지 않는다.

### 결과 화면 등급

`HUDManager.FillEndGameStats()`가 등급을 매긴다. 전멸이면 계산 없이 즉시 `F`다. 아니면 점수 0에서
시작해서, 전원 생존이면 +1, 사망자가 2명을 넘으면 −1, 수거율(`scrapCollectedInLevel / totalScrapValueInLevel`)이
0.99 이상이면 +2, 0.6 이상이면 +1, 0.25 이하면 −1을 더한다. 최종 점수가 −1이면 D, 0이면 C, 1이면 B,
2면 A, 3이면 S다.

XP는 `SetPlayerLevel(isDead, mostProfitable, allPlayersDead)`가 계산한다. 생존 +10 / 사망 −3,
그날 최고 수익자면 +15, 전멸이면 −5. 결과가 양수일 때만 수거율을 곱해 스케일한다.
직급 이름은 `PlayerLevel`(`levelName`, `XPMin`, `XPMax`, `badgeMesh`) 배열에서 가져온다.

`EndOfGameStats`는 `daysSpent`, `scrapValueCollected`, `deaths`, `allStepsTaken`과
플레이어별 `PlayerStats`(`profitable`, `turnAmount`, `jumps`, `stepsTaken`, `damageTaken`,
`isActivePlayer`, `playerNotes`)를 담는다. 노트는 `WritePlayerNotes()`가 채우고 결과 화면에 최대 3줄까지 나온다.

`StoryLog`는 맵에 흩어진 수집형 로그다. 주우면 `HUDManager.GetNewStoryLogServerRpc(storyLogID)`로
`Terminal.unlockedStoryLogs`에 등록되고, 이미 읽은 로그는 `Start()`에서 메시 · 콜라이더를 꺼 버린다.

## 게임오버와 전멸

전멸은 게임오버가 아니다. `KillPlayerServerRpc` 안에서 `livingPlayers--` 후 0이 되면
`allPlayersDead = true`와 `ShipLeaveAutomatically()`가 실행되고, 우주선이 자동으로 떠난다.
결과는 등급 F, XP −5, 그날 수익 0, 우주선 안 스크랩까지 전부 소실이다. 하지만 크레딧·쿼터·마감은
그대로 남고 하루만 소모된다. 그 뒤 `ReviveDeadPlayers()`가 전원을 체력 100으로 되살리고
좌석 위치로 텔레포트시킨다. 되살리는 과정에서 치명 부상, 출혈, 잠김, 중독, 취기, 젖음, 무게 등
거의 모든 상태 플래그가 초기화되고, 들고 있지 않은 랙돌 시체 오브젝트는 디스폰되며,
모든 `DeadBodyInfo` 게임오브젝트가 파괴된다.

## 플레이어

`GameNetcodeStuff.PlayerControllerB`(339KB) 한 클래스가 이동·체력·인벤토리·관전·상호작용을 전부 들고 있다.
아래 필드 값은 모두 코드 기본값이다.

### 이동

기본 속도는 코드 필드 초기값이 `movementSpeed = 0.5f`이지만, **플레이어 프리팹에 직렬화된
실값은 4.6이다** (2026-09-15 에셋 실측 — `sharedassets3.assets`의 프리팹 원본과 `level3`의
인스턴스 4개가 전부 4.6, `climbSpeed`도 코드 4f가 아니라 3.0). 1 유닛 = 1 m라 걷기 약 4.6 m/s,
질주는 여기에 아래 `sprintMultiplier`(최대 2.25)가 곱해진다. 실제 속도는 무게로 나누는 데서 출발한다.

```csharp
float speed = movementSpeed / carryWeight;
if (sinkingValue > 0.73f) speed = 0f;
else {
    if (isCrouching) speed /= 1.5f;
    else if (criticallyInjured) speed *= limpMultiplier;
    if (movementHinderedPrev > 0) speed /= 2f * hinderedMultiplier;
    if (poison > 0f) speed *= 0.75f;
    // 경사 보정: speed = Mathf.Max(speed * 0.8f, speed + slopeIntensity * slopeModifier);
}
```

무게 감속(`carryWeight`)은 다른 분석 영역이므로 여기서는 "속도를 나누는 제수이자
스태미나 소모의 곱셈 계수"라는 점만 적어 둔다. 늪에 잠기는 값 `sinkingValue`가 0.73을 넘으면
속도가 0이 되어 완전히 갇힌다.

스프린트 배수는 즉시 붙지 않는다. 달리는 동안 `sprintMultiplier`가 `Mathf.Lerp(..., 2.25f, dt * 1f)`로
천천히 올라가고, 손을 떼면 `Mathf.Lerp(..., 1f, 10f * dt)`로 빠르게 돌아온다. 가속에는 시간이 걸리지만
감속은 즉각적이라는 뜻이다.

### 스태미나

`sprintTime = 5f`가 기준이다. `num3`는 취기 보정 배수(맨정신이면 1)다.

```csharp
if (isSprinting)
    sprintMeter -= Time.deltaTime / sprintTime * carryWeight * num3;   // 달릴 때
else if (isMovementHindered > 0 && isWalking)
    sprintMeter -= Time.deltaTime / sprintTime * num3 * 0.5f;          // 늪 등에서 걸을 때
else if (!isWalking)
    sprintMeter += Time.deltaTime / (sprintTime + 4f) * num3;          // 서 있을 때
else
    sprintMeter += Time.deltaTime / (sprintTime + 9f) * num3;          // 걸을 때
```

빈손(`carryWeight = 1`)으로는 5초를 달리면 게이지가 바닥나고, 서서 쉬면 9초, 걸으면서 회복하면 14초가 걸린다.
**무게는 소모 속도에 그대로 곱해진다** — 무게가 2배면 달릴 수 있는 시간이 절반이다.

달리기 조건은 `sprintMeter > 0.1f`, 이동 방해 없음, 치명 부상 아님이다. 게이지가 0.3 미만인 상태에서
새로 달리려 하면 `isExhausted = true`가 되어 아예 달릴 수 없고, 회복해서 0.2를 넘어야 풀린다.
0.1 밑으로 떨어져도 탈진한다. 즉 "바닥까지 쓰면 한동안 못 뛴다"가 규칙이다.

피해를 받으면 스태미나가 조금 찬다. `damageNumber >= 10`일 때 `sprintMeter += damageNumber / 125f`다.
30 대미지면 0.24, 아드레날린처럼 도망칠 여유를 준다.

### 체력과 치명 부상

`health`는 최대 100이고, `DamagePlayer()`는 `base.IsOwner`가 아니면 즉시 반환한다 —
**피해 판정은 전적으로 맞은 사람의 클라이언트에서 일어난다.**

```csharp
if (health - damageNumber <= 0 && !criticallyInjured && damageNumber < 50)
    health = 5;
else
    health = Mathf.Clamp(health - damageNumber, 0, 100);
```

치명 부상 상태가 아니고 피해량이 50 미만이면 체력이 5로 살아남는다. 즉 **첫 치명타는 죽이지 못한다.**
50 이상의 피해(낙하 50/80/100, 폭발 등)는 이 보호를 무시한다.

체력이 10 미만이 되면 `MakeCriticallyInjured(true)`가 호출되어 `criticallyInjured`와 `bleedingHeavily`가
켜지고 `Limp` 애니메이션이 붙는다. 이때부터 달릴 수 없고 이동 속도에 `limpMultiplier`가 곱해진다.

회복은 `health < 20`일 때만 초당 1씩 진행되고(`healthRegenerateTimer = 1f`), 20에 닿으면
`MakeCriticallyInjured(false)`로 부상이 풀린다. **20 이상에서는 자연 회복이 없다** — 회복 아이템이 필요하다.

### 낙하 대미지

`PlayerHitGroundEffects()`가 착지 순간 `fallValueUncapped`로 판정한다.

| 조건 | 대미지 |
|---|---|
| `fallValueUncapped < -48.5` | 100 (즉사) |
| `fallValueUncapped < -45` | 80 |
| `fallValueUncapped < -40` | 50 |
| `fallValue < -38` | 30 |

`fallValue`는 매 프레임 `-38f * dt`로 감소하며 `[-150, jumpForce]`로 클램프된다(`jumpForce = 5f`).
`fallValueUncapped`는 클램프 없이 `-26f * dt` 또는 `-38f * dt`로 계속 떨어지므로, 아주 긴 낙하에서만
−45 이하가 나온다. 사인은 `CauseOfDeath.Gravity`다.

`fallValue < -16`이면 반경 7의 소음(`RoundManager.PlayAudibleNoise`)이 발생해 적이 반응한다.
`-9` 이하면 착지 소리가 나고, 웅크리지 않은 채 `-9` 이하로 착지하면 미끄러짐 처리가 붙는다.
y좌표가 −600 밑으로 떨어지면 `StartOfRound.LateUpdate()`가 `CauseOfDeath.Gravity`로 즉사시킨다.

### 죽음과 시체

`KillPlayer(bodyVelocity, spawnBody, causeOfDeath, deathAnimation, positionOffset, setOverrideDropItems)`도
소유자만 실행한다. 여기서 모델을 끄고, 무게·중독·취기·잠김·물속 상태를 전부 리셋하고,
`SpawnDeadBody(...)`로 시체를 만들고, 오디오 리스너와 카메라를 관전 카메라로 옮긴 뒤
`KillPlayerServerRpc(...)`를 보낸다.

서버 쪽에서는 `livingPlayers--`를 하고, 시체를 안 남기는 경우(`spawnBody == false`) 인벤토리의
모든 `ItemSlots`와 `ItemOnlySlot`을 디스폰해 버린다. 시체를 남기면
`ragdollGrabbableObjectPrefab`을 스폰해 **다른 사람이 들어 옮길 수 있는 시체 오브젝트**를 만든다
(`deathAnimation != 9`인 경우).

`DeadBodyInfo`는 랙돌 본체다. `bodyParts`(Rigidbody 배열), `attachedLimb`/`attachedTo`로 적이 물고
끌고 다니는 상태, `grabBodyObject`(주울 수 있는 형태), `isInShip`(우주선 안 여부 — 보험 회수 판정),
`canBeGrabbedBackByPlayers`, `causeOfDeath`, `detachedHead`(머리 분리), `bodyBleedingHeavily`,
`parentedToShip`, 물에 빠져 죽으면 떠오르는 처리 등을 갖는다. `isInShip`은 매 프레임
`parentedToShip || (grabBodyObject가 들려 있고 든 사람이 isInElevator)`로 갱신된다.
**시체 자체에는 판매 가치가 없고**, 우주선으로 회수하면 사망 페널티를 20%에서 8%로 줄여 주는 것이 보상이다.
`GetBodiesInShip()`이 그 개수를 세어 `EndOfGameClientRpc`의 `bodiesInsured`로 넘긴다.

`PlayerPhysicsRegion`은 시체가 움직이는 발판 위에 있을 때 `SetPhysicsParent()`로 붙여 준다.

### 관전

죽은 뒤에는 `SetWhoToSpectate()`가 매 프레임(소유자, `isPlayerDead`, `!overrideSpectateCamera` 조건) 돈다.
`spectatedPlayerScript`가 비어 있으면 `SpectateNextPlayer(getClosest: true)`가 **죽은 자리에서 가장 가까운
살아 있는 플레이어**를 고른다. 그 다음부터는 인덱스를 순환하며 다음 사람으로 넘긴다.

관전 중인 사람이 죽으면 `spectatedPlayerDeadTimer`가 1.5초에 도달할 때까지 시체를 비추다가 다음 사람으로
넘어간다(전멸 상태에서는 타이머가 3배 빨리 흐르고, 게임오버 카메라로 전환한다).
카메라는 `RaycastSpectateCameraAroundPivot()`가 피벗에서 뒤로 1.4m 레이캐스트를 쏴서, 벽에 막히면
`hit.distance - 0.25f` 지점에 놓고 아니면 1.3m 뒤에 놓는다.
`isInGameOverAnimation > 0`인 동안에는 자기 시체(`deadBody.bodyParts[0]`)를 비춘다.

### 공포와 광기

두 값의 성격이 다르다. `fearLevel`은 `StartOfRound`에 하나만 있는 **로컬 플레이어 전용 연출 값**이고,
`insanityLevel`은 각 `PlayerControllerB`가 들고 있는 상태다.

`fearLevel`은 적이나 상황이 `IncreaseFearLevelOverTime(amountMultiplier, cap)`으로 조금씩 올리거나
`JumpToFearLevel(target)`으로 즉시 끌어올린다. 아무도 올리지 않은 프레임에는 `StartOfRound.LateUpdate()`에서
살아 있으면 초당 0.055, 죽었으면 초당 0.5씩 내려간다. 0.6을 넘으면 `timeSinceFearLevelUp`이 리셋되는데,
이 값은 (`TimeOfDay`의 광고 재생 조건에서) "지금 플레이어가 여유로운가"를 판단하는 데 쓰인다.

`insanityLevel`은 `SetPlayerSanityLevel()`이 관리한다. 우주선 단계이거나 하루가 시작되기 전이면 0으로 고정된다.
17m 안에 다른 플레이어가 없고 워키토키로도 목소리가 안 들리면 오르고, 있으면
`insanitySpeedMultiplier = -3f`로 빠르게 내려간다.

| 상황 | `insanitySpeedMultiplier` |
|---|---|
| 시설 내부에서 혼자 | 0.8 |
| 우주선 안에서 혼자 | 0.2 |
| 싱글플레이 (`connectedPlayersAmount == 0`) | −2 (오히려 내려감) |
| 일몰 이후 바깥에서 혼자 | 0.5 |
| 그 외 바깥에서 혼자 | 0.3 |
| 근처에 다른 플레이어가 있음 | −3 |

실제 변화는 `Mathf.MoveTowards(insanityLevel, maxInsanityLevel, dt * multiplier * 0.6f)`이고
`maxInsanityLevel = 50f`다. 즉 시설에서 혼자면 50에 닿는 데 약 104초가 걸린다.
싱글플레이면 배수가 다시 절반이 된다.

### 물과 익사

얼굴이 물속(`underwaterCollider.bounds.Contains(gameplayCamera.transform.position)`)이면
`StartOfRound.drowningTimer`가 `dt / 12f`로 줄어든다. 0 밑으로 내려가면 타이머를 1로 되돌리고
`KillPlayer(Vector3.zero, spawnBody: true, CauseOfDeath.Drowning)`을 부른다. **잠수 가능 시간은 12초**다.
0.3 이하가 되면 "Oxygen critically low!" 상태 경고가 뜬다. 물 밖에서는 `+= Time.deltaTime`로
1까지(하한 0.1) 회복되므로 회복이 소모보다 12배 빠르다.
얼굴이 잠긴 사실은 `SetFaceUnderwaterServerRpc()`로 서버에 알려 음성 필터에 반영한다.

`PlayerInfection`은 별도 상태 묶음이다. `infected`, `severe`, `emittingSpores`, `bloomOnDeath`,
`healing`, `infectionMeter`, `multiplier`, `burstMeter`, 등에 자라는 꽃(`backFlowers`와 그 스캔 노드),
얼굴 포자 VFX, 그리고 `hinderingPlayerMovement`를 갖는다. CadaverBloom 계열 적과 연결된 것으로 보인다(추정).

### 입력 액션

`PlayerActions`에 정의된 액션 이름은 다음과 같다.

| 분류 | 액션 |
|---|---|
| 이동 | `Move`, `Look`, `Jump`, `Sprint`, `Crouch` |
| 상호작용 | `Interact`, `Use`, `ActivateItem`, `InspectItem`, `QEItemInteract` |
| 인벤토리 | `SwitchItem`, `Discard`, `ReloadBatteries` |
| 정보 | `PingScan` |
| UI | `OpenMenu`, `EnableChat`, `SubmitChat`, `Delete` |
| 음성 | `VoiceButton` |
| 우주선 | `BuildMode`, `ConfirmBuildMode` |
| 디버그 | `SetFreeCamera`, `SpeedCheat` |

### 스캔

`HUDManager.PingScan_performed`는 `CanPlayerScan()`(특수 상호작용 애니메이션 중이 아니고 죽지 않았을 것)이고
`playerPingingScan <= -1f`일 때만 `playerPingingScan = 0.3f`로 스캔을 시작한다. 쿨다운이 있는 셈이다.

노드 수집은 `AssignNewNodes()`가 한다.

```csharp
int num = Physics.SphereCastNonAlloc(
    new Ray(camera.position + camera.forward * 20f, camera.forward),
    20f, scanNodesHit, 80f, 4194304);
```

카메라 앞 20m 지점에서 정면으로 반지름 20의 구를 80m 날린다. 레이어 마스크 4194304는 레이어 22(스캔 노드)다.
결과 배열은 25개로 고정(`scanNodesHit = new RaycastHit[25]`)이고, 화면에 붙일 수 있는 UI 요소 수만큼만 쓴다.
`nodeType`이 1이나 2인 노드를 먼저 시도해 중요한 것(스크랩)이 우선 표시되게 한다.

개별 노드의 통과 조건은 `MeetsScanNodeRequirements()`다.

```csharp
float d = Vector3.Distance(player.position, node.position);
if (d < node.maxRange && d > node.minRange) {
    if (node.requiresLineOfSight)
        return !Physics.Linecast(camera.position, node.position, 134217984, QueryTriggerInteraction.Ignore);
    return true;
}
return false;
```

`ScanNodeProperties`의 코드 기본값은 `maxRange = 7`, `minRange = 5`, `requiresLineOfSight = true`이고,
표시 내용은 `headerText`, `subText`, `scrapValue`, 도감용 `creatureScanID`, 분류용 `nodeType`이다.
**너무 가까워도(minRange 미만) 스캔되지 않는다**는 점이 특징이다.
`nodeType == 2`는 스크랩이고, 화면에 있는 동안 값이 `totalScrapScanned`에 합산되어 "이 근처에 얼마어치"가 표시된다.

HUD는 스태미나를 `sprintMeterUI.fillAmount = sprintMeter`로, 체력을 `UpdateHealthUI(health)`와
바이저 균열 `SetCracksOnVisor(health)`로, 무게를 `StartOfRound.SendChangedWeightEvent()`가 쏘는
이벤트로 각각 따로 그린다.

## 우주선

우주선 안 구역 판정은 `StartOfRound.LateUpdate()`에서 **로컬 플레이어에 대해서만** 계산한다.
`shipBounds.bounds`에 들어 있으면 `isInElevator`, 그 안쪽의 `shipInnerRoomBounds`까지 들어가면
`isInHangarShipRoom`이 켜진다. 판정은 `thisController.isGrounded`일 때만 갱신되고, 상태가 바뀌면
`SetAllItemsInElevator(isInHangarShipRoom, isInElevator)`로 들고 있는 물건의 소속까지 같이 바꾼다.
이 플래그가 하루 끝 아이템 소실 판정(`isInShipRoom`)의 근거가 되므로 사실상 "물건을 우주선에 넣었는가"를
결정하는 핵심 상태다.

`PlayerPhysicsRegion`은 움직이는 발판(차량 등) 위에 플레이어·시체·아이템을 붙이는 트리거다.
`physicsTransform`을 부모로 지정하고, `maxTippingAngle`(기본 180°)을 넘게 기울면
`IsPhysicsRegionActive()`가 false가 되어 콜라이더를 끈다. 리전이 파괴되면 붙어 있던 플레이어의 부모를 풀고,
아이템은 `RoundManager.mapPropsContainer`로 옮긴 뒤 들려 있지 않은 것은 `FallToGround()`한다.
우선순위(`priority`)와 아이템 드롭 허용(`allowDroppingItems`) 플래그가 있다.

`AutoParentToShip`은 이름과 달리 실제 부모-자식 관계를 쓰지 않는다. `Awake()`에서 우주선
`elevatorTransform` 기준 로컬 오프셋을 계산해 두고, 매 `LateUpdate()`마다 `MoveToOffset()`으로
우주선 위치·회전에 오프셋을 다시 적용한다. **매 프레임 상대 좌표를 재계산해 붙이는 방식**이다.
해고 연출 중에는 `StartSuckingOutOfShip()`이 이 갱신을 멈추고
`middleOfSpaceNode` 방향으로 밀어낸다.

`HangarShipDoor`는 문 전력을 관리한다. 문이 닫혀 있고 착륙 상태면 `doorPower`가
`Time.deltaTime / doorPowerDuration`(20초)씩 줄고, 0이 되면 서버가 문을 강제로 열면서
`SetShipDoorsOverheatServerRpc()`로 과열 상태를 만든다. 열려 있으면 `dt / (20 * 0.22)`,
약 4.4초 만에 다시 100%로 찬다. 즉 **문은 최대 20초만 닫아 둘 수 있다.**
전력은 `doorPowerDisplay`에 퍼센트로 표시되고, 화면은 보고 있는 플레이어가 우주선 안에 있을 때만 켜진다.

`ShipLights.ToggleShipLights()`는 로컬에서 먼저 상태를 뒤집고 `SetShipLightsServerRpc` →
`SetShipLightsClientRpc`로 전파하는, 이 게임에서 반복되는 "낙관적 로컬 적용 + 서버 중계" 패턴을 쓴다.
`ShipAlarmCord`는 줄을 잡고 있는 동안 근/원거리 경적을 울리고 `PullCordServerRpc`/`StopPullingCordServerRpc`로
동기화한다. `ShipTeleporter`는 세션당 하나만 존재하고(`hasBeenSpawnedThisSession`), 쿨다운을 초 단위로 표시하며
역방향 텔레포터는 `inShipPhase`가 아닐 때만 쓸 수 있다. `ShipBuildModeManager`는 가구 배치 모드로,
고스트 오브젝트를 미리 보여 준 뒤 `PlaceShipObjectServerRpc → PlaceShipObjectClientRpc`로 확정하고
그 위치·회전을 세이브에 남긴다.

## 저장

Easy Save 3(`ES3`)를 쓴다. 파일은 슬롯 3개(`LCSaveFile1/2/3`)와 챌린지 전용 `LCChallengeFile`,
그리고 계정 전역 `LCGeneralSaveData`로 나뉜다. 선택된 슬롯 번호는 `SelectedFile` 키로 전역 파일에 저장된다.

**저장하는 쪽은 호스트뿐이다.** `SaveGameValues()`는 첫머리에서 `if (!isHostingGame) return;`으로
클라이언트를 걸러낸다(챌린지 결과 기록만 그 앞에서 처리된다). 저장 시점은 하루가 끝나 궤도로 돌아온
`SetShipReadyToLand()`의 `AutoSaveShipData()`와, 게임을 떠나는 경로다.

| 파일 | 키 | 내용 |
|---|---|---|
| 슬롯 | `GroupCredits` | 팀 크레딧 |
| 슬롯 | `ProfitQuota` / `QuotaFulfilled` / `QuotasPassed` | 목표액 / 달성액 / 통과 횟수 |
| 슬롯 | `DeadlineTime` | 남은 마감 시간(초 단위 값) |
| 슬롯 | `CurrentPlanetID` | 현재 선택된 달 |
| 슬롯 | `RandomSeed` | 맵 시드 |
| 슬롯 | `ShownAdThisQuota`, `LuckyFurniture` | 광고 표시 여부, 행운 가구 목록 |
| 슬롯 | `shipGrabbableItemIDs` / `shipGrabbableItemPos` / `shipScrapValues` / `shipItemSaveData` | 우주선 안 아이템 종류·좌표·값·개별 저장값 |
| 슬롯 | `UnlockedShipObjects`, `ShipUnlockMoved_*`, `ShipUnlockStored_*`, `ShipUnlockPos_*`, `ShipUnlockRot_*` | 구매한 가구와 배치 |
| 슬롯 | `StoryLogs`, `EnemyScans` | 수집한 로그, 도감 |
| 슬롯 | `VehicleWarrantyTicket`, `AttachedVehicleID` | 차량 관련 |
| 슬롯 | `Stats_DaysSpent` / `Stats_Deaths` / `Stats_ValueCollected` / `Stats_StepsTaken` | 누적 통계 |
| 슬롯 | `Level{id}Mold`, `Level{id}MoldOrigin`, `Level{id}DestroyedMold` | 달별 곰팡이 확산 상태 |
| 슬롯 | `Level{id}TimesSavescumming` | 그 달에서 중간 저장한 횟수 |
| 슬롯 | `Reimburse`, `FileGameVers` | 환불액, 저장 당시 게임 버전 |
| 전역 | `SelectedFile`, `PlayerLevel`, `PlayerXPNum`, `FinishedShockMinigame`, `TimesLanded` | 슬롯 선택, 직급·XP, 튜토리얼, 착륙 횟수 |

세이브 스커밍 방지 장치가 있다. 달 위에서(`!inShipPhase`) 저장이 일어나면 `Level{id}TimesSavescumming`을
1 올리고, 그 값이 5 이상이면서 그 달이 `canSpawnMold`면 곰팡이를 퍼뜨린다.

불러오기는 `StartOfRound.SetTimeAndPlanetToSavedSettings()`가 시드·행성·쿼터·마감·누적 통계를 되살린 뒤
`SetBuyingRateForDay()`와 `UpdateProfitQuotaCurrentTime()`을 호출한다. 아이템은
`LoadShipGrabbableItems()`가 스폰하는데, 저장된 좌표가 `shipBounds` 밖이면 1번 스폰 지점 근처로 보정한다.
스크랩 값(`SetScrapValue`)과 아이템별 저장값(`LoadItemSaveData`)도 각각 복원한다.
`daysSpent == 0`이면 첫날 애니메이션을 재생한다.

해고되면 `ResetSavedGameValues()`가 `GlobalTime` 100, `QuotaFulfilled` 0, `QuotasPassed` 0,
`ProfitQuota` = 시작값, `DeadlineTime` = `totalTime * deadlineDaysAmount`, `GroupCredits` = 시작 크레딧,
`CurrentPlanetID` 0으로 되돌리고, 아이템·가구·곰팡이 키를 지우고 `RandomSeed`를 +1 한다.
**전멸은 세이브를 초기화하지 않는다** — 스크랩만 사라진 채 크레딧·쿼터·마감이 그대로 저장된다.

## 멀티플레이 구조

Unity Netcode for GameObjects + Facepunch Steamworks(Steam 로비) 조합이다.

인원은 4명이 상한이다. `GameNetworkManager.maxAllowedPlayers = 4`(코드 기본값)이고,
로비도 `SteamMatchmaking.CreateLobbyAsync(4)`로 만든다. `ConnectionApproval` 콜백은 페이로드로
`"게임버전,SteamID"` 문자열을 받아 다음 순서로 거절한다.

1. `disallowConnection`이면 "The host was not accepting connections."
2. 페이로드가 비었으면 "Unknown; please verify your game files."
3. `connectedPlayers >= 4`면 "Lobby is full!"
4. `gameHasStarted`면 "Game has already started!"
5. 버전 불일치면 양쪽 버전을 알려 주며 거절
6. 킥당한 SteamID(`StartOfRound.KickedClientIds`)면 "You cannot rejoin after being kicked."

승인해도 `response.CreatePlayerObject = false`다. **Netcode가 플레이어 오브젝트를 만들지 않고,
씬에 미리 배치된 4개(`allPlayerObjects`)를 인덱스로 할당해 소유권만 넘긴다.**

늦은 참가는 우주선이 궤도에 있는 동안만 가능하고, 첫 출발과 함께 로비가 닫힌 뒤로는 영구 불가다.
호스트가 스폰될 때 `SetLobbyJoinable(joinable: true)`가 켜지고,
`StartGame()` 또는 `openingDoorsSequence()`에서 `LeaveLobbyAtGameStart()`가 로비를 떠난다.

### 접속 동기화

`Singleton_OnClientConnectedCallback`(서버에서 `connectedPlayers++`) → `StartOfRound.OnClientConnect(clientId)` →
`OnPlayerConnectedClientRpc(clientId, connectedPlayers, connectedPlayerIdsOrdered, assignedPlayerObjectId,
serverMoneyAmount, levelID, profitQuota, timeUntilDeadline, quotaFulfilled, randomSeed, isChallenge)` 순이다.
**한 번의 RPC에 크레딧·행성·쿼터·마감·시드까지 세계 상태 전부를 실어 보낸다.**
받은 클라는 `groupCredits`를 세팅하고 `ChangeLevel`/`ChangePlanet`을 실행한 뒤
`SetBuyingRateForDay()`와 `UpdateProfitQuotaCurrentTime()`으로 파생값을 다시 계산한다.

새로 들어온 본인은 이어서 두 개를 더 요청한다. `SyncAlreadyHeldObjectsServerRpc(joiningClientId)`로
이미 남들이 들고 있는 물건 상태를 받고(응답은 `SyncAlreadyHeldObjectsClientRpc(gObjects, playersHeldBy,
itemSlotNumbers, isObjectPocketed, syncWithClient)`), `SyncShipUnlockablesServerRpc()`로 슈트·조명·가구 배치·
보관 아이템·차량까지 받는다(`SyncShipUnlockablesClientRpc(playerSuitIDs, shipLightsOn, placeableObjectPositions,
placeableObjectRotations, unlockedObjects, storedItems, scrapValues, itemSaveData, vehicleID)`).
`SyncSuitsServerRpc`/`ClientRpc`도 따로 있다.

### 위치 복제

이 빌드에 `ClientNetworkTransform` 클래스는 존재하지 않는다. 플레이어 위치는
소유 클라이언트가 직접 RPC로 통보한다.

```csharp
[Rpc(SendTo.NotMe)]
private void UpdatePlayerPositionRpc(Vector3 newPos, bool inElevator, bool isInShip,
                                     bool exhausted, bool isPlayerGrounded)
```

보내는 조건은 `(oldPlayerPosition - transform.localPosition).sqrMagnitude`가 임계값을 넘을 때다.
임계값은 특수 상호작용 중 0.06, 근처에 다른 플레이어가 있으면 0.1, 없으면 0.24로 바뀐다 —
**남이 보고 있을 때만 촘촘히 보낸다.** 받는 쪽은 `serverPlayerPosition`에 저장하고
`Vector3.Lerp(localPosition, serverPlayerPosition, 16f * dt)` 또는 거리 비례
`MoveTowards`로 따라간다. 걸음 수 통계와 혈흔도 이 RPC 안에서 처리된다.
서버가 위치를 검증하는 코드는 없다 — **위치는 전적으로 소유 클라 권위**다.

물리 오브젝트는 `NetworkRigidbodyModifiable`이 다룬다. `NetworkTransform.CanCommitToTransform`으로
권한을 판별해, 권한이 없으면 Rigidbody를 `isKinematic = true`로 만들고 보간을 끈다.
`kinematicOnOwner`, `nonKinematicWhenDropping` 플래그로 "던지는 순간에만 물리를 켜는" 처리를 한다.
`SyncWithServerOnSpawn`은 서버가 아니면 자기 자신을 파괴하고, 서버면
`NetworkObject.Spawn(destroyWithScene: true)`을 부르는 짧은 헬퍼다.

로비 UI는 `LobbySlot`이 담당한다. 방을 누르면 `Lobby.Refresh()`를 걸고 `OnLobbyDataChanged` 콜백에서
`LobbyDataIsJoinable(lobby)`를 확인한 뒤 `JoinLobby`를 부른다. 응답이 없을 때를 대비한 타임아웃 코루틴이 붙는다.
`HostSettings`는 `lobbyName`, `serverTag`, `isLobbyPublic` 셋뿐이고, 실제 로비 데이터 키는
`name`, `vers`(버전), `tag`, `dmods`(모드 사용 여부), `chal`(챌린지 파일)이다.
음성은 Dissonance + NFGO 통합(`DissonanceVoip`)이고, `UpdatePlayerVoiceEffects()`가 5초마다 돌며
거리·벽·워키토키에 따라 저역 필터와 피치를 조정한다.

### 판정 주체

| 서버(호스트)가 판정 | 소유 클라이언트가 판정 |
|---|---|
| 라운드 시작·씬 로드·맵 시드 | 자기 캐릭터의 이동·회전·애니메이션 |
| 시간의 정본과 주기 동기화(`SyncTimeClientRpc`) | 자기 체력과 피해(`DamagePlayer`는 `IsOwner`만) |
| 쿼터 갱신(`SetNewProfitQuota`)·매입률 | 자기 죽음 선언(`KillPlayer` → ServerRpc) |
| 마감 판정과 해고 | 낙하 대미지, 익사 타이머 |
| `livingPlayers` 카운트와 전멸 판정 | 스태미나·광기·공포 |
| 아이템 스폰/디스폰, 판매 정산 | 우주선 안/밖 구역 판정(`isInElevator`, `isInHangarShipRoom`) |
| 적 스폰과 AI | 스캔, 상호작용 대상 선택 |
| 세이브 파일 | |

대표 RPC 이름은 서버 방향으로 `StartGameServerRpc`, `EndGameServerRpc`, `KillPlayerServerRpc`,
`SetShipLeaveEarlyServerRpc`, `SyncAlreadyHeldObjectsServerRpc`, `SyncShipUnlockablesServerRpc`,
`SetShipDoorsOverheatServerRpc`, `BuyShipUnlockableServerRpc`, `ChangeLevelServerRpc`,
`SyncGroupCreditsServerRpc`가 있고, 클라 방향으로 `EndOfGameClientRpc`, `EndGameClientRpc`,
`SyncNewProfitQuotaClientRpc`, `SetShipToLeaveOnMidnightClientRpc`, `FirePlayersAfterDeadlineClientRpc`,
`AllPlayersHaveRevivedClientRpc`, `OnPlayerConnectedClientRpc`, `SellItemsClientRpc`,
`KillPlayerClientRpc`가 있다.

## 수치

모든 값은 디컴파일된 C# 필드 초기값(코드 기본값)이거나 코드에 직접 박힌 상수다.

| 항목 | 값 | 출처 |
|---|---|---|
| 최대 인원 | 4 | `GameNetworkManager.maxAllowedPlayers`, `CreateLobbyAsync(4)` |
| 시작 쿼터 | 300 | `QuotaSettings.startingQuota` |
| 시작 크레딧 | 60 | `QuotaSettings.startingCredits` |
| 마감 일수 | 4 | `QuotaSettings.deadlineDaysAmount` |
| 쿼터 증가 기본량 | 200 | `QuotaSettings.baseIncrease` |
| 쿼터 증가 완만도 | 4 | `QuotaSettings.increaseSteepness` |
| 하루 길이 | 700 (= 100 × 7) | `lengthOfHours` × `numberOfHours` |
| 자동 이륙 시각 | 정규화 0.998 (하루 시작 시 세팅) | `shipLeaveAutomaticallyTime` |
| 이륙 경고 시각 | 정규화 0.9 | `TimeOfDayEvents()` |
| 하루 위상 | 0.33 정오 / 0.63 일몰 / 0.9 자정 | `GetDayPhase` |
| 매입률 하한 | 0.3 | `SetBuyingRateForDay()` |
| 매입률(남은 날 3/2/1/0) | 0.475 / 0.65 / 0.825 / 1.0 | 위 공식에 기본값 대입 |
| 초과 보너스 | `초과분 / 5 + 15 × 남은 날` | `SetNewProfitQuota()` |
| 사망 페널티 | 시신 미회수 20% / 회수 8%, 하한 min(크레딧, 60) | `ApplyPenalty()` |
| 최대 체력 | 100 | `ReviveDeadPlayers`, `DamagePlayer` 클램프 |
| 치사 방지 | 피해 < 50이고 치명 부상 아니면 체력 5로 생존 | `DamagePlayer()` |
| 치명 부상 진입 | 체력 < 10 | `DamagePlayer()` |
| 자연 회복 | 체력 < 20일 때 초당 1, 20에서 정지 | 회복 루프 |
| 낙하 대미지 | −40 → 50 / −45 → 80 / −48.5 → 100 / `fallValue` −38 → 30 | `PlayerHitGroundEffects()` |
| 낙하 소음 반경 | `fallValue < -16`에서 7 | `PlayAudibleNoise` |
| 즉사 고도 | y < −600 | `StartOfRound.LateUpdate()` |
| 기본 이동 속도 | 0.5 (`/ carryWeight`) | `movementSpeed` |
| 스프린트 배수 | 최대 2.25, `Lerp(dt × 1)`로 상승 | `sprintMultiplier` |
| 스태미나 기준 시간 | 5초 | `sprintTime` |
| 스태미나 소모 | `dt / 5 × carryWeight` | 달릴 때 |
| 스태미나 회복 | 정지 `dt / 9` (9초) / 보행 `dt / 14` (14초) | `sprintTime + 4`, `+ 9` |
| 탈진 임계 | 0.3 미만에서 재시도 시 탈진, 0.2 초과에서 해제, 0.1 미만도 탈진 | 이동 분기 |
| 피격 스태미나 회복 | 피해 ≥ 10일 때 `+= 피해 / 125` | `DamagePlayer()` |
| 점프력 | 5 | `jumpForce` |
| 등반 속도 | 4 | `climbSpeed` |
| 아이템 집기 거리 | 5 | `grabDistance` |
| 던지는 힘 | 17 | `throwPower` |
| 익사 시간 | 12초 | `drowningTimer -= dt / 12` |
| 최대 광기 | 50 | `maxInsanityLevel` |
| 광기 판정 거리 | 17m | `NearOtherPlayers(17f)` |
| 공포 자연 감소 | 생존 0.055/s, 사망 0.5/s | `StartOfRound.LateUpdate()` |
| 스캔 거리 | 노드별 `minRange` 5 ~ `maxRange` 7 | `ScanNodeProperties` |
| 스캔 탐색 | 앞 20m 지점에서 반지름 20, 사거리 80, 최대 25개 | `AssignNewNodes()` |
| 스캔 쿨다운 | `playerPingingScan` 0.3에서 시작 | `PingScan_performed` |
| 우주선 문 전력 | 20초 소모 / 약 4.4초 충전 | `HangarShipDoor` |
| 등급 점수 | −1 D / 0 C / 1 B / 2 A / 3 S, 전멸 F | `FillEndGameStats()` |
| XP | 생존 +10, 사망 −3, 최고 수익 +15, 전멸 −5, 수거율로 스케일 | `SetPlayerLevel()` |
| 세이브 슬롯 | 3개 + 챌린지 1 + 전역 1 | `LCSaveFile1/2/3`, `LCChallengeFile`, `LCGeneralSaveData` |
| 세이브 스커밍 | 달 위 저장 5회 이상이면 곰팡이 확산 | `SaveGameValues()` |
| 위치 RPC 임계값 | 특수 0.06 / 근처 0.1 / 원거리 0.24 (제곱 거리) | `PlayerControllerB.Update` |

## CL9 시사점

관찰만 적는다. 결정은 하지 않는다.

**하루 루프의 상태 변수가 딱 두 개다.** `inShipPhase`(안전 지대에 정박)와 `shipHasLanded`(원정지에서
돌아갈 수 있음)의 조합으로 전체 흐름이 표현된다. CL9의 "주문서 수령 → 목적지 선택 → 원정 → 귀환 →
요리 → 납품 → 구매"도 푸드트럭 정차 상태와 원정지 도착 상태의 두 축으로 정리해 볼 여지가 있다.
UE라면 GameState의 복제 변수 두 개가 그 자리에 대응한다.

**시간이 서버의 float 하나(`globalTime`)로 관리되고, 장소별 시각은 거기에 오프셋과 배속을 곱해 파생시킨다.**
CL9이 `GetServerWorldTimeSeconds`를 기준으로 삼기로 한 것과 방향이 같고, 목적지별 난이도를
"밤이 더 빨리 온다"로 표현할 수 있는 여지가 여기 있다. 주기적으로 `SyncTimeClientRpc`를 쏘는 것도
드리프트 보정 패턴으로 그대로 옮겨진다.

**"밤이 만료되면 원정 종료"의 처리 방식이 참고가 된다.** 예고(정규화 0.9에서 대사), 강제 실행(0.998에서
서버 RPC), 그리고 밖에 남은 사람은 시체도 없이 사망이라는 세 단계다. 특히 남겨진 사람이 시신조차
남기지 못하게 만든 것은 사망 페널티(시신 회수 감면)와 맞물려 "늦으면 손해가 두 배"라는 압박을 만든다.
CL9은 "잡혀도 바로 죽지 않는다"를 원칙으로 두었으니, 시간 만료 처벌을 죽음이 아닌 다른 손실로
번역해야 하는 자리다.

**쿼터 갱신 공식이 제곱 성장 + 커브 기반 난수 + 플레이어가 조정 가능한 행운 값의 세 겹으로 되어 있다.**
증가 배수 `1 + n²/steepness`가 뼈대고, `AnimationCurve`가 난수 분포를 조형하며, 우주선에 놓은 가구의
`luckValue` 합이 그 난수를 낮은 쪽으로 민다. 즉 "돈을 써서 다음 쿼터의 상한을 낮춘다"는 선택지를 만들어
경제에 되먹임 고리를 넣었다. CL9의 주문서 쿼터("오늘 오믈렛 3, 꿀버터 1")는 금액이 아니라 품목이라
곱셈 공식이 그대로 오지는 않지만, "달성 횟수에 따라 난이도가 제곱으로 오르되 난수 폭을 커브로 제어한다"는
구조는 품목 수·희소도 결정에 그대로 옮겨 볼 수 있다.

**매입률이 마감에 따라 오르는 것이 "미루기"에 대한 보상이자 함정이다.** 마감 당일이 1.0이고 시작이 0.3이므로
납품을 미룰수록 같은 재료가 3배 이상 값이 나간다. 대신 초과 보너스(`15 × 남은 날`)는 일찍 채울수록 크다.
두 힘이 반대로 작용해서 "언제 파느냐"가 판단거리가 된다. CL9의 요리 납품에도 같은 종류의
시간 대 가치 곡선을 넣을 자리가 있는지 확인해 볼 만하다.

**실패의 단계가 세 개로 나뉘어 있다.** 개인의 죽음(부활 가능, 크레딧 20% 손실), 전멸(하루와 스크랩 전부 손실,
그러나 세이브는 유지), 해고(세이브 초기화). CL9은 1~4인 협동이므로 "전멸했지만 계속 진행된다"는 중간 단계가
페이스 유지에 어떤 역할을 하는지 눈여겨볼 대목이다. 특히 전멸했을 때 **우주선 안에 있던 스크랩까지 사라지지만
장비는 남는다**는 규칙은, "재료는 잃고 도구는 지킨다"는 형태로 CL9의 푸드트럭에도 대응시켜 볼 수 있다.

**스태미나가 무게에 정확히 반비례한다.** `dt / sprintTime * carryWeight` 한 줄이 전부이고,
회복은 정지 9초 · 보행 14초로 소모(5초)보다 훨씬 느리다. 게다가 바닥까지 쓰면 0.2를 넘길 때까지
아예 못 뛰는 탈진 상태가 있다. CL9의 "스태미나·무게 감속"도 곱셈 한 번으로 끝내고, 회복이 소모보다
2~3배 느리다는 비대칭과 탈진 문턱을 넣을지가 결정 지점이다.

**체력에 "첫 치명타는 죽이지 않는다"는 규칙이 명시적으로 들어 있다.** 50 미만 피해로는 체력이 5로 남고,
그때부터 절뚝이며 출혈한다. 20 아래에서만 초당 1씩 회복되므로 "부상 상태에서 기어서 돌아온다"는
플레이가 성립한다. CL9의 "잡혀도 바로 죽지 않는다"와 정확히 같은 문제를 푸는 방식이고,
UE에서는 이 상태가 PlayerState에 놓일 값이다.

**판정 분할이 CL9의 계획과 다르다.** Lethal Company는 자기 체력과 자기 죽음을 소유 클라이언트가 선언하고
서버는 그걸 받아 적기만 한다(`DamagePlayer`가 `IsOwner`만 실행, `KillPlayer` → `KillPlayerServerRpc`).
위치도 서버 검증이 없다. CL9은 "호스트 = GameMode 서버 전용 판정"을 택했으니, 이쪽이 더 보수적이다.
Lethal Company 방식의 이점은 자기 클라에서 즉시 반응한다는 것이고, 대가는 검증 부재다.
어느 쪽이든 "누가 무엇을 선언하는가"를 항목별로 표로 못 박아 두는 편이 낫다는 것은 공통이다.

**위치 복제를 거리 임계값으로 조절한다.** 근처에 다른 플레이어가 있으면 0.1, 없으면 0.24(제곱 거리)로
전송 빈도를 바꾼다. CL9이 GameState/PlayerState를 20Hz로 복제하기로 한 것과는 다른 축의 최적화이고,
4인 규모에서는 굳이 필요 없을 수도 있다.

**"우주선에 붙어 다니는 물건"이 실제 부모-자식이 아니라 매 프레임 오프셋 재적용이다.**
`AutoParentToShip.MoveToOffset()`이 매 `LateUpdate`마다 기준 트랜스폼에 로컬 오프셋을 다시 적용한다.
CL9의 열린 버그("수레가 안 따라옴")와 같은 부류의 문제를 이렇게 푼 사례로 참고할 수 있다.
움직이는 발판 위의 플레이어·시체·아이템은 별도로 `PlayerPhysicsRegion`이 트리거로 부모를 붙였다 떼며,
발판이 뒤집히면(`maxTippingAngle`) 콜라이더를 꺼서 붙지 않게 한다.

**세이브를 호스트만 쓴다.** `if (!isHostingGame) return;` 한 줄이 전부다. 저장 시점도 하루가 끝나
안전 지대로 돌아온 직후 하나뿐이라, 중간 저장 · 되돌리기가 원천 봉쇄된다. 그런데도 세이브 스커밍
방지 카운터(`Level{id}TimesSavescumming`)를 따로 두고 5회 이상이면 그 달에 곰팡이를 퍼뜨린다 —
파일을 직접 백업하는 우회까지 염두에 둔 장치다.

**스캔에 최소 거리가 있다.** `minRange` 5 미만이면 스캔되지 않는다. 코앞의 물건은 눈으로 보라는 뜻이다.
CL9의 스캔은 이미 몬스터와 시설물까지 잡도록 확장했으니, 종류별로 `minRange`/`maxRange`/
시야 필요 여부를 따로 두는 이 구조(`ScanNodeProperties` 5필드)가 참고가 된다.

**늦은 참가를 완전히 막았다.** 첫 출발과 동시에 로비를 떠나 버리므로, 도중에 나간 사람은 돌아올 수 없다.
`response.CreatePlayerObject = false`로 플레이어 오브젝트를 씬에 미리 4개 박아 두고 인덱스로 할당하는 것도
같은 결정의 연장이다. 이 선택이 동기화 코드 전체를 얼마나 단순하게 만드는지가 눈에 띈다 —
"참가 시점은 궤도에 있을 때뿐"이라는 전제 덕분에 접속 동기화 RPC가 세 개(`OnPlayerConnectedClientRpc`,
`SyncAlreadyHeldObjects*`, `SyncShipUnlockables*`)로 끝난다.
