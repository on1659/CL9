# 참고 — R.E.P.O.의 귀중품·운반 물리·추출

> 2026-09-08, 설치본의 `Assembly-CSharp.dll`을 디컴파일해 읽은 기록.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 분석 방법과 다른 영역은 [README.md](../README.md).
> CL9의 대응 문서는 [../data/item/common.md](../../plan/data/item/common.md) ·
> [../data/item/egg.md](../../plan/data/item/egg.md) · [../data/item/cart.md](../../plan/data/item/cart.md)다.

R.E.P.O.의 귀중품(Valuable) 시스템은 "종류당 ScriptableObject 프리셋(가치 범위, 파손 내구도, 물리 속성) + 런타임 컴포넌트(`ValuableObject`)"로 데이터와 로직을 분리한 구조다. 가치는 레벨 생성 시점에 종류별 min~max 범위에서 랜덤으로 굴려 100달러 단위로 반올림하며, 레벨 전체의 스폰 개수·가치 총량은 난이도 곡선(`AnimationCurve`)으로 정해지는 상한을 향해 소진형으로 채워진다. 손상은 충돌 충격량을 세 등급(Light/Medium/Heavy)으로 나눠 원가 대비 최대 10배까지 벌어지는 손실 비율을 적용하고, 가치가 원가의 15% 밑으로 떨어지면 자동 소멸한다. 잡기·운반은 손이 잡은 지점에 스프링-댐퍼 힘을 거는 방식이라 여러 명이 잡으면 물리 엔진이 자연스럽게 힘을 합성하고, 무거울수록 가속도가 줄어드는 구조로 "혼자 못 든다"를 구현한다. 네트워크는 전 구간에서 Photon 마스터 클라이언트(호스트) 권위이며, 물리 오브젝트의 소유권은 잡는 사람에게 넘어가지 않고 항상 마스터가 들고 있다. 추출 지점은 런 전체 목표액을 지점 수로 균등 분배하고, 초과분(잉여)은 물리적인 돈 오브젝트로 스폰돼 다음 지점 목표에 자동 합산된다.

## 귀중품 데이터 모델

**`Value`** (ScriptableObject, 코드 기본값 `valueMin=100`, `valueMax=200`) — 귀중품 종류 하나당 하나씩 만드는 가치 범위 프리셋이다. 필드는 최소·최대 두 개뿐이며, 이 범위 안에서 랜덤 값을 뽑는 것이 그 종류의 가치가 된다.

**`RarityPreset`** (ScriptableObject) — 필드가 `chance`(0~100 range) 하나뿐이다. 이름과 달리 가치 범위와는 무관하고, 스폰 확률 가중치로만 쓰인다(추정 — 사용처는 `ValuableDirector`의 코스메틱 오브젝트 스폰 쪽에서 확인됨).

**`Durability`** (ScriptableObject, 코드 기본값 `fragility=100`, `durability=100`, 둘 다 0~100 range) — 파손 프리셋. enum이 아니라 두 float만 가진 데이터 애셋이다. `ValuableObject.durabilityPreset`이 이 프리셋을 참조하고, `PhysGrabObjectImpactDetector.Start()`가 여기서 런타임 `fragility`/`durability` 값을 복사해 초기화한다.

**`ValuableVolume`** — 귀중품이 배치될 슬롯을 나타내는 마커 컴포넌트다. `enum Type { Tiny, Small, Medium, Big, Wide, Tall, VeryTall }` 7단계 크기 등급이 이 순서로 선언돼 있다. `Setup()`이 호출되면 자기 위치 2m 반경을 `Physics.OverlapSphere`로 스캔해 같은 `VolumeType`이고 0.1m 이내에 있는 `ValuableObject`(또는 `CosmeticWorldObject`)를 찾아 그 부모를 이 볼륨의 부모로 재설정(reparent)하고, 못 찾으면 자기 자신을 파괴한다. 즉 **볼륨은 스폰 지점이 아니라 "이미 스폰된 오브젝트를 모듈 계층에 편입시키는 슬롯 매칭 지점"**이며, 실제 인스턴스 생성은 `ValuableDirector.SpawnValuable()`이 볼륨의 위치·회전에서 수행한다.

**`ValuablePropSwitch`** — 한 모듈 안에서 "귀중품 버전"과 "장식 소품(prop) 버전"을 확률적으로 스위칭하는 컴포넌트다. `ValuableParent`/`PropParent` 두 게임오브젝트를 갖고, `Setup()`에서 자식 중 활성화된 `ValuableObject`나 `CosmeticWorldObject`가 있으면 `ValuableParent`를 켜고 `PropParent`를 끄며, 없으면 반대로 한다. 즉 스폰 결과가 이미 나온 뒤 사후적으로 어느 쪽을 보여줄지 정하는 구조다. 자식에 반드시 `ValuableVolume`이 하나 있어야 하고, 스위치 중첩은 지원하지 않는다.

**`LevelValuables`** (ScriptableObject) — 레벨(맵)마다 등록하는 "이 레벨에서 나올 수 있는 귀중품 프리팹 목록" 프리셋이다. `tiny/small/medium/big/wide/tall/veryTall` 7개의 프리팹 리스트로, `ValuableVolume.Type`과 1:1 대응한다.

**`ValuableObject`** — 귀중품 하나의 런타임 데이터를 담는 핵심 컴포넌트다.
- 프리셋 참조: `durabilityPreset`(`Durability`), `valuePreset`(`Value`), `physAttributePreset`(`PhysAttribute`), `audioPreset`.
- 가치 필드: `dollarValueOriginal`(코드 기본값 100), `dollarValueCurrent`(손상으로 깎이는 현재값), `dollarValueSet`(중복 산정 방지 플래그), `dollarValueOverride`(0이 아니면 랜덤 대신 이 값을 강제 사용).
- `volumeType`(`ValuableVolume.Type`) — 자신이 어느 크기 등급 슬롯에 들어갈지 스스로 갖고 있다.
- `discovered`, `discoveredReminder` — 발견 여부와 "발견했지만 오래 방치됨" 상태.
- `Start()`에서 `physAttributePreset.mass`를 `Rigidbody.mass`와 `physGrabObject.massOriginal`에 그대로 적용한다.

가치 확정 로직(`DollarValueSetLogic()`)은 다음과 같다.
```csharp
dollarValueOriginal = Mathf.Round(UnityEngine.Random.Range(valuePreset.valueMin, valuePreset.valueMax));
dollarValueOriginal = Mathf.Round(dollarValueOriginal / 100f) * 100f;
dollarValueCurrent = dollarValueOriginal;
```
`valueMin~valueMax` 사이 랜덤값을 뽑은 뒤 **100 단위로 반올림**한다. `dollarValueOverride != 0`이면 랜덤을 생략하고 고정값을 쓴다.

`DollarValueSet()` 코루틴은 레벨 생성이 `Valuable` 단계를 지날 때까지(멀티면 `photonView.ViewID` 배정까지) 대기한 뒤, 싱글이거나 마스터 클라이언트면 `DollarValueSetLogic()`을 실행하고 `DollarValueSetRPC`로 값을 전파한다. 이때 **`RoundDirector.instance.haulGoalMax += (int)dollarValueCurrent;`가 실행되어, 추출 목표 금액(haul goal)의 기반이 되는 `haulGoalMax`는 고정 상수가 아니라 그 레벨에 실제로 스폰된 모든 귀중품 가치의 실측 합산으로 누적 결정된다.** (자세한 목표액 산출은 `## 추출 지점` 참고.)

`Discover(state)`는 첫 발견 시 `discovered=true`로 만들고(`DiscoverRPC`), `Map.Instance.AddValuable(this)`로 미니맵에 등록하며 `ValuableDiscover.instance.New(...)`를 호출해 발견 알림 UI를 띄운다. `DiscoverReminderLogic()`은 발견은 됐지만 카트 밖에서 플레이어와 20m 이상 떨어져 있으면(2~5초 간격 재판정) `discoveredReminder=true`로 표시한다.

네트워크: `[PunRPC]` 메서드는 모두 `SemiFunc.MasterOnlyRPC(_info)` 가드를 거친다 — **가치 산정·발견 판정은 마스터 클라이언트(호스트) 권위**이고, 다른 클라이언트는 결과만 RPC로 통보받는다.

## 배치와 가치 산정

**`ValuableDirector`**는 싱글턴(`static instance`)이며, 호스트만 실제 스폰 로직(`SetupHost()`)을 실행하고 클라이언트는 카운트가 맞을 때까지 대기 후 준비 완료 RPC만 보낸다.

**난이도별 목표값 산출**: 두 세트의 `AnimationCurve` 쌍(`totalMaxValueCurve1/2`, 크기 등급별 `~MaxAmountCurve1/2`)을 갖고, 어느 커브를 쓸지 다음 분기로 정한다.
```csharp
if (SemiFunc.RunGetDifficultyMultiplier2() > 0f && !SemiFunc.RunIsArena())
    time = SemiFunc.RunGetDifficultyMultiplier2();
else
    time = SemiFunc.RunIsArena() ? 0.75f : SemiFunc.RunGetDifficultyMultiplier1();
```
`totalMaxValue`와 등급별 `~MaxAmount`는 각각 `Mathf.RoundToInt(해당Curve.Evaluate(time))`로 정해진다. "아레나"(특수 라운드로 추정) 모드에서는 이렇게 구한 총 개수 상한을 추가로 2로, 등급별 최대 개수는 2~3으로 더 줄인다. `RunGetDifficultyMultiplier1/2`가 정확히 무엇의 함수인지(런 진행 레벨 수인지 시간인지)는 `SemiFunc.cs`를 봐야 하며 이번 조사 범위 밖이다(미확인).

레벨에 등록된 모든 프리셋(`LevelValuables`)의 크기별 프리팹 목록을 합치고, 씬에 배치된 모든 `ValuableVolume`을 크기별로 분류·셔플해 둔다. 디버그 모드(`valuableDebug == All`)면 모든 상한이 볼륨 개수 자체로, `totalMaxValue`는 사실상 무제한(99999)으로 덮어쓴다.

**스폰 루프**(개수 상한이 1차 제약, 가치 총량은 조기 종료용 2차 제약):
```
for (i = 0; i < totalMaxAmount; i++) {
    // 아직 개수 상한에 안 걸리고 볼륨·프리팹이 남은 등급들 중,
    // Random.Range(0, chance[등급]) 값이 가장 큰 등급을 뽑는 가중 추첨
    // 그 등급의 미사용 볼륨 하나 + 프리팹 목록 중 랜덤 하나로 SpawnValuable() 호출
}
```
등급별 가중치는 `tinyChance`~`veryTallChance`(정수) 필드로, 클수록 뽑힐 확률이 커진다. 모든 등급이 후보에서 빠지면 루프가 즉시 종료된다.

`SpawnValuable(prefab, volume, path)`은 오브젝트를 생성하고(싱글: `Instantiate`, 멀티: `PhotonNetwork.InstantiateRoomObject`) 그 자리에서 곧바로 `DollarValueSetLogic()`을 호출해 가치를 확정한다.
```csharp
totalCurrentValue += component.dollarValueCurrent * 0.001f;
if (totalCurrentValue > totalMaxValue)
    totalMaxAmount = valuableTargetAmount;   // 총 가치 상한 초과 시 스폰 루프를 사실상 조기 종료
```
즉 **가치 산정은 개당 랜덤 굴림이 기본이고, 레벨 전체 누적 가치가 난이도 곡선으로 정해진 상한(1000달러 단위)을 넘으면 그 시점에서 스폰을 조기 종료**한다. "총 가치 목표"는 상한선 역할이지, 정확히 그 금액을 채우는 하한 보장은 아니다.

같은 클래스가 귀중품과 별도로 장식용 오브젝트(`CosmeticWorldObject`)도 스폰한다. 레벨 클리어 횟수(`RunManager.instance.levelsCompleted`)에 따라 스폰 확률이 오르는 루프 구조를 갖고, 귀중품과 같은 `ValuableVolume` 슬롯 풀을 공유한다 — 이는 귀중품 자체 범위를 벗어나 세부는 생략한다.

**미니맵·발견 UI**: `MapValuable`은 귀중품 하나당 붙는 미니맵 아이콘으로, 플레이어와 같은 층이면 불투명, 다른 층이면 반투명(alpha 0.3)으로 표시한다. 생성 시점은 이 파일엔 없지만 `ValuableObject.DiscoverRPC()`가 부르는 `Map.Instance.AddValuable(this)`로 추정된다(즉 발견 전에는 미니맵에 안 뜨는 구조로 보인다). `WorldSpaceUIValue`는 귀중품 위에 뜨는 "$숫자" 텍스트로, 싱글턴 하나를 여러 오브젝트가 돌려쓰며(`Show(대상, value, isCost, offset)`) `showTimer`가 0.1초마다 리필되는 것으로 보아 매 프레임 호출되는 대상(주시 중이거나 들고 있는 물건)에 한해 계속 표시되는 구조로 추정된다. `ValuableDiscover`/`ValuableDiscoverGraphic`은 발견 시 뜨는 모서리 브라켓 팝업 UI로 `State` enum이 `Discover, Reminder, Bad, Custom` 4종이다. `ValuableDiscoverCustom`은 달러 가치가 없는 특수 오브젝트(열쇠류로 추정)를 위한 별도 발견·미니맵 시스템이며 `haulGoalMax`에는 관여하지 않는다.

**부가 발견**: `SurplusValuable`은 이름과 달리 "추출 초과분 계산" 로직이 아니라, 코인 더미 같은 소품에 붙어 `PhysGrabObjectImpactDetector`의 파손 콜백(`BreakLight/Medium/Heavy/DestroyImpulse`)에 맞춰 코인 파티클을 방출하는 이펙트 트리거다(자세한 내용은 `## 손상과 파손` 참고). 다만 이 클래스가 붙는 실제 프리팹은 추출 지점이 잉여를 물리적으로 스폰할 때 쓰는 오브젝트와 같아서(`## 추출 지점` 참고), 결과적으로 "잉여로 쏟아지는 돈"이 부서지면 코인이 튀는 연출로 이어진다.

### 배치·가치 산정 요약 공식표

| 항목 | 결정 방식 |
|---|---|
| 개당 가치 | `Random.Range(valueMin, valueMax)`를 100 단위로 반올림, `dollarValueOverride`가 있으면 고정값 |
| 레벨 전체 개수 상한 | `totalMaxAmount = curve(난이도)`, 아레나면 추가로 2로 나눔 |
| 등급별 개수 상한 | `{등급}MaxAmount = {등급}Curve.Evaluate(난이도)`, 아레나면 2~3으로 추가 축소 |
| 등급 선택 확률 | `{등급}Chance`(정수 가중치) 기반 "가장 큰 난수" 가중 추첨 |
| 레벨 전체 가치 상한 | `totalMaxValue = curve(난이도)`(1000달러 단위), 누적 가치가 넘으면 스폰 조기 종료 |
| 추출 목표 기반값(`haulGoalMax`) | 고정값이 아니라 스폰된 모든 귀중품 `dollarValueCurrent`의 실측 합산 |

## 손상과 파손

**충격 판정** (`PhysGrabObjectImpactDetector`): `ImpactState` enum은 `None, Light, Medium, Heavy` 네 값이다. `FixedUpdate()`가 매 물리 틱마다 속도·각속도 변화량으로 파손 판정용 `breakForce`와 연출용 `impactForce`를 따로 계산한다.
```csharp
breakForce *= 8f;
impactForce = breakForce / 8f * impactFragilityMultiplier;
breakForce = breakForce * (fragility / 100f) * fragilityMultiplier;
```
`fragility`가 낮을수록 같은 충돌에서도 `breakForce`가 작아져 잘 안 깨진다 — 이름과 반대로 "내구성 배율"처럼 동작한다(읽은 사실).

`OnCollisionStay()`는 두 갈래로 갈린다.
- **연출 등급**: `impactForce > 20`→Light, `>80`→Medium, `>150`→Heavy. 소리·이펙트만 재생하고 가치 손실은 없다.
- **파손 등급**: `breakForce > impactLevel1(300) * mass`→Light, `> impactLevel2(400) * mass`→Medium, `> impactLevel3(500) * mass`→Heavy. `mass`는 `Max(rb.mass, 1f)`. 등급별 0.3~0.6초 쿨다운으로 연속 판정을 막는다.

카트에 실려 있으면(`inCart`) `breakForce`가 강제로 0이 되어 파손 판정 자체가 스킵된다 — **카트 운반 중에는 귀중품이 깨지지 않는다**.

**가치 손실 공식** — 세 등급 모두 같은 형태이며 등급별 기본 손실률만 다르다(Light 0.01, Medium 0.05, Heavy 0.1).
```csharp
float num = 0.01f * (1f + 9f * (100f - durability) / 100f);   // BreakLight 예시
valueLost = Mathf.Round(valuableObject.dollarValueOriginal * num);
valueLost = Mathf.Clamp(valueLost, 0f, valuableObject.dollarValueCurrent);
```
`durability=100`이면 배율은 1(원가의 1/5/10%), `durability=0`이면 배율 10(원가의 최대 10/50/100%)까지 커지는 선형 보간이다. `BreakHeavy`만 `Clamp` 하한이 `1f`(최소 1달러 손실 보장), Light/Medium은 하한 `0f`.

**완전 파괴**: `BreakRPC()`에서 `dollarValueCurrent -= valueLost` 적용 후 `dollarValueCurrent < dollarValueOriginal * 0.15f`이면 오브젝트가 완전 파괴된다(`DestroyObject()`, 튜토리얼 중엔 예외로 리셋만). **원가의 15% 미만으로 가치가 떨어지면 자동 소멸**하는 구조다. `DestroyObject()`는 `SemiFunc.IsMasterClientOrSingleplayer()`가 아니면 즉시 리턴해 호스트만 실행한다.

**`ShatterEffect`**는 로직이 아니라 순수 연출 컴포넌트다. `SpawnParticles(direction)` 한 번으로 파편·연기 파티클과 사운드를 재생한 뒤 즉시 자기 자신을 파괴한다. 파손 판정과는 별개로, 이미 파손이 확정된 뒤 시각 마무리로 호출되는 것으로 보인다(추정).

**`BreakableProp`/`BreakableWall`**은 귀중품 가치 시스템과 무관한 별개 시스템이다. `BreakableProp`은 파괴 시 지정 오브젝트를 활성화만 하는 헬퍼이고, `BreakableWall`은 자체 `Crack()`(균열 1→2→3 단계)과 `BreakWall()` 메서드를 외부에서 호출받는 레벨 기믹(벽 부수기)용으로 보인다(추정).

**`WorldSpaceUIValueLost`**는 가치 손실 시 뜨는 "-$N" 월드스페이스 텍스트다. `value < 1000`이면 스케일을 0.75배로 줄이고(코드 기본값), `timer = 3f`(코드 기본값) 동안 확대→축소 애니메이션 커브로 흔들리다가 사라진다. 생성 트리거는 `BreakRPC()` 안의 `WorldSpaceUIParent.instance.ValueLostCreate(_contactPoint, (int)valueLost)` 호출이다.

**적을 때리는 용도로 쓰면 강제 파손**: `OnCollisionStay()`에 적(`Enemy` 태그)과의 충돌을 처리하는 별도 분기가 있다. 귀중품을 적에게 던져 맞히면 `volumeType`별 고정 손실 비율(Tiny/Small 0.2, Medium 약 0.143, Big/Wide/Tall/VeryTall 0.1, ±0.05 랜덤 가산)로 `BreakHeavy`가 강제 호출된다 — 귀중품을 무기로 쓰면 무조건 그만큼 가치를 잃는다.

**네트워크 판정 주체**: `FixedUpdate()`/`OnCollisionStay()` 첫 줄에 `if (GameManager.instance.gameMode == 1 && !PhotonNetwork.IsMasterClient) return;`이 있다 — 멀티플레이어일 때는 **마스터 클라이언트만** 충격·파손을 계산한다. 싱글이면 로컬에서 바로 `BreakRPC`를 호출하고, 멀티면 `photonView.RPC("BreakRPC", RpcTarget.All, ...)`로 전파한다. `[PunRPC] BreakRPC`/`HealRPC`/`DestroyObjectRPC`는 모두 `SemiFunc.MasterOnlyRPC(_info)`로 마스터가 보낸 RPC만 수락한다 — **판정은 전적으로 마스터 클라이언트 권위**이고, 다른 클라이언트는 결과(가치 감소·이펙트)만 반영한다.

## 개별 기믹

`ValuableEgg`는 CL9의 계란과 직접 대응하므로 자세히 다룬다. 그 외 42개는 표로 정리한다.

### `ValuableEgg` 상세

`ValuableEgg`는 `ValuableObject`가 아니라 `Trap`을 상속한다 — R.E.P.O.의 계란은 순수 귀중품이 아니라 "귀중품처럼 생긴 몬스터/트랩"이다(읽은 사실). `physGrabObject`(잡기 가능한 물리 오브젝트)이면서 `enemyInvestigate = true`로 몬스터를 유인하는 이중 역할을 한다.

상태 머신은 `EggState` enum이 `State1 → State2 → State3 → State4 → State5 → Explode` 순으로 진행한다. `Crack()`이 호출될 때마다 한 단계씩 진행하며(`SetNextState()`), 단계마다 크기를 원래의 90%→70%→50%→25%로 줄이고 머티리얼을 단계별로 교체한다. `State4`에서 `Crack()`이 호출되면 팔다리(`armL/armR/legL/legR`)가 활성화된다. `Crack()`을 호출하는 외부 트리거는 이 파일에 없어 `PhysGrabObjectImpactDetector`가 충격을 받으면 호출하는 구조로 추정된다(추정). `Crack()`이 호출되면 잡고 있던 플레이어 손에서 강제로 놓이고(`OverrideGrabRelease`), 카메라 셰이크와 파티클이 재생된다.

질량은 단계마다 `SetMass()`가 2.0 → 1.75 → 1.25 → 1.0 → 0.5(코드 기본값)로 낮춘다 — **깨질수록 가벼워진다.**

`State5`(최종 단계)는 매 프레임 비명 사운드를 루프 재생하고 팔다리를 흔들며 조명을 켠다. 호스트만 `explosionTimer`를 누적해 `explosionTime`(코드 기본값 3초)이 지나면 `Explode` 상태로 전이한다. 그 외에도 항상 무작위 토크로 뒤집으려 하며(`OverrideTorqueStrength`), `State5`에서는 추가 무작위 흔들림(`ScreamShake`)이 겹친다.

`Explode()`는 폭발 파티클(반경 1.5, 100~300개)을 스폰하고 `physGrabObject.dead = true`로 설정한 뒤 오브젝트를 파괴한다. 가치 관련 필드는 이 클래스에 없어, 폭발 시 가치가 사라지는 처리는 별도 `ValuableObject` 컴포넌트가 `dead` 플래그를 보고 수행하는 구조로 추정된다(추정). 상태 전이는 `[PunRPC] SetStateRPC` + `SemiFunc.MasterOnlyRPC`로 동기화되는, 이 프로젝트 전역과 동일한 패턴이다.

### 나머지 42종 요약

| 클래스 | 무엇이 트리거되고 무슨 일이 생기는지 |
|---|---|
| `MilkValuable`(`Trap`) | 흔들림 속도가 임계값(2, 코드 기본값)을 넘으면 찰랑거리는 소리가 나고, 매 프레임 토크가 낮게 눌려 액체처럼 덜 회전한다. |
| `BarrelValuable`(`Trap`) | 충격을 `MaxHitCount`(3, 코드 기본값)번 맞으면 폭발하며 충격 감지기에 파괴를 위임한다. |
| `PowerCrystalValuable` | 외부 호출로 `GlowDim/Med/Strong()` 3단계 발광이 커브 보간되고, `Explode()`로 폭발·조명 제거가 일어난다(호출자는 범위 밖 추정). |
| `BabyHeadValuable`(`Trap`) | 기울기가 45도 미만이면 시간이 지나 "깨어나" 울기 시작하고, 근처 플레이어가 있으면 무작위 간격으로 튀어 오르며 몬스터를 부른다. 다시 기울이면 잠든다. |
| `ScreamDollValuable` | 잡히면 비명 루프와 함께 7% 확률로 무작위 임펄스·토크로 발버둥치듯 움직인다. 적과 충돌하면 강한 파손을 유도한다. |
| `CrystalBallValuable` | 로컬 플레이어가 잡으면 1인칭 카메라를 강제로 수정구 시점으로 전환해 렌즈 왜곡·비네트를 사인파로 흔들고, 마우스로 별도 조준을 돌려 정찰 카메라처럼 쓴다. |
| `GumballValuable`(`Trap`) | 잡으면 눈을 뜨고 최면 상태로 진입해 시야 안 플레이어에게 비네트·소용돌이·동공 확대 디버프를 걸고 카메라를 눈에 고정시킨다. |
| `PhoneValuable` | 5초마다 근처 플레이어·적을 감지해 벨이 울리며 진동하고, 매 프레임 몬스터를 부른다. 상호작용으로 집으면 즉시 멈춘다. |
| `TrafficLightValuable`(`Trap`) | 잡으면 초록→노랑→빨강 신호가 자동 순환하고, 빨강에서 계속 잡고 있으면 전기 충격과 넘어뜨림을 가한다 — 오래 들면 처벌받는 폭탄 돌리기형 트랩. |
| `FlamethrowerValuable`(도구) | 방아쇠를 당기면 화염을 분사하며 연료가 소모되고 몬스터를 부른다. 연료가 떨어지면 빈 소리만 남는다. |
| `FireExtinguisherValuable`(도구) | 플레임스로워와 거의 동일한 구조로 소화 스프레이를 분사한다(소화 대상 상호작용은 미확인). |
| `JackhammerValuable`(도구) | 잡기만 해도 자동으로 진동하며 흙먼지·카메라 셰이크를 내고, 15초 간격으로 몬스터를 부른다. 적에게는 피해를 주는 무기로도 쓰인다. |
| `IceSawValuable`(`Trap`, 도구 겸용) | 잡거나 트랩으로 활성화되면 톱날이 가속 회전하며 몬스터를 부른다. 트랩 활성화 시 손에서 강제로 튕겨 나가는 방식으로 함정 역할도 한다. |
| `ValuableCar`(`Trap`) | 방치되면 스스로 전진 주행하며 경적을 울리고 무작위로 방향을 튼다(추정 포함). |
| `ValuablePlane`(`Trap`) | 활주 후 이륙해 날아가 버리는 5단계 상태기계 — 못 잡으면 회수 불가한 도주형 귀중품(추정). |
| `ValuableLevitationPotion` | 활성화되면 부양 효과를 내는 구체를 소환한다(트리거는 추정: 파손 시). |
| `ValuableLovePotion` | 들고 있으면 쿨다운마다 근처 다른 플레이어에게 무작위 애정 표현 문장을 자동 채팅으로 내뱉는다. |
| `ValuableSpiderPotion` | 파손되면 근처 플레이어 화면에 거미 이펙트·글리치를 띄운다. 거미공포증 옵션 시 대체 연출로 전환된다. |
| `ValuableTeethBot` | 들고 있는 동안 음성 채팅의 피치·위치를 왜곡시키고 음량에 맞춰 입 모형을 립싱크시킨다. |
| `ValuableWizardStaff` | 트리거되면 1~4초 레이저를 발사하며 반동으로 자신이 밀려난다. |
| `ValuableBoombox`(`Trap`) | 들고 있으면 음악을 재생하며 빛나고 몬스터를 부르며, 든 사람의 카메라·표정을 흔든다. 특정 조건에서 숨겨진 곡으로 전환된다. |
| `ValuableCamera` | 파손되면 플래시와 함께 근처를 기절시키는 플래시뱅형 폭발을 일으킨다(추정). |
| `BlenderValuable`(`Trap`) | 들면 블레이드가 돌며 내용물을 갈고, 일정 시간 뒤 뚜껑이 터진다. 칼날에 피해 판정이 있는 것으로 보인다(추정). |
| `MoneyValuable` | 파손되면 돈이 터지는 파티클 연출만 재생한다(추정: 파손 트리거). |
| `ValuablePills` | 흔들림을 감지해 알약통 소리를 재생하는 순수 오디오 기믹이다. |
| `ValuableCocktail`(`Trap`) | 기울이거나(내적 0.4 미만) 빠르게 던지면 자동으로 쏟아지며 구토 이펙트와 함께 코드가 직접 경미한 파손을 강제 발생시킨다. |
| `ValuableCubeBall` | 충돌 강도별 콜백에서 무작위 위 방향 힘·토크가 추가돼 여러 번 통통 튄다. |
| `ValuableSawBlade` | `Start`/`Update`가 비어 있다 — 코드상 로직 없는 순수 메시용 자리표시자로 보인다(추정). |
| `ValuableFlashlight` | 잡으면 켜지고 놓으면 꺼진다. 충격을 받으면 깜빡이는 연출이 재생된다. |
| `ValuableForeverCandle` | 충격 시 사운드만 재생하는 단순 소품이다. |
| `ValuableEyeOfOrpigox` | 들고 있는 동안 눈알이 든 사람의 시선을 따라다니며 회전 속도에 비례한 소리를 낸다. |
| `ValuableStarWand` | 발동될 때마다 느린 투사체를 발사하고 반동을 받으며, 발사 횟수에 비례해 자기 가치를 스스로 깎는다 — **사용할수록 가치가 소모되는 귀중품**의 명확한 예. |
| `ValuableWizardTimeGlass` | 활성화되면 소용돌이·반짝임 파티클과 피치가 변하는 루프 사운드를 재생한다(트리거 조건 미확인). |
| `ValuableTray`(`Trap`) | 기울이면(내적 0.85 미만) 기울기가 클수록 짧아지는 간격으로 위에 놓인 물건을 하나씩 떨어뜨리며, 떨어질 때마다 코드가 직접 중간 파손을 강제 발생시킨다. |
| `ValuableScale` | 체중계형 코미디 소품. 올라간 사람·물체 질량을 실시간 측정해 Winner/Loser/Error를 판정한다. 가치와는 무관하다. |
| `ValuableSmallPotion` | 파손되면 주변을 회복시키는 힐링 오라를 생성한다. |
| `ValuableCauldronBox` | 상자로 위장돼 있다가 트랩 활성화 시 상자가 부서지며 안의 실제 오브젝트(가마솥)가 드러나는 언박싱형 귀중품이다. |
| `ValuableArcticSnowBike` | 핸들을 잡으면 조종 가능한 탈것형 귀중품이다(추정: 눈밭 맵 전용). |
| `WizardBroomValuable`(`Trap`) | 깨어나면 스스로 전진·회전하며 도망 다니다가, 잡히면 얌전해지고 오래 방치되면 다시 잠드는 살아있는 빗자루 기믹이다. |
| `ItemValuableBox` | 개별 귀중품이 아니라 **압축기** — 근처 귀중품들을 흡수해 값을 합산(상한 5000, 코드 기본값)하고, 추출 지점 안에서 순서대로 뱉어내며 추출한다. |
| `MusicalValuableLogic` | 건반형 귀중품 공용 로직 — 접촉 위치에 따라 음을 재생하는 순수 오디오 기믹이다. |
| `NotValuableObject` | 귀중품이 아닌 "잡을 수 있는 일반 소품"용 범용 컴포넌트. 체력이 있으면 충격 등급별로 깎이다 0 이하가 되면 파괴된다 — 귀중품과 비귀중품이 같은 파손 시스템을 공유한다는 근거다. |

공통 관찰: 대부분의 `Trap` 상속 클래스는 `enemyInvestigate`/`EnemyDirector.SetInvestigate` 필드·호출로 "몬스터를 부르는 소음 유발"이라는 기믹을 공유한다. 상태 전이는 거의 모두 `enum States` + `[PunRPC] SetStateRPC` + `SemiFunc.MasterOnlyRPC` + 싱글플레이어 시 RPC 우회 패턴으로 통일돼 있다. `ValuableStarWand`/`ValuableCocktail`/`ValuableTray`처럼 자기 로직에서 직접 충격 감지기의 `Break` 계열 메서드를 호출해 가치를 깎는 귀중품도 있다 — 파손이 항상 외부 충돌에서만 오는 것은 아니다.

## 잡기·운반 물리

**잡기 거리**(`PhysGrabber`, 코드 기본값): `grabRange = 4f`(탐지 최대 거리), `maxDistanceFromPlayer = 2.5f` / `minDistanceFromPlayer = 1f`(유지 거리 범위), `grabReleaseDistance = Max(grabRange * 2f, 10f)`(초과 시 강제로 손을 놓음). `grabRange`는 `StatsManager.instance.playerUpgradeRange` 상점 업그레이드로 레벨당 `+1f`씩 늘어난다.

**잡는 힘 — 스프링·댐퍼**: 실제 힘 계산은 `PhysGrabBeam`(순수 시각 빔 이펙트, `[RequireComponent(typeof(LineRenderer))]`)이 아니라 `PhysGrabObject.PhysicsGrabbingManipulation()`에서 이뤄진다.
```csharp
Vector3 vector = Vector3.ClampMagnitude(physGrabPointPullerPosition - item.physGrabPoint.position, num) * 10f;
Vector3 pointVelocity = rb.GetPointVelocity(item.physGrabPoint.position);
Vector3 vector2 = Vector3.ClampMagnitude(vector * item.springConstant - pointVelocity * item.dampingConstant, num) * 2f / mass;
```
목표 지점과의 변위에 `springConstant`(코드 기본값 0.9)를 곱하고 현재 속도에 `dampingConstant`(0.5)를 곱해 뺀 전형적인 스프링-댐퍼 공식이며, `AddForceAtPosition(..., ForceMode.Acceleration)`로 **잡은 지점(손 위치)**에 적용된다. `forceConstant`/`forceMax`는 각각 코드 기본값 4f.

**질량과 여러 명이 함께 들 때 힘 합산 — 핵심**: `playerGrabbing`은 `List<PhysGrabber>`이며, 힘 계산 루프가 각 플레이어의 잡은 지점마다 독립적으로 스프링-댐퍼 힘을 계산해 각각 `AddForceAtPosition`으로 적용한다. 즉 힘의 "합산"은 명시적 덧셈 공식이 아니라 **리지드바디 물리 엔진이 여러 지점에 걸린 여러 힘을 자연스럽게 합성**하는 방식이다(읽은 사실). 힘 자체가 `2f / mass`로 질량에 반비례해 가속도 형태로 적용되므로, 질량이 클수록 동일한 변위 오차에서 나오는 가속도가 작아진다. 여기에 더해 `grabStrength`(코드 기본값 1f, 상점 업그레이드로 레벨당 `+0.2f`)는 질량 구간별 문턱값(`num8`)에 따라 `Lerp`로 완만하게 감쇠된다 — 질량이 커질수록 업그레이드로 얻은 초과 힘이 깎여, 업그레이드로 무한정 강해지는 것을 막는다. 개인당 힘은 `forceMax`(기본 4f)로도 클램프된다. 종합하면 "혼자 못 드는" 것은 하드코딩된 인원수 제한이 아니라 **질량 대비 가속도 부족 + 개인 힘 상한**의 결과다(읽은 사실 기반 종합). 카트에 실린 물체를 직접 밀 때는 그랩 지점이 카트 표면에 거의 닿아 있으면 개인 힘 상한이 1/4로 추가 감쇠된다.

**던지기**: 손을 놓는 순간(`Throw(PhysGrabber player)`) 호출된다.
```csharp
float num = Mathf.Max(rb.mass * 1.5f, 1f);
Vector3 force = Vector3.ClampMagnitude(player.physGrabPointPullerPosition - player.physGrabPoint.position, player.forceMax) * num;
force *= 0.5f + player.throwStrength;
rb.AddForce(force, ForceMode.Impulse);
```
놓기 직전 당기던 위치와 실제 위치의 벡터에 질량 비례 계수를 곱하고, `throwStrength`(코드 기본값 0, 업그레이드로 레벨당 `+0.3f`)를 반영해 순간 임펄스로 적용한다. 질량이 클수록 임펄스가 커지는 구조다.

**회전**: 잡은 물체의 목표 회전과 현재 회전의 차이를 축으로 삼아 토크를 만들고, `overrideTorqueStrengthX/Y/Z`로 축별 강도를 개별 조정한다. 마우스 회전 입력 중이면(`isRotating`) 토크 강도가 5배로 증폭된다. 힌지가 달린 물체(`hasHinge && !dead && !broken`)는 이 토크 블록 전체를 건너뛴다 — 회전은 힌지가 담당하고 그랩 힘은 위치 이동에만 쓰인다(이동 성분은 2배로 증폭됨).

**소유권(Photon ownership) — 이전하지 않는다**: `PhysGrabObject`의 `PhotonView` 소유권은 잡는 플레이어에게 넘어가지 않는다. `Start()`에서 멀티플레이어 모드면 무조건 `photonView.TransferOwnership(PhotonNetwork.MasterClient)`로 마스터 클라이언트 소유로 고정한다(`StaticGrabObject`도 동일). 대신 물리 시뮬레이션 자체가 호스트 권위형이다 — 힘 계산 진입부에서 `if (isGrabForceZero || (Multiplayer && !isMaster) || rb.isKinematic) return;`으로 마스터가 아니면 계산 자체를 스킵한다. 클라이언트는 잡기 "의도"만 RPC로 보낸다(`GrabStartedRPC`→`RpcTarget.MasterClient`), 마스터가 승인하면 `GrabPlayerAddRPC`를 전체 브로드캐스트한다. 던지기 계산도 마스터에서만 실행되며, 위치·잡기 인원 동기화는 `GrabLinkRPC`/`GrabPointSyncRPC`/`GrabPlayerAddRPC`/`GrabPlayerRemoveRPC`가 담당한다. CL9 맥락으로 옮기면 Photon 마스터 클라이언트가 담당하는 이 역할이 언리얼 리슨 서버 호스트에 대응한다(관찰).

**`PhysGrabObjectGrabArea`**: 하나의 `PhysGrabObject`가 여러 `GrabArea`(각자 콜라이더 목록 + 잡기 시작/유지/놓음 3개의 유니티이벤트)를 가질 수 있다. 예를 들어 손잡이 부분만 잡았을 때 특정 이벤트를 발동시키는 용도로 보인다(추정). 마스터 클라이언트만 매 프레임 판정하고 결과를 RPC로 동기화한다.

**`StaticGrabObject`/`KineticUngrabbablePhysGrabObject`**: `StaticGrabObject`는 리지드바디 물리가 아니라 **속도만 계산하는 그랩 가능 오브젝트**다(`velocity += (당김위치 - 잡은위치) * 5f * deltaTime`, `AddForce` 호출 없음) — 크랭크·레버 같은 손잡이류로 추정된다. `KineticUngrabbablePhysGrabObject`는 매 프레임 강제로 `OverrideKinematic()`과 `OverrideGrabDisable()`을 호출해, 원래 잡을 수 있는 오브젝트를 임시로 비물리·잡기불가 상태로 오버라이드하는 스위치다.

**`PhysGrabHinge`**: Unity `HingeJoint`를 사용하며 `OnJointBreak(float)` 콜백으로 `broken=true`가 된다 — 과도한 힘이 걸리면 힌지가 물리적으로 부서진다. `dead`(임시)와 `broken`(영구) 두 상태를 구분하고, 닫힘/열림 속도에 문턱값(`closeHeavySpeed=5`, `openHeavyThreshold=3` 등)이 있어 세게 닫거나 열면 별도의 "heavy" 처리(진동·소리로 추정)가 갈린다.

**`PlayerPhysPusher`**: 마스터 클라이언트가 "다른 플레이어" 아바타마다 생성하는 물리 대역이다. 자기 자신 소유 아바타거나 싱글플레이면 즉시 파괴된다. 원격 플레이어의 이동 속도를 상대 좌표계 임펄스로 변환해 밀어붙여, 호스트 시뮬레이션 안에서 다른 플레이어의 몸이 물체에 물리적으로 부딪혀 밀 수 있게 만드는 장치다.

## 카트

**담긴 물건의 물리 상태**: 고정(키네마틱)되지 않는다. `PhysGrabCart.ObjectsInCart()`가 0.5초 주기로 `Physics.OverlapBox`를 걸어 카트 박스 영역 안의 `PhysGrabObject`들을 매번 다시 스캔한다.
```csharp
Collider[] array = Physics.OverlapBox(inCart.position, inCart.localScale / 2f, inCart.rotation);
haulCurrent += (int)componentInParent2.dollarValueCurrent;
valueScreen.UpdateValue(haulCurrent);
```
즉 "담겼다"는 매 틱 위치 검사로만 판정되고, 물건은 여전히 자유 물리 오브젝트로 카트 바닥 위에서 움직인다. 카트 상태에 따라 벽·바닥의 물리 재질(마찰)이 바뀌어 짐이 덜 튀거나 더 미끄러지게 만드는 정도로 보조한다. `PhysGrabInCart`는 별개 보조 클래스로 1초 타이머 목록을 관리하지만, 정확히 무엇이 이 리스트를 채우는지는 확인하지 못했다(추정: 별도 트리거 콜라이더). `CartHoverCollider`는 `cartHover` 불리언만 갱신하는 단순 트리거 감지기로, 다른 스크립트가 참조하는 UI 힌트용으로 보인다(추정).

**가치 표시**: 카트에 담긴 물건들의 `dollarValueCurrent` 합계(`haulCurrent`)를 `ValueScreen.UpdateValue(int)`로 카트 화면(TextMeshPro)에 표시한다. 값이 오르면 증가음, 내리면 감소음이 난다. 이 `ValueScreen`은 추출 지점의 목표 표시와는 별개 인스턴스다.

**카트 자체의 가치**: `PhysGrabCart.cs` 안에는 자기 자신에 대한 `ValuableObject` 참조가 없다 — 카트 자체가 귀중품으로 카운트되는지는 이 파일만으로 확인할 수 없다(미확인).

**이동 물리**: 카트는 `State` enum(`Locked`/`Dragged`/`Handled`)을 가진다. 전용 그랩 영역으로 밀거나 당기면 `CartSteer()`가 카트 위치를 플레이어 뒤쪽 목표점으로 `MoveTowards`시키고 회전도 서서히 맞춘다(속도 상한 5, 각속도 상한 4). 손잡이를 오래 누르면 `State.Handled`(강한 추진)로, 짧게 밀면 `State.Dragged`(약한 추진)로 전환된다. 카트 질량은 기본 8이고 조작 중엔 4로 낮춰 밀기 쉽게 만든다. 소형 카트(`isSmallCart`)는 조작 중에만 히트박스를 켜는 것으로 보아 무기로도 쓰이는 변형이 있는 것으로 추정된다.

**네트워크**: 상태 전환은 `photonView.RPC("StateSwitchRPC", RpcTarget.All, _state)`로 브로드캐스트되며, `[PunRPC] StateSwitchRPC`는 `SemiFunc.MasterOnlyRPC(_info)`를 통과해야 실행된다 — 상태 판정은 마스터 권위, 시각 효과(색·사운드·마찰 재질)는 전원에 동기화된다.

## 추출 지점

**지점 개수**: `RoundDirector.extractionPoints`는 정수 필드로 존재하지만, `ExtractionPoint.cs`/`RoundDirector.cs` 안에서는 이 값에 대입하는 코드를 찾지 못했다 — 레벨 생성기 쪽(조사 범위 밖)에서 설정되는 것으로 추정된다(미확인).

**목표 금액 산정 — 두 단계**. 1단계, 런 전체 목표(`RoundDirector.haulGoal`, `StartRound()`에서 계산)는 레벨 최대 획득 가능 총액(`haulGoalMax`, `## 배치와 가치 산정`에서 다룬 스폰 누적값)의 70%에 난이도 곡선을 곱한다.
```csharp
float num = (float)haulGoalMax * 0.7f;
num = (!(SemiFunc.RunGetDifficultyMultiplier2() > 0f))
    ? (num * haulGoalCurve1.Evaluate(SemiFunc.RunGetDifficultyMultiplier1()))
    : (num * haulGoalCurve2.Evaluate(SemiFunc.RunGetDifficultyMultiplier2()));
```
튜토리얼/저사양 디버그 모드에서는 `num = 1000 * extractionPoints`로 대체된다. 2단계, 지점별 목표는 지점이 실제로 활성화되는 순간(`Idle→Active` 전환 시) 런 전체 목표를 남은 지점 수로 균등 분배한다.
```csharp
int value = RoundDirector.instance.haulGoal / RoundDirector.instance.extractionPoints;
```
즉 지점마다 별도 랜덤값이 아니라 "전체 목표 ÷ 지점 개수"의 균등 분배이며, 사전 배분이 아니라 활성화 시점에 계산된다.

**추출 확정 절차** (`ExtractionPoint.State`: `Idle → Active → (Surplus) → Warning → Extracting → Complete/TaxReturn`, 실패 시 `Cancel`):
1. 버튼(`StaticGrabObject`)을 눌러 `Idle→Active`. 활성화 요청은 마스터에게 전달되고, 마스터가 승인하면 전체에 브로드캐스트하며 동시에 다른 모든 지점을 잠근다 — **한 번에 하나의 지점만 활성화**할 수 있다. 잠긴 지점 버튼을 잡으면 강제로 손을 놓게 하는 경고 연출이 뜬다.
2. `Active` 상태에서 지점 내 하울 총액이 목표 이상이 되면 별도 버튼 조작 없이 1.5초 지연 후 자동으로 `Success`로 전환된다. 초과분이 있으면 잉여 연출을 거쳐 `Warning`으로 진입한다.
3. `Warning`은 3초 카운트다운(경고등·사이렌)이며, 이 동안 목표 미달로 떨어지면(카운트다운 중 물건을 빼내면) 즉시 `Cancel`로 되돌아간다 — **경고 중 물건을 빼내면 추출이 취소**된다.
4. 카운트다운이 끝나면 `Extracting` 진입. 이 시점에 `RoundDirector.HaulCheck()`가 하울 리스트의 각 아이템이 실제로 지점 룸 볼륨 안에 있는지 재검증하고, 없는 것은 제외한다. 그 뒤 `DestroyTheFirstPhysObjectsInHaulList()`가 상태 지속시간을 아이템 개수로 나눈 간격으로 하나씩 순차 파괴한다. 파괴 직전 `ValuableExtracting`(메시·머티리얼만 복사한 연출용 오브젝트, 위로 떠오르며 회전하다 자동 파괴)을 스폰해 빨려 들어가는 연출을 낸 뒤, 실제로는 `PhysGrabObject.DestroyPhysGrabObject()`로 소멸시키며 `RoundDirector.totalHaul`에 값을 누적한다.
5. `Complete`(또는 마지막 지점이 아니면 `TaxReturn`)로 종료된다.

**잉여(Surplus) 처리**:
```csharp
private int CalculateSurplus() { int num = haulCurrent - haulGoal; return Mathf.Max(0, num); }
```
잉여는 `RoundDirector.extractionPointSurplus`에 저장되고, **다음 지점의 시작 하울에 자동 합산**된다(`haulCurrent = currentHaul + extractionPointSurplus`) — 초과분은 다음 지점 목표 달성에 그대로 보탬이 된다. 잉여가 남아 있으면 `SpawnTaxReturn()`이 임계값(5000/10000)에 따라 `AssetManager.instance.surplusValuableSmall/Medium/Big` 프리팹을 지점 앞에 물리적으로 스폰하고 잉여를 0으로 리셋한다. 이 프리팹에 `## 손상과 파손`에서 다룬 `SurplusValuable` 컴포넌트가 붙어 있어, 잉여로 쏟아진 돈을 부수면 코인이 튀는 연출로 이어진다.

**마지막 지점**: `RoundDirector.ExtractionCompletedAllCheck()`가 `extractionPointsCompleted >= extractionPoints - 1`이면 전원에 `allExtractionPointsCompleted = true`를 브로드캐스트하고 카메라 셰이크·소등 사운드를 재생한다. 트럭으로의 복귀 처리는 `ExtractionPoint`/`RoundDirector` 안에서 직접 발견하지 못했다 — 별도의 트럭/복귀 스크립트가 이 플래그를 구독하는 것으로 추정된다(범위 밖, 미확인).

**UI**: `HaulUI`는 활성화된 지점이 있을 때만 "현재 하울 / 그 지점의 목표"를 보여주며, 값이 오르면 초록 플래시, 내리면 빨강 플래시가 뜬다. `GoalUI`는 레벨 진행 중 상시 "완료한 지점 수 / 전체 지점 수"만 보여주는 단순 카운터다.

## 네트워크

전 영역에서 반복되는 패턴은 **"요청은 아무나 → 판정은 항상 마스터 클라이언트 → 확정 결과만 `RpcTarget.All`로 브로드캐스트"**다. 싱글플레이어에서는 RPC를 건너뛰고 같은 메서드를 로컬 직접 호출한다.

| 영역 | RPC | 흐름 | 판정 주체 |
|---|---|---|---|
| 귀중품 가치 | `DollarValueSetRPC` | 마스터→전체 | 마스터(스폰 시 1회) |
| 귀중품 발견 | `DiscoverRPC` | 마스터→전체 | 마스터 |
| 귀중품 파손 | `BreakRPC`/`HealRPC`/`DestroyObjectRPC` | 마스터→전체(`MasterOnlyRPC` 가드) | 마스터 전담 계산 |
| 잡기 시작 | `GrabStartedRPC` | 클라이언트→마스터 | 마스터 승인 후 `GrabPlayerAddRPC`로 전체 반영 |
| 잡기 종료 | `GrabEndedRPC` | 클라이언트→마스터 | 마스터가 `GrabPlayerRemoveRPC`로 반영, 던지기 계산도 마스터 전담 |
| 잡기 위치 동기화 | `GrabLinkRPC`/`GrabPointSyncRPC` | 마스터→전체 | 마스터 |
| 카트 상태 | `StateSwitchRPC` | 마스터→전체(`MasterOnlyRPC`) | 마스터 |
| 지점 활성화 요청 | `RequestExtractionPointActivationRPC` | 클라이언트→마스터 | 마스터가 중복 체크 후 승인 |
| 지점 활성화 확정 | `ExtractionPointActivateRPC` | 마스터→전체 | 마스터, 다른 지점 동시 잠금 |
| 지점 목표 설정 | `HaulGoalSetRPC` | 마스터→전체 | 마스터 |
| 지점 상태 전이 | `StateSetRPC` | 마스터→전체 | 마스터 |
| 잠긴 지점 접근 거부 | `ButtonDenyRPC` | 마스터→전체 | 마스터 |
| 잉여 값 동기화 | `ExtractionPointSurplusRPC` | 마스터→전체 | 마스터 |
| 런 시작 | `StartRoundRPC` | 마스터→전체 | 마스터(멀티에서만 호출) |
| 전체 지점 완료 | `ExtractionCompletedAllRPC` | 마스터→전체 | 마스터 |

물리 오브젝트(`PhysGrabObject`, `StaticGrabObject`)의 `PhotonView` 소유권은 **잡는 플레이어에게 이전되지 않고 항상 마스터 클라이언트로 고정**된다(`Start()`에서 `TransferOwnership(PhotonNetwork.MasterClient)`). `RoundDirector`는 RPC 외에 `IPunObservable` 스트림으로 `currentHaul`/`haulGoal` 등을 주기적으로 스냅샷 동기화하는 것으로 보인다(추정: 구현 시그니처는 직접 확인하지 못함).

## 수치 (공식·상수 표)

| 항목 | 값/공식 | 출처 |
|---|---|---|
| 잡기 탐지 거리 | `grabRange = 4f`(코드 기본값), 업그레이드당 `+1f` | `PhysGrabber` |
| 잡은 물체 유지 거리 | `1f`(최소) ~ `2.5f`(최대) | `PhysGrabber` |
| 강제 놓음 거리 | `Max(grabRange*2, 10f)` = 10 | `PhysGrabber` |
| 잡기 스프링/댐핑 계수 | `springConstant=0.9`, `dampingConstant=0.5`, `forceMax=4` | `PhysGrabber`/`PhysGrabObject` |
| 힘-질량 관계 | `힘 ∝ 1/mass`(가속도 모드) + `grabStrength`의 질량구간별 완만한 감쇠 | `PhysGrabObject` |
| 던지기 임펄스 | `Max(mass*1.5, 1) * clamp(당김벡터, forceMax) * (0.5+throwStrength)` | `PhysGrabObject` |
| 개당 가치 랜덤 범위 | `Random.Range(valueMin, valueMax)`(종류별 프리셋), 100 단위 반올림 | `ValuableObject` |
| 파손 연출 등급(impactForce) | Light `>20`, Medium `>80`, Heavy `>150` | `PhysGrabObjectImpactDetector` |
| 파손 판정 등급(breakForce) | Light `>300*mass`, Medium `>400*mass`, Heavy `>500*mass` | 〃 |
| 등급별 가치 손실 배율 | 기본 `0.01/0.05/0.1`, `durability`에 따라 최대 10배(`1+9*(100-durability)/100`) | 〃 |
| 완전 파괴 임계값 | `dollarValueCurrent < dollarValueOriginal * 0.15` | 〃 |
| 적 타격 시 강제 손실 비율 | Tiny/Small 0.2, Medium 약 0.143, 나머지 0.1(±0.05) | 〃 |
| 카트 질량 | 기본 8, 조작 중 4로 오버라이드 | `PhysGrabCart` |
| 추출 런 전체 목표 | `haulGoalMax * 0.7 * curve(난이도)` | `RoundDirector` |
| 지점별 목표(균등 분배) | `haulGoal / extractionPoints`(활성화 시점 계산) | `ExtractionPoint` |
| 지점 경고 카운트다운 | 3초 | `ExtractionPoint` |
| 잉여 프리팹 임계값 | 5000(중)/10000(대) | `ExtractionPoint` |
| `ItemValuableBox` 흡수 상한 | 5000(코드 기본값) | `ItemValuableBox` |

## CL9 시사점

이 절은 결정이 아니라 관찰이다.

- CL9의 계란은 `ValuableEgg`와 대응시킬 때, R.E.P.O. 쪽이 순수 귀중품(`ValuableObject`)이 아니라 트랩(`Trap`) 계열의 별도 상태 머신이라는 점이 눈에 띈다. 크기·질량이 단계마다 줄어들고, 최종 단계에서 폭발까지 가는 것은 "깨지는 계란"이라는 CL9 개념보다 한 단계 더 나아간 몬스터화 기믹이다.
- 가치는 스폰 시점의 1회 랜덤 굴림 + 파손에 따른 감산으로 확정되고, 레벨 전체 목표(추출 지점의 quota에 해당)는 사전에 디자이너가 못 박은 숫자가 아니라 그 레벨에 실제로 스폰된 귀중품 가치의 실측 합산에 난이도 배수를 곱해 나온다. CL9의 "오늘 오믈렛 3, 꿀버터 1" 같은 주문서 기반 쿼터와는 목표 산정 방식 자체가 다르다 — 하나는 스폰 결과 역산형, 하나는 사전에 주문서로 못 박는 방식이라는 차이가 있다.
- 파손에 따른 가치 손실은 등급(연출용 3단계, 판정용 3단계로 분리)과 `durability` 프리셋의 선형 보간으로 정해지며, 원가의 15% 밑으로 떨어지면 자동 소멸한다는 하한 규칙이 있다. "계란은 깨진다"는 CL9 요구를 손상 누적형(여러 번 맞아야 깨짐, 단계별 시각 변화)으로 구현할지 즉시 파괴형으로 할지는 이 소스가 제시하는 두 갈래 예시(`ValuableEgg`의 5단계 크랙 vs `BarrelValuable`의 히트 카운트 3회) 모두를 참고할 수 있다.
- 잡기·운반은 손이 잡은 지점에 개별 힘을 걸고 물리 엔진이 다중 인원의 힘을 자연 합성하는 방식이며, "무거운 짐을 들면 느려진다"는 CL9 요구와는 결이 다르다 — R.E.P.O.는 이동 속도 감쇠가 아니라 "가속도 자체가 질량에 반비례해 굼떠지는" 방식으로 무게감을 표현한다.
- 카트에 실린 짐은 고정되지 않고 자유 물리 오브젝트인 채로 남으며, 카트에 있는 동안 파손 판정이 아예 꺼진다는 점은 "운반 중 안전지대"라는 개념으로 참고할 만하다.
- 네트워크는 전 영역이 완전한 호스트(마스터 클라이언트) 권위이며, 물리 오브젝트의 소유권 자체를 플레이어에게 넘기지 않는다는 점이 특징적이다 — CL9의 리슨 서버 호스트 판정 방침과 구조적으로 가장 가깝게 대응되는 지점이다.
- 추출 목표를 지점 수로 균등 분배하고 초과분(잉여)을 물리적인 돈 오브젝트로 만들어 다음 지점에 자동 이월하는 방식은, 트럭이 맵 밖에 고정된 CL9 구조와는 다른 "다중 추출 지점 + 잉여 이월" 모델이라는 점이 관찰된다.
- 데이터 모델은 "종류당 하나의 ScriptableObject 프리셋(`Value`, `Durability`, `PhysAttribute`) + 그것을 참조하는 범용 런타임 컴포넌트(`ValuableObject`)" 구조로, CL9이 지향하는 "종류당 DataAsset 하나" 방침과 설계 원리 자체는 유사하다.
