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
| 동물 5종 자체 제작 SK + 클립 (닭·거위·너구리·돼지·양) | `Content/Monsters/<종>A/` | 2026-09-11 연결 — `Content/Monsters/ABP_<종>_A`(ABP_Chicken 복제·종별 스켈레톤), `MonsterTable` 행이 가리킴. 제작 기록은 [docs/asset/monster/](../asset/monster/README.md), 결정은 decisions.md 2026-09-11 |
| (구) Dreamscape 닭·돼지·양 SK | `Content/DreamscapeSeries/.../Chars/` | 09-10 팩 축소로 애님 시퀀스가 사라져 **더 이상 쓰지 않는다** — `ABP_Chicken`·`ABP_Pig`·`ABP_Sheep`은 빈 플레이어인 채 남아 있음 |
| Farmlands 환경 (농장 건물·지형·폴리지) + Horror 조명 깜빡임 | `DreamscapeFarmlands/` · `Variant_Horror/` | 5a 맵·light.md 참고 |

거위·너구리도 2026-09-11부터 전용 메시(`SK_Goose_A`·`SK_Raccoon_A`)를 쓴다 — 닭 빌려 쓰기·AVP 여우는 끝.

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

### 몬스터 원샷 — 상태 진입 시각 평가기 + 밀침 클립 (2026-09-15)

`ABP_<종>_A` 는 상태→알파 부울 블렌드 체인이라 원샷 클립(Catch·HitReact·WakeTelegraph·Death)을 시퀀스 플레이어로 두면 가지가 켜질 때
처음부터 재생된다는 보장이 없고 늦게 들어온 클라가 다른 프레임을 본다. 농부 휘두르기와 같은 규칙으로 간다: 호스트가 상태를 바꿀 때마다
`StateStartServerTime`(복제 신설)을 찍고, ABP 는 `GetStateElapsed()` 를 **시퀀스 평가기의 ExplicitTime** 으로 쓴다(클립 끝을 넘으면 마지막
프레임 유지 — Death 의 끝 자세, Catch 의 복귀 자세).

- **경직 두 종류**: `OnDamage`(고기 등 Hit — 행 `HitStunMs` 0.3초)는 `AN_<종>_HitReact`(0.3초), `OnShove`(프라이팬 Push — 행 `PushStun` 0.8초 +
  넉백)는 **`AN_<종>_PushReact`(0.8초, 신설)**. 어느 쪽인지는 `bPushStun`(복제)로 안다 — `IsPushStun()`. 그전엔 0.3초 HitReact 하나뿐이라 밀린
  0.8초 중 0.5초를 선 자세로 미끄러졌다.
- **Catch 는 0.8초** = 코드의 `CatchPauseSeconds`. 판정(OnDamage)이 상태 진입 순간이라 예비동작은 없다 — 첫 0.1초에 덮치고 종별 2차 동작
  (닭 연속 쪼기·돼지/양 머리 흔들기·너구리 반대 앞발) 뒤 돌아온다. 거위는 잡지 않는 종이라 없다(Honk 그대로).
- 클립 규격·제작은 [../asset/monster/README.md](../asset/monster/README.md) 표(`animate_creature_reacts.py`), 재임포트는
  `Tools/Unreal/reimport_creature_clips.py`(reworked 표시 클립만 교체).

### 로코모션 재생 배율 — 풋 슬라이딩 방지 (기본 규칙)

제자리(in-place) 이동 애님을 고정 배속으로 틀지 않는다. **모든 로코모션 애님은
`PlayRate = 현재 속도 ÷ 기준 속도`로 구동한다** (2026-09-05 확정 — 이후 만드는 모든
이동 애님의 기본). 기준 속도는 "1.0배속에서 발이 미끄러지지 않는 이동 속도"로, 팩 애님엔
명시가 없으므로 PIE에서 눈으로 실측 보정한다.

- **블렌드 문턱은 테이블 속도와 정합**시킨다 — 배회 속도 = 완전 걷기, 추격 속도 = 완전
  달리기. 그 사이 속도는 포즈 블렌드가 아니라 재생 배율이 흡수한다.
- 기준 속도 (2026-09-11, 자체 제작 클립은 표 속도에 맞춰 발을 심었으므로 manifest의
  `reference_speed_cm_s`가 곧 SafeDivide 상수): Walk = WanderSpeed(닭·거위·돼지·양 126, 너구리 140),
  Run = ChaseSpeed(닭·돼지·양 396, 거위 528, 너구리 462). 팩 애님 시절의 눈대중 값(닭 165/396,
  여우 210/462)은 폐기.
- 종마다 ABP가 따로 있으므로(`ABP_<종>_A`) 속도 상수도 종마다 제 값이다 — 이제 공유 ABP는 없다.

## 1인칭 팔 (FPP Arms)

시점이 1인칭이고 **자기 화면에는 손(팔)만 보인다** ([character.md](character.md)). 2026-09-13 저녁 fp-viewmodel 에서 **리썰
컴퍼니식 뷰모델**로 바꿨다 — 사용자 "애니메이션이 부자연스러워, 억지로 팔을 들고 있는 모습", "위아래를 볼 때 손이 움직이는 거랑
시야가 이상해, 리썰 조작감으로". 그 전(2026-09-12~13 밤)의 팔 쪽 구조 — 몸 포즈 복사(`ABP_FP_Copy` CopyPoseFromMesh)·상체 레이어·
ControlRig FPWarp·`Transform (Modify) Bone` 피치 추종·`FPHoldIdle` — 는 전부 폐기했다. 전신(남이 보는 것)은 아래 "전신 ABP_Unarmed" 그대로.

- **팔은 카메라 자식** — 팔 메시 `SK_Farmer_A_Arms`(OnlyOwnerSee, `FirstPersonPrimitiveType::FirstPerson`, FOV 70·스케일 0.6)는
  **카메라 컴포넌트의 자식**이다: 상대 `(0,0,−EyeHeight)`·yaw −90 (리그 root 가 원점, 눈이 (0,0,EyeHeight)에서 +X 를 봄 —
  `ACL9Character` 생성자, EyeHeight 는 CharacterTable). 시선 피치·요는 카메라가 통째로 돌리므로 팔은 항상 화면의 같은 자리에 있고,
  든 아이템은 `FirstPersonMesh:HandGrip_R` 에 붙어 같이 돈다(시선·이동에 따라 손 자세가 흔들려 "아이템이 자꾸 회전"으로 보이던 것이
  사라진다). 리썰과 같이 두는 것: 스태미나 카메라 셰이크는 POV 만 흔들어 팔이 화면에 고정된 채 세상이 흔들려 보인다.
- **전용 `ABP_FP_View`** — 몸과 무관한 카메라 로컬 클립만 튼다. 우선순위 순 부울 블렌드(BlendTime 0.15):
  0. `KnockdownElapsed ≥ 0`(넘어짐·죽음, 2026-09-15) → `KnockdownElapsed < 0.5` 이면 시퀀스 평가기 `AN_Farmer_FPKnockdown`(두 손이 얼굴 앞으로
     확 올라왔다 빠짐, `ExplicitTime = KnockdownElapsed`), `KnockdownRemaining < 0.6` 이면 평가기 `AN_Farmer_FPGetUp`(`ExplicitTime = 0.6 −
     KnockdownRemaining`), 그 사이(누움·죽음)는 `AN_Farmer_FPArmsDown`(프레임 밖). 카메라 하강은 C++ Tick(DownedZ)이 이미 한다.
  1. `EatElapsed ∈ [0, 0.6)` → 시퀀스 평가기 `AN_Farmer_FPEat`(`ExplicitTime = EatElapsed`)
  2. `ThrowElapsed ∈ [0, 0.333)`(2026-09-15) → 평가기 `AN_Farmer_FPThrow`(주먹이 화면 중앙 위로 뻗으며 펴지고 아래로 빠짐) ·
     `ThrowWindupElapsed ≥ 0` → 플레이어 `AN_Farmer_FPThrowWindup`(1.0초 루프, 주먹이 오른쪽 가장자리)
  3. `SwingWindupElapsed ≥ 0` → 시퀀스 플레이어 `AN_Farmer_FPSwingWindup`(1.0초 루프)
  4. `SwingElapsed ∈ [0, 0.347)` → 시퀀스 평가기 `AN_Farmer_FPSwing`(`ExplicitTime = SwingElapsed + 0.12` — 치켜든 자세부터, 클립 0.467초)
  5. `HoldPose == None ? (IsRunning ? AN_Farmer_FPRun[루프, PlayRate = Speed / 654] : AN_Farmer_FPArmsDown[루프])
     : (Large ? FPHoldLarge : Flashlight ? FPHoldFlashlight : FPHoldSmall)`
  변수 `HoldPose`·`IsRunning`·`Speed`·`SwingElapsed`·`SwingWindupElapsed`·`EatElapsed`·`KnockdownElapsed`·`KnockdownRemaining`·`ThrowElapsed`·
  `ThrowWindupElapsed` 는 매 틱 `ACL9Character` 게터로 채운다
  (전신 ABP 와 같은 게터, `BlueprintUpdateAnimation → TryGetPawnOwner`). 빈손 규칙(기본값 19·20)은 그대로: 서 있거나 걸을 때는 손이
  안 보이고(`FPArmsDown` 은 프레임 밖 — 카메라 자식이라 피치와 무관하게 항상), 달릴 때만 주먹이 화면 아래 좌우로 번갈아(`FPRun`,
  "로코모션 재생 배율" 규칙). 걷기 전용 팔 클립은 없다.
- **카메라 로컬 클립 규약** (`Tools/Blender/farmer_pose.py` 머리말, [../asset/character/farmer_a.md](../asset/character/farmer_a.md) 031):
  클립 공간 = 리그 root 원점, 눈 (0,0,1.56m)에서 −Y(Blender)/+X(UE)를 보는 공간. **피치 0 고정 프레이밍**으로만 저작한다(피치·요는
  카메라가 돌린다). 몸 포즈 복사·척추 레이어가 없으니 **클립 하나가 전 뼈를 소유한다** — root·pelvis·spine 은 identity, 숨쉬기는 척추가
  아니라 손목 z 사인. manifest `animations[*]` 에 `space: "camera"`·`eye_height_m: 1.56` 이 붙은 항목만 이 ABP 에 넣는다(전신용
  `full body` 항목과 섞지 않는다). 자세는 리썰식 — 팔꿈치 ~90° 굽힌 느슨한 자세, 아래팔이 우하단 구석에서 안쪽으로 올라오고 주먹이
  구석에서 화면 안으로 들어오며 손등·엄지가 카메라를 향한다(억지로 들지 않는다; 사용자 기준 캡처 `preview/my/1.png`).
- **미리보기 모델** — `farmer_pose.render_fp(tag, dir, pitch_deg=0)` 가 게임과 같은 카메라(1280×720·가로 FOV 70 → 세로 반각 21.5°·
  눈 1.56m·근접 5cm/0.6 = 8.3cm)로 팔 메시만 그리고, `fp_screen_uv(point)` 가 리그 공간 점의 화면 (u, v)(왼쪽 위 0,0, 프레임 밖이면
  None)를 준다 — 문서·보고서의 화면 위치 수치는 전부 이 함수의 실측이다. FirstPersonScale 0.6 은 카메라 원점 기준 닮음 변환이라 화면상
  위치·크기가 그대로다(근접 클립만 환산). 미리보기 게이트: 사용자가 `preview/*_fp.png` 를 보고 "가"를 준 뒤 임포트한다(기본값 12).
- **클립 9종(031)** — 1인칭 8: `FPHoldSmall`(2.0s 루프, 오른 주먹이 우하단 구석에서 들어옴 — 너클 (0.83, 0.83), 주먹 중심 ≈ (0.90, 0.92))·
  `FPHoldFlashlight`(같은 높이·구석, 꽉 쥔 주먹 — 렌즈 정면은 ItemTable `flashlight.GripRotation` 새 행이 맞춘다)·`FPHoldLarge`(두 주먹 중심
  ≈ 0.28/0.72·v 0.9)·`FPArmsDown`·
  `FPRun`(0.6s 루프, `reference_speed_cm_s` 654, 앞 극점 주먹 u 0.65~0.82·v 0.58~0.77)·`FPSwingWindup`(1.0s 루프, 주먹 우상단)·
  `FPSwing`(0.467s 원샷)·`FPEat`(0.6s 원샷, 입 앞 = 렌즈 앞 27cm 중앙) + 전신 `AN_Farmer_Swing` 30fps 재수출. 제작 `Tools/Blender/
  export_farmer_fp_clips.py`(hold·fparms·swing·windup·eat·verify 일괄), 임포트 `Tools/Unreal/import_farmer_clips.py`(기본 목록 = 이 9클립)
  → `/Game/Characters/FarmerA/Anims`. `AN_Farmer_FPHoldIdle`·`ABP_FP_Copy` 는 참조 해제 뒤 삭제 여부 리드 판단.
- **전신 `ABP_Unarmed`** (남이 보는 것 — 2026-09-12~13 구조 유지): `Slot(DefaultSlot)` → `본별 레이어드 블렌딩`(`spine_01`, 로컬 공간,
  BlendDepth 0) → `ControlRig(발 IK)`. 레이어의 블렌드 포즈는 **부울 블렌드 2단** `HoldPose == Large ? HoldLarge : (HoldPose == Flashlight ?
  HoldFlashlight : HoldSmall)`, 가중치 `ToFloat(NOT(HoldPose == None))` — 빈손이면 템플릿 로코모션 그대로. 레이어 뒤 `Transform (Modify)
  Bone`(`spine_03`, 회전 Add, 피치 = `AimPitch × 0.6`, `AimPitch` ← `GetAimPitch()`)로 아래를 보면 상체가 기운다(계수는 초안 — PIE 로 확정).
  `BlendPoses(ECL9HoldPose)` 노드를 안 쓴 이유: 항목 핀 노출(`ExposeEnumElementAsPin`)이 에디터 컨텍스트 메뉴 전용이라 MCP/파이썬으로
  만들 수 없다. ABP 변수 `HoldPose`(ECL9HoldPose)는 EventGraph 가 매 틱 `Cast To CL9Character → GetHoldPose` 로 채운다(`SetIsFalling`
  뒤); 변수 생성은 `BlueprintEditorLibrary.add_member_variable` + `EdGraphPinType.import_text('(PinCategory="byte",
  PinSubCategoryObject="/Script/CL9.ECL9HoldPose")')`(MCP `add_variable` 은 프리미티브만 받는다).
  - **휘두르기 가지** (2026-09-13, 사용자 "아예 액션을 안 해"): 좌클 `Swing` 의 표현. 몽타주·RPC 가 아니라 **상태 파생** — `ACL9Character::
    SwingStartServerTime`(복제)에서 `GetSwingElapsed()` 가 지난 초를 주고 ABP 변수 `SwingElapsed` 가 매 틱 받는다. Hold 체인 머리 앞에
    부울 블렌드 한 단: `InRange(SwingElapsed, 0, 0.347)` 이면 **시퀀스 평가기** `AN_Farmer_Swing`(`ExplicitTime = SwingElapsed + 0.12`),
    아니면 Hold 체인. 원샷을 평가기+경과 시각으로 트는 이유: 시퀀스 플레이어는 가지가 켜질 때 처음부터 재생된다는 보장이 없고, 복제된 시작
    시각이면 모든 머신이 같은 프레임을 본다. 클립은 Small 들기 자세에서 치켜들었다(t=0.12) 내려치는 15프레임 30fps **0.467초**(031 에서
    재수출 — 옛 24fps 0.583초·문턱 0.58·오프셋 0.15 폐기). 판정은 뗀 뒤 `CL9Rules.SwingHitStart~End` 구간 동안 전신 손의 아이템
    히트박스를 이전 틱 → 현재 틱으로 스윕해서 하고, 클립과 **같은 `SwingElapsed` 시계**를 쓴다(2026-09-15 — 옛 "누르는 즉시 판정, 애님은
    따라올 뿐" 폐기).
  - **휘두르기 클립 재작업** (2026-09-15, 사용자 "무기 공격이 어색하다"): 키 6개(HoldSmall → 오른쪽 어깨 위·뒤로 치켜듦 0.12 → 타격 직전 0.18 →
    몸 앞으로 뻗은 타격 0.25 → 팔로스루 0.31 → HoldSmall)를 구간별 완급(치켜듦까지 완만·타격까지 가속·타격 뒤 급감속)으로 잇고, 몸통이 오른쪽으로
    비틀며 젖혔다가 왼쪽으로 비틀며 숙인다(척추 3마디 분배, 머리는 반대로 돌려 정면 유지, 왼팔은 뒤로). 옛 4키 직선 보간·옆으로 벌어진 치켜듦·
    허리 옆 타격·몸통 정지가 어색의 원인이었다. ABP 상수(오프셋 0.12·범위 0.347)·히트 구간은 그대로 —
    [../asset/character/farmer_a.md](../asset/character/farmer_a.md) 032.
  - **공격 대기·먹기 가지** (2026-09-13 밤, use-hint-anim — "좌클 누르면 공격 대기, 떼면 공격", "먹을 때 손이 입 쪽으로"): 같은 상태 파생.
    ABP 변수 `SwingWindupElapsed`(`GetSwingWindupElapsed()`, **−1 이면 대기 아님**)·`EatElapsed`(`GetEatElapsed()`, 아니면 −1). 휘두르기
    부울 블렌드 앞에 우선순위 순으로 (1) `EatElapsed ∈ [0, 0.6)` → 시퀀스 평가기 `AN_Farmer_Eat`(`ExplicitTime = EatElapsed`; 시작·끝이
    HoldSmall 이라 튀지 않는다, 먹는 동안 칸 전환 허용 — 기본값 18) (2) `SwingWindupElapsed >= 0` → 시퀀스 플레이어 `AN_Farmer_SwingWindup`
    (1.0초 루프, 휘두르기 t=0.12 자세 + 손목 ±1cm; 무한 대기). 뗀 뒤는 휘두르기 가지가 치켜든 자세부터 잇는다 — 탭도 같은 경로라 "평범한
    공격"은 0.347초(기본값 17).
  - **던지기 가지** (2026-09-15): Shift+G 차징·놓기의 표현. `ThrowChargeStartTime`(호스트 시각 — 이제 **복제**, 판정은 여전히 호스트 값만)과
    `ThrowServerTime`(놓은 시각, 복제)에서 `GetThrowWindupElapsed()`(차징 중 아니면 −1, 소유자는 로컬 시각으로 즉시)·`GetThrowElapsed()` 가 나온다.
    Hold 체인 머리에 (1) `ThrowElapsed ∈ [0, 0.333)` → 시퀀스 평가기 `AN_Farmer_Throw`(`ExplicitTime = ThrowElapsed`; 판정은 놓는 순간이라 t=0 에
    이미 물건이 손을 떠났고 클립은 뻗기·팔로스루 뒤 **기준 자세(identity)로 끝난다** — 던지면 HoldPose=None 이라 레이어 가중치가 빠질 때 안 튀게)
    (2) `ThrowWindupElapsed ≥ 0` → 플레이어 `AN_Farmer_ThrowWindup`(1.0초 루프, 오른손을 어깨 뒤로 당김·왼손 앞으로 조준). 상체 레이어 가중치는
    `HoldPose != None OR 던지기 가지 활성`.
  - **넘어짐 가지** (2026-09-15, 사용자 "피격 애님이 부족하다"): 잡힘(OnDamage → `KnockdownTime` 1.5초)과 죽음(무기한)의 표현. 그전엔 클립이 0개라
    남이 보기엔 선 채로 얼어붙었고, 죽음은 "롤 90°" 계획만 있었다. `KnockdownStartServerTime`(복제 신설 — 끝 시각만으로는 죽음(끝 = 무한)의 경과를
    못 구한다)에서 `GetKnockdownElapsed()`(넘어져 있지 않으면 −1)·`GetKnockdownRemaining()`(풀릴 때까지 남은 초, 죽었으면 큰 값)이 나온다.
    **로코모션·Hold 레이어·발 IK 전부 뒤**(ControlRig 다음)에 부울 블렌드 한 단: `KnockdownElapsed ≥ 0` 이면 `KnockdownElapsed < 0.5` →
    평가기 `AN_Farmer_Knockdown`(젖혀져 등으로 넘어감, `ExplicitTime = KnockdownElapsed`), `KnockdownRemaining < 0.6` → 평가기 `AN_Farmer_GetUp`
    (`ExplicitTime = 0.6 − KnockdownRemaining`), 그 사이 → 평가기 `AN_Farmer_Down`(2초 루프 클립, 등을 대고 누움·숨쉬기; `ExplicitTime =
    IsDead ? 1.0 : (KnockdownElapsed − 0.5) mod 2` — 시체는 한 프레임에 멈춘다. 플레이어 대신 평가기인 이유: PlayRate 핀 노출 없이 죽음 정지를 만들고
    늦게 들어온 클라도 같은 위상을 보게). 전신 골반 위치 키가 든 유일한 클립들이라 발 IK 뒤에 둬야 한다(IK 가 누운 다리를 세우려 든다) —
    실제 배선(2026-09-15): `ControlRig → [부울 블렌드 KnockdownElapsed≥0] → 출력`, 1인칭은 `HoldPose==None` 분기 위에 같은 블렌드.
    타임라인(1.5초): 넘어짐 0~0.5 → 누움 0.5~0.9 → 일어남 0.9~1.5. 죽음: 넘어짐 → 누움 무기한.
  클립 제작 `Tools/Blender/animate_farmer_hold.py`(tp)·`animate_farmer_swing.py`·`animate_farmer_windup.py`·`animate_farmer_eat.py`·
  `animate_farmer_knockdown.py`·`animate_farmer_throw.py`, 30fps·Manny 스켈레톤 — [../asset/character/farmer_a.md](../asset/character/farmer_a.md)
  025·026·028·032.
- 데이터 연결: `UCL9CharacterDef`에 팔 메시·팔 AnimBP 컬럼 추가 필요 (미결 — 1c에서).

## 몽타주 사용처

전부 렌더 전용 — 길이는 판정 값에 맞춘다:

| 몽타주 | 판정 값 | 비고 |
|---|---|---|
| 죽음 | HP 0 | 기본은 같은 메시의 죽는 모션 — 래그돌 등 다른 연출은 종별 선택 |
| 경직 | `HitStunMs` / `PushStun` | 전용 애니메이션 필수 (plan/data/monster/common.md) — 타격 `HitReact` 0.3초 / 밀침 `PushReact` 0.8초 (2026-09-15) |
| 잡힘·넘어짐 | CatchRadius 판정 · 캐릭터 `KnockdownTime` 1.5초 | 짐 흘림과 함께. 몬스터 `Catch` 0.8초, 캐릭터 `Knockdown` 0.5 → `Down` 루프 → `GetUp` 0.6 (2026-09-15) |
| 던지기 | 놓는 순간 판정 | `ThrowWindup` 루프(차징) → `Throw` 0.333초, 끝 = 기준 자세 (2026-09-15) |
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
