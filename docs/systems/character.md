# 캐릭터 시스템 (이동·스태미나 — 커스텀 CMC)

> build-order가 "1d가 제일 어렵다"고 한 바로 그 시스템. `ACL9Character : ACharacter` +
> **커스텀 CMC + `FSavedMove`** (A4 결정). 값의 정본은
> [../plan/data/character.md](../plan/data/character.md) · [../plan/data/rules.md](../plan/data/rules.md).

## 시점 — 1인칭 (2026-09-01 확정)

- 카메라는 눈높이 `EyeHeight` 156cm에 붙는 **1인칭**이다. 손전등 조준·던지기 조준·시선이
  전부 카메라 방향에서 나온다.
- **자기 화면에는 손(팔)만 보인다** (2026-09-02 확정) — 1인칭 팔 전용 메시(FPP arms)에
  든 오브젝트를 **부착**하는 방식이다. 계란·손전등·프라이팬이 전부 손 소켓에 붙는다.
  타인 시점에서는 풀바디 메시 + `CarryHeight` 부착이 그대로다 ([animation.md](animation.md)).
- 3인칭 카메라 충돌 검사(segkind.md의 `bBlocksSight` 세 용도 중 하나)는 관전 시점 등
  3인칭 상황에서만 의미가 있다.
- 관전은 동료 시점 관전 (1인칭 그대로 — 초안).

## 구조

```
ACL9Character : ACharacter                  ← 캡슐 39/90, 입력, 카메라, 네트워크
  └─ UCL9CharacterMovement : UCharacterMovementComponent
       ← 달리기·스태미나·유효 무게 감속이 전부 여기(예측 경로) 산다
BP_CL9Character                             ← 메시·AnimBP만 (C++에 에셋 참조 0줄)
```

**이동에 영향을 주는 상태는 전부 CMC 안에 있어야 한다** (conventions.md § 4 예외) —
PlayerState에 두면 호스트·클라 적용 시점이 갈라져 러버밴딩이 난다. PlayerState에는
UI용 미러만 복제한다.

## 달리기 — FSavedMove 패턴

1d가 어려운 이유: 속도가 달라지는 순간 서버·클라가 같은 식으로 계산하지 않으면 캐릭터가
끌려다닌다. 표준 패턴:

```
1. 입력: IA_Run → CMC의 bWantsToRun 플래그 (입력만 저장, 속도 직접 변경 금지)
2. FSavedMove_CL9: bWantsToRun을 압축 플래그로 저장
   (CompressedFlags — 서버로 가는 이동 패킷에 실린다)
3. UCL9CharacterMovement::UpdateFromCompressedFlags: 서버가 같은 플래그를 복원
4. GetMaxSpeed() override: bWantsToRun && CanRun() ? RunSpeed : WalkSpeed
   → 서버·자율 프록시가 같은 함수로 같은 결과 — 예측이 어긋나지 않는다
```

**스태미나도 CMC 틱 안에서 시뮬레이션한다** — 서버와 클라(자율 프록시)가 같은 입력 플래그로
같은 공식을 돌리므로 결정론적으로 일치한다. 서버 보정이 오면 저장무브 리플레이로 재계산돼
대체로 수렴하지만, **표준 보정 페이로드(`ClientAdjustPosition`)에는 커스텀 상태가 없다** —
서버가 이동을 기각한 경우 스태미나 드리프트가 남을 수 있으니, 관측되면 MoveResponse 컨테이너
확장을 검토한다 (1e). 스태미나를 RPC로 주고받지 않는다.

## 스태미나 — 세 조항 (character.md 계산 전제)

| 조항 | 구현 |
|---|---|
| **탈진 래치** | 스태미나 0 도달 → `bExhausted = true`, 달리기 불가 |
| **재개 히스테리시스** | `StaminaMinResume`(1.0) 이상 회복해야 래치 해제 — 만충까지 잠근다 (2026-09-05 결정) |
| **실이동 조건** | 실제로 이동 중일 때만 소모 — 제자리에서 달리기 키만 누르면 소모 없음 |

- 소모/회복은 시간 기반: 만충→0 `StaminaDrainTime`(6.0초), 0→만충 `StaminaRegenTime`(8.0초)
- 셋 중 하나라도 빠지면 "지친 빈손 지속 속도 506" 산수가 무너진다 — 로드 검사 A1의 전제
- HUD에 바를 그리지 않는다 — 화면 흔들림 진폭으로만 (rules.md). 구현: 소유 클라 로컬 틱이
  셰이크 표(`UCL9ShakeTable`, GameInstance 주입)의 `CL9ShakeNames::Stamina` 행을 읽어,
  `Stamina < StartAt`면 그 행의 셰이크를 재생하고 스케일을 `(StartAt−Stamina)/StartAt`로
  키운다 (표시 전용·복제 무접촉 — 셰이크는 규칙 컬럼이 아니라 표의 행이다,
  [../plan/data/rules.md](../plan/data/rules.md) 스태미나 표시).

## 유효 무게 감속

```
GetMaxSpeed() = 기본 속도 × 감속 f(유효 무게)     // 식·값 미결 (plan/data/item/common.md)
유효 무게 = Σ(든 것 Weight) × CarryMult
```

- 든 것의 변경(줍기/놓기)은 호스트 판정이지만, **CMC가 읽는 무게 값은 캐릭터의 복제 변수**로
  둬서 자율 프록시도 같은 값으로 예측한다 (PlayerState 미러와 별개).
- 수레를 끌면 `CartMult`(0.75)가 곱해진다.

## 캐릭터 Def 적용 경로

캐릭터는 ACharacter 계열이라 actor-base의 SetAlias 틀이 없다 — 별도 경로 (2026-09-02 설계):

```
로비: PlayerState.SelectedCharacter (Replicated — session.md)
  ↓ 밤 시작, GameMode가 스폰할 때
호스트: CharacterDef = DefRegistry->FindCharacter(SelectedCharacter)
  → 속도·스태미나·캡슐 값 적용 (판정 값 — 호스트가 권위)
  → 캐릭터의 Replicated CharacterAlias 설정
  ↓ OnRep_CharacterAlias (각 클라)
클라: Def 로드 → 메시·AnimBP·TintColor·VoiceSet 적용 (표현)
```

- 속도 등 판정 값은 CMC가 Def에서 읽는다 — 로드 검사(A1 행별)가 이 값 기준.
- 슬롯 색(`TintColor`)은 슬롯 번호가 정한다 — 배정 규칙은 [session.md](session.md).

## 흔들림 (들고 달리기) — 판정·표현 (2026-09-02 설계)

- **게이지**: 취약물(`bFragile`)을 들고 달리는 동안 축적 (`ShakeFullTime` 3.0초 만충),
  걷거나 서면 회복 (`ShakeDrainTime` 1.5초). CMC 틱에서 시뮬 (스태미나와 같은 예측 경로).
- **판정 효과 (초안)**: **만충 순간 손의 취약물이 금 간다** (Crack — 이미 금 갔으면 깨짐).
  "계란 들고 전력질주 3초 = 금"이 운반 마찰의 산수가 된다. 판정은 호스트.
- **표현**: 1인칭 카메라 셰이크 진폭을 게이지 값으로 스케일 (게이지 0.3부터 감지 가능하게) +
  손 뷰모델 흔들림. **바·게이지 UI는 없다** — 화면 흔들림이 곧 게이지 표시다 (rules.md).
- 타인 시점: 게이지는 복제하지 않고, 달리기+취약물 소지 상태에서 파생한 애님 흔들림만.

## 점프

- `JumpVelocity` 780 / `JumpGravity` 1950 (정점 130cm) — 캐릭터별 값
- 판정 역할은 **낮은 장애물 통과** 하나 (segkind.md 점프 통과 셋). 거리·소음·감각 판정은
  평면(2D) 기준 — 점프로 회피 불가 (rules.md)
- 던지기 원점에 점프 높이를 더하지 않는다

## 피격·넘어짐·죽음 — OnDamage 가상 함수

**데미지 수신 통로는 가상 함수 하나다** (2026-09-02 확정 — `TakeDamage` 대체):

```cpp
// 캐릭터·몬스터가 각자 구현 (베이스가 달라서 인터페이스로)
class ICL9Damageable {
  virtual void OnDamage(int32 Amount, AActor* Source) = 0;  // 호스트에서만 호출
};
// ACL9Character::OnDamage — HP 차감, HurtStun, 넘어짐, 짐 흘림
// ACL9Monster::OnDamage  — HP 차감, HitStun, 죽음 (monster.md)
```

- 호출은 **호스트 판정 코드에서만** (몬스터 잡기, 프라이팬 밀치기 등) — 클라 호출 금지.
- HP는 PlayerState의 int (기본 3 — 값의 문제, character.md). 잡히면 `CatchDamage`만큼
  OnDamage로 깎이고 `HurtStun`(0.35초) 비틀 + 넘어짐 + **짐 흘림(손+품 전부 — 초안)** —
  판정은 호스트, 표현은 OnRep/Multicast
- 넘어짐 동안 이동 불가 — CMC 최대 속도 0 방식으로 확정 (Replicated
  `KnockdownEndServerTime`을 양쪽 CMC가 읽는다; `KnockdownTime` 캐릭터 표 컬럼 —
  2026-09-03 decisions.md)
- HP 0 → 사망 → `SpectateDelay` 뒤 관전 ([ui.md](ui.md)) — 회복 채널은 트럭 패드
- 트럭 패드 회복: `TruckPadHealTime` (미정 — rules.md)

## 네트워크 정리

| 상태 | 위치 | 이유 |
|---|---|---|
| 달리기 플래그·스태미나·유효 무게 속도 | **CMC (예측 경로)** | FSavedMove로 서버·클라 동기 |
| HP·넘어짐·운반 목록·탈진 표시 | PlayerState (Replicated + OnRep) | 판정·UI·타 클라 표시 |
| 위치·회전 | ACharacter 표준 (네트워크 스무딩 내장) | |
| 잡힘·피격 판정 | 호스트 전용 (몬스터 → 캐릭터 함수 직접 호출) | |

## 구현 스텝 매핑 (build-order ①)

| 스텝 | 이 문서에서 만드는 것 |
|---|---|
| 1a | ACL9Character + 걷기·1인칭 시점 입력 (기본 CMC) |
| 1b | 속도 값을 규칙 에셋에서 — 에디터 변경 즉시 반영 |
| 1d | **UCL9CharacterMovement + FSavedMove_CL9** — 달리기 예측 (제일 어렵다, 며칠 각오) |
| 1e | 스태미나 세 조항 (CMC 틱 안에서) |
| 1f | 2인 리슨 접속 — 예측·보정 검증 |

## 미결

- 무게→감속 식과 값 (전역 시스템 몫 — 로드 검사에 최대 무게 기준으로 들어가야 함)
- 관전 카메라 상세 (동료 1인칭 승계 — 초안)
- 낙하 데미지 유무 — 어느 문서에도 정의가 없다 (①에서 정한다. 없다면 그것도 명시)
- 먹기 입력 — 요리를 먹는 행위의 IA가 input.md 표에 없다 (요리 버프는 있는데 입력이 미정)
- 캐릭터 발 소음 이벤트의 발신 구조 — 몬스터 감각 파이프라인(③)과 함께 정한다

## 참고

- 몸 수치·스태미나 값: [../plan/data/character.md](../plan/data/character.md)
- 운반 상수·소음: [../plan/data/rules.md](../plan/data/rules.md)
- 입력 바인딩: [input.md](input.md) · 운반 상태: [inventory.md](inventory.md)
- 애니메이션 연동: [animation.md](animation.md)
