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
| `IMC_Spectate` | 사망 후 관전 | 시점·관전 대상 전환·(밤 종료 대기) |

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
| `IA_PickPlace` | Pressed | 줍기 (빈손) / 놓기 (든 상태) | Server RPC |
| `IA_Throw` | Pressed(Started/Completed) | 던지기 차징 — 누른 시간 0~`ThrowChargeTime`(0.85s) → `ThrowMin~MaxSpeed`. **Hold 트리거 금지** — 임계 전에 떼면 Canceled가 떠서 짧은 던지기가 먹힌다 | Server RPC (아래) |
| `IA_Swap` | Pressed | 손↔품 교체 — `SwapTime` 1.2s 잠금 | Server RPC |
| `IA_Flashlight` | Pressed | **들고 있는 손전등** 켜기/끄기 — 손전등은 아이템이다 ([light.md](light.md) · [../plan/data/item/flashlight.md](../plan/data/item/flashlight.md)) | Server RPC (아이템 상태 Replicated) |
| `IA_Interact` | Pressed | 컨텍스트 상호작용 — 반납·트럭 패널·창구 | Server RPC |
| `IA_Skip` | Pressed | 시퀀스 로컬 스킵 (`bSkippable`) | 로컬 |
| `IA_Pause` | Pressed | 일시정지 화면 (멀티에선 게임 안 멈춤) | 로컬 |

**키 배치 초안** (전부 리바인딩 가능 — 미결, 플레이해보고 확정):
WASD 이동 · 마우스 시점 · Shift 달리기 · Space 점프 · **E 상호작용/줍기·놓기** ·
**마우스좌 던지기 차징** · R 품 교체 · F 손전등 · **M 일시정지** · Esc 화면 닫기
(2026-09-05: 일시정지 열기를 Esc→M으로 변경 — 닫기는 M·Esc 둘 다, Esc 닫기는 고정 키).

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
- **실행**: `IA_Interact`/`IA_PickPlace` → Server RPC → 호스트가 같은 검사를 다시 하고
  실행 (클라 감지 결과를 신뢰하지 않는다).
- 항아리 반납·트럭 패널·창구 판매가 전부 이 인터페이스 구현체다 — 상호작용마다 새 입력
  경로를 만들지 않는다.

## 리바인딩

- **Enhanced Input User Settings** (UE5.3+)로 키 리매핑 — 저장·로드 내장.
- 설정 화면은 [settings.md](settings.md). 감도(Look 배율)도 거기서.
- 고정 키: Esc (화면 닫기)는 리바인딩 대상에서 제외.

## 네트워크 정리

| 입력 | 경로 |
|---|---|
| 이동·달리기·점프 | 커스텀 CMC + `FSavedMove` 예측 (1d 스텝) |
| 줍기·놓기·던지기·교체·손전등·상호작용 | **캐릭터(소유 액터)의 Server RPC** → 호스트 판정 → 대상 액터 함수 호출 → Replicated 결과 |
| 시점·스킵·일시정지·UI | 로컬 전용 |

## 참고

- 운반 조작의 규칙·수치: [inventory.md](inventory.md) · [../plan/data/rules.md](../plan/data/rules.md)
- 화면 상태와 IMC 전환: [ui_system.md](ui_system.md)
- 프롬프트 표시: [ui.md](ui.md)
