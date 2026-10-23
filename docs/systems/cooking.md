# 요리 시스템

> 열원 위에서 시간이 지나면 재료가 변한다 — 날것 → 익음 → 탄 것. 그게 전부다.
> **설계 정본은 plan이다** — [../plan/loop.md](../plan/loop.md)의 "요리",
> [../plan/data/item/common.md](../plan/data/item/common.md)의 요리 컬럼.

## 개요

- **두 번째 게임이 아니다.** 요리는 미니게임이 아니라 원정과 같은 물리 조작이다 (plan/loop.md).
- 하는 일은 **놓고 · 기다리고 · 집기**뿐이다. 원정과 같은 물리 조작이다.
- 새로 생기는 시스템은 **열원 위 상태 변화** 하나뿐이다.
- **성공률·난이도·실패 판정이 없다.** 방치하면 타는 것이 유일한 실패다.

## 상태 변화 체인

데이터는 아이템 표의 요리 컬럼이다 (`CookResult` / `BurnResult` / `CookTime` / `BurnTime`):

```
계란 → (CookTime 12초) → 익은 계란(egg+cooked) → (BurnTime 24초) → 탄 계란(egg+burnt)
```

- 상태 변화는 시간이 흐르면 저절로 일어난다. 뒤집기·불 조절은 없다 (현재 설계).
- **탄 요리는 값이 0이다.** 그리고 연기가 난다 — `BurnWarnTime`(6초) 연기 기둥 예고는
  **렌더 전용**이라 판정에 안 들어간다.
- 탄 요리는 미끼가 된다 — **2026-10-17 구현**: 바닥에 멎으면 `decoy` 파동이 한 번 피고, `DecoyRadius` 안의 사냥 중이 아닌 몬스터가 와서 `DecoyEatTime` 동안
  먹고(사람 무시) 아이템이 사라진다 ([../goal/burnt-bait.md](../goal/burnt-bait.md), [monster.md](monster.md) 미끼).

## 구현 구조

요리는 별도 시스템이 아니라 **열원 액터 + 아이템 상태 전이**다.

```cpp
// 조리대 위에 아이템이 놓이면 — 호스트 전용 판정
void ACL9Kitchen::TickCooking(float Now) {
  if (!HasAuthority()) return;

  for (ACL9Item* Item : ItemsOnHeat) {
    const UCL9ItemDef* Def = DefRegistry->FindItem(Item->GetAlias(), Item->GetSubAlias());
    if (Def->CookResult.IsNone()) continue;   // 익지 않는 물건

    if (Now - Item->HeatStartTime >= Def->CookTime) {
      Item->SetAlias(Def->CookResult.ItemAlias, Def->CookResult.ItemSubAlias);  // egg → egg+cooked
      // SubAlias 교체 → OnRep으로 전 클라 메시/표현 갱신 (actor-base.md)
    }
  }
}
```

- 상태 전이는 아이템의 `Alias`+`SubAlias` 교체다 — 새 액터를 만들지 않는다.
- 놓기·집기는 운반 시스템의 조작 그대로다 ([inventory.md](inventory.md)). 요리 전용 조작이 없다.

## 열원은 조리대다

**요리는 부엌에서 한다.** 열원은 조리대(`ACL9Kitchen`)이고, **조리대 하나가 한 번에 하나를
익힌다** — 동시 요리 수는 조리대가 몇 개인가로 정해진다 (2026-09-06 사용자 결정).

- 상판에 여러 개를 올려도 먼저 자리를 잡은 하나만 시계가 돌고, 그것이 내려가면 남은 것 중
  하나가 이어받는다. 기다리는 재료는 열을 받지 않는다(게이지도 뜨지 않는다).
- **프라이팬·냄비는 열원이 아니다.** 그냥 아이템 하나일 뿐이며 요리와 무관하다
  ([../plan/data/equipment.md](../plan/data/equipment.md)).
- 조리대는 지금 레벨에 손으로 놓는다. **나중에는 던전 안에 맵 생성기가 배치한다**
  ([map.md](map.md)) — 요리하는 자리가 원정지 안에 있다는 뜻이다.
  **2026-10-12 구현(7c, [../goal/loop-7c-kitchen-in-building.md](../goal/loop-7c-kitchen-in-building.md))**: 생성된 맵의 `kitchen` 모듈마다 조리대 **하나**를
  호스트가 판 시작에 세운다(모듈 표 `PropSets[].Fixtures` — 작업대 자리, 벽에 등을 대고 방 안을 본다). 판이 끝나면 밤 개체와 함께 없어지고,
  스테이지 레벨의 로비에는 조리대가 없다 — 부엌을 찾아야 요리할 수 있다. 손으로 놓는 조리대는 생성기가 없는 시험 레벨 `Lvl_Flat` 의 3개뿐이다.
  양계장 1·돼지 농장 2·대농장 4개(사이트 `SpecialCopies`).

## 네트워크 동기화

- **상태 전이 판정**: 호스트 전용 (열원 액터에서, HasAuthority 관문)
- **표현**: Alias/SubAlias가 Replicated이므로 OnRep → LoadData로 자동 반영
  ([actor-base.md](actor-base.md))
- 연기·소리는 표현 계층에서 (conventions.md § 9)

## UI

진행도 바·취소 버튼·레시피 창이 없다 (현재 설계). 익음/탐은 **월드에서 읽힌다** —
메시 변화와 연기 기둥이 정보의 전부다.

## 참고

- 무엇이 무엇이 되는지: [../plan/data/item/common.md](../plan/data/item/common.md) 요리 절 (계란 체인은 [../plan/data/item/egg.md](../plan/data/item/egg.md))
- 완성물 판매·납품: [economy.md](economy.md) · [order.md](order.md)
- 먹기 효과 (Dish 타입): [../plan/data/item/common.md](../plan/data/item/common.md) 타입과 먹기 절

---

## 구현 상태 (④ 4d — 2026-09-06, 열원 재설계 반영)

`ACL9Kitchen`(Source/CL9/Items, BP `Content/Items/BP_CL9Kitchen`) — 레벨에 놓는 고정물이며 상호작용이 없다
(놓기·집기가 곧 조작이라 `ICL9Interactable`을 구현하지 않는다).

- 상판 위 `HeatZone`(몸체 크기에서 만들어지는, 상판 위 60cm 상자)에 **놓인**(Free) 아이템을 호스트가 매 틱 질의한다.
  열 구역은 몸체 스케일과 무관하게(절대 스케일) 계산되므로 조리대를 넓혀도 열 높이는 그대로다.
- **한 번에 하나만 익는다.** 자리가 빈 동안에만 다음 재료를 집어 들고, 기다리는 재료는 `None`으로 지워 게이지가
  뜨지 않게 한다. 익히던 것이 집혀 가거나 굴러 나가면 시계는 0으로 돌아간다.
- 행에 `CookResult`가 있으면 `CookTime` 뒤 그 행으로, 없고 `BurnResult`만 있으면 `BurnTime` 뒤 그 행으로 `SetAlias`
  (액터 교체 없음 — OnRep으로 클라 메시 갱신). 탄 것이 되면 자리를 비워 다음 재료가 올라온다.
- **진행도는 아이템에 실려 복제된다** — `ACL9Item`의 `GetCookProgress()`(0~1) · `IsOnHeat()` · `IsNextBurnt()` ·
  `GetCookPhase()`(`ECL9CookPhase` None/Cooking/Burning). 복제 실체는 `uint8` 하나와 enum 하나로, 표현(게이지) 전용이다.
  HUD의 요리 게이지가 이걸 읽는다(UI 세션).
- **"무엇을 익히는 중인가"는 조리대에 실려 복제된다** — `ACL9Kitchen::GetCookingItem()`. HUD가 익히는 중인
  조리대마다 그 꼭대기(`GetTopLocation()`) 위에 게이지를 띄우는데, 클라도 조리대↔재료 짝을 알아야 해서다
  (2026-09-10 — ui.md 조리 게이지).
- `BurnWarnTime` 전 "연기" 로그는 그대로 있고(호스트), 연기·김·불꽃·소리 표현은 2026-10-09에 붙었다 — 아래 "요리 진행 표현".
- **불(버너)은 아직 없다** — 조리대는 늘 뜨겁다.

조작은 운반 그대로: 재료를 들고 조리대 앞에서 **놓기(G)**를 누르면 상판 위에 살며시 올라간다 — 아래 "놓기 — 상판 안착"
([input.md](input.md) `IA_Drop` · [inventory.md](inventory.md) 놓기 행. E는 줍기·상호작용이고 놓기가 아니다).

### 놓기 — 상판 안착 (2026-10-09, 사용자 "상판 위에 살며시 놓기" — [../goal/cooking-progress-feedback.md](../goal/cooking-progress-feedback.md) F1)

**손 높이 낙하로 계란이 금 가는 문제(4d 미결)는 ② "놓기를 낙하 없이 안착으로"로 풀렸다.** 조리대 높이로 낙차를 줄이는 것으로는
안 됐다 — 놓기가 손 자리(폰 25 cm 앞·24 cm 오른쪽)에서 떨어뜨리므로, 조리대 앞(앞 가장자리에서 42 cm)에 서면 손이 상판 앞 가장자리보다
20 cm 앞이라 계란이 상판이 아니라 바닥에 떨어져 545 cm/s로 부딪혀(`CrackSpeed` 초과) 금이 갔고, 금 간 계란은 익지 않는다(2026-10-09 실측).
"[G] 올려서 익히기" 안내와 동작이 어긋났다. 그래서 그 안내가 뜨는 조건이면 G는 떨어뜨리지 않고 올린다:

- **조건** — 놓기 안내와 같다: 활성 칸 행에 `CookResult`나 `BurnResult`가 있고(날계란·익은 계란), `ResolveFocus`의 조리대 선택(`PickRadius` 오버랩에
  걸리고, 캡슐 중심에서 몸체의 가장 가까운 점까지 `PickRadius` 이하, 그 방향과 조준의 내적 `FocusMinAimDot` 이상, 여럿이면 내적이 가장 큰 것)이
  조리대를 고른다. 같은 조리대 위의 다른 재료가 조준돼 프롬프트가 "[E] 줍기"여도 G는 올린다(키가 다르다).
- **판정은 호스트** — `ACL9Character::Server_Drop`이 자기 쪽 캡슐 위치·컨트롤 회전으로 `ResolveFocus(&조리대)`를 다시 부른다. 클라가 고른 조리대는
  받지 않는다 — **새 RPC 인자·복제 속성 없음**(놓기 요청 `Server_Drop()` 그대로, 결과는 아이템의 `HoldState`·복제 이동으로 간다).
- **자리** — `ACL9Kitchen::FindPlaceLocation(Item, 캡슐 중심, Out)`: 상판 = 몸체 메시 로컬 상자의 윗면(`GetClosestBodyPoint`와 같은 상자,
  `SM_Kitchen_A`는 상판 위로 솟는 부분이 없다). 재료의 수평 반경만큼 안쪽으로 당긴 직사각형 위에서 캡슐 중심에 수평으로 가장 가까운 점 —
  조리대 앞에 서면 앞 가장자리에서 반경만큼 안쪽, 내 앞자리다. 높이는 재료 밑면이 상판 면에 닿게 원점 = 상판 + 밑면 오프셋.
  - 재료 크기는 `ACL9Item::GetUprightBodySize` — 구 몸체(날계란, `Radius` 21)는 반경·밑면 모두 반지름, 메시 몸체(`bMeshCollisionBody` 익은·탄 계란)는
    그 메시의 로컬 바운드 상자에서 원점→가장 먼 수평 모서리(어느 요로 놓여도 이 원 안)와 원점→밑면(바닥 중앙 피벗이면 0).
  - 회전은 요만 남긴다(`ACL9Item::OnPlaced(위치, 회전)`) — 원반이 눕고 밑면 계산이 맞는다. 위치는 텔레포트, 선속도·각속도 0, `Place` 소리는 보통 놓기와 같다.
- **겹치지 않게** — 열 구역(`HeatZone`) 안의 자유 아이템(익지 않는 것·탄 것 포함)을 수평 원(중심, `GetUprightBodySize` 반경)으로 보고, 가장 가까운 점이
  어느 것과 겹치면 당긴 직사각형을 5 cm 격자로 훑어 겹치지 않는 점 중 캡슐 중심에 가장 가까운 점으로 간다(대개 앞줄 옆자리). **물리에 맡기지 않는
  이유**: 구 몸체는 초기 겹침 해소 속도를 따로 정하지 않아(바디 기본 −1 → 솔버 기본 −1, 상한 없음) 겹친 채 텔레포트하면 둘이 튕겨 나가며
  `CrackSpeed`(450)를 넘길 수 있다(원반 몸체는 0이라 튀지 않고 겹친 채 남는다 — 그것도 보기 나쁘다). 빈 자리가 없으면 경고 로그를 남기고 아래 보통 경로(짐칸·손 자리 낙하)로 간다.
- **우선순위** — 조리대 안착 > 짐칸 얹기 > 손 자리 낙하. 안내가 "[G] 올려서 익히기"를 띄운 채 짐칸으로 가면 안내와 어긋나기 때문이다. 던지기(달리기 키+G)·
  창구 납품·수레 놓기(E)는 그대로다.
- 로그: 호스트 `Kitchen place '<아이템>' (<Alias>/<SubAlias>) → '<조리대>' at <위치> (hand was <손 자리>)`, 자리를 비켜 갔으면 조리대가
  `nearest plate spot … is taken — moved N cm`, 빈 자리가 없으면 `no free spot on the plate` (Warning).
- 바닥에 놓아 금이 가는 것 자체는 여전히 규칙이다(2026-09-16 — 깨짐은 이 게임의 마찰, decisions.md). 바뀐 것은 조리대 앞의 G 하나다.

### 부엌 세트 — 꾸밈·불꽃·가스불 소리·놓기 안내 (2026-10-09, [../goal/kitchen-identity.md](../goal/kitchen-identity.md))

조리대가 "그냥 흔한 책상"으로 읽힌다는 신고에 대한 답이다. **전부 표현이고 요리 판정은 하나도 바뀌지 않는다** —
`TickCooking`·`GatherOnHeat`·`HeatZone`·`GetTopLocation()`·복제 속성(`Cooking`) 그대로, 새 RPC·복제 속성·틱 없음.

- **컴포넌트 셋** — `ACL9Kitchen` 생성자가 만들고 셋 다 몸체(`MeshComponent` = `SM_Kitchen_A`)에 **상대 변환 0**으로 붙는다.
  아트가 상판과 같은 로컬 좌표계(바닥 중앙 피벗, +X 정면)로 만들고, BP가 몸체에 준 요 90°를 자식이 같이 받는다. 메시는
  `BP_CL9Kitchen`에서 지정한다(코드에 경로 없음). 치수·tris·UCX는 [../asset/map/kitchen.md](../asset/map/kitchen.md).
  | 컴포넌트 | 종류 | 무엇 | 충돌·그림자·내비 |
  |---|---|---|---|
  | `DressingMesh` | `UStaticMeshComponent` | 화구·다이얼·벽판·걸린 프라이팬/국자/뒤집개·후드·연통·부엌 표지·냄비(선반 위, 상판 면적 밖) | `BlockAllDynamic`에 **카메라·WorldStatic 채널 무시**. 막는 모양은 메시 UCX(벽판·냄비 받침만 — 도구·후드·연통·표지는 UCX 없음). 그림자 있음, 내비 영향 있음(벽판이 막는다) |
  | `PilotFlameMesh` | `UStaticMeshComponent` | 화구마다 작은 파란 불꽃 — 깜빡임은 재질(`M_CL9_Kitchen_PilotFlame`) | 충돌 없음·그림자 없음·내비 영향 없음·간접광 없음(`bAffectDynamicIndirectLighting=false` — Lumen GI가 발광으로 상판을 물들이지 않게) |
  | `AmbienceAudio` | `UAudioComponent` (`bAutoActivate=false`) | 가스 쉬익 + 후드 팬 웅웅 루프 | — |
- **열 구역·게이지 자리가 그대로인 이유** — 둘 다 `MeshComponent->Bounds`에서 나오는데, 컴포넌트의 `Bounds`는 그 컴포넌트
  하나의 것이라 자식이 아무리 높아도(후드 ≥ 200 cm) 섞이지 않는다. 꾸밈은 상판 면적 안 90~200 cm에 아무것도 두지 않으므로
  놓기·집기·익기도 그대로다. 꾸밈이 `WorldDynamic`이라 이웃 조리대의 열 구역 질의에 걸릴 수는 있지만 `ACL9Item`이 아니라 걸러진다.
- **꾸밈의 충돌은 Simple And Complex다** — 폰·물리·단순 트레이스는 UCX(벽판·냄비 받침)에만 맞지만, **복잡 트레이스**(`bTraceComplex`)는
  UCX 없는 후드·도구 삼각형에도 맞는다. 조준·줍기·놓기·카메라는 복잡 트레이스를 쓰지 않는다(2026-10-09 확인 — 조준·수레·짐칸은
  오브젝트 타입 오버랩, 휘두르기·스캔·몬스터 시야·파동 가림은 `bTraceComplex=false`, 놓기는 트레이스 없음(상판 안착은 조준과 같은 오버랩 + 열 구역 상자 오브젝트 타입 오버랩), 인트로 카메라는 `WorldStatic`
  오브젝트 스윕이라 `WorldDynamic`인 꾸밈에 안 맞는다). 유일한 복잡 트레이스는 평지 스폰 지면 붙이기(`ACL9GameMode::FindSpawnPoint`,
  기본 질의 인자 = 복잡, `ECC_WorldStatic` 채널)다. 그래서 꾸밈은 **`ECC_WorldStatic` 채널도 무시한다** — 그 지면 붙이기가 후드·벽판·선반을
  지나쳐 꾸밈 전과 똑같은 자리(바닥이나 상판)에 스폰한다(명세 Must-Preserve). 이 채널 트레이스는 그것 하나뿐이고 WorldStatic 오브젝트는
  움직이지 않으므로 폰·물리·조준에는 영향이 없다.

- **불꽃은 늘 켜져 있다** — 조리대가 늘 뜨겁기 때문이다(버너 켜고 끄기는 아직 없다). **광원 컴포넌트가 아니다** — 재질 발광만이고
  그 발광도 간접광(Lumen GI)에서 빠지므로 어둠 밸런스(light.md)가 바뀌지 않는다. 꾸밈과 따로 둔 것은 요리 진행 표현(익히는 동안 불꽃이 커진다)이 이것만
  움직이게 하려는 것이다 — 아래 절의 CPD[0] 불꽃 키움.
- **가스불 소리** — `ACL9Kitchen::BeginPlay`가 `CL9Sound::Resolve(this, CL9SoundNames::KitchenAmbience)`로 음원을 받아 `AmbienceAudio`에
  넣고 `Play()`. 전용 서버(프로세스 또는 월드 넷 모드)면 `Resolve`가 null이라 안 튼다 — 리슨 호스트·클라는 각자 자기 조리대에서 튼다.
  반복·감쇠·동시 발성 제한은 사운드 애셋에 있고, 행·애셋이 없으면 조용히 안 튼다(로그 없음). 몬스터가 듣는 소음이 아니다
  ([sound.md](sound.md) `KitchenAmbience`).
- **놓기 안내 "[G] 올려서 익히기"** (`Prompt.KitchenPlace`, ui.md 조리대 놓기 안내) — 2026-09-13 "놓기 프롬프트는 없다"의 유일한 예외.
  `ACL9Character`가 만든다:
  - 조건: 로컬 폰의 `HeldItem` 행에 `CookResult`나 `BurnResult`가 있고(`GatherOnHeat`와 같은 잣대 — 날계란·익은 계란은 뜨고,
    탄 계란·프라이팬·빈손은 안 뜬다), 조리대를 바라본다.
  - "바라본다": 조준(`ResolveFocus`)이 이미 하는 `PickRadius` 구 오버랩에 조리대가 걸리고, **캡슐 중심(`GetActorLocation()`)에서
    몸체 상자의 가장 가까운 점(`ACL9Kitchen::GetClosestBodyPoint`)까지가 `PickRadius` 이하**이며, 그 방향과 조준 방향의 내적이
    `FocusMinAimDot`(0.3) 이상. 액터 원점(상판 가운데 바닥)이 아니라 가장 가까운 점이라 300 cm 상판의 끝자리 앞에서도 맞다.
    몸체 상자는 `SM_Kitchen_A`의 로컬 상자를 몸체 변환으로 돌린 것이고 꾸밈은 들어가지 않는다. 여럿이면 내적이 가장 큰 것.
    새 월드 스캔은 없다.
  - 조리대는 포커스가 되지 않는다 — 윤곽선이 켜지지 않고 E(`Server_PickPlace`)의 대상도 아니다. 이 조건에서 G는 손 자리 낙하가 아니라 상판 안착이다(위 "놓기 — 상판 안착", 2026-10-09 —
    이 안내를 붙인 kitchen-identity 때는 아직 손 자리 낙하였다).
  - 우선순위: 착용 > 줍기 > 상호작용 > **조리대 놓기** > 수레 놓기. 처형·데스캠·인트로 잠금 중에는 없다(포커스와 같은 게이트).
  - `{0}`은 놓기 키(`DropAction`)의 지금 표기 — `RefreshKeyText`가 줍기 키와 함께 캐시하므로 키를 바꾸면 문구도 따라간다.

### 요리 진행 표현 — 불꽃·상판 열기·아지랑이·김·연기·소리·갈변 (2026-10-09, [../goal/cooking-progress-feedback.md](../goal/cooking-progress-feedback.md))

"익을 때 주변에 열기가 나온다던가 그런 거 없어?"에 대한 답이다. **전부 표현이고 판정은 하나도 바뀌지 않는다** — `TickCooking`·`GatherOnHeat`·
`HeatZone`·`GetTopLocation()`·복제 속성 그대로, **새 복제 속성·RPC·Multicast·틱 클래스 없음**. 전용 서버가 아닌 모든 머신이 이미 복제되는
것에서 매 틱 같은 규칙으로 파생한다: 조리대의 `Cooking`(`GetCookingItem()`), 그 재료의 `CookPhase`·`CookAlpha`(`GetCookPhase()`·
`GetCookProgress()`)와 `Alias`/`SubAlias`, 그리고 로컬 표의 그 행 값(`BurnTime`·`BurnWarnTime`·`CookResult`·`BurnResult`). 전용 서버(월드 넷 모드)는
아무것도 켜지 않는다 — 파생 함수 자체를 돌지 않는다. 메시·재질·음원은 BP·`SoundTable`에서 오고 코드에 경로가 없다.

**무엇이 무엇을 움직이나** — 조리대는 `ACL9Kitchen::TickLook`(조리대의 기존 틱, 호스트에서는 `TickCooking` 다음이라 이번 틱의 전이를 같은 틱에 본다),
재료는 `ACL9Item::TickCookLook`(아이템의 기존 틱, 시뮬레이션 관문 앞). 컴포넌트는 **값이 바뀔 때만** 건드린다 — CPD는 씬 갱신 하나(렌더 상태를
다시 만들지 않는다), 보이기·오버레이·루프 Play/Stop은 단계가 바뀔 때만.

**조리대 표현 단계** (`ECL9KitchenLook`):

| 단계 | 조건 (익히는 재료의 복제 상태) | 불꽃 키움 | 상판 발광·아지랑이 | 김 (`SteamMesh`) | 루프 |
|---|---|---|---|---|---|
| `Idle` | 익히는 것이 없거나 그 재료의 단계가 None | 0.6초에 걸쳐 0으로 | 불꽃 키움과 함께 사그라들고 0이면 숨김 | 마지막 값 × 불꽃 키움으로 사그라들고 0이면 숨김 | 지글·탁탁 멈춤(가스불 소리는 그대로) |
| `Cooking` | `CookPhase == Cooking` | 0.4초에 걸쳐 1로 | 세기 = 불꽃 키움 | 밀도 0.3 → 1(진행 따라), 연기 0 | 지글 |
| `Burning` | `CookPhase == Burning` ∧ 진행 < 경고 시작 | 1 | 같음 | 밀도 1 → 0.4(옅어진다), 연기 0 | 지글 |
| `BurnWarning` | `CookPhase == Burning` ∧ 진행 ≥ 경고 시작 | 1 | 같음 | 밀도 0.4 → 1, 연기 0 → 1(경고 구간의 앞 25 %에 다 바뀐다) | 지글 + 탁탁 |

- 경고 시작 = `1 − BurnWarnTime / BurnTime`(행 값, 0~1로 자른다. `BurnTime` 0이면 0) — 판정에는 쓰이지 않는다(호스트의 "연기" 로그와 같은 지점).
- 김 CPD[0] = 표의 밀도 × 불꽃 키움 — 재료를 내리면 김도 불꽃과 함께 0.6초에 사그라든다. 비어 있는 동안 카드는 마지막 자리에 남는다.
- **Idle은 kitchen-identity 뒤 모습 그대로다** — 불꽃 CPD 0, 나머지 넷 숨김, 요리 루프 둘 정지. 재질의 CPD 기본값은 Idle 값(0)이어야 한다:
  엔진은 한 번도 쓰지 않은 CPD 칸에 0을 쓰면 씬을 갱신하지 않으므로(같은 값으로 본다) 첫 틱에는 재질 기본값이 보인다.
- 값: 0.4초·0.6초·1.2초(덩어리)는 계약 값, 김·연기 곡선(0.3·0.4·25 %)은 `CL9Kitchen.cpp` 익명 이름공간의 **표현 상수**다(표 열이 아니다 —
  판정과 무관, PIE로 맞춘다. `ACL9PowerSwitch` 빛 상수와 같은 자리매김).

**조리대 컴포넌트** (C++ 기본 서브오브젝트, 메시·재질은 `BP_CL9Kitchen`. 넷 다 충돌 없음 · 그림자 없음 · 내비 영향 없음 ·
`bAffectDynamicIndirectLighting=false` · 처음엔 숨김(`SetVisibility(false)`). **광원 컴포넌트는 없다**. 정적 메시·오디오라 스스로 틱하지 않는다):

| 컴포넌트 | 종류 · 붙는 곳 | CustomPrimitiveData (코드가 쓴다) |
|---|---|---|
| `PilotFlameMesh` (기존) | — | [0] 불꽃 키움 0(작은 파란 불)~1(센 불) |
| `PlateGlowMesh` | `UStaticMeshComponent`, `MeshComponent` 자식 상대 변환 0 | [0] 세기 = 불꽃 키움 · [1] 뜨거운 자리 X · [2] Y — 익히는 재료의 액터 위치를 **이 메시의 로컬로 옮겨, 메시 로컬 바운드의 중심·반폭으로 정규화**한 −1~1(재질은 `ObjectLocalBounds`로 같은 식을 쓴다. 메시가 없으면 몸체 메시의 로컬 상자). 익히는 동안만 갱신, 비면 마지막 자리 |
| `HeatHazeMesh` | 같음, Movable · 절대 스케일 | [0] 세기 = 불꽃 키움. 익히는 동안 매 틱 **재료 위**로(바뀔 때만 옮긴다) |
| `SteamMesh` | 같음 | [0] 밀도 · [1] 연기 정도 0(하얀 김)~1(회색 연기). 아지랑이와 같은 자리 |
| `PuffMesh` | 같음 | [0] 나이 0~1(전이 순간 0, 1.2초에 1, 1이면 숨김) · [1] 어둡기 0(익음 — 하얀 김 덩어리)/1(탐 — 검은 연기). 전이 순간의 재료 위에 한 번 놓이고 따라가지 않는다 |
| `SizzleAudio` | `UAudioComponent`(`bAutoActivate=false`), `MeshComponent` 자식 | 행 `CookSizzle` — Cooking·Burning·BurnWarning 동안 |
| `CrackleAudio` | 같음 | 행 `CookCrackle` — BurnWarning 동안만 |

"재료 위" = 재료 메시 바운드의 윗면 가운데 — 카드(바닥 중앙 피벗)가 놓이는 자리다. 카드는 몸체 스케일을 받지 않아(절대 스케일) 제 크기다.
루프 음원은 `BeginPlay`가 `CL9Sound::Resolve`로 받아 넣어 둔다(가스불 소리와 같은 방식·같은 셈 — [sound.md](sound.md)). 감쇠·동시 발성(최대 2,
`SCC_CookSizzle`·`SCC_CookCrackle`)·가상화는 애셋에 있다.

**전이 한 번 — 소리·덩어리** (`ACL9Kitchen::TickLookEdges`, conventions § 9 — OnRep·RPC·Multicast 없이 각 머신이 틱 비교로 한 번씩):

1. 지켜보는 재료와 그 행(Alias·SubAlias, 그 행의 `CookResult`·`BurnResult`)을 지난 틱에 적어 둔다.
2. 이번 틱에 그 재료의 (Alias, SubAlias)가 달라졌으면: 새 행 = 지난 행의 `CookResult` → `CookDone` 한 번(`PlayAt`, 재료 자리) + 하얀 덩어리,
   새 행 = 지난 행의 `BurnResult` → `CookBurnt` 한 번 + 검은 덩어리. 그 밖의 행 변화(열 위에서 금 감 등)는 아무것도 없다.
3. 다음에 지켜볼 것 = 지금 익히는 것, 없으면 **아직 열 위로 보이는(`IsOnHeat`) 지난 것**, 그것도 아니면 없음. 처음 보는 재료는 기준값만 적는다 —
   늦게 들어온 클라가 지난 전이를 되풀이하지 않는다.

3의 "아직 열 위로 보이는 지난 것"은 클라를 위한 것이다: 탐 순간 호스트는 재료의 행 교체·단계 None과 조리대의 `Cooking = null`을 같은 틱에 하지만,
둘은 다른 액터라 클라에 따로 도착한다. `Cooking = null`이 먼저 와도 재료의 단계는 아직 Burning이라 붙들고 있다가, 재료가 도착하는 틱에 전이를 잡는다.
들려 간 것·다 탄 것은 단계가 None이 되어 놓는다 — 다른 조리대에서 다시 익을 때 두 조리대가 같은 전이를 겹쳐 내지 않는다.
그래서 전이 하나에 귀가 있는 월드마다 정확히 한 번이다(`GetPlayCount(CookDone|CookBurnt, true)` +1, 전용 서버 월드는 0).

**재료** (`ACL9Item`):

| 이름 | 무엇 |
|---|---|
| `CookOverlayMaterial` (`EditDefaultsOnly`, `TObjectPtr<UMaterialInterface>`) | `BP_CL9Item`에서 `M_CL9_Cook_Overlay`를 넣는다. 열 위(`CookPhase != None`)에 있는 동안만 `MeshComponent`의 오버레이로 건다. 조준 윤곽선(`ACL9Character::SetFocusHighlight`)이 같은 오버레이 칸을 잠시 쓰므로 **빈 칸일 때만 걸고 내가 건 것만 걷는다** — 윤곽선이 걷히면 다음 틱에 다시 걸린다. 비어 있으면 덧칠 없음 |
| 메시 CPD [0] 익음 색 | `Cooking` ∧ 행에 `CookResult` 있음: 진행 × 0.45, 그 밖 0. 행 조건은 호스트에서 익음으로 넘어간 그 틱(행은 이미 익음, 단계는 한 틱 Cooking)에 익은 메시를 덧칠하지 않으려는 것 |
| 메시 CPD [1] 그을음 | `Burning`: 진행 × 0.6, 그 밖 0 — 행이 바뀌면(익음·탐 메시로 교체) 단계·진행이 새로 시작하므로 둘 다 0부터 다시, 열에서 내려가면 0 |
| `BurntSmokeMesh` (`UStaticMeshComponent`) | `SM_Cook_Steam_A` + `M_CL9_Cook_Steam`. **탄 행 ∧ `HoldState == Free`** 동안 보인다 — 손(Held)·머리(Worn)는 꺼지고, 비활성 퀵슬롯 칸은 액터가 없다. CPD [0] 밀도 0.25 · [1] 연기 1. 구(몸체) 중심에 붙되 절대 회전·스케일이라 굴러도 똑바로 서고 메시 자동 스케일과 무관하다. 충돌 없음(물리 몸체에 용접되지 않는다)·그림자·내비·간접광 없음, 처음엔 숨김. `Mesh` 다음에 만든다 — 윤곽선이 첫 정적 메시(`FindComponentByClass`)를 고른다 |

- **탄 행 = 어느 행의 `BurnResult`가 가리키는 행**이다(`IsBurntRow()`). `LoadData`가 행이 바뀔 때 표의 아이템 행 목록(`GetItems()`, 수십 개)을 한 번
  훑어 정한다 — 별칭 이름(`burnt`)을 코드에 두지 않는다. 모든 머신이 같은 표를 가지므로 같은 답이다.
- **미결 — 수레 짐칸 안**: 짐칸의 짐도 `Free`라(부착이 아니라 위치가 진실 — [../plan/data/item/cart.md](../plan/data/item/cart.md)) 지금은 짐칸에 실린
  탄 재료에도 연기가 난다. 계약은 "짐칸이 아님"이다 — 판정 방법이 정해지면 붙인다(실행 보고서 질문).

**PIE 확인용 게터** (`BlueprintPure`, 에디터 파이썬에서 읽는다 — 이 머신이 이번 틱에 쓴 값 그대로, 표현 코드는 읽지 않는다):
조리대 `GetCookLook()`·`GetFlameFlare()`·`GetPlateGlowStrength()`·`GetPlateHotSpot()`·`GetHeatHazeStrength()`·`GetSteamDensity()`·`GetSmokeAmount()`·
`GetPuffAge()`·`GetPuffDarkness()`, 재료 `GetCookTint()`·`GetCharTint()`·`IsCookOverlayOn()`·`IsBurntSmokeOn()`·`IsBurntRow()`. 보이기는 컴포넌트의
`is_visible()`, 루프는 `SizzleAudio`/`CrackleAudio`의 `is_playing()`, 한 번 소리는 `UCL9FeelDebug::GetSoundCount`로.

**히치** — 익기 시작하는 순간에 로드가 없다: 루프 음원은 `GameInstance::Init`이 미리 올려 붙들고(`SoundTable` 전 행) `BeginPlay`에서 컴포넌트에
넣어 두며, 카드 메시·재질과 `CookOverlayMaterial`은 BP의 하드 참조라 클래스와 함께 올라온다. 남는 첫 비용은 엔진 쪽 — 보이기를 켜거나
오버레이를 거는 순간의 렌더 상태 재생성과 `SetOverlayMaterial`의 PSO 프리캐시다(명세 B10/U로 확인).

**익는 순간 몸체가 구에서 원반으로 (계란후라이 — 2026-10-09, 표 열 `bMeshCollisionBody`, [../plan/data/item/common.md](../plan/data/item/common.md) 물리)**.
날계란은 반경 구(`Radius` 21) 몸체이고, 익은·탄 계란은 납작한 후라이(`SM_Egg_Cooked_A`·`SM_Egg_Burnt_A`, 바닥 중앙 피벗, UCX 원반 1개)라 구로는 눕지 못한다.
그래서 그 두 행은 `bMeshCollisionBody`를 켜고, 행이 바뀌는 순간(상판 위, `TickCooking`의 `SetAlias` → `LoadData`) 몸체가 넘어간다 — 판정은 그대로다.

- 몸체는 `ACL9Item::BodyMeshComponent`(`"BodyMesh"`) — 행의 메시를 받아 그 UCX로 시뮬레이션하는 **보이지 않는 루트**다. 보이는 것은 그대로 `Mesh`(구 아래,
  실물 크기 — 자동 배율 없음)라 1인칭 손 옮기기·조준 윤곽선·갈변 덧칠·탄 연기가 손대지 않고 돈다. 채널·응답은 구와 같다(`WorldDynamic`, 모두 막음, 폰 겹침,
  카메라 무시, 충돌 알림 → `OnPhysicsHit`) — 열 구역 질의(`GatherOnHeat`)·조준(`ResolveFocus`)·짐칸·창구가 구와 같이 잡는다. 얇은 원반이라 CCD를 켜고,
  초기 겹침 해소 속도는 0(겹쳐도 튀어 나가지 않는다 — 수레와 같은 값).
- **튀거나 뚫지 않게**: 구 행에서 넘어올 때 원반 밑면을 구 밑면에 맞춘다 — 호스트는 구 중심에서 옛 `Radius`만큼 내리고, 클라는 호스트가 같은 갱신에 보낸
  루트 자리(`ReplicatedMovement`)를 쓴 뒤 그 복제 목표를 새 루트에 다시 건다. 구르던 구의 기울기는 버리고 요만 남긴다(원반이 눕는다). 익음 → 탐은 같은
  몸체가 그 자리에서 탄 메시로 바디만 새로 만든다.
- 손에 든 채 스폰된 것(퀵슬롯 전환으로 다시 나온 익은 계란)은 구의 부착을 몸체가 같은 상대 변환으로 물려받는다. 첫 로드(스폰)는 스폰 자리 그대로다.
- 한 방향이다 — 표의 사슬이 앞으로만 가므로(날 → 익음 → 탐) 원반이 구로 돌아가는 길은 없다.
- 무게: `Mass` 0이면 엔진이 원반 부피로 정한다. 로드 때 로그 `mesh body half-size …, mass … kg`가 실제 값을 보여 준다(날계란 구도 엔진 계산이다).
