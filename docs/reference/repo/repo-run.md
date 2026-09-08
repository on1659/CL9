# 참고 — R.E.P.O.의 런 루프·진행·플레이어·멀티

> 2026-09-08, 설치본의 `Assembly-CSharp.dll`을 디컴파일해 읽은 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](../README.md).
> CL9의 대응 문서는 [../loop.md](../../plan/loop.md) · [../data/progression.md](../../plan/data/progression.md) ·
> [../data/rules.md](../../plan/data/rules.md)다.

R.E.P.O.의 한 판(런)은 "트럭 로비 → 레벨 → 추출 완료 → 상점 → 트럭 로비 → 다음 레벨"을 반복하는
씬 전환 루프이며, 이 전환을 전부 `RunManager.ChangeLevel()` 한 메서드가 결정한다. 진행도는
`RunManager.levelsCompleted` 정수 하나로 표현되고, 난이도 곡선은 이 값을 0~1로 정규화한
세 개의 난이도 배율(`SemiFunc.RunGetDifficultyMultiplier1/2/3`)을 `AnimationCurve`에 먹여서
목표 금액·몬스터 수·귀중품 총액·맵 크기를 뽑아내는 방식으로 통일돼 있다. 판정 주체는 언제나
마스터 클라이언트(호스트)이고, 클라이언트는 RPC를 받아 값을 따라 쓰기만 한다. 세이브 파일도
마스터 클라이언트만 쓰고 읽는다. 아래는 각 영역을 소스에서 읽어 정리한 것이다.

---

## 런 루프

### 씬 전환의 단일 관문

레벨 전환은 전부 `RunManager.ChangeLevel(bool _completedLevel, bool _levelFailed, ChangeLevelType)`
을 거친다. 이 메서드는 첫 줄에서 `SemiFunc.IsMasterClientOrSingleplayer()`가 아니면(그리고
메뉴 레벨이 아니면) 즉시 반환하므로, 진행 판정은 호스트 전용이다. 전환이 결정되면 호스트가
`runManagerPUN.photonView.RPC("UpdateLevelRPC", RpcTarget.OthersBuffered, levelCurrent.name, levelsCompleted, gameOver)`
로 레벨 이름·진행도·게임오버 여부를 버퍼드 RPC로 뿌리고, 그 다음 `RestartScene()`이 실제 씬을 다시 로드한다.
`OthersBuffered`를 쓰는 것은 나중에 들어온 사람도 같은 레벨 정보를 받게 하려는 의도로 보인다(추정).

씬 로드 자체는 두 갈래다. 싱글플레이는 `SceneManager.LoadSceneAsync("Main")`, 멀티플레이는
마스터가 `PhotonNetwork.LoadLevel("Reload")`를 부른다. 즉 모든 레벨이 같은 씬("Main"/"Reload")을
재사용하고, 실제 레벨 내용은 `LevelGenerator`가 런타임에 조립한다.

`RestartScene()`은 곧장 씬을 갈아치우지 않고, `GameDirector.instance.PlayerList`의 모든
`PlayerAvatar`에게 `OutroStart()`를 걸어 두고 매 프레임 다시 호출되면서 전원이 `outroDone`이 될 때까지
기다린다. 즉 "모든 플레이어의 아웃트로가 끝나야 씬이 넘어간다"는 동기화 지점이 하나 있다.

### 레벨 종류

`RunManager`가 들고 있는 레벨 목록은 `levelLobby`(트럭), `levelLobbyMenu`, `levelMainMenu`,
`levelTutorial`, `levelRecording`, `levelSplashScreen`, 그리고 리스트인 `levelShop`,
`levelArena`, `levels`(실제 원정 레벨)이다. 원정 레벨은 `SetRunLevel()`에서 고르는데,
직전에 플레이한 레벨(`previousRunLevel`)과 같은 것이 뽑히면 다시 뽑는 while 루프를 돈다.
즉 같은 레벨이 연속으로 나오지 않는 것만 보장하고 그 이상의 순서 규칙은 없다.

### 정상 진행의 분기 (ChangeLevel의 default 케이스)

읽은 그대로의 분기 순서는 이렇다.

1. 현재가 메인 메뉴 또는 로비 메뉴면 → `levelLobby`(트럭)로 간다.
2. 현재가 원정 레벨이고 `_completedLevel`이 참이면 → `previousRunLevel`을 기억하고
   `levelsCompleted++` 한 뒤 `levelShop`에서 무작위로 하나 골라 간다.
3. 현재가 트럭 로비면 → `SetRunLevel()`로 다음 원정 레벨을 고른다.
4. 현재가 상점이면 → 트럭 로비로 돌아간다.

정리하면 텍스트 흐름도는 다음과 같다.

```
메인 메뉴/로비 메뉴 → 트럭(Lobby) → 원정 레벨 → (추출 전부 완료 + 전원 승차)
   → 상점(Shop) → 트럭(Lobby) → 다음 원정 레벨 → …
전멸 → 아레나(Arena) → 아레나 종료 → 진행 초기화 → 로비 메뉴(멀티) 또는 새 런(싱글)
```

**상점은 몇 레벨마다 나오는가**: 코드상 조건 없이 "원정 레벨을 완료할 때마다" 상점이 낀다.
`levelsCompleted++` 바로 다음 줄이 `levelCurrent = levelShop[Random.Range(0, levelShop.Count)]`이므로
매 레벨 뒤에 상점이 한 번씩 들어간다. 상점 레벨은 여러 개 중 무작위다.

### 세이브 슬롯 위치 기록

전환할 때마다 현재 위치를 정수로 남긴다. 상점이면 `saveLevel = 1`, 트럭 로비면 `2`, 그 외에는 `0`이고
`SemiFunc.StatSetSaveLevel(saveLevel)`로 런 통계에 저장한다. 세이브를 다시 불러올 때는
`RunManager.loadLevel`이 이 값을 받아 `SemiFunc.MenuActionSingleplayerGame()` /
`MenuActionHostGame()`에서 상점(1)/트럭(2)/새 원정(그 외)으로 복귀 지점을 정한다.

### 게임 상태 머신 (`GameDirector`)

`GameDirector.gameState`는 `Load, Start, Main, Outro, Result, End, EndWait, Death` 여덟 가지다.
흐름은 `Load`(레벨 생성 대기) → `Start`(로딩 UI 애니메이션이 끝나고 0.5초 뒤 스냅샷 전환) →
`Main`(실제 플레이) → `Outro`(HUD 숨김, 보이스 채팅을 로비 믹서로) → `Result`(결과 화면) →
`End`(로컬 플레이어를 `SetDisabled()` 하고 로딩 시작) → `EndWait`이다.
`Death`는 로컬 플레이어가 죽었을 때만 끼어드는 별도 상태로, 0.2초 프리즈 후 렌더 텍스처를
20% 해상도로 줄였다가 흔들고, 0.5초 뒤 `SetSpectate()`를 호출한 다음 다시 `Main`으로 돌아간다.
즉 죽어도 상태 머신은 `Main`을 유지하고 카메라만 관전으로 바뀐다.

`GameDirector`는 또 `fpsImpulse1/5/15/20/30/60` 불리언을 매 `LateUpdate`에서 갱신해서,
다른 스크립트들이 "초당 N회만 실행"을 싸게 구현하도록 공용 틱을 제공한다.
같은 목적의 `SemiFunc.PerSecond(float _frequency, Object _obj)`는 오브젝트 인스턴스 ID로
위상을 흩뜨려서 같은 프레임에 몰리지 않게 한다.

### 라운드 진행과 완료 판정 (`RoundDirector`)

`RoundDirector`는 레벨 안의 목표 금액과 추출 지점 개수를 관리한다.

- `extractionPoints`는 각 `ExtractionPoint`가 자기 `Start`에서 `RoundDirector.instance.extractionPoints++`
  로 스스로 등록해 세어진다. 실제 개수는 맵 크기가 결정한다(아래 난이도 곡선 참조).
- 목표 총액 `haulGoal`은 레벨 시작 코루틴 `StartRound()`에서 호스트만 계산한다. 계산식은 그대로 옮기면

  ```csharp
  float num = (float)haulGoalMax * 0.7f;
  num = ((!(SemiFunc.RunGetDifficultyMultiplier2() > 0f))
      ? (num * haulGoalCurve1.Evaluate(SemiFunc.RunGetDifficultyMultiplier1()))
      : (num * haulGoalCurve2.Evaluate(SemiFunc.RunGetDifficultyMultiplier2())));
  ```

  여기서 `haulGoalMax`는 맵에 실제로 스폰된 귀중품 값의 합이다
  (`ValuableObject.cs:251`에서 `RoundDirector.instance.haulGoalMax += (int)dollarValueCurrent`).
  즉 목표는 "이 맵에 놓인 총 가치의 70%에 난이도 곡선을 곱한 값"이다.
  `haulGoalCurve1/2`는 인스펙터 `AnimationCurve`라 소스만으로는 수치를 알 수 없다.
  계산이 끝나면 `photonView.RPC("StartRoundRPC", RpcTarget.All, haulGoal)`로 전원에게 뿌린다.
- 추출 지점 하나가 요구하는 금액은 `ExtractionPoint.cs:1033`에서
  `int value = RoundDirector.instance.haulGoal / RoundDirector.instance.extractionPoints;`
  로, 총 목표를 추출 지점 개수로 균등 분할한 값이다. 이 값을 `HaulGoalSet(value)`가
  RPC로 전파한다. 남는 초과분(`CalculateSurplus()` = `Mathf.Max(0, haulCurrent - haulGoal)`)은
  `RoundDirector.extractionPointSurplus`에 쌓여 다음 추출 지점으로 이월된다.
- 추출이 하나 끝날 때마다 `ExtractionCompleted()`가 `extractionPointsCompleted++`를 하고,
  `ExtractionCompletedAllCheck()`가 `extractionPointsCompleted >= extractionPoints - 1`이면
  전원에게 `ExtractionCompletedAllRPC`를 보내 `allExtractionPointsCompleted = true`로 만들고
  조명을 끄고 카메라를 흔든다. 이때부터 트럭 치유기가 열린다.
- `currentHaul`은 `Update()`에서 매 프레임 `dollarHaulList`와 `valuableBoxHaulList`를 훑어
  다시 합산한다. 네트워크로는 `OnPhotonSerializeView`에서 `currentHaul`과 `haulGoal` 두 개만 흘린다.

추출 완료 시 통화 지급은 `ExtractionPoint.cs:1929` 근처에서 일어난다.

```csharp
runCurrencyBefore = SemiFunc.StatGetRunCurrency();
extractionHaul = Mathf.CeilToInt(extractionHaul / 1000);
SemiFunc.StatSetRunCurrency(runCurrencyBefore + extractionHaul);
SemiFunc.StatSetRunTotalHaul(SemiFunc.StatGetRunTotalHaul() + extractionHaul);
```

즉 내부 달러 값을 1000으로 나눠 "K 단위" 정수로 바꿔 런 통화에 더한다. UI가 `$123K`처럼
보이는 이유가 이것이다.

### 트럭 화면 흐름 (`TruckScreenText`)

트럭·로비·상점의 CRT 화면은 같은 스크립트가 `ScreenType`(`TruckScreen`, `TruckLobbyScreen`,
`TruckShopScreen`)으로 갈라 쓴다. 페이지는 열거형으로 고정돼 있다. 트럭 화면은
`Start / EndNotEnough / EndEnough / AllPlayersInTruck`, 로비 화면은
`Start / FailFirst / FailSecond / FailThird / HitRoad`, 상점 화면은
`Start / NotEnough / Enough / Stealing / AllPlayersInTruck`이다.

플레이어가 채팅 박스에 무언가를 쳐 넣으면 `ChatMessageLevel()`이 판정한다.
추출 지점을 전부 끝냈고 `SemiFunc.PlayersAllInTruck()`이면 성공음과 함께
`PlayerChatBoxState.LockedStartingTruck`으로 잠그고 페이지 2로, 전부 끝냈지만 밖에 남은 사람이
있으면 `LockedDestroySlackers` 상태로 페이지 3으로 간다. 추출이 덜 끝났으면 실패음과 페이지 1이다.

밖에 남은 사람을 처리하는 것이 `SelfDestructPlayersOutsideTruck()`이다. 호스트가
전원에게 RPC를 보내 `selfDestructingPlayers = true`로 만들고, `PlayerSelfDestruction()`이
초당 5회 검사하면서 트럭 안에 있는 사람은 `selfDestructPrevented`로 면제하고, 살아 있으면서
트럭 밖이고 면제도 안 된 사람이 하나도 없어야 출발을 허가한다. 출발은
`GotoNextLevel()` → `RunManager.instance.ChangeLevel(_completedLevel: true, _levelFailed: false)`이고,
상점에서는 `ShopGotoNextLevel()` 코루틴이 0.5초 대기 후 같은 호출을 한다.

화면 텍스트에는 `[goal]`, `[goalmax]`, `[extraction]`, `[allplayerintruck]` 같은 치환 태그가 있어
`RoundDirector.haulGoal`, `haulGoalMax`, 추출 진행도, 트럭 밖 인원 이름 목록을 그대로 박아 넣는다.

`runLives`(코드 기본값 3)는 `TruckScreenText.LobbyScreenStartLogic()`에서 2/1/0에 따라
페이지 1/2/3을 띄우는 데만 쓰이고 `StatsManager.LoadGame()`에서 복원된다. 그런데
`RunManager.levelFailed`에 값을 대입하는 코드는 소스 전체에서 찾을 수 없고 `runLives`를
감소시키는 코드도 없다. 목숨 3개 시스템의 잔재로 보인다(추정).

### 전멸과 게임 오버 (`Arena`)

`RunManager.Update()`가 호스트에서 매 프레임 전멸을 감시한다. 조건은 재시작 중이 아니고,
런이 시작됐고, 아레나가 아니고, `GameDirector.currentState == Main`이며,
`GameDirector.instance.PlayerList`의 모든 `PlayerAvatar`가 `isDisabled`인 상태다.
그러면 `AllPlayersDeadSet(true)`를 하고 `ChangeLevel(_completedLevel: false, _levelFailed: true)`을 부른다.

`_levelFailed`가 참이고 현재가 로비/상점이 아니면, 현재 레벨이 아레나가 아닌 경우
`levelArena` 중 하나로 간다(인원이 2명 이상이면 무작위, 1명이면 `levelArena[0]` 고정).
이미 아레나에 있었다면 `ResetProgress()`로 진행을 전부 지우고, 멀티면 로비 메뉴로,
싱글이면 새 원정 레벨로 보내면서 `gameOver = true`를 세운다. `gameOver`면
`RestartScene()`에서 `NetworkManager.instance.DestroyAll()`로 네트워크 오브젝트를 전부 파괴한다.

아레나 자체는 `Arena.States`(`Idle, Level1~3, Falling, Starting, PlatformWarning, PlatformRemove, GameOver`)
로 도는 배틀로얄식 미니게임이다. 시작할 때 `numberOfPlayers = SemiFunc.PlayerGetAll().Count`를 재고
전원을 5초 무적(`playerHealth.InvincibleSet(5f)`)으로 만든다. `StateGameOver`는 승자가 있으면
"KING OF THE LOSERS!", 여럿인데 승자가 없으면 "EVERYONE'S A LOSER!"를 띄우고,
인원이 2명 이상이면 10초, 혼자면 3초 뒤 `ChangeLevel(_completedLevel: false, _levelFailed: true)`로
런을 정리한다. 아레나에서는 스태미나 회복이 5배(`sprintRechargeAmount * 5f`),
회복 대기 시간이 절반이며 체력이 최대치로 채워진다.

### 튜토리얼 (`TutorialDirector`)

`TutorialDirector`는 두 역할을 겸한다. 하나는 별도 튜토리얼 레벨(`levelTutorial`) 진행이고,
다른 하나는 일반 런 중에 뜨는 상황별 팁이다. 팁은 `DataDirector.Setting.TutorialXxx` 설정값을
카운터로 써서 `TutorialSettingCheck(setting, n)`으로 "아직 n번 안 봤으면 보여준다"를 판정하고,
`playerJumped`, `playerSprinted`, `playerRevived` 같은 플래그를 라운드 끝(`TipsStore()`)에 모아
안 해 본 동작에 대한 팁을 다음 라운드에 예약한다. 튜토리얼을 끝내면
`TruckScreenText.TutorialFinish()`가 아웃트로를 시작하고 `NetworkManager.leavePhotonRoom = true`로 방을 뜬다.

---

## 난이도 곡선

모든 난이도 스케일링은 `levelsCompleted` 하나에서 나온 세 개의 정규화 값으로 시작한다.

```csharp
public static float RunGetDifficultyMultiplier1() { return Mathf.Clamp01((float)RunManager.instance.levelsCompleted / 9f); }
public static float RunGetDifficultyMultiplier2() { return Mathf.Clamp01((float)(RunManager.instance.levelsCompleted - 9) / 10f); }
public static float RunGetDifficultyMultiplier3() { return Mathf.Clamp01((float)(RunManager.instance.levelsCompleted - 19) / 10f); }
```

배율 1은 레벨 0~9 구간, 배율 2는 10~19 구간, 배율 3은 20~29 구간을 담당한다.
소비하는 쪽은 대개 "배율 2가 0보다 크면 두 번째 커브, 아니면 첫 번째 커브"라는 식으로
구간에 따라 다른 `AnimationCurve`를 고른다. 즉 곡선을 계단식으로 이어 붙인 구조다.

| 무엇이 커지나 | 어디서 | 공식·규칙 |
|---|---|---|
| 목표 총액 `haulGoal` | `RoundDirector.StartRound()` | `haulGoalMax * 0.7f * haulGoalCurve1.Evaluate(m1)` (레벨 10 이상이면 `haulGoalCurve2.Evaluate(m2)`) |
| 추출 지점당 목표 | `ExtractionPoint.cs:1033` | `RoundDirector.haulGoal / RoundDirector.extractionPoints` |
| 맵 모듈 수 | `LevelGenerator.cs:601` | `ModuleAmount = Mathf.Min(5 + levelsCompleted, 10)`, 레벨 10 이상이면 `+= Mathf.Min(levelsCompleted - 9, 5)` |
| 막다른 길 수 | `LevelGenerator.cs:610` | `DeadEndAmount = Mathf.CeilToInt(ModuleAmount / 3)` |
| 추출 지점 수 | `LevelGenerator.cs:611~` | 모듈 15개 이상 4개, 10개 이상 3개, 8개 이상 2개, 6개 이상 1개 |
| 모듈 희귀도 가중치 | `LevelGenerator.cs:811~813` | `ModuleRarity1/2/3 = DifficultyCurve1/2/3.Evaluate(m1)` |
| 몬스터 수 | `EnemyDirector.AmountSetup()` | 난이도 등급별로 `amountCurve1/2/3_1.Evaluate(m1)`, 레벨 10 이상이면 `_2.Evaluate(m2)`. 합계는 `totalAmount` |
| 몬스터 재등장 간격 | `EnemyDirector.cs:325~` | `despawnTimeCurve_2.Evaluate(m3)` (레벨 20+) → `despawnTimeCurve_1.Evaluate(m2)` (레벨 10+) → 그 외 1.0 |
| 몬스터 종류 해금 | `EnemyDirector.PickEnemies()` | `EnemySetup.levelsCompletedMin/Max` 범위 밖이면 후보에서 제외. 추가로 `DataDirector.Setting.RunsPlayed`가 `EnemySetup.runsPlayed`보다 작아도 제외 |
| 귀중품 총액·크기별 개수 | `ValuableDirector.SetupHost()` | `totalMaxValueCurve1/2`, `tiny/small/medium/big/wide/tall/veryTallMaxAmountCurve1/2`를 m1(또는 m2)로 평가 |
| 몬스터 유휴 스폰 간격 | `EnemyDirector.cs:121` | `60f * Random.Range(2f, 3f) * spawnIdlePauseCurve.Evaluate(m1)` |
| 상점 회복 아이템 가격 | `ShopManager.HealthPackValueGet()` | `_value += _value * healthPackValueIncrease * Mathf.Min(levelsCompleted, 15)` |
| 상점 파워 크리스탈 가격 | `ShopManager.CrystalValueGet()` | `_value += _value * crystalValueIncrease * Mathf.Min(levelsCompleted, 15)` |
| 트럭 최종 회복량 | `PlayerAvatar.FinalHealRPC()` | 달 레벨 0이면 50, 1이면 35, 2 이상이면 25 |

`ModuleAmount` 스케일링에는 전제가 있다. `if (ModuleAmount > 4)` 안에서만 위 공식을 쓰므로,
`Level` 에셋이 지정한 기본 모듈 수가 5 이상인 레벨만 진행도에 따라 커진다. 또
`DebugLevelSize` 배수가 마지막에 곱해진다.

### 달 레벨

`RunManager.CalculateMoonLevel(int _levelsCompleted)`는 `(_levelsCompleted + 1) / 5`를 돌려준다.
즉 5레벨마다 달 단계가 하나 올라간다. `Moon`은 `ScriptableObject`로 이름·아이콘·속성 텍스트
목록만 들고 있어서(=`Moon.cs` 전체가 그것뿐이다) 그 자체로는 난이도를 바꾸지 않는 표시용 티어다.
다만 위 표에 적은 트럭 최종 회복량은 `SemiFunc.MoonLevel()`을 직접 참조해 줄어들기 때문에
"달이 올라가면 회복이 짜진다"는 실제 효과가 하나 있다.

### 인원수는 난이도에 거의 반영되지 않는다

`PlayerCount`, `PlayerList.Count`, `PlayerGetAll().Count`를 전부 훑어봤을 때,
목표 금액·몬스터 수·맵 크기 중 어느 것도 인원수를 참조하지 않는다. 인원수가 실제로 쓰이는 곳은 다음이다.

- 상점 가격 할인: `ShopManager.UpgradeValueGet()`과 `HealthPackValueGet()`이
  `int num = Mathf.Min(6, GameDirector.instance.PlayerList.Count); _value -= _value * 0.1f * (num - 1);`
  로 1명 늘 때마다 10%씩 깎아 준다(최대 6명분).
- 아레나 진행: `Arena.numberOfPlayers`가 게임오버 대기 시간과 "EVERYONE'S A LOSER" 문구를 가른다.
- 전멸 시 아레나 선택: 2명 이상이면 무작위 아레나, 1명이면 `levelArena[0]`.
- 배터리 충전 속도: `PlayerBattery`가 잡고 있는 사람 수(`amountPlayersGrabbing`)만큼 곱해서 회복한다.
- 아이템 등장 조건: `Item.minPlayerCount`.

바꿔 말해 R.E.P.O.는 "인원이 늘면 몬스터가 늘거나 목표가 오르는" 식의 스케일링을 하지 않고,
사람이 늘면 그대로 쉬워지도록 두었다.

---

## 저장과 통계

### `StatsManager` — 딕셔너리의 딕셔너리

세이브 대상 데이터는 전부 `SortedDictionary<string, Dictionary<string, int>> dictionaryOfDictionaries`
하나에 등록된다. `Start()`에서 등록하는 순서대로 나열하면 `runStats`, `itemsPurchased`,
`itemsPurchasedTotal`, `itemsUpgradesPurchased`, `itemBatteryUpgrades`, `playerHealth`,
그리고 업그레이드 계열 `playerUpgradeHealth / Stamina / ExtraJump / Launch / TumbleClimb /
DeathHeadBattery / MapPlayerCount / Speed / Strength / Range / Throw / CrouchRest / TumbleWings`,
이어서 `playerColor`, `playerInventorySpot1~3`, `playerHasCrown`, `item`, `itemStatBattery`다.
안쪽 딕셔너리의 키는 플레이어면 Steam ID, 아이템이면 아이템 이름이다.

두 개의 필터 목록이 붙는다. `doNotSaveTheseDictionaries`에는 `playerInventorySpot1~3`과
`playerColor`가 들어가 저장에서 아예 빠지고, `stripTheseDictionaries`는 "이 값이면 기록하지 않는다"는
기본값 맵이다(`itemsPurchased` 0, `itemStatBattery` 100, `playerHealth` 100 등). 이름이
`player`로 시작하는 나머지 딕셔너리는 기본값 0으로 자동 등록된다. 세이브 파일 크기를
줄이려는 최적화다.

런 단위 통계는 `runStats` 하나에 담긴다. `RunStartStats()`가 새 런에서 채우는 초기값은
`level = 0`, `currency = 0`, `lives = 3`, `chargingStationCharge = 1`,
`chargingStationChargeTotal = 100`, `totalHaul = 0`이고, 이때 `itemsPurchased`에
`Item Power Crystal`과 `Item Cart Medium`을 1개씩 미리 넣어 준다(시작 지급품).

접근자는 `GetRunStatCurrency/Lives/Level/SaveLevel/TotalHaul`이고, 쓰기는 전부
`SemiFunc.StatSetRunLevel/RunCurrency/SaveLevel/RunTotalHaul/RunLives` → `PunManager.SetRunStatSet()`
를 거친다. `SetRunStatSet()`은 `SemiFunc.IsMasterClientOrSingleplayer()`일 때만 값을 쓰고
멀티면 `photonView.RPC("SetRunStatRPC", RpcTarget.Others, ...)`로 나머지에게 알린다.

플레이어 최대 체력은 통계에서 파생된다. `GetPlayerMaxHealth(steamID)`는
`playerUpgradeHealth` 값 × 20을 돌려주고, `PlayerHealth.Fetch()`가
`maxHealth = 100 + StatsManager.instance.GetPlayerMaxHealth(...)`로 조립한다.
`SetPlayerHealth(steamID, health, setInShop)`는 상점 레벨에서는 `setInShop`이 참일 때만 기록해서,
상점에서 입은 피해가 저장되지 않게 막는다.

`timePlayed`는 `Update()`에서 로비 메뉴와 메인 메뉴가 아닐 때만 `Time.deltaTime`을 누적한다.

### `DataDirector` — 파일 포맷과 설정

저장 라이브러리는 Easy Save 3(ES3)다. 세이브 게임은 `StatsManager.SaveGame(fileName)`에서
`Application.persistentDataPath + "/saves/<이름>/<이름>.es3"` 경로에 쓰이고, 설정은
`DataDirector.SaveSettings()`가 `SettingsData.es3`에 따로 쓴다.

세이브 게임은 **암호화한다**. 설정이 그대로 옮기면 이렇다.

```csharp
ES3Settings eS3Settings = new ES3Settings(ES3.Location.Cache);
eS3Settings.encryptionType = ES3.EncryptionType.AES;
eS3Settings.encryptionPassword = totallyNormalString;   // "Why would you want to cheat?... :o It's no fun. :') :'D"
```

비밀번호가 소스에 그대로 박혀 있어 실질적인 보호는 아니고 변조를 귀찮게 하는 수준이다.
반면 `SettingsData.es3`(설정·플레이어 색·마이크 장치)는 암호화하지 않는다.

세이브 파일에 들어가는 키는 `teamName`, `teamNameChanged`, `dateAndTime`, `timePlayed`,
`playerNames`, `dictionaryOfDictionaries`, `saveVersion` 일곱 개다. `latestSaveVersion`은 1이고,
`LoadGame()`에서 `saveVersion < 1`이면 아이템 배터리 키 형식이 바뀐 것을 정리하는 마이그레이션을 돈다.

백업은 덮어쓰기 대신 이름 변경으로 만든다. 같은 이름의 `.es3`가 이미 있으면 `backupIndex++` 후
`<이름>_BACKUP<n>.es3`로 `File.Move` 한 다음 새로 쓴다. 백업 목록은 `SaveFileGetBackups()`가 훑고,
로드할 때 정규식 `_BACKUP(\d+)$`로 마지막 인덱스를 복원한다.

세이브 슬롯 이름은 기본적으로 `"REPO_SAVE_" + DateTime.Now.ToString("yyyy_MM_dd_HH_mm_ss")`다.
`savedLobbyTypes`에는 `LobbyTypes.Private`만 들어 있어서, `SaveFileSave()`는 비공개 로비에서만
실제로 파일을 쓴다. 공개 로비와 매치메이킹 판은 저장되지 않는다.

**저장 시점**은 `SemiFunc.OnSceneSwitch()` 한 곳으로 모여 있다.

```csharp
if (IsMasterClientOrSingleplayer() && (!_leaveGame || levelPrevious == levelLobby) && !_gameOver
    && levelPrevious != levelMainMenu && levelPrevious != levelLobbyMenu && levelPrevious != levelRecording)
{ SaveFileSave(); }
```

즉 씬이 바뀔 때마다, 호스트만, 게임 오버가 아닐 때만 저장한다. 이어서 `DataDirector.SaveDeleteCheck()`가
호출되어 아레나로 갔거나(=런이 끝났거나) 전멸했거나 레벨 0에서 나갔으면
`SemiFunc.SaveFileDelete(saveFileCurrent)`로 세이브를 **지운다**. 런이 끝나면 세이브가 남지 않는
로그라이크식 처리다. 애플리케이션 종료 시에도 `RunManager.OnApplicationQuit()`이 같은 검사를 한다.

`DataDirector.Setting`에는 게임플레이 설정 외에 `RunsPlayed`(누적 판 수)와
`TutorialPlayed`, `TutorialXxx` 팁 카운터들이 섞여 있다. `RunsPlayedAdd()`는
로비 메뉴에서 레벨이 갱신될 때(`RunManager.UpdateLevel()`)와 싱글플레이 시작 시 올라간다.
`RunsPlayed`는 몬스터 후보 필터(`EnemySetup.runsPlayed`)에도 쓰여, 누적 플레이 수가 적으면
일부 몬스터가 아예 안 나온다.

### `ProgressionManager`

라운드 단위 점수 집계기다. 필드는 `roundPointsHauling`, `roundPointsScouting`,
`roundPointsCombat`, `roundPointsSupport`, `roundPointsTotalHaul`,
보너스 플래그 `roundPointsBonusAllSurvived`/`roundPointsBonusNoDeaths`, 그리고 `penaltyDeath`다.
`ResetAllPointsAtStartOfRound()`가 라운드 시작마다 초기화한다.
값을 넣는 곳은 확인된 범위에서 두 군데다. 죽으면 `PlayerAvatar.PlayerDeathRPC()`가
`BonusNoDeathsFailed()`를 부르고 로컬 플레이어면 `penaltyDeath += 0.05f`를 `0~0.5` 범위로 클램프한다.
추출 시에는 `ExtractionPoint`가 `roundPointsHauling += dollarValueCurrent / 10f * localPlayerHaulScore`,
죽은 머리를 추출 지점까지 옮겨 준 사람에게는 `roundPointsSupport += 100f * localPlayerHaulScore`를 준다.
이 점수를 최종적으로 어디에 쓰는지는 이 파일들만으로는 확인되지 않았다(추정: 미완성이거나
메타 진행용 계측).

### `SessionManager`

세션(런이 아니라 게임 실행) 수준의 얕은 저장소다. 들고 있는 것은 왕관 소유자의 Steam ID
(`crownedPlayerSteamID`)와 마이크 장치 목록뿐이다. `CrownPlayer()`는 호스트만 실행하며
`StatsManager.playerHasCrown`에서 값이 0이 아닌 첫 키를 찾아 `PunManager.CrownPlayerSync()`로 동기화한다.
`Reset()`은 방을 나갈 때(`OnSceneSwitch(_leaveGame: true)`) 호출되어 왕관을 지운다.
`playerHasCrown`은 `ResetAllStats()`에서도 아레나 중이면 지우지 않는 예외를 받는다.

---

## 플레이어

### 이동 (`PlayerController`)

물리 기반 캐릭터다. `Rigidbody`에 `AddRelativeForce`로 속도 차이를 임펄스로 밀어 넣는 방식이며,
`CharacterController`가 아니다. 핵심 루프는 `FixedUpdate()`에 있고
`GameDirector.currentState != Main`이면 통째로 건너뛴다.

속도 계산은 상태별로 목표 속도 벡터 `zero`를 만든 뒤 두 단계 `Lerp`로 부드럽게 따라간다.

```csharp
VelocityRelativeNew = Vector3.Lerp(VelocityRelativeNew, zero, MoveFriction * Time.fixedDeltaTime);
VelocityRelative = Vector3.Lerp(VelocityRelative, VelocityRelativeNew, MoveFriction * Time.fixedDeltaTime);
VelocityRelative.y = vector.y;                       // 수직 성분은 물리에 맡긴다
rb.AddRelativeForce(VelocityRelative - vector, ForceMode.Impulse);
```

상태별 목표 속도는 달리기 중이면 `Mathf.Lerp(MoveSpeed, SprintSpeed, SprintSpeedLerp)`,
웅크림이면 `CrouchSpeed`, 평소면 `MoveSpeed`다. `SprintSpeedLerp`는
`SprintAcceleration * Time.fixedDeltaTime`씩 0→1로 차오르므로 전력 질주는 즉발이 아니라 가속이 붙는다.
입력이 거의 없으면(`InputDirection.magnitude <= 0.1f`) `SprintSpeedLerp`가 0으로 리셋된다.

중력은 직접 넣는다. 접지 상태가 아니고 점프 임펄스도 깃털 상태도 아니면
`rb.AddForce(new Vector3(0f, -CustomGravity * Time.fixedDeltaTime, 0f), ForceMode.Impulse)`를 매 프레임 더한다.

점프는 입력 버퍼와 코요테 타임을 둘 다 쓴다. 입력 버퍼 `JumpInputBuffer = 0.25f`,
접지 버퍼 `JumpGroundedBuffer = 0.25f`, 점프 후 쿨다운 `JumpCooldown = 0.1f`다.
추가 점프는 `JumpExtra`(업그레이드 개수)만큼 공중에서 더 쓸 수 있다. 점프할 때 자기가 잡고 있는
물체 위에 서 있었다면 그 물체를 놓게 하고 점프력을 `JumpForce * 0.5f`로 반감한다.

웅크림은 `CrouchTimeMin = 0.2f`의 최소 유지·최소 해제 시간을 강제해서 연타를 막는다.
웅크린 채 머리 위가 막혀 있으면(`PlayerCollisionStand.instance.CheckBlocked()`) 자동으로
기어가기(`Crawling`)로 전환된다. 넘어진 상태에서도 강제로 기어가기가 된다.
달리기나 웅크림 중에는 `CanInteract = false`가 되어 상호작용이 잠긴다.

슬라이딩은 "달리다가 웅크리기"로 발동한다. 달리는 중에 `CanSlide = true`가 서고, 웅크리는 순간
`SlideTimer = SlideTime`으로 시작해 `SlideDirectionCurrent`가 `SlideDecay`만큼 매 프레임 줄어든다.

### 에너지(스태미나)

에너지는 오직 로컬에서만 계산되고 네트워크로 복제되지 않는다(`PlayerController.EnergyCurrent`).

- 최대치: `EnergyStart` 코드 기본값 100f. 시작 시 `EnergyStart += playerUpgradeStamina * 10f`,
  업그레이드를 사면 `PunManager.UpdateEnergyRightAway()`가 `EnergyStart += 10 * value`를 하고
  현재치를 최대치로 채운다.
- 소모: 달리는 동안 `SprintDrainTimer`가 살아 있는 프레임마다

  ```csharp
  float energySprintDrain = EnergySprintDrain;      // 코드 기본값 1f
  energySprintDrain += SprintSpeedUpgrades;          // 속도 업그레이드가 소모도 늘린다
  energySprintDrain *= Mathf.Lerp(0f, 1f, SprintSpeedLerp);
  EnergyCurrent -= energySprintDrain * Time.fixedDeltaTime;
  ```

  가속이 붙을수록 더 많이 깎이는 구조다. 속도 업그레이드는 `SprintSpeed`와 `SprintSpeedUpgrades`를
  같이 올리므로 빨라지는 대신 지구력이 줄어드는 상충이 설계돼 있다.
- 슬라이딩은 발동 순간 `EnergyCurrent -= 5f` 정액이다.
- 회복: 달리는 동안 `sprintRechargeTimer = sprintRechargeTime`(1f)로 계속 리셋되고,
  멈춘 뒤 1초가 지나야 `EnergyCurrent += sprintRechargeAmount(2f) * Time.deltaTime`로 회복이 시작된다.
  벽을 타는 중(`PhysGrabber.GrabState.Climb`)에는 회복량이 0이 된다. 아레나에서는 회복량 5배,
  대기 시간 0.5배다.
- 에너지가 0이면 달리기 자체가 시작되지 않고(`EnergyCurrent > 0f` 조건), 토글 달리기도 꺼진다.
- 외부 충전: `PlayerBattery`가 트럭의 충전 장치로, 잡고 있는 사람 수만큼
  0.5초마다 `EnergyCurrent += 1f * amountPlayersGrabbing`을 준다.
- `PlayerController.Revive()`는 에너지를 최대치로 되돌린다.

`EnergyUI`는 `Mathf.Ceil(EnergyCurrent)` / `Mathf.Ceil(EnergyStart)` 두 숫자를 그대로 그린다.

### 체력 (`PlayerHealth`)

- 기본값은 `health = 100`, `maxHealth = 100`(코드 기본값). 실제 최대치는
  `100 + playerUpgradeHealth * 20`이고, 체력 업그레이드 1개당 최대치 +20에 즉시 +20 회복이 붙는다.
- 소유권 규칙이 엄격하다. `Hurt()`는 `GameManager.Multiplayer() && !photonView.IsMine`이면 반환하므로
  피해 적용은 **피해자 본인 클라이언트**만 한다. 남을 때리려면 `HurtOther()`가
  `RPC("HurtOtherRPC", RpcTarget.All, ...)`로 던지고, 받는 쪽에서
  `SemiFunc.MasterAndOwnerOnlyRPC()`로 발신자를 검증하고 `hurtPosition`이 자기 위치에서 2m 이내인지까지
  확인한 뒤 자기 `Hurt()`를 부른다. 위치 검증을 두는 것은 원격 치트 방지로 보인다(추정).
- 결과는 `RPC("UpdateHealthRPC", RpcTarget.Others, health, maxHealth, effect, hurtByHeal)`로 알린다.
- 무적: `invincibleTimer`가 있고 `InvincibleSet(float)`으로만 설정된다. 실제 호출은 두 곳뿐이다.
  부활 직후 1초(`PlayerAvatar.ReviveRPC` → `playerHealth.InvincibleSet(1f)`)와
  아레나 시작 5초다. **피격 후 무적 시간은 없다.**
- "구제(saving grace)"라는 자비 규칙이 있다.

  ```csharp
  if (savingGrace && damage <= 25 && health > 5 && health <= 20)
  { health -= damage; if (health <= 0) health = Random.Range(1, 5); }
  ```

  체력이 6~20 사이에 25 이하의 피해를 받으면 죽지 않고 1~4로 남는다. 낙하·충돌 계열 피해가
  `savingGrace: true`로 들어온다(`PlayerTumble.BreakImpact()`가 그 예다).
- 체력 회복은 `Heal()`이고 역시 소유자만 적용한다. `HealOther()`는 `RpcTarget.All`로 던져
  대상 본인이 실행한다.
- 트럭 최종 회복: 추출을 전부 끝내고 트럭에 들어가면 `PlayerAvatar.FinalHealCheck()`가 초당 5회 검사하다가
  `FinalHeal()`을 부른다. 회복량은 달 레벨에 따라 50 / 35 / 25이고 런당 한 번(`finalHeal` 플래그)이다.
- `HurtVignette`는 체력이 10 미만일 때 화면 가장자리를 붉게 물들이는 순수 표현 요소다.
- `CameraShake`는 `Shake(strength, duration)`와 거리 감쇠 버전 `ShakeDistance(strength, min, max, position, duration)`
  두 가지를 제공하고, `GameDirector`가 `CameraShake`(흔들림)와 `CameraImpact`(짧은 충격) 두 인스턴스를 들고 있다.

### 넘어짐 (`PlayerTumble`)

넘어짐은 "캐릭터 컨트롤러를 끄고 물리 인형(`PhysGrabObject`)으로 대체하는" 상태다.
`isTumbling`이 서면 `PlayerController`가 `col.enabled = false; rb.isKinematic = true;`로 자기 몸을 끄고,
매 `FixedUpdate`마다 자기 위치를 `tumble.transform.position + Vector3.down * 0.3f`로 따라가게 한다.

- **유발**: 플레이어가 직접 `InputKey.Tumble`을 눌러도 되고(`_playerInput: true`), 게임이 강제로
  걸 수도 있다(`_playerInput: false`). 강제 유발 지점은 대부분 몬스터다 —
  `EnemyHidden`, `EnemyOogly`, `EnemySlowMouth`, `EnemySpinny`, `EnemyThinMan`, `EnemyUpscream`,
  `FloaterAttackLogic`, `EnemyHeartHugger` 계열. 그 밖에 `HurtCollider`, 물체 충돌
  (`PhysGrabObjectImpactDetector`), 차량, 상점 기물 등이 있다.
- **네트워크**: `TumbleRequest()`는 `RpcTarget.MasterClient`로 요청을 보내고, 마스터가
  `TumbleSet()`을 실행해 `RpcTarget.All`로 결과를 뿌린다. 상태 전이의 권위는 호스트에 있다.
- **해제**: 로컬에서 `InputKey.Jump` 또는 `InputKey.Tumble`을 누르면 풀리고, 추가로
  "0.5초 이상 움직임이 없는 상태(`notMovingTimer > 0.5f`)에서 이동 입력을 넣으면" 자동으로 해제된다.
  단 `tumbleOverride`가 걸려 있으면(누가 나를 붙잡고 있으면) 풀 수 없다.
  누군가 나를 잡고 있는 동안 `TumbleOverrideTime(1f)`이 계속 갱신되기 때문이다.
  플레이어가 죽어서 `isDisabled`가 되면 넘어짐이 강제 해제된다.
- **탈출**: 잡혀 있을 때 점프 키를 누르면 `breakFreeCooldown = 0.5f`를 두고
  `TumbleForce(카메라 앞 방향 * 15f)`와 `TumbleTorque(오른쪽 * 10f)`로 튕겨 나가며,
  나를 잡고 있던 사람들에게 `SemiFunc.PlayerCarryThrust()`가 들어간다.
- **흉기가 된다**: 넘어진 몸의 속도가 4를 넘고 카트 안이 아니면 `hurtCollider`가 켜진다.
  아무도 나를 안 잡고 있으면 `hurtCollider.enemyLogic = true`가 되어 몬스터에게도 피해를 준다.
- **낙하 피해**: `ImpactHurtSet(time, damage)`로 예약해 두고 `BreakImpact()`에서
  `playerHealth.Hurt(impactHurtDamage, savingGrace: true)`로 적용한다. 구제 규칙이 붙어 있어
  낙하로 즉사하기 어렵다.
- **연출**: 넘어진 로컬 플레이어는 화면이 55도로 줌아웃되고 비네트가 걸린다.
- 업그레이드가 관여한다. `tumbleLaunch`가 있으면 자발적 넘어짐에
  `카메라 앞 방향 * (3f * tumbleLaunch)`의 발사력이 붙는다. 그 외 `TumbleClimb`, `TumbleWings` 업그레이드가 있다.

### 죽음과 머리 (`PlayerAvatar`, `PlayerDeathHead`)

체력이 0 이하가 되면 `PlayerHealth.Hurt()`가 `playerAvatar.PlayerDeath(enemyIndex)`를 부르고,
이것이 `RpcTarget.All`로 `PlayerDeathRPC`를 던진다. 죽은 사람 본인 클라이언트에서는
잡고 있던 물체를 놓고, 아바타 메시를 끄고, `GameDirector.DeathStart()`로 사망 연출 상태로 들어간다.

연출이 끝나면 `PlayerDeathDone()`이 `isDisabled = true`를 세우고,
`deathSpot`을 인스턴스화하고, 인벤토리를 강제 해제하고,
`playerDeathHead.Trigger()`로 머리를 활성화한 뒤 아바타 게임오브젝트 자체를 꺼 버린다.

`PlayerDeathHead`는 `PhysGrabObject`를 달고 있는 **집을 수 있는 물리 오브젝트**다.
살아 있는 사람이 이 머리를 들고 옮길 수 있고, 머리를 어디로 가져가느냐가 부활 규칙을 정한다.

부활 경로는 세 가지다.

1. **추출 지점**: `PlayerDeathHead.Revive()`는 `triggered && inExtractionPoint`일 때만
   `playerAvatar.Revive()`를 부른다. 추출이 실행될 때 `ExtractionPoint`가
   `DestroyAllPhysObjectsInHaulList()`/`...ShopList()`에서 전 플레이어의 머리에 대해
   `player.playerDeathHead.Revive()`를 돌린다. 즉 **죽은 사람의 머리를 추출 지점 안에 넣어 두면
   추출과 함께 되살아난다.** 머리를 옮겨 준 사람에게는 `roundPointsSupport += 100f * localPlayerHaulScore`가 붙는다.
2. **트럭**: 추출을 전부 끝낸 뒤(`allExtractionPointsCompleted`) 트럭 치유기가 열려 있으면
   (`TruckHealer.instance.currentState == TruckHealer.State.Open`) 머리가 트럭 안에 들어온 상태로
   `inTruckReviveTimer`가 2초에서 0까지 내려가고, 그때 `playerAvatar.Revive(_revivedByTruck: true)`가 실행된다.
   이 판정은 `SemiFunc.IsMasterClientOrSingleplayer()` 안에서만 돈다.
3. **부활 아이템**: `ItemReviveItem.ReviveAndDestroy()`가 호스트에서
   `playerTarget.Revive()` 후 `playerTarget.playerHealth.HealOther(49, effect: true)`로 49를 회복시키고
   아이템을 파괴한다. 대상 머리가 사라지는 등의 예외 상황이면 `_failSafe`로 트럭 안전 스폰 지점에
   넘어진 상태로 떨어뜨린다.

`PlayerAvatar.ReviveRPC()`는 마스터 전용 RPC이고, 부활 위치를
`playerDeathHead.physGrabObject.centerPoint - Vector3.up * 0.25f`, 즉 머리가 있던 자리로 잡는다.
부활하면 1초 무적이 붙고, 몬스터의 조사 반응이 잠깐 꺼지며(`OverrideDisableEnemyInvestigate`),
`PlayerController.Revive()`가 입력을 0.5초 막고 0.2초 kinematic으로 만든 뒤 기어가기 자세로 놓고
에너지를 최대로 채운다. `PlayerReviveEffects.Trigger()`가 빛·파티클·소리와 거리 감쇠 화면 흔들림을 낸다.

머리가 레벨 밖으로 떨어지면 `outsideLevelTimer`를 재다가 `TruckSafetySpawnPoint`로 순간이동시킨다.

### 관전 (`SpectateCamera`)

관전 카메라는 `State`가 `Death`, `Normal`, `Head` 세 가지다.
`Death`는 죽는 순간의 연출 시점이고, `Normal`은 살아 있는 다른 플레이어를 따라다니는 일반 관전,
`Head`는 자기 머리의 시점으로 들어가는 모드다.

`Normal`에서 `InputKey.SpectateNext`/`SpectatePrevious`로 대상을 바꾼다.
`Head` 모드는 자원을 소비한다. `headEnergy`는 0~1 값으로,

- `Normal` 상태에서 `headEnergy += Time.deltaTime / 100f`로 차오르고,
  0.25 이상이면 `headEnergyEnough = true`가 되어 `InputKey.Interact`로 머리 시점에 들어갈 수 있다.
- `Head` 상태에서는 `headEnergy -= Time.deltaTime / num`으로 줄어드는데,
  `num`은 기본 25초에서 시작해 `upgradeDeathHeadBattery` 하나마다 5초씩(그 증가분이 매번 0.95배로 체감) 늘어난다.
  머리로 점프를 충전 중이면 소모가 두 배다.
- 0이 되면 자동으로 `Normal`로 돌아간다.

즉 죽은 사람도 자기 머리를 조금씩 움직여 스스로 이동할 여지가 있고, 그 시간이 업그레이드 대상이다.
튜토리얼 레벨에서는 관전 대기가 끝나면 전원을 그냥 `Revive()` 해 버린다.

### 왕관과 음성

`PlayerCrown`은 왕관 소유자의 머리 위에 메시를 붙여 따라다니게 하는 표시용 컴포넌트다.
소유자는 `StatsManager.playerHasCrown`(Steam ID 하나만 1)로 정해지고
`SessionManager.CrownPlayer()` → `PunManager.CrownPlayerSync()`로 동기화된다.

음성 채팅은 Photon Voice다. `PlayerVoiceChat`이 `Recorder`와 `Speaker`를 들고 3D 공간 음향으로 재생하며,
`NetworkManager.Start()`에서 `PhotonNetwork.Instantiate("Voice", ...)`로 별도 오브젝트를 띄운다.
죽거나 아웃트로에 들어가면 `voiceChat.ToggleMixer(_lobby: true)`로 믹서를 바꿔 공간감을 없앤다.

---

## 멀티플레이 구조

### 싱글과 멀티의 분기

기준은 단 하나, `GameManager.gameMode`(0 = 싱글, 1 = 멀티)다. 이것을 감싼 헬퍼가
`SemiFunc`에 모여 있다.

```csharp
public static bool IsMultiplayer() { return GameManager.instance.gameMode == 1; }
public static bool IsMasterClient() { return GameManager.Multiplayer() ? PhotonNetwork.IsMasterClient : false; }
public static bool IsNotMasterClient() { return GameManager.Multiplayer() ? !PhotonNetwork.IsMasterClient : false; }
public static bool IsMasterClientOrSingleplayer() { /* 싱글이거나 마스터면 true */ }
```

코드 전반의 관용구는 이렇다. 권위 있는 판정은 `if (SemiFunc.IsMasterClientOrSingleplayer())`로 감싸고,
그 결과를 뿌릴 때는 `if (SemiFunc.IsMultiplayer()) photonView.RPC("XxxRPC", RpcTarget.All, ...); else XxxRPC();`
로 **같은 RPC 메서드를 싱글에서는 직접 호출**한다. 덕분에 로직이 한 벌로 유지된다.
`RoundDirector.RequestExtractionPointActivation()`이 전형적인 예다.

### RPC 보안 검증

받는 쪽은 거의 모든 RPC 첫 줄에서 발신자를 검증한다. 세 종류가 있다.

- `SemiFunc.MasterOnlyRPC(info)` — 싱글이거나 발신자가 마스터일 때만 통과.
- `SemiFunc.OwnerOnlyRPC(info, photonView)` — 발신자가 그 뷰의 소유자일 때만 통과.
- `SemiFunc.MasterAndOwnerOnlyRPC(info, photonView)` — 마스터 또는 소유자면 통과.

실패하면 디버그 빌드에서 메서드 이름과 플레이어 이름을 찍고 false를 돌려준다.
역할 분담이 여기서 드러난다. 레벨 전환·라운드 목표·추출 활성화 같은 **세계 상태는 마스터 전용**,
자기 체력·죽음·넘어짐 해제·아웃트로 완료 같은 **개인 상태는 소유자 권한**이다.

### 접속과 방

`NetworkConnect`가 `RoomOptions { MaxPlayers = GameManager.instance.maxPlayers, IsVisible = ... }`로
방을 만든다. 커스텀 로비는 `IsVisible = true`, 비공개(친구) 로비는 `IsVisible = false`에
`PhotonNetwork.JoinOrCreateRoom(RoomName, ...)`을 쓴다. 매치메이킹은
`PhotonNetwork.JoinRandomOrCreateRoom(null, expectedMaxPlayers, MatchmakingMode.FillRoom, ...)`이며
`expectedMaxPlayers`는 `matchmakingMaxPlayers`가 참이면 6, 아니면 0(무제한)이다.

**로비 최대 인원**은 `GameManager.maxPlayersDefault = 6`, `maxPlayers = 6`(코드 기본값)이고
`maxPlayersPhoton = 20`이 상한이다. `SetMaxPlayers()`는 `Math.Clamp(target, 1, maxPlayersPhoton)`로 조이고
런타임에 `PhotonNetwork.CurrentRoom.MaxPlayers`, Steam 로비 `MaxMembers`, Discord 파티 크기를 함께 갱신한다.

Steam 연동은 `SteamManager`가 맡는다. 마스터가 `HostLobby(_open)`으로 Steam 로비를 만들고
`SetLobbyData(PhotonNetwork.CurrentRoom.Name)`로 Photon 방 이름을 심은 뒤,
`RunManagerPUN.SendJoinSteamLobby()`가 `RPC("JoinSteamLobbyRPC", RpcTarget.OthersBuffered, 로비ID)`로
나머지를 같은 Steam 로비에 넣는다. 공개 여부는 `UnlockLobby(_open)`이
`SetPublic()` / `SetPrivate()+SetFriendsOnly()`로 갈라 준다. 게임이 시작되면
`SetJoinable(false)`로 잠근다. `RunManager.Update()`는 Steam이 유효하지 않으면
`Application.Quit()`을 부를 만큼 Steam 의존이 강하다.

### 동기화 설정과 프리팹 풀

`NetworkManager.Start()`가 멀티일 때 `PhotonNetwork.SerializationRate = 25;`와
`PhotonNetwork.SendRate = 25;`를 지정한다. 즉 상태 복제는 25Hz다.
`PlayerAvatar` 프리팹과 `Voice` 프리팹은 각자가 `PhotonNetwork.Instantiate()`로 띄운다.

`MultiplayerPool`은 `IPunPrefabPool` 구현체로, `Resources.Load` 또는 `AssetBundles/<번들>/<경로>`에서
프리팹을 찾아 `ResourceCache` 딕셔너리에 캐시한다. 인스턴스화 직전에 원본을 비활성화했다가
복제 후 되돌려서 `Awake`가 미리 도는 것을 막는 기법을 쓴다.
싱글플레이용으로는 `RunManager.singleplayerPool`이라는 별도 딕셔너리가 있다.

### 로딩 동기화

레벨 생성은 인원 수를 기다린다. `LevelGenerator`는
`while (gameMode == 1 && ModulesReadyPlayers < PhotonNetwork.CurrentRoom.PlayerCount)`로 모듈 준비를,
`EnemyReadyPlayers >= PhotonNetwork.CurrentRoom.PlayerCount`로 몬스터 준비를 기다린다.
`NetworkManager.Update()`는 마스터에서 `instantiatedPlayerAvatars >= PlayerCount`가 되면
`RPC("AllPlayerSpawnedRPC", RpcTarget.AllBuffered)`를 보내 `LevelGenerator.AllPlayersReady`를 세운다.
로딩이 25초를 넘으면 `LoadingUI.stuckActive`가 켜지고 메뉴 키로 강제 탈출할 수 있다.

### 통계 동기화

`SemiFunc.StatSyncAll()` → `PunManager.SyncAllDictionaries()`가 씬 전환 때마다 실행된다.
마스터만 실행하며, `dictionaryOfDictionaries`를 딕셔너리 4개씩 묶어
`ExitGames.Client.Photon.Hashtable`로 변환해 `RPC("ReceiveSyncData", RpcTarget.Others, chunk, isFinalChunk)`로
쪼개 보낸다. 마지막 청크를 받으면 수신 측이 `statsSynced = true`를 세운다.
보낼 때 `stripTheseDictionaries` 기본값과 같은 항목, 그리고 현재 방에 없는 플레이어의 항목은 걸러낸다.
받는 쪽은 해당 딕셔너리를 `Clear()`하고 통째로 덮어쓴다. 즉 통계의 정본은 언제나 호스트다.

### 늦게 들어온 사람

명시적인 "게임 중 난입" 경로는 보이지 않는다. 대신 두 가지 장치가 있다.
하나는 `UpdateLevelRPC`를 `RpcTarget.OthersBuffered`로 보내 나중에 들어온 클라이언트도
현재 레벨과 `levelsCompleted`를 받게 하는 것이고, 다른 하나는 씬 전환마다 도는 통계 전체 동기화다.
`SteamManager.OnLobbyMemberJoined`와 `NetworkManager.OnPlayerEnteredRoom`은 로그와 로비 UI 갱신만 한다.
게임이 시작되면 Steam 로비가 `SetJoinable(false)`가 되므로, 실질적으로 난입은 로비 대기 중에만
가능한 것으로 보인다(추정).

호스트가 나가면 `OnMasterClientSwitched`가 "Host disconnected" 팝업을 띄우고
`TriggerLeavePhotonRoomForced()`로 전원을 내보낸다. 즉 **호스트 이전(migration)을 하지 않는다.**
`RunManager.masterSwitched` 필드는 있으나 사용처가 확인되지 않는다.
강퇴·차단은 `PhotonNetwork.RaiseEvent(123/124, ...)` 커스텀 이벤트로 처리하고,
받는 쪽 코드 199는 "호스트에게 강퇴당함" 팝업이다.

### 세이브의 소유

세이브는 전적으로 호스트 것이다. `StatsManager.SaveGame()`과 `LoadGame()`은 첫 줄에서
`SemiFunc.IsMasterClientOrSingleplayer()`가 아니면 반환하고, `PlayersAddAll()`도 마찬가지다.
클라이언트는 자기 파일을 갖지 않고 호스트가 동기화해 준 딕셔너리만 메모리에 들고 있다가
방을 나가면 잃는다. 업그레이드 구매 이력이 Steam ID 키로 저장되므로,
같은 호스트의 세이브에 다시 들어가면 개인 업그레이드가 복원된다.

### 문과 상호작용

문은 물리다. `PhysGrabHinge`가 Unity `HingeJoint`를 직접 잡아
`joint.anchor = hingePoint.localPosition`으로 축을 설정하고, 힌지가 열린 각도에 따라 소리와
파괴(부서지면 카메라 흔들림 `hingeBreakShake = 3f`)를 처리한다. 즉 문은 스크립트 애니메이션이 아니라
플레이어가 밀고 당기는 관절이다.

트럭 뒷문(`TruckDoor`)은 예외로 커브 애니메이션이다. 레벨 생성이 끝나고 `doorDelay = 2f`가 지나면
`doorEval`이 초당 1.5씩 올라가며 `startYPosition + 2.5f * doorCurve.Evaluate(doorEval)`로 올라간다.
문이 열린 뒤 누군가 트럭을 벗어나면 `ActivateTheFirstExtractionPointAutomaticallyWhenAPlayerLeaveTruck()`이
첫 추출 지점을 자동 활성화한다. 즉 "출발하면 자동으로 첫 목표가 켜진다".

`Interaction.cs`는 `InteractionType` 열거형(`None, VacuumCleaner, Duster, Sledgehammer, DirtFinder, Picker`)과
스프라이트만 들고 있는 껍데기로, 청소 관련 전작의 잔재로 보인다(추정).

### 음악

`MusicManager`는 `MusicMixerOn/Off/ScareOnly` 세 개의 `AudioMixerSnapshot`과
`MusicEnemyNear`, `MusicEnemySighting`, `MusicEnemyCatch` 세 개의 상태 음악을 들고 있고,
`Update()`가 하는 일은 "목격 음악이 켜져 있으면 근접 음악의 볼륨을 낮춘다" 한 줄뿐이다.
레벨 시작/종료 시 `GameDirector`가 `MusicMixerOn.TransitionTo(0.1f)` 식으로 스냅샷을 전환한다.

---

## 수치

소스에서 직접 읽은 상수와 공식만 모았다. `AnimationCurve`로 정의된 값은 인스펙터에 있어
소스만으로는 알 수 없다.

| 항목 | 값·공식 | 위치 |
|---|---|---|
| 난이도 배율 1 | `Clamp01(levelsCompleted / 9f)` | `SemiFunc.RunGetDifficultyMultiplier1` |
| 난이도 배율 2 | `Clamp01((levelsCompleted - 9) / 10f)` | `SemiFunc.RunGetDifficultyMultiplier2` |
| 난이도 배율 3 | `Clamp01((levelsCompleted - 19) / 10f)` | `SemiFunc.RunGetDifficultyMultiplier3` |
| 달 레벨 | `(levelsCompleted + 1) / 5` | `RunManager.CalculateMoonLevel` |
| 목표 총액 기준 | 맵 총가치 `haulGoalMax`의 0.7배 × 커브 | `RoundDirector.StartRound` |
| 추출 지점당 목표 | `haulGoal / extractionPoints` | `ExtractionPoint.cs:1033` |
| 통화 환산 | `Mathf.CeilToInt(extractionHaul / 1000)` | `ExtractionPoint.cs:1930` |
| 맵 모듈 수 | `Min(5 + levelsCompleted, 10)` (+ 레벨 10~14에서 `Min(levelsCompleted-9, 5)`) | `LevelGenerator.cs:601` |
| 추출 지점 수 | 모듈 15/10/8/6 이상 → 4/3/2/1개 | `LevelGenerator.cs:611` |
| 최대 인원(기본/상한) | 6 / 20 | `GameManager.maxPlayersDefault`, `maxPlayersPhoton` |
| 네트워크 전송률 | `SerializationRate = 25`, `SendRate = 25` | `NetworkManager.Start` |
| 체력 기본/최대 | 100 / `100 + playerUpgradeHealth * 20` | `PlayerHealth`, `StatsManager.GetPlayerMaxHealth` |
| 체력 업그레이드 1개 | 최대치 +20, 즉시 +20 회복 | `PunManager.UpdateHealthRightAway` |
| 구제 규칙 | 체력 6~20에 25 이하 피해 → `Random.Range(1, 5)`로 생존 | `PlayerHealth.Hurt` |
| 부활 무적 | 1초 | `PlayerAvatar.ReviveRPC` |
| 아레나 시작 무적 | 5초 | `Arena.cs:287` |
| 에너지 기본 | 100 (`EnergyStart`) | `PlayerController` 코드 기본값 |
| 에너지 업그레이드 | +10 / 1단계 | `PlayerController.LateStart`, `PunManager` |
| 달리기 소모 | `(1f + SprintSpeedUpgrades) * Lerp(0,1,SprintSpeedLerp)` / 초 | `PlayerController.FixedUpdate` |
| 슬라이딩 소모 | 5 (정액) | `PlayerController.FixedUpdate` |
| 에너지 회복 | 정지 1초 후 초당 2 | `sprintRechargeTime = 1f`, `sprintRechargeAmount = 2f` |
| 아레나 에너지 | 회복량 ×5, 대기 ×0.5 | `PlayerController.Update` |
| 배터리 충전 | 0.5초마다 `1 * 잡은 사람 수` | `PlayerBattery` |
| 이동 속도(코드 기본값) | `MoveSpeed = 0.5f`, `SprintSpeed = 1f`, `CrouchSpeed = 1f` | `PlayerController` (실제 값은 프리팹) |
| 이동 마찰 | `MoveFriction = 5f` | `PlayerController` |
| 점프 | `JumpForce = 20f`, 쿨다운 0.1초, 입력·접지 버퍼 각 0.25초 | `PlayerController` |
| 물체 위 점프 | `JumpForce * 0.5f` + 강제 놓기 | `PlayerController.FixedUpdate` |
| 중력 | `CustomGravity = 20f` (수동 적용) | `PlayerController` |
| 웅크림 최소 시간 | `CrouchTimeMin = 0.2f` | `PlayerController` |
| 넘어짐 발사 | `카메라 앞 * (3f * tumbleLaunch)` | `PlayerTumble.TumbleSet` |
| 넘어짐 탈출 | 앞 방향 15f 힘 + 오른쪽 10f 토크, 쿨다운 0.5초 | `PlayerTumble.Update` |
| 넘어짐 흉기화 | 속도 4 초과 | `PlayerTumble.Update` |
| 넘어짐 자동 해제 | 정지 0.5초 후 이동 입력 | `PlayerTumble.Update` |
| 트럭 부활 대기 | 2초 | `PlayerDeathHead.inTruckReviveTimer` |
| 부활 아이템 회복 | 49 | `ItemReviveItem.ReviveAndDestroy` |
| 트럭 최종 회복 | 달 0/1/2+ → 50/35/25 | `PlayerAvatar.FinalHealRPC` |
| 머리 시점 유지 | 기본 25초, 배터리 업그레이드당 +5초(증가분 ×0.95 체감) | `SpectateCamera.HeadEnergyLogic` |
| 머리 시점 진입 | `headEnergy >= 0.25`, 회복은 초당 0.01 | `SpectateCamera.HeadEnergyLogic` |
| 상점 인원 할인 | 인원 1명당 10%, 최대 6명 | `ShopManager.UpgradeValueGet` |
| 파워 크리스탈 충전 | 구매 시 `chargingStationChargeTotal += 17` (상한 100) | `ExtractionPoint.cs:2255` |
| 런 시작 통계 | `level 0 / currency 0 / lives 3 / chargingStationCharge 1 / chargingStationChargeTotal 100 / totalHaul 0` | `StatsManager.RunStartStats` |
| 세이브 경로 | `persistentDataPath/saves/<이름>/<이름>.es3` | `StatsManager.SaveGame` |
| 세이브 암호화 | ES3 AES, 비밀번호는 소스 내 하드코딩 문자열 | `StatsManager.SaveGame` |
| 세이브 버전 | `latestSaveVersion = 1` | `StatsManager` |
| 저장되는 로비 종류 | `LobbyTypes.Private`만 | `StatsManager.savedLobbyTypes` |
| 죽음 페널티 | `penaltyDeath += 0.05f`, 0~0.5 클램프 | `PlayerAvatar.PlayerDeathRPC` |
| 로딩 멈춤 판정 | 25초 | `NetworkManager.Update` |
| 체력 위험 연출 | 체력 10 미만 | `HurtVignette.Update` |

---

## CL9 시사점

관찰만 적는다. 결정은 하지 않는다.

- **진행도가 정수 하나다.** R.E.P.O.는 `levelsCompleted` 하나에서 모든 난이도를 파생시킨다.
  이것을 0~1 배율 세 개(0~9, 10~19, 20~29)로 나누고 각 구간에 다른 `AnimationCurve`를 물리는 구조라,
  디자이너가 코드를 건드리지 않고 곡선만으로 난이도를 다시 그릴 수 있다.
  CL9의 "하루 루프"에서 날짜(day)가 같은 역할을 할 수 있고, UE에서는 `UCurveFloat` 에셋이 대응물이다.
- **목표가 절대값이 아니라 맵에 실제로 놓인 총가치의 비율이다.** `haulGoalMax * 0.7f * curve`이므로
  스폰 결과가 나쁜 맵에서 목표가 불가능해지는 일이 구조적으로 생기지 않는다.
  CL9에서 주문서의 요구량을 절대 개수로 둘지, 그 원정지에 실제로 배치된 재료의 비율로 둘지는
  같은 성질의 선택이다.
- **인원수가 난이도를 올리지 않는다.** 몬스터 수도 목표 금액도 인원을 보지 않고,
  상점 가격만 인원당 10% 싸진다. 즉 "여럿이면 쉬워진다"를 그대로 허용했다.
  CL9가 1~4인을 지원할 때 같은 결정을 할지, 인원 비례 스케일링을 넣을지가 갈림길이다.
- **판정 주체가 일관된다.** 모든 권위 판정이 `IsMasterClientOrSingleplayer()` 안에 들어 있고,
  전파는 `IsMultiplayer()`면 RPC, 아니면 같은 메서드 직접 호출이라는 한 가지 관용구로 통일돼 있다.
  덕분에 싱글 전용 분기가 따로 없다. CL9의 리슨 서버(호스트=GameMode 서버 전용 판정)와
  성격이 같고, UE에서는 `HasAuthority()` + Multicast RPC / `Server_` RPC가 대응한다.
  다만 R.E.P.O.는 개인 상태(체력·죽음)를 소유자 권한으로 남겨 두었다는 점이 다르다 —
  CL9는 HP를 PlayerState에 두기로 했으므로 이 부분은 더 서버 권위 쪽에 가깝다.
- **RPC마다 발신자를 검증한다.** `MasterOnlyRPC` / `OwnerOnlyRPC` / `MasterAndOwnerOnlyRPC` 세 가지를
  거의 모든 RPC 첫 줄에 넣었고, 피해 RPC는 위치가 2m 이내인지까지 확인한다.
  UE는 `Server_` RPC에 소유권 검사가 내장돼 있어 사정이 낫지만, Multicast로 넘기는 값에 대한
  같은 종류의 위생 검사는 여전히 필요하다.
- **잡혀도 바로 죽지 않는다를 물리 상태로 구현했다.** 넘어짐(`PlayerTumble`)은 별도 HP 상태가 아니라
  "컨트롤러를 끄고 물리 인형으로 대체"하는 전환이고, 누가 붙잡고 있으면
  `tumbleOverride`가 계속 갱신되어 스스로 못 풀며, 점프 키 연타로 탈출력을 낸다.
  CL9의 "잡혀도 바로 죽지 않는다"에 그대로 대응하는 구조다.
- **죽은 사람이 물건이 된다.** 죽으면 머리가 `PhysGrabObject`가 되어 동료가 들고 옮길 수 있고,
  추출 지점 안에 두거나 트럭에 넣으면 부활한다. 즉 "부활"이 별도 UI가 아니라
  기존 물체 운반 규칙을 재사용한다. CL9가 짐 운반 시스템을 이미 가진다면,
  쓰러진 동료를 같은 규칙의 짐으로 만드는 선택지가 있다.
- **죽은 사람도 할 일이 있다.** 머리 시점(`Head` 모드)에 에너지를 붙여 유한 자원으로 만들고,
  그 지속 시간 자체를 상점 업그레이드 대상으로 뒀다. 관전이 순수한 대기 시간이 되지 않게 한 장치다.
- **씬 전환이 단일 관문이고, 그 관문에서 저장한다.** `ChangeLevel` → `OnSceneSwitch` → `SaveFileSave`가
  한 줄기이며, 전멸·아레나·레벨 0 이탈이면 오히려 세이브를 지운다.
  CL9의 하루 경계(밤 만료 → 귀환)가 같은 관문 역할을 할 수 있다.
- **저장 구조가 문자열 키 딕셔너리 한 뭉치다.** `Dictionary<string, Dictionary<string, int>>` 하나에
  전부 몰아넣고, 기본값과 같은 항목은 빼고 저장해 파일을 줄인다. 타입 안전성은 포기했지만
  항목을 추가할 때 직렬화 코드를 건드릴 필요가 없다. UE의 `USaveGame`은 정반대(강타입 UPROPERTY)라
  같은 유연성을 원하면 `TMap<FName, int32>` 몇 개를 두는 형태가 대응물이 된다.
- **세이브는 호스트만 갖는다.** 클라이언트는 씬 전환마다 청크로 쪼갠 통계를 받아 통째로 덮어쓰고,
  방을 나가면 잃는다. 개인 업그레이드가 Steam ID 키로 저장되므로 같은 호스트의 세이브에
  다시 들어가면 복원된다. CL9의 리슨 서버 구성에서 세이브 소유를 어떻게 정할지와 같은 문제다.
- **호스트 이전을 하지 않는다.** 마스터가 나가면 팝업을 띄우고 전원을 로비로 내보낸다.
  구현 비용을 아낀 대신 호스트 이탈이 곧 판의 종료다.
- **상태 머신이 얇다.** `GameDirector.gameState` 여덟 개, `ExtractionPoint`·`Arena`·`TruckHealer`·
  `SpectateCamera`·`ResultScreenUI` 모두 같은 형태의 `stateStart` 임펄스 + `stateTimer` 카운트다운
  패턴을 쓴다. 프레임워크 없이 `switch` 하나로 처리하는데, 코드가 반복되는 대신 읽기 쉽다.
- **공용 틱이 있다.** `GameDirector.fpsImpulseN`과 `SemiFunc.PerSecond(freq, obj)`로
  "초당 N회" 로직을 싸게 만들고, 후자는 인스턴스 ID로 위상을 흩뜨려 같은 프레임에 몰리는 것을 막는다.
  UE의 `SetTimer`나 틱 간격 조정과 같은 목적이고, 위상 분산 아이디어는 그대로 가져올 만하다.
- **문을 물리 힌지로 두었다.** `PhysGrabHinge`가 `HingeJoint`를 직접 쓰고, 부서지면 관절이 끊어진다.
  애니메이션 문보다 상황이 많이 생기고(몬스터가 밀고 들어옴, 문이 막힘) 대신 동기화 비용이 든다.
- **트럭 화면이 게임의 진행 UI 전부다.** 목표·진행도·출발 승인·남은 사람 이름을 전부 CRT 텍스트로
  처리하고, 승인 조건("전원 승차")을 화면 안에서 다룬다. 별도 로비 UI를 만들지 않고
  월드 안의 물체 하나로 끝낸 사례다.
