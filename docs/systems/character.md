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

## 카메라 손맛 — 보브·롤·질주 FOV·착지 스프링 (2026-10-05)

[../goal/control-feel.md](../goal/control-feel.md) C8~C12. **새 카메라 모디파이어는 없다.** 소유 머신의 `ACL9Character::Tick`(로컬 조작
게이트 뒤)에서 `UpdateViewFeel(Dt)` 하나가 옛 넘어짐 카메라 하강 블록을 대신해 아래 순서로 합친다. 값은 `CL9Rules.ViewFeel`
(`FCL9ViewFeel`, [../plan/data/rules.md](../plan/data/rules.md) ViewFeel), 순수 수학은 `Source/CL9/Character/CL9FeelMath.h`.

1. **바탕 높이 `CameraBaseZ`** = FInterpTo(바탕, 넘어짐·죽음 ? 바닥 + 30 : 눈높이, 6/s). **컴포넌트에서 되읽지 않는다** — 옛 블록은
   컴포넌트 Z 를 읽어 보간했는데, 거기에 손맛 오프셋을 더하면 서로 싸운다. `ApplyCharacterDef`가 행의 눈높이로 심는다. 데스캠 동안은
   얼려 두고(옛 블록과 같다 — 부활하면 죽은 순간의 높이에서 올라온다), 1인칭 카메라의 롤·FOV 가산 오프셋만 지운다(부활 틱에 죽기 전 값이
   한 프레임 남지 않게, 2026-10-05 리뷰 CAM-2).
2. **`FeelWeight`** = FInterpTo(가중치, 넘어짐·죽음·처형·데스캠 ? 0 : 1, 10/s) — 아래 손맛 전부에 곱한다.
3. **세로**: 보브 + 착지/점프 스프링을 [−22, +5]cm 로 자른 뒤 CameraMotion × FeelWeight 를 곱해 카메라 **컴포넌트**의 상대 Z =
   바탕 + 그 값. 팔·든 물건·헤드램프가 카메라의 자식이라 함께 움직인다(POV 만 옮기면 팔이 화면에서 미끄러진다).
4. **롤과 FOV**: 옆걸음 롤 + 걸음 롤을 ±2.5° 로 자르고, 질주 FOV 킥(≤ 8°, ≤ 20°/s)과 함께 CameraMotion × FeelWeight 를 곱해
   `ClearAdditiveOffset` → `AddAdditiveOffset(롤만 있는 변환, FOV)`. 가산 오프셋은 `GetCameraView`에서만 얹혀 이 카메라에만
   걸린다 — 데스캠(다른 컴포넌트)·처형 카메라(몬스터)·관전(남의 폰은 이 함수가 안 돈다)에 게이트 코드가 필요 없다. 1인칭 팔 FOV 70 은 그대로.
5. 그 뒤는 그대로: 포커스·프롬프트, `UpdateStaminaShake`(세기 × CameraMotion), `UpdateHitShakes`, 피격 시선 모디파이어와 셰이크는
   `GetCameraView` 뒤에 돈다.

- **이동 손맛은 시선의 피치·요를 절대 건드리지 않는다** (C9) — 세로 이동·롤·FOV 만. 롤은 카메라 전방 축 회전이라 화면 중앙(조준선)이
  컨트롤 회전 그대로다. 피치·요 오프셋은 전투 피드백(타격 킥·피격 흔들림·피격 시선)만의 언어다. 보브 요·착지 끄덕임·회전 기울임·
  가로 보브·숨쉬기는 없다.
- **보브는 발소리와 한 시계다** — 아래 "발소리"의 걸음 위상(`StepPhase`)을 읽는다. 모양 = Lerp(0.5 − 0.5cos(2π·위상), |sin(π·위상)|, 날카로움),
  세로 = 진폭(위아래 폭) × (모양 − 0.6) 이라 위상 0(= 발 디딤 = 발소리)이 골이다. 진폭은 정지 0 → 걷기 2.8cm → 달리기 6.8cm(속도 선형,
  공중 0), 10/s 로 따라간다. 걸음 롤 ±0.10°(걷기)/±0.30°(달리기)는 걸음마다 좌우가 바뀐다.
- **옆걸음 롤** = 1.5° × clamp(속도·오른쪽 / 걷기 속도, −1, 1) — 오른쪽으로 가면 양수, 12/s 로 따라간다(처음 8/s 는 "0.35초 안 90 %"
  경계에 걸려 2026-10-05 리뷰에서 올렸다 — 목표 자체가 0.2초 걸리는 속도를 따르기 때문).
- **질주 FOV 킥** = 6° × (수평 속도 − 걷기)/(달리기 − 걷기) — 질주 키가 아니라 실제 속도를 따라 벽에 대고 달리면 0. 올라갈 때 6/s,
  내려갈 때 4/s 지수 추종에 변화율 상한. 기본 FOV 는 `CharacterTable.CameraFov`(98, 2026-10-20 플레이테스트 9번 — 리썰 세로 66° 를 16:9 가로로 옮긴 값, 그 전 엔진 기본 90)이고 킥은 그 위에 더한다.
- **착지·점프 스프링**: 감쇠 스프링 하나(ω 16, ζ 0.6)를 닫힌 꼴로 정확히 적분한다(오일러는 30fps 에서 딥이 반으로 준다). 충격 =
  딥 / `SpringGain(ω, ζ)`(0.03118, 코드가 계산). 착지 딥 = min(18, 8 × (|Vz| / 점프 초속)^1.6)cm, |Vz| < 350 이면 없음 — 평지 점프 8cm,
  1000 cm/s 11.9cm. 점프 순간 2.5cm. 충격은 소유 머신의 `Landed`·`OnJumped_Implementation`이 주고, 보정 리플레이(`bClientUpdating`)는
  건너뛰며 같은 사건은 0.15초에 한 번이다.
- **`화면 움직임`(`UCL9GameUserSettings::CameraMotion`, 0..1, 기본 1)**: 보브·롤·질주 FOV·스프링·스태미나 흔들림에 곱한다. 타격 킥·
  피격 흔들림·피격 시선·넘어짐 하강·데스캠/처형 카메라·소리에는 곱하지 않는다. 0 이면 시점이 고정된다 — 카메라 회귀 확인용이기도 하다.
  슬라이더는 설정 화면([settings.md](settings.md)).
- **남의 폰에는 이 함수가 안 돈다** — 호스트가 보는 클라이언트 폰의 카메라 상대 Z 는 눈높이 그대로다. QA 표본은
  `UCL9FeelDebug::GetFeelSample`(화면에 실제로 들어간 값 — CameraMotion·FeelWeight 를 곱한 뒤). QA 가 화면 움직임을 바꿀 때는
  `UCL9FeelDebug::SetCameraMotion`(메모리 값만, 저장 안 함)을 쓴다 — `CameraMotion` 필드도 `EditAnywhere, BlueprintReadWrite`라 파이썬에서 읽고 쓸 수 있다.
- **팔(뷰모델) 층** (C16, `UpdateViewFeel` 끝의 `UpdateViewmodel`): 팔 메시는 카메라의 자식이라 위의 세로 이동은 그대로 따라오고, 그 위에
  팔만의 작은 어긋남을 상대 변환으로 얹는다. ① 시선 뒤처짐 — 매 틱 컨트롤 회전량의 `ArmsLagGain`(0.5) 몫만큼 반대로 남고 `ArmsLagDecay`
  (15/s)로 돌아온다, 상한 요 4°·피치 3°, 눈(카메라 원점) 기준 회전. 컨트롤 회전은 읽기만 한다. ② 세로 — 보브 × `ArmsBobScale`(−0.25,
  팔이 보브의 75 % 만 출렁) + 느린 팔 스프링(ω 12·ζ 0.5, 카메라 착지·점프 충격의 `ArmsImpulseScale` 0.2 몫 — 평지 착지에 약 2.3cm,
  손이 시선보다 조금 늦게 내려간다), 상한 3.5cm. ①②에 CameraMotion × FeelWeight 를 곱한다. ③ 칸 전환·줍기 딥 — 소유자의 `Pickup`·
  `SlotSwitch` 소리와 같은 에지에서 6cm 아래로 0.06초, 0.12초에 돌아온다. **CameraMotion 을 곱하지 않는다**(정보다). 바탕 변환은
  `BeginPlay`가 잡은 회전과 `ApplyCharacterDef`의 위치(−눈높이). 값은 `CL9Rules.ViewFeel.Arms*`·`EquipDip*`.

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
4. GetMaxSpeed() override: Lerp(Walk', Run', SprintRamp) — Walk'/Run' 은 각자의 먹기 보너스를 얹은 값,
   그다음 수레 배율·넘어짐/처형 0 (2026-10-05 — 아래 "이동 손맛")
   → 서버·자율 프록시가 같은 함수로 같은 결과 — 예측이 어긋나지 않는다
```

**저장무브를 합칠 때 시간 적분 상태를 되돌린다** (2026-10-05, [../goal/control-feel.md](../goal/control-feel.md) C2).
엔진은 클라이언트 프레임이 송신 간격보다 짧으면 대기 무브와 새 무브를 합쳐(`ReplicateMoveToServer` → `CombineWith`) 위치·속도·
점프 카운터만 대기 무브 시작 값으로 되돌리고 합친 시간을 **다시** 시뮬레이션한다. `SetMoveFor`는 대기 프레임이 이미 한 번
적분된 뒤의 값을 찍으므로, 되돌리지 않으면 144fps 클라이언트의 스태미나가 호스트보다 두 배 빨리 줄었다(코드 읽기로 확인, 미측정).
`FSavedMove_CL9::CombineWith`가 Super 뒤에 `Stamina`·`bExhausted`·`SprintRamp`를 대기 무브의 스냅샷(`Saved*`)으로 되돌리고
같은 값을 새 무브의 스냅샷에도 넣는다 — 엔진이 점프 카운터를 되돌리는 것과 같은 꼴. 시간 적분 예측 상태를 새로 더하면 여기에도
더한다. **참여 클라이언트에서의 검증(보정 수)은 2인 세션 전까지 미확인이다.**

**보정 리플레이 뒤 달리기 입력을 되돌린다** (2026-10-05 리뷰 MP-2, 이전부터 있던 결함). 서버 보정이 오면 엔진은 미확인 저장 무브를 다시
돌리는데(`ClientUpdatePositionAfterServerUpdate`), 무브마다 `PrepMoveFor`·`UpdateFromCompressedFlags`가 `bWantsToRun`을 그 무브의 값으로
덮는다. 이 플래그는 입력 이벤트만 쓰므로 그대로 두면 보정과 같은 프레임의 달리기 누름·뗌이 사라지고 다음 키 이벤트까지 틀린 상태로
남는다. `UCL9CharacterMovement::ClientUpdatePositionAfterServerUpdate` 오버라이드가 리플레이 전 값을 들고 있다가 끝나면 되돌린다 — 엔진이
`bPressedJump`·`bWantsToCrouch`를 보존하는 것과 같은 꼴. 스태미나·탈진·램프·코요테는 무브 스냅샷에서 다시 도출되므로 되돌리지 않는다.

## 이동 손맛 — 가속·제동·질주 램프·공중 제어 (2026-10-05)

[../goal/control-feel.md](../goal/control-feel.md) C1·C3~C7. 최고 속도(396/654)·스태미나(6/8/1.0)·점프 높이(156cm)는 그대로 두고
속도가 변하는 **곡선**만 정한다. 값은 전부 `UCL9CharacterDef`의 `Movement` 열이고 헤더 기본값이 의도한 값이다(기존 행에 셀이 없어도
이 값으로 돈다) — 열 설명은 [../plan/data/character.md](../plan/data/character.md).

| 무엇 | 어떻게 | 열 (기본값) |
|---|---|---|
| 지상 가속 | `ApplyCharacterDef`가 CMC `MaxAcceleration`에 넣는 상수. `GetMaxAcceleration`은 덮지 않는다 — 엔진이 입력 배율(`AnalogInputModifier`)을 이 값으로 나누므로 상태에 따라 바뀌면 합친 무브에서 플레이어가 브레이크를 밟는다 | `Accel` (2000) |
| 제동 | 엔진 쌍 하나: `bUseSeparateBrakingFriction = true`, `BrakingFrictionFactor = 1`, `BrakingFriction`, `BrakingDecelerationWalking`. 걷기 정지 0.20초 ≈ 32cm, 질주 정지 0.28초 ≈ 71cm(파이썬 복제, 30~240Hz). 넘어짐·처형·수레·버프 만료처럼 속도가 상한을 넘는 모든 경우도 같은 제동이다 | `BrakingFriction` (3.5) · `BrakingDecel` (1300) |
| 공중 제동 가드 | `ApplyVelocityBraking` 오버라이드 하나 — `MOVE_Falling`이면 마찰을 `FallingLateralFriction`(0)으로 되돌린다. 안 그러면 위 플래그 때문에 `BrakingFriction`이 공중에도 걸려 점프 중 손을 떼면 수평 속도가 녹는다 | — |
| 질주 램프 | 예측 상태 `SprintRamp` 0..1 이 `GetMaxSpeed` 상한을 걷기 ↔ 달리기 사이로 잇는다. `UpdateCharacterStateBeforeMovement`에서 스태미나 블록 **뒤**에 진행 — 조건 `bWantsToRun && !bExhausted && 수평 속도 > 0`(탈진은 갱신 뒤 값). 오를 때 ease-out quad 를 정확히 적분(진행도 a = 1 − √(1 − S) 를 역산해 dt/UpTime 더하고 S = 1 − (1 − a)²), 내릴 때 선형. 시간 0 = 즉시(옛 동작). 프레임률과 무관하게 0.1/0.2/0.3/0.4/0.6/0.85초에 상한 453/503/546/582/632/654. `FSavedMove_CL9`가 스냅샷(`SavedSprintRamp`)을 들고 `PrepMoveFor`가 복원, `CombineWith`가 되돌린다. 복제·보정 페이로드·압축 플래그 없음(스태미나와 같은 등급 — 0.85초 안에 포화해 어긋나도 저절로 낫는다) | `SprintRampUpTime` (0.85) · `SprintRampDownTime` (0.25) |
| 질주 해제 | 별도 감속 열이 없다 — 상한이 `SprintRampDownTime`에 걸쳐 내려가고, 제동 권한(1300 + 3.5v)이 상한 하강 속도(258/0.25 = 1032 cm/s²)보다 커서 속도가 상한에 붙어 내려간다(엔진의 "상한 아래로는 제동하지 않음" 클램프). 이 부등식이 깨지는 행은 로드 때 Warning 한 줄 | — |
| 질주 중 조향 무게 | 무브마다 `GroundFriction = Lerp(GroundFriction, SprintTurnFriction, SprintRamp)` — 엔진 `PhysWalking`이 이 멤버를 `CalcVelocity`에 넘긴다. 속도가 아니라 예측 상태에 걸어 걷기 속도 문턱에서 갈라지지 않는다. 전력 질주 중 90° 급회전: 0.25초에 정렬, 옛 방향으로 ≈ 103cm 밀림. 마우스로 도는 모퉁이는 손해 0 | `GroundFriction` (8) · `SprintTurnFriction` (5.0) |
| 공중 제어 | `AirControl` 상수(가속 2000 × 0.32 = 640 cm/s² 횡가속). 점프 높이·달리며 점프한 거리는 그대로, 제자리 점프 앞 도달거리가 33 → 약 205~225cm | `AirControl` (0.32) |

- **`IsRunning()`은 여전히 플래그 기준이다**(`bWantsToRun && !bExhausted && 움직임`) — 스태미나 소모·팔 애님·몬스터 청각이 그대로다.
  램프가 차는 동안에도 스태미나는 이진 규칙대로 준다(C7 — "6초 스태미나"의 뜻을 바꾸지 않는다).
- **로드 검사 A1·A2에 램프 항** (`UCL9DefRegistry::CheckCharacterRelations`): 램프 손실 = (Run − Walk) × UpTime / 3(=73.1cm),
  해제 이득 = (Run − Walk) × DownTime / 2(=32.3cm). 질주 예산 = Run × Drain − 램프 손실(3851cm), 지친 지속 속도 =
  (Run × Drain + Walk × Regen − 램프 손실 + 해제 이득) / (Drain + Regen)(503.7cm/s). 어느 부등식도 뒤집히지 않는다(너구리 462 대비 +41.7).
  `Tools/Balance/check_inequalities.py`는 같은 두 식을 아직 안 쓴다 — 리드 후속.
- **데이터 끄기**: `SprintRampUpTime`/`SprintRampDownTime` 0 = 즉시 상한(옛 동작), `SprintTurnFriction` = `GroundFriction` = 질주 조향 무게 없음.
- 검증: 순수 수학은 `CL9.Feel.Ramp` 자동화 테스트(고정 dt 1/30·1/60·1/120), 곡선 전체는 `Tools/Unreal/qa_control_feel.py`(PIE, t.MaxFPS 60).

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
- **메시 요(`MeshYaw`, 2026-10-22)**: 리그마다 앞 방향이 달라 전신·1인칭 팔 메시의 요를 행이 정한다. 마네킹 규격(`SK_Farmer_A_*`)은 −90(기본값),
  계란 요리사(`SK_RoundCook_*`, 리그 `RIG_RoundCook_Body`)는 0이다. `ApplyCharacterDef`가 두 메시에 걸고, 팔 뷰모델 바탕(`ArmsBaseRotation`)과
  엔진 메시 오프셋(`CacheInitialMeshOffset` — 남의 폰 보간·앉기 기준)도 다시 잡는다. 그래서 계란 요리사로 바꿀 때 시험 폰 `BP_RoundCookTrial`처럼
  메시 회전만 다른 BP를 따로 둘 필요가 없다 — 행의 외형 열(`SkeletalMesh`·`Skeleton`·`AnimBP`·`ArmsMesh`·`ArmsAnimBP`)과 `MeshYaw`만 바꾼다.
- **기본 플레이어는 계란 요리사(2026-10-22 사용자 "아예 round로")**: 정식 게임모드 `BP_CL9GameMode`·`BP_CL9GameMode_Flat`의 `DefaultCharacterAlias`를
  `round_cook_trial`로 돌린다. `round_cook_trial` 행은 `MeshYaw` 0이다. 적용 스크립트는 `C:\Work\CL9\Tools\Unreal\switch_player_to_round_cook.py`이고,
  `farmer_a` 행은 남아 있어서 별칭만 되돌리면 원래대로 돌아간다.

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
- **코요테 타임** (2026-10-05, [../goal/control-feel.md](../goal/control-feel.md) C15): 턱에서 **걸어** 떨어진 뒤 `CoyoteTime`(0.12초) 안이면
  아직 땅 점프와 같은 점프가 된다. CMC `HandleWalkingOffLedge` 오버라이드가 타이머를 건다(점프·발사는 이 길을 안 탄다 — 땅 점프 뒤에는
  코요테가 없다). 타이머 `CoyoteTimeRemaining`은 질주 램프와 같은 예측 상태로, 낙하 중 시뮬레이션 시간으로 줄고 낙하가 아니면 0, 저장무브
  스냅샷·`CombineWith` 되돌림이 있다. 엔진의 전용 훅 `ACharacter::CanJumpWhileFalling()`을 덮어 이 타이머를 읽는다 — `DoJump`·
  `CanJumpInternal`은 덮지 않는다. 점프 뒤에는 `JumpCurrentCount`가 1 이라 두 번째 점프가 없다. 정점은 턱 위 약 142cm(≤ 156).
- **점프 버퍼** (같은 날): `IA_Jump` 누름은 `JumpInputStart`(= `Jump()` + 코요테 없이 공중이면 누른 시각 기억). 소유 머신의 `Tick`이
  착지(`IsMovingOnGround`) 뒤 `JumpBufferTime`(0.15초) 안이면 `Jump()`를 다시 낸다 — 다음 무브가 평범한 땅 점프로 서버에 간다(서버에는
  특별한 것이 없다). `Landed` 안에서 내면 안 된다: 엔진이 `Landed` 직후 착지 물리 전환에서 점프 입력을 지운다.
- 데이터 끄기: `CoyoteTime` 0 · `JumpBufferTime` 0 (둘 다 0..0.3, 로드 검사). 버퍼 0 은 완전한 끄기다 — `JumpInputStart`가 무장하지
  않고 `Tick`도 창 0 이면 바로 버린다(착지 프레임의 누름도 엔진 기본처럼 사라진다, 리뷰 MP-1). **참여 클라이언트에서의 코요테 예측은 2인
  세션 전까지 미확인이다.**

## 앉기 — Ctrl 누르는 동안 앉아 걷기 (2026-10-22)

이동은 세 가지다. 아무 키 없이 움직이면 걷기(`WalkSpeed`), Shift를 누르면 달리기(`RunSpeed`, 질주 램프·스태미나), **Ctrl을 누르는 동안은
앉아 걷기**(`CrouchSpeed`)다. 사용자가 정한 두 가지(2026-10-22)는 이렇다. 앉아 걸으면 **소음이 0**이고, **카메라와 몸(캡슐)이 모두 낮아진다**.

- **엔진 앉기를 그대로 쓴다.** `IA_Crouch` Started → `Crouch()`, Completed → `UnCrouch()`(누르는 동안만, 토글이 아니다). 엔진의 `bWantsToCrouch`는
  이미 예측 무브의 압축 플래그(`FLAG_WantsToCrouch`)에 실리고 `bIsCrouched`는 남의 폰에 복제되므로 커스텀 플래그를 새로 두지 않았다.
  `ApplyCharacterDef`가 `NavAgentProps.bCanCrouch = true`, `SetCrouchedHalfHeight(CrouchedHalfHeight)`를 심는다.
- **속도**: `UCL9CharacterMovement::GetMaxSpeed`는 앉은 동안 걷기·달리기 보간 대신 `CrouchSpeed`를 상한으로 쓴다. 수레 배율과 넘어짐·처형·인트로
  0은 그 위에 그대로 걸린다. 앉은 동안 달리기 키는 효과가 없다 — `IsRunning()`이 거짓이고 스태미나 소모와 질주 램프 상승도 멈춘다
  (`UpdateCharacterStateBeforeMovement`, Super가 이번 무브의 앉기를 먼저 끝낸다). 앉은 채로는 점프가 안 된다(엔진 `CanJumpInternal`).
  앉은 채로 턱 끝에서 떨어지지 않는 것도 엔진 기본값(`bCanWalkOffLedgesWhenCrouching` false)이다.
- **몸**: 캡슐 반높이가 `CrouchedHalfHeight`로 줄어 그 높이(×2)보다 높은 틈 밑을 지나간다. 땅 위에서는 엔진이 발을 제자리에 두고 중심을 내리며,
  전신 메시는 엔진 `OnStartCrouch`가 바닥에 붙여 둔다. 머리 위가 막혀 있으면 Ctrl을 떼도 설 수 있을 때까지 앉은 채로 남는다(엔진 `UnCrouch`
  겹침 검사). **일어설 때 엔진은 클래스 기본(CDO) 반높이로 돌아가므로** 표의 `CapsuleHalfHeight`는 `CL9::CapsuleHalfHeight`(90)와 같아야 한다.
- **눈높이**: `ACL9Character::GetCurrentEyeHeight()`는 앉았으면 `CrouchedEyeHeight`, 아니면 `EyeHeight`를 돌려준다(바닥 기준). 카메라 바탕 높이
  (`UpdateViewFeel` (1)), 줍기 시선 원점, 휘두르기 원점, 던지기 원점이 모두 이 값과 지금 캡슐 반높이에서 눈 자리를 잡는다. 앉는 순간 캡슐
  중심이 내려가면 캡슐 자식인 카메라도 같이 뚝 떨어지므로, `OnStartCrouch`·`OnEndCrouch` 오버라이드가 `bCrouchMaintainsBaseLocation`일 때
  `CameraBaseZ`를 그만큼 되돌린다. 그 뒤 앉을 때는 `CL9Feel::CrouchCameraInterp` 18/s로 빠르게 새 눈높이에 도달한다. 일어서기·넘어짐/사망의 기존 높이 보간은 `CameraSinkInterp` 6/s를 유지한다.
- **소음 0**: 몬스터 청각(`SenseTick`의 걷기 청각)과 잠든 몬스터의 걸음 기상(`WakeRadius`)은 앉은 사람의 걸음을 세지 않는다. 시야 판정은
  바뀌지 않는다 — 보이면 보인다. 발소리 **소리**(사람에게 들리는 효과음)는 걷기 볼륨 그대로다. 파동은 `TickStepNoise`가 앉은 동안 `crouch` 행으로
  낸다 — 판정 반경이 없는 아주 작은 링으로 "거의 소리가 없다"만 보여 준다(2026-10-22 사용자 — 뛰기·걷기·앉아 걷기 셋의 파동이 달라야 한다,
  [wave.md](wave.md) 행 목록). 행을 끄면 아무것도 뜨지 않는다. 수레 소음은 그대로 난다.
- **애니메이션**: 1인칭은 눈높이 이동만 하고 전용 클립이 없다. 전신(남이 보는 몸)에는 앉아 서 있기·앉아 걷기 클립 두 개가 필요하고, ABP는
  엔진의 복제 변수 `bIsCrouched`를 읽는다(아트 몫, [animation.md](animation.md)).
- 표 열: `CharacterTable`의 `CrouchSpeed`·`CrouchedHalfHeight`·`CrouchedEyeHeight`(Crouch 분류). 헤더 기본값 200·55·90은 자리표시다 — 수치는
  balance가 정한다. 로드 검사: `CrouchSpeed` > 0, `CapsuleRadius` ≤ `CrouchedHalfHeight` ≤ `CapsuleHalfHeight`, 0 < `CrouchedEyeHeight` ≤
  `CrouchedHalfHeight` × 2.

## 피격·넘어짐·죽음 — OnDamage 가상 함수

**데미지 수신 통로는 가상 함수 하나다** (2026-09-02 확정 — `TakeDamage` 대체):

```cpp
// 캐릭터·몬스터가 각자 구현 (베이스가 달라서 인터페이스로)
class ICL9Damageable {
  virtual void OnDamage(int32 Amount, AActor* Source, float Knockback) = 0;  // 호스트에서만 호출 (2026-10-16 정정: 넉백 인자까지 3개 — CL9Damageable.h)
};
// ACL9Character::OnDamage — HP 차감, HurtStun, 넘어짐, (죽을 때만) 짐 흘림
// ACL9Monster::OnDamage  — HP 차감, HitStun, 죽음 (monster.md)
```

- 호출은 **호스트 판정 코드에서만** (몬스터 잡기, 프라이팬 밀치기 등) — 클라 호출 금지.
- HP는 PlayerState의 int (기본 3 — 값의 문제, character.md). 잡히면 `CatchDamage`만큼
  OnDamage로 깎이고 `HurtStun`(0.35초) 비틀 + 넘어짐 — 판정은 호스트, 표현은 OnRep/Multicast
- ~~**짐을 흘리는 것은 죽을 때뿐이다** (2026-09-15 사용자)~~ → **넘어지면 0~2개를 무작위로 흘린다** (2026-10-17 결정 B, 사용자 "0~2개, 2개는 낮음" —
  [../goal/knockdown-drop.md](../goal/knockdown-drop.md)). HP가 남는 타격은 `HostCancelCarryActions()`(수레 놓기 `StopPull`·던지기 차징·휘두르기 대기 취소) 뒤
  `HostDropOnKnockdown()` — `CL9Rules.KnockdownDropWeight0/1/2` 가중치로 개수를 굴리고, 찬 칸(활성 칸 포함·머리 칸 제외) 중 무작위로 그만큼을 죽음과 같은
  `SpillAtFeet` 로 발밑에 흘린다(손의 것은 액터를 놓고, 비활성 칸은 저장된 값·조명 스위치 그대로 다시 스폰). 활성 칸 번호는 그대로. 셋 다 0 이면 옛 규칙.
  HP 0이 되는 타격은 그대로 `HostDropEverything()`가 손과 네 칸을 전부 발밑에 흘린다(동료가 시체 옆에서 주워 가게).
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
  - **몸·팔** `ApplyThirdPersonMeshes()`(`ApplyExecuted` 와 공통부): 처형 중 ∨ 데스캠 ∨ 전원 인트로 카메라가 내 1인칭 카메라 자리에서 120 cm 넘게 떨어졌으면
    (`ACL9PlayerController::IsIntroBodyShown`, 2026-10-11 K10-F F5 — 눈 위치에서 출발하는 동안은 1인칭 그대로, 한 번 바뀌면 그 인트로 끝까지) 몸 `FirstPersonPrimitiveType`
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
  (HP = `MaxHp`, 넘어짐 해제, **넘어짐 시각을 "아직 없음"(−1000)으로 되돌림**, 데스캠 끔)하고 트럭 패드 위로 옮기며 관전을 끝낸다(뷰 타깃 = 자기 폰).
  (트럭이 특정 장소에 닿아야 되살아나는 규칙은 다음 명세다 — 지금은 위 흐름 그대로.)
  - **`HostRevive` 가 넘어짐 시각을 되돌리는 이유** (2026-10-06, round-start-truck-intro K5): 판 끝에 트럭 밖에 남겨진 사람은 새벽에 죽고 곧바로
    로비에서 되살아난다(`DawnLength` 0 이면 같은 프레임). 그러면 다른 머신은 죽음의 `KnockdownStartServerTime` 을 **산 몸에서 처음 보게 되어**
    피격 셰이크·Hurt 소리·빨간 플래시·비네트가 부활한 몸에 떴다. 그래서 `HostRevive` 가 `KnockdownStartServerTime = NoKnockdownServerTime`(−1000)으로 되돌린다 —
    넘어진 적 없는 몸과 같은 상태다. 셰이크(`UpdateHitShakes`)와 Hurt 소리(`UpdateHitSounds`)는 이 값이 **올라갈 때만**(`>`) 내므로 되돌림(내려감)은
    사건이 아니다(옛 `!=` 비교를 상승 에지로 바꿨다). 플래시·비네트는 경과 시간(`GetHurtElapsed`)이 아주 커져 저절로 꺼진다.
- **판 끝에 트럭 밖에 있으면 남겨져 죽는다** (2026-10-06, K5 — 흐름·순서는 [session.md](session.md) "판 끝 — 남겨진 사람"). 캐릭터 쪽 입구는
  **`ACL9Character::HostLeaveBehind()`**(호스트 전용, 이미 죽었으면 아무 일도 없다): `HostCancelCarryActions()` 로 수레를 놓고 차징·대기를 취소한 뒤
  **손에 든 것(`HeldItem`)과 머리에 쓴 것(`WornItem`)을 `Destroy` 하고 퀵슬롯·`HeadItem` 미러를 비운다**(바뀐 게 있으면 `NotifyCarryChanged`) —
  흘리지 않고 없앤다. 머리 칸을 따로 처리하는 이유: 죽음의 흘림 `HostDropEverything` 은 `WornItem` 을 건드리지 않는다.
  그다음 `HostApplyDamage(남은 HP 전부, nullptr, "left behind")` 로 죽음 경로(데스캠·`HandlePlayerDeath`)를 그대로 탄다. `HostApplyDamage` 가 protected 라
  이 래퍼가 캐릭터 안에 있다. 넘어짐 무적·인트로 무적을 거치지 않는다(처형의 `HostKill` 과 같다) — 로그는 `left behind 'None': -N HP → 0 (dead — down for the night)`.
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
  유일한 예외는 아래 처형의 `HostKill` — 이 문을 아예 거치지 않는다. (판 끝 남겨짐의 `HostLeaveBehind` 도 `HostApplyDamage` 를 직접 불러 이 문을 거치지 않는다.)
- **전원 인트로 동안은 잠기고 무적이다** (2026-10-06, round-start-truck-intro K11 — 건물 전원 스위치를 켠 판의 연출, [light.md](light.md) "전원 단계"):
  `IsIntroLocked()` = 이 플레이어가 켠 순간의 **참가자**(`IntroTargets` 에 든 사람 — 살아 있고 처형·관전 중이 아니었던 사람)이고, 판(Night)이며,
  켠 시각 T 로부터 `IntroLeadIn + IntroFlyTime + IntroFlickerTime + IntroReturnTime`(기본 9.4 초 — 비행 7.0 초, 2026-10-10) 안이다. `IsIntroInvulnerable()` 은 같은 창에 `IntroGrace`(기본 1.0 초)를 더한 것(10.4 초) —
  조작이 돌아온 뒤 한 박자의 반응 시간이다. 둘 다 `ACL9GameState::IsIntroLocked/IsIntroInvulnerable(PlayerState)` 에서 복제된 값(켠 시각·참가자 목록·페이즈)과 서버 시각만으로
  파생하므로 서버와 소유 클라이언트가 같은 값을 내고 새 복제 필드가 없다. **Night 에서만 참이다** — 판이 창 안에서 끝나면(귀가·시간 끝) 새벽·로비에 잠금이 남지 않는다.
  - **잠금은 한 곳이 아니라 다섯 곳이다**: `IsIncapacitated()`(= 넘어짐 ∨ 처형 ∨ **인트로 잠금** — 서버 RPC 게이트 전부), `UCL9CharacterMovement::GetMaxSpeed()`(0 — 넘어짐·처형 옆),
    `LookInput`(시선 — 돌아왔을 때 떠난 자리 그대로), `JumpInputStart`(점프 초속은 속도 상한을 거치지 않는다), 조준 대상·프롬프트 게이트(`Tick` — 포커스 없음).
  - **무적 게이트**: `OnDamage` 머리의 `IsKnockedDown()` 바로 뒤에 `IsIntroInvulnerable()`(로그 `hit by '…' during the power intro — ignored (invulnerable)`)와, 몬스터 쪽 관문 —
    잡기 진입(`bCanCatch`)·덮침 결과(`bHit`)·`TryBeginExecute` 의 타깃 상태 검사·`HostCheatExecute`(막힘 사유 `target-intro`). `cl9.damage` 도 같은 문이라 우회가 없다.
    이미 진행 중인 처형의 `HostKill` 은 이 게이트를 거치지 않는다. 스토커의 미발견 계수기는 무적 동안에도 흐른다 — 창이 끝나면 곧바로 처형 조건이 설 수 있어 `IntroGrace` 가 반응할 틈을 준다.
  - 참가자가 아닌 사람(늦게 들어온 사람·이미 죽은 사람)은 잠기지 않는다. 후보가 0개인 레이아웃은 참가자가 비어 아무도 잠기지 않는다(불만 켜졌다 죽는다).
- **처형 — 종 전용 즉사** (2026-09-20, [../goal/execute-sequence-pig.md](../goal/execute-sequence-pig.md) D2·D3·D6·D9;
  몬스터 쪽은 [monster.md](monster.md) 스토킹과 처형 — 너구리는 스토커, 돼지는 잡히면 처형이지만 캐릭터 쪽 흐름은 같다):
  - **상태** = `ExecuteStartServerTime`(float, `ReplicatedUsing=OnRep_ExecuteStartServerTime`, 모두에게 — 남의 ABP 와
    CMC 가 읽는다; −1000 = 아님) + `ExecutedBy`(`TObjectPtr<ACL9Monster>`, Replicated). 게터(BlueprintPure):
    `IsExecuted()`(= 시각 ≥ 0) · `GetExecuteElapsed()`(처형 아니면 −1) · `GetExecuteDuration()`(`ExecutedBy` 행의
    `ExecuteDuration`, 없으면 0) · `GetExecutedByAlias()`(`ExecutedBy` 행의 `Alias` — `pig`/`raccoon`, 없으면 `None`;
    ABP 가 종별 2인 클립을 고른다, 2026-09-21); C++ `IsIncapacitated()`(= 넘어짐 ∨ 처형 ∨ 전원 인트로 잠금 — 위 "전원 인트로 동안은 잠기고 무적이다") · `GetExecutedBy()`. **`ExecutedBy` 는
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

### 겁먹는 표현 — 공포 게이지·소리 겹·스팅어·추격 신호·시선 끌림 (2026-10-17, 결정 H)

"무섭지 않다"의 표현 층이다 ([../reference/scare-mechanics.md](../reference/scare-mechanics.md) §3 — 가장 싼 다섯 개, 전부 로컬).
`UCL9FearComponent`(`Source/CL9/Character/`, 캐릭터의 기본 서브오브젝트 `Fear`)가 **소유 폰에서만** 틱마다 돈다. 판정·호스트 로직·복제는
하나도 건드리지 않는다 — 입력은 이미 복제되는 몬스터의 `State`·`GetStateElapsed()`·위치와 내 `GetKnockdownElapsed()` 뿐이다. 값은 전부
`CL9Rules` 겁먹는 표현 열([../plan/data/rules.md](../plan/data/rules.md)), 소리는 `SoundTable` 행, 흔들림은 `ShakeTable` 행이다.

- **공포 게이지** `FearLevel` 0~1 (리썰 `fearLevel`). 사건이 **점프**시킨다(`JumpTo` — 올라갈 때만, 차이 0.05 미만 무시): 화면에 보이는 깨어 있는 몬스터가
  `FearSeenNearRadius` 안이면 `FearSeenNear`, `FearSeenFarRadius` 안이면 `FearSeenFar`; Chase 전이(아래 조건)에 `FearChaseBegin`; Pounce 전이에
  `FearPounce`; 내 넘어짐의 상승 에지에 `FearHurt`. 추격 중인 몬스터가 `FearSeenNearRadius` 안에 있는 동안은 `FearNearRate`/초로 **쌓인다**(보이든 안 보이든).
  이번 틱에 아무도 안 올렸으면 `FearDecayPerSecond` 로 식는다. 죽었거나 관전이면 0.
- **"화면에 보인다"** = 카메라 전방 원뿔(가로 FOV 반각) 안 ∧ `ACL9Monster::TraceWorld`(세상 지형만)로 눈→몬스터 중심이 안 막힘. `FearSeenFarRadius` 밖·
  자는 것·시체는 트레이스도 긋지 않는다.
- **소리 네 겹** (리썰 `SetFearAudio`): `FearLow` 는 게이지가 `FearLowMin`~`FearLowMax` 사이, `FearHigh1` 은 > `FearHigh1Min`, `FearHigh2` 는 > `FearHigh2Min`
  일 때 켜지고(목표 볼륨 1), `Heartbeat` 는 문턱 없이 볼륨 = 게이지. 볼륨은 `FearLayerFadeSpeed` 로 목표를 따라가고 0.01 아래면 멈춘다. 음원은 처음 0 을
  벗어날 때 `CL9Sound::Resolve` 로 받아 `CreateSound2D` 루프로 튼다 — 표·행·애셋이 없으면 그 겹은 조용히 없다. 애셋에 Looping 이 켜져 있어야 한다.
- **첫 목격 스팅어** (리포 `MusicEnemySighting`): 몬스터가 내 화면에 **처음** 들어오는 틱(지난 틱엔 안 보였다)에 `SightingRadius` 안이고 쿨다운이 끝났으면
  `MonsterSighting` 을 몬스터 자리에서 + 셰이크 `Sighting`, 그 뒤 `SightingCooldown` 동안 쉰다(플레이어 하나에 하나 — 종별이 아니다).
- **추격 시작 신호** (리포 `ChaseBegin`): 몬스터의 전이(`State` 가 바뀌거나 같은 상태의 경과가 줄어듦 — `TickStateSounds` 와 같은 틀, 첫 틱은 기준값만)가
  Chase 로 들어오고 화면에 보이거나 `ChaseBeginRadius` 안이면 `MonsterChaseBegin` 을 몬스터 자리에서 + 셰이크 `ChaseBegin` + 게이지 점프. 리포는
  "누구를 쫓는지"로 세기를 바꾸지만 CL9 는 타깃을 복제하지 않으므로 거리·화면이 그 대신이다(새 복제를 만들지 않았다).
- **시선 끌림** (리포 `EnemyHeadController`): 컴포넌트가 추격 중(Chase·Pounce·Stalk)인 가장 가까운 몬스터를 `ChaseLookRadius` 안에서 고르고
  (`GetChaseLookTarget`), `UCL9HurtLookModifier` 가 **넘어져 있지 않을 때** POV 요를 그쪽으로 각도 차이 × `ChaseLookStrength`, 상한 `ChaseLookYawMax`,
  시정수 `ChaseLookTime` 으로 끌고 없어지면 되돌린다. 기준은 컨트롤 요(POV 요를 기준으로 재면 되먹임). 넘어지면 끌림은 풀리고 피격 요가 대신한다.
  피격 카메라와 같은 **POV 만** 원칙 — 조준은 그대로다. `HurtLookTime` 0 은 이제 피격 오프셋만 끄고, 끌림은 `ChaseLookYawMax` 0 으로 끈다.
- **없는 것**: 리썰 화면 필터(게이지 > 0.4 비네트), 동료 시체 목격 점프, 리포의 추격 시작 때 배경음악 20초 중단, "보면 돌아본다"(판정 층 — A·E 결정 뒤).
- **상태**: 코드·열·행은 들어갔고 **음원·셰이크 애셋은 비어 있다**(2026-10-17) — 들리는 것은 아직 없고, 시선 끌림만 눈에 보인다.

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
  (`UpdateHitShakes`). 접속 직후 첫 틱은 값을 받아 적기만 하고 재생하지 않는다. **피격(`Hurt`) 셰이크는 `KnockdownStartServerTime` 이 올라갈 때만**
  재생한다(2026-10-06 — `HostRevive` 가 이 값을 "아직 없음"으로 **되돌리는** 것은 사건이 아니다; 위 "부활" 항목). 타격(`Hit`) 셰이크는 `SwingHitServerTime` 의 변화 그대로다.
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
  일어난 뒤에도 계속 커진다)를 쓴다. 넘어진 적이 없으면 −1000 기본값 덕에 아주 큰 값이 되어 꺼져 있다 — 부활이 되돌린 값도 같은 −1000 이라
  부활한 몸의 플래시·비네트가 같은 이유로 꺼져 있다.
- HUD 비네트 이미지는 `GetHurtFlash()`를 불투명도로 읽는다 ([ui.md](ui.md)).

### 소리·팔 히트스톱·발소리 (2026-09-18 구현)

2026-09-15 표 위에 얹은 층이다 ([../goal/game-feel.md](../goal/game-feel.md) §1·§2). 새로 복제하는 값은 없고, 전부
이미 복제된 시계에서 파생한다. 몬스터 쪽(`Hit`·`MonsterDeath` 소리·몬스터 히트스톱·퍼프·죽음 발사)은
[monster.md](monster.md) "타격의 표현", 표·에셋·재생 헬퍼는 [sound.md](sound.md).

| 무엇 | 누가 보나 | 원천 | 어디서 |
|---|---|---|---|
| `Swing` 소리 (폰 자리, 맞든 빗나가든) | 전원 | `SwingStartServerTime` 변화 | `UpdateHitSounds` |
| `HitSurface` 소리 (폰 자리 — 벽·바닥을 쳤다, 2026-09-22) | 전원 | `SwingSurfaceHitServerTime` 변화 (Replicated, 조건 없음) | `UpdateHitSounds` |
| `Hurt` 소리 (폰 자리) | 전원 | `KnockdownStartServerTime` **상승**(되돌림은 제외, 2026-10-06) | `UpdateHitSounds` |
| 1인칭 팔 히트스톱 | 때린 본인 | `SwingHitServerTime` 뒤 `CL9Rules.HitStopTime`(0.08초), 그 명중이 죽였으면(`bSwingHitKill`, COND_OwnerOnly) `KillHitStopTime`(0.12초, 2026-10-17) | `GetSwingAnimElapsed()` → ABP `SwingElapsed` |
| 발소리 `Footstep` (발밑) | 전원 | 걸음 위상이 1 을 넘음(2026-10-05 — 아래) | `TickFootstepSound` |

- **소리도 셰이크와 같은 틱 비교다** — `UpdateHitSounds()`가 지난 틱과 값이 다르면 `CL9Sound::PlayAt`으로 한 번 튼다.
  다만 셰이크의 `UpdateHitShakes`는 소유자 전용인데 소리는 **남의 폰에서도 들려야** 하므로 로컬 조작 게이트 **앞**에서
  모든 머신이 돌고, 비교값도 따로 둔다(`LastSeenSwingStartTime`·`LastSeenHurtSoundTime`). 접속 직후 첫 틱은 기준값만
  받아 적는다(`bHitSoundsPrimed`). `Hurt`는 넘어지는 순간 한 번이고 일어날 때는 없다. 비교는 `!=` 가 아니라 **상승 에지**(`>`)다 — 부활이 시각을
  "아직 없음"(−1000)으로 내려도 소리가 나지 않는다(위 "부활" 항목).
- **팔 히트스톱은 판정 시계가 아니라 ABP 전용 게터다.** `GetSwingAnimElapsed()`(BlueprintPure)는 이번 휘두르기가 맞은 뒤
  (`SwingHitServerTime >= SwingStartServerTime`, `COND_OwnerOnly`라 때린 본인 화면에서만) `HitStopTime` 동안 명중 시각의
  경과값에 머물고, 지나면 실제 경과에서 정지 길이를 뺀 값으로 이어진다 — 팔 클립이 명중 프레임에 0.08초 멈췄다가 이어서
  끝까지 돈다. 남의 화면·헛스윙·표 없음이면 `GetSwingElapsed()`와 같다. **`ABP_FP_View`·`ABP_Unarmed`의 `SwingElapsed`
  변수 소스가 `GetSwingElapsed` → `GetSwingAnimElapsed`로 바뀐다** — 분기 조건 `SwingElapsed ∈ [0, 0.347)`도 같은 변수를
  보므로 클립이 0.08초 늦게 끝까지 간다([animation.md](animation.md) 휘두르기 가지). 판정 `HostTickSwingHit`은 여전히
  `GetSwingElapsed()`를 쓴다 — 히트박스 창은 그대로다. 전역 시간 지연·`CustomTimeDilation`은 쓰지 않는다(리슨 호스트에서
  남에게 번지고 이동 예측을 깨뜨린다).
- **발소리는 애님 노티파이가 아니라 걸음 위상이다** (클립에 노티파이가 없다. 2026-10-05 거리 누적기 → 정규화 위상,
  [../goal/control-feel.md](../goal/control-feel.md) C10). `TickFootstepSound(Dt)`가 모든 머신에서 매 틱 `GetFootLocation()`의
  수평 이동 거리를 보폭으로 나눠 `StepPhase`(0..1)에 더하고, 1 을 넘는 순간 1 을 빼고 발밑에서 `Footstep` 행을 피치 ±8 %로
  튼다(걸음 좌우 `bStepParity`를 뒤집고 `StepCount`를 센다 — 한 틱에 한 발). 보폭은 수평 속도로 `CL9Rules.FootstepStrideWalk`(150cm,
  396 에서 2.64 걸음/초) → `FootstepStrideRun`(196cm = 654 × 0.3초, `AN_Farmer_FPRun` 0.6초 루프의 반)을 **선형으로 잇는다** —
  달리기 입력 `bWantsToRun`은 남의 폰에 복제되지 않아 속도로 가른다. 보폭이 바뀌어도 위상을 버리지 않으므로 질주 램프 동안
  케이던스가 2.64 → 3.34 걸음/초로 단조롭게 오른다(옛 누적기는 걷기·달리기가 갈릴 때마다 0 으로 돌아가 램프 초반에 박자가 오히려
  늘어졌다). 볼륨은 세 단계로 섞지 않는다(2026-10-22) — 걷기 `ViewFeel.FootstepVolumeWalk` 0.7, 뛰기(쉬프트 — 속도 비율 `FootstepRunAlpha` 0.5 이상) `FootstepVolumeRun` 1.0, 앉아 걷기(엔진 웅크리기 `bIsCrouched`) `FootstepVolumeCrouch` 0 — 0 인 단계는 틀지 않는다. `IsMovingOnGround()`일 때만 — 서 있거나 공중이거나
  넘어졌거나 죽었으면 위상을 얼리고, `ViewFeel.StepIdleResetTime`(0.25초)이 지나면 0.5 로 둔다(다시 걸으면 반 보폭 뒤에 첫 발).
  걷기 보폭 0 이면 발소리가 없다. 소유자의 카메라 보브가 같은 위상을 읽는다(위 "카메라 손맛"). 소음 판정의 `TickStepNoise`(파동,
  호스트·소유 클라만, 표의 간격)와는 **별개 누적**이다 — 발소리는 연출이지 몬스터 청각이 아니다.

### 일상 동작 소리 — 소유자 전용 (2026-10-05)

[../goal/control-feel.md](../goal/control-feel.md) C14. 새 행 8개(`Jump`·`LandSoft`·`LandHard`·`SlotSwitch`·`LightToggle`·`Eat`·`Exhausted`·
`CartGrab`)와 기존 `Pickup` 행이 **그 폰을 조종하는 머신에서만** 난다 — `PlayAt`에 감쇠가 없어 남에게 들리게 하면 맵 전체에 같은 크기로
들리기 때문이다(임시, [sound.md](sound.md)). 새 복제·RPC·Multicast 없음, 몬스터가 듣는 소음 없음(기존 `Land` 파동·줍기 소음 반경은 그대로).

| 소리 | 언제 | 어디서 |
|---|---|---|
| `Jump` (피치 ±6 %) | 점프한 순간 — 보정 리플레이 제외, 0.15초에 한 번 | `OnJumped_Implementation` |
| `LandSoft` / `LandHard` | 착지 낙하 속도 350~1000 cm/s = `LandSoft`(볼륨 0.5 → 1.0, ±6 %), 1000 이상 = `LandHard`(±4 %), 350 미만 없음 — 같은 가드 | `Landed` |
| `Pickup` | 채워진 퀵슬롯 칸 수가 늘었거나 머리 칸이 채워짐(착용) | `UpdateOwnerSounds` |
| `SlotSwitch` | 활성 칸이 바뀜 — 줍기로 바뀐 것은 `Pickup`만 | `UpdateOwnerSounds` |
| `LightToggle` | 든 손전등·쓴 헤드램프(`ACL9FlashItem`) **같은 액터**의 스위치가 바뀜 — 이 머신이 아직 들고(`!IsFree`)·쓰고(`IsWorn`) 있다고 보는 것만 찍고, 조명 키(F·든 손전등 좌클, `LocalLightInputTime`)를 누른 뒤 0.5초 안일 때만, 정전이 든 손전등을 끈 것은 뺀다 | `UpdateOwnerSounds` |
| `Eat` | `EatStartServerTime` 변화 | `UpdateOwnerSounds` |
| `CartGrab` | `PullingCart`가 비었다가 채워짐 | `UpdateOwnerSounds` |
| `Exhausted` | 탈진 래치 상승 에지(예측 상태) — 2초에 한 번까지 | `UpdateOwnerSounds` |

- `UpdateOwnerSounds`는 셰이크와 같은 "지난 틱 값과 비교"다 — 자기 PlayerState·Character 의 복제 상태와 CMC 예측 상태를 찍어
  두고 다르면 낸다. 빙의가 바뀐 뒤 첫 틱(`NotifyControllerChanged`가 기준값을 지운다)은 받아 적기만 한다.
- 참여 클라이언트에서는 상태가 복제돼 온 뒤라 RTT 만큼 늦다 — 다른 손 동작 피드백과 같다(클라 표현 예측 M6 은 이번에 안 한다).

## 네트워크 정리

| 상태 | 위치 | 이유 |
|---|---|---|
| 달리기 플래그·스태미나·질주 램프·유효 무게 속도 | **CMC (예측 경로)** | FSavedMove로 서버·클라 동기 (합칠 때 `CombineWith`가 되돌린다) |
| 카메라 손맛(보브·롤·FOV·스프링)·걸음 위상·일상 동작 소리 | 복제 없음 — 소유 머신이 위 상태에서 파생(걸음 위상만 모든 머신) | 표현은 상태에서 나온다 |
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
아이템 표의 타입이 가른다). 행의 `EatBuffs`가 가리키는 버프 행(`UCL9BuffDef`, 표 에셋 `Content/Data/BuffTable`)들의 효과(스탯·양·지속초)가 걸리고
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
  **첫 명중**을 찍는 틱에 `SwingReadyTime += HitStopTime`(죽였으면 `KillHitStopTime` — `GetArmHitStopTime`, 2026-10-17) — 팔 히트스톱만큼 늦춰 맞든 빗나가든 팔이 돌아온 뒤 같은 여유가 남고,
  다음 `SwingStartServerTime` 리셋이 멈춘 팔 애님 도중에 떨어지지 않는다.
- **예약** `CL9Rules.SwingBufferTime`(초, 헤더 **0** = 끔 = 옛 동작): `Server_UseRelease`가 회복 중(`Now < SwingReadyTime`)이고
  `SwingReadyTime − Now <= SwingBufferTime`이면 `bSwingBuffered = true`(호스트 전용, 비복제, 하나뿐·덮어쓰기). 회복이 끝나는
  호스트 틱에 `HostTickSwingBuffer`가 `CanSwingNow()`(손에 `Swing` 행 ∧ `!IsIncapacitated()`)를 다시 거쳐 `HostSwing`. 취소 분기는
  따로 없다 — `HostCancelSwingWindup()`이 대기와 함께 예약도 지우고, 칸 전환·놓기·던지기·먹기·넘어짐·사망이 이미 그것을 부른다.
  처형 진입은 부르지 않지만 발화 틱의 `CanSwingNow()`가 막는다(예약은 그냥 사라진다).
- 측정 치트 `cl9.swingtaps`·`cl9.swinglog` ([cheats.md](cheats.md)).
