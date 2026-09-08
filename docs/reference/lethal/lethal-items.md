# 참고 — Lethal Company의 아이템·상점·판매

> 2026-09-08, 설치본의 `Assembly-CSharp.dll`을 디컴파일하고 에셋 값을 읽은 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](../README.md).
> CL9의 대응 문서는 [../data/item/common.md](../../plan/data/item/common.md) ·
> [../data/equipment.md](../../plan/data/equipment.md) · [../economy.md](../../plan/economy.md)다.

Lethal Company의 아이템 계층은 매우 얇다. 종류마다 `Item`이라는 ScriptableObject 하나가
"이 물건이 무엇인가"를 전부 기술하고, 씬에 놓이는 실체는 `GrabbableObject`를 상속한
NetworkBehaviour 하나다. 스크랩(고철)과 상점 물건은 별도의 클래스가 아니라 같은 `Item`
안의 `isScrap` 플래그 하나로 갈린다. 값은 서버가 라운드 시작 때 한 번 뽑아 클라이언트에
뿌리고, 판매는 회사 데스크에서 매입률을 곱해 크레딧으로 바뀌며, 구매는 터미널 텍스트
파서를 통해 이루어진 뒤 배달선이 실제 프리팹을 스폰한다. 아래는 그 각 부분의 구조와
실제 수치다.

---

## 데이터 모델

### `Item.cs` — 종류당 하나인 ScriptableObject

`[CreateAssetMenu(menuName = "ScriptableObjects/Item")]`로 만들어지는 순수 데이터 자산이다.
필드가 전부 public이고 로직이 하나도 없다. 아래가 전체 목록이며, 값이 적힌 것은 C# 코드
기본값이다.

| 필드 | 형 | 코드 기본값 | 무엇을 정하는가 |
|---|---|---|---|
| `itemId` | int | — | 정수 식별자. 코드가 특정 아이템을 하드코딩으로 분기할 때 쓴다(예: 13=제트팩, 14=열쇠로 추정) |
| `itemName` | string | — | 표시 이름 |
| `spawnPositionTypes` | List\<ItemGroup\> | 빈 리스트 | 이 아이템이 스폰될 수 있는 자리의 분류 |
| `twoHanded` | bool | false | 두 손 아이템. 들고 있으면 다른 물건을 못 잡고 슬롯 전환도 막힌다 |
| `twoHandedAnimation` | bool | false | 두 손 애니메이션만 쓰는가(들기 제약과 별개) |
| `disableHandsOnWall` | bool | false | 벽에 손 짚는 연출 억제 |
| `canBeGrabbedBeforeGameStart` | bool | false | 게임 시작 전(우주선 단계)에 집을 수 있는가 |
| `disallowUtilitySlot` | bool | false | 유틸리티 벨트 슬롯에 넣지 못하게 한다 |
| `weight` | float | 1f | 무게. **1이 기준이고 초과분만 누적된다**(아래 무게 항목) |
| `itemIsTrigger` | bool | false | 1회성 사용. 배터리를 누를 때마다 `batteryUsage`만큼 통째로 깎는다 |
| `holdButtonUse` | bool | false | 버튼을 누르고 있는 동안 사용(토글이 아니라) |
| `itemSpawnsOnGround` | bool | true | 스폰 시 바닥으로 떨어뜨리는가 |
| `isConductiveMetal` | bool | false | 전도성 금속(번개 표적) |
| `isScrap` | bool | false | **고철인가**. 값 추첨·판매·레이더 아이콘·전멸 소멸 규칙이 전부 이 플래그로 갈린다 |
| `creditsWorth` | int | — | 상점 정가 |
| `lockedInDemo` | bool | false | 데모 잠금 |
| `highestSalePercentage` | int | 80 | 이 물건에 붙을 수 있는 최대 할인 폭(%) |
| `maxValue` / `minValue` | int | — | 고철 값 추첨 구간 |
| `spawnPrefab` | GameObject | — | 실제로 스폰되는 프리팹 |
| `requiresBattery` | bool | **true** | 배터리를 쓰는가 |
| `batteryUsage` | float | 15f | 지속형이면 **완충 상태의 사용 가능 초**, 1회성(`itemIsTrigger`)이면 1회당 소모 비율 |
| `automaticallySetUsingPower` | bool | true | 사용 토글을 `isBeingUsed`에 자동 반영 |
| `itemIcon` | Sprite | — | 슬롯 아이콘 |
| `grabAnim` / `useAnim` / `pocketAnim` / `throwAnim` | string | — | 플레이어 애니메이터 파라미터 이름 |
| `grabAnimationTime` | float | 0 | 잡기 동작 길이. 0이면 코드가 0.4초를 쓴다 |
| `grabSFX` / `dropSFX` / `pocketSFX` / `throwSFX` | AudioClip | — | 플레이어 쪽 효과음 |
| `syncGrabFunction` | bool | true | 잡기를 RPC로 동기화할지 |
| `syncUseFunction` | bool | true | 사용을 RPC로 동기화할지 |
| `syncDiscardFunction` | bool | true | 버리기를 RPC로 동기화할지 |
| `syncInteractLRFunction` | bool | true | 좌/우 보조 조작을 RPC로 동기화할지 |
| `saveItemVariable` | bool | false | **세이브에 int 하나를 추가로 저장**할지(스프레이 잔량 등) |
| `isDefensiveWeapon` | bool | false | 몬스터가 위협도를 계산할 때 참조 |
| `toolTips` | string[] | — | 조작 안내 문구 |
| `verticalOffset` | float | 0 | 바닥 스폰 시 위로 띄우는 양 |
| `floorYOffset` | int | 0 | 바닥에 놓일 때 Y 회전 보정 |
| `allowDroppingAheadOfPlayer` | bool | true | 앞쪽에 던져 놓을 수 있는가 |
| `restingRotation` | Vector3 | **(0, 0, 90)** | 바닥에 눕는 자세 |
| `rotationOffset` / `positionOffset` | Vector3 | zero | 손에 들었을 때의 부착 오프셋 |
| `meshOffset` | bool | true | 메시 오프셋 사용 |
| `meshVariants` / `materialVariants` | Mesh[] / Material[] | — | 같은 종류의 외형 변주 |
| `clinkAudios` | AudioClip[] | — | 흔들릴 때 나는 짤랑 소리 |
| `usableInSpecialAnimations` | bool | false | 특수 애니메이션 중에도 쓸 수 있는가 |
| `canBeInspected` | bool | true | 들여다보기 가능 |

### `GrabbableObject.cs` — 씬에 존재하는 실체

`public abstract class GrabbableObject : NetworkBehaviour`이며, 모든 아이템의 공통 베이스다.
1,386줄이지만 상태는 단순하다. 소유·위치 상태로 `isHeld`, `isHeldByEnemy`, `isPocketed`,
`isBeingUsed`, `isInElevator`, `isInShipRoom`, `isInFactory`, `playerHeldBy`,
`heldByPlayerOnServer`, `parentObject`가 있고, 낙하 상태로 `targetFloorPosition`,
`startFallingPosition`, `floorYRot`, `fallTime`, `hasHitGround`, `reachedFloorTarget`가 있다.
값과 배터리는 `scrapValue`(int)와 `insertedBattery`(`Battery`) 두 개뿐이며,
`scrapPersistedThroughRounds`가 "이건 지난 라운드에서 살아남은 물건"임을 표시한다.

파생 클래스가 덮어쓰는 가상 함수는 잡기(`GrabItem`, `EquipItem`, `PocketItem`,
`DiscardItem`), 사용(`ItemActivate`, `ItemInteractLeftRight`), 배터리(`UseUpBatteries`,
`ChargeBatteries`), 저장(`GetItemDataToSave`, `LoadItemSaveData`), 그리고
`OnHitGround`·`OnBroughtToShip`·`ReactToSellingItemOnCounter` 같은 이벤트 훅이다.

`PhysicsProp.cs`는 `GrabbableObject`를 상속만 하고 아무것도 더하지 않는 빈 클래스로,
"특별한 동작이 없는 순수 고철"이 쓰는 기본 컴포넌트다. 실제로 고철 대부분이 이것이다.

`AnimatedItem.cs`는 여기에 애니메이터·오디오·소음을 얹는다. `grabItemBoolString`·
`dropItemTriggerString`으로 애니메이터를 몰고, `chanceToTriggerAnimation`(기본 100)과
`chanceToTriggerAlternateMesh`로 개체마다 변주를 준다. 중요한 건 `Update()`에서
오디오가 재생 중이면 0.75초마다 소음을 뿌린다는 점이다. 같은 자리(4 m 이내)에서
반복하면 `timesPlayedInOneSpot`을 올려 몬스터가 학습하게 만들고, 주머니에 넣은 상태면
사거리와 크기를 절반으로 줄인다.

### 부속 자산들

`ItemGroup`은 `itemSpawnTypeName` 문자열 하나뿐인 ScriptableObject로, 스폰 지점 분류에만
쓰인다. `AllItemsList`는 `List<Item>` 하나뿐이며 **세이브 파일의 아이템 ID가 이 리스트의
인덱스**라서 순서가 곧 저장 포맷이다. `SpawnableItemWithRarity`는 `Item` + `rarity`(0~100)
쌍으로, 행성(`SelectableLevel.spawnableScrap`)마다 다른 확률표를 만든다.

`ScanNodeProperties`는 스캔 표시를 담당하는 MonoBehaviour다. `maxRange = 7`, `minRange = 5`,
`requiresLineOfSight = true`가 코드 기본값이고, `headerText`/`subText`/`scrapValue`/
`creatureScanID`(-1이면 생물 아님)/`nodeType`을 가진다. 값이 정해질 때
`GrabbableObject.SetScrapValue()`가 `subText`를 `"Value: $N"`으로 덮어쓴다.

`Battery`는 `[Serializable]` 순수 데이터로 `empty`(bool)와 `charge`(float, 기본 1f)뿐이다.
`RandomScrapSpawn`은 씬에 뿌려두는 스폰 마커로 `spawnableItems`(ItemGroup),
`itemSpawnRange`, `spawnedItemsCopyPosition`, `spawnUsed`, `spawnWithParent`를 가진다.
`HoarderBugItem`은 아이템이 아니라 사재기벌레의 기억 구조체로,
`itemGrabbableObject` + `itemNestPosition` + `status`를 묶는다.

---

## 실측 — 아이템 58종의 실제 값

아래는 코드 기본값이 아니라 **설치본의 `Item` 에셋에서 직접 읽은 값**이다. 무게는 `weight`
필드 원값이고 괄호 안은 게임이 화면에 쓰는 파운드 표기(`(weight - 1) * 105`)다. 무게 1.000은
**들어도 전혀 느려지지 않는다**는 뜻이다 — 초과분만 누적되기 때문이다.

| 이름 | 무게 | 값(min~max) | 스크랩 | 두 손 | 배터리 | 전도성 | 상점가 |
|---|---|---|---|---|---|---|---|
| Gold bar | 1.730 (76.7lb) | 255~525 | 예 |  |  | 예 |  |
| Apparatus | 1.300 (31.5lb) | 40~120 | 예 | 예 |  | 예 |  |
| Chemical jug | 1.300 (31.5lb) | 80~210 | 예 | 예 |  |  |  |
| Painting | 1.300 (31.5lb) | 150~310 | 예 | 예 |  |  |  |
| Heart | 1.300 (31.5lb) | 60~250 | 예 | 예 |  |  |  |
| Clock | 1.250 (26.2lb) | 110~140 | 예 |  |  |  |  |
| Metal sheet | 1.250 (26.2lb) | 25~55 | 예 |  |  | 예 |  |
| Bell | 1.230 (24.2lb) | 120~200 | 예 |  |  | 예 |  |
| Fancy lamp | 1.200 (21.0lb) | 150~320 | 예 | 예 |  | 예 |  |
| Toy robot | 1.200 (21.0lb) | 140~220 | 예 |  |  | 예 |  |
| Tea kettle | 1.200 (21.0lb) | 80~140 | 예 |  |  | 예 |  |
| Toy train | 1.200 (21.0lb) | 130~210 | 예 |  |  |  |  |
| Egg | 1.180 (18.9lb) | 150~210 | 예 | 예 |  |  |  |
| Big bolt | 1.180 (18.9lb) | 50~80 | 예 |  |  | 예 |  |
| Bottles | 1.180 (18.9lb) | 110~140 | 예 | 예 |  |  |  |
| Soccer ball | 1.180 (18.9lb) | 110~180 | 예 | 예 |  |  |  |
| Magic 7 ball | 1.150 (15.7lb) | 90~180 | 예 |  |  |  |  |
| Large axle | 1.150 (15.7lb) | 90~140 | 예 | 예 |  | 예 |  |
| Control pad | 1.150 (15.7lb) | 85~160 | 예 | 예 |  | 예 |  |
| V-type engine | 1.150 (15.7lb) | 50~140 | 예 | 예 |  | 예 |  |
| Golden cup | 1.150 (15.7lb) | 100~200 | 예 |  |  |  |  |
| Flask | 1.150 (15.7lb) | 40~110 | 예 |  |  |  |  |
| Cookie pan | 1.150 (15.7lb) | 30~100 | 예 |  |  | 예 |  |
| Jar of pickles | 1.150 (15.7lb) | 80~150 | 예 |  |  |  |  |
| Ring | 1.150 (15.7lb) | 130~200 | 예 |  |  | 예 |  |
| Steering wheel | 1.150 (15.7lb) | 40~80 | 예 |  |  |  |  |
| Body | 1.100 (10.5lb) | 15~90 | 예 | 예 |  |  |  |
| Brush | 1.100 (10.5lb) | 20~90 | 예 |  |  |  |  |
| Candy | 1.100 (10.5lb) | 15~90 | 예 |  |  |  |  |
| Egg beater | 1.100 (10.5lb) | 30~110 | 예 |  |  | 예 |  |
| Magnifying glass | 1.100 (10.5lb) | 110~150 | 예 |  |  |  |  |
| Red soda | 1.070 (7.4lb) | 45~225 | 예 |  |  | 예 |  |
| Mug | 1.050 (5.2lb) | 60~170 | 예 |  |  |  |  |
| Old phone | 1.050 (5.2lb) | 120~160 | 예 |  |  |  |  |
| Knee | 1.050 (5.2lb) | 20~45 | 예 |  |  |  |  |
| Toilet paper | 1.050 (5.2lb) | 150~220 | 예 | 예 |  |  |  |
| Easter egg | 1.030 (3.1lb) | 55~130 | 예 |  |  |  |  |
| Teeth | 1.000 (0.0lb) | 150~210 | 예 |  |  |  |  |
| Dust pan | 1.000 (0.0lb) | 30~80 | 예 |  |  |  |  |
| Plastic fish | 1.000 (0.0lb) | 70~100 | 예 |  |  |  |  |
| Garbage lid | 1.000 (0.0lb) | 50~110 | 예 | 예 |  | 예 |  |
| Perfume bottle | 1.000 (0.0lb) | 120~260 | 예 |  |  |  |  |
| Pill bottle | 1.000 (0.0lb) | 40~100 | 예 |  |  |  |  |
| Plastic cup | 1.000 (0.0lb) | 30~90 | 예 |  |  |  |  |
| Hive | 1.000 (0.0lb) | 90~140 | 예 | 예 |  | 예 |  |
| Rubber Ducky | 1.000 (0.0lb) | 5~250 | 예 |  |  |  |  |
| Bone | 1.000 (0.0lb) | 17~37 | 예 |  |  |  |  |
| Ribcage | 1.000 (0.0lb) | 20~65 | 예 | 예 |  |  |  |
| Ear | 1.000 (0.0lb) | 7~35 | 예 |  |  |  |  |
| Foot | 1.000 (0.0lb) | 15~55 | 예 |  |  |  |  |
| Hand | 1.000 (0.0lb) | 10~30 | 예 |  |  |  |  |
| Tongue | 1.000 (0.0lb) | 8~40 | 예 |  |  |  |  |
| Toothpaste | 1.000 (0.0lb) | 35~120 | 예 |  |  |  |  |
| Toy cube | 1.000 (0.0lb) | 60~110 | 예 |  |  |  |  |
| Whoopie cushion | 1.000 (0.0lb) | 15~90 | 예 |  |  |  |  |
| Zed Dog | 1.000 (0.0lb) | 1~500 | 예 |  |  | 예 |  |
| Ammo | 1.000 (0.0lb) | - |  |  |  |  |  |
| box | 1.000 (0.0lb) | - |  |  |  |  |  |

## 종류 목록

`GrabbableObject`를 상속한 구현 클래스들이다. "판정"은 그 아이템의 결과를 확정하는 쪽을
가리킨다.

| 클래스 | 상점/고철 | 사용 방식 | 배터리 | 소음(몬스터가 듣는가) | 특이 기믹 / 판정 |
|---|---|---|---|---|---|
| `PhysicsProp` | 고철 | 없음 | 없음 | 떨어질 때만 (8 m / 0.5) | 로직 없는 순수 고철 |
| `AnimatedItem` | 고철 | 없음 | 없음 | **예** — 재생 중 0.75초마다 `noiseRange`/`noiseLoudness` | 같은 자리 반복 시 `timesPlayedInOneSpot` 누적, 주머니에선 절반 |
| `Shovel` | 상점 | 홀드(당겼다 놓기) | 없음 | 타격음 | `shovelHitForce = 1`. 0.35초 당기기 → 놓으면 스윙 → 0.13초 뒤 판정. `HitShovelServerRpc` / `ReelUpSFXServerRpc` |
| `KnifeItem` | 상점 | 1회 | 없음 | 타격음 | 삽과 같은 구조지만 즉발. `HitShovelServerRpc`(이름 재사용) |
| `ShotgunItem` | 고철(주움) | 1회, 좌/우로 안전장치 | 없음 | 총성(매우 큼) | `gunCompatibleAmmoID = 1410`, `shellsLoaded`, `safetyOn`, `misfireTimer = 30f`. `ShootGunServerRpc(position, forward)`로 **서버가 명중을 판정**. `GetItemDataToSave`로 장전 수 저장 |
| `GunAmmo` | 고철 | 없음 | 없음 | — | `ammoType` 하나뿐. 산탄총 재장전 재료 |
| `WalkieTalkie` | 상점 | 토글 + 송신 홀드 | **예** | 스피커로 재전송 | `SetPlayerSpeakingOnWalkieTalkieServerRpc` / `Unset…`. `TransmitOneShotAudio` 정적 함수를 다른 아이템들이 호출해 무전으로 소리를 흘린다 |
| `FlashlightItem` | 상점 | 토글 | **예** | 켤 때 7 m/0.4, 방전 시 13 m/0.65 | `flashlightInterferenceLevel`(2 이상이면 못 켬), 헬멧등 겸용(`usingPlayerHelmetLight`). `PocketFlashlightServerRpc(stillUsingFlashlight)` |
| `BoomboxItem` | 상점 | 토글 | **예** | **가장 큼** — 1초마다 16 m/0.9, `timesPlayedWithoutTurningOff` 누적 | 배터리 0.05 미만이면 피치가 떨어지고, `UseUpBatteries`에서 30프레임 피치 다운 후 정지 |
| `NoisemakerProp` | 고철 | 1회 | 없음 | **예** — `noiseRange`, 크기 `minLoudness`~`maxLoudness` 랜덤 | `randomMapSeed + 85` 시드로 클립·크기·피치를 모두가 똑같이 뽑는다. 0.6 이상이면 플레이어의 `timeSinceMakingLoudNoise`도 리셋 |
| `WhoopieCushionItem` | 고철 | 물리 충돌 | 없음 | **예** — 8 m/0.8 | `ActivatePhysicsTrigger`에서 0.2초 디바운스로 발동. 밟거나 떨어뜨리면 저절로 운다 |
| `SoccerBallProp` | 고철 | 좌/우로 차기 | 없음 | 굴러가는 소리 | `KickBallServerRpc(dest, playerWhoKicked, setInElevator, setInShipRoom)` — 목적지를 서버에 보내는 방식 |
| `LungProp` | 고철(최고가) | 없음 | 없음 | 뽑을 때 스파크 | 뽑는 순간 `DisconnectFromMachinery` 코루틴이 돌며 시설 전원이 나간다. `radMechEnemyType` 필드를 들고 있어 뽑기가 이벤트 트리거임을 드러낸다 |
| `RemoteProp` | 고철 | 1회 | 없음 | 8 m/0.4 | 우주선 조명을 **로컬 클라이언트에서만** 토글한다(`ToggleShipLightsOnLocalClientOnly`) |
| `KeyItem` | 상점 | 1회 | 없음 | 없음 | 3 m 레이캐스트로 `DoorLock`을 찾아 `UnlockDoorSyncWithServer()` 후 **자기 자신을 소멸**(`DespawnHeldObject`). 1회용 |
| `LockPicker` | 상점 | 1회(설치) | 없음 | 작동음 | 문에 붙여 두면 시간이 지나 열린다. `PlaceLockPickerServerRpc(doorObject, lockPicker1)` |
| `ExtensionLadderItem` | 상점 | 1회(설치) | 없음 | **예** — 펼칠 때 18 m/0.8, 접힐 때 17~18 m/0.7~0.8 | 놓으면 사다리로 변신. 두 손 아이템 |
| `JetpackItem` | 상점 | 홀드 | **예** | 엔진음 | `itemId == 13`으로 하드코딩 분기(제트팩 조작 중엔 슬롯 전환 금지). 착용자가 죽으면 `ExplodeJetpackServerRpc`로 폭발 |
| `RadarBoosterItem` | 상점 | 설치 후 원격 조작 | 없음 | 핑 소리(반복 시 누적) | 지도에 점을 추가하고, 터미널에서 이름으로 조작한다. `RadarBoosterFlashServerRpc`(섬광), `PingRadarBoosterServerRpc(playerWhoPlayedPing)`. 이름을 `GetItemDataToSave`로 저장 |
| `TetraChemicalItem` | 고철 | 홀드 | 없음(자체 `fuel = 1f`) | 가스 분사음 | 마시면 취한다(중독 연출). 연료 소진 시 `RunOutOfFuelServerRpc` |
| `SprayPaintItem` | 상점 | 홀드 + 좌/우로 흔들기 | 없음(자체 `sprayCanTank`) | 분사음 | 벽에 낙서를 남기고 잡초·시체식물을 죽인다. `GetItemDataToSave`가 `(int)(sprayCanTank * 100)` — **세이브에 잔량이 남는 대표 사례** |
| `StunGrenadeItem` | 상점 | 핀 뽑기 → 던지기 | 없음 | 폭발음 | `TimeToExplode = 2.25f`, `chanceToExplode = 100f`, `explodeOnCollision`, `dontRequirePullingPin` 등이 전부 인스펙터 변수라 같은 클래스로 여러 폭발물을 만든다. `SetExplodeOnThrowServerRpc` |
| `BeltBagItem` | 상점 | 좌/우로 담기·꺼내기 | 없음 | 지퍼음 | **가방 안에 아이템 15개까지**(`objectsInBag.Count >= 15`). 가방 안 가방(`insideAnotherBeltBag`)도 가능. `TryAddObjectToBagServerRpc(netObjectRef, playerWhoAdded)`, `GrabItemInBagServerRpc` |
| `GiftBoxItem` | 고철 | 1회(열기) | 없음 | 없음 | 안에 다른 아이템을 품는다. `OpenGiftBoxServerRpc`가 **서버에서** `objectInPresent`를 스폰하고 `SetScrapValue(objectInPresentValue)` 후 `Spawn()`. 내용물은 `GetItemDataToSave`로 저장 |
| `HauntedMaskItem` | 고철 | 1회(쓰기) | 없음 | 없음 | `IVisibleThreat`도 구현. 얼굴에 붙으면(`AttachServerRpc`) 착용자를 죽이고 `CreateMimicServerRpc(inFactory, playerPositionAtDeath)`로 모방 몬스터를 만든다 |
| `KiwiBabyItem` | 고철(생물) | 없음(들면 반응) | 없음 | **예** — 비명 | `SetScreamingServerRpc(scream)`으로 울음 상태를 동기화하고, 오래 들고 있으면 `BreakOutServerRpc`로 탈출한다. `ReactToSellingItemOnCounter`를 덮어써 판매 순간에도 반응 |
| `RagdollGrabbableObject` | 시체 | 없음 | 없음 | 없음 | `bodyID` + `DeadBodyInfo`. 적이 들 수 있어(`heldByEnemy`, `DiscardItemFromEnemy`) 별도 처리가 붙는다. 레이더 아이콘은 생성 즉시 파괴 |
| `ClipboardItem` | 상점(고정 비품) | 좌/우로 페이지 넘김 | 없음 | 종이 소리 | `currentPage`를 1~4로 제한. `truckManual`이면 트럭에 자동으로 달라붙는다 |
| `BinocularsItem` | 상점 | — | 없음 | 없음 | **클래스 본문이 완전히 비어 있다.** 확대 동작은 전적으로 프리팹/애니메이션 쪽에 있다(추정) |
| `MicrowaveItem` | 시설물 | 문 열고 닫기 | 없음 | 0.25초 뒤 8 m/0.6 | **`MonoBehaviour`이며 `GrabbableObject`가 아니다.** 반경 5 m 안의 `GrabbableObject`에 `rotateObject`를 켜서 회전시키는 소품 |
| `EventWhenDroppedItem` | 고철 | 떨어질 때 | 없음 | **예** — `noiseRange`/`noiseLoudness`, `timesPlayedInSameSpot` 누적 | 떨어뜨리는 것 자체가 사건인 아이템 |
| `ClockProp` | 고철 | 없음 | 없음 | 시계 소리(추정) | — |

`Item` 하나에 있는 `syncGrabFunction`/`syncUseFunction`/`syncDiscardFunction`/
`syncInteractLRFunction` 네 개의 토글이 위 RPC들의 발신 여부를 데이터로 끈다. 즉 "이
아이템은 사용을 굳이 동기화할 필요가 없다"를 코드가 아니라 자산에서 정한다.

---

## 인벤토리·잡기·무게

### 슬롯

`PlayerControllerB.ItemSlots`는 `new GrabbableObject[4]` — **주 슬롯 4칸**이다. 여기에
더해 `ItemOnlySlot` 하나가 있고, 이것은 슬롯 인덱스 **50**이라는 매직 넘버로 다뤄진다.
`FirstEmptyItemSlot()`이 정하는 규칙은 이렇다. 유틸리티 벨트가 비어 있고, 잡으려는 것이
고철이 아니며, 두 손 아이템이 아니고, `disallowUtilitySlot`도 아니면 **무조건 벨트로
간다**. 그렇지 않으면 현재 슬롯이 비었을 때 그 자리, 아니면 앞에서부터 첫 빈칸이다.
즉 벨트는 "한손 도구 전용 5번째 칸"이며 고철은 절대 못 들어간다.

### 전환 딜레이

`timeSinceSwitchingSlots` 하나가 여러 문턱으로 쓰인다.

| 동작 | 필요 경과 시간 |
|---|---|
| 아이템 사용 | 0.075초 |
| 숫자키·Q/E 슬롯 지정 | 0.2초 |
| 휠 스크롤 전환 | 0.3초 |

전환은 두 손 아이템을 들고 있거나(`twoHanded`), 잡기 애니메이션 중이거나, 채팅·터미널·
특수 애니메이션 중이거나, 제트팩(`itemId == 13`) 조작 중이면 전부 막힌다.

### 잡기 절차 (`BeginGrabObject`)

카메라에서 `grabDistance = 5f` 레이캐스트를 쏴 태그가 `"PhysicsProp"`인 콜라이더를 찾는다.
두 손 아이템을 들고 있거나 `sinkingValue > 0.73f`(늪에 빠짐)면 즉시 실패한다. 성공하면
클라이언트가 **먼저 로컬에서 확정**하고 — 무게를 더하고 애니메이션을 켜고 —
그 다음에 `GrabObjectServerRpc(networkObject)`를 보낸다. 잡기 시간은
`itemProperties.grabAnimationTime`이고, 0이면 0.4초를 쓴다. 서버가 거부하면
`GrabObjectClientRpc(grabValidated: false, …)`가 돌아오고 클라이언트는 더했던 무게를
그대로 빼서 되돌린다.

### 무게 — 공식 그대로

`carryWeight`는 1.0에서 시작하고, 아이템은 **1을 넘는 초과분만** 누적된다.

```csharp
carryWeight = Mathf.Clamp(carryWeight + (item.itemProperties.weight - 1f), 1f, 10f);
```

무게 1 이하인 아이템은 아무리 들어도 느려지지 않는다는 뜻이다. 상한은 10.

속도에 미치는 영향은 세 군데다.

```csharp
float num3 = movementSpeed / carryWeight;              // movementSpeed = 0.5f (코드 기본값)
// 가속(walkForce의 MoveTowards 최대 변화량)
//   걷기:  10f / carryWeight
//   달리기: 5f / (carryWeight * 1.5f)
sprintMeter -= Time.deltaTime / sprintTime * carryWeight * num3;   // sprintTime = 5f
```

즉 최고 속도가 무게에 **반비례**하고, 가속도 반비례하며, 스태미나 소모는 무게에
**비례**한다. 무거우면 느리게 가속해서 느리게 달리고 빨리 지친다. 세 겹으로 걸리기
때문에 체감 페널티는 `1/carryWeight` 하나보다 훨씬 크다.

HUD 표시는 파운드로 환산한다.

```csharp
float num = Mathf.RoundToInt(Mathf.Clamp(carryWeight - 1f, 0f, 100f) * 105f);
weightCounter.text = $"{num} lb";
weightCounterAnimator.SetFloat("weight", num / 130f);
```

애니메이터가 130 lb를 1.0으로 삼으므로 게임이 상정한 "꽉 찬 무게"는 대략 `carryWeight`
2.24 언저리다(추정). 실제로 몬스터의 위협도 판정(`IVisibleThreat.GetInterestLevel`)도
`carryWeight > 1.22f`에서 +1, `> 1.5f`에서 +1을 준다 — **무거운 플레이어가 더 잘 표적이
된다.** 다른 플레이어를 밀어내는 힘도 `Mathf.Lerp(2f, 7f, carryWeight / 2f)`로 무게에
비례한다.

무게를 되돌리는 지점은 네 군데이며 전부 같은 `Clamp(… - (weight - 1f), 1f, 10f)` 식을
쓴다. 잡기 실패 롤백, `SetObjectAsNoLongerHeld`(버리기), `DespawnHeldObjectOnClient`(던져
없애기), 배치(`PlaceObject`)다. 죽거나 라운드가 바뀔 때는 `carryWeight = 1f`로 통째로
리셋한다.

### 배달 (`ItemDropship.cs`)

주문이 있으면(`orderedItemsFromTerminal.Count > 0`) 서버가 `shipTimer`를 굴린다. 단
**우주선이 착륙해 있는 동안만** 증가한다 — 궤도에 있으면 배달이 오지 않는다. 첫 주문은
`playersFirstOrder` 플래그로 타이머를 20에서 시작시켜 절반만 기다리게 한다.

| 단계 | 시간 |
|---|---|
| 주문 → 배달선 착륙 | `shipTimer > 40f` (첫 주문은 20에서 시작하므로 실질 20초) |
| 착륙 → 자동 이륙 | 30초 |
| 차량 배달 시 결속 해제 대기 | 6초씩 두 단계 |

문이 열리면 서버가 `terminalScript.buyableItemsList[itemsToDeliver[i]].spawnPrefab`를
`playersManager.propsContainer` 아래에 인스턴스화하고 `NetworkObject.Spawn()`한다.
못 가져간 물건이 있어도 배달선은 30초 뒤 그냥 떠나며 `itemsToDeliver`를 비운다.

---

## 스크랩 가치와 판매

### 값이 정해지는 순간

값은 서버의 `RoundManager.SpawnScrapInLevel()`에서 딱 한 번 정해진다. 개수부터 보면,

```csharp
int num = (int)((float)AnomalyRandom.Next(currentLevel.minScrap, currentLevel.maxScrap) * scrapAmountMultiplier);
if (currentDungeonType == 4) num += 6;
```

`scrapValueMultiplier`와 `scrapAmountMultiplier`는 둘 다 코드 기본값 1f이며, 날씨·행성
난이도가 이걸 조정하는 손잡이다. 종류는 `currentLevel.spawnableScrap`의 `rarity`를
가중치로 뽑는다(`GetRandomWeightedIndex`).

값 추첨은 한 줄이다.

```csharp
list.Add((int)((float)AnomalyRandom.Next(ScrapToSpawn[i].minValue, ScrapToSpawn[i].maxValue) * scrapValueMultiplier));
```

여기에 **"한 종류만 쏟아지는" 희귀 이벤트**가 붙는다. `AnomalyRandom.Next(0, 500) <= 20`
(약 4%)이면 한 종류를 골라 맵 전체를 그것으로 채운다. 이때는 값이
`Mathf.Clamp(…, 50, 170)`으로 좁게 잘리고, 이벤트 후 총합을 보정한다 — 총합이 4500을
넘으면 전부 ×0.7, 기준값(한손 600, 두 손 1500) 미만이면 전부 ×1.4.

값 동기화는 게으르다. 서버가 스폰한 뒤 **11초 기다렸다가**
`SyncScrapValuesClientRpc(spawnedScrap[], allScrapValue[])`를 한 번에 보낸다. 클라이언트는
받아서 `SetScrapValue()`를 부르고, 동시에 `randomMapSeed + 210` 시드로 `meshVariants`·
`materialVariants`를 골라 외형까지 맞춘다. `totalScrapValueInLevel`은 이 시점에 확정된다.

플레이어가 처음 아이템을 집으면(`EquipItem`, `hasBeenHeld`가 false일 때)
`RoundManager.valueOfFoundScrapItems += scrapValue`가 올라간다. 이 값은 몬스터 스폰
난이도 판정에 쓰인다 — 마감 2일 이내에 쿼터의 80%를 모았거나 맵 총액의 65%를 찾았으면
추가 몬스터가 나온다.

### 판매 (`DepositItemsDesk.cs`)

데스크에 물건을 올리는 것은 `PlaceItemOnCounter()`이며, 카운터 위 아이템이 150개
미만일 때만 받는다. 클라이언트가 카운터 콜라이더 안의 임의 지점을 뽑아
`AddObjectToDeskServerRpc(netObject)`를 보내고 `DiscardHeldObject(placeObject: true, …)`로
내려놓는다.

정산은 서버 전용이다.

```csharp
// 고철이 아니면 itemId == 14 (레이더 아이콘으로 보아 열쇠로 추정)일 때만 값을 친다
num = (int)((float)num * StartOfRound.Instance.companyBuyingRate);
terminal.groupCredits += num;
SellItemsClientRpc(num, terminal.groupCredits, itemsOnCounterAmount, StartOfRound.Instance.companyBuyingRate);
```

즉 **`isScrap`이 아닌 물건은 팔아도 0원**이다. 손전등이든 삽이든 회사는 사주지 않는다.
정산 후 `TimeOfDay.Instance.quotaFulfilled += profit`로 쿼터에 반영된다.

### 매입률이 마감일에 따라 변하는 법

`TimeOfDay.SetBuyingRateForDay()`가 하루가 바뀔 때마다 다시 계산한다.

```csharp
daysUntilDeadline = (int)Mathf.Floor(timeUntilDeadline / totalTime);
if (daysUntilDeadline == 0) { companyBuyingRate = 1f; return; }
float num = 0.3f;
float num2 = (1f - num) / (float)quotaVariables.deadlineDaysAmount;
companyBuyingRate = num2 * (deadlineDaysAmount - daysUntilDeadline) + num;
```

`deadlineDaysAmount = 4`(코드 기본값)이므로 하루에 0.175씩 오른다.

| 남은 날 | 매입률 |
|---|---|
| 3일 | 47.5% |
| 2일 | 65% |
| 1일 | 82.5% |
| 0일(마감 당일) | **100%** |

`ResetShip()`은 0.3으로 되돌린다. 설계 의도는 명확하다 — **모아뒀다가 마감 날 한 번에
파는 것이 압도적으로 이득**이고, 그래서 플레이어는 위험한 물건을 오래 이고 다닌다.
`DisplayCompanyBuyingRate`는 `Update()`마다
`$"{Mathf.RoundToInt(companyBuyingRate * 100f)}%"`를 찍는 열 줄짜리 컴포넌트다.

회사의 인내심(`patienceLevel`)은 별개 축이다. `INoiseListener`를 구현해 소음을 들으면
줄고, 이익률이 낮으면(`profit / 물건 수 <= 3f && patienceLevel <= 2f`) 더 나빠지며,
0 이하가 되면 공격한다. 판매를 성사시키면 `patienceLevel += 3f` 또는 `+= 6f`로 회복한다.

### 쿼터

`QuotaSettings`의 코드 기본값은 `startingQuota = 300`, `startingCredits = 60`,
`deadlineDaysAmount = 4`, `increaseSteepness = 4f`, `baseIncrease = 200f`,
`randomizerMultiplier = 1f`다. 증가 공식은,

```csharp
float num2 = Mathf.Clamp(1f + timesFulfilledQuota * (timesFulfilledQuota / increaseSteepness), 0f, 10000f);
num2 = baseIncrease * num2 * (randomizerCurve.Evaluate(num3) * randomizerMultiplier + 1f);
profitQuota = (int)Mathf.Clamp(profitQuota + num2, 0f, 1E+09f);
int overtimeBonus = (quotaFulfilled - profitQuota) / 5 + 15 * daysUntilDeadline;
```

즉 증가폭이 달성 횟수의 제곱에 비례해 가속한다. 초과 달성분은 5분의 1만, 남은 날은
하루당 15크레딧이 보너스로 돌아온다.

### 스캔 HUD

`ScrapItemHUDDisplay`는 애니메이터·컨테이너·텍스트 참조만 묶은 `[Serializable]` 구조체다.
실제 합계는 `HUDManager.totalScrapScanned`가 들고 있으며, **2개 이상 스캔했을 때만**
표시되고 `Mathf.MoveTowards(…, 1500f * Time.deltaTime)`로 숫자가 굴러 올라간다.

---

## 상점(터미널)

터미널은 텍스트 어드벤처 파서다. `TerminalKeyword`가 단어 하나(`word`)와
동사 여부(`isVerb`), 그리고 `compatibleNouns`(= `CompatibleNoun[]`, 각각 `noun` 키워드 +
`result` 노드)를 들고 있다. `TerminalNode`가 실제 화면이며 `displayText`,
`clearPreviousText`, `maxCharactersToType`(기본 25)에 더해 구매 정보를 담는다 —
`buyItemIndex`(-1이 기본), `buyVehicleIndex`, `isConfirmationNode`, `buyRerouteToMoon`,
`shipUnlockableID`, `buyUnlockable`, `returnFromStorage`, `itemCost`. 도감·로그
(`creatureFileID`, `storyLogFileID`)와 표시 자산(`displayTexture`, `displayVideo`)도
같은 노드에 얹혀 있다. **동사 + 명사 → 노드**라는 표 하나로 상점·항해·도감이 전부
표현된다.

가격 계산은 이렇다.

```csharp
totalCostOfItems = (int)((float)buyableItemsList[node.buyItemIndex].creditsWorth
                       * ((float)itemSalesPercentages[node.buyItemIndex] / 100f)
                       * (float)playerDefinedAmount);
```

`itemSalesPercentages`는 `buyableItemsList.Length + buyableVehicles.Length` 길이의 int
배열로, 100이 정가다. 세일은 `SetItemSales()`가 정한다.

```csharp
System.Random random = new System.Random(StartOfRound.Instance.randomMapSeed + 90);
int num2 = Mathf.Max(random.Next(-10, 5), 0);       // 세일 품목 수: 대개 0, 최대 4
int maxValue2 = Mathf.Clamp(buyableItemsList[num3].highestSalePercentage, 0, 90);
int i2 = RoundToNearestTen(100 - random.Next(0, maxValue2));
```

`random.Next(-10, 5)`에 `Max(…, 0)`을 씌우므로 **약 2/3의 확률로 세일이 아예 없고**,
있어도 1~4개다. 할인율은 10 단위로 반올림되며 상한은 아이템별 `highestSalePercentage`
(코드 기본값 80), 차량은 고정 80이다. 10월 23일에는 범위가 `(-2, 9)`로 바뀌어 세일이
훨씬 잦아진다 — 게임 출시 기념일로 보인다(추정). 시드가 `randomMapSeed + 90`이므로
세일 목록은 세이브마다 고정이고 모든 클라이언트가 같은 값을 본다.

주문 제한은 두 가지다. 한 번에 12개를 넘을 수 없고(`playerDefinedAmount > 12`),
이미 배달 중인 것을 합쳐도 12개를 넘을 수 없다(`num2 + numberOfItemsInDropship > 12f`).
차량 주문 중에는 물건을 못 사고, 그 반대도 막힌다. 크레딧은
`Mathf.Clamp(groupCredits - totalCostOfItems, 0, 10000000)`으로 깎인다.

구매가 확정되면 `orderedItemsFromTerminal`에 인덱스를 그 수만큼 밀어 넣고
`numberOfItemsInDropship`을 올린다. 호스트가 아니면
`SyncBoughtItemsWithServer(orderedItemsFromTerminal.ToArray(), numberOfItemsInDropship)`,
호스트면 `SyncGroupCreditsClientRpc(groupCredits, numberOfItemsInDropship)`로 퍼뜨린다.
`buyItemIndex == -7`은 특수 묶음(콤보 세트)으로, 인덱스 5·1×4·6×4를 한꺼번에 넣고
`numberOfItemsInDropship += 9`를 한다.

### 우주선 개조품과 슈트

`UnlockablesList`는 `List<UnlockableItem>` 하나뿐이고, 세이브·터미널·배치가 전부 이
리스트의 **인덱스**(`shipUnlockableID`)로 물건을 가리킨다. `UnlockableItem`은
`[Serializable]` 클래스이며 `unlockableName`, `prefabObject`, `unlockableType`,
`shopSelectionNode`, `alwaysInStock`, `IsPlaceable`, `hasBeenMoved`, `placedPosition`,
`placedRotation`, `luckValue`, `inStorage`, `canBeStored`(기본 true), `maxNumber`(기본 1),
`hasBeenUnlockedByPlayer`, `suitMaterial`, `headCostumeObject`, `lowerTorsoCostumeObject`,
`alreadyUnlocked`, `spawnPrefab`(기본 true), `jumpAudio`를 가진다. **가구와 슈트가 같은
구조체를 공유**하며 `suitMaterial`이 채워져 있으면 슈트로 동작한다.

구매는 `StartOfRound.BuyShipUnlockableServerRpc(shipUnlockableID, groupCredits)`다. 이미
가진 물건은 창고(`inStorage`)에서 무료로 꺼내며, 이때는
`ReturnUnlockableFromStorageServerRpc`를 탄다. 슈트 교체는 `UnlockableSuit`가 맡는데
`NetworkVariable<int> syncedSuitID`로 옷걸이 자체의 상태를 동기화하고, 입는 행위는
`SwitchSuitServerRpc(playerID)` → `SwitchSuitClientRpc(playerID)`로 퍼뜨린다. 적용은
정적 함수 `SwitchSuitForPlayer(player, suitID)`가 모델 4개(본체·LOD1·LOD2·팔)의 머티리얼을
갈아끼우고 머리·하체 코스튬 오브젝트를 교체하는 방식이다.

### 배치 모드

`PlaceableShipObject`는 배치 가능한 물건에 붙는 MonoBehaviour로 `unlockableID`,
`parentObject`(`AutoParentToShip`), `mainMesh`, `placeObjectCollider`, `yOffset`,
`wallOffset`, `AllowPlacementOnWalls`, `AllowPlacementOnCounters`(기본 true) 등을 가진다.

`ShipBuildModeManager`는 싱글턴이며 고스트 메시(초록/빨강 머티리얼)를 띄워 미리보기를
한다. 카메라에서 4 m 레이캐스트를 쏘고 안 맞으면 4 m 지점에서 아래로 20 m를 쏴 바닥을
찾는다. 마스크는 `placementMask = 2305`, `placementMaskAndBlockers = 134220033`이며,
겹침 판정은 콜라이더의 **57%로 줄인 박스**로 한다.

```csharp
bool flag = Physics.CheckBox(ghostObject.position, currentCollider.size * 0.5f * 0.57f,
                             Quaternion.Euler(ghostObject.eulerAngles),
                             placementMaskAndBlockers, QueryTriggerInteraction.Ignore);
```

확정은 `PlaceShipObjectServerRpc(newPosition, newRotation, objectRef, playerWhoMoved)` →
`PlaceShipObjectClientRpc(…)`, 창고에 넣는 것은 `StoreObjectServerRpc(objectRef,
playerWhoStored)` → `StoreShipObjectClientRpc(objectRef, playerWhoStored, unlockableID)`다.

`BuyableVehicle`은 `vehiclePrefab`, `secondaryPrefab`, `vehicleDisplayName`, `creditsWorth`
네 필드짜리 `[Serializable]` 클래스로, 차량은 배달선이 아니라 별도 경로로 내려온다.

---

## 저장

세이브는 **호스트가 접속을 끊을 때** `GameNetworkManager.SaveItemsInShip()`에서 일어난다.
ES3(Easy Save 3)로 네 개의 배열을 쓴다.

| 키 | 내용 |
|---|---|
| `shipGrabbableItemIDs` | `AllItemsList.itemsList`에서의 **인덱스** |
| `shipGrabbableItemPos` | 월드 좌표 `Vector3` |
| `shipScrapValues` | `isScrap`인 아이템만, 등장 순서대로 `scrapValue` |
| `shipItemSaveData` | `saveItemVariable`인 아이템만, `GetItemDataToSave()`가 준 **int 하나** |

저장에서 빠지는 것은 `allItemsList`에 없는 아이템, `deactivated`, `itemUsedUp`,
`spawnPrefab`이 비어 있는 것이다. 순회는 `i <= StartOfRound.Instance.maxShipItemCapacity`
(**45**)에서 멈춘다 — 저장된 개수가 아니라 순회 인덱스 기준이라 발견 순서에 따라
45개보다 적게 저장될 수 있다.

**저장되지 않는 것**이 더 중요하다. 배터리 잔량(`insertedBattery`), 회전, 들고 있던
상태는 전부 사라진다. 아이템별 커스텀 상태는 오직 `int` 하나뿐이며, 스프레이는
`(int)(sprayCanTank * 100f)`처럼 스스로 압축해 넣는다.

복원은 `StartOfRound.LoadShipGrabbableItems()`다. 저장 위치가 `shipBounds` 밖이면
`playerSpawnPositions[1]` 근처로 끌어오고(x ±0.7, z +2, y +0.5), `elevatorTransform` 아래에
인스턴스화한 뒤 `scrapPersistedThroughRounds = true`, `isInElevator = true`,
`isInShipRoom = true`를 세우고 레이더 아이콘을 파괴한 다음 `SetScrapValue` →
`LoadItemSaveData` → `NetworkObject.Spawn()` 순서로 되살린다.

### 언제 사라지는가

라운드가 끝날 때 `RoundManager.DespawnPropsAtEndOfRound(despawnAllItems)`가 판정한다.

```csharp
if (despawnAllItems || (!array[l].isHeld && !array[l].isInShipRoom) || array[l].deactivated
    || (StartOfRound.Instance.allPlayersDead && array[l].itemProperties.isScrap))
{ /* despawn */ }
else { array[l].scrapPersistedThroughRounds = true; }
```

읽어보면 **전멸 시 사라지는 것은 `isScrap`인 물건뿐**이다. 구매한 도구·장비는 우주선
안에 있으면 전멸해도 남는다. "다 죽으면 전부 잃는다"는 통념보다 규칙이 관대하며, 잃는
것은 그 원정에서 번 돈뿐이라는 뜻이다. 바닥에 굴러다니는 물건도 `isInShipRoom`이 아니면
전멸과 무관하게 사라진다.

벨트백은 예외 처리를 받는다. 가방이 우주선 안이거나 들려 있으면 **가방 속 물건도**
`isInElevator`/`isInShipRoom`을 true로 물려받아 함께 살아남는다.

파산해 처음부터 다시 시작할 때는 `ResetShip()`이 쿼터를 300으로, 크레딧을 60으로,
매입률을 0.3으로 되돌리고 `randomMapSeed++`를 한다.

---

## 네트워크

아이템 하나가 `NetworkObject` 하나다. 스폰은 언제나 서버가 하고
(`component2.Spawn()`), 소유권은 **잡는 순간 그 플레이어에게 넘어간다**.

```csharp
networkObject.ChangeOwnership(actualClientId);
GrabObjectClientRpc(grabValidated: true, networkObject);
```

흐름은 일관되게 "로컬 예측 → 서버 승인 → 전체 통보"다. 잡기는
`BeginGrabObject`(로컬에서 무게·애니메이션을 즉시 반영) → `GrabObjectServerRpc` →
`GrabObjectClientRpc(grabValidated, …)`이고, 거부되면 클라이언트가 스스로 되돌린다.
버리기는 `SetObjectAsNoLongerHeld(...)`를 로컬에서 실행한 뒤
`ThrowObjectServerRpc(netObj, droppedInElevator, droppedInShipRoom, targetFloorPosition,
floorYRot)` → `ThrowObjectClientRpc(…)`다.

**들고 있는 동안의 위치는 네트워크로 보내지 않는다.** `LateUpdate()`가 매 프레임
`parentObject`(내 손 또는 원격 플레이어의 `serverItemHolder`)의 위치·회전에
`itemProperties.positionOffset`/`rotationOffset`을 더해 붙일 뿐이다. 떨어질 때도
좌표를 흘려보내지 않고, **목표 바닥 좌표 하나만** 보낸 뒤 각 클라이언트가
`FallWithCurve()`로 `StartOfRound.objectFallToGroundCurve`(또는 낙차 5 m 초과면
`objectFallToGroundCurveNoBounce`)를 따라 똑같이 보간한다. 즉 아이템에는 이동 동기화가
사실상 없고, **부모 관계와 목표점만 동기화**한다.

아이템은 항상 두 컨테이너 중 하나 아래에 있다 — `StartOfRound.propsContainer`(맵) 또는
`elevatorTransform`(우주선). 우주선이 이동하면 통째로 따라간다.

배터리도 소유자만 시뮬레이션한다. `Update()`의 소모는 `if (base.IsOwner)` 안에 있고,
동기화는 버릴 때 `SyncBatteryServerRpc((int)(insertedBattery.charge * 100f))` 한 번이
전부다. 받는 쪽은 `insertedBattery = new Battery(num <= 0f, num)`으로 새로 만든다.
**들고 있는 동안 남의 배터리 잔량은 아무도 모른다.**

랜덤도 네트워크 대신 시드로 맞춘다. `AnomalyRandom`, `randomMapSeed + 210`(외형 변주),
`+ 90`(세일), `+ 85`(소음기), `- 10`(붐박스 음악) 등 모든 결정론적 선택이 같은 시드에서
나오므로 RPC 없이 결과가 일치한다.

---

## 수치

| 항목 | 값 | 출처 |
|---|---|---|
| 인벤토리 슬롯 | 4 + 유틸리티 벨트 1(인덱스 50) | `ItemSlots = new GrabbableObject[4]` |
| 잡기 사거리 | 5 m | `grabDistance = 5f` |
| 잡기 동작 시간 | `grabAnimationTime`, 0이면 0.4초 | `BeginGrabObject` |
| 슬롯 전환 쿨다운 | 스크롤 0.3초 / 지정 0.2초 / 사용 0.075초 | `timeSinceSwitchingSlots` |
| 기본 이동 속도 | `movementSpeed = 0.5f` | `PlayerControllerB` |
| 무게 누적 | `Clamp(carryWeight + (weight - 1), 1, 10)` | 4곳 동일 |
| 속도 | `movementSpeed / carryWeight` | `Update` |
| 가속 | 걷기 `10 / carryWeight`, 달리기 `5 / (carryWeight * 1.5)` | `walkForce` |
| 스태미나 | `Δt / 5f * carryWeight * num3` | `sprintTime = 5f` |
| 무게 HUD | `round(clamp(carryWeight-1, 0, 100) * 105)` lb, 애니메이터는 /130 | `HUDManager` |
| 위협도 문턱 | `carryWeight > 1.22`, `> 1.5` | `GetInterestLevel` |
| 배터리 소모(지속) | `charge -= Δt / batteryUsage` (기본 15초) | `GrabbableObject.Update` |
| 배터리 소모(1회) | `charge -= batteryUsage` | `UseItemBatteries` |
| 충전기 | 0.75초 후 `Battery(false, 1f)` + `SyncBatteryServerRpc(100)` | `ItemCharger` |
| 고철 값 | `(int)(Random(minValue, maxValue) * scrapValueMultiplier)` | `SpawnScrapInLevel` |
| 고철 개수 | `(int)(Random(minScrap, maxScrap) * scrapAmountMultiplier)`, 던전 4형은 +6 | 같은 곳 |
| 단일종 이벤트 | 확률 `Next(0,500) <= 20`, 값 50~170, 총합 >4500이면 ×0.7 / 기준 미만이면 ×1.4 | 같은 곳 |
| 값 동기화 지연 | 11초 | `waitForScrapToSpawnToSync` |
| 스캔 노드 | maxRange 7, minRange 5, 시야 필요 | `ScanNodeProperties` |
| 매입률 | 0일 100%, 그 외 `0.175 * (4 - 남은날) + 0.3` | `SetBuyingRateForDay` |
| 판매 대상 | `isScrap`만 (예외: `itemId == 14`) | `SellItemsOnServer` |
| 데스크 수용량 | 150개 | `PlaceItemOnCounter` |
| 시작 쿼터 / 크레딧 / 마감 | 300 / 60 / 4일 | `QuotaSettings` |
| 쿼터 증가 | `200 * clamp(1 + n*(n/4)) * (curve + 1)` | `SetNewProfitQuota` |
| 초과 보너스 | `(달성 - 쿼터)/5 + 15 * 남은날` | 같은 곳 |
| 상점 가격 | `creditsWorth * salesPercentage/100 * 수량` | `Terminal` |
| 세일 품목 수 | `max(Random(-10, 5), 0)` → 0~4개 | `SetItemSales` |
| 최대 할인 | 아이템별 `highestSalePercentage`(기본 80, 상한 90), 차량 80 | 같은 곳 |
| 주문 상한 | 1회 12개, 배달 중 포함 12개 | `LoadNewNodeIfAffordable` |
| 배달 시간 | 40초(첫 주문은 20에서 시작), 착륙 후 30초 뒤 이륙 | `ItemDropship` |
| 벨트백 용량 | 15개 | `objectsInBag.Count >= 15` |
| 세이브 아이템 상한 | 45 (`maxShipItemCapacity`) | `SaveItemsInShip` |
| 붐박스 소음 | 1초마다 16 m / 0.9 | `BoomboxItem` |
| 손전등 소음 | 켤 때 7 m / 0.4, 방전 13 m / 0.65 | `FlashlightItem` |
| 낙하 소음 | 8 m / 0.5 | `PlayDropSFX` |
| 소음 감쇠 | 닫힌 우주선 안이면 사거리 ÷2 | `PlayAudibleNoise` |

---

## CL9 시사점

관찰만 적는다. 결정은 하지 않는다.

**데이터 자산의 크기.** `Item` 하나가 무게·값·배터리·애니메이션 문자열·효과음·오프셋·
네트워크 동기화 토글까지 50개 남짓의 필드를 전부 들고 있고, 로직은 하나도 없다.
CL9이 "종류당 DataAsset 하나"로 가는 방향과 같은 모양이며, 참고할 만한 점은 **동기화
여부까지 데이터에 넣었다**는 것(`syncGrabFunction` 등 4개)과 **저장 슬롯이 int 하나**
(`saveItemVariable` + `GetItemDataToSave`)라는 극단적 절약이다.

**무게 페널티가 세 겹이다.** 최고 속도(반비례), 가속(반비례), 스태미나(비례)에 동시에
걸린다. CL9의 "짐을 들면 느려진다"가 최고 속도만 건드리는 설계라면 체감이 상당히 다를
것이다. 그리고 `weight - 1`만 누적하므로 **가벼운 물건은 아무리 많이 들어도 공짜**라는
문턱 구조를 쓰고 있다. 계란 같은 소형 재료를 여러 개 드는 게임이라면 이 문턱이 어디에
놓이는지가 곧 게임의 리듬이다.

**슬롯이 4 + 1이고, 전용 슬롯이 종류로 갈린다.** 유틸리티 벨트는 `isScrap`이 아니고
두 손이 아닌 물건만 받는다. 즉 "재료 칸"과 "도구 칸"이 자산의 플래그 하나로 분리된다.
CL9에 손전등·헤드램프·수레가 있고 재료를 따로 든다면 같은 형태의 분리가 가능하다.
전환 딜레이는 0.075/0.2/0.3초 세 단계로 상황별로 다르며, CL9이 정한 1.2초보다 훨씬
짧다 — 그쪽은 교체 자체를 위험으로 삼는 설계다.

**쿼터 형태의 차이.** Lethal Company는 금액 쿼터라 매입률(0.3 → 1.0)을 시간 축으로
써서 "마감까지 참기"라는 긴장을 만든다. CL9은 주문서 쿼터("오늘 오믈렛 3, 꿀버터 1")를
택했으므로 이 손잡이가 그대로 오지 않는다. 다만 **같은 물건이 날마다 다른 가치를
가진다**는 구조 자체는 주문서에도 옮길 수 있다(추정) — 예를 들어 오늘 주문에 없는
재료의 처분 가치를 낮게 두는 식.

**값이 서버에서 한 번, 11초 뒤에 동기화된다.** 리슨 서버 호스트 판정 구조에서 값 추첨을
게으르게 몰아 보내는 방식이며, 그 사이 클라이언트는 값을 0으로 본다. 스캔 표시가
`SetScrapValue()`에서 `subText`를 덮어쓰는 방식이라 값과 표시가 한 지점에서만 만난다.

**아이템 이동을 동기화하지 않는다.** 들고 있으면 부모 트랜스폼에 붙이고, 놓으면 목표
바닥 좌표 하나만 보내 각자 커브로 보간한다. 계란이 굴러가고 깨지는 CL9이라면 이
방식으로는 부족한 지점(깨짐 판정이 어디서 나는가)이 생긴다는 점이 눈에 띈다.

**전멸 규칙이 생각보다 좁다.** 잃는 것은 `isScrap`뿐이고 구매한 장비는 우주선에 있으면
남는다. "다 잃는다"는 인상은 실제 규칙이 아니라 그 라운드의 수익만 날아가는 것에서
온다. 재료(계란·고기)와 장비(손전등·수레)를 다른 규칙에 두는 근거가 될 수 있다.

**소음이 아이템의 1급 속성이다.** `PlayAudibleNoise(위치, 사거리, 크기,
같은자리반복횟수, 닫힌우주선안인가, 소음ID)` 하나로 통일돼 있고, 같은 자리에서
반복하면 카운터가 올라가 몬스터가 더 강하게 반응한다. 닫힌 공간이면 사거리를 절반으로
줄이는 것도 이 한 함수 안에 있다. 동물이 주인인 장소에서 훔치는 CL9이라면 이 시그니처가
거의 그대로 쓰일 만한 모양이다.

**상점 세일이 대체로 없다.** `max(Random(-10, 5), 0)`은 약 2/3의 확률로 0이다. 세일은
드물게 오는 사건이지 상시 변동이 아니며, 시드 고정이라 세이브마다 같다. 배달도 즉시가
아니라 20~40초 지연이고 착륙 30초 뒤 떠난다 — **구매가 즉시 손에 들어오지 않는다**는
점이 준비 단계를 하나의 국면으로 만든다.
