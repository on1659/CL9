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
  `GetGroundSpeed()`(cm/s 수평 속도) · `GetHoldPose()` · `GetSwingAnimElapsed()`(2026-09-18 — 히트스톱이 얹힌
  `GetSwingElapsed()`, 피격 절)/`GetSwingWindupElapsed()`/`GetEatElapsed()`.
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
- HUD 바(`UCL9HUDWidget::StaminaBar`)와 화면 흔들림을 **함께** 쓴다(2026-09-05 번복 — rules.md 스태미나 표시). 셰이크 구현: 소유 클라 로컬 틱이
  셰이크 표(`UCL9ShakeTable`, GameInstance 주입)의 `CL9ShakeNames::Stamina` 행을 읽어,
  `Stamina < StartAt`면 그 행의 셰이크를 재생하고 스케일을 `(StartAt−Stamina)/StartAt`(× 행 `Scale`, 2026-09-18)로
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
  값이 계속 구동하는 지속형 행에만 뜻이 있고, 순간 사건인 원샷 행은 행의 `Scale` 세기로 한 번 재생할 뿐이다
  (`ACL9Character::PlayShakeOnce`, 2026-09-15 — 옛 "세기 1"은 아래 `Scale` 열로 바뀌었다).
- **셰이크 표 행마다 배율 `Scale`이 있다** (2026-09-18 사용자 결정, `FCL9ShakeInfo.Scale`, 기본 1.0). 진폭·주파수·블렌드
  시간은 여전히 에셋(`CS_*`) 안에 있고, 표는 그 위에 곱하는 배율만 갖는다 — 세기를 재빌드도 에셋 교체도 없이 표에서
  맞추기 위한 유일한 예외 열이다. 원샷 행(`Hit`·`Hurt`)은 `Scale` 세기로 한 번(`StartCameraShake(Class, Scale)`),
  게이지 행(`Stamina`)은 시작 세기와 매 틱 `ShakeScale`이 **게이지 세기 × `Scale`**이다(`UpdateStaminaShake`).
  1.0이면 에셋 그대로다. 셰이크 클래스는 `UCL9GameInstance::Init` 이 미리 올려 `WarmAssets` 로 붙든다 — 첫 재생 자리(잡히는 순간의
  틱)에서 로드하면 BP 컴파일까지 붙어 0.6초를 세웠다(2026-09-21 실측, [performance.md](performance.md) "첫 사용 동기 로드 금지").

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
// ACL9Character::OnDamage — HP 차감, HurtStun, 넘어짐, (죽을 때만) 짐 흘림
// ACL9Monster::OnDamage  — HP 차감, HitStun, 죽음 (monster.md)
```

- 호출은 **호스트 판정 코드에서만** (몬스터 잡기, 프라이팬 밀치기 등) — 클라 호출 금지.
- HP는 PlayerState의 int (기본 3 — 값의 문제, character.md). 잡히면 `CatchDamage`만큼
  OnDamage로 깎이고 `HurtStun`(0.35초) 비틀 + 넘어짐 — 판정은 호스트, 표현은 OnRep/Multicast
- **짐을 흘리는 것은 죽을 때뿐이다** (2026-09-15 사용자 — 2026-09-02의 "잡히면 손+품 전부 흘림" 폐기).
  HP가 남는 타격은 넘어뜨리기만 하고 **손에 든 것과 퀵슬롯 네 칸을 그대로 둔다** — 일어나면 들고 있던 그
  물건을 그대로 들고 있고(활성 칸도 그대로), 바닥에 흘린 것은 없다. 다만 두 경우 다 `HostCancelCarryActions()`가
  수레를 놓고(`StopPull`) 던지기 차징·휘두르기 대기를 취소한다 — 넘어진 채 차징이 이어지면 안 된다.
  HP 0이 되는 타격에서만 `HostDropEverything()`가 손과 네 칸을 전부 발밑에 흘린다(동료가 시체 옆에서 주워 가게).
  **넘어져 있는 동안은 놓기(G)·던지기도 무시된다** (2026-09-15 사용자 — 사용·먹기와 같은 규칙):
  `Server_Drop`·`Server_ThrowChargeStart`·`Server_ThrowRelease`가 `IsKnockedDown()`이면 조용히 돌아간다.
  짐을 든 채 넘어지게 된 뒤로 필요해진 검사다 — 누운 채로 물건을 내려놓거나 던질 수는 없다.
  이동은 CMC가 속도 0으로 묶고, 사용·휘두르기·먹기는 이전부터 같은 검사를 하고 있었다.
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
- **데스캠** (2026-09-20 밤, 실행 death-camera — D6 의 "죽은 뒤 뷰 타깃을 폰으로 되돌리지 않는다"를 **번복**): 죽은 순간부터
  관전이 시작되기 전까지(혼자면 밤이 끝날 때까지) 피해자 화면은 **자기 시체를 3인칭으로** 본다 — 처형 즉사·보통 사망 공통.
  - **컴포넌트** `ACL9Character::DeathCamera`(`UCameraComponent`, 생성자에서 캡슐에 부착, `bAutoActivate` false). 자리 =
    `CL9Rules.DeathCameraOffset`(**액터 공간**, X = 전방·Y = 오른쪽·Z = 위 — 처형 카메라와 같은 규약), 회전 = 그 자리에서
    `(0,0,DeathCameraLookAtZ)`(캡슐 중심 기준)를 보는 `FRotationMatrix::MakeFromX` — 둘 다 액터 공간이라 한 번 계산이고,
    캡슐이 돌아도(컨트롤 요를 따른다) 시체는 화면의 같은 자리에 잡힌다. 1인칭 카메라와 **별개 컴포넌트**라 D6 이 피하려던
    "넘어짐 카메라가 보이는 몸 속에 앉는" 문제가 없다.
  - **켜기/끄기 `ApplyDeathCamera()`** (넘어짐·처형과 같은 "복제된 상태에서 파생" 틀): `ShouldUseDeathCamera()` =
    `IsDead()`(PlayerState HP, 이미 복제) ∧ `DeathCameraOffset ≠ (0,0,0)`. 참이면 1인칭 카메라 `SetActive(false)`·데스캠
    `SetActive(true)`(+자리·회전), 아니면 반대(1인칭 복구). 뷰 타깃이 폰이고 활성 카메라가 데스캠뿐이면 `CalcCamera` 가 그것을
    쓴다(둘 다 활성이면 첫 것 — 그래서 1인칭을 끈다). 호출 자리: 호스트는 `HostApplyDamage`(HP 0 이 되는 자리)·`HostRevive`,
    소유 클라(와 늦게 도착한 PlayerState)는 `Tick` 의 `bDeathCameraOn != ShouldUseDeathCamera()` 비교 — `OnRep_Hp` 가 캐릭터에
    닿는 경로가 없어서다. `HostApplyDamage` 는 PlayerState 를 `ForceNetUpdate` 해 HP 와 뷰 타깃 RPC 가 같은 틱에 가게 한다.
  - **뷰 타깃**: HP 0 이 되는 순간 서버가 `PC->SetViewTargetWithBlend(폰, CL9Rules.DeathCameraBlend)` — 처형 중이었으면 몬스터
    카메라에서 데스캠으로 블렌드(`HostBeginExecuted` 의 짝). 보통 사망은 뷰 타깃이 이미 자기 폰이라 엔진이 블렌드하지 않는다 =
    넘어짐 카메라에서 데스캠으로 **즉시 전환**(같은 액터 안의 두 카메라 사이엔 `SetViewTargetWithBlend` 가 없다). `SpectateDelay`
    뒤 `BeginSpectating` 의 `SetViewTarget(동료)` 는 그대로(데스캠 → 동료 1인칭); 혼자면 데스캠 유지 → 전멸 → 밤 종료.
  - **몸·팔** `ApplyThirdPersonMeshes()`(`ApplyExecuted` 와 공통부): 처형 중 ∨ 데스캠이면 몸 `FirstPersonPrimitiveType`
    `WorldSpaceRepresentation → None` + `SetOwnerNoSee(false)`·팔(과 손의 아이템) 숨김, 아니면 1인칭(타입 복구). 타입을 같이
    내리는 이유: `WorldSpaceRepresentation` 프록시는 `bOwnerNoSee` 를 강제로 true 로 덮어써서(엔진 `PrimitiveSceneProxy.cpp`)
    OwnerNoSee 토글만으로는 몸이 안 보이고 그림자만 남는다(2026-09-21 PIE 에서 발견). 처형 표시가 먼저 걷혀도(`HostEndExecuted`)
    데스캠이 켜져 있으면 3인칭 그대로 — 부활이 되돌린다.
  - **게이트**: 넘어짐 카메라(1인칭을 바닥으로 내리는 `Tick`)는 데스캠이 켜져 있으면 건너뛴다(살아 있는 넘어짐은 그대로).
    `UCL9HurtLookModifier`·`PlayShakeOnce`·스태미나 셰이크·`GetHurtFlash`·`HasHurtDirection`·조준 대상/프롬프트는
    `ShouldUseDeathCamera()` 면 아무것도 하지 않는다(처형 게이트 옆) — 데스캠 위에 1인칭 표현을 얹지 않는다. 마우스 시선은
    막지 않는다 — 캡슐 요가 컨트롤 요를 따르므로 죽은 채 마우스를 돌리면 데스캠이 시체를 중심에 둔 채 함께 돈다.
  - **부활** `HostRevive` 가 `ApplyDeathCamera()` 로 1인칭·팔을 되돌린다(뷰 타깃은 이미 자기 폰; 관전 중이었으면 `ReviveDead` 가
    되돌린다). 죽은 채 로비로 가는 길은 전부 `EnterLobby → ReviveDead` 를 지난다.
  - **롤백** `DeathCameraOffset (0,0,0)`: 데스캠 없음 — 옛 동작(넘어짐 카메라가 바닥으로, 처형이면 몬스터 카메라에 머묾, 피격
    카메라·셰이크 유지). 뷰 타깃도 건드리지 않는다. 빌드 없이.
  - 처형한 몬스터는 `SpectateDelay` 까지 시체 옆에 머문다(D6 유지) — 데스캠에 시체와 몬스터가 함께 잡히는 그림.
  - 값: `DeathCameraOffset`(−320, 140, 260)·`DeathCameraLookAtZ`(−60)·`DeathCameraBlend`(0.5) —
    [../plan/data/rules.md](../plan/data/rules.md) 죽음과 회복, 아트 튜닝. 로드 검사: 오프셋·높이 유한, 블렌드 유한·비음수.
- **부활은 없다** — 다음 로비(`EnterLobby`)에서 `ReviveDead()`: 죽은 전원을 `HostRevive()`
  (HP = `MaxHp`, 넘어짐 해제, 데스캠 끔)하고 트럭 패드 위로 옮기며 관전을 끝낸다(뷰 타깃 = 자기 폰).
- **트럭 패드 회복** (`ACL9TruckPad`, Source/CL9/World): HealZone 안에 **살아 있고 다친**
  캐릭터가 `TruckPadHealTime`초 연속으로 서 있으면 HP가 `MaxHp`로 찬다. 벗어나면 카운트
  초기화. 죽은 사람은 여기서 안 살아난다. 호스트 전용 틱, 복제 없음(HP는 PlayerState로 도착).
- `MaxHp`는 PlayerState에 HP와 함께 한 번 굴린다(`HpMin..HpMax`). `IsDead()` = MaxHp>0 && Hp≤0
  (굴리기 전 0은 죽음이 아니다).
- 치트 `cl9.damage [n]` — 몬스터 없이 죽음→관전→패드 회복을 검증한다 ([cheats.md](cheats.md)).
  2026-09-20부터 **가장 가까운 살아 있는 몬스터를 `Source`로** 넘긴다(없으면 null) — 피격 사건이 진짜 잡힘과 같은
  그림(카메라·방향 호)이 된다.
- **넘어진 동안은 무적이다** (2026-09-20 사용자 결정): `OnDamage` 머리의 `IsKnockedDown()` 게이트가 HP·넘어짐 시계·
  피격 사건을 전부 그대로 두고 로그 한 줄만 남긴다. 다른 닭의 덮침도 `cl9.damage`도 같은 문이라 **우회가 없다**.
  몬스터 쪽은 넘어진 타깃에 예고·잡기를 시작하지 않는다 ([monster.md](monster.md) 잡기 = 공격).
  유일한 예외는 아래 처형의 `HostKill` — 이 문을 아예 거치지 않는다.
- **처형 — 종 전용 즉사** (2026-09-20, [../goal/execute-sequence-pig.md](../goal/execute-sequence-pig.md) D2·D3·D6·D9;
  몬스터 쪽은 [monster.md](monster.md) 스토킹과 처형 — 너구리는 스토커, 돼지는 잡히면 처형이지만 캐릭터 쪽 흐름은 같다):
  - **상태** = `ExecuteStartServerTime`(float, `ReplicatedUsing=OnRep_ExecuteStartServerTime`, 모두에게 — 남의 ABP 와
    CMC 가 읽는다; −1000 = 아님) + `ExecutedBy`(`TObjectPtr<ACL9Monster>`, Replicated). 게터(BlueprintPure):
    `IsExecuted()`(= 시각 ≥ 0) · `GetExecuteElapsed()`(처형 아니면 −1) · `GetExecuteDuration()`(`ExecutedBy` 행의
    `ExecuteDuration`, 없으면 0) · `GetExecutedByAlias()`(`ExecutedBy` 행의 `Alias` — `pig`/`raccoon`, 없으면 `None`;
    ABP 가 종별 2인 클립을 고른다, 2026-09-21); C++ `IsIncapacitated()`(= 넘어짐 ∨ 처형) · `GetExecutedBy()`. **`ExecutedBy` 는
    죽은 뒤에도 남는다**(ABP·카메라 게이트 — 게터들도 그동안 같은 값) — 로비 정리·부활이 걷는다.
  - **`HostBeginExecuted(Executioner)`** (호스트, 몬스터가 부른다): 두 필드 세팅 → `HostCancelCarryActions()`
    (**수레는 진입 순간 놓는다** — 무력화된 사람이 수레를 끌 수 없다; 던지기 차징·휘두르기 대기도 취소. 짐은 죽는
    순간 흘린다) → `ApplyExecuted()` → **피해자 요 정렬**(아래) → 소유 PC 에 서버 `SetViewTargetWithBlend(Executioner, CL9Rules.ExecuteCameraBlend)`
    (관전과 같은 경로 — 엔진이 `ClientSetViewTarget` 으로 전달, 리슨 호스트 자기 폰도 같은 호출) → `ForceNetUpdate`.
  - **피해자 요 정렬** (2026-09-21 사용자 결정 — 2인 클립은 두 리그의 상대 요를 가정한다): 잡히면 처형인 종(`bExecuteOnCatch` — 돼지 클립은
    "정면에서 박힘")이면 요 = (처형자 − 피해자).Yaw 로 **처형자를 정면으로**, 앵커 텔레포트 처형(스토커 — 너구리 클립은 "등 뒤")이면
    요 = (피해자 − 처형자).Yaw 로 **처형자를 등진 채**(앵커가 피해자 전방 기준이라 서버 요는 이미 등 — 클라 요와의 오차만 없앤다).
    갈림은 `ExecutedBy` 행의 `bExecuteOnCatch` 열이지 종 이름이 아니다. 피치·롤 0, 처형자가 같은 XY 위면 요 그대로. 적용은 **액터 요 +
    컨트롤 회전 양쪽**: 피해자는 자율 프록시라 서버 `SetActorRotation` 만으로는 다음 `ServerMove` 가 되돌린다(CMC `ServerMove_HandleMoveData` 가
    매 이동마다 `SetControlRotation(클라 컨트롤 회전)` → `UpdateRotation` → `FaceRotation`, 캡슐 요는 `bUseControllerRotationYaw`). 그래서
    서버 `SetActorRotation` + `PC->SetControlRotation`(서버 사본 즉시 — 조준·`GetBaseAimRotation`) + `PC->ClientSetRotation`(엔진 Client RPC:
    클라 `SetControlRotation` + `Pawn->FaceRotation`; 리슨 호스트 자기 폰은 로컬 호출). 피치는 지금 값 유지. 이후 마우스는 `LookInput` 의
    `IsExecuted()` 게이트(D2)와 내려간 IMC 가 막으므로 별도 요 잠금은 없다. 클라의 RPC 도착 전에 보낸 `ServerMove` 한두 개가 서버 요를 잠깐
    되돌릴 수 있으나 RPC 가 닿으면 다시 맞는다(≤ RTT).
  - **`ApplyExecuted()`** (OnRep 과 호스트 양쪽 — `HostSetSpectating` 패턴): 소유 클라면
    `PC->SetGameplayInputEnabled(!IsExecuted())`, 몸·팔은 `ApplyThirdPersonMeshes()`(처형 중 ∨ 데스캠: 몸
    `FirstPersonPrimitiveType None` + `SetOwnerNoSee(false)` — 처형 카메라에서 자기 몸이 보인다, 팔 `FirstPersonMesh->SetVisibility(false, 자식까지)` — 팔과 손의
    아이템이 사라진다). 죽은 뒤 관전 전환 때 몸이 보이는 채로 남아도 관전은 남의 시점이라 무방.
  - **잠금**: `UCL9CharacterMovement::GetMaxSpeed()` 가 `IsExecuted()` 면 0(넘어짐 옆), `LookInput` 은 처형 중
    무시. **서버 RPC 게이트는 `IsIncapacitated()`** — 사용 시작/뗌·던지기 차징/놓기·놓기(G)·먹기(`HostEat`)·**줍기(E)·
    칸 전환(숫자·휠)·헤드램프(F)** 전부. 뒤의 셋은 이전에 넘어짐 검사가 없었으므로 **넘어진 동안도 새로 막힌다**(D2).
  - **`HostKill(Source)`**: 넘어짐 무적 게이트를 거치지 않고 남은 HP 전부를 `HostApplyDamage`(= `OnDamage` 의 게이트
    뒤 공통부: HP 차감 → 짐 흘림 → 넘어짐 ∞ → 피격 사건 → 로그 `executed by` → `HandlePlayerDeath`)로 넣는다.
    `OnDamage` 는 같은 공통부를 `caught by` 로 부른다. 이미 죽었으면 아무 일도 없다.
  - **카메라·HUD 게이트** (D6): `GetHurtFlash()` → 0, `HasHurtDirection()` → false, 조준 대상·프롬프트 없음(`Tick`),
    스태미나 셰이크·`PlayShakeOnce` 재생 안 함, `UCL9HurtLookModifier::ModifyCamera` 는 아무것도 더하지 않는다.
    HUD 파일은 무변경 — 게터로만 게이트한다.
  - **`HostEndExecuted()`**: 두 필드를 비우고 `ApplyExecuted()`; **관전 중이 아니면** 뷰 타깃을 자기 폰으로(관전 중이면
    관전 흐름의 것 — 로비 부활이 되돌린다). `ClearNightPopulation` 이 몬스터를 파괴하기 **전에** 전 캐릭터에,
    `ReviveDead` 도 전 캐릭터에(D9). 처형과 무관하면 아무 일도 없다. 죽어서 데스캠이 켜져 있으면 뷰 타깃은 이미 자기 폰이라
    아무것도 바뀌지 않고, 몸·팔도 데스캠 쪽 토글이 유지한다.
  - 죽음 뒤 흐름은 보통 죽음과 같다 — `HostKill` → `HostApplyDamage` 가 HP 0 이 되는 순간 **데스캠**(자기 폰)으로
    `SetViewTargetWithBlend(폰, DeathCameraBlend)` → `HandlePlayerDeath` → `SpectateDelay` → `BeginSpectating` 이 뷰 타깃을 덮어쓴다.
    처형한 몬스터는 그때까지 시체 위에 머문다(데스캠에 함께 잡힌다). 옛 D6 문장 "뷰 타깃은 죽은 자기 폰으로 돌아오지 않는다
    (넘어짐 카메라가 이제 보이는 몸 속에 앉는다)"는 2026-09-20 밤 데스캠으로 번복 — 데스캠은 넘어짐 카메라와 별개 컴포넌트다
    (위 데스캠 항목).
  - ABP(리드): `ABP_Unarmed` Executed 가지 — 넘어짐 가지보다 출력에 가까운 자리. **Run 2**(2026-09-21): 조건 `IsExecuted` 단독(구간 검사
    없음 — 죽은 뒤 부활까지 마지막 프레임 유지), 포즈 = `GetExecutedByAlias()` 로 고른 종별 2인 클립 `AN_Farmer_Executed_Pig`/
    `AN_Farmer_Executed_Raccoon` 평가기(`ExplicitTime = GetExecuteElapsed`, 길이 = `GetExecuteDuration`). Run 1 은 조건
    `IsExecuted ∧ InRange(GetExecuteElapsed, 0, GetExecuteDuration)` + Idle 평가기 0 프레임 고정 → 킬 뒤 넘어짐 사슬이었다.
  - 치트 `cl9.execute` ([cheats.md](cheats.md)).

### 잡히는 순간 카메라가 범인을 본다 — 피격 사건 + 카메라 모디파이어 (2026-09-20)

"엥 뭐지, 왜 맞았지"를 없애는 층이다 ([../goal/monster-pounce-hurt-readability.md](../goal/monster-pounce-hurt-readability.md) B).
판정·조작은 하나도 건드리지 않는다.

- **피격 사건** `FCL9HurtEvent { float ServerTime; FVector_NetQuantize From; TObjectPtr<AActor> Attacker; uint16 Seq; }` —
  `ACL9Character::HurtEvent`, `COND_OwnerOnly` 복제(잡힌 본인만 쓴다). `OnDamage`가 넘어짐 시각과 함께 채운다:
  `ServerTime = KnockdownStartServerTime`, `From = Source ? Source 위치 : 자기 위치`, `Attacker = Source`, `Seq++`.
  방향의 원천은 **위치**(`From`)라 공격자가 파괴·이동해도 그 순간의 방향이 남고, `ServerTime == KnockdownStartServerTime`
  (`IsHurtEventCurrent()`)이 "이번 넘어짐의 사건"을 보증한다 — 옛 사건이 새 넘어짐에 잔존하지 않는다. `Attacker`는
  `IsValid()`로만 읽는다(파괴 직후 pending-kill).
- **게터** (HUD·ABP 용): `GetHurtEvent()` · `HasHurtDirection()`(이번 넘어짐의 사건 ∧ `IsValid(Attacker)` ∧ 관전 중 아님 —
  치트 null·관전이면 false) · `GetHurtDirectionAngle()`(도, `FRotator::NormalizeAxis(공격자 방향 요 − 카메라 요)`, 0 =
  화면 위, 시계 방향 양수, 없으면 0; 카메라 요는 카메라 매니저의 실제 POV 요) · 기존 `GetHurtFlash()`. 방향 호를 그릴지는
  `CL9Rules.bHurtDirectionIndicator`(기본 true — 장르 톤이 안 맞으면 빌드 없이 끈다, [ui.md](ui.md)).
- **카메라 모디파이어** `UCL9HurtLookModifier : UCameraModifier` (`Source/CL9/GameMode/`): 소유 클라(리슨 호스트의 자기 폰
  포함)에서 `NotifyControllerChanged`(PossessedBy·UnPossessed·클라 `OnRep_Controller`)가 `PlayerCameraManager->
  AddNewCameraModifier`로 등록하고, 소유가 풀리거나 매니저가 바뀌거나 `EndPlay`면 뗀다. `ModifyCamera`가
  `GetKnockdownElapsed() ∈ [0, KnockdownTime)` 동안 **POV에만** 두 오프셋을 더한다:
  - 피치: 목표 `CL9Rules.HurtLookPitch`(−25°)까지 `HurtLookTime`(0.45초) 동안 EaseOut — **도달 보장**(`FInterpTo` 아님).
    넘어짐 시계만으로 시작하므로 사건 복제가 늦어도 피치는 먼저 간다.
  - 요: 사건의 `From` 방향으로, **공격자가 화면 가장자리에 걸칠 때까지만**(|Δ요| − 가로 반각 FOV), `±HurtLookYawMax`(60°)
    안에서. 사건 순간의 컨트롤 요 기준으로 목표를 한 번 재고 같은 EaseOut으로 간다 — 그 뒤 마우스를 돌려도 오프셋은
    상수라 시선이 밀리지 않는다. 가로 FOV 90° + 60° = 시선에서 105° 안의 공격자만 화면에 걸친다; 그 밖(등 뒤)은
    방향 호가 답한다.
  - 일어나기 구간(넘어짐 끝 `CL9HurtLook::GetUpSeconds` 0.6초 = `AN_Farmer_GetUp`)에 둘 다 EaseOut으로 0.
  - 재피격(`Seq` 변화)은 요 목표만 지금 값에서 새 목표로 갱신, 피치 진입은 다시 시작하지 않는다(덜컹 방지 — 넘어짐 무적
    뒤로는 같은 넘어짐 안의 재피격이 없지만 규칙은 남긴다). 죽음은 데스캠이 켜지면(`ShouldUseDeathCamera()`) 아무것도 더하지
    않는다 — 오프셋 0 롤백이면 옛대로 넘어짐 카메라에 진입 뒤 유지. 관전(카메라 매니저의 뷰 타깃이 남의 폰)이면 아무것도
    더하지 않는다.
  - **컨트롤 회전·`LookInput`·조준(`GetControlRotation`)은 그대로다** — 스태미나 셰이크와 같은 "POV만" 원칙. 1인칭 팔은
    카메라 **컴포넌트**의 자식이라 POV 오프셋을 따라가지 않는다(넘어짐 0~0.5초의 손이 25° 위에 보인다 — PIE 육안 항목).
- **값**: `CL9Rules` `HurtLookTime`(0.45, **0 = 끔**)·`HurtLookPitch`(−25)·`HurtLookYawMax`(60)·`bHurtDirectionIndicator`(true)
  — balance 초안. **로드 검사** `HurtLookTime + 0.6 <= CharacterDef.KnockdownTime`(1.5): 규칙 표와 캐릭터 행이 둘 다 보이는
  `ApplyCharacterDef`에서 Warning — 어기면 진입·복귀 보간이 겹쳐 카메라가 떤다.

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
  보지 않고 행의 `Scale` 세기로 한 번 재생한다(2026-09-18 — 옛 세기 1, 위 "흔들림" 절). 행이 없으면 조용히 지나간다.
- **셰이크 에셋 값** (정본은 `Tools/Unreal/create_hit_feedback_assets.py`의 `HIT_SHAKE`·`HURT_SHAKE` — 에디터에서 손으로
  만진 값은 재실행이 덮어쓴다. 2026-09-15 결정대로 실측값도 여기 적는다):

  | 에셋 | 패턴 | 길이 / BlendIn / BlendOut | 회전 | FOV | 위치 | 실측 |
  |---|---|---|---|---|---|---|
  | `CS_HitShake` | **사인파 오실레이터**(`WaveOscillatorCameraShakePattern`, 모든 축 InitialOffset Zero — 2026-09-18) | 0.12 / 0 / 0.04초 | Pitch −3.0°@4Hz, Yaw·Roll 0 | −3.0°@4Hz | 0 | 펄린이던 2026-09-15: 0.66°(진폭 2.2°의 0.30). 사인파 실측은 PIE 뒤 채운다(기대 30fps ≥2.2°, 60fps ≈3.0°) |
  | `CS_HurtShake` | 펄린 노이즈 (그대로) | 0.50 / 0 / 0.30초 | Pitch 2.5°@7Hz, Yaw 1.5°@6Hz, Roll 3.5°@5Hz | 0 | 0 | 0.98°(2026-09-15) |

  타격만 펄린에서 사인파로 바꾼 이유(balance 2026-09-18): 펄린은 임의 위상에서 시작해 순간값이 진폭에 못 미치고 블렌드
  아웃이 길이의 2/3를 깎아, 진폭을 키워도 "정점 ≥2°"가 매번 나오지 않으면서 드물게 조준을 잃을 만큼 튄다. 사인파는
  0 위상에서 시작해 정점 = 진폭 × 블렌드 포락선으로 결정적이다. 부호 −는 먼저 아래로 꺾인다는 방향일 뿐이고, FOV −3°는
  반주기 동안 화각이 좁아졌다 돌아오는 펀치 인이다(팔 메시는 자체 FOV 70이라 따라 줄지 않는다 — PIE에서 어색하면 FOV만 0).
- **빨간 오버레이는 `HitFlashMaterial` 하나를 사람과 몬스터가 같이 쓴다** (`SetOverlayMaterial` —
  줍기 하이라이트와 같은 기제). 캐릭터 몸에는 하이라이트를 걸지 않지만(그쪽은 아이템 메시다),
  **내가 켠 것만 끈다**는 가드를 둔다 — 켜고 끌 때만 세팅하고, 켠 적이 없으면 지우지 않는다.
- **표현 시계는 `GetKnockdownElapsed()`가 아니다.** 그 게터는 일어나는 순간 −1로 돌아가서(애님 분기용)
  플래시가 영원히 켜진 것으로 읽힌다. 플래시·비네트는 `GetHurtElapsed()`(= 서버 시각 − 넘어진 시각,
  일어난 뒤에도 계속 커진다)를 쓴다. 넘어진 적이 없으면 −1000 기본값 덕에 아주 큰 값이 되어 꺼져 있다.
- HUD 비네트 이미지는 `GetHurtFlash()`를 불투명도로 읽는다 ([ui.md](ui.md)).

### 소리·팔 히트스톱·발소리 (2026-09-18 구현)

2026-09-15 표 위에 얹은 층이다 ([../goal/game-feel.md](../goal/game-feel.md) §1·§2). 새로 복제하는 값은 없고, 전부
이미 복제된 시계에서 파생한다. 몬스터 쪽(`Hit`·`MonsterDeath` 소리·몬스터 히트스톱·퍼프·죽음 발사)은
[monster.md](monster.md) "타격의 표현", 표·에셋·재생 헬퍼는 [sound.md](sound.md).

| 무엇 | 누가 보나 | 원천 | 어디서 |
|---|---|---|---|
| `Swing` 소리 (폰 자리, 맞든 빗나가든) | 전원 | `SwingStartServerTime` 변화 | `UpdateHitSounds` |
| `HitSurface` 소리 (폰 자리 — 벽·바닥을 쳤다, 2026-09-22) | 전원 | `SwingSurfaceHitServerTime` 변화 (Replicated, 조건 없음) | `UpdateHitSounds` |
| `Hurt` 소리 (폰 자리) | 전원 | `KnockdownStartServerTime` 변화 | `UpdateHitSounds` |
| 1인칭 팔 히트스톱 | 때린 본인 | `SwingHitServerTime` 뒤 `CL9Rules.HitStopTime`(0.08초) | `GetSwingAnimElapsed()` → ABP `SwingElapsed` |
| 발소리 `Footstep` (발밑) | 전원 | 수평 이동 거리 누적 ≥ 보폭 | `TickFootstepSound` |

- **소리도 셰이크와 같은 틱 비교다** — `UpdateHitSounds()`가 지난 틱과 값이 다르면 `CL9Sound::PlayAt`으로 한 번 튼다.
  다만 셰이크의 `UpdateHitShakes`는 소유자 전용인데 소리는 **남의 폰에서도 들려야** 하므로 로컬 조작 게이트 **앞**에서
  모든 머신이 돌고, 비교값도 따로 둔다(`LastSeenSwingStartTime`·`LastSeenHurtSoundTime`). 접속 직후 첫 틱은 기준값만
  받아 적는다(`bHitSoundsPrimed`). `Hurt`는 넘어지는 순간 한 번이고 일어날 때는 없다.
- **팔 히트스톱은 판정 시계가 아니라 ABP 전용 게터다.** `GetSwingAnimElapsed()`(BlueprintPure)는 이번 휘두르기가 맞은 뒤
  (`SwingHitServerTime >= SwingStartServerTime`, `COND_OwnerOnly`라 때린 본인 화면에서만) `HitStopTime` 동안 명중 시각의
  경과값에 머물고, 지나면 실제 경과에서 정지 길이를 뺀 값으로 이어진다 — 팔 클립이 명중 프레임에 0.08초 멈췄다가 이어서
  끝까지 돈다. 남의 화면·헛스윙·표 없음이면 `GetSwingElapsed()`와 같다. **`ABP_FP_View`·`ABP_Unarmed`의 `SwingElapsed`
  변수 소스가 `GetSwingElapsed` → `GetSwingAnimElapsed`로 바뀐다** — 분기 조건 `SwingElapsed ∈ [0, 0.347)`도 같은 변수를
  보므로 클립이 0.08초 늦게 끝까지 간다([animation.md](animation.md) 휘두르기 가지). 판정 `HostTickSwingHit`은 여전히
  `GetSwingElapsed()`를 쓴다 — 히트박스 창은 그대로다. 전역 시간 지연·`CustomTimeDilation`은 쓰지 않는다(리슨 호스트에서
  남에게 번지고 이동 예측을 깨뜨린다).
- **발소리는 애님 노티파이가 아니라 거리 누적기다** (클립에 노티파이가 없다). `TickFootstepSound()`가 모든 머신에서 매 틱
  `GetFootLocation()`의 수평 이동 거리를 누적(`FootstepDistanceAccum`)해 보폭에 닿을 때마다 발밑에서 `Footstep` 행을 피치
  ±8 %로 튼다. `IsMovingOnGround()`일 때만 — 서 있거나 공중이거나 넘어졌거나 죽었으면 안 난다. 보폭은 수평 속도가 행의
  `WalkSpeed`를 넘으면 `CL9Rules.FootstepStrideRun`(196cm = 654 × 0.3초, `AN_Farmer_FPRun` 0.6초 루프의 반 — 주먹이 앞
  극점에 올 때마다 한 발), 아니면 `FootstepStrideWalk`(150cm, 2.64 걸음/초) — 달리기 입력 `bWantsToRun`은 남의 폰에
  복제되지 않아 속도로 가른다. 보폭 종류가 바뀌면 누적을 0으로 되돌린다. 무게로 느려지면 간격이 같이 벌어진다.
  소음 판정의 `TickStepNoise`(파동, 호스트·소유 클라만, 표의 간격)와는 **별개 누적**이다 — 발소리는 연출이지 몬스터
  청각이 아니다.

## 네트워크 정리

| 상태 | 위치 | 이유 |
|---|---|---|
| 달리기 플래그·스태미나·유효 무게 속도 | **CMC (예측 경로)** | FSavedMove로 서버·클라 동기 |
| HP(`Hp`·`MaxHp`)·퀵슬롯 목록(`Slots`·`ActiveSlot`)·머리 장비(`HeadItem`) | PlayerState (Replicated + OnRep) | 판정·UI·타 클라 표시 |
| 넘어짐 시계 `KnockdownStartServerTime`/`KnockdownEndServerTime` · 처형 `ExecuteStartServerTime`/`ExecutedBy` · 소지 실물 `HeldItem`/`WornItem`/`PullingCart` · 휘두르기·먹기·던지기 시각 | **Character** (Replicated) | 표현 파생용 서버 시각과 실물 참조 — conventions § 4 의 명시된 예외 (decisions.md 2026-09-13·09-15·09-20). PlayerState 는 미러만 |
| 위치·회전 | ACharacter 표준 (네트워크 스무딩 내장) | |
| 잡힘·피격 판정 | 호스트 전용 (몬스터 → 캐릭터 함수 직접 호출) | |
| 타격 킥 시각 | `SwingHitServerTime` (Replicated, `COND_OwnerOnly`) | 때린 본인만 느끼는 연출 |
| 표면 타격 시각 | `SwingSurfaceHitServerTime` (Replicated, 조건 없음) | 벽·바닥을 친 소리는 남의 폰에서도 들려야 한다 (2026-09-22) |
| 회복·예약 `SwingReadyTime`·`bSwingBuffered` | 호스트 전용, 비복제 | 판정 상태 — 표현은 복제된 시작 시각에서 나온다 |
| 빨간 플래시·비네트·셰이크 | 복제 없음 — 위 시각들에서 각 머신이 파생 | 표현은 시계에서 나온다 |
| 피격 사건 `HurtEvent` | Replicated `COND_OwnerOnly` | 카메라 모디파이어·방향 호 — 잡힌 본인만 |
| 처형 `ExecuteStartServerTime`(OnRep)·`ExecutedBy` | Replicated (모두) | 남의 ABP·CMC 속도 0·처형 카메라 게이트. 뷰 타깃은 서버 `SetViewTarget`(관전 경로) |
| 데스캠 | 복제 없음 — 각 머신이 PlayerState HP(`IsDead`)에서 파생(호스트 자기 폰은 죽음·부활 자리, 소유 클라는 `Tick` 비교). 뷰 타깃은 서버 `SetViewTargetWithBlend(폰)` | 표현은 상태에서 나온다 |

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

### 판정 — 컨트롤 회전 + 손 높이 원점 (2026-09-22, [../goal/combat-feel-pass-1.md](../goal/combat-feel-pass-1.md) D-1~D-4)

2026-09-15의 "히트박스 = 손에 든 메시"와 2026-09-20의 `SwingHitScale`은 폐기됐다(decisions.md). 호스트 `HostTickSwingHit`이
히트 구간이 열린 틱마다:

- **원점** = `GetActorLocation() + Z·(EyeHeight − CapsuleHalfHeight − 66)` ≈ 손 높이 90cm(`SwingHandDrop` 66은 농부 A 리그의
  물리 치수라 코드 상수). **방향** = `GetControlRotation().Vector()` — 던지기와 같은 방식. 카메라 컴포넌트 트랜스폼은 읽지 않는다:
  `bUsePawnControlRotation`의 피치는 로컬 `GetCameraView` 안에서만 적용되어 리슨 호스트에서 클라 소유 폰의 카메라는 피치 0이다.
- **모양** = `Origin + Dir·SwingHitRadius → Origin + Dir·SwingHitReach` 선분을 반경 `SwingHitRadius` 구로 스윕. 선분이 반경
  하나 앞에서 시작해 구가 등 뒤로 나가지 않는다. 채널·응답은 그대로(WorldDynamic 채널에 Overlap 응답 — 닿은 전부가 돌아오고
  벽·폰은 결과에 없다; 자신·든 아이템 무시). 최대 닿는 거리 = Reach + Radius + 몬스터 반경.
- **히트 구간** `[SwingHitStart, SwingHitEnd)`는 그대로 "판정이 열린 시간"이다(손이 지나는 자리가 아니다). 틱이 덮는 구간
  `(Prev, Now]`와의 겹침 판정(2026-09-15 낮은 프레임률 안전)도 그대로. 매 틱 그 순간의 컨트롤 회전으로 선분을 다시 잡으므로
  구간 동안 시선을 돌리면 따라간다. 몬스터당 한 휘두르기 1회(`SwingHitMonsters`).
- **가림**: 스윕에 걸린 살아 있는 몬스터마다 원점 → 몬스터 구 **중심** `ECC_Visibility` 선 트레이스(`ACL9Monster::TraceWorld`).
  세상 지형이 먼저 막으면 제외. 폰·아이템·다른 몬스터는 가리지 않는다(사람은 가리지 않는다). 열 없음, 항상 켜짐.
- **사람도 맞는다 — 팀킬 (2026-10-12, 7d — [../goal/loop-7d-team-kill.md](../goal/loop-7d-team-kill.md), 2026-10-11 사용자 결정 "몬스터 타격 도구가 팀도 때린다")**:
  위 몬스터 스윕은 그대로 두고, 같은 틱에 **같은 구로 `ECC_Pawn` 채널을 한 번 더 겹침 스윕**해 나 아닌 살아 있는 `ACL9Character` 를 고른다.
  규칙은 몬스터와 같다 — 한 휘두르기에 한 사람 한 번(`SwingHitPlayers`), 원점 → 그 사람 위치 `TraceWorld` 가림(막히면 `occluded`), 첫 명중이면 카메라 킥·히트스톱,
  `cl9.swinglog` 의 `tested`·`occluded` 에 함께 센다. 피해는 **든 아이템의 `HitDamage`**(프라이팬 2·고기 1, 새 칸 없음)를 사람이 받는 단일 문
  `OnDamage(HitDamage, 공격자, HitKnockback)` 으로 넣는다 — 넘어짐 무적·인트로 무적·죽음의 흘림·데스캠·피격 사건(맞은 사람 카메라가 공격자를 본다)이 다 그대로다.
  HP 3 이라 프라이팬은 한 대에 넘어뜨리고(3 → 1) 일어난 뒤 한 대에 죽인다. 넘어진 동안의 타격은 무시된다(맞힌 쪽 킥은 난다).
  로그 동사는 사람이 때리면 `struck by`(몬스터는 `caught by`).
- **표면음**: 같은 틱, 원점 → `Origin + Dir·SwingHitReach` **선** 트레이스(구가 아니다 — 손 높이에서 아래를 보면 구는 바닥에
  먼저 닿는다). 세상 지형에 막히고 그보다 가까운 몬스터 명중이 이번 휘두르기에 없으면 휘두르기당 한 번
  `SwingSurfaceHitServerTime`(Replicated, **조건 없음**)을 찍는다. 모든 머신이 `UpdateHitSounds`의 틱 비교로 `HitSurface` 행을
  **폰 자리**에서 튼다(충돌점은 복제하지 않는다). 카메라 킥·팔 히트스톱·소음 사건은 없다.
- 규칙 열: `SwingHitReach`(cm, 헤더 0)·`SwingHitRadius`(cm, 헤더 0) — 둘 다 **헤더 0 = 미설정 = 로드 오류**, 옛 경로 폴백 없음.
  `Validate`: 유한·> 0·`Reach >= Radius`. 값은 [../plan/data/rules.md](../plan/data/rules.md).
- 휘두르기당 트레이스 수 = 스윕 1 + 걸린 몬스터 수 + 표면 선 1 ([performance.md](performance.md)). 2026-10-12 팀킬부터 틱마다 Pawn 스윕 1 + 걸린 사람 수가 더해진다.
- **사망 원인 — `ACL9PlayerState::KilledByPlayer`** (호스트 전용, 복제 없음, 2026-10-12 7d): 죽는 타격(`HostApplyDamage` 에서 HP 0)의 출처가 플레이어 캐릭터면 그 사람의
  PlayerState, 몬스터·남겨짐·치트면 null. 로그 `'<이름>' was killed by player '<이름>'.` `HostRevive` 가 지운다. 임포스터의 "직접 처치 = 실패" 판정(7g)이 읽는다.
- 권한 전신 메시의 `AlwaysTickPoseAndRefreshBones`(`BeginPlay`)는 남는다 — 판정이 아니라 놓기(`Server_Drop`이 손 소켓에 붙은
  `HeldItem->GetActorLocation()`을 읽는다, [inventory.md](inventory.md) 놓기)가 쓴다.

### 회복·입력 예약 (2026-09-22, D-6·D-7)

- **회복** `CL9Rules.SwingRecovery`(초, 헤더 **0.6** = 옛 상수 동작): `HostSwing`이 `SwingReadyTime = 지금 + SwingRecovery`를
  찍고 그 전의 뗌은 거부한다. `Validate`: `SwingRecovery >= SwingClipWindow(0.347)` ∧ `SwingHitEnd < SwingRecovery`. 한 휘두르기의
  **첫 명중**을 찍는 틱에 `SwingReadyTime += HitStopTime` — 팔 히트스톱만큼 늦춰 맞든 빗나가든 팔이 돌아온 뒤 같은 여유가 남고,
  다음 `SwingStartServerTime` 리셋이 멈춘 팔 애님 도중에 떨어지지 않는다.
- **예약** `CL9Rules.SwingBufferTime`(초, 헤더 **0** = 끔 = 옛 동작): `Server_UseRelease`가 회복 중(`Now < SwingReadyTime`)이고
  `SwingReadyTime − Now <= SwingBufferTime`이면 `bSwingBuffered = true`(호스트 전용, 비복제, 하나뿐·덮어쓰기). 회복이 끝나는
  호스트 틱에 `HostTickSwingBuffer`가 `CanSwingNow()`(손에 `Swing` 행 ∧ `!IsIncapacitated()`)를 다시 거쳐 `HostSwing`. 취소 분기는
  따로 없다 — `HostCancelSwingWindup()`이 대기와 함께 예약도 지우고, 칸 전환·놓기·던지기·먹기·넘어짐·사망이 이미 그것을 부른다.
  처형 진입은 부르지 않지만 발화 틱의 `CanSwingNow()`가 막는다(예약은 그냥 사라진다).
- 측정 치트 `cl9.swingtaps`·`cl9.swinglog` ([cheats.md](cheats.md)).
