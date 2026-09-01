# 아이템 드롭 시스템

> 몬스터를 잡았을 때 아이템이 떨어지는 시스템.
> **사냥의 보상은 이더 직접 지급이 아니라 `DropItems`로 떨어지는 아이템이다**
> ([../plan/data/monster/common.md](../plan/data/monster/common.md)).

## 개요

- **드롭 규칙**: 몬스터가 죽을 때 표에 연결된 아이템을 떨군다
- **드롭 확률·수량**: 몬스터별 `DropItems` 배열 (`FDropItem`)
- **떨어진 것은 실물이다**: 여느 아이템과 똑같이 줍는다 ([inventory.md](inventory.md)) —
  자석 효과·자동 수집·소멸 타이머를 만들지 않는다
- **돈은 안 떨어진다**: `Money`/`Gold_Coin` 드롭 금지 — 이더는 판매·납품에서만 생긴다

## 데이터 구조

```cpp
// 몬스터 표 (UCL9MonsterDef — plan/data/monster/common.md)
TArray<FDropItem> DropItems;   // 어떤 몬스터가 뭘 떨구는지는 개발자가 표에서 직접 연결

struct FDropItem : FCL9ItemCount {
  // ItemAlias — 베이스에서 상속 (item 표에 있는 것만 참조 — ../../data/schema.md)
  float DropChance = 1.0f;     // 0.0 ~ 1.0
  int32 MinQuantity = 1;
  int32 MaxQuantity = 1;
};
```

닭의 드롭 연결은 **미정**이다 ([../plan/data/monster/chicken.md](../plan/data/monster/chicken.md)) —
값은 표에서 정한다.

## 드롭 흐름

### 1. 몬스터 사망 (액터 자신의 로직 — K1 규약)

```cpp
// ACL9Monster::Die()
if (!HasAuthority()) return;  // 호스트만

DropLoot();

// 사망 연출(죽는 모션·래그돌·사체 유지/제거)은 종별 표현 선택이다 — 원칙 없음.
Multicast_OnDied();  // 죽는 모션·소리 (표현)
```

사살은 공짜가 아니다 — `KillAlarmRadius`(2100cm) 사살 경보가 퍼진다 (층/존 분리는 맵 존
세팅과 함께 — [monster.md](monster.md)).

### 2. 드롭 판정

```cpp
void ACL9Monster::DropLoot() {
  const UCL9MonsterDef* Def = GetMonsterDef();

  for (const FDropItem& Drop : Def->DropItems) {
    if (FMath::FRand() < Drop.DropChance) {
      int32 Quantity = FMath::RandRange(Drop.MinQuantity, Drop.MaxQuantity);
      SpawnDropItem(Drop.ItemAlias, Quantity, GetActorLocation());
    }
  }
}
```

### 3. 드롭 아이템 생성 (실물 스폰)

```cpp
void ACL9Monster::SpawnDropItem(FName ItemAlias, int32 Quantity, FVector Location) {
  for (int32 i = 0; i < Quantity; ++i) {
    ACL9Item* Item = GetWorld()->SpawnActor<ACL9Item>();
    Item->SetAlias(ItemAlias);   // Replicated → 전 클라 OnRep 로드
    Item->SetActorLocation(Location + FVector(0, 0, 50));

    // 물리 활성화 (구르기) — UPrimitiveComponent::AddImpulse 사용
    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Item->GetRootComponent())) {
      FVector ImpulseDirection = FVector(
        FMath::RandRange(-200, 200),
        FMath::RandRange(-200, 200),
        100
      ).GetSafeNormal();
      RootPrim->AddImpulse(ImpulseDirection * 500.0f);
    }
  }
}
```

- 떨어진 아이템은 여느 실물 아이템과 같다 — 깨질 수 있는 것은 깨진다 (`bFragile` 물리 규칙).
- **수집은 운반 시스템의 줍기다.** 접근 자동 흡수·수집 반경·타이머 소멸이 없다.
- 두고 온 것은 밤이 끝나면 맵과 함께 사라진다 — 별도 소멸 타이머를 두지 않는다.

## 네트워크 동기화

- **드롭 판정**: 호스트에서만 (`if (!HasAuthority()) return;`)
- **아이템 생성**: 표준 액터 복제 (SetAlias → OnRep) — 별도 Multicast 불필요
- **죽는 모션·소리**: Multicast RPC (표현 전용)

## UI 표시

화면 좌하단 활동 로그 정도만 — "+ 고기 1" (줍기 성공 시, 운반 시스템의 표현).
드롭 아이템의 미니맵 표시 여부는 개체별 값으로 제어한다 (`bShowOnMinimap`류 — [ui.md](ui.md)).

## 참고

- 몬스터 데이터·드롭 연결: [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 아이템 물리·값: [../plan/data/item/common.md](../plan/data/item/common.md)
- 줍기·운반: [inventory.md](inventory.md)
