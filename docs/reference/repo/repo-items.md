# 참고 — R.E.P.O.의 아이템·장비·업그레이드·상점

> 2026-09-08, 설치본의 `Assembly-CSharp.dll`을 디컴파일해 읽은 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](../README.md).
> CL9의 대응 문서는 [../data/equipment.md](../../plan/data/equipment.md) ·
> [../data/item/common.md](../../plan/data/item/common.md)다.

R.E.P.O.의 아이템 체계는 "종류당 ScriptableObject 하나(`Item`) + 프리팹에 붙는 공용 컴포넌트 묶음(`ItemAttributes`·`ItemToggle`·`ItemEquippable`·`ItemBattery`) + 종류별 동작 스크립트(`ItemGun`, `ItemMelee` …)"로 이루어져 있고, 런 상태는 전부 `StatsManager`의 `Dictionary<string,int>` 묶음에 문자열 키로 저장된다. 상점은 별도 레벨이며, 아이템을 트럭의 추출 구역에 놓고 추출 버튼을 누르면 잔액 범위 안에서 하나씩 "빨아들이며" 결제가 확정된다. 가격은 `Value` 프리셋 값에 배수 4를 곱해 천 단위(K)로 올림하고, 업그레이드는 구매 횟수마다 50 %씩, 힐팩·크리스털은 클리어한 레벨 수에 비례해 오른다. 판정은 거의 전부 마스터 클라이언트가 하고, 클라이언트는 `MasterOnlyRPC` 검사를 통과한 RPC만 받아들인다. 인벤토리는 플레이어당 3칸이며, 장착한 아이템은 물리에서 빠져 크기 0.01로 숨겨지고, 레벨을 옮길 때는 인스턴스 이름의 해시로 다시 장착된다.

근접 무기의 움직임 감지·접촉 피드백은 2026-09-21 설치본 `ItemMelee`를 다시 읽어
[리썰·CL9와의 조작감 비교](../lethal/lethal-combat-feel.md)에 보강했다. 코드 기본값과 프리팹 실값을 구분한다.

## 데이터 모델

### `Item` — 종류당 ScriptableObject 하나

`Item.cs`는 `[CreateAssetMenu(menuName = "Other/Item")]`이 붙은 ScriptableObject이고, 다음 필드를 든다(초기값은 코드 기본값).

| 필드 | 뜻 | 코드 기본값 |
|---|---|---|
| `disabled` | 진열·트럭 배치에서 제외 | false |
| `itemName`, `itemNameLocalized`, `description` | 표시 이름과 설명 | "N/A" |
| `itemType` | `SemiFunc.itemType` 열거형(아래) | — |
| `emojiIcon`, `colorPreset` | UI 이모지, 색 프리셋(배터리 색에도 쓰임) | — |
| `itemVolume` | 진열·트럭 배치 자리의 크기 등급 | — |
| `itemSecretShopType` | `none` / `shop_attic`(비밀 상점 진열) | — |
| `prefab` | `PrefabRef`(리소스 경로와 프리팹) | — |
| `value` | `Value` 프리셋(가격 범위) | — |
| `maxAmount` | 트럭에 동시에 놓일 수 있는 최대 개수 | 1 |
| `maxAmountInShop` | 보유분 포함 진열 가능한 최대 개수 | 1 |
| `minPlayerCount` | 이 인원 미만이면 진열하지 않음 | 1 |
| `maxPurchase`, `maxPurchaseAmount` | 런 전체 누적 구매 상한 | false, 1 |
| `spawnRotationOffset` | 진열 시 회전 보정 | — |
| `physicalItem` | false면 구매해도 인스턴스 이름을 만들지 않음 | true |

`SemiFunc.itemType`은 `drone, orb, cart, item_upgrade, player_upgrade, power_crystal, grenade, melee, healthPack, gun, tracker, mine, pocket_cart, tool, vehicle, launcher` 열여섯 가지다. 코드가 분기하는 것은 `item_upgrade`·`healthPack`·`power_crystal`(가격 공식과 진열 슬롯), `cart`(로비에서 트럭 배치 제외) 정도이고 나머지는 분류용이다. `SemiFunc.itemVolume`은 `small, medium, large, large_wide, power_crystal, large_high, upgrade, healthPack, large_plus, vehicle` 열 가지이며, 진열대(`ItemVolume`)와 아이템의 `itemVolume`이 같아야 그 자리에 놓인다.

### 값·내구도 프리셋

`Value`는 `valueMin = 100`, `valueMax = 200`(코드 기본값)을 가진 ScriptableObject이고, 실제 가격은 여기에 `ShopManager.itemValueMultiplier`(4)를 곱한 뒤 1000으로 나눠 올림한 "K" 단위다. `Durability`(`fragility`·`durability`, 각 0~100)는 아이템 전용이 아니라 `ValuableObject`·`NotValuableObject`의 `durabilityPreset`으로 쓰이는 물리 파손 프리셋이다. 근접 무기의 "내구도"는 이 프리셋이 아니라 `ItemBattery`의 배터리 수치로 구현돼 있다.

### 프리팹에 붙는 공용 컴포넌트

- `ItemAttributes`: `Item` 참조, 런타임 가격 `value`, 저장용 인스턴스 이름 `instanceName`("Item X/3" 꼴), 상점에 있는지 `shopItem`, 아이콘 생성(`SemiIconMaker`로 렌더텍스처를 찍어 `Cache/Icons/Items/*.png`에 캐시). `Start`에서 상점이 아니면 `ItemManager.AddSpawnedItem`에 등록하고, 레벨 생성이 끝나면 마스터가 `StatsManager.ItemFetchName`으로 인스턴스 이름을 붙인다.
- `ItemToggle`: 들고 있는 동안 Interact 키로 `toggleState`를 뒤집는다. 누가 켰는지 `playerTogglePhotonID`에 남고, `onToggle` UnityEvent로 종류별 스크립트가 반응한다. `autoTurnOffWhenEquipped`가 켜져 있으면 인벤토리에 넣는 순간 꺼진다. `disabled`는 마스터만 바꾼다.
- `ItemEquippable`: 인벤토리 장착 상태 기계(`Idle → Equipping → Equipped → Unequipping`), 소유자 `ownerPlayerId`, 칸 번호 `inventorySpotIndex`. 장착 애니메이션은 0.1초에 걸쳐 크기를 0.01배로 줄이고, 해제는 0.2초에 걸쳐 되돌린다.
- `ItemBattery`: `batteryBars = 6`, `batteryLife`(0~100 float), 정수 칸 수 `batteryLifeInt`, `batteryDrainRate = 1`, `autoDrain = true`, `isUnchargable`. `ChargeBattery(charger, rate)`·`Drain(rate)`·`RemoveFullBar(n)`이 외부 인터페이스이고, 칸 수가 바뀔 때마다 `SemiFunc.StatSetBattery(instanceName, (int)batteryLife)`로 저장된다. 상점에 있는 아이템은 매 프레임 100으로 고정된다. `BatteryDrainPresets`(코드 기본값 0.1)를 오브·드론이 참조한다.
- `ItemUpgrade`: `isPlayerUpgrade`, `upgradeEvent`. 상점에서는 `ItemToggle`을 꺼 둔다.
- `ItemVolume`: 씬에 놓인 진열 자리 마커. `itemVolume` 등급과 `itemSecretShopType`을 갖고, 마스터가 아니면 `Start`에서 스스로 파괴된다.

### 관리자

`ItemManager`(DontDestroyOnLoad 싱글턴)는 `itemVolumes`, 구매 목록을 개수만큼 펼친 `purchasedItems`, `powerCrystals`, 현재 레벨에 스폰된 `spawnedItems`, 로컬 인벤토리 이름 목록 `localPlayerInventory`를 든다. `ShopManager`는 진열 후보 목록 네 개(`potentialItems`·`potentialItemConsumables`·`potentialItemUpgrades`·`potentialItemHealthPacks`)와 비밀 상점용 딕셔너리, 장바구니 `shoppingList`, `totalCost`·`totalCurrency`·`isThief`를 든다.

## 종류 목록

| 클래스 | 무엇을 하나 | 배터리 | 소모 | 판정 |
|---|---|---|---|---|
| `ItemGun` (+`ItemGunBullet`, `ItemGunLaser`, `ItemGunTranq`, `ItemGunMuzzleFlash`) | 상태 `Idle/OutOfAmmo/Buildup/Shooting/Reloading`. 토글로 발사, `numberOfBullets`, `gunRandomSpread`, `gunRange 50`. 바닥에 놓인 총은 `misfirePercentageChange`(50 %) 확률로 오발. 마취총은 맞은 플레이어의 동공·음성 피치를 바꾼다 | 발사당 `batteryDrain 0.1` 또는 `batteryDrainFullBars`칸 | 아니오 | 마스터가 `ShootRPC`·`ShootBulletRPC` |
| `ItemMelee` (+`ItemMeleeInflatableHammer`, `ItemStunBaton`) | 상태 `Spawn/Idle/Swinging/Broken/Unequipped`. 휘두르기 감지·타격 판정. 배터리가 0이 되면 `MeleeBreakRPC`로 부러지고(`meshBroken`), 충전되면 `MeleeFixRPC`로 고쳐진다. 풍선 망치는 타격마다 1/19 확률로 적 250 피해 폭발. 스턴 배턴 스크립트는 파티클·소리만 갖는다 | 물체 타격 `durabilityDrain 2.5`, 적·PVP 타격 `durabilityDrainOnEnemiesAndPVP 5` | 아니오(고쳐짐) | 마스터 |
| `ItemGrenade` (+`Explosive`, `DuctTaped`, `Human`, `Shockwave`, `Stun`) | 토글하면 `tickTime 3`초 뒤 폭발. 폭발형은 반경 1.2·플레이어 75·적 160, 덕트테이프형 0.8·50·100 | 없음 | 예(`ItemRemove`) | 마스터가 `TickStartRPC`·`SetActivatorRPC` |
| `ItemMine` (+`Explosive`, `Stun`, `Trigger`) | `MineType None/Explosive/Shockwave/Stun`, 상태 `Disarmed/Arming/Armed/Disarming/Triggering/Triggered`. `armingTime`·`triggeringTime`은 에셋 값. 폭발형은 반경 1.2·플레이어 75·적 200 | 없음 | 예 | 마스터 |
| `ItemDrone` (+`Battery`, `Feather`, `Heal`, `Indestructible`, `Torque`, `ZeroGravity`) | 상태 `Off/Searching/BeamDeployed/Attached/NoBattery`. 대상 필터 `targetValuables/Players/Enemies/NonValuables`. 힐 드론 10 HP, 배터리 드론은 다른 아이템에 `Drain(5)` | `BatteryDrainPresets` | 아니오 | 마스터가 `StateSetRPC` |
| `ItemOrb` (+`Battery`, `Feather`, `Heal`, `Indestructible`, `Magnet`, `Torque`, `ZeroGravity`) | 상태 `Off/Appearing/Active/NoBattery/Disappearing`, `OrbType Constant/Pulse`, 반경 `orbRadius 1`. 범위 안 대상에 효과. 힐 오브 10 HP | `BatteryDrainPresets` | 아니오 | 마스터 |
| `ItemStaffTorque` / `ItemStaffVoid` / `ItemStaffZeroGravity` | 상태 기계로 장전 후 투사체 발사. 상점에서 쓰면 점원 소란 점수 200 | 시전당 `RemoveFullBar(1)` | 아니오 | 마스터가 `PhotonNetwork.Instantiate`로 투사체 |
| `ItemHealthPack` | 토글하면 켠 플레이어에게 `healAmount`(에셋 값)만큼 회복. 체력이 가득하면 `RejectRPC`로 튕겨낸다 | 없음 | 예(`UsedRPC`에서 `ItemEquippable` 제거) | 마스터 |
| `ItemReviveItem` | 죽은 머리 옆에서 `reviveTime 5`초 유지하면 부활. 15초 안전장치 | 없음 | 예 | 마스터 |
| `ItemTracker` | `TrackerType Valuable/Extraction`. 켜져 있는 동안 가장 가까운 대상 방향·거리 표시 | 초당 0.5 | 아니오 | 마스터가 `SetTargetRPC` |
| `ItemLadder` | 상태 `Denied/Neutral/Grabbed/OutOfBattery/Flickering`. `extensionAmount 2`단 연장 | 연장당 `extensionBatteryDrain 5` | 아니오 | 마스터 |
| `ItemWalkieTalkie` + `ItemWalkieBox` | 상자는 상점 밖으로 나오면 워키 두 개를 스폰해 `LinkWalkiesRPC`로 짝지음. 음성 채널 공유 | 코드상 배터리 없음 | 아니오 | 마스터 |
| `ItemRubberDuck`, `ItemDuckBucket` | 오리는 꽥꽥·점프, 폭발 시 적 250 피해·플레이어 0. 양동이는 반경 안 오리를 조용히 하고 안에 든 플레이어 점프를 막는다 | 없음 | 아니오 | 마스터 |
| `ItemLeafBlower` | 바람으로 물체·적을 민다 | 작동 중 `batteryDrainRate 0.5`, 적 명중 시 2 | 아니오 | 마스터가 `BlowActivateRPC` |
| `ItemCartCannon`/`ItemCartCannonMain`, `ItemCartLaser` | 카트에 붙는 포탑·레이저. 상태 `inactive/active/buildup/shooting/goingBack` | 발사당 `RemoveFullBar(1)` | 아니오 | 마스터 |
| `ItemValuableBox` | 상태 `Idle/Open/Absorbing/Close/Full/…/Extracting`. 귀중품을 흡수해 담고 추출 지점에서 꺼낸다. `valueLimit 5000` | 없음 | 아니오 | 마스터 |
| `ItemUpgrade` + `ItemUpgradePlayer*`(13종) | 아래 업그레이드 절 | 없음 | 예(사용 즉시 파괴) | 마스터 |
| `ItemVehicle`, `ItemVehicleBoostPad` | 탈것. 소스가 300 KB로 이 문서의 범위를 넘어 읽지 않았다 | — | — | — |
| `ItemLight` | 아이템에 붙는 조명(들면 켜짐, `alwaysActive` 옵션) | — | — | 로컬 |
| 파워 크리스털(`itemType.power_crystal`) | 충전소 연료. 전용 스크립트 없이 `ChargingStation`이 구매 수를 읽는다 | — | 충전소가 소모 | 마스터 |
| 카트(`itemType.cart`) | 시작 시 "Item Cart Medium" 1개 지급 | — | — | — |

배터리를 쓰는 아이템은 `ItemBattery`를 붙이고, 소모품은 사용 시점에 `StatsManager.ItemRemove(instanceName)`을 호출해 `itemsPurchased`에서 하나를 뺀다. 소모품·업그레이드·크리스털 외에는 `itemsPurchased`를 줄이는 코드 경로가 없으므로, 레벨에 두고 온 아이템이나 배터리가 0인 아이템도 다음 레벨 트럭에 다시 놓인다(코드상 확인).

## 동작 흐름

### 상점 진열

`LevelGenerator`가 생성을 끝내면 `ShopManager.ShopInitialize()`를 부른다. 마스터만 다음을 한다.

1. `GetAllItemsFromStatsManager()`: `itemDictionary`의 모든 `Item`을 돌며 후보를 고른다. 이미 산 개수 `num`이 `maxAmountInShop` 이상이면 제외, `maxPurchase`면 누적 구매(`itemsPurchasedTotal`)가 `maxPurchaseAmount` 이상일 때 제외, `minPlayerCount` 미달 제외, 예상 가격이 잔액보다 크면 75 % 확률로 제외(25 %는 남는다). 남은 것은 `maxAmountInShop - num`개씩 종류별 목록에 넣고 셔플한다. `itemConsumablesAmount`는 여기서 `Random.Range(4, 6)`으로 다시 정해진다.
2. `GetAllItemVolumesInScene()`: 씬의 `ItemVolume`을 모아 비밀 상점용과 일반용으로 나누고 셔플한다.
3. `PunManager.ShopPopulateItemVolumes()`: 비밀 진열대는 각각 1/3 확률로 채운다. 일반 진열대는 순서대로 돌며 일반 아이템(`itemSpawnTargetAmount 8`까지) → 소모품(4~5개까지) → 업그레이드(`itemUpgradesAmount 3`)·힐팩(`itemHealthPacksAmount 3`) 순으로 `itemVolume` 등급이 맞는 첫 후보를 `PhotonNetwork.InstantiateRoomObject`로 스폰한다. 진열대 오브젝트는 스폰 후 파괴된다.

`UpgradeStand`는 업그레이드 진열대로, 안에 놓인 업그레이드를 버튼으로 다시 뽑을 수 있다. 비용은 `RerollCost = 5 + rerollCount * 5`(K)이고, 첫 리롤 때 `maxRerollCount`를 `Random.Range(1, 4)`로 정해 그 횟수에 닿으면 `SetBroken()`으로 망가진다. 새 후보는 `GetWeightedUpgradeExcluding()`이 상점과 같은 필터로 고르며, 잔액을 넘는 것은 1/4 확률로만 남긴다.

### 가격 결정

각 `ItemAttributes.Start`에서 마스터가 `GetValue()`를 계산하고 `GetValueRPC`로 나머지에게 보낸다.

```csharp
float num = Random.Range(itemValueMin, itemValueMax) * ShopManager.instance.itemValueMultiplier; // ×4
if (num < 1000f) num = 1000f;
num = Mathf.Ceil(num / 1000f);   // K 단위
```

그 뒤 종류별 보정을 거친다.

- 업그레이드 `UpgradeValueGet`: 인원 `n = min(6, 플레이어 수)`에 대해 `value -= value * 0.1 * (n-1)`, 이어서 `value += value * 0.5 * (그 업그레이드를 이번 런에 산 횟수)`, 올림.
- 힐팩 `HealthPackValueGet`: 같은 인원 할인 뒤 `value += value * 0.05 * min(levelsCompleted, 15)`, 올림.
- 크리스털 `CrystalValueGet`: `value += value * 0.2 * min(levelsCompleted, 15)`, 올림.

진열 후보를 고를 때는 `valueMax`만으로 같은 공식을 돌려 "예상 가격"을 만든다.

### 구매 확정

상점 레벨의 트럭은 `ExtractionPoint`(`isShop = true`)를 겸한다.

1. `ItemAttributes.ShopInTruckLogic()`이 0.5초마다 아이템이 추출 구역 `RoomVolume`(`Extraction`) 안에 있는지 보고 `ShopManager.ShoppingListItemAdd/Remove`를 부른다. 그때마다 `PunManager.ShopUpdateCost()`가 구역 안 아이템의 `value` 합을 `UpdateShoppingCostRPC`로 전원에게 보내고, `ShopCostUI`가 "-$XK"로 띄운다.
2. 플레이어가 추출 버튼을 누르면 `ExtractionPoint`가 `ShopManager.ShopCheck()`로 목록을 정리하고, 흡입 상태에서 `DestroyTheFirstPhysObjectsInShopList()`를 반복한다. 잔액에서 `value`를 뺄 수 있을 때만 `StatSetRunCurrency(잔액 - value)` → `StatsManager.ItemPurchase(item.name)` → 업그레이드면 `AddItemsUpgradesPurchased`, 크리스털이면 `runStats["chargingStationChargeTotal"] += 17`(상한 100) → 오브젝트 파괴. 마지막에 `DestroyAllPhysObjectsInShoppingList()`가 남은 것을 같은 조건으로 처리한다.
3. 처리 뒤에도 `ShopGetTotalCost() > 0`이면(잔액이 모자라 남은 아이템이 있으면) `ThiefPunishment()`가 `isThief`를 켜고 트럭 화면을 3쪽으로 넘기며 수류탄 다섯 개를 추출 지점 위에 스폰한다. 상점 점원 `ShopKeeper`는 이와 별개로 "소란 점수"(총격 100, 폭발 200, 지팡이 200, 근접으로 물체 타격 100, 상한 1000)를 누적해 상태 기계(`Sleeping … Shoot`)로 반응한다.

`ItemPurchase`는 `itemsPurchased`·`itemsPurchasedTotal`을 1씩 올리고, `physicalItem`이면 `ItemAdd`로 인스턴스 이름("Item X/N")을 `item` 딕셔너리에 등록한다.

### 트럭 배치와 레벨 이동

레벨·로비 시작 시 `ItemManager.ItemsInitialize()`가 마스터에서 돈다.

- `GetPurchasedItems()`: `itemsPurchased`의 (아이템 이름, 개수)를 `maxAmount`로 잘라 `purchasedItems`에 펼친다. 파워 크리스털은 로비에서만, 카트는 로비가 아닐 때만 놓는다.
- `PunManager.TruckPopulateItemVolumes()`: 트럭의 `ItemVolume` 중 등급이 맞는 자리에 하나씩 `InstantiateRoomObject`한다.
- 스폰된 `ItemAttributes`가 `LateStart`에서 `StatsManager.ItemFetchName`을 부르면, `item` 딕셔너리에 남아 있는 "이름/번호" 중 아직 배정되지 않은 것을 골라 `SetItemNameRPC`로 전원에게 알린다. 이 이름으로 `itemStatBattery`에서 배터리를 되살리고(`SetBatteryLife`), `playerInventorySpotN[steamID]`가 이름의 `GetHashCode()`와 같으면 그 플레이어의 그 칸에 `RequestEquip`한다.

### 인벤토리

`Inventory`는 플레이어 컨트롤러에 붙은 로컬 싱글턴으로, 칸을 3개 만든다(`Awake`에서 `for (int i = 0; i < 3; i++)`). `InventorySpot`이 UI이자 입력 처리기이고, `Inventory1/2/3` 키가 각각 0·1·2번 칸에 대응한다.

- 장착: 칸이 비어 있고 `ItemEquippable`이 있는 물체를 잡고 있으면 `RequestEquip(spot, 내 PhysGrabber ViewID)` → 마스터의 `RPC_RequestEquip` → 상태가 `Idle`일 때만 `RPC_UpdateItemState(2, spot, owner)`를 전원에게. 받는 쪽은 `MasterAndOwnerOnlyRPC`를 검사한 뒤 칸을 채우고 `StatsManager.PlayerInventoryUpdate`로 저장한다. 장착된 물체는 잡기가 풀리고(`OverrideGrabRelease`), 크기가 0.01배가 되면 `OverrideDeactivate`로 물리에서 빠진다. 장착 중에는 `ItemBattery.autoDrain`이 멈추고 `ItemToggle`이 자동으로 꺼진다.
- 해제: 같은 키를 다시 누르면 `RequestUnequip` → `RPC_StartUnequip`(전원) → 소유자만 `PerformUnequip`에서 시선 방향 1 m 레이캐스트로 놓을 위치를 정해 `RPC_CompleteUnequip`을 마스터에게 → 마스터가 `OwnerOnlyRPC` 확인 뒤 텔레포트·질량 0.1→원래값 복원·`RPC_UpdateItemState(3, …)`. 해제 직후 1.5초 동안은 자동으로 다시 잡힌다(`forceGrabTimer`).
- 사망: `PlayerAvatar`가 `Inventory.ForceUnequip()`으로 시선 위치 근처에 전부 떨어뜨린다(상점·아레나에서는 하지 않음).
- 배터리 표시: `InventorySpot`이 `BatteryVisualLogic`으로 칸 안에 막대를 그리고, 로비에서 절반 이하이면 충전 필요 표시를 낸다. 상점에서는 `InventoryUI`가 숨겨진다. 상점에서 장착 자체를 막는 코드는 찾지 못했다(추정: 장착하면 물리에서 빠져 추출 구역 판정에서도 빠질 것이다).

### 업그레이드 사용

업그레이드는 물리 아이템이며, 레벨이나 로비에서 들고 Interact를 누르면 `ItemToggle.onToggle` → `ItemUpgrade.PlayerUpgrade()`가 돈다. 여기서 `upgradeEvent`가 `ItemUpgradePlayerX.Upgrade()`를 불러 `PunManager.UpgradePlayerX(누른 플레이어의 steamID)`를 호출하고, 마스터가 `playerUpgradeX[steamID]`를 1 올리며 즉시 효과를 준다. 그 뒤 `itemsPurchased[item.name]`을 1 줄이고 오브젝트를 파괴한다. 효과는 다음과 같다.

| 딕셔너리 | 즉시 효과(레벨당) |
|---|---|
| `playerUpgradeHealth` | `maxHealth += 20`, 로컬이면 20 회복 |
| `playerUpgradeStamina` | `EnergyStart += 10` |
| `playerUpgradeExtraJump` | `JumpExtra += 1` |
| `playerUpgradeSpeed` | `SprintSpeed += 1`(스프린트 에너지 소모도 그만큼 증가) |
| `playerUpgradeStrength` | `grabStrength += 0.2` |
| `playerUpgradeThrow` | `throwStrength += 0.3` |
| `playerUpgradeRange` | `grabRange += 1` |
| `playerUpgradeMapPlayerCount` | 맵에 다른 플레이어 표시 |
| `playerUpgradeLaunch`, `playerUpgradeTumbleClimb`, `playerUpgradeTumbleWings` | 텀블(구르기) 발사·기어오르기·날개 |
| `playerUpgradeCrouchRest` | 웅크려 쉬기 |
| `playerUpgradeDeathHeadBattery` | 죽은 머리 배터리 연장 |

가격 상승용 `itemsUpgradesPurchased`는 구매 시점에 오르고, 사용 시점에는 오르지 않는다. 플레이어가 새로 스폰될 때 `PlayerController`·`PhysGrabber`·`PlayerAvatar`가 같은 딕셔너리를 읽어 같은 배율을 다시 적용한다.

### 충전소와 트럭 회복

`ChargingStation`은 트럭 안의 충전대다. `chargeInt`는 구매한 "Item Power Crystal" 수, `chargeTotal`(0~100)은 `runStats["chargingStationChargeTotal"]`에 저장된 잔량이며, 크리스털 1개가 10 에너지다(`energyPerCrystal 10`, `maxCrystals 10`). 0.5초마다 `chargeArea` 박스 안의 `ItemBattery` 중 가득 차지 않은 것을 모아 `ChargeBattery(this, 10)`(초당 10 %)로 채우고, 충전 중인 아이템 하나당 0.05초마다 `chargeFloat -= chargeRate`를 뺀다. `chargeRate`는 `Start`에서 `1 / energyPerCrystal / chargeTotal`로 계산되므로 기본 상태에서 0.001, 즉 초당 약 2 %다. 잔량이 크리스털 단위 아래로 내려가면 `ChargingStationCrystalBrokenRPC`가 크리스털 하나를 깨고 `itemsPurchased`에서 뺀다. 상점 레벨에서는 충전하지 않는다.

`TruckHealer`는 레벨에서 모든 추출 지점이 완료되면 해치를 열고, `PlayerAvatar.finalHeal`이 전원에게 찍힐 때까지 `Heal()` 빔을 쏜 뒤 닫힌다. 회복량 자체는 이 클래스에 없고 플레이어 쪽에서 처리한다(추정).

## 관리·저장

`StatsManager`(DontDestroyOnLoad 싱글턴)는 모든 런 상태를 `SortedDictionary<string, Dictionary<string,int>> dictionaryOfDictionaries`에 이름으로 등록한다.

| 딕셔너리 이름 | 키 | 값 |
|---|---|---|
| `runStats` | `"level"`, `"currency"`, `"lives"`, `"chargingStationCharge"`, `"chargingStationChargeTotal"`, `"totalHaul"` | 정수 |
| `itemsPurchased` | `Item` 에셋 이름(예: `"Item Power Crystal"`) | 현재 보유 개수 |
| `itemsPurchasedTotal` | 같음 | 런 누적 구매 수(`maxPurchase` 판정) |
| `itemsUpgradesPurchased` | 같음 | 가격 상승용 구매 수 |
| `itemBatteryUpgrades` | 같음 | 배터리 업그레이드 단계 |
| `item` | 인스턴스 이름 `"Item X/N"` | `itemDictionary` 안 인덱스 |
| `itemStatBattery` | 인스턴스 이름 | 배터리 0~100 |
| `playerHealth`, `playerUpgrade*`(13개), `playerColor`, `playerHasCrown` | steamID 문자열 | 정수 |
| `playerInventorySpot1~3` | steamID | 인스턴스 이름의 `GetHashCode()` |

`itemDictionary`는 `Resources.LoadAll<Item>("ScriptableObjects")`로 모든 `Item` 에셋을 이름 키로 적재한다. 런 시작(`RunStartStats`) 기본값은 `level 0, currency 0, lives 3, chargingStationCharge 1, chargingStationChargeTotal 100, totalHaul 0`이며 파워 크리스털 1개와 "Item Cart Medium" 1개를 지급한다.

저장은 마스터만 하며(`SaveGame`), Easy Save 3로 `persistentDataPath/saves/<이름>/<이름>.es3`에 AES 암호화해 쓰고 기존 파일은 `_BACKUP<n>`으로 옮긴다. `playerInventorySpot1~3`·`playerColor`는 저장하지 않고(`doNotSaveTheseDictionaries`), `stripTheseDictionaries`에 등록된 기본값(구매 0, 배터리 100, 체력 100, 업그레이드 0)과 같은 항목은 지워서 저장한다. `DictionaryUpdateValue`도 같은 규칙으로 기본값이면 키를 지운다. 씬 전환(`SemiFunc.OnSceneSwitch`)마다 `SaveFileSave`·`StatSyncAll`이 돌고, `takenItemNames`와 `playerInventorySpotNTaken`이 초기화된다.

## 네트워크

원칙은 "마스터가 결정하고 전원에게 RPC, 받는 쪽은 `SemiFunc.MasterOnlyRPC(_info)`로 발신자를 검사"다. 예외적으로 소유자 판정이 필요한 곳은 `OwnerOnlyRPC`·`MasterAndOwnerOnlyRPC`를 쓴다. 스폰은 항상 `PhotonNetwork.InstantiateRoomObject`(룸 소유)라 마스터가 바뀌어도 살아남는다.

| 영역 | RPC | 방향·검사 |
|---|---|---|
| 가격 | `ItemAttributes.GetValueRPC` | 마스터 → Others, MasterOnly |
| 토글 | `ItemToggle.ToggleItemRPC` | 든 사람 → All, 검사 없음 |
| 토글 잠금 | `ItemToggle.ToggleDisableRPC` | All, MasterOnly |
| 배터리 | `BatteryToggleRPC`, `BatteryChargeStartRPC`, `BatteryFullPercentChangeRPC` | 마스터 → All, MasterOnly |
| 장착 | `RPC_RequestEquip`(→ 마스터), `RPC_UpdateItemState`(All, MasterAndOwnerOnly), `RPC_StartUnequip`(All), `RPC_CompleteUnequip`(→ 마스터, OwnerOnly), `RPC_SendPing` | 위 참조 |
| 이름·통계 | `PunManager.SetItemNameRPC`, `AddingItemRPC`, `UpdateStatRPC(dictionaryName, key, value)`, `SetRunStatRPC`, `UpdateShoppingCostRPC`, `UpgradeItemBatteryRPC`, `SyncHaul`, `ReceiveSyncData` | 마스터 → All/Others, MasterOnly |
| 충전소 | `StartChargeRPC`, `StopChargeRPC`, `ChargingStationSegmentChangedRPC`(AllBuffered, 이전 버퍼 제거), `ChargingStationCrystalBrokenRPC` | 마스터, MasterOnly |
| 업그레이드 진열대 | `UpgradeStand.StateSetRPC`, `SyncRerollCountRPC`, `SyncMaxRerollCountRPC` | 마스터 |
| 키카드 문 | `ShopKeycardDoor.StateSetRPC` | 마스터 |
| 종류별 | `ShootRPC`·`ShootBulletRPC`(총), `StateSetRPC`(총·근접·드론·지뢰·사다리·귀중품 상자), `TickStartRPC`·`TickEndRPC`·`SetActivatorRPC`(수류탄), `MeleeBreakRPC`·`MeleeFixRPC`·`SwingHitRPC`·`DinkRPC`(근접), `TriggeredRPC`(지뢰), `SetTargetRPC`(트래커), `UsedRPC`·`RejectRPC`(힐팩), `ReviveParticlesRPC`·`UpdatePlayerTargetRPC`(부활), `LinkWalkiesRPC`(워키), `BlowActivateRPC`(리프블로어), `SlowDownVoiceRPC`(마취총, OwnerOnly) | 대부분 마스터 → All, MasterOnly |

업그레이드 값은 RPC 이름이 따로 없고, 마스터가 `playerUpgradeX` 딕셔너리를 고친 뒤 `SyncAllDictionaries`/`UpdateStat`으로 밀어 준다. 클라이언트는 `PlayerController`·`PhysGrabber`가 스폰 시 `StatsManager`를 읽어 로컬 수치를 재계산한다.

## 수치

| 항목 | 값 | 출처 |
|---|---|---|
| 가격 배수 `itemValueMultiplier` | 4 | `ShopManager` |
| 최소 가격 | 1 K | `ItemAttributes.GetValue` |
| 업그레이드 인원 할인 | 추가 인원당 −10 %, 최대 6인 | `UpgradeValueGet` |
| 업그레이드 반복 구매 상승 `upgradeValueIncrease` | +50 %/회 | `ShopManager` |
| 힐팩 레벨 상승 `healthPackValueIncrease` | +5 %/레벨, 최대 15레벨 | `ShopManager` |
| 크리스털 레벨 상승 `crystalValueIncrease` | +20 %/레벨, 최대 15레벨 | `ShopManager` |
| 진열 목표 수 | 일반 8, 소모품 4~5, 업그레이드 3, 힐팩 3 | `ShopManager`, `GetAllItemsFromStatsManager` |
| 비싼 아이템 진열 확률 | 25 % | `GetAllItemsFromStatsManager` |
| 비밀 진열대 채움 확률 | 1/3 | `ShopPopulateItemVolumes` |
| 리롤 비용 | 5 + 5 × 횟수 (K), 한도 1~3회 | `UpgradeStand` |
| 도둑 처벌 | 수류탄 5개 | `ExtractionPoint.ThiefPunishment` |
| 인벤토리 칸 | 3 | `Inventory.Awake` |
| 장착·해제 애니메이션 | 0.1 s / 0.2 s, 해제 후 자동 잡기 1.5 s | `ItemEquippable` |
| 배터리 칸 | 6칸, 100 % | `ItemBattery` |
| 충전 속도 | 아이템 +10 %/s, 충전소 −0.001/0.05 s(≈2 %/s) | `ChargingStation` |
| 크리스털 | 1개 = 10 에너지, 최대 10개, 구매 시 +17 | `ChargingStation`, `ExtractionPoint` |
| 근접 내구도 소모 | 물체 2.5, 적·PVP 5 (100 중) | `ItemMelee` |
| 총 배터리 소모 | 발사당 0.1 또는 n칸, 오발 50 % | `ItemGun` |
| 수류탄 | 3 s, 폭발형 1.2/75/160 | `ItemGrenade`, `ItemGrenadeExplosive` |
| 지뢰 폭발형 | 1.2/75/200 | `ItemMineExplosive` |
| 트래커 | −0.5 %/s | `ItemTracker` |
| 사다리 | 2단, 단당 −5 % | `ItemLadder` |
| 리프블로어 | −0.5 %/s, 적 명중 −2 % | `ItemLeafBlower` |
| 힐 드론·오브 | 10 HP | `ItemDroneHeal`, `ItemOrbHeal` |
| 부활 | 5 s 유지, 15 s 안전장치 | `ItemReviveItem` |
| 업그레이드 효과 | 체력 +20, 스태미나 +10, 속도 +1, 잡기 힘 +0.2, 던지기 +0.3, 거리 +1, 점프 +1 | `PunManager` |
| 시작 지급 | 크리스털 1, 카트 1, 목숨 3, 잔액 0 | `RunStartStats` |
| 점원 소란 점수 | 총격 100, 폭발 200, 지팡이 200, 근접 물체 타격 100, 상한 1000 | `ShopKeeper` |

## CL9 시사점

- REPO의 `Item` ScriptableObject는 CL9의 "종류당 DataAsset 하나" 원칙과 같은 자리에 있다. 필드 중 `itemVolume`(진열 자리 등급)·`maxAmountInShop`·`minPlayerCount`·`maxPurchase`·`physicalItem`은 CL9 장비 표(equipment)에 그대로 열을 추가할 수 있는 항목이다. 특히 `physicalItem = false`인 "구매하면 즉시 적용되는 항목"이 데이터로 구분돼 있는 점이 눈에 띈다.
- 공용 컴포넌트 넷(`ItemAttributes`·`ItemToggle`·`ItemEquippable`·`ItemBattery`)이 종류별 스크립트와 분리돼 있다. CL9에서 손전등·헤드램프(배터리·토글)와 수레(장착 불가, 배터리 없음)를 나눌 때 이 네 가지가 "있음/없음" 조합으로 표현된다는 점이 참고가 된다.
- 근접 무기의 내구도가 별도 시스템이 아니라 배터리 수치로 구현되고, 0이 되면 부러졌다가 충전하면 고쳐진다. CL9에서 내구도와 배터리를 하나의 게이지로 합칠지 나눌지 판단할 때 대응되는 사례다.
- 가격 공식은 "기본값 × 배수 → K 단위 올림 → 인원 할인 → 반복 구매·레벨 진행 상승"의 순서다. CL9의 주문서 쿼터가 런 진행에 따라 오르는 구조라면, 힐팩(+5 %/레벨, 상한 15)과 크리스털(+20 %/레벨)처럼 "레벨 수에 상한을 둔 선형 상승"이 눈여겨볼 수치다. 인원 할인은 최대 6인이지만 CL9는 4인이 상한이다.
- 구매 확정이 "트럭 추출 구역에 놓고 버튼"이라는 물리 행위로 이루어지고, 잔액이 모자라면 자동으로 도둑 처벌이 붙는다. CL9는 푸드트럭이 판매와 요리 장소라서 "재료를 트럭에 실으면 팔린다"는 흐름이 여기 추출 구역 판정(`RoomVolume.Extraction` + 0.5초 폴링)과 대응한다.
- 트럭 배치는 `itemsPurchased` 개수를 `maxAmount`로 잘라 `itemVolume` 등급이 맞는 자리에 채우는 방식이고, 인스턴스 이름("Item X/N")의 해시로 배터리와 인벤토리 칸을 되살린다. CL9가 호스트 판정·레벨 간 인벤토리 유지를 설계할 때 "에셋 이름 + 일련번호" 키가 가장 단순한 형태라는 점을 보여 준다.
- 인벤토리 3칸, 장착 시 물리에서 완전히 제거, 사망 시 전부 드롭, 장착 중 배터리 소모 정지. CL9의 장비 개수를 정할 때 비교 기준이 된다.
- 업그레이드는 플레이어별 딕셔너리(`playerUpgradeX[steamID]`)에 누적되고, 가격 상승 카운터(`itemsUpgradesPurchased`)는 팀 단위로 하나다. 즉 "누가 먹었는지"와 "몇 번 샀는지"가 분리돼 있다. CL9에서 협동 업그레이드를 개인 소유로 할지 팀 공유로 할지 정할 때 참고가 된다.
- 소모품·업그레이드·크리스털 외에는 보유 수를 줄이는 경로가 없어 두고 온 장비가 다음 레벨에 되돌아온다. CL9에서 몬스터(닭·거위·돼지·너구리·양)에게 장비를 잃는 규칙을 둘지 말지가 설계 결정 지점이다.
- 충전소는 "구매한 크리스털 수 = 연료"로, 별도 자원 없이 아이템 하나가 트럭 시설의 연료 역할을 겸한다. CL9의 트럭 시설(요리 화구 등)에 연료 개념을 둔다면 같은 방식이 가능하다.
- 네트워크는 마스터 판정 + `MasterOnlyRPC` 발신자 검사 + 룸 소유 스폰이 전부다. CL9의 리슨 서버(호스트 판정)와 같은 모델이므로, RPC 이름 표는 CL9에서 필요한 복제 이벤트 목록의 초안으로 쓸 수 있다.
