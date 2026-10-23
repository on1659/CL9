# 세션 및 매칭 시스템

> 로비에서 게임 시작까지의 흐름. **2026-10-05(start-lobby)에 개발용 뼈대가 섰다** — 시작 레벨·방 만들기·나가기·주소 접속·
> 출발 레버·판 종료. **2026-10-11(voice-chat)에 Null LAN 세션(방 만들기·`cl9.join lan`)이 더해졌고, 2026-10-12(voice-eos-redpoint)에 그 세션이 Redpoint EOS 로비로 바뀌었다** — 마이크 음성의 전제다([voice.md](voice.md)). 그것이 아래 "지금 지어진 것" 절이다. 그 뒤의 화면 흐름·Steam·준비 상태 절은 **아직 짓지 않은 계획**(사전 문서)이다.
> **2026-10-06(round-start-truck-intro)에 판 흐름이 바뀌었다** — 판은 스테이지가 서자마자 시작하고, 출발 레버는 트럭 안의 ON/OFF 스위치이며,
> 판 끝에 트럭 밖에 있던 사람은 남겨져 죽는다. 아래 그림과 "출발 레버"·"판 끝" 절이 그 뒤의 모습이다
> ([../goal/round-start-truck-intro.md](../goal/round-start-truck-intro.md)). 사용자는 플레이 구간을 "판"이라 부른다(코드 이름 Night·`StartNight` 는 그대로).

## 지금 지어진 것 (2026-10-05, docs/goal/start-lobby.md · 2026-10-06 판 흐름 개편)

```
Lvl_Start (빈 레벨, World Settings 게임 모드 = /Script/CL9.CL9MenuGameMode — 폰 없음, HUD 없음)
  타이틀(뿌리 화면) [방 만들기] → 사이트 목록 → UCL9GameInstance::HostGame(alias) → 사이트 행의 Level 을 리슨으로 연다
스테이지 레벨 — StartPlay: 사이트·시드 → 트럭(SpawnTruck) → 출발 레버(SpawnDepartLever) → OfferOrders(끝에서 자동 확정) → StartNight
  (부지 짓기·내비가 끝날 때까지는 StartNight 가 스스로 미룬다) = 판이 레벨이 서자마자 열린다. 레버 손잡이 내림(ON)
판(Phase Night) … 개인 죽음 → 관전(전과 같다)
  호스트가 트럭 안 레버를 올린다(두 번 누르기) 또는 판 시간이 끝난다 → EndNight → 새벽 → 트럭 밖에 있는 사람은 남겨져 죽는다
  전멸(밤 중 — SpectateDelay 뒤) 또는 전원이 남겨짐(박자 없이 바로) + CL9Rules.bWipeEndsRun → 밤을 걷고 Phase 를 Dawn 에 멈춘 채 GameState.RunOver 복제
  → 게임 오버(뿌리 화면, 모든 머신이 상태에서 파생) → [시작 화면으로]/[나가기] = UCL9GameInstance::LeaveGame() → Lvl_Start
  아니면(셀 false = 의도값, 2026-10-12) 전멸이면 누적 이더 WipePenaltyRate(30%) 차감 → EnterLobby: 죽은 전원 부활 → 정산 → 저장 → 다음 주문 뽑기·자동 확정
대기실(Phase Lobby, 트럭 마당 — 주문·상점·트럭 패드는 전과 같다) 레버 손잡이 올림(OFF)
  호스트가 트럭 안 레버를 내린다(두 번 누르기) → StartNight → 다음 판(같은 사이트, 새 시드)
```

임시 흐름이다 — 트럭 이동 연출·트럭 안 정산·목적지 선택·특정 장소 부활·3판마다 상점은 다음 명세다(범위 밖).

**대기실 = 스테이지 레벨의 Lobby 페이즈다.** 따로 된 로비 레벨도 ServerTravel 도 없다 — 레벨 이동은 `OpenLevel` 한 번(하드 트래블)이고,
`ACL9GameMode : AGameModeBase` 는 그대로다. 스테이지 레벨을 직접 연 PIE(`Lvl_Flat`)도 이제 로비가 아니라 **판에서 시작한다**
(스위치 없는 월드라 전원 단계는 None — 건물 어둠 없이 예전 조명으로 연다).

### 시작 레벨 — `ACL9MenuGameMode` (`Source/CL9/GameMode/CL9MenuGameMode.*`)

- 네이티브·비추상 클래스라 BP 자식이 없다. `Lvl_Start` 의 World Settings 게임 모드 덮어쓰기에 클래스 경로 `/Script/CL9.CL9MenuGameMode` 를 건다.
- `DefaultPawnClass`·`HUDClass` = null, `HandleStartingNewPlayer` 는 아무것도 하지 않는다(폰을 세우지 않는다).
- `InitGame` 이 **전역 기본 게임 모드(`GlobalDefaultGameMode`)의 기본값에서 `PlayerControllerClass` 를 복사**한다 —
  `BP_CL9PlayerController` 를 그대로 써서 UI 부팅·IMC 등록(설정 화면 키 목록)·볼륨 적용이 같은 코드로 돈다. ini 를 직접 읽는다(EngineSettings 모듈 의존 없음).
- `TitleUIRow`(기본 `"Title"`) — 컨트롤러가 이 행을 뿌리 화면으로 연다.
- `ACL9PlayerController::IsInMenuWorld()` = 권한 게임 모드가 `ACL9MenuGameMode` 다(클라이언트에서는 늘 false). `BeginPlay`(로컬, `InitForPlayer` 뒤)가
  메뉴면 `UCL9UIManager::OpenRootScreen(TitleUIRow)`, `DebugPanelInput`(Tab)은 메뉴에서 아무것도 하지 않는다. `PauseInput`(M)은 고치지 않았다 —
  뿌리 화면은 UI 매니저가 닫지 않는다(ui_system.md). 열린 Screen 이 게임플레이 IMC 를 떼므로 메뉴 전용 입력 가드는 따로 없다.

### 세션 API — `UCL9GameInstance` (`UCLASS(abstract, Config=Game)`)

| 함수 | 하는 일 |
|---|---|
| `HostGame(FName SiteAlias)` | 레지스트리의 사이트 행 → `Level` 의 패키지 이름 → `UGameplayStatics::OpenLevel(…, "listen")`. **`listen` 은 Shipping 이 아닐 때만** 붙는다. 행이 없거나 `Level` 이 비면 false + 로그. **Shipping 이 아니면 레벨을 열기 전에 EOS 로비를 세운다**(2026-10-12, voice-eos-redpoint — 2026-10-11 의 Null LAN 세션을 대체): 이미 로비 안이면 먼저 떠나고(`NotifyLobbyLeft` 에서 이어서), `BP_HostLobby(4)` 를 부른다. 블루프린트가 끝나면 `NotifyLobbyReady(bOk)` 를 부르고 **그 안에서(성공이든 실패든)** `OpenLevel(listen)` 을 연다. 로비를 세우지 못해도 레벨은 열린다(음성만 없다). `BP_HostLobby` 가 구현돼 있지 않으면 경고 후 바로 레벨을 연다. 로그 `Voice: creating a lobby for 4 players.` → `Voice: session created GameSession` |
| `LeaveGame()` | 설정 `StartLevel` 로 `OpenLevel`(리슨 없음). 비어 있으면 ini 의 `GameDefaultMap` 으로(경고 한 줄). 호스트·클라이언트가 같은 함수를 부른다 — 리슨 없이 레벨을 여는 호스트는 접속한 클라이언트를 끊고, 끊긴 쪽은 엔진 기본 경로로 `GameDefaultMap` 에 떨어진다. **로비를 떠난다**(`LeaveLobby()` → `BP_LeaveLobby()` → `NotifyLobbyLeft()`, 로그 `Voice: session destroyed GameSession`). 로비를 만드는 중이던 방도 버린다 — 늦게 온 `NotifyLobbyReady(true)` 는 레벨을 열지 않고 그 로비를 도로 떠난다 |
| `JoinGame(const FString& Address)` | Shipping 이 아니면 첫 로컬 컨트롤러가 `ClientTravel(Address, TRAVEL_Absolute)`. Shipping 에서는 false. **주소 `"lan"` 이면 로비 경로**(2026-10-12 — 이름은 그대로, 뜻은 "EOS 로비 찾기"): `BP_FindAndJoinLobby()`(로그 `JoinGame: searching for a lobby.`) → 블루프린트가 끝나면 `NotifyLobbyJoined(bOk, ConnectAddress)` → 성공이면 `Voice: session joined GameSession`, `JoinGame: connecting to <주소> (lobby).` 뒤에 `ClientTravel`. 로비가 0개이거나 실패(bOk=false)면 경고 `JoinGame: no lobby joined — …` 후 끝. `BP_FindAndJoinLobby` 가 구현돼 있지 않으면 경고 후 false. 이미 로비 안이면 먼저 떠나고 이어서 찾는다. **주소 경로는 로비를 거치지 않으므로 이 연결에는 원격 음성이 없다** — 세션이 없을 때 로그 `Voice: no session — remote voice disabled on this connection`(A10), 접속 동작은 전과 같다 |
| `CancelJoin()` | 접속 대기 넷 게임을 버린다(`UEngine::CancelPending(World)`) — 지금 레벨(타이틀)에 남는다. 로비 찾기·참가 중이면 그것도 접는다 — 블루프린트의 비동기 노드는 멈출 수 없으므로 표시만 내리고, 늦게 온 `NotifyLobbyJoined(true)` 는 그 로비를 도로 떠난다. 접속 대기 넷 게임이 있으면 버리고 들어간 로비도 떠난다 |
| `IsJoining()` | `bLobbyJoinPending ∨ GetWorldContext()->PendingNetGame != nullptr` — 로비 찾기·참가 중에도 true 다. 주소 접속은 `JoinGame` 직후 한 틱 동안은 아직 false 다(엔진이 다음 틱에 접속을 시작한다) |
| `IsJoinAvailable()` | Shipping 이 아니면 true |
| `HasVoiceSession()` | 음성 쪽 질의 — managed 음성 채널이 있는 로비에 들어가 있나(`bLobbyJoined`, [voice.md](voice.md)). `IsLocalTalkerRegistered()`·`SyncLocalTalker(bool)` 은 2026-10-12 에 지웠다 |
| `GetHostableSites()` | `Level` 이 있는 사이트 행의 alias, 표 순서. 표시 이름은 그 행의 `DisplayName` |

- **레벨 경로는 C++ 에 없다** — 스테이지는 사이트 행의 `Level`(MapTable), 시작 레벨은 `Config/DefaultGame.ini` 의
  `[/Script/CL9.CL9GameInstance] StartLevel=/Game/Maps/Lvl_Start`.
- 레벨 이동은 `SetClientTravel` 로 예약돼 **다음 엔진 틱**에 일어난다 — 버튼·치트·데미지 호출 스택 안에서 월드가 내려가지 않는다.
- `HostGame`·`LeaveGame` 은 레벨을 열기 전에 `CancelJoin()` 을 부른다 — 로컬 레벨 이동은 대기 중인 주소 접속을 지우지 않아서,
  남겨 두면 붙는 순간 새 방에서 끌려 나가거나 접속 제한 시간 뒤 `GameDefaultMap` 으로 떨어진다.
- **개발용 전송이다.** 리슨 포트는 URL 기본값 7777, 주소 접속은 `ip[:port]`. "IP 직결 조인을 만들지 않는다"(아래 Steam 연동)는 제품의 규칙으로
  그대로다 — 그래서 리슨과 `JoinGame` 은 Shipping 에서 컴파일되지 않는다. **EOS 로비를 세우는 진입부도 같은 `#if !UE_BUILD_SHIPPING` 안이다 — Shipping 에는 Steam 층 전까지 로비도 음성도 없다.**
  *2026-10-12 정정(voice-eos-redpoint):* 세션은 이제 Redpoint EOS 로비가 맡는다 — 2026-10-11 의 Null LAN 세션과 steam-release 가 계획하던 Steam 세션을 둘 다 대체한다. 리슨 연결은 Redpoint 의 P2P 연결 위에 선다(명세 R3, 접속 시간·틱 레이트는 미측정).
  주소 접속(`cl9.join <ip>`)이 Redpoint 설치 뒤에도 IP 연결로 그대로 되는지는 **리드 확인 대기**. Steam 층(steam-release W1/W2)이 들어오면 바뀌는 것은 세션 함수의 속이 아니라 인증이다 —
  "Steam 인증 → EOS Connect 와 AppID 등록"([voice.md](voice.md) "Steam 전환").
- **세션은 음성의 전제다**(2026-10-12, voice-eos-redpoint 결정 1 — 옛 E4·E5 의 엔진 VOIP 전제를 대체): 음성 채널은 EOS 로비의 managed 채널이다. 그래서 방 만들기와 로비 참가(`cl9.join lan`)가 로비를 세우거나 들어간다.
  로비에 들어가면(`OnLobbyEntered`) 오디오 기능 셋을 켜고(로그 `Voice: audio features echo=1 noise=1 agc=1`) `ApplyVoice` 를 다시 부른다. 주소 접속은 이 전제를 못 채운다.
  로비 노드는 블루프린트(`BP_CL9GameInstance`)이고 C++ 는 결정과 상태(`bLobbyJoined`)만 가진다 — 블루프린트가 구현돼 있지 않으면 로비 없이 진행한다(경고 후 바로 레벨을 연다). 이벤트 목록은 [voice.md](voice.md) "블루프린트 브리지".

### 출발 레버 — `ACL9DepartLever` (`Source/CL9/Items/CL9DepartLever.*`)

트럭 안의 **ON/OFF 스위치**다(2026-10-06, round-start-truck-intro K2) — 판 중에는 내려가 있고(ON), 올리면 "집으로"(판 끝)다.
로비에서는 올라가 있고(OFF), 내리면 다음 판이 시작된다.

- `ACL9Fixture` + `ICL9Interactable`. 레벨에 놓지 않는다 — `ACL9GameMode::StartPlay` 가 `SpawnTruck()` 뒤에 `SpawnDepartLever()` 로 하나 스폰한다.
  **자리 — 트럭 공간이 먼저다**: 트럭(`ACL9Truck`)이 있으면 `CL9Rules.DepartLeverOffset` 을 **트럭 액터 트랜스폼** 기준 상대 변환으로 읽는다
  (값 = 아트 매니페스트의 경첩 자리 — 짐칸 앞벽 콘솔 위, 그래서 사람이 걸어 들어와야 닿는다).
  **패드 폴백**: 트럭이 없는 레벨(패드 없음)에서는 옛 해석 — 첫 `ACL9TruckPad` 공간에서 콜리전 바운드의 +X 면 가운데·패드 바닥(액터 원점) 높이.
  패드도 없으면 로그 한 줄만 남기고 세우지 않는다(`cl9.depart`·`cl9.night` 는 그대로 된다).
- **메시** — 받침 `CL9Rules.DepartLeverMesh` + 손잡이 `CL9Rules.DepartLeverHandleMesh`(둘 다 소프트 참조)를 **각 머신이 로컬로** 올린다(GameInstance 가 Init 에서 예열).
  비어 있으면 그 부분이 보이지 않지만 동작한다. 규약(art·gameplay): 받침과 손잡이는 **같은 원점 = 경첩**, +X = 사람 쪽, 손잡이는 기본 수직(+Z).
  스캔 라벨 이름은 `PostInitializeComponents` 가 시설물의 `DisplayName` 에 `LOCTABLE("CL9", "Facility.DepartLever")` 로 넣는다.
  충돌은 쿼리 전용·모든 채널 무시 — 캡슐·내비를 막지 않는다(막는 일은 트럭 본체 메시에 들어간 콘솔이 한다). 포커스는 60×60×80 상자(`FocusBox`)가 받는다(포커스는 오브젝트 타입 오버랩이라 응답과 무관하게 잡힌다).
- **손잡이 자세는 복제하지 않는다** — 모든 머신이 매 틱 복제된 페이즈에서 파생한다: Night = 피치 **−35°**(손잡이 끝이 사람 쪽으로 기움 = 내림·ON), 그 밖(로비·새벽·끝난 판) = **+35°**(벽 쪽 = 올림·OFF).
  틱은 켜 두되 값이 바뀔 때만 컴포넌트를 돌린다(performance.md 틱 규칙). 각도는 명세 고정값이라 표 열이 없다.
- **두 번 누르기**(길게 누르기 없음, **양방향 모두 호스트만** — 사용자가 따로 정하지 않아 대신 정한 기본값, decisions.md 2026-10-06): `CanInteract` = (로비 ∨ 판) ∧ 판이 끝나지 않음(클라이언트도 true — 안내를 받아야 한다).
  `Interact`(호스트, 누른 사람이 호스트 머신의 로컬 컨트롤러일 때만): 로비에서 주문 제시가 남아 있으면 `Client_OpenScreen(OrderSelect)`(주문이 자동 확정이라 거치지 않는 가지) →
  준비 아님이면 `ArmedUntilServerTime = 지금 + DepartArmTime` → 준비 중이면 준비를 풀고 **로비는 `ACL9GameMode::RequestDepart`, 판(Night)은 `EndNight()` 를 바로 부른다(귀가)**.
  준비 상태는 복제된 서버 시각 하나(타이머·OnRep 없음)라 시간이 지나면 저절로 풀린다.
- **페이즈가 바뀌면 준비가 풀린다** — 서버가 `BeginPlay` 에서 `OnNightPhaseChanged` 에 묶여(`EndPlay` 에서 푼다) 페이즈가 바뀔 때마다 `ArmedUntilServerTime = 0` + `ForceNetUpdate`.
  로비에서 건 준비가 판으로, 판에서 건 준비가 새벽으로 넘어가지 않는다.
- 프롬프트(`LOCTABLE("CL9", …)`, 행은 ui):
  - 호스트·로비: `Prompt.DepartOrders` / `Prompt.Depart` / `Prompt.DepartConfirm`.
  - 호스트·판: `Prompt.GoHome`("[{0}] 귀가 준비") → 준비 중이면 `Prompt.GoHomeConfirm`("[{0}] 한 번 더 눌러 귀가 · 트럭 밖 {1}명은 남는다"). `{0}` 은 캐릭터가 키 표기로 채우고,
    `{1}` = 레버가 채우는 **살아 있고 트럭 짐칸 밖인 사람 수**(판 끝 판정과 같은 잣대 — 인원이 바뀔 때만 다시 서식하는 캐시).
  - 클라이언트: 로비에서 준비 중이면 `Prompt.DepartSoon`, **그 밖(판 포함)은 준비 여부와 무관하게 늘 `Prompt.DepartHostOnly`**("호스트만 레버를 움직일 수 있습니다") —
    귀가 준비에 "곧 출발"은 뜻이 맞지 않는다(리드·ui 결정, 2026-10-06).
- **판정은 게임 모드다** — 로비의 출발은 `ECL9DepartResult ACL9GameMode::RequestDepart(ACL9PlayerController*)`: 요청자가 호스트 머신의 로컬 컨트롤러가 아니면 `NotHost`,
  로비가 아니거나 판이 끝났으면 `NotLobby`, 제시된 주문이 남아 있으면 `OrdersPending`, 밤 시작이 이미 대기 중(구역·내비)이면 `AlreadyDeparting`, 아니면 `StartNight` 후 `Ok`.
  `StartNight` 자신은 페이즈만 보므로 "주문을 골랐다"는 레버 쪽의 조건이다(주문이 자동 확정이라 `OrdersPending` 은 이제 거치지 않는 가지다).
  판의 귀가는 `RequestDepart` 가 아니라 레버가 `EndNight` 를 직접 부른다 — 남겨진 사람은 그 안에서 정한다(아래 "판 끝 — 남겨진 사람").
  `cl9.night` 는 그대로 남는다(판정 없이 시작·종료 — 이제 첫 판이 자동으로 열리므로 **로비로 가려고 판을 끝내는 데** 주로 쓴다).

### 판 종료 — `CL9Rules.bWipeEndsRun` (헤더 기본 false)

**2026-10-12(루프 개편 7a, [../goal/loop-7a-wipe-penalty.md](../goal/loop-7a-wipe-penalty.md)): 의도값이 false 로 돌아왔다** — 헤더·셀 모두 false.
전멸은 게임 오버가 아니라 그 판만 끝내고 누적 이더를 깎는다(사용자 2026-10-11 번복, 30% 는 2026-10-12). 아래 "켜져 있으면" 경로는 2026-10-05 규칙을 셀 하나로 되돌리는 스위치로 남는다.

- **꺼져 있으면(의도값)** — 밤 중 전멸 → `HandlePlayerDeath` 안에서 즉시 `EndNight` → 새벽·남겨짐 판정 뒤 **전멸 패널티** → 같은 호출 안에서 다음 날 로비(전원 부활, 아래 "구현됨 — 밤 수명주기").
- **전멸 패널티 — `ACL9GameMode::ApplyWipePenalty()`**: `EndNight` 가 `LeaveBehindOutsideTruck()` 이 false 를 돌려준 뒤 부른다 — 산 사람이 없을 때만
  `Taken = GameState->Deduct(Rules->WipePenaltyFor(SharedMoney))`(floor(잔액 × `CL9Rules.WipePenaltyRate` 0.30), 잔액 하한 0). **기간 수입은 그대로** — 정산 판정에 영향이 없다.
  판 중 전멸과 판 끝에 전원이 남겨진 경우(아래 절)가 같은 자리를 지나 **한 번**만 깎는다. 켜져 있으면 판이 끝나 이 자리까지 오지 않는다.
  로그 `Wipe penalty: 30% of <전> ether — <N> taken. Money <전> -> <후>.` 결과 `FCL9WipePenalty{bActive, Taken}` 를 `ACL9GameState::SetWipePenalty` 로 싣는다
  (복제 `WipePenalty` + `OnWipePenaltyChanged` — 클라이언트는 OnRep, 호스트는 setter 가 직접 발동) → 새벽 화면이 `전멸 — 이더 N 차감` 한 줄([ui.md](ui.md)).
  다음 `StartNight` 가 남겨진 이름과 함께 비운다. `EnterLobby`(새벽 화면·정산·저장)보다 먼저라 깎인 잔액이 그날 세이브에 들어간다.
- **켜져 있으면**: 밤 중 전멸이 `RunOverHandle = SetTimer(FinishRunOver, max(SpectateDelay, 0.1))` 를 걸고 돌아간다(이미 걸려 있으면 그대로) — 데스캠을
  그만큼 보여 주고, 철거는 데미지 호출 스택 밖에서 한다. 박자가 도는 동안 `EndNight` 는 아무것도 하지 않는다(만료 타이머·`cl9.night` 가 와도) —
  대신 막았다는 표시를 남기고, 박자 끝에 판 종료가 취소되면(그 사이 들어온 사람이 살아 있다) `FinishRunOver` 가 그 `EndNight` 를 대신 낸다.
- `FinishRunOver`: 아직 밤이고 산 폰이 없을 때만(아니면 취소) 철거 몸통 **`EndRunAtDawn()`** 을 부른다(2026-10-06 — 판 끝의 남겨진 사람 경로가 같은 몸통을 쓰려고 떼어 냈다).
  `EndRunAtDawn`: 만료·새벽 타이머 해제, `CancelNightWork`, `ClearNightPopulation`, `ClearNightClock`,
  `BestDay = RecordBestDay(CurrentDay)`(이제 기록을 돌려준다), `SetPhase(Dawn)`, `GameState->SetRunOver(Day, BestDay)`.
  **하지 않는 것**: 날짜 진행·주문 만료·장비 분실·맵 재생성·부활·새벽 화면·정산·저장·다음 제시·세이브 삭제.
- **새벽에 멈춘다** — 로비 전용 관문(상점·주문·`StartNight`·레버)이 새 검사 없이 전부 닫히고, 구독자가 이미 다루는 전이라 새 페이즈 값이 없다.
  출구는 레벨을 떠나는 것뿐이다(`LeaveGame`). 다시 방을 만들면 1일차부터 깨끗하게 시작한다(시작 레벨 → `HostGame`).
- `FCL9RunOver { bActive, Day, BestDay }` 를 `ACL9GameState::RunOver` 가 복제한다(OnRep·델리게이트 없음) — `IsRunOver()`·`GetRunOver()`.
  HUD 가 매 틱 읽어 게임 오버 화면을 뿌리 화면으로 파생한다(ui.md). 최고 기록은 호스트에만 있는 기록 파일에서 오므로 함께 싣는다.
- 정산 미달은 바뀌지 않았다 — 지금처럼 같은 레벨에서 1일차로 새 판이 열린다. 자동 저장도 그대로다(판 종료가 세이브를 지우지 않는다).

### 판 끝 — 남겨진 사람 (2026-10-06, round-start-truck-intro K5)

판이 끝나는 길은 둘이고 둘 다 `ACL9GameMode::EndNight` 를 지난다: 호스트가 트럭 안 레버를 올린다(귀가 — 위 "출발 레버") 또는 판 시간이 끝난다(`NightExpiryHandle`).
시간이 끝나면 레버는 저절로 올라간다 — 손잡이 자세가 페이즈에서 파생되기 때문이라 별도 코드가 없다. `cl9.night` 도 같은 길이다.

```
EndNight: 만료 타이머 해제 → CancelNightWork → SetPhase(Dawn) → JudgeImposter()(임포스터가 있던 판만, 7g) → LeaveBehindOutsideTruck()
          → true(전원 남겨져 판 종료)면 여기서 끝 · false 면 ApplyWipePenalty()(산 사람이 없을 때만, 2026-10-12)
          → PublishContributions()(처치 기여 목록, 7h) → GrantRoundWallet()(모두에게 바깥 재화 한 번 — 귀가·임포스터·유도 보상, 7f·7g·7h)
          → DawnLength 뒤 EnterLobby(죽은 전원 부활·정산·저장·다음 주문)
```

- **`LeaveBehindOutsideTruck()`**: 트럭(`ACL9Truck::IsInside`, 짐칸 상자 — [map.md](map.md) "전원 인트로")이 없는 월드(패드 없음)면 아무도 남겨지지 않는다.
  대상 = 살아 있고 폰 자리(캡슐 중심)가 짐칸 상자 밖인 플레이어. **판정 순간의 목록(캐릭터·이름)을 먼저 만든 뒤** 처리한다 — 죽음 처리가 도는 동안 `PlayerArray` 를 훑지 않게.
- **순서 — `SetPhase(Dawn)` 뒤다.** 새벽이 된 뒤에 죽여야 죽음이 `HandlePlayerDeath` 의 밤 전멸 가지(Night 일 때만 — `EndNight` 재진입)를 타지 않는다.
- **`ACL9Character::HostLeaveBehind()`** (호스트 전용, 이미 죽었으면 아무 일도 없다): `HostCancelCarryActions`(수레 놓기·차징 취소) → 손에 든 것(`HeldItem`)과 머리에 쓴 것(`WornItem`)을 `Destroy` →
  퀵슬롯과 `HeadItem` 미러를 비움 → `HostApplyDamage(남은 HP 전부, nullptr, "left behind")`. 짐은 **흘리지 않고 없앤다**(건물에 두고 온 짐은 부지와 함께 사라진다).
  머리 칸을 따로 없애는 이유: 죽음의 흘림(`HostDropEverything`)이 `WornItem` 을 건드리지 않는다. 넘어짐 무적을 거치지 않는다(처형의 `HostKill` 과 같다) — 죽음 경로(데스캠·`HandlePlayerDeath`)는 그대로 탄다.
- **이름·로그**: 로그 `Round end: N player(s) left behind outside the truck — 이름, …`(0명이어도 한 줄). 이름은 `ACL9GameState::SetLeftBehind` 로 싣는다(복제 `LeftBehindNames` +
  `OnLeftBehindChanged` — 클라이언트는 OnRep, 호스트는 setter 가 직접 발동) → 새벽 화면이 한 줄 덧붙인다([ui.md](ui.md)). 목록은 다음 `StartNight` 에서 `ClearLastSettlement` 옆에서 비운다.
- **전원이 남겨졌으면**: 산 사람이 없고 `bWipeEndsRun` 이면 로그 `Round end: nobody made it into the truck — the run is over.` → **박자(`SpectateDelay`) 없이 바로 `EndRunAtDawn()`**(게임 오버).
  밤 중 전멸의 박자는 데스캠을 보여 주고 데미지 호출 스택 밖에서 철거하려는 것이었는데, 여기는 이미 새벽이고 철거도 `EnterLobby` 경로와 같은 스택이라 박자가 필요 없다(결정 2026-10-06).
  꺼져 있으면 `EnterLobby` 가 죽은 전원을 되살린다(옛 전멸과 같다) — 그 전에 `EndNight` 가 전멸 패널티를 한 번 깎는다(2026-10-12, 위 "판 종료").
- **부활한 몸에 피격 표현이 다시 뜨지 않게** — 죽음과 부활이 한 프레임에 오는(`DawnLength` 0) 이 경로 때문에 `HostRevive` 가 넘어짐 시각을 되돌린다([character.md](character.md)).
- 바뀌지 않은 것: 부활 위치(`WakeSpot` — 패드 +X 면), 장비 분실 규칙(`EquipmentKeepRadius`). 트럭이 특정 장소에 닿아야 부활하는 규칙은 다음 명세다.

### 임포스터 (2026-10-12, [../goal/loop-7g-imposter.md](../goal/loop-7g-imposter.md))

- **배정** — `AssignImposter()`: 그 순간 살아 있는 사람 N 명으로 `CL9Rules.ImposterChanceFor(N)`(3인 10%·4인 25%, 1·2인 0)을 굴려
  한 명을 고르게 고른다. `cl9.imposter` 지정이 있으면 그것을 한 판 쓴다. 굴리는 시점은 판의 종류로 갈린다(2026-10-14 사용자 결정 — 아래 "배정 시점"). 역할은 `ACL9PlayerState::bImposter` — **본인 머신에만 복제**(`COND_OwnerOnly`)라
  다른 머신은 새벽 공개 전까지 모른다. 로그 `Imposter: '이름' this round (…)` / `Imposter: none this round (…)` 는 호스트 머신에만 남는다.
- **판정** — `EndNight` 에서 **남겨짐보다 먼저** `JudgeImposter()`. 이긴다 = 임포스터가 살아서 트럭 짐칸 안(트럭 없는 레벨은 안으로 본다), 다른 사람이 한 명 이상이고
  모두 이미 죽었고, 그 누구의 `KilledByPlayer` 도 임포스터가 아니다. 트럭 밖에 산 사람은 이 시점엔 "산 사람"이라 남겨짐 죽음은 승리가 되지 않는다(2026-10-12 사용자).
  죽은 임포스터는 `KilledByPlayer` 가 있으면 **처치**(그 사람이 보상), 없고 그를 죽인 몬스터를 깨운 사람(`InducedByPlayer`, 7h)이 있으면 그 사람의 처치, 둘 다 없으면 실패. 결과는 `ACL9GameState::SetImposterResult`(복제 `FCL9ImposterResult`)로 싣고 역할을 거둔다.
  로그 `Imposter: '이름' won …` / `… was killed by '이름'.` / `… failed — 이유.`
- **보상** — `GrantRoundWallet` 의 한 번 지급에 접는다: 이긴 임포스터는 귀가 보상 대신 `ImposterWinReward`, 임포스터를 죽인 사람은 `ImposterKillReward` 를 더 받는다.
  판의 임포스터는 이겼든 졌든 자기가 깨운 몬스터에게 죽은 사람 하나마다 `ImposterAssistReward` 를 더 받는다(7h).
- **처치 기여 목록**(7h) — `PublishContributions()` 가 판 끝에 죽어 있는 사람 중 `InducedByPlayer` 가 있는 사람마다 한 건(죽은 사람·깨운 사람·몬스터 이름)을
  `ACL9GameState::SetContributions` 로 싣는다(새벽 화면). 판 시작에 비운다.
- **배정 시점** (2026-10-14 사용자 "트럭에서 움직이는 시퀀스에서 임포스터인 거 알려주는 거지"):
  - **레버로 연 판**(도착 없음) — 지금처럼 `StartNight` 의 스폰 뒤 바로 `AssignImposter()`.
  - **도착 판**(레벨의 첫 판 — 세션 시작·회사에서 온 판) — 레벨이 서자마자 판이 열리고(`StartPlay` → `StartNight`) 클라이언트는 그 뒤에 다시 접속하므로,
    `StartNight` 는 굴림을 보류한다(`bImposterRollPending`, 0.25 s 반복 타이머 `ImposterRollHandle` → `TryRollImposter()`, 로그 `Imposter roll: pending until …`).
    굴리는 조건은 **도착이 끝났고**(`GetArrivalAlpha()` ≥ 1, 음수 = 도착 없음) **그리고** 살아 있는 폰을 가진 사람이 `ExpectedPlayers` 이상이거나 도착이 끝난 뒤
    `CL9Rules.ImposterRollGrace`(초안 30 s)가 지났을 때다(판정은 `UCL9RulesDef::IsImposterRollDue`). 굴리면 타이머를 풀고 로그 `Imposter roll: arrival done, 2/2 player(s) in after 1.3s.` 또는
    `Imposter roll: grace 30s over with 1/2 — rolling anyway.` 를 남긴 뒤 `AssignImposter()`.
  - **`ExpectedPlayers`** — `StartPlay` 에서 정한다: 트래블이 들고 온 인원(`TravelForRun` 이 `UCL9GameInstance::SetRunCarry(FromSite, GetNumPlayers())` 로 싣고
    새 레벨이 `GetRunCarryPlayers()` 로 읽는다), 없으면(세션 시작) 그 순간의 `GetNumPlayers()` — 호스트 1 이라 세션 시작 판은 도착이 끝나면 바로 굴린다(그때 붙어 있는 사람이 후보).
  - 보류 중에 판이 끝나면(귀가·만료·`cl9.night`·전멸·판 종료) `CancelNightWork` 가 타이머와 보류를 걷고 **굴리지 않는다**.
  - 알림(`Hud.ImposterAssigned`)은 바뀌지 않았다 — 역할 플래그가 켜질 때 뜨므로 굴림 시점을 따라온다. 도착 판에서는 7j 주문서 카드와 같은 순간이다.
- **알 수 있는 한계** — 출발 레버는 호스트 전용이라 호스트가 죽은 판에서 클라이언트 임포스터는 판 시계가 끝날 때 떠난다.
  상한(`ImposterRollGrace`)이 지난 뒤에 붙는 사람은 그 판의 후보가 아니다. 모든 판에 트럭 이동 연출을 넣는 것은 다음 명세다([../goal/loop-rework.md](../goal/loop-rework.md)).

### 회사 — 레벨을 건너는 판 (2026-10-12, [../goal/loop-7i-company.md](../goal/loop-7i-company.md))

- **언제**: 정산일(`IsSettlementDay(일차)`) 로비에서 출발 레버(`RequestDepart`, `cl9.depart`)가 판 대신 **회사로 간다**(`IsCompanyDue`). 새벽 화면이 "오늘은 정산일 — 출발 레버를 당기면 회사로" 줄을 보인다.
- **이동**: `ACL9GameMode::TravelForRun` — `SaveRun(1)` → 게임 인스턴스 `SetRunCarry(떠나는 사이트, 인원)` → **다음 틱에** 비심리스 `ServerTravel(레벨?listen)`(상대 이동).
  클라이언트는 `ClientTravel` 로 따라와 다시 접속한다(PIE 는 심리스가 꺼져 있다 — `net.AllowPIESeamlessTravel`). 다음 틱인 이유: 레버·치트의 호출 스택 안에서 월드를 내리지 않는다(세션 API 와 같은 규칙).
- **회사 레벨**(`CL9Rules.CompanyLevel` = `Lvl_Company`, 게임 모드는 기본 `BP_CL9GameMode`): `StartPlay` 가 패키지로 알아보고 부지·판 없이 트럭·레버만 세운 뒤 `LoadRun(1, false)` → `Settle()`.
  정산 화면은 각 머신의 HUD 가 복제된 지난 정산에서 파생해 연다(`DeriveCompanyScreen`) — 접속 순간(PostLogin)에는 UI 매니저가 아직 없다.
  패널(`BP_CL9TruckPanel`)에서 장비를 사고, 목적지 판(`ACL9DestinationBoard`)을 호스트가 눌러 다음 사이트를 고른다(처음 값 = 떠나온 사이트). 레버 → 그 사이트의 `Level` 로 같은 이동.
- **도착한 사이트**: `StartPlay` 가 이어 온 판을 **미로보다 먼저** 되돌리고(일차가 부지 크기를 정한다) 주문은 다시 뽑지 않는다(떠나기 전 로비에서 받은 주문을 들고 왔다) → K1 그대로 판이 바로 시작된다.
- **한계**: 트럭 이동 연출 없음(그냥 레벨이 바뀐다). 첫 기간은 1·2판(정산일이 3·6·9일차라 — 7a), 다음부터 세 판. 음성(EOS) 세션이 이동을 건너는지는 둘째 기기가 없어 확인하지 않았다.
- **스테이지 별·클리어 (2026-10-13, [../goal/loop-7l-stage-stars.md](../goal/loop-7l-stage-stars.md))**: 회사의 `Settle()` 이 이 기간의 목적지(떠나온 사이트)에 별을 매기고(`CL9Rules.StageStarsFor`),
  `ACL9GameState::RecordSiteStars` 로 목적지별 최고 별을 남긴다(복제, 판 저장본 v3 로 레벨을 건넌다). 별이 `StageUnlockStars` 이상이면 사이트 표의 다음 목적지가 열리고
  목적지 판은 열린 사이트만 넘긴다. **마지막 목적지면 클리어** — `SetRunCleared` 가 `FCL9RunOver{bCleared, SiteStars}` 를 복제하고, HUD 는 회사 정산 화면을 닫은 뒤
  게임 오버 자리에 클리어 화면을 연다(버튼 = 시작 레벨). 판 종료라 출발 레버는 막힌다.

### 판 시작 연출 (2026-10-12, [../goal/loop-7j-round-start.md](../goal/loop-7j-round-start.md))

- **트럭 도착**: 레벨이 서고 여는 첫 판(세션 시작·회사에서 온 판 — `StartPlay` 가 `bArrivalPending`)만. `StartNight` 가 시계를 연 뒤 `ACL9GameState::SetArrival()` —
  복제 `bArrivalThisRound`·`ArrivalSeq`·`ArrivalServerTime`(= 판 시작). 레벨의 첫 판은 서버 시각이 0 이라 시각만으로는 "없음"과 가를 수 없어 표시·순번을 둔다.
  `GetArrivalAlpha()` 가 0~1(1 이상 = 끝, 없으면 −1). 그 동안 `IsIntroLocked`·`IsIntroInvulnerable` 이 모두에게 참 — 이동·시선·행동이 막히고 맞지 않는다.
  트럭(`ACL9Truck::Tick`)은 **몸체·계단 메시만** 앞쪽에서 후진해 들어온다(감속 1 − (1 − a)³) — 액터·짐칸 상자·레버는 처음부터 제자리라 판정은 움직이는 트럭을 보지 않는다.
  각 로컬 컨트롤러(`TickArrivalCamera`)가 패드 공간 `ArrivalCameraOffset` 에 카메라를 세워 몸체를 보고 캐릭터를 숨긴다(늦게 들어온 사람도 매 틱). 접속 직후 서버가 뷰 타깃을 폰으로 돌려놓으면 다시 잡고,
  다른 것(데스캠·관전)이 가져가면 놓는다. 끝나면 폰으로 0.6초 블렌드.
  도착 판의 임포스터 굴림은 도착이 끝나고 들고 온 인원이 다 붙을 때까지 보류된다(위 "임포스터" 배정 시점, 2026-10-14).
- **레버로 연 판**은 트럭이 이미 서 있어 도착이 없다. **판 시계는 판 시작(= 도착 시작)부터** 흐른다 — 마당에서 머뭇거리는 만큼 손해(시간 압박, 사용자 답).

### 개발 콘솔 (Shipping 제외 — [cheats.md](cheats.md))

`cl9.host <alias>` · `cl9.leave` · `cl9.join <ip[:port]>|lan|cancel` 은 게임 인스턴스를 바로 부르는 로컬 명령이라 시작 레벨에서도 된다(`lan` 은 2026-10-11 — 처음에는 같은 망의 Null LAN 세션을 찾아 붙었고, 2026-10-12 부터는 호스트의 EOS 로비를 찾아 붙는다).
`cl9.depart`(호스트 컨트롤러로 `RequestDepart`, 결과 enum 출력) · `cl9.lever`(레버를 호스트 캐릭터로 한 번 누름 — 로비는 출발, 판은 귀가) · `cl9.power`(전원 스위치를 호스트 캐릭터로 누름)는 호스트 판정 치트다.
`cl9.lever` 는 결과를 `opened-orders|armed|departed|went-home` 으로 찍는다 — 판에서 두 번째 누름이면 `went-home`(귀가 = 판 끝, 남겨짐 판정까지 돈다).
`cl9.truck` 은 로컬 읽기 전용이다.

### 아직 없는 것 (이번에 만들지 않았다)

- 참가 관문(PreLogin)·밤 중 참가 거절 — 지금은 밤 중에도 들어와 PlayerStart 에 선다. **인원 상한 4 는 엔진 기본이 건다**(2026-10-11): `DefaultGame.ini [/Script/Engine.GameSession] MaxPlayers=4` 를
  `AGameSession::ApproveLogin` 이 `AtCapacity` 로 검사한다(엔진 소스 `GameSession.cpp` 231·338~340 — 다섯째 접속은 `Server full.` 로 거절). 거절 실측은 아직 없다. 세션의 `NumPublicConnections` 4 는 광고용이다.
- 슬롯 번호·로스터·준비 상태(아래 PlayerState 절), 캐릭터 선택.
- 접속 실패·호스트 이탈·이동 실패 알림 — 실패하면 엔진 기본 경로로 말없이 `GameDefaultMap` 에 떨어진다. 사용자 지정 접속 제한 시간도 없다(엔진 기본).
- 이탈(Logout) 때의 전멸 재판정 — 마지막 생존자가 나가면 밤은 만료로만 끝난다. (`ACL9GameMode::PostLogin`·`Logout` 오버라이드는 2026-10-11 부터 있다 — 하지만 음성 뮤트 표를 다시 짜고
  나간 사람을 받는 사람으로 둔 핫라인을 끄는 일만 한다. 전멸 재판정은 그 안에 없다.)
- 메뉴(M) 화면의 [시작 화면으로] 버튼, 제자리 재시작(`RestartRun`), 판 종료 사유.
- Steam(OnlineSubsystemSteam)·참가 코드·친구 초대 — 아래 계획 절 그대로.
- 클라이언트가 붙은 상태의 게임 오버·나가기·레버 프롬프트·주소 접속은 2인 세션에서 아직 확인하지 않았다.

## 개요 (계획)

- **상태**: 로고 → 타이틀 → 로비 → 게임 시작
- **호스트/클라**: Steam 리슨 서버 (호스트가 게임 진행)
- **플레이어 관리**: 로비에서 4명까지만 허용
- **준비 상태**: 모든 플레이어가 "준비" 전환 후 게임 시작

## 화면 흐름

### 1. 로고 화면

```
┌─────────────────────────────┐
│  [스튜디오 로고]             │
│  회사 로고 애니메이션        │
│  (3초 후 자동 타이틀로 전환) │
└─────────────────────────────┘
```

### 2. 타이틀 화면

```
┌──────────────────────────┐
│  양계장 습격              │
│  [MAIN TITLE]            │
├──────────────────────────┤
│  [혼자 하기]              │ ← 리슨 서버 1인 (같은 구조 — 게임은 1~4인이 기본 범위)
│  [멀티플레이]            │ ← 온라인 게임
│  [설정]                  │
│  [종료]                  │
└──────────────────────────┘
```

**싱글은 "나중 추가"가 아니다.** 1~4인이 기본 범위(plan/overview.md)라 1인도 같은 리슨 서버
구조로 처음부터 돈다 — 별도의 로컬 모드를 만들지 않는다.

### 3. 게임 모드 선택

```
┌──────────────────────────┐
│  게임 모드 선택           │
├──────────────────────────┤
│  [호스트로 플레이]        │
│   - 새로운 게임 시작      │
│   - 다른 플레이어 기다림   │
│                          │
│  [참가 (코드 입력)]       │
│   - 호스트 코드 입력      │
│   - 다른 게임에 참가      │
│                          │
│  [친구 참가]             │
│   - Steam 친구 목록      │
│   - 친구 게임에 참가      │
│                          │
│  [돌아가기]              │
└──────────────────────────┘
```

### 4. 로비 화면 (호스트)

```
┌────────────────────────────┐
│  로비 - 대기 중             │
│  코드: ABC123              │ ← 참가 코드
├────────────────────────────┤
│                            │
│  플레이어 목록:            │
│  • [호스트] 당신 ✅         │
│  • Player2 ⏳ (준비 안 함)  │
│  • Player3 ✅              │
│  • Player4 ⏳              │
│                            │
│  [클립보드 복사] (코드)    │
│                            │
│  캐릭터 선택: [farmer_a ▾] │ ← 고르는 곳은 로비다 (plan/data/character.md)
│                            │
│  [판]                      │
│   3일차 · 누적 1,240 이더   │ ← 세이브 이어하기 (호스트의 판)
│   [새 판 시작]             │
│                            │
│  [게임 시작] (모두 준비 시) │
│  또는                      │
│  [취소]                    │
│                            │
└────────────────────────────┘
```

### 5. 로비 화면 (클라)

```
┌────────────────────────────┐
│  로비 - 대기 중             │
├────────────────────────────┤
│                            │
│  플레이어 목록:            │
│  • [호스트] Player1        │
│  • [당신] Player2 ✅        │
│  • Player3 ⏳              │
│  • Player4 ✅              │
│                            │
│  캐릭터 선택: [farmer_b ▾] │
│  3일차 · 누적 1,240 이더    │ ← 호스트의 판 정보 표시
│                            │
│  호스트 대기 중...         │
│                            │
│  [준비 완료]               │
│  [준비 취소]               │
│  [나가기]                  │
│                            │
└────────────────────────────┘
```

### 6. 게임 시작

모든 플레이어가 "준비" 상태:

```
호스트 [게임 시작] 클릭
  ↓
호스트: ServerTravel (심리스 트래블 — 전원 진행도 표시·대기 목록은 심리스여야 가능하다.
       비심리스는 연결이 끊겼다 재수립되어 로드 중 상태를 주고받을 수 없다)
  ↓
각 클라: 로딩 화면 표시 (진행도)
  ↓
GameMode: PostLogin/HandleSeamlessTravelPlayer로 전원 도착 확인 (bDelayedStart)
  ↓
GameMode: N일차 밤 시작 (세이브의 현재 일차)
```

## 로딩 화면

```
┌──────────────────────┐
│  게임 로딩 중...      │
├──────────────────────┤
│  ▓▓▓▓▓░░░░ 60%      │
│                      │
│  로딩 중...          │
│  - 맵 로드           │
│  - 에셋 준비         │
│  - 플레이어 생성     │
│                      │
│  대기 중인 플레이어: │
│  Player1, Player3    │
│                      │
└──────────────────────┘
```

## 데이터 구조

### 게임 세션 정보

세션 수준 상태는 **GameState**에, 개인별 상태는 **PlayerState**에 둔다 — 클라는 남의
PlayerController를 볼 수 없으므로, 로비 UI가 전원의 준비 상태를 보여주려면 Replicated
PlayerState여야 한다 (PC 참조를 구조체에 담지 않는다).

**밤 페이즈(게임플레이 낮밤)와 세션 상태(매칭·로딩)는 다른 축이다** — 아래처럼 나눈다.

**구현됨 — 밤 수명주기** (build-order ④, 커밋 dc407d3 · `ACL9GameState`):

```
ECL9NightPhase Phase                     // Lobby / Night / Dawn — 밤 페이즈 (Loading/InGame 아님)
int32 CurrentDay                         // 판 일차
float NightStart/End/BlackoutServerTime  // 밤 시계 — 클라가 서버시간에서 잔여·정전 파생
```

- 페이즈 전이는 호스트 GameMode가 판정: `StartNight`(로비→밤) → `EndNight`(밤→새벽) →
  `EnterLobby`(새벽→로비, 일차+1). 클라는 복제된 Phase에서 화면을 파생한다.
- 표현 훅 `OnNightPhaseChanged`(BlueprintAssignable) + 게터 `GetNightRemaining`·`IsBlackout`·
  `GetPhase`·`GetCurrentDay`. 조명(5c `ACL9NightLighting` — [light.md](light.md))과 HUD가 구독한다.
- 전이 검증 치트: `cl9.night`(로비면 시작, 밤이면 종료). 시계 값은 `CL9Rules`에서 읽는다
  (`NightLength`·`DarkBase`·`DarkAccel`·`DarkMin` — [../plan/data/rules.md](../plan/data/rules.md) 밤).
- 4a(2026-09-05)가 얹은 것: **전멸**(생존자 0, `HandlePlayerDeath`)도 `EndNight`로 들어오고,
  `EnterLobby`는 `ReviveDead()`로 죽은 전원을 트럭 패드 위에 만HP로 세운 뒤 로비 페이즈로
  바꾼다 — [character.md](character.md) 죽음 절. **`CL9Rules.bWipeEndsRun` 이 켜져 있으면** 전멸은 `EndNight` 대신
  판 종료로 간다 — 위 "판 종료" 절.
- 밤 시작의 입구는 둘이다 — **첫 판은 `StartPlay` 가 `StartNight` 를 직접 부르고**(2026-10-06), 이후 판은 **출발 레버**(`RequestDepart`, 위 절)다.
  `cl9.night` 는 판정 없는 대역으로 남는다(로비면 시작, 판이면 끝 — 판을 끝내면 트럭 밖 사람이 남겨진다).
- 2026-10-06(round-start-truck-intro)이 얹은 것: `ACL9GameState` 에 `PowerOnServerTime`·`IntroTargets`(건물 전원 인트로 — 단계·잠금·무적 창이 이 둘과 서버 시각에서 매 틱 파생된다,
  [light.md](light.md) "전원 단계")와 `LeftBehindNames`(위 "판 끝 — 남겨진 사람"). 전원 상태는 `StartNightClock`·`ClearNightClock` 이, 남겨진 이름은 다음 `StartNight` 가 비운다. 전부 복제된 값에서 파생이라 Multicast 가 없다.

**미구현 — 세션/매칭 수명주기**: 위 로비 대기·심리스 트래블·재접속·참가 코드는 Steam 리슨
세션 층을 지을 때(1f 이후) 이 GameState에 더해진다 — `ESessionState`(Lobby/Loading/InGame)·
`SessionCode` 같은 상태. 밤 페이즈의 `Lobby`는 밤 사이의 정차이고, 이 세션 상태의 `Lobby`는
접속 대기 화면이다 (겹치는 이름, 다른 축).

## 세이브 (판)

**진행은 호스트의 것이다. 판 하나가 세이브 하나.** 저장 구조·시점·슬롯·기록(최고 일차)은
[save.md](save.md) — 여기서는 로비가 그 슬롯을 읽고 쓴다는 것만.

- 누가 접속하든 호스트의 같은 판을 이어간다 — 클라 개인 진행이 없다
- 새 판은 정산 실패 시 자동으로 열리고, 로비에서 수동으로도 시작할 수 있다
- 난이도 설정이 없다 — 난이도는 일자 곡선이 만든다 ([balance.md](balance.md))

## Steam 연동

> **2026-10-12 정정 (voice-eos-redpoint):** 세션과 로비는 **Redpoint EOS 로비**가 맡는다. 아래 계획 중 OnlineSubsystemSteam 세션·로비(presence)에 관한 서술은 이 정정이 우선한다.
> Steam 은 인증 층으로 남는다 — Redpoint 가 Steam → EOS Connect 를 이어 주고, 자체 AppID 가 있어야 한다(480 은 EOS 가 받지 않는다). steam-release W1/W2 의 세션 항목은
> "Steam 인증 → EOS Connect 와 AppID 등록"으로 좁아진다([../goal/steam-release.md](../goal/steam-release.md) 정정). 참가 경로(친구 초대·참가 코드)와 `CL9TuningHash` 는 계획 그대로이며 로비 위에서 steam-release 가 다시 읽는다.

- **OnlineSubsystemSteam** + 리슨 서버. 개발 중에는 Steam 테스트 AppID **480**(Spacewar)으로
  돌리고, 출시 준비 때 자체 AppID로 교체한다. **480은 전 세계 Spacewar 세션과 공용**이라
  세션 검색에 게임 식별 키(커스텀 속성) 필터가 필수다.
- **연결은 Steam P2P 릴레이** — 포트포워딩·NAT 설정 불필요. IP 직결 조인을 만들지 않는다.
  (SteamSockets 플러그인과 구세대 OSS Steam 넷드라이버는 별개 구성 — 조합은 1f에서 검증.)
- **세션은 로비(presence) 경로로** — 참가 코드 검색은 로비 데이터 필터로만 성립한다
  (`bUsesPresence` 전제).
- **참가 경로 두 개:**
  1. **친구 초대** — Steam 오버레이 초대 → 수락 시 조인 (표준 흐름)
  2. **참가 코드** — 세션 커스텀 속성(게임 식별자 + 코드)으로 등록, 코드로 검색 → 조인 (초안)
- 랜덤 매칭·공개 서버 목록은 만들지 않는다 — 아는 사람과 하는 협동 게임이다.
- 접속 시 `CL9TuningHash` 대조 — 표를 고친 클라는 접속이 막힌다 (plan/build-order.md). **설계만, 미구현** —
  소스에 심볼이 없다(2026-09-21 감사).
- **미결**: 코드 길이·검색 필터·넷드라이버 구성은 1f(2인 접속) 스텝에서 실측 확정.

## 중간 참가 (밤 진행 중 접속)

- 허용 여부 자체가 **미결** — 허용한다면: 스폰은 트럭 존, 게임 상태는 GameState 복제로 자동
  복원, 화면 상태(`UCL9UIManager::OpenScreenName`, `FName`)는 Multicast를 놓치므로 **GameState의 현재 페이즈에서 파생**해
  로컬로 맞춘다 (ui_system.md).
- 연출(시퀀스)은 재생하지 않는다 — 트리거가 Multicast라 놓친 신호는 오지 않는 게 정상이다
  (sequence.md).

### 플레이어 준비 상태 (PlayerState)

```
ACL9PlayerState (Replicated):
  bool bIsReady                 // 준비 상태 — 로비 UI가 이걸 읽는다
  FName SelectedCharacter       // 캐릭터 선택 (Alias)
  int32 SlotIndex               // 슬롯 0~3 — 신원·TintColor의 키
  ELoadingState LoadingState    // 심리스 트래블 도착 여부
```

**슬롯 배정은 입장 순서다** (2026-09-02 확정) — 호스트가 0, 이후 접속 순으로 1·2·3.
이탈하면 그 슬롯은 비고, 새 입장자는 빈 슬롯 중 가장 낮은 번호를 받는다 (초안).
슬롯 색(`TintColor`)·발각 상태·이름표가 전부 이 번호에 물린다 (plan/data/character.md).

## 세션 생명주기

### 호스트 흐름

1. **생성**: 타이틀 → 호스트 선택 → 세션 생성
2. **대기**: 로비 화면, 다른 플레이어 기다림
3. **준비**: 모든 플레이어 준비 확인
4. **시작**: 게임 시작 신호 전송
5. **진행**: 밤 진행 중 호스트 권위 유지
6. **종료**: 게임 종료 → 세션 파괴

### 클라 흐름

1. **참가**: 타이틀 → 코드 입력 → 호스트 연결
2. **로비**: 로비 화면, 준비 상태 설정
3. **준비**: 호스트가 시작하기 전까지 대기
4. **로드**: 맵 로드 시작, 호스트 기다림
5. **시작**: 호스트 신호 후 게임 시작
6. **진행**: 호스트의 명령에 따라 게임 진행
7. **종료**: 연결 해제 → 타이틀로

## 에러 처리

### 호스트가 오프라인

```
클라: "호스트와의 연결이 끊어졌습니다"
  → 타이틀로 복귀 (리슨 서버라 호스트 이탈 = 세션 소멸 — 복귀할 로비도, 재연결 대상도 없다)

호스트 크래시: 다음 실행 시 판은 **마지막 새벽 세이브**에서 이어진다 — 진행 중이던 밤은
소실된다 (저장 단위가 "날"이라서 — save.md)
```

### 참가 코드 오류

```
"잘못된 코드입니다"
  → 다시 입력
  → 또는 코드 확인 요청
```

### 로딩 시간 초과

```
30초 이상 로드 안 됨
  → "연결 문제" 표시
  → 자동 재시도
  → 실패 시 게임 종료
```

## 향후 구현 예정

- **복구**: 게임 도중 연결 끊김 시 재연결 (클라 이탈 — 호스트 이탈은 세션 소멸)

## 참고

- 현재: Steam 리슨 서버 (호스트 권위)
- 입장 시 PlayerController 생성 (conventions.md § 4 참고)
- GameMode는 호스트에만 존재 (conventions.md § 2 참고)
