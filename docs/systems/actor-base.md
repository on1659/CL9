# 액터 베이스 구조 (ACL9ActorBase)

> 아이템·몬스터 등 CL9 액터의 공통 베이스 클래스 설계 — 네트워크, RPC, 사운드, Alias 데이터 로드.
> 상속·Alias 규칙은 [../conventions.md](../conventions.md) § 10 참고.

## 원칙

아이템·몬스터 등 CL9 액터는 `ACL9ActorBase`를 상속한다. 공통 로직(네트워크, RPC, 사운드)은 베이스에 두고, 상세 구현은 자식에서.

**예외 — 캐릭터:** `ACL9Character`는 `ACharacter`를 상속한다 (2026-08-31 A4 결정). 멀티플레이어 예측 이동에 CharacterMovementComponent(커스텀 CMC + `FSavedMove`)가 필요해서 AActor 계열 베이스를 쓸 수 없다. Alias 데이터 로드가 필요하면 같은 패턴(Replicated `Alias` + `OnRep_Alias`)을 캐릭터 쪽에 별도로 구현한다.

## 액터 상속 구조

```cpp
// Public/Core/CL9ActorBase.h
class ACL9ActorBase : public AActor {
public:
  ACL9ActorBase();
  
  // === 네트워크 ===
  virtual void GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
  ) const override;
  
  // === 데이터 ===
  // ReplicatedUsing이어야 OnRep이 호출된다 (Replicated만으로는 콜백이 안 붙는다)
  UPROPERTY(ReplicatedUsing=OnRep_AliasPair, EditAnywhere, BlueprintReadOnly)
  FName Alias;   // DataAsset 식별자

  UPROPERTY(ReplicatedUsing=OnRep_AliasPair, EditAnywhere, BlueprintReadOnly)
  FName SubAlias;  // 상태 변형 (egg+cooked 등 — 요리·깨짐이 이걸 교체한다)

  // === 종류 ===
  ECL9ActorKind GetKind() const;   // Item/Monster — 자식 클래스 생성자가 Kind를 정한다 (클래스마다 고정이라 복제 안 함)
  
  // === 사운드 ===
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  class UAudioComponent* AudioComponent;
  
  // === RPC ===
  // ⚠️ Server RPC를 이 베이스에 두지 않는다 — 아이템·몬스터는 어느 클라의 소유도
  // 아니라서, 클라가 비소유 액터의 Server RPC를 부르면 조용히 드롭된다.
  // 클라발 요청은 전부 소유 액터(Character/PlayerController)의 Server RPC로 올리고,
  // 호스트가 이 액터의 함수를 직접 호출한다 (input.md 상호작용 틀).
  UFUNCTION(NetMulticast, Unreliable)   // 호스트→전 클라 표현은 OK
  virtual void Multicast_PlaySoundAtLocation(
    const FName& SoundEventName,
    FVector Location
  );
  
  // === 편의 함수 ===
  UFUNCTION(BlueprintCallable, Category="CL9|Audio")
  void PlaySFX(const FName& SoundEventName);
  
  void PlaySFX3D(const FName& SoundEventName, FVector Location);
  
protected:
  // 데이터 로드 (자식에서 override)
  virtual void LoadData();
  
  // 네트워크 초기화
  virtual void BeginPlay() override;
};

// === 자식 클래스들 ===
class ACL9Monster : public ACL9ActorBase { };
class ACL9Item : public ACL9ActorBase { };

// 예외: 캐릭터는 CMC 예측 이동이 필요해 ACharacter를 상속한다 (A4 결정)
class ACL9Character : public ACharacter { };
```

## RPC 패턴 — 소유권 규칙

**Server RPC는 호출 클라가 소유한 액터에서만 전송된다.** 아이템·몬스터는 비소유라
클라→서버 요청 경로는 항상 이렇다:

```
클라 입력 → (소유 액터) ACL9Character의 Server RPC → 호스트 판정
                                  ↓
                    호스트가 대상 액터의 일반 함수 호출 (Item->OnPickedUp() 등)
                                  ↓
                    결과는 Replicated 상태 / Multicast로 전 클라 표현
```

**사운드 재생 (호스트 → 전 클라, 표현):**
```cpp
// 호스트 판정 코드 안에서만
void ACL9Monster::Die() {
  if (!HasAuthority()) return;
  Multicast_PlaySoundAtLocation("MonsterDeath", GetActorLocation());  // Unreliable
}
```

## Alias 설정 & 동기화 (데이터 주도)

**원칙:** Alias는 **데이터에서 읽어오기** (하드코딩 X). 로드 진입점은 둘이다 —
**BeginPlay(초기값)와 OnRep(런타임 변경)**.

```cpp
void GetLifetimeReplicatedProps(...) const override {
  DOREPLIFETIME(ACL9ActorBase, Alias);
  DOREPLIFETIME(ACL9ActorBase, SubAlias);
}

// 초기 로드 — 레벨 배치 액터는 클라도 맵에서 같은 값을 이미 갖고 있어
// 복제 변화가 없고 OnRep이 발화하지 않는다. BeginPlay가 초기값을 책임진다.
void BeginPlay() override {
  Super::BeginPlay();
  if (!Alias.IsNone()) LoadData();
}

// 런타임 변경 전용 (스폰 후 SetAlias, 요리·깨짐의 SubAlias 교체)
UFUNCTION()
void OnRep_AliasPair() { LoadData(); }

// 서버: 설정 (클라에 자동 동기화)
void SetAlias(FName InAlias, FName InSubAlias = NAME_None) {
  if (HasAuthority()) {
    Alias = InAlias; SubAlias = InSubAlias;  // 변경된 클라에서 OnRep 발화
    LoadData();                              // 호스트는 직접 로드
  }
}
```

---

## 사용 예 (데이터 주도)

### 1️⃣ 레벨에 배치된 액터들 초기화 (GameMode)

```cpp
void ACL9GameMode::BeginPlay() {
  Super::BeginPlay();
  
  // 레벨의 모든 Item 액터 찾기
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(
    GetWorld(),
    ACL9Item::StaticClass(),
    FoundActors
  );
  
  // 레벨 배치 액터는 별도 처리가 필요 없다 — 에디터에서 설정된 Alias를
  // 각 머신(호스트·클라 모두)이 자기 BeginPlay에서 LoadData()로 읽는다.
  // OnRep은 여기서 발화하지 않는다 (초기값이 같아서 복제 변화가 없다).
}
```

### 2️⃣ 서버에서 액터 스폰 + 데이터 설정

```cpp
// 게임모드 또는 게임 시스템에서
void ACL9GameMode::SpawnItem(const FName& ItemAlias, const FVector& Location) {
  if (!HasAuthority()) return;
  
  ACL9Item* Item = GetWorld()->SpawnActor<ACL9Item>(Location);
  Item->SetAlias(ItemAlias);  // 서버: Alias 설정
  
  // 자동 동기화:
  // 서버: SetAlias("egg")
  // ↓
  // 각 클라: BeginPlay(초기 스폰) 또는 OnRep_AliasPair(런타임 변경) 호출
  // ↓
  // 클라: LoadData() → ItemDef 로드 → Mesh/Icon 표시
}
```

### 3️⃣ 목적지 설정에서 액터들 초기화 (권장)

```cpp
void ACL9GameMode::InitializeNight(int32 Day) {
  const UCL9SiteDef* Site = DefRegistry->FindSite(CurrentSiteAlias);
  
  // 목적지 기준값 × 일자 곡선으로 개수를 정하고 (balance.md), 시드 좌표에 스폰
  for (const FCL9SpawnEntry& Entry : Site->ItemSpawns) {
    ACL9Item* Item = GetWorld()->SpawnActor<ACL9Item>();
    Item->SetAlias(Entry.Alias);  // 데이터 설정에서 읽은 Alias
  }
  
  for (const FCL9SpawnEntry& Entry : Site->MonsterSpawns) {
    ACL9Monster* Monster = GetWorld()->SpawnActor<ACL9Monster>();
    Monster->SetAlias(Entry.Alias);  // 마찬가지
  }
}
```

---

## 네트워크 흐름

```
[스폰 액터]
서버: SetAlias("egg") → LoadData()      클라: Alias 복제 수신 → OnRep_AliasPair()
                                              → LoadData() → Mesh·Icon 표시

[레벨 배치 액터]
모든 머신: BeginPlay() → Alias 있음 → LoadData()   (OnRep 발화 없음 — 값 변화가 없어서)
```

---

## 네트워크 체크리스트

- ✅ Alias·SubAlias는 `ReplicatedUsing` + `DOREPLIFETIME` 등록 — 하나라도 빠지면 클라 로드가 죽는다
- ✅ 초기 로드는 BeginPlay, 런타임 변경은 OnRep — 두 진입점 모두 필요
- ✅ SetAlias()는 **서버에서만** 호출
- ✅ 클라발 요청은 소유 액터(Character/PC)의 Server RPC 경유 — 비소유 액터에 Server RPC 금지
- ✅ 사운드·연출은 호스트발 Multicast(Unreliable)로

## 참고

- Alias 하드코딩 금지 (모두 데이터 기반)
- 모든 사운드 로직은 베이스에서 처리 — 사운드 상세는 `sound.md` 참고
- 네트워크 테스트: 호스트 + 클라 1명 이상
- 데이터 구조 정의는 `../data/schema.md` 참고

## 액터 종류와 시설물 (2026-09-07)

`ECL9ActorKind`는 `None / Item / Monster / Facility` 넷이다. **값은 뒤에만 붙인다** — 앞을 밀면 이미
저장된 표 값(`WBP_Scan.ScanKinds` 등)이 다른 뜻이 된다.

- **`ACL9Fixture`**(Source/CL9/World) — 레벨에 붙박이로 서 있는 것들의 공통 베이스: 조리대·납품 창구·
  트럭 패널·반납 항아리. `ACL9ActorBase`를 상속하는 이유는 **스캔이 이들을 보게 하려는 것**이다
  (스캔은 `ACL9ActorBase`를 훑고 `Kind`로 고른다 — 2026-09-07 사용자 요청). 넷이 각자 들고 있던
  `MeshComponent`도 여기로 합쳤다. `Alias` 칸이 함께 딸려 오지만 지금은 비워 둔다 — 시설물 표가
  생기면 그 자리에 그대로 쓴다.
- **`GetDisplayName()`(virtual)** — 화면에 뜰 이름을 액터가 스스로 답한다. 아이템·몬스터는 행의
  `DisplayName`, 시설물은 자기 프로퍼티, 그 외에는 액터 이름. **UI가 하위 클래스를 캐스팅해 갈래를
  타지 않게** 하려는 것이다 — 종류가 늘어도 화면 코드는 그대로다(스캔 라벨이 이걸 쓴다).
