# UI 구조 비교 — WidgetSwitcher(docs) vs UIManager(NPCSimulation) (2026-09-05)

Status: **결정 완료 — B안(UIManager) 채택** (2026-09-05, 사용자 확정 —
[../decisions.md](../decisions.md) 기록, [../systems/ui_system.md](../systems/ui_system.md)
반영). 아래 A안 권장 논지는 결정 당시의 비교 기록으로 남긴다. 결정의 결정타는 B의
"UI 추가 작업이 가볍다"(표 행 추가·C++ 무접촉·WBP_Main 무접촉)였다.

참조한 것:
- CL9 기존 설계: [../systems/ui_system.md](../systems/ui_system.md) · [../systems/ui.md](../systems/ui.md)
- 이전 프로젝트: `D:\Work\unreal\NPCSimulation-master` — `Source/NPCSimulation/Manager/UIManager.h(.cpp)`,
  `UI/WarpMainWidget.h(.cpp)`, `UI/WarpWidget.h`, `Data/WarpDefine.h`, `Data/WarpDataAsset.h`

## 공통점 — 이건 어느 쪽이든 같다

**뷰포트에 붙는 위젯은 메인 위젯 하나뿐이고, 나머지는 전부 그 하위에 단다.**
NPCSimulation도 `AddToViewport()`는 `WarpMainWidget` 생성 한 곳에서만 부른다. 갈리는 것은
"그 메인 위젯 안에서 하위 화면을 어떻게 관리하는가"다.

---

## A안 — CL9 docs 방식: WidgetSwitcher + 화면 enum

```
WBP_Main (UCL9MainWidget)
  ├─ HUD 레이어          항상 표시 (짐 슬롯·프롬프트·차징 …)
  └─ WidgetSwitcher      전면 화면 — 동시에 정확히 하나
      ├─ 로비 / 새벽 / 관전 / 트럭 패널 / 일시정지
```

```cpp
enum class ECL9Screen : uint8 { None, Lobby, Dawn, Spectate, TruckPanel, Pause };
void UCL9MainWidget::ShowScreen(ECL9Screen Screen);  // 스위처 인덱스 + 입력 모드 + IMC_UI 전환
```

- 화면 위젯들은 **WBP_Main 디자이너에 미리 배치**한다. 동적 생성이 없다.
- "지금 어떤 화면인가"가 enum 값 하나다.
- 새 화면 추가 절차: enum에 값 추가(C++ 빌드) → WBP_Main 디자이너에 자식 위젯 추가 →
  `ShowScreen`의 입력 모드 분기 확인. 끝.

## B안 — NPCSimulation 방식: UIManager + 레이어 + UI 데이터 표

```
WarpMainWidget (뷰포트 유일)
  ├─ Layer_0 (Default)   ← CanvasPanel — AddWidget이 ZOrder 계산해 부착
  ├─ Layer_1 (Popup)
  └─ Layer_2 (NoHidden)

UIManager (싱글턴, AddToRoot)
  ├─ UIData 에셋: TMap<EWarpUIName, {WidgetClass, Layer}>   ← UI를 표로 등록
  ├─ OpenWidget(UIName)  → 표에서 클래스 찾아 생성 → MainWidget.AddWidget(레이어·ZOrder)
  └─ 열린 위젯 추적: TMap<EWarpUIName, UUserWidget*>

WarpWidget (공통 베이스) — BindWidget()/UnBindWidget() 훅 + UIName
```

- UI가 **데이터**다: enum 이름 + 데이터에셋 행(클래스·레이어)으로 등록하고, 이름으로 연다.
- 화면을 **열 때 생성하고 닫으면 파괴**한다. 같은 레이어에 여러 장 중첩이 된다(ZOrder 자동).
- 새 화면 추가 절차: enum에 값 추가(C++ 빌드) → WBP 제작 → UIData 에셋에 행 추가.

**그대로 이식은 안 되고 수선이 필요한 부분** (CL9 규약 충돌):

| NPCSim 코드 | CL9 규약과의 충돌 | 수선 |
|---|---|---|
| `#define UI_DATA_PATH TEXT(".../UIData.UIData")` | C++ 에셋 경로 하드코딩 금지 (conventions § 5) | PlayerController/BP 프로퍼티로 주입 |
| `UUIManager::Get()` 싱글턴 + `AddToRoot()` | 수명 관리 주체 없음 — 멀티 세션·레벨 전환에 취약 | GameInstance Subsystem 또는 PC 소유로 |
| UI는 어디서나 `Get()`으로 접근 | UI는 로컬 전용 (ui_system.md) — 서버 코드가 접근 가능해지는 문을 열어둠 | 로컬 PC 경유로 한정 |

또한 B안 채택은 ui_system.md의 "WidgetSwitcher, 위젯별 ZOrder 관리 금지" 결정의 번복이라
[../decisions.md](../decisions.md)에 기록해야 한다. A안은 기존 결정 유지라 기록이 필요 없다.

---

## 실제로 달라지는 것

| 관점 | A — WidgetSwitcher | B — UIManager |
|---|---|---|
| 화면 열기 | `ShowScreen(ECL9Screen::Dawn)` | `UIManager->OpenWidget(EUIName::Dawn)` |
| 동시 표시 | 전면 화면 정확히 1장 (HUD는 별도 상시) | 레이어별 여러 장 중첩 가능 (팝업 위 팝업, 토스트) |
| 현재 상태 표현 | enum 값 하나 | "열린 위젯 집합" — 목록으로 관리 |
| 중간 참가 화면 복원 (session.md) | GameState 페이즈 → enum 하나 파생하면 끝 | 무엇이 열려 있어야 하는지 목록을 재구성해야 함 |
| 입력 모드·IMC_UI 전환 (input.md) | `ShowScreen` 한 곳에서 | 열림/닫힘 이벤트에 물리고 "마지막 UI가 닫혔나" 판정 필요 |
| 위젯 생성 | WBP_Main 로드 시 전부 생성 (상시 메모리) | 열 때 생성, 닫으면 파괴 |
| 새 화면 추가 | enum + 디자이너 배치 | enum + WBP + 데이터에셋 행 |
| 새로 짓는 코드량 | 위젯 클래스 1개 + enum | 매니저 + 베이스 위젯 + 데이터에셋 + 메인 위젯 (NPCSim 실측 약 380줄) + 규약 수선 |

두 방식의 화면 수 5개 기준 체감 차이: 생성 시점·메모리는 무의미한 수준이고, 실질 차이는
**"중첩·동적 UI가 되느냐"** 와 **"상태가 값 하나냐 목록이냐"** 두 가지다.

## 언제 B가 맞아지는가

- 화면 종류가 수십 개로 늘어날 때 (매 화면마다 스위처 자식을 늘리는 게 부담이 될 때)
- 팝업이 **중첩**되어야 할 때 — 확인 대화상자 위 보상 팝업 위 토스트 같은 라이브 게임 UI
- UI 추가를 코드 없이 데이터로 하고 싶을 때

CL9의 현재 기획은 반대편에 있다: 전면 화면 5개 고정·배타적, HUD 상시 1장, 부활 UI 없음,
능력 바 없음 (ui.md). 일시정지 → 설정 이동도 화면 전환(스위처)으로 표현된다.

## 절충안 C — 구조는 A, 베이스 클래스만 NPCSim에서

A 구조 그대로 가되, `WarpWidget`의 아이디어만 가져온다: 화면 위젯 공통 베이스
`UCL9ScreenWidget`에 `OnScreenShown()`/`OnScreenHidden()` 훅을 두고 `ShowScreen`이 불러준다.
각 화면이 진입/이탈 초기화를 자기 안에 갖게 된다.

단, 지금 만들 화면들(플레이스홀더 수준)이 이 훅을 당장 쓰지는 않는다 — 단순함 우선 원칙대로
**필요해지는 시점에 추가해도 늦지 않다** (스위처 구조에서는 나중에 끼워 넣기 쉽다).

## 권장

**A안.** 근거:

1. CL9의 화면은 5개 고정·배타적이라 B의 장점(중첩·동적 등록)에 수요가 없다.
2. 중간 참가 화면 복원이 "enum 값 하나 파생"으로 끝난다 — B는 이 요구사항과 결이 나쁘다.
3. B는 이식 자체에 규약 수선 3건 + 결정 번복 기록이 붙는다.
4. 나중에 토스트·중첩 팝업이 정말 필요해지면, WBP_Main에 오버레이 레이어 하나를 **추가**하는
   식으로 점진 확장이 된다 — 지금 B로 시작해야만 가능한 미래가 없다.
