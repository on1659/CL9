# UI 시스템 (개발)

> UI 코드 구조. **뷰포트에 붙는 위젯은 `WBP_Main` 하나뿐**이고, 하위 UI는 UIManager가
> **UI 표를 읽어 열 때 생성해** 레이어에 붙인다 (2026-09-05 결정 — WidgetSwitcher 방식
> 번복, [../review/ui-structure-2026-09-05.md](../review/ui-structure-2026-09-05.md)).
> UI 추가는 WBP 제작 + 표에 행 추가다 — C++을 고치지 않는다.

## 구조

```
WBP_Main (UCL9MainWidget)        ← 뷰포트에 붙는 유일한 위젯 (로컬 전용)
  ├─ HUD 레이어                   디자이너에 직접 배치 — 배선(TryWire)된 뒤로 항상 표시
  │                               (밤 시계 · 이더 · 퀵슬롯 · 주문 · 상호작용 프롬프트)
  ├─ Layer_Screen (CanvasPanel)   전면 화면 — **배타** (하나 열면 이전 것이 닫힌다)
  └─ Layer_Popup  (CanvasPanel)   중첩 허용 (확인 대화상자·토스트가 생기면 여기)

UCL9UIManager (GameInstanceSubsystem — 싱글턴 접근 Get())
  ├─ UI 표: UCL9UITable (FName → {위젯 클래스, 레이어}) — BP_CL9GameInstance가 주입
  ├─ OpenWidget(Name) / CloseWidget(Name) / IsOpen(Name) / GetOpenScreen()
  ├─ OpenRootScreen(Name) / IsRootScreen(Name) — 닫히지 않는 바닥 화면(타이틀·게임 오버)
  └─ 입력 모드 · IMC_UI 전환 — "Screen 레이어가 비었는가" 하나로 판단

UCL9Widget : UUserWidget          ← 모든 CL9 위젯의 베이스 (UIName + CloseSelf)
```

- 전면 화면(로비 · 새벽/정산 · 관전 · 트럭 패널 · 메뉴)은 전부 Screen 레이어 행이다.
- **새 UI는 표에 행으로만 추가한다.** 뷰포트에 직접 붙이는 위젯을 늘리지 않고, `WBP_Main`도
  다시 열지 않는다 — 독립 WBP를 만들고 `UITable`에 행을 넣으면 끝이다.
- **크기는 전역 배율로 잡는다** — 프로젝트 설정 UI `ApplicationScale` 1.4(2026-09-05 사용자 "UI 좀 크게").
  위젯은 1080p·배율 1.0 기준으로 디자인하고(글자 기본 24pt) 전체 크기 조정은 이 값 하나로 한다.
  DPI 곡선(1080p=1.0, 720p=0.666)은 기본값 그대로 — 작은 PIE 창에서 작게 보이는 것은 곡선 때문이다.

## 코드 구조

```cpp
// GameInstanceSubsystem — UI는 로컬 전용이라 GameInstance 수명이 정확히 맞는다
class UCL9UIManager : public UGameInstanceSubsystem {
public:
  static UCL9UIManager* Get(const UObject* WorldContext);   // 싱글턴 접근

  void InitForPlayer(APlayerController* PC);  // 로컬 PC의 BeginPlay — WBP_Main을 짓는다(낡았으면 다시)
  UCL9Widget* OpenWidget(FName UIName, bool bResetHistory = false);   // 표에서 클래스·레이어 조회 → 생성 → 부착
  UCL9Widget* OpenRootScreen(FName UIName);  // 히스토리를 비우고 열고, 이름을 루트로 표시
  bool IsRootScreen(FName UIName) const;
  void CloseWidget(FName UIName);         // 루트 화면이면 아무것도 하지 않는다
  bool IsOpen(FName UIName) const;
  FName GetOpenScreen() const;            // 열려 있는 Screen 행 이름 (None이면 없음)
};
```

- **레벨 이동 뒤에는 UI를 처음부터 다시 짓는다** (2026-10-05 start-lobby L3). `CreateWidget`의 outer는
  GameInstance라서 `WBP_Main`은 월드와 함께 죽지 않는다 — 엔진은 월드를 내릴 때 뷰포트에서 떼기만 한다. 그래서
  하드 트래블(시작 레벨 → 스테이지, 스테이지 → 시작 레벨) 뒤에는 **유효하지만 뷰포트 밖이고 주인 컨트롤러가 죽은**
  메인 위젯이 남는다. 옛 코드는 `IsValid`만 봐서 다시 짓지 않았고, 그 뒤의 `OpenWidget`이 전부 nullptr였다.
  지금 `InitForPlayer`는 `!IsValid(MainWidget) || OwningPlayer != 이 PC || !MainWidget->IsInViewport()`이면 낡았다고
  보고 — 열린 위젯 전부와 메인 위젯을 부모에서 떼고, 열린 목록·화면 이름·히스토리·루트 표시를 비운 뒤 새로 짓는다.
  지을 때마다 로그 한 줄: `UIManager: bootstrapped for <PC> in <World>` (LogCL9, Log). 같은 PC가 두 번 불러도
  낡지 않았으면 아무것도 하지 않는다.
- **루트 화면** (2026-10-05 start-lobby L3): `OpenRootScreen(Name)`은 `OpenWidget(Name, true)`로 히스토리를 비우고 연 뒤
  이름을 루트로 표시한다. `CloseWidget`은 루트 이름을 받으면 **아무것도 하지 않는다** — M(`PauseInput`)·[X]·`CloseSelf`가 전부
  이 함수로 오므로 타이틀과 게임 오버는 이 셋으로 닫히지 않는다. 위에 다른 Screen을 열면 평소처럼 대체되고(타이틀 → 설정),
  그 화면을 닫으면 히스토리가 루트를 새 인스턴스로 되살린다. 루트 표시는 다음 부트스트랩(레벨 이동)에서 지워진다.
  루트 규칙이 매니저 안에 있어서 `PauseInput`은 고치지 않았다. 지금 루트로 여는 곳은 둘 — 시작 레벨의 컨트롤러가 메뉴
  GameMode의 `TitleUIRow`(`Title`)를, HUD가 `GameOver`를 연다.

- **Screen 레이어는 배타다**: `OpenWidget`이 Screen 행이면 열려 있던 Screen 위젯을 먼저
  닫는다. "전면 화면은 동시에 하나"라는 기획(ui.md)이 레이어 정책으로 지켜진다.
- **화면 히스토리** (2026-09-12 사용자 "레이어가 쌓이면 하나씩 뒤로"): 매니저는 Screen 이름의 스택
  `ScreenHistory`를 든다. 위젯은 여전히 하나만 살아 있고(열 때 생성, 닫으면 파괴) 스택에는 **이름만** 쌓인다.
  - `OpenWidget(Screen)`: 그 이름이 이미 스택에 있으면 거기까지 잘라내고(관전 → 메뉴 → 관전 재파생 같은 순환
    방지), 현재 위젯을 닫고, 이름을 밀어 넣고, 새로 만든다.
  - `CloseWidget(name)`: 스택 꼭대기면 꺼내고 **이전 이름을 다시 연다**(새 인스턴스 — 화면은 열릴 때 현재 상태로
    초기화한다는 규칙 그대로). 꼭대기가 아니면(메뉴가 위에 있는데 관전 플래그가 꺼짐) 스택에서만 뺀다.
    `GetOpenScreen()`은 꼭대기.
  - M(`PauseInput`)은 "열린 Screen이 있으면 `CloseWidget(꼭대기)`, 없으면 `OpenWidget(Pause)`" — 한 단계 뒤로.
    옛 관전 특례(M이 관전 화면을 "없음"으로 취급하고 닫을 때 다시 열던 것)는 스택이 대신한다: 관전 → 메뉴 → 설정 →
    M → M → 관전. 메뉴의 [계속] 버튼(`CloseSelf`)도 같은 경로라 관전 화면이 복원된다(2026-09-12 전에는 M으로 닫을 때만
    복원되던 공백).
  - 서버가 여는 화면(새벽·트럭 패널)도 같은 스택을 탄다 — 새벽 화면이 닫히면 그 전에 열려 있던 메뉴로 돌아온다.
    상태 파생 화면(주문 선택)은 여전히 다른 Screen이 열려 있으면 양보한다.
- `UCL9Widget` 공통 (2026-09-12 사용자 "X 버튼… 공통 기능은 전부 UCL9Widget에서"): `CloseButton`
  (`BindWidgetOptional` — WBP에 그 이름의 버튼이 있으면 `CloseSelf`에 자동 배선, 없으면 무시)과
  `protected virtual OnClosing()` — 매니저가 닫아 `PlayOutro`가 시작되는 순간 한 번 불린다. 페이드 뒤에 오는
  `NativeDestruct`보다 먼저 정리해야 하는 것(설정 화면의 키 캡처 프리프로세서)이 여기에 건다. 기존 화면의
  자체 닫기 버튼([계속]·[닫기])은 그대로 `CloseSelf`를 부른다.
- **상태에서 파생되는 화면은 사람이 연 화면에 양보한다** (2026-09-07). 배타 규칙 때문에
  파생 화면이 아무 때나 열리면 방금 열린 화면을 밀어낸다 — 실제로 새벽 결과 화면이 로비 진입과
  같은 순간에 열리는 주문 선택 화면에 즉시 밀려서, 플레이어가 결과를 읽을 새가 없었다.
  그래서 `DeriveOrderScreen`은 **다른 Screen이 열려 있으면 열지 않고**, 그 화면이 닫히면
  다시 연다(닫힘 이벤트가 없어서 HUD 틱에서 다시 파생한다). 게임 진행(주문 추첨)은 그대로 두고
  화면 순서만 UI가 정하는 방식이다 — 반대로 게임이 UI를 기다리게 하면 진행이 화면에 묶인다.
- 위젯은 **열 때 생성하고 닫으면 파괴한다.** C++에서 열어야 하는 잘 알려진 이름(메뉴 등)은
  `CL9UINames` 상수로 둔다 — 문자열 오타를 컴파일 타임에 잡지는 못하므로 상수만 쓴다.
- `WBP_Main`은 로컬 PlayerController의 BeginPlay에서 한 번 생성해 `AddToViewport()` 한 번만
  부른다. UI 표·메인 위젯 클래스는 에셋이 가진다 — C++에 경로가 없다 (conventions § 5).
- 입력 모드(GameOnly ↔ GameAndUI + 커서)와 `IMC_UI` 부착·해제는 Open/Close 안 한 곳에서
  처리한다 — IMC 상태가 화면 상태와 어긋나는 버그를 원천 차단 ([input.md](input.md)).

## 스스로 짓는 위젯 (2026-10-05)

타이틀(`UCL9TitleWidget`)과 게임 오버(`UCL9GameOverWidget`)는 **WBP에 아무것도 없어도 화면이 선다.** 디자이너 트리 대신
C++이 위젯 트리를 짓기 때문이다 — WBP를 디자이너로 꾸미는 대신 MCP 호출 세 번(부모 클래스로 만들기 · 컴파일 · 저장)으로
껍데기만 만들면 되고, `BindWidget` 이름이 어긋나 컴파일 모달이 뜰 일도 없다(start-lobby L10).

- **언제 짓나**: `NativeOnInitialized`에서. 슬레이트 위젯은 그 뒤 `TakeWidget`이 `WidgetTree->RootWidget`에서 만들므로,
  `NativeConstruct`에서 루트를 바꾸면 이미 늦다. 엔진의 `UUserWidget::Initialize`는 네이티브 클래스에도 빈 `WidgetTree`를
  만들어 주고(루트 null), `CreateWidget(PC, …)`가 플레이어 문맥을 넣어 주므로 `NativeOnInitialized`가 반드시 불린다.
  디자이너 미리보기에서는 불리지 않는다.
- **어디에 싣나**: 루트가 비었으면 지은 덮개(`UBorder`)가 루트가 된다. 껍데기 WBP가 빈 패널(기본 캔버스) 하나를 들고
  왔으면 그 안을 (0,0,1,1)로 꽉 채워 붙인다. 디자이너가 이미 무언가를 놓았으면 짓지 않고 경고 한 줄을 남긴다.
- **필수 바인딩이 없다**: `UCL9Widget`의 가정은 둘 다 선택 사항이다 — `CloseButton`은 `BindWidgetOptional`이라 없으면
  무시되고(루트 화면이라 어차피 닫히지 않는다), `SlidePanelName`은 생성자 기본값이 지은 안쪽 패널의 이름
  (`TitlePanel`·`GameOverPanel`)이라 덮개는 두고 패널만 올라온다. `ShowAnim`은 `UCL9Widget`에 없다(스캔 라벨만 쓴다).
- **모양은 기존 화면 값을 따른다**: 버튼 바탕 (0.16, 0.16, 0.16, 0.92), 흰 글자, 제목 30 Bold · 버튼 20 Regular · 최소 16
  ([ui.md](ui.md) 글자 위계). 색·크기·간격은 전부 위젯 프로퍼티라 WBP CDO에서 바꿀 수 있다. 문구는 `LOCTABLE` 리터럴.
- **버튼 배선**: 값이 없는 버튼은 위젯의 `UFUNCTION`에 바로, 값이 다른 버튼(타이틀의 목적지)은 릴레이 UObject
  (`UCL9TitleStageRelay`)에 — 디버그 패널과 같은 방식이다.
- **UI 표에 무엇을 넣나**: 껍데기 WBP(`WBP_Title_C` · `WBP_GameOver_C`)든 네이티브 클래스(`/Script/CL9.CL9TitleWidget` ·
  `/Script/CL9.CL9GameOverWidget`, 둘 다 비추상)든 같은 화면이 뜬다.

## 데이터 연결

- HUD 값은 전부 GameState/PlayerState의 Replicated 값에서 읽는다 — OnRep 콜백이 위젯 갱신을
  호출한다 (conventions.md § 5·§ 6). 위젯이 Tick으로 폴링하지 않는다.
- **예외는 로컬 입력 피드백 둘 — 스태미나 바와 던지기 차징 게이지.** 스태미나는 CMC 틱에서
  시뮬레이션되고 변경 이벤트가 없어 (복제되지도 않는다) HUD가 `GetStamina()`를 매 프레임
  읽는다. 차징 게이지는 버튼을 누른 시간의 함수라 차징 중 매 프레임 `GetThrowChargeAlpha()`
  (로컬 예측 — [input.md](input.md) 던지기 차징)를 읽는다. 둘 다 소유 클라의 로컬 값이라
  네트워크 비용이 없다. 복제 값에는 이 예외를 넓히지 않는다.
  **셋째 — 말하는 사람 목록** (2026-10-11, [voice.md](voice.md)): HUD `NativeTick` 이 `PlayerArray`(≤4)를 훑어 `ACL9PlayerState::IsVoiceTalking()` 을 묻는다. 로컬 마이크 상태·
  원격의 말하는 중 값(`BP_ReadVoiceTalking` 을 PlayerState 가 0.1초마다 읽는다)에는 변경 이벤트가 없고 복제 값도 아니라서 폴링이다 — 대신 **집합(사람 + 슬롯 색)이 지난 틱과 같으면 위젯을 건드리지 않고**, 달라질 때만 `TalkingBox` 를 다시 만든다.
  복제 값에 이 예외를 넓히는 것이 아니다.
- **상호작용 프롬프트는 이벤트다** — 캐릭터가 포커스 틱에서 `GetFocusPrompt()` 문구를 계산하고
  **문구가 바뀔 때만** `OnFocusPromptChanged`를 쏜다. HUD는 그 이벤트에서 문구를 읽어 표시하고,
  비면 숨긴다. 액터 변경이 아니라 문구 변경 기준인 이유: 수레처럼 같은 액터라도 상태에 따라
  문구가 바뀐다 ("[E] 수레 밀기" → "[E] 수레 놓기"). 문구 자체(줍기·놓기·GetPrompt 위임)는
  캐릭터가 Server_PickPlace의 분기 순서대로 조립한다 — HUD는 문구를 만들지 않는다. 문구의 `[E]`는 박힌 글자가
  아니라 `{0}` 서식이고, 캐릭터가 캐시한 현재 바인딩 키로 출구에서 한 번 채운다 ([input.md](input.md) 키 표기).
- **주문 선택 화면은 HUD가 상태에서 파생해 연다** — `GameState.OnOrdersChanged` → `OfferedOrders`가 있으면
  `OpenWidget(OrderSelect)`, 비면 `CloseWidget`. 관전 화면을 PlayerController의 `bSpectating` OnRep이 여는
  것과 같은 원칙(열려 있어야 할 Screen = 상태의 함수)이고, HUD는 항상 떠 있어 GameState 델리게이트를
  이미 받고 있어서 거기 둔다. 화면 자신은 열고 닫지 않는다.
  **저절로 여는 것은 제시마다 한 번이다** (2026-10-05 start-lobby L11): HUD가 연 순간 `bOrderScreenAutoOpened`가 서고,
  `HandleOrdersChanged`에서 **제시 자체가 바뀌었을 때만**(날 + `OfferedOrders`가 마지막으로 본 것과 다를 때, 배선 시점에 한 번
  적어 둔다) 풀린다. `OnOrdersChanged`는 받은 주문만 바뀌어도(납품으로 주문이 채워짐) 오므로 그때마다 풀면 닫아 둔 화면이
  다시 뜬다(검토 UI-1). 사람이 M으로 닫으면 닫힌 채라 로비를 걸어 다닐 수 있다 — 예전에는 매 틱
  다시 열어서, 확정할 수 없는 참가자까지 호스트가 고를 때까지 커서에 묶였다. 호스트가 다시 열 길은 출발 레버다(제시가 남아
  있으면 레버가 `Client_OpenScreen(OrderSelect)`). 다른 Screen에 양보하는 동안은 아직 연 것이 아니라 표시가 서지 않는다.
- **게임 오버 화면도 HUD가 상태에서 파생한다** (2026-10-05 start-lobby L8) — `DeriveRunOverScreen`이 틱마다(주문 파생보다 먼저)
  `GameState->IsRunOver()`를 보고, 참이면 열린 화면이 `GameOver`·`Settings`·`DebugPanel`·`Pause`가 아닐 때
  `OpenRootScreen(GameOver)`를 부른다. 관전 화면이 밀어내도, 판 종료 복제가 늦게 와도 다음 틱에 다시 선다 — RPC도
  컨트롤러 훅도 없다. 판이 끝난 동안 `DeriveOrderScreen`은 바로 돌아간다. 열기에 실패하면(표에 행이 없음 등) HUD 프로퍼티
  `RunOverRetryTime`(1초)마다만 다시 시도하고 HUD 경고는 한 번만 남긴다 — 매 틱 시도하면 매니저 경고가 프레임마다 찍힌다. 판 종료 값은 바뀌면 레벨을 떠나는 길밖에 없어서
  틱 폴링이 이벤트보다 단순하다(GameState에 델리게이트가 없다).
- **HUD는 배선될 때까지 숨는다** (2026-10-05 start-lobby L10) — `NativeConstruct`가 디자이너 가시성을 `WiredVisibility`에
  기억하고 접은 뒤, `TryWire`가 성공하는 순간(PlayerState·GameState가 둘 다 CL9 것으로 도착) 되돌린다. 시작 레벨은 메뉴
  GameMode라 CL9 GameState·PlayerState가 없어 배선이 끝내 안 되므로 HUD가 타이틀 뒤에 비치지 않는다. 늦게 붙는 클라이언트는
  복제가 도착할 때 HUD가 나타난다.
- **스캔은 로컬 표현이고 Popup 레이어의 첫 사용자다** — `ACL9PlayerController::RequestScan()`(캐릭터가 IA_Scan 탭에 부른다 — input.md)이
  `OpenWidget(Scan)`을 부르면 UI 표의 Popup 행 `Scan`(`WBP_Scan`, `UCL9ScanWidget`)이 하나 열린다. 그 위젯이
  열리는 순간 화면에 보이는 액터 중 `ScanKinds`(`ECL9ActorKind` 목록, 기본 아이템만)에 든 종류를 모으고, 파동 링을 키우며 자기 `LabelLayer`에 `WBP_ScanLabel`을 개별로
  붙인다(개수 제한 없음). 파동과 라벨이 다 끝나면 `CloseSelf()`로 닫히고 라벨도 함께 사라진다. 우클릭을
  다시 누르면 열린 것을 닫고 새로 연다. 서버·RPC 없음, 판정 없음. 스캔 라벨과 포커스 라벨은
  `UCL9ScanLabelWidget::MakeNameText/MakePriceText` 한 곳의 문구를 쓴다. Popup은 입력 모드를 바꾸지 않는다.
- **아이템 라벨의 위치는 표시 문제라 매 프레임 재투영한다** — 캐릭터 `GetFocusedActor()`(아웃라인과 같은
  대상)가 아이템이면 `ProjectWorldLocationToWidgetPosition`으로 머리 위 좌표를 구해 HUD 캔버스 슬롯을
  옮긴다. 라벨 **내용**은 대상이 바뀔 때만 채운다(아이템 표 `DisplayName`·`Weight`·개체의 `GetValue()`·
  `CarrySlot`·`bFragile`). 값 폴링이 아니라 좌표 추적이므로 위의 폴링 예외와는 별개다. 액터에
  WidgetComponent를 붙이지 않는다 — 위젯은 WBP_HUD 하나에 있다.
- **조리 게이지도 같은 방식이다** — HUD가 매 프레임 `ACL9Kitchen`을 훑어 익히는 중인 조리대마다
  `WBP_CookGauge` 하나를 자기 `CookLayer`에 붙이고 조리대 꼭대기로 투영한다(스캔 라벨과 같은 꼴).
  끝난 조리대의 것은 뗀다. 포커스와 무관하고, 진행도는 재료의 복제값이라 클라도 같은 바를 본다.
- 서버가 화면 전환을 지시할 때는 Multicast RPC → 로컬에서 `OpenWidget` 호출
  (예: 밤 종료 → 전원 새벽 화면). 단 "지금 열려 있어야 할 Screen"은 **GameState 페이즈에서
  파생 가능**해야 한다 — Multicast를 놓친 중간 참가자가 페이즈에서 파생한 이름과
  `GetOpenScreen()`을 대조해 현재 화면을 복원한다 (session.md 중간 참가).
- 갱신 경로는 "위젯이 열려 있으면 갱신"이다 — 열려 있지 않은 화면에 값이 먼저 도착하는
  경우는 그 화면이 **열릴 때 현재 상태로 초기화**하는 것으로 흡수한다 (열림 시점 전체 갱신).

## 네트워크 고려사항

- UI는 **로컬 전용**이다 — 서버(호스트의 판정 코드)에서 위젯을 만들지 않는다.
- 판정 결과의 표시는 Replicated 값 OnRep 또는 Multicast RPC로만 흘러온다.

## 참고

- 각 화면의 내용: [ui.md](ui.md)
- 구조 결정 경위 (WidgetSwitcher와의 비교): [../review/ui-structure-2026-09-05.md](../review/ui-structure-2026-09-05.md)
- 트럭 패널: [shop.md](shop.md)
- 사운드 호출: conventions.md § 9
