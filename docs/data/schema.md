# 데이터 스키마 (DataAsset 구조)

> **표별 컬럼 정의의 정본은 plan이다** — 이 문서는 컬럼 목록을 복사하지 않는다
> (두 벌 관리가 2026-08-31 충돌 보고의 원인이었다). 여기에는 베이스 클래스 · 공용 구조체 ·
> 에셋 생성 규칙만 둔다.

## 표별 정의 (정본 포인터)

| 클래스 | 정본 문서 | 파일 위치 |
|---|---|---|
| `UCL9ItemDef` | [../plan/data/item/common.md](../plan/data/item/common.md) (개별: egg·meat·cart·flashlight) | `Content/Data/ItemTable` (행 인라인) |
| `UCL9MonsterDef` | [../plan/data/monster/common.md](../plan/data/monster/common.md) (개별: chicken·goose·raccoon) | `Content/Data/MonsterTable` (행 인라인) |
| `UCL9CharacterDef` | [../plan/data/character.md](../plan/data/character.md) | `Content/Data/CharacterTable` (행 인라인) |
| `UCL9SiteDef` | [../plan/data/site.md](../plan/data/site.md) | `Content/Data/MapTable` (행 인라인) |
| `UCL9OrderDef` | [../plan/data/order.md](../plan/data/order.md) | `Content/Data/OrderTable` (행 인라인) |
| `UCL9EquipmentDef` | [../plan/data/equipment.md](../plan/data/equipment.md) | `Content/Data/EquipmentTable` (행 인라인) |
| `UCL9SequenceDef` | [../plan/data/sequence.md](../plan/data/sequence.md) | `Content/Data/SequenceTable` (미구현) |
| `CL9Rules` (단일) | [../plan/data/rules.md](../plan/data/rules.md) | `Content/Data/CL9Rules` |
| `CL9Progression` (단일) | [../plan/data/progression.md](../plan/data/progression.md) | `Content/Data/CL9Progression` |

이전 판에 있던 별도 스키마(계란말이 레시피 표 · 스테이지 밸런스 표 · HP 100/속도 600 캐릭터
표 · 늑대 몬스터 표)는 plan과 충돌해 폐기했다 (decisions.md 2026-08-31). 요리는 레시피가
아니라 아이템의 `CookResult` 체인이고, 밸런스는 스테이지 표가 아니라 곡선 × 기준값이다.

---

## 0. 베이스 클래스 (모든 데이터의 부모)

**클래스**: `UCL9DefBase : UPrimaryDataAsset`
(정본: [../plan/data/README.md](../plan/data/README.md) 공통 뼈대 — 2026-08-31 A3 결정)

모든 게임 데이터는 이 클래스를 상속한다. **조회 키는 `Alias` + `SubAlias`다** — 파일 이름이 아니다.

⚠️ **Mesh는 아이템용 (StaticMesh)** — 캐릭터/몬스터는 각 클래스에서 SkeletalMesh를 따로 정의

| 컬럼 | 타입 | 필수 | 설명 | 예시 |
|------|------|------|------|------|
| `Alias` | FName | ✅ | 식별자 (조회 키) | `egg`, `chicken`, `henhouse` |
| `SubAlias` | FName | | 하위 분류 (선택) | `gold`, `cooked`, `burnt` |
| `DisplayName` | FText | ✅ | 화면 표시명 — **String Table `CL9` 참조만** (직접 친 텍스트는 Broken, [../systems/localization.md](../systems/localization.md)) | `Item.egg.Name`, `Monster.chicken.Name` |
| `Description` | FText | | 상세 설명 — DisplayName과 같은 규칙 | `Item.egg.Desc` |
| `Icon` | TSoftObjectPtr<UTexture2D> | ✅ | UI 아이콘 | `/Game/UI/Icons/Item_Egg` |
| `Mesh` | TSoftObjectPtr<UStaticMesh> | | 월드 메시 **(아이템만)** | `/Game/Meshes/Items/Egg` |
| `ActorClass` | TSoftClassPtr<AActor> | | 전용 BP. **비우면 종류별 공용 BP** | (대부분 비움) |
| `Gimmicks` | TArray<TObjectPtr<UCL9Gimmick>> (Instanced) | | 이 종류에만 있는 특수 동작 | 깨짐, 발광 |

```cpp
// 베이스 클래스 (모든 데이터의 부모)
// UPrimaryDataAsset이어야 Instanced 기믹이 동작한다 — DataTable 행(구조체)에서는 불가
UCLASS()
class UCL9DefBase : public UPrimaryDataAsset {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
  FName Alias;
  
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
  FName SubAlias;
  
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
  FText DisplayName;
  
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
  FText Description;
  
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
  TSoftObjectPtr<UTexture2D> Icon;
  
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
  TSoftObjectPtr<UStaticMesh> Mesh;
  
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
  TSoftClassPtr<AActor> ActorClass;  // 비우면 종류별 공용 BP
  
  UPROPERTY(EditAnywhere, Instanced, Category="Data")
  TArray<TObjectPtr<UCL9Gimmick>> Gimmicks;
};
```

**SubAlias 사용 예** — 상태 변형은 별개 Alias가 아니라 SubAlias로 표현한다:

```
계란            Alias: egg                       (ItemTable의 행)
익은 계란       Alias: egg,  SubAlias: cooked    (같은 표의 다른 행)
탄 계란         Alias: egg,  SubAlias: burnt     (〃)
```

---

## 공용 구조체

### FCL9ItemRef — 아이템 참조 (Alias + SubAlias 쌍)

수량 없이 "어떤 아이템의 어떤 상태"만 가리킬 때 — 요리 전이 `CookResult`/`BurnResult`가
쓴다 (plan/data/item/common.md · systems/cooking.md의 `.ItemAlias`/`.ItemSubAlias`).

```cpp
USTRUCT()
struct FCL9ItemRef {
  UPROPERTY(EditAnywhere)
  FName ItemAlias;        // item/ 표에 있는 것만 참조 (로드 검사가 확인)

  UPROPERTY(EditAnywhere)
  FName ItemSubAlias;     // 상태 지정 (예: cooked) — None = 기본 상태
};
```

### FCL9ItemCount — 아이템 참조 + 수량 (상속: FCL9ItemRef)

주문 재료(`Ingredients`)와 드롭의 공통 베이스.

```cpp
USTRUCT()
struct FCL9ItemCount : public FCL9ItemRef {
  UPROPERTY(EditAnywhere)
  int32 Count = 1;
};
```

### FDropItem — 몬스터 드롭 (상속: FCL9ItemCount)

```cpp
USTRUCT()
struct FDropItem : public FCL9ItemCount {
  // 수량은 MinQuantity~MaxQuantity 굴림만 쓴다 — 베이스의 Count는 드롭에서 무시된다
  UPROPERTY(EditAnywhere)
  float DropChance = 1.0f;     // 0.0 ~ 1.0

  UPROPERTY(EditAnywhere)
  int32 MinQuantity = 1;

  UPROPERTY(EditAnywhere)
  int32 MaxQuantity = 1;
};
```

### FCL9SpawnEntry — 스폰 기준값 (site.md)

```cpp
USTRUCT()
struct FCL9SpawnEntry {
  UPROPERTY(EditAnywhere)
  FName Alias;             // MonsterAlias 또는 ItemAlias

  UPROPERTY(EditAnywhere)
  int32 MinCount = 0;

  UPROPERTY(EditAnywhere)
  int32 MaxCount = 0;      // 여기에 일자 곡선 배율이 곱해진다 (progression.md)
};
```

### FCL9StatEffect — 먹기 효과 (plan/data/item/common.md)

```cpp
USTRUCT()
struct FCL9StatEffect {
  UPROPERTY(EditAnywhere)
  ECL9Stat Stat;           // 9종 — character.md의 스탯 enum (캡슐·눈높이·HP 없음)

  UPROPERTY(EditAnywhere)
  float Amount = 0.f;

  UPROPERTY(EditAnywhere)
  float DurationSec = 0.f; // 0 = 즉시 1회
};
```

### FSoundEffect — 사운드 정의

```cpp
USTRUCT()
struct FSoundEffect {
  UPROPERTY(EditAnywhere)
  TSoftObjectPtr<USoundBase> Sound;  // 소프트 참조

  UPROPERTY(EditAnywhere)
  float Volume = 1.0f;

  UPROPERTY(EditAnywhere)
  float Pitch = 1.0f;
};
```

---

## 조회 — UCL9DefRegistry

```cpp
UCL9DefRegistry              // 게임 인스턴스가 들고 있는다
  FindItem(Alias, SubAlias)      → UCL9ItemDef*
  FindMonster(Alias, SubAlias)   → UCL9MonsterDef*
  FindSite / FindOrder / FindEquipment / FindCharacter / FindSequence
```

에셋 목록은 전용 목록 에셋에 등록한다. **코드에 에셋 경로를 쓰지 않는다** — 등록된 목록을
순회한다 ([../plan/data/README.md](../plan/data/README.md)).

---

## 데이터 에셋 생성 규칙

1. **파일명**: 종류마다 표 하나 — `ItemTable.uasset`·`MonsterTable.uasset`·`MapTable.uasset`처럼 **접두사 없이**
   (2026-09-12 사용자 — "DA 붙는 거 필요없음"). 행은 표 안에 인라인으로 산다(2026-09-02 결정)
2. **Alias는 경로가 아니라 이름이다** — 파일을 옮기거나 이름을 바꿔도 데이터가 안 깨진다
   ```cpp
   FindItem("ItemTable:CL9ItemDef_0")   // ❌ 경로명
   FindItem("egg")           // ✅ Alias
   ```
3. **소프트 참조 사용** (Icon, Mesh, Sound) — 하드 참조는 표를 여는 순간 모든 에셋을 올린다
4. **화폐 값은 int** (이더 — [../plan/economy.md](../plan/economy.md)). float 가격 컬럼 금지
5. **곡선 값은 `UCurveFloat`** — 일자별 배율은 표가 아니라 곡선 에셋으로

## 로드 검사

에셋 로드 시 관계 검사가 돈다 — 깨지면 어느 줄인지 로그에 찍는다:

- 참조 무결성: `DropItems`·`Ingredients`·`CookResult`가 가리키는 Alias(+SubAlias)가 실제로 존재하는가
- 부등식 목록: [../plan/economy.md](../plan/economy.md) "지켜야 하는 부등식"
  (A1·A2·A4는 캐릭터 행마다)
- 멀티 접속 시 `CL9TuningHash`로 호스트·클라 값 대조

### 사용 가능/불가 구분 — EDefStatus

검사를 통과 못 한 행은 조용히 섞이지 않는다. 레지스트리가 행마다 상태를 나눠 들고 있는다:

| 상태 | 조건 | 동작 |
|---|---|---|
| **Usable** | 필수 필드가 차 있고 참조가 전부 해석됨 | `Find*`가 정상 반환 — 게임에서 쓴다 |
| **Broken** | 필수 필드 누락·깨진 참조(`CookResult`가 없는 Alias/SubAlias) | 로드 시 **어느 행·어느 컬럼**인지 로그. `Find*`는 nullptr — 스폰·주문 추첨·상점 목록 어디에도 안 섞인다 |

**필수 필드 집합은 Def 클래스별로 정의한다** (가상 함수) — 베이스의 ✅가 전 클래스 공통이
아니다. 예: `UCL9SequenceDef`는 Icon·Mesh가 없어도 Usable, `UCL9ItemDef`는 Icon 필수.

- `cl9.list`가 상태를 같이 표시한다 — `3: meat ✗ (Icon 미지정)`. 에디터에서 뭘 고쳐야
  하는지 콘솔에서 바로 보인다 ([../systems/cheats.md](../systems/cheats.md)).
- 소프트 참조는 로드 시 **경로가 해석되는지만** 검사하고, 실제 에셋 로드는 사용 시점이다.
  런타임 로드 실패(에셋 삭제 등)는 개발 빌드에서 화면 로그 + 대체 표시(엔진 기본 메시)로
  드러낸다 — **조용한 투명 액터를 만들지 않는다.** 시퀀스의 같은 규칙은
  [../systems/sequence.md](../systems/sequence.md) 경계 상황 절.

## 참고

- 왜 DataTable이 아니라 DataAsset인가: [../plan/data/README.md](../plan/data/README.md)
- 값이 서로 엮이는 방식: [../plan/economy.md](../plan/economy.md)
