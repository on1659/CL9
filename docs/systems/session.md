# 세션 및 매칭 시스템

> 로비에서 게임 시작까지의 흐름. 현재는 구현하지 않음 — 문서만 사전 작성.

**2026-10-12(루프 개편 7a, [../goal/loop-7a-wipe-penalty.md](../goal/loop-7a-wipe-penalty.md)): 의도값이 false 로 돌아왔다** — 헤더·셀 모두 false.
전멸은 게임 오버가 아니라 그 판만 끝내고 누적 이더를 깎는다(사용자 2026-10-11 번복, 30% 는 2026-10-12). 아래 "켜져 있으면" 경로는 2026-10-05 규칙을 셀 하나로 되돌리는 스위치로 남는다.

- **꺼져 있으면(의도값)** — 밤 중 전멸 → `HandlePlayerDeath` 안에서 즉시 `EndNight` → 새벽·남겨짐 판정 뒤 **전멸 패널티** → 같은 호출 안에서 다음 날 로비(전원 부활, 아래 "구현됨 — 밤 수명주기").
- **전멸 패널티 — `ACL9GameMode::ApplyWipePenalty()`**: `EndNight` 가 `LeaveBehindOutsideTruck()` 이 false 를 돌려준 뒤 부른다 — 산 사람이 없을 때만
  `Taken = GameState->Deduct(Rules->WipePenaltyFor(SharedMoney))`(floor(잔액 × `CL9Rules.WipePenaltyRate` 0.30), 잔액 하한 0). **기간 수입은 그대로** — 정산 판정에 영향이 없다.
  판 중 전멸과 판 끝에 전원이 남겨진 경우(아래 절)가 같은 자리를 지나 **한 번**만 깎는다. 켜져 있으면 판이 끝나 이 자리까지 오지 않는다.
  로그 `Wipe penalty: 30% of <전> ether — <N> taken. Money <전> -> <후>.` 결과 `FCL9WipePenalty{bActive, Taken}` 를 `ACL9GameState::SetWipePenalty` 로 싣는다
  (복제 `WipePenalty` + `OnWipePenaltyChanged` — 클라이언트는 OnRep, 호스트는 setter 가 직접 발동) → 새벽 화면이 `전멸 — 이더 N 차감` 한 줄([ui.md](ui.md)).
  다음 `StartNight` 가 남겨진 이름과 함께 비운다. `EnterLobby`(새벽 화면·정산·저장)보다 먼저라 깎인 잔액이 그날 세이브에 들어간다.
### 임포스터 (2026-10-12, [../goal/loop-7g-imposter.md](../goal/loop-7g-imposter.md))

- **배정** — `StartNight` 의 스폰 뒤 `AssignImposter()`: 판을 시작할 때 살아 있는 사람 N 명으로 `CL9Rules.ImposterChanceFor(N)`(3인 10%·4인 25%, 1·2인 0)을 굴려
  한 명을 고르게 고른다. `cl9.imposter` 지정이 있으면 그것을 한 판 쓴다. 역할은 `ACL9PlayerState::bImposter` — **본인 머신에만 복제**(`COND_OwnerOnly`)라
  다른 머신은 새벽 공개 전까지 모른다. 로그 `Imposter: '이름' this round (…)` / `Imposter: none this round (…)` 는 호스트 머신에만 남는다.
- **판정** — `EndNight` 에서 **남겨짐보다 먼저** `JudgeImposter()`. 이긴다 = 임포스터가 살아서 트럭 짐칸 안(트럭 없는 레벨은 안으로 본다), 다른 사람이 한 명 이상이고
  모두 이미 죽었고, 그 누구의 `KilledByPlayer` 도 임포스터가 아니다. 트럭 밖에 산 사람은 이 시점엔 "산 사람"이라 남겨짐 죽음은 승리가 되지 않는다(2026-10-12 사용자).
  죽은 임포스터는 `KilledByPlayer` 가 있으면 **처치**(그 사람이 보상), 없고 그를 죽인 몬스터를 깨운 사람(`InducedByPlayer`, 7h)이 있으면 그 사람의 처치, 둘 다 없으면 실패. 결과는 `ACL9GameState::SetImposterResult`(복제 `FCL9ImposterResult`)로 싣고 역할을 거둔다.
  로그 `Imposter: '이름' won …` / `… was killed by '이름'.` / `… failed — 이유.`
- **보상** — `GrantRoundWallet` 의 한 번 지급에 접는다: 이긴 임포스터는 귀가 보상 대신 `ImposterWinReward`, 임포스터를 죽인 사람은 `ImposterKillReward` 를 더 받는다.
  판의 임포스터는 이겼든 졌든 자기가 깨운 몬스터에게 죽은 사람 하나마다 `ImposterAssistReward` 를 더 받는다(7h).
- **처치 기여 목록**(7h) — `PublishContributions()` 가 판 끝에 죽어 있는 사람 중 `InducedByPlayer` 가 있는 사람마다 한 건(죽은 사람·깨운 사람·몬스터 이름)을
  `ACL9GameState::SetContributions` 로 싣는다(새벽 화면). 판 시작에 비운다.
- **알 수 있는 한계** — 출발 레버는 호스트 전용이라 호스트가 죽은 판에서 클라이언트 임포스터는 판 시계가 끝날 때 떠난다.
  스테이지의 **첫 판**은 레벨이 서자마자 시작하고(`StartPlay` → `StartNight`) 클라이언트는 그 뒤에 접속하므로 굴림이 호스트 한 명만 보고 임포스터가 나오지 않는다 —
  레버로 시작하는 둘째 판부터 전원이 후보다(2026-10-12 확인, 첫 판 굴림 시점은 나중에 정한다).

### 회사 — 레벨을 건너는 판 (2026-10-12, [../goal/loop-7i-company.md](../goal/loop-7i-company.md))

- **언제**: 정산일(`IsSettlementDay(일차)`) 로비에서 출발 레버(`RequestDepart`, `cl9.depart`)가 판 대신 **회사로 간다**(`IsCompanyDue`). 새벽 화면이 "오늘은 정산일 — 출발 레버를 당기면 회사로" 줄을 보인다.
- **이동**: `ACL9GameMode::TravelForRun` — `SaveRun(1)` → 게임 인스턴스 `SetRunCarry(떠나는 사이트)` → **다음 틱에** 비심리스 `ServerTravel(레벨?listen)`(상대 이동).
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
- **레버로 연 판**은 트럭이 이미 서 있어 도착이 없다. **판 시계는 판 시작(= 도착 시작)부터** 흐른다 — 마당에서 머뭇거리는 만큼 손해(시간 압박, 사용자 답).

## 개요

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
  바꾼다 — [character.md](character.md) 죽음 절.

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
