# 아이템 공통 시스템 (ACL9Item)

> 모든 아이템이 공유하는 액터·데이터·상태 틀. 개별 시스템은 이 폴더의
> [inventory.md](inventory.md)(운반) · [drop.md](drop.md)(드롭), 값의 정본은
> [../plan/data/item/common.md](../plan/data/item/common.md).

## 액터 틀

- `ACL9Item : ACL9ActorBase` — 바탕 액터. `SetAlias(+SubAlias)` → Replicated → OnRep →
  `LoadData()`로 메시·물리 값을 표에서 읽는다 ([actor-base.md](actor-base.md)).
  **행동이 같은 아이템은 클래스 하나를 공유**하고(`ActorClass`를 비우면 공용 `BP_CL9Item`),
  행동이 다른 종류만 파생한다 — `ACL9LightItem`(빛) → `ACL9FlashItem`(손전등·헤드램프),
  `ACL9Cart`(수레). 2026-09-11 "item - lightitem - flashitem" 결정. 파생 클래스는 `OnAcquired`/
  `OnReleased` 훅으로 소지 변화에 반응하고, 행은 `ActorClass`로 그 클래스를 가리킨다.
- 손에 들면 `OnPickedUp`이 소지자 전신 메시의 손 소켓 `HandGrip_R`에 액터를 붙이고 행의
  `GripOffset`/`GripRotation`을 준다; 소유자 화면에서는 메시 컴포넌트만 1인칭 팔 소켓으로 옮겨
  그린다. 팔 자세는 행의 `HoldPose` 범주로 ABP가 고른다 (2026-09-12) — 부착·자세 규칙의 정본은
  [inventory.md](inventory.md), ABP 배선은 [animation.md](animation.md).
- **핫바 칸에 넣으면(비활성) 액터는 파괴되고 데이터만 남는다** — 꺼낼 때 `ACL9GameMode::SpawnItem(…, PresetValue)`
  로 다시 스폰하고 `HostSetValue`로 굴린 값을 되돌린다(칸 전환은 가격을 다시 굴리지 않는다, 2026-09-13).
  행이 바뀌면 다시 굴리는 규칙(아래 "상태 = SubAlias")은 그대로다.
- **몸체는 보통 반경 구(`Radius`)이고 메시는 그 구에 맞춰 자동 스케일된다.** 행의 `bMeshIsBody`가 켜진 물건(수레)은
  이 분기를 타지 않는다 — 로드 때 메시가 루트이자 물리 몸체가 되고(임포트한 UCX 심플 콜리전, 실물 크기, 구는 콜리전
  없이 메시 아래로), `Mass`를 질량으로, 행의 `Friction`·`Restitution`을 물리 재질로 준다. 이런 물건은 손에 들리지 않는다.
  힘·깨우기·시뮬 on/off는 전부 `ACL9Item::GetBody()`(= 루트 프리미티브)를 거친다 (2026-09-13 수레 물리).
- 물리 시뮬레이션(구르기·낙하·충돌)은 호스트가 권위 — 클라는 복제된 물리 상태를 보간한다. **클라 코드는 시뮬레이션을
  켜고 끄지 않는다**: 엔진이 복제된 `bRepPhysics`에 맞춰 켜고 끈다(`SyncReplicatedPhysicsSimulation`). 클라가 꺼 버리면
  복제 목표가 시뮬레이션하지 않는 바디에 버려져 아이템이 얼어붙는다(엔진 `FPhysicsReplication::ApplyRigidBodyState`).
  루트를 메시로 바꾼 직후에도 같은 동기화를 한 번 더 맞춘다.
- 깨짐 판정: 충격 속도 vs `CrackSpeed`/`BreakSpeed` — 호스트 전용, 결과는 SubAlias/상태
  Replicated. 수레 짐칸 안이라고 예외가 없다.

## 상태 = SubAlias

상태 변형(익음·탐·금감·깨짐)은 새 액터가 아니라 **Alias+SubAlias 교체**다:

```
egg → egg+cooked → egg+burnt      (열원 위 시간 — cooking.md)
성함 → 금감 → 깨짐                 (충격 — 물리 판정)
```

교체는 호스트 전용, OnRep으로 전 클라 메시·표현 갱신. 새 상태가 필요한 시스템은 SubAlias를
추가하지 별도 상태 변수를 만들지 않는다.

## 수명

- 스폰: 밤 시작 배치(raiding.md) · 몬스터 드롭([drop.md](drop.md)) · 상점 구매(shop.md)
- 소멸: 밤 종료 시 맵과 함께 — 개별 소멸 타이머 없음. **정리 주체**: 복제 액터(아이템·몬스터)는
  호스트가 Destroy(복제로 전파), 각 머신이 로컬 생성한 지형은 GameState 페이즈 OnRep에 물린
  **로컬 정리 패스**가 철거한다 ([map.md](map.md)).
- 장비 보존: 밤 종료 시 **트럭 존 안의 실물만** OwnedEquipment로 남는다 ([save.md](save.md))
- 검증: Broken 행은 스폰에 안 섞인다 ([../data/schema.md](../data/schema.md) EDefStatus)

## 참고

- 운반(핫바·수레): [inventory.md](inventory.md)
- 드롭: [drop.md](drop.md)
- 요리 상태 전이: [cooking.md](cooking.md)
- 조명(발광·손전등): [light.md](light.md)
