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

## 주요 사운드 이벤트 (현 기획 기준)

**운반 (inventory.md · raiding.md):**
- `SFX_Pickup` — 줍기 (걷기 줍기는 조용, 달리기 줍기는 시끄러움 — 소음 반경은 rules.md 판정 값,
  사운드는 그 표현)
- `SFX_Place` / `SFX_Throw` — 놓기 / 던지기
- `SFX_Crack` / `SFX_Break` — 금 감 / 깨짐
- `SFX_JarDeposit` — 항아리 반납
- `SFX_Swap` — 손↔품 교체 (1.2초 동작음)

**요리 (cooking.md):**
- `SFX_Sizzle` — 열원 위에서 익는 소리 (루프)
- `SFX_CookDone` — 익음 전이 (egg → egg+cooked)
- `SFX_Smoke` / `SFX_Burnt` — 연기 예고 / 타버림 전이

**몬스터 (monster.md):**
- `SFX_WakeTelegraph` — 기상 예고 퍼덕임·울음 (0.8초 — 전 플레이어에게 들린다)
- `SFX_Alert` — 경보 울음 (거위 `AlertWindup` 뒤)
- `SFX_Catch` — 잡힘 (플레이어 넘어짐과 함께)
- `SFX_MonsterDeath` — 사살 (사살 경보의 소리 그 자체)
- `SFX_Footstep_Monster` — 몬스터 발소리 (읽히는 위협의 청각 채널)

**타격·피격 (goal/hit-feedback.md — 계획 이름만, 음원 에셋 없음):**
- `SFX_Hit` — 팬이 몬스터에 닿는 순간 (몬스터가 빨개지고 카메라가 킥하는 바로 그 순간 —
  testing/pie-scenarios.md C4 ⑩)
- `SFX_Hurt` — 잡혀 넘어지는 순간, 맞은 사람 쪽 (`SFX_Catch`가 몬스터가 덮치는 소리라면
  이쪽은 맞은 몸의 소리다 — C3 피격감)
- 위 둘은 **이름만 잡아둔 것**이다. 2026-09-15 hit-feedback 실행은 시각 표현(빨간 플래시·
  비네트·카메라 셰이크)까지만 넣었고 음원은 만들지 않았다 — 실행 범위 밖.

**캐릭터:**
- `VoiceSet` 반응 음성 — 잡힘·넘어짐·탈진 (동료 상황을 귀로 알리는 채널 — character.md)
- `FootstepSet` 발소리 (연출 전용 — 몬스터 청각 판정과 무관)

**거래·전이:**
- `SFX_Sell` / `SFX_Buy` — 창구 판매 / 트럭 패널 구매
- `SFX_OrderComplete` — 주문 납품 완료
- `SFX_Blackout` — 정전 (light.md — 소등 연출과 함께)

**UI:**
- `UI_Click` · `UI_Open` · `UI_Close` · `UI_Confirm` · `UI_Error`

## 에셋 위치

```
Content/Sounds/
  ├─ SFX/          (Carry/ Cooking/ Monster/ Trade/)
  ├─ VO/           (캐릭터별 VoiceSet)
  ├─ UI/
  ├─ Ambience/     (Henhouse/ — 목적지별)
  ├─ Music/        (GameplayBGM, MenuBGM)
  └─ Master/       (MasterSoundMix — 클래스별 게인)
```

## 재생 API (표현 계층)

```cpp
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
- 사운드 데이터 정의(FSoundEffect)는 `../data/schema.md` 참고
