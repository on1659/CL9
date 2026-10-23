# 참고 — R.E.P.O. 조작감(손맛): 실측 수치와 근거

> 설치본 R.E.P.O. 의 1인칭 조작감(이동·스태미나·시점·카메라 레이어·상호작용·조작 소리)을 코드식과 직렬화 실값으로 풀어 적은 노트다.
> 조사 날짜 2026-10-05. 방법은 디컴파일 코드와 UnityPy 직렬화 값 읽기에, 코드식에 실값을 넣어 직접 계산한 "수식 재현"을 더한 것이다. **플레이 실측(PIE·인게임 측정)이 아니다.**
> 디컴파일 소스는 옮겨 적지 않았다. 구조와 수치만 적는다. 코드·에셋은 이식하지 않는다.

대상 설치본은 `C:/Program Files (x86)/Steam/steamapps/common/REPO`다 (Unity 2022.3.67f2, Steam buildid 23363152, `Assembly-CSharp.dll` SHA256 `ce995a18…ee6526` — 2026-09-21 [lethal-combat-feel.md](../lethal/lethal-combat-feel.md)가 본 DLL과 같은 해시). [README.md](../README.md)에 적힌 옛 설치 경로(`D:\SteamLibrary\...`)는 이 노트북에 없다. 이번 조사는 위 C: 경로를 읽었다.

**표기 규칙**

- `코드:Foo.cs:123` = 디컴파일 소스 `Foo.cs` 의 줄 번호.
- `직렬화:level0/PlayerController.MoveSpeed` = UnityPy로 읽은 프리팹·씬 실값.
- `수식 재현` = 코드 식에 직렬화 값을 넣어 직접 계산한 값. **플레이 실측이 아니다.**
- 거리·속도는 `Unity m (UE cm)`로 함께 적는다. UE = Unity × 100.
- 코드 필드 초기값과 직렬화 실값이 다른 경우가 많아, 둘 다 적고 **실값을 기준**으로 삼는다.

---

## 0. 한눈에 — 실값이 코드 기본값과 크게 다른 항목

| 항목 | 코드 기본값 | **직렬화 실값** | 출처 |
|---|---|---|---|
| 걷기 `MoveSpeed` | 0.5 | **2 m/s (200 cm/s)** | 직렬화:level0/PlayerController.MoveSpeed |
| `MoveFriction`(속도 추종률) | 5 | **10 /s** | 직렬화:level0/PlayerController.MoveFriction |
| 질주 `SprintSpeed` | 1 | **5 m/s (500 cm/s)** | 직렬화:level0/PlayerController.SprintSpeed |
| `SprintAcceleration` | 1 | **10 /s** | 직렬화:level0/PlayerController.SprintAcceleration |
| `CrouchSpeed` | 1 | 1 m/s (100 cm/s) | 직렬화:level0/PlayerController.CrouchSpeed |
| `SlideTime` / `SlideDecay` | 1 / 0.1 | **0.5 s / 1** | 직렬화:level0/PlayerController |
| `JumpForce` | 20 | **17** | 직렬화:level0/PlayerController.JumpForce |
| `CustomGravity` | 20 | **30** | 직렬화:level0/PlayerController.CustomGravity |
| `EnergyStart` / `EnergySprintDrain` | 100 / 1 | **40 / 10 /s** | 직렬화:level0/PlayerController |
| `CameraTilt.tiltZ` / `tiltX` | 250 / 250 | **0.03 / 0.05** | 직렬화:level0/CameraTilt |
| 잡기 `grabRange` | 4 | **2.5 m (250 cm)** | 직렬화:sharedassets0/PhysGrabber.grabRange |
| 잡기 `springConstant`/`dampingConstant` | 0.9 / 0.5 | **0.6 / 0.3** | 직렬화:sharedassets0/PhysGrabber |
| 잡기 `forceConstant`/`forceMax` | 4 / 4 | **5 / 6** | 직렬화:sharedassets0/PhysGrabber |
| `maxDistanceFromPlayer` | 2.5 | **6** (단, 실효 상한은 `min(grabRange, 6)` = 2.5) | 직렬화:sharedassets0/PhysGrabber |

> **기존 문서 정정:** [repo-valuables.md](repo-valuables.md)의 잡기 수치(4 / 0.9 / 0.5 / 4 / 4)는 코드 기본값이었다. 실제 플레이어 프리팹 값은 위 표다 — 2026-10-05 에 그 문서에 정정을 달았다.
> [lethal-combat-feel.md](../lethal/lethal-combat-feel.md)가 "코드 기본값만 확인"이라 적은 `ItemMelee.hitFreeze = 0.2`는 이번에 근접 무기·지팡이 프리팹 9종 모두 직렬화 0.2로 **확인**됐다(5절).

실제 플레이어 오브젝트 위치: 로컬 컨트롤러·카메라 리그는 씬 `level0`의 `Player/Controller`, 아바타는 `sharedassets0.assets`의 `PlayerAvatar` 프리팹(`PlayerController.playerAvatarPrefab = PPtr(2,1913)`이 가리킴). `resources.assets`의 `Module - Debug …` 안 복제본들은 디버그 모듈이라 제외했다.

---

## 1. 이동 모델

### 1.1 구조 — 리지드바디 + "속도 목표를 2단 지연 후 임펄스로 밀기"

- **CharacterController가 아니라 Rigidbody**다. 질량 `m_Mass = 2`, `m_Drag = 0`, 보간 켬(`m_Interpolate = 1`), 회전 XYZ 고정(`m_Constraints = 112`), 연속 충돌(`m_CollisionDetection = 2`). 출처: 직렬화:level0/Player/Controller Rigidbody(pid 1450).
- 충돌체: 캡슐 반지름 0.2 m(20 cm), 높이 1.7 m(170 cm), 중심 0.85 m. 출처: 직렬화:level0/Collision CapsuleCollider(pid 1495).
- 물리 프레임 50 Hz(`Fixed Timestep 0.02`). 출처: 직렬화:globalgamemanagers/TimeManager. 중력 −9.81 m/s². 출처: 직렬화:globalgamemanagers/PhysicsManager.m_Gravity.
- 몸의 yaw는 매 물리 프레임 카메라 yaw로 즉시 맞춰진다(코드:PlayerController.cs:659~684). 입력은 정규화돼 아날로그 크기가 없다(코드:PlayerController.cs:780).

**속도 계산 순서** (코드:PlayerController.cs:771~897, 매 고정 프레임):
1. 목표 속도 = 입력 방향 × (걷기 2 / 질주 2→5 / 웅크림 1) + 슬라이드 벡터 + 외부 임펄스.
2. 1단 지연: `VelocityRelativeNew`가 목표 쪽으로 `MoveFriction·dt = 0.2`만큼 lerp (연속 환산 약 11.16 /s).
3. 2단 지연: `VelocityRelative`가 1단 값 쪽으로 다시 0.2만큼 lerp.
4. 리지드바디에 `(2단 값 − 현재 속도)`를 **Impulse 모드**로 가한다. 질량 2라서 매 프레임 남은 차이의 **50 %**만 메워진다(연속 환산 약 34.7 /s). 세로 속도는 건드리지 않는다.
5. 1단 값의 크기가 0.1 m/s 이하로 떨어지면 "정지" 상태로 바꾸고 **임펄스를 끊는다.** 이때 충돌체 재질을 바꾼다.

**마찰 재질 전환** (출처: 직렬화:sharedassets0 PhysicMaterial):

| 상태 | 재질 | 동/정 마찰 | 결합 방식 |
|---|---|---|---|
| 움직이는 중 | `PlayerMove` | 0 / 0 | Minimum(1) → 바닥과 무관하게 **마찰 0** |
| 정지 | `PlayerIdle` | 5 / 5 | Average(0) |
| 바닥 기본값 | `Default` | 0.5 / 0.1, 반발 0.3 | Average |

움직이는 동안은 마찰 0으로 미끄러지듯 컨트롤러 속도를 그대로 따르고, 입력을 놓아 1단 값이 0.1 m/s 아래가 되는 순간 마찰 계수 약 2.75(기본 바닥과 평균)로 바로 붙잡힌다.

### 1.2 가감속 시간 — 수식 재현 (마찰 효과는 정지 판정 이후에만 반영)

| 상황 | 50 % | 90 % | 99 % | 비고 |
|---|---|---|---|---|
| 걷기 0 → 2 m/s (200 cm/s) | 0.18 s | **0.38 s** | 0.62 s | |
| 질주 0 → 5 m/s (500 cm/s) | 0.22 s | **0.42 s** | 0.66 s | 질주 목표(`SprintSpeedLerp`)는 0.1 s 만에 5에 도달(`SprintAcceleration 10`), 나머지는 지연 필터 몫 |
| 걷기 반전 +2 → −2 | 0 통과 0.10 s | 0.30 s | | |
| 질주 5 → 정지 | 정지 판정 **0.36 s**(그때 실속도 0.61 m/s) | | | 이후 Idle 마찰(μ≈2.75, 감속 ≈27 m/s²)로 약 0.02 s 내 정지 → 총 **≈0.38 s** |
| 걷기 2 → 정지 | 정지 판정 **0.28 s**(실속도 0.48 m/s) | | | 총 ≈0.30 s |

느낌으로 옮기면: 출발이 0.1~0.2 s 정도 눌렸다가(S자) 붙고, 멈출 때도 0.3 s 남짓 미끄러지다 "탁" 붙는다. 선형 가속이 아니라 **2차 지연 + 정지 순간 마찰 스냅**이다.

### 1.3 질주

- 조건: 질주 키(또는 토글) && 텀블 아님 && 웅크림 아님 && 에너지 > 0 && 실제 속도 > 0.01 m/s (코드:PlayerController.cs:685~695).
- 질주 목표는 `Lerp(2, 5, SprintSpeedLerp)`, `SprintSpeedLerp`는 초당 10씩 증가 → 0.1 s에 최대. 입력을 놓거나 방향 입력이 0.1 이하이면 즉시 0으로 초기화(코드:PlayerController.cs:786~799, 707).
- 질주 중엔 `CanInteract = false`(코드:PlayerController.cs:763~770).
- 업그레이드: 속도 업그레이드 1단당 `SprintSpeed +1`, 소모도 +1 (코드:PlayerController.cs:370~372, 712~713).

### 1.4 슬라이드

- 트리거: 질주하다가(또는 질주를 놓은 뒤 **0.5 s 이내**, `SprintedTimer`) 웅크림 → `CanSlide`가 살아 있으면 슬라이드 (코드:PlayerController.cs:692, 698~705, 806~817).
- 슬라이드 벡터 = 입력 방향 × 그 순간의 질주 목표 속도(최대 5). 초당 `5 × SlideDecay(1)`씩 줄며 `SlideTime 0.5 s` 동안 유지, 끝나면 뚝 빠진다. 여기에 웅크림 입력 1 m/s가 더해진다.
- 비용: 에너지 **−5** 1회 (코드:PlayerController.cs:809~813).
- 수식 재현(질주 5 m/s에서 시작): 최고 **5.37 m/s(537 cm/s) @0.2 s**, 0.5 s에 4.46 m/s, 1.0 s에 1.09 m/s, 처음 1 s 이동 3.6 m(360 cm). 목표는 6 → 3.5 m/s로 감소하다 0.5 s에 1 m/s로 떨어진다.
- 카메라: 웅크림 카메라 하강 속도 2배, 슬라이드 중엔 기는 카메라(−0.45 m)까지 같이 내려가 **눈높이 1.5 → 약 0.35 m** (4.5절).
- 소리: `slideSound` 볼륨 0.3 + 바닥 재질 `SlideOneShot` 0.5(로컬은 2D) (직렬화:sharedassets0/PlayerAvatar.slideSound, MaterialPreset.SlideOneShot; 코드:PlayerAvatar.cs:1080~1090).

### 1.5 웅크림·기기

- 웅크림은 **접지 상태에서만** 시작(코드:PlayerController.cs:732). 최소 유지 `CrouchTimeMin 0.5 s`, 일어선 뒤 다시 웅크리기까지도 0.5 s (직렬화:level0/PlayerController.CrouchTimeMin; 코드:PlayerController.cs:734~758).
- 웅크리면 충돌 캡슐 전체가 y 0.25배 → **높이 0.425 m(42.5 cm)**. 즉 REPO의 "웅크림" 충돌은 기는 높이다 (직렬화:level0/Collision Crouch Transform.localScale = (1, 0.25, 1); 코드:PlayerCollision.cs:20~27, 카메라 하강이 절반 넘었을 때 적용).
- 서기 판정 캡슐(반지름 0.19, 높이 1.7)이 막히면 `Crawling` (코드:PlayerController.cs:723~731, PlayerCollisionStand.cs:27~44).
- 속도: 웅크림·기기 모두 1 m/s(100 cm/s).

### 1.6 점프

| 항목 | 값 | 출처 |
|---|---|---|
| 점프 임펄스 | `JumpForce 17` − 현재 세로속도, Impulse 모드 → 질량 2라서 **초기 상승속도 8.5 m/s (850 cm/s)** | 직렬화:level0/PlayerController.JumpForce, 코드:PlayerController.cs:924~930 |
| 공중 추가 중력 | `CustomGravity 30` × dt를 Impulse로 → **+15 m/s²**, 총 **24.81 m/s² (2481 cm/s², 2.53 g)** | 직렬화 CustomGravity, 코드:PlayerController.cs:898~908 |
| 정점 높이 | 연속식 1.46 m, 50 Hz 이산 재현 **1.37 m (137 cm)** | 수식 재현 |
| 정점 도달 / 체공 | 0.34 s / **0.68 s** (평지) | 수식 재현 |
| 코요테 | 접지 후 0.25 s(`JumpGroundedBuffer`) + 접지 판정 자체의 0.1 s 유예 | 코드:PlayerController.cs:1067~1080, PlayerCollisionGrounded.cs:114~124 |
| 선입력 버퍼 | 0.25 s (`JumpInputBuffer`) | 코드:PlayerController.cs:1053 |
| 쿨다운 | 0.1 s, 점프 직후 접지 판정 0.1 s 끔 | 코드:PlayerController.cs:931~935 |
| 공중 점프 | 업그레이드 단계 수만큼 추가 점프(파워업 이펙트·별도 소리) | 코드:PlayerController.cs:369, 1089~1100 |
| 들고 있는 물체 위에서 점프 | 그 물체를 놓고 점프 힘 절반 | 코드:PlayerController.cs:909~929 |
| 접지 판정 | 발 위 0.1 m의 반지름 0.15 m 트리거 구 OverlapSphere | 직렬화:level0/Collision Grounded SphereCollider, 코드:PlayerCollisionGrounded.cs:67~127 |

**착지 이벤트**: 세로속도가 −4 m/s 아래로 내려간 적이 있으면 다음 접지 때 착지 처리(카메라 착지 + 소리 + 적 조사 반경 10 m), 쿨다운 1 s (코드:PlayerController.cs:1111~1132, PlayerAvatar.cs:1147~1180). 총중력 24.81이면 −4 m/s는 낙하 0.16 s·0.32 m에 도달하므로 **모든 점프와 32 cm 넘는 낙하는 착지 연출을 낸다**(수식 재현). 착지 지점이 물리 물체면 그 물체에 중간 파손 임펄스를 주고 소리는 생략(코드:PlayerAvatar.cs:1153~1161).

**낙하 피해**: 누적 낙하거리 ≥ 8 m면 강제 텀블 + 충돌 시 피해. 8~9 m 5, 9~13 m 15, 13~17 m 25, 17~21 m 50, 21 m 초과 100 (코드:PlayerAvatar.cs:759~790).

### 1.7 공중 제어·계단

- **공중 제어 = 지상과 동일(100 %)**. 수평 속도 계산(1.1절)에 접지 조건이 없다(코드:PlayerController.cs:771~897).
- 계단 오르기: `StepUpForce`는 직렬화 7이지만 **코드 어디에서도 읽지 않는다**(선언 코드:PlayerController.cs:136 외 참조 0건, 전체 grep). 별도 스텝업 로직이 없고, 캡슐 + 이동 중 마찰 0 재질에 의존한다. 레벨 쪽 경사·계단 충돌체(`PlayerSlope` 재질 0/0 Maximum이 resources.assets에 있음)는 조사하지 않았다.

### 1.8 텀블(스스로 공이 되는 래그돌)

- 텀블 키 → 플레이어가 질량 2, 각항력 1의 물리 몸(`Player Tumble`)으로 바뀐다(직렬화:sharedassets0/Player Tumble Rigidbody). 진입 순간 플레이어 속도를 VelocityChange로 넘기고 진행방향 직각축으로 회전 속도 2배를 준다(코드:PlayerTumble.cs:340~351).
- 텀블 몸 추가 중력 `customGravity 10`(Force 모드, 질량 2 → +5 m/s²) (직렬화 PlayerTumble.customGravity, 코드:PlayerTumble.cs:370~373).
- 입력 차단: 텀블 중 이동 입력 0. 풀기는 점프/텀블 키, 또는 0.5 s 이상 멈춘 상태에서 이동 키 (코드:PlayerController.cs:1137~1149). 토글 직후 1 s 재토글 불가(`tumbleInputDisableTimer`, 코드:PlayerTumble.cs:695).
- 2 s 이상 멈추면 몸이 카메라 방향으로 서서히 돌아온다(코드:PlayerTumble.cs:391~407).
- 4 m/s 넘게 굴러가면 적을 해치는 판정이 켜짐(코드:PlayerTumble.cs:220~240). 다른 사람에게 잡혀 있을 때 점프로 탈출: 정면 힘 15 + 토크 10, 쿨다운 0.5 s (코드:PlayerTumble.cs:278~298).
- 텀블끼리 충돌: 양쪽 0.15 s 정지 후 ±5 밀어냄(코드:PlayerTumble.cs:496~531).
- 적 피격 대부분이 이 텀블로 이어진다(1.9절).

### 1.9 외부 임펄스(피격 밀림)

- 적 `HurtCollider`가 텀블을 걸지 않는 경우 `playerHitForce`를 `PlayerController.ForceImpulse`로 넣는다(코드:HurtCollider.cs:765~768). 이 값은 질량으로 나누지 않고 1단 목표속도에 **그대로 더해져** 같은 지연 필터를 탄다(코드:PlayerController.cs:390~393, 870~872).
- 수식 재현: 임펄스 10 → 0.12 s에 최고 3.87 m/s(387 cm/s), 30 → 11.6 m/s, 50 → 19.4 m/s. 즉 맞은 순간이 아니라 0.1 s 남짓 뒤에 밀림이 최고가 된다(이후 감속은 정지 판정·마찰에 따름, 이동거리는 마찰을 넣지 않아 산출하지 않음).
- 텀블을 거는 피격은 `playerTumbleForce`(Impulse, 텀블 몸 질량 2 → Δv = 힘/2)와 `playerTumbleTorque`, 지속 `playerTumbleTime`. 적 프리팹 표본(직렬화:resources.assets/Enemy - * HurtCollider): 피해 0~100, 히트포스 0~50, 텀블 힘 0~50, 토크 0~40, 텀블 시간 0~3 s. 예: Runner 10 / 2 / 15 / 5 / 0 s, Beamer 근접 10 / 30 / 30 / 20 / 3 s, Slow Walker 내려찍기 20 / 50 / 50 / 40 / 2 s.

### 1.10 무거운 물건을 들고 움직일 때

- **이동속도 페널티 없음.** `PlayerController`의 속도 계산에 잡기 상태가 들어가지 않고(코드:PlayerController.cs:771~897), 외부에서 `OverrideSpeed`·`MoveMult`를 부르는 곳은 시간 유리병·지도 도구뿐이다(전체 grep: `ValuableWizardTimeGlass`, `MapToolController`).
- 잡은 물체에 걸리는 힘은 물체에만 작용하고 플레이어에게 반작용이 없다(텀블 중 제외, 코드:PhysGrabObject.cs:1146~1150). 그래서 무거운 물체는 빔 끝에서 **처지고 뒤처진다**. 눈↔잡은 점 거리가 `max(grabRange×2, 10)` = **10 m(1000 cm)**를 넘으면 강제로 놓친다(코드:PhysGrabber.cs:1656, 1676).
- 수레: 손잡이를 잡으면 수레 질량 4로 덮고 플레이어 앞 2~2.5 m(질주 중 앞으로 갈 땐 3~4 m) 지점을 향해 최대 5 m/s로 끌려온다 — 질주하며 밀 수 있다(코드:PhysGrabCart.cs:346~407).

---

## 2. 스태미나(에너지)

| 항목 | 값 | 출처 |
|---|---|---|
| 최대 | **40** (+10/업그레이드) | 직렬화:level0/PlayerController.EnergyStart, 코드:PlayerController.cs:368 |
| 질주 소모 | **10 /s** × `SprintSpeedLerp`(0.1 s에 1) (+속도 업그레이드 단계) → 만충 기준 **질주 4.0 s** | 직렬화 EnergySprintDrain, 코드:PlayerController.cs:710~722 |
| 회복 지연 | 질주를 멈춘 뒤 **1 s** (아레나 0.5 s) | 코드:PlayerController.cs:154, 1020~1031 |
| 회복 속도 | **2 /s** → 0에서 만충까지 1 + 20 = **21 s** | 코드:PlayerController.cs:156, 1032~1047 |
| 웅크림·기기 추가 회복 | 업그레이드 0단계에서도 **+1 /s**(움직이면 0.5 /s), 슬라이드 중 제외 → 웅크려 가만히 있으면 만충 ≈14.3 s | 코드:PlayerAvatar.cs:383~412 (`upgradeCrouchRest + 1`) |
| 슬라이드 | −5 1회 | 코드:PlayerController.cs:809~813 |
| 텀블 등반(업그레이드) | 시작 시 −round(40/40)=−1, 유지 중 −round(40/8)=−5 /s, 등반 중 자연회복 0 | 코드:PhysGrabber.cs:465~522, PlayerController.cs:1039~1042 |
| 탈진 | 0이 되면 질주 불가·질주 토글 해제뿐. **재질주 문턱이 없다** — 1 s 뒤 2 /s로 차오르는 즉시 다시 질주 가능. 탈진 전용 소리·감속·화면 효과는 코드에서 찾지 못함 | 코드:PlayerController.cs:685, 717~720 |

---

## 3. 시점(Look)

### 3.1 감도·스무딩·클램프

| 항목 | 값 | 출처 |
|---|---|---|
| 마우스 원값 배율 | `InputManager.mouseSensitivity`를 매 프레임 **0.05**로 덮어씀(코드 기본 0.1) | 코드:CameraAim.cs:147, InputManager.cs:43, 570~581 |
| 감도 슬라이더 | `AimSpeedMouse = Lerp(0.2, 4, 설정/100)`, 기본 설정 **35** → 1.53 | 코드:CameraAim.cs:141~142, DataDirector.cs:193 |
| 결과 | 마우스 델타 1당 **0.0765°** (설정 범위 0.01~0.2°). 직렬화된 `AimSpeedMouse 1.25`는 매 프레임 덮어써져 무의미 | 수식 재현 |
| 피치 클램프 | **위 70° / 아래 80°** (`Clamp(-70, 80)`) | 코드:CameraAim.cs:177 |
| 스무딩 | 지수 추종률 = `Lerp(50, 8, 카메라스무딩/100)`, 기본 **80 → 16.4 /s**: 시상수 61 ms, 50 % 42 ms, **90 % 140 ms**. 설정 0이면 스무딩 없음 | 코드:CameraAim.cs:140, 230, 248~249, DataDirector.cs:195 |
| 게임패드 시점 | 읽은 직후 0으로 덮어써 **무효** (디컴파일상) | 코드:CameraAim.cs:149~150 |
| 무거운 물건 잡을 때 시점 둔화 | **없음.** `OverrideLookSpeed` 호출은 `ValuableWizardTimeGlass` 한 곳(0.5배) | 전체 grep |
| 물체 회전 중 | 마우스가 물체 회전으로 넘어가고 카메라 시점은 멈춤(`CameraOverrideStopAim`, 0.2 s 유지) | 코드:PhysGrabber.cs:1371~1381, CameraAim.cs:107~110 |

### 3.2 강제 시선(시점 탈취)

`CameraAim.AimTargetSet(위치, 시간, 속도)` — 진행값을 `속도×dt`로 올려 `AimTargetCurve`로 보간. 곡선은 시작에 −1.6 % 당겼다가 0.745에서 **+2.2 % 넘치고** 1로 안착(직렬화:level0/CameraAim.AimTargetCurve, 가중 접선 재현). 사용처: 사망 시 죽인 적을 1 s 동안 속도 80(≈12 ms에 완료)으로 확 돌려봄(코드:PlayerAvatar.cs:710), 스폰 때 트럭 화면을 0.25 s에(코드:PlayerAvatar.cs:634), 적 붙잡기·천장눈 등 적 12종(전체 grep). **적에게 당할 때 시선이 위협 쪽으로 끌려간다.**

---

## 4. 카메라 레이어

### 4.1 리그 구조 (부모 → 자식, 각 Transform 스케일 1 확인)

`Camera Controller`(FOV) / `Position`(몸 따라가기) / `Crouch Position` / `Crawl Position` / `Aim`(시점) / `Crouch Rotation` / `Crouch Noise` / `Bob` / `Noise` / `Aim Offset` / `Jump / Land` / `Shake` / `Impact` / `Tilt` / `Camera Main` / `Glitch`
(직렬화:level0 각 MonoBehaviour의 go_path와 Transform 확인). **효과 하나 = Transform 한 단계**인 가산 스택이다. 위치형(앉기·기기)은 시점 회전 바깥(부모), 흔들림형(보브·노이즈·점프·셰이크·틸트)은 시점 안쪽(자식)이라 보는 방향 기준으로 흔들린다.

| 레이어 | 동작 | 실값 | 출처 |
|---|---|---|---|
| Position | 컨트롤러 위치 + (0, **1.5 m**, 0)으로 매 프레임 `30·dt` 비율 추종(≈33 ms). 텀블 전환 후 0.5 s는 15 | `playerOffset (0,1.5,0)`=150 cm, `positionSmooth 30` | 직렬화:level0/CameraPosition, 코드:CameraPosition.cs:32~65 |
| FOV | 아래 4.4 | | |

### 4.2 보브(걸음 흔들림) — 발소리와 같은 시계

- 위상 속도 = `bobUpLerpSpeed 2` × 활성도 × 상태배율 → **걷기 2걸음/s, 질주 3걸음/s(×1.5, 속도 업그레이드당 +0.1), 웅크림 1걸음/s(×0.5)** (직렬화:level0/CameraBob, 코드:CameraBob.cs:101~116). 보폭: 걷기 1 m, 질주 1.67 m, 웅크림 1 m (속도÷걸음수).
- 상하: `0.15 × bobUpCurve(위상)` — 곡선은 0에서 솟아 위상 0.415에 **0.577**, 0.855에 −0.029 후 0 → **최대 +8.7 cm 위로**, 바닥 −0.4 cm. 웅크림은 진폭 ×0.5(4.3 cm). 즉 한 걸음마다 카메라가 **위로 튀었다 내려온다**(대칭 사인 아님).
- **위상이 1을 넘는 순간 발소리**: 질주=Heavy, 걷기(활성도>0.75, 웅크림 아님)=Medium, 그 외(출발 직후·웅크림)=Light. 접지 중이고 점프 카메라 연출(1 s) 중이 아닐 때만 (코드:CameraBob.cs:116~134). → **화면 보브와 발소리가 같은 위상에 묶여 있다.**
- 좌우: `±0.075 × bobSideCurve`가 왕복(걷기 1 s 주기)하여 **yaw ±0.75°, roll ±0.375°** (코드:CameraBob.cs:143~170).
- 켜짐/꺼짐: 활성도 0→1에 0.667 s(`ActiveLerpSpeedIn 1.5`, 곡선 0.745에서 1.032로 살짝 넘침), 1→0에 1 s. 즉 걷기 시작 후 보브·발걸음 박자가 서서히 붙는다.
- "움직임" 판정은 1단 목표속도(입력) 기준 > 0.1 m/s → 벽에 대고 걸어도 보브가 돈다(코드:PlayerController.cs:938~951).
- 설정 `Camera Animation`(기본 4 → 1.0) 배율. 곡선 키는 직렬화된 CameraBob 값에서 읽었고 평가값은 수식 재현으로 얻었다.

### 4.3 틸트(회전·스트레이프 롤)

| 항목 | 값 | 출처 |
|---|---|---|
| 회전 롤 | yaw 각속도(°/s) × **0.03** + 좌우 입력 × **2°** | 직렬화:level0/CameraTilt.tiltZ, strafeAmount |
| 피치 끄덕임 | pitch 각속도(°/s) × **0.05** | 직렬화 tiltX |
| 상한 | ±10° | 직렬화 tiltZMax/tiltXMax |
| 추종 | 목표로 `3·dt` Slerp(시상수 0.33 s), 목표가 정확히 0일 때만 10 | 코드:CameraTilt.cs:81~86 |
| 웅크림 | ×0.5 | 직렬화 CrouchMultiplier |

예: 200°/s로 돌면 6° 롤, 옆걸음만 해도 2° 롤 (수식 재현). 부호(어느 쪽으로 기우는지)는 확인하지 않았다.

### 4.4 FOV(줌)

| 상황 | 값 (Unity 세로 FOV) | 16:9 가로 FOV 환산 | 출처 |
|---|---|---|---|
| 기본 | **70°** (두 카메라 모두 70) | 102.4° | 직렬화:level0/CameraZoom.playerZoomDefault, Camera(pid 1360/1363).field of view |
| 질주 | **+20°** × `SprintSpeedLerp`(0.1 s에 1), 목표로 `2·dt` 추종 → 0.5 s 63 %, 1 s 86 %, 1.5 s 95 %. 놓으면 같은 속도로 복귀. 속도 업그레이드당 +2° | 90° → 121.3° | 직렬화 SprintZoom 20, 코드:CameraZoom.cs:106~116 |
| 텀블 | 55°로 오버라이드(1 s 커브 전환) + 텀블 속도 × 5(최대 +30°)를 `2·dt`로 | 55° → 85.6° (+속도) | 코드:PlayerTumble.cs:216, CameraZoom.cs:117~127 |
| 오버라이드 전환 커브 | 0.275에서 −7 % 당겼다가 0.67에서 **+6.5 % 넘침** | | 직렬화 OverrideZoomCurve(가중 접선) |
| 범위 | 5~170° | | 직렬화 Min/MaximumZoom |

웅크림·착지에 따른 FOV 변화는 없다(코드에서 찾지 못함).

### 4.5 웅크림·기기 카메라

| 레이어 | 이동 | 전환 시간 | 커브 특징 | 출처 |
|---|---|---|---|---|
| Crouch Position | **−0.70 m (−70 cm)** | 0.5 s (`PositionSpeed 2`, 슬라이드 중 0.25 s) | 처음 0.2까지 −4 % 반대로 살짝 뜬 뒤 내려가 0.81에서 **105 %(−73.7 cm)** 넘치고 안착 | 직렬화:level0/CameraCrouchPosition |
| Crawl Position | 추가 **−0.45 m (−45 cm)** | 0.667 s (`1.5`, 슬라이드 중 0.33 s) | 0.24에서 −7 % 반동 후 0.66에서 **107 %** | 직렬화:level0/CameraCrawlPosition |
| Crouch Rotation | `Rotation 10°` × 커브 | 0.667 s (`RotationSpeed 1.5`) | 앉을 때 최대 **−4.2°**(위로 고개), 설 때 최대 **+5.9°**(아래로 끄덕) 후 0 | 직렬화:level0/CameraCrouchRotation |
| Crouch Noise | 웅크림 동안 회전 ±0.5° × 0.5 = **±0.25°**, 구간당 2.5~5회/s 떨림 | 진입 `LerpSpeed 5` | | 직렬화:level0/CameraCrouchNoise + AnimNoise(Crouch Noise) |

눈높이: 서기 1.5 m(150 cm) → 웅크림 0.8 m(80 cm) → 기기·슬라이드 0.35 m(35 cm). 전환 순간마다 셰이크(웅크림 2/0.1 s, 기기 1/0.1 s, 코드:CameraCrouchPosition.cs:61, CameraCrawlPosition.cs:63)와 소리(6절).

### 4.6 대기 노이즈(숨쉬기)

`CameraNoise.StrengthDefault 0.2` × 설정(기본 1.0)이 `AnimNoise` 강도가 된다. 축마다 무작위 목표로 0.5~1 구간/s 속도로 S자 보간: **회전 ±0.1°**(±0.5×0.2), **위치 ±2 cm**(±0.1 m×0.2) (직렬화:level0/CameraNoise, AnimNoise(Noise); 코드:AnimNoise.cs:148~217, CameraNoise.cs:23~43). 가만히 서 있어도 화면이 미세하게 산다.

### 4.7 점프·착지 카메라

| | 위치 | 회전 | 길이 | 커브 | 출처 |
|---|---|---|---|---|---|
| 점프 | (0, **−5 cm**, +2.5 cm) | 피치 **+5°**(Unity X+ = 아래로) | 1 s (`jumpSpeed 1`) | 0.285에 98 %, 0.6 근처 0 통과, 0.665에 **−16.5 % 반동**, 1에 0 | 직렬화:level0/CameraJump |
| 착지 | (0, **+5 cm**, +2.5 cm) | 피치 +5° | 0.667 s (`landSpeed 1.5`) | 같은 커브 | 직렬화:level0/CameraJump |

결과에 다시 `30·dt` lerp가 걸려 ≈33 ms 부드러워진다(코드:CameraJump.cs:101~106). 착지 연출은 진행 중이면 재시작하지 않는다(코드:CameraJump.cs:50).

### 4.8 셰이크 두 채널

두 개의 `CameraShake`가 있다. `GameDirector.CameraShake` = `Shake`(pid 2842), `GameDirector.CameraImpact` = `Impact`(pid 3028) (직렬화:level0/GameDirector).

| | Shake | Impact | 출처 |
|---|---|---|---|
| 동작 | 축마다 무작위 목표 ±강도로, 구간 길이 1/(Speed×0.8~1.2) 동안 커브 보간 | 같은 방식이지만 **새 흔들림을 즉시 적용**(`InstantShake`) | 코드:CameraShake.cs:111~187 |
| Speed | 6 (구간 ≈0.14~0.21 s) | 20 (≈0.04~0.06 s) | 직렬화 |
| 회전 / 위치 배율 | 강도 1당 **±0.5°** / **±0.2 cm** | 같음 | 직렬화 RotationMultiplier 0.5, PositionMultiplier 0.002 |
| 감쇠 | 지속시간 뒤 **10 /s** | **30 /s** | 직렬화 StrengthLoss |
| 상한 | 20 | 20 | 직렬화 StrengthMax |
| 규칙 | 새 강도가 현재보다 클 때만 **더함**(대체 아님), 지속시간은 큰 쪽 | | 코드:CameraShake.cs:75~85 |

**이벤트별 프리셋** (강도/지속 s; `Distance`형은 최소·최대 거리 m 사이에서 선형 감소):

| 이벤트 | Impact | Shake | 출처 |
|---|---|---|---|
| 점프 | 1 / 0.05 | 2 / 0.1 | 코드:CameraJump.cs:41~42 |
| 착지 | 1 / 0.05 | 2 / 0.1 | 코드:CameraJump.cs:54~55 |
| 웅크림 진입·해제 | — | 2 / 0.1 | 코드:CameraCrouchPosition.cs:61 |
| 기기 진입·해제(슬라이드 포함) | — | 1 / 0.1 | 코드:CameraCrawlPosition.cs:63 |
| 잡기 시작 / 놓기 | 0.5 / 0.1 | — | 코드:PhysGrabber.cs:1751~1767 |
| 텀블 진입·해제 | 1 / 0.1 | 2 / 0.5 | 코드:PlayerTumble.cs:696~697 |
| 텀블 충돌(플레이어끼리) | 5 / 0.1 (5~15 m) | 3 / 0.5 (5~15 m) | 코드:PlayerTumble.cs:524~525 |
| 피격 < 25 | 3 / 0.2 | 3 / 0.5 | 코드:CameraGlitch.cs:144~166 |
| 피격 ≥ 25 | **5** / 0.2 | 3 / 0.5 | 코드:CameraGlitch.cs:120~142, PlayerHealth.cs:422~429 |
| 회복 소/대 | 1.5 / 0.2, 2.5 / 0.2 | 1.5 / 0.2 | 코드:CameraGlitch.cs:168~186 |
| 근접 무기 명중 | 5 / 0.1 (3~10 m) | — | 코드:ItemMelee.cs:1419 |
| 물체 파손 대/중/소 | 5 / 3 / 1, 0.1 (1~6 m) | — | 코드:PhysGrabObjectImpactDetector.cs:980, 997, 1016 |
| 물체 소멸 | 10 / 0.1 (1~6 m) | — | 코드:PhysGrabObjectImpactDetector.cs:680 |
| 총 발사(멀어질수록) | 5×배율 / 0.1 | 0.1×배율 | 코드:ItemGun.cs:369~370 |

예: 점프 = Impact ±0.5° 순간 튐(≈0.08 s) + Shake ±1° 부드러운 흔들림(≈0.3 s). 피격 ≥25 = Impact ±2.5° 순간 + Shake ±1.5°가 0.5 s 유지 후 감쇠. 설정 `Camera Shake`(기본 100) 배율, 관전 중엔 0.5/0.1배(코드:GameplayManager.cs:65~81).

### 4.9 피격 화면

피격 시 `CameraGlitch`가 애니메이터 트리거(HurtShort/HurtLong)로 화면 글리치 오버레이 + 전용 소리 + 위 셰이크. 아바타 재질이 빨갛게 번쩍(`materialEffectLerp` 초당 2.5 → 0.4 s) (코드:PlayerHealth.cs:274~300, 398~436). 피격 후 무적 시간은 `Hurt` 경로에 없다(가해 측 `HurtCollider.playerDamageCooldown` 기본 0.25 s, 코드:HurtCollider.cs:53). "구사일생": 피해 ≤25이고 체력 6~20이면 죽지 않고 1~4 남김(코드:PlayerHealth.cs:404~411). 글리치 애니메이션 클립 내용은 읽지 않았다.

### 4.10 다른 플레이어에게 보이는 몸(참고)

아바타 시각 스프링이 전부 **감쇠비 0.5(과소감쇠)**, 고유진동수 10~15 rad/s: 기울기(lean)·틸트·몸통·다리 비틀림·고개 (직렬화:sharedassets0/PlayerAvatar/Player Visuals PlayerAvatarVisuals.leanSpring 등; `SpringQuaternion`의 `damping`=감쇠비, `speed`=고유진동수 — 코드:SemiFunc.cs:3147~3280). 이동 방향으로 ±3° 기울고, 질주 시작·멈춤 순간 기울기 목표를 2~3배로 0.25 s 튕긴다(코드:PlayerAvatarVisuals.cs:472~522). 동료가 **출렁거려 보이는** 근원이다. 1인칭 로컬 화면 감각과는 별개.

---

## 5. 상호작용

### 5.1 물리 잡기(PhysGrabber → PhysGrabObject)

| 항목 | 값 | 출처 |
|---|---|---|
| 잡기 사거리 | **2.5 m (250 cm)**, 범위 업그레이드당 +1 m | 직렬화:sharedassets0/PhysGrabber.grabRange, 코드:PhysGrabber.cs:755, 2086 |
| 잡는 거리(빔 길이) | **1 ~ 2.5 m** (`min(grabRange, maxDistanceFromPlayer 6)`) | 코드:PhysGrabber.cs:1548 |
| 휠 밀고 당기기 | 휠 한 칸(입력이 0이 아닌 프레임)당 **±0.2 m (20 cm)** → 1~2.5 m를 7.5칸 | 코드:PhysGrabber.cs:1527~1538, InputManager.cs:420~ |
| 강제 놓침 | 눈↔잡은 점 > 10 m | 코드:PhysGrabber.cs:1656 |
| 탐색 주기 | 0.02 s 레이 | 코드:PhysGrabber.cs:1633~1636 |

**힘 모델**(물체 쪽, 마스터에서, 코드:PhysGrabObject.cs:1029~1316):
- 변위 d(목표점−잡은 점)를 `min(10d, 6)`으로 포화(**0.6 m에서 포화**), 여기에 0.6을 곱하고 잡은 점 속도×0.3을 빼서 다시 6으로 자른 뒤 `×2/질량 × grabStrength × 5`를 **가속도(Acceleration) 모드로 잡은 지점에** 건다. 잡은 점이 목표 10 cm 안이면 힘을 비례로 줄인다. 이전 프레임 힘과 80 % 섞어 튐을 줄이고, 잡혀 있는 동안 물체 속도를 매 물리 프레임 0.98배(≈초당 1.0 감쇠).
- 정지 상태 최대 가속 = **36/질량 m/s²**, 처지는 물체(아래로 움직이는 중)는 감쇠항까지 **60/질량**.
- 결과(수식 재현, `grabStrength 1`):

| 질량(프리셋) | 정적 처짐 | 혼자 들기 |
|---|---|---|
| 0.5 Light | 8 cm | 가능 |
| 1 Medium | 16 cm | 가능 |
| 2 Medium++ | 33 cm | 가능 |
| 3 / 3.5 | 49 / 57 cm | 가능(겨우) |
| **4 이상** Heavy | — | **불가**(최대 9 m/s² < g). 바닥을 끌거나 2인 이상 |
| 8 Very Heavy | — | 2인(합 9 m/s²)도 불가, 3인 이상 또는 근력 업그레이드 |

  처짐 ≈ **질량 1 kg당 16.4 cm**, 혼자 들 수 있는 한계 **3.67 kg**. 근력 업그레이드 1단계(+0.2)는 질량 구간별 완충(질량 <2는 7, 그 이상 20으로 나눔) 때문에 실효 +18~19 % 정도(코드:PhysGrabObject.cs:1096~1121).
- 귀중품 질량 프리셋(`PhysAttribute`, 직렬화): Light 0.5 / Light+ 0.65 / Light++ 0.8 / Medium 1 / Medium+ 1.5 / Medium++ 2 / Medium+++ 3 / Medium++++ 3.5 / Heavy 4 / Heavy+ 5 / Heavy++ 6 / Very Heavy 8. 귀중품 프리팹 이름 299종 기준 분포: 0.5 72, 0.65 18, 0.8 10, 1 63, 1.5 7, 2 25, 3 23, 3.5 13, 4 32, 5 16, 6 12, 8 8 → **약 23 %가 혼자 못 드는 4 이상**(같은 이름 중복 제거, 디버그 모듈 포함 여부는 이름으로만 걸렀음).
- 물체 재질: `PhysGrabObject` 동 0.25 / 정 0.05 / **반발 0.3**, 일부 `PhysGrabObjectBouncy` 반발 0.75 (직렬화 PhysicMaterial). 물체 속도 상한 40 m/s, 각속도 30 rad/s (코드:PhysGrabObject.cs:953~964).

**던지기**: 전용 버튼이 없다. 놓는 순간 `clamp(목표점 − 잡은 점, 6) × max(1.5×질량, 1) × (0.5 + throwStrength)` 임펄스(코드:PhysGrabObject.cs:1924~1930). `throwStrength` 실값 0(업그레이드당 +0.3) → 질량 ≥0.67에서 **Δv ≈ 0.75 × 지연거리(m)** m/s. 즉 던지기 속도는 대부분 **마우스·몸으로 휘두르며 스프링이 이미 실어 준 물체 속도**에서 나오고, 놓을 때 보너스는 작다.

**회전**: 회전 키를 누른 채 마우스 → 회전 속도가 쌓이고(×15, 상한 70) 누르는 동안 초당 1, 놓으면 초당 10으로 줄어든다. 목표 회전은 실제보다 45° 이상 앞서지 못하고, 무거울수록 1/질량으로 둔해진다(코드:PhysGrabber.cs:1212~1350). 회전 중 물체 각항력 `40 × min(1/질량, 0.5)`.

**잡기 피드백**: 시작·놓기에 Impact 0.5/0.1 s + 소리(시작 0.2, 루프 0.05, 끝 0.2; 3D) (직렬화:sharedassets0/PhysGrabber.startSound/loopSound/stopSound, 코드:PhysGrabber.cs:1751~1767).

### 5.2 아이템 장착·사용 (코드 상수 — 직렬화 필드 아님)

인벤토리 3칸(키 1~3). 장착 시 0.1 s에 크기 0.01배로 줄어 물리에서 빠짐, 해제 0.2 s에 복원, 해제 직후 1.5 s 자동 다시 잡기, 해제 타이머 0.4 s (코드:ItemEquippable.cs:440~451, 501, 543~549, 329). 토글형 아이템은 Interact 키(`ItemToggle`).

### 5.3 근접 무기(ItemMelee) — 휘두르는 것도 물리

| 항목 | 값 | 출처 |
|---|---|---|
| 잡기 힘 덮어쓰기 | 근접무기를 쥐면 최소 grabStrength **17**(+근력 보너스 최대 42) → 커서를 바짝 따라옴, 미끄러운 재질로 교체 | 코드:ItemMelee.cs:926~964 |
| 휘두름 판정 | 약 0.1 s 간격 표본 사이 Swing Point 변위 > **0.8 m × 무기 배율**(쥔 상태; 바닥이면 0.4) → 대략 8 m/s 이상 | 코드:ItemMelee.cs:913~924, 992~1022 |
| 무기별 배율 `swingDetectSpeedMultiplier` | 풍선망치 0.7, 슬레지 0.8, 스턴봉 0.8, 프라이팬 1.0, 검 1.15, 지팡이 3종 1.15, 야구방망이 1.2 | 직렬화:resources/Item Melee * ItemMelee |
| Swinging 유지 | 0.2 s, 계속 빠르면 갱신, 처음 방향과 같은 쪽으로 계속 가면 조기 종료 | 코드:ItemMelee.cs:462~542, 1115~1130 |
| 명중 | 무기 속도·각속도 즉시 0 → **0.06 s 뒤 `hitFreeze 0.2 s` 동안 무기 위치 고정**(게임 전체 정지 아님), 명중 쿨다운 0.2 s, Impact 셰이크 5(3~10 m) | 직렬화 hitFreeze 0.2(9종 전부), 코드:ItemMelee.cs:636~646, 1361~1420 |
| 바닥 타격 | 소리 피치 2·볼륨 0.5, 파티클 별도, 정지 없음 | 코드:ItemMelee.cs:1389~1394 |
| 휠 휘두르기 | 휠 한 칸에 Swing Point에 즉시 **5 m/s**(VelocityChange, `scrollSwingForce 5`; 지팡이 2) 한 번, 0.05 s 뒤 반대로 한 번 → "찍었다 돌아오기". 쿨다운 0.2 s | 코드:ItemMelee.cs:1132~1215 |
| 무기 자세 | 앞으로 `forwardTilt 45°`(지팡이 90°) 기울여 들고, 휘두르는 방향으로 날을 돌림(`turnWeaponStrength` 1~40) | 직렬화, 코드:ItemMelee.cs:890~911, 1217~1277 |
| 무기 질량 | 대부분 0.5, 풍선망치 1.0 | 직렬화 Rigidbody |
| 내구도 | 물체 타격 2.5(코드 상수), 적·PVP `durabilityDrainOnEnemiesAndPVP` 0.5(풍선)~11(슬레지) | 직렬화, 코드:ItemMelee.cs:96 |

---

## 6. 조작과 묶인 소리

| 소리 | 언제 | 볼륨(±무작위) / 피치 무작위 | 출처 |
|---|---|---|---|
| 발소리 | 보브 위상이 1을 넘을 때(4.2): 걷기 **2회/s**, 질주 **3회/s**, 웅크림 **1회/s**. 질주 Heavy·걷기 Medium·출발/웅크림 Light | 재질별 예: 나무 0.012 / 0.075 / 0.2, 타일 0.07 / 0.12 / 0.247(피치 3), 금속 0.07 / 0.071 / 0.195, 환기구 0.517 / 0.689 / 0.818. 피치 ±0.1. 일부 재질은 10~20걸음마다 "희귀 발소리" | 직렬화:sharedassets0/MaterialPreset(17종), 코드:Materials.cs:77~216 |
| 발소리 파티클 | 카메라 15 m 안, 동시 25개 미만 | | 코드:Materials.cs:205~215 |
| 다른 플레이어 발소리 | 볼륨 ×0.5 | | 코드:Materials.cs:88~92 |
| 소리 = 위험 | 질주 발소리 → 적 조사 반경 5 m, 걷기 1 m, 착지 10 m | | 코드:PlayerAvatar.cs:1164, 1203~1213 |
| 점프 | `jumpSound` + 해당 재질 Light 발소리 | 0.15(±0.05) / ±0.1 | 직렬화:sharedassets0/PlayerAvatar.jumpSound, 코드:PlayerAvatar.cs:1113~1131 |
| 공중 점프 | `extraJumpSound` 추가 | 0.15 | 직렬화 |
| 착지 | `landSound` + 재질 Heavy 발소리 | 0.15(±0.05) | 직렬화, 코드:PlayerAvatar.cs:1171~1172 |
| 슬라이드 | `slideSound` + 재질 `SlideOneShot` | 0.3(±0.1) + 0.5 | 1.4절 |
| 앉기/서기 | standToCrouch / crouchToStand (질주 중엔 앉는 소리 생략) | 0.1 | 직렬화, 코드:PlayerAvatar.cs:1217~1232 |
| 기기 진입/해제 | crouchToCrawl / crawlToCrouch (슬라이드·질주 중 생략) | 0.3 | 직렬화, 코드:PlayerAvatar.cs:1234~1254 |
| 텀블 | 시작 0.3, 끝 0.3, 구르는 루프 0.05(속도 따라 피치), 발사 0.4, 충돌 0.3, 탈출 0.15 | | 직렬화:sharedassets0/PlayerAvatar, Player Tumble |
| 낙하 바람 | 텀블 속도 > 6 m/s가 0.5 s 넘거나 > 8 m/s면 루프, 볼륨 초당 0.75로 차오름, 피치 = clamp(속도/20, 0.8, 1.25) | 0.3 | 직렬화:level0/PlayerCollisionController.soundFallLoop, 코드:PlayerCollisionController.cs:74~113 |
| 잡기 | 시작 0.2, 루프 0.05, 끝 0.2 | | 5.1절 |
| 근접 휘두름 | 루프: 속도 3 m/s 넘으면 재생, 볼륨 0→0.2(속도 3→20 m/s), 피치 1→10(속도·시야 밖 각도·회전속도). 원샷 "휙": 방향이 꺾일 때마다 또는 180° 회전마다, 0.3 s 안에 연속이면 피치 2배 | | 코드:ItemMelee.cs:270~311, 1032~1102 |
| 피격 | HurtShort/HurtLong + 글리치 소리, 원격은 `hurtOther` 0.2 | | 코드:CameraGlitch.cs:120~166, 직렬화 PlayerHealth.hurtOther |

모든 `Sound`는 재생마다 피치 ±`PitchRandom`, 볼륨 ±`VolumeRandom` 무작위(코드:Sound.cs:106, 171). 플레이어 소리는 대개 피치 ±0.1.

---

## 7. 종합 — R.E.P.O.의 "통통 튀고 물리적인" 감각을 만드는 재료 (영향 큰 순)

순위는 수치 크기와 "모든 조작에 매번 끼어드는가"로 매긴 이 노트의 판단이다(플레이 테스트로 검증하지 않았다).

1. **몸이 질량 있는 리지드바디라서 속도가 늦게 붙고 늦게 빠진다.**
   걷기 90 %까지 0.38 s, 질주 0.42 s, 멈춤 ≈0.3~0.38 s. 이동 중 마찰 0 → 정지 순간 마찰 5로 "탁" 붙는다. 공중에서도 똑같이 조종된다.
   *UE*: CMC 기본 가속(선형 MaxAcceleration·BrakingDeceleration)으로는 이 S자 곡선이 안 나온다. 커스텀 CMC의 `CalcVelocity`(걷기·낙하 공통)에서 목표속도 → 1단 지수추종(11.16 /s) → 2단 지수추종(11.16 /s) → 실속도 추종(34.7 /s)을 `1−exp(−k·dt)`로 프레임 독립 구현한다. `AirControl`은 1.0과 같은 효과가 되게 낙하에도 같은 필터를 쓴다. 입력이 0이고 1단 값이 10 cm/s 아래면 즉시 0으로 스냅한다(Idle 마찰 대체). MaxWalkSpeed 200, 질주 500, 웅크림 100 cm/s.

2. **카메라가 "가산 Transform 스택"이고 거의 모든 커브가 넘쳤다 돌아온다.**
   앉기 105 %·기기 107 % 넘침, 줌 +6.5 %, 강제 시선 +2.2 %, 점프/착지 −16.5 % 반동, 앉기 끄덕임 −4.2°/+5.9°. 숫자는 작아도 매번 "스프링처럼" 느껴지게 하는 핵심이다.
   *UE*: `APlayerCameraManager` 파생 또는 `UCameraModifier` 몇 개로 레이어를 나눠, 위치형(앉기·기기 −70/−45 cm)은 컨트롤 회전 바깥, 흔들림형은 뷰 공간에 더한다. 커브는 `UCurveFloat`에 REPO 키를 그대로 옮긴다(가중 접선이라 키 값만 옮기면 모양이 달라진다 — 곡선의 in/outWeight까지 옮기거나 평가한 샘플로 다시 맞춘다).

3. **큰 상하 보브(최대 +8.7 cm, 위로 튀는 혹 모양)와 발소리가 같은 시계로 묶여 있다.**
   걷기 2 Hz, 질주 3 Hz, 웅크림 1 Hz. 좌우 yaw 0.75°·roll 0.375°. 보브 박자는 0.67 s에 걸쳐 서서히 붙는다.
   *UE*: 발소리를 애님 노티파이가 아니라 **카메라 보브 위상이 1을 넘는 순간**에 낸다(1인칭은 몸 애니가 안 보이므로 화면과 소리가 어긋나지 않는다). 진폭은 cm로 8.66, 질주 시 위상 속도만 1.5배다(진폭 그대로).

4. **질주 = FOV +20°(세로) + 발소리 Heavy·3 Hz + 4 s짜리 짧은 스태미나.**
   FOV는 2 /s로 천천히 벌어져 1 s에 86 %. 스태미나 40, 소모 10 /s, 1 s 뒤 2 /s 회복, 탈진 잠금 없음.
   *UE*: UE FOV는 가로 기준이다. `DefaultEngine.ini`의 `[/Script/Engine.LocalPlayer] AspectRatioAxisConstraint=AspectRatio_MaintainYFOV`로 세로 고정하면 70/90/55를 그대로 쓸 수 있다(UE 5.8 소스에서 이 설정 이름은 이번에 확인하지 않았다). 가로 유지라면 16:9 기준 102.4° / 121.3° / 85.6°.

5. **상태가 바뀔 때마다 두 채널 셰이크가 짧게 들어간다.**
   점프·착지(Impact 1 + Shake 2), 앉기(2), 기기(1), 잡기·놓기(Impact 0.5), 텀블(1+2), 피격(3/5 + 3). 강도 1 = ±0.5° · ±0.2 cm. Impact는 즉시·20 Hz·30 /s 감쇠, Shake는 6 Hz·10 /s 감쇠.
   *UE*: `UCameraShakeBase` + `UPerlinNoiseCameraShakePattern` 두 개(짧은 고주파 "Impact", 부드러운 "Shake")로 근사하거나, REPO처럼 "무작위 목표 재설정 + 커브 보간" 패턴을 직접 만든다. "더 클 때만 더한다" 규칙이 연타 시 과포화를 막는다.

6. **시점 스무딩(기본 16.4 /s, 시상수 61 ms)과 회전 틸트.**
   돌면 0.03°/(°/s) 롤, 옆걸음 2° 롤, 0.33 s 추종. 피치 위 70°·아래 80°. 무거운 물체를 들어도 시점 감도는 그대로다.
   *UE*: 컨트롤 회전 자체를 로컬에서 지수 스무딩(REPO는 스무딩된 회전이 곧 조준이고 몸 yaw도 그걸 따름). 스무딩 0 옵션은 반드시 둔다(REPO도 설정 0이면 끔). 피치 한계는 UE 부호로 `ViewPitchMax = 70`(위), `ViewPitchMin = −80`(아래) — Unity X+는 아래, UE Pitch+는 위라서 부호가 뒤집힌다.

7. **물체는 스프링 빔으로 들고, 질량이 손에 그대로 느껴진다.**
   1 kg당 16 cm 처짐, 혼자 3.67 kg까지, 그 이상은 끌거나 협동. 반발 0.3 재질이라 떨어뜨리면 통통 튄다. 던지기는 휘두른 관성 + 작은 보너스.
   *UE*: `UPhysicsHandleComponent`보다 REPO식 커스텀 힘이 낫다 — `AddForceAtLocation(질량 × a)` 또는 `AddForce(a, bAccelChange=true)`로 a = 10·clamp(0.6·min(10d,6) − 0.3v, 6)/m·grabStrength (d·v는 m 단위로 계산 후 ×100). 여러 명이 잡으면 힘이 자연히 합쳐진다.

8. **점프가 빠르고 높다.**
   초속 850 cm/s, 중력 2.53 g, 정점 ≈137 cm, 체공 0.68 s. 코요테 0.25 s(+0.1), 선입력 0.25 s. 32 cm 넘게 떨어지면 착지 연출이 나온다.
   *UE*: `JumpZVelocity 850`, `GravityScale 2.53`(낙하에만 의미), `JumpMaxHoldTime 0`. 코요테·버퍼는 CMC에 없으니 캐릭터에서 타이머로 구현한다.

9. **근접 무기도 물리 물체다.**
   쥐는 힘 17배, 속도로 휘두름 판정(≈8 m/s), 명중 시 0.06 s 뒤 무기만 0.2 s 정지, 휠 한 칸에 5 m/s 찍기, 휘두름 소리 피치가 속도를 따라 1→10.
   *UE*: CL9가 애니메이션 스윙을 유지한다면 "명중 시 무기·손 0.2 s 고정 + Impact 셰이크 5 + 속도 연동 휘두름 소리" 세 가지만 가져와도 같은 방향이다(물리 스윙 전환은 별도 결정).

10. **슬라이드와 웅크림이 낮고 빠르다.**
    질주 → 웅크림(0.5 s 유예)으로 최고 537 cm/s, 1 s에 3.6 m. 카메라 150 → 35 cm까지 0.25~0.33 s에 내려간다. 웅크림 충돌 42.5 cm.
    *UE*: `CrouchedHalfHeight ≈ 21.25`(반지름 20), 눈높이는 캡슐과 별개로 카메라 오프셋 커브로.

11. **텀블(스스로 굴러가는 공) + FOV 55 + 속도 줌 + 비네트.**
    맞으면 대부분 이 상태로 날아간다(적 HurtCollider 텀블 힘 2~50, 시간 0~3 s). 몸이 물리 물체가 되는 순간이 REPO 코미디의 큰 몫이다.
    *UE*: 캡슐을 끄고 구 물리 바디로 바꾸는 "래그돌 볼" 모드. CL9 범위에 넣을지는 별도 판단.

12. **가만히 있어도 사는 화면** — 대기 노이즈 ±0.1°·±2 cm, 웅크림 떨림 ±0.25°.
    *UE*: Perlin 노이즈 카메라 셰이크를 상시(무한 지속, 낮은 진폭)로.

13. **(멀티) 동료 아바타가 과소감쇠 스프링으로 출렁인다** — 감쇠비 0.5, 10~15 rad/s, 기울기 ±3°, 질주 전환 시 2~3배 튕김.
    *UE*: 3인칭 메시의 Spine·Head 본에 AnimBP `SpringController` 또는 코드 스프링.

---

## 8. 재현 방법

값은 설치본에서 UnityPy로 뽑은 덤프와 디컴파일 코드에서 읽었다. 덤프와 계산 스크립트는 이 저장소에 두지 않았다. 재현하려면 [README.md](../README.md)의 "어떻게 조사했나" 순서를 따른다. 도구는 `UnityPy`와 `TypeTreeGeneratorAPI`이며(예: `uv run --with UnityPy --with TypeTreeGeneratorAPI python <스크립트>`), 이 게임은 기본 타입트리 생성기로 EOFError 없이 전부 읽혔다.

덤프로 읽은 범위는 다음과 같다.

| 범위 | 내용 |
|---|---|
| level0 | 플레이어·카메라 MonoBehaviour 실값(곡선 키 전체), 리지드바디·충돌체·재질·카메라, TimeManager·PhysicsManager |
| 플레이어 프리팹 | PlayerAvatar·PhysGrabber·PlayerTumble·PlayerAvatarVisuals·PlayerHealth |
| 아이템·재질 | ItemMelee·PhysAttribute·MaterialPreset |
| 계산 | 이동·점프 수식 재현과 가중 접선 커브 평가, FOV 환산, 외부 임펄스 응답, 잡기 평형, 스태미나, 귀중품 질량 분포, 적 HurtCollider 표본 |

## 9. 확인 못 한 것

- **플레이 실측 없음.** 가감속 시간·점프 높이·처짐은 코드 식 + 직렬화 값으로 계산한 것이다. PhysX 접촉·마찰 결합 우선순위, 바닥별 재질, 경사·계단은 반영하지 않았다.
- 점프 정점은 연속식 1.46 m와 50 Hz 이산 재현 1.37 m가 다르다. 실제는 PhysX 적분 순서에 따라 그 근처일 것으로 본다(추정).
- 가중 접선(`weightedMode 3`) 커브는 베지어+가중치로 재구현해 평가했다. 유니티 내부 구현과 소수점 차이가 있을 수 있다.
- `CameraTilt`가 회전 방향 대비 어느 쪽으로 기우는지 부호는 확인하지 않았다(디컴파일 조건문이 쿼터니언 성분과 오일러를 비교하는 형태라 동작 세부도 불확실).
- 마우스 델타 단위를 "카운트"로 가정했다(Input System Mouse delta). 윈도우 포인터 설정의 영향은 보지 않았다.
- `StepUpForce`는 미사용으로 판단(전체 grep). 계단 처리 방식(레벨 충돌체·`PlayerSlope` 재질)은 조사하지 않았다.
- 피격 글리치·아바타 애니메이터 클립, 오디오 클립 길이·내용은 읽지 않았다.
- 귀중품 질량 분포는 GameObject 이름으로 중복을 걸렀다. 레벨 모듈 안 배치 빈도(실제로 몇 번 나오는지)는 반영되지 않았다.
- 적 피격 반응 표(HurtCollider)는 표본이며 적별 정리·검증은 하지 않았다.
- 탈진 시 숨소리 등 전용 연출은 코드에서 찾지 못했다 — 없다고 단정하지는 않는다(애니메이터·오디오 쪽 가능성).
- 게임패드 시점이 0으로 덮어써지는 것은 디컴파일 결과 그대로 적었다. 런타임 동작은 확인하지 않았다.

## 10. CL9 에 옮긴 것

이 노트를 근거로 한 CL9 의 결정과 구현은 [../../goal/control-feel.md](../../goal/control-feel.md)가 정본이다(2026-10-05, 결정 C1~C19). 리썰 쪽 요지는 [../lethal/lethal-control-feel.md](../lethal/lethal-control-feel.md)에 있다. R.E.P.O. 와 관련한 것만 적는다.

- **따른 것 — 카메라가 몸이다.** 걸음마다 위로 튀는 상하 보브(CL9 는 걷기 2.8 cm·질주 6.8 cm), 옆걸음 롤(R.E.P.O. 2° → CL9 1.5°), 질주 FOV 확대(R.E.P.O. 세로 +20° → CL9 수평 +6°, 기본 FOV 90 유지), 점프·착지에서 넘쳤다 돌아오는 스프링(과소감쇠 스프링 하나, ω 16·ζ 0.6)을 가져왔다(C8·C11). 수치는 CL9 의 키(캡슐 180 cm)와 수평 FOV 에 맞게 새로 잡았다.
- **따른 것 — 발소리와 보브가 한 시계.** 7절 3)의 "보브 위상이 1을 넘는 순간 발소리"를 CL9 는 정규화한 발 위상(`StepPhase`)으로 옮겼다. 위상이 감길 때 발소리를 내고 보브의 저점이 그 순간에 오게 한다(C10).
- **따른 것 — 코요테와 점프 버퍼.** R.E.P.O. 의 0.25 s / 0.25 s 를 방향으로 삼아 CL9 는 0.12 s / 0.15 s 를 스트레치로 넣었다(C15).
- **버린 것.** 시점을 움직이는 요소 전부 — 보브 yaw(±0.75°)·착지 끄덕임·유휴 호흡(대기 노이즈)·피치 틸트(C9). 시점 스무딩(7절 6)은 마우스 스무딩이 범위 밖이라 옮기지 않았다. 리지드바디식 2단 지연 가감속(7절 1)은 옮기지 않고 CMC 의 제동 값과 질주 램프로 풀었다(C1·C3). 웅크림·슬라이드는 이번 범위 밖으로 뺐다(goal 문서 Out-of-scope: crouch / slide / lean).
- **밸런스는 그대로.** 최고 속도·스태미나·점프 높이는 건드리지 않았다. 새 카메라 움직임은 설정 슬라이더 "화면 움직임"(0~1)이 줄이며, 히트 킥·피격 흔들림은 슬라이더가 건드리지 않는다(C12).
- **측정.** `Tools/Unreal/qa_control_feel.py` 로 Lvl_Flat PIE 에서 재고 Gate A 73/73 행을 통과했다(2026-10-05) — [../../testing/pie-scenarios.md](../../testing/pie-scenarios.md) V절.
