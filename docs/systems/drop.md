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

전부 `ACL9Monster::Die(Source)` 안에서 호스트가 한 번에 한다(`Source/CL9/Monsters/CL9Monster.cpp`) — 별도의
`DropLoot`/`SpawnDropItem` 함수도, 물리 임펄스도, 죽음 멀티캐스트도 없다.

### 1. 몬스터 사망 (액터 자신의 로직 — K1 규약)

```
ACL9Monster::Die(AActor* Source)            // 호스트만 (HasAuthority 관문)
  ├─ 처형 중이면 피해자 HostKill 먼저          // monster.md 스토킹과 처형 D4
  ├─ DeathLaunchVelocity 계산                 // 죽음 발사 — 타격 원점 반대편 (monster.md 타격의 표현)
  ├─ DropItems → GameMode->SpawnItem + OnThrown   // 드랍 호 (아래 2)
  ├─ 사살 경보 — 들은 개체 ReceiveKillAlarm(RageTime)
  └─ EnterState(Dead)                         // 죽는 클립·소리·플래시는 복제된 State·StateStartServerTime 에서 각 머신이 파생
```

사살은 공짜가 아니다 — `KillAlarmRadius`(2100cm) 사살 경보가 퍼진다 (층/존 분리는 맵 존
세팅과 함께 — [monster.md](monster.md)).

### 2. 드롭 판정·스폰·발사 (코드 요약 — `Die` 안)

```cpp
const float LootSpeed = Rules ? Rules->DeathLootSpeed : 0.f;     // CL9Rules 열 (초안 300)
for (const FDropItem& Drop : MonsterDef->DropItems) {
  if (FMath::FRand() > Drop.DropChance) continue;
  const int32 Quantity = FMath::RandRange(Drop.MinQuantity, Drop.MaxQuantity);
  for (int32 i = 0; i < Quantity; ++i) {
    // 몬스터 중심 높이, 옆으로 30~90cm 무작위 — 둘 이상이 30cm 넘게 떨어지게 하는 보장(호만으로는 3/4 확률)
    const FVector Origin = GetActorLocation() + 무작위 요 수평 단위벡터 * FMath::FRandRange(30.f, 90.f);
    ACL9Item* Item = GameMode->SpawnItem(Drop.ItemAlias, Drop.ItemSubAlias, FTransform(Origin));  // SetAlias → 복제 → OnRep 로드
    if (Item && LootSpeed > 0.f) {
      const FRotator Lob(FMath::FRandRange(45.f, 70.f) /*앙각*/, FMath::FRandRange(0.f, 360.f) /*요*/, 0.f);
      Item->OnThrown(Origin, Lob.Vector() * LootSpeed);  // 플레이어 던지기와 같은 함수 — 이후는 아이템 물리
    }
  }
}
```

- 스폰은 `ACL9GameMode::SpawnItem`(치트 `cl9.additem`·칸에서 꺼내기와 같은 경로), 발사는 `ACL9Item::OnThrown`
  (던지기와 같은 함수) — 드랍 전용 코드 경로가 없다.
- `DeathLootSpeed` 300은 계란 표 중력 1950·앙각 70°에서 착지 371 < `CrackSpeed` 450이 되게 잡은 값
  (decisions.md 2026-09-18 "죽음 발사"). 드랍 호의 규칙 행은 [monster.md](monster.md) "타격의 표현" 표.
- 떨어진 아이템은 여느 실물 아이템과 같다 — 깨질 수 있는 것은 깨진다 (`bFragile` 물리 규칙).
- **수집은 운반 시스템의 줍기다.** 접근 자동 흡수·수집 반경·타이머 소멸이 없다.
- 두고 온 것은 밤이 끝나면 맵과 함께 사라진다 — 별도 소멸 타이머를 두지 않는다.

## 네트워크 동기화

- **드롭 판정·스폰·발사 초기 속도**: 호스트에서만 (`Die` 진입부 `HasAuthority` 관문)
- **아이템 생성**: 표준 액터 복제 (SetAlias → OnRep) — 별도 Multicast 불필요. 이후 궤적은 아이템 물리 복제
- **죽는 모션·소리·플래시·죽음 발사**: Multicast RPC 없음 — 복제된 `State`/`StateStartServerTime`(발사는 트랜스폼 복제)에서
  각 머신이 파생한다 ([monster.md](monster.md) 타격의 표현 · [sound.md](sound.md))

## UI 표시

화면 좌하단 활동 로그 정도만 — "+ 고기 1" (줍기 성공 시, 운반 시스템의 표현).
드롭 아이템의 미니맵 표시 여부는 개체별 값으로 제어한다 (`bShowOnMinimap`류 — [ui.md](ui.md)).

## 참고

- 몬스터 데이터·드롭 연결: [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 아이템 물리·값: [../plan/data/item/common.md](../plan/data/item/common.md)
- 줍기·운반: [inventory.md](inventory.md)
