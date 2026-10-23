# 치트 시스템 (CL9CheatManager)

> 개발·테스트용 콘솔 명령. **치트 하나 추가 = 함수 하나 + `HandleCommand` 분기 한 줄 + 콘솔 등록 블록 하나** — 스텝 검증
> ("표에 행 추가하면 바닥에 뜬다")을 루프 전체를 돌지 않고 바로 확인하는 도구다.

## 원칙

- **접두는 `cl9.`** — 프로젝트 치트·디버그 명령을 전부 이 접두로 통일한다 (`bp_` 접두 폐기).
- **인자는 Alias 또는 번호** — 문자면 Alias(+SubAlias), 숫자면 `cl9.list`가 찍어준 번호.
- **치트도 판정 규약을 탄다** — 스폰·상태 변경은 호스트에서만. 클라 콘솔에서 치면 Server RPC로
  올라가 호스트가 실행한다. 치트로 소환한 액터도 실제와 같은 경로(SetAlias → 복제 → OnRep →
  LoadData)를 타므로, **치트 테스트가 곧 네트워크 테스트다.**
- **Shipping 빌드에서 컴파일 제외** (`#if !UE_BUILD_SHIPPING`).

## 구조

```
UCL9CheatManager : UCheatManager          ← 치트 본체 (Source/CL9/GameMode/CL9CheatManager.*). 치트 = 멤버 함수 하나씩
  ├─ HandleCommand(CommandLine)           ← "additem egg 2" 를 공백으로 나눠 첫 토큰으로 else-if 분기 (#if !UE_BUILD_SHIPPING)
  ├─ AddItem · AddMonster · HitMonster · ListDefs · Night · Blackout · Damage · Day · Orders · Pick
  │  · Money · Buy · Eat · Settle · SaveRun / LoadRun · Seed · Wave · Execute · Depart · Lever · Power · Truck   (각 TArray<FString> Args)
  └─ ResolveItem(Token) / ResolveMonster(Token) → 행    ← 공용: 번호/Alias 해석

CL9Cheats.cpp (Source/CL9/GameMode/)       ← 등록부. 명령 이름·도움말 ↔ HandleCommand 연결
  static FAutoConsoleCommandWithWorldAndArgs GCL9Cheat<Name>(TEXT("cl9.<name>"), 도움말, 람다)
    람다 → RouteCL9Cheat("<name>", Args, World, bLocalOnly)
  RouteCL9Cheat: 호스트(또는 bLocalOnly)면 첫 PC 의 CheatManager->HandleCommand(명령줄),
                 클라면 ACL9PlayerController::Server_CL9Cheat(명령줄) 로 호스트에 올린다
  로컬 스위치(cl9.shownoise · cl9.lang · cl9.voice · cl9.talk)는 CheatManager 를 거치지 않고 람다 안에서 바로 처리
  cl9.voiceroute 도 CheatManager 를 거치지 않는다 — 람다가 ACL9PlayerController::RequestVoiceRoute 를 부르고, 호스트가 아니면 그 Server RPC(Server_SetVoiceRoute)가 올린다
  세션 이동(cl9.host · cl9.leave · cl9.join)도 람다 안에서 UCL9GameInstance 를 바로 부른다 — 시작 레벨(메뉴 게임 모드)에는
  호스트 판정할 게임 모드가 없고 CheatManager 도 없을 수 있어서다 (session.md 개발 콘솔)
```

`cl9.` 처럼 점(.)이 들어간 명령은 exec 함수명으로 안 되므로, `IConsoleManager` 등록
(`FAutoConsoleCommandWithWorldAndArgs`)으로 명령 이름을 만들고 `RouteCL9Cheat` 가 CheatManager의
`HandleCommand` 로 포워딩한다. PlayerController에는 범용 RPC 하나만 둔다:

```cpp
// 클라에서 친 치트를 호스트로 — 콘솔 콜백이 로컬 PlayerController의 이 RPC를 직접 부른다
// (원격 PC 사본에는 CheatManager가 없으므로 CheatManager를 경유하지 않는다)
UFUNCTION(Server, Reliable, WithValidation)
void Server_CL9Cheat(const FString& CommandLine);
// 함정 셋 대비:
// 1) AllowCheats: 패키지 개발 빌드에선 기본 false → GameMode에서 비Shipping 상시 true 오버라이드
// 2) 번호 인자는 명령줄 그대로 호스트로 가고 호스트가 자기 레지스트리 순서로 푼다 — 클라·호스트가 같은 표라 순서가 같다
// 3) #if !UE_BUILD_SHIPPING으로 UFUNCTION 선언을 통째로 제외하지 않는다 (RPC 테이블 불일치)
//    — 선언은 유지하고 함수 본문만 비운다
```

### 치트 추가 절차 (이게 전부다 — 세 곳)

```cpp
// 1. UCL9CheatManager 에 함수 하나 (CL9CheatManager.h 선언 + .cpp 정의)
void UCL9CheatManager::AddItem(const TArray<FString>& Args) {
  // 호스트에서만 불린다 — 클라 입력은 RouteCL9Cheat 가 Server_CL9Cheat RPC 로 보낸다 (구조 절)
  const UCL9ItemDef* Def = ResolveItem(Args[0]);            // 없으면 로그 한 줄("cl9.list items 참고") + return
  const int32 Count = Args.IsValidIndex(1) ? FMath::Clamp(FCString::Atoi(*Args[1]), 1, 50) : 1;
  // 플레이어 전방에 GameMode->SpawnItem — drop.md 의 스폰과 같은 경로
}

// 2. HandleCommand 의 else-if 사슬에 분기 한 줄 (CL9CheatManager.cpp)
else if (Command == TEXT("additem")) { AddItem(Tokens); }

// 3. CL9Cheats.cpp 에 콘솔 등록 블록 하나 — 도움말 문자열은 여기 한 곳
static FAutoConsoleCommandWithWorldAndArgs GCL9CheatAddItem(
	TEXT("cl9.additem"),
	TEXT("cl9.additem <alias|number> [count] — spawns items in front of the player (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("additem"), Args, World, /*bLocalOnly*/ false);   // 로컬 읽기 전용(cl9.list)만 true
	}));
```

판정에 안 닿는 로컬 스위치(`cl9.shownoise`·`cl9.lang`)는 2·3 없이 `FAutoConsoleCommand` 람다 하나로 끝난다.

### 번호/Alias 해석 — ResolveItem / ResolveMonster

```
cl9.additem egg           → Alias "egg"
cl9.additem egg+cooked    → Alias "egg" + SubAlias "cooked"  ('+' 구분)
cl9.additem 1             → cl9.list가 마지막으로 찍은 목록의 1번
```

- 숫자 해석은 **레지스트리 등록 순**(= `cl9.list`가 찍는 순서, `Registry->GetItems()[n-1]`) 기준이다.
- 번호는 행 추가·삭제로 밀릴 수 있으므로 **타이핑 편의용**이고, 재현 가능한 리포트·스크립트에는
  Alias를 쓴다.

## 명령 목록 (코드 등록 40개 — 2026-10-21 `cl9.god`·`cl9.lookasplaced` 추가 뒤 `grep -c 'static FAutoConsoleCommand'` 기준. 이전 기록: 코드 등록 37개 + `CL9Wallet.cpp` 의 `cl9.wallet`·`cl9.skin` 2개와 1:1 — `grep -n 'TEXT("cl9\.' Source/CL9/GameMode/CL9Cheats.cpp`, 2026-10-06 — `cl9.power`·`cl9.truck` 추가, 2026-10-11 — `cl9.voice`·`cl9.voiceloop`·`cl9.voiceroute`·`cl9.talk` 추가, `cl9.join` 에 `lan`, 2026-10-12 — `cl9.voiceloop` 삭제로 37 → 36, 2026-10-16 정정 — `CL9Cheats.cpp` 의 `FAutoConsoleCommand` 등록은 37개다(`grep -c 'static FAutoConsoleCommand'`; 위 grep 은 사용법 문자열까지 세므로 쓰지 않는다))

| 명령 | 인자 | 하는 일 |
|---|---|---|
| `cl9.list <종류>` | items · monsters · characters (기본 items) — 로컬 읽기 전용(`bLocalOnly`) | 번호+Alias(+SubAlias) 출력, 뒤에 Broken 행과 사유 ([../data/schema.md](../data/schema.md) EDefStatus). 다른 종류는 경고 한 줄 — 그 표의 치트가 생기면 추가한다. 도움말 문자열은 `<items\|characters>` 로 낡아 있으나 monsters 도 받는다 |
| `cl9.additem <alias\|번호> [수량]` | 수량 1~50 | 플레이어 전방에 아이템 실물 스폰 (`GameMode->SpawnItem`) |
| `cl9.addmonster <alias\|번호> [수량]` | 수량 1~20 | 전방 바닥에 몬스터 스폰 (AI 즉시 가동). 디버그 패널(Tab)의 몬스터 버튼이 이 명령을 그대로 친다 ([ui.md](ui.md) 디버그 패널) |
| `cl9.clearmonsters` | 호스트 판정 | 월드의 몬스터를 **전부** 없앤다 — 레벨에 손으로 놓은 것(전시 우리·경기장)과 죽은 몸까지. 밤 정리와 같은 순서로 처형 중인 캐릭터를 먼저 풀어 준다(`HostEndExecuted`). 밤 몬스터 큐는 건드리지 않는다. 레벨에 놓인 것은 PIE를 다시 켜면 돌아온다. 디버그 패널(Tab)의 [몬스터 모두 지우기]가 친다 (2026-10-05) |
| `cl9.restart` | 호스트 판정 | 지금 맵을 처음부터 다시 연다 — `ServerTravel("?restart")`로 같은 맵을 같은 주소 옵션(`?listen`)으로 다시 불러 판 시각·몬스터·아이템이 레벨이 막 선 상태로 돌아간다. 판 저장(`TravelForRun`)을 거치지 않아 일차·코인은 이어지지 않는다. 클라이언트는 따라온다. 로그 `cl9.restart: reloading '<맵>' from the start.` 디버그 패널 플레이어 탭의 [처음부터 다시]가 친다 (2026-10-23 사용자 "시간부터 다시할수있게") |
| `cl9.hitmonster [데미지]` | 기본 1 · 호스트 판정 | 가장 가까운 몬스터에 데미지 — 프라이팬 히트박스가 생기기 전 킬 경보·경직 검증용으로 남아 있다 |
| `cl9.damage [양] [플레이어 번호]` | 기본 1 · 번호 생략 = 자기 | 캐릭터에 데미지 — 잡힘과 같은 OnDamage 경로(넘어짐·죽음 — 짐 흘림은 HP 0 일 때만, 2026-09-15). 번호는 접속 순서(PlayerArray 인덱스, 호스트 0)라 리슨 호스트 콘솔에서 클라를 죽여 관전을 검증할 수 있다 (4a). **2026-09-20**: `Source`로 **가장 가까운 살아 있는 몬스터**(`Dead` 제외)를 넘긴다 — 피격 사건이 진짜 잡힘과 같은 그림(카메라 요·방향 호, [character.md](character.md) "잡히는 순간 카메라가 범인을 본다")이 되고 로그에 source 이름이 찍힌다. 몬스터가 하나도 없으면 `nullptr` 그대로 → 카메라 피치만 내려가고 방향 호는 숨는다(`HasHurtDirection()` false). **넘어져 있는 동안은 무시된다** — 넘어짐 무적 게이트(`OnDamage` 머리의 `IsKnockedDown()`)가 치트에도 걸려 우회가 없다(로그 `hit by … while knocked down — ignored`). HP 0 까지 치려면 1.5초 간격으로 |
| `cl9.god [0\|1]` | 인자 없으면 뒤집기 · 호스트 판정 | 내 캐릭터 무적(2026-10-21) — `ACL9Character::HostSetGodMode`, 복제 `bGodMode`. 피해 없음(`OnDamage` 관문 — `cl9.damage` 도 막힌다), 몸 메시 숨김(모든 머신), 몬스터의 타깃·기상·잡기·덮침·처형(`target-god`)·도둑 추격·도망·근접 울음·응시·양 혼자 판정·쥐 떼 발동에서 빠지고 쫓던 몬스터는 바로 놓는다. 로그 `'<캐릭터>' god mode on/off`. 디버그 패널 플레이어 탭이 이것을 친다 ([ui.md](ui.md)) |
| `cl9.lookasplaced [0\|1]` | 인자 없으면 뒤집기 · 로컬 | 화면 프리셋 "맵 그대로"(2026-10-21) — 디버그 패널 화면 탭 맨 위 체크와 같은 `UCL9LookDef::SetLevelAsPlaced`. 룩 표의 색 보정·레트로·저해상도와 조명 액터의 페이즈 덮어쓰기·노출 고정을 걷어 맵에 배치된 그대로 보인다. 이 머신만 ([ui.md](ui.md)) |
| `cl9.night` | 호스트 판정 | 로비에서는 판을 시작하고, 판 중에는 판을 끝낸다 — 로비 UI [밤 시작] 의 대역 ([ui.md](ui.md) 로비). **2026-10-06부터 판이 레벨이 서자마자 저절로 열리므로** 지금은 로비로 가려고 판을 끝내는 데 주로 쓴다. 끝낼 때는 귀가와 같은 `EndNight` 라 **남겨짐 판정까지 돈다** — 트럭 짐칸 밖에 선 사람은 짐을 잃고 죽고(로그 `Round end: N player(s) left behind outside the truck`), 아무도 안 남으면 `CL9Rules.bWipeEndsRun` 이 켜진 동안 곧장 게임 오버다. 로비로 가려면 짐칸 안(`cl9.truck` 이 `INSIDE`)에서 친다 ([session.md](session.md) "판 끝 — 남겨진 사람") |
| `cl9.depart` | 호스트 판정 | 호스트 컨트롤러로 출발을 요청한다 — 출발 레버 두 번 누르기의 대역. `ACL9GameMode::RequestDepart` 의 판정을 `cl9.depart: Ok\|NotHost\|NotLobby\|OrdersPending\|AlreadyDeparting` 로 찍는다(Ok 면 판이 시작된다). 로비 전용 — 판 중에는 `NotLobby`. 주문은 제시되는 자리에서 자동 확정되므로 `OrdersPending` 은 이제 만나지 않는 가지다 ([session.md](session.md) 출발 레버) |
| `cl9.lever` | 호스트 판정 | 이 레벨의 출발 레버를 **호스트 캐릭터로 한 번 누른다** — 실제 누름과 같은 `CanInteract` → `Interact`. 결과를 `cl9.lever: opened-orders\|armed\|departed\|went-home (phase …)` 로 찍는다: 첫 누름은 `DepartArmTime`(5초) 준비(`armed`), 그 안의 두 번째가 실행이고 지나서 누르면 다시 준비. **로비의 두 번째 = 출발(`departed`, 판 시작), 판의 두 번째 = 귀가(`went-home` — 판이 끝나고 남겨짐 판정이 돈다).** `opened-orders` 는 로비에 제시가 남은 경우뿐이라 이제 거치지 않는다. 새벽·판 종료에는 `the lever is closed`, 레버가 없으면(트럭 패드 없음) 경고. 판에서 첫 누름의 로그가 `Depart lever: armed to go home — … (N player(s) outside the truck)` 로 트럭 밖 인원을 알려 준다 |
| `cl9.power` | 호스트 판정 | 이 월드의 전원 스위치(`ACL9PowerSwitch`)를 **호스트 캐릭터로 누른다** — 실제 누름과 같은 `CanInteract` → `Interact`(→ `RequestPowerOn`)라 사람이 E 로 누른 것과 같은 경로다. 스위치가 없는 월드(`Lvl_Flat` 등 스위치 모듈이 없는 레이아웃)는 경고 `no power switch in this world`, 판이 아니거나 이미 켰거나 정전이면 `the switch is closed (phase …, stage …)`. 켜지면 `cl9.power: T <서버 시각>, stage LightsOn — LightsOn until +7.50s, Flicker until +9.00s, lock until +9.40s, invulnerable until +10.40s; targets [id N -> module M, …]` 를 찍는다(`+7.50s` 등의 시각은 `CL9Rules` 의 `IntroLeadIn`·`IntroFlyTime`·`IntroFlickerTime`·`IntroReturnTime`·`IntroGrace` 에서 읽은 값이고, 위 숫자는 기본값 0.5/7.0/1.5/0.4/1.0 초일 때다 — 비행 7.0 초는 2026-10-10 사용자 결정). 게임 모드의 `Power on by '…' at server time …, candidates [...], targets: …` 로그도 함께 남는다 ([light.md](light.md) "전원 단계") |
| `cl9.truck` | **로컬** · 읽기 전용 | 트럭(`ACL9Truck`)의 이름·위치·요, 짐칸 상자의 중심·요·반크기(규칙 행의 중심 값과 함께)를 찍고, 플레이어마다 폰 자리가 짐칸 안인지 `INSIDE`/`outside`(죽었으면 `(dead)`)로 찍는다 — 판 끝 남겨짐 판정과 **같은 잣대**(`IsInside(폰 위치)`)다. 짐칸 상자를 **10초 동안 시안색 디버그 상자로 그린다**. 트럭이 없으면(패드 없음) 경고 — 그 월드에서는 아무도 남겨지지 않는다 ([map.md](map.md) "전원 인트로") |
| `cl9.host <alias>` | **로컬** · 시작 레벨에서도 | 그 사이트 행의 `Level` 을 리슨으로 연다(`UCL9GameInstance::HostGame` — 타이틀의 스테이지 버튼과 같은 함수). 인자가 없으면 방을 만들 수 있는 사이트(`GetHostableSites`)를 찍는다. Shipping 이 아니면 레벨을 열기 전에 EOS 로비를 세운다(2026-10-12 — 음성의 전제, [session.md](session.md) 세션 API) |
| `cl9.leave` | **로컬** | 시작 레벨로 돌아간다(`LeaveGame` — 설정 `StartLevel`, 없으면 `GameDefaultMap`). 호스트가 치면 접속한 클라이언트가 끊긴다. 들어가 있던 EOS 로비도 떠난다 |
| `cl9.join <ip[:port]> \| lan \| cancel` | **로컬** · 개발용 | 그 주소의 리슨 호스트에 접속한다(`JoinGame`, 포트 기본 7777) — **로비를 거치지 않으므로 이 연결에는 음성이 없다**(로그 `Voice: no session — remote voice disabled on this connection`). **`lan`** 은 2026-10-12 부터 **EOS 로비 찾기**다 — 호스트의 로비를 찾아(`JoinGame: searching for a lobby.`) 첫 로비에 참가하고 그 접속 주소로 이동한다(`Voice: session joined GameSession`, `JoinGame: connecting to <주소> (lobby).`). 음성이 된다. 로비가 없으면 경고 `JoinGame: no lobby joined — …`. 2026-10-11 의 Null LAN 찾기(`Voice: lan search found <n>`)는 없어졌다. `cancel` 은 진행 중인 접속·로비 찾기를 버리고(`CancelJoin`) 지금 레벨에 남는다. 인자가 없으면 접속 중 여부(`IsJoining` — 로비 찾기·참가 중에도 `yes`)를 찍는다 — 주소 접속은 다음 엔진 틱에 시작되므로 친 직후 한 틱은 아직 `no` 다 ([voice.md](voice.md) "전송과 세션 — EOS 로비") |
| `cl9.voice` | **로컬** · 읽기 전용 | 이 머신의 음성 상태를 찍는다(2026-10-12 형식 — 엔진 `ONLINE VOICE DUMP`·`local talker`·`mic level`·`heard` 열은 없어졌다): 머리 한 줄 `cl9.voice: session yes\|no, enabled <0\|1>, mode VA\|PTT, transmit <0\|1>, local talking yes\|no, net mode <n>.` + 플레이어마다 한 줄 `cl9.voice:   [n] '<이름>' route prox\|direct(<상대>) alive\|dead talking yes\|no muted-by [..]`. `session` 은 `HasVoiceSession()`(EOS 로비 안), `transmit` 은 `IsVoiceTransmitting`(켜짐 ∧ (음성 감지 ∨ PTT 누름)), `muted-by` 는 이 머신이 아는 것뿐이다 — 뮤트 목록이 듣는 사람 머신에만 복제되므로 호스트는 전원, 클라이언트는 자기 목록만. 세 창에서 `route` 가 같아야 한다(A6) |
| `cl9.voiceroute prox \| direct <n>` | 로컬 → **호스트 판정** | 내 핫라인을 잇거나 끊어 달라고 청한다 — `RequestVoiceRoute`: 호스트면 `HostSetVoiceRoute` 직접, 클라이언트면 `Server_SetVoiceRoute` RPC(`Server_CL9Cheat` 가 아니다). `n` 은 `cl9.damage` 와 같은 PlayerArray 번호(호스트 0, 접속 순). **핫라인이 없으면 말하는 사람 주변이 3D 로 듣는다. 핫라인 ①(2026-10-12): 연결된 동안 말하는 사람의 목소리는 받는 사람 무전기 자리(상대의 폰)에서만 난다 — 말하는 사람 곁에서는 들리지 않는다.** **`direct <n>` 은 나와 n 을 양방향으로 잇는다**(두 무전기 스피커의 연결) — 서로의 말이 상대 무전기 자리에서 난다: 상대는 자기 자리에서, 상대 주변 사람도 거기서 듣는다. **상대가 죽어 있어도 시체 자리에서 울린다**(폰이 없으면 없음). 한 사람은 하나만 가져서 **새 연결이 두 사람의 옛 연결을 끊고**(옛 상대는 근접만 남는다), **`prox` 는 내 연결을 끊는다(상대도 근접)**. 한쪽이 나가도 끊긴다. 로그는 바뀐 사람마다 `Voice route: <p> -> direct(<상대>)` / `-> prox` 한 줄이라 연결 한 번이 두 줄이다. 자기 자신·없는 번호는 경고 ([voice.md](voice.md)) |
| `cl9.talk on\|off` | **로컬** | **HUD 확인용** — 마이크 없이 이 머신의 로컬 플레이어를 "말하는 중"(`IsVoiceTalking()` 참)으로 둔다(마이크 아이콘·`HUD talking:` 로그). 판정·복제·소리에는 닿지 않는다 — 아무것도 보내지 않으므로 다른 머신의 HUD 에는 뜨지 않는다. 인자가 없으면 지금 값을 찍는다. 자동 PIE 가 HUD 를 마이크 없이 돌리는 도구다 |
| `cl9.blackout` | 호스트 판정 · 밤 중에만 | 정전 시각을 지금으로 당긴다 — 손전등·시설 조명 소등 검증 ([light.md](light.md) 구현 상태) |
| `cl9.music` | 로컬 · 읽기 전용 | 이 머신에서 지금 나오는 배경음악과 목적지 바탕의 `SoundTable` 행·애셋 이름을 로그와 화면에 띄운다 ([sound.md](sound.md)) |
| `cl9.day <N>` | 로비에서만 | 로비의 일차를 N으로 — 다음 `cl9.night`가 그 일차의 곡선으로 굴리고, 건너뛴 날의 주문 마감을 처리한 뒤(위약금) 제시를 다시 뽑는다 (4b 구현 · 4e 마감 처리, 구 bp_debug_day). 제시는 뽑히는 자리에서 자동 확정되므로(2026-10-06) **확정된 주문이 하나 더 쌓인다** |
| `cl9.orders` | 로컬 읽기 | 로비 제시(번호·재료·보상·마감)와 수락한 주문(마감 일차) 출력 (4c). 제시는 늘 비어 있고 확정된 주문만 찍힌다 |
| `cl9.pick <n> [m] …` | 로비에서만 | 제시 번호로 주문 확정 — 정확히 M개 (주문 선택 화면의 스탠드인, 4c). **2026-10-06부터 제시가 뽑힌 자리에서 바로 확정돼 비므로 늘 `AcceptOrders: nothing is on offer` 경고로 거부된다**(판 중에는 `orders are picked in the lobby only`) |
| `cl9.buy [alias\|번호]` | 호스트 판정 | 인자가 없으면 장비 목록(가격·해금 일차·보유 수·구매 가능 여부)을 출력하고, 있으면 산다 — 트럭 패널의 스탠드인 (4f) |
| `cl9.eat` | 호스트 판정 | 활성 퀵슬롯 칸의 요리를 먹는다 — 좌클 길게 누르기(먹기)의 대역 (4i). 품·교체 치트는 없다 — 퀵슬롯 전환은 휠·숫자키가 곧 테스트 경로다 |
| `cl9.settle [force]` | 호스트 판정 | 정산 상태(일차·목표·기간 수입) 출력 + 정산일이면 판정 실행; `force`면 정산일이 아니어도 판을 지금 끝낸다 (정산) |
| `cl9.money [양]` | 호스트 판정 | 판 공유 자금(GameState.SharedMoney) 출력; 양이 붙으면 지급(양수)하거나 차감(음수) — 4f 구매 검증 대역 (4e 구현) |
| `cl9.wallet [delta]` | **로컬** | 이 머신의 바깥 재화 지갑(화면 이름 "이더")을 찍는다 — 슬롯(`CL9Wallet` / PIE 인스턴스 1 이상 `CL9Wallet_PIE<n>`)·보유·이번 판 지급. delta 가 있으면 더하고 저장한다(0 아래로는 안 내려간다). 시험용 (2026-10-12 7f, [save.md](save.md) 지갑) |
| `cl9.imposter [번호]` | 호스트 판정 | 인자 없이: 지금 임포스터와 다음 판 지정을 찍고, 도착 판의 굴림이 보류 중이면 `roll pending (arrival)` 줄을 더 찍는다. 번호(플레이어 목록 순서, 0 = 호스트): 판 중이면 그 사람을 지금 임포스터로(다른 사람은 거둠), 로비면 다음 판 한 번에 쓴다. **판 중이라도 도착 판의 굴림이 보류 중이면**(2026-10-14) 지금 정하지 않고 보류 중인 굴림이 그 지정을 쓴다 — 지금 정하면 뒤에 오는 굴림이 역할을 거두기 때문이다. -1 = 없음. 2인 판(확률 0)의 시험용 ([../goal/loop-7g-imposter.md](../goal/loop-7g-imposter.md)) |
| `cl9.skin [alias]` | **로컬** | 이 머신 지갑의 스킨 목록(보유·입음·가격). alias 면 필요하면 사서(이더) 입고 호스트에 알린다. 치트 매니저를 거치지 않아 클라이언트에서도 된다 ([../goal/loop-7k-skins.md](../goal/loop-7k-skins.md)) |
| `cl9.save [슬롯]` | 호스트 판정 | 판(일차·이더·기간 수입·진행 중 주문·보유 장비)을 지금 그 슬롯에 저장 — 로비 [이어하기] UI 의 대역 ([save.md](save.md)) |
| `cl9.load [슬롯]` | 호스트 판정 · 로비에서만 | 그 슬롯에서 판을 되돌린다 |
| `cl9.seed <N>` | 0 = 외곽만 | 지금 그 시드로 맵을 다시 깔고 다음 밤도 그 시드로 고정 (호스트). **PIE 없이 에디터 콘솔에서 치면** 같은 생성기로 레벨에 트랜지언트 액터를 깐다 — 5a 맵 배치 툴의 창 없는 판 (`cl9.seed clear`로 제거, 두 번째 인자로 목적지 alias) |
| `cl9.wave <alias\|enum>` | 호스트 판정 | 내 발밑에서 그 소음원의 파동을 한 번 낸다 — `run` · `pickup_loud` 처럼 표의 alias나 enum 이름(`PickupLoud`)을 받는다 ([wave.md](wave.md) 소음원 enum). 발신자가 사람이라 `monster_call`은 읽을 몬스터 행이 없어 조용히 끝난다 |
| `cl9.execute` | 호스트 판정 | **가장 가까운 살아 있는 처형 종**(`MonsterTable` 의 `ExecuteRadius > 0` 또는 `bExecuteOnCatch` — `IsExecutioner()`; `Dead`·`Execute` 제외) 몬스터가 나를 처형한다. 스토커(너구리)는 스토킹 계수기·거리·등 뒤·미발견·시야·`KillCooldown` 검사를 **우회**, 타깃 상태(넘어짐·처형 중·죽음)와 **앵커 스윕**·`HostBeginExecuted` 는 그대로; 잡히면 처형인 종(돼지)은 타깃 상태 검사만 하고 앵커 스윕·텔레포트 없이 그 자리에서 즉시 진입 ([monster.md](monster.md) 스토킹과 처형). 처형 종이 없으면 로그 한 줄(`cl9.addmonster pig` 또는 `raccoon` 먼저), 앵커가 막히면 몬스터가 사유(`anchor-blocked`)를 찍는다. 내가 죽었거나 처형 중이면 무시 |
| `cl9.swingtaps <N> [interval_ms]` | 호스트 판정 · interval 기본 100 | **휘두르기 입력 유실 측정**(goal/combat-feel-pass-1.md D-8 ④). 호스트 폰에 월드 타이머로 N 번 `ACL9Character::HostCheatSwingTap()`(= `Server_UseStart`+`Server_UseRelease` 와 같은 경로, 누르고 바로 뗌)을 넣고 탭마다 `cl9.swingtaps: tap=<i> t=<서버초 .3f> result=<swing\|buffered\|lost>`(swing = 즉시 휘두름, buffered = 회복 끝에 예약, lost = 거부)를, N 번째 뒤에 `cl9.swingtaps: N=<N> interval_ms=<ms> swing=<a> buffered=<b> lost=<c> recovery=<SwingRecovery> buffer=<SwingBufferTime>` 을 찍는다. 시각은 틱 위상이 아니라 서버 시각. 실행 중이면 새 요청은 거부(`a run is still in progress`). 손에 `Swing` 행이 없으면 전부 lost 다 — `cl9.additem frying_pan` 먼저 |
| `cl9.swinglog [0\|1]` | 호스트 판정 · 인자 없으면 토글 | 휘두르기마다 히트 구간이 닫히는 틱(구간 끝 또는 손이 바뀜)에 `cl9.swing: start=<t> input=<t> hit=<t\|-> surface=<t\|-> ready=<t> arm_return=<start+0.347> tested=<n> occluded=<m>` — start = `SwingStartServerTime`, input = 받아들인 뗌의 서버 시각(예약이면 뗀 순간), hit = 첫 명중 `SwingHitServerTime`, surface = `SwingSurfaceHitServerTime`, ready = `SwingReadyTime`(첫 명중 뒤 `+HitStopTime` 반영), arm_return = start + `SwingClipWindow`, tested = 스윕에 걸려 가림 검사까지 간 살아 있는 몬스터 수(틱 합계), occluded = 그중 지형에 가려 빠진 수. 회복 중이라 거부된 뗌(예약도 안 됨)은 `cl9.swing: rejected t=<t> ready=<t>`. 스위치는 `ACL9Character::bCheatSwingLog`(정적, 호스트) |
| `cl9.knockbacklog [0\|1]` | 호스트 판정 · 인자 없으면 토글 | 몬스터 넉백(타격 `HitKnockback`·밀침 `PushKnockback` 둘 다)이 끝나는 틱에 `cl9.knockback: <alias>#<UniqueID> from=(x y z) to=(x y z) dist=<수평 cm .1f> steps=<n> blocked=<0\|1> hitstop=<지연 s> time=<총거리/속도 s>` — from = Stun 진입 자리, to = 멈춘 자리, steps = 실제로 옮긴 틱 수, blocked = `TraceWorld` 또는 스윕이 막아 조기 종료, hitstop = 이동 전 대기(타격 `HitStopTime`, 밀침 0). 스위치는 `ACL9Monster::bCheatKnockbackLog`(정적, 호스트) |
| `cl9.griplive [0\|1]` | 호스트 판정 · 인자 없으면 토글 | 켜져 있으면 손에 든 아이템이 매 틱 `ItemTable`의 `GripOffset`/`GripRotation`을 다시 읽어 적용한다 — 꺼져 있을 때는 손에 붙는 순간(줍기) 한 번만 적용되므로, 값 튜닝 중 PIE를 다시 켜거나 놓았다 줍지 않아도 회전이 바로 보인다. 스위치는 `ACL9Item::bCheatGripLive`(정적, 호스트) |
| `cl9.shownoise` | 로컬 전용 | 소음 판정 반경을 디버그 원으로 그리고, 파동마다 반경·차폐 광선 수를 `LogCL9`에 찍는다 — 링이 멈추는 자리와 판정 반경이 같은지 대조하는 용도 ([wave.md](wave.md) PIE 확인) |
| `cl9.wavestyle N` | 로컬 전용 (콘솔 변수) | 파동을 `WaveTable.Styles[N]` 벌로 그린다(0부터, -1 = 표의 `DefaultStyle` 벌, -2 = 전역 `Dots`·`Ripple` 점만). 살아 있는 파면도 다음 틱부터 그 벌이다. 디버그 패널 "파동" 탭이 같은 값을 바꾼다 ([wave.md](wave.md) 모양 벌, 2026-10-22) |
| `cl9.lang [ko\|en\|ja\|zh-Hans]` | 로컬 전용 (에디터/PIE는 게임 언어 미리보기) | 인자 없으면 현재 언어 출력, 있으면 지금 바꾸고 `GameUserSettings.ini`에 저장 — 설정 화면 언어 항목과 같은 `UCL9GameUserSettings::ApplyLanguage`를 부른다 ([localization.md](localization.md)) |
| `cl9.shownav` | **미구현** | 몬스터 이동·경로 시각화 (구 bp_show_cells) |
| `cl9.showjars` | **미구현** | 항아리 배치·1500u 커버리지 확인 (구 bp_show_jars) — 2026-09-07 확인: 소스 어디에도 없다 |

삭제된 계획 명령(2026-09-21 감사): `cl9.resetkeys`(키 초기화는 설정 화면 [키 초기화] 버튼 `HandleResetKeysClicked` 가 한다 —
[settings.md](settings.md)) · `cl9.playseq`(시퀀스 시스템 미구현) · `cl9.testwake`/`cl9.testvoice`/`cl9.fxoff`(연출 시스템 없음).
필요해지면 위 절차로 만든다.

목록은 열려 있다 — 스텝을 만들며 그 스텝을 검증하는 치트를 같이 추가한다
(예: ② 던지기에서 `cl9.breaktest`, ③ 발각에서 `cl9.showdetect`).

## 네트워크 정리

| 상황 | 동작 |
|---|---|
| 호스트 콘솔에서 침 | CheatManager 함수가 바로 판정 실행 |
| 클라 콘솔에서 침 | `Server_CL9Cheat(명령줄)` → 호스트 CheatManager가 해석·실행 |
| 로컬 전용 (`cl9.list`·`cl9.truck`·`cl9.shownoise`·`cl9.wavestyle`·`cl9.lang`·`cl9.voice`·`cl9.talk`) | 로컬 읽기·렌더·설정 — RPC 안 탄다 |
| 세션 이동 (`cl9.host`·`cl9.leave`·`cl9.join`) | 친 머신의 게임 인스턴스가 바로 레벨을 옮긴다 — RPC 안 탄다. `cl9.host`·`cl9.join lan` 은 그 머신이 EOS 로비를 세우거나 찾아 참가한다(2026-10-12 — 블루프린트 브리지, [voice.md](voice.md)) |
| 음성 경로 (`cl9.voiceroute`) | 친 머신의 컨트롤러가 `RequestVoiceRoute` — 호스트면 바로, 클라면 `Server_SetVoiceRoute`(이번 실행의 유일한 새 Server RPC). 호스트가 `HostSetVoiceRoute` 로 판정 |
| 결과 | 스폰물은 표준 복제로 전 클라에 나타난다 — 유령 액터 없음 |

## 참고

- 스폰 경로: [actor-base.md](actor-base.md) (SetAlias → LoadData) · [drop.md](drop.md)
- 데이터 조회: [../data/schema.md](../data/schema.md) (`UCL9DefRegistry`)
- 각 시스템 문서의 디버그 절이 이 접두 체계를 쓴다: [balance.md](balance.md) ·
  [map.md](map.md) · [warning.md](warning.md)
