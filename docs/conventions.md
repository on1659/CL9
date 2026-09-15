# CL9 규약

> 이 문서는 프로젝트 설계 규약(§1~10)을 담는다.
> CCL8 승계 "다섯 줄"·"열리지 않는 문" 전역 원칙은 2026-09-01에 폐지 — 필요한 내용은
> 각 시스템의 설정·현재 설계 서술로 존재한다 (decisions.md).
> 코딩 표준(변수/함수 네이밍, 포맷팅 등)은 별도 문서: [ue-cpp-conventions.md](ue-cpp-conventions.md)
> 사운드·액터 베이스의 설계 전문은 [systems/sound.md](systems/sound.md), [systems/actor-base.md](systems/actor-base.md)에 있다.
> 결정 이력은 [decisions.md](decisions.md), 기획 전반은 [plan/](plan/README.md)에서 시작한다.

---

## § 1. 프로젝트 핵심 원칙

**CL9은 데이터 드리븐 프로젝트다. 하드코딩을 지양한다.**

### 데이터 드리븐 원칙

모든 게임 값(아이템, 몬스터, 규칙, 일자 진행 등)은 **DataAsset(+곡선 에셋)**에서 읽는다.

```cpp
// ❌ 하드코딩 (금지)
if (ItemName == "egg") {
  ItemPrice = 100;
  ItemWeight = 0.5f;
}

// ✅ 데이터 기반 (필수)
const UCL9ItemDef* ItemData = DefRegistry->FindItem("egg");
if (ItemData) {
  // 가격 굴림은 판매 시점에 개체마다 (plan/economy.md) — 여기선 범위만 읽는다
  ItemWeight = ItemData->Weight;
}
```

### 액터 생성 방식

```cpp
// ❌ 하드코딩 액터 생성 (금지)
ACL9Item* Item = SpawnActor<ACL9Item>();
Item->SetupAsEgg();  // 하드코딩된 함수

// ✅ 데이터 기반 액터 생성 (필수)
ACL9Item* Item = SpawnActor<ACL9Item>();
Item->SetAlias("egg");  // 데이터에서 읽은 Alias
// → LoadData() 자동 호출 → DataAsset에서 모든 정보 로드
```

### 설정 방식

```cpp
// ❌ 코드에 숫자 박기 (금지)
#define DEFAULT_PLAYER_HP 100
#define ROUND1_TIME 600
#define ROUND1_TARGET_MONEY 1500

// ✅ DataAsset에서 읽기 (필수)
float NightLength = Rules->NightLength;                        // CL9Rules
int32 Quota = Progression->QuotaCurve->GetFloatValue(Day);     // CL9Progression (곡선)
```

### 체크리스트

- ✅ 모든 **게임 값**(아이템, 몬스터, 규칙, 일자 곡선)은 DataAsset에서 관리
- ✅ 모든 **액터**는 Alias를 통해 데이터 참조 (느슨한 결합)
- ✅ **코드에는 구체적인 값을 박지 않음** (상수 정의 최소화)
- ✅ **레벨 또는 서버에서 데이터를 읽어서 전달** (하드코딩 X)
- ✅ **에디터에서 설정 변경 후 바로 테스트** (코드 빌드 불필요)

---

## § 2. 네트워크 아키텍처

**전제:** Steam 리슨 서버. 데디 없음. 호스트가 판정, 클라는 표현만.

- **판정/스폰/데미지/상태 전이 코드는 서버에서만 실행한다.**
  함수 진입부에 `if (!HasAuthority()) return;`을 둔다.
  
- **GameMode는 서버에만 존재한다.** 클라 코드에서 `GetGameMode()` 호출 금지.
  공유 상태는 GameState에만 두고, GameMode는 로직 전용이다.

- **클라에 보여야 하는 상태는 Replicated UPROPERTY 또는 Multicast RPC로만 전송한다.**
  직접 데이터를 보내는 구조는 만들지 않는다.

- **입력은 Server RPC로 올린다.** 클라에서 직접 상태 변경 금지.
  캐릭터가 할 일은 입력 받기와 표현뿐이다.

- **HasAuthority() 추천.** GetNetMode() 직접 비교는 Standalone/PIE를 빠뜨리기 쉬워 버그가 남는다.

---

## § 3. 모듈·폴더 구조

**원칙:** 처음부터 모듈을 많이 나누지 않는다. 필요해질 때 나눈다.

**소스 코드 (Source/CL9/):**
```
CL9/
  ├─ Character/        (ACL9Character, 이동 로직)
  ├─ GameMode/         (ACL9GameMode, ACL9GameState, ACL9PlayerState)
  ├─ Items/            (ACL9Item, 아이템 로직)
  ├─ Monsters/         (ACL9Monster, AI)
  ├─ Data/             (ACL9Types.h, CL9Constants.h, 구조체)
  └─ UI/               (ACL9HUD, 위젯 클래스)
```

**Constants.h 정책:**
- ✅ **물리 상수** (플레이어 반경, 눈높이, 통로 폭 등) — Constants.h에 OK
- ✅ **엔진 상수** (네트워크 업데이트 빈도, 프레임 레이트 등) — Constants.h에 OK
- ❌ **게임 밸런스** (아이템 가격, HP, 스태미나, 몬스터 스텟 등) — 모두 DataAsset에서

**콘텐츠 (Content/):**
```
Content/
  ├─ Characters/       (BP_CL9Character, 마네킹 스켈레톤·ABP, FarmerA/ 자체 제작 몸·팔·클립)
  ├─ Items/            (아이템·시설 BP — BP_CL9Item/Cart/Kitchen/Counter/Jar/TruckPad… + 그 메시·재질 폴더 CartA/Kitchen/…)
  ├─ Monsters/         (BP_CL9Monster, 동물별 ABP, <종>A/ 자체 제작 SK·클립)
  ├─ GameMode/         (BP_CL9GameMode/GameInstance/PlayerController)
  ├─ Input/            (IMC_Gameplay/Spectate/UI, Actions/)
  ├─ Maps/             (Lvl_Flat 테스트 맵, Lvl_Henhouse, Henhouse/·FoodTruck/ 맵 조각·소품 메시)
  ├─ Data/             (DataAsset 표 — *Table, CL9DefList/Rules/Progression. 접두사 없음 — 2026-09-12)
  ├─ UI/               (WBP_* 위젯 블루프린트, UITable)
  ├─ Effects/          (카메라 셰이크·파티클·사운드)
  ├─ Strings/          (CL9_Strings.csv 마스터 시트 + 생성물 CL9.csv — systems/localization.md)
  ├─ Localization/CL9/ (수집·번역 산출물 manifest·po·locres — 생성물, 커밋함)
  ├─ Tests/            (검수 레벨 Lvl_AssetQA01·리그 왕복 프로브 — 게임이 참조하지 않는 테스트물)
  └─ (벤더·템플릿) DreamscapeSeries/·AnimalVarietyPack/·FirstPerson/·Variant_Horror/·LevelPrototyping/
                       — 저장소 제외 또는 템플릿 잔재, 새 에셋을 두지 않는다
```

---

## § 4. 클래스 및 상태 관리

**핵심:** 게임 상태는 중앙집중식 (GameState)이고, 액터는 표현만 한다.

| 클래스 | 책임 | 범위 |
|--------|------|------|
| **ACL9GameMode** | 게임 로직, 판정, 데이터 로드 | 서버만 (클라에 없음) |
| **ACL9GameState** | 공유 게임 상태 (일차, 밤 시계, 판 자금 — 런타임 상태만) | Replicated (모두) |
| **ACL9PlayerState** | 각 플레이어 게임 상태 (스태미나, 체력, 짐) | Replicated (모두) |
| **ACL9Character** | 입력 처리, 이동(커스텀 CMC 예측), 표현 | Replicated (모든 머신에 폰 존재) |
| **ACL9Monster** | AI 로직 (호스트), 표현 (클라) | Replicated |
| **ACL9Item** | 필드 아이템 액터 | Replicated |

**상태 소유권:**
- 플레이어 상태 → `ACL9PlayerState`에만 둔다. Character는 PlayerState 값을 읽기만 한다.
- **예외 — 이동 예측 상태:** 스태미나·유효 무게에서 파생되는 이동 속도처럼 CMC 예측에
  들어가는 값은 CMC/`FSavedMove` 경로에 산다 (안 그러면 러버밴딩). PlayerState에는
  표시용 미러만 둔다.
- 몬스터 상태 → `ACL9Monster`에 Replicated 변수로. 호스트에서 수정, 클라에서 읽기.
- 게임 상태 (일차, 밤 시계, 판 자금) → `ACL9GameState`에만.

---

## § 5. 데이터 흐름

**원칙:** 정적 표는 각 머신이 로컬 레지스트리로 읽고, GameState는 **런타임 진행 상태만** 나른다.
(모든 머신이 같은 에셋을 갖고 있으므로 표를 복제할 이유가 없다 — 해시로 검증한다.)

1. **로드** (각 머신, 게임 시작 시)
   - `UCL9DefRegistry`가 DataAsset을 로드 — 호스트·클라 모두 로컬에서
2. **검증**
   - 로드 검사(참조 무결성·관계 부등식) + 접속 시 `CL9TuningHash`로 호스트·클라 값 대조
3. **런타임 상태만 복제** (GameState/PlayerState)
   - 일차·밤 시계·판 자금·주문 진행처럼 **변하는 상태**만 Replicated
4. **표현** (로컬)
   - 클라는 Replicated 상태 + 로컬 레지스트리 데이터로 UI·애니메이션 구성

**중요:** C++ 코드에 에셋 경로를 하드코딩하지 않는다. 정적 표를 통째로 GameState에 복제하지
않는다.

---

## § 6. 금지사항 및 필수사항

### ❌ 절대 금지

- 클라 코드에서 `GetGameMode()` 호출
- Actor가 자신의 Replicated 상태를 직접 수정 (서버에서만 가능)
- `TakeDamage()`, `UDamageType` — 엔진 데미지 파이프라인 대신 `ICL9Damageable::OnDamage(Amount, Source)`
  가상 함수 + 단순 HP 값을 쓴다 (호스트에서만 호출 — systems/character.md)
- 게임 전체 진행 판정(밤 전이·정산·주문 완료)을 액터에 작성 — GameMode에서만.
  액터 자신의 로직(Monster.Die(), Item.OnPickup())은 HasAuthority 관문 아래 자신에서 (아래 체크리스트)

GAS(`UAttributeSet` 등)는 금지가 아니라 **현재 미사용**이다 — 지금 설계는 Replicated 값이면
충분해서 쓰지 않고, 필요해지면(학습 겸 포함) 도입을 검토한다.

### ✅ 필수사항

- 판정 코드 진입부: `if (!HasAuthority()) return;`
- 공유 상태: GameState/PlayerState에만 두기
- Replicated 변수: 서버에서만 수정하고, 클라는 OnRep_ 콜백으로 반응한다
- 데이터: 모두 DataAsset/에디터에서 관리
- 복제 업데이트: `SetNetUpdateFrequency(20)` (20 FPS 고정)
- 게임 시간: `GameState->GetServerWorldTimeSeconds()` 기준 — 엔진이 클라 시계 오프셋을
  동기화해주므로 공유 타임스탬프(밤 시계·정전 시각·잠금)를 클라가 그대로 해석할 수 있다
- 화면 문자열: **String Table `CL9`의 키만** 쓴다 — 코드는 `LOCTABLE("CL9", "Shop.NoMoney")`, UMG·데이터
  행은 텍스트 피커의 String Table 참조. 원문·번역은 `Content/Strings/CL9_Strings.csv`(No·Key·DevNotes·언어 열) 한 장에만 있고, `NSLOCTEXT`/직접 친
  텍스트는 금지(행은 Validate가 Broken으로 잡는다). `FText::FromString`은 플레이어 이름·숫자·줄바꿈처럼
  번역 대상이 아닌 것에만. **평소엔 `ko` 열만 채운다 — 번역은 정해진 시점에 몰아서**, 문구 작업 중엔 신경 쓰지 않는다.
  키 규칙·추가 절차·번역 명령은 [systems/localization.md](systems/localization.md)
  (2026-09-12, 사용자 "string alias를 박아서 언어설정 바뀔때마다 싹다 바뀌어야")

### 📋 개발 중 체크리스트

- [ ] 새 플레이어 상태 변수를 만들 때: PlayerState에 Replicated로, 절대 Character에 안 됨
- [ ] 새 판정 로직을 만들 때: 액터의 자신 로직은 자신에서 (Monster.Die(), Item.OnPickup()), 게임 전체 진행은 GameMode에서만
- [ ] 데이터를 읽을 때: 정적 표는 로컬 레지스트리(DefRegistry)로, 런타임 상태는 GameState로
- [ ] 네트워크 테스트: 호스트 1명 + 클라 최소 1명으로 항상 검증
- [ ] 새 기능을 만들 때: 플레이어가 바꿀 값인지 [plan/settings.md](plan/settings.md) 체크리스트로 판단 — 옵션이면 [systems/settings.md](systems/settings.md) "옵션 추가" 절대로. 어느 시스템 문서를 볼지는 [systems/README.md](systems/README.md)

---

## § 7. 빌드 전략

**문제:** C++ 빌드는 매번 시간이 많이 든다.

**원칙:** 여러 파일을 모아서 한 번에 빌드한다. 빌드 중에는 블루프린트/에셋은 자유롭게 수정.

### Live Coding vs Full Rebuild

| 작업 | 방식 | 시간 |
|------|------|------|
| 함수 로직 수정 | Live Coding (에디터 켜둠) | 수 초 |
| 새 함수 추가 | Live Coding | 수 초~십 초 |
| 새 클래스 추가 | Full Rebuild (에디터 끔) | 분 단위 |
| 헤더/구조체 변경 | Full Rebuild | 분 단위 |
| UPROPERTY 추가 | Full Rebuild | 분 단위 |

**주의:** 구조체/클래스 선언 변경이 있으면 Live Coding 불가. 반드시 에디터 종료 후 빌드.

### 권장 워크플로우 (Claude와 협업)

**작업 흐름:**

```
1️⃣  Claude가 코드 작성/수정
     • ACL9Character 함수 추가
     • ACL9Monster 로직 변경
     • 파일 저장 (Edit 도구)

2️⃣  당신: 에디터 켜둔 상태 유지
     • 에디터는 닫지 않음
     • 다른 작업 진행 (블루프린트, 에셋)

3️⃣  Claude가 빌드 명령어 전달 또는 PowerShell에서 실행
     GenerateProjectFiles.bat (필요시)
     Visual Studio 또는 Unreal Engine Build

4️⃣  Live Coding 자동 작동
     • 에디터가 켜져 있으면 자동 Hot Reload
     • 구조체/클래스 선언 변경 없으면 성공
     • 새 함수/로직 변경만 해도 즉시 반영

5️⃣  결과 확인
     • 에디터 출력 로그 보기
     • 또는 Claude가 빌드 로그 분석

6️⃣  블루프린트/에셋은 자동 최신 상태
     바로 테스트 가능
```

**구조체/클래스 선언 변경이 필요한 경우:**

```
1️⃣  Claude가 .h 파일 수정 (UPROPERTY, 새 변수 등)

2️⃣  당신: 에디터 종료 (중요!)

3️⃣  빌드 실행 (Full Rebuild)

4️⃣  에디터 재시작

5️⃣  Unreal Header Tool이 자동으로 인식
```

### 증분 빌드

Visual Studio의 캐시를 활용하면 두 번째 빌드부터 빠르다:
- 첫 번째: 모든 파일 컴파일 (느림)
- 이후: 변경된 파일만 컴파일 (빠름)

**Intermediate/ 폴더를 지우지 않는다** — 캐시가 날아감.

### 개발 중 시간 절약

- **C++ 변경 작은 것들**: Live Coding으로 즉시 확인
- **선언 변경 여러 개**: 모아둘 때까지 기다렸다가 한 번에 빌드
- **블루프린트/데이터 수정**: C++ 빌드 기다릴 필요 없음 (독립적)

---

## § 8. Claude와의 개발 계약

**Claude가 할 수 있는 것:**

✅ C++ 코드 작성/수정
✅ conventions.md, 설계 문서 작성
✅ 빌드 명령어 제시
✅ 컴파일 로그 분석
✅ 코드 패턴 검증 (conventions 준수)
✅ 논리 오류 찾기
✅ 파일 수정 사항 추적

**당신이 해야 할 것:**

✅ 에디터 켜기/끄기 시점 결정
✅ PowerShell에서 빌드 명령어 실행
✅ 에디터에서 결과 확인
✅ Play 테스트 (게임 동작 검증)
✅ 네트워크 테스트 (호스트 + 클라)
✅ 에디터 로그 확인 및 에러 보고

**협업 흐름:**

```
Claude: 코드 작성 (Edit)
  ↓
You: 빌드 실행 (PowerShell)
  ↓
Claude: 로그 분석 (Read)
  ↓
You: 에디터에서 확인
  ↓
[반복]
```

**주의:**

- 에디터 켜둔 상태에서 `.cpp` 함수 로직 수정만 해도 Live Coding 자동 작동
- 헤더 파일(`.h`) 구조체/클래스 선언 변경 → 에디터 반드시 종료
- 새 파일 추가 → `GenerateProjectFiles.bat` 실행 후 빌드

---

## § 9. 사운드 규칙

**원칙:** 모든 게임 이벤트에 사운드를 첨부한다.

- 사운드 재생은 **표현 계층(Character, Monster, UI)에서만** — 판정 코드(GameMode, 서버)에서 직접 재생하지 않는다.
- 동기화는 **Multicast RPC** 또는 Replicated 콜백으로.
- 시스템 공용 에셋(MasterSoundMix)만 경로 하드코딩 예외 — 게임 콘텐츠는 항상 DataAsset에서 읽는다.

전문(분류·이벤트 목록·에셋 위치·재생/볼륨 설정)은 [systems/sound.md](systems/sound.md).

---

## § 10. 액터 베이스 규칙

**원칙:** 아이템·몬스터 등 CL9 액터는 `ACL9ActorBase`를 상속한다. 공통 로직(네트워크, RPC, 사운드)은 베이스에 두고, 상세 구현은 자식에서.

- **예외 — 캐릭터:** `ACL9Character`는 `ACharacter`를 상속한다 (2026-08-31 A4 결정). CMC 예측 이동(커스텀 CMC + `FSavedMove`)이 필요해서 AActor 계열 베이스를 쓸 수 없다.
- `Alias`는 Replicated — **SetAlias()는 서버에서만** 호출한다.
- 데이터 로드: 호스트는 SetAlias() 안에서 직접 `LoadData()`. 클라의 **초기 로드는 BeginPlay**
  (레벨 배치 액터는 값 변화가 없어 OnRep이 안 뜬다), 런타임 Alias/SubAlias 변경만
  `OnRep_AliasPair()` 콜백으로 실행된다.
- Alias 하드코딩 금지 — 레벨 배치 값 또는 스테이지 설정(데이터)에서 읽는다.

전문(클래스 설계·RPC 패턴·동기화 흐름·사용 예)은 [systems/actor-base.md](systems/actor-base.md).
