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

## 호출 규칙

- **절대** 판정 코드(GameMode, 서버)에서 직접 재생하지 않음 — 항상 표현 계층에서
- 동기화는 **호스트발 Multicast RPC(Unreliable — 사운드는 유실 허용)** 또는 Replicated
  상태의 OnRep에서
- 클라발 Server RPC로 사운드를 요청하지 않는다 (비소유 액터 함정 — actor-base.md)
- **구현된 효과음(2026-09-18)은 Multicast도 OnRep도 아니다** — 이미 복제되는 시계·상태(`SwingStartServerTime`·
  `KnockdownStartServerTime`·몬스터 `StateStartServerTime`·아이템 `HoldState`)를 매 틱 지난 틱과 비교해 **상승 에지**에서
  모든 머신이 각자 튼다. 새 RPC가 없고, 리슨 호스트도 자기 폰에서 같은 길을 탄다(OnRep은 호스트 자신에게 뜨지 않는다 —
  character.md 피격 절의 셰이크와 같은 이유). 아래 "구현 — `SoundTable`" 절. 예외 하나(2026-09-20): `MonsterNear`는 파동
  멀티캐스트(`Multicast_Noise`, Unreliable)에 얹혀 온다 — 파동 표 행의 `SoundRow` 열이 "이 소음이 나면 낼 소리"를 가리킨다.
  파동이 이미 그 채널을 쓰고 있어 새 RPC는 여기에도 없다([wave.md](wave.md)).

## 주요 사운드 이벤트 (현 기획 기준)

이름이 **표 행**(`Pickup`처럼 접두사 없는 이름)이면 `SoundTable`에 행·에셋이 있고 코드에 재생 지점이 붙은 것이고,
`SFX_…`·`UI_…`·`VoiceSet`은 아직 **계획 이름**이다(에셋도 재생 지점도 없다). 행이 된 옛 이름은 괄호에 남겼다.

**운반 (inventory.md · raiding.md):**
- `Pickup` 행 (옛 `SFX_Pickup`) — 줍기 (걷기 줍기는 조용, 달리기 줍기는 시끄러움 — 소음 반경은 rules.md 판정 값,
  사운드는 그 표현). **행과 에셋은 있지만 재생 지점은 아직 없다** — 다음 실행(game-feel §4)에서 `PickupServerTime`이
  생기면 그 변화에서 튼다.
- `Place` 행 (옛 `SFX_Place`·`SFX_Throw` — 하나로 합침) — 놓기·던지기·수레 짐칸에 얹기(G) 모두. 아이템 `HoldState`가
  Held·Worn에서 Free로 바뀌는 순간 아이템 자리에서. 수레를 놓는 것(Pushed→Free)과 죽음 드랍(Free→Free)에는 안 난다.
- `SFX_Crack` / `SFX_Break` — 금 감 / 깨짐
- `SFX_JarDeposit` — 항아리 반납

**요리 (cooking.md):**
- `SFX_Sizzle` — 열원 위에서 익는 소리 (루프)
- `SFX_CookDone` — 익음 전이 (egg → egg+cooked)
- `SFX_Smoke` / `SFX_Burnt` — 연기 예고 / 타버림 전이

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

| 행 | 에지 | 어느 머신 | 어디서 | 위치 |
|---|---|---|---|---|
| `Swing` | `SwingStartServerTime` 변화 | 모든 머신 | `ACL9Character::UpdateHitSounds` | 폰 |
| `Hurt` | `KnockdownStartServerTime` 변화 | 모든 머신 | `ACL9Character::UpdateHitSounds` | 폰 |
| `Hit` | Stun·Dead ∧ `GetStateElapsed() < HitFlashTime` 창의 상승 에지, 또는 창 안에서 `StateStartServerTime` 변화 | 모든 머신 | `ACL9Monster::TickHitFeel` | 몬스터 메시 중심(`Bounds.Origin`) |
| `MonsterDeath` | 같은 에지인데 상태가 Dead | 모든 머신 | `ACL9Monster::TickHitFeel` | 몬스터 메시 중심, `Hit`과 함께 |
| `MonsterPounce` | **상태 전이** `State == Pounce ∧ (지난 틱 State != Pounce ∨ StateStartServerTime 변화)` — 창(elapsed)이 아니라 전이라 늦게 온 복제도 놓치지 않는다. 첫 틱은 기준값만(`bStateSoundPrimed`) | 모든 머신 | `ACL9Monster::TickStateSounds` (`TickHitFeel` 다음) | 몬스터 메시 중심 |
| `MonsterCatch` | 같은 전이 감지, 상태가 Catch. `TickHitFeel`의 Stun·Dead 창과 별개라 Catch가 관측 전에 Stun·Dead로 덮이면(같은 틱 피격) 그 잡기 소리는 빠진다 — `Hurt`가 이미 나므로 허용 | 모든 머신 | `ACL9Monster::TickStateSounds` | 몬스터 메시 중심, 잡힌 폰의 `Hurt`와 같은 순간 |
| `MonsterNear` | 근접 소나 사건 `monster_prox`가 이 머신에 도착할 때(호스트는 `HostReportNoise` 직후, 클라는 `Multicast_Noise`) — 파동 행의 `SoundRow` | 모든 머신 | `UCL9WaveRenderer::PlayWave` (`MinInterval` 뒤·시각 게이트 앞) | 소음 발원지(몬스터 자리) |
| `Place` | 아이템 `HoldState` Held·Worn → Free | 모든 머신 | `ACL9Item::ApplyHoldState` (호스트는 OnPlaced/OnThrown 경유, 클라는 OnRep) | 아이템 |
| `Footstep` | 수평 이동 거리 누적 ≥ 보폭 | 모든 머신 | `ACL9Character::TickFootstepSound` · `ACL9Monster::TickFootstep` | 발밑 |
| `Pickup` | (없음 — 다음 실행 `PickupServerTime`) | — | — | — |

**캐릭터:**
- `VoiceSet` 반응 음성 — 잡힘·넘어짐·탈진 (동료 상황을 귀로 알리는 채널 — character.md)
- `Footstep` 행 (옛 `FootstepSet`) — 발소리 (연출 전용 — 몬스터 청각 판정과 무관). **애님 노티파이가 아니라 거리 누적기**다:
  땅 위에서 움직인 수평 거리가 보폭(`CL9Rules.FootstepStrideRun` 196cm / `FootstepStrideWalk` 150cm)에 닿을 때마다
  발밑에서 한 번, 피치 ±8 %. 속도가 행의 `WalkSpeed`를 넘으면 달리기 보폭(달리기 입력은 남의 폰에 복제되지 않아 속도로
  가른다), 서 있거나 공중이거나 넘어졌으면 안 난다. 소음 판정(파동, `TickStepNoise`)과는 별개 누적이다 — character.md 피격 절.

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
  └─ Master/       (MasterSoundMix — 클래스별 게인)
```

효과음 에셋 → 행 연결은 `Content/Data/SoundTable`이 갖는다(아래 구현 절의 표). 원본 WAV는 `ArtSource/audio/sfx/`.

## 구현 — `SoundTable` (2026-09-18, game-feel §1)

**표.** `UCL9SoundTable`(`Source/CL9/Data/CL9SoundTable.h`, DataAsset — `UCL9ShakeTable`과 같은 틀)은
`TMap<FName, TSoftObjectPtr<USoundBase>> Rows` 하나다. 효과음을 추가한다는 것은 사운드 에셋을 만들고 여기에 행을 더하는
것이지 rules 에셋에 열이 늘어나는 일이 아니다. C++가 조회하는 행 이름은 `CL9SoundNames` 상수 10개 — `Pickup`·`Place`·`Swing`·
`Hit`·`Hurt`·`MonsterDeath`·`Footstep`·`MonsterPounce`·`MonsterCatch`·`MonsterNear`(뒤 셋은 2026-09-20) — 그 외 행은 데이터
전용이다. 다만 **파동 표의 `SoundRow` 열은 어느 행이든 이름으로 가리킬 수 있다**([wave.md](wave.md)) — `MonsterNear`는 C++
상수가 있지만 재생 지점은 그 열이다. 에셋은 `/Game/Data/SoundTable`이고
`BP_CL9GameInstance.SoundTableAsset`(`UCL9GameInstance`, `ShakeTableAsset`과 같은 자리)이 가리킨다 — C++에는 에셋 경로가
없다(conventions § 5). `UCL9GameInstance::Init`이 한 번 로드해 `GetSoundTable()`로 내주고, 비어 있으면 Warning 한 줄이다.
**`Init`은 `Rows`의 음원을 전부 `LoadSynchronous`로 미리 올린다**(2026-09-20) — 첫 잡힘에서 소프트 참조를 동기 로드하느라
프레임이 끊기지 않게(콜드 로드, testing/pie-scenarios.md C3-다마리). 2026-09-21 부터는 올린 음원을 `WarmAssets` 로 붙들어 GC 에
풀리지 않게 하고, 같은 자리에서 셰이크·파동·rules·몬스터·캐릭터 행의 소프트 참조도 함께 올린다 — 규칙은 [performance.md](performance.md)
"첫 사용 동기 로드 금지".

**재생.** 게임 코드는 `UGameplayStatics`를 직접 부르지 않고 `CL9Sound::PlayAt(WorldContext, Row, Location, PitchJitter = 0)`
하나로 튼다: 전용 서버면 return, 표·행·에셋이 없으면 **로그 없이 조용히** return(없는 행은 오류가 아니다 — 소리는 있어도
그만 없어도 그만인 표현이다), 있으면 `PlaySoundAtLocation(World, Sound, Location, 1.0, 1 ± PitchJitter)`. 볼륨·감쇠·
동시 발성 제한은 전부 행이 가리키는 에셋 쪽이고 코드는 볼륨 1.0으로 튼다. 코드가 흔드는 것은 `Footstep`의 피치뿐
(`CL9Sound::FootstepPitchJitter` 0.08 = ±8 % — 걸음마다 조금씩 다르게 들리기 위한 것이지 밸런스 값이 아니다).

**에지.** 재생 지점은 전부 위 "주요 사운드 이벤트" 표대로 복제된 시계·상태의 틱 비교다. 첫 틱은 기준값만 받아 적는다
(`bHitSoundsPrimed`·`bHitFeelPrimed`) — 늦게 들어온 클라가 이미 지난 휘두르기·타격을 접속 순간 되풀이하지 않는다.

**에셋 — 전부 합성 임시 음원이다.** 실음원이 0개라 `Tools/Audio/synth_sfx.py`(순수 파이썬, 44.1 kHz·16-bit 모노·피크
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

뒤 세 행(2026-09-20)은 WAV·임포트 스크립트까지 art가 냈고 **에셋 임포트와 `SoundTable` 행 추가는 리드가 에디터에서 한다**
(`import_sfx.py` 실행 → `SoundTable.Rows`에 `MonsterPounce`·`MonsterCatch`·`MonsterNear`). 행이 들어오기 전까지는 재생
지점이 조용히 통과한다 — 소리만 없다.

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
`SC_Master` 1.0 ├ `SC_Effects` 0.8 ├ `SC_VO` 1.0 ├ `SC_UI` 1.0 ├ `SC_Ambience` 0.5 └ `SC_Music` 0.7
(`Content/Sounds/Master/`, 믹스 `MasterSoundMix`). **모든 사운드 에셋은 이 중 하나의 클래스를 가져야** 설정 화면의
볼륨 슬라이더가 닿는다.

```cpp
// 설정 화면의 슬라이더 값(기본 전부 1.0)이 클래스 볼륨 위에 곱해진다 — UCL9GameUserSettings::ApplyAudio(World)
UGameplayStatics::SetSoundMixClassOverride(World, Mix, MusicClass,
  NewVolume, /*Pitch*/1.f, /*FadeTime*/0.1f, /*bApplyToChildren=*/true);
UGameplayStatics::PushSoundMixModifier(World, Mix);   // 믹스 활성화는 별도 호출 — 로컬 PC BeginPlay에서 한 번
```

- 믹스·클래스 참조는 `SettingsTable`(DataAsset, `UCL9SettingsTable`)이 갖고 `BP_CL9GameInstance`가 주입한다 —
  conventions § 5의 경로 예외는 `MasterSoundMix` 하나뿐이라 SoundClass 6개는 에셋 참조여야 한다.
- 월드·오디오 디바이스가 없으면 `SetSoundMixClassOverride`가 조용히 return하므로 `GameInstance::Init`에서는 못 부른다.

슬라이더 UI·저장은 [settings.md](settings.md).

## 참고

- 모든 사운드 에셋은 로열티 프리 또는 자작
- 같은 음이 너무 자주 나면 피로 → Concurrency로 제한
- 사운드 표·행 이름의 정의는 `Source/CL9/Data/CL9SoundTable.h`(`UCL9SoundTable`·`CL9SoundNames`) — 이 문서의
  "구현 — `SoundTable`" 절이 정본이고, [../data/schema.md](../data/schema.md)는 색인일 뿐이다
