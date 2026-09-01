# 애니메이션 규칙

> 애니메이션은 표현이다 — 판정에 들어가지 않고, 상태에서 파생된다. 이 문서는 스켈레톤·AnimBP
> 연결 규칙과 상태→애님 흐름을 정의한다.

## 규칙 셋

1. **C++에 애님·메시 경로 0줄** (build-order 원칙 2). 연결은 전부 데이터다 —
   캐릭터는 `UCL9CharacterDef`의 `SkeletalMesh`/`Skeleton`/`AnimBP` 소프트 참조, 몬스터도 동일
   ([../plan/data/character.md](../plan/data/character.md) · [../plan/data/monster/common.md](../plan/data/monster/common.md)).
   `BP_CL9Character`(블루프린트 자식)가 메시·AnimBP를 갖고, C++은 캡슐과 로직만.
2. **애님은 판정에 못 들어간다.** 애님 노티파이로 상태를 바꾸지 않는다 — 발소리 사운드·파티클
   같은 렌더 전용 노티파이만 허용. 판정 타이밍(경직 길이 등)은 값(`HitStunMs`)이 정하고
   애님이 값에 맞춘다, 반대가 아니다.
3. **캡슐이 기준이고 메시가 맞춘다** (39/90). 산 에셋이 커도 캡슐을 키우지 않는다 —
   통로 폭 255cm가 걸려 있다.

## 보유 에셋 (2026-09-02 확인 — 프로젝트 Content에 이미 있음)

| 에셋 | 위치 | 용도 |
|---|---|---|
| UE5 마네킹 (Manny/Quinn) + 애님 (Walk/Jog/Jump/Attack/Death/HitReact) | `Content/Characters/Mannequins/` | 1c 임시 캐릭터 메시 — 계획 그대로 |
| FPP 팔 (ABP_FP + 컨트롤릭) | `Content/FirstPerson/Anims/` | 1인칭 손 뷰모델의 출발점 |
| 닭 SK_Chicken + 애님 8종 | `Content/DreamscapeSeries/DreamscapeFarmlands/Meshes/Chars/Chicken/` | ③ 1호 몬스터 — 커버리지 전수 확인 ([chicken.md](../plan/data/monster/chicken.md) 에셋 절) |
| 돼지·양 SK + 애님 풀셋 | 같은 팩 `Chars/Pig` · `Chars/Sheep` | 예비 (기획에 아직 없는 종) |
| Farmlands 환경 (농장 건물·지형·폴리지) + Horror 조명 깜빡임 | `DreamscapeFarmlands/` · `Variant_Horror/` | 5a 맵·light.md 참고 |

**없는 것: 거위·너구리 메시.** 거위는 닭과 기능이 같고 에셋만 다르므로(2026-09-01 확인),
전용 에셋을 구하기 전까지 `SK_Chicken` 스케일·틴트 변형으로 ③을 돌 수 있다 (초안).

## 스켈레톤 규칙

- **캐릭터: UE5 마네킹 스켈레톤 호환 에셋을 산다 (권장 구매 기준).** build-order 5e —
  임시 메시로 만든 애님이 산 캐릭터와 뼈대가 다르면 다시 만들게 된다. 마네킹 호환이면
  1c 임시 메시(마네킹)의 애님 작업을 그대로 승계한다. 리타깃이 필요한 에셋은 그 비용을
  알고 산다.
- **몬스터: 종별 스켈레톤 1개.** 닭 100마리가 스켈레톤 하나를 공유한다 — `Skeleton` 컬럼이
  가리키고, 같은 종의 변형(에셋 교체)은 같은 스켈레톤을 쓰는 게 기본.

## 상태 → 애님 흐름

애님은 Replicated 상태에서 **파생**된다. RPC로 애님을 직접 트리거하지 않는다 (상태가 정본,
애님은 구독자). 일회성 연출만 Multicast를 쓴다 (사운드 규칙과 동일 — conventions.md § 9).

### 캐릭터 (AnimBP가 읽는 값)

| 값 | 출처 |
|---|---|
| 속도·방향·공중 여부 | CMC (예측 이동 — 자연히 전 머신 동기) |
| 달리기 여부 | CMC 커스텀 플래그 (`FSavedMove`에 실린 것) |
| 손에 든 것 | PlayerState.HandItem (OnRep) — 들기 포즈 전환 |
| 넘어짐·비틀(완충타)·탈진 | PlayerState 상태 (OnRep) |

### 몬스터 (AnimBP가 읽는 값)

| 값 | 출처 |
|---|---|
| `ECL9MonsterState` | Replicated + OnRep ([monster.md](monster.md)) — 상태별 로코모션/포즈 |
| 이동 속도 | 액터 속도 (복제 위치에서 파생 — 클라 보간) |
| 경직·기상 예고 | 상태 전이 (WakeTelegraph는 전 플레이어에게 보인다) |

## 1인칭 팔 (FPP Arms)

시점이 1인칭이고 **자기 화면에는 손(팔)만 보인다** ([character.md](character.md)):

- **팔 전용 메시 + 팔 전용 AnimBP**를 풀바디와 별도로 둔다 (표준 FPS 관행) — 카메라에는
  팔 메시만, 타인에게는 풀바디만 보이게 Owner See / Owner No See로 분리.
- 든 오브젝트는 팔 메시의 손 소켓에 부착 — 줍기·던지기·교체·손전등 토글의 1인칭 애님이
  여기 산다.
- 데이터 연결: `UCL9CharacterDef`에 팔 메시·팔 AnimBP 컬럼 추가 필요 (미결 — 1c에서).

## 몽타주 사용처

전부 렌더 전용 — 길이는 판정 값에 맞춘다:

| 몽타주 | 판정 값 | 비고 |
|---|---|---|
| 죽음 | HP 0 | 기본은 같은 메시의 죽는 모션 — 래그돌 등 다른 연출은 종별 선택 |
| 경직 | `HitStunMs` / `PushStun` | 전용 애니메이션 필수 (plan/data/monster/common.md) |
| 잡힘·넘어짐 | CatchRadius 판정 | 짐 흘림과 함께 |
| 기상 예고 | `WakeTelegraph` 0.8s | 퍼덕임 + 울음 |
| 거위 울음 예비 | `AlertWindup` 0.55s | |

## 구현 시점

- 1c: 임시 메시(마네킹) + 기본 로코모션 — "몸이 보인다"
- ③: 몬스터 상태별 애님 (죽음·경직·예고가 상태 기계와 함께)
- 5d/5e: 산 에셋으로 교체 + 본 애니메이션 — 데이터 컬럼만 갈아끼우면 되는 게 이 규칙의 목적

## 참고

- 데이터 컬럼: [../plan/data/character.md](../plan/data/character.md) ·
  [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 상태 정의: [monster.md](monster.md) · [inventory.md](inventory.md)
- 표현 계층 규칙: conventions.md § 9 (사운드와 같은 원칙)
