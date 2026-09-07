<!-- 한글 번역판 — 정본은 같은 폴더의 steam-release.md (영어). 두 파일이 다르면 영어본이 우선한다. -->
# goal: steam-release (한글 번역판)

상태: 초안 (/autoplan 리뷰 진행 중)
날짜: 2026-09-06 · 브랜치: master

## 한 줄 목표
"양계장 습격"(CL9)을 1~4인 리슨 서버 협동 게임으로 Steam에 올린다. Steamworks 계정과
AppID, Steam 오버레이가 뜨고 두 PC가 Steam 중계망으로 연결되도록 하는 엔진 배선,
세션 층(호스트 / 친구 초대 / 참가 코드 / 로비 / 출발), 그리고 빌드 업로드 루프까지 —
**운영할 서버는 없다.**

## 배경 / 동기

사용자가 세 가지를 물었다 (2026-09-06):

1. "스팀에 올릴 건데 뭐 해야 돼? 스팀 오버레이 같은 거 뜨게 해야 되잖아" — Steam에
   올리려면 무엇이 필요한가, 오버레이 포함.
2. "사람들이 서로 어떻게 알고 방에 들어가?" — 플레이어들이 서로를 어떻게 찾고 방에
   들어오는가.
3. "서버 운영을 어떻게 해야 할지 모르겠어" — 서버를 어떻게 운영하는가.

**3번의 답부터: 운영할 서버가 없다.** 프로젝트는 이미 Steam *리슨 서버*로 결정했다
(docs/conventions.md § 2, docs/plan/overview.md). 한 플레이어가 자기 게임 안에서
호스트가 되고, 나머지는 Steam의 중계망(Steam Datagram Relay, 엔진의 SteamSockets
넷드라이버)을 거쳐 그 호스트에 붙는다. 중계망은 Valve가 모든 Steam 게임에 무료로
운영한다. 포트포워딩도, 호스팅 비용도, 전용 서버 실행 파일도, 밤새 살려 둘 것도 없다.
이 모델의 대가는 docs/systems/session.md에 이미 받아들여져 있다: 호스트가 나가면 세션이
끝난다, 호스트 이주는 없다.

**2번의 답:** Steam 프레즌스. 호스트가 Steam *로비 세션*을 만들면 친구들은 Steam 친구
목록/오버레이에서 호스트 이름 옆의 "게임 참가"를 보거나, 호스트가 오버레이로 초대를
보낸다. 이 게임은 "아는 사람과 하는 게임"이라(session.md: 랜덤 매칭 없음, 공개 서버
목록 없음) 두 번째 경로는 호스트가 불러 주는 6자 **참가 코드**다. 클라이언트가 코드를
입력하면 게임이 그 코드로 Steam 로비를 검색한다.

**1번의 답:** Steamworks 서류(계정, 앱 수수료, AppID, 상점 페이지)와 엔진 변경 네 가지
(플러그인 두 개 켜기, Build.cs 모듈 두 개, ini 블록 하나). 오버레이는 Steam 클라이언트가
실행 중일 때 게임이 Steam API를 초기화하는 순간 뜬다 — OnlineSubsystemSteam 플러그인이
그 일을 하고, 오버레이 자체를 위해 더 필요한 것은 없다.

### 검증한 현재 상태 (2026-09-06)

- **프로젝트에 Steam·세션 코드가 0줄이다.** `CL9.uproject`는 Online 플러그인을 하나도
  켜지 않았고, `Config/DefaultEngine.ini`에 `[OnlineSubsystem*]`이나
  `NetDriverDefinitions`가 없으며, `Source/CL9/CL9.Build.cs`에 OnlineSubsystem 모듈이
  없다. `Source/` 아래 어디에도 `IOnlineSession` / `CreateSession` / `ServerTravel` /
  `?listen`이 없고, `CL9TuningHash`는 문서에만 존재한다.
- **세이브 시스템은 동시에 도는 게임플레이 세션에서 작업 중이며 미커밋이다**
  (2026-09-06 22:16 재확인): `Source/CL9/GameMode/CL9SaveGame.h`(미추적)와
  `ACL9GameMode::SaveRun/LoadRun`(`CL9GameMode.cpp:723-780`), 그리고 `EnterLobby`(`:260`)가
  새벽마다 `SaveRun()`을 부른다. 이 계획은 그 슬롯을 읽지도 보여 주지도 않는다(v1
  로스터에는 새 판/이어하기가 없다 — 디자인 결정 30). T9가 그 작업을 W1보다 먼저
  커밋시켜 `CL9GameMode`에서 둘이 충돌하지 않게 한다.
- 지금까지 멀티플레이는 PIE 안에서 클라이언트 2개로만 테스트했다
  (docs/testing/pie-scenarios.md A3). 빌드 오더 1f "2인 리슨 접속"은 그 근거로 닫혔다 —
  별개의 게임 프로세스 둘이 연결된 적은 한 번도 없다.
- 설계는 이미 써 있다: docs/systems/session.md(로고 → 타이틀 → 모드 선택 → 로비; 참가
  경로 두 개: Steam 친구 초대 + 참가 코드; 개발용 AppID 480; 프레즌스 로비 세션; 접속 시
  TuningHash 대조; 호스트 이탈 = 세션 소멸; 밤중 참가는 미결).
- **엔진 5.8 실측** (`D:\Program Files\Epic Games\UE_5.8\Engine`):
  `Plugins/Online/OnlineSubsystemSteam`(v1.2, Steamworks SDK **v1.64**가
  `Source/ThirdParty/Steamworks/Steamv164`에 동봉), `Plugins/Runtime/Steam/SteamSockets`
  (SDR 넷드라이버)와 `SteamShared` 존재. `Plugins/Online/SocketSubsystemSteamIP`는 IP
  전용("NAT 펀치스루를 쓰지 않음")이라 우리가 원하는 것이 아니다.
  `Config/BaseEngine.ini` 기본값: `[OnlineSubsystemSteam] bEnabled=false,
  SteamDevAppId=0, bInitServerOnClient=false, bAllowP2PPacketRelay=true,
  bVACEnabled=1`. `[/Script/SteamSockets.SteamSocketsNetDriver]` 섹션은 합리적인
  타임아웃과 함께 이미 존재한다.
- `ACL9GameState`는 이미 밤 페이즈 / 일차 / 맵 시드 / 주문 / 돈을 복제하고,
  `ACL9PlayerState`는 HP와 소지 슬롯을 복제한다. 둘 다 session.md가 계획한 세션 필드
  (`bIsReady`, `SlotIndex`, `SelectedCharacter`, 세션 상태, 세션 코드)는 없다.
- `ACL9GameMode::StartNight()`가 이미 "게임 시작"이다(`cl9.night` 치트가 부른다).
  `EnterLobby()`가 밤 사이의 정차를 맡는다. 밤 수명주기 전체가 **레벨 하나** 안에 있다 —
  밤마다 레벨을 옮기는 트래블은 없다.

## 범위 안

작업 항목 W0~W5. W0는 서류 작업이라 첫날부터 병행하고, W1 → W2 → W3 → W4 → W5는
순차다. W4는 W0의 AppID가 필요하다.

### W0 — Steamworks 등록 (코드 없음; 지금 시작 — 대기 시간이 전체 일정의 병목이다)
- **지금:** 미결 사항 1~2에 답하고, Steamworks 파트너로 등록하고(신원·세금·은행 서류 —
  세금 검증은 며칠 걸린다; 앱당 수수료 100달러, 매출 1,000달러 뒤 환급), 앱을 만들어
  **자체 AppID**를 받는다. 수수료를 내는 순간 Valve의 의무 30일 대기가 시작된다.
- 검증한 출시 순서(Steamworks 문서, 2026-09-06 확인 — W0 시작 때
  partner.steamgames.com에서 재확인): 수수료 → 30일 대기 → 상점 페이지 심사(영업일
  3~5일) → "Coming Soon" 페이지 공개 14일 이상 → 빌드 심사(영업일 3~5일) → 출시.
  수수료 뒤 최소 약 7주.
- **나중에, 별도 항목 (CEO 음성 지적 4):** Coming Soon 상점 페이지. 캡슐 이미지·
  스크린샷·설명이 필요한데, 이 계획이 제외한 에셋이고 ⑤(아트)가 아직 만들지 않았다.
  W1~W4에는 필요 없다: 베타 브랜치 업로드와 테스트 키는 미출시 앱에서도 동작한다.
  Steam Playtest를 페이지 공개 전에 붙일 수 있는지는 **미검증** — W4에서 Steamworks
  화면을 앞에 두고 Playtest와 테스트 키 중 고른다.
- 코드가 의존하는 Steamworks 설정: Installation → 실행 옵션(Win64 exe), Application →
  멀티플레이/협동 카테고리 플래그, 기본 네트워킹. Steam Datagram Relay는 Steamworks
  문서상 파트너에게 무료로 제공된다(W0에서 재확인).
- 한국 쪽 선행 조건(사업자등록·게임제작업 등록, GRAC 등급분류 vs Steam의 IARC 경로) —
  **미결 사항 1**; 확인할 것, 추측하지 말 것.

### W1 — 엔진 배선: 오버레이가 뜨고 두 PC가 연결된다 (1f가 원래 뜻했던 것)
- `CL9.uproject`: `OnlineSubsystemSteam`과 `SteamSockets`를 켠다(이들이
  `OnlineSubsystem`, `OnlineSubsystemUtils`, `SteamShared`를 끌어온다).
- `Source/CL9/CL9.Build.cs`: `OnlineSubsystem`, `OnlineSubsystemUtils`(public) 추가.
  게임 코드는 `IOnlineSession` / `IOnlineIdentity` 인터페이스만 상대한다 — `Source/CL9`에
  `steam_api.h`는 없다.
- `Config/DefaultEngine.ini`:
  ```ini
  [/Script/Engine.GameEngine]
  !NetDriverDefinitions=ClearArray
  +NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SteamSockets.SteamSocketsNetDriver",DriverClassNameFallback="/Script/OnlineSubsystemUtils.IpNetDriver")

  [OnlineSubsystem]
  DefaultPlatformService=Steam

  [OnlineSubsystemSteam]
  bEnabled=true
  ; Spacewar 테스트 AppID — W4에서 자체 AppID로 교체
  SteamDevAppId=480
  ; 엔진 기본값; SteamSockets 리슨 호스트에는 필요 없다 (아래 참조)
  bInitServerOnClient=false
  ```
  (주석은 반드시 별도 줄에 — UE의 ini 파서는 줄 머리의 `;`만 주석으로 본다. `true ; 설명`은
  값으로 파싱된다.)
  `[/Script/SteamSockets.SteamSocketsNetDriver]` 블록은 엔진 기본값 그대로 둔다. **PIE가
  계속 동작하는 이유(AC7), 5.8 소스로 검증:** OSS Steam은 `-game`/전용 서버로 실행되지
  않는 한 에디터에서 스스로 꺼진다(`OnlineSubsystemSteam.cpp:481-489`, `UE_EDITOR` 블록).
  SteamSockets 모듈은 "SteamSockets: Disabled for editor process."를 찍고 소켓
  서브시스템을 켜지 않는다(`SteamSocketsModule.cpp:16-21, 55-56`). 그리고
  `USteamSocketsNetDriver::IsAvailable()`(`SteamSocketsNetDriver.cpp:41-50`)이 false를
  돌려주므로 엔진은 `DriverClassNameFallback` = IpNetDriver를 만든다. 정의를
  `[/Script/Engine.GameEngine]`(게임 실행만 읽고 `UEditorEngine`은 읽지 않는 섹션)에 두는
  것은 그 위에 얹는 이중 안전장치다. **W1의 첫 확인**: ini 변경 뒤 PIE 2클라이언트, 그리고
  위 로그 줄(T14).
  5.8 플러그인 소스로 검증(2026-09-06, 스펙 리뷰가 첫 판정을 뒤집은 뒤 두 번째 확인):
  `SteamSocketsSubsystem.cpp:107-139`는 Steam *서버* 핸들과 `OnServerLoginComplete`
  델리게이트를 `if (bIsDedicated)`일 때만 만든다. 클라이언트 프로세스에서
  `IsLoggedInToSteam()`(`:809-813`)은 `SteamUser()->BLoggedOn()`을 돌려주고,
  `SteamSocketsNetDriver.cpp:84-92`는 그것이 false일 때만 지연 리스너 경로를 탄다. 따라서
  Steam 클라이언트에 로그인된 리슨 호스트는 P2P 리슨 소켓을 즉시 열며,
  `bInitServerOnClient`는 `false`로 둔다. 이 값이 의미를 갖는 것은 W1의 중단 기준이
  발동해 구세대 `OnlineSubsystemSteam.SteamNetDriver`를 대신 쓸 때뿐이다.
  `bUseSteamNetworking`(기본 true, Epic 문서)은 SteamSockets를 기본 소켓 서브시스템으로
  만든다; 그대로 둔다.
- **알려진 엔진 버그 UE-239936** (이전 드라이버의 소켓 정보가 아직 등록된 상태에서
  *비심리스 서버 트래블* 중 SteamSockets가 리슨 소켓을 만들지 못함; 커뮤니티 수정은
  5.4에서 확인됐고, 5.8에도 `CleanSocketInformation`을 부르는 월드 정리 훅은 여전히
  없다 — 주기적 정리와 개발 전용 콘솔 명령뿐). 우리 흐름에는 세션 중 레벨 트래블이
  없지만, 호스트의 나가기(`OpenLevel(Lvl_Title)`, 리슨 드라이버가 살아 있는 상태)에 이어
  방 만들기를 하면 버그가 말하는 "옛 드라이버 종료 → 새 리슨 소켓" 순서와 같아진다.
  주기적 정리(`SteamSocketsSubsystem.cpp:507`)가 타이틀에 머무는 몇 초 사이에 돌아
  즉각적인 `ServerTravel`보다 위험이 낮지만 0은 아니다 — **AC2를 한 프로세스에서 두 번
  반복해야 한다**(호스트 → 나가기 → 다시 호스트, T7). 재현되면 게임 쪽 우회는 재호스트
  전에 타이틀에서 월드 틱을 몇 번 기다리는 것이다. 런처 빌드에서는 엔진 패치가
  선택지가 아니다.
- `steam_appid.txt`(480)는 비Shipping 빌드에서 플러그인이 써 준다. Shipping 빌드는
  Steam 클라이언트로 실행한다. `-nosteam`은 패키지 빌드의 폴백 실행을 위해 런타임에
  Steam OSS를 끈다. W5가 PIE 동작과 `-nosteam`을 pie-scenarios.md에 문서화한다(오늘은
  둘 다 문서에 없다).
- **진입 조건 (CEO 음성 지적 1):** *다른* Steam 계정이 로그인된 두 번째 PC는 W1의
  선행 조건이지 미결 사항이 아니다. 1f는 PlayerStart 배치 커밋(`058563a`)과 PIE로
  닫혔다. W1은 한 대의 기계로는 끝나지 않는다.
- **시간 상자 + 중단 기준:** SteamSockets 리슨 호스트로 두 PC를 두 작업 세션 안에
  연결하지 못하면 구세대 `/Script/OnlineSubsystemSteam.SteamNetDriver`(Steam P2P
  네트워킹, 역시 Steam이 중계, `BaseEngine.ini`에 섹션이 이미 있음)로
  `bInitServerOnClient=true`와 함께 후퇴한다. session.md가 이 대안을 이미 언급한다.
  W2~W5는 어느 드라이버가 이겼는지에 의존하지 않는다.
- 완료 조건: Steam이 실행 중인 상태에서 띄운 패키지 Development 빌드에 Shift+Tab
  오버레이가 뜨고, 두 번째 PC(두 번째 Steam 계정)가 포트포워딩 없이 SteamSockets로
  호스트의 트럭 패드에 도착한다 — **한 프로세스에서 두 번**(호스트 → 나가기 → 다시
  호스트, UE-239936 확인).

### W2 — 세션 층 (C++): 호스트 / 초대 / 참가 코드 / 나가기
- 새 `UCL9SessionSubsystem : UGameInstanceSubsystem`
  (`Source/CL9/Session/CL9SessionSubsystem.{h,cpp}`)이 `IOnlineSession`을 감싼다:
  `HostSession()`, `JoinByCode(FString)`, `LeaveSession()`, 그리고 UI용 델리게이트
  (`OnSessionCreated`, `OnJoinFailed(reason)`, `OnSessionEnded`).
  **가용성 판정** (엔지니어링 음성 지적 1): `IsSteamAvailable()` =
  `IOnlineSubsystem::Get(STEAM_SUBSYSTEM) != nullptr && ->IsEnabled()`. 절대로
  `IOnlineSubsystem::Get()`의 null 여부로 판단하지 말 것 — Steam이 실패하면 엔진은
  **Null** OSS로 폴백하므로(`OnlineSubsystemModule.cpp:216-219`, 플러그인
  `EnabledByDefault: true`) 맨 `Get()`은 PIE·`-nosteam`·Steam 꺼짐 상태에서도 살아 있고,
  조용히 LAN 세션을 만들어 버린다. AC7은 PIE와 `-nosteam`이 세션을 **만들지 않음**을
  단언한다(로그 줄 "session: Steam unavailable — skipping").
- 세션 설정 — 정확한 튜플로 적는다. OSS Steam이 여기서 Steam 로비 타입을 유도하기
  때문이다(`OnlineSessionAsyncLobbySteam.cpp:69-91 BuildLobbyType`):
  - **방 만들기 (멀티플레이 호스트):** `NumPublicConnections=4`,
    `bUsesPresence=true`, `bUseLobbiesIfAvailable=true`, `bShouldAdvertise=true`,
    `bAllowJoinViaPresence=true`, `bAllowJoinViaPresenceFriendsOnly=false`,
    `bAllowInvites=true`, `bAllowJoinInProgress=true`(페이즈마다 뒤집음, 아래 참조),
    `bIsLANMatch=false` → **Public** 로비: 친구는 "게임 참가"를 보고, 초대가 동작하며,
    친구가 아닌 사람도 코드로 찾을 수 있다.
  - **혼자 하기 (솔로):** 같되 `bShouldAdvertise=false`, `NumPublicConnections=1` →
    **Private** 로비: 아무도 들어올 수 없고 코드도 굴리지 않는다. 코드 경로를 똑같이
    두기 위해서만 존재한다(session.md). GameMode에는 `bSoloRun`(트래블 URL `?solo`에서
    설정)도 생겨 LAN `open <ip>`로 오는 두 번째 접속까지 `PreLogin`이 거부한다. Steam
    OSS가 없으면(위 판정 참조) 솔로는 세션 호출을 아예 건너뛴다. 유닛 테스트가
    `MakeSessionSettings(Solo)`가 정확히 이 튜플을 내는지 단언한다.
  로비에 광고하는 커스텀 설정: `CL9_GAME = "cl9"`(게임 식별 — 전 세계 Spacewar 세션과
  공용인 AppID 480에 있는 동안 필수), `CL9_CODE`, `CL9_HASH`(TuningHash),
  `CL9_VER`(빌드 버전). OSS Steam은 쿼리 설정을 문자열 필터
  (`AddRequestLobbyListStringFilter`, `OnlineSessionAsyncLobbySteam.cpp:838-902`)로
  결과 수 필터와 함께 Steam의 서버 측 로비 검색에 넘긴다. Steamworks 문서상 필터는 개수
  상한보다 먼저 적용된다(여기서는 미검증 — 아래의 0/1/여러 개 처리가 반대 경우를
  덮는다).
- **참가 코드**: 헷갈리지 않는 알파벳(0/O/1/I 제외) 6자, 호스트가 굴리고 로비에 보여
  주며 복사할 수 있다. 클라이언트: `FindSessions`에
  `QuerySettings.Set(SEARCH_LOBBIES, true, Equals)` — **이 키가 있어야 OSS Steam이
  로비를 검색한다**(`OnlineSessionInterfaceSteam.cpp:778-789`; 없으면 쿼리가 마스터
  서버 목록으로 가는데 리슨 호스트는 거기 등록되지 않으므로 모든 코드가 "잘못된 코드"가
  된다) — 그리고 `CL9_GAME == cl9 && CL9_CODE == <code>` → 결과 정확히 하나 →
  `JoinSession` → `ClientTravel(접속 문자열)`. 알려진 한계: OSS Steam은
  `k_ELobbyDistanceFilterDefault`를 하드코딩하는데(`OnlineSessionAsyncLobbySteam.cpp:845`)
  Steamworks 문서상 같은/근처 지역 로비만 돌려준다(여기서는 미검증) — 다른 대륙의
  친구는 코드를 못 찾을 수 있지만 초대(로비 id로 참가)는 동작한다. 못 찾음 문자열에 둘째
  줄이 붙는다: "멀리 있는 친구는 Steam 초대로 들어올 수 있습니다". S3가 동작을 기록한다.
- **친구 초대**: 오버레이 초대(`ShowInviteUI`)와 `OnSessionUserInviteAccepted` →
  `JoinSession`. 콜드 스타트(게임이 꺼진 상태에서 Steam "게임 참가"로 실행)는 시작 시
  같은 델리게이트를 탄다. 5.8의 명령줄 경로(`+connect_lobby`)는 W1의 두 PC 테스트에서
  확인한다.
- **`CL9TuningHash`**가 실체가 된다: `UCL9DefRegistry`가 Init 때 로드된 def 행들을 한 번
  해시한다. 호스트는 `CL9_HASH`로 광고하고, 클라이언트는 `JoinSession` 전에 비교하며
  (UX용 소프트 거부 + 메시지) `?hash=` 트래블 옵션으로 넘긴다. 호스트의
  `ACL9GameMode::PreLogin`이 불일치를 거부한다(권위).
- **`PreLogin`의 호스트 게이트** (전부 클라이언트가 보는 `ErrorMessage`를 낸다): 세션
  만원, TuningHash 불일치, 버전 불일치, 솔로 판, 그리고 **페이즈 ≠ Lobby → 거부**(밤중
  참가는 v1에서 *거부*로 결정 — session.md는 미결로 뒀다; 게이트 항목 참조). "만원"은
  `GetNumPlayers()`에 **대기 중인 접속**(`NetDriver->ClientConnections` 중 아직
  `PlayerController`가 없는 것)을 더해 센다. `ACL9GameMode`는 `AGameModeBase`를
  상속하는데 거기에는 `NumTravellingPlayers`가 없고(그 필드는 `AGameMode`의 것,
  `GameMode.h:130`), PreLogin → 맵 로드 사이에 다섯 클라이언트가 전부 `NumPlayers < 4`를
  통과할 수 있기 때문이다. `PostLogin`이 페이즈와 함께 인원도 재확인해 다섯 번째
  도착자를 만원 문자열과 함께 돌려보낸다.
- `Logout`이 슬롯을 비운다. 호스트 종료 → `DestroySession`. 클라이언트는
  `NetworkFailure` / `TravelFailure`(`UCL9GameInstance`의 핸들러)를 받으면 **먼저
  `LeaveSession()`을 부르고** 타이틀로 가서 "호스트와의 연결이 끊어졌습니다"를 보여 준다
  (session.md 에러 처리). 이 leave가 중요하다: OSS Steam은 명시적 destroy
  (`OnlineSessionInterfaceSteam.cpp:576-639`, `:618`의 `RemoveNamedSession`)나 실패한
  create/join 정리(`OnlineSessionAsyncLobbySteam.cpp:562,575,787`)에서만 이름 있는 세션을
  지운다. 호스트가 사라졌을 때의 로비 채팅 갱신은 멤버 목록만 새로 고치고
  (`OnlineAsyncTaskManagerSteam.cpp:189-211`), Steam은 남은 멤버에게 로비 소유권을
  넘긴다 — leave가 없으면 고아가 된 클라이언트가 참가 가능한 로비를 계속 들고 있고
  친구들은 그 사람 옆에서 여전히 "게임 참가"를 보게 된다(스펙 리뷰 2차 지적 1; S6에서
  검증).
- **버전 게이트**: `Config/DefaultGame.ini`에 `ProjectVersion`을 넣는다
  (`[/Script/EngineSettings.GeneralProjectSettings]`, 오늘은 없음). `CL9_VER`로
  광고하고, `?ver=`로 넘기며, 불일치는 `PreLogin`에서 거부한다.
- **UI 없이 테스트하기 위한 콘솔 치트** (수용한 확장, P2): `cl9.host`,
  `cl9.join <code>`, `cl9.leave`, `cl9.session`(코드·플레이어·페이즈·넷드라이버 덤프)을
  기존 `UCL9CheatManager`로 — W3 화면이 하나도 없어도 W1/W2를 검증할 수 있다(빌드 오더
  원칙 1).
- **로비 참가 가능 여부는 페이즈를 따른다** (스펙 리뷰 지적 3,
  `OnlineSessionAsyncLobbySteam.cpp:447,657`에서 검증: Steam 로비 참가 가능 =
  `bAllowJoinInProgress && members < max`). 호스트가 `IOnlineSession::UpdateSession`을
  `StartNight`에서 `bAllowJoinInProgress=false`로, `EnterLobby`에서 `true`로 부른다 —
  **세션 서브시스템 안에 배선**한다. 서브시스템이(호스팅 중일 때)
  `ACL9GameState::OnNightPhaseChanged`(`CL9GameState.h:157-159`)를 구독하는 식이지,
  GameMode가 Steam을 부르는 식이 아니다(엔지니어링 음성 지적 11: conventions § 2 "Steam은
  전송만 한다" — GameMode에는 온라인 타입이 들어가지 않는다). 그러면 친구들이 밤중에
  전부 실행해서 PreLogin 거부를 맞는 대신 Steam이 로비 입장을 거절한다. Steam
  클라이언트가 "게임 참가" 항목까지 숨기는지는 미검증이므로 같은 `UpdateSession`이
  `CL9_PHASE` 로비 키(`lobby` / `night`)도 쓴다. 클라이언트는 검색 결과/초대에서
  `JoinSession` 전에 이 키를 확인해 밤 문자열을 스스로 보여 준다. 그래도 새어 들어온
  `JoinSession` 거절은 `CL9_PHASE != lobby`면 밤 문자열, 아니면 "연결할 수 없습니다"로
  매핑한다(T4 표). PreLogin 페이즈 게이트가 권위 있는 최후 방어선으로 남는다.
- ~~리치 프레즌스 "N일차 · k/4"~~ — **TODOS로 미룸**: OSS Steam은 상태를
  `steam_display` 키에 강제로 `#` 접두어를 붙여 보낸다
  (`OnlinePresenceInterfaceSteam.cpp:143-151`). 즉 Steamworks에 리치 프레즌스 현지화
  파일을 업로드해야 하는 토큰이라 AppID 480에서는 불가능하다. "게임 참가"는 이것에
  의존하지 않는다 — 프레즌스 로비 자체에서 나온다.

### W3 — 화면: 타이틀 → 호스트 / 참가 → 로비 → 출발
- 작은 `Content/Maps/Lvl_Title.umap`을 새로 만든다(프로젝트 `GameDefaultMap`). 호스트:
  세션 생성 → `OpenLevel(Lvl_Henhouse, listen)`. 클라이언트: 호스트의 게임 레벨로 바로
  들어온다. **레벨 하나, 트래블 하나** — 세션 로비는 트럭 패드 위의 로스터 패널(HUD
  레이어)이지 별도 로비 맵이 아니다. 목적지(빌드 오더 4g)는 같은 레벨 안에서 사이트
  행마다 *생성기*를 고르므로(`ECL9Generator`, "목적지 하나에 생성기 하나") 두 번째
  목적지가 레벨 트래블을 되살리지 않는다. 그래도 `HostSession`은 레벨 이름을 인자로 받아
  서브시스템이 레벨에 묶이지 않게 한다.
- 기존 `UCL9UIManager`(ui_system.md)를 통한 UI 표 행 — **`Lobby` Screen 행도,
  `ModeSelect`도 없다**(디자인 리뷰 결정 2와 7): `Title`(Screen, `Lvl_Title`: 혼자 하기
  / 방 만들기 / 코드로 참가 / 설정 / 종료 + 안내 한 줄 "친구 게임은 Steam 친구 목록의
  '게임 참가'로"), `JoinCode`(Popup: 여섯 칸, [붙여넣기] [참가] [취소]; 내용이 연결 중
  상태로 바뀌었다 돌아오고, 코드 경로의 거절은 **코드를 유지한 채 여기로** 이유와 함께
  돌아온다), `Notice`(Popup, 두 모드: 한 줄 모달 + [확인], 그리고 "복사됨" / "Player2 님이
  들어왔습니다" / "…나갔습니다"용 3초 자동 소멸 토스트, sound.md의 `UI_Confirm` 사용),
  `Roster`(HUD 레이어 행, 아래). 캐릭터 선택기(항목 하나)와 새 판은 v1에서 뺀다 — 선택지
  하나짜리 드롭다운은 소음이다. 나가기는 ui.md가 이미 자리를 잡아 둔 곳에 있다:
  `WBP_Pause` [로비로 나가기] → `LeaveSession` → 타이틀.
- `ACL9PlayerState`: `bIsReady`, `SlotIndex`(입장 순서, 가장 낮은 빈 슬롯) — 복제.
  `bIsReady`는 PlayerController의 `Server_SetReady`가 설정한다. `Server_Depart` →
  `ACL9GameMode::RequestDepart(PC)`(리슨 호스트만, 전원 준비, 대기 중인 주문 제시 없음
  → `StartNight`; 엔지니어링 E3/E4). 슬롯 색은 C++ 배열이 아니라 캐릭터 표에서 읽는다
  (`GetSlotColor()`, 엔지니어링 E5). 두 번째 캐릭터가 생기기 전까지 **`SelectedCharacter`
  필드는 없다**(선택기는 v1에서 뺐다 — 캐릭터 목록과 함께 돌아온다).
- `ACL9GameState`: `FString SessionCode`(복제, `OnRep_SessionCode`). **`ECL9SessionState`는
  없다**(CEO 리뷰 결정 12, 취향 — 게이트 항목): 밤 페이즈가 이미 참가 가능/준비 축을
  나른다. `Lobby` = 참가 가능 + 로스터/준비 표시; `Night`/`Dawn` = 참가 불가. session.md의
  "다른 축" 문단은 심리스 트래블 중의 `Loading` 상태를 위한 것이었는데, 이 설계에는 그
  상태가 없다.
- **로스터 패널은 HUD 레이어에 살고 표시 전용이다**: `WBP_Roster`는 페이즈가 `Lobby`일
  때마다(첫 정차와 이후 모든 정차) 보이며 참가 코드, 준비 표시가 달린 슬롯 넷, 상태 줄을
  보여 준다. ui_system.md는 커서/`GameAndUI`를 "Screen 레이어가 비어 있지 않은가"로
  정하므로 HUD 패널에는 클릭 가능한 버튼을 둘 수 없다(디자인 리뷰 결정 1) — 조작은
  **키**다: `IA_LobbyAction`(Enter) = 클라이언트는 준비 토글 / 호스트는 출발,
  `IA_CopyCode`(C) = 코드 복사. 패널이 힌트를 찍는다("Enter: 준비", "Enter: 출발", "C: 코드
  복사"). 게임패드는 나중에 같은 두 액션에 바인딩한다. 주문 선택(`OrderSelect`, HUD
  파생) 같은 Screen 행은 배타 Screen 자리를 두고 다투지 않고 패널 위에 열린다. 검토한
  대안(게이트 선택 5): 준비/출발을 기존 포커스 프롬프트 시스템을 통해 트럭 운전석의
  물리적 `IA_Interact`(E)로 만든다 — 더 세계 안에 있고, 세션 코드는 같음; 양방향 문.
- 출발 = 기존 `StartNight()`. 호스트에게만, **모든 PlayerState가 준비되고 주문 선택이
  끝났을 때**(`OfferedOrders` 비어 있음 — 결정 13) 활성화된다. `cl9.night`는 테스트용으로
  제한 없이 남는다. 밤 사이에도 같은 패널이 출발을 맡으므로 "떨어진 친구가 다음 정차
  때 다시 들어온다"에는 `PreLogin` 페이즈 게이트 외에 아무것도 더 필요 없다.
- 경합 방어: `PostLogin`이 페이즈를 재확인한다. `PreLogin`을 `Lobby`에서 통과했지만
  `StartNight` 뒤에 도착한 플레이어는 밤중에 스폰되는 대신 밤 진행 중 메시지와 함께
  타이틀로 돌아간다.
- **준비 규칙 (스펙 리뷰 지적 6):** `EnterLobby`에서 모든 PlayerState의 `bIsReady`를
  `false`로 되돌린다(호스트 쪽, `ReviveDead`와 나란히). 호스트는 준비를 토글하지 않는다 —
  출발을 누르는 것이 호스트의 준비다. 혼자 하기(호스트 혼자)는 주문 선택이 끝나는 즉시
  출발이 활성화된다. 버튼 이름은 어디서나 **출발**이다(session.md의 "게임 시작"은 W5에서
  바꾼다).
- **밤 사이의 참가는 새 참가이며 허용된다** (스펙 리뷰 지적 7): 나중 정차 때 들어오는
  친구는 `PreLogin` 페이즈 게이트를 통과해 새 PlayerState(가장 낮은 빈 슬롯, 만HP, 빈
  손)와 `OnRep_SiteMap`의 맵을 받는다. 이것은 로비 전용 참가(CCL8 D3)이지 재접속이
  아니다. 상태를 복원한 채 *진행 중인 밤*에 재접속하는 것은 TODOS에 남는다.
- "혼자 하기" = `bShouldAdvertise=false`인 호스트 세션(**Private** Steam 로비 — W2 튜플;
  친구조차 들어올 수 없음), 같은 코드 경로(session.md: 별도 로컬 모드 없음). OSS가
  없을 때(Steam 꺼짐, `-nosteam`)도 혼자 하기는 **동작한다**: Steam 세션 없이 레벨을 리슨
  서버로 연다(디자인 리뷰 결정 3). 솔로 로스터는 "혼자 · Enter: 출발"만 보여 준다 —
  코드도, 빈 슬롯도 없다.
- **실패 라우팅** (디자인 리뷰 결정 4): `UCL9GameInstance`가 `OpenLevel`을 살아남는
  `PendingNotice`(FText)를 들고 있다. 코드 경로의 실패(검색 0건/타임아웃, `JoinSession`
  결과, PreLogin 거부 — 맵이 로드되기 전에 트래블이 중단되므로 클라이언트는 아직
  `Lvl_Title`에 있다)는 입력한 코드를 유지한 채 `JoinCode`를 다시 열고 이유를 인라인으로
  보여 준다. PreLogin 거부는 호스트의 `ErrorMessage`를 실은
  `ENetworkFailure::PendingConnectionFailure`로 클라이언트에 도착한다
  (`UnrealEngine.cpp:16015-16017`가 `ConnectionError`에서 방송한다). 핸들러는
  `PendingConnectionFailure` / `FailureReceived`에 대해 그 문자열을 그대로 넘기고, 고정 표는
  `ConnectionLost` / `ConnectionTimeout` / `OutdatedClient` / `OutdatedServer`에만 쓴다
  (엔지니어링 음성 지적 4, E14). 세션 중 실패(호스트 이탈, PostLogin 추방, 연결 끊김)는
  `PendingNotice`를 설정하고 타이틀로 이동하며, 타이틀이 `Notice` 모달을 보여 준다. 참가
  경로는 연결 중 상태에서 자체 **20초** 타이머를 쓴다(취소 = `LeaveSession` + 트래블
  중단). 엔진의 80초 / 120초 드라이버 타임아웃은 세션 중 끊김용으로 남는다.
- **플레이어에게 보이는 문자열** (`FText` 표 하나, W2/T4):
  만원 "방이 가득 찼습니다 (4/4)" · 못 찾음 "잘못된 코드입니다" · 검색 실패 "검색에
  실패했습니다 — 다시 시도" · 버전 "버전이 다릅니다 — 둘 다 업데이트하세요" · 해시 "게임
  데이터가 호스트와 다릅니다 — 게임 파일을 확인하세요" · 밤 "밤이 진행 중입니다 — 다음
  정차 때 들어오세요" · 타임아웃 "연결 시간이 초과됐습니다" · 호스트 이탈 "호스트와의
  연결이 끊어졌습니다" · Steam 꺼짐 "Steam이 실행 중이 아닙니다 — 멀티플레이 비활성" ·
  복사 "복사됨".

### W4 — 패키징, Steam 업로드, 실제 AppID 테스트 루프
- Win64 패키징 설정: `Lvl_Title`, `Lvl_Henhouse`를 쿡한다(테스트용 `Lvl_Flat` 제외).
  `-nosteam`으로도 부팅된다. **Steam에는 Shipping 빌드만 올린다**(베타 브랜치 포함):
  `AllowCheats`는 비Shipping 빌드 전부에서 true를 돌려주고(`CL9GameMode.cpp:55-64`)
  `Server_CL9Cheat_Validate`는 무조건 통과라, 베타 브랜치의 Development 빌드는 어떤
  친구든 호스트에서 `cl9.money` / `cl9.night`를 돌릴 수 있게 한다(엔지니어링 음성 지적 8).
  Development 빌드는 두 PC W1/W2 테스트용으로 로컬에만 둔다.
- SteamPipe 스크립트는 `Build/Steam/`에 커밋한다(`app_build.vdf`, `depot_build.vdf`;
  저장소에 자격 증명 없음). `steamcmd`로 `beta` 브랜치에 업로드하고, 기본 브랜치는
  출시까지 건드리지 않는다.
- 480 → 자체 AppID 전환: `SteamDevAppId`, Steamworks 실행 옵션, 그다음 AC8.
- VAC: **결정 사항이 아니다** — `bVACEnabled`는 Steam 게임 서버 초기화
  (`SteamSharedModule.cpp:418-440`)에서만 읽히는데, 이 계획은 그것을 실행하지 않는다
  (`bInitServerOnClient=false`, 전용 서버 전용 핸들). 기본값을 두고, 게이트 항목은
  뺐다(엔지니어링 음성 지적 10).
- 자체 AppID에서 2~4인 테스트용 Steam Playtest 또는 친구 키.
- **두 프로세스 로컬호스트 스모크** (수용한 확장, P2): `Build/Smoke/` 스크립트가
  `-game -nosteam -log` 인스턴스 둘을 띄우고(호스트 `?listen`, 클라이언트
  `open 127.0.0.1?hash=…?ver=…` — UE 트래블 옵션은 `?`로 구분한다.
  `UGameplayStatics::GrabOption`은 `?`로만 자르므로(`GameplayStatics.cpp:3410-3428`) `&`를
  쓰면 `ver`가 해시 값 안으로 접혀 들어가 버전 게이트가 전원을 거부한다 — 접속 문자열
  빌더는 `ParseOption`으로 유닛 테스트한다) 양쪽 로그에서 참가 / 로스터 / 준비 줄을
  grep한다 — Steam도 두 번째 PC도 없이 IpNetDriver 위에서 `PreLogin`/`PostLogin` 게이트,
  준비/슬롯 복제, 출발, 클라이언트 강제 종료 케이스를 검증한다. Steam 세션 호출
  (생성/검색/참가/초대)은 **건드리지 않는다**; 그것은 두 PC의 S1~S6다. 선택적 S0 확장
  (엔지니어링 음성 지적 9, 미검증): 개발 전용 `-cl9nullsession` 스위치로
  `HostSession`/`JoinByCode`를 Null OSS의 LAN 세션 위에서 돌려 로컬 인스턴스 둘이 비동기
  콜백(검색 중 취소, `Deinitialize` 뒤의 콜백)을 검증한다. 한 PC의 Null 인스턴스 둘이
  서로를 못 찾으면 버린다. pie-scenarios S0로 문서화한다.
- 값싼 자동화 테스트 (P2): 참가 코드 정규화(알파벳, 대소문자, 공백, 길이)와 TuningHash
  결정성(같은 def 목록 → 같은 해시, 값 하나 바뀜 → 다른 해시)에 대한
  `FAutomationTestBase` 케이스. 순수 로직, Steam 없음.

### W5 — 문서와 메모리 동기화
- session.md: "미구현/미결" 문단 → 구현된 상태로; 참가 코드 알파벳; 밤중 참가 = 거부;
  단일 레벨 로비 오버레이; docs/review/review-2026-09-01.md M-2가 지적한 자기모순
  "향후 구현 예정 → 랜덤 매칭" 줄 삭제.
- 게이트 결정마다 decisions.md 항목. build-order.md에 "⑦ Steam 출시" 표(W0~W5)를 더하고
  1f를 "PIE 전용 — 실제 리슨 접속은 W1"로 표시. pie-scenarios.md에 S1~S6(오버레이, 두
  PC 참가, 코드 참가, 콜드 초대, 거부 경로, 호스트 종료) 추가.

## 범위 밖
- 전용 서버, 호스트 이주, 진행 중인 밤으로의 재접속(session.md "향후 복구"), 랜덤
  매칭 / 공개 서버 브라우저(하지 않기로 결정), Epic Online Services나 크로스플레이, 콘솔.
- Steam 도전과제, 리더보드, 클라우드 세이브, 창작마당, Steam Input 리매핑 UI, 음성 채팅,
  리치 프레즌스(자체 AppID에서 Steamworks 현지화 업로드 필요 — TODOS).
- Coming Soon 상점 페이지와 그 에셋(⑤ 아트 뒤의 별도 항목; W1~W4의 선행 조건 아님).
- 로스터의 세이브 슬롯(세이브 시스템은 동시 진행 중인 게임플레이 세션에서 만들고 있다;
  v1 로스터에는 슬롯이 없다), 추가 플레이어블 캐릭터, 상점 아트 / 트레일러 / 가격 /
  마케팅 문구, 한국어 이외의 현지화.
- session.md의 심리스 트래블과 밤마다의 로딩 진행 화면 — 단일 레벨 설계에서는 불필요.
  두 번째 목적지(빌드 오더 4g)가 다른 레벨을 필요로 하면 재검토.

## 수용 기준
- [ ] AC1 패키지 Development 빌드, Steam 클라이언트 실행 중: OSS 로그가 AppID 480으로
      Steam 초기화를 보고하고 Shift+Tab에 오버레이가 뜬다.
- [ ] AC2 PC 두 대, Steam 계정 두 개, 포트포워딩 없음: 호스트가 세션을 만들고 상대가
      Steam 친구 목록 "게임 참가"로 들어온다. 둘 다 트럭 패드에 서 있고 한쪽 화면에서
      주운 것이 다른 쪽에서 사라진다.
- [ ] AC3 클라이언트에 6자 참가 코드를 입력해도 같은 결과.
- [ ] AC4 콜드 스타트(게임 꺼짐)에서 수락한 오버레이 초대가 클라이언트를 호스트의 로비에
      데려다 놓는다.
- [ ] AC5 거부 경로가 올바른 메시지와 함께 클라이언트를 타이틀로 돌려보낸다: 다섯 번째
      플레이어(만원), TuningHash 불일치(클라이언트에서 DA 값 수정), 버전 불일치, 밤 진행
      중. 그리고 밤중에는 친구가 "게임 참가" 항목을 보지 못하거나, **또는** 트래블 전에 밤
      문자열과 함께 거절된다(클라이언트가 `JoinSession` 전에 `CL9_PHASE` 로비 키를 읽는다.
      `SetLobbyJoinable(false)` 때 Steam 클라이언트가 항목을 숨기는지는 미검증 — S5가
      어느 쪽인지 기록한다).
- [ ] AC6 전원 준비 + 주문 선택 → 호스트 출발 → 모두에게 밤이 시작된다. 호스트 종료 →
      클라이언트들이 "호스트와의 연결이 끊어졌습니다"를 받고 타이틀에 도착한다. *다음*
      정차 때 들어온 친구는 새 슬롯을 받는다.
- [ ] AC7 PIE 2클라이언트와 `-nosteam`이 기존 pie-scenarios.md 케이스를 전부 여전히
      돌린다(IpNetDriver 폴백) — 회귀 없음.
- [ ] AC8 자체 AppID `beta` 브랜치로 `steamcmd` 업로드. Steam 클라이언트가 설치하고, 실제
      AppID에서 오버레이 + 초대가 동작한다.
- [ ] AC9 문서 동기화(session.md, decisions.md, build-order.md, pie-scenarios.md)와
      work-progress 메모리 갱신.

## 관련 파일 / 모듈
| 파일 | 역할 |
|------|------|
| D:\Work\unreal\CL9\CL9.uproject | OnlineSubsystemSteam + SteamSockets 켜기 |
| D:\Work\unreal\CL9\Source\CL9\CL9.Build.cs | OnlineSubsystem, OnlineSubsystemUtils 모듈 |
| D:\Work\unreal\CL9\Config\DefaultEngine.ini | [OnlineSubsystem], [OnlineSubsystemSteam], NetDriverDefinitions |
| D:\Work\unreal\CL9\Source\CL9\Session\CL9SessionSubsystem.{h,cpp} | 신규 — IOnlineSession 래퍼, 참가 코드, 초대 델리게이트 |
| D:\Work\unreal\CL9\Source\CL9\Data\CL9DefRegistry.{h,cpp} | 로드된 def 행에 대한 TuningHash |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9GameMode.{h,cpp} | PreLogin/PostLogin 게이트, Logout 슬롯 반환, EnterLobby의 준비 초기화, 페이즈 변경 시 로비 참가 가능 여부, 출발 → StartNight |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9GameState.{h,cpp} | SessionCode (복제) |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9PlayerState.{h,cpp} | bIsReady, SlotIndex, GetSlotColor() |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9PlayerController.{h,cpp} | Server_SetReady / Server_Depart; IA_LobbyAction / IA_CopyCode 라우팅 |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9GameInstance.{h,cpp} | NetworkError / TravelError → 타이틀 |
| D:\Work\unreal\CL9\Source\CL9\UI\CL9TitleWidget, CL9JoinCodeWidget, CL9NoticeWidget, CL9RosterWidget .{h,cpp}; D:\Work\unreal\CL9\Content\UI\WBP_Title, WBP_JoinCode, WBP_Notice, WBP_Roster, WBP_Pause, DA_UITable.uasset | Title(Screen) / JoinCode(Popup) / Notice(Popup, 모달 + 토스트) / Roster(HUD, 표시 전용) 행; Pause [로비로 나가기] 배선 |
| D:\Work\unreal\CL9\Content\Input\Actions\IA_LobbyAction, IA_CopyCode; D:\Work\unreal\CL9\Content\Input\IMC_Gameplay.uasset | Enter = 역할별 준비/출발, C = 코드 복사 (Lobby 페이즈에서만) |
| D:\Work\unreal\CL9\Content\Maps\Lvl_Title.umap | 새 타이틀 레벨 (GameDefaultMap) |
| D:\Work\unreal\CL9\Source\CL9\GameMode\CL9CheatManager.{h,cpp}, CL9Cheats.cpp | cl9.host / cl9.join / cl9.leave / cl9.session |
| D:\Work\unreal\CL9\Config\DefaultGame.ini | ProjectVersion (CL9_VER 게이트) |
| D:\Work\unreal\CL9\Build\Smoke\two-process-smoke.ps1 | -nosteam 로컬호스트 스모크 (호스트 + 클라이언트) |
| D:\Work\unreal\CL9\Source\CL9\Tests\CL9SessionCodeTest.cpp | 자동화 테스트: 코드 정규화, TuningHash 결정성 |
| D:\Work\unreal\CL9\Build\Steam\app_build.vdf, depot_build.vdf | SteamPipe 업로드 스크립트 |
| D:\Work\unreal\CL9\docs\systems\session.md, docs\decisions.md, docs\plan\build-order.md, docs\testing\pie-scenarios.md | 문서 동기화 |

## 반드시 지킬 것
- 호스트 권위(conventions § 2): Steam은 전송만 한다. 판정이 Steam 백엔드로 옮겨 가지
  않는다. 준비 / 슬롯 / 캐릭터는 Server RPC로 설정하고 복제로 돌려받는다 — 클라이언트를
  절대 믿지 않는다.
- 밤 수명주기 API(`StartNight` / `EndNight` / `EnterLobby`, `ECL9NightPhase`,
  `OnNightPhaseChanged`)와 그 구독자(NightLighting, HUD)는 바꾸지 않는다.
- 기존 PIE 2클라이언트 시나리오는 Steam 없이 계속 동작한다(AC7).
- C++에 에셋 경로 없음. 새 화면은 UI 표 + UIManager를 통한다. 주석은 한글
  (decisions.md 2026-09-06).
- MAX_PLAYERS = 4. 슬롯 = 입장 순서(session.md 2026-09-02).
- 새 Replicated 필드는 전부 `GetLifetimeReplicatedProps`에 등록한다(빌드 오더 ⑥
  주석). 복제되는 액터는 `SetNetUpdateFrequency(20)`을 부른다.

## 실행 메모
- 권장 모델: W1(5.8 플러그인 소스 검증, ini 의미), W2(세션 서브시스템, PreLogin 게이트,
  실패 경로), 두 PC 디버깅은 Claude Fable 5.1. W3 위젯 보일러플레이트, W4 vdf 스크립트,
  W5 문서 편집은 Sonnet도 허용.
- 이 문서는 모델을 강제하지 못한다 — 실행 세션의 `/model` 설정이 결정한다. 세션 모델이
  권장보다 낮으면 사용자에게 알리고 확인 후 진행한다.
- 머신 제약: 정식 빌드는 에디터를 닫고(CLAUDE.md); OSS 초기화에는 Steam 클라이언트
  실행 필요; AC2~AC6에는 **다른** Steam 계정의 두 번째 PC(사용자 것이든 친구 것이든)가
  필요하다 — PIE로는 Steam 세션을 검증할 수 없다.
- 대략적 규모: W0는 대기가 지배(며칠~몇 주, 대부분 Valve); W1 CC 약 2시간 + 첫 두 PC
  테스트; W2 CC 약 4시간; W3 CC 약 4시간; W4 CC 약 2시간 + 업로드; W5 약 1시간.

## 미결 사항
1. 한국에서 판매하기 위한 법적 선행 조건(사업자등록·게임제작업 등록, GRAC 등급분류 또는
   Steam IARC) — W0 상점 페이지 전에 현행 규정 확인.
2. Steamworks 파트너 계정이 이미 있는가 / 앱 수수료를 냈는가? W0 시계를 정한다.
3. 두 번째 PC + 두 번째 Steam 계정 — **W1 진입 조건**, 선택이 아님: 어느 기계, 누구
   계정? (패밀리 셰어링은 두 번째 SteamID를 주지 않는다.)
4. ~~`bInitServerOnClient` / `bUseSteamNetworking` 의미~~ — 2026-09-06 5.8 플러그인
   소스를 읽어 해결: SteamSockets 리슨 호스트에는 `bInitServerOnClient`가 필요 없다(W1
   참조).
5. 밤중 참가: 제안 = v1은 거부 (게이트 항목).
6. ~~VAC~~ — 결정 사항 아님: 이 플래그는 Steam 게임 서버 초기화에만 관련 있는데 이
   계획은 그것을 실행하지 않는다(엔지니어링 리뷰).
7. 단일 레벨 로비: 트럭 패드 위 HUD 레이어 로스터 패널, `ECL9SessionState` 없음, 심리스
   트래블 / 로딩 화면 없음 — session.md의 별도 로비 흐름 대신 (게이트 항목, 취향).

## 공정성 제약
- TuningHash 불일치는 `PreLogin`에서의 서버 측 하드 거부다. 클라이언트 사전 확인은
  메시지를 좋게 할 뿐이다.
- 참가 코드 검색은 항상 `CL9_GAME` 식별 키를 포함한다 — AppID 480은 전 세계 Spacewar
  세션과 공용이다.
- 클라이언트를 믿는 세션 상태는 없다. 준비 / 슬롯 / 캐릭터는 Server RPC로.

## 기존 통합 계약
- `ACL9GameMode::StartNight()`가 게임 시작이다. 로비의 출발이 이것을 부른다. 밤 사이의
  `EnterLobby()`는 건드리지 않는다.
- `ACL9GameState` 밤 필드와 `OnNightPhaseChanged` 소비자는 건드리지 않는다. 새
  GameState 필드는 `SessionCode` 하나뿐이다.
- `UCL9UIManager::OpenWidget(FName)` Screen 배타성. 상태 파생 화면(주문)은 계속 상태에서
  파생한다. 로스터 패널은 HUD 레이어다.
- 기존 `cl9.*` 치트는 Steam 넷드라이버 위에서도 `Server_CL9Cheat`로 계속 라우팅된다. 새
  `cl9.host/join/leave/session`은 호스트가 생기기 전에 돌기 때문에 기존 `bLocalOnly`
  경로(`CL9Cheats.cpp:86`)를 쓴다.

---

# /autoplan — 1단계: CEO 리뷰 (선택적 확장, 자동 결정)

2026-09-06 /autoplan으로 실행. CEO 스킬이 던졌을 AskUserQuestion은 전부 여섯 원칙으로
자동 결정했고, 취향 판단과 사용자 도전은 최종 승인 게이트로 넘겼다. Codex 음성:
**사용 불가**(2026-09-14까지 사용량 한도, 실제 `codex exec` 프로브로 확인) →
`[subagent-only]`.

## 시스템 감사 (리뷰 전)
- HEAD `ba08285`. 최근 커밋 30개는 전부 게임플레이/UI 스텝(4a~4d, 5a, 5c, HUD)이고
  **세션이나 Steam 커밋은 없다.** 스태시 없음, TODOS.md 없음, Source/와 docs/에
  TODO/FIXME 표식 없음.
- 작업 트리가 더럽다(docs와 많은 Source 파일이 수정된 채 미커밋 — work-progress 메모리에
  따르면 UI 세션의 진행 중 작업). 이 계획은 그 파일들을 건드리지 않지만 `CL9GameMode`,
  `CL9GameState`, `CL9PlayerState`, `CL9PlayerController`, `CL9GameInstance`는 예외인데 이들도
  더럽다 → **W1~W3은 현재 UI 작업이 커밋된 뒤 시작해야 한다**(병합 위험, 바이너리 에셋은
  편집 주체 하나).
- 회고: 이전 프로젝트(CCL8)는 눈에 보이는 것이 하나도 없는 상태에서 세션과 생성기를
  짓다가 죽었다(build-order.md "왜 이 계획인가"). 이 계획은 CL9의 첫 "보이지 않는 인프라"
  스텝이다. W1의 수용 조건은 일부러 눈에 보이게 했고(오버레이 + 두 PC가 만남), 치트
  (`cl9.host/join`)가 화면이 생기기 전에도 모든 스텝을 직접 해볼 수 있게 한다.
- 설계 문서: CCL8 그린필드 문서(2026-08-27)가 Steam 리슨 서버, 로비 전용 참가, 전용
  서버/호스트 이주 없음을 확정했고, steam-multiplayer-intent A1~A5(리슨 서버,
  MAX_PLAYERS=4, 로비 전용 참가, 버전 게이트, 20Hz)도 있다. 이 계획은 전부와 정합한다.
- 취향 보정 — 따라 할 좋은 패턴: `UCL9UIManager` + UI 표(데이터 기반 화면), 호스트 전용
  전이와 `OnNightPhaseChanged`를 가진 `ACL9GameState`의 밤 수명주기(상태 → 표현),
  `UCL9DefRegistry` 별칭 해석(C++에 에셋 경로 없음). 반복하지 말아야 할 안티패턴: PIE만으로
  "닫힌" 1f; 자기모순의 "향후 구현 예정" 목록을 가진 session.md(review-2026-09-01 M-2).
- UI 범위: **예**(타이틀, 모드 선택, 참가 코드 팝업, 연결 중, 로스터 패널). DX 범위:
  **아니오** — 제품은 소비자용 게임이다. "SDK/API" 검색 결과는 우리가 소비하는 서드파티
  의존성이지 개발자 대상 표면이 아니다(2.5단계 생략, 감사 기록에 남김).

## 0A단계 — 전제 도전
| # | 전제 | 판정 |
|---|---------|---------|
| P1 | 플랫폼은 Steam이다(itch/Epic이 먼저가 아님) | 수용 — 확정된 결정(overview.md, CCL8 D5); Steam 중계망 + 친구 목록이 참가 이야기의 전부다 |
| P2 | 리슨 서버 + Steam 중계망이면 1~4인 협동에 충분하다 | 수용 — 장르 표준(Lethal Company / Content Warning / R.E.P.O.의 형태); 호스트 권위의 지연 비대칭은 conventions § 2에서 이미 수용 |
| P3 | "운영할 서버가 없다" | 더 날카로운 표현으로 수용: *런타임* 서버가 없다. 개발자는 여전히 빌드 파이프라인, 상점 페이지, 패치를 운영하고, 중계망 가동은 Valve의 몫이다 |
| P4 | OSS Steam이 초기화되면 오버레이가 뜬다 | 이제 W1에 적은 단서와 함께 수용: Steam 클라이언트 실행 중, Steam이나 `steam_appid.txt`로 실행, PIE에서는 아님 |
| P5 | AppID 480에서 로비 검색으로 참가 코드가 동작한다 | 수용 — `bShouldAdvertise` + 식별 키 필요; 친구 전용 세션은 의도적으로 검색되지 않는다 |
| P6 | 단일 레벨 로비(심리스 트래블 없음) | 계획의 제안으로 수용, **취향** — session.md에서 벗어남; 게이트로 넘김(선택 1) |
| P7 | Valve의 출시 시계는 짧다 | **암묵적 가정이 틀렸다** — 검증한 순서는 수수료 → 30일 → 페이지 심사 → Coming Soon 14일 이상 → 빌드 심사: 최소 7주. 수정: W0를 지금 병행 시작; AC8은 상점 페이지를 기다리지 않는다 |
| P8 | 1f "2인 리슨 접속"은 끝났다 | **틀렸다** — PIE 전용; 두 프로세스가 연결된 적 없음. 수정: W1이 진짜 1f; build-order.md는 W5에서 정정 |

아무것도 안 할 때의 비용: 게임은 PIE 전용으로 남고, 개발자 기계 밖의 누구도 플레이할
수 없으며, 이후의 모든 시스템(세이브, 주문 경제)이 실제 네트워크 왕복 없이 만들어진다 —
CCL8의 실패를 거꾸로 반복하는 것이다.

## 0B단계 — 기존 코드 활용 지도
| 하위 문제 | 기존 코드 | 재사용? |
|---|---|---|
| 참가 가능 / 불가 상태 | `ACL9GameState::Phase`(Lobby/Night/Dawn), 호스트 전용 `SetPhase` | 예 — PreLogin 게이트가 읽는다; 새 세션 enum 없음 |
| 게임 시작 | `ACL9GameMode::StartNight()`(phase == Lobby 가드) | 예 — 출발이 부른다 |
| 플레이어별 복제 상태 + HUD 훅 | `ACL9PlayerState`(Hp, 소지, `OnRep_*` + HUD 델리게이트) | 예 — `bIsReady`, `SlotIndex`가 같은 형태를 따른다 |
| Server RPC 패턴 | `Server_AcceptOrders`, `Server_CL9Cheat`(WithValidation) | 예 — `Server_SetReady`, `Server_SelectCharacter` |
| 화면 | `UCL9UIManager::OpenWidget(FName)` + `DA_UITable` 행, `CL9UINames` | 예 — Title / ModeSelect / JoinCode / Connecting 행; 로스터는 HUD 레이어 |
| def 표 → 해시 | `UCL9DefRegistry::Init(DefList)` | 예 — 거기서 한 번 계산 |
| 치트 | `UCL9CheatManager`, `CL9Cheats.cpp`, `AllowCheats` 오버라이드 | 예 — `cl9.host/join/leave/session` |
| 전송 | 엔진 `OnlineSubsystemSteam` 1.2 + `SteamSockets`(SDK 1.64) | 예 — 커스텀 없음 |
| 오류 → 타이틀 | 없음(오늘은 `NetworkFailure` 핸들러 없음) | `UCL9GameInstance`에 신규 |
| 로비의 세이브 "판" | 없음(`UCL9SaveGame` 미구현) | 보류(TODOS) |
다시 만드는 것은 없다.

## 0C단계 — 꿈의 상태
```
  현재 상태                       이 계획                          12개월 이상향
  PIE 전용 2클라이언트 테스트;    Steam 오버레이; 친구 초대 +      Steam에 출시; 친구가 한 클릭으로
  Steam도 세션 코드도 없음;       참가 코드; 트럭 패드 위          들어옴; 진행 중인 밤에 재접속;
  1f는 에디터를 떠난 적 없음      로스터/준비; PreLogin 게이트;    로비에서 저장한 판 이어하기;
                        --->    베타 브랜치 업로드 루프;   --->  Playtest 피드백 루프; Steam Deck
                                W0 서류 시계 진행 중              검증; 도전과제는 선택
```
차이: 이 계획은 "Steam에서 친구와 플레이" 간극 전체를 닫고, 의도적으로 문 두 개(밤중
재접속, 세이브/이어하기)만 닫아 둔다. 둘 다 이미 깔끔하게 거부하는 게이트 뒤에 있다.

## 0C-bis단계 — 구현 대안
```
접근 A: 최소 실행 — 플러그인 + 초대만
  요약: OSS Steam + SteamSockets를 켜고, 콘솔 치트로 호스트하고, Steam 오버레이
        초대로만 참가; 참가 코드도, 타이틀/로스터 UI도, 게이트도 없음.
  노력: S   위험: 중 (거부 메시지 없음 → 조용한 실패; 버전 게이트 없음)
  장점: 두 PC가 하루 만에 연결; 전송을 증명
  단점: 코드로 오는 비친구 경로 없음; 만원/해시/버전 거부가 조용함; 개발자가
        Discord에 없으면 플레이어가 쓸 수 있는 것이 없음
  재사용: 엔진 플러그인, CheatManager
  완성도: 4/10

접근 B: 이 계획 (세션 서브시스템, 참가 경로 둘, 로스터, 게이트, 업로드)
  요약: W1~W5 그대로, 단일 레벨 로비, 세션 enum 없음.
  노력: M~L   위험: 중 (Steam은 실제 계정 둘로만 테스트 가능)
  장점: 모든 실패에 메시지가 있음; -nosteam 스모크로 Steam 없이 테스트 가능;
        서브시스템이 나중의 재접속/세이브의 토대
  단점: 파일 ~10개; AC2~AC6에 두 번째 PC + 계정 필요
  재사용: 0B의 전부
  완성도: 9/10

접근 C: 이상 — B + 진행 중 재접속 + 로비에서 세이브/이어하기
  요약: "판" 이어하기와 밤중 재접속까지 session.md를 완전히 실현.
  노력: L~XL   위험: 높음 (세이브 시스템 전체를 끌어들임; 스폰/상태 복원)
  장점: 첫날부터 로비 기능 완비
  단점: 세이브 시스템은 자체 결정을 가진 별도 목표; 영향 범위가 두 배
  완성도: 10/10이지만 이 계획의 영향 범위 밖

권장: B — 영향 범위 안에서 P1(완성도); C의 추가 범위는 세이브 시스템 목표에 속한다
(P2: 영향 범위 밖 → TODOS). A와 B는 근접하지 않다(4 대 9) → 취향 결정이 아님.
```

## 0D단계 — 선택적 확장 분석
- 복잡도 검사: 파일 8개 초과, 새 클래스 1개(`UCL9SessionSubsystem`) + 위젯 4개. 정당함 —
  전송/세션 층은 본성상 설정, GameMode, GameState, PlayerState, PC, GI, UI를 건드린다.
  `ECL9SessionState`와 심리스 트래블/로딩 화면을 뺀 것이 피할 수 있는 부분을 제거했다.
- 명시된 목표를 위한 최소 집합: W1 + W2(호스트/초대/코드 + 게이트) + 로스터 패널 + W4
  업로드. W3의 타이틀/모드 화면은 치트 뒤로 미룰 수도 있지만 AC1~AC6가 플레이어
  대면이라 유지.
- 확장 스캔 → 체리픽 의식을 자동 결정(제안 13개; 표는
  `C:\Users\user\.gstack\projects\Work\ceo-plans\2026-09-06-steam-release.md`):
  **수용** `cl9.host/join/leave/session`(P2), 코드 복사/붙여넣기, 두 프로세스 `-nosteam`
  스모크(P2), 코드 정규화와 TuningHash 유닛 테스트(빌드 오더 ⑥에서 앞당김 — ⑥은 "유닛
  테스트 · 2인 스모크"를 예정하고 스냅샷 테스트만 제외한다), `ProjectVersion`/`CL9_VER`,
  `ECL9SessionState` 제거 + HUD 레이어 로스터(취향 → 게이트), 주문 선택에 묶인 출발;
  **보류** 리치 프레즌스(Steamworks 현지화 업로드 필요 — 처음엔 수용했다가 스펙 리뷰 뒤
  번복), 진행 중 재접속, 세이브/이어하기, Steam Deck, 도전과제; **생략** "다시 호스트"
  단축.
- 플랫폼 잠재력: 세션 서브시스템 + PreLogin 게이트 + 로스터 = 클라이언트를 건드리지
  않고 재접속과 세이브/이어하기를 얹을 토대.

## 0E단계 — 시간 순 심문 (사람 시간 / CC 분)
- 1시간째(기초 / CC 약 10분): 개발 PC에 어느 Steam 계정이 로그인돼 있는지; Steam
  클라이언트는 에디터보다 먼저 실행; PIE는 IpNetDriver에 남음(PIE에서 `HostSession` 호출
  없음); 플러그인 켜는 빌드는 에디터를 닫고.
- 2~3시간째(핵심 / CC 약 40분): `EOnlineDataAdvertisementType::ViaOnlineService`를 쓴
  `FOnlineSessionSettings` 커스텀 키; `SEARCH_PRESENCE=true` + `CL9_GAME`/`CL9_CODE` 동등
  검색; 서브시스템의 `Deinitialize`에서 델리게이트 핸들 해제; `GetResolvedConnectString`로
  접속 문자열; `Initialize`에서 `OnSessionUserInviteAccepted` 바인딩(콜드 스타트는 첫 월드
  뒤에 발화).
- 4~5시간째(통합 / CC 약 40분): `PreLogin`이 `UGameplayStatics::ParseOption`으로
  `?hash=`/`?ver=`를 읽음; `ErrorMessage` 텍스트는 사용자에게 보이는 한글 문자열이어야
  함(엔진이 그대로 보여 줌); `PostLogin` 페이즈 재확인; `Logout` 슬롯 반환; GI에서
  `UEngine::OnNetworkFailure` 바인딩.
- 6시간째 이후(마무리 / CC 약 30분): 실패 문구 표(만원 / 해시 / 버전 / 밤 / 호스트 이탈 /
  타임아웃 / Steam 꺼짐); 오버레이는 패키지 빌드에서만; Shipping에는 `steam_appid.txt`
  없음; AC2~AC6용 두 번째 계정.

## 0F단계 — 모드
선택적 확장(autoplan 오버라이드; 기존 시스템 위의 기능에 대한 맥락 기본값이기도 함).
접근 B 확정.

## 1절 — 아키텍처
```
                         STEAM (Valve): 로비 목록 · 프레즌스 · 초대 · 중계망 (SDR)
   ┌──────────────┐◄──────────────────────────────────────────────►┌──────────────┐
   │ Steam client │                                                │ Steam client │
   └──────┬───────┘ steam_api (OSS Steam, AppID 480 → 자체)         └──────┬───────┘
   HOST PC│                                                       CLIENT PC│
   ┌──────▼──────────────────────────────┐                ┌───────────────▼─────────────┐
   │ UCL9GameInstance                     │                │ UCL9GameInstance            │
   │  ├ UCL9DefRegistry → TuningHash      │                │  ├ UCL9DefRegistry → hash   │
   │  ├ UCL9SessionSubsystem [NEW]        │  Steam lobby   │  ├ UCL9SessionSubsystem     │
   │  │   HostSession / LeaveSession ─────┼───────────────►│  │   JoinByCode / Invite    │
   │  │   UpdateSession(joinable↔phase)   │                │  │   → ClientTravel(?hash?ver)
   │  ├ OnNetworkFailure → Title [NEW]    │                │  ├ OnNetworkFailure → Title │
   │  └ UCL9UIManager (Title/ModeSelect/  │                │  └ UCL9UIManager            │
   │       JoinCode/Connecting rows)      │                │                             │
   │ ACL9GameMode (authority)             │  SteamSockets  │                             │
   │  ├ PreLogin gates [NEW] ◄────────────┼────────────────┼── connect                   │
   │  ├ PostLogin phase re-check [NEW]    │  NetDriver     │                             │
   │  ├ StartNight (existing) ◄ 출발       │                │                             │
   │  └ Logout → slot free [NEW]          │                │                             │
   │ ACL9GameState ─── replicated ────────┼────────────────┼─► ACL9GameState (mirror)    │
   │  ├ Phase / Day / Orders (existing)   │                │    └ WBP_Roster (HUD) [NEW] │
   │  └ SessionCode [NEW]                 │                │                             │
   │ ACL9PlayerState ── replicated ───────┼────────────────┼─► ACL9PlayerState           │
   │  └ bIsReady / SlotIndex [NEW]        │◄───────────────┼── Server_SetReady (RPC up)  │
   └──────────────────────────────────────┘                └─────────────────────────────┘
```
- 새로 생기는 결합: GameMode ↔ SessionSubsystem(Logout/Destroy), GI ↔ 엔진 네트워크
  실패 델리게이트, HUD ↔ PlayerState 준비 필드. 전부 기존 이음매(서브시스템, 델리게이트,
  OnRep)를 통한다. 게임플레이 액터와 Steam 사이의 새 결합은 없다.
- 상태 기계:
```
 호스트: [Title] ─HostSession→ [Creating] ─ok→ [Lobby(phase)] ─StartNight→ [Night] ─EndNight→ [Dawn] ─EnterLobby→ [Lobby]
           ▲                      │fail→Title+msg      │ LeaveSession / quit → DestroySession → Title
           └───────────────────────────────────────────┘
        비정상: Lobby 밖의 StartNight(오늘 이미 가드됨); 세션이 있는데 HostSession(서브시스템이 거부, 로그)
 클라이언트:[Title] ─JoinByCode→ [Searching] ─0→ Title+"잘못된 코드" ─1→ JoinSession ─ok→ [Connecting] ─PreLogin ok→ [Lobby]
                 ─Invite→ (검색 생략) ─────────────────┘                    │reject→ Title+reason  │
        [Lobby/Night/Dawn] ─NetworkFailure(host left, timeout)→ Title+"호스트와의 연결이 끊어졌습니다"
        비정상: Searching/Connecting 중의 Join(버튼 비활성 + 서브시스템 상태 확인)
 참가 게이트 (PreLogin): phase==Lobby && NumPlayers<4 && hash== && ver==  아니면 ErrorMessage
```
- 확장성: 4인이라 무의미. 10배 = AppID 480에 식별 키 로비 40개 — 여전히 필터 검색 한 번.
  단일 장애점: 호스트 PC(수용)와 Valve 중계망(수용, 무료).
- 보안: 3절 참조. 통합 지점별 실제 장애: Steam 클라이언트 미실행(OSS 초기화 실패 → 이유와
  함께 멀티플레이 비활성); 로비 생성 타임아웃(`OnlineAsyncTaskSteamCreateServerTimeout`)
  → 메시지; 중계 연결 끊김 → `ConnectionTimeout`(기본 80초) → 타이틀.
- 롤백: `.uproject` + `Build.cs` + `DefaultEngine.ini`를 `git revert`하면 빌드 한 번으로
  오늘의 IpNetDriver PIE 상태로 돌아간다. `-nosteam`은 런타임에 같은 일을 한다.
- 아름다운 버전: 로스터 패널이 유일한 새 UI 개념이다 — 나머지는 전부 UI 표의 행과
  복제 필드 몇 개다. 그것이 "영리하면서 뻔한" 시험을 통과하는 지점이다.

## 2절 — 오류와 구조 지도
```
  코드 경로                             | 무엇이 잘못될 수 있나                        | 오류 클래스 / 신호
  --------------------------------------|--------------------------------------------|-----------------------------
  OSS Steam 초기화 (모듈 시작)          | Steam 클라이언트 미실행 / 480 차단           | IOnlineSubsystem::Get()==null, LogOnline error
  SessionSubsystem::HostSession         | 이미 세션 중; CreateSession 실패            | OnCreateSessionComplete(false)
                                        | 생성 타임아웃                               | OnlineAsyncTaskSteamCreateServerTimeout
  SessionSubsystem::JoinByCode          | 잘못된 문자 / 길이                          | 로컬 검증
                                        | FindSessions 실패 / 타임아웃                | OnFindSessionsComplete(false)
                                        | 결과 0개 / 여러 개                          | SearchResults.Num()
                                        | JoinSession 결과 ≠ Success                 | EOnJoinSessionCompleteResult (SessionIsFull, SessionDoesNotExist, CouldNotRetrieveAddress, AlreadyInSession, UnknownError)
                                        | GetResolvedConnectString 실패              | bool false
  초대 수락 (델리게이트)                | 플레이어와 호스팅 중; 밤중                  | 상태 확인; PreLogin 거부
  ClientTravel → PreLogin               | 만원 / 해시 / 버전 / 페이즈                 | ErrorMessage 설정 → 클라이언트 HandleNetworkFailure(FailureReceived)
  PostLogin                             | PreLogin 뒤 페이즈가 Night로 바뀜           | 재확인 → ClientReturnToMainMenuWithTextReason
  세션 중                               | 호스트 종료 / 중계 끊김 / Steam 클라이언트 종료 | ENetworkFailure ConnectionLost / ConnectionTimeout
  LeaveSession / DestroySession         | DestroySession 실패                        | OnDestroySessionComplete(false)
  TuningHash                            | def 목록 없음 (오늘 양쪽 다 Error 로그)     | 빈 레지스트리의 해시
  UpdateSession (페이즈별 참가 가능)    | 실패 / 타임아웃                             | OnUpdateSessionComplete(false)

  오류 클래스 / 신호                    | 구조? | 구조 동작                                       | 사용자가 보는 것
  --------------------------------------|-------|-------------------------------------------------|-----------------------------
  OSS null (Steam 꺼짐)                 | Y     | 서브시스템이 Unavailable 보고; IpNetDriver       | 멀티플레이 버튼 비활성: "Steam이 실행 중이 아닙니다"
  OnCreateSessionComplete(false)        | Y     | 로그 + 타이틀에 머묾                             | "세션을 만들지 못했습니다 — 다시 시도"
  생성 타임아웃                         | Y     | 위와 같음 (엔진이 complete(false) 발화)          | 같음
  로컬 코드 검증                        | Y     | 인라인, 버튼 비활성                              | "코드는 6자 (0·O·1·I 제외)"
  OnFindSessionsComplete(false)/타임아웃| Y     | 로그 + 재시도 버튼                               | "검색 실패 — 다시 시도"
  결과 0개                              | Y     | 팝업에 머묾                                      | "잘못된 코드입니다"
  결과 여러 개                          | Y     | 최신 것 선택, 두 코드로 WARNING 로그             | 없음 (투명)
  SessionIsFull                         | Y     | 타이틀                                           | "방이 가득 찼습니다 (4/4)"
  SessionDoesNotExist                   | Y     | 타이틀                                           | "방이 사라졌습니다 (호스트가 나갔습니다)"
  CouldNotRetrieveAddress/Unknown       | Y     | 타이틀 + 결과 enum 로그                          | "연결할 수 없습니다"
  AlreadyInSession                      | Y     | DestroySession 뒤 한 번 재시도                   | 없음 (투명)
  PreLogin ErrorMessage                 | Y     | 엔진 → OnNetworkFailure(FailureReceived,msg)     | ErrorMessage 그 자체 (이유별 한글)
  PostLogin 페이즈 경합                 | Y     | 이유와 함께 메뉴로 복귀                          | "밤이 진행 중입니다 — 다음 정차 때 들어오세요"
  ConnectionLost / ConnectionTimeout    | Y     | GI 핸들러 → OpenLevel(Title)                     | "호스트와의 연결이 끊어졌습니다"
  OnDestroySessionComplete(false)       | Y     | WARNING 로그, 타이틀로 계속                      | 없음
  빈 레지스트리의 해시                  | Y(기존)| Init이 이미 Error 로그; 해시는 여전히 결정적     | (개발) 로그
  OnUpdateSessionComplete(false)        | Y     | WARNING 로그, 다음 페이즈 변경 때 한 번 재시도; PreLogin 게이트가 최후 방어 | 없음
```
포괄 처리 없음: `EOnJoinSessionCompleteResult` 값 하나하나와 `ENetworkFailure` 값
하나하나가 자기 문자열에 매핑된다. 갭: 이 표 뒤에는 없음. 리뷰 전에는 OSS 꺼짐 경로도
PostLogin 경합도 계획에 없었다.

## 3절 — 보안과 위협 모델
| 위협 | 가능성 | 영향 | 완화? |
|---|---|---|---|
| AppID 480의 낯선 사람이 보내는 임의 로비 데이터(FindSessions가 필터에 맞는 아무 Spacewar 로비나 돌려줌) | 중 | 낮음 | 예 — 결과는 비교만 하고 표시하지 않음; 식별 키 + 코드 동등; 여러 개면 로그 |
| 잘못된 `?hash=` / `?ver=` 옵션(길이, 유니코드) | 낮음 | 낮음 | 예 — `ParseOption` + 고정 길이 비교; 그 밖은 일반 메시지로 거부 |
| 클라이언트가 다른 플레이어의 준비/캐릭터를 설정(엉뚱한 PlayerState에 RPC) | 낮음 | 낮음 | 예 — Server RPC는 호출자 자신의 PlayerState에만 작용; 캐릭터 별칭은 레지스트리로 검증 |
| 클라이언트가 출발을 트리거 | 중 | 중 | 예 — StartNight는 호스트 전용(기존 `HasAuthority`, PC가 호스트인지 확인); 클라이언트 요청은 `Server_AcceptOrders`처럼 거부 + 로그 |
| 클라이언트가 DA 표(튜닝)를 수정 | 중 | 중 | 예 — PreLogin에서 TuningHash 거부(권위); 호스트는 여전히 치트 가능 — 수용(호스트 권위 모델) |
| 클라이언트의 치트 명령 | 중(비Shipping) | 중 | `AllowCheats`는 비Shipping 빌드 전부에서 true → **Steam에는 Shipping 빌드만 업로드**; Development 빌드는 두 테스트 PC를 떠나지 않는다(엔지니어링 리뷰) |
| 비밀 정보 | — | — | 저장소에 없음: `steamcmd` 로그인은 대화형; vdf 파일에는 AppID/depot ID만 |
| 새 의존성 | 낮음 | 낮음 | Epic이 관리하는 플러그인 + 엔진 동봉 Steamworks SDK 1.64 |
| 개인정보 | 낮음 | 낮음 | 로스터의 Steam 표시 이름(이미 PlayerState 이름); 저장하지 않음 |
| VAC | — | — | 꺼짐(결정 6, 게이트 항목): 공개 세션 없음, 낯선 사람이 닿을 것이 없음 |
높음 등급 지적 없음.

## 4절 — 데이터 흐름과 상호작용 경계 사례
```
 참가 코드 (호스트)   6자 굴림 ──► Settings.Set(CL9_CODE) ──► CreateSession ──► GameState.SessionCode ──► WBP_Roster
                    RNG: nil 불가 │ 고정 길이             │ 실패 → 타이틀 메시지 │ 클라이언트 OnRep    │ 복사 버튼
 참가 코드 (클라이언트) 입력 ──► 정규화(대문자, 트림, 알파벳) ──► FindSessions(CL9_GAME==cl9 && CL9_CODE==X) ──► Join ──► ClientTravel
                    nil/빈 값→비활성 │ 잘못된 문자→인라인 오류     │ 타임아웃/0/여러 개 → 위 표                  │ 거부 → 메시지
 준비 (클라이언트)   클릭 ──► Server_SetReady(bool) ──► PlayerState.bIsReady (복제) ──► OnRep → 전원 로스터 ──► 호스트 출발 활성?
                    더블 클릭 → 멱등 │ Lobby 아님 → 거부+로그 │ 소유자만 │ 4/4 전부 true && OfferedOrders.Num()==0
```
| 상호작용 | 경계 사례 | 처리 | 방법 |
|---|---|---|---|
| 호스트 버튼 | 더블 클릭 | Y | 서브시스템 상태 != Idle → 무시 |
| 호스트 버튼 | Steam 꺼짐 | Y | 이유와 함께 버튼 비활성 |
| 코드 입력 | 소문자, 공백, 0/O 혼동 | Y | 정규화; 알파벳에서 0/O/1/I 제외 |
| 코드 입력 | 7자 이상 붙여넣기 | Y | 6자로 자르고 검증 |
| 참가 | 검색 진행 중 클릭 | Y | 상태로 버튼 비활성 |
| 초대 | 플레이어와 호스팅 중 수락 | Y | 무시 + 로그(v1); 혼자일 때 수락 → 나간 뒤 참가 |
| 초대 | 밤중 수락 | Y | PreLogin 거부 메시지 |
| 초대 | 콜드 스타트 | Y | 첫 월드 뒤 델리게이트 발화; 타이틀에 "연결 중…" |
| 로스터 | 47자 Steam 이름 | Y | 텍스트 클립, 툴팁에 전체 |
| 로스터 | 로비 중 플레이어 이탈 | Y | Logout이 슬롯 반환; OnRep이 행 제거; 준비 재집계 |
| 출발 | 클라이언트가 아직 연결 중일 때 누름 | Y | PostLogin 페이즈 재확인이 그 클라이언트를 돌려보냄 |
| 출발 | 주문 미선택 | Y | 비활성(결정 13) |
| 연결 중 | 클라이언트 로드 중 호스트 종료 | Y | NetworkFailure → 타이틀 |
| 모든 곳 | 클라이언트 Alt+F4 | Y | 호스트에서 Logout; 슬롯 반환 |
| 모든 곳 | 세션 중 Steam 클라이언트 종료 | Y | SteamSockets 연결 소멸 → ConnectionLost → 타이틀 |
| 타이틀 | 세션 실행 중 두 번째 실행(Steam이 막음) | N/A | Steam은 계정당 인스턴스 하나만 허용 |

## 5절 — 코드 품질
- 기존 패턴에 맞음: GameInstance 서브시스템(`UCL9UIManager`처럼), Server RPC + OnRep
  (주문처럼), UI 표 행, 한글 주석, 에셋 경로 없음.
- DRY: 새 행에 `CL9UINames` 재사용; `cl9.host/join`에 `Server_CL9Cheat` 라우팅 재사용
  (이들은 호스트가 아니라 로컬 기계에서 돈다 — 참고: 이 치트들은 **로컬**이지 호스트를
  거치지 않는다; 치트 표에 기록). 실패 문자열은 흩어진 리터럴이 아니라 `FText` 표 하나에
  산다(현지화 준비).
- 이름: `HostSession`, `JoinByCode`, `LeaveSession`, `SessionCode`, `bIsReady`, `SlotIndex` —
  동작은 동사, 상태는 명사.
- 과잉 설계 검사: 세션 enum 없음, 서브시스템 하나 외에 `IOnlineSession` 위의 추상화 없음,
  재시도 프레임워크 없음(AlreadyInSession에만 한 번 재시도).
- 과소 설계 검사: 월드 변경 뒤의 비동기 OSS 콜백 → `TWeakObjectPtr`/서브시스템 수명으로
  바인딩하고 `Deinitialize`에서 핸들 해제; `PreLogin`은 트래블 중 `NumPlayers`만 믿으면 안
  된다(`GetNumPlayers() + NumTravellingPlayers` 사용).
- 복잡도: `PreLogin`은 게이트 4개가 순서대로(분기 5 미만); 괜찮다.
- 지적(자동 결정): 계획이 치트가 호스트를 거친다고 했는데 `cl9.host/join/leave`에는 틀림
  (호스트가 생기기 전에 로컬에서 동작). 위의 계획 본문을 수정했다.

## 6절 — 테스트 리뷰
```
 새 UX 흐름: 타이틀→호스트, 타이틀→코드 참가, 초대(웜/콜드), 로스터 준비/출발, 모든 거부 메시지, 호스트 이탈 복귀
 새 데이터 흐름: 세션 설정 ↔ 로비, 코드 검색, 준비/슬롯 복제, 해시/버전 옵션, 로비 참가 가능 ↔ 페이즈
 새 코드 경로: SessionSubsystem(비동기 완료 6개), PreLogin 게이트 ×4, PostLogin 재확인, Logout 슬롯 반환, GI 네트워크 실패(+LeaveSession), 치트 ×4
 새 비동기: CreateSession, FindSessions, JoinSession, UpdateSession, DestroySession, 초대 델리게이트
 새 통합: Steam 클라이언트/OSS, SteamSockets, steamcmd 업로드
 새 오류 경로: 2절 표 (18행)
```
- 프로젝트는 결정에 따라 스냅샷 테스트를 제외하고, 유닛 테스트 + 2인 스모크를 ⑥에
  예정한다(build-order.md "⑥ 마무리 정리"). 이 계획은 지금 쓰는 코드를 지키기 위해 값싼
  두 조각을 앞당긴다. 커버리지는 수동 시나리오 더하기 그 둘이다:
  - 유닛(FAutomationTestBase): 코드 정규화/알파벳; TuningHash 결정성과 민감도. 결정적,
    Steam 없음.
  - 통합(두 프로세스 `-nosteam` 스모크 스크립트): 로컬호스트의 호스트+클라이언트, 로그에서
    PreLogin 수락, 로스터 행, 준비 true, 출발 → 양쪽 Night를 grep. PC 한 대, Steam 없음.
    게이트와 복제를 덮는다.
  - 시스템/E2E(수동, PC 두 대, 계정 두 개 — pie-scenarios S1~S6): 오버레이, 초대 웜/콜드,
    코드 참가, 만원/해시/버전/밤 거부, 호스트 종료, 호스트→타이틀→다시 호스트(UE-239936
    확인).
- 금요일 새벽 2시 테스트: 스모크 스크립트 통과 + 자체 AppID에서 S2(친구 목록으로 참가).
  적대적 QA: 출발 중 초대로 오는 다섯 번째 플레이어; `0O1I`가 든 코드; 밤중에 Steam
  클라이언트 종료. 카오스: 호스트 네트워크 선을 뽑음 → 클라이언트들이 80초 타임아웃 안에
  메시지와 함께 타이틀로.
- 불안정성: 모든 Steam 테스트는 Valve + 계정 둘에 의존 → 설계상 수동 전용. 피라미드: 유닛
  2, 통합 1, 수동 E2E 6 — 뒤집혔지만 의존성을 싸게 모킹할 수 없는 네트워크 층에는
  정직하다.

## 7절 — 성능
- 식별+코드 필터의 FindSessions는 로비 1개 이하를 돌려주고, 로비 데이터는 짧은 문자열
  4개다. `UpdateSession`은 페이즈 변경 때만 돈다(밤당 두 번).
- TuningHash: Init 때 def 행을 한 번 훑는다(수백 행, 마이크로초).
- 로스터 갱신은 OnRep으로, 틱 폴링 없음. SteamSockets 기본값(`NetServerMaxTickRate=30`)은
  프로젝트의 20Hz 액터 정책 위에 있다 — 변경 불필요. 지적 없음.

## 8절 — 관측 가능성
- 모든 세션 전이에 SteamID, 코드, 페이즈, 이유(Creating/Hosting/Searching/Joining/Left/
  Failed:<enum>)를 담은 `LogCL9` 줄. 모든 PreLogin 거부는 클라이언트의 해시/버전을
  호스트 것과 함께 로그.
- `cl9.session`이 코드, 플레이어(슬롯/준비/이름), 페이즈, 넷드라이버 클래스, OSS 이름을
  덤프 — 친구가 "안 들어가져요"라고 할 때 붙여 넣을 명령 하나.
- pie-scenarios에 "찾아볼 로그 줄" 열 추가(Steam 문제에는 `LogOnline`, `LogNet`,
  `LogSockets` 상세도 `Verbose`).
- 텔레메트리 백엔드 없음(계획 없음; "서버 없음"과 일관): 플레이어 로그는 `Saved/Logs`에서.
  런북 행: Steam 꺼짐 → 플레이어에게 Steam 실행 안내; 버전 불일치 → 둘 다 업데이트; 밤
  진행 중 → 정차 때까지 대기; UE-239936 증상(`SteamSockets: ... listen`) → 게임 재시작.
- 운영의 즐거움: `cl9.session`이 넷드라이버 클래스와 OSS 이름을 찍으므로 잘못된 설정은
  명령 하나면 드러난다(별도 개발 빌드 UI 없음 — 스펙 리뷰 2차가 중복을 뺐다).

## 9절 — 배포와 롤아웃
- 데이터베이스 없음, 마이그레이션 없음. 롤아웃 순서: W1 설정 커밋(Steam 없이도 빌드
  가능) → W2/W3 → 자체 AppID의 W4 `beta` 브랜치 → 출시 때만 기본 브랜치. Steam은 이전
  빌드를 보관한다: 롤백 = Steamworks에서 "이전 빌드를 라이브로"(몇 분) 또는 코드 경로는
  설정 파일 셋을 `git revert`.
- 혼합 버전 구간(친구는 옛 빌드, 호스트는 새 빌드)은 둘 다 업데이트하라는 메시지를 내는
  `CL9_VER` 게이트가 닫는다.
- 기능 플래그: 런타임 `-nosteam`; 완전 후퇴는 `.uproject`에서 플러그인 비활성화.
- 배포 후 확인(첫 5분): Steam 라이브러리에서 실행 → 오버레이 → `cl9.session`이 Steam
  넷드라이버 표시 → 호스트 → 친구 "게임 참가" → 로스터 2/4 → 출발 → 둘 다 Night. 업로드
  전마다 스모크 스크립트 통과.

## 10절 — 장기 궤적
- 부채: Steam 수동 테스트는 영원히(의존성을 모킹할 수 없음); 세 곳의 문서(session.md,
  build-order, pie-scenarios)를 W5에서 동기화해야 한다.
- 경로 의존: OSS v1. Epic은 5.8에 `OnlineServices`(v2) Steam 구현을 싣지 않았다(엔진
  목록: EOS/Null/OSSAdapter뿐)라 오늘은 v1이 지원되는 길이다. 서브시스템 래퍼가 모든 OSS
  호출을 클래스 하나에 가둔다 → 나중의 이전은 파일 하나를 건드린다. 가역성 **3/5**.
- 지식 집중: W5 뒤에는 새 엔지니어가 session.md와 로스터/패널 메모를 읽으면 모든 조각을
  찾을 수 있다. 1년 뒤 질문: 명백함.
- 2/3단계: 진행 중 재접속(PreLogin 페이즈 게이트 열기 + 트럭에서 스폰), 세이브/이어하기
  (로비 "판" 슬롯), Playtest 프로그램 — 전부 재설계 없이 수용한 범위 위에 얹힌다.

## 11절 — 디자인과 UX (UI 범위 감지; 2단계가 깊은 리뷰를 돈다)
```
 [Title] ── 혼자 하기 ──► host(friends-only, no code) ──► [Truck pad + Roster(1/4, 출발)]
    │ ── 멀티플레이 ──► [ModeSelect] ── 호스트 ──► [Connecting: 세션 만드는 중] ──► [Truck pad + Roster(code, k/4)]
    │                      │ ── 코드 참가 ──► [JoinCode popup] ──► [Connecting] ──► [Truck pad + Roster]
    │                      │ ── 친구 참가 ──► Steam overlay (friends list) ──► same as invite
    │ ── 설정 / 종료
    │ ◄── every reject / host-left ──── [Title + message banner]
```
(1단계 스케치 — 2단계로 대체됨: ModeSelect 없음(결정 29), 코드 경로 거부는 JoinCode
칸에 머물고, 세션 중 실패는 Notice 팝업을 쓴다.)
| 기능 | 로딩 | 비어 있음 | 오류 | 성공 | 부분 |
|---|---|---|---|---|---|
| Title | Steam 초기화 스피너(1초 이하) | Steam 꺼짐 → 멀티플레이 비활성 + 이유 | — | 버튼 활성 | — |
| JoinCode | 검색 중… | 빈 입력 → 버튼 비활성 | 잘못된 코드 / 검색 실패 / 방 가득 / 버전 | 연결 중… | — |
| Connecting | 스피너 + 취소 | — | 거부 이유 → 타이틀 | 로스터 등장 | 타임아웃 80초 → 타이틀 |
| Roster | — | 1/4 혼자: "친구를 초대하세요 · 코드 ABC123" | 나간 플레이어 행 페이드 | 전원 준비 → 출발 활성 | 일부 미준비 → "k/4 준비"와 함께 출발 비활성 |
- 위계: 참가 코드가 호스트 로스터에서 가장 큰 텍스트다. 친구의 첫 시선이 "친구에게 뭘
  말하지"에 답한다.
- 슬롭 위험: 평범한 타이틀 메뉴. 2단계의 완화: 타이틀은 밤의 트럭이고 메뉴는 그 옆
  패널에 — 게임과 같은 시각 언어; 별도 아트 불필요.
- 접근성: Title/ModeSelect/JoinCode의 키보드/게임패드 포커스 순서(CommonUI 포커스
  cvar는 DefaultGame.ini에 이미 켜져 있음); 코드 텍스트는 크고 고정폭.

## 범위 밖 (미룸, 이유 포함)
- 진행 중인 밤으로의 재접속 — 스폰/상태 복원이 필요; PreLogin 게이트가 깔끔하게
  닫아 둔다(TODOS P2).
- 로비에서 세이브/이어하기 — 세이브 시스템 목표에 속한다(TODOS P2).
- 심리스 트래블 + 밤마다의 로딩 진행 화면 — 밤마다 레벨을 옮기는 트래블이 없다.
- 전용 서버 / 호스트 이주 / 랜덤 매칭 / 공개 서버 목록 — 하지 않기로 결정
  (session.md, CCL8 D3/D5).
- Steam Deck 검증, 도전과제, 리더보드, 클라우드 세이브, 창작마당, 음성,
  Steam Input UI — 출시 후(TODOS P3/P4).
- 상점 아트, 트레일러, 가격, 마케팅 문구, 한국어 이외의 현지화.

## 기존에 있는 것 (재사용)
0B단계 참조: 참가 가능 축으로서의 밤 페이즈, `StartNight`, PlayerState OnRep 패턴,
Server RPC 패턴, UIManager 행, DefRegistry, CheatManager, 엔진 Steam 플러그인. 신규:
세션 서브시스템, PreLogin/PostLogin/Logout 게이트, GI 네트워크 실패 핸들러,
로스터/타이틀 위젯, 업로드 스크립트.

## 꿈의 상태 차이
이 계획 뒤: 친구가 Steam 친구 목록이나 코드로 참가한다; 모든 실패에 한글 메시지가
있다; 자체 AppID에서 베타 브랜치 루프가 동작한다. 12개월 이상향을 위해 남긴 것:
밤중 재접속, 세이브/이어하기, Playtest 피드백 루프, Steam Deck, 출시일 상점 작업.

## 실패 모드 등록부
```
  코드 경로                    | 실패 모드                         | 구조? | 테스트?        | 사용자가 보는 것?           | 로그?
  ----------------------------|----------------------------------|-------|----------------|----------------------------|--------
  OSS 초기화                  | Steam 클라이언트 꺼짐             | Y     | S1 (수동)      | 버튼 비활성 + 이유          | Y
  HostSession                 | 생성 실패 / 타임아웃              | Y     | S1             | 타이틀에 메시지             | Y
  JoinByCode                  | 잘못된 입력                       | Y     | 유닛           | 인라인 오류                 | Y
  JoinByCode                  | 검색 실패 / 타임아웃 / 0 / 여러 개| Y     | S3             | 메시지                     | Y
  JoinSession                 | 만원 / 사라짐 / 주소 / 알 수 없음 | Y     | S5             | enum별 메시지               | Y
  초대                        | 밤중 / 호스팅 중                  | Y     | S4, S5         | 메시지 / 무시               | Y
  PreLogin                    | 만원 / 해시 / 버전 / 페이즈       | Y     | 스모크 + S5    | ErrorMessage 텍스트         | Y
  PostLogin                   | 페이즈 경합                       | Y     | S5(참가 중 출발)| 메시지                     | Y
  세션 중                     | 호스트 종료 / 중계 끊김 / Steam 종료 | Y  | S6             | 연결 끊김 메시지            | Y
  DestroySession              | 실패                              | Y     | S6             | 없음                       | Y
  TuningHash                  | def 목록 없음                     | Y(기존)| 유닛          | (개발) 로그                | Y
  UpdateSession (참가 가능)   | 실패                              | Y     | S5(Join Game 없음) | 없음(게이트 최후 방어)  | Y
  GI 실패 핸들러              | LeaveSession 생략 → 고아 로비     | Y     | S6             | 친구가 낡은 Join Game을 봄  | Y
  SteamSockets 재호스트       | UE-239936 낡은 소켓 정보          | ?     | S2 두 번       | "세션을 만들지 못했습니다"  | Y
```
심각한 갭: 0 (UE-239936 행은 테스트와 눈에 보이는 메시지가 있는 알려진 미지수이지
조용한 실패가 아니다).

## TODOS.md
`D:\Work\unreal\CL9\TODOS.md`(신규 파일, gstack 형식)에 작성: 밤중 재접속(P2), 로비
세이브/이어하기(P2), Steam Deck 검증(P3), 도전과제/리더보드(P4).

## 구현 태스크 (CEO 단계)
- [ ] **T1 (P1, 사람: ~1시간 / CC: ~10분)** — W0 — 오늘 Steamworks 등록 시작(수수료 → 30일
  시계); 앱 생성; 계획에 AppID 기록
  - 근거: 0A단계 P7 — 검증한 출시 순서는 최소 7주 바닥이 있다
  - 파일: docs/goal/steam-release.md (AppID), Config/DefaultEngine.ini (나중)
  - 검증: Steamworks 앱 페이지 존재; 30일 카운트다운 보임
- [ ] **T2 (P1, 사람: ~2시간 / CC: ~15분)** — W1 — OSS 꺼짐 경로 추가: `IOnlineSubsystem::Get()`이
  null일 때 멀티플레이 비활성 + 이유
  - 근거: 2절 — 리뷰 전엔 첫 행이 갭이었다
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp, WBP_Title
  - 검증: `-nosteam`로 실행 → 멀티플레이 버튼 비활성, 이유 표시
- [ ] **T3 (P1, 사람: ~1시간 / CC: ~10분)** — W2 — `PostLogin` 페이즈 재확인 + 메뉴 복귀
  이유(PreLogin과 출발 사이의 경합)
  - 근거: 4절 — 클라이언트가 아직 연결 중일 때 출발을 누름
  - 파일: Source/CL9/GameMode/CL9GameMode.cpp
  - 검증: 스모크 스크립트 변형: StartNight 중 클라이언트 참가 → 메시지와 함께 돌아옴
- [ ] **T4 (P1, 사람: ~1시간 / CC: ~10분)** — W2 — 모든 `EOnJoinSessionCompleteResult` /
  `ENetworkFailure` / PreLogin 이유를 매핑하는 실패 문자열 표(`FText`)
  - 근거: 2절 — 포괄 처리 없음 규칙
  - 파일: Source/CL9/Session/CL9SessionSubsystem.h
  - 검증: S5/S6 시나리오마다 자기 문자열을 보임
- [ ] **T5 (P2, 사람: ~3시간 / CC: ~20분)** — W4 — 두 프로세스 `-nosteam` 스모크 스크립트 +
  pie-scenarios S0
  - 근거: 6절 — 게이트를 위한 유일한 자동화 가능 통합 테스트
  - 파일: Build/Smoke/two-process-smoke.ps1, docs/testing/pie-scenarios.md
  - 검증: 스크립트가 양쪽 로그에서 Night를 보여 주며 0으로 종료
- [ ] **T6 (P2, 사람: ~2시간 / CC: ~15분)** — W2 — `cl9.host` / `cl9.join <code>` / `cl9.leave` /
  `cl9.session` 치트(로컬, 호스트 경유 아님)
  - 근거: 0D단계 결정 3; 5절 라우팅 정정
  - 파일: Source/CL9/GameMode/CL9Cheats.cpp, CL9CheatManager.h
  - 검증: W3 UI 없이 W1 AC2 도달 가능
- [ ] **T7 (P2, 사람: ~2시간 / CC: ~15분)** — W1 — UE-239936을 위해 한 프로세스에서 AC2를
  두 번 반복(호스트 → 나가기 → 호스트)
  - 근거: 조사 — 알려진 SteamSockets 리슨 소켓 버그
  - 파일: docs/testing/pie-scenarios.md (S2b)
  - 검증: 두 번째 호스트 성공; 안 되면 타이틀 틱 우회 적용 후 재검증
- [ ] **T8 (P2, 사람: ~2시간 / CC: ~15분)** — W4 — 자동화 테스트: 코드 정규화, TuningHash
  결정성
  - 근거: 6절
  - 파일: Source/CL9/Tests/CL9SessionCodeTest.cpp
  - 검증: `Automation RunTests CL9` 통과
- [ ] **T9 (P2, 사람: ~1시간 / CC: ~10분)** — W3 — W1~W3이 GameMode/GameState/PlayerState/PC/GI를
  건드리기 전에 현재 UI 세션 작업이 커밋됐는지 확인
  - 근거: 시스템 감사 — 같은 파일이 더러운 작업 트리
  - 파일: (git) — 검증: W1 시작 전 Source/CL9/GameMode의 `git status`가 깨끗
- [ ] **T10 (P1, 사람: ~1시간 / CC: ~10분)** — W2 — 로비 참가 가능 플래그가 페이즈를
  따름(`StartNight`에서 `UpdateSession`의 `bAllowJoinInProgress` false, `EnterLobby`에서 true)
  - 근거: 스펙 리뷰 지적 3 — Steam 로비 참가 가능 = `bAllowJoinInProgress && members < max`
    (`OnlineSessionAsyncLobbySteam.cpp:447,657`)
  - 파일: Source/CL9/GameMode/CL9GameMode.cpp, Source/CL9/Session/CL9SessionSubsystem.cpp
  - 검증: AC5 — 밤중에 호스트에 "Join Game" 없음; 정차 때 돌아옴
- [ ] **T11 (P3, 사람: ~30분 / CC: ~5분)** — W5 — session.md "향후 구현 예정 → 랜덤 매칭"
  자기모순 제거; build-order 1f 메모 동기화; "게임 시작" → 출발 이름 변경
  - 근거: 시스템 감사 — review-2026-09-01 M-2; 스펙 리뷰 지적 6
  - 파일: docs/systems/session.md, docs/plan/build-order.md
  - 검증: "랜덤 매칭" grep이 "만들지 않는다" 줄만 찾음
- [ ] **T12 (P1, 사람: ~1시간 / CC: ~5분)** — W1 — W1 시작 전 두 번째 PC + 두 번째 Steam
  계정 확보(진입 조건); W1은 구세대 `SteamNetDriver` 폴백과 함께 두 세션으로 시간 제한
  - 근거: CEO 음성 지적 1 — 1f는 `058563a`(PlayerStart 배치)와 PIE로 닫혔다
  - 파일: docs/goal/steam-release.md (미결 사항 3 답)
  - 검증: 물리적 기계 두 대에서 AC2 실행, 양쪽 SteamID와 함께 로그
- [ ] **T13 (P2, 사람: ~1시간 / CC: ~10분)** — W3 — `EnterLobby`에서 준비 초기화; 호스트는
  준비가 필요 없음; 혼자 하기 = 주문 선택이 끝나면 출발 활성
  - 근거: 스펙 리뷰 지적 6
  - 파일: Source/CL9/GameMode/CL9GameMode.cpp, Source/CL9/GameMode/CL9PlayerState.cpp
  - 검증: 스모크 스크립트 — 두 번째 밤: 누르기 전까지 클라이언트 준비가 다시 false
- [ ] **T14 (P2, 사람: ~30분 / CC: ~5분)** — W1 — `[/Script/Engine.GameEngine]` 전용
  `NetDriverDefinitions` 변경 뒤 PIE가 IpNetDriver에 남는지(AC7) W1의 다른 무엇보다
  먼저 확인
  - 근거: CEO 음성 지적 2 — 폴백은 단언됐을 뿐 검증되지 않았다
  - 파일: Config/DefaultEngine.ini, docs/testing/pie-scenarios.md
  - 검증: PIE 2클라이언트 A3 통과; 로그가 PIE에서 `IpNetDriver`, `-game`에서
    `SteamSocketsNetDriver` 표시

## 이중 음성 (0.5단계)

**CODEX SAYS (CEO — 전략 도전):** `[codex-unavailable]` — 실제 프로브
2026-09-06이 "사용량 한도 도달 … 2026년 9월 14일에 다시 시도"를 돌려줬다
(codex-cli 0.136.0). 단계는 `[subagent-only]`로 표시.

**CLAUDE SUBAGENT (CEO — 전략적 독립성)** — 지적 8개, 원문 그대로:
1. (치명) 두 번째 PC가 미결인 채 W1의 완료 조건이 그것을 요구; 1f는 `058563a`
   "첫 것을 마주 보는 두 번째 PlayerStart" + PIE로 닫혔다. → **수용**: W1 진입
   조건 + 시간 제한(T12).
2. (높음) "`-nosteam`은 pie-scenarios.md에 문서화됨"은 거짓이었다; PIE가
   IpNetDriver로 폴백한다는 것은 단언만 됐다. → **수용**: 문구 수정; 메커니즘을
   이제 명시(`GameEngine` 전용 정의)하고 T14가 먼저 검증한다.
3. (높음) 세 질문의 답이 ④ 4e~4i와 세이브 시스템이 미완성인 채 여섯 항목짜리
   출시 프로그램이 됐다; W1을 스파이크로 분리하고 W2~W5를 미루자. →
   **부분 수용**: W1은 이미 첫 번째이고 이제 중단 기준을 가진다; W2~W4를
   미루는 것은 build-order.md가 세션 층이 ④ 시작 전에 있어야 한다고 했으므로
   **주 리뷰어가 기각** — 게이트용 **사용자 도전 1**로 넘김(개발자가 결정).
4. (높음) W0 "지금 Coming Soon"이 미결 1/2보다 앞서고 계획이 제외한 에셋을
   필요로 함. → **수용**: W0 = 등록 + 수수료 + AppID; 상점 페이지는 나중의
   별도 항목.
5. (중간) "서버 없음"의 비용이 과소평가됨; SDR 조건 미검증; 클라이언트 낙오는
   그 밤을 잃는다. → **부분 수용**: SDR 문구를 이제 "Steamworks 문서상, W0에서
   재확인"으로; 재접속은 의존성이 있는 명명된 TODO.
6. (중간) 단일 레벨 가정 vs 4g. → **증거로 기각**: 4g는 같은 레벨 안에서 사이트
   행마다 생성기를 고른다(`ECL9Generator`, "목적지 하나에 생성기 하나"); 서브시스템은
   어차피 레벨에 묶이지 않게 유지.
7. (중간) 필터 전 AppID 480 검색 상한. → **증거로 기각**: OSS Steam은 결과 수
   상한 전에 서버 측 문자열 필터를 적용한다(`OnlineSessionAsyncLobbySteam.cpp:838-902`);
   0/1/여러 개 결과 전부 처리됨.
8. (중간) 대안/시장: Playtest/데모/Next Fest vs 출시, 솔로 우선, 구세대 넷드라이버
   폴백이 저울질되지 않음. → **부분 수용**: 구세대 드라이버가 이제 W1 폴백;
   "첫 산출물 = Playtest/데모?"는 게이트의 **취향 선택 3**으로 넘김; 상점 페이지
   마케팅은 이 계획 밖에 둔다.

**CEO 문서의 스펙 리뷰** (독립 서브에이전트, 1차: 5/10, 지적 11개; 전부 계획과
CEO 문서에서 고쳤고 하나는 근거와 함께 기각): `bInitServerOnClient` 불필요(수용 —
엔진 소스 재확인, W1 정정); 리치 프레즌스는 현지화 토큰 필요(수용 — 보류); 로비
참가 가능 플래그 누락(수용 — T10); 낡은 `SessionState` 문구(수용 — 재작성);
`OfferedOrders`-비어있음 게이트가 주문 0개일 때 통과(**기각**: 하루에 주문 행이
없을 때의 의도된 비-소프트락 동작; 문서화); 준비 초기화 의미(수용 — T13); 밤 사이
참가 명확화(수용 — AC6); 치트 로컬 전용(수용); 정의 안 된 라벨(범례 추가); 스모크
스크립트 문구(수용); Valve 숫자(수용 — W0에 날짜 명시).

```
CEO 이중 음성 — 합의 표:
═══════════════════════════════════════════════════════════════
  차원                                   Claude   Codex   합의
  ────────────────────────────────────  ───────  ──────  ─────────
  1. 전제가 타당한가?                    부분     N/A     부분 → 2개 수정(P7, P8) + 소스 재확인 2회
  2. 옳은 문제를 푸는가?                 부분     N/A     부분 → 사용자 도전 1
  3. 범위 보정이 맞는가?                 아니오   N/A     불일치 → 사용자 도전 1
  4. 대안을 충분히 탐색했는가?           아니오   N/A     불일치 → 취향 선택 3(Playtest/데모 먼저)
  5. 경쟁/시장 위험을 다뤘는가?          아니오   N/A     해당 없음 — 상점 페이지는 의도적으로 범위 밖
  6. 6개월 궤적이 건전한가?              부분     N/A     부분 → W1 분리 + 중단 기준 채택
═══════════════════════════════════════════════════════════════
0/6 확인됨(두 번째 모델 없음), 2개 불일치 → 게이트, 치명적 지적 1개 채택.
```

## 결정 감사 기록 (1단계)

| # | 단계 | 결정 | 분류 | 원칙 | 근거 | 기각됨 |
|---|-------|----------|----------------|-----------|-----------|----------|
| 1 | 0 | 계획 파일 = docs/goal/steam-release.md, 영어 goal 템플릿 | 기계적 | P3 | 이전 /autoplan 실행(CCL8 docs/goal)과 goaldoc 관례에 맞춤 | 스크래치패드 전용 계획 |
| 2 | 0 | UI 범위 예, DX 범위 아니오 | 기계적 | P3 | 소비자용 게임; "SDK/API" 검색 결과는 소비하는 의존성 | 2.5단계 DX 리뷰 |
| 3 | 0.5 | Codex 음성 사용 불가 → subagent-only | 기계적 | P6 | 실제 프로브: 2026-09-14까지 사용량 한도 | 8일 대기 |
| 4 | 1-0A | 전제 P7(짧은 Valve 시계) 틀림 → W0 지금 시작 | 기계적 | P1 | 검증한 7주 이상 바닥 | — |
| 5 | 1-0A | 전제 P8(1f 완료) 틀림 → W1이 진짜 1f | 기계적 | P1 | `058563a` + PIE 전용 | — |
| 6 | 1-0C-bis | 접근 B(계획)를 A(초대 전용)와 C(+세이브/재접속)보다 선택 | 기계적 | P1/P2 | 영향 범위 안 9/10; C는 세이브 목표를 끌어들임 | A(4/10), C |
| 7 | 1-0F | 모드 선택적 확장 | 기계적 | 오버라이드 | autoplan CEO 오버라이드; 맥락 기본값이기도 함 | — |
| 8 | 1-0D | 치트, 복사/붙여넣기, 스모크 스크립트, ProjectVersion, 출발 게이트 수용 | 기계적 | P2 | 영향 범위 안, 각각 CC 하루 미만 | — |
| 9 | 1-0D | `ECL9SessionState` 제거, HUD 레이어 로스터 | **취향** | P5 | 더 단순; session.md에서 벗어남 → 게이트 선택 1 | 세션 enum 유지 |
| 10 | 1-0D | 재접속, 세이브/이어하기, Deck, 도전과제를 TODOS로 미룸 | 기계적 | P3 | 영향 범위 밖 | 지금 만듦 |
| 11 | 1-0D | "다시 호스트" 단축 생략 | 기계적 | P5 | 안 물어본 분기 | — |
| 12 | 1-0D | 리치 프레즌스: 수용 → 스펙 리뷰 뒤 보류 | 기계적 | P1 | `#토큰`이 Steamworks 현지화 업로드 필요; 480에서 불가능 | W2에 유지 |
| 13 | 1-S2 | OSS 꺼짐 경로, PostLogin 경합 방어, 실패 문자열 표 추가 | 기계적 | P1 | 조용한 실패 0 | — |
| 14 | 1-S5 | 새 치트는 로컬 전용, 호스트 경유 아님 | 기계적 | P5 | `bLocalOnly` 경로가 이미 있음(`CL9Cheats.cpp:86`) | — |
| 15 | 1-voice | 두 번째 PC = W1 진입 조건; W1 시간 제한 + 구세대 드라이버 폴백 | 기계적 | P1 | CEO 음성 지적 1/3/8 | — |
| 16 | 1-voice | W0 = 등록만; 상점 페이지 별도 | 기계적 | P3 | CEO 음성 지적 4 | 지금 Coming Soon |
| 17 | 1-voice | W2~W4를 4e~4i + 세이브 이후로 미룰지? | **사용자 도전** | — | build-order가 세션 층이 늦었다고 함 — 개발자가 결정 | — |
| 18 | 1-voice | 첫 Steam 산출물 Playtest/데모 vs 출시 | **취향** | — | 전략적; 코드 영향 없음 → 게이트 선택 3 | — |
| 19 | 1-voice | 4g가 단일 레벨을 깨는가? | 기계적 | P5 | 증거로 기각(사이트 행별 `ECL9Generator`) | 트래블 유지 |
| 20 | 1-spec | `bInitServerOnClient` 필요한가? | 기계적 | P5 | 소스 재확인 뒤 내 초안을 기각; 거짓 | true 유지 |
| 21 | 1-spec | 로비 참가 가능 플래그가 페이즈를 따름 | 기계적 | P1 | 엔진 동작 검증; T10 | — |
| 22 | 1-spec | `OfferedOrders`-비어있음 게이트 vs 명시적 수락 플래그 | 기계적 | P5 | 명시적 플래그는 주문 행 없는 날 소프트락 | 명시적 플래그 |
| 23 | 1-spec | 준비 초기화 규칙, 단일 라벨 출발, 밤 사이 새 참가 허용 | 기계적 | P1 | 명시되지 않은 의미를 명확히 함 | — |
| 24 | 1 | 밤중 참가 = v1은 거부 | **취향** | P5 | session.md가 미결로 둠 → 게이트 선택 2 | 허용 |
| 25 | 1 | VAC 꺼짐 | **취향** | P3 | 공개 세션 없음 → 게이트 선택 4 | VAC 켬 |
| 26 | 1 | TODOS.md를 저장소 루트에 생성(5항목) | 기계적 | P3 | gstack 형식; 저장소에 없었음 | docs/ 위치 |

## 완료 요약 (1단계)
```
  +====================================================================+
  |            메가 계획 리뷰 — 완료 요약                              |
  +====================================================================+
  | 선택된 모드           | 선택적 확장 (autoplan 오버라이드)            |
  | 시스템 감사           | 세션/Steam 커밋 없음; 같은 GameMode 파일에    |
  |                      | 더러운 트리; 1f = PIE 전용                   |
  | 0단계                | 전제 8개(2개 틀림, 수정); 접근 B;             |
  |                      | 범위 제안 19개                               |
  | 1절 (아키텍처)        | 지적 0(다이어그램 + 상태 기계 2개)            |
  | 2절 (오류)            | 오류 경로 18개 매핑, 리뷰 뒤 갭 0             |
  | 3절 (보안)            | 위협 10개, 높음 심각도 0                     |
  | 4절 (데이터/UX)       | 경계 사례 16개 매핑, 미처리 0                 |
  | 5절 (품질)            | 지적 1(치트 라우팅) — 수정                   |
  | 6절 (테스트)          | 다이어그램 생성, 갭 0(유닛 2, 통합 1, E2E 6) |
  | 7절 (성능)            | 지적 0                                       |
  | 8절 (관측)            | 갭 0(cl9.session + 로그 줄 명시)             |
  | 9절 (배포)            | 위험 1건(혼합 버전) — 게이트됨               |
  | 10절 (미래)           | 가역성: 3/5, 부채 항목: 2                    |
  | 11절 (디자인)         | 메모 3개 → 2단계 깊은 리뷰                   |
  +--------------------------------------------------------------------+
  | 범위 밖               | 작성됨(6항목)                                 |
  | 기존에 있는 것        | 작성됨                                       |
  | 꿈의 상태 차이         | 작성됨                                       |
  | 오류/구조 등록부       | 코드 경로 14개, 심각한 갭 0                  |
  | 실패 모드              | 총 13개, 심각한 갭 0(알려진 미지수 1)        |
  | TODOS.md 갱신          | 5항목 작성됨                                 |
  | 범위 제안              | 19개 제안, 12개 수용, 5개 보류, 1개 생략     |
  | CEO 계획               | 작성됨(~/.gstack/projects/Work/ceo-plans)    |
  | 외부 음성              | 실행됨(claude 서브에이전트) [subagent-only] |
  | 완전성 점수            | 6/6 권장이 완전한 옵션을 선택함              |
  | 다이어그램 생성        | 5개(아키텍처, 상태 기계 2개, 데이터 흐름,    |
  |                      | 사용자 흐름)                                 |
  | 낡은 다이어그램 발견   | 0(session.md ASCII 화면 → W5 재작성)        |
  | 미해결 결정            | 진행 중 0; 취향 4 + 사용자 도전 1 → 게이트  |
  +====================================================================+
```

> **1단계 완료.** Codex: 사용 불가. Claude 서브에이전트: 지적 8개(5개 수용, 2개
> 증거로 기각, 1개 → 사용자 도전). 스펙 리뷰: 지적 11개(10개 수정, 1개 근거로
> 기각). 합의: 0/6 확인(단일 모델), 불일치 2개 → 게이트로 넘김. 2단계로
> 넘어감(디자인 — UI 범위 감지).

---

# /autoplan — 2단계: 디자인 리뷰 (자동 결정)

범위 게이트: 계획 모드/autoplan — B 자동 선택(이 계획의 W3 화면 리뷰). 목업:
gstack designer 바이너리는 설치돼 있지만 OpenAI 키가 설정되지 않음(`$D setup`이
안내를 찍는다) → **생성 목업 대신 텍스트 와이어프레임**(점진적 개선이지 게이트가
아님).

## 0단계 — 디자인 범위
- **0A 초기 평점: 5/10.** 계획은 화면을 이름 붙였고, 상태 표와 사용자 흐름
  다이어그램(11절)이 있으며, 참가 코드가 핵심 요소임을 알지만 — 로스터가 HUD
  그리드의 *어디*에 앉는지, 로스터 행이 어떻게 생겼는지, 실패 메시지가 어디에
  나타나는지, 포커스 순서, 프로젝트의 1080p/24pt 배율의 크기에 대해서는 아무
  말이 없다. 이 계획의 10점: ui.md의 HUD 구역에 놓인 화면별 ASCII 와이어프레임
  하나씩, 실제 한글 문구가 있는 상태 표, 로스터 행 구조, 키보드 포커스 순서, 결정된
  메시지 표면("타이틀 + 배너"라는 손짓 아닌).
- **0B DESIGN.md: 없음.** `DESIGN.md`도 `docs/designs/`도 없다. 나중에
  `/design-consultation`을 권장; 이 계획을 위한 사실상의 시스템은 ui_system.md +
  ui.md다: `WBP_Main` 하나, HUD 레이어 항상 켜짐, 배타 Screen 레이어, 토스트/대화상자용
  Popup 레이어, 위젯은 1080p × 1.0에 24pt 기본 텍스트로 디자인, 전역
  `ApplicationScale` 1.4, 최소 16pt, HUD 구역(위: 낮/밤 시계 · 좌상단: HP/스태미나/
  이더 · 좌하단: 주문 · 하단 중앙: 프롬프트 · 우하단: 소지 슬롯). **오른쪽은
  비어 있다.**
- **0C 기존 활용:** `WBP_Spectate`(작은 상단 중앙 패널: 제목 한 줄 + 안내 한 줄 —
  로스터가 그 절제를 따라 함), `WBP_Pause`(수직 메뉴 목록 — Title/ModeSelect가 그
  목록 스타일을 따라 함), `WBP_OrderSelect`(런타임 생성 행 — 로스터 행도
  `PlayerArray`에서 같은 방식으로 만들어짐), `WBP_ScanLabel`(라벨 풀 + 자동 소멸
  애니메이션 — Notice 토스트가 그 페이드를 재사용 가능), UI 표 행 + `CL9UINames`.
- **0D 초점 영역:** 일곱 개 전부(P1).

## 1패스 — 정보 구조: 4/10 → 9/10
플레이어가 처음, 두 번째, 세 번째로 보는 것 — 이제 화면별로 명시:
```
 타이틀 (Lvl_Title: 밤의 트럭, 램프 켜짐, 카메라 고정; 메뉴는 왼쪽 3분의 1)
 ┌──────────────────────────────────────────────────────────────┐
 │  양계장 습격                       (제목 48pt, 좌상단)         │
 │                                                              │
 │  ▶ 혼자 하기                       (목록 24pt, 왼쪽 열)        │
 │    방 만들기                       친구를 초대하거나 코드를 알려줍니다 │
 │    코드로 참가                     친구가 알려준 6자 코드           │
 │    설정                                                       │
 │    종료                                                       │
 │  친구 게임은 Steam 친구 목록의 '게임 참가'로       (16pt 힌트)  │
 │  Steam: 연결됨 · <닉네임>      v0.1.0     (16pt, 좌하단)      │
 └──────────────────────────────────────────────────────────────┘
 1번: 게임 이름 + 트럭(정체성) · 2번: 혼자 하기(기본 포커스) · 3번: Steam 상태
 줄 — 첫 클릭 전의 신뢰. 모드 선택 경유 없음(디자인 리뷰 결정 7): 호스팅은
 타이틀에서 한 클릭이다.
 Steam 꺼짐: 방 만들기 / 코드로 참가 흐려짐; 상태 줄이 "Steam이 실행 중이
 아닙니다 — 멀티플레이 비활성"; 혼자 하기는 계속 동작.

 참가 코드 (Popup 행, 중앙, 560×260 @1.0):
 ┌───────────────────────────────┐
 │  코드 입력                     │
 │  ┌─┬─┬─┬─┬─┬─┐                 │  여섯 칸, 40pt, 자동 대문자
 │  │K│7│M│X│4│P│                 │
 │  └─┴─┴─┴─┴─┴─┘                 │
 │  (상태 줄 — 상태 참조)         │
 │  [붙여넣기]      [참가]  [취소] │  Enter = 참가 · Esc = 취소
 └───────────────────────────────┘

 연결 중 (같은 Popup, 내용만 바뀜): 스피너 · "연결 중… 호스트: <이름>" · [취소]
 · 20초 참가 타이머. 실패가 다른 표면으로 튕기지 않는다: 패널이 입력한 코드를
 유지한 채 코드 칸으로 돌아가고, 이유는 인라인이며, [참가]가 다시 활성화된다 —
 읽을 곳 하나, 다시 누를 키 하나.

 로스터 (HUD 레이어, 우상단, 420×300 @1.0; 페이즈 == Lobby일 때만 보임;
 표시 전용 — HUD 패널엔 커서가 없으므로 조작은 키다)
 ┌ 로비 ──────────────────────────────┐
 │ 코드  K7MX4P          C: 코드 복사   │  코드 40pt, 화면에서 가장 큰 텍스트
 │ 1 ● 나 (호스트)                     │  슬롯 번호 + 슬롯 색 점(TintColor); 호스트는 준비 표시 없음
 │ 2 ● Player2                 준비 ✓  │
 │ 3 ● 아주긴이름아주긴이름아주…   대기  │  14자에서 자름
 │ 4 ○ 빈 자리 — 초대하세요            │
 │ 2/4 준비 · 주문 확정 전              │  상태 줄 = 왜 출발이 아직 안 되는지
 │ Enter: 출발 (호스트)  /  Enter: 준비 (클라이언트)│  키 힌트 줄
 └────────────────────────────────────┘
 1번: 코드(친구에게 뭘 알려줄지) · 2번: 누가 있고 준비됐는지 · 3번: 키 하나.
 혼자 하기: "혼자 · Enter: 출발"만. 참가/이탈 → 3초 토스트("Player2 님이
 들어왔습니다")로 다른 곳을 보는 호스트도 알 수 있게.

 NOTICE (Popup 행 `Notice`, 재사용 가능): 모달 한 줄 + [확인](호스트 이탈,
 PostLogin 추방); 토스트 모드는 3초 뒤 자동 소멸(복사됨, 참가, 이탈).
```
제약 존중: 로스터가 세 가지만 보여 줄 수 있다면 — 코드, 준비 인원, 출발 — 그렇게
한다; 이름은 네 번째다.

## 2패스 — 상호작용 상태 커버리지: 6/10 → 9/10
| 기능 | 로딩 | 비어 있음 | 오류 | 성공 | 부분 |
|---|---|---|---|---|---|
| Title | — (OSS 초기화는 모듈 시작 때 발생; 타이틀이 열릴 때는 이미 상태를 앎) | — | Steam 꺼짐 → 방 만들기/코드로 참가 흐려짐 + 상태 이유; 혼자 하기는 동작 | 상태 "Steam: 연결됨 · 닉네임" | — |
| 참가 코드 | 칸 잠김, 상태 "검색 중…" | 빈 입력 → 참가 비활성, 상태 "친구가 알려준 6자 코드" | 인라인, **코드 유지**: "잘못된 코드입니다" / "검색에 실패했습니다 — 다시 시도" / "방이 가득 찼습니다 (4/4)" / "버전이 다릅니다 — 둘 다 업데이트하세요" / "게임 데이터가 호스트와 다릅니다 — 게임 파일을 확인하세요" / "밤이 진행 중입니다 — 다음 정차 때 들어오세요" | 패널이 연결 중으로 전환 | 1~5자 입력 → 참가 비활성, 아직 오류 없음 |
| 연결 중 | 스피너 + "연결 중… 호스트: 이름" + [취소] | — | 이유가 인라인으로 칸에 돌아옴(같은 패널) | 패널이 닫히고 로스터 등장 | 20초 참가 타이머 → "연결 시간이 초과됐습니다", 칸으로 돌아감 |
| Roster (호스트) | — | 혼자(멀티플레이 호스트): "4 ○ 빈 자리 — 초대하세요" 행 + 코드; 솔로: "혼자 · Enter: 출발" | — | 전원 준비 + 주문 선택 → "Enter: 출발" 힌트가 켜짐 | "k/4 준비 · 주문 확정 전" 줄이 막힌 이유를 말함 |
| Roster (클라이언트) | — | — | — | 복제 한 틱 안에 자기 행에 준비 ✓ | 호스트가 시작 안 함 → "호스트 대기 중" |
| Roster (아무나) | — | — | 플레이어 이탈 → 행이 "빈 자리"로 돌아감 + 토스트 "…나갔습니다" | 플레이어 참가 → 행이 채워짐 + 토스트 "…들어왔습니다"(UI_Confirm) | — |
| 코드 복사 | — | — | — | 토스트 "복사됨" | — |
| 밤 시작 | — | — | — | 페이즈 Night에서 로스터가 숨음(기존 조명 변화가 피드백) | — |
| 호스트 이탈/추방 | — | — | Notice "호스트와의 연결이 끊어졌습니다" [확인] → 타이틀 | — | — |
모든 비어 있는 상태에 다음 동작이 있다(초대하세요 · 코드); 모든 오류가 원인과
복구를 이름 붙인다.

## 3패스 — 사용자 여정과 감정 곡선: 5/10 → 8/10
| 단계 | 사용자가 함 | 사용자가 느낌 | 계획이 명시 |
|---|---|---|---|
| 1 | Steam에서 게임을 연다 | "이거 되는 건가?" | 타이틀 상태 줄: Steam 연결됨 + 닉네임(1초 안에 신뢰) |
| 2 | 멀티플레이 → 호스트로 플레이 | 한 가지 뻔한 선택 | 호스트로 플레이가 기본 포커스; 부제가 무엇을 하는지 말함 |
| 3 | 트럭 패드에 도착, 로스터가 코드를 보여 줌 | "친구에게 뭘 말해야 하지?" | 코드가 가장 큰 텍스트; [복사]; "초대하세요" 힌트; Steam 친구 목록이 다른 경로 |
| 4 | 친구가 게임 참가를 클릭 / 코드 입력 | 기다림… | 친구의 행이 이름 + 슬롯 색과 함께 등장(안도) |
| 5 | 친구가 준비를 누름 | 기대 | ✓가 나타남; 상태 줄이 k/4를 셈 |
| 6 | 호스트가 주문을 확정하고 출발을 누름 | "간다" | 밤 조명 변화(기존)가 보상; 로스터가 숨음 |
| 단절 A | 호스트가 혼자 2분 기다림 | 의심 | 수정: 빈 슬롯 행이 초대하세요라고 말함; 코드는 계속 보임; 스피너 없음 |
| 단절 B | 참가 실패 | 좌절 | 수정: 이유 + 버튼 하나, 같은 패널; 조용한 복귀 없음 |
| 단절 C | 밤중 호스트 종료 | 혼란 | 수정: 이유가 담긴 Notice, [확인] → 타이틀 |
5초: 트럭 + 이름. 5분: 코드 → 친구 행 → 출발. 5년: 로스터는 밤마다 똑같아서 —
출발 전의 의식이 된다.

## 4패스 — AI 슬롭 위험: 6/10 → 9/10
분류: **앱 UI**(게임 내 패널). 즉시 실격 패턴 확인: 카드 그리드 없음(로스터 행은
카드가 아니라 행), 텍스트 뒤 바쁜 이미지 없음(패널은 장면 위 흐린 스크림에
앉음), 장식용 그림자 없음, 강조색 하나(어두운 장면 위 따뜻한 램프 앰버), 유틸리티
문구만("코드 · 준비 · 출발"). 자체 리트머스: 브랜드가 확실함(트럭 + 제목) 예 ·
시각적 닻 하나(코드) 예 · 헤드라인만으로 훑어 읽힘 예 · 패널당 할 일 하나 예 ·
카드 필요 해당 없음 · 움직임이 위계를 개선함(Notice 페이드가 ScanLabel 재사용) 예
· 그림자 없이도 고급스러움 예. 서체: 프로젝트의 기존 UI 폰트(system-ui 아님); 코드는
헷갈리지 않는 알파벳과 함께 같은 서체를 40pt로 쓰므로 별도 고정폭 서체가
필요 없다.

## 5패스 — 디자인 시스템 정합: 5/10 → 8/10
DESIGN.md 없음(갭 → `/design-consultation` 권장, TODO). 사실상의 시스템과 정합:
모든 새 표면이 UI 표 행이고(`Lvl_Title`의 `Title`, `ModeSelect` Screen 행;
`JoinCode`, `Notice` Popup 행; `WBP_HUD` 안이나 HUD 레이어 행인 `Roster` 요소 —
아래 결정), 24pt 기본 / 16pt 최소 / 1080p 디자인 / `ApplicationScale` 1.4는
그대로, Screen 배타성 보존(주문 선택이 여전히 로스터 위에 열림), OnRep 기반
갱신(틱 폴링 없음). 새 어휘: **Notice**(한 줄 팝업)와 **Roster**(HUD 패널) —
컴포넌트 둘, 둘 다 재사용 가능.

## 6패스 — 반응형과 접근성: 3/10 → 8/10
- 입력: 지금은 마우스 + 키보드; 모든 Screen/Popup에 포커스 순서(Title: 혼자 하기
  → 방 만들기 → 코드로 참가 → 설정 → 종료; JoinCode: 칸 → 참가 → 취소), Enter/Esc
  바인딩, CommonUI 포커스 cvar는 DefaultGame.ini에 이미 켜져 있음. 로스터는
  **포커스가 없다** — HUD 레이어에 표시 전용이며 두 액션은 `IMC_Gameplay`의
  `IA_LobbyAction`(Enter)과 `IA_CopyCode`(C)로, 페이즈가 `Lobby`일 때만 활성(E
  상호작용, R 품, F 손전등, Tab 관전, M 메뉴와 충돌 없음). 게임패드는 나중에
  같은 두 액션에 바인딩; 힌트 줄은 리터럴이 아니라 매핑에서 바인딩된 키를
  읽는다.
- 해상도: 1080p × 1.0으로 디자인, `ApplicationScale`로 배율; 로스터는 24px
  여백으로 우상단에 고정돼 720p/1440p에서도 화면 안에 머문다; 텍스트는 16pt
  아래로 내려가지 않는다.
- 색맹 안전 슬롯: 슬롯 **번호** + 색 점(색만으로는 절대 유일한 신호가 아님);
  준비 = ✓ 기호 + "준비" 텍스트, 색이 아님.
- 대비: 어두운 스크림 위 24pt 밝은 텍스트 ≥ 4.5:1; 앰버 강조색은 활성화된 출발
  버튼과 코드에만.
- 스크린 리더: 엔진 안에서는 해당 없음; 텍스트는 나중의 현지화를 위해 (구운
  이미지가 아니라) 실제 텍스트로 남는다.

## 7패스 — 미해결 디자인 결정 (자동 결정, P5 명시)
| 결정 | 선택함 | 미루면 |
|---|---|---|
| 로스터 배치 | HUD 레이어, 우상단(유일한 빈 HUD 구역) | 구현자가 주문 선택 위 화면 중앙에 놓음 |
| `WBP_HUD` 자식 vs 자체 HUD 레이어 UI 표 행으로서의 로스터 | 자체 행 `Roster`(Layer=HUD)를 phase == Lobby일 때 HUD가 열고 Night에 닫음 — `WBP_HUD`를 건드리지 않음(더러운 UI 세션) | 리팩터 중간에 로스터 로직이 WBP_HUD에 뒤엉킴 |
| 로스터 조작: 버튼 vs 키 | 키(`IA_LobbyAction` Enter, `IA_CopyCode` C) — ui_system.md의 입력 규칙 아래 HUD 패널엔 커서가 없다; 대안 "포커스 프롬프트를 통한 트럭 운전석의 E" → 게이트 선택 5 | 클릭 불가능한 버튼이 출시됨 |
| 실패 메시지 표면 | 재사용 가능한 `Notice` Popup 행 하나(모달 + 토스트); 코드 경로 실패는 코드를 유지한 채 JoinCode 칸으로 돌아감; `GameInstance.PendingNotice`가 `OpenLevel(Title)`을 넘어 세션 중 실패를 나른다 | 배너 세 종류; 레벨 변경 때 메시지 소실 |
| 타이틀 구조 | 다섯 항목, ModeSelect 없음; Steam 친구 힌트 줄 | 전문 용어("멀티플레이 → 호스트")가 있는 두 단계 메뉴 |
| Steam 없이 혼자 하기 | 동작함: 리슨 레벨, 세션 호출 없음 | `-nosteam`이 솔로 런을 시작 못함 |
| 참가 타임아웃 | Connecting에서 20초 게임 측 타이머; 엔진의 80/120초는 그대로 | 80초짜리 스피너 |
| 코드 표현 | 여섯 칸, 40pt, 프로젝트 UI 폰트, 헷갈리지 않는 알파벳, 자동 대문자, [복사]/[붙여넣기] | 플레이스홀더-라벨인 텍스트 박스 |
| 슬롯 정체성 | 번호 + TintColor 점; 이름은 14자에서 자르고 툴팁 | 47자 이름이 행을 깬다 |
| 혼자 하기 로스터 | 코드 없음, 빈 행 없음, "혼자" + 출발 | 솔로 런에서 호스트가 "초대하세요"를 봄 |
| 타이틀 아트 | `Lvl_Title`: 게임의 트럭 패드 액터 + 램프, 고정 카메라, 밤 | 메뉴가 있는 검은 화면(슬롭) |
| 세션 나가기 | `WBP_Pause` [로비로 나가기](ui.md에 이미 예약) → `LeaveSession` → 타이틀 | 네 번째 "나가기" 표면 |

## 범위 밖 (디자인)
- 게임패드 레이아웃/글리프(입력 매핑은 있음; 시각 글리프는 나중).
- 설정 화면 내용(settings.md — 별도).
- 캐릭터 선택기 아트(오늘은 캐릭터 하나).
- 한국어 이외의 현지화(어디나 FText를 써서 문을 열어 둠).

## 기존에 있는 것 (디자인)
`WBP_Spectate` 패널의 절제, `WBP_Pause` 목록, `WBP_OrderSelect` 런타임 행,
`WBP_ScanLabel` 페이드, UI 표 / 레이어, `ApplicationScale` 1.4, ui.md의 HUD 구역
지도, plan/data/character.md의 슬롯 `TintColor`.

## 디자인 TODOS
- ⑤ 아트가 플레이스홀더를 대체하기 전에 `/design-consultation` → DESIGN.md(폰트,
  간격 배율, 강조색) — P3(TODOS.md에 추가).

## 구현 태스크 (디자인 단계)
- [ ] **D1 (P1, 사람: ~2시간 / CC: ~15분)** — W3 — `Notice` Popup 행 + 위젯(모달 한 줄
  + [확인]; 3초 자동 소멸 토스트 모드, `UI_Confirm` 사용); `UCL9GameInstance::PendingNotice`를
  타이틀이 도착 때 보여 줌
  - 근거: 1/2패스; 디자인 음성 지적 4/5 — 메시지 표면 하나, `OpenLevel`을 넘는
    운반체
  - 파일: Content/UI/WBP_Notice, DA_UITable, Source/CL9/GameMode/CL9GameInstance.{h,cpp},
    Source/CL9/UI/CL9NoticeWidget.{h,cpp}
  - 검증: S6 호스트 종료 → 타이틀이 Notice 표시 → 확인; 호스트에게 참가/이탈
    토스트
- [ ] **D2 (P1, 사람: ~3시간 / CC: ~20분)** — W3 — `Roster` HUD 레이어 행, 표시 전용:
  코드 + "C: 코드 복사", 행 4개(번호 · TintColor 점 · 14자로 자른 이름 · 준비 ✓ /
  대기; 호스트 행은 준비 없음), 막힌 이유를 말하는 상태 줄, 키 힌트 줄; phase
  Lobby에서 HUD가 열고 Night에 닫음; 솔로 변형 "혼자 · Enter: 출발"
  - 근거: 1/2/7패스; 디자인 음성 지적 1/3/8
  - 파일: Content/UI/WBP_Roster, DA_UITable, Source/CL9/UI/CL9RosterWidget.{h,cpp}
  - 검증: 스모크 스크립트 스크린샷; 47자 이름으로 S2; 솔로 런은 코드 안 보임
- [ ] **D3 (P2, 사람: ~2시간 / CC: ~15분)** — W3 — `JoinCode` Popup: 여섯 칸, 자동
  대문자, 알파벳 필터, [붙여넣기], 상태 줄 상태, 연결 중 내용으로 전환(20초 타이머,
  취소)하고 코드 경로 실패 시 **코드를 유지한 채 돌아옴**, Enter/Esc
  - 근거: 2/6패스; 디자인 음성 지적 4/9
  - 파일: Content/UI/WBP_JoinCode, Source/CL9/UI/CL9JoinCodeWidget.{h,cpp}
  - 검증: 소문자 + 공백 입력 → 정규화; 결과 0개 → "잘못된 코드입니다" 인라인,
    코드는 여전히 있음; 만원 → "(4/4)" 인라인
- [ ] **D4 (P2, 사람: ~2시간 / CC: ~15분)** — W3 — 트럭 패드 + 램프, 고정 카메라의
  `Lvl_Title`; `Title` Screen 행: 다섯 항목(혼자 하기 / 방 만들기 / 코드로 참가 /
  설정 / 종료), Steam 친구 힌트 줄, Steam 상태 줄, 포커스 순서; Steam 꺼짐은
  온라인 항목 둘을 흐리고 혼자 하기는 계속 시작됨
  - 근거: 1/3/4패스; 디자인 음성 지적 3/7/8
  - 파일: Content/Maps/Lvl_Title.umap, Content/UI/WBP_Title, Source/CL9/UI/CL9TitleWidget.{h,cpp}
  - 검증: `-nosteam` → 혼자 하기 실행, 온라인 항목은 이유와 함께 흐림; 키보드만
    으로 탐색 가능
- [ ] **D6 (P1, 사람: ~1시간 / CC: ~10분)** — W3 — `IMC_Gameplay`의 `IA_LobbyAction`
  (Enter: 준비 토글 / 역할별 출발)과 `IA_CopyCode`(C), phase Lobby에서만 활성;
  PC가 준비 → `Server_SetReady`, 출발 → 호스트 `StartNight` 경로로 라우팅
  - 근거: 디자인 음성 지적 1 — HUD 패널엔 커서가 없음
  - 파일: Content/Input/Actions/IA_LobbyAction, IA_CopyCode, IMC_Gameplay,
    Source/CL9/GameMode/CL9PlayerController.{h,cpp}
  - 검증: 스모크 스크립트: 클라이언트 Enter → 호스트에서 준비 ✓; 전원 준비 상태로
    호스트 Enter → Night
- [ ] **D7 (P2, 사람: ~1시간 / CC: ~10분)** — W3 — `WBP_Pause` [로비로 나가기] →
  `LeaveSession` → 타이틀(호스트: DestroySession; 클라이언트: 나가기)
  - 근거: 7패스 — ui.md가 이미 예약한 나가기 표면
  - 파일: Content/UI/WBP_Pause, Source/CL9/UI/*Pause*, Source/CL9/Session/CL9SessionSubsystem.cpp
  - 검증: S6 — 클라이언트 이탈 → 호스트 로스터 행 비고 + 토스트; 호스트 이탈 →
    클라이언트가 Notice를 받음
- [ ] **D5 (P3, 사람: ~1시간 / CC: ~10분)** — W5 — ui.md: 로스터 구역(우상단),
  Notice/Roster/JoinCode 행, 로비 입력 액션 둘; input.md: `IA_LobbyAction`,
  `IA_CopyCode` 행; session.md: 모드 선택 없는 타이틀
  - 근거: 5패스; 디자인 음성 지적 7
  - 파일: docs/systems/ui.md, docs/systems/ui_system.md, docs/systems/input.md,
    docs/systems/session.md
  - 검증: 문서가 모든 새 행과 액션을 나열함

## 이중 음성 (디자인)

**CODEX SAYS (디자인 — UX 도전):** `[codex-unavailable]`(사용량 한도, 1단계
참조). `[subagent-only]`.

**CLAUDE SUBAGENT (디자인 — 독립 리뷰)** — 2단계 패스가 쓰이기 *전*의 계획을
읽음(W3 + 11절만 존재했을 때). 지적 9개:
1. (중대) HUD 레이어 로스터 버튼이 ui_system.md의 입력 규칙(Screen 계층이
   비어있지 않을 때만 커서) 아래에서 클릭 불가능. → **수락됨**: 로스터는
   표시 전용; `IA_LobbyAction` (Enter) / `IA_CopyCode` (C); 트럭 운전석
   상호작용은 게이트 선택지 5로 유지.
2. (중대) W3가 자체 모순(Lobby 화면 행에 캐릭터 선택자 / 새 판 vs HUD 로스터).
   → **수락됨**: 행은 Title / JoinCode / Notice / Roster; 선택자와 새 판
   삭제; 나가기는 `WBP_Pause`에.
3. (높음) 혼자 하기 로스터 상태 미정의; Steam 없이도 작동해야 함. →
   **수락됨**: "혼자 · Enter: 출발"; 솔로는 OSS가 null일 때 세션 서브시스템을
   호출하지 않음.
4. (높음) 거부하면 친구를 타이틀에 남기고 코드를 잃음; `OpenLevel` 전에
   메시지를 운반하는 것이 없음. → **수락됨**: 코드 경로 실패는 코드가
   유지된 상태로 셀로 돌아감; `GameInstance.PendingNotice`.
5. (높음) 멀리 보고 있는 호스트를 위한 참가/탈퇴 피드백 없음. →
   **수락됨**: 토스트 + `UI_Confirm` (sound.md에 큐 있음).
6. (중간) 플레이어 대면 해시/버전 문자열 미작성. → **수락됨**: 문자열 테이블을
   W3에 작성.
7. (중간) ModeSelect는 사용자가 필요하지 않은 홉. → **수락됨**: 타이틀에 5개
   항목; Steam 친구 힌트 행이 친구 참가를 대체.
8. (중간) 슬롯 색상 / 호스트 마크 / 유령 "Steam 초기화 스피너". →
   **수락됨**(슬롯 색상 + 숫자는 이미 패스 1 와이어프레임에 있었음; 타이틀
   로딩 상태 제거됨; 호스트 행에 준비 마크 없음).
9. (중간) 80초 스피너. → **수락됨**: 20초 게임 측 참가 타이머.

```
설계 리트머스 스코어카드 (서브에이전트 = 패스 전 계획; 1차 = 수정 후):
═══════════════════════════════════════════════════════════════════════
  차원                          서브에이전트  1차(수정 후)  합의
  ──────────────────────────────  ────────  ──────────────  ─────────
  1. 정보 계층 구조               5         9               불일치 → 해결(결정 2/7 채택)
  2. 상호작용 상태                4         9               불일치 → 해결(3/4/5/9 채택)
  3. 여정 응집력                  4         8               불일치 → 해결(4/5 채택)
  4. 명확성(안티-슬롭)           5         9               불일치 → 해결(6/8 채택)
  5. 설계-시스템 정렬             3         8               불일치 → 해결(1 채택: 키, 버튼 아님)
  6. 반응형 / 입력 의도           3         8               불일치 → 해결(1/6 채택)
  7. 접근성 기본                  4         8               불일치 → 해결(8 채택)
═══════════════════════════════════════════════════════════════════════
0/7이 두 음성으로 확인됨(단일 모델); 모든 서브에이전트 지적이 계획에 채택되었으므로
음성 불일치는 게이트에 도달하지 않음 (선택지 5 제외: 키 vs 트럭 운전석 상호작용 —
순수 취향 선택).
```

## 결정 감사 기록 (2단계)

| # | 단계 | 결정 | 분류 | 원칙 | 근거 | 거부됨 |
|---|-------|----------|----------------|-----------|-----------|----------|
| 27 | 2-0 | 목업 없음 (디자이너에 OpenAI 키 필요) → 텍스트 와이어프레임 | 기계적 | P6 | 프로그레시브 개선, 게이트 아님 | `$D setup` 대기 중 |
| 28 | 2-P1 | 로스터 우상단, 표시 전용, 키 Enter/C | **취향**(선택지 5) | P5 | 유일한 자유 HUD 영역; HUD 패널에 커서 없음; 트럭 운전석 E가 대체 | HUD 패널의 버튼 |
| 29 | 2-P1 | ModeSelect 삭제; 5개 항목 타이틀 + 친구 힌트 | 기계적 | P5 | 한 홉, 전문 용어 없음(Krug) | 2홉 메뉴 |
| 30 | 2-P1 | v1에서 1개 옵션 캐릭터 선택자 및 새 판 삭제 | 기계적 | P5 | 1개 항목의 드롭다운은 노이즈 | 자리표시자 유지 |
| 31 | 2-P2 | 코드 경로 실패는 코드가 유지된 JoinCode 셀로 반환; 세션 내 실패는 `PendingNotice` | 기계적 | P1 | 사용자 작업을 잃지 말 것; 1개 메시지 운반자 | 타이틀 배너 |
| 32 | 2-P2 | 참가/탈퇴 토스트 + `UI_Confirm`; "복사됨" 토스트 | 기계적 | P1 | 호스트가 멀리 볼 수 있음 | 조용한 참가 |
| 33 | 2-P2 | 20초 게임 측 참가 타이머 | 기계적 | P3 | 엔진 80/120초는 세션 내 드롭용 | 80초 스피너 |
| 34 | 2-P3 | 혼자 하기는 Steam 없이도 작동 | 기계적 | P1 | `-nosteam`은 솔로 실행을 부팅해야 함 | Steam 전용 솔로 |
| 35 | 2-P7 | 나가기 = `WBP_Pause` [로비로 나가기] | 기계적 | P4 | ui.md는 이미 예약함 | 새 표면 |
| 36 | 2-P5 | DESIGN.md 누락 → `/design-consultation` TODO (P3) | 기계적 | P3 | 이 계획의 범위 밖 | 지금 실행 |
| 37 | 1-spec2 | 단위 테스트 + 스모크는 빌드 순서 ⑥에서 앞당김(CEO 행 20) | 기계적 | P1 | ⑥는 스냅샷 테스트만 제외 | ⑥ 대기 |
| 38 | 1-spec2 | 호스트 손실 시 타이틀로 이동하기 전에 `LeaveSession()` (CEO 행 21) | 기계적 | P1 | 그렇지 않으면 고아 로비(엔진 소스) | 이동만 |
| 39 | 1-spec2 | 정확한 세션 설정 튜플: 방 만들기는 Public, 혼자 하기는 Private (CEO 행 22) | 기계적 | P5 | `BuildLobbyType`이 로비 유형 도출 | 미지정 플래그 |
| 40 | 1-spec3 | `CL9_PHASE` 로비 키를 `JoinSession` 전에 확인; AC5는 "입장 불가 또는 야간 문자열로 거부"를 수락 | 기계적 | P1 | Steam 클라이언트 "Join Game" 숨김 미확인 | 숨김 확인 |
| 41 | 1-spec3 | `SelectedCharacter` / `Server_SelectCharacter` 제거(결정 30에서 선택자 삭제) | 기계적 | P5 | 고아 복제 필드 | 나중을 위해 유지 |

```
  +====================================================================+
  |         설계 계획 리뷰 — 완료 요약                                   |
  +====================================================================+
  | 시스템 감사         | DESIGN.md 없음; UI 범위: 4행 + 1레벨          |
  | 스텝 0              | 5/10; 모든 7개 차원 집중                     |
  | 패스 1  (정보 아키) | 4/10 → 수정 후 9/10                         |
  | 패스 2  (상태)     | 6/10 → 수정 후 9/10                         |
  | 패스 3  (여정)     | 5/10 → 수정 후 8/10                         |
  | 패스 4  (AI 슬롭)  | 6/10 → 수정 후 9/10                         |
  | 패스 5  (설계 시스) | 5/10 → 수정 후 8/10 (DESIGN.md 없음)        |
  | 패스 6  (반응형)    | 3/10 → 수정 후 8/10                         |
  | 패스 7  (결정)      | 12개 해결, 1개 → 게이트(선택지 5)           |
  +--------------------------------------------------------------------+
  | 범위 밖              | 작성됨(4개 항목)                            |
  | 기존에 있는 것       | 작성됨                                      |
  | TODOS.md 업데이트   | 1개 항목(DESIGN.md via /design-consultation)|
  | 승인된 목업         | 0개 생성(API 키 없음), 0개 승인             |
  | 내린 결정           | 계획에 12개 추가                           |
  | 미룬 결정           | 1개(게이트 선택지 5)                       |
  | 전체 설계 점수      | 5/10 → 8/10                               |
  +====================================================================+
```

> **2단계 완료.** Codex: 불가용. Claude 서브에이전트: 9개 지적(9개 채택).
> 합의: 0/7이 두 음성으로 확인됨(단일 모델), 1개 취향 불일치 → 게이트에서
> 노출됨(선택지 5). 2.5단계(DX) 스킵 — 개발자 대면 범위 없음. 3단계(엔지니어링
> — 수정된 계획 검토)로 진행 중.

---

# /autoplan — 3단계: 엔지니어링 리뷰 (자동 결정, 수정된 계획 검토)

범위 게이트: autoplan — B를 자동 선택(이 계획, 1–2단계에서 수정됨).
설계 문서: CCL8 그린필드 설계(2026-08-27)를 0단계에서 읽음; 일치함.
Codex: `[codex-unavailable]` → `[subagent-only]`.

## 스텝 0 — 범위 도전(코드 포함)
1. **서브 문제별 기존 코드**(회상이 아닌 읽음):
   - Joinable 상태: `ACL9GameState::Phase` + `SetPhase`(호스트 전용),
     `ACL9GameMode::StartNight()`이 `Phase != Lobby`를 보호
     (`CL9GameMode.cpp:155-160`). 그대로 재사용.
   - 로그인 훅: `ACL9GameMode`는 오늘 **아무것도** `PreLogin` / `PostLogin` /
     `Logout` 오버라이드하지 않음(`CL9GameMode.h` grep: 매칭 없음) → 3개 새
     오버라이드, 리팩토링할 것 없음.
   - 서버 RPC 패턴: `Server_AcceptOrders_Validate`는 true 반환하고
     `_Implementation`은 로그로 비-호스트 거부
     (`CL9PlayerController.cpp:224-229`); `Server_CL9Cheat` 동일 형태. 새
     `Server_SetReady` / `Server_Depart`는 이를 따름(`_Validate` false는
     클라이언트를 킥 — 페이즈 체크에 잘못된 도구).
   - 레지스트리: `UCL9DefRegistry::Init(const UCL9DefList*)`은 입력된 목록을 구성
     (`GetItems/GetOrders/GetSites/GetSegKinds…`, `CL9DefRegistry.h:78-129`).
     TuningHash = 모든 목록을 반복, 행을 (클래스 이름, Alias)로 정렬, 각 행의
     스크립트 속성을 메모리 라이터로 직렬화
     (`UObject::SerializeScriptProperties` via `FMemoryWriter`), 버퍼를 `CityHash64`로
     해싱, 16자 16진수로 포맷. 같은 빌드를 실행하는 머신 전체에서 결정론적
     (버전 게이트는 동일한 레이아웃 보장).
   - 치트: `RouteCL9Cheat(Command, Args, World, bLocalOnly)`
     (`CL9Cheats.cpp:68-96`) + `UCL9CheatManager::HandleCommand`
     (`CL9CheatManager.h:27`) — 4개 새 명령, `bLocalOnly=true`.
   - UI: `UCL9UIManager::OpenWidget/CloseWidget/IsOpen` + `CL9UINames`
     (`CL9UIManager.h:33-49`); HUD는 상태에서 화면 도출
     (`CL9HUDWidget.cpp:404-419`는 `OfferedOrders`가 비어있지 않을 때
     `OrderSelect` 열음) — 로스터 행은 동일 도출로 `Phase`에서 열음/닫음.
   - 전송: 엔진 플러그인만(확인됨, Steamworks 1.64).
   - 저장: 동시 세션에서 진행 중(`SaveRun/LoadRun`), 미터치.
2. **최소 집합:** W1 + W2 + 로스터 + Notice + W4 업로드. Title/JoinCode는
   사용자 대면 수용 기준(AC1–AC6), 유지. 추가 미룰 것 없음.
3. **복잡성 체크:** ~14개 소스 파일 + 5개 위젯 + 1개 레벨 + 2개 입력 액션,
   1개 새 클래스(`UCL9SessionSubsystem`) + 4개 위젯 클래스. 8파일 냄새 초과,
   정당화됨: 세션 계층은 필연적으로 설정, GI, GameMode, GameState, PlayerState,
   PC 및 UI를 건드림; 1단계는 이미 회피 가능한 부분 제거(`ECL9SessionState`,
   シームless 이동, Lobby 화면, ModeSelect, 캐릭터 선택자). 기본 제공 기능이
   있는 커스텀 솔루션 없음(**[Layer 1]** OSS Steam 세션, SteamSockets,
   `PreLogin` `ErrorMessage`, `UEngine::OnNetworkFailure`).
4. **검색 확인:** 엔진 소스에서 기본 제공 확인(1/2단계 기록):
   로비 세션, joinable 플래그, 지연된 리스너, 에디터 자동 비활성화,
   `DriverClassNameFallback`. 알려진 함정 포착: UE-239936, ini 인라인
   주석, `#` 존재 토큰, AppID 480 공유 검색 공간, `_Validate` 의미론.
   **[Layer 3]** 인사이트 유지: 야간 페이즈는 세션 축.
5. **TODOS 상호 참조:** TODOS.md(1단계에서 생성)는 6개 항목; 아무것도 이 계획을
   차단하지 않음; JIP 및 저장/계속은 이를 통해 언락.
6. **완전성:** 완전한 버전을 모든 곳에서 선택(아래 Lake Score).
7. **배포:** 새 산출물 = Steam 빌드. 파이프라인은 범위 내(W4: 패키징 설정,
   `Build/Steam/*.vdf`, steamcmd to `beta`); 대상 = Win64만(SteamSockets
   플러그인 허용 목록: Win64/Mac/Linux; Mac/Linux 및 Steam Deck은 TODOS).
   롤백 = Steamworks에서 라이브로 설정한 이전 빌드.

## 1절 — 아키텍처 (엔지니어링)
```
 의존성 그래프 (새로운 → 기존; 화살표 = "호출 / 읽음")
   Config/DefaultEngine.ini ──► OnlineSubsystemSteam ──► SteamSockets ──► SteamSocketsNetDriver
                                        ▲                                       ▲
   UCL9SessionSubsystem [NEW] ──────────┘ (IOnlineSession/Identity/Friends)     │ (ClientTravel URL)
        │ 델리게이트: OnSessionCreated / OnJoinFailed(reason) / OnSessionEnded  │
        ├──► UCL9GameInstance [+PendingNotice, +OnNetworkFailure/OnTravelFailure 핸들러 (클라이언트 전용)]
        ├──► UGameplayStatics::OpenLevel / APlayerController::ClientTravel
        └◄── ACL9GameMode [+PreLogin/PostLogin/Logout, +RequestDepart, +SetJoinable on phase] ──► StartNight/EnterLobby (기존)
                  │ 쓰기                                     │ 쓰기
                  ▼                                            ▼
        ACL9PlayerState [+bIsReady,+SlotIndex,+SelectedCharacter]   ACL9GameState [+SessionCode]
                  │ OnRep                                      │ OnRep
                  ▼                                            ▼
        WBP_Roster (HUD 행, 표시 전용) ◄── UCL9UIManager ◄── WBP_HUD Phase 도출
        WBP_Title / WBP_JoinCode / WBP_Notice (행) ──► UCL9SessionSubsystem (호출) 
        IA_LobbyAction / IA_CopyCode ──► ACL9PlayerController [+Server_SetReady, +Server_Depart, +CopyCode]
        UCL9CheatManager [+host/join/leave/session] ──► UCL9SessionSubsystem
        Build/Steam/*.vdf ──► steamcmd ──► Steamworks beta 분기     Build/Smoke/*.ps1 ──► 2개 -game 프로세스
```
- 결합: GameMode → GI 서브시스템(joinable 플래그)은 유일한 새 게임플레이→온라인
  엣지; 1개 메서드(`SetJoinable(bool)`)를 통해 진행, OSS 불가용 시 no-op. 위젯은
  절대 OSS 타입을 건드리지 않음.
- **통합 지점당 프로덕션 실패:** (1) Steam 클라이언트가 야간 중 죽음 →
  SteamSockets 연결 닫힘 → 클라이언트 `ConnectionLost` → LeaveSession +
  타이틀(고려됨). (2) Valve 로비 서비스 딜레이 중 `UpdateSession` →
  joinable 플래그 부실 → PreLogin 페이즈 게이트는 여전히 거부(고려됨, 로그됨).
  (3) 호스트가 Alt+Tab 하고 출발 중에 오버레이 열림 → 없음(입력이 오버레이로
  감; 게임 계속 실행). (4) **호스트가 *클라이언트* 타임아웃에 대해
  `OnNetworkFailure` 수신** — 엔진이 서버 측에서도 실패를 브로드캐스트; 단순한
  핸들러는 호스트를 타이틀로 보냄. → `GetNetMode() == NM_Client` 보호(찾기
  E1, P1, 신뢰도 9: `UEngine::BroadcastNetworkFailure`는 양쪽에서
  `UNetDriver`에서 호출됨).
- 보안: 1단계 §3에서 변경 없음; 클라이언트 공급 `?hash=`/`?ver=`는
  비교되지만 해석되지 않음; RPC는 호출자 자신의 PlayerState에서 작동.
- 배포: Win64 빌드 → steamcmd → `beta`; `Lvl_Title`은 패키징 맵 목록에
  있어야 함(GameDefaultMap이므로 요리됨) — W4에서 확인.
- 인라인 ASCII 다이어그램 대상: `CL9SessionSubsystem.h`(1단계 §1의 호스트/클라이언트
  상태 머신), `CL9GameMode.cpp` 위의 `PreLogin`(게이트 순서: 페이즈 → 카운트
  → 해시 → 버전), `CL9GameInstance.cpp` 위의 실패 핸들러(클라이언트 전용
  분기 + PendingNotice 흐름).

## 2절 — 코드 품질 (엔지니어링)
| # | 지적 | 심각도 | 신뢰도 | 결정 |
|---|---|---|---|---|
| E1 | `OnNetworkFailure`는 클라이언트 드롭에 대해 호스트에서 발생; 핸들러는 `NM_Client`가 아닌 한 조기 반환해야 함 | P1 | 9 | W2 + 스모크에 추가(클라이언트 프로세스 킬; 호스트는 유지) |
| E2 | TuningHash는 `DefList` 에셋 순서 또는 `TMap` 반복 순서에 의존하면 안 됨 | P1 | 8 | 행을 (클래스, Alias)로 정렬한 후 해싱; 단위 테스트 "섞인 목록 → 동일 해시" |
| E3 | `_Validate` false 반환은 클라이언트 연결 해제; 페이즈/역할 체크는 로그와 함께 `_Implementation`에 속함(기존 `Server_AcceptOrders` 패턴) | P2 | 9 | 패턴을 따름; W2에서 명시 |
| E4 | 출발 권한: `RequestDepart(PC)`는 `PC->IsLocalController() && HasAuthority()`(리스닝 호스트 자신의 PC) 확인, "권한이 있는 임의 PC" 아님 | P2 | 8 | 명시적 호스트 체크 + 거부 시 로그 |
| E5 | 슬롯 → `TintColor` 매핑은 C++ 배열이 아닌 데이터에서 와야 함(conventions § 5) | P2 | 8 | `FindCharacter(alias)->SlotColors[SlotIndex]` 또는 규칙 행; 로스터가 읽음 |
| E6 | 실패 문자열은 `FText` (`NSLOCTEXT`)여야 함, 1개 테이블에, 위젯 전체에 흩어진 `FString` 리터럴이 아님(로컬라이제이션 나중, settings.md) | P2 | 8 | 1개 `CL9SessionStrings` 네임스페이스 서브시스템 헤더에 |
| E7 | 비동기 OSS 콜백은 세계 변경 후 발생할 수 있음; 델리게이트 핸들은 `Deinitialize`에서 지워야 하고 콜백은 `IsValid(this)` 보호 | P2 | 8 | 서브시스템 계약의 일부; 스모크: 취소 중 검색 |
| E8 | DRY: `cl9.host`와 타이틀의 방 만들기는 동일 `HostSession()` 호출; `cl9.join`과 JoinCode 팝업은 동일 `JoinByCode()` — 중복 플래그 튜플 없음 | P2 | 9 | 튜플은 1개 개인 함수 `MakeSessionSettings(EMode)`에 있음 |
과잉 설계 검사: 없음(세션 상태 enum 없음, 재시도 프레임워크 없음, 1개 서브시스템).
과소 설계 검사: E1/E7은 2개 조용한 실패 후보였음; 지금 둘 다 명시적. 건드린 파일의
기존 ASCII 다이어그램: 오늘 `CL9GameMode.cpp` / `CL9PlayerState.h`에 없음;
session.md의 화면 스케치는 낡음 → W5는 이를 재작성(D5/T11).

## 3절 — 테스트 리뷰 (엔지니어링)
프레임워크 탐지: CLAUDE.md에 Testing 섹션 없음; 트리에 테스트 파일 없음(`git ls-files`는
찾지 못함); UE Automation(`FAutomationTestBase`)은 에디터 빌드에서 사용 가능 —
계획은 첫 2개 테스트를 도입(T8). 수동 시나리오는 docs/testing/pie-scenarios.md에 있음.
```
코드 경로                                                           사용자 흐름
[+] Source/CL9/Session/CL9SessionSubsystem.cpp                     [+] 방 호스트
  ├── HostSession(mode)                                              ├── [GAP] [→E2E S1] Steam 켜짐 → 코드 표시, 오버레이 열림
  │   ├── [GAP]        OSS null → 불가용 (솔로는 여전히 작동)     ├── [GAP] [→INT S0] -nosteam → 리스닝 레벨, 세션 호출 없음
  │   ├── [GAP]        이미 세션에 있음 → 거부 + 로그             └── [GAP] [→E2E S2b] 호스트 → 나가기 → 다시 호스트(UE-239936)
  │   ├── [GAP]        OnCreateSessionComplete(false) → 메시지    [+] 코드로 참가
  │   └── [GAP]        ok → OpenLevel(?listen) + SessionCode 설정  ├── [GAP] [★unit T8] 정규화: 소문자/공백/0O1I/길이 7
  ├── JoinByCode(code)                                               ├── [GAP] [→E2E S3] 유효 코드 → 양쪽 PC에 로스터
  │   ├── [GAP] [★unit] 유효성 검사/정규화                           ├── [GAP] [→E2E S5] 5번째 플레이어 → "(4/4)" 인라인, 코드 유지
  │   ├── [GAP]        FindSessions 실패/타임아웃 → 메시지           ├── [GAP] [→E2E S5] 해시 불일치 → 해시 문자열 인라인
  │   ├── [GAP]        0 / 1 / >1 결과                              ├── [GAP] [→E2E S5] 버전 불일치 → 버전 문자열
  │   ├── [GAP]        JoinSession 결과 enum ×5 → 문자열            ├── [GAP] [→E2E S5] 야간 중 → 야간 문자열; 친구 목록에 Join Game 없음
  │   └── [GAP]        20초 타이머 → 취소 + 문자열                  └── [GAP] [→E2E S3] 검색 중 취소 → 콜백 크래시 없음
  ├── OnInviteAccepted                                             [+] 초대
  │   ├── [GAP]        따뜻함(타이틀) → JoinSession                   ├── [GAP] [→E2E S4] 따뜻한 초대 → 로스터
  │   ├── [GAP]        찬 시작 → 첫 세계 이후                        └── [GAP] [→E2E S4] 찬 초대(게임 종료) → 로스터
  │   └── [GAP]        플레이어와 호스팅 중 → 무시 + 로그          [+] 로비 → 야간
  ├── LeaveSession / DestroySession (+ 실패 로그)                   ├── [GAP] [→INT S0] 클라이언트 Enter → 호스트 로스터에 준비 ✓
  └── SetJoinable(bool) (+ OnUpdateSessionComplete(false) 로그)     ├── [GAP] [→INT S0] 모두 준비되기 전에 호스트 Enter → 거부 + 상태 행
[+] Source/CL9/GameMode/CL9GameMode.cpp                             ├── [GAP] [→INT S0] 제안 보류 중 호스트 Enter → 거부
  ├── PreLogin: 페이즈 / 카운트(+이동 중) / 해시 / 버전            ├── [GAP] [→INT S0] 모두 준비 + 선택 → Night (StartNight)
  │   └── [GAP] [→INT S0] 각 게이트 → ErrorMessage 문자열           ├── [GAP] [→INT S0] EnterLobby → 준비 false로 재설정
  ├── PostLogin: 슬롯 할당(가장 낮은 자유), 페이즈 재확인 → 킥      └── [GAP] [→INT S0] 회귀: cl9.night는 여전히 야간 시작
  │   └── [GAP] [→INT S0] StartNight 윈도 중 참가                 [+] 탈퇴 / 드롭
  ├── Logout: 슬롯 자유                                              ├── [GAP] [→INT S0] 클라이언트 Alt+F4 → 슬롯 해제, 호스트에 토스트
  ├── RequestDepart(PC): 호스트 전용, 모두 준비, 제안 보류 없음     ├── [GAP] [→E2E S6] 호스트 종료 → 클라이언트: LeaveSession + Notice + 타이틀
  └── SetPhase 경로는 SetJoinable 호출(StartNight/EnterLobby)       ├── [GAP] [→INT S0] 호스트는 CLIENT 타임아웃 시 유지(E1)
[+] Source/CL9/GameMode/CL9GameInstance.cpp                         └── [GAP] [→E2E S6] 클라이언트 → Pause → 로비로 나가기 → 타이틀
  ├── OnNetworkFailure: NM_Client만 → LeaveSession → PendingNotice → 타이틀   [+] 회귀
  └── OnTravelFailure: 동일                                          ├── [GAP] [→INT A3] 회귀: PIE 2-클라이언트 A3 (T14 후)
[+] Source/CL9/Data/CL9DefRegistry.cpp                               └── [GAP] [→E2E]    회귀: Steam 드라이버의 모든 기존 pie-scenarios
  ├── [GAP] [★unit T8] 동일 목록 → 동일 해시; 섞임 → 동일; 1개 값 변경 → 다름
[+] Source/CL9/UI/* (Title/JoinCode/Notice/Roster)                 [+] UI 상태
  └── [GAP] [→E2E]    2단계 테이블에서 상태당 복사                  └── [GAP] [→E2E]    47자 이름 클립; 솔로 로스터; Steam 끄기 타이틀

보장 범위 현재: 0/41 경로 테스트됨(0%) — 프로젝트에 테스트 없음
계획됨: unit ★★★ 2개(T8) | 통합 S0 스모크는 14개 포함 | 수동 E2E S1–S6은 20개 포함 | 5개 UI 상태 체크 수동
품질 목표: ★★★ 단위 테스트, ★★ S0(로그 행의 확정문), ★ 수동 시각 체크
계획 후 갭: 0개 테스트되지 않은 경로; 0개 회귀 발견(A3 + cl9.night S0/T14에서)
```
- 금요일 오전 2시 테스트: S0 녹색 + S2 자신의 AppID에. 적대 QA: E1(클라이언트
  킬 → 호스트는 타이틀로 가면 안 됨), 검색 중 취소(E7), 3명의 친구와 호스팅
  중 초대. 혼돈: 호스트 케이블 뽑기 → 80초 내 클라이언트가 Notice 받음; Steam
  클라이언트 킬 → 동일.
- 불안정성: Steam 경로는 본질상 수동; S0은 결정론적(로컬호스트, `-nosteam`,
  타임아웃이 있는 로그 grep).
- LLM/프롬프트 변경: 없음.
- 테스트 계획 산출물 작성:
  `C:\Users\user\.gstack\projects\Work\user-master-eng-review-test-plan-20260906.md`.

## 4절 — 성능 (엔지니어링)
4명 플레이어; `PlayerArray` 스캔은 O(4). TuningHash는 초기화 시 1개
직렬화 경로(수백 행, 밀리초 이하). 로비 검색은 서버 측 필터됨;
`UpdateSession` 야간 2회. SteamSockets 틱은 기본값이 20 Hz 복제 정책 초과.
로스터 업데이트는 OnRep만. 지적 없음.

## 실패 모드 (엔지니어링 — 1단계 등록부 대비 변화)
```
  코드경로                   | 실패 모드                          | 테스트   | 처리됨 | 사용자 보임                 | 중대?
  ───────────────────────────|────────────────────────────────|--------|---------|────────────────────────────────|──────
  GI OnNetworkFailure(호스트) | 호스트가 클라이언트 드롭을 자신의 것으로 처리 | S0     | E1 보호| 없음(정확)         | E1 전: 중대
  TuningHash                 | 순서 의존 해시 → 거짓 거부    | T8 unit| E2 정렬 | (될 것) 해시 문자열    | E2 전: 중대
  Server_SetReady            | _Validate false는 클라이언트 킥  | S0     | E3      | 없음                       | 아니요
  RequestDepart              | 권한이 있는 클라이언트? (없음)  | S0     | E4      | 거부 로그                  | 아니요
  OSS 콜백 후 세계  | 흔들리는 this / 부실 위젯    | S3 취소| E7      | 없음                       | 아니요
  로스터 TintColor           | 하드코드된 색상이 데이터에서 드리프트 | 시각 | E5      | 잘못된 색상                | 아니요
```
중대 갭: E1/E2 후 0개.

## 워크트리 병렬화
| 스텝 | 건드린 모듈 | 의존 |
|---|---|---|
| W0 서류 | —(Steamworks 웹) | — |
| W1 설정 | `CL9.uproject`, `Source/CL9/CL9.Build.cs`, `Config/` | 동시 세션의 T9 커밋 |
| W2 세션 + 게이트 | `Source/CL9/Session/`, `Source/CL9/GameMode/`, `Source/CL9/Data/` | W1 |
| W3 UI + 입력 | `Source/CL9/UI/`, `Content/UI/`, `Content/Input/`, `Content/Maps/`, `Source/CL9/GameMode/CL9PlayerController` | W2(PlayerState 필드, 서브시스템 API) |
| W4 스모크 + 테스트 + 업로드 | `Build/`, `Source/CL9/Tests/`, 패키징 설정 | W2(스모크), W0(업로드) |
| W5 문서 | `docs/` | W1–W4 |
레인: **A** = W1 → W2(순차, 공유 `GameMode/`). **B** = W3(A의
PlayerState/서브시스템 API 착지 후; 바이너리 에셋 → 1개 에디터 세션). **C** =
W4 테스트 + 스모크 스크립트(A 후; B와 공유 모듈 없음). **W0**는 1일차부터
병렬 실행. 실행: A, 그다음 B ∥ C, 그다음 W5. 충돌 플래그: B와 A는
둘 다 `GameMode/CL9PlayerController` 건드림 — PC RPC 추가는 레인 A에 유지.

## 범위 밖 (엔지니어링) / 기존에 있는 것 (엔지니어링)
1단계에서 변경 없음 더하기: Mac/Linux 빌드(플러그인 허용 목록 허용; 미테스트),
Steam 인증 티켓(`OnlineAuthInterfaceSteam` — VAC 끄기, 친구 협력). 재연결
토큰 / SteamID 슬롯 지속성(JIP TODO). 기존 코드 재사용: 스텝 0의 모든 것.

## 구현 태스크 (엔지니어링 단계)
- [ ] **E1 (P1, 사람: ~1시간 / CC: ~10분)** — W2 — `UCL9GameInstance` 실패 핸들러는 `GetNetMode() == NM_Client`일 때만 작동; 호스트는 로그하고 계속
  - 근거: 1절 — 프로덕션 실패 4(서버 측 `OnNetworkFailure` 브로드캐스트)
  - 파일: Source/CL9/GameMode/CL9GameInstance.cpp
  - 검증: S0 — 클라이언트 프로세스 킬; 호스트는 로비에서 유지, 로스터 행 비움
- [ ] **E2 (P1, 사람: ~2시간 / CC: ~15분)** — W2 — TuningHash: 행을 (클래스, Alias)로 정렬, `SerializeScriptProperties`를 `FMemoryWriter`로, `CityHash64`, hex16; 단위 테스트: 섞인 DefList → 동일 해시
  - 근거: 2절 E2 — 순서 의존 해시는 거짓 거부
  - 파일: Source/CL9/Data/CL9DefRegistry.{h,cpp}, Source/CL9/Tests/CL9SessionCodeTest.cpp
  - 검증: `Automation RunTests CL9.TuningHash`
- [ ] **E3 (P2, 사람: ~1시간 / CC: ~10분)** — W2 — `Server_SetReady` / `Server_Depart`: `_Validate` true, `_Implementation`의 페이즈/역할 체크 로그 포함(`Server_AcceptOrders` 패턴)
  - 근거: 2절 E3
  - 파일: Source/CL9/GameMode/CL9PlayerController.{h,cpp}
  - 검증: S0 — 야간 중 클라이언트 Enter → 거부 + 로그, 연결 해제 없음
- [ ] **E4 (P2, 사람: ~1시간 / CC: ~10분)** — W2 — `ACL9GameMode::RequestDepart(PC)`: 서버의 `PC->IsLocalController()`(리스닝 호스트) + 모두 준비 + 제안 보류 없음 → `StartNight()`; 그렇지 않으면 이유 로그
  - 근거: 2절 E4
  - 파일: Source/CL9/GameMode/CL9GameMode.{h,cpp}
  - 검증: S0 — 클라이언트 Enter → 거부; 제안 보류 중 호스트 Enter → 거부
- [ ] **E5 (P2, 사람: ~1시간 / CC: ~10분)** — W3 — 슬롯 → `TintColor`는 캐릭터 테이블에서 읽음(plan/data/character.md 슬롯 색상), `ACL9PlayerState::GetSlotColor()`로 노출
  - 근거: 2절 E5 — conventions § 5(에셋에서 숫자)
  - 파일: Source/CL9/Data/CL9CharacterDef.h, Source/CL9/GameMode/CL9PlayerState.{h,cpp}, Content/UI/WBP_Roster
  - 검증: DA에서 색상 변경 → 빌드 없이 로스터 업데이트
- [ ] **E6 (P2, 사람: ~1시간 / CC: ~10분)** — W2 — 실패 문자열을 `NSLOCTEXT("CL9Session", …)`로 1개 헤더에; 위젯은 이를 읽음, 리터럴 없음
  - 근거: 2절 E6; 2단계 문자열 테이블
  - 파일: Source/CL9/Session/CL9SessionStrings.h
  - 검증: Source/CL9/UI에서 세션 오류의 한글 리터럴 검색 없음
- [ ] **E7 (P2, 사람: ~1시간 / CC: ~10분)** — W2 — 모든 OSS 델리게이트 핸들을 `Deinitialize`에서 지움; 콜백은 약한 컨텍스트의 서브시스템으로 바인드; 검색 중 취소는 깔끔하게
  - 근거: 2절 E7
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp
  - 검증: S3 — 검색 중 취소 → 콜백 없음; S0 — 검색 중 종료 → 크래시 로그 없음
- [ ] **E8 (P2, 사람: ~30분 / CC: ~5분)** — W2 — 1개 `MakeSessionSettings(EMode)`는 타이틀과 `cl9.host` 둘 다 사용; `JoinByCode`는 팝업과 `cl9.join`에 공유
  - 근거: 2절 E8 — DRY
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp, Source/CL9/GameMode/CL9Cheats.cpp
  - 검증: grep은 `FOnlineSessionSettings` 빌드 위치 1개 표시
- [ ] **E9 (P3, 사람: ~30분 / CC: ~5분)** — W2 — 인라인 ASCII 다이어그램: `CL9SessionSubsystem.h`의 세션 상태 머신, `PreLogin` 위의 게이트 순서, `CL9GameInstance.cpp`의 실패-핸들러 흐름(한글 주석)
  - 근거: 1절 — 다이어그램 대상
  - 파일: 명시된 대로
  - 검증: 다이어그램 현재이고 계획의 1단계 §1 상태 머신과 일치
- [ ] **E10 (P1, 사람: ~1시간 / CC: ~10분)** — W2 — `IsSteamAvailable()` = `IOnlineSubsystem::Get(STEAM_SUBSYSTEM)` 비-null 및 활성화; 절대 그냥 `Get()`(Null OSS 폴백); AC7은 PIE / `-nosteam`에서 세션 없음 확정
  - 근거: Eng 음성 지적 1 — `OnlineSubsystemModule.cpp:216-219`, Null 플러그인 `EnabledByDefault: true`
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp
  - 검증: `-nosteam` 로그는 "session: Steam unavailable — skipping" 표시; 혼자 하기는 여전히 실행
- [ ] **E11 (P1, 사람: ~30분 / CC: ~5분)** — W2/W4 — 이동 옵션은 `?`-분리(`?hash=X?ver=Y`); 연결-문자열 빌더 + `ParseOption` 왕복 단위 테스트
  - 근거: Eng 음성 지적 2 — `GrabOption`은 `?`만 분리(`GameplayStatics.cpp:3410-3428`)
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp, Source/CL9/Tests/CL9SessionCodeTest.cpp, Build/Smoke/two-process-smoke.ps1
  - 검증: 단위 테스트 녹색; S0 버전 게이트는 일치 버전으로 통과
- [ ] **E12 (P1, 사람: ~1시간 / CC: ~10분)** — W2 — "Full" 게이트는 `GetNumPlayers()` + PlayerController 없는 대기 `ClientConnections` 카운트; `PostLogin`은 카운트 및 페이즈 재확인
  - 근거: Eng 음성 지적 3 — `NumTravellingPlayers`는 `AGameMode`만; `ACL9GameMode : AGameModeBase`
  - 파일: Source/CL9/GameMode/CL9GameMode.cpp
  - 검증: S0 변형 — 5명 클라이언트 1초 내 연결; 정확히 4명 착지, 5번째는 "(4/4)"
- [ ] **E13 (P1, 사람: ~30분 / CC: ~5분)** — W2 — `FindSessions`는 `SEARCH_LOBBIES=true` 설정; 미발견 문자열은 먼 지역 힌트 획득
  - 근거: Eng 음성 지적 5/6 — `OnlineSessionInterfaceSteam.cpp:778-789`; `k_ELobbyDistanceFilterDefault`는 `OnlineSessionAsyncLobbySteam.cpp:845`에서 하드 코드
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp, Source/CL9/Session/CL9SessionStrings.h
  - 검증: S3는 코드 찾음; S3 노트는 먼 지역 행동 기록
- [ ] **E14 (P2, 사람: ~1시간 / CC: ~10분)** — W2 — 실패 핸들러는 호스트의 `ErrorString`을 `PendingConnectionFailure` / `FailureReceived`로 통과; 고정 테이블은 Lost / Timeout / Outdated*만
  - 근거: Eng 음성 지적 4 — `UnrealEngine.cpp:16015-16017`
  - 파일: Source/CL9/GameMode/CL9GameInstance.cpp
  - 검증: S5 — 모든 PreLogin 거부는 자신의 한글 문자열 표시, 일반적인 것 아님
- [ ] **E15 (P2, 사람: ~1시간 / CC: ~10분)** — W2 — 솔로 튜플 `bShouldAdvertise=false`, `NumPublicConnections=1`; `bSoloRun`은 `?solo` → `PreLogin`은 모두 거부; `MakeSessionSettings(Solo)`의 단위 테스트
  - 근거: Eng 음성 지적 7; 스펙 리뷰 라운드 3 지적 1
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp, Source/CL9/GameMode/CL9GameMode.cpp, Source/CL9/Tests/CL9SessionCodeTest.cpp
  - 검증: 단위 테스트; S0 — 솔로 호스트로 열기 거부
- [ ] **E16 (P2, 사람: ~30분 / CC: ~5분)** — W4 — Steam 업로드는 Shipping만; Development 빌드는 2개 테스트 PC에 유지
  - 근거: Eng 음성 지적 8 — `AllowCheats` true in non-Shipping(`CL9GameMode.cpp:55-64`)
  - 파일: Build/Steam/app_build.vdf, docs/testing/pie-scenarios.md
  - 검증: 베타-분기 빌드는 `cl9.*` 명령 거부
- [ ] **E17 (P3, 사람: ~1시간 / CC: ~10분)** — W2 — Joinable 플래그 + `CL9_PHASE`는 서브시스템이 `OnNightPhaseChanged`에 구독하면서 업데이트(호스트 측); GameMode는 온라인 호출 없음
  - 근거: Eng 음성 지적 11 — conventions § 2 "Steam은 전송만"
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp
  - 검증: grep은 `CL9GameMode.cpp`에 `IOnlineSession` 포함 없음; S5는 여전히 통과
- [ ] **E18 (P2, 사람: ~2시간 / CC: ~15분, 선택)** — W4 — Dev 전용 `-cl9nullsession` 스위치는 Null OSS에 대해 `HostSession`/`JoinByCode` 드라이브 S0(2개 로컬 인스턴스가 서로 발견할 수 없으면 드롭)
  - 근거: Eng 음성 지적 9 — 비동기 세션 경로는 그렇지 않으면 수동만
  - 파일: Source/CL9/Session/CL9SessionSubsystem.cpp, Build/Smoke/two-process-smoke.ps1
  - 검증: S0은 크래시 없이 취소-검색-중 및 콜백-후-Deinitialize 실행

## 이중 음성 (엔지니어링)

**CODEX SAYS (엔지니어링 — 아키텍처 도전):** `[codex-unavailable]`(사용량
한도, 2026-09-14까지). `[subagent-only]`.

**CLAUDE SUBAGENT (엔지니어링 — 독립 리뷰)** — 1–2단계 수정 후 계획을 읽음(마지막 스펙 리뷰 3 수정 전). 11개 지적, 엔진 또는 프로젝트 소스에 대해 각각 확인:
1. (P1, 9) `IOnlineSubsystem::Get()`은 절대 null이 아님 — Null OSS 폴백 →
   **수락됨**(E10). 이는 혼자 하기가 PIE에서 LAN 세션을 만들고 "Steam 끄기"
   상태를 완전히 숨겼을 것.
2. (P1, 9) URL 옵션은 `&` 아닌 `?`로 분리 → **수락됨**(E11). 작성된 대로
   스모크 스크립트는 버전에서 모두 거부했을 것.
3. (P1, 9) `NumTravellingPlayers`는 `AGameModeBase`에 존재하지 않음 →
   **수락됨**(E12): 대기 `ClientConnections` + PostLogin 카운트 재확인.
4. (P2, 8) PreLogin 거부는 호스트 문자열로 `PendingConnectionFailure`로 도착 →
   **수락됨**(E14).
5. (P2, 9) `SEARCH_LOBBIES` 키는 로비 검색에 필요 → **수락됨**(E13); 계획은
   `SEARCH_PRESENCE`였으며, 코드를 절대 찾지 못했을 것.
6. (P2, 6) 거리 필터는 기본값에 하드 코드 → **기록된 한계로 수락**(E13 문자열
   + S3 노트); 초대는 영향 없음.
7. (P2, 8) 솔로 튜플 모순 → 이미 스펙 리뷰 3으로 고정; `NumPublicConnections=1`
   + `bSoloRun` 게이트 추가(E15).
8. (P2, 8) Development 베타는 호스트 치트 배포 → **수락됨**(E16):
   Shipping 전용 업로드.
9. (P2, 5) 세션 서브시스템은 자동화된 범위 없음; Null OSS LAN은 이를 드라이브할 수
   있음 → **선택 실험으로 수락**(E18, 미검증).
10. (P3, 7) VAC 게이트 항목은 no-op → **수락됨**: 게이트 선택지 4 제거.
11. (P3, 7) GameMode → `UpdateSession`은 "Steam은 전송만" 위반 →
    **수락됨**(E17): 서브시스템이 `OnNightPhaseChanged`에 구독.

```
엔지니어링 이중 음성 — 합의 테이블:
═══════════════════════════════════════════════════════════════
  차원                                 Claude   Codex  합의
  ──────────────────────────────────  ───────  ─────  ─────────
  1. 아키텍처 사운드?                 부분    N/A    부분 → E10/E17 채택
  2. 테스트 범위 충분?                아니요   N/A    불일치 → E11/E12/E15 단위+스모크 추가; E18 선택; Steam 경로 수동 유지(수락 한계)
  3. 성능 위험 처리?                 예      N/A    N/A(단일 음성) — 양쪽 지적 없음
  4. 보안 위협 커버?                 부분    N/A    부분 → E16 채택
  5. 오류 경로 처리?                 부분    N/A    부분 → E10/E14 채택
  6. 배포 위험 관리 가능?             예      N/A    N/A(단일 음성) — 베타 분기 + 되돌림 + 버전 게이트
═══════════════════════════════════════════════════════════════
0/6이 두 음성으로 확인됨(단일 모델). 3개 P1 지적 채택. Eng 음성의 사용자
도전 없음.
```

## 결정 감사 기록 (3단계)

| # | 단계 | 결정 | 분류 | 원칙 | 근거 | 거부됨 |
|---|-------|----------|----------------|-----------|-----------|----------|
| 42 | 3-0 | 복잡성 냄새(>8 파일) 수락 — 세션 계층은 본질상 스택을 걸침; 회피 가능한 부분 이미 제거 | 기계적 | P2 | autoplan의 범위 냄새는 절대 감소 안 함 | 감소 |
| 43 | 3-S1 | 실패 핸들러 클라이언트 전용(E1) | 기계적 | P1 | 서버 측 브로드캐스트는 호스트를 타이틀로 보냄 | — |
| 44 | 3-S2 | TuningHash 정렬 + 직렬화 + CityHash64(E2) | 기계적 | P5 | 순서 독립, 1개 경로 | 속성 대 속성 리플렉션 워크 |
| 45 | 3-S2 | RPC는 `_Implementation`에서 체크(E3), `RequestDepart`의 호스트 체크(E4), 데이터 드라이브 슬롯 색상(E5), `NSLOCTEXT` 테이블(E6), 델리게이트 정리(E7), 1개 설정 빌더(E8) | 기계적 | P4/P5 | 기존 패턴, DRY | — |
| 46 | 3-S3 | 단위 ×3 + S0 스모크 + 수동 S1–S6; Steam 경로는 본질상 수동 | 기계적 | P1 | 의존성은 저렴하게 모킹 불가능; E18 선택 | 테스트 스킵 |
| 47 | 3-voice | Steam 가용성 술어 = `Get(STEAM_SUBSYSTEM)`(E10) | 기계적 | P1 | 소스에서 Null OSS 폴백 검증 | 그냥 `Get()` |
| 48 | 3-voice | `?`-분리 이동 옵션(E11) | 기계적 | P1 | 엔진 파서 | `&` |
| 49 | 3-voice | 대기 연결 카운트 + PostLogin 카운트 재확인(E12) | 기계적 | P1 | `AGameModeBase`에 `NumTravellingPlayers` 없음 | `NumPlayers` 신뢰 |
| 50 | 3-voice | `SEARCH_LOBBIES`; 먼 지역 한계 기록(E13) | 기계적 | P1 | OSS Steam 검색 라우팅 | 마스터 서버 검색 |
| 51 | 3-voice | PreLogin `ErrorString` 통과(E14) | 기계적 | P1 | `PendingConnectionFailure`는 이를 운반 | enum당만 |
| 52 | 3-voice | Shipping 전용 Steam 업로드(E16) | 기계적 | P3 | non-Shipping에서 `AllowCheats` true | 개발 베타 |
| 53 | 3-voice | VAC 게이트 항목 제거 | 기계적 | P5 | 플래그는 이 경로에서 읽지 않음 | no-op 결정 유지 |
| 54 | 3-voice | Joinable/`CL9_PHASE`는 서브시스템 구독을 통해(E17) | 기계적 | P5 | GameMode는 Steam 자유(conventions § 2) | GameMode는 Steam 호출 |
| 55 | 3-voice | Null OSS 세션 스모크를 선택 실험으로(E18) | 기계적 | P6 | 미검증 가능성; 저렴한 시도 | 의무 |

## 완료 요약 (엔지니어링)
- 스텝 0: 범위 도전 — 범위는 그대로 수락(복잡성 냄새 정당화; 추가 미룰 것 없음)
- 아키텍처 리뷰: 1개 문제 발견(E1) + 음성의 3개(E10, E12, E17)
- 코드 품질 리뷰: 8개 발견(E2–E8, E11)
- 테스트 리뷰: 다이어그램 생성, 41개 경로, 계획 후 0개 갭(3개 단위, 1개 통합, 6개 수동 E2E); E18 선택
- 성능 리뷰: 0개 문제 발견
- 범위 밖: 작성됨 · 기존에 있는 것: 작성됨
- TODOS.md 업데이트: 엔지니어링의 0개 새 것(1–2단계에서 이미 6개 항목 수집)
- 실패 모드: 0개 중대 갭(E1/E2 전: 2개가 중대였음)
- 외부 음성: 실행(claude 서브에이전트) `[subagent-only]`
- 병렬화: 3개 레인 — A(W1→W2) 순차, 그다음 B(W3) ∥ C(W4 테스트/스모크); W0 1일차부터 병렬
- Lake Score: 9/9 권장은 완전한 옵션 선택

> **3단계 완료.** Codex: 불가용. Claude 서브에이전트: 11개 지적(11개 채택,
> 2개를 기록된 한계/선택으로). 합의: 0/6이 두 음성으로 확인됨(단일 모델),
> 1개 불일치(테스트 범위) → E11/E12/E15/E18로 해결, 게이트에서 노트. 4단계
> (최종 게이트)로 진행 중.

---

# /autoplan — 교차 단계 테마

- **테마: "엔진 소스로 확인하기 전엔 믿지 말 것"** — 1단계(`bInitServerOnClient`
  주장을 스펙 리뷰가 뒤집음)와 3단계(Null OSS 폴백, `SEARCH_LOBBIES`,
  `?`-구분 옵션, `AGameModeBase`에는 `NumTravellingPlayers`가 없음)에서 독립적으로
  드러난 지적. 높은 신뢰도 신호: 모든 W1/W2 스텝은 지금 부르려는 호출에 대해 5.8
  플러그인 소스를 읽는 것으로 시작하고, 계획은 검증한 사실마다 file:line을
  인용하며 나머지는 "미검증"으로 표시한다.
- **테마: 혼자 하기의 의미가 계속 빠짐** — 디자인 음성(상태 미정의, Steam 없이도
  동작해야 함), 스펙 리뷰 3차(로비 타입 모순), 엔지니어링 음성(튜플 + 솔로 게이트
  없음)에서 반복 지적. 이제 고정: Private 로비 튜플, `NumPublicConnections=1`,
  `bSoloRun`으로 PreLogin 거부, 유닛 테스트, "혼자 · Enter: 출발" 로스터.
- **테마: Steam 경로는 자동화할 수 없음** — 1단계 스펙 리뷰(스모크가 서브시스템을
  건너뜀)와 3단계 엔지니어링 음성(비동기 경로는 수동뿐)이 공통으로 지적. 한계로
  수용하되 완화책을 둠: S0이 모든 게이트와 복제 경로를 덮고, E18이 Null OSS
  경로를 시도하며, S1~S6이 두 PC에서의 수동 계약으로 남는다.

게이트 입력: 취향 선택 4개(단일 레벨 로비 / 밤중 참가 거부 / 첫 Steam 산출물 /
로스터 조작은 키 vs 트럭 운전석 E)와 사용자 도전 1개(W2~W4를 4e~4i·세이브
이후로 미룰지). VAC는 게이트에서 뺐다(no-op).

- [ ] **D5 (P3, 사람: ~1시간 / CC: ~10분)** — W5 — ui.md: 로스터 구역(우상단), HUD/UI 테이블에 Notice/Roster/JoinCode 행 추가
  - 근거: 5패스
  - 파일: docs/systems/ui.md, docs/systems/ui_system.md
  - 검증: 문서가 모든 새 행을 나열
