# 데이터 표 문서 ↔ C++ 데이터 정의 ↔ Content/Data 대조 감사

감사 시점의 작업 트리 기준이다(헤더 여러 개가 아직 커밋되지 않은 수정 상태이므로 줄 번호는 작업 트리의 것이다). 읽기 전용으로 수행했고 저장소 파일은 건드리지 않았다.

## 방법과 한계

- 헤더는 `C:\Work\CL9\Source` 전체의 `.h`를 스크립트(`C:\Users\Radar\AppData\Local\Temp\claude\C--Work-CL9\199f7b2b-76ff-413d-9ace-220ec8f79b1f\scratchpad\audit.py`)로 훑어 `UCLASS`/`USTRUCT`마다 `UPROPERTY` 다음 선언의 멤버 이름을 뽑았다(클래스 141개, 멤버 1098개). 클래스가 멤버 타입으로 쓰는 `FCL9*` 구조체와 부모(`UCL9DefBase`)까지 한 묶음으로 보았다.
- 문서 쪽 열 이름은 헤더 첫 칸이 `컬럼`, `열`, `필드`인 마크다운 표의 첫 칸에 백틱으로 적힌 식별자만 뽑았다. 머리글이 `종류`, `가족`, `프로필`, `행` 같은 표와 한글 머리글 표(예: look.md 램프 종류·구조면 표)는 식별자가 없어서 열 이름으로 세지 못했다. 그런 열은 아래에서 (b)로 나온다.
- (a)는 컬럼표에 적혀 있으나 클래스 묶음(부모 `UCL9DefBase` 포함)에 같은 이름이 없는 것이다. (b)는 클래스 묶음의 멤버 중 해당 문서(들) 어디에도 그 단어가 없는 것이다. (b')는 컬럼표에는 없지만 본문에는 나오는 것이다(참고용).
- 이름 차이는 정규화(접두사 b 제거, 대소문자 무시)와 유사도 0.75 이상으로 자동 탐지를 돌렸고, 걸린 것이 없었다. 아래 "이름 차이"는 사람이 읽고 확인한 축약 표기만 적었다.
- 문서 묶음: `item/*`(6개 문서)는 `UCL9ItemDef` 하나에, `monster/*`(6개 문서)는 `UCL9MonsterDef` 하나에 합쳐 대조했다.

## 요약 (확인된 것만)

1. 문서에는 열로 적혀 있으나 헤더에 실제로 없는 것, 그리고 그 사유:
   - `LightRadius` — `C:\Work\CL9\docs\plan\data\item\egg.md:29` 는 계란 행 컬럼으로 192 를 적지만, `C:\Work\CL9\Source\CL9\Data\CL9ItemDef.h:215-216` 은 빛 수치를 `LampKind` 한 칸으로 옮겼고 `LightRadius` 는 없다. 취소선 표기가 아니므로 문서가 낡은 것이다. 같은 표의 `bEmitsLight`(egg.md:28)는 문서가 스스로 취소선으로 폐기했다.
   - `OrdersOfferedCurve` — `C:\Work\CL9\docs\plan\data\progression.md:48`, `FinalDay` — progression.md:83. `Source` 전체에 없다(`UCL9ProgressionDef` 에는 `OrdersOfferedPerDay`·`OrdersRequiredPerDay`·`QuotaCurve`·`SettlementIntervalDays` 등만 있다).
   - `TravelDays` — `C:\Work\CL9\docs\plan\data\site.md:261`. `Source` 전체에 없고 문서가 "당분간 0으로 둔다"고 적는다.
   - `DarkAccel` — `C:\Work\CL9\docs\plan\data\site.md:273` 의 "밤 시계" 표는 사이트 열로 적었으나, `UCL9SiteDef` 에는 `NightLength`·`DarkBase`·`DarkMin` 만 있고(`C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:120,124,128`) `DarkAccel` 은 `UCL9RulesDef` 에만 있다(`C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:281`).
   - `LobbyLighting`·`NightLighting`·`BlackoutLighting`·`SunScale` 등 site.md:299-318 의 조명 열은 문서가 스스로 "옮기기 전의 기록"(site.md:282 부근)이라 밝혔고, 지금은 `UCL9LookDef` 의 `FCL9LightingProfile`/`FCL9LightingPhase`(`C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:19,91`)에 있다. `UCL9SiteDef` 에는 `LightingProfile`(CL9SiteDef.h:315)만 남았다.
   - `CollisionProfile`·`MinimapColor` — `C:\Work\CL9\docs\plan\data\segkind.md:27,29`. `C:\Work\CL9\Source\CL9\Data\CL9SegKindDef.h:42` 주석이 "아직 없는 것"이라고 적는다.
   - `VoiceSet`·`FootstepSet` — `C:\Work\CL9\docs\plan\data\character.md:22,23`. 문서 character.md:30 이 "아직 `UCL9CharacterDef`에 없다"고 스스로 적는다.
   - `Tier`(`C:\Work\CL9\docs\plan\data\monster\common.md:26`)와 `RageThresholdMin`·`RageThresholdMax`·`RageGainPerEvent`·`RageDecayPerSec`(common.md:41-44). 문서가 "컬럼은 초안, 구현 전"(common.md:16, 37)이라고 적는다. 헤더에는 `RageTime`(`C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h:302`)만 있고 그것은 다른 뜻의 열이다.
   - sequence.md 의 컬럼 6개(`Alias`·`Sequence`·`bLockLocalInput`·`bSkippable`·`bHostWaits`·`Gimmicks`, `C:\Work\CL9\docs\plan\data\sequence.md:28-33`)는 `UCL9SequenceDef` 클래스 자체가 `Source` 에 없다. 문서 sequence.md:8 이 "현재는 구현하지 않음"이라고 적는다.
2. 문서에 취소선으로 폐기 표기된 rules.md:137 의 `Look*` 여섯 열(`LookMaxResolution` 등)은 헤더에서 이미 사라졌고 일치한다(look.md 의 `MaxResolution` 등으로 옮김).
3. 이름 차이로 분류할 것(축약 표기이며 실제 불일치는 아님):
   - `C:\Work\CL9\docs\plan\data\monster\raccoon.md:35` 의 `DetectRateNear / Far / Decay / Grace` — 헤더는 `DetectRateNear`·`DetectRateFar`(`C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h:119,123`)·`DetectDecay`·`DetectGrace`(CL9MonsterDef.h:127,131)이고 monster/common.md:96-99 는 같은 이름으로 적는다. 따라서 raccoon.md:35 의 `Far`·`Decay`·`Grace` 는 축약 표기일 뿐 실제 불일치가 아니다.
   - `C:\Work\CL9\docs\plan\data\wave.md:201,205` 의 `FloorHeightMin / Max`, `StampLifeMin / Max` — 헤더의 `FloorHeightMax`(`CL9WaveTable.h:375`)·`StampLifeMax`(`CL9WaveTable.h:394`)를 줄여 쓴 것이다. wave.md:14,216 의 `bUrgencyFrom*` 도 `bUrgencyFromMonsterHp`(CL9WaveTable.h:187)·`bUrgencyFromRage`(CL9WaveTable.h:195)를 묶어 부른 표기다.
4. 헤더에 있는데 문서에 (이름으로) 없는 것 중 눈에 띄는 것:
   - `C:\Work\CL9\docs\plan\data\look.md` 의 램프 종류·구조면 표(look.md:95-126)는 한글 머리글(세기 cd·거리 cm 등)이라 `FCL9LampKind`(`Intensity`·`Radius`·`Color`·`Scattering`·`ConeAngle`·`Flicker`·`FlickerSpeed`)와 `FCL9SurfaceFamily`(`Normal`·`Roughness`·`Tiling`·`NormalStrength`·`RoughnessMin`·`RoughnessMax`·`Materials`)의 필드 이름이 문서에 적히지 않았다. `FCL9LightingProfile::Name`(CL9LookDef.h:97), `UCL9LookDef::SurfaceFamilies`(CL9LookDef.h:328)도 단어로 나오지 않는다.
   - `C:\Work\CL9\docs\plan\data\site.md` 에 `UCL9SiteDef::Districts`(`CL9SiteDef.h:306`)와 `FCL9SiteDistrict` 의 `Offset`·`Yaw`·`ModuleCount`(CL9SiteDef.h:59,63,67)가 없다.
   - `C:\Work\CL9\docs\plan\data\module.md` 에 `FCL9DoorSlot::Cell`(`CL9ModuleDef.h:188`)이라는 이름이 없다(module.md:137 은 "칸"이라고 풀어 적음).
   - monster 문서에 `FDropItem` 의 `DropChance`·`MinQuantity`·`MaxQuantity`(`C:\Work\CL9\Source\CL9\Data\CL9Types.h:527-533`), `FCL9ItemCount::Count`(CL9Types.h:516), `FCL9ItemRef` 의 `ItemAlias`·`ItemSubAlias`(CL9Types.h:469,473)가 없다(monster/common.md:267-273 은 `DropItems` 열만 적음). `ItemAlias`·`ItemSubAlias` 는 equipment.md·item/common.md 에도 없다.
   - `C:\Work\CL9\docs\plan\data\wave.md` 에 `UCL9WaveTable::Rows`(`CL9WaveTable.h:444`)와 `FCL9WaveReveal::Material`(CL9WaveTable.h:258)이 이름으로 없다.

아래는 문서별 기계 추출 결과 전체다(오탐은 위 요약에서 설명한 축약·취소선·한글 머리글 때문이다).

---

# 문서별 상세 (기계 추출)

## buff.md  (대응 클래스: `UCL9BuffDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9BuffDef.h:21; 포함 구조체: UCL9BuffDef, FCL9StatEffect, UCL9DefBase
- 컬럼표 항목 3개, 클래스·구조체 UPROPERTY 4개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- (없음)
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- (없음)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `Stat` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:573); `Amount` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:576); `DurationSec` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:580)

## character.md  (대응 클래스: `UCL9CharacterDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9CharacterDef.h:20; 포함 구조체: UCL9CharacterDef, UCL9DefBase
- 컬럼표 항목 37개, 클래스·구조체 UPROPERTY 32개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- `VoiceSet` — C:\Work\CL9\docs\plan\data\character.md:22 [비고: character.md:30 이 스스로 'UCL9CharacterDef 에 아직 없다'고 적음]
- `FootstepSet` — C:\Work\CL9\docs\plan\data\character.md:23 [비고: character.md:30 이 스스로 'UCL9CharacterDef 에 아직 없다'고 적음]
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- (없음)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `CoyoteTime` (C:\Work\CL9\Source\CL9\Data\CL9CharacterDef.h:128); `JumpBufferTime` (C:\Work\CL9\Source\CL9\Data\CL9CharacterDef.h:132)

## equipment.md  (대응 클래스: `UCL9EquipmentDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9EquipmentDef.h:30; 포함 구조체: UCL9EquipmentDef, FCL9ItemRef, UCL9DefBase
- 컬럼표 항목 13개, 클래스·구조체 UPROPERTY 9개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- (없음)
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- `ItemAlias` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:469 (FCL9ItemRef)
- `ItemSubAlias` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:473 (FCL9ItemRef)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- (없음)

## item/*  (대응 클래스: `UCL9ItemDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9ItemDef.h:61; 포함 구조체: UCL9ItemDef, FCL9ItemRef, UCL9DefBase
- 컬럼표 항목 78개, 클래스·구조체 UPROPERTY 48개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- `bEmitsLight` — C:\Work\CL9\docs\plan\data\item\egg.md:28 [문서가 취소선(~~)으로 폐기 표기] [비고: 취소선 — 컬럼을 제거했다고 문서가 적음]
- `LightRadius` — C:\Work\CL9\docs\plan\data\item\egg.md:29 [비고: 취소선 아님. 헤더는 빛 수치를 LampKind(룩 표 램프 종류) 한 칸으로 옮겼고 LightRadius 는 없다 (CL9ItemDef.h:215-216)]
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- `ItemAlias` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:469 (FCL9ItemRef)
- `ItemSubAlias` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:473 (FCL9ItemRef)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `HoldPose` (C:\Work\CL9\Source\CL9\Data\CL9ItemDef.h:98); `bStackable` (C:\Work\CL9\Source\CL9\Data\CL9ItemDef.h:101)

## look.md  (대응 클래스: `UCL9LookDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:216; 포함 구조체: UCL9LookDef, FCL9LightingProfile, FCL9LightingPhase, FCL9LampKind, FCL9SurfaceFamily, UCL9DefBase
- 컬럼표 항목 22개, 클래스·구조체 UPROPERTY 47개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- (없음)
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- `SurfaceFamilies` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:328 (UCL9LookDef)
- `Name` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:97 (FCL9LightingProfile)
- `Intensity` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:140 (FCL9LampKind)
- `Radius` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:144 (FCL9LampKind)
- `Color` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:148 (FCL9LampKind)
- `Scattering` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:152 (FCL9LampKind)
- `ConeAngle` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:159 (FCL9LampKind)
- `Flicker` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:163 (FCL9LampKind)
- `FlickerSpeed` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:167 (FCL9LampKind)
- `Normal` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:181 (FCL9SurfaceFamily)
- `Roughness` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:185 (FCL9SurfaceFamily)
- `Tiling` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:189 (FCL9SurfaceFamily)
- `NormalStrength` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:193 (FCL9SurfaceFamily)
- `RoughnessMin` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:196 (FCL9SurfaceFamily)
- `RoughnessMax` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:199 (FCL9SurfaceFamily)
- `Materials` — C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:203 (FCL9SurfaceFamily)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `Profiles` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:283); `NightDark` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:287); `NightBright` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:291); `LampKinds` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:309); `bSurfaceDetail` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:325); `Brightness` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:106); `Lobby` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:110); `Night` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:114); `Blackout` (C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:118)

## module.md  (대응 클래스: `UCL9ModuleDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:238; 포함 구조체: UCL9ModuleDef, FCL9DoorSlot, FCL9PropSet, FCL9PropPlacement, FCL9FixturePlacement, UCL9DefBase
- 컬럼표 항목 20개, 클래스·구조체 UPROPERTY 31개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- (없음)
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- `Cell` — C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:188 (FCL9DoorSlot)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `Side` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:192); `Level` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:196); `Props` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:218); `Fixtures` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:222); `Mesh` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:116); `Local` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:120); `Yaw` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:124); `bSolid` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:131); `SideRule` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:138); `LampKind` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:145); `LightOffset` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:149); `ActorClass` (C:\Work\CL9\Source\CL9\Data\CL9ModuleDef.h:165)

## monster/*  (대응 클래스: `UCL9MonsterDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h:22; 포함 구조체: UCL9MonsterDef, FDropItem, FCL9ItemCount, FCL9ItemRef, UCL9DefBase
- 컬럼표 항목 220개, 클래스·구조체 UPROPERTY 80개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- `Tier` — C:\Work\CL9\docs\plan\data\monster\common.md:26 [비고: monster/common.md:16 이 '컬럼은 초안, 구현 전'이라고 적음]
- `RageThresholdMin` — C:\Work\CL9\docs\plan\data\monster\common.md:41 [비고: monster/common.md:37 '구현 전']
- `RageThresholdMax` — C:\Work\CL9\docs\plan\data\monster\common.md:42 [비고: monster/common.md:37 '구현 전']
- `RageGainPerEvent` — C:\Work\CL9\docs\plan\data\monster\common.md:43 [비고: monster/common.md:37 '구현 전']
- `RageDecayPerSec` — C:\Work\CL9\docs\plan\data\monster\common.md:44 [비고: monster/common.md:37 '구현 전']
- `Far` — C:\Work\CL9\docs\plan\data\monster\raccoon.md:35 [비고: 축약 표기 — `DetectRateNear / Far / Decay / Grace` = 헤더 DetectRateNear/Far(CL9MonsterDef.h:119,123)와 이어지는 Decay·Grace 의 축약. 이름 차이]
- `Decay` — C:\Work\CL9\docs\plan\data\monster\raccoon.md:35 [비고: 축약 표기 (위 Far 와 같은 줄)]
- `Grace` — C:\Work\CL9\docs\plan\data\monster\raccoon.md:35 [비고: 축약 표기 (위 Far 와 같은 줄)]
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- `DropChance` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:527 (FDropItem)
- `MinQuantity` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:530 (FDropItem)
- `MaxQuantity` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:533 (FDropItem)
- `Count` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:516 (FCL9ItemCount)
- `ItemAlias` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:469 (FCL9ItemRef)
- `ItemSubAlias` — C:\Work\CL9\Source\CL9\Data\CL9Types.h:473 (FCL9ItemRef)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `SkeletalMesh` (C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h:32); `Skeleton` (C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h:36); `AnimBP` (C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h:39); `MeshYawOffset` (C:\Work\CL9\Source\CL9\Data\CL9MonsterDef.h:47)

## order.md  (대응 클래스: `UCL9OrderDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9OrderDef.h:17; 포함 구조체: UCL9OrderDef, FCL9ItemCount, FCL9ItemRef, UCL9DefBase
- 컬럼표 항목 12개, 클래스·구조체 UPROPERTY 11개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- (없음)
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- (없음)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `Count` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:516); `ItemAlias` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:469); `ItemSubAlias` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:473)

## progression.md  (대응 클래스: `UCL9ProgressionDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9ProgressionDef.h:21; 포함 구조체: UCL9ProgressionDef, FCL9DayOverride, UCL9DefBase
- 컬럼표 항목 13개, 클래스·구조체 UPROPERTY 15개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- `OrdersOfferedCurve` — C:\Work\CL9\docs\plan\data\progression.md:48 [비고: Source 어디에도 없음]
- `FinalDay` — C:\Work\CL9\docs\plan\data\progression.md:83 [비고: Source 어디에도 없음]
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- (없음)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `Day` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:608); `MonsterCountMult` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:612); `DangerMult` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:616); `bNoBlackout` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:620)

## rules.md  (대응 클래스: `UCL9RulesDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:185; 포함 구조체: UCL9RulesDef, FCL9ViewFeel, UCL9DefBase
- 컬럼표 항목 111개, 클래스·구조체 UPROPERTY 141개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- `LookMaxResolution` — C:\Work\CL9\docs\plan\data\rules.md:137 [문서가 취소선(~~)으로 폐기 표기]
- `LookAntiAliasing` — C:\Work\CL9\docs\plan\data\rules.md:137 [문서가 취소선(~~)으로 폐기 표기]
- `LookPosterizeBands` — C:\Work\CL9\docs\plan\data\rules.md:137 [문서가 취소선(~~)으로 폐기 표기]
- `LookPosterizeMix` — C:\Work\CL9\docs\plan\data\rules.md:137 [문서가 취소선(~~)으로 폐기 표기]
- `LookGrain` — C:\Work\CL9\docs\plan\data\rules.md:137 [문서가 취소선(~~)으로 폐기 표기]
- `LookPosterizeMaterial` — C:\Work\CL9\docs\plan\data\rules.md:137 [문서가 취소선(~~)으로 폐기 표기]
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- (없음)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `ViewFeel` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:827); `BobWalk` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:34); `BobRun` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:38); `BobSharpWalk` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:42); `BobSharpRun` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:46); `BobAmpInterp` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:50); `BobRollWalk` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:54); `BobRollRun` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:58); `StrafeRoll` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:62); `StrafeRollInterp` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:69); `SprintFovKick` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:73); `FovInterpUp` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:77); `FovInterpDown` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:81); `FovRateCap` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:85); `SpringOmega` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:89); `SpringZeta` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:93); `LandMinSpeed` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:97); `LandHardSpeed` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:101); `LandDipAtJumpSpeed` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:105); `LandDipExponent` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:109); `LandDipMax` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:113); `JumpDip` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:117); `FootstepVolumeWalk` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:121); `StepIdleResetTime` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:125); `ArmsLagGain` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:132); `ArmsLagDecay` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:136); `ArmsLagMaxYaw` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:140); `ArmsLagMaxPitch` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:144); `ArmsBobScale` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:148); `ArmsSpringOmega` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:152); `ArmsSpringZeta` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:156); `ArmsImpulseScale` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:160); `ArmsOffsetMax` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:164); `EquipDip` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:168); `EquipDipDownTime` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:172); `EquipDipUpTime` (C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:176)

## segkind.md  (대응 클래스: `UCL9SegKindDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9SegKindDef.h:45; 포함 구조체: UCL9SegKindDef, UCL9DefBase
- 컬럼표 항목 8개, 클래스·구조체 UPROPERTY 4개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- `CollisionProfile` — C:\Work\CL9\docs\plan\data\segkind.md:27 [비고: CL9SegKindDef.h:42 주석이 '아직 없는 것'이라고 적음]
- `MinimapColor` — C:\Work\CL9\docs\plan\data\segkind.md:29 [비고: CL9SegKindDef.h:42 주석이 '아직 없는 것'이라고 적음]
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- (없음)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- (없음)

## site.md  (대응 클래스: `UCL9SiteDef`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:82; 포함 구조체: UCL9SiteDef, FCL9SpawnEntry, FCL9SiteZone, FCL9SiteDistrict, UCL9DefBase
- 컬럼표 항목 84개, 클래스·구조체 UPROPERTY 45개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- `LampSurviveShare` — C:\Work\CL9\docs\plan\data\site.md:37 — 단, 다른 클래스에는 있다: FCL9LightingProfile(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:125) [문서가 취소선(~~)으로 폐기 표기]
- `TravelDays` — C:\Work\CL9\docs\plan\data\site.md:261 [비고: Source 어디에도 없음. site.md:262 '당분간 0으로 둔다 — 먼 목적지가 생기면 켠다']
- `DarkAccel` — C:\Work\CL9\docs\plan\data\site.md:273 — 단, 다른 클래스에는 있다: UCL9RulesDef(C:\Work\CL9\Source\CL9\Data\CL9RulesDef.h:281) [비고: 헤더에는 UCL9RulesDef 에만 있다(CL9RulesDef.h:281). site.md '밤 시계' 표가 사이트 열로 적고 있으나 UCL9SiteDef 에는 NightLength·DarkBase·DarkMin 만 있다]
- `LobbyLighting` — C:\Work\CL9\docs\plan\data\site.md:299 [비고: site.md:280-282 가 '옮기기 전의 기록'이라고 밝힘 — 지금은 UCL9LookDef 프로필]
- `NightLighting` — C:\Work\CL9\docs\plan\data\site.md:300
- `BlackoutLighting` — C:\Work\CL9\docs\plan\data\site.md:301
- `SunScale` — C:\Work\CL9\docs\plan\data\site.md:307 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:25)
- `SunColor` — C:\Work\CL9\docs\plan\data\site.md:308 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:29)
- `SkyScale` — C:\Work\CL9\docs\plan\data\site.md:309 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:36)
- `SkyColor` — C:\Work\CL9\docs\plan\data\site.md:310 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:40)
- `SkyLeak` — C:\Work\CL9\docs\plan\data\site.md:311 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:47)
- `FogDensity` — C:\Work\CL9\docs\plan\data\site.md:312 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:51)
- `FogColor` — C:\Work\CL9\docs\plan\data\site.md:313 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:55)
- `SkyMaterial` — C:\Work\CL9\docs\plan\data\site.md:314 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:70)
- `bClouds` — C:\Work\CL9\docs\plan\data\site.md:315 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:74)
- `SkyScattering` — C:\Work\CL9\docs\plan\data\site.md:316 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:78)
- `SkyLuminance` — C:\Work\CL9\docs\plan\data\site.md:317 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:66)
- `FogSkyAmbient` — C:\Work\CL9\docs\plan\data\site.md:318 — 단, 다른 클래스에는 있다: FCL9LightingPhase(C:\Work\CL9\Source\CL9\Data\CL9LookDef.h:62)
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- `Districts` — C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:306 (UCL9SiteDef)
- `Offset` — C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:59 (FCL9SiteDistrict)
- `Yaw` — C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:63 (FCL9SiteDistrict)
- `ModuleCount` — C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:67 (FCL9SiteDistrict)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `Alias` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:591); `MinCount` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:594); `MaxCount` (C:\Work\CL9\Source\CL9\Data\CL9Types.h:598); `Name` (C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:27); `DepthMin` (C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:31); `DepthMax` (C:\Work\CL9\Source\CL9\Data\CL9SiteDef.h:35)

## wave.md  (대응 클래스: `UCL9WaveTable`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:437; 포함 구조체: UCL9WaveTable, FCL9WaveInfo, FCL9WaveDotLook, FCL9WaveRipple, FCL9WaveUrgency, FCL9WaveReveal
- 컬럼표 항목 49개, 클래스·구조체 UPROPERTY 84개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- `Max` — C:\Work\CL9\docs\plan\data\wave.md:201 [비고: 축약 표기 — `XxxMin / Max` = XxxMin·XxxMax]
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- `Rows` — C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:444 (UCL9WaveTable)
- `bUrgencyFromMonsterHp` — C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:187 (FCL9WaveInfo)
- `bUrgencyFromRage` — C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:195 (FCL9WaveInfo)
- `FloorHeightMax` — C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:375 (FCL9WaveDotLook)
- `StampLifeMax` — C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:394 (FCL9WaveDotLook)
- `Material` — C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:258 (FCL9WaveReveal)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- `DotSystem` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:459); `Dots` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:463); `Ripple` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:467); `Urgency` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:471); `Reveal` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:475); `bRevealBodies` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:152); `bUrgencyFromMonsterState` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:183); `bUrgencyFromCharacterHp` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:191); `UrgentColor` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:202); `CrestSpacing` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:220); `CrestDotGap` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:224); `CrestLife` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:228); `CrestSize` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:232); `CrestBrightness` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:236); `EchoGap` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:240); `EchoFalloff` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:244); `StateUrgency` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:296); `bColor` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:300); `BrightnessMax` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:303); `bEchoCount` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:307); `EchoMin` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:310); `EchoMax` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:313); `bJitter` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:317); `JitterMax` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:320); `bPulseRate` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:324); `PulseRateMax` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:327); `Spacing` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:262); `DotRadius` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:266); `Push` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:270); `Hold` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:278); `Fade` (C:\Work\CL9\Source\CL9\Data\CL9WaveTable.h:282)

## sequence.md  (대응 클래스: `UCL9SequenceDef`)
- 클래스 `UCL9SequenceDef` 는 Source 에 없다. 문서 컬럼표 항목 6개는 대응 헤더가 없다.
  - `Alias` — C:\Work\CL9\docs\plan\data\sequence.md:28
  - `Sequence` — C:\Work\CL9\docs\plan\data\sequence.md:29
  - `bLockLocalInput` — C:\Work\CL9\docs\plan\data\sequence.md:30
  - `bSkippable` — C:\Work\CL9\docs\plan\data\sequence.md:31
  - `bHostWaits` — C:\Work\CL9\docs\plan\data\sequence.md:32
  - `Gimmicks` — C:\Work\CL9\docs\plan\data\sequence.md:33

## README.md(공통 뼈대)  (대응 클래스: `UCL9DefBase`)
- 클래스 위치: C:\Work\CL9\Source\CL9\Data\CL9DefBase.h:41; 포함 구조체: UCL9DefBase
- 컬럼표 항목 8개, 클래스·구조체 UPROPERTY 8개.
### (a) 문서 컬럼표에 있으나 헤더에 없는 것
- (없음)
### 이름 차이 (문서 표기 / 헤더 표기)
- (없음)
### (b) 헤더에 있으나 문서에 없는 것 (문서 어디에도 해당 단어가 없음)
- (없음)
### (b') 헤더에 있고 문서 본문에는 나오나 컬럼표에는 없는 것
- (없음)


---

# plan/data 문서에 컬럼표가 없는 데이터 클래스의 멤버 (참고)

멤버 이름이 `C:\Work\CL9\docs\plan\data` 아래 어느 문서에 단어로 나오는지만 본 것이다(열 이름으로 쓰였는지까지는 확인하지 않았다). 원자료는 `C:\Users\Radar\AppData\Local\Temp\claude\C--Work-CL9\199f7b2b-76ff-413d-9ace-220ec8f79b1f\scratchpad\audit_extra.md` 에 있다.

- `UCL9PieceDef`(`C:\Work\CL9\Source\CL9\Data\CL9PieceDef.h:25`): 멤버 `Variants`(CL9PieceDef.h:38)는 plan/data 어디에도 단어로 나오지 않는다. segkind.md:149-181 이 조각 표를 말로만 설명한다.
- `UCL9SkinDef`(`C:\Work\CL9\Source\CL9\Data\CL9SkinDef.h:17`): `BodyMesh`·`TintSlot`·`TintParameter`·`Tint`·`bDefault` 는 plan/data 에 없고, `Price` 는 `UCL9EquipmentDef` 의 열(equipment.md:15)로만 나온다. 스킨 표의 plan/data 문서는 없다.
- `UCL9SettingsTable`(`C:\Work\CL9\Source\CL9\UI\CL9SettingsTable.h:21`): 멤버 11개(`SoundMix`, `MasterClass`, `EffectsClass`, `UIClass`, `AmbienceClass`, `MusicClass`, `VOClass`, `VoiceClass`, `VoiceAttenuation`, `VoiceSpeakerAttenuation`, `PreviewSound`) 전부 plan/data 에 없다.
- `UCL9UITable`/`FCL9UIInfo`(`C:\Work\CL9\Source\CL9\UI\CL9UITable.h:26,43`): `MainWidgetClass`·`UIMappingContext`·`WidgetClass`·`Layer` 는 plan/data 에 없다. rules.md:346 이 클래스 이름만 언급한다.
- `UCL9ShakeTable`/`FCL9ShakeInfo`(`C:\Work\CL9\Source\CL9\Data\CL9ShakeTable.h:14,48`): `Rows`·`Shake`·`StartAt`·`Scale` 은 rules.md:346-347 에 적혀 있다(일치).
- `UCL9SoundTable`(`C:\Work\CL9\Source\CL9\Data\CL9SoundTable.h:19`): 멤버 `Rows` 하나이며 rules.md:267 이 표 이름을 적는다.
- `UCL9DefList`(`C:\Work\CL9\Source\CL9\Data\CL9DefRegistry.h:53`): 멤버 `Defs`(CL9DefRegistry.h:60)는 plan/data 에 없다.

---

# Content/Data 표 파일 존재 대조

`C:\Work\CL9\Content\Data` 에 있는 `.uasset` 19개: BuffTable, CharacterTable, EquipmentTable, MonsterTable, OrderTable, ShakeTable, PieceTable, SegKindTable, SoundTable, SettingsTable, CL9Progression, SkinTable, CL9Rules, ModuleTable, ItemTable, CL9DefList, LookTable, MapTable, WaveTable. (파일 존재만 확인했고 내용은 열지 않았다.)

## 문서에 이름 붙은 표인데 Content/Data 에 .uasset 이 없는 것

- `SequenceTable` — `C:\Work\CL9\docs\plan\data\sequence.md:8` 이 "`UCL9SequenceDef` 클래스도 `SequenceTable`"을 아직 구현하지 않았다고 적는다. `C:\Work\CL9\Content\Data\SequenceTable.uasset` 은 없고 `Content` 전체에도 이 이름의 에셋은 없다.
- 그 밖에 plan/data 문서가 이름으로 부른 표(`ItemTable`, `MonsterTable`, `CharacterTable`, `OrderTable`, `LookTable`, `CL9Progression`, `CL9Rules`, `ModuleTable`, `SegKindTable`, `PieceTable`, `MapTable`, `WaveTable`, `SoundTable`, `ShakeTable`)은 전부 파일이 있다. 참고로 rules.md:346 이 같은 모양이라고 언급한 `UITable` 은 `Content/Data` 가 아니라 `C:\Work\CL9\Content\UI\UITable.uasset` 에 있다.

## Content/Data 에 있는데 어느 plan/data 문서에도 (그 이름으로) 안 나오는 표

`C:\Work\CL9\docs\plan\data` 아래에서 단어 검색으로 한 건도 걸리지 않은 것이다.

- `BuffTable` — 다만 클래스 `UCL9BuffDef` 는 buff.md 가 표 전체를 설명한다. 에셋 이름만 안 적혀 있다.
- `EquipmentTable` — 클래스 `UCL9EquipmentDef` 는 equipment.md 가 설명한다. 에셋 이름만 안 적혀 있다.
- `SkinTable` — 클래스 `UCL9SkinDef` 를 설명하는 plan/data 문서가 없다.
- `SettingsTable` — 클래스 `UCL9SettingsTable` 을 설명하는 plan/data 문서가 없다.
- `CL9DefList` — plan/data 에 이름이 없다(README.md:75-90 은 `UCL9DefRegistry` 조회만 설명한다).
