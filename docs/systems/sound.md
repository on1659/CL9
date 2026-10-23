# 사운드 시스템

> 사운드 분류, 이벤트 목록, 에셋 위치, 재생·볼륨 설정.
> 호출 규칙(표현 계층에서만 재생, Multicast 동기화)은 [../conventions.md](../conventions.md) § 9 참고.

## 원칙

모든 게임 이벤트에 사운드를 첨부한다. 사운드 호출은 표현(클라) 레이어에서 발생.

## 사운드 분류

| 분류 | 용도 | 예시 | 재생 방식 |
|------|------|------|----------|
| **SFX (Effect)** | 게임 동작 효과음 | 줍기음, 깨짐음, 반납음 | 2D 또는 3D (근처) |
| **UI Sound** | 메뉴 조작음 | 버튼 클릭, 성공음 | 2D (UI 레이어) |
| **Ambience** | 배경음 | 바람, 닭장 소음 | 3D 루프 |
| **Music** | 배경음악 | 게임 플레이 BGM, 메뉴 BGM | 2D 루프 |
| **VO (Voice)** | 반응 음성 | 캐릭터 `VoiceSet` — 잡힘·넘어짐·탈진 | 3D (근처) |
| **음성 스트림** | 동료의 마이크 말소리 (2026-10-11, [voice.md](voice.md)) | 사운드 파일이 아니라 EOS Voice(Redpoint) 재생 (2026-10-12 — 옛 엔진 VOIP 를 대체). 볼륨 경로는 리드 확인 대기: `SC_Voice` 는 Redpoint 가 언리얼 오디오 컴포넌트로 재생할 때만 닿는다 | 3D (말하는 사람 자리, 핫라인 중에는 받는 사람 무전기 자리 하나) 또는 2D (죽은 화자) |

## 호출 규칙

- **절대** 판정 코드(GameMode, 서버)에서 직접 재생하지 않음 — 항상 표현 계층에서
- 동기화는 **호스트발 Multicast RPC(Unreliable — 사운드는 유실 허용)** 또는 Replicated
  상태의 OnRep에서
- 클라발 Server RPC로 사운드를 요청하지 않는다 (비소유 액터 함정 — actor-base.md)
- **구현된 효과음(2026-09-18)은 Multicast도 OnRep도 아니다** — 이미 복제되는 시계·상태(`SwingStartServerTime`·
  `SwingSurfaceHitServerTime`(2026-09-22)·`KnockdownStartServerTime`·몬스터 `StateStartServerTime`·아이템 `HoldState`)를 매 틱 지난 틱과 비교해 **상승 에지**에서
  모든 머신이 각자 튼다. 새 RPC가 없고, 리슨 호스트도 자기 폰에서 같은 길을 탄다(OnRep은 호스트 자신에게 뜨지 않는다 —
  character.md 피격 절의 셰이크와 같은 이유). 아래 "구현 — `SoundTable`" 절. **예외 둘.** 첫째 — 음성 스트림(2026-10-11, §9 예외): 동료의 마이크 말소리는
  EOS RTC 로 오가고 Redpoint 가 재생한다(2026-10-12 — 옛 엔진 `VoiceChannel`·`UVOIPTalker` 를 대체) — 서버 시간 틱 비교의 대상이 아니고 `CL9Sound::PlayAt` 도 거치지 않는다. 새 Multicast 도 없다.
  자리·감쇠·볼륨은 [voice.md](voice.md). 둘째(2026-09-20): `MonsterNear`는 파동
  멀티캐스트(`Multicast_Noise`, Unreliable)에 얹혀 온다 — 파동 표 행의 `SoundRow` 열이 "이 소음이 나면 낼 소리"를 가리킨다.
  파동이 이미 그 채널을 쓰고 있어 새 RPC는 여기에도 없다([wave.md](wave.md)).
- **임시 규칙 — 일상 동작 소리는 소유자 전용이다** (2026-10-05, [../goal/control-feel.md](../goal/control-feel.md) C14). 새 행
  `Jump`·`LandSoft`·`LandHard`·`SlotSwitch`·`LightToggle`·`Eat`·`Exhausted`·`CartGrab`과 기존 `Pickup` 행은 **그 폰을 조종하는
  머신에서만**(`IsLocallyControlled`) 난다 — 위의 "모든 머신이 각자 튼다"(conventions § 9)의 예외다. 이유: `PlayAt`에 감쇠가 없어
  (`PlaySoundAtLocation`에 Attenuation 을 넘기지 않고 에셋에도 없다) 남에게 들리게 하면 네 명의 점프·착지·줍기가 맵 전체에 같은
  크기로 깔리고, 그걸 고치려면 감쇠 에셋과 2인 검증이 필요하다. 감쇠 에셋(`SA_CL9_Near/Mid` 류)이 생기면 동료에게 들리는 쪽을
  **따로** 붙인다(2인 세션 후속). 이 소리들은 몬스터가 듣는 소음을 만들지 않는다 — 기존 `Land` 파동·줍기 소음 반경은 그대로다.
  재생 지점과 에지는 [character.md](character.md) "일상 동작 소리".

## 주요 사운드 이벤트 (현 기획 기준)

이름이 **표 행**(`Pickup`처럼 접두사 없는 이름)이면 `SoundTable`에 행·에셋이 있고 코드에 재생 지점이 붙은 것이고,
`SFX_…`·`UI_…`·`VoiceSet`은 아직 **계획 이름**이다(에셋도 재생 지점도 없다). 행이 된 옛 이름은 괄호에 남겼다.

**운반 (inventory.md · raiding.md):**
- `Pickup` 행 (옛 `SFX_Pickup`) — 줍기와 헤드램프 착용 (걷기 줍기는 조용, 달리기 줍기는 시끄러움 — 소음 반경은 rules.md
  판정 값이고 그대로다). 2026-10-05 부터 **소유자 전용**으로 난다 — 소유 폰이 자기 PlayerState 의 채워진 칸 수 증가·머리 칸
  채워짐을 지난 틱과 비교해 튼다(`PickupServerTime` 복제는 만들지 않았다, control-feel C14).
- `SlotSwitch` 행 (2026-10-05) — 활성 칸 전환(줍기로 바뀐 것은 `Pickup`만). 소유자 전용.
- `LightToggle` 행 (2026-10-05) — 든 손전등·쓴 헤드램프의 스위치를 바꾼 순간(정전이 끈 것은 빼고). 소유자 전용.
- `CartGrab` 행 (2026-10-05) — 수레를 잡은 순간. 소유자 전용.
- `Place` 행 (옛 `SFX_Place`·`SFX_Throw` — 하나로 합침) — 놓기·던지기·수레 짐칸에 얹기(G) 모두. 아이템 `HoldState`가
  Held·Worn에서 Free로 바뀌는 순간 아이템 자리에서. 수레를 놓는 것(Pushed→Free)과 죽음 드랍(Free→Free)에는 안 난다.
- `SFX_Crack` / `SFX_Break` — 금 감 / 깨짐
- `SFX_JarDeposit` — 항아리 반납

**겁먹는 표현 (character.md 겁먹는 표현, 2026-10-17 — 전부 소유자 전용, `UCL9FearComponent`):**
- `FearLow`·`FearHigh1`·`FearHigh2` 행 — 공포 게이지 문턱(`CL9Rules.FearLowMin/Max`·`FearHigh1Min`·`FearHigh2Min`)으로 켜지는 2D 루프 세 겹.
  볼륨은 코드가 `FearLayerFadeSpeed` 로 0↔1. **애셋 미정** — Looping 필수.
- `Heartbeat` 행 — 2D 루프, 볼륨 = 게이지. **애셋 미정**.
- `MonsterSighting` 행 — 몬스터가 내 화면에 처음 들어온 순간 몬스터 자리에서 한 번(`SightingRadius`·`SightingCooldown`). **애셋 미정**.
- `MonsterChaseBegin` 행 — 몬스터가 Chase 로 들어오는 전이가 `ChaseBeginRadius` 안이거나 화면에 보일 때 몬스터 자리에서 한 번. **애셋 미정**.

**요리 (cooking.md):**
- `KitchenAmbience` 행 (2026-10-09, [../goal/kitchen-identity.md](../goal/kitchen-identity.md)) — 조리대의 가스 쉬익 + 후드 팬 웅웅 **루프**.
  사건이 아니라 늘 켜진 **시설물 소리**다: 전용 서버가 아닌 모든 머신에서 조리대마다 `BeginPlay`에 켜져 판 내내 난다. 감쇠·동시 발성
  제한 애셋을 쓰는 **첫 효과음**이다(아래 구현 절). 몬스터가 듣는 소음은 만들지 않는다.
- `CookSizzle` 행 (옛 `SFX_Sizzle`, 2026-10-09 [../goal/cooking-progress-feedback.md](../goal/cooking-progress-feedback.md)) — 지글지글 **루프**.
  조리대가 익히는 동안·타는 동안 내내(표현 단계 Cooking·Burning·BurnWarning) 조리대 자리에서. 비면 멈춘다.
- `CookCrackle` 행 (옛 `SFX_Smoke` 의 소리 쪽) — 거칠게 탁탁 튀는 **루프**. 탈 때까지 행의 `BurnWarnTime` 초 안(BurnWarning)에만 지글 위에 겹친다 —
  "지글이 거칠어진다"가 이 층이다.
- `CookDone` 행 (옛 `SFX_CookDone`) — 익음 전이 한 번("치익", egg → egg+cooked). 재료 자리에서.
- `CookBurnt` 행 (옛 `SFX_Burnt`) — 탐 전이 한 번(둔한 "퍽" + 짧은 쉿). 재료 자리에서.
- 넷 다 전용 서버가 아닌 **모든 머신**이 복제된 조리 상태에서 파생해 낸다(`ACL9Kitchen::TickLook` — [cooking.md](cooking.md) 요리 진행 표현).
  감쇠는 애셋(`SA_CL9_Kitchen`)에 있어 `PlayAt` 의 "감쇠 없음"에 걸리지 않는다. 몬스터가 듣는 소음은 만들지 않는다.

**몬스터 (monster.md):**
- `SFX_WakeTelegraph` — 기상 예고 퍼덕임·울음 (0.8초 — 전 플레이어에게 들린다)
- `SFX_Alert` — 경보 울음 (거위 `AlertWindup` 뒤)
- `MonsterPounce` 행 (2026-09-20) — **잡기 예고**에 들어가는 순간 몬스터 자리에서(꽥+퍼덕임). 몬스터 `State`가 `Pounce`로
  **전이**하는 것을 모든 머신이 감지해 튼다(아래 표). 예고가 헛치면 이 소리 뒤에 아무것도 없다.
- `MonsterCatch` 행 (옛 `SFX_Catch`, 2026-09-20) — 잡힘, 몬스터가 덮치는 소리. 몬스터 `State`가 `Catch`로 전이하는 순간 몬스터
  자리에서 — 잡힌 사람 쪽 `Hurt`(맞은 몸의 소리)와 **같은 순간에 겹친다**(둘 다 `SCC_Effects4` 안이라 함께 난다).
- `MonsterNear` 행 (2026-09-20) — 다가오는 소리(짧은 꼬꼬댁). 재생 지점이 몬스터 코드가 아니라 **파동**이다: 근접 소나 사건
  `monster_prox`([wave.md](wave.md))의 `SoundRow` 열이 이 행을 가리켜 링과 같은 박자(`RepeatInterval` 1.2초)로 난다. 링이
  안 그려져도(컬링 밖) 소리는 나고, 행이 꺼지면 사건 자체가 없어 소리도 없다. 잠든 계열은 소나가 없으니 조용하다.
- `MonsterDeath` 행 (옛 `SFX_MonsterDeath`) — 사살 (사살 경보의 소리 그 자체). 몬스터가 Dead에 들어오는 순간 `Hit`과
  **함께** 몬스터 자리에서 (monster.md "타격의 표현").
- `Footstep` 행 (옛 `SFX_Footstep_Monster`) — 몬스터 발소리 (읽히는 위협의 청각 채널). 사람과 **같은 행·같은 보폭**을
  쓴다(2026-09-18 사용자 — 종별 보폭 열은 이번엔 두지 않는다). Chase면 달리기 보폭, 그 외는 걷기 보폭, Dead·정지면 안 난다.

**타격·피격 (goal/game-feel.md §1 — 2026-09-18 구현):**
- `Swing` 행 — 휘두르기 시작(`SwingStartServerTime` 변화). **맞든 빗나가든** 난다 — 헛스윙에는 이것만 나고 `Hit`은 없다.
- `Hit` 행 (옛 `SFX_Hit`) — 팬·고기가 몬스터에 닿는 순간 = 몬스터가 Stun·Dead에 들어오는 상승 에지(빨간 플래시·히트스톱·
  퍼프와 같은 에지 — testing/pie-scenarios.md C4 ⑩). 경직 중 다시 맞으면(`StateStartServerTime`이 바뀜) 또 난다. 나란한 둘을
  한 번에 치면 **닭마다 하나씩** 난다(카메라 킥은 휘두르기당 한 번이지만 소리는 맞은 몬스터가 낸다). 시체를 쳐도 안 난다.
- `Hurt` 행 (옛 `SFX_Hurt`) — 잡혀 넘어지는 순간, 맞은 사람 쪽(`KnockdownStartServerTime` 변화 — C3 피격감). 일어날 때는
  없다. 남의 화면에서도 그 폰 자리에서 들린다.
- `HitSurface` 행 (2026-09-22, goal/combat-feel-pass-1.md D-4) — 휘두르기가 벽·바닥에 닿은 순간(`SwingSurfaceHitServerTime`
  변화 — 조건 없는 복제라 남의 폰에서도 난다). 휘두르기당 한 번, 그보다 가까운 몬스터 명중이 없을 때만 — 허공·벽·몬스터가
  `Swing`·`HitSurface`·`Hit` 세 소리로 갈린다. 충돌점은 복제하지 않아 **폰 자리**에서 난다. 재질별 소리·킥·소음 사건은 없다.

| 행 | 에지 | 어느 머신 | 어디서 | 위치 |
|---|---|---|---|---|
| `Swing` | `SwingStartServerTime` 변화 | 모든 머신 | `ACL9Character::UpdateHitSounds` | 폰 |
| `HitSurface` | `SwingSurfaceHitServerTime` 변화 (휘두르기당 1회, 호스트 `HostTickSwingHit`의 표면 선 트레이스) | 모든 머신 | `ACL9Character::UpdateHitSounds` | 폰 |
| `Hurt` | `KnockdownStartServerTime` 변화 | 모든 머신 | `ACL9Character::UpdateHitSounds` | 폰 |
| `Hit` | Stun·Dead ∧ `GetStateElapsed() < HitFlashTime` 창의 상승 에지, 또는 창 안에서 `StateStartServerTime` 변화 | 모든 머신 | `ACL9Monster::TickHitFeel` | 몬스터 메시 중심(`Bounds.Origin`) |
| `MonsterDeath` | 같은 에지인데 상태가 Dead | 모든 머신 | `ACL9Monster::TickHitFeel` | 몬스터 메시 중심, `Hit`과 함께 |
| `MonsterPounce` | **상태 전이** `State == Pounce ∧ (지난 틱 State != Pounce ∨ StateStartServerTime 변화)` — 창(elapsed)이 아니라 전이라 늦게 온 복제도 놓치지 않는다. 첫 틱은 기준값만(`bStateSoundPrimed`) | 모든 머신 | `ACL9Monster::TickStateSounds` (`TickHitFeel` 다음) | 몬스터 메시 중심 |
| `MonsterCatch` | 같은 전이 감지, 상태가 Catch. `TickHitFeel`의 Stun·Dead 창과 별개라 Catch가 관측 전에 Stun·Dead로 덮이면(같은 틱 피격) 그 잡기 소리는 빠진다 — `Hurt`가 이미 나므로 허용 | 모든 머신 | `ACL9Monster::TickStateSounds` | 몬스터 메시 중심, 잡힌 폰의 `Hurt`와 같은 순간 |
| `MonsterNear` | 근접 소나 사건 `monster_prox`가 이 머신에 도착할 때(호스트는 `HostReportNoise` 직후, 클라는 `Multicast_Noise`) — 파동 행의 `SoundRow` | 모든 머신 | `UCL9WaveRenderer::PlayWave` (`MinInterval` 뒤·시각 게이트 앞) | 소음 발원지(몬스터 자리) |
| `Place` | 아이템 `HoldState` Held·Worn → Free | 모든 머신 | `ACL9Item::ApplyHoldState` (호스트는 OnPlaced/OnThrown 경유, 클라는 OnRep) | 아이템 |
| `Footstep` | 사람: 걸음 위상이 1 을 넘음(볼륨 0.7 → 1.0, 2026-10-05) · 몬스터: 수평 이동 거리 누적 ≥ 보폭 | 모든 머신 | `ACL9Character::TickFootstepSound` · `ACL9Monster::TickFootstep` | 발밑 |
| `Jump` | 점프한 순간 — 보정 리플레이 제외, 0.15초에 한 번 | **소유자만** | `ACL9Character::OnJumped_Implementation` | 발밑 |
| `LandSoft` · `LandHard` | 착지 낙하 속도 350~1000 cm/s = Soft(볼륨 0.5 → 1.0), 1000 이상 = Hard — 같은 가드 | **소유자만** | `ACL9Character::Landed` | 발밑 |
| `Pickup` | 채워진 퀵슬롯 칸 수 증가 · 머리 칸 채워짐 | **소유자만** | `ACL9Character::UpdateOwnerSounds` | 폰 |
| `SlotSwitch` | 활성 칸 변화(줍기가 아닐 때) | **소유자만** | `ACL9Character::UpdateOwnerSounds` | 폰 |
| `LightToggle` | 이 머신이 아직 들고(쓰고) 있다고 보는 손전등·헤드램프, 같은 액터의 `bLightOn` 변화 — 조명 키(F·든 손전등 좌클)를 누른 뒤 0.5초 안일 때만, 정전으로 꺼진 것 제외 | **소유자만** | `ACL9Character::UpdateOwnerSounds` | 폰 |
| `Eat` | `EatStartServerTime` 변화 | **소유자만** | `ACL9Character::UpdateOwnerSounds` | 폰 |
| `CartGrab` | `PullingCart` null → 수레 | **소유자만** | `ACL9Character::UpdateOwnerSounds` | 폰 |
| `Exhausted` | 탈진 래치(CMC 예측) 상승 에지, 2초에 한 번까지 | **소유자만** | `ACL9Character::UpdateOwnerSounds` | 폰 |
| `KitchenAmbience` | 에지가 아니다 — 조리대 `BeginPlay`에서 한 번 켜고 판 내내 루프(반복은 애셋의 Looping) | 모든 머신(전용 서버 제외 — 프로세스·월드 넷 모드 둘 다) | `ACL9Kitchen::BeginPlay` → `CL9Sound::Resolve` → `AmbienceAudio->Play()` | 조리대(몸체 원점에 붙은 `AmbienceAudio`) |
| `CookSizzle` | 표현 단계가 Idle ↔ (Cooking·Burning·BurnWarning)으로 바뀔 때 Play/Stop — 음원은 조리대 `BeginPlay`에서 `Resolve`로 넣어 둔다 | 모든 머신(전용 서버 제외 — `TickLook` 자체가 안 돈다) | `ACL9Kitchen::TickLook` → `SizzleAudio->Play()/Stop()` | 조리대(몸체에 붙은 `SizzleAudio`) |
| `CookCrackle` | 표현 단계가 BurnWarning에 들고 날 때 Play/Stop — 음원은 같은 식 | 모든 머신(같음) | `ACL9Kitchen::TickLook` → `CrackleAudio` | 조리대(`CrackleAudio`) |
| `CookDone` | 조리대가 지켜보던 재료의 (Alias, SubAlias)가 지난 틱과 다르고 새 행이 **지난 행의 `CookResult`**. 첫 관측은 기준값만 | 모든 머신(같음) | `ACL9Kitchen::TickLookEdges` → `PlayAt` | 재료(`GetActorLocation`) |
| `CookBurnt` | 같은 에지인데 새 행이 **지난 행의 `BurnResult`**. 금 감 같은 다른 행 변화에는 소리가 없다 | 모든 머신(같음) | `ACL9Kitchen::TickLookEdges` → `PlayAt` | 재료 |

**캐릭터:**
- `VoiceSet` 반응 음성 — 잡힘·넘어짐·탈진 (동료 상황을 귀로 알리는 채널 — character.md)
- `Footstep` 행 (옛 `FootstepSet`) — 발소리 (연출 전용 — 몬스터 청각 판정과 무관). **애님 노티파이가 아니라 걸음 위상**이다
  (2026-10-05): 땅 위에서 움직인 수평 거리를 보폭으로 나눠 위상에 더하고 1 을 넘을 때마다 발밑에서 한 번, 피치 ±8 %. 보폭은
  수평 속도로 `CL9Rules.FootstepStrideWalk` 150cm → `FootstepStrideRun` 196cm 를 선형으로 잇고(달리기 입력은 남의 폰에 복제되지
  않아 속도로 가른다), 볼륨은 세 단계로 섞지 않는다(2026-10-22) — 걷기 `ViewFeel.FootstepVolumeWalk` 0.7, 뛰기(쉬프트 — 속도 비율 `FootstepRunAlpha` 0.5 이상) `FootstepVolumeRun` 1.0, 앉아 걷기(엔진 웅크리기 `bIsCrouched`) `FootstepVolumeCrouch` 0 — 0 인 단계는 틀지 않는다. 서 있거나 공중이거나 넘어졌으면 안 난다.
  소유자의 카메라 보브가 같은 위상을 읽는다 — 발소리와 보브의 골이 한 시계다. 소음 판정(파동, `TickStepNoise`)과는 별개 누적이다 —
  character.md 피격 절.
- `Jump`·`LandSoft`·`LandHard` 행 (2026-10-05) — 점프·가벼운 착지·무거운 착지. 소유자 전용(위 임시 규칙). 착지 소리 문턱과 카메라 딥
  문턱이 같은 `CL9Rules.ViewFeel.LandMinSpeed`(350)·`LandHardSpeed`(1000)다.
- `Eat` 행 (2026-10-05) — 먹은 순간. 소유자 전용.
- `Exhausted` 행 (2026-10-05) — 탈진한 순간의 한 번 헐떡임(루프 없음). 소유자 전용, 2초 대기. 동료에게 들리는 탈진 음성은 탈진이
  남에게 복제되지 않아 `VoiceSet`과 함께 후속이다.

**거래·전이:**
- `SFX_Sell` / `SFX_Buy` — 창구 판매 / 트럭 패널 구매
- `SFX_OrderComplete` — 주문 납품 완료
- `SFX_Blackout` — 정전 (light.md — 소등 연출과 함께)

**UI:**
- `UI_Click` · `UI_Open` · `UI_Close` · `UI_Confirm` · `UI_Error`

## 에셋 위치

```
Content/Sounds/
  ├─ SFX/          (Carry/ Character/ Monster/ — 2026-09-18 실재 · Cooking/ Trade/ — 계획)
  ├─ VO/           (캐릭터별 VoiceSet)
  ├─ UI/
  ├─ Ambience/     (Henhouse/ — 목적지별)
  ├─ Music/        (GameplayBGM, MenuBGM)
  └─ Master/       (MasterSoundMix — 클래스별 게인 · SC_Voice — 음성 스트림 클래스)
```

효과음 에셋 → 행 연결은 `Content/Data/SoundTable`이 갖는다(아래 구현 절의 표). 원본 WAV는 `ArtSource/audio/sfx/`.

## 구현 — `SoundTable` (2026-09-18, game-feel §1)

**표.** `UCL9SoundTable`(`Source/CL9/Data/CL9SoundTable.h`, DataAsset — `UCL9ShakeTable`과 같은 틀)은
`TMap<FName, TSoftObjectPtr<USoundBase>> Rows` 하나다. 효과음을 추가한다는 것은 사운드 에셋을 만들고 여기에 행을 더하는
것이지 rules 에셋에 열이 늘어나는 일이 아니다. C++가 조회하는 행 이름은 `CL9SoundNames` 상수 30개(+ 배경음악 3개 `MenuBGM`·`LobbyBGM`·`NightBGM`, 2026-10-17 — 아래 "배경음악" 절; 겁먹는 표현 6개
`FearLow`·`FearHigh1`·`FearHigh2`·`Heartbeat`(루프)·`MonsterSighting`·`MonsterChaseBegin`, 2026-10-17 — [character.md](character.md) 겁먹는 표현) — `Pickup`·`Place`·`Swing`·
`Hit`·`Hurt`·`MonsterDeath`·`Footstep`·`MonsterPounce`·`MonsterCatch`·`MonsterNear`(뒤 셋은 2026-09-20)·`HitSurface`(2026-09-22)·
`Jump`·`LandSoft`·`LandHard`·`SlotSwitch`·`LightToggle`·`Eat`·`Exhausted`·`CartGrab`(2026-10-05, 소유자 전용)·`KitchenAmbience`
(2026-10-09, 시설물 루프)·`CookSizzle`·`CookCrackle`·`CookDone`·`CookBurnt`(2026-10-09, 요리 진행) — 그 외 행은 데이터 전용이다. 다만 **파동 표의 `SoundRow` 열은 어느 행이든 이름으로 가리킬 수 있다**([wave.md](wave.md)) — `MonsterNear`는 C++
상수가 있지만 재생 지점은 그 열이다. 에셋은 `/Game/Data/SoundTable`이고
`BP_CL9GameInstance.SoundTableAsset`(`UCL9GameInstance`, `ShakeTableAsset`과 같은 자리)이 가리킨다 — C++에는 에셋 경로가
없다(conventions § 5). `UCL9GameInstance::Init`이 한 번 로드해 `GetSoundTable()`로 내주고, 비어 있으면 Warning 한 줄이다.
**`Init`은 `Rows`의 음원을 전부 `LoadSynchronous`로 미리 올린다**(2026-09-20) — 첫 잡힘에서 소프트 참조를 동기 로드하느라
프레임이 끊기지 않게(콜드 로드, testing/pie-scenarios.md C3-다마리). 2026-09-21 부터는 올린 음원을 `WarmAssets` 로 붙들어 GC 에
풀리지 않게 하고, 같은 자리에서 셰이크·파동·rules·몬스터·캐릭터 행의 소프트 참조도 함께 올린다 — 규칙은 [performance.md](performance.md)
"첫 사용 동기 로드 금지".

**재생.** 게임 코드는 `UGameplayStatics`를 직접 부르지 않고 `CL9Sound::PlayAt(WorldContext, Row, Location, PitchJitter = 0,
VolumeMultiplier = 1)` 하나로 튼다: 전용 서버면 return, 표·행·에셋이 없으면 **로그 없이 조용히** return(없는 행은 오류가 아니다 —
소리는 있어도 그만 없어도 그만인 표현이다), 있으면 `PlaySoundAtLocation(World, Sound, Location, VolumeMultiplier, 1 ± PitchJitter)`.
볼륨·감쇠·동시 발성 제한의 바탕은 전부 행이 가리키는 에셋 쪽이고, 코드가 얹는 것은 피치 흔들림(`Footstep` ±8 %, `Jump`·`LandSoft`
±6 %, `LandHard` ±4 % — `CL9Sound::*PitchJitter`, 같은 소리가 조금씩 다르게 들리기 위한 것이지 밸런스 값이 아니다)과 볼륨 배율
(발소리 걷기 0.7 → 달리기 1.0, `LandSoft` 0.5 → 1.0 — 2026-10-05)뿐이다. 기존 호출부는 기본값 1 그대로다.

**루프 — `CL9Sound::Resolve` (2026-10-09).** 사건마다 한 번 나는 소리가 아니라 액터가 자기 `UAudioComponent`로 계속 트는 소리는
`USoundBase* CL9Sound::Resolve(const UObject* WorldContext, FName Row)`로 음원만 받는다 — 재생(`SetSound`·`Play`)은 호출부 몫이다. 태도는
`PlayAt`과 같다: 입구에서 요청을 세고, 전용 서버면 null, 표·행·애셋이 없으면 로그 없이 null, 찾으면 해결을 세고 돌려준다. 전용 서버
판정만 하나 더 엄하다 — `IsRunningDedicatedServer()`에 **월드 넷 모드 `NM_DedicatedServer`** 를 더 본다. 에디터 PIE의 데디 서버 월드는
에디터 프로세스 안이라 프로세스 검사로는 안 걸리고, 루프는 한번 틀면 판 내내 남기 때문이다(`PlayAt`은 바꾸지 않았다). 지금 쓰는 곳은
`ACL9Kitchen::BeginPlay`의 `KitchenAmbience`·`CookSizzle`·`CookCrackle` 셋이다 — 가스불 소리는 받자마자 틀고, 요리 루프 둘은 음원만
`SizzleAudio`·`CrackleAudio`에 넣어 두었다가 `TickLook`이 표현 단계가 바뀔 때만 Play/Stop 한다(익기 시작하는 순간에는 로드도 조회도 없다).
이 소리들은 `PlayAt`의 "감쇠 없음"에 걸리지 않는다 — 애셋에 감쇠가 붙어 있고 컴포넌트가 그 애셋을 그대로 쓴다.
`CookDone`·`CookBurnt`는 사건이라 `PlayAt`이지만 역시 애셋에 감쇠(`SA_CL9_Kitchen`)가 있다 — `PlaySoundAtLocation`에 감쇠를 넘기지 않으면
애셋의 감쇠를 쓴다.

**재생 횟수.** `PlayAt`은 행마다 두 수를 센다(이 프로세스 전체 누적) — 입구에서 **요청**, 애셋을 찾아 실제로 재생하면 **해결**.
`CL9Sound::GetPlayCount(Row, bResolved)` · `UCL9FeelDebug::GetSoundCount`(파이썬 `get_sound_count`)로 읽는다. 자동 PIE 는
소리가 안 나므로 QA(`Tools/Unreal/qa_control_feel.py`)가 "재생 지점이 불렸나"(요청)와 "행·애셋이 있나"(해결)를 이 둘로 가른다 —
요청만 늘고 해결이 안 느는 행은 행·애셋이 빠진 것이다. `Resolve`도 같은 두 수를 올리므로 `KitchenAmbience`는 **조리대 `BeginPlay`
한 번에 요청 1**, 귀가 있는 월드면 **해결 1**이다 — 한 월드의 `Lvl_Flat`(조리대 3)이면 해결 +3, 데디 서버 월드는 요청만 +3·해결 0.
수는 프로세스 전체 누적이라 한 에디터 안의 PIE 2인(리슨 + 클라 월드 둘)은 해결 +6이고, 앞선 PIE 판의 수도 남는다 — QA는 판 시작 전후의 차이로 본다.
`CookSizzle`·`CookCrackle`도 같은 셈이다 — 조리대 `BeginPlay` 한 번에 행마다 요청 1·해결 1(데디 서버 월드는 요청만). **몇 번 틀었는지는 세지
않는다**(Play/Stop은 셈 밖) — 트는 중인지는 컴포넌트의 `IsPlaying()`으로 본다. `CookDone`·`CookBurnt`는 `PlayAt`이라 **전이마다 귀가 있는
월드마다** 요청 1·해결 1이고, 데디 서버 월드는 `TickLook`을 돌지 않아 요청도 0이다 — 한 에디터의 PIE 2인이면 전이 하나에 해결 +2.

**에지.** 재생 지점은 전부 위 "주요 사운드 이벤트" 표대로 복제된 시계·상태의 틱 비교다. 첫 틱은 기준값만 받아 적는다
(`bHitSoundsPrimed`·`bHitFeelPrimed`) — 늦게 들어온 클라가 이미 지난 휘두르기·타격을 접속 순간 되풀이하지 않는다.

**에셋 — 2026-10-17 부터 ElevenLabs 생성음이다**(26개 전부, 생성 표·절차·라이선스 주의는 [../../ArtSource/audio/README.md](../../ArtSource/audio/README.md)
머리글 — 표 아래 길이는 합성 때 값이고 생성음은 0.1~0.4초쯤 길다, 피크는 같다). 아래는 그 전 합성 시절의 기록이다. 실음원이 0개라 `Tools/Audio/synth_sfx.py`(순수 파이썬, 44.1 kHz·16-bit 모노·피크
−3 dBFS가 기본이고 항목마다 피크를 따로 둘 수 있다, 고정 시드라 두 번 돌리면 바이트가 같다)가 `ArtSource/audio/sfx/*.wav`
12개를 만들고, `Tools/Unreal/import_sfx.py`(에디터 파이썬, 멱등)가 `/Game/Sounds/SFX/…`로 임포트해 클래스 `SC_Effects`·
Concurrency `SCC_Effects4`(`USoundConcurrency`, `MaxCount` 4 — 같은 음이 다섯 번째부터는 겹쳐 나지 않는다)를 건다.
크기 차이는 합성 피크로 둔다(예고 −5 < 잡기 −3, 근접 −10 dBFS — 1.2초마다 반복되는 `MonsterNear`가 깔리게) — 임포트
스크립트도 코드도 볼륨을 만지지 않는다.
발소리 변형 3개는 SoundCue `S_Footstep`의 Random 노드 하나로 묶여 있어 **행은 하나**다 — `Footstep2/3` 같은 다중 행은
코드가 고르지 않는다. 실음원으로 바꾸는 절차(같은 이름의 WAV를 덮어쓰고 `import_sfx.py` 재실행 — 행은 그대로)는
[../../ArtSource/audio/README.md](../../ArtSource/audio/README.md).

| 행 | 에셋 | 원본 WAV | 길이 |
|---|---|---|---|
| `Pickup` | `/Game/Sounds/SFX/Carry/S_Pickup` | `pickup.wav` | 0.12초 |
| `Place` | `/Game/Sounds/SFX/Carry/S_Place` | `place.wav` | 0.15초 |
| `Swing` | `/Game/Sounds/SFX/Carry/S_SwingWhoosh` | `swing_whoosh.wav` | 0.25초 |
| `Hit` | `/Game/Sounds/SFX/Monster/S_HitPan` | `hit_pan.wav` | 0.35초 |
| `Hurt` | `/Game/Sounds/SFX/Character/S_Hurt` | `hurt.wav` | 0.20초 |
| `MonsterDeath` | `/Game/Sounds/SFX/Monster/S_MonsterDeath` | `monster_death.wav` | 0.50초 |
| `Footstep` | `/Game/Sounds/SFX/Character/S_Footstep` (SoundCue, Random → `S_Footstep_1/2/3`) | `footstep_1/2/3.wav` | 0.09~0.11초 |
| `MonsterPounce` | `/Game/Sounds/SFX/Monster/S_MonsterPounce` | `monster_pounce.wav` (톱니파 꽥 + 날개짓 4번, 피크 −5 dBFS) | 0.45초 |
| `MonsterCatch` | `/Game/Sounds/SFX/Monster/S_MonsterCatch` | `monster_catch.wav` (급강하 꽥 + 쿵 + 날개짓, 피크 −3 dBFS — 예고보다 낮고 크게) | 0.50초 |
| `MonsterNear` | `/Game/Sounds/SFX/Monster/S_MonsterNear` | `monster_near.wav` (블립 두 개 꼬꼬댁, 피크 −10 dBFS) | 0.16초 |
| `HitSurface` | `/Game/Sounds/SFX/Carry/S_HitSurface` | `hit_surface.wav` (프라이팬이 벽·바닥에 닿는 "퍽/툭", 피크 −6 dBFS — 명중 `hit_pan` −3 보다 작게) | 0.20초 |
| `Jump` | `/Game/Sounds/SFX/Character/S_Jump` | `jump.wav` | 0.10초 |
| `LandSoft` | `/Game/Sounds/SFX/Character/S_LandSoft` | `land_soft.wav` | 0.12초 |
| `LandHard` | `/Game/Sounds/SFX/Character/S_LandHard` | `land_hard.wav` | 0.22초 |
| `SlotSwitch` | `/Game/Sounds/SFX/Carry/S_SlotSwitch` | `slot_switch.wav` | 0.06초 |
| `LightToggle` | `/Game/Sounds/SFX/Carry/S_LightToggle` | `light_toggle.wav` | 0.04초 |
| `Eat` | `/Game/Sounds/SFX/Character/S_Eat` | `eat.wav` | 0.30초 |
| `Exhausted` | `/Game/Sounds/SFX/Character/S_Exhausted` | `exhausted.wav` | 0.60초 |
| `CartGrab` | `/Game/Sounds/SFX/Carry/S_CartGrab` | `cart_grab.wav` | 0.15초 |
| `KitchenAmbience` | `/Game/Sounds/SFX/Kitchen/S_KitchenAmbience` (Looping, `SC_Effects`, 감쇠 `SA_CL9_Kitchen`, 동시 발성 `SCC_KitchenAmbience`) | `kitchen_ambience.wav` (가스 쉬익 + 팬 웅웅, 이음매 없는 루프, 초안 피크 −18 dBFS) | 4~8초 루프 |

`KitchenAmbience`(2026-10-09)는 **감쇠·동시 발성 애셋을 쓰는 첫 효과음**이다. 위 경로는 art 의 `Tools/Unreal/synth_kitchen_sfx.py`(합성)·
`Tools/Unreal/import_kitchen_identity.py`(`assets` 모드 — 임포트, 감쇠·동시 발성 애셋 생성·지정, `SoundTable` 행 추가)가 만드는 이름이고
리드가 에디터에서 돌려 넣는다 — 실제 값은 그 스크립트의 `verify` 출력과 [../asset/map/kitchen.md](../asset/map/kitchen.md)가 정본이다.
초안 값(명세): 감쇠 `/Game/Sounds/Master/SA_CL9_Kitchen` — 200 cm 안은 최대 크기, 1200 cm에서 0. 동시 발성
`/Game/Sounds/Master/SCC_KitchenAmbience` — 최대 1, 가장 먼 것부터 멈춤(`Lvl_Flat`처럼 셋이 붙어 있어도 하나만큼만 크다).
코드는 이 값들을 모른다 — `ACL9Kitchen`의 `AmbienceAudio`는 애셋을 그대로 튼다.

요리 진행 4행(2026-10-09, [../goal/cooking-progress-feedback.md](../goal/cooking-progress-feedback.md))도 art 가 합성·임포트하고 `SoundTable`에
넣는다 — 실제 경로·길이는 그 임포트 스크립트의 `verify` 출력이 정본이고, 아래는 계약(`.claude/runs/cooking-progress-feedback/contract.md` 3)의 성격이다.
분류는 넷 다 효과음 `SC_Effects`(가스불·환풍기 배경음과 달리 요리 사건 소리다), 감쇠는 넷 다 `SA_CL9_Kitchen` 재사용.

| 행 | 성격 | 동시 발성 |
|---|---|---|
| `CookSizzle` | 루프 3~5초, 지글지글(기름 튀는 잡음 + 잔 팝) | 새 `SCC_CookSizzle` — 최대 2, 가장 먼 것부터 멈춤, 가상화 Restart |
| `CookCrackle` | 루프, 거칠고 탁탁 튐(타기 직전 층) | 새 `SCC_CookCrackle` — 최대 2, 같은 규칙 |
| `CookDone` | 한 번, 밝은 "치익"(0.6초 안팎) | 계약에 없다 — art 임포트 스크립트가 정한다 |
| `CookBurnt` | 한 번, 둔한 "퍽" + 짧은 쉿 | 같음 |

루프의 동시 발성은 **최대 2**다(계약 — 명세 B7의 "조리대 무리마다 하나만큼"보다 계약이 우선, 리드 2026-10-09) — 셋이 붙은 `Lvl_Flat`에서 셋 다
익히면 가장 먼 하나가 멈추고, 가상화 Restart라 가까워지면 다시 난다. 코드는 이 값들을 모른다.

뒤 세 행(2026-09-20)과 `HitSurface`(2026-09-22)는 WAV·임포트 스크립트까지 art가 냈고 **에셋 임포트와 `SoundTable` 행 추가는
리드가 에디터에서 한다**(`import_sfx.py` 실행 → `SoundTable.Rows`에 행 추가). 행이 들어오기 전까지는 재생 지점이 조용히
통과한다 — 소리만 없다.

2026-10-05 의 소유자 전용 8행(위 표의 `Jump`~`CartGrab`)은 art 의 `Tools/Unreal/synth_feel_sfx.py`(합성, `ArtSource/audio/sfx/`)·
`Tools/Unreal/import_feel_sfx.py`(임포트 + `SoundTable` 행 추가, 기존 11행 불변 확인)를 리드가 돌려 넣는다. 크기 차이는 합성 피크로 둔다
(control-feel.md Parameters "Placeholder synth targets" — 발소리 −3 dBFS 기준으로 점프 −8, 가벼운 착지 −5, 무거운 착지 −1.5, 칸 전환 −14,
조명 −10, 먹기 −9, 탈진 −12, 수레 −8).

## 배경음악 — `ACL9MusicPlayer` (2026-10-17)

머신마다 로컬로 하나(로컬 플레이어 컨트롤러가 `BeginPlay` 에서 세운다, 복제 없음, `Source/CL9/FX/CL9MusicPlayer.*`). 0.5초마다 틀 곡을 고른다 —
시작 레벨(`ACL9MenuGameMode`)이면 `MenuBGM`, 판이 끝났으면(전멸·남겨짐 — `ACL9GameState::IsRunOver`, 2026-10-21) `RunOverBGM`, 그 밖에는 복제된 판 단계로 `Night` 면 `NightBGM`, `Lobby`·`Dawn` 이면 `LobbyBGM`.
판 끝 곡이 따로 생긴 것은 죽은 뒤 새벽으로 넘어가면 따뜻한 로비 곡이 나와 너무 밝다는 지적 때문이다(2026-10-21).
행이 바뀌면 앞 곡은 `FadeOut`, 새 곡은 `CreateSound2D` + `FadeIn` — 시간은 `SoundTable.MusicFadeSeconds`(기본 2초, 빌드 없이 에디터에서 고친다).
행이 없으면 그 구간은 조용하다. 루프·볼륨은 애셋 쪽이다(`/Game/Sounds/Music/S_MenuBGM`·`S_LobbyBGM`·`S_NightBGM`·`S_RunOverBGM`, 루프, 클래스 `SC_Music`).

| 행 | 원본 WAV | 성격 |
|---|---|---|
| `MenuBGM` | `ArtSource/audio/music/menu_bgm.wav` (2분 51초) | 타이틀 — 낡은 오르간·퍼즈 기타, 느린 왈츠, 우스꽝스럽지만 불길하게 |
| `LobbyBGM` | `lobby_dark_bgm.wav` (2분 30초, 2026-10-21) | 트럭 대기·귀가 새벽 — 어두운 단조 패드와 먼 들바람 위로 드문 단조 기타·먼 종소리·낮은 피아노·희미한 속삭임. 판 곡보다 조용하다(RMS 0.6배). 옛 밝은 어쿠스틱 곡 `lobby_bgm`은 지웠다 |
| `NightBGM` | `night_horror_bgm.wav` (3분, 2026-10-21) | 판 진행 — 낮은 드론 위로 비명 같은 현악·긁는 쇳소리·위층 발소리·속삭임·조율 어긋난 피아노·저음 금관·큰 쿵이 7~16초 간격으로 흩어진다. 비명과 쿵 앞 3초는 바탕을 줄여 정적을 만든다 |
| `RunOverBGM` | `runover_bgm.wav` (1분 30초, 2026-10-21) | 판 끝(전멸·남겨짐) — 낮은 첼로 드론과 빈 바람 위로 고장 난 오르골 |

음원은 **바르코 사운드 무료 크레딧으로 만든 시험용**이다(약관상 비상업 — 출시 전에 상업 이용이 되는 곡으로 바꾼다). MP3 로 받아
`Tools/Audio/mp3_to_wav.py`(Blender 오디오 라이브러리 — 이 머신엔 ffmpeg 가 없다)로 WAV 로 바꾸고 `Tools/Unreal/import_music.py`(멱등, 행 추가까지)로 넣는다.
곡 끝과 처음이 이어지게 만든 것이 아니라 한 바퀴 돌 때 이음매가 들릴 수 있다. **2026-10-21 공포 곡.** 옛 `night_bgm.wav`(낮은 드론만)는 하나도 무섭지 않다는 지적을 받아 바꿨다. ElevenLabs 음악 API(`/v1/music`)는 무료 요금제에서 막혀 있어(402 `paid_plan_required`)
효과음 API 로 바탕 루프와 사건 소리 16개(대기 곡 4개 포함)를 받고(`C:\Work\CL9\Tools\Audio\gen_ai_music.py` → `Saved/AudioGen/music_stems/`),
`C:\Work\CL9\Tools\Audio\mix_music.py`(Blender 의 numpy)가 정해진 씨앗으로 섞는다. 사건은 곡 끝에서 처음으로 감아 넣어 이음매 없이 돌고,
음량은 옛 `night_bgm.wav` 의 RMS 에 맞춘다(옛 파일은 그 기준으로만 남는다). 사건 간격·세기·씨앗은 `mix_music.py` 머리의 `SONGS` 에서 고친다.

검증(2026-10-17 PIE): `Lvl_Flat` 에서 `LobbyBGM` → 판 시작에 `NightBGM` 으로 바뀌고,
`open Lvl_Start` 에서 `MenuBGM` 이 난다.

## 소리 모음집 — `Saved/AudioGen/listen.html` (2026-10-22)

서버 없이 파일로 여는 듣기 페이지다(`file:///C:/Work/CL9/Saved/AudioGen/listen.html`). 배경음악·상황별 소리·몬스터별 소리(열마다 언제, 비면 공용 행)·
스테이지별 소리(바탕 두 칸·나오는 몬스터)·효과음 파일 전체(생성 문장, 아무 행도 부르지 않는 파일은 빨갛게) 다섯 칸이고, 원본 WAV 를 바로 재생한다.
표를 고친 뒤 다시 만든다 — 에디터에서 `py -3 Tools/Unreal/run_python.py Tools/Audio/dump_sound_links.py`(표 덤프 → `Saved/AudioGen/sound_links.json`),
그다음 `py -3 Tools/Audio/make_listen_page.py`. 상황 행 이름표는 소리 지도 페이지(`soundmap.html` 의 `CODE`)와 같은 목록이라 한쪽에 행을 더하면 다른 쪽도 더한다.
2026-10-22 생성 기준으로 효과음 행 27개(닭 무리·미끼·출현·탈 몬스터·정전 예고·차단기·트럭 경적·출발·전멸·문 쾅·위층 발소리 등)는 사운드 표에 있지만 코드·표 어디서도 부르지 않는다.

## 소리 지도 — 데이터 패널 `/soundmap` (2026-10-21)

`py -3 Tools/DataPanel/data_panel.py serve` 뒤 `http://127.0.0.1:8766/soundmap`. 사이트별·몬스터별·상황별·전체 소리 행 네 탭으로
어떤 `SoundTable` 행이 어디서 나는지 보여 주고, 칸마다 ▶ 미리듣기(원본 WAV, `/art/`)·바꾸기·✕ 끊기를 한다. 쓰는 표는 소리·몬스터·사이트·파동 넷이고,
고친 값은 에디터에 바로 들어가며 "모두 저장" 을 눌러야 디스크에 남는다(잠금 규칙은 다른 표 페이지와 같다).
- 소리 표 행을 비우면 그 행을 쓰는 모든 곳이 조용해진다. 몬스터·사이트·파동 열을 비우면 그 자리만 끊긴다.
- 몬스터 열 가운데 덮치기·잡기·처형·죽음·발소리는 비면 공용 행으로 대신 난다(`CL9Sound::RowOr`). 공용으로도 안 나게 하려면 표에 없는 행 이름 `Mute` 를 넣는다 — 페이지의 ✕ 가 그렇게 한다.
- 페이지 `Tools/DataPanel/pages/soundmap.html`, 애셋 → 원본 목록은 `panel_bridge.sound_sources()`(`GET /api/sound_sources`). 상황별 설명은 페이지의 `CODE` 표가 갖는다 — `CL9SoundNames` 에 행을 더하면 거기에도 한 줄 더한다.

## 2026-10-19 효과음 — 종별 소리·판 흐름·목적지 바탕

음원 65개는 ElevenLabs 생성음(표 `C:\Work\CL9\ArtSource\audio\ai_sfx.csv`, 목록은 `C:\Work\CL9\docs\plan\draft\sequence-sound.md` 2절 가운데 지금 게임에 붙는 것)이고,
`C:\Work\CL9\Tools\Unreal\import_horror_sfx_20261019.py` 가 임포트·`SoundTable` 행·표 열을 한 번에 쓴다(멱등).

- **감쇠** — 기획서 반경을 계단 애셋으로 묶었다: `/Game/Sounds/Master/SA_CL9_Tell600`·`Tell900`·`Tell1500`·`Tell2100`·`SA_CL9_Truck3000`
  (NaturalSound, 안쪽 반경 = 반경 × 0.15). 반경은 그 애셋에서 고친다. 공용 4행(`MonsterPounce`·`MonsterCatch`·`MonsterNear`·`MonsterDeath`)은 예전처럼 감쇠가 없다.
- **종별 소리** — `UCL9MonsterDef` 의 Sound 묶음 12열(`MonsterTable`). 전이 한 번 소리: `WakeSoundRow`(WakeTelegraph)·`InvestigateSoundRow`·
  `ChaseSoundRow`(새 추격만 — Pounce·Catch·Chase 에서 돌아온 것은 뺀다)·`PounceSoundRow`·`CatchSoundRow`·`ExecuteSoundRow`·`DeathSoundRow`·`FootstepSoundRow`.
  상태 루프(`ACL9Monster::LoopAudio`): `SleepLoopSoundRow`(Asleep)·`CalmLoopSoundRow`(Wander·Patrol·Investigate·Return)·`StalkLoopSoundRow`(Stalk)·`ChaseLoopSoundRow`(Chase·Pounce).
  빈 열은 공용 행(`MonsterPounce`·`MonsterCatch`·`MonsterDeath`·`Footstep`)이거나 소리 없음. 닭 1·돼지 5·거위 6·너구리 6·양 5열이 채워져 있다.
- **판 흐름** — `ACL9SiteSoundPlayer`(`Source/CL9/FX/CL9SiteSoundPlayer.*`, 머신마다 로컬 하나, 음악 옆): 전원 단계 Dark → LightsOn 에 `PowerSwitch`(스위치 자리)·`PowerOnHum`,
  LightsOn → Flicker 에 `LampFlicker`, Flicker → Dead 에 `PowerDie`, 정전 순간에 `BlackoutHit`(모두 2D). 트럭 도착 `TruckArrive`(`ACL9Truck::Tick`, 몸체 자리),
  데스 스팅어 `DeathSting`(`ACL9Character::ApplyDeathCamera`, 죽은 사람 머신만).
- **목적지 바탕** — `UCL9SiteDef.AmbienceSoundRow`·`BlackoutAmbienceSoundRow`(`MapTable`). 판 동안 2D 루프, 정전에 바뀐다(`SoundTable.AmbienceFadeSeconds` 1.5초).
  `henhouse`·`henhouse_review`·`farm` 에 양계장 바탕(`HenhouseAmbPower`/`HenhouseAmbBlackout`).
- **행만 있고 트는 곳이 없는 것** — 그 사건이 아직 코드에 없다: `TruckHorn`·`TruckDepart`·`WipeSting`·`BlackoutWarn`·`BreakerPull`·`BreakerRestore`·`DoorSlam`·
  `OverheadSteps`, 광폭화·빈 것·탈 터짐(`ChickenOmenGroan`·`ChickenBurst`·`MaskTear`·`BurstSting`·`RageOmenStitch`·`HollowDeflate`·`Pig*Burst/Omen`·`Hollow*`),
  먹기(`ChickenEatBait`·`PigEat`), 판 중간 예고 `ChickenEmergeTell`, 무리 웅성 `ChickenFlockMurmur`, 포대 쓴 거위 `GooseHooded`, 양에게 둘러싸인 1인칭 `SheepPinFP`·밀기 `SheepShoved`,
  부화실 열등 `HenhouseIncubatorLamp`.
- **검증(2026-10-19 PIE, `Lvl_Henhouse`, 봇 동작 `sound_counts`)** — 도착·전원·깜빡임·꺼짐·정전·바탕 둘·데스 스팅어·공포 6행, 거위 잠든 숨·죽음, 양 방울·울음·발굽·짓밟기·죽음,
  너구리 스토킹·발소리·죽음, 돼지 발소리·처형이 모두 요청 = 해결로 났다. 그 판에 일어나지 않은 전이(거위 깨움·경보, 돼지 냄새·덮침·죽음, 너구리 격분·잡기·처형, 닭 깨움)는 0회다.

## 재생 API (표현 계층)

```cpp
// CL9 게임 코드 — 표의 행을 이름으로 (위 구현 절). 효과음은 전부 이 길로 간다
CL9Sound::PlayAt(this, CL9SoundNames::Hit, MeshComponent->Bounds.Origin);
CL9Sound::PlayAt(this, CL9SoundNames::Footstep, GetFootLocation(), CL9Sound::FootstepPitchJitter);

// 2D (UI·음악)
UGameplayStatics::PlaySound2D(World, Sound, Volume, Pitch, StartTime /*, Concurrency...*/);

// 3D (효과음·음성) — 뒤 파라미터는 Attenuation·Concurrency·OwningActor
UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, Volume, Pitch);

// 루프 (배경음) — 반복은 사운드 에셋 설정(Looping)에서
AudioComponent->SetSound(Sound);
AudioComponent->Play();
```

## 볼륨 관리

사운드 클래스 트리와 **설계 게인**(2026-09-12 — 슬라이더 기본값이 아니라 `SoundClass.Properties.Volume`에 둔다):
`SC_Master` 1.0 ├ `SC_Effects` 0.8 ├ `SC_VO` 1.0 ├ `SC_UI` 1.0 ├ `SC_Ambience` 0.5 ├ `SC_Music` 0.7 └ `SC_Voice` 1.0
(`Content/Sounds/Master/`, 믹스 `MasterSoundMix`). **모든 사운드 에셋은 이 중 하나의 클래스를 가져야** 설정 화면의
볼륨 슬라이더가 닿는다.

**`SC_Voice` (2026-10-11, [voice.md](voice.md))** — `SC_Master` 의 자식이고 **마이크 음성 스트림 전용**이다. `SC_VO` 는 캐릭터 반응 음성(`VoiceSet`) 그대로 두고 둘을 섞지 않는다.
*2026-10-12 정정(voice-eos-redpoint):* 옛 `DefaultEngine.ini [/Script/Engine.AudioSettings] VoiPSoundClass=/Game/Sounds/Master/SC_Voice.SC_Voice`(엔진 VOIP 재생에 이 클래스를 입히던 줄)는 엔진 VOIP 와 함께 없어진다 —
Redpoint 의 재생이 `SC_Voice` 를 타는지는 **리드 확인 대기**([voice.md](voice.md) "볼륨 경로").
슬라이더 `VoiceChatVolume` 이 `SettingsTable.VoiceClass` 로 이 클래스의 믹스 오버라이드를 건다 — 내가 죽어 관전 중이면 `× CL9Rules.VoiceSpectatorVolume`(별도 믹스 에셋 없음). 같은 값이 EOS 출력 볼륨에도 들어간다.
에셋 생성은 `Tools/Unreal/create_voice_assets.py`([../asset/README.md](../asset/README.md) "음성 채팅 에셋").

**음성 감쇠도 에셋이다**(2026-10-12, [../goal/voice-eos-redpoint.md](../goal/voice-eos-redpoint.md) 결정 5 — 2026-10-11 의 "음성 감쇠는 에셋이 아니다"를 폐기한다). Redpoint 가 숫자가 아니라 감쇠 에셋을 받으므로 `SA_CL9_Voice`·`SA_CL9_VoiceSpeaker`
둘을 `create_voice_assets.py` 가 만들어 `SettingsTable.VoiceAttenuation`·`VoiceSpeakerAttenuation` 에 넣는다. `CL9Rules` 의 감쇠 셀 다섯과 런타임 감쇠 객체는 없어졌다.
값은 [../plan/data/rules.md](../plan/data/rules.md) "음성 감쇠 에셋 값" 표(초안 — `Inner` 150·`SilentAt` 1500·`Linear`), 고정 속성과 쓰임은 [voice.md](voice.md) "감쇠 — 에셋 둘".
`SA_CL9_Kitchen` 과 `SC_VO`·`VOVolume` 은 건드리지 않았다.

```cpp
// 설정 화면의 슬라이더 값(기본 전부 1.0)이 클래스 볼륨 위에 곱해진다 — UCL9GameUserSettings::ApplyAudio(World)
UGameplayStatics::SetSoundMixClassOverride(World, Mix, MusicClass,
  NewVolume, /*Pitch*/1.f, /*FadeTime*/0.1f, /*bApplyToChildren=*/true);
UGameplayStatics::PushSoundMixModifier(World, Mix);   // 믹스 활성화는 별도 호출 — 로컬 PC BeginPlay에서 한 번
```

- 믹스·클래스 참조는 `SettingsTable`(DataAsset, `UCL9SettingsTable`)이 갖고 `BP_CL9GameInstance`가 주입한다 —
  conventions § 5의 경로 예외는 `MasterSoundMix` 하나뿐이라 SoundClass 6개(+ 음성 스트림용 `VoiceClass`)는 에셋 참조여야 한다.
- 월드·오디오 디바이스가 없으면 `SetSoundMixClassOverride`가 조용히 return하므로 `GameInstance::Init`에서는 못 부른다.

슬라이더 UI·저장은 [settings.md](settings.md).

## 참고

- 모든 사운드 에셋은 로열티 프리 또는 자작
- 같은 음이 너무 자주 나면 피로 → Concurrency로 제한
- 사운드 표·행 이름의 정의는 `Source/CL9/Data/CL9SoundTable.h`(`UCL9SoundTable`·`CL9SoundNames`) — 이 문서의
  "구현 — `SoundTable`" 절이 정본이고, [../data/schema.md](../data/schema.md)는 색인일 뿐이다
