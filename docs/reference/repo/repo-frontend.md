# 참고 — R.E.P.O. 의 프런트엔드와 세션 흐름

> 설치본 R.E.P.O. 의 부팅 → 메인 메뉴 → 호스트/조인 → 로비 메뉴 → (트럭 로비) → 레벨 → 사망 → 전멸(아레나·게임오버) → 복귀 흐름을 정리한 노트다.
> 조사 날짜 2026-10-05. 방법은 디컴파일 코드 읽기다(씬·프리팹의 직렬화 값은 이 노트에서 읽지 않았다). **플레이 실측이 아니다.**
> 코드·에셋은 이식하지 않는다. 디컴파일 소스는 옮겨 적지 않았고 구조와 수치만 적었다.

## 0. 읽는 법

- 출처 표기는 모두 디컴파일 소스 `.cs`의 `파일:줄`이다. 줄 번호는 디컴파일 결과 기준이다.
- 선행 문서 [repo-run.md](repo-run.md)가 이미 다룬 것(ChangeLevel 분기, GameDirector 상태 8개, 전멸→아레나, 세이브 구조·암호화·백업, 호스트 이전 없음, 머리·부활 규칙, 트럭 화면 페이지 enum, 최대 인원 6/20)은 되풀이하지 않았다. 이 문서는 "그 문서에 없는 것"과 "그 문서를 고치는 것"만 적는다.
- 코드에서 직접 읽은 것만 단정하고, 나머지는 `(추정)` 또는 `(프리팹 값, 미확인)`으로 구분했다. 메뉴 버튼의 화면 표시 문구, 애니메이터 클립 길이, 씬·프리팹에 연결된 UnityEvent는 디컴파일 코드에 나타나지 않으므로 이 문서에서 확정하지 못했다(13절에 목록).
- 게임 안의 문구는 대부분 로컬라이즈 에셋이고 코드에는 영어 기본값만 있다. 저작권 때문에 원문을 옮기지 않고 뜻으로 적었다.

### 0.1 repo-run.md에 대한 정정·보강

1. **새 판의 첫 레벨 앞에는 걸어 다니는 트럭 로비가 없다.** repo-run.md의 흐름도는 "메인 메뉴/로비 메뉴 → 트럭(Lobby) → 원정 레벨"이라고 적었는데, 이것은 `ChangeLevelType.Normal`일 때의 분기다. 로비 메뉴의 Start 버튼과 싱글 New Game/Load는 `loadLevel`에 따라 호출 종류가 갈린다. 0이면 `RunLevel`(곧바로 첫 원정 레벨), 1이면 `Shop`, 2이면 `Normal`(트럭 로비)이다(`MenuPageLobby.cs:278-289`, `SemiFunc.cs:899-910`). `loadLevel`은 세이브에 저장된 `saveLevel`(상점 1, 트럭 2, 그 외 0)이고(`RunManager.cs:322-334`, `StatsManager.cs:1248`), 새 세이브는 0이다. 따라서 걸어 다니는 트럭 로비는 "상점 다음, 다음 원정 전"에만 나오고, 그 지점에서 저장한 세이브를 불러왔을 때에만 로비 메뉴에서 곧장 간다.
2. **늦게 들어온 사람(추정 → 코드 확인).** Photon 방과 Steam 로비를 "열어 주는" 코드는 로비 메뉴 레벨의 `LobbyMenuOpen` 한 곳뿐이고, Start를 누르는 순간 둘 다 닫힌다. 레벨 중에 다시 여는 코드는 없다(4.3절).
3. **세이브가 지워지는 시점은 아레나가 끝날 때가 아니라 "전멸해서 아레나로 넘어가는 순간"이다.** `ChangeLevel`이 `levelCurrent`를 아레나로 바꾼 직후 `OnSceneSwitch`가 불리고(`RunManager.cs:338`), 그 안의 `SaveDeleteCheck`가 `RunIsArena()`를 현재 레벨 기준으로 평가해 참이 되기 때문이다(`SemiFunc.cs:2992`, `DataDirector.cs:433`). 같은 호출 안에서 `SaveFileSave`가 먼저 실행되지만(`SemiFunc.cs:2988-2991`) 곧바로 폴더가 삭제된다.
4. **`PlayerDeathHead.outsideLevelTimer`의 값은 5초**다(`PlayerDeathHead.cs:752-753`). repo-run.md에는 값이 빠져 있다.
5. **상점이나 트럭 로비에서 전원이 죽어도 게임오버가 아니다.** 전멸 감시는 아레나가 아닌 모든 레벨에서 돌지만(`RunManager.cs:187`), `ChangeLevel`의 아레나 분기는 `levelCurrent`가 트럭 로비나 상점이면 건너뛴다(`RunManager.cs:230`). 그래서 기본 분기로 떨어져 상점이면 트럭으로, 트럭이면 다음 원정 레벨로 간다(`RunManager.cs:293-300`).

## 1. 씬과 레벨의 목록

### 1.1 Unity 씬 (코드가 부르는 이름)

| 씬 | 역할 | 근거 |
|---|---|---|
| `Main` | 모든 "레벨"이 들어가는 단 하나의 껍데기 씬. `LevelGenerator`가 `RunManager.levelCurrent` 에셋을 읽어 런타임에 방·모듈을 조립한다 | `RunManager.cs:405`, `LevelGenerator.cs:258-290` |
| `Reload` | 레벨 전환 사이의 빈 중간 씬. 전환 때마다 `Main` → `Reload` → `Main` 순으로 한 번씩 다시 로드한다 | `RunManager.cs:409,533`, `ReloadScene.cs:34-62` |
| `LobbyJoin` | 클라이언트가 방에 접속할 때 들어가는 별도 씬. 안에 `NetworkConnect`가 있을 것으로 읽히지만 씬 내용은 코드에 없다(추정) | `RunManager.cs:390-395` |

`ReloadScene`은 최소 0.1초 머문 뒤, 싱글이면 바로 `Main`을 로드하고, 멀티면 마스터가 "방 안 전원이 `PlayerReady` RPC를 보냈고 Photon 레벨 로딩이 멈춰 있을 때" `PhotonNetwork.LoadLevel("Main")`을 부른다(`ReloadScene.cs:17,42-62`). 이 지점이 멀티플레이 씬 전환의 동기화 장벽이다.

첫 부팅 씬이 무엇인지는 빌드 설정을 읽을 수 없어 확인하지 못했다. 다만 부팅 직후 `levelCurrent`가 스플래시 레벨이라는 것은 `LevelGenerator.Start`의 `IsSplashScreen()` 분기와 `SplashScreen`이 `GameDirector.Main`을 기다리는 구조(`LevelGenerator.cs:260`, `SplashScreen.cs:66-73`)로 읽힌다.

### 1.2 `Level` 에셋 종류 (RunManager가 들고 있는 것)

| 종류(`RunManager` 필드) | 무엇인가 | 걸어 다닐 수 있나 | 어디로 가고 어디서 오나 |
|---|---|---|---|
| `levelSplashScreen` | 부팅 직후 로고·경고 화면 | 아니오 | → 메인 메뉴(`SplashScreen.cs:109-126`) |
| `levelMainMenu` | 3D 메인 메뉴 방. 메뉴 페이지가 UI로 얹힌다 | 아니오(`PlayerController` 오브젝트가 꺼진다, `PlayerController.cs:378-382`) | 호스트·조인·싱글·튜토리얼로 나간다 |
| `levelLobbyMenu` | 호스트가 파티를 모으는 "로비 메뉴" 방. 메뉴 페이지 `Lobby`가 열린다 | 아니오(위와 같다) | 시작하면 원정 레벨/상점/트럭 중 하나로 간다 |
| `levelLobby` | 상점 다음, 원정 전에 들르는 **걸어 다니는 트럭 로비** | 예 | 상점에서 오고 원정 레벨로 간다(`RunManager.cs:293-300`) |
| `levels[]` | 실제 원정 레벨(테마별). 같은 레벨이 연속으로 나오지 않는 것만 보장 | 예 | `SetRunLevel`(`RunManager.cs:515-522`) |
| `levelShop[]` | 상점. 원정 레벨을 완료할 때마다 하나가 무작위로 낀다 | 예 | `RunManager.cs:291` |
| `levelArena[]` | 전멸 시 가는 "게임오버" 미니게임 레벨. 서바이벌형 `Arena`와 레이스형 `ArenaRace`("Level - Arena Race", `DebugCommandHandler.cs:508`)가 있다 | 예(죽은 사람은 관전) | 멀티면 무작위, 싱글이면 `[0]`(`RunManager.cs:245-252`) |
| `levelTutorial` | 싱글 전용 튜토리얼 레벨 | 예 | 메인 메뉴의 Tutorial 버튼, 끝나면 방을 떠나 메인 메뉴로(`TruckScreenText.cs:322-326`) |
| `levelRecording` | 녹화·촬영용 레벨. 디버그 명령으로만 들어간다 | — | `DebugCommandHandler.cs:586` |

`Level` 에셋 자체의 필드(모듈 수, 시작 방, 난이도별 모듈 목록, 로딩 그래픽 3장, 안개·색보정·블룸 등)는 `Level.cs:5-116`에 있다. 로딩 그래픽 3장과 이름이 레벨 카드 연출에 쓰인다(10.2절). 테마·모듈 구성은 [repo-map.md](repo-map.md)가 다룬다.

### 1.3 `Level` 종류를 가르는 헬퍼

`MenuLevel()`은 메인 메뉴·로비 메뉴·스플래시 셋 중 하나일 때 참이다(`SemiFunc.cs:1756-1763`). 이 한 함수가 "이 레벨에서는 ESC 메뉴를 열 수 없다", "플레이어 오브젝트를 끈다", "음성 믹서를 로비용으로 둔다" 같은 분기를 가른다(`GameDirector.cs:332`, `PlayerController.cs:378`, `PlayerAvatar.cs:526`). `RunIsLevel()`은 상점·트럭·메뉴·스플래시·튜토리얼·아레나가 전부 아닐 때만 참이다(`SemiFunc.cs:2818-2825`).

## 2. 부팅 → 메인 메뉴

```
부팅(스플래시 레벨, Main 씬)
  └ LevelGenerator가 스플래시 레벨 생성 (0.2초 + 0.2초 대기 포함, 9절 표)
  └ GameDirector.Main에 들어가면 SplashScreen 시작
        Wait → Semiwork(제작사 로고) 2초 → Warning(경고 카드) 6초 → Done
  └ Done: 설정 저장(스플래시 버전 기록) → 초대로 접속해야 하면 그쪽으로,
        아니면 스킵 로딩 UI로 아웃트로 → 방 나가기 플래그 → 메인 메뉴
```

- 로고 2초(`SplashScreen.cs:80`), 경고 6초(`SplashScreen.cs:97`)이고 경고 시작 때 경고음이 난다(`SplashScreen.cs:99`).
- **한 번 본 스플래시는 아무 키로 건너뛴다.** 저장된 `SplashScreenCount` 설정이 `RunManager.splashScreenLatestVersion` 이상일 때만 건너뛰기가 허용된다(`SplashScreen.cs:158-164`). 스플래시를 고쳐서 다시 보여주고 싶으면 `splashScreenLatestVersion` 정수를 올리면 된다(`RunManager.cs:29-31`). 첫 실행에는 건너뛸 수 없다.
- `SplashScreenCompleted`는 설정이 로드된 상태에서만 스플래시 횟수를 기록하고 즉시 저장한다. 첫 실행이면 "아이템 해제 자동 홀드" 설정을 0으로 되돌린다(`SemiFunc.cs:3857-3873`).
- **초대로 부팅한 경우**: 명령줄 `+connect_lobby <id>`를 `SteamManager.Start`가 읽어 `lobbyIdToAutoJoin`에 담아 두고(`SteamManager.cs:92-99`), 스플래시가 끝나면 메인 메뉴를 거치지 않고 곧바로 `JoinSteamLobby`를 부른다(`SplashScreen.cs:116-120`). 스플래시 중에 Steam 초대를 수락한 경우에도 같은 변수에 보관했다가 스플래시 종료 시 처리한다(`SteamManager.cs:156-166`).
- 메인 메뉴로 가는 길은 "방 나가기" 공용 경로(`NetworkManager.leavePhotonRoom` → `LeavePhotonRoom` → `LeaveToMainMenu`)를 그대로 쓴다(`SplashScreen.cs:121-124`, `NetworkManager.cs:144-156`, `RunManager.cs:524-535`). 스플래시용 별도 전환 코드가 없다.
- Steam이 초기화되지 않았으면 레벨 생성이 끝난 시점에 `Application.Quit()`로 게임이 꺼진다(`RunManager.cs:153-157`).
- 디버그 전용 자동 건너뛰기(`SplashScreenSkip`)는 에디터/디버그 설정일 때만 켜진다(`SemiFunc.cs:3875-3910`).

## 3. 메인 메뉴의 UI 목록

### 3.1 메뉴 시스템의 뼈대

`MenuManager`가 모든 페이지의 열기·닫기·덮기를 맡는다. 페이지 enum은 22개다(`MenuPageIndex.cs`): Main, Escape, Settings, SettingsGameplay, SettingsGraphics, SettingsAudio, SettingsControls, SettingsLanguage, PopUp, PopUpTwoOptions, Lobby, Color, Cosmetics, Saves, SavesRename, Password, Regions, ServerList, ServerListCreateNew, ServerListSearch, PublicGameChoice, Moons.

페이지를 여는 방식은 네 가지다(`MenuManager.cs:373-447`).

| 방식 | 동작 | 쓰는 곳 |
|---|---|---|
| `PageOpen` | 현재 페이지로 교체 | 대부분의 이동 |
| `PageOpenOnTop` | 아래 페이지를 비활성(Inactive)으로 두고 위에 덮음. 닫으면 아래 페이지가 다시 활성화 | 설정, 꾸미기, 서버 만들기·검색 |
| `PageAddOnTop` | 같은 부모 아래 하위 탭처럼 덧붙임 | 설정의 Gameplay/Graphics/Audio/Controls/Language 탭 |
| `PagePopUp`/`PagePopUpTwoOptions` | 모달 팝업. 확인 한 개짜리와 두 선택지짜리 | 오류, 확인 질문 |

- 페이지 하나는 `Opening → Active → Closing`(+ `Inactive`, `Activating`) 상태를 가진다. 열릴 때 위에서 내려오고 닫힐 때 아래로 떨어지며, 둘 다 `Lerp(…, 40 × deltaTime)`로 움직인다(`MenuPage.cs:156-260`). 열고 닫을 때마다 페이지 인트로/아웃트로 효과음이 난다(`MenuPage.cs:167-176,235-244`).
- 거의 모든 페이지는 `Back` 입력(ESC)으로 한 단계 되돌아간다. 각 페이지의 `Update`에 같은 한 줄이 반복되어 있다(예: `MenuPageSaves.cs:185`, `MenuPageRegions.cs:216`, `MenuPageServerList.cs:169`, `MenuPageSettings.cs:17`). 두 선택지 팝업에서 Back은 "두 번째 선택지"(대개 취소)로 처리된다(`MenuPageTwoOptions.cs:32-35`).
- 메뉴가 열려 있는 동안 `MenuManager.StateOpen`이 커서를 풀고(`CursorUnlock(0.1)`), `PlayerController.InputDisableTimer = 0.1`로 게임 입력을 막으며, 페이지가 HUD 요소(체력·에너지·인벤토리·목표 등)를 매 프레임 숨긴다(`MenuManager.cs:327-336`, `MenuPage.cs:309-324`).
- 입력은 마우스·키보드 중심이다. 메뉴는 가상 커서(`MenuCursor`)와, 호버한 버튼으로 `Lerp(…, 20 × deltaTime)`로 미끄러져 가며 박동하는 "선택 상자"(`MenuSelectionBox`)로 조작한다(`MenuSelectionBox.cs:103-111`). 게임패드 전용 코드는 이 파일들에서 보이지 않는다.
- 클릭 피드백은 5종이다. Action(노랑 플래시), Confirm(초록), Deny(빨강), Dud(회색), Tick(파랑)이며 각각 효과음이 따로 있다(`MenuManager.cs:22-29,239-300`).

### 3.2 메인 메뉴 페이지의 버튼

`MenuPageMain`에는 버튼 핸들러가 8개 있다. 화면에 찍히는 문구는 프리팹이라 확인하지 못했고, 아래 이름은 핸들러 이름에서 따온 것이다.

| 핸들러 | 하는 일 | 근거 |
|---|---|---|
| `ButtonEventSinglePlayer` | 메인 메뉴 상태를 싱글로 두고 모든 페이지를 닫은 뒤 `Saves` 페이지를 연다 | `MenuPageMain.cs:191-196` |
| `ButtonEventHostGame` | `Regions` 페이지를 `HostGame` 유형으로 연다 | `MenuPageMain.cs:206-210` |
| `ButtonEventJoinGame` | Steam 오버레이의 친구 창을 연다(게임 안에는 접속 코드 입력이나 목록이 없다) | `MenuPageMain.cs:212-215`, `SteamManager.cs:381-384` |
| `ButtonEventPlayRandom` | `Regions` 페이지를 `PlayRandom` 유형으로 연다 | `MenuPageMain.cs:217-221` |
| `ButtonEventTutorial` | 튜토리얼 재생 기록을 남기고 진행도를 초기화한 뒤 튜토리얼 레벨로 간다 | `MenuPageMain.cs:198-204` |
| `ButtonEventSettings` | `Settings`를 위에 덮어 연다 | `MenuPageMain.cs:234-237` |
| `ButtonEventChangeColor` | 꾸미기(`Cosmetics`)를 위에 덮어 연다. 꾸미기 세이브가 준비된 뒤에만 열린다 | `MenuPageMain.cs:239-245` |
| `ButtonEventQuit` | 로딩 UI를 생략하고 모든 아바타에 종료 플래그를 단 뒤 아웃트로 → `Application.Quit()` | `MenuPageMain.cs:223-232`, `PlayerAvatar.cs:1467-1472` |

메인 메뉴의 세부 연출은 다음과 같다.

- **첫 진입 인트로.** 레벨 생성이 끝나면 3초를 기다린 뒤, 페이지가 화면 왼쪽(x = -600)에서 `Lerp(…, 2 × deltaTime)`로 미끄러져 들어와 50픽셀 안으로 오면 활성화된다(`MenuPageMain.cs:149-169`). 이후 메뉴로 돌아올 때는 이 인트로를 건너뛴다(`MenuPageMain.cs:45-55`).
- **튜토리얼 버튼 깜빡임.** `TutorialPlayed` 설정이 0이면 버튼이 주황과 흰색으로 0.5초마다 번갈아 깜빡인다(`MenuPageMain.cs:56-62,73-92`). 한 번 해 보면 멈춘다.
- **꾸미기 버튼 강조.** 새로 얻고 아직 보지 않은 꾸미기가 있으면 버튼이 주황색이 되고 개수가 표시된다. 없으면 회색이다(`MenuPageMain.cs:261-283`).
- **예약 팝업.** 이전 세션에서 쌓인 오류 팝업(`PagePopUpScheduled`)은 메인 메뉴 생성 완료 후 1.5초 뒤에 뜬다(`MenuPageMain.cs:20,141-147`). 사용자가 새 화면에 "도착한 다음" 읽게 하려는 배치다.
- **자동 조인.** Steam 초대로 들어온 경우 `joinLobby` 플래그가 서고, 0.1초 뒤 진행도를 초기화하고 `lobbyJoin`으로 로비 메뉴 접속 경로를 탄다(`MenuPageMain.cs:175-188`).

### 3.3 호스트·플레이 랜덤의 페이지 이동도

```
메인
 ├ Single Player ─→ Saves ─┬ New Game ─→ MenuActionSingleplayerGame(새 세이브)
 │                         └ Load      ─→ MenuActionSingleplayerGame(세이브)
 ├ Host Game ──→ Regions ──→ Saves ─┬ New Game ─→ MenuActionHostGame(새 세이브)
 │                                  └ Load      ─→ MenuActionHostGame(세이브)
 │                                      └ (로딩 중) Password(2초 뒤, 건너뛰기 가능) → 로비 메뉴
 ├ Join Game ──→ Steam 친구 오버레이 (게임 안 UI 없음)
 ├ Play Random ─→ Regions ──→ PublicGameChoice ─┬ Random Matchmaking ─→ 로비 메뉴
 │                                              └ Server List ─┬ 서버 선택 → 확인 팝업 → 로비 메뉴
 │                                                             ├ Create New → 이름 입력 → 로비 메뉴
 │                                                             └ Search
 ├ Tutorial ───→ 튜토리얼 레벨
 ├ Settings ───→ Gameplay / Graphics / Audio / Controls / Language
 ├ Customize ──→ Cosmetics
 └ Quit
```

(각 이동의 코드 근거는 `MenuPageMain.cs`, `MenuPageRegions.cs:228-242`, `MenuPageSaves.cs:195-257`, `MenuPagePublicGameChoice.cs:13-28`, `MenuPageServerList.cs:361-377`.)

### 3.4 페이지별 상세

**Regions(서버 지역 선택).** 페이지가 뜨면 Photon 마스터 서버에 접속해 활성 지역 목록을 받아 온다. 연결이 끝나면 0.3초 쉬었다가 "최적 지역 자동 선택" 항목을 맨 위에 넣고, 이어서 지역 항목을 **0.15초 간격**으로 한 줄씩 띄운다(`MenuPageRegions.cs:71-207`). 줄이 하나 뜰 때마다 효과음의 음높이가 0.1씩 올라간다(`MenuPageRegions.cs:192-196`). 핑은 50 미만 초록, 100 미만 노랑, 그 이상 주황, 999 초과나 실패는 빨강 `>999`로 표시한다(`MenuPageRegions.cs:165-185`). 목록이 다 뜨면 연결을 끊고 스크롤 영역이 알파 0→1로 페이드인한다(`MenuPageRegions.cs:199-206`). 고른 지역은 `DataDirector.networkRegion`에 저장된다. 호스트 경로는 `Saves`로, 랜덤 경로는 `PublicGameChoice`로 간다. 이 페이지에는 로딩 그래픽(`MenuLoadingGraphics`)이 붙어 있고 `SetDone`으로 마무리된다(`MenuPageRegions.cs:88`).

**Saves(세이브 슬롯).** 싱글과 멀티가 같은 페이지를 쓰고, 맨 위 헤더 문구만 "싱글플레이 모드/멀티플레이 모드"로 바뀐다(`MenuPageSaves.cs:122-132`).
- 슬롯 최대 **10개**. 10개가 차면 새 게임 대신 "세이브 한도 도달" 팝업이 뜬다(`MenuPageSaves.cs:12,195-210`).
- 슬롯을 클릭하면 오른쪽 정보 패널이 그 슬롯의 내용으로 바뀐다. 팀 이름, 저장 날짜, 트럭 아이콘과 함께 현재 레벨(난이도 색으로 칠해진 숫자), 플레이 시간, 보유 화폐(K 단위), 총 수확금, 달 단계 아이콘(있을 때), 함께한 플레이어 이름 목록이 나온다. 혼자 한 세이브는 "혼자서 다 해냈다"는 문구가 나온다(`MenuPageSaves.cs:319-398`).
- 팀 이름을 바꾼 적이 없으면 이름 자리에 "클릭해서 이름 바꾸기" 안내가 나오고, 클릭하면 `SavesRename` 페이지(텍스트 입력, 빈 문자열이면 Deny 효과음)가 위에 덮인다(`MenuPageSaves.cs:360-369,400-408`, `MenuPageSavesRename.cs:32-58`).
- 패널 색이 상태를 말한다. 선택 전에는 붉은 기운, 선택하면 푸른 기운으로 `Lerp`한다(`MenuPageSaves.cs:189-192,295,326`).
- 손상된 세이브는 슬롯에 "손상됨" 문구가 붉게 뜨고, 백업이 있으면 "복원" 버튼이 대신 나오며 백업 파일을 정본으로 되돌린다. 백업도 없으면 불러오기 버튼이 사라지고 삭제만 가능하다(`MenuPageSaves.cs:221-245,330-353`, `MenuElementSaveFile.cs:66-72`).
- 삭제하면 아래 슬롯들이 한 칸씩 위로 올라오며 흔들리는 애니메이션이 붙는다(`MenuPageSaves.cs:259-286`).
- Ctrl+Shift를 누른 채 슬롯을 클릭하면 세이브 폴더가 탐색기로 열린다(개발자용, `MenuElementSaveFile.cs:52-62`).
- 불러오기/새 게임을 누르면 모드(싱글/멀티)에 따라 `MenuActionSingleplayerGame` 또는 `MenuActionHostGame`이 호출된다(`MenuPageSaves.cs:195-257`).

**PublicGameChoice.** 단순한 갈림길로, "랜덤 매치메이킹"과 "서버 목록" 두 버튼이 있다(`MenuPagePublicGameChoice.cs`).

**ServerList.** Photon 마스터 서버에서 `custom` 로비의 방 목록을 받아 온다. 보여 주는 방은 열려 있고(`IsOpen`) 정원이 차지 않은 방뿐이며, 순서는 매번 무작위로 섞는다(`MenuPageServerList.cs:111-135`). 한 페이지에 **8개**, 줄마다 서버 이름과 `현재/최대` 인원을 표시한다(`MenuPageServerList.cs:33,181-192`). 방이 한 줄 뜰 때마다 0.1초 간격으로 오르는 음높이의 효과음이 난다(`MenuPageServerList.cs:94-108`). 이전/다음 페이지 화살표 버튼이 있고 페이지를 넘길 때는 0.05초 간격으로 방향(좌/우)에 맞춰 들어온다(`MenuPageServerList.cs:194-229`). **검색**은 공백으로 나눈 단어가 모두 서버 이름에 들어 있어야(대소문자 무시) 통과하는 AND 검색이고, 검색 중에는 페이지 버튼이 숨는다(`MenuPageServerList.cs:231-335`). **방 만들기**는 서버 이름을 입력받는 페이지를 위에 덮고, 이름이 비어 있으면 Deny 효과음으로 거부한다(`MenuPageServerListCreateNew.cs:31-55`). 서버를 클릭하면 "정말 이 서버에 들어가겠느냐"는 두 선택지 팝업이 뜬다(`MenuElementServer.cs:54-58`, `MenuPageServerList.cs:190`).

**Password(비밀번호 입력).** 4.1절에서 다룬다. 최대 10자, 입력은 자동으로 대문자·공백 제거, 초과하면 빨간 플래시와 흔들림과 Deny 효과음, 글자마다 음높이 높은 Tick 효과음, 보기/숨기기 토글, 복사 버튼, 비어 있으면 버튼이 "건너뛰기", 한 글자라도 있으면 "확인"으로 애니메이션과 함께 바뀐다(`MenuPagePassword.cs:62-137,171-204`).

**Settings.** 허브 페이지이고 하위 탭은 Gameplay, Graphics, Audio, Controls, Language다(`MenuPageSettings.cs:23-70`). Back의 목적지가 현재 레벨에 따라 다르다. 메인 메뉴에서는 메인, 로비 메뉴에서는 로비 페이지, 게임 중에는 ESC 메뉴로 돌아간다(`MenuPageSettings.cs:35-52`). 설정 항목과 기본값은 아래 표다(`DataDirector.cs:168-206`).

| 탭 | 항목 (기본값) |
|---|---|
| Audio | 마스터 75, 음악 75, 효과음 75, 근접 음성 75, 텍스트 음성(TTS) 75, 마이크 장치, 마이크 음량 100, 푸시투토크 0 |
| Graphics | 해상도, 전체화면, 조명 거리 3, 수직동기 0, 블룸 1, 색수차 1, 그레인 1, 모션 블러 1, 렌즈 효과 1, 글리치 반복 1, 최대 FPS 144, 그림자 품질 2, 그림자 거리 3, 창 모드, 픽셀화(렌더 크기) 2, 감마 40 |
| Gameplay | 팁 1, 조준 감도, 카메라 부드러움, 카메라 흔들림 100, 카메라 노이즈 100, 카메라 애니메이션 4, 플레이어 이름 표시 1, 수직 반전 0, 광과민성 0, 거미 공포증 0, 리치 프레전스 2, 아이템 해제 자동 홀드 1, 꾸미기 1 |
| Controls / Language | 키 바인딩, 언어 |

접근성 항목(광과민성, 거미 공포증, 카메라 흔들림·노이즈 슬라이더)이 기본 설정 탭에서 바로 제공된다는 점이 눈에 띈다. 광과민성이 켜져 있으면 머리 강제 관전 중의 무작위 글리치·충격이 생략되는 식으로 연출 코드가 이 설정을 확인한다(`SpectateCamera.cs:503`).

### 3.5 로딩·연결·실패 팝업

한 가지 규칙으로 모든 오류가 처리된다. **오류는 팝업 한 장(제목 빨강, 본문 "원인: …", 확인 버튼 하나)으로 보이고, 팝업은 "예약"되었다가 메인 메뉴에 도착한 뒤에 뜬다**(`MenuManager.cs:577-598`). 네트워크 오류 팝업의 확인 버튼 영어 기본 문구는 한 종류(조금 장난스러운 "Ok Dang")다(`NetworkConnect.cs:123,314`, `NetworkManager.cs:178`). 세이브 한도 팝업만 평범한 "OK"다(`MenuPageSaves.cs:207`). 상황별 팝업은 다음과 같다.

| 상황 | 제목/내용(뜻) | 후속 동작 | 근거 |
|---|---|---|---|
| 방 만들기·입장·랜덤 입장 실패 | "연결 끊김 / 원인: Photon 오류 문구". 방 최대 인원 값이 서버 한도를 넘었다는 오류면 서버가 알려 준 값으로 `maxPlayers`를 자동 조정하고 그 사실을 주황색으로 덧붙인다 | Photon·Steam 로비를 나가고 싱글 모드로 돌려 메인 메뉴로 | `NetworkConnect.cs:298-376` |
| 차단당한 방 입장 | "이 로비에서 차단되었다" | 같음 | `NetworkConnect.cs:325-328` |
| Steam 로비의 빌드 이름이 다름 | "잘못된 게임 버전 / 로비는 X 버전" | 로비 이탈, `Reload`로 메인 메뉴 | `NetworkConnect.cs:109-140` |
| Steam 로비의 빌드 이름이 비어 있음(호스트가 이미 떠남) | "로비가 닫혔다" | 위와 같음 | `NetworkConnect.cs:126-135`, `SteamManager.cs:268-277` |
| 레벨 중 연결 끊김(서버/클라 의도 아님) | "연결 끊김 / 원인: …" | 아웃트로 후 방 나가기, 메인 메뉴에서 팝업 | `NetworkManager.cs:201-228` |
| 호스트가 나감 | "연결 끊김 / 원인: 호스트 연결 끊김" | 강제로 방 나가기 | `NetworkManager.cs:172-181` |
| 로딩 25초 초과 후 ESC | "연결 끊김 / 원인: 로딩에서 멈춤" | 강제로 방 나가기 | `NetworkManager.cs:183-191` |
| 강퇴·밴 이벤트(코드 199) | "추방됨 / 호스트에게 추방되었다" | 강제로 방 나가기 | `NetworkManager.cs:230-243` |
| 세이브 슬롯 10개 초과 | "세이브 한도 도달"(한도 숫자 포함) | 팝업 닫으면 제자리 | `MenuPageSaves.cs:195-210` |
| 서버 선택 | "정말 이 서버에 들어가겠느냐" 두 선택지 | 확인하면 접속 | `MenuPageServerList.cs:190` |
| 강퇴 확인 | "정말 {이름}을 추방하겠느냐" 두 선택지 | 확인하면 밴 이벤트 | `MenuButtonKick.cs:43` |

"연결 중"이라는 별도의 연결 팝업은 없다. 호스트가 New Game을 누른 뒤에는 **로딩 화면(레벨 전환용 LoadingUI)이 그대로 연결 대기 화면 역할**을 한다. `waitToChangeScene`이 켜진 동안 `RestartScene`이 씬 교체를 미루고(`RunManager.cs:396-411`), `NetworkConnect.OnJoinedRoom`이 이 플래그를 꺼야 비로소 씬이 넘어간다(`NetworkConnect.cs:275`). 지역 목록·서버 목록 페이지에는 로딩 그래픽이 붙는다.

## 4. 호스트와 조인에서 정확히 일어나는 일

### 4.1 호스트 (친구 전용, 비공개)

1. `Host Game` → `Regions`에서 지역 선택 → `Saves`에서 새 세이브 또는 불러오기. 이때 `MenuActionHostGame`이 호출되어 진행도를 초기화하고, 매치메이킹 플래그를 끄고, 로비 종류를 `Private`으로 두고, 세이브를 만들거나 읽는다(`SemiFunc.cs:913-930`).
2. 같은 함수가 `waitToChangeScene = true`로 두고 로비 메뉴 레벨로 `ChangeLevel`을 요청하며 `NetworkConnect` 프리팹을 인스턴스화한다. 아웃트로가 돌고 로딩 화면이 뜨지만 씬은 교체되지 않는다.
3. `NetworkConnect`가 Photon 지역·버전·앱 ID를 설정하고 음성 클라이언트를 만든 뒤 Steam 로비를 만든다. 로비는 비공개·친구 전용·**입장 불가(`SetJoinable(false)`)** 상태로 시작한다(`SteamManager.cs:245-266`). Photon 방 이름은 **Steam 로비 ID 문자열**이다(`NetworkConnect.cs:147-152`).
4. **비밀번호 입력.** 호스트는 아직 Steam 로비가 없으므로 비밀번호 확인 분기에 그대로 들어가고, 2초를 기다린 뒤 비밀번호 페이지를 **로딩 화면 위에** 띄운다. 오디오는 "컷신 전용" 스냅샷으로 바뀌고, 페이지와 가상 커서가 한 단계 위 부모로 옮겨져 로딩 UI 위에 그려진다(`NetworkConnect.cs:75-102`). 비워 두고 "건너뛰기"를 누르면 비밀번호 없이 방이 만들어진다. 이 동안 25초 "로딩에서 멈춤" 타이머는 멈춘다(`NetworkManager.cs:122`). 비밀번호는 Photon 방 속성 `PASSWORD`와 접속자의 플레이어 속성에 실리고, 비어 있지 않으면 Steam 로비 데이터 `HasPassword = 1`로 기록된다(`NetworkConnect.cs:245-263`, `SteamManager.cs:324-338`). **비밀번호를 서버가 어떻게 대조하는지는 코드에 없다**(Photon 서버 설정 쪽, 미확인).
5. Photon 마스터 서버에 접속(Steam 인증 티켓 포함)하고 `JoinOrCreateRoom(로비 ID, 최대 인원, 비공개 로비 타입)`을 호출한다. 방은 목록에 보이지 않는다(`IsVisible = false`, `NetworkConnect.cs:251-262`). 방이 만들어져 입장되면 `AutomaticallySyncScene`을 켜고 `waitToChangeScene`을 끈다(`NetworkConnect.cs:270-296`).
6. 비로소 씬 교체가 일어난다. 마스터가 `PhotonNetwork.LoadLevel("Reload")`를 부르고 `Reload`가 `Main`으로 넘긴다. `LevelGenerator`가 로비 메뉴 레벨을 만들고, 호스트 자신의 아바타가 메뉴용 스폰 지점에 생성된다.
7. 로비 메뉴 레벨이 시작되고 **2초 뒤** `LobbyMenuOpen`이 카메라를 가볍게 흔들며(`Shake(0.25, 0.05)` 두 번) 로비 페이지를 연다. 같은 순간 호스트가 Photon 방을 `IsOpen = true`로 열고 Steam 로비를 **친구 전용 + 입장 가능**으로 푼다(`LobbyMenuOpen.cs:8-43`, `SteamManager.cs:292-307`).

호스트가 선택하는 "공개/비공개"는 UI 스위치가 아니라 **진입 경로 자체로 정해진다**. 호스트 게임은 친구 전용 Steam 로비 + 보이지 않는 Photon 방이고, 서버 목록에서 방을 만들거나 랜덤 매치메이킹을 쓰면 공개 방이다. 비밀번호는 비공개 호스트 게임에만 있다.

### 4.2 조인

조인 경로는 세 가지다.

**(a) Steam 초대·친구 "게임 참가".** 메인 메뉴의 Join Game은 단지 Steam 친구 오버레이를 연다. 로비 메뉴의 Invite 버튼은 Steam 게임 초대 오버레이를 연다(오버레이가 꺼져 있으면 친구 창, `SteamManager.cs:386-396`). 사용자가 초대를 수락하거나 친구의 "참가"를 누르면 `OnGameLobbyJoinRequested`가 호출된다(`SteamManager.cs:156-166`).
- 스플래시 중이면 ID만 저장했다가 스플래시 종료 후 처리한다.
- 그 외에는 **확인 질문 없이 즉시** `JoinSteamLobby`가 실행된다. 이미 진행 중인 방이 있으면 먼저 Photon 연결과 Steam 로비를 떠나 싱글 모드로 돌리고, 튜토리얼이면 튜토리얼을 끝내고, 현재 화면에 아웃트로를 건 뒤, Steam 로비 입장을 기다린다(`SteamManager.cs:181-213`). 게임 도중에도 초대를 받으면 곧바로 끌려 나간다는 뜻이다.
- 입장이 끝나면 현재 화면이 메인 메뉴가 아니면 `lobbyJoin = true`와 함께 로비 메뉴로 `ChangeLevel`을 요청하고, 메인 메뉴이면 `joinLobby` 플래그를 세워 `MenuPageMain`이 처리한다(`SteamManager.cs:201-212`, `MenuPageMain.cs:175-188`).
- 두 경우 모두 `RestartScene`이 `lobbyJoin`을 보고 `LobbyJoin` 씬을 로드한다(`RunManager.cs:390-395`).
- `NetworkConnect`가 Steam 로비 데이터를 읽는다. `HasPassword`가 1이면 비밀번호 페이지(2초 대기 후)를 띄운다. 방 이름과 지역은 로비 데이터에서 가져오고, 빌드 이름이 내 빌드와 다르면 버전 불일치 팝업으로 돌아간다(`NetworkConnect.cs:77-155`). 통과하면 같은 방에 `JoinOrCreateRoom` 한다(`NetworkConnect.cs:245-263`).
- 입장 후 호스트가 아니면 `saveFileCurrent`를 비운다. 클라이언트는 세이브 파일을 갖지 않는다(`NetworkConnect.cs:288-291`).

**(b) 랜덤 매치메이킹.** `PublicGameChoice → Random Matchmaking`이 `MenuActionRandomMatchmaking`을 호출해 `connectRandom`을 켜고 로비 종류를 `Matchmaking`으로 둔 뒤 `lobbyJoin`으로 로비 메뉴 접속 경로를 탄다(`SemiFunc.cs:932-948`). `NetworkConnect`가 고른 지역으로 접속해 `JoinRandomOrCreateRoom(최대 인원 6, FillRoom 모드)`을 호출한다(`NetworkConnect.cs:201-224`). 방이 없으면 내가 방을 만든 호스트가 되고, 그때는 곧바로 `Main` 씬을 로드한다(`NetworkConnect.cs:276-287`).

**(c) 서버 목록.** 선택한 방 이름을 `networkJoinServerName`에 담아 `connectRandom` 경로로 `JoinRoom`한다(`MenuElementServer.cs:95-106`, `NetworkConnect.cs:195-199`). 방 만들기는 `networkServerName`을 담아 `CreateRoom`(`IsVisible = true`, 방 속성 `server_name`)한다(`NetworkConnect.cs:178-193`).

공개 방(서버 목록·랜덤)에서도 Steam 로비는 만들어진다. 로비 메뉴 레벨이 시작되면 마스터가 Steam 로비를 공개로 풀고, 나머지 인원에게 `JoinSteamLobbyRPC`(버퍼드)를 보내 같은 Steam 로비에 들어오게 한다(`RunManagerPUN.cs:12-63`). 친구 그룹 표시와 초대용이다.

### 4.3 언제 들어올 수 있나 (조인 가능 상태)

| 구간 | Photon 방 | Steam 로비 | 근거 |
|---|---|---|---|
| 호스트가 연결 중 (로딩 화면) | `IsOpen`을 명시하는 코드가 없어 Photon 기본값(열림)이지만, 방이 목록에 보이지 않고(`IsVisible = false`) 이름이 Steam ID라 Steam 로비가 막혀 있는 동안은 들어올 경로가 없다 | **입장 불가** | `SteamManager.cs:256-264`, `NetworkConnect.cs:251-262` |
| 로비 메뉴 열림(레벨 시작 +2초) | `IsOpen = true`(공개 방이면 `IsVisible = true`도) | 친구 전용 + 입장 가능(공개 방이면 공개) | `LobbyMenuOpen.cs:31-42`, `RunManagerPUN.cs:33-37` |
| 호스트가 Start를 누른 뒤 | `IsOpen = false`, `IsVisible = false` | **입장 불가**로 잠금 | `MenuPageLobby.cs:271-276`, `SteamManager.cs:309-316` |
| 상점·트럭·원정·아레나 중 | 닫힘 유지 (다시 여는 코드 없음) | 잠금 유지 | 위 두 줄 + `IsOpen`을 true로 쓰는 곳이 `LobbyMenuOpen` 한 곳뿐 |
| 아레나가 끝나 로비 메뉴로 돌아옴 | 다시 `IsOpen = true` | 다시 입장 가능 | `LobbyMenuOpen.cs`가 로비 메뉴 레벨이 열릴 때마다 실행되기 때문 |

- **난입은 없다.** 진행 중인 판에 중간 참가하는 경로가 없다. 한 번 나간 사람이 판 도중에 돌아오는 경로도 없다.
- **로비 대기 중 새로 들어오는 사람**은 `OnPlayerEnteredRoom`(Photon)과 `OnLobbyMemberJoined`(Steam)이 로비 페이지의 "접속 중" 알림을 켠다(`NetworkManager.cs:158-165`, `SteamManager.cs:101-112`). 알림이 켜져 있는 동안(이름당 최대 10초, 아바타가 나타난 뒤 1초 더) 호스트의 **Start 버튼이 비활성**이 되고, 눌러도 Deny 효과음만 난다(`MenuPageLobby.cs:77-101,229-236,266-270`). 새 사람이 아직 로딩 중인데 게임이 시작되어 버리는 일을 막는 장치다.
- 새로 들어온 클라이언트는 Photon의 `AutomaticallySyncScene`으로 마스터의 현재 씬을 따라 로드하고, 마스터가 보낸 `UpdateLevelRPC`(버퍼드)로 현재 레벨 이름과 진행도를 받는다(`RunManagerPUN.cs:65-72`, `RunManager.cs:319,414-473`). 레벨 생성은 "방 안 전원의 아바타가 만들어졌다"는 마스터의 버퍼드 RPC를 기다리기 때문에(`NetworkManager.cs:115-121`, `LevelGenerator.cs:283-287`), 로비에 늦게 들어온 사람도 같은 방식으로 합류한다(추정).
- **최대 인원.** 코드 기본값은 6이고(`GameManager.cs:40,44`) Photon 상한은 20이다(`GameManager.cs:42`). Photon 방, Steam 로비, 매치메이킹 모두 이 값을 쓴다(`NetworkConnect.cs:183,203,254`, `SteamManager.cs:248`). `maxPlayers`를 바꾸는 코드는 디버그 설정 한 곳뿐이므로(`DebugComputerCheck.cs:201`) 출시 빌드에는 인원 선택 UI가 없다. 이 필드는 인스펙터 공개 필드라 프리팹 값이 다를 수 있다(프리팹 값, 미확인). 랜덤 매치메이킹은 기대 인원 6인 방만 채운다(`NetworkConnect.cs:208`).
- 정원이 찬 방에 들어가려 하면 Photon의 실패 사유가 3.5절의 "연결 끊김" 팝업으로 나온다.

### 4.4 강퇴·차단

호스트에게만 로비의 플레이어 줄에 강퇴 버튼이 보인다(자기 자신 제외, `MenuButtonKick.cs:33-51`). 눌러서 확인하면 `BanPlayer`가 사용자 지정 Photon 이벤트 124를 모두에게 보낸다. 강퇴당한 쪽은 이벤트 199를 받아 3.5절의 "추방됨" 팝업과 함께 방을 나간다(`NetworkManager.cs:230-278`). 강퇴 이벤트의 서버 측 처리는 코드에 없다(Photon 서버 쪽, 미확인). 차단된 사용자가 다시 들어오려 하면 "차단됨" 문구가 나온다.

## 5. 로비 — 로비 "메뉴"와 걸어 다니는 "트럭 로비"는 별개다

### 5.1 로비 메뉴 (`levelLobbyMenu` + 메뉴 페이지 `Lobby`)

**걸어 다닐 수 없다.** 3D 방 안에 메뉴 UI가 얹힌 화면이다. 플레이어 컨트롤러 오브젝트는 꺼져 있고(`PlayerController.cs:378-382`), 로비 메뉴에서는 체력 초기 로드와 넘어짐 설정을 시작하지 않으며(`PlayerHealth.cs:126`, `PlayerTumble.cs:104-110`), 레벨 생성기가 메뉴 레벨에서는 사망 머리·넘어짐 오브젝트를 만들지 않는다(`LevelGenerator.cs:472-475`). 오디오는 로비 메뉴에서 "관전" 스냅샷으로 전환되고 음성 믹서도 로비용 상태로 남아(`GameDirector.cs:194-197`, `PlayerAvatar.cs:526-529`), 파티원 모두가 서로의 목소리를 거리와 무관하게 듣는 쪽으로 읽힌다(스냅샷·믹서 이름에서 읽은 것으로 실제 믹서 설정은 미확인).

로비 페이지의 구성(`MenuPageLobby.cs`):

- **플레이어 목록.** 접속한 사람마다 한 줄이 생긴다(1초마다 갱신, `MenuPageLobby.cs:103-152`). 줄에는 얼굴 아이콘(`MenuPlayerHead`), 이름, 핑이 있다. 핑은 50 미만 초록, 100 미만 노랑, 200 미만 주황, 그 이상 빨강이다(`MenuPageLobby.cs:202-227`). 줄의 순서는 포톤 뷰 ID 순이고 사람이 나가면 줄이 사라지며 위쪽으로 정리된다(`MenuPageLobby.cs:127-135,183-200`). 줄 위치는 `-listSpot × 32`다(`MenuPlayerListed.cs:147`).
- 이름 색이 상태를 말한다. 말하는 중이면 밝은 노랑 계열로, 가만히 있으면 어두운 회색으로 `Lerp`한다(`MenuPlayerListed.cs:120-137`). 얼굴 아이콘도 말할 때 입이 움직이고, 음소거 상태면 음소거 아이콘이 튀어나온다(`MenuPlayerHead.cs:138-187`).
- 얼굴 아이콘의 눈동자는 다른 사람의 얼굴이나 마우스 커서를 따라간다(`MenuPlayerHead.cs:155-158,255-295`).
- 이름 줄 옆에 호버하면 **Steam 프로필 버튼**이 팝업처럼 나타나 클릭 시 Steam 오버레이로 그 사람의 프로필을 연다(`MenuPlayerListedSteam.cs:36-56`, `SteamManager.cs:403-421`).
- **다른 사람마다 마이크 음량 슬라이더**가 붙는다(자기 줄에는 없다, `MenuPageLobby.cs:162-166`).
- 호스트에게만 **강퇴 버튼**이 보인다.
- 아레나 우승자의 **왕관**이 이름 옆에 붙는다(`MenuPlayerListed.cs:82-90`). 왕관 소유자는 세션을 떠날 때(`SemiFunc.cs:2997-3000`)나 다음 아레나가 시작될 때(`Arena.cs:160`) 초기화되므로, 우승한 판이 끝나 로비 메뉴로 돌아온 뒤에도 유지된다.
- 상단에 방 이름이 `지역 코드 + 방 이름`으로 찍힌다(`MenuPageLobby.cs:55`).
- 하단에 "[채팅 키]를 눌러 채팅" 안내가 있고, 채팅 입력은 로비에서도 쓴다(`MenuPageLobby.cs:246-250`). 글자를 보내면 **TTS 음성으로 읽히고**(11.2절) 말하는 단어마다 줄 옆의 텍스트가 노랗게 번쩍이며 위아래로 튄다(`LobbyChatUI.cs:76-82`).
- 버튼은 다음과 같다.

| 버튼 | 누가 보나 | 동작 | 근거 |
|---|---|---|---|
| Start | **호스트만** (클라는 숨김) | 접속 중인 사람이 있으면 거부(Deny). 아니면 Photon 방을 닫고(`IsOpen = false`, `IsVisible = false`) Steam 로비를 잠그고, 누적 플레이 횟수를 올리고, `loadLevel`에 따라 레벨을 고른다 | `MenuPageLobby.cs:59-70,264-290` |
| Invite | 멀티일 때 모두(싱글이면 숨김) | Steam 초대 오버레이 | `MenuPageLobby.cs:61-64,292-295` |
| Leave | 모두 | 아웃트로 → 방 나가기 → 메인 메뉴 | `MenuPageLobby.cs:252-257` |
| Settings | 모두 | 설정을 위에 덮음 | `MenuPageLobby.cs:259-262` |
| Customize | 모두 | 꾸미기를 위에 덮음. 새 꾸미기 개수가 주황 배지로 강조 | `MenuPageLobby.cs:238-244,297-319` |

- 이 페이지에서는 **A/D(좌우 이동) 입력으로 메뉴 속 아바타를 돌릴 수 있다**. 메뉴 전용 3D 아바타는 화면 밖 `z = -2000`에 따로 놓이고 전용 카메라로 찍히며, 좌우 입력이 토크를 준다(`PlayerAvatarMenu.cs:33-66,107-114`).

**"모두 준비됨" 같은 준비 상태 개념은 없다.** 호스트가 Start를 누르면 즉시 시작한다. 막는 것은 오직 "누군가 접속 중" 하나다. 클라이언트에는 대기 중이라는 표시가 있을 뿐 준비 버튼이 없다.

### 5.2 걸어 다니는 트럭 로비 (`levelLobby`)

위치는 "상점 다음, 다음 원정 전"이다(`RunManager.cs:297-300`). 새 판에서는 건너뛰고, 상점에서 저장한 세이브를 불러왔을 때에만 로비 메뉴에서 바로 올 수 있다(0.1절 1번).

- **플레이어는 자유롭게 걷는다.** 스폰 직후 카메라가 트럭 화면(`TruckScreenText`)을 0.3초 동안 강제로 바라본다. 이 구속은 모든 트럭 화면 레벨의 공통 처리다(`PlayerAvatar.cs:625-647`).
- 첫 목표 문구가 3초 동안 뜬다. 트럭 로비에서는 "여정을 즐기고, 물건을 충전하고, 장비를 챙겨라"는 뜻이고, 충전소 코루틴이 씬 시작 2초 뒤에 띄운다. 상점에서는 "상점에서 물건을 사라"가 트럭 문이 열릴 때 같은 방식으로 뜬다(`ChargingStation.cs:484-495`, `TruckDoor.cs:87-90`, `SemiFunc.cs:2513`). 충전소 코루틴의 대기 조건이 `Generated`가 참인 동안 기다리는 형태라 정확한 시점은 런타임에서 확인해야 한다.
- 구입한 아이템이 이 방에 놓인다. 단, 파워 크리스탈은 트럭 로비에서만, 카트는 트럭 로비를 제외한 레벨에서만 스폰된다(`ItemManager.cs:149-150`). 아이템 배터리는 여기서 충전되고 배터리 표시가 강제로 켜져 "충전이 필요하다"는 점멸이 붙는다(`ItemBattery.cs:220-231`, `BatteryUI.cs:58`). 손전등은 트럭 로비에서 꺼진다(`FlashlightController.cs:150`).
- **출발은 호스트가 정하는 것이 아니다. 누구든 트럭의 CRT 화면을 "꾹 누르면" 출발한다.** 5.3절에서 설명한다.
- 이 레벨에서는 "전원 승차" 조건을 검사하지 않는다. `ChatMessageLobby`가 `PlayersAllInTruck`을 확인하지 않고 곧바로 성공 처리하기 때문이다(`TruckScreenText.cs:564-569`). 방 자체가 트럭 안이라는 전제다. 반면 레벨 안의 출발 화면과 상점은 전원 승차를 요구한다(`TruckScreenText.cs:540-562,578-592`).
- 락 문구는 트럭 로비에서는 "도로로 출발"에 해당하는 문구이고, 그 외 레벨에서는 "시동 거는 중"이다. 후자만 2~4초마다 엔진 공회전 소리를 낸다(`TruckScreenText.cs:1380-1401`).
- 출발 뒤에는 `GotoNextLevel`이 `ChangeLevel(true, false)`를 불러 원정 레벨로 간다(`TruckScreenText.cs:1120-1124`). 이 함수를 언제 부르는지(어느 페이지 줄이 끝날 때인지)는 프리팹의 UnityEvent라 코드에서 확인하지 못했다. 같은 일을 하는 `TruckScreenOpen`(화면 문이 올라가고 2초 뒤 레벨 전환)이 있으나 어느 레벨의 프리팹이 쓰는지는 확인하지 못했다(`TruckScreenOpen.cs:94-104`).

### 5.3 트럭 화면 "꾹 누르기" 출발 장치

repo-run.md는 페이지 enum과 전원 승차 판정을 다뤘으므로 여기서는 입력 방식과 연출만 보탠다. 모든 수치는 `TruckScreenText.cs`다. 누르기를 시작·해제하는 호출(`StartChat`, `ReleaseChat`)은 프리팹 이벤트로 연결되어 있어 코드에 호출처가 없다. 아래 "잡기 키"는 이 오브젝트가 `StaticGrabObject`를 달고 있고 전송 시 잡고 있는 사람의 이름을 쓰는 것(`TruckScreenText.cs:941-948`)에서 읽은 것이다.

- 화면 앞의 "채팅 상자"는 **정적 잡기 오브젝트**다. 플레이어가 잡고 있는 동안 `chatActive`가 켜지고, 누른 시간만큼 `chatMessageTimer`가 **초당 1.5씩** 차올라 2에 닿으면 전송된다. 즉 **약 1.33초를 누르고 있어야 한다**(`TruckScreenText.cs:131,935-936`).
- 누르는 동안 미리 정해진 메시지 문자열이 진행도에 비례해 한 글자씩 찍히고, 글자마다 타이핑 효과음이 난다. 충전 루프음의 음높이는 `1 + 진행도/2`로 올라간다(`TruckScreenText.cs:912-913,960-997`). 이모지 태그는 이모지 전용 효과음이 난다. 로딩 바(`chatMessageLoadingBar`)도 같은 값으로 `Lerp(…, 10 × deltaTime)` 채워진다(`TruckScreenText.cs:405-406`).
- 도중에 놓으면 진행이 0.2초 뒤 초기화된다(`TruckScreenText.cs:407-417`).
- 전송에 성공하면 누른 사람의 이름과 그 사람 색으로 채팅 줄이 화면에 찍히고(`TruckScreenText.cs:1080-1087`) **3초 동안 다시 누를 수 없다**(`chatDeactivatedTimer = 3`, `TruckScreenText.cs:1085,918-922`).
- 결과는 화면 아래 막대의 **초록 번쩍(성공)** 또는 **빨강 번쩍(실패)** 0.2초와 성공/실패 효과음, 막대 조명 색 변화로 보인다(`TruckScreenText.cs:507-538`).
- 출발이 결정되면 상태가 `LockedStartingTruck`(노랑·주황 잠금 배너와 문구)로 바뀌고 잡기 콜라이더가 꺼진다. 아직 못 탄 사람이 있으면 `LockedDestroySlackers`(자홍·빨강 배너)로 바뀐다(`TruckScreenText.cs:1380-1414`).
- 레벨 시작 후 마스터가 1초를 기다린 다음 모든 클라이언트에 화면 텍스트를 시작시킨다(`TruckScreenText.cs:419-430`).
- 이 모든 결정은 마스터 클라이언트가 내리고 RPC로 퍼뜨린다(`GotoPageRPC`, `PlayerChatBoxStateUpdateRPC` 등).
- 이 화면은 호스트 전용 버튼이 아니다. 전송 판정과 상태 전파는 모두 마스터에서 일어나지만(`StartChatRPC`·`ChatMessageSendRPC`가 `MasterOnlyRPC` 검사를 한다, `TruckScreenText.cs:902-908,953-956,1112-1118`), 누르는 사람이 호스트일 필요는 없고 전송 메시지에는 누른 사람의 이름이 쓰인다. (잡기 상태가 마스터에 모이는 구조로 읽히지만 이 부분은 `PhysGrabber` 쪽을 읽지 않았다.)

**두고 간 사람에 대한 연출(자기 파괴 카운트다운).** 출발 선언 시 전원 승차가 아니었다면 `LockedDestroySlackers` 상태로 들어가고(5.3절 앞 단락), 이때 쓰이는 것으로 읽히는 `SelfDestructPlayersOutsideTruck`이 모두에게 전파된다(`TruckScreenText.cs:1089-1109`; 이 함수의 호출처는 페이지 이벤트라 코드에서는 확인하지 못했다). 트럭 밖에 있는 살아 있는 플레이어의 **채팅창이 빙의**되어, 본인이 아니라 게임이 대신 타이핑한다. 내용은 "버려졌다"는 푸념을 무작위로 조합한 한 줄, "트럭까지 가야 한다…" 안내, 10부터 1까지의 카운트다운, 마지막으로 "배신" 계열 문구다. 문구는 붉은색이고 모두 트럭 화면 채팅에도 같이 찍힌다(`ChatManager.cs:578-604`, `SemiFunc.cs:857-869`). 마지막 문구가 끝나면 0.2~3초 뒤 본인이 죽는다(`ChatManager.cs:572-576,649-654`). 그 사이 **트럭 안에 들어가면** "자기 파괴 취소" 문구가 초록색으로 나오고 빙의가 풀린다(`ChatManager.cs:606-618,806-808`). 마스터는 초당 5번 "살아 있는 사람이 전부 트럭 안이거나 이미 면제되었는가"를 검사해, 만족하면 출발한다(`TruckScreenText.cs:368-396`). 즉 낙오자는 막다른 길이 아니라 **도망칠 시간이 있는 짧은 코미디 컷신**이다.

## 6. 사망 흐름 (개인)

repo-run.md가 다룬 것은 머리의 부활 규칙과 관전 카메라의 상태 이름이다. 여기서는 화면에 일어나는 일의 **순서와 시간**을 보탠다.

### 6.1 죽는 순간부터의 타임라인 (로컬 플레이어 기준)

| 시각 | 일어나는 일 | 근거 |
|---|---|---|
| 0.00 | 체력이 0 이하가 되면 `PlayerDeath`가 모두에게 RPC를 보낸다. 수신 측 모두에서 `deadSet = true`가 되고, 죽은 본인의 클라이언트에서는 체력 통계에 0이 기록된다. 이미 `deadSet`이면 무시 | `PlayerHealth.cs:416-420,351-363`, `PlayerAvatar.cs:1342-1369` |
| 0.00 | **남의 화면**: 죽는 사람 위치에서 "죽음 빌드업" 효과음 | `PlayerAvatar.cs:1370-1373` |
| 0.00 | **본인 화면**: 잡고 있던 것을 놓고, 컨트롤러 오브젝트 비활성(이동 정지), 긴 피격 글리치, `GameDirector.DeathStart()` → 상태 `Death` | `PlayerAvatar.cs:1390-1397` |
| 0.00~0.5 | 매 프레임 카메라가 **죽인 몬스터의 "킬 룩앳" 지점**을 바라보게 강제된다(유지 시간 1초를 매 프레임 갱신, 회전 속도 80). 몬스터 에셋에 이 지점이 없으면 오류 로그 | `PlayerAvatar.cs:706-711,1378-1389`, `CameraAim.cs:77-88` |
| 0.00 | 첫 프레임: HUD 숨김, 사망 효과음, 렌더 텍스처 해상도를 **20%로** 낮추는 변화를 0.5초에 걸쳐 시작(픽셀 뭉개짐) | `GameDirector.cs:289-303` |
| 0.00~0.2 | **프리즈**(`deathFreezeTime = 0.2`) | `GameDirector.cs:99,305-308` |
| 0.2 | 오디오 스냅샷 "컷신 전용", 화면 흔들림, 화면 1.25배 확대, **카메라 프리즈**(남은 시간 + 0.1초). 이 효과들은 `gameStateTimer`의 남은 값(≈0.3초)에 맞춰 상태 종료와 함께 끝난다 | `GameDirector.cs:308-314` |
| 0.5 | `PlayerDeathDone`: 음성 1초 음소거 후 음성 믹서를 로비용(공간감 없음)으로 전환, `isDisabled = true`, 사망 지점 표식 생성, 집기 해제와 인벤토리 강제 해제, 사망음, **머리 활성화**, **사망 이펙트**, 아바타 게임오브젝트 끄기 | `PlayerAvatar.cs:191,712-716,1400-1444` |
| 0.5 | 사망 이펙트: 플레이어 꾸밈 색으로 칠한 파편 입자와 연기·불, 사라지는 조명, 주변 카메라 흔들림(거리 3~8 m, 세기 5), **0.5초 동안 켜지는 피해 콜라이더** | `PlayerDeathEffects.cs:31,69-103` |
| 0.5 | `Death` 상태가 `Main`으로 돌아가며 오디오 "관전" 스냅샷, HUD 다시 표시, 렌더 변화 복원, **`SetSpectate()`로 관전 카메라 생성** | `GameDirector.cs:316-327`, `PlayerAvatar.cs:993-997` |

이후 로컬 플레이어의 게임 상태는 `Main`을 유지하고 관전 카메라가 화면을 담당한다(repo-run.md가 적은 대로). 위 순서에서 새로 확인한 것은 **죽인 자를 쳐다보는 강제 시선, 20% 해상도 뭉개짐, 프리즈 후 줌, 파편 폭발이 인접한 사람에게 주는 피해 콜라이더**다.

### 6.2 관전 카메라의 세 단계

1. **Death(사망 시점 카메라)**: 죽은 자리 위에서 거의 수직(피치 88°)으로 내려다보는 탑다운 궤도 카메라다. 시야각 8°(시작 16°에서 줄어듦), 거리 2 m, 근평면은 위쪽 "천장" 태그 물체를 레이캐스트해 자동으로 조정한다. 마우스 좌우로 시점을 돌릴 수 있다(`SpectateCamera.cs:150-225`). 첫 프레임에 카메라 충격(2, 0.5초)·화면 흔들림(2, 1초)·짧은 글리치가 있다(`SpectateCamera.cs:180-183`). **지속 시간: 4초(아레나에서는 1.5초)**(`SpectateCamera.cs:171-177`).
   - 튜토리얼 레벨이면 시간이 끝나는 순간 전원을 부활시킨다(`SpectateCamera.cs:257-264`).
   - 마스터 기준 "전원 사망" 조건에 걸리지 않은 한, 지속 시간이 끝나면 Normal로 넘어간다. 아레나에서는 살아 있는 사람이 하나라도 있어야 넘어가고, 모두 죽었으면 1초 간격으로 계속 머문다(`SpectateCamera.cs:265-286`).
   - **부활 가능성이 있으면 대기가 연장된다.** 현재 추출 지점이 작동 중이고(대기·활성 상태가 아님) 모든 플레이어가 죽었으며 그중 누군가의 머리가 추출 지점 안에 있으면 `OverrideDeathDelay`가 불려 0.25초짜리 대기를 계속 건다. 부활 아이템을 쓰는 중에도 같다(`SpectateCamera.cs:226-250,754-776`, `ItemReviveItem.cs:279`). 추출이 끝나 부활이 일어날 시간을 주려는 장치다.
   - 4초 중 3초 이하가 남았는데 머리가 화면 밖으로 굴러갔으면 카메라가 머리 쪽으로 순간 이동한다(`SpectateCamera.cs:761-774`).
2. **Normal(일반 관전)**: 살아 있는 다른 사람을 **3인칭 궤도 카메라**로 따라다닌다. 마우스로 돌리고(좌우 ±360° 순환, 상하 ±70°), 스크롤로 거리를 1~6 m로 조절하며(기본 3 m), 벽에 파묻히지 않도록 구체 레이캐스트로 거리를 줄이고 근평면을 조정한다(`SpectateCamera.cs:289-431`). 대상을 바꾸는 입력은 점프, 다음 관전, 이전 관전이다(`SpectateCamera.cs:437-448`). 관전 대상이 죽으면 카메라가 그 시신 위의 Death 카메라로 되돌아가 다시 4초를 쓴다(`SpectateCamera.cs:604-610`, `PlayerAvatar.cs:1419-1425`). 음성: 관전 대상의 목소리는 공간감을 잠깐 끈다(`SpectateCamera.cs:449-452`).
   - 첫 진입 시 팁이 두 개 예약된다("머리 관전") (`SpectateCamera.cs:310-314`).
3. **Head(머리 시점)**: `headEnergy`가 0.25 이상이고 Normal에서 0.5초가 지났으면 상호작용 키로 진입한다. 머리의 눈높이(머리 `centerPoint`)에서 1인칭으로 보며, 색은 어둡고 채도가 -50으로 빠진다. 진입 시 카메라가 `Lerp(…, 25 × deltaTime)`로 머리를 따라간다(`SpectateCamera.cs:453-537`). 에너지 수치는 repo-run.md 참고.
   - 머리를 **점프 키로 충전해 튕겨 이동**할 수 있다. 머리 시점에서 점프 키를 누르고 있으면 머리가 바닥에 닿아 있는 동안 충전량이 초 단위로 쌓이고(최대 2), 떼면 그 충전량만큼 튕겨 오르며 재사용 대기가 2초다. 충전할수록 머리 모델이 더 떨린다(`PlayerDeathHead.cs:138,858-867,898-942`). 에너지가 낮을 때는 본인 음성의 피치가 낮아지는 연출도 있다(`PlayerDeathHead.cs:850-853`). 죽은 사람이 아무것도 못 하고 기다리지 않게 하는 장치다.
   - 특정 효과로 "머리가 강제로 관전 대상이 되는" 상태(`overrideSpectated`)가 되면 붉은 비네트·대비 강화와 함께 0.5초짜리 충격이 나며 에너지가 가득 차고 에너지가 줄지 않는다(`SpectateCamera.cs:454-463,474-507`).
   - 머리 시점이 끝나면 머리에서 가장 가까운(999 m 이내) 살아 있는 플레이어를 다음 관전 대상으로 예약한다(`SpectateCamera.cs:526-533`).

### 6.3 관전 중의 UI

- 멀티플레이이고 Normal 상태일 때만 **관전 대상 이름**이 표시된다. 아레나가 `GameOver` 상태이면 숨는다(`SemiFunc.cs:2562-2568`, `SpectateNameUI.cs`).
- 죽은 플레이어 목록(`MenuSpectateList`)이 멀티에서만 뜬다. 0.25초마다 "죽은 사람"을 추려 얼굴 아이콘과 이름으로 쌓는다. 머리 시점 중에는 숨는다(`MenuSpectateList.cs:15-69`). 지금 머리 시점에 들어가 있는 죽은 사람의 이름은 거의 검정으로 어두워져, 누가 머리로 움직이는 중인지 구별된다(`MenuPlayerListed.cs:115-119`).
- **머리 시점 진입 UI**(`SpectateHeadUI`): 눈 모양 아이콘이 있고, 눈꺼풀이 `headEnergy`에 비례해 열린다(`SpectateHeadUI.cs:335-336`). 에너지가 충분해지면 "[상호작용 키] 누르기" 안내 문구가 뜨고 준비 효과음이 난다(`SpectateHeadUI.cs:246-270`).
- 관전 중에도 **채팅은 가능하다.** 죽은 사람의 TTS는 항상 속삭임(웅크린 음성)으로 처리되어 산 사람에게 방해가 덜 된다(`PlayerAvatar.cs:1672-1676`). 음성 채팅도 죽으면 공간감 없는 믹서로 옮겨져(`PlayerAvatar.cs:1402-1406`) 죽은 사람들끼리와 산 사람 모두가 말할 수 있다.
- 관전 상태 동안 체력·에너지·인벤토리·조준·미션 UI는 매 프레임 숨겨진다(`SpectateCamera.cs:122-129`).

### 6.4 다음 레벨에서의 모습

- 레벨이 바뀌면 씬을 다시 로드하고 모든 아바타를 새로 만든다. 죽었던 사람도 살아서 시작한다. 단 **체력은 저장된 값이고, 0 이하이면 1로 올려서 시작한다**(`PlayerHealth.cs:196-214`). 죽음의 대가가 "1 HP로 다음 레벨을 시작함"이다.
- 아레나에서는 체력이 최대로 채워진다(`PlayerHealth.cs:210-213`).
- 죽는 횟수는 진행 점수에도 영향을 준다(0.05씩 감점, 최대 0.5, repo-run.md 참조).

## 7. 전원 사망 → 아레나 → 게임오버 → 복귀

### 7.1 전멸 판정까지

repo-run.md가 적은 판정 조건에 **시간 조건**을 보탠다. 마스터가 전원 `isDisabled`를 감지하면 즉시 `AllPlayersDeadSet(true)`를 호출해 다른 클라이언트에 `AllPlayersDeadRPC`를 보내 놓는다. 그러나 `ChangeLevel`은 **마스터의 관전 카메라가 `Death` 상태를 벗어난 뒤에만** 호출된다(`RunManager.cs:198-205`). 따라서 마지막 사람이 죽은 뒤 대략 이렇게 흐른다.

```
마지막 사망 T=0
 ├ 0.5s  Death 상태(프리즈·픽셀화) 종료, isDisabled 설정 → 전멸 감지 시작(AllPlayersDeadSet true)
 ├ 0.5~4.5s  관전 카메라 Death 상태(탑다운)  ← 이 동안 ChangeLevel 보류
 │            (추출 중 머리가 추출 지점 안이면 여기서 더 연장될 수 있음)
 └ ≈4.5s ChangeLevel(false, true) 호출
```

(숫자는 `GameDirector.cs:297`, `PlayerAvatar.cs:191`, `SpectateCamera.cs:177`에서 도출한 값이다. 마스터가 마지막으로 죽은 경우의 계산이며, 마스터가 먼저 죽은 경우 마스터의 4초가 먼저 지나 있다.)

살아 있던 마지막 동료를 지켜보던 중 그 사람이 죽으면, 관전 카메라가 그 시신으로 되돌아가 새 Death 4초를 쓴다(6.2절). 그래서 마지막 죽음 뒤의 "침묵의 4초"는 항상 보장된다.

### 7.2 화면의 순서 (전원 사망 시)

1. 죽음 프리즈·픽셀화(0.5초)
2. 탑다운 사망 카메라(약 4초)
3. **즉시 검정 컷**: `GameDirector.OutroStart`가 `Main` 상태를 `Outro`로 바꾸며 `FadeOverlay`를 곧바로 검정으로 만든다(`GameDirector.cs:390-403`). 아웃트로 효과음, HUD 숨김, 음성 믹서를 로비용으로. **0.1초** 뒤 `Result`(결과 화면 상태)로 넘어간다(`GameDirector.cs:220-259`).
4. `Result`는 전원 사망이면 바로 `End`로 건너뛴다(`ResultScreenUI.cs:145-150`). 결과 화면 UI(문 닫는 트럭 + 꾸미기 토큰 연출)는 **꾸미기 토큰을 얻었을 때에만** 뜨는 보상 연출이고 점수표가 아니다(7.5절).
5. `End`: 오디오를 끄고 0.5초에 걸쳐, 로컬 아바타를 비활성, 로딩 UI 시작, 0.5초 후 `OutroDone` → `EndWait`(`GameDirector.cs:266-285`).
6. 마스터가 모든 아바타의 `outroDone`을 기다린 뒤 `Reload` → `Main` 로드(`RunManager.cs:359-411`).
7. 새 레벨(아레나)이 생성되는 동안 로딩 UI. 레벨 생성은 0.2초 + 전원 대기 + 0.2초 + 모듈 생성 + 아이템·귀중품·몬스터 설정 순서이고(`LevelGenerator.cs:276-407`), 아레나는 `ItemSetup`을 건너뛴다(`LevelGenerator.cs:538`). 25초를 넘기면 "로딩에서 멈춤" 안내가 뜬다(3.5절).
8. **레벨 카드**: 아레나는 제목이 **빨간 "게임오버"**, 부제는 아레나 레벨 이름이다(`LoadingUI.cs:189-192,205`). 10.2절 참조.
9. 카드 애니메이션이 모든 플레이어에게 끝나고 0.5초 뒤 로딩 UI가 걷히며 아레나가 시작된다(`GameDirector.cs:180-213`).

### 7.3 아레나 (`Arena`) — 게임오버 이후의 진행

전멸한 팀이 마지막으로 노는 미니게임이다. 시간은 모두 `Arena.cs`의 상수다.

| 단계 | 멀티플레이 | 싱글플레이 |
|---|---|---|
| 시작 | 바닥 문이 열리고(`OpenHatch`가 `Falling`으로. 호출 시점은 프리팹의 이벤트라 미확인) 안내 문구 "마지막까지 살아남은 패자"가 뜬다 | 안내 문구 "게임 오버"가 뜬다 |
| `Falling` 5초 | 전원 5초 무적, 바닥이 열려 떨어짐, 글리치·흔들림 | 플랫폼이 곧바로 내려감 |
| `Starting` 2초 | 파이프 조명 끄고 충돌 판정 해제 | — |
| `Level1`~`Level3` | 각 30초. 사이마다 `PlatformWarning` 3초(경보음, 조명 맥동, 화면 흔들림), `PlatformRemove` 3초(플랫폼 낙하). 마지막 플랫폼이 사라진 뒤에는 생존 판정으로만 끝남 → 최종 플랫폼 제거까지 합계 **약 115초**(5+2+30+3+3+30+3+3+30+3+3) | — |
| 중반 무기 | 두 번째 플랫폼 경고가 시작된 뒤부터(`level >= 2`) 5초마다 중앙에서 근접·총 무기 한 개 스폰 | — |
| 왕관 | 중앙 왕관 우리는 생존자 1명 이하가 되면 부서진다. 왕관을 잡은 사람이 우승(`CrownGrab`) | — |
| `GameOver` | **10초**. 우승자가 있으면 "패자들의 왕!"(승자의 얼굴 + 양쪽 왕관), 없으면 "전원이 패자!"(`EVERYONE'S A LOSER!`) + 패배 징글 | **3초** |
| 종료 | `ChangeLevel(false, true)` | 같음 |

- 멀티에서는 입장 시 마스터가 아이템 지급 목록을 무작위로 만든다. 소모품 1개와 근접 무기 5개는 확정이고, 총·카트·드론/구체·회복 아이템은 후보마다 30% 확률이다(`Arena.cs:165-232`).
- 살아남은 인원이 줄 때마다 경고음과 조명 색 변화, 발판 스크린의 숫자가 바뀐다(`Arena.cs:610-680`). 아레나 음악 루프가 `Falling`에 시작해 `GameOver`에 끈다(`Arena.cs:560-568`).
- 아레나에서도 죽은 사람은 관전한다. 관전 카메라의 Death 상태가 1.5초로 짧고, 산 사람이 있는 한에만 Normal로 넘어간다(6.2절).
- 아레나에서는 스태미나 회복이 5배, 회복 대기가 절반이다(repo-run.md 참조).
- **레이스형(`ArenaRace`)**: 같은 `ArenaMessageUI`/`ArenaMessageWinUI`를 쓰고 승자 연출 10초, "전원 패자" 연출 5초 뒤 `ChangeLevel(false, true)`를 부른다(`ArenaRace.cs:631-679`). 시작 카운트다운은 트럭 화면에 "3…", "2…", "1…"이 찍힌다(`ArenaRace.cs:420-482`).
- 이 모든 것이 **죽은 뒤의 즐길 거리**다. 게임오버가 "빈 화면 + 확인 버튼"이 아니라 "한 판 더 놀 수 있는 레벨"이다.

### 7.4 게임오버 이후: 복귀 지점과 세이브

`ChangeLevel`의 아레나 분기(`RunManager.cs:230-253`):

| 모드 | 다음 레벨 | 진행도 | 세이브 |
|---|---|---|---|
| 멀티 | **로비 메뉴(같은 방)** | `ResetProgress()` | 이미 전멸 때 삭제됨 |
| 싱글 | **새 원정 레벨(1레벨째)** — 메인 메뉴로 가지 않는다 | `ResetProgress()` | 이미 삭제됨 |

- 멀티에서는 `gameOver = true`가 서고, 모든 클라이언트가 `UpdateLevelRPC`로 이 사실을 받는다. `RestartScene`에서 `NetworkManager.DestroyAll()`이 모든 네트워크 오브젝트(아바타 포함)를 파괴하고(`RunManager.cs:384-389`), 로비 메뉴 레벨에서 `NetworkManager.Start`가 아바타를 새로 만든다(`NetworkManager.cs:42-67`). **같은 Photon 방과 Steam 로비가 그대로 유지되므로** 파티가 해산되지 않는다. `LobbyMenuOpen`이 다시 방을 열고(4.3절) 호스트가 Start를 눌러 새 판을 시작할 수 있다.
- `gameOver`가 서 있는 동안 `OnSceneSwitch`는 저장하지 않는다(`SemiFunc.cs:2988`).
- 세이브 삭제는 두 번 확인된다. 전멸 → 아레나 전환 시점(0.1절 3번)에 지워지고, 아레나 → 로비 메뉴 전환에서도 `allPlayersDead`가 서 있으므로 다시 시도한다(이미 없으면 경고 로그만 남는다, `DataDirector.cs:437-447`, `StatsManager.cs:1099-1102`). 단 `SaveFileDelete`는 `saveFileCurrent` 문자열을 비우지 않으므로(`StatsManager.cs:1091-1103`) 호스트가 다음 판을 시작해 다음 씬 전환에서 저장하면 **같은 이름의 새 세이브 폴더가 다시 만들어지는 것으로 읽힌다**(런타임 미확인).
- 싱글의 게임오버는 `SetRunLevel()`을 불러 새 판의 첫 레벨로 곧장 이어진다. 즉 싱글의 "게임오버 → 재도전" 간격은 레벨 카드 하나 + 아레나 약 8초(Falling 5 + GameOver 3)다.

### 7.5 결과 화면은 점수표가 아니다

`ResultScreenUI`는 `FadeIn 0.2초 → CloseDoor 0.5초(트럭 문이 닫힘) → (꾸미기 토큰이 있으면) MiddleFade 1초 → Cosmetics(토큰 연출, 메뉴 키로 건너뛰기) → FadeOut 1초`로 돈다(`ResultScreenUI.cs:188-299`). 원정 레벨이 아닌 곳에서는 시작부터 건너뛴다(`ResultScreenUI.cs:139-143`). **돈, 목표, 사망자 같은 정산을 보여 주는 UI는 이 클래스에 없다.** 정산은 레벨 안의 추출 지점 화면과 트럭 화면, 상점이 맡는다(repo-run.md 참조). 따라서 "죽고 → 점수표 → 메뉴"가 아니라 "죽고 → 곧바로 다음 레벨"이다.

### 7.6 호스트 이탈·연결 끊김

| 사건 | 처리 | 근거 |
|---|---|---|
| 호스트가 ESC 메뉴에서 "메뉴로 나가기" | 아웃트로 → 방 나가기. 로비 Steam 데이터의 `BuildName`을 빈 문자열로 지워 "방을 망가뜨린다" | `MenuPageEsc.cs:103-108`, `SteamManager.cs:268-277` |
| 클라이언트가 호스트 이탈을 감지 | `OnMasterClientSwitched`로 "호스트 끊김" 팝업 예약 + 강제 퇴장. **호스트 이전 없음** | `NetworkManager.cs:172-181` |
| 이후 늦게 들어오려는 사람 | Steam 로비 빌드 이름이 비어 있어 "로비가 닫혔다" 팝업 | `NetworkConnect.cs:126-135` |
| 클라이언트가 중간에 나감 | `OnPlayerLeftRoom`은 로그만 남긴다. Photon이 그 사람의 아바타를 파괴하고 `OnDestroy`가 `PlayerList`에서 제거하므로 전멸 판정에서 빠진다 | `NetworkManager.cs:167-170`, `PlayerAvatar.cs:311-314` |
| 클라이언트 연결 끊김(일반 원인) | 아웃트로 → 방 나가기. 메인 메뉴 도착 후 팝업. 메인 메뉴 중이면 페이지를 닫고 메인을 다시 열며 즉시 팝업 | `NetworkManager.cs:201-228` |
| 레벨 전환 중 어떤 클라이언트가 멈춤 | 마스터는 전원 `outroDone`/`PlayerReady`를 무한정 기다린다. **마스터 쪽 타임아웃은 없다**. 대신 멈춘 클라이언트가 25초 뒤 "멈춤" 안내를 받아 스스로 나갈 수 있다 | `RunManager.cs:359-382`, `NetworkManager.cs:122-133` |
| 방을 나가고 있는데 Photon이 아직 연결 중 | `LeaveToMainMenu`가 Photon이 "끊김" 상태가 될 때까지 기다렸다가 메인 메뉴 레벨 `Reload` | `RunManager.cs:524-535` |

## 8. 일시정지·종료 흐름

### 8.1 ESC 메뉴는 정지가 아니다

- **게임은 멈추지 않는다.** 코드 전체에서 `Time.timeScale`을 건드리는 곳이 없다(디버그 배율 필드와 시간 유리병 아이템의 개인 배율만 있다, `PlayerController.cs:463-476`). 메뉴가 열려도 몬스터와 다른 플레이어는 계속 움직인다.
- 열린 동안 마우스 커서가 풀리고 게임 입력이 막히며 HUD가 숨는다(3.1절).
- ESC로 열고 닫는다. 이미 메뉴가 열려 있고 그것이 ESC 메뉴면 닫는다. 채팅 입력 중이면 ESC는 채팅을 닫는 데 쓰이고 메뉴는 열리지 않는다(`GameDirector.cs:332-346`). 메뉴 레벨(메인 메뉴·로비 메뉴·스플래시)에서는 ESC 메뉴 자체가 없다(`MenuPageEsc.cs:31-37`, `GameDirector.cs:332`).

### 8.2 ESC 메뉴의 버튼 (`MenuPageEsc`)

| 핸들러 | 동작 | 근거 |
|---|---|---|
| Continue | 페이지 닫기 | `MenuPageEsc.cs:39-42` |
| Settings | 설정과 교체(Back이면 ESC로 복귀) | `MenuPageEsc.cs:118-121`, `MenuPageSettings.cs:47-51` |
| Customize | 꾸미기와 교체 | `MenuPageEsc.cs:110-116` |
| Moons | 달 단계 정보. 달 레벨이 1 이상일 때만 보인다(없으면 항목을 감추고 나머지를 위로 올림) | `MenuPageEsc.cs:123-135` |
| Self-destruct | 멀티: 채팅창을 빙의시켜 작별 문구를 타이핑한 뒤 0.2~3초 후 본인이 죽는다. 싱글: 즉시 사망 | `MenuPageEsc.cs:78-90`, `ChatManager.cs:620-637,649-654` |
| Quit to Menu | 아웃트로 후 방 나가기 → 메인 메뉴 | `MenuPageEsc.cs:103-108` |
| Quit | 종료 플래그 → 아웃트로 → `Application.Quit()` | `MenuPageEsc.cs:92-101` |
| (슬라이더) | **다른 플레이어마다** 음성 음량 슬라이더가 생긴다. 사람이 들어오고 나가면 목록이 갱신된다 | `MenuPageEsc.cs:44-76` |

**"자기 파괴" 버튼이 있다는 점이 눈에 띈다.** 낙오하거나 어디에 끼어서 팀에 짐이 될 때 스스로 정리하는 정식 수단이다.

### 8.3 나가기 경로와 저장

- **메뉴로 나가기**: `leavePhotonRoom = true`를 켜고 아웃트로를 건다. `PlayerAvatar.OutroDone`이 이 플래그를 보고 `RestartScene`을 거치지 않고 곧장 `LeavePhotonRoom`을 호출한다(`PlayerAvatar.cs:1467-1485`). Photon 연결 해제, Steam 로비 퇴장(내가 방장이면 `BuildName`을 비움), 싱글 모드로 복귀, 튜토리얼이면 튜토리얼 종료, 그리고 `LeaveToMainMenu`로 메인 메뉴 레벨 `Reload`(`NetworkManager.cs:144-156`).
- **저장 규칙**: 나갈 때 저장은 **직전 레벨이 걸어 다니는 트럭 로비였을 때에만** 한다(`SemiFunc.cs:2988` — `!_leaveGame || levelPrevious == levelLobby`). 원정 레벨이나 상점에서 나가면 저장하지 않는다. 대신 그 전환 시점(상점 입장·원정 시작)에 이미 저장되어 있다(`RunManager.cs:322-340`). 진행도가 0(첫 레벨을 마치기 전)인 상태에서 나가면 세이브가 **삭제된다**(`DataDirector.cs:441-444`). 앱 종료 시에도 같은 검사가 돈다(`RunManager.cs:218-221`).
- **종료**: 메인 메뉴 종료 버튼도 같은 아웃트로를 거치되 `skipLoadingUI = true`로 로딩 카드를 생략한다(`MenuPageMain.cs:223-232`).

### 8.4 튜토리얼

`ButtonEventTutorial`은 호스트가 아니라 **싱글 레벨 진입**이고, 끝나면 트럭 화면의 `TutorialFinish`가 아웃트로와 방 나가기를 걸어 메인 메뉴로 돌려보낸다(`TruckScreenText.cs:322-326`). `ChangeLevel`은 튜토리얼 유형일 때 `OnSceneSwitch`(저장·동기화)를 건너뛴다(`RunManager.cs:336-339`). 튜토리얼에서 죽으면 관전 카메라 4초 뒤 모두 부활한다(6.2절).

## 9. 전환 시간표 (초)

코드에 상수가 있는 것만 적었다. 애니메이터·`AnimationCurve`·UnityEvent로 정해지는 시간은 "미확인"이다.

| 전환 | 시간 | 근거 |
|---|---|---|
| 스플래시: 로고 | 2 | `SplashScreen.cs:80` |
| 스플래시: 경고 카드 | 6 | `SplashScreen.cs:97` |
| 메인 메뉴 첫 진입 인트로 대기 | 3 (이후 슬라이드) | `MenuPageMain.cs:152` |
| 예약 팝업 표시 지연 | 1.5 | `MenuPageMain.cs:20` |
| 튜토리얼 버튼 깜빡임 | 0.5 간격 | `MenuPageMain.cs:77` |
| 지역 목록: 연결 후 대기, 줄 간격 | 0.3, 0.15 | `MenuPageRegions.cs:89,197` |
| 서버 목록: 줄 간격 / 페이지 넘김 줄 간격 | 0.1 / 0.05 | `MenuPageServerList.cs:107,224` |
| 호스트: 비밀번호 페이지 표시 지연 | 2 | `NetworkConnect.cs:26,84-88` |
| 레벨 생성: 시작 대기 + 전원 대기 후 | 0.2 + 0.2 | `LevelGenerator.cs:278,288` |
| `Reload` 씬 최소 체류 | 0.1 | `ReloadScene.cs:17` |
| 전환 아웃트로(검정 컷 후 `Result`로) | 0.1 | `GameDirector.cs:238` |
| `End`(오디오 끄고 로딩 UI) | 0.5 | `GameDirector.cs:273` |
| 레벨 시작 `Start` 상태 | 0.5 + 카드 애니메이션 완료 대기 | `GameDirector.cs:175,180-190` |
| 로딩 멈춤 안내 | 25 | `NetworkManager.cs:125` |
| 로비 메뉴 페이지 열기(레벨 시작 후) | 2 | `LobbyMenuOpen.cs:8` |
| 로비 "접속 중" 알림 지속 / 아바타 등장 후 | 10 / 1 | `MenuPageLobby.cs:234,177` |
| 로비 플레이어 목록 갱신 | 1 | `MenuPageLobby.cs:108` |
| 트럭 문 열림 지연 / 열리는 데 걸리는 시간 | 2 / 1÷1.5≈0.67 | `TruckDoor.cs:24,95` |
| 트럭 화면 시작 대기 | 1 | `TruckScreenText.cs:421` |
| 트럭 출발 "꾹 누르기" | 2÷1.5≈1.33 | `TruckScreenText.cs:131,935` |
| 전송 후 재입력 잠금 | 3 | `TruckScreenText.cs:1085` |
| 상점 다음 레벨 이동 지연 | 0.5 | `TruckScreenText.cs:1157` |
| 화면 문(`TruckScreenOpen`) 자동 닫힘 / 열린 뒤 레벨 전환 | 2 / 2 | `TruckScreenOpen.cs:96,102` |
| 스폰 직후 트럭 화면 강제 응시 | 0.3 | `PlayerAvatar.cs:634` |
| 레벨 시작 팔 인트로 지연 | 0.25 | `ArmIntroController.cs:16` |
| 목표 문구(UIFocusText) 기본 지속 | 3 | `SemiFunc.cs:2513` |
| 사망: 프리즈 / 상태 총 길이 | 0.2 / 0.5 | `GameDirector.cs:99,297` |
| 사망: 아바타 비활성화·머리 생성 | 0.5 | `PlayerAvatar.cs:191` |
| 사망 이펙트 피해 콜라이더 | 0.5 | `PlayerDeathEffects.cs:31` |
| 사망 카메라(탑다운) | 4 (아레나 1.5) | `SpectateCamera.cs:171-177` |
| 사망 카메라 연장(부활 가능 시) | 0.25씩 갱신 | `SpectateCamera.cs:754` |
| 머리 시점 진입 대기(Normal 진입 후) | 0.5 | `SpectateCamera.cs:308` |
| 머리가 레벨 밖에 5초 → 안전 지점 | 5 | `PlayerDeathHead.cs:753` |
| 트럭에서 머리 자동 부활 대기 | 2 | `PlayerDeathHead.cs:827` |
| 부활 후 무적 | 1 | `PlayerAvatar.cs:1537` |
| 전멸 → 아레나 호출까지(마스터 마지막 사망 기준) | ≈4.5 | 7.1절 |
| 아레나: Falling / Starting | 5 / 2 | `Arena.cs:276,345` |
| 아레나: 레벨 / 경고 / 제거 | 30 / 3 / 3 | `Arena.cs:420,370,407` |
| 아레나: 게임오버(멀티 / 싱글) | 10 / 3 | `Arena.cs:463,467` |
| 아레나: 시작 무적 | 5 | `Arena.cs:287` |
| 아레나 레이스: 우승 / 전원 패배 | 10 / 5 | `ArenaRace.cs:637,664` |
| 결과 화면(토큰): FadeIn / CloseDoor / MiddleFade / FadeOut | 0.2 / 0.5 / 1 / 1 | `ResultScreenUI.cs:193,208,240,287` |
| 채팅 연속 전송 제한 | 1 | `ChatManager.cs:856` |
| 낙오 자기 파괴 지연(마지막 문구 뒤) | 0.2~3 무작위 | `ChatManager.cs:568` |
| 네트워크 송신·직렬화 | 25 Hz | `NetworkManager.cs:50-51` |

## 10. 흐름이 "좋게 느껴지게" 하는 세부 장치

### 10.1 메뉴

1. **모든 것에 소리와 몸짓이 있다.** 호버 소리, 클릭 5종 효과음과 선택 상자 색 플래시, 페이지 열기·닫기 소리, 팝업 열기·닫기 소리가 따로 있다(3.1절). 오류 입력은 항상 "빨강 플래시 + 가로 흔들림 + Deny 효과음" 세트, 정상 입력은 "노랑 플래시 + 세로 튕김 + 높은 Tick" 세트다(`MenuPagePassword.cs:84-107`, `ChatManager.cs:368-433`).
2. **목록은 한꺼번에 뜨지 않고 한 줄씩 차오른다.** 줄마다 음높이가 올라가는 효과음이 나서 "로딩 중"이 아니라 "일어나고 있는 일"처럼 읽힌다(지역 0.15초, 서버 0.1초, 페이지 넘김 0.05초).
3. **지역 핑 색 신호등**과 "최적 자동 선택" 항목이 맨 위에 있다. 사용자가 판단하지 않아도 되는 기본값이 가장 위에 있다.
4. **오류 팝업을 도착한 뒤에 보여 준다.** 예약 후 메인 메뉴 도착 1.5초 뒤에 뜬다(3.2절). 화면이 바뀌는 중에 읽히지 못하는 문구를 없앤다.
5. **첫 실행 안내는 튜토리얼 버튼 하나를 깜빡이는 것**으로 끝난다. 모달이나 강제 튜토리얼이 없다.
6. **새 꾸미기를 얻으면 꾸미기 버튼이 주황 배지로 알려 준다**(메인·로비 둘 다).
7. **세이브 슬롯은 정보가 풍부하다.** 난이도 색 숫자, 플레이 시간, 화폐, 달 아이콘, 함께한 사람 이름, 손상 시 복원. 슬롯을 지우면 아래 줄이 올라오며 몸을 흔든다.
8. **비밀번호는 건너뛸 수 있고 호스트 로딩 화면 위에서 입력한다.** 대기 시간이 낭비되지 않는다. 입력 도중에 "25초 멈춤" 안내는 꺼진다.

### 10.2 로딩과 레벨 시작

1. **로딩 애니메이션이 씬을 넘어 이어진다.** 로딩 UI의 애니메이션 진행도와 페이드 색을 `RunManager`에 보관했다가 새 씬의 `LoadingUI`가 그 값으로 이어서 재생한다(`LoadingUI.cs:76-80,126-127`). 씬이 두 번(`Main`→`Reload`→`Main`) 바뀌어도 화면이 끊기지 않는다.
2. **레벨 카드.** 원정 레벨은 "레벨 N"과 이름, 상점은 "상점", 아레나는 **빨간 "게임오버"**가 뜨며 로딩 그래픽 3장이 애니메이션되고 트럭 소리(방향 전환, 엔진 가속, 충돌)와 글자 소리가 붙는다(`LoadingUI.cs:162-212,219-241`). 트럭 로비·메뉴·튜토리얼 레벨에는 레벨 카드가 없다(`LoadingUI.cs:165`). 트럭 로비와 메뉴는 카드 대기 없이 0.5초 뒤 시작한다(`GameDirector.cs:180-190`).
3. **전원이 함께 시작한다.** 각 플레이어의 카드 애니메이션이 끝났다는 표시(`levelAnimationCompleted`)를 모두가 가진 뒤 0.5초를 더 기다려 로딩 UI를 걷는다(`LoadingUI.cs:128-145`, `GameDirector.cs:180-190`). 느린 컴퓨터가 늦게 보여 혼자 늦게 시작하는 일이 없다.
4. **시작 직후 팔 인트로**. 0.25초 뒤 장갑을 끼는 1인칭 팔 애니메이션과 카메라 충격이 나온다(`ArmIntroController.cs`).
5. **스폰 직후 트럭 화면을 바라보게 한다**(0.3초). 플레이어가 "어디를 봐야 하는지" 알게 되는 순간이다.
6. **트럭 문이 레벨 생성 완료 2초 뒤에 올라가며 카메라 충격**이 있다(`TruckDoor.cs:78-105`).
7. **목표 문구 한 줄(3초)**이 가장 필요한 순간에 뜬다(상점 입장, 트럭 로비 입장, 추출 지점 안내, 튜토리얼 페이지 등 `SemiFunc.UIFocusText`를 부르는 곳이 여러 군데다).
8. **메뉴 레벨에서는 음성 믹서가 달라진다.** 로비에서 모두가 모두의 소리를 들을 수 있다.

### 10.3 로비 메뉴

1. 열릴 때 **2초 뒤 카메라 흔들림 + 페이지 등장**으로 "도착했다"는 신호를 준다.
2. **사람이 들어오면 Start 버튼이 막힌다.** 새로 들어오는 사람의 로딩이 끝날 때까지 시작하지 못하게 해 "나 빼고 시작해 버림"을 구조적으로 없앤다.
3. **얼굴 아이콘이 서로를 쳐다보고, 말하면 입이 움직이며, 말하는 사람 이름이 밝아진다.** 접속한 사람이 "살아 있는 존재"로 느껴진다.
4. **프로필·강퇴·음량 조절이 줄 안에 있다.** 별도 메뉴로 들어가지 않는다.
5. **채팅 = TTS.** 글자를 보내면 합성 음성으로 읽혀 마이크 없이도 말할 수 있고, 말하는 단어마다 줄 옆 글자가 노랗게 번쩍인다. 죽은 사람은 속삭임 음성이다.
6. **왕관**이 이름 옆에 남아 아레나 우승자가 다음 판 로비에서도 보인다(세션 유지, `SemiFunc.cs:2997-3000`).

### 10.4 트럭 화면 "꾹 누르기"

1. 출발이 "메뉴 버튼"이 아니라 **월드 안의 물건을 직접 누르는 행위**다. 호스트 권한 없이 누구나 할 수 있다.
2. **누르는 동안 문장이 한 글자씩 찍히고 충전음이 올라가며 로딩 바가 찬다.** 1.33초가 길지도 짧지도 않아 "실수로 출발"을 막으면서 "답답함"도 줄인다.
3. **성공/실패가 초록·빨강 번쩍과 소리로 즉시 구분된다.**
4. **낙오자 연출이 처벌이 아니라 개그 컷신이다.** 채팅창이 빙의되어 푸념과 카운트다운을 말하며, 도중에 트럭에 타면 "취소됨"이 초록으로 뜬다.

### 10.5 죽음

1. **죽는 순간의 "손맛".** 프리즈 0.2초, 해상도 20% 뭉개짐, 줌, 파편 폭발과 연기·불, 근처 카메라 흔들림. 죽은 자리에서 정말로 "터진다".
2. **죽인 놈을 끝까지 쳐다본다.** 킬 룩앳 지점으로 카메라가 강제 이동한다. 누가 죽였는지 잊히지 않는다.
3. **4초의 탑다운 시점**이 전환 완충제 역할을 한다. 곧바로 관전 화면으로 던지지 않는다.
4. **죽은 사람이 할 일이 있다.** 머리 시점(에너지 자원, 점프 충전 튕김), 채팅(속삭임), 음성 대화, 동료 시점 전환. 머리가 물건이 되어 동료가 들고 와 부활시킨다(repo-run.md 참조).
5. **다음 레벨은 1 HP로 시작한다.** 죽음이 "탈락"이 아니라 "다음 판의 약점"으로 이어진다.
6. **전멸해도 곧바로 메인 메뉴로 쫓겨나지 않는다.** 아레나라는 마지막 놀이와 왕관 보상이 있고, 멀티에서는 같은 방의 로비로 돌아가 파티가 유지된다.
7. **전멸 후 대기가 한 번에 정리된다.** 4.5초 + 검정 컷 + 로딩이 한 호흡이라 결과 점수 화면이 흐름을 끊지 않는다.

## 11. 기타 참고

### 11.1 세션 상태와 RPC 규칙

- 모든 레벨 전환은 마스터가 `ChangeLevel`로 결정하고 `UpdateLevelRPC`(버퍼드)로 나머지에게 알린다(repo-run.md). 비마스터가 `ChangeLevel`을 부를 수 있는 예외는 메뉴 레벨에서 "내가 접속하러 가는" 경우뿐이다(`RunManager.cs:225`).
- 방 나가기·결과 화면 끄기·아웃트로 시작은 로컬에서 처리하고, "전원 아웃트로 완료"는 마스터가 `PlayerList`로 판정한다.

### 11.2 채팅·음성

- 채팅은 멀티에서만 작동한다(`ChatManager.cs:793-801`). 열기 키를 누르면 입력줄이 열리고, 최대 **50자**, 위·아래 화살표로 지난 문장 20개를 불러오며, 확인으로 전송, Back으로 취소, 연속 전송은 1초 제한이다(`ChatManager.cs:351-483,832-857`).
- 전송한 문장은 **TTS로 읽히고** 웅크린 상태이면 속삭임이 된다. 슬래시로 시작하는 문장은 디버그 명령으로 처리를 시도한다(`PlayerAvatar.cs:1660-1686`).
- ESC 메뉴·설정 페이지가 열려 있으면 채팅이 열리지 않는다(`ChatManager.cs:356`).

### 11.3 Steam·리치 프레전스

로비·상점·아레나·원정 레벨마다 Steam 리치 프레전스가 "메뉴 / 게임 중 / 트럭 / 상점 / 아레나"와 "레벨 N - 이름"으로 갱신된다(설정 값 2일 때만 레벨 정보 노출, 1이면 큰 분류만, 0이면 없음). 로비 멤버가 바뀔 때마다 친구 그룹 표시와 Discord 파티 크기가 갱신된다(`RunManager.cs:580-643`, `SteamManager.cs:101-132,431-446`).

## 12. CL9 설계에 가져갈 수 있는 관찰

(결정이 아니라 관찰이다.)

1. **새 판은 "메뉴 → 로비(파티 모으기) → 첫 레벨"이고, 걸어 다니는 대기 공간은 "상점 다음 원정 전"에만 있다.** 걸어 다니는 대기 공간의 존재 이유는 충전·장비 정리·출발 의식이지 파티 모으기가 아니다. 파티 모으기는 걸을 수 없는 로비 메뉴가 맡는다.
2. **로비 메뉴는 3D 방 + 2D UI**이고 컨트롤러가 꺼져 있다. 구현 비용이 낮고 "접속 중인 사람이 있으면 시작 금지" 한 가지 조건만 필요하다.
3. **출발 장치를 월드 오브젝트(꾹 누르기)로 두면 호스트 전용 버튼을 없앨 수 있다.** 대신 "누가 눌렀는지 채팅 줄에 이름·색으로 남기기", "전원 승차 검사", "낙오자 처리"가 같이 필요하다.
4. **전멸 시 나가는 곳이 점수표가 아니라 놀이 레벨**이다. 비용이 큰 선택이므로 CL9에서는 규모를 정해야 한다.
5. **세이브는 레벨 전환 시점에 쓰고, 전멸 시점에 지운다.** 사용자가 직접 "삭제" 하지 않아도 진행이 끝났음을 구조가 알려 준다.
6. **파티는 한 방·한 Steam 로비로 유지**되고 게임오버에서는 네트워크 오브젝트만 전부 파괴해 로비 메뉴로 돌아온다. UE에서는 같은 세션을 유지한 채 로비 맵으로 서버 이동하는 것과 대응한다.
7. **방 열기·닫기는 로비 메뉴 한 곳에서만 일어난다.** 난입 정책을 정할 때 필요한 상태가 `IsOpen` 하나뿐이다.
8. **레벨 전환마다 중간 씬(`Reload`)을 한 번 거치며 "전원 준비" 장벽을 둔다.** UE의 시임리스 트래블 또는 서버 이동 후 `PostLogin` 대기와 같은 목적이다. 호스트가 영원히 기다리는 위험(마스터 타임아웃 없음)은 CL9가 시간 제한을 두어 피할 수 있다.

## 13. 확인 못 한 것

아래는 디컴파일 코드만으로는 알 수 없어 확정하지 않은 것이다.

1. **메인 메뉴·로비·ESC·설정 버튼의 실제 표시 문구와 배치**. 프리팹·로컬라이제이션 에셋에 있다. 설치본은 `C:/Program Files (x86)/Steam/steamapps/common/REPO`에 있으나, 이 조사 때는 UnityPy가 설치되어 있지 않아 에셋을 열지 않았다.
2. **`Level` 에셋의 실제 값**(로비·상점·아레나의 `HasEnemies`, 모듈 수, 로딩 그래픽, 레벨 이름 목록).
3. **레벨 카드 애니메이터 클립 길이**와 "카드가 끝나기까지 걸리는 초". `LoadingUI.LevelAnimationComplete`는 애니메이션 이벤트로 호출된다.
4. **프리팹 UnityEvent로 연결된 호출**: `TruckScreenText.StartChat`/`ReleaseChat`/`GotoNextLevel`/`ChatMessageLobby`가 어느 입력·어느 페이지 줄에서 불리는지, `Arena.OpenHatch`가 언제 불리는지, `TruckScreenOpen`이 어느 레벨에서 쓰이는지.
5. **`LobbyJoin` 씬의 내용**(`NetworkConnect`가 들어 있는지). `GameManager.maxPlayers`의 프리팹 값.
6. **서버 측 처리**: 비밀번호 대조(Photon 방 속성 `PASSWORD`), 강퇴·밴 이벤트(123/124)의 서버 처리와 199 이벤트 발신. Photon 서버·플러그인 설정은 코드에 없다.
7. **`ReloadScene`/`NetworkManager`가 늦게 들어온 사람에게 주는 버퍼드 RPC의 도달 동작**. Photon의 버퍼드 RPC 의미에 의존하는 추정이다.
8. **세이브 재생성**: 게임오버 뒤 같은 이름의 세이브가 다시 쓰이는지는 코드 읽기만으로 추정했다(7.4절).
9. **낙오 카운트다운의 총 시간**. 각 문구의 "타이핑 속도" 값의 단위가 코드에서 확정되지 않았다.
10. 메뉴 3D 방(메인 메뉴·로비 메뉴)의 연출 요소(트럭 외관 애니메이션 `TruckMenuAnimated`가 8~16초 간격으로 "속도 증가/감속/스워브/해골 충돌/트럭 지나감" 트리거를 무작위 순서로 발동하는 것까지만 확인, `TruckMenuAnimated.cs:52-168`). 이것이 메인 메뉴 배경인지 트럭 로비 배경인지는 확인하지 못했다.

## 14. CL9 에 옮긴 것

이 노트를 근거로 한 CL9 의 결정은 [../../goal/start-lobby.md](../../goal/start-lobby.md)가 정본이다(2026-10-05, 결정 L1~L12). **구현 중이다 — 결과는 그 문서 상단 Status 에 적힌다.** 리썰 쪽 요지는 [../lethal/lethal-frontend.md](../lethal/lethal-frontend.md)의 마지막 절에 있다. R.E.P.O. 와 관련한 것만 적는다.

- **따른 것 — 대기 공간이 월드 안에 있다.** R.E.P.O. 의 걸어 다니는 트럭 로비(5.2절)처럼 CL9 도 스테이지 레벨 안의 Lobby 단계(트럭 마당)를 대기실로 쓴다. 별도 로비 레벨은 두지 않는다(L1). 12절 1)의 "파티 모으기는 로비 메뉴가 맡는다"는 구조는 옮기지 않았다 — CL9 에는 로비 메뉴 레벨이 없고 시작 레벨의 타이틀에서 곧바로 방을 만든다(L2·L5).
- **따른 것 — 출발은 월드 안의 물건이다.** R.E.P.O. 의 트럭 화면 꾹 누르기(5.3절)처럼 CL9 도 월드에 놓인 출발 레버로 시작한다(L6). 다만 꾹 누르기는 옮기지 않고 한 번 눌러 무장(5초) → 다시 눌러 출발의 두 번 누르기이며, 호스트만 출발한다. R.E.P.O. 의 "누구나 출발 가능"과 낙오자 자기 파괴 연출은 가져오지 않았다.
- **게임 오버.** R.E.P.O. 는 전멸하면 아레나를 거쳐 같은 방의 로비 메뉴(싱글은 새 판)로 돌아가고 세이브는 전멸 때 지운다(7.4절). CL9 는 아레나 없이 게임 오버 화면에서 시작 레벨로 돌려보낸다(L7·L8). 세이브는 지우지 않는다 — 시작 화면에서 다시 호스트하면 첫날부터 시작할 뿐이다(Out-of-scope: save deletion on run over). 죽음 연출(프리즈·해상도 뭉개짐 등, 6.1절)과 관전 구조는 이번 범위에 없고, 기존 CL9 관전을 그대로 쓴다.
- **이번에 자른 것.** 접속 승인·최대 인원 제한·명단·알림·실패 처리(L9). R.E.P.O. 의 "접속 중인 사람이 있으면 Start 금지"(4.3절)·강퇴·호스트 이탈 팝업·지역 선택·서버 목록·세이브 슬롯 선택은 옮기지 않았다. 지금 동작(최대 16명·밤 중 합류·호스트 이탈 때 엔진 기본 동작)을 그대로 둔다.
- **전송과 시작 화면.** 호스트는 `OpenLevel`(비 Shipping 빌드에서만 listen), 나가기는 설정의 시작 레벨 줄을 연다(L4). Steam 은 이번 범위 밖이다. 타이틀 화면은 [방 만들기] → 스테이지 목록, [설정], [종료]로 R.E.P.O. 의 메인 메뉴보다 훨씬 단순하다(L5·L10).
