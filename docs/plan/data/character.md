# 캐릭터 — `UCL9CharacterDef`

플레이어 외형과 몸 수치·스태미나. **지금은 캐릭터마다 같은 값이다** — 값을 가르려면 별도
결정이 필요하다.

---

## 컬럼

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `Alias` | `FName` | `farmer_a` · `farmer_b` … |
| `DisplayName` | `FText` | 선택 화면에 뜨는 이름 |
| `Icon` | 소프트 참조 | 선택 화면 썸네일 |
| `SkeletalMesh` | 소프트 참조 | 스켈레탈 메시 (베이스 `Mesh`는 아이템용 StaticMesh라 별도 필드) |
| `Skeleton` | 소프트 참조 | 스켈레톤 |
| `AnimBP` | 소프트 참조 | 애니메이션 블루프린트 |
| `ArmsMesh` | 소프트 참조 | 1인칭 팔 메시 — **소유자에게만 보인다** |
| `ArmsAnimBP` | 소프트 참조 | 1인칭 팔 애니메이션 블루프린트 |
| `ActorClass` | 소프트 참조 | 전용 BP (보통 비운다) |
| `TintColor` | `FLinearColor` | 슬롯 색 |
| `VoiceSet` | 소프트 참조 | 반응 음성 묶음 — 잡힘·넘어짐·탈진 |
| `FootstepSet` | 소프트 참조 | 발소리 에셋 — 같은 걸 연결하면 같은 소리 |
| `Gimmicks` | Instanced | 연출 전용 |

**`VoiceSet`·`FootstepSet`은 순수 연출이다** — 몬스터 청각 판정에 들어가지 않고, 발각 소음
반경은 규칙 값이 정하는 별개 축이다. 반응 음성은 어둠 속에서 동료의 상황을 귀로 알리는
채널이다.

> **이 두 열은 아직 `UCL9CharacterDef`에 없다.** 소리 자체는 있다 — `SoundTable`(`Content/Data`) 행
> 11개(`CL9SoundNames`: Pickup·Place·Swing·Hit·Hurt·MonsterDeath·Footstep·MonsterPounce·MonsterCatch·MonsterNear·HitSurface,
> 2026-09-18~22, [../../systems/sound.md](../../systems/sound.md))에 control-feel(2026-10-05)이 몸 동작 소리 8행
> (Jump·LandSoft·LandHard·SlotSwitch·LightToggle·Eat·Exhausted·CartGrab — 소유자에게만 들린다)을 더한다 — 그러나
> **종별 음성·발소리 세트 열은 없다.** 발소리는 종 공통 `Footstep` 행 하나를 `CL9Rules.FootstepStrideRun/Walk` 공용
> 보폭으로 낸다(2026-09-18, 종별 보폭 열은 다음에). control-feel 부터는 거리 누적기 대신 정규화 걸음 위상(`StepPhase`)이다 —
> 보폭이 걷기 150 cm ↔ 달리기 196 cm 사이에서 속도에 따라 보간되고, 1인칭 카메라 보브와 같은 시계를 쓴다
> (docs/goal/control-feel.md C10). 종별 열은 소비처가 생길 때 넣고, 그때 이 두 줄이 명세가 된다.

---

## 몸 수치

| 컬럼 | 뜻 | 기본값 | 구 CCL8 상수 |
|---|---|---|---|
| `WalkSpeed` | 걷기 (cm/s) | 396 | `WalkSpeedU 132` |
| `RunSpeed` | 달리기 (cm/s) | 654 | `RunSpeedU 218` |
| `CarryMult` | 드는 무게에 곱하는 배율 — 1보다 작으면 가볍게 든다 | 1.0 (초안) | 구 `CarryMult` 0.70은 속도 배율 — 뜻이 다르니 이주 시 환산 |
| `CartMult` | 수레 끌 때 배율 | 0.75 | `CartMult` |
| `CapsuleRadius` | 캡슐 반경 (cm) | 39 | `PlayerRadiusU 13` |
| `CapsuleHalfHeight` | 캡슐 반높이 (cm) | 90 | 이식 판정 |
| `EyeHeight` | 눈높이 (cm) — **시점은 1인칭이다** (2026-09-01 확정), 카메라가 여기 붙는다 | 156 | `EyeHeightU 52` |
| `JumpVelocity` | 점프 초속 (cm/s) — 정점 156cm (= 780²/(2×1950), 2026-09-02 재계산 정정) | 780 | `JumpVelocityU 260` |
| `JumpGravity` | 점프 중력 (cm/s²) | 1950 | `JumpGravityU 650` |

`RunSpeed`는 누르는 즉시가 아니라 아래 질주 램프를 따라 0.85초에 걸쳐 도달한다 — 최고 속도 자체는 그대로다.

## 이동 감각 — 가속·제동·질주 램프·공중 제어 (Category `Movement`)

조작 손맛(docs/goal/control-feel.md C1·C3~C7·C13)의 열이다. 2026-10-05 전까지는 이 값들이 전부 엔진 기본값이었다
(가속·제동 2048, 마찰 8, 공중 제어 0.05 — 달리다 멈추는 데 0.11초, 질주를 놓으면 0.025초라 무게가 없었다).
**아래 기본값은 전부 초안(2026-10-05, PIE 관찰 전)이다** — 레퍼런스(Lethal Company·R.E.P.O.)에서 띠를 정하고 파이썬
모사(30~240 Hz)로 고른 값이며, Gate A 의 PIE 측정(`Tools/Unreal/qa_control_feel.py`)과 사람의 손으로 아직 확인하지 않았다.
헤더 기본값이 곧 의도한 값이다 — `CharacterTable`의 `farmer_a` 행에 셀이 없으면 이 기본값을 받는다. 값은 `ACL9Character::ApplyCharacterDef`가
`MaxWalkSpeed` 옆에서 CMC 멤버로 넣는다.

| 컬럼 | 뜻 | 기본값 | 구 CCL8 상수 |
|---|---|---|---|
| `Accel` | 입력 가속 (cm/s²) → CMC `MaxAcceleration`. 0에서 걷기 90 %까지 0.183초 | 2000 — 초안(2026-10-05, PIE 관찰 전) | **신규** |
| `BrakingDecel` | 입력을 놓았을 때의 제동 감속 (cm/s²) → `BrakingDecelerationWalking` | 1300 — 초안(2026-10-05, PIE 관찰 전) | **신규** |
| `BrakingFriction` | 제동 마찰 (1/s) → `BrakingFriction` (`bUseSeparateBrakingFriction` 켬, `BrakingFrictionFactor` 1) | 3.5 — 초안(2026-10-05, PIE 관찰 전) | **신규** |
| `GroundFriction` | 걷기 조향 마찰 (1/s) → `GroundFriction`. 걷기 속도의 방향 전환은 오늘과 같다 | 8 — 초안(2026-10-05, PIE 관찰 전) | **신규** |
| `SprintTurnFriction` | 전력 질주 중 조향 마찰 (1/s) — 무브마다 실제 마찰 = Lerp(`GroundFriction`, 이 값, 질주 램프) | 5.0 — 초안(2026-10-05, PIE 관찰 전) | **신규** |
| `SprintRampUpTime` | 질주 램프 0→1 시간 (초, ease-out quad). 속도 상한 = 걷기 + (달리기 − 걷기) × 램프 | 0.85 — 초안(2026-10-05, PIE 관찰 전) | **신규** |
| `SprintRampDownTime` | 질주를 놓았을 때 램프 1→0 시간 (초, 선형) | 0.25 — 초안(2026-10-05, PIE 관찰 전) | **신규** |
| `AirControl` | 공중 제어 (0~1) → `AirControl`. `Accel` × 0.32 = 가로 640 cm/s² | 0.32 — 초안(2026-10-05, PIE 관찰 전) | **신규** |

**기대하는 움직임(60 Hz 모사값 — Gate A 가 PIE 에서 다시 잰다):** 걷다 멈춤 0.200초·31.8 cm, 전력 질주에서 멈춤
0.283초·71.4 cm, 걷기 중 질주를 누르면 늘어날 속도의 90 %(628 cm/s)까지 0.583초, 앞을 쥔 채 질주를 놓으면 0.250초에 걷기 속도,
전력 질주 중 90도 꺾기는 0.25초에 새 방향으로 정렬되고 옛 방향으로 103 cm 밀린다. 점프 정점 156 cm 와 달리기 점프 거리는 그대로이고,
선 채로 점프하며 앞을 쥐면 약 205~225 cm 를 간다(공중 제어 0.05 → 0.32 의 결과).

- **공중에서는 제동 마찰을 쓰지 않는다** — `BrakingFriction`을 따로 켜면 엔진이 공중에도 넘기므로 CMC 가 낙하 중에는
  `FallingLateralFriction`만 쓰게 되돌린다(C3). 그래서 질주 점프의 가로 속도는 착지까지 유지된다.
- **질주 램프는 예측 상태다** — 스태미나처럼 CMC 무브 안에서 적분하고 저장 무브가 스냅샷을 들고 합칠 때 되돌린다(C1·C2).
  `IsRunning()`은 그대로 플래그 기준이다(스태미나 소모 조건·애니메이션이 읽는다).
- **끄는 법(데이터만):** 램프 시간 0 이면 즉시(옛 동작에 가깝다). 제동 둘·마찰·공중 제어도 셀 값으로 되돌릴 수 있다 — 빌드가 필요 없다.
- **검증 조건(`Validate`):** `Accel` > 0, `GroundFriction` > 0, 0 < `SprintTurnFriction` ≤ `GroundFriction`, 제동 둘 ≥ 0,
  램프 시간 ≥ 0, `AirControl` ∈ [0, 1]. "제동이 떨어지는 상한을 따라잡나"(1300 + 3.5 v ≥ 1032)는 행을 빼지 않고 Warning 로그만 낸다.
- **스태미나 산수(C7):** 소모 규칙은 그대로다(질주 중 움직이면 이진 소모). 램프 때문에 질주 거리가 조금 줄고 놓을 때 조금 는다 —
  램프 손실 `(Run−Walk)×SprintRampUpTime/3` = 73.1 cm, 놓을 때 이득 `(Run−Walk)×SprintRampDownTime/2` = 32.3 cm,
  질주 예산 `Run×Drain − 램프 손실` = 3851 cm, 지친 지속 속도 `(Run×Drain + Walk×Regen − 램프 손실 + 놓을 때 이득)/(Drain+Regen)`
  = 503.7 cm/s. `CL9DefRegistry::CheckCharacterRelations`가 이 식으로 행마다 검사하고, 부등식은 뒤집히지 않는다.
- 조건부 열 `CoyoteTime`(0.12초)·`JumpBufferTime`(0.15초)은 스트레치 S1 이 들어올 때만 생긴다 — 0 이면 꺼진다.

## 체력

| 컬럼 | 뜻 | 기본값 | 구 CCL8 상수 |
|---|---|---|---|
| `HpMin` | 최소 체력 | 3 | `HpMax 3` |
| `HpMax` | 최대 체력 | 3 | `HpMax 3` |
| `HurtStun` | 완충타 비틀 (초) | 0.35 | `HurtStun` |
| `KnockdownTime` | 잡혔을 때 넘어져 못 움직이는 시간 (초) | 1.5 (초안 — 3e에서 신설) | **신규** |

즉사 방지가 필요하면 HP 값으로 해결한다 — 시스템 원칙이 아니라 캐릭터별 값의 문제다.

## 스태미나

| 컬럼 | 뜻 | 기본값 | 구 CCL8 상수 |
|---|---|---|---|
| `StaminaDrainTime` | 만충에서 0까지 (초) | 6.0 | `StamDrainT` |
| `StaminaRegenTime` | 0에서 만충까지 (초) | 8.0 | `StamRegenT` |
| `StaminaMinResume` | 탈진 후 재개 문턱 (스태미나는 0~1 정규화 — 1.0 = 만충까지 잠금) | 1.0 | `StamMinResume` |

**세 조항이 전부 있어야 한다** — 탈진 래치 · 재개 히스테리시스 · 실이동 조건.
하나라도 빠지면 탈출 보증의 산수 전제가 무너진다. 지친 빈손 지속 속도 506(질주 램프 반영 뒤 503.7 — 위 "이동 감각")이
"질주 1.5초 + 걷기 2.0초 사이클"에서 나온 값이기 때문이다.

## 스탯 enum — `ECL9Stat`

먹기 효과([item/common.md](item/common.md))가 이 enum으로 "무엇을 움직이는지"를 가리킨다 — 효과의
대상·양·지속을 전부 표에서 정하기 위해서다.

멤버는 **9개**다 — 이동 4종(`WalkSpeed` · `RunSpeed` · `CarryMult` · `CartMult`) +
점프 2종(`JumpVelocity` · `JumpGravity`) + 스태미나 3종(`StaminaDrainTime` ·
`StaminaRegenTime` · `StaminaMinResume`).

**캡슐·눈높이는 목록에 없다** — 통로 폭 255cm와 요충지 거리가 걸린 기하 전제라서다.
**HP도 없다** — 회복 채널은 트럭 패드 하나뿐이다 ([rules.md](rules.md)).

**점프가 들어간 대가:** 점프 통과/불가를 가르는 기하 검사는 버프가 적용된 최대치 기준으로
돌아야 한다 — 안 그러면 버프 하나가 "걸어서는 못 넘는다"는 전제를 뒤집는다.

---

## ★ 로드 검사가 캐릭터 행마다 돈다

```
지친 빈손 지속 속도 503.7 (질주 램프 반영 — 램프 전 506)  >  닭 추격 396
```

이 값은 `RunSpeed`·`WalkSpeed`·스태미나 3종과 질주 램프 두 열(`SprintRampUpTime`·`SprintRampDownTime`)에서
**계산되어 나오는 파생값**이다 — 식은 위 "이동 감각" 절.
캐릭터마다 값이 다르면 부등식도 캐릭터마다 달라지므로, 검사는 전역 한 번이 아니라
**행마다** 돈다.

검사 없이 값만 갈면 그 캐릭터를 고른 사람은 짐을 다 버려도 못 도망치고, **화면에는 아무 표시도
안 난다.**

값을 가르려면 별도 결정이 필요하다 — 지금은 캐릭터마다 같은 값이다.

---

## `TintColor`와 슬롯

**슬롯 0~3이 곧 신원이다.** 발각 상태·이름표·색을 슬롯 번호 기준으로 식별한다 (구현 시
슬롯 인덱스가 식별 키).

`TintColor`는 **어둠 속에서 누가 누군지 알아보는 유일한 단서**라 채도가 충분해야 한다.
캐릭터를 고르는 것과 슬롯을 받는 것은 별개다 — **고르는 곳은 로비다.** 캐릭터가 2종인 동안
중복 선택은 불가피하므로, 식별은 캐릭터 외형이 아니라 슬롯 색이 담당한다.

---

## 에셋 교체 대비

C++ `ACL9Character`는 **메시나 애니메이션 경로를 한 줄도 갖지 않는다.**

```
ACL9Character (C++)           BP_CL9Character (블루프린트 자식)
  캡슐 39 / 90                   메시 · 머티리얼 · AnimBP
  이동 · 네트워크 · 카메라 · 입력
```

`DefaultPawnClass`는 C++ 클래스가 아니라 **BP를 가리킨다.**

**캡슐 크기가 기준이고 메시가 거기 맞춘다.** 산 캐릭터가 크다고 캡슐을 키우면 통로 폭 255cm와
요충지 거리가 깨진다.

---

## 지금 있는 것

`CharacterTable`(`Content/Data`)의 `farmer_a` 행 — 자체 제작 농부 A 전신·1인칭 팔(`Content/Characters/FarmerA`,
2026-09-11 연결, 09-13 카메라 자식 뷰모델 `ABP_FP_View` — [../../asset/character/farmer_a.md](../../asset/character/farmer_a.md))이
붙어 있고, 몸 수치는 위 표 값이다. `TintColor`는 파동 링의 슬롯 색으로 쓰인다 —
`ACL9PlayerState::GetSlotColor`가 캐릭터 행에서 읽는다(2026-09-16, 슬롯 번호 배정은 아직 없음).
외형은 데이터 에셋이라 행 추가로 늘린다 — 두 번째 외형(`farmer_b`)은 로비 캐릭터 선택
([../../systems/session.md](../../systems/session.md))과 함께 온다.
