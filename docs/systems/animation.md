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

## 보유 에셋 (2026-09-21 감사 기준 — 프로젝트 Content 실물)

| 에셋 | 위치 | 용도 |
|---|---|---|
| **농부 A 자체 제작** — 전신 `SK_Farmer_A_Body`·1인칭 팔 `SK_Farmer_A_Arms` + 클립 `Anims/AN_Farmer_*`(전신 Hold 3·Swing·SwingWindup·Eat·Knockdown·Down·GetUp·Throw·ThrowWindup, 1인칭 `AN_Farmer_FP*` 13) + 팔 ABP `Anims/ABP_FP_View` | `Content/Characters/FarmerA/` | `CharacterTable.farmer_a` 행이 가리킴. 스켈레톤은 마네킹 `SK_Mannequin` 호환(레거시 FBX 임포트). 제작 기록 [docs/asset/character/farmer_a.md](../asset/character/farmer_a.md), 1인칭 팔 구조는 아래 "1인칭 팔" 절 |
| 템플릿 마네킹 — 스켈레톤 `SK_Mannequin`·`SKM_Manny/Quinn_Simple` + 전신 ABP `Anims/Unarmed/ABP_Unarmed`(+ `BS_Idle_Walk_Run`·Jog/Walk/Jump/Attack 클립) | `Content/Characters/Mannequins/` | **스켈레톤·전신 ABP 호환용으로만** — 농부 A 몸이 이 스켈레톤을 쓰고 `ABP_Unarmed` 가 남이 보는 전신 애님이다. 마네킹 메시 자체는 화면에 안 나온다 |
| 동물 5종 자체 제작 SK + 클립 (닭·거위·너구리·돼지·양) | `Content/Monsters/<종>A/` | 2026-09-11 연결 — `Content/Monsters/ABP_<종>_A`(ABP_Chicken 복제·종별 스켈레톤), `MonsterTable` 행이 가리킴. 제작 기록은 [docs/asset/monster/](../asset/monster/README.md), 결정은 decisions.md 2026-09-11 |
| (구) 템플릿 동물 ABP `ABP_Chicken`·`ABP_Fox`·`ABP_Pig`·`ABP_Sheep` | `Content/Monsters/` 바로 아래 | 참조 0·빈 플레이어인 채 남아 있음 — 삭제 보류 |
| (구) Dreamscape Farmlands | 없음 | **쓰지 않는다.** 2026-10-04(f300179)에 팩 폴더를 지웠다. 맵 조각·소품은 자체 제작 `Content/Maps/Henhouse/`다. 위 구 ABP 셋 중 `ABP_Chicken`·`ABP_Pig`·`ABP_Sheep`은 지워진 팩 경로를 가리킨 채 남아 있다 |
| Horror 조명 깜빡임 | `Variant_Horror/` | light.md 참고 |

옛 `Content/FirstPerson/Anims/`(빈 폴더만 남음)·`ABP_FP`·`ABP_FP_Copy`·`CtrlRig_FPWarp` 는 없다 — 1인칭 팔은 2026-09-13 `ABP_FP_View`(카메라 자식 뷰모델)로
갈아탔다(아래 "1인칭 팔" 절). 거위·너구리도 2026-09-11부터 전용 메시(`SK_Goose_A`·`SK_Raccoon_A`)를 쓴다 — 닭 빌려 쓰기·AVP 여우는 끝.

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
- **PounceWindup 은 0.6초** = 표 `PounceWindup` 기본값(2026-09-20 monster-pounce-hurt-readability — 잡기 예고 상태 `Pounce`, monster.md).
  **닭만** `AN_Chicken_PounceWindup`: 0~0.18초 몸을 뒤로 젖히며 날개를 옆·위로 벌리고 꽥(부리), 0.43초까지 잘게 떨며 버티다가 마지막 0.17초에
  웅크리며 **Catch 첫 프레임(기본 자세 + Catch 발 자리)으로 돌아와 끝난다** — 예고 끝을 Catch(덮침)나 Catch 앞 0.3초(헛침)가 이어받으므로 끝 프레임이
  같아야 튀지 않는다. 다른 종은 클립 없이 자기 Catch 첫 프레임을 유지한다(가지 없음). 표 값이 0.6 보다 길면 평가기가 마지막 프레임(=기본 자세)을
  유지하고, 짧으면 잘린다 — 클립은 0.6 고정, 스케일하려면 표 값 게터가 필요하다(지금은 없다).
- **Execute 는 2.0초** = 행 `ExecuteDuration`(2026-09-21 execute-clips — Run 2 2인 클립; 초안 2.0 은 balance 가 확정). **종당 주 공격 하나** — 잡는 종은
  Catch, 처형 종(`IsExecutioner()` = 돼지·너구리)은 Execute 가 곧 공격 모션이다(사용자: 처형과 공격은 같은 모션). `AN_Pig_Execute`(정면 박치기, 접촉
  0.5)·`AN_Raccoon_Execute`(등 뒤 도약, 접촉 0.6) 와 농부 짝 `AN_Farmer_Executed_Pig`(뒤로 넘어져 DOWN)·`AN_Farmer_Executed_Raccoon`(앞으로 엎어져
  PRONE)이 액터 공간 앵커 표 `Tools/Blender/execute_anchor.py` 를 공유한다. 죽는 순간 = 클립 끝. 몬스터 끝 자세 = Idle 첫 프레임(시체 옆에 선 자세,
  `SpectateDelay` 까지 평가기가 마지막 프레임 유지 → Break 로 로코모션에 튐 없이), 농부 끝 자세는 부활까지 유지(아래 `ABP_Unarmed` Run 2).
  제작 카드 [../asset/monster/pig_a.md](../asset/monster/pig_a.md)·[raccoon_a.md](../asset/monster/raccoon_a.md)·[../asset/character/farmer_a.md](../asset/character/farmer_a.md) 034.
- 클립 규격·제작은 [../asset/monster/README.md](../asset/monster/README.md) 표(`animate_creature_reacts.py`), 재임포트는
  `Tools/Unreal/reimport_creature_clips.py`(reworked 표시 클립만 교체 — `CL9_CLIPS` 로 클립을, `CL9_EVIDENCE` 로 증거 JSON 을 고를 수 있다).

#### `ABP_Chicken_A` Pounce 가지 사양 (리드가 MCP 로 만든다 — 2026-09-20)

읽는 게터(`ACL9Monster`, BlueprintPure — gameplay): `GetState()`(`ECL9MonsterState::Pounce` 추가), `GetStateElapsed()`, `bool IsPounceHit()`,
`float GetPounceResultElapsed()`(= 서버 시각 − `PounceResultServerTime`, 헛침·덮침 결과가 찍힌 뒤 흐른 시간). EventGraph 는 기존
`StateElapsed`·`IsPushStun` 세터 뒤에 변수 셋을 더 채운다:

| 변수 | 값 | 뜻 |
|---|---|---|
| `IsPounce` (bool) | `GetState() == Pounce` | 예고 중 |
| `IsPounceMiss` (bool) | `GetState() == Chase ∧ !IsPounceHit() ∧ GetPounceResultElapsed() < 0.3` | 헛친 직후 0.3초 창(Chase 로 복귀한 채) |
| `PounceResultElapsed` (float) | `GetPounceResultElapsed()` | 헛침 클립의 ExplicitTime |

AnimGraph — 기존 상태→알파 부울 블렌드 체인에 **가지 둘**을 끼운다. 체인은 출력에 가까운 블렌드가 이기므로 자리는 **로코모션(Idle/Walk/Run)
바로 뒤, Catch 블렌드 바로 앞**(로코모션 → ① 헛침 → ② 예고 → Catch → HitReact/PushReact → Death 순). Pounce·Catch·Stun·Dead 는 상태라
서로 배타적이지만, 헛침 창은 Chase 상태 안의 시간 창이라 로코모션은 이겨야 하고 그 뒤에 오는 Catch·Stun·Dead 에는 져야 한다 — 그래서
둘 다 Catch 앞이다.

1. **① 헛침**: `부울로 포즈 블렌딩`, `bActiveValue = IsPounceMiss`, true 포즈 = **시퀀스 평가기 `AN_Chicken_Catch`**(`ExplicitTime =
   PounceResultElapsed` — 클립 앞 0.3초, 0.8초 클립을 끝까지 틀지 않는다: 창이 닫히면 로코모션으로 돌아간다), false 포즈 = 로코모션 출력.
   BlendTime 0.05/0.1(들어갈 때 짧게, 나올 때 달리기로 부드럽게).
2. **② 예고**: `부울로 포즈 블렌딩`, `bActiveValue = IsPounce`, true 포즈 = **시퀀스 평가기 `AN_Chicken_PounceWindup`**(`ExplicitTime =
   StateElapsed` — 평가기가 클립 끝(0.6)에서 마지막 프레임을 유지하므로 따로 클램프 노드는 없어도 된다; 표 `PounceWindup` 이 0.6 을 넘으면
   기본 자세로 서서 기다린다), false 포즈 = ①의 출력. BlendTime 0.1/0.0(예고 끝은 Catch 가 같은 프레임에서 잇는다 — 끝 프레임 = Catch 첫 프레임).
3. 기존 Catch 블렌드의 false 입력을 ②의 출력으로 옮긴다. Catch(덮침) 가지는 그대로(`AN_Chicken_Catch` 평가기, `ExplicitTime = StateElapsed`).
4. 다른 4종 ABP 는 건드리지 않는다 — Pounce 상태에서는 어느 알파도 켜지지 않아 로코모션(속도 0 → Idle)이 나온다. "Catch 첫 프레임 유지"가
   필요하면 같은 ① 없이 ② 만 `AN_<종>_Catch` 평가기(`ExplicitTime = 0`)로 넣는다 — 지금은 요청 없음.

**실제 적용(리드, 2026-09-20 MCP)** — 위 사양을 `ABP_Chicken_A` 의 **기존 관례**(상태별 float 알파 변수 + `양방향 블렌드`, `AlphaAttack`·`AlphaStun` 과
같은 방식)로 옮겨 넣었다. 변수는 `AlphaPounce`(float)·`AlphaPounceMiss`(float)·`PounceResultElapsed`(float): EventGraph 는 `SetIsPushStun` 뒤에
`SetPounceResultElapsed(= CL9.GetPounceResultElapsed)` → `AlphaPounce 0` → `AlphaPounceMiss 0` 을 잇고, `Switch on ECL9MonsterState` 의 `Pounce` 핀이
`AlphaPounce 1`, `Chase` 핀이 `Branch(NOT IsPounceHit() AND InRange(PounceResultElapsed, 0, 0.3))` → `AlphaPounceMiss 1`. AnimGraph 는 Wake 블렌드
출력 → ① `양방향 블렌드`(B = `AN_Chicken_Catch` 평가기, ExplicitTime = `PounceResultElapsed`, Alpha = `AlphaPounceMiss`) → ② `양방향 블렌드`(B =
`AN_Chicken_PounceWindup` 평가기, ExplicitTime = `StateElapsed`, Alpha = `AlphaPounce`) → 기존 Catch 블렌드 A 입력. 블렌드 시간은 양방향 블렌드라
없다(알파 즉시 전환 — 기존 Catch·Stun 가지와 같음).

확인(PIE C3-예고): 닭이 130cm 안에서 0.6초 동안 몸을 세우고 날개를 벌리는 게 보이고, 뒤로 걸어 헛치게 하면 Catch 앞 0.3초(덮치는 첫 동작)만
보인 뒤 다시 달린다. 늦게 들어온 클라도 `StateElapsed` 기준이라 같은 프레임을 본다.

#### `ABP_Pig_A` Execute 가지 사양 (리드가 MCP 로 만든다 — 2026-09-20, execute-sequence-pig Run 1)

돼지만의 처형(`Execute`)·스토킹(`Stalk`) 상태의 표현 — 설계는 [../goal/execute-sequence-pig.md](../goal/execute-sequence-pig.md) D6·D7, 상태·판정은
[monster.md](monster.md) "스토킹과 처형". 읽는 게터는 기존 그대로: `GetState()`(`ECL9MonsterState::Stalk`·`Execute` 추가 — enum 은 뒤에 붙였고
순서를 바꾸지 않았다), `GetStateElapsed()`. **Run 1 은 전용 클립이 없다** — 기존 `AN_Pig_Catch`(0.8초)를 마지막 프레임 유지로 튼다. Run 2 에서
`AN_Pig_Execute`(길이 = `ExecuteDuration`)로 평가기의 클립만 바꾼다.

`ABP_Chicken_A` Pounce 가지의 **실제 적용 관례**(상태별 float 알파 + `양방향 블렌드`)를 따른다:

| 변수 | 값 | 뜻 |
|---|---|---|
| `AlphaExecute` (float) | EventGraph 매 틱 0 으로 지운 뒤 `Switch on ECL9MonsterState` 의 `Execute` 핀 → 1 | 처형 중 |
| (없음) | `Stalk` 핀은 **어느 알파도 켜지 않는다** | 스토킹 = 로코모션 그대로(속도에 따라 Idle/Walk — `StalkSpeed` 로 걷고, 멈추면 Idle, 물러나면 `RetreatSpeed` 로 뒷걸음) |

AnimGraph — Wake 블렌드 출력 → **`양방향 블렌드`**(A = Wake 블렌드 출력, B = **시퀀스 평가기 `AN_Pig_Catch`**, `ExplicitTime = StateElapsed`,
Alpha = `AlphaExecute`) → 기존 Catch 블렌드의 A 입력. 평가기는 `StateElapsed` 가 클립 끝(0.8)을 넘으면 **마지막 프레임을 유지**하므로 따로
클램프 노드는 없다 — `ExecuteDuration` 이 0.8 보다 길면 덮친 자세로 얼어 있고, 짧으면 클립이 잘린다(둘 다 Run 1 허용). 처형이 끝나
`Break` 로 가면 알파가 0 이 되어 로코모션으로 돌아온다(블렌드 시간 없음 — 기존 Catch·Stun 가지와 같음). 자리가 Catch 블렌드 **앞**인
이유: Execute·Catch·Stun·Dead 는 상태라 배타적이고, 처형 중엔 Stun 에 들어가지 않으므로(구조 불가 — `OnDamage`/`OnShove` 면제) 순서가
실제로 겹치는 일은 없다 — Pounce 가지와 같은 자리에 두는 것이 가장 단순하다. 처형 중 돼지가 죽으면 상태가 `Dead` 라 Death 가지가 이긴다.
돼지 메시 스무딩(`SmoothedLocation/Rotation`)은 처형 동안 코드가 고정하므로(D1) ABP 는 관여하지 않는다.

**2026-09-20 종 재배치(실행 execute-species-swap — 사용자 결정: 스토커 = 너구리, 돼지 = 잡히면 처형)**: `ABP_Raccoon_A` 에 **같은 사양의
Execute 가지**를 만든다 — `AlphaExecute` 변수·`Switch on ECL9MonsterState` 의 `Execute` 핀·`양방향 블렌드` 자리(Catch 블렌드 앞)까지 위와 동일하고,
평가기의 클립만 **`AN_Raccoon_Catch`**(`ExplicitTime = StateElapsed`, 클립 끝을 넘으면 마지막 프레임 유지). `Stalk` 핀은 역시 어느 알파도 켜지
않는다(`StalkSpeed` 450 이면 Walk/Run 문턱에 따라 로코모션이 뛰는 자세일 수 있다 — 표 값의 결과, ABP 는 손대지 않는다). `ABP_Pig_A` 는 **위 사양
그대로 둔다** — 돼지는 이제 잡히는 순간이 처형이라(`bExecuteOnCatch`, `HostCatch` 가 `Catch` 대신 `Execute` 로 들어간다) `Catch` 가지가 아니라
Execute 가지에서 같은 클립 `AN_Pig_Catch` 가 뛴다(덮친 자리에서 순간 이동 없이 — 스무딩 고정은 같다). 종 재배치의 정본은
[../goal/execute-sequence-pig.md](../goal/execute-sequence-pig.md) 상단 절.

**Run 2(2026-09-21 execute-clips — 리드가 MCP 로 적용)**: 두 ABP 모두 **Execute 가지의 평가기 클립만 바꾼다** — `ABP_Pig_A` 는 `AN_Pig_Catch` →
**`AN_Pig_Execute`**, `ABP_Raccoon_A` 는 `AN_Raccoon_Catch` → **`AN_Raccoon_Execute`**(둘 다 `/Game/Monsters/<종>A/`, 2.0초, `ExplicitTime = StateElapsed`
그대로). 변수·알파·자리는 Run 1 그대로. 클립 길이 = `ExecuteDuration` 이라 죽는 순간이 클립 끝이고, 그 뒤 `SpectateDelay` 까지 평가기가 마지막
프레임(= Idle 첫 프레임, 시체 옆에 선 자세)을 유지하다가 `Break` 로 알파 0 → 로코모션 Idle 과 같은 자세라 튀지 않는다. 표 `ExecuteDuration` 1.2 → 2.0
은 balance(짧으면 클립이 잘리고, 길면 선 자세로 기다린다 — 둘 다 허용).

#### `ABP_Unarmed` Executed 가지 사양 (리드가 MCP 로 만든다 — 2026-09-20, execute-sequence-pig Run 1)

처형당하는 농부(남이 보는 전신)의 표현. 읽는 게터(`ACL9Character`, BlueprintPure — gameplay): `bool IsExecuted()`(`ExecuteStartServerTime ≥ 0`),
`float GetExecuteElapsed()`(= 서버 시각 − `ExecuteStartServerTime`), `float GetExecuteDuration()`(= 범인 `ExecutedBy` 의 표 `ExecuteDuration`).
EventGraph 는 기존 `HoldPose`·`SwingElapsed`·`KnockdownElapsed` 세터 뒤에 변수 셋을 더 채운다:

| 변수 | 값 | 뜻 |
|---|---|---|
| `IsExecuted` (bool) | `IsExecuted()` | 처형 중(죽은 뒤에도 true 로 남는다 — 밤 종료 `HostEndExecuted`·부활 `ReviveDead` 에서 지워진다, D9. Run 2 가 이 잔류에 기댄다) |
| `ExecuteElapsed` (float) | `GetExecuteElapsed()` | 처형 시작 뒤 흐른 초 |
| `ExecuteDuration` (float) | `GetExecuteDuration()` | 죽는 시각(표 값 — 애님은 값에 맞춘다) |

AnimGraph — **넘어짐 가지보다 출력에 가까운 자리**(`ControlRig → [부울 블렌드 KnockdownElapsed≥0] → [부울 블렌드 Executed] → 출력`):
`부울로 포즈 블렌딩`, `bActiveValue = IsExecuted ∧ InRange(ExecuteElapsed, 0, ExecuteDuration)`, true 포즈 = **시퀀스 평가기 Idle**
(템플릿 로코모션의 Idle 클립, `ExplicitTime = 0` — 선 자세 정지), false 포즈 = 넘어짐 블렌드 출력. 블렌드 시간 0.1/0.0(진입은 짧게, 죽는
순간은 넘어짐 가지가 같은 틱에 이어받는다). 1인칭 `ABP_FP_View` 는 건드리지 않는다 — 피해자는 돼지 카메라를 보고 있어 자기 팔이
보이지 않는다.

- **우선순위는 Run 1 에서 고정한다(D7)**: Executed 가지가 넘어짐 가지 **위**(출력 쪽)에 있어야 한다. Run 1 에선 조건에 `InRange(…, 0,
  ExecuteDuration)` 이 들어 있어 `ExecuteDuration` 이 지나 `HostKill` 로 죽는 순간 조건이 꺼지고 **기존 넘어짐 사슬**(`AN_Farmer_Knockdown` →
  `Down` 정지)이 보인다 — "선 채로 얼어 있다가 쓰러진다".
- **Run 2 에서 바뀌는 것**: 조건이 `IsExecuted` 만으로(구간 검사 삭제), 포즈가 종별 `AN_Farmer_Executed_<종>` 평가기(`ExplicitTime = ExecuteElapsed`,
  길이 = `ExecuteDuration`, 끝 자세(DOWN/PRONE)를 골반 XY 0 으로 마지막 프레임 유지 — 부활까지)로 바뀐다. 그때 넘어짐 사슬은 처형
  사망에서 더 이상 보이지 않는다(Executed 가지가 이긴다). 가지 자리·변수 이름은 Run 1 그대로라 Run 2 는 조건 한 핀과 평가기 클립만 바꾼다.
- 왜 넘어짐보다 위인가: 처형 사망도 `HostKill` → 죽음 경로라 `KnockdownStartServerTime` 이 찍혀 넘어짐 가지가 켜진다. Run 2 의 전용 클립이
  그 위를 덮어야 하므로 Run 1 부터 같은 자리에 둔다(자리를 나중에 옮기면 두 번 배선한다).

**Run 2 사양(2026-09-21 execute-clips — 리드가 MCP 로 만든다)**. 클립은 종별 둘: `/Game/Characters/FarmerA/Anims/AN_Farmer_Executed_Pig`(정면 박치기 →
0.5 접촉 → 뒤로 넘어져 1.40 부터 DOWN = `AN_Farmer_Down` 첫 프레임과 같은 자세)·`AN_Farmer_Executed_Raccoon`(등 뒤 덮침 → 0.6 접촉 → 앞으로 엎어져 1.25 부터
PRONE), 둘 다 2.0초 = `ExecuteDuration`, 골반 위치 키 포함(넘어짐 클립과 같은 층 — 로코모션·발 IK 를 통째로 대체). 읽는 게터 추가(gameplay):
`FName GetExecutedByAlias()`(`pig`/`raccoon`/`None` — 범인 `ExecutedBy` 의 행 Alias, 죽은 뒤에도 `ExecutedBy` 가 남아 있는 동안 유효).

| 변수 | 값 | 뜻 |
|---|---|---|
| `ExecutedByPig` (bool) | EventGraph `SetExecuteDuration` 뒤 `CL9.GetExecutedByAlias() == "pig"` | 어느 종의 처형인가 — true 돼지, false 너구리(다른 처형 종이 생기면 종 수만큼 늘린다) |

AnimGraph 변경 세 가지(자리는 Run 1 그대로 — 넘어짐 블렌드 뒤·출력 앞):
1. 기존 `부울로 포즈 블렌딩`(Executed)의 `bActiveValue` 를 **`IsExecuted` 단독**으로 — `AND`·`InRange(ExecuteElapsed, 0, ExecuteDuration)` 노드 삭제.
   `IsExecuted` 는 죽은 뒤에도 부활(`ReviveDead`)·밤 종료(`HostEndExecuted`)까지 true 라(D9) 마지막 프레임이 그때까지 유지된다.
2. true 포즈를 새 **`부울로 포즈 블렌딩`**(`bActiveValue = ExecutedByPig`)으로 — true 포즈 = **시퀀스 평가기 `AN_Farmer_Executed_Pig`**, false 포즈 =
   **시퀀스 평가기 `AN_Farmer_Executed_Raccoon`**, 둘 다 `ExplicitTime = ExecuteElapsed`(클립 끝 2.0 을 넘으면 평가기가 마지막 프레임을 유지 — 클램프 노드
   없음). 이 안쪽 블렌드 시간 0/0(처형 중 종이 바뀌는 일은 없다).
3. Run 1 의 **Idle 평가기(`ExplicitTime 0`) 삭제**. 바깥 블렌드 시간은 Run 1 그대로 0.1/0.0.

죽는 순간(`ExecuteDuration`) 넘어짐 가지도 켜지지만 Executed 가지가 위라 보이지 않는다. 표 `ExecuteDuration` 이 2.0 과 다르면 짧을 땐 클립 중간에서
얼고 길 땐 끝 자세로 기다린다 — 길이가 확정되면 `execute_anchor.DURATION` 을 맞춰 네 클립을 다시 굽는다. 1인칭 `ABP_FP_View` 는 그대로(피해자는
몬스터 카메라 → 데스캠을 본다).

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
     : (Large ? FPHoldLarge : Flashlight ? FPHoldFlashlight : Weapon ? FPHoldWeapon : FPHoldSmall)`
     — `Weapon`(2026-10-20, 휘두를 수 있는 고기·프라이팬)은 Small 자리 앞에 부울 블렌드 한 단(`HoldPose == Weapon`)을 끼운 것이다
     (`Tools/Unreal/mcp_wire_fp_weapon_hold.py`).
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
  `FPSwing`(0.467s 원샷)·`FPEat`(0.6s 원샷, 입 앞 = 렌즈 앞 27cm 중앙) + 전신 `AN_Farmer_Swing` 30fps 재수출.
  2026-10-20 추가 `FPHoldWeapon`(2.0s 루프, 숨쉬기 손목 z ±0.5cm — HoldPose `Weapon`): 플레이테스트 3번 "들고 있는 모습만으로는 때릴 수
  있다는 느낌이 없다". 오른 주먹을 화면 오른쪽 가운데 아래로 치켜들고(손목 (−0.24, −0.40, 1.49)m·너클 uv (0.80, 0.63)·손목 uv (0.93, 0.72),
  팔꿈치 103°·손목 꺾임 23°), 아래팔이 우하단 구석에서 올라온다. 물건은 주먹에 몽둥이처럼 세워 쥐어 때리는 끝이 위·화면 안쪽으로 28° 기운다
  (미리보기는 고기 HeldScale 0.4 가정 — 끝 uv ≈ (0.66, 0.20). 실제 표 값은 0.3 이라 조금 더 작다) — 공격 대기(`FPSwingWindup`, 손목 (−0.17, −0.33, 1.55))보다 낮고 덜 젖힌 편한 대기 자세라
  화면 가운데는 비어 있다. 스펙 `farmer_hold_spec.json` `Weapon.fp`(1인칭 전용 — 전신 클립은 없다), 미리보기 `preview/hold_Weapon_fp_fp.png`
  (팔만)·`hold_Weapon_meat_fp.png`·`hold_Weapon_pan_fp.png`(물건을 꽂은 모습). 그립은 `measure_item_grip.py` 의 `club` 해. 제작 `Tools/Blender/
  export_farmer_fp_clips.py`(hold·fparms·swing·windup·eat·verify 일괄), 임포트 `Tools/Unreal/import_farmer_clips.py`(기본 목록 = 이 9클립)
  → `/Game/Characters/FarmerA/Anims`. `AN_Farmer_FPHoldIdle`·`ABP_FP_Copy` 는 참조 해제 뒤 삭제 여부 리드 판단.
- **전신 `ABP_Unarmed`** (남이 보는 것 — 2026-09-12~13 구조 유지): `Slot(DefaultSlot)` → `본별 레이어드 블렌딩`(`spine_01`, 로컬 공간,
  BlendDepth 0) → `ControlRig(발 IK)`. 레이어의 블렌드 포즈는 **부울 블렌드 2단** `HoldPose == Large ? HoldLarge : (HoldPose == Flashlight ?
  HoldFlashlight : HoldSmall)`, 가중치 `ToFloat(NOT(HoldPose == None))` — 빈손이면 템플릿 로코모션 그대로. `Weapon`(2026-10-20)은 따로
  배선하지 않는다 — 두 비교가 모두 거짓이라 `HoldSmall` 로 떨어진다(남이 보는 전신은 Small 들기 자세, 손의 물건은 같은 그립 행이라 위로 선다). 레이어 뒤 `Transform (Modify)
  Bone`(`spine_03`, 회전 Add, 피치 = `AimPitch × 0.6`, `AimPitch` ← `GetAimPitch()`)로 아래를 보면 상체가 기운다(계수는 초안 — PIE 로 확정).
  `BlendPoses(ECL9HoldPose)` 노드를 안 쓴 이유: 항목 핀 노출(`ExposeEnumElementAsPin`)이 에디터 컨텍스트 메뉴 전용이라 MCP/파이썬으로
  만들 수 없다. ABP 변수 `HoldPose`(ECL9HoldPose)는 EventGraph 가 매 틱 `Cast To CL9Character → GetHoldPose` 로 채운다(`SetIsFalling`
  뒤); 변수 생성은 `BlueprintEditorLibrary.add_member_variable` + `EdGraphPinType.import_text('(PinCategory="byte",
  PinSubCategoryObject="/Script/CL9.ECL9HoldPose")')`(MCP `add_variable` 은 프리미티브만 받는다).
  - **휘두르기 가지** (2026-09-13, 사용자 "아예 액션을 안 해"): 좌클 `Swing` 의 표현. 몽타주·RPC 가 아니라 **상태 파생** — `ACL9Character::
    SwingStartServerTime`(복제)에서 `GetSwingElapsed()` 가 지난 초를 주고 ABP 변수 `SwingElapsed` 가 매 틱 받는다(2026-09-18 — ABP 변수 소스는 `GetSwingAnimElapsed()`, 히트스톱 0.08초가 얹힌 값. 판정은 여전히 `GetSwingElapsed()` — character.md 피격 절). Hold 체인 머리 앞에
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
| 잡힘·넘어짐 | CatchRadius 판정 · 캐릭터 `KnockdownTime` 1.5초 | 짐은 흘리지 않는다(죽을 때만, character.md). 몬스터 `Catch` 0.8초, 캐릭터 `Knockdown` 0.5 → `Down` 루프 → `GetUp` 0.6 (2026-09-15) |
| 던지기 | 놓는 순간 판정 | `ThrowWindup` 루프(차징) → `Throw` 0.333초, 끝 = 기준 자세 (2026-09-15) |
| 기상 예고 | `WakeTelegraph` 0.8s | 퍼덕임 + 울음 |
| 잡기 예고 | 행 `PounceWindup` 0.6s | 닭만 `AN_Chicken_PounceWindup`(몸 젖히고 날개 벌림, 끝 = Catch 첫 프레임) — 헛침은 `Catch` 앞 0.3초. 다른 종은 클립 없음 (2026-09-20) |
| 처형 | 처형 종(`IsExecutioner()` — 돼지·너구리)만 · 길이 `ExecuteDuration` 2.0 | **전용 2인 클립**(2026-09-21 Run 2, execute-clips) — 몬스터 `AN_Pig_Execute`(정면 박치기, 접촉 0.5)·`AN_Raccoon_Execute`(등 뒤 도약, 접촉 0.6) 평가기 `ExplicitTime = StateElapsed`, 끝 = Idle 첫 프레임(시체 옆에 서서 `SpectateDelay` 까지); 농부 `AN_Farmer_Executed_Pig`(뒤로 넘어져 DOWN)·`AN_Farmer_Executed_Raccoon`(앞으로 엎어져 PRONE) 평가기 `ExplicitTime = ExecuteElapsed`, 마지막 프레임을 부활까지 유지. 앵커 표 `Tools/Blender/execute_anchor.py`. 이력: Run 1(2026-09-20)은 Catch 클립 재사용 + 농부 Idle 정지 → 죽는 순간 넘어짐 사슬이었다 |
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


## 2026-10-17 소·닭 새 본체의 시간 이동 공격

`/Game/Monsters/MotionReview20261017/AttackV01/ABP_{cow,chicken}_Attack`은 승인 v07 본체와 제자리 로코모션을 유지한다. `GetPounceTiming()`은 행의 (예고, 이동, 복귀) 시간 벡터. Pounce의 SequenceEvaluator 시간은 `GetStateElapsed()`, 명중 Catch의 시간은 예고+이동+`GetPounceResultElapsed()`. 공격 포즈 블렌드 뒤 명시적 local↔component 변환과 접촉 커브 월드 발 고정 IK를 둔다. 판정/연속 이동/방향 고정/복귀는 공유 서버 AI, 애님 노티파이는 판정을 실행하지 않는다. 기존 일반 ChickenA ABP는 그대로다. 상세 제작·영상·검수 한계는 [현재 카드](../asset/monster/disguise-trio-next-production.md) 상단.


### 구매 공격 리타깃 v06 (2026-10-17)

`AttackPackV01/ABP_{cow,chicken}_Attack`이 현재 새 본체 소비 경로다. 원래 v07 메시/바인드와 검증한 제자리 로코모션을 유지하고, 구매 모션을 Blender에서 체형별로 보정한 공격만 대체한다. Pole_i_X/Y/Z 커브는 공격 포즈의 저작된 관절 굽힘 방향을 전달하며, AttackAlpha 블렌드 때 기존 로코모션 폴과 연결한다. 이동/명중/복귀는 기존 공유 서버 상태 시간으로 처리한다. 닭 새 본체의 이동/복귀 초안은 0.42/0.72초, 일반 ChickenA 행은 변경하지 않았다. 실제 영상·후보 선택·검수 한계는 현재 제작 카드 상단 참조.

## 계란 요리사 동작 (2026-10-23, round_cook_trial)

플레이어 기본 캐릭터를 계란 요리사(`CharacterTable.round_cook_trial`)로 바꾸기 위해, 농부의 동작 클립을 계란 리그에 다시 저작해 새 애님 BP 에 연결했다.
원본 `ABP_EggCrouchV04_Body_Fold`·`ABP_GripV02_Arms` 는 그대로 두고 복제본 `/Game/Characters/FarmerA/EggActionsV01/ABP_EggActions_Body`·`ABP_EggActions_Arms`
를 만들었으며, 행의 AnimBP·ArmsAnimBP 가 이 복제본을 가리킨다(`farmer_a` 행은 그대로).

- **클립**: 전신 16 개(HoldSmall·HoldLarge·HoldFlashlight·SwingWindup·Swing·ThrowWindup·Throw·Eat·Knockdown·Down·GetUp·Executed_Pig·Executed_Raccoon·Jump·FallLoop·Land),
  1인칭 팔 13 개(FPArmsDown·FPRun·FPHold{Small,Large,Flashlight,Weapon}·FPSwingWindup·FPSwing·FPThrowWindup·FPThrow·FPEat·FPKnockdown·FPGetUp), 이름은 `AN_Egg_<이름>`, 30fps.
  길이·반복·키 시각은 농부 manifest(`ArtSource/character/farmer_a/manifest.json` `animations`)의 같은 이름 항목, Jump/FallLoop/Land 는 MM_Jump·MM_Fall_Loop·MM_Land(0.867·3.0·0.867초).
  제작 `Tools/Blender/animate_egg_actions_v01.py`, 임포트 `Tools/Unreal/import_egg_actions_v01.py`(압축 포즈 대조), 원본·미리보기 `ArtSource/character/farmer_a/egg_actions_v01/`.
- **계란은 단단하다**: 상체 레이어 클립(spine_01 위)은 척추를 기준 자세로 두고 팔만 움직인다(몸통을 비틀면 계란이 찌그러진다 — egg_crouch_v04 원칙).
  넘어짐·처형·점프류는 골반 회전·위치로 계란 전체를 굴리고, 매 프레임 메시 최저점을 바닥에 맞췄다. 처형 2종은 농부 `execute_anchor.farmer_times`
  (돼지 접촉 0.5·DOWN 1.4 / 너구리 접촉 0.6·PRONE 1.25)에 맞물리고 마지막 자세에서 멈춘다. 팔이 짧아(어깨 0.76m·팔 0.25m) 먹기는 입 옆 뺨 높이까지만 올라간다.
- **몸 리그의 `_r` 뼈는 캐릭터의 왼쪽에 있다**(이름이 거울). 남이 보는 전신에서는 아이템이 캐릭터 왼손에 들린다 — 1인칭 팔 리그는 오른쪽이 맞다.
- **1인칭**: 카메라 공간 클립이라 농부 1인칭 키의 손목·방향을 좌표만 옮겨(농부 −Y 정면 → 계란 +X 정면) 화면 위치가 농부와 같다. 어깨가 낮아 닿지 않는 목표는
  화면 밖 쇄골을 돌려 어깨를 옮긴다(팔 메시는 쇄골에 가중치가 없다). 지정이 없는 팔은 FPArmsDown(프레임 밖) 자세.
- **소켓 `HandGrip_R`**: SK_RoundCook_Body·SK_RoundCook_Arms·SK_GripV02_Body·SK_GripV02_Arms 의 hand_r 아래, 장갑 손바닥 중심(몸 5.9cm·팔 4.5cm),
  축은 마네킹과 같은 규약(+X 손바닥 바깥, +Y 손가락 쪽). 계란 리그는 루트 뼈가 100배 스케일(Blender m)이라 소켓 상대 스케일을 0.01 로 두어 소켓 월드 스케일을 1 로 맞췄다
  (아니면 아이템 GripOffset cm 가 100배로 먹는다). `Tools/Unreal/add_egg_handgrip_sockets.py`.
- **ABP 연결** (`Tools/Unreal/connect_egg_actions_v01.py`): 변수·게터·조건은 ABP_Unarmed·ABP_FP_View 그대로(위 "1인칭 팔"·"전신 ABP_Unarmed" 절).
  전신 순서 = 로코모션·수레밀기 → 앉기 → **점프 가지** → 상체 레이어(spine_01) → 수레 IK → 넘어짐 → 처형 → 출력. 점프 가지는 ABP_Unarmed 의 MainStates 상태 기계 대신
  상태 파생 값으로 고른다 — `IsFalling` 이면 `AirTime < 0.867` 동안 Jump 평가기(`ExplicitTime = AirTime`) 다음 FallLoop, 착지 뒤 `LandElapsed < 0.867 ∧ LastAirTime > 0.2 ∧ MoveAlpha < 0.5`
  동안 Land 평가기(걷기 시작하면 바로 로코모션). `AirTime`·`LandElapsed` 는 EventGraph 가 DeltaTime 으로 쌓는다. 앉으면(CrouchActive) 기존 앉기 분기가 그대로 고른다.
  1인칭은 ABP_FP_View 의 우선순위 사슬(넘어짐 > 던짐 > 던지기 준비 > 먹기 > 휘두르기 준비 > 휘두르기 > 들기/빈손)을 옛 AN_Arms 자리에 넣고 수레 IK 는 그 뒤에 둔다.
  ABP_Unarmed 의 AimPitch(spine_03 기울임)는 옮기지 않았다 — 계란 몸통이 찌그러진다.
  상체 레이어 가중치는 `HoldPose != None ∨ 던지기 ∨ 먹는 중` 이다 — 먹는 순간 요리가 소모돼 HoldPose 가 None 이 되므로 먹기 항을 더했다
  (ABP_Unarmed 에는 이 항이 없어 남이 보는 농부의 먹기가 보이지 않는다). 수레 EventGraph 의 HoldPose switch 는 "None 이면 보간, 아니면 0" 선택식으로 옮겼다(Weapon 도 0).
- **PIE 확인**(2026-10-23, `Saved/Film/egg_probe*/probe.jsonl` — 봇 `probe` 동작이 몸·팔 메시의 애님 클래스·골반·손 위치·PushAlpha 를 찍는다):
  휘두르기 대기 손 99.8cm → 0.16초 69cm, 먹기 0.26~0.36초 손 97cm, 점프 공중 손 88cm·착지 골반 32cm, 앉기 골반 23.5cm, 넘어짐 누움 골반 55cm → 1.3초 일어나기,
  돼지 처형 0.6초 뒤 뒤로 넘어가 1.4초 누움, 수레 PushAlpha 1·손 IK 손잡이(97cm). 시험 맵(혼자)에서는 처형 사망 뒤 2.2초에 다시 선 자세가 된다.
- 상체 레이어 가중치는 블렌드 시간이 없어 들기·먹기가 끝나는 프레임에 팔이 기준 자세로 한 번에 돌아온다(농부와 같음).
