# 입력 시스템 (Enhanced Input)

> 조작의 "무엇"은 plan이 이미 정의했다 (줍기·던지기 차징·품 교체·손전등·점프 —
> [../plan/data/rules.md](../plan/data/rules.md)). 이 문서는 그것을 **Enhanced Input
> (IMC + Input Action)**으로 어떻게 구현하는지와, 상호작용 공통 틀을 정의한다.

## 원칙

- **Enhanced Input 전용** (build-order.md "입력: Enhanced Input"). 레거시 입력 바인딩 금지.
- **입력은 로컬, 동작은 Server RPC** (conventions.md § 2). 이동만 예외 — CMC 예측 (A4 결정).
- 조작 세트는 plan의 것이 정본이다 — E 채집·회피·공격·능력 바 같은 폐기된 조작을 되살리지 않는다.

## IMC 구성

컨텍스트 3개 — 상태에 따라 붙였다 뗀다. 우선순위 높은 것이 겹치는 키를 가로챈다.

| IMC | 언제 활성 | 담는 것 |
|---|---|---|
| `IMC_Gameplay` | 폰 빙의 중 (기본) | 이동·시점·달리기·점프·줍기/놓기·던지기·품 교체·손전등·상호작용 |
| `IMC_UI` | 전면 화면 열림 (`WBP_Main.ShowScreen ≠ None`) | 메뉴 탐색·닫기 — Gameplay보다 우선 |
| `IMC_Spectate` | 사망 후 관전 (`ACL9PlayerController.SpectateMappingContext`, 우선순위 5 — `bSpectating` OnRep이 관전 화면과 함께 붙였다 뗀다. DefaultMappingContexts에 넣지 않는다) | 관전 대상 전환·(밤 종료 대기). 시점은 동료 1인칭 승계라 별도 Look 없음 |

- 전환은 `ShowScreen`/관전 진입과 한 곳에서 묶어서 처리한다 ([ui_system.md](ui_system.md)) —
  IMC 부착 상태가 화면 상태와 어긋나는 버그를 원천 차단.
- 시퀀스 재생 중 `bLockLocalInput`이면 `IMC_Gameplay`를 잠시 뗀다 ([sequence.md](sequence.md)).

## Input Action 목록

| IA | 타입 | 동작 | 판정 경로 |
|---|---|---|---|
| `IA_Move` | Axis2D | 이동 | CMC 예측 (Server RPC 아님) |
| `IA_Look` | Axis2D | 시점 | 로컬 |
| `IA_Run` | Hold | 달리기 — 스태미나 소모, `FSavedMove`에 실림 | CMC 예측 |
| `IA_Jump` | Pressed | 점프 — 낮은 장애물 통과용. 거리·감각 판정은 평면 기준 (rules.md) | CMC |
| `IA_PickPlace` | Pressed | 줍기 (빈손) / 놓기 (든 상태) / 컨텍스트 상호작용(반납·트럭 패널·창구 — 별도 `IA_Interact`는 없다, 아래 공통 틀) | Server RPC |
| `IA_Use` | Started/Completed | **좌클 = 든 아이템 사용** (2026-09-12) — **탭**(0.4초 안에 뗌)은 행의 `UseAction`(`ToggleLight` 손전등 켜고 끄기 / `Swing` 정면 휘두르기 → 몬스터 `OnStrike(HitDamage, HitStun)` / `None` 무동작), **길게 누르기**(0.4초 문턱, 로컬 타이머)는 `ItemType == Dish`면 먹기. 닭다리처럼 먹을 수도 휘두를 수도 있는 것을 한 키로 가르는 규칙(사용자: "길게 누르면 먹기, 짧게 누르면 휘두르기"). 빈손은 무동작 — 옛 빈손 밀치기는 프라이팬 행(`Swing`+`Push`)이 이어받았다 | Server RPC (`Server_Use(bHold)` → `HostEat`/`ToggleLight`/`HostSwing`) |
| `IA_Throw` | Pressed(Started/Completed) | 던지기 차징(**G**, 2026-09-12 좌클에서 이동) — 누른 시간 0~`ThrowChargeTime`(0.85s) → `ThrowMin~MaxSpeed`. **Hold 트리거 금지** — 임계 전에 떼면 Canceled가 떠서 짧은 던지기가 먹힌다 | Server RPC (아래) |
| `IA_Swap` | Pressed | 손↔품 교체 — `SwapTime` 1.2s 잠금 | Server RPC |
| `IA_Flashlight` | Pressed | **쓴 헤드램프** 켜기/끄기 — 손전등은 `IA_Use`(좌클)가 켠다 (2026-09-12; [light.md](light.md) · [../plan/data/item/headlamp.md](../plan/data/item/headlamp.md)) | Server RPC (아이템 상태 Replicated) |
| `IA_Skip` | Pressed | 시퀀스 로컬 스킵 (`bSkippable`) — **에셋 미생성** (시퀀스 시스템과 함께) | 로컬 |
| `IA_Pause` | Pressed | 메뉴 화면 (게임은 멈추지 않는다 — 온라인이라 멈춤 자체가 없다) | 로컬 |
| `IA_SpectateNext` | Pressed | 관전 대상 전환 — `ACL9PlayerController.SpectateNextAction` → `SpectateNext()` (관전 화면 버튼도 같은 함수) | Server RPC (호스트가 다음 생존자로 뷰 타깃 교체) |
| `IA_Scan` | Pressed | 우클릭 스캔 — 시야 안 아이템에 이름·가격 라벨을 잠깐 띄우는 표현 (`ACL9PlayerController.ScanAction` → `OnScanRequested` → HUD, [ui.md](ui.md) 스캔) | 로컬 (판정 없음) |

**기본 키 배치** (시점·게임패드 빼고 전부 리바인딩 가능 — 아래 리바인딩 절):
WASD 이동 · 마우스 시점 · Shift 달리기 · Space 점프 · **E 상호작용/줍기·놓기** ·
**마우스좌 사용**(탭: 손전등 켜기·휘두르기 — 행이 정함 / 길게: 먹기) · **마우스우 스캔**(시야 안 아이템 라벨) · R 품 교체 · F 헤드램프 ·
**G 던지기 차징** · **M 메뉴 열기·닫기** · **Tab 관전 대상 전환**(관전 중에만 — IMC_Spectate)
(2026-09-05: 이 화면을 Esc→M으로 변경. 같은 날 사용자 결정으로 **Esc는 게임에서 쓰지 않는다** —
PIE 정지 키와 겹쳐 에디터가 꺼진다. IMC_Gameplay·IMC_UI 모두 M→IA_Pause 한 행, Esc 행 없음).

## 던지기 차징 — 시간 판정은 호스트가

```
클라: IA_Throw Started → Server_ThrowChargeStart()   // 호스트가 시작 시각 기록
클라: 차징 게이지 표시 (로컬 예측 — ui.md)
클라: IA_Throw Completed → Server_ThrowRelease()
호스트: 경과 시간 = 자기 시계 기준 → 속도 계산 → 던짐 판정
```

- 차징량을 클라가 보내지 않는다 — 클라 값(치팅·시계 조작)을 판정에 쓰지 않는다.
- 로컬 게이지와 실제 결과의 오차는 지연 한 틱 수준 — 허용.
- `SwapTime` 잠금 중 던지기·줍기·교체 입력은 **호스트 판정에서 거부**한다
  (PlayerState.SwapLockUntil). 입력 자체를 클라에서 막는 건 표시용 보조일 뿐, 관문이 아니다.

## 상호작용 공통 틀

줍기·반납·트럭 패널·창구가 전부 "상호작용"이다. 감지·프롬프트·실행을 한 틀로:

```cpp
// 상호작용 가능한 것들이 구현하는 인터페이스 — RPC가 아니라 일반 함수다.
// (아이템·항아리는 비소유 액터라 클라가 직접 Server RPC를 부를 수 없다 — actor-base.md)
class ICL9Interactable {
  FText GetPrompt(const ACL9Character* Who);   // "[E] 반납" — HUD 프롬프트용
  bool CanInteract(const ACL9Character* Who);  // 잠금·거리·상태 검사
  void Interact(ACL9Character* Who);           // 호스트에서만 호출된다
};

// 실행 경로: 클라 입력 → ACL9Character::Server_Interact(대상) [소유 액터의 RPC]
//            → 호스트가 재검사 후 대상->Interact(Who) 직접 호출
```

- **감지**: 로컬 틱에서 카메라 전방 트레이스 + `PickRadius`(120cm) 구형 검사.
  후보가 여럿이면 우선순위: **들 수 있는 아이템 > 상호작용 액터**, 같은 급이면 조준점에
  가까운 것.
- **프롬프트**: 감지 결과를 HUD가 표시 ([ui.md](ui.md) 하단 중앙). 몬스터·위협 정보는
  프롬프트에 싣지 않는다.
- **실행**: `IA_PickPlace` → Server RPC → 호스트가 같은 검사를 다시 하고
  실행 (클라 감지 결과를 신뢰하지 않는다).
- 항아리 반납·트럭 패널·창구 판매가 전부 이 인터페이스 구현체다 — 상호작용마다 새 입력
  경로를 만들지 않는다.

## 리바인딩

**엔진 Enhanced Input User Settings**(`UEnhancedInputUserSettings`, 5.8 기본 켜짐, 서브클래스 없음)를 그대로 쓴다.
저장·로드는 엔진이 `Saved/SaveGames/EnhancedInputUserSettings.sav`에 한다. 설정 화면·감도는 [settings.md](settings.md),
결정 근거는 [../decisions.md](../decisions.md) 2026-09-12 "옵션 시스템·키 리바인딩". 엔진 소스로 확인한 규칙:

### 에셋 — 무엇이 리바인딩 대상인가

| 대상 | 설정 위치 | 매핑 이름(`Name`) | 표시 이름(`DisplayName`) |
|---|---|---|---|
| `IA_Jump` `IA_Run` `IA_PickPlace` `IA_Throw` `IA_Use` `IA_Swap` `IA_Flashlight` `IA_Scan` `IA_Pause` `IA_SpectateNext` | IA 에셋의 `PlayerMappableKeySettings` (IMC 행은 `InheritSettingsFromAction`) | IA 이름에서 `IA_`를 뗀 것 (`Jump`…) | String Table `Input.<Name>.Name` |
| `IA_Move` | `IMC_Gameplay`의 W/S/A/D 행마다 `OverrideSettings` | `MoveForward` `MoveBack` `MoveLeft` `MoveRight` | `Input.MoveForward.Name` … |
| `IA_Look`, 게임패드 행(`Gamepad_*`) | 액션에 설정 없음 / 행 `IgnoreSettings` | — | 대상 아님 |

- **`Name`은 전부 고유해야 한다** — 비우면 `NAME_None` 행 하나로 합쳐진다(에디터 검증 없음).
- 게임패드 행을 `IgnoreSettings`로 두는 이유: 액션 설정을 상속하면 같은 행에 들어가고, 하드웨어 판별이 항상
  Invalid라 슬롯이 **IMC 행 순서**로 매겨진다 — 게임패드가 Space보다 앞이면 First 슬롯이 게임패드가 되어 화면과
  프롬프트가 게임패드 키를 보여준다.
- `DisplayName`은 반드시 String Table 참조 — 직접 친 텍스트는 `CL9_Gather.ini`가 `Content/Input/*`도 긁어
  GUID 키로 드러내고, 설정 화면이 행을 만들 때 `FTextInspector::GetTableIdAndKey`로 확인해 `LogCL9` Error를 낸다.

### 등록 — `ACL9PlayerController::BeginPlay`

```cpp
// 로컬 PC BeginPlay: IMC 3개를 유저 설정에 등록한다 (멱등 — 이미 등록된 것은 무시)
UserSettings->RegisterInputMappingContexts({IMC_Gameplay, IMC_UI, IMC_Spectate});
```

- **`SetupInputComponent`에서는 못 한다.** `UEnhancedInputUserSettings`는 `SetPlayer → ReceivedPlayer →
  PlayerControllerChanged`에서 만들어지는데 `SetupInputComponent`는 그보다 앞(`InitInputSystem`)이라 그 시점의
  `GetUserSettings()`는 null이고, `AddMappingContext(bNotifyUserSettings=true)`도 같은 이유로 **첫 PC에서 조용히
  아무것도 등록하지 않는다**(맵 트래블 뒤 두 번째 PC부터만 된다 — "됐다 안 됐다" 버그). `BeginPlay`는 호스트·클라
  모두 `SetPlayer` 뒤다.
- `Add/RemoveMappingContext`에 `bNotifyUserSettings`를 **주지 않는다** — Remove에 주면 Screen을 여는 순간
  `IMC_Gameplay` 행이 프로필에서 빠진다.
- `IMC_Spectate`는 관전 전엔 부착되지 않지만 등록은 여기서 미리 한다 — 등록 안 된 IMC의 저장 행은 이름·기본키가
  빈 채로 올라온다(저장 파일에는 매핑 이름·키·슬롯·장치 넷만 남는다).

### 행 나열·수정 규칙 (설정 화면이 따르는 것)

- 행은 `GetPlayerMappingRows()`(TMap, 해시 순서)를 통째로 나열하지 않고, 등록한 IMC(**Gameplay → Spectate**)의
  `GetMappings()` 순서로 매핑 이름을 뽑아 프로필에서 찾는다 — 표시 순서는 IMC 에셋이 정하고 잔여 행이 새지 않는다.
  `IMC_UI`의 Pause 행은 `IMC_Gameplay`에 같은 이름이 있어 한 행이다(재빌드가 매핑 이름으로 모든 IMC에 적용한다).
- 표시·수정하는 매핑은 행의 `FPlayerKeyMapping` 중 **키보드/마우스 키인 첫 것**(`!IsGamepadKey`) — `Slot::First`
  고정이 아니다.
- **`MapPlayerKey`는 `Slot`·`HardwareDeviceId`를 그 매핑의 값으로 채우고 `bCreateMatchingSlotIfNeeded=false`로
  부른다.** 기본 인자(`Slot=Unspecified`)로 부르면 못 찾고 새 매핑을 **추가**해 옛 키와 새 키가 공존한다 —
  맞바꿈이 중복이 된다. 슬롯 없음은 실패 태그 없이 조용히 끝나므로 사전에 `FindKeyMapping`으로 확인한다.
- 저장은 `SaveSettings()`를 키를 바꾼 직후 바로(동기, 수 KB).

### 캡처 (키 버튼을 누른 뒤 "키를 누르세요…")

- **`IInputProcessor`**(`FSlateApplication::RegisterInputPreProcessor`, 인덱스 0, Lyra 방식) — 위젯·뷰포트 라우팅보다
  먼저 모든 키·마우스 버튼을 받는다. UMG `NativeOnKeyDown`은 안 된다: `SButton`이 LMB·Space·Enter를 먼저 먹고,
  `UUserWidget`은 기본 비포커스이며, 덮개 빈 곳 클릭이 뷰포트로 포커스를 넘겨 캡처가 소리 없이 끝난다.
- 허용: 키보드 키·마우스 버튼(Down 시점). 거부(힌트만, 캡처 유지): 게임패드·마우스 축/휠·Esc·`~`(콘솔)·F11(전체화면
  토글). `IMC_UI` 출신 행(Pause)은 마우스 버튼도 거부 — 메뉴 덮개(SBorder)는 클릭을 Unhandled로 흘려 PlayerInput까지
  내려가므로 Pause=LMB면 메뉴 안 클릭이 화면을 닫는다.
- 취소: **Esc** 또는 **5초 무입력**(사용자 2026-09-12). Esc는 프리프로세서가 소비하므로 PIE 정지와 겹치지 않는다 —
  "Esc는 게임 키가 아니다"(2026-09-05)의 유일한 예외이며 IMC에는 여전히 Esc 행이 없다.
- 캡처 중 모든 키·버튼을 소비한다 — M(IA_Pause)이 화면을 닫지 못한다.
- 프로세서는 앱 전역이다 — 위젯을 `TWeakObjectPtr`로 들고 위젯이 무효면 통과시키며 스스로 해제하고, 위젯 쪽에서도
  취소·`OnClosing`(닫힘 훅, [ui_system.md](ui_system.md))·`NativeDestruct` 셋에서 해제한다. PIE 종료 뒤 남으면 에디터
  키를 먹는다.

### 충돌 — 알리고 맞바꾼다

같은 키를 쓰는 다른 행이 있으면 두 행의 키를 맞바꾸고(`MapPlayerKey` 두 번, 두 번째는
`bDeferOnSettingsChangedBroadcast`) 힌트 `Settings.KeySwapped`("손전등: E · 줍기: F" — 새 키를 받은 동작, 그 키,
내준 동작, 그 동작이 받은 키)를 띄운다. 전 행이 한 그룹이다(IMC별 그룹은 필요해질 때). 여럿이면 첫 행만 맞바꾸고
나머지는 힌트에 적는다. 빈 행은 생기지 않는다.

### 키 표기 — 프롬프트의 `[{0}]`

- 문구는 키를 박지 않고 `{0}` 서식이다(`Prompt.PickUp` "[{0}] 줍기", `Spectate.Next` "다음 동료 ({0})") —
  괄호는 문자열이 정한다. 규칙은 [localization.md](localization.md).
- `Source/CL9/Core/CL9InputText.h` `CL9Input::GetKeyText(FKey)`: 시트에 `Key.<FKey 이름>` 행(`Key.SpaceBar`
  "Space", `Key.LeftMouseButton` "LMB"…)이 있으면 그것, 없으면 엔진 영문 원문(`GetDisplayName(false)`의 소스
  문자열)으로 폴백. 엔진 ko 번역은 "스페이스"·"왼쪽 Shift"·"LMB"로 섞이고 PIE 미리보기는 Engine 타깃을 싣지 않아
  PIE와 패키지가 다르게 보이기 때문이다.
- `CL9Input::GetActionKeyText(PC, IA)`: IA의 매핑 이름으로 프로필 행 → 키보드/마우스 첫 매핑 → `GetKeyText`.
- `ACL9Character`는 `PickPlace` 키 텍스트를 **캐시**하고(`OnSettingsChanged`에서 갱신, `LastFocusPrompt` 비움)
  `GetFocusPrompt` 출구 한 곳에서 `FText::Format`한다 — `ICL9Interactable::GetPrompt`는 원문 패턴을 그대로 돌려준다.
  틱마다 프로필을 조회하면 할당에다 못 찾을 때 Warning 스팸이다. 관전 위젯은 `NativeConstruct`에서 한 번.
- 개발 편의: `cl9.resetkeys`([cheats.md](cheats.md)) — 프롬프트가 현재 키를 찍으니 "왜 `[F] 줍기`지?"가 생긴다.

## 네트워크 정리

| 입력 | 경로 |
|---|---|
| 이동·달리기·점프 | 커스텀 CMC + `FSavedMove` 예측 (1d 스텝) |
| 줍기·놓기·던지기·교체·손전등·상호작용 | **캐릭터(소유 액터)의 Server RPC** → 호스트 판정 → 대상 액터 함수 호출 → Replicated 결과 |
| 시점·스킵·메뉴·UI | 로컬 전용 |

## 참고

- 운반 조작의 규칙·수치: [inventory.md](inventory.md) · [../plan/data/rules.md](../plan/data/rules.md)
- 화면 상태와 IMC 전환: [ui_system.md](ui_system.md)
- 프롬프트 표시: [ui.md](ui.md)
