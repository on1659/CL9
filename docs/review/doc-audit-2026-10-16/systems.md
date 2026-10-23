# docs/systems · conventions.md 대 C++ 소스 대조 감사 (2026-10-16, 읽기 전용)

대상: `C:\Work\CL9\docs\systems\*.md` 31개 + `C:\Work\CL9\docs\conventions.md` 대 `C:\Work\CL9\Source\CL9` (헤더 88개, cpp 86개).
방법: 문서의 `[AUFE]CL9*` 클래스, `CL9*::함수`, 백틱 CamelCase 식별자, `cl9.*` 콘솔 명령, `Server_/Multicast_/Client_` RPC, `Source/CL9/...` 경로를 뽑아 소스 전체 토큰 집합과 대조했다. 소스 쪽은 모든 UCLASS/USTRUCT/UENUM 선언 190개를 뽑아 문서 등장 여부를 봤다.

한계: 데이터 표(Content/Data/*.uasset)의 열 이름은 C++ UPROPERTY 로만 확인했고, uasset 안의 행 값과 애님 BP 변수는 볼 수 없었다. 한 단어짜리 열 이름(`Alias`, `Weight` 등)은 대조 대상에서 빠졌다.

---

## 1. 문서에 있으나 소스에 없는 식별자 (실제 불일치로 본 것)

"지웠다/없다/미구현"이라고 문서가 스스로 밝힌 것은 이 표에서 뺐다(3절에서 다룸). 아래는 문서가 **존재한다고 쓰는데** 소스에 없거나 이름·시그니처가 다른 것이다.

| 문서 | 문서에 있으나 소스에 없는(또는 다른) 식별자 | 비고 |
|---|---|---|
| `C:\Work\CL9\docs\systems\input.md:92` | `ACL9Character::Server_Interact(대상)` | 소스 어디에도 없다(Grep 0건). 실제 상호작용 RPC 는 `Server_PickPlace()` (`C:\Work\CL9\Source\CL9\Character\CL9Character.h:586`). 같은 파일 86~90행의 `ICL9Interactable` 시그니처(`GetPrompt/CanInteract/Interact`)는 `C:\Work\CL9\Source\CL9\Core\CL9Interactable.h:11,14,17` 과 일치한다. |
| `C:\Work\CL9\docs\systems\character.md:245-246` | `ICL9Damageable::OnDamage(int32 Amount, AActor* Source)` | 소스는 3인자다: `OnDamage(int32 Amount, AActor* Source, float Knockback)` (`C:\Work\CL9\Source\CL9\Core\CL9Damageable.h:30`). `C:\Work\CL9\docs\systems\monster.md:175` 는 이미 3인자로 고쳐 쓰는데 character.md 만 옛 시그니처다. |
| `C:\Work\CL9\docs\systems\map.md:166` | "지운 것" 목록의 `CellToWorld` | 목록에는 삭제됐다고 적혀 있으나 `ACL9SiteMap::CellToWorld(FIntPoint, float)` 가 살아 있다: 선언 `C:\Work\CL9\Source\CL9\World\CL9SiteMap.h:106`, 정의 `C:\Work\CL9\Source\CL9\World\CL9SiteMap.cpp:573`, 사용처 `CL9SiteMap.cpp:637`(전원 스위치 스폰), `C:\Work\CL9\Source\CL9\GameMode\CL9PlayerController.cpp:902,1055,1083`(인트로 카메라). 같은 줄의 나머지(`FCL9HenhouseParams`·`FCL9SiteFrame`·`FCL9FloorLayout`·`CarveFloor`·`CeilingBase`·`BaseZMode`)는 소스에 실제로 없어 문서와 맞다. |
| `C:\Work\CL9\docs\systems\map.md:263-265` ("아직 안 한 것") | "문 옆 벽 토막이 −75%로 눌린다 … 토막 전용 조각이 필요하다" | 소스에는 토막 전용 seg kind 가 이미 있다: `BarnStub()`/`WallStub()` (`C:\Work\CL9\Source\CL9\World\CL9SiteLayout.h:337-338`, 사용 주석 `CL9SiteLayout.cpp:1900`). `C:\Work\CL9\docs\systems\asset-swap.md:22,24,170-171` 도 `barnStub → wallQuarter` 로 해결된 것으로 적는다. 즉 map.md 의 이 항목은 낡았다. 데이터 값(실제 눌림 배율)은 uasset 이라 확인하지 못했다. |
| `C:\Work\CL9\docs\systems\light.md:196,200` | `LookAntiAliasing`, `LookGrain`, `LookPosterizeBands`, `LookPosterizeMix` | 소스 이름은 접두사 `Look` 이 없다: `AntiAliasing`(`C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:261`), `PosterizeBands`(:265), `PosterizeMix`(:269), `Grain`(:273). 룩 표 이관(2026-10-16) 뒤 이름이 갱신되지 않았다. |
| `C:\Work\CL9\docs\systems\light.md:150` | 사이트 행 `BlackoutLighting` | 소스에 없다. 정전 조명은 이제 `FCL9LightingPhase Blackout` (`C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:118`)다. |
| `C:\Work\CL9\docs\systems\light.md:119` | `YardLampIntensity` | 소스에 없다. `CL9RulesDef.h` 에는 `YardLampMesh`(:463)·`YardLampHeight`(:467)만 있다. 세기는 램프 종류(`FCL9LampKind`, `CL9LookDef.h:130`)로 옮겨졌을 가능성이 있으나 확인하지 못했다. |
| `C:\Work\CL9\docs\systems\light.md:22,188-189` | 소품 컬럼 `LightIntensity`·`LightRadius`·`LightColor`·`LightConeAngle`·`SourceRadius` (`FCL9PropPlacement`) | 소스에 없다. light.md:22 가 "2026-10-16부터 빛 값은 룩 표 램프 종류(`LampKind`)로 옮겼고 아래는 옮기기 전 기록"이라고 스스로 밝혀 두었으므로 의도된 이력이다. 다만 188~191행 본문은 현재형이라 독자가 혼동할 수 있다. |
| `C:\Work\CL9\docs\systems\performance.md:123` | `ACL9SiteMap::SetLampsOn` | 소스에는 `SetLampGroup(ELampGroup, bool)` (`C:\Work\CL9\Source\CL9\World\CL9SiteMap.h:225`)뿐이다. 이름이 바뀌었다. |
| `C:\Work\CL9\docs\systems\actor-base.md:156` | `ACL9GameMode::InitializeNight(int32 Day)` | 소스에 없다. 같은 절 제목이 "(권장)"인 예시 코드이므로 실제 함수로 오해되지 않게 표시할 필요만 있다. |
| `C:\Work\CL9\docs\systems\cheats.md:89` | "코드 등록 36개와 1:1" | `C:\Work\CL9\Source\CL9\GameMode\CL9Cheats.cpp` 의 `static FAutoConsoleCommand` 는 **37개**다(Grep -c 37). 문서 표의 구현 행도 37개(+ `CL9Wallet.cpp:103,126` 의 `cl9.wallet`·`cl9.skin` 2개, 미구현 2행 별도)라 숫자만 하나 틀렸다. 문서 명령 목록과 소스 등록 명령의 이름은 서로 빠짐이 없다. |

### 확인했으나 불일치가 아닌 것 (오탐 정리)
- `cheats.md:132,133` 의 `cl9.shownav`·`cl9.showjars` 는 "미구현"이라 적혔고 소스에도 없다. 일치.
- `cheats.md:136` 의 `cl9.resetkeys`·`cl9.playseq`·`cl9.testwake`·`cl9.testvoice`·`cl9.fxoff`·`cl9.voiceloop`·`cl9.breaktest`·`cl9.showdetect` 는 "삭제된 계획 명령"으로 적혔고 소스에 없다. 일치.
- 문서가 "지웠다"고 밝힌 `IsLocalTalkerRegistered`·`SyncLocalTalker`(session.md:52), `GetVoiceLevel`·`GetSilenceThreshold`·`AddMicLevelRow`·`UpdateMicLevel`(settings.md:108-109), `MakeVoiceAttenuation`(voice.md:188), `SwapTime`(inventory.md:4), `DropLoot`/`SpawnDropItem`(drop.md:35), `CL9VoiceTalker`(voice.md:12)는 소스에서 모두 0건. 일치.
- `Multicast_PlaySequence`·`FindSequence`·`GetSequenceDuration`(sequence.md:23,59,60)은 시퀀스 시스템 설계 의사코드이며 해당 문서가 "소스에 없음"을 명시한다(3절).
- `animation.md` 의 `IsPounce`·`AlphaAttack`·`StateElapsed` 등 25개와 `light.md` 일부(`LightIntensity` 등)는 애님 BP 변수·재질 파라미터 이름이라 C++ 소스에서 검증할 수 없다. 단, 그 원천 게터(`GetStateElapsed`, `IsPounceHit`, `GetPounceResultElapsed`)는 `C:\Work\CL9\Source\CL9\Monsters\CL9Monster.h:138,149,160` 에 있다. 소스에 `UAnimInstance` 파생 C++ 클래스는 없다(Grep: `CL9CharacterDef.h`·`CL9MonsterDef.h` 의 참조뿐).
- 엔진 API 이름(`ReplicateMoveToServer`, `GetAllActorsOfClass`, `HandleStartingNewPlayer`, `NativeOnKeyDown` 등)과 asset/모듈 이름(`CL9_Strings`, `CL9_Wave` 등)은 소스 식별자가 아니라 대조하지 않았다.
- 모든 `Source/CL9/...(.h|.cpp)` 경로와 파일명 언급(`CL9ReplicationCheck.cpp` 등)은 실존한다. 깨진 경로 0건.
- `Content/Data` 에 실제 자산이 있는 표: `MonsterTable`, `ItemTable`, `OrderTable`, `EquipmentTable`, `MapTable`, `ModuleTable`, `PieceTable`, `SegKindTable`, `SettingsTable`, `SoundTable`, `WaveTable`, `CharacterTable`. `UITable` 은 `C:\Work\CL9\Source\CL9\UI\CL9UITable.h:43` 의 `UCL9UITable` 로 있다. `SequenceTable` 만 자산·클래스 모두 없다(문서가 "미구현"이라 적음).

---

## 2. 소스에 있으나 어느 systems 문서에도 안 나오는 클래스

기준: `docs/systems/*.md` 전체에 클래스 이름(접두사 포함)이 한 번도 등장하지 않는 것. 중요도 순.

대분류 UCLASS/AActor:
- `UCL9SkinDef` — `C:\Work\CL9\Source\CL9\Data\CL9SkinDef.h:17`. (스킨 자체는 `cheats.md`·`save.md`·`ui.md` 에서 "스킨 상점·옷장·지갑"으로 다뤄지나 행 클래스 이름은 없다. 다른 plan 문서에는 이름이 있다.)
- `UCL9BuffDef` — `C:\Work\CL9\Source\CL9\Data\CL9BuffDef.h:21`. `character.md:583-586` 이 "`EatBuffs`가 가리키는 버프 행"을 설명하지만 클래스·`BuffTable` 이름은 안 나온다.
- `UCL9DefTable` — `C:\Work\CL9\Source\CL9\Data\CL9DefRegistry.h:32`. (`UCL9DefList` 는 문서에 있다.)
- `UCL9Gimmick` — `C:\Work\CL9\Source\CL9\Data\CL9DefBase.h:17`, `UCL9Gimmick_Spin` — `C:\Work\CL9\Source\CL9\Data\CL9Gimmicks.h:15`. systems 문서는 `Gimmicks` 열(order.md:27, wave.md:126)만 언급하고 클래스 이름은 없다.
- `UCL9LandscapeTools` — `C:\Work\CL9\Source\CL9\World\CL9LandscapeTools.h:17`. systems 문서 0건(다른 docs 에는 있음).
- `ACL9Jar` — `C:\Work\CL9\Source\CL9\Items\CL9Jar.h:18`. conventions.md:114,133 에만 있고 systems 에는 항아리를 일반어로만 쓴다(map.md:144).
- `ACL9LampItem` — `C:\Work\CL9\Source\CL9\Items\CL9LampItem.h:17`. conventions.md:114 에만 있다.

conventions.md 의 디렉터리 설명(104~122행)이 빠뜨린 실존 클래스(conventions.md 에 이름 0건, 일부는 systems 에는 있음):
- Items: `ACL9DepartLever`(`C:\Work\CL9\Source\CL9\Items\CL9DepartLever.h:27`), `ACL9DestinationBoard`(`CL9DestinationBoard.h:18`), `ACL9PowerSwitch`(`CL9PowerSwitch.h:26`), `ACL9SkinCloset`(`CL9SkinCloset.h:16`)
- World: `ACL9Truck`(`C:\Work\CL9\Source\CL9\World\CL9Truck.h:27`), `ACL9YardTension`(`CL9YardTension.h:21`), `UCL9LandscapeTools`
- GameMode: `UCL9WalletSubsystem`·`UCL9WalletSave`(`C:\Work\CL9\Source\CL9\GameMode\CL9Wallet.h:15,44`)
- Character: `UCL9FeelDebug`(`C:\Work\CL9\Source\CL9\Character\CL9FeelDebug.h:87`)
- Data 의 `CL9Constants.h` 는 문서(conventions.md:120)에 있고 실존한다(`C:\Work\CL9\Source\CL9\Data\CL9Constants.h`).

내부 보조 타입(문서화 필요성 낮음, 목록만): `FCL9OwnerSoundSnapshot`(`Character\CL9Character.h:40`), `FNetworkPredictionData_Client_CL9`(`Character\CL9CharacterMovement.h:147`), `FCL9FeelSample`(`Character\CL9FeelDebug.h:17`, 단 `character.md:75` 가 `GetFeelSample` 로 간접 언급), `FCL9PendingMonsterSpawn`·`FCL9NightMonsterQueue`·`FCL9VoiceMute`(`GameMode\CL9GameMode.h:25,32,75`; voice.md 는 `RebuildVoiceMutes` 로 설명), `ECL9AfterLobbyLeft`(`GameMode\CL9GameInstance.h:18`; voice.md:56 가 동작은 설명), 위젯 릴레이 `UCL9DebugPanelRelay`·`UCL9OrderCardRelay`·`UCL9SettingsRelay`·`UCL9SkinRelay`(settings.md:146 이 "릴레이 UObject"를 일반어로 언급), `FTalkingEntry`·`FScanEntry`·`FKeyRow`, `FCL9UIInfo`·`ECL9UILayer`(`UI\CL9UITable.h:16,26`), `FCL9LightingProfile`·`FCL9LampKind`·`FCL9SurfaceFamily`(`Data\CL9LookDef.h:91,130,172`), `FCL9DistrictSlotRules`(`Data\CL9SiteDef.h:320`), `FCL9WaveReveal`, 렌더러 내부 `FDot`·`FReveal`, `FCL9ModulePlacement`(`World\CL9SiteLayout.h:142`), `FFixtureSpawn`·`ELampGroup`·`FFlickerLamp`, `FEntrance`.

(`UCL9LookDef`·`UCL9RulesDef`·`UCL9SiteDef`·`UCL9ProgressionDef` 등 나머지 주요 클래스는 systems 에 이름이 나온다.)

---

## 3. "미구현/설계만/아직 없음" 표기 대 실제 소스

### 문서도 "없음", 소스도 "없음" (일치)
- `CL9TuningHash` — 문서 `conventions.md:188-189`("소스에 심볼 없음"), `session.md:425,438`, `wave.md:56-57,617`. Source·Config·Tools 전체 Grep 0건. 일치. 아직 구현되지 않았다.
- 시퀀스 시스템 — `sequence.md:6-7,14,82`. `UCL9SequenceSubsystem`·`UCL9SequenceDef`·`SequenceTable`·`ULevelSequencePlayer` 전부 0건, `CL9.Build.cs` 에 LevelSequence 모듈 없음. 일치. `input.md:25,41` 의 `bLockLocalInput`·`bSkippable`·`IA_Skip` 도 소스 0건, `C:\Work\CL9\Content\Input\Actions` 에 IA_Skip 이 있는지는 폴더 목록만 봤고 파일 단위로는 확인하지 못했다.
- 미끼(`Decoy`) 발생 이벤트 — `wave.md:20,254`. 소스에는 열거형 `Decoy`(`CL9Types.h:125`, `CL9WaveTable.h:44`)와 `DecoyRadius`(`CL9RulesDef.h:212`)뿐이고 미끼를 놓는 호스트 코드는 없다. 일치.
- `UCL9MonsterDef::SightDark` 소비처 없음 — `light.md:76`. 소스에서 `C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h:108` 선언 한 곳뿐. 일치.
- 불(버너) — `cooking.md:105,162`. `Burner` Grep 0건. 일치.
- 거래 로그 — `economy.md:100`. 0건. 일치.
- `FinalDay`·`QuotaOverride`·`OrdersOfferedCurve` — `balance.md:164`. 소스 0건. 일치.
- 구매 연출(Multicast), `bUsableAsTool` 소비처 — `shop.md:98`. `bUsableAsTool` 은 `C:\Work\CL9\Source\CL9\Data\CL9EquipmentDef.h:68` 선언만 있고 다른 읽는 곳 없음(Grep 1건), 구매 연출 관련 심볼 0건. 일치.
- `cl9.shownav`·`cl9.showjars` — 위 1절. 일치.
- `penStub`(asset-swap.md:200) — `penStub` 소스 0건. 일치.
- 낮은 조각의 `CL9_Occlusion` Ignore(`wave.md:21,667`) — 소스 `C:\Work\CL9\Source\CL9\FX\CL9WaveRenderer.cpp:34` 가 전용 채널을 쓰나 "맵 코드 몫이라 아직 없다"는 설명은 이 채널과 모순되지 않는다. 판정 불가로 둔다.

### "미구현"인데 실제론 구현된 것
- 없음(확정). 가장 가까운 경우는 `map.md:263-265` 의 "토막 눌림 / 전용 조각 필요"로, "아직 안 한 것" 절에 남아 있으나 소스에 `BarnStub`/`WallStub` seg kind 가 이미 있다(1절 표 참조). 눌림 배율 실측은 uasset 이라 확인 못 했다.
- `map.md:266` "`door` seg kind 행 — 지금은 문 자리가 문틀 없는 개구부" : 소스는 `door` 행이 없어도 되게 처리하고(`C:\Work\CL9\Source\CL9\Data\CL9DefRegistry.cpp:458,462`) 없으면 로그만 낸다. 실제 `SegKindTable` 에 행이 있는지는 uasset 이라 확인하지 못했다. 판정 못 함.

### 구현됐다고 쓰였는데 소스에 없는 것 (반대 방향)
- `input.md:92` `Server_Interact` (1절).
- 1절 표의 이름 변경 건들(`SetLampsOn`, `Look*` 접두사, `BlackoutLighting`, `YardLampIntensity`).
- 그 밖에 "구현됨"이라 쓰였는데 소스 식별자가 통째로 없는 클래스는 못 찾음.

---

## 4. 요약 수치
- 문서에 등장하는 `[AUFE]CL9*` 클래스 식별자 중 소스에 없는 것: 의도된 삭제 기록(`FCL9HenhouseParams` 등 3개, `UCL9SequenceSubsystem`·`UCL9SequenceDef`)을 빼면 0개.
- 이름·시그니처가 실제로 어긋난 것: 11건(1절 표).
- 소스에 있으나 systems 에 이름이 없는 "주요" 클래스: 8개(`UCL9SkinDef`, `UCL9BuffDef`, `UCL9DefTable`, `UCL9Gimmick`, `UCL9Gimmick_Spin`, `UCL9LandscapeTools`, `ACL9Jar`, `ACL9LampItem`; 뒤 둘은 conventions.md 에만 있다).
- conventions.md 디렉터리 설명이 빠뜨린 클래스: 9개(Items 4, World 3 중 LandscapeTools 포함, Wallet, FeelDebug).
