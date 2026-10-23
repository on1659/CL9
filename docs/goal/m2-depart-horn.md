# goal: m2-depart-horn

백로그 #16 · 큰 그림 `C:\Work\CL9\docs\plan\roadmap.md` 5절 단계 1-A **M2**. 트랙: LIGHT(새 복제·새 시스템 없음, 소리 하나를 기존 길로 트는 일).

## One-line Goal

판 시계가 끝나기 `CL9Rules.DepartWarnTime`(기본 20초) 전에, 모든 머신에서 트럭 몸체 자리의 3D 경적(`SoundTable` 행 `TruckHorn`)이 판마다 한 번 울린다.

## Background / Motivation

- 판 끝(시계 0)에 트럭 짐칸 상자 밖에 있는 산 사람은 짐을 잃고 죽는다(`C:\Work\CL9\docs\plan\data\rules.md` "판 종료와 출발"). 지금은 끝이 다가온다는 신호가 HUD 판 시계뿐이다.
- 시나리오 1판이 가르치는 다섯째 줄이 "판 끝엔 트럭 안이어야 한다"이고, 그 순간을 키우는 장치로 M2 경적이 잡혀 있다(`C:\Work\CL9\docs\plan\scenario.md` 1.7절·115행, `C:\Work\CL9\docs\plan\roadmap.md` 182행).
- 기획 초안이 값을 이미 적어 두었다. `C:\Work\CL9\docs\plan\draft\round-loop.md` 71·663행(`DepartWarnTime` 20초, 새 열, 새 복제 없이 각 머신이 파생), `C:\Work\CL9\docs\plan\draft\sequence-sound.md` R13(76행)·632행(트럭 자리 3D 경적 두 번, 반경 6000 추정)이 그것이다. 경적은 HUD 문구 없이 월드 연출로만 둔다(round-loop.md 930행).

### 정찰로 확인한 사실 (2026-10-20, 공유 체크아웃 기준)

- **음원과 행은 이미 있다.** `C:\Work\CL9\ArtSource\audio\sfx\truck_horn.wav`가 있고, ElevenLabs 표 `C:\Work\CL9\ArtSource\audio\ai_sfx.csv` 83행에 `truck_horn.wav,2.00,-3,0,2.0,"An old truck honking its horn twice, …"`가 있다. 임포트 스크립트 `C:\Work\CL9\Tools\Unreal\import_horror_sfx_20261019.py` 98행이 `/Game/Sounds/SFX/Flow/S_TruckHorn` 애셋과 `SoundTable` 행 `TruckHorn`을 만들었다(감쇠 `SA_CL9_Truck3000`). `C:\Work\CL9\Content\Sounds\SFX\Flow\S_TruckHorn.uasset`이 있고 `C:\Work\CL9\Content\Data\SoundTable.uasset`에 `TruckHorn` 문자열이 있다. `C:\Work\CL9\docs\systems\sound.md` 314행도 `TruckHorn`을 "행만 있고 트는 곳이 없는 것"으로 적고 있다.
  → 따라서 **ElevenLabs 표에 행을 더하지 않고, 새 음원도 만들지 않는다.** 로드맵이 말한 "`SoundTable` 행"은 이미 있는 `TruckHorn` 행을 쓰는 것으로 충족된다.
- 코드 이름 `CL9SoundNames::TruckHorn` 상수는 **없다**(`C:\Work\CL9\Source\CL9\Data\CL9SoundTable.h` 189행 근처에는 `TruckArrive`까지만 있다).
- `DepartWarnTime`은 소스 어디에도 없다. 새 열이다.
- 판 시계는 이미 복제된다. `ACL9GameState::NightEndServerTime`이 복제되고(`C:\Work\CL9\Source\CL9\GameMode\CL9GameState.cpp` 28행), `ACL9GameState::GetNightRemaining()`은 판(Night)이 아니면 0을, 판이면 `NightEndServerTime − GetServerWorldTimeSeconds()`를 돌려준다(같은 파일 696~702행). 클라이언트에서도 같은 값이 나온다.
- 트럭 도착 소리가 따를 본보기다. `ACL9Truck::Tick`(`C:\Work\CL9\Source\CL9\World\CL9Truck.cpp` 64~90행)은 모든 머신에서 복제된 서버 시각으로 에지를 잡아 `CL9Sound::PlayAt(this, CL9SoundNames::TruckArrive, GetBodyLocation())`을 한 번 틀고, `bWasArriving`·`bArrivalSoundPrimed`(`C:\Work\CL9\Source\CL9\World\CL9Truck.h` 71·74행)로 첫 틱을 다룬다. `ACL9Truck`은 `bReplicates = true`이고 게임 모드가 첫 트럭 패드 기준으로 스폰한다.

## In-scope

1. **새 열 `CL9Rules.DepartWarnTime`**
   - `UCL9RulesDef`(`C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h`)의 `Category="Depart"` 묶음, `DepartArmTime` 옆에 `float DepartWarnTime = 20.f;`를 둔다. 단위는 초이고, 판 시계가 끝나기 전 남은 시간이다.
   - **0이면 경적이 없다.** 음수·NaN·무한은 `UCL9RulesDef::Validate`(`C:\Work\CL9\Source\CL9\Data\CL9RulesDef.cpp`, `DepartArmTime` 검사 옆)에서 로드 오류로 낸다.
   - 값은 `C:\Work\CL9\Content\Data\CL9Rules.uasset`에서 빌드 없이 고친다. 헤더 기본값 20이 곧 의도값이라 애셋 셀을 따로 채울 필요는 없다. 다만 애셋에 저장된 값이 헤더와 다르지 않은지 구현 때 확인한다.
2. **코드 이름 `CL9SoundNames::TruckHorn`**
   - `C:\Work\CL9\Source\CL9\Data\CL9SoundTable.h`의 "판 흐름" 묶음, `TruckArrive` 옆에 `inline const FName TruckHorn(TEXT("TruckHorn"));`와 한 줄 주석("판 끝 `DepartWarnTime` 전 — 트럭 몸체 자리에서")을 더한다.
3. **경적 재생 — `ACL9Truck::Tick`**
   - 도착 소리와 같은 자리, 같은 방식으로 둔다. 머신마다 로컬로 판정하고 새 복제는 없다.
   - 판정: 이번 틱에 `GameState->GetPhase() == ECL9NightPhase::Night`이고 `Rules->DepartWarnTime > 0`이며 `Remaining = GameState->GetNightRemaining()`이 `0 < Remaining ≤ DepartWarnTime`일 때, **바로 앞 틱에 판(Night)이면서 `Remaining > DepartWarnTime`이었으면** `CL9Sound::PlayAt(this, CL9SoundNames::TruckHorn, GetBodyLocation())`을 한 번 부른다.
   - 상태: `ACL9Truck`에 `bool bWasBeforeDepartWarn = false;`(앞 틱에 "판이고 경적 시각 전"이었나) 하나를 둔다. 판이 아니면 매 틱 false가 된다. 그래서 판마다 한 번만 울리고, 판 도중에 들어온 사람이나 첫 틱에 이미 경적 시각을 지난 머신은 지난 경적을 듣지 않는다(도착 소리의 priming 태도와 같다).
   - 전용 서버에서는 `CL9Sound::PlayAt`이 이미 조용히 return하므로 따로 막지 않는다.
4. **경적 감쇠 반경을 6000으로**
   - 지금 `S_TruckHorn`은 `SA_CL9_Truck3000`(30m)을 쓰는데, 양계장 판의 경계가 x −1700~11000으로 100m가 넘는다(`C:\Work\CL9\docs\decisions.md` 1612행). 30m로는 건물 안쪽에서 들리지 않는다.
   - `C:\Work\CL9\Tools\Unreal\import_horror_sfx_20261019.py`의 `ATTENUATIONS`에 `"SA_CL9_Truck6000": 6000.0`을 더하고, 98행 `truck_horn.wav` 항목의 감쇠를 `"SA_CL9_Truck6000"`으로 바꾼다. 그 뒤 스크립트를 다시 돌린다(멱등).
   - `TruckArrive`·`TruckDepart`는 `SA_CL9_Truck3000`을 그대로 쓴다. 반경은 그 애셋에서 고친다(sound.md 303행의 관례).
5. **문서 갱신**
   - `C:\Work\CL9\docs\plan\data\rules.md` "판 종료와 출발" 표에 `DepartWarnTime` 행을 넣는다(기본값 20 초안, 뜻, **신규**).
   - `C:\Work\CL9\docs\systems\sound.md` 2026-10-19 절의 "판 흐름" 줄에 `TruckHorn`(`ACL9Truck::Tick`, 몸체 자리, `SA_CL9_Truck6000`)을 넣고, "행만 있고 트는 곳이 없는 것" 목록에서 `TruckHorn`을 뺀다. 감쇠 줄에는 `SA_CL9_Truck6000`을 더한다.
   - `C:\Work\CL9\docs\plan\roadmap.md` M2와 `C:\Work\CL9\docs\plan\scenario.md` 115행의 상태 표시는 구현이 끝난 뒤 그 세션이 고친다.

## Out-of-scope

- **레버 첫 누름의 경적**(round-loop.md 663행, sequence-sound.md 991행). 레버 준비 상태의 복제 여부에 달린 별건이고, 이번 명세는 판 끝 20초 전 경적만 다룬다.
- **경적 순간의 음악 덕킹**(`SoundTable.HornDuckMusic`, sequence-sound.md 770·908행). 음악 플레이어(`ACL9MusicPlayer`)는 다른 시스템이라 미리 연동하지 않는다. M6 음악 톤이나 사운드 믹스 작업의 몫이다.
- **HUD 문구·꼬리표·화면 표시.** 경적은 월드 연출로만 둔다(round-loop.md 930행). 판 시계 HUD는 지금 그대로다.
- **몬스터 소음 사건.** 경적을 `Multicast_Noise`나 몬스터 청각에 물리지 않는다. 판정에 닿지 않는 표현이다.
- 새 음원 생성, `ai_sfx.csv` 행 추가, `truck_horn.wav` 교체(이미 있다).
- 트럭이 없는 레벨(트럭 패드가 없는 평지 시험 레벨)의 경적. 그런 레벨에는 `ACL9Truck`이 없으니 경적도 없다.
- M3 정전 예고(`BlackoutWarnTime`) — 별도 항목이다.

## Acceptance Criteria

- [ ] `UCL9RulesDef`에 `DepartWarnTime`(float, 기본 20, `Category="Depart"`)이 있다. `Validate`는 음수·NaN·무한을 로드 오류로 내고, 0은 받아 준다.
- [ ] `CL9SoundNames::TruckHorn`이 있고, 그 이름을 부르는 곳은 `ACL9Truck::Tick` 하나뿐이다.
- [ ] 새 복제 속성·RPC가 없다(`DOREPLIFETIME`·`UFUNCTION(Server|Client|NetMulticast)` 추가 0).
- [ ] PIE `Lvl_Henhouse`, 리슨 서버 + 클라이언트 1에서 판을 끝까지 둔다. 그러면 시계가 남은 `DepartWarnTime`초를 지나는 순간 양쪽 머신에서 `CL9Sound::GetPlayCount(TEXT("TruckHorn"), true)`가 판마다 정확히 1씩 는다. 판 시작부터 그 순간까지와 그 뒤 판 끝까지는 늘지 않는다.
  - 시간을 줄여 검증하려면 `C:\Work\CL9\Content\Data\CL9Rules.uasset`의 `DepartWarnTime`을 잠시 판 길이 가까이(예: 240) 두고, 끝나면 20으로 되돌린다. 코드는 바꾸지 않는다.
- [ ] `DepartWarnTime = 0`이면 판을 끝까지 둬도 `TruckHorn` 요청 수가 0이다.
- [ ] 경적 시각 전에 레버로 귀가하거나, 전멸했거나, `cl9.night`로 판을 끝내면 그 판에는 경적이 없다(요청 수 0).
- [ ] 경적 시각 뒤에 판에 들어온(합류한) 클라이언트는 그 판 경적을 듣지 않는다(그 머신 요청 수 0).
- [ ] `S_TruckHorn`의 감쇠가 `SA_CL9_Truck6000`(반경 6000)이고, `S_TruckArrive`·`S_TruckDepart`는 `SA_CL9_Truck3000` 그대로다.
- [ ] 판이 둘 이어질 때(로비 → 다음 판) 둘째 판에서도 경적이 한 번 난다.
- [ ] rules.md·sound.md가 위 In-scope 5대로 갱신됐다.
- [ ] 눈과 귀로 볼 확인은 `/cl9film`으로 경적 순간이 들어간 짧은 영상을 찍어 올린다(`C:\Work\CL9\.claude\skills\cl9film\SKILL.md`). 봇 동작 `sound_counts`(`C:\Work\CL9\Tools\Film\bot.py`)로 요청 = 해결인지도 함께 남긴다.

## Related Files / Modules

| File | Role |
|------|------|
| `C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h` / `.cpp` | 새 열 `DepartWarnTime`과 `Validate` |
| `C:\Work\CL9\Source\CL9\Data\CL9SoundTable.h` | `CL9SoundNames::TruckHorn` 상수, `CL9Sound::PlayAt`·`GetPlayCount` |
| `C:\Work\CL9\Source\CL9\World\CL9Truck.h` / `.cpp` | `Tick`의 경적 에지, `GetBodyLocation`, 새 상태 `bWasBeforeDepartWarn` |
| `C:\Work\CL9\Source\CL9\GameMode\CL9GameState.h` / `.cpp` | `GetNightRemaining`·`GetPhase`(읽기만, 변경 없음) |
| `C:\Work\CL9\Tools\Unreal\import_horror_sfx_20261019.py` | `SA_CL9_Truck6000` 감쇠 추가와 `S_TruckHorn` 감쇠 교체 |
| `C:\Work\CL9\Content\Data\CL9Rules.uasset` | 열 값(헤더 기본 20) |
| `C:\Work\CL9\Content\Data\SoundTable.uasset` · `C:\Work\CL9\Content\Sounds\SFX\Flow\S_TruckHorn.uasset` | 이미 있는 행과 애셋 |
| `C:\Work\CL9\ArtSource\audio\ai_sfx.csv` (83행) · `C:\Work\CL9\ArtSource\audio\sfx\truck_horn.wav` | 이미 있는 음원과 그 생성 행(변경 없음) |
| `C:\Work\CL9\docs\plan\data\rules.md` · `C:\Work\CL9\docs\systems\sound.md` | 문서 갱신 |

## Must-Preserve

- `ACL9Truck::Tick`의 도착 연출(감속 곡선, `TruckArrive` 1회, `bWasArriving`·`bArrivalSoundPrimed`)은 동작이 그대로다.
- 판 끝 판정(남겨짐, 짐 사라짐, 전멸 패널티, 지갑 지급)과 `EndNight`의 순서·시각은 바꾸지 않는다. 경적은 판정을 읽기만 하고 쓰지 않는다.
- `CL9Sound::PlayAt`의 태도(행·애셋이 없으면 조용히 아무것도 안 함, 전용 서버 return, 요청·해결 집계)를 그대로 쓴다. 새 재생 경로를 만들지 않는다.
- 애셋 경로는 C++에 넣지 않는다(`C:\Work\CL9\docs\conventions.md` § 5). 소리는 행 이름으로만 부른다.
- 주석은 한글로 쓴다(Copyright 헤더만 예외).
- `SA_CL9_Truck3000`을 쓰는 `TruckArrive`·`TruckDepart`의 들리는 거리는 그대로다.

## Fairness Constraints

- 경적 시각은 모든 머신이 같은 복제 값(`NightEndServerTime`과 서버 시각)에서 파생한다. 호스트와 클라이언트가 같은 순간에 듣는다(서버 시각 동기 오차 안). 호스트만 듣거나 먼저 듣는 일이 없다.
- 경적은 정보만 주고 판정을 바꾸지 않는다. 판 길이, 남겨짐 판정, 몬스터 행동, 소음 체계에 닿지 않는다.
- 몰래 역할(임포스터)을 포함해 모든 플레이어가 같은 경적을 듣는다. 역할별로 가르지 않는다.
- 3D 소리라 트럭에서 먼 사람일수록 작게 들린다. 이 차이는 의도된 거리 정보다. 반경 6000은 판 경계 대부분을 덮도록 고른 값이고, 조절은 감쇠 애셋에서 한다.
- 경적을 다시 울리게 하는 입력이나 치트를 새로 만들지 않는다.

## Existing Integration Contract

- 판 시계: `ACL9GameState::StartNightClock`이 `NightEndServerTime`을 찍고(복제), `ClearNightClock`이 0으로 돌리며, `GetNightRemaining`은 판이 아니면 0을 돌려준다. 경적은 이 세 가지만 읽는다.
- 판 단계: `ACL9GameState::GetPhase()` / `ECL9NightPhase::Night`. 로비·새벽에서는 판정하지 않는다.
- 트럭: `ACL9Truck`은 게임 모드가 `StartPlay`에서 첫 트럭 패드 기준으로 하나 스폰하는 복제 액터다. `GetBodyLocation()`은 몸체 메시 자리다(도착 연출 동안 움직이지만, 경적 시각에는 도착이 끝나 있다).
- 소리 표: `SoundTable` 행 `TruckHorn` → `S_TruckHorn`(클래스 `SC_Effects`, 루프 아님). 행이 빠져도 게임은 조용히 계속된다.
- 규칙 표: `UCL9DefRegistry::GetRules()` → `UCL9RulesDef`. 로드 오류는 기존 `Validate` 길로 보고된다.
- 출발 레버(`ACL9DepartLever`, `DepartArmTime`)와 귀가 흐름(`RequestDepart`·`EndNight`)은 건드리지 않는다.

## Execution Notes

- 권장 모델: 이번 항목은 판단할 거리가 적고 기계적이다. 패턴이 정해진 변경(열 1, 상수 1, Tick 에지 1, 스크립트 2줄, 문서 2곳)이라 Sonnet으로 충분하다. 2인 PIE 검증 해석과 영상 촬영 판단은 그때의 최상위 Claude 모델을 권장한다.
- 이 문서는 모델을 강제하지 못한다. 실행 세션의 `/model` 설정이 결정한다. 세션 모델이 권장보다 낮으면 사용자에게 알리고 확인한 뒤 진행한다.
- 빌드는 브로커에 요청한다(`C:\Work\CL9\Tools\Build\build_request.py --kind build`). 임포트 스크립트를 다시 돌리는 일과 `CL9Rules` 값 확인은 에디터 잠금을 확인한 뒤에 한다(`C:\Work\CL9\docs\workflow\sessions.md`).
- 역할 배분: C++·표는 gameplay, 임포트 스크립트·감쇠 애셋은 art, 문서는 docs다.

## 밸런스 몫 (숫자 — 기본값으로 정했고, 빌드 없이 고친다)

| 무엇 | 기본값 | 고치는 곳 | 근거 |
|---|---|---|---|
| `CL9Rules.DepartWarnTime` | 20초 (0 = 경적 없음) | `C:\Work\CL9\Content\Data\CL9Rules.uasset` | roadmap M2, round-loop.md 71행 |
| 경적 감쇠 반경 | 6000cm (안쪽 반경 = 반경 × 0.15 = 900cm, 기존 계단 규칙) | `/Game/Sounds/Master/SA_CL9_Truck6000` | sequence-sound.md 632행 "6000(추정)" |
| 경적 볼륨·길이 | 음원 그대로(피크 −3 dBFS, 2.0초, 두 번 울림) | `S_TruckHorn` 볼륨 / `ai_sfx.csv` 83행 | 이미 생성된 음원 |
