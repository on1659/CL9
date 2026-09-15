# 캐릭터 시스템 (이동·스태미나 — 커스텀 CMC)

> build-order가 "1d가 제일 어렵다"고 한 바로 그 시스템. `ACL9Character : ACharacter` +
> **커스텀 CMC + `FSavedMove`** (A4 결정). 값의 정본은
> [../plan/data/character.md](../plan/data/character.md) · [../plan/data/rules.md](../plan/data/rules.md).

## 시점 — 1인칭 (2026-09-01 확정)

- 카메라는 눈높이 `EyeHeight` 156cm에 붙는 **1인칭**이다. 손전등 조준·던지기 조준·시선이
  전부 카메라 방향에서 나온다.
- **자기 화면에는 손(팔)만 보인다** (2026-09-02 확정). 든 오브젝트는 **전신 메시의 손 소켓
  `HandGrip_R`**에 붙고(2026-09-12), 소유자 본인 화면에서는 메시만 1인칭 팔의 같은 소켓으로 옮겨
  보인다 — 계란·손전등·프라이팬·냄비·램프 전부 같은 소켓이다. 타인 시점도 같은 소켓이다(`CarryHeight`
  부착은 폐기). 세부는 [inventory.md](inventory.md). 빈손이면 팔은 화면 밖이고 달릴 때만
  팔을 흔들며 들어온다(2026-09-13 — inventory.md `HoldPose` `None`).
- **1인칭 팔은 카메라의 자식(뷰모델)이다** (2026-09-13, 사용자 "리썰 보고 그 조작감으로" —
  리썰 방식으로 전환). `FirstPersonMesh`는 `FirstPersonCameraComponent`에 상대 위치 `(0,0,−EyeHeight)`·
  요 −90°로 붙는다(전신 마네킹과 같은 규격 — 리그 root가 원점, 눈이 (0,0,EyeHeight)에서 +X를 보는
  공간; `ApplyCharacterDef`가 행의 `EyeHeight`로 다시 맞춘다). **소유자 화면의 팔·손전등은 카메라를
  그대로 따른다** — 피치·요 통째. 팔 ABP(`ABP_FP_View`)는 몸 포즈를 보지 않고 이 공간에서 저작한
  카메라 로컬 클립만 재생한다(몸 포즈 복사 `ABP_FP_Copy`·`CtrlRig_FPWarp`·팔 쪽 `spine_03` 기울임 폐기).
  스태미나 셰이크는 POV만 흔들어 화면이 흔들리고 팔은 따라 떨리는 듯 보인다(리썰과 같음).
- **손전등 빔은 손의 손전등 렌즈에서 나오고, 남의 화면에서는 시선 피치를 상체가 따른다** (2026-09-13,
  사용자 "발원지가 손전등이어야 한다"). 라이트 컴포넌트는 아이템 루트(구)가 아니라 **메시의 자식**이라
  (`ACL9LightItem`, 발원점 `GetLightOrigin()` = 손전등은 메시 바운드 +X 끝) 소유자 화면에서 메시가
  1인칭 팔 소켓으로 옮겨 가도 빛이 렌즈에서 메시 +X 방향으로 나간다 — 팔이 카메라 자식이니 아래를 보면
  손의 손전등이 바닥을 비춘다. 남의 화면에서 손이 기울어지는 것은 전신 ABP(`ABP_Unarmed`)가 한다 —
  `GetAimPitch()`(소유자는 컨트롤 피치, 타인은 복제된 `RemoteViewPitch`, −90..90; **전신 ABP 전용**)를
  Hold 레이어 뒤 `Transform (Modify) Bone`(`spine_03`, 컴포넌트 공간 피치 = AimPitch × 비율, 초안 0.6)에
  넣어 상체를 기울인다. 헤드램프는 그대로 카메라 부착이다([light.md](light.md)).
  ABP가 읽는 게터: `GetAimPitch()`(전신만) · `IsRunning()`(CMC 위임 — 소유자·호스트에서만 참) ·
  `GetGroundSpeed()`(cm/s 수평 속도) · `GetHoldPose()` · `GetSwingElapsed()`/`GetSwingWindupElapsed()`/
  `GetEatElapsed()`.
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
- **셰이크 표의 `Hit`·`Hurt` 행은 일회성이라 `StartAt`을 읽지 않는다** — 게이지 문턱은 `Stamina`처럼
  값이 계속 구동하는 지속형 행에만 뜻이 있고, 순간 사건인 원샷 행은 세기 1로 한 번 재생할 뿐이다
  (`ACL9Character::PlayShakeOnce`, 2026-09-15).

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
- HP 0 → 사망 (4a 구현, 2026-09-05). 폰은 그 자리에 누운 채 남고(넘어짐 무기한, 몬스터는 죽은
  대상을 건너뛴다) 캐릭터가 호스트 GameMode `HandlePlayerDeath`에 넘긴다:
  - **전멸** — 생존자가 0이면 트럭이 일행을 두고 떠난다 = `EndNight()` (밤 중일 때만. rules.md 실패 축)
  - 아니면 `SpectateDelay` 뒤 **관전**: 호스트가 `PlayerController::SetViewTarget(생존 동료 폰)`
    (엔진이 소유 클라에 ClientSetViewTarget 전달) + `bSpectating`(ReplicatedUsing) = true.
    OnRep이 로컬에서 관전 화면(`CL9UINames::Spectate` 행)과 `IMC_Spectate`를 **함께** 켠다 —
    화면·IMC 불일치 원천 차단([input.md](input.md)). 대상 전환은 `SpectateNext()`
    (BlueprintCallable → Server RPC → 접속 순서로 다음 생존자, 순환). 보던 동료가 죽으면
    자동으로 다음 생존자로 넘어간다.
  - 관전 시점은 동료의 1인칭 그대로 — 동료 폰이 뷰 타깃이 되면 OnlyOwnerSee/OwnerNoSee가
    뷰 타깃 기준으로 풀려 그 동료의 팔·시야를 본다.
- **부활은 없다** — 다음 로비(`EnterLobby`)에서 `ReviveDead()`: 죽은 전원을 `HostRevive()`
  (HP = `MaxHp`, 넘어짐 해제)하고 트럭 패드 위로 옮기며 관전을 끝낸다(뷰 타깃 = 자기 폰).
- **트럭 패드 회복** (`ACL9TruckPad`, Source/CL9/World): HealZone 안에 **살아 있고 다친**
  캐릭터가 `TruckPadHealTime`초 연속으로 서 있으면 HP가 `MaxHp`로 찬다. 벗어나면 카운트
  초기화. 죽은 사람은 여기서 안 살아난다. 호스트 전용 틱, 복제 없음(HP는 PlayerState로 도착).
- `MaxHp`는 PlayerState에 HP와 함께 한 번 굴린다(`HpMin..HpMax`). `IsDead()` = MaxHp>0 && Hp≤0
  (굴리기 전 0은 죽음이 아니다).
- 치트 `cl9.damage [n]` — 몬스터 없이 죽음→관전→패드 회복을 검증한다 ([cheats.md](cheats.md)).

### 맞는 느낌 — 타격감·피격감 (2026-09-15 구현)

판정은 하나도 건드리지 않는 표현 층이다. **모든 트리거가 이미 복제된 시각에서 파생되고**, 새로 복제하는
값은 `SwingHitServerTime` 하나뿐이다 ([../goal/hit-feedback.md](../goal/hit-feedback.md)).

| 무엇 | 누가 보나 | 원천 | 시간 |
|---|---|---|---|
| 맞은 몬스터가 빨개진다 | 전원 | `ECL9MonsterState` + `StateStartServerTime` | `HitFlashTime` |
| 공격자 카메라 킥 | 때린 본인 | `SwingHitServerTime` (신규, `COND_OwnerOnly`) | 셰이크 행 `Hit` |
| 잡힌 사람의 몸·1인칭 팔이 빨개진다 | 전원 | `KnockdownStartServerTime` | `HurtFlashTime` |
| 피해자 화면의 빨간 비네트 | 잡힌 본인 | 같은 값 → `GetHurtFlash()` 0..1 | `HurtVignetteTime` |
| 피해자 카메라 셰이크 | 잡힌 본인 | 같은 값 | 셰이크 행 `Hurt` |

- **카메라 킥은 한 휘두르기에 한 번이다.** `HostTickSwingHit`이 그 휘두르기에서 **처음으로** 살아 있는
  몬스터를 맞힌 틱에만 `SwingHitServerTime`을 찍는다 — 나란한 둘을 한 번에 맞혀도 한 번이고, 헛스윙과
  **시체 타격**에는 아예 찍히지 않는다(시체는 맞은 목록에도 넣지 않는다).
- **셰이크는 OnRep이 아니라 틱 비교로 낸다.** 리슨 호스트는 권한이자 소유자라 자기 폰에서 OnRep이
  뜨지 않는다 — 지난 틱에 본 값과 다르면 한 번 재생하는 방식이 호스트·클라 양쪽에서 똑같이 돈다
  (`UpdateHitShakes`). 접속 직후 첫 틱은 값을 받아 적기만 하고 재생하지 않는다.
- **셰이크는 표의 행이다.** `CL9ShakeNames::Hit`·`Hurt` (rules.md 셰이크 표) — 원샷이라 `StartAt`은
  보지 않고 세기 1로 한 번 재생한다. 행이 없으면 조용히 지나간다.
- **빨간 오버레이는 `HitFlashMaterial` 하나를 사람과 몬스터가 같이 쓴다** (`SetOverlayMaterial` —
  줍기 하이라이트와 같은 기제). 캐릭터 몸에는 하이라이트를 걸지 않지만(그쪽은 아이템 메시다),
  **내가 켠 것만 끈다**는 가드를 둔다 — 켜고 끌 때만 세팅하고, 켠 적이 없으면 지우지 않는다.
- **표현 시계는 `GetKnockdownElapsed()`가 아니다.** 그 게터는 일어나는 순간 −1로 돌아가서(애님 분기용)
  플래시가 영원히 켜진 것으로 읽힌다. 플래시·비네트는 `GetHurtElapsed()`(= 서버 시각 − 넘어진 시각,
  일어난 뒤에도 계속 커진다)를 쓴다. 넘어진 적이 없으면 −1000 기본값 덕에 아주 큰 값이 되어 꺼져 있다.
- HUD 비네트 이미지는 `GetHurtFlash()`를 불투명도로 읽는다 ([ui.md](ui.md)).

## 네트워크 정리

| 상태 | 위치 | 이유 |
|---|---|---|
| 달리기 플래그·스태미나·유효 무게 속도 | **CMC (예측 경로)** | FSavedMove로 서버·클라 동기 |
| HP·넘어짐·운반 목록·탈진 표시 | PlayerState (Replicated + OnRep) | 판정·UI·타 클라 표시 |
| 위치·회전 | ACharacter 표준 (네트워크 스무딩 내장) | |
| 잡힘·피격 판정 | 호스트 전용 (몬스터 → 캐릭터 함수 직접 호출) | |
| 타격 킥 시각 | `SwingHitServerTime` (Replicated, `COND_OwnerOnly`) | 때린 본인만 느끼는 연출 |
| 빨간 플래시·비네트·셰이크 | 복제 없음 — 위 시각들에서 각 머신이 파생 | 표현은 시계에서 나온다 |

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
- 캐릭터 발 소음 이벤트의 발신 구조 — 몬스터 감각 파이프라인(③)과 함께 정한다

## 참고

- 몸 수치·스태미나 값: [../plan/data/character.md](../plan/data/character.md)
- 운반 상수·소음: [../plan/data/rules.md](../plan/data/rules.md)
- 입력 바인딩: [input.md](input.md) · 운반 상태: [inventory.md](inventory.md)
- 애니메이션 연동: [animation.md](animation.md)

## 먹기 효과 (④ 4i — 2026-09-07)

`ACL9Character::HostEat()` — 손에 든 것이 `Dish` 타입일 때만 먹힌다("먹을 수 있나" 플래그를 따로 두지 않고
아이템 표의 타입이 가른다). 행의 `EatBuffs`가 가리키는 버프 행들의 효과(스탯·양·지속초)가 걸리고
먹은 물건은 사라진다 — 값은 아이템 행이 아니라 버프 표가 갖는다 ([../plan/data/buff.md](../plan/data/buff.md), 2026-09-08).

- **효과는 복제되고 서버 시각으로 만료된다**(`FCL9ActiveEffect{Stat, Amount, EndServerTime, BuffAlias}`) — 넘어짐의
  `KnockdownEndServerTime`과 같은 이유다. CMC가 이동 공식에서 읽으므로 **서버와 자율 프록시가 같은 값을
  도출해야 예측이 어긋나지 않는다.** 호스트에서만 적용하면 클라 캐릭터가 튄다.
- `GetStatBonus(ECL9Stat)`(BlueprintPure)이 만료되지 않은 효과의 합을 준다. **양은 배율이 아니라 더하는 값이다.**
  CMC는 걷기/달리기 속도, `CartMult`, 스태미나 소모·회복 시간에 이 값을 더한다.
- **HP는 효과 대상이 아니다** — 회복 채널은 트럭 패드다(docs/plan/data/rules.md).
- 지속 0(즉시형)은 아직 구현하지 않았다 — 지금 범위에 즉시 스탯 변화가 없어서 로그만 남긴다.
- 입력은 **우클 길게**(`IA_Scan`을 `CL9Rules.UseHoldTime` 0.4초 넘게 — 탭은 스캔) → `Server_Eat`(2026-09-13,
  옛 `IA_Eat` G·좌클 길게 폐기). 판정은 문턱을 넘는 순간이고 `EatStartServerTime`(복제)에서 `GetEatElapsed()`가
  먹기 클립(0.6초)을 태운다 — 애니가 도는 동안 칸 전환을 막지 않는다. 검증 치트는 `cl9.eat`.

## 휘두르기 — 공격 대기 → 뗌 = 타격 (2026-09-13)

좌클은 리썰 삽 방식이다: **누르면 공격 대기, 떼면 휘두르기.** `Server_UseStart`가 든 행의 `UseAction`이 `Swing`이면
`SwingWindupServerTime`(복제)을 기록하고, `Server_UseRelease`가 대기 중이었을 때만 `HostSwing`을 부른다 — 피해·사거리·
경직은 대기 시간과 무관하게 같다(밸런스 열 없음, 대기는 무한). 평범한 클릭은 대기 한 틱 뒤 타격이라 평범한 공격이다.
`ToggleLight`는 누르는 순간 토글이고 대기가 없다. 대기 중 손의 것이 바뀌는 모든 경로(칸 전환·놓기·던지기·먹기·잡힘)가
`HostCancelSwingWindup()`으로 대기를 지우므로 떼도 타격이 없다. 표현: `GetSwingWindupElapsed()`(대기 아니면 −1)로
ABP가 대기 자세(`AN_Farmer_SwingWindup` 루프)를, `GetSwingElapsed()`로 휘두르기 클립을 태운다 ([animation.md](animation.md)).
