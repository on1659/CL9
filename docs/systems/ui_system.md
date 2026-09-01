# UI 시스템 (개발)

> UI 코드 구조. **뷰포트에 붙는 위젯은 `WBP_Main` 하나뿐이고, 전환은 WidgetSwitcher다**
> (plan/build-order.md 4h). 위젯마다 `AddToViewport(ZOrder)`를 부르는 구조를 만들지 않는다 —
> z 순서·정리·전환이 전부 손으로 관리하는 일이 된다.

## 구조

```
WBP_Main                ← 뷰포트에 붙는 유일한 위젯 (로컬 전용)
  ├─ HUD 레이어          (밤 시계 · 이더 · 짐 슬롯 · 주문 · 상호작용 프롬프트)
  └─ WidgetSwitcher      (전면 화면 — 동시에 하나만)
      ├─ 로비 화면        (주문서 선택 · 목적지 선택 · 캐릭터 선택)
      ├─ 새벽/정산 화면
      ├─ 관전 화면
      ├─ 트럭 패널
      └─ 일시정지 메뉴
```

- **새 UI는 `WBP_Main` 하위로만 추가한다.** 뷰포트에 직접 붙이는 위젯을 늘리지 않는다.
- HUD 레이어는 항상 떠 있고, WidgetSwitcher가 전면 화면을 배타적으로 전환한다 —
  별도의 Z-Order 관리·요청 큐·우선순위 시스템이 필요 없다.

## 코드 구조

```cpp
// ACL9HUD (또는 PlayerController) — 로컬 전용
class UCL9MainWidget : public UUserWidget {
public:
  // 전면 화면 전환 — WidgetSwitcher 인덱스 하나로 상태가 정해진다
  UFUNCTION(BlueprintCallable)
  void ShowScreen(ECL9Screen Screen);   // Lobby / Dawn / Spectate / TruckPanel / Pause / None

  // HUD 갱신은 GameState/PlayerState의 OnRep에 물린다
};

UENUM()
enum class ECL9Screen : uint8 { None, Lobby, Dawn, Spectate, TruckPanel, Pause };
```

- `WBP_Main`은 게임 시작 시 한 번 생성해 `AddToViewport()` 한 번만 부른다.
- 화면 상태는 enum 하나 — "지금 어떤 화면인가"를 큐가 아니라 값 하나가 말한다.
- 입력 모드(UIOnly/GameOnly)는 `ShowScreen` 안에서 화면에 따라 전환한다.

## 데이터 연결

- HUD 값은 전부 GameState/PlayerState의 Replicated 값에서 읽는다 — OnRep 콜백이 위젯 갱신을
  호출한다 (conventions.md § 5·§ 6). 위젯이 Tick으로 폴링하지 않는다.
- 서버가 화면 전환을 지시할 때는 Multicast RPC → 로컬에서 `ShowScreen` 호출
  (예: 밤 종료 → 전원 새벽 화면). 단 화면 상태는 **GameState 페이즈에서 파생 가능**해야
  한다 — Multicast를 놓친 중간 참가자가 현재 화면을 복원할 수 있게 (session.md 중간 참가).
- 하위 위젯 클래스 참조는 `WBP_Main`의 프로퍼티로 에디터에서 연결한다 — C++에 위젯 경로를
  하드코딩하지 않는다 (conventions.md § 5).

## 네트워크 고려사항

- UI는 **로컬 전용**이다 — 서버(호스트의 판정 코드)에서 위젯을 만들지 않는다.
- 판정 결과의 표시는 Replicated 값 OnRep 또는 Multicast RPC로만 흘러온다.

## 참고

- 각 화면의 내용: [ui.md](ui.md)
- 트럭 패널: [shop.md](shop.md)
- 사운드 호출: conventions.md § 9
