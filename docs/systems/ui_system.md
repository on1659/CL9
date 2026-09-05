# UI 시스템 (개발)

> UI 코드 구조. **뷰포트에 붙는 위젯은 `WBP_Main` 하나뿐**이고, 하위 UI는 UIManager가
> **UI 표를 읽어 열 때 생성해** 레이어에 붙인다 (2026-09-05 결정 — WidgetSwitcher 방식
> 번복, [../review/ui-structure-2026-09-05.md](../review/ui-structure-2026-09-05.md)).
> UI 추가는 WBP 제작 + 표에 행 추가다 — C++을 고치지 않는다.

## 구조

```
WBP_Main (UCL9MainWidget)        ← 뷰포트에 붙는 유일한 위젯 (로컬 전용)
  ├─ HUD 레이어                   디자이너에 직접 배치 — 항상 표시
  │                               (밤 시계 · 이더 · 짐 슬롯 · 주문 · 상호작용 프롬프트)
  ├─ Layer_Screen (CanvasPanel)   전면 화면 — **배타** (하나 열면 이전 것이 닫힌다)
  └─ Layer_Popup  (CanvasPanel)   중첩 허용 (확인 대화상자·토스트가 생기면 여기)

UCL9UIManager (GameInstanceSubsystem — 싱글턴 접근 Get())
  ├─ UI 표: UCL9UITable (FName → {위젯 클래스, 레이어}) — BP_CL9GameInstance가 주입
  ├─ OpenWidget(Name) / CloseWidget(Name) / IsOpen(Name) / GetOpenScreen()
  └─ 입력 모드 · IMC_UI 전환 — "Screen 레이어가 비었는가" 하나로 판단

UCL9Widget : UUserWidget          ← 모든 CL9 위젯의 베이스 (UIName + CloseSelf)
```

- 전면 화면(로비 · 새벽/정산 · 관전 · 트럭 패널 · 일시정지)은 전부 Screen 레이어 행이다.
- **새 UI는 표에 행으로만 추가한다.** 뷰포트에 직접 붙이는 위젯을 늘리지 않고, `WBP_Main`도
  다시 열지 않는다 — 독립 WBP를 만들고 `DA_UITable`에 행을 넣으면 끝이다.

## 코드 구조

```cpp
// GameInstanceSubsystem — UI는 로컬 전용이라 GameInstance 수명이 정확히 맞는다
class UCL9UIManager : public UGameInstanceSubsystem {
public:
  static UCL9UIManager* Get(const UObject* WorldContext);   // 싱글턴 접근

  UCL9Widget* OpenWidget(FName UIName);   // 표에서 클래스·레이어 조회 → 생성 → 부착
  void CloseWidget(FName UIName);
  bool IsOpen(FName UIName) const;
  FName GetOpenScreen() const;            // 열려 있는 Screen 행 이름 (None이면 없음)
};
```

- **Screen 레이어는 배타다**: `OpenWidget`이 Screen 행이면 열려 있던 Screen 위젯을 먼저
  닫는다. "전면 화면은 동시에 하나"라는 기획(ui.md)이 레이어 정책으로 지켜진다.
- 위젯은 **열 때 생성하고 닫으면 파괴한다.** C++에서 열어야 하는 잘 알려진 이름(일시정지 등)은
  `CL9UINames` 상수로 둔다 — 문자열 오타를 컴파일 타임에 잡지는 못하므로 상수만 쓴다.
- `WBP_Main`은 로컬 PlayerController의 BeginPlay에서 한 번 생성해 `AddToViewport()` 한 번만
  부른다. UI 표·메인 위젯 클래스는 에셋이 가진다 — C++에 경로가 없다 (conventions § 5).
- 입력 모드(GameOnly ↔ GameAndUI + 커서)와 `IMC_UI` 부착·해제는 Open/Close 안 한 곳에서
  처리한다 — IMC 상태가 화면 상태와 어긋나는 버그를 원천 차단 ([input.md](input.md)).

## 데이터 연결

- HUD 값은 전부 GameState/PlayerState의 Replicated 값에서 읽는다 — OnRep 콜백이 위젯 갱신을
  호출한다 (conventions.md § 5·§ 6). 위젯이 Tick으로 폴링하지 않는다.
- **예외는 로컬 입력 피드백 둘 — 스태미나 바와 던지기 차징 게이지.** 스태미나는 CMC 틱에서
  시뮬레이션되고 변경 이벤트가 없어 (복제되지도 않는다) HUD가 `GetStamina()`를 매 프레임
  읽는다. 차징 게이지는 버튼을 누른 시간의 함수라 차징 중 매 프레임 `GetThrowChargeAlpha()`
  (로컬 예측 — [input.md](input.md) 던지기 차징)를 읽는다. 둘 다 소유 클라의 로컬 값이라
  네트워크 비용이 없다. 복제 값에는 이 예외를 넓히지 않는다.
- **상호작용 프롬프트는 이벤트다** — 캐릭터가 포커스 틱에서 `GetFocusPrompt()` 문구를 계산하고
  **문구가 바뀔 때만** `OnFocusPromptChanged`를 쏜다. HUD는 그 이벤트에서 문구를 읽어 표시하고,
  비면 숨긴다. 액터 변경이 아니라 문구 변경 기준인 이유: 수레처럼 같은 액터라도 상태에 따라
  문구가 바뀐다 ("[E] 수레 끌기" → "[E] 수레 놓기"). 문구 자체(줍기·놓기·GetPrompt 위임)는
  캐릭터가 Server_PickPlace의 분기 순서대로 조립한다 — HUD는 문구를 만들지 않는다.
- **아이템 라벨의 위치는 표시 문제라 매 프레임 재투영한다** — 캐릭터 `GetFocusedActor()`(아웃라인과 같은
  대상)가 아이템이면 `ProjectWorldLocationToWidgetPosition`으로 머리 위 좌표를 구해 HUD 캔버스 슬롯을
  옮긴다. 라벨 **내용**은 대상이 바뀔 때만 채운다(아이템 표 `DisplayName`·`ValueMin/Max`·`Weight`·
  `CarrySlot`·`bFragile`). 값 폴링이 아니라 좌표 추적이므로 위의 폴링 예외와는 별개다. 액터에
  WidgetComponent를 붙이지 않는다 — 위젯은 WBP_HUD 하나에 있다.
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
