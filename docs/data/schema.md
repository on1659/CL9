# 데이터 스키마 색인 — 키워드로 정본 찾기

> 이 문서는 **색인**이다. 코드·로그·에디터에서 본 이름(표 이름, 클래스, 구조체, 컬럼)을 여기서 찾아
> 정본 문서와 헤더로 건너간다. 컬럼 정의·값·이유는 여기 적지 않는다 — 두 벌 관리가 2026-08-31 충돌
> 보고의 원인이었다(decisions.md). 정본이 바뀌면 여기는 **이름과 링크만** 맞춘다.

## 표 (Content/Data)

| 에셋 | 행 클래스 | 기획 정본 | 구현 정본 | 헤더 |
|---|---|---|---|---|
| `ItemTable` | `UCL9ItemDef` | [plan/data/item/common.md](../plan/data/item/common.md) (개별: [egg](../plan/data/item/egg.md) · [meat](../plan/data/item/meat.md) · [cart](../plan/data/item/cart.md) · [flashlight](../plan/data/item/flashlight.md) · [headlamp](../plan/data/item/headlamp.md)) | [systems/item.md](../systems/item.md) · [inventory.md](../systems/inventory.md) · [cooking.md](../systems/cooking.md) · [light.md](../systems/light.md) | `Source/CL9/Data/CL9ItemDef.h` |
| `MonsterTable` | `UCL9MonsterDef` | [plan/data/monster/common.md](../plan/data/monster/common.md) (개별: chicken · goose · pig · raccoon · sheep) | [systems/monster.md](../systems/monster.md) · [drop.md](../systems/drop.md) · [animation.md](../systems/animation.md) | `CL9MonsterDef.h` |
| `CharacterTable` | `UCL9CharacterDef` | [plan/data/character.md](../plan/data/character.md) | [systems/character.md](../systems/character.md) | `CL9CharacterDef.h` |
| `MapTable` (←DA_SiteTable) | `UCL9SiteDef` | [plan/data/site.md](../plan/data/site.md) | [systems/map.md](../systems/map.md) · [light.md](../systems/light.md)(조명 3상) | `CL9SiteDef.h` |
| `ModuleTable` | `UCL9ModuleDef` | [plan/data/module.md](../plan/data/module.md) | [systems/map.md](../systems/map.md) · [asset-swap.md](../systems/asset-swap.md)(소품 세트) | `CL9ModuleDef.h` |
| `PieceTable` | `UCL9PieceDef` | [plan/data/segkind.md](../plan/data/segkind.md) | [systems/asset-swap.md](../systems/asset-swap.md) | `CL9PieceDef.h` |
| `SegKindTable` | `UCL9SegKindDef` | [plan/data/segkind.md](../plan/data/segkind.md) | [systems/asset-swap.md](../systems/asset-swap.md) · [map.md](../systems/map.md) | `CL9SegKindDef.h` |
| `OrderTable` | `UCL9OrderDef` | [plan/data/order.md](../plan/data/order.md) | [systems/order.md](../systems/order.md) | `CL9OrderDef.h` |
| `EquipmentTable` | `UCL9EquipmentDef` | [plan/data/equipment.md](../plan/data/equipment.md) | [systems/shop.md](../systems/shop.md) · [economy.md](../systems/economy.md) | `CL9EquipmentDef.h` |
| `BuffTable` | `UCL9BuffDef` | [plan/data/buff.md](../plan/data/buff.md) | [systems/item.md](../systems/item.md)(먹기) | `CL9BuffDef.h` |
| `CL9Rules` (단일) | `UCL9RulesDef` | [plan/data/rules.md](../plan/data/rules.md) | [systems/balance.md](../systems/balance.md) | `CL9RulesDef.h` |
| `CL9Progression` (단일) | `UCL9ProgressionDef` | [plan/data/progression.md](../plan/data/progression.md) | [systems/balance.md](../systems/balance.md) | `CL9ProgressionDef.h` |
| `CL9DefList` | `UCL9DefList` | [plan/data/README.md](../plan/data/README.md) | 아래 "조회·등록" | `CL9DefRegistry.h` |
| `SettingsTable` | `UCL9SettingsTable` | [plan/settings.md](../plan/settings.md) | [systems/settings.md](../systems/settings.md) | `Source/CL9/UI/CL9SettingsTable.h` |
| `ShakeTable` | `UCL9ShakeTable` | — | [systems/character.md](../systems/character.md)(스태미나 셰이크) · [warning.md](../systems/warning.md) | `CL9ShakeTable.h` |
| `UI/UITable` | `UCL9UITable` | — | [systems/ui_system.md](../systems/ui_system.md) | `Source/CL9/UI/` |
| `SequenceTable` (미구현) | `UCL9SequenceDef` | [plan/data/sequence.md](../plan/data/sequence.md) | [systems/sequence.md](../systems/sequence.md) | — |

`CL9Rules`·`CL9Progression`·`CL9DefList`만 단일 에셋이고 나머지 표는 `UCL9DefTable` 안에 행이 인라인으로 산다
(2026-09-02). 헤더는 적지 않은 것 전부 `Source/CL9/Data/`.

## 공용 타입 (모든 표가 나눠 쓰는 것)

| 키워드 | 무엇 | 정본 | 헤더 |
|---|---|---|---|
| `UCL9DefBase` · `Alias` · `SubAlias` · `DisplayName` · `Description` · `Icon` · `Mesh` · `ActorClass` · `Gimmicks` | 모든 행의 공통 컬럼 | [plan/data/README.md](../plan/data/README.md) 공통 뼈대 · DisplayName 규칙은 [systems/localization.md](../systems/localization.md) | `CL9DefBase.h` |
| `UCL9Gimmick` | 행마다 붙는 특수 동작(Instanced) | [systems/actor-base.md](../systems/actor-base.md) | `CL9DefBase.h` · `CL9Gimmicks.h` |
| `FCL9ItemRef` · `FCL9ItemCount` · `FCL9CarriedItem` · `FCL9CounterItem` | 아이템 참조(Alias+SubAlias)·수량·소지·재고 | [plan/data/item/common.md](../plan/data/item/common.md) · [systems/inventory.md](../systems/inventory.md) | `CL9Types.h` |
| `FDropItem` | 몬스터 드롭 | [systems/drop.md](../systems/drop.md) | `CL9Types.h` |
| `FCL9StatEffect` · `FCL9ActiveEffect` · `ECL9Stat` | 먹기 효과·걸린 버프·스탯 종류 | [plan/data/item/common.md](../plan/data/item/common.md) · [plan/data/buff.md](../plan/data/buff.md) | `CL9Types.h` |
| `FCL9SpawnEntry` · `FCL9DayOverride` | 스폰 기준값·일자별 덮어쓰기 | [plan/data/site.md](../plan/data/site.md) · [plan/data/progression.md](../plan/data/progression.md) | `CL9Types.h` |
| `FCL9AcceptedOrder` · `FCL9SettlementResult` | 받은 주문·정산 결과 | [systems/order.md](../systems/order.md) · [economy.md](../systems/economy.md) | `CL9Types.h` |
| `ECL9CarrySlot` · `ECL9ItemType` · `ECL9CookPhase` | 운반 슬롯·아이템 종류·조리 단계 | [plan/data/item/common.md](../plan/data/item/common.md) · [systems/cooking.md](../systems/cooking.md) | `CL9Types.h` |
| `ECL9HoldPose` · `GripOffset` · `GripRotation` | 손에 들 때 자세·부착 오프셋 | [systems/inventory.md](../systems/inventory.md) | `CL9ItemDef.h` |
| `ECL9UseAction` · `ECL9HitStun` | 좌클 사용·타격 | [systems/item.md](../systems/item.md) · [goal/use-action-left-click.md](../goal/use-action-left-click.md) | `CL9ItemDef.h` |
| `ECL9NightPhase` · `ECL9Generator` · `ECL9ActorKind` · `ECL9BuyResult` | 밤 단계·맵 생성기·액터 종류·구매 결과 | [systems/raiding.md](../systems/raiding.md) · [map.md](../systems/map.md) · [actor-base.md](../systems/actor-base.md) · [shop.md](../systems/shop.md) | `CL9Types.h` |
| `FCL9LightingPhase` | 사이트 조명 3상(로비·밤·정전) | [plan/data/site.md](../plan/data/site.md) 조명 · [systems/light.md](../systems/light.md) | `CL9SiteDef.h` |
| `FCL9PropSet` · `FCL9PropPlacement` · `ECL9PropSide` · `ECL9PropSideRule` | 모듈 소품 세트 | [plan/data/module.md](../plan/data/module.md) · [systems/asset-swap.md](../systems/asset-swap.md) | `CL9ModuleDef.h` |
| `ECL9SegPlacement` (Stretch·Tile·Grid) | 조각 배치 모드 | [systems/asset-swap.md](../systems/asset-swap.md) | `CL9SegKindDef.h` |
| `FSoundEffect` | 사운드 정의 | [systems/sound.md](../systems/sound.md) | `CL9Types.h` |

## 조회·등록·검사

| 키워드 | 무엇 | 정본 |
|---|---|---|
| `UCL9DefRegistry` · `FindItem` · `FindMonster` · `FindSite` · `FindOrder` · `FindEquipment` · `FindCharacter` · `FindBuff` · `FindPiece` · `FindSegKind` · `FindModule` · `GatherModules` | Alias(+SubAlias)로 행 찾기 — 경로가 아니라 이름으로 | [plan/data/README.md](../plan/data/README.md) · 헤더 `CL9DefRegistry.h` |
| `UCL9DefList` · `RefreshFromFolder` | 표 등록 목록 — 코드에 에셋 경로를 쓰지 않는다. MCP 저장은 재스캔이 안 돼 `defs`를 다시 쓴다 | [plan/data/README.md](../plan/data/README.md) · `CL9DefRegistry.h` 주석 |
| `Validate` · `Usable` / `Broken`(코드에선 주석의 EDefStatus 개념) · `CheckSiteReferences` | 로드 검사 — 깨진 행은 `Find*`에서 nullptr, 어느 행·컬럼인지 로그 | [systems/actor-base.md](../systems/actor-base.md) · [systems/cheats.md](../systems/cheats.md)(`cl9.list`) |
| 부등식 A1 · A2 · A4 | 캐릭터·몬스터 속도/스태미나 관계 검사 | [plan/economy.md](../plan/economy.md) "지켜야 하는 부등식" |
| `CL9TuningHash` (설계만, 미구현) | 멀티 접속 시 호스트·클라 표 값 대조 | [systems/session.md](../systems/session.md) |

## 표를 만들거나 고칠 때의 규칙 (요지 — 이유는 링크)

- 파일명은 **접두사 없이** `<이름>Table` (`ItemTable`, `MapTable`) — 2026-09-12, [decisions.md](../decisions.md)
- 새 표는 `Content/Data`에 두고 `CL9DefList`가 그것을 등록한다 — [plan/data/README.md](../plan/data/README.md)
- 행 조회는 `Alias`(+`SubAlias`)로, 경로·파일명으로 찾지 않는다 — [plan/data/README.md](../plan/data/README.md)
- 에셋 참조(Icon·Mesh·Sound)는 소프트 참조 — [systems/actor-base.md](../systems/actor-base.md)
- 화면 문자열(`DisplayName`)은 String Table 참조만 — [systems/localization.md](../systems/localization.md)
- 화폐는 int(이더), 일자별 배율은 표가 아니라 곡선(`UCurveFloat`) — [plan/economy.md](../plan/economy.md) · [plan/data/progression.md](../plan/data/progression.md)
- 치수는 표에 적지 않고 메시 바운드에서 읽는다(맵 조각) — [systems/asset-swap.md](../systems/asset-swap.md)
- 왜 DataTable이 아니라 DataAsset인가 — [plan/data/README.md](../plan/data/README.md)
