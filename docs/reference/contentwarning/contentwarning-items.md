# 참고 — Content Warning의 카메라·점수·도구: 공포가 보상이 되는 구조

> 2026-10-17. 설치본(Steam buildid 22999514, Unity 6000.0.67f1, Mono, Photon PUN)의 `Assembly-CSharp.dll`을 MCP 도구
> `unity-analysis`로 디컴파일해 촬영·채점·업로드·도구 코드를 읽고, 채점 상수가 모인 ScriptableObject `BigNumbers`(`resources.assets:22522`),
> `Item` 68개, 카메라·도구 프리팹의 MonoBehaviour 실값을 타입트리(백엔드 AssetsTools)로 읽었다. **코드 초기값 ≠ 에셋 실값**이라 다르면 둘 다 적었다.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다. 인계 문서 §5의 질문 3·5를 다룬다.
> 몬스터 쪽은 [contentwarning-monsters.md](contentwarning-monsters.md), 한 판 루프는 [contentwarning-run.md](contentwarning-run.md).

## 결론

- **점수는 "카메라 화면에 무엇이 얼마나 오래, 얼마나 크게 잡혔나"다.** 녹화 중 0.15초마다 카메라 화면에 20×20 = 400개 광선을 쏘아
  `ContentProvider`가 붙은 물체(몬스터·플레이어·아티팩트)에 맞은 비율을 재고, 물체마다 `기본값 × 화면비율곡선 × 지속곡선`으로 점수를 매긴다.
- **같은 대상은 한 번만 센다.** 녹화 전체에서 같은 몬스터 개체(같은 `viewID`)는 가장 좋은 한 장면만 남는다 — `MaxSameEventType` 실값 **1**(코드 3).
  그래서 점수를 올리는 길은 한 놈을 오래 찍기보다 **다른 놈을 많이**, **화면 30% 크기로**, **15초 가까이 끊김 없이** 찍는 것이다.
- **무서울수록 비싸다.** 기본값이 Zombe 25 · Mouthe 40에서 BigSlap(분노) 200 · Flicker 350까지 벌어지고, **같은 BigSlap이라도 분노 중이면 200, 평온하면 100**이다.
  동료는 서 있으면 5~10, 넘어지면 15, 죽으면 15, 큰 낙하 20이고, **맞는 장면은 피해량 그대로**(Mime 한 방 105)다.
- **조합 보너스가 공포를 연출하게 만든다.** 한 장면에 몬스터 둘 이상이면 합의 0.3배가 덤으로, 리포터 마이크를 든 동료가 몬스터 10m 안이면 0.2배(인터뷰),
  감정표현 중인 동료가 15m 안이면 0.2배(도발)가 붙는다.
- **도구는 몬스터를 죽이지 못한다.** 몬스터에게 체력이 없다. 전기 충격봉은 5초 넘어뜨리고, 구조 갈고리는 붙잡힌 동료를 떼어 내고, 손전등은 두 종(Snatcho·EyeGuy)의
  행동을 바꾼다. **손전등은 몸에 붙은 것이 아니라 4칸 인벤토리의 아이템**이라, 손에 든 한 칸만 쓴다 — 카메라를 들면 손전등을 못 든다. 대신 카메라에는 **녹화 중에만 켜지는 자체 조명**이 있다.

---

## 1. 촬영 — `VideoCamera`

| 항목 | 값 | 근거 |
|---|---|---|
| 필름 | 카메라 하나에 **90초** (새 데이터 항목을 만들 때의 코드 값, 녹화 중 `unscaledDeltaTime`으로 줄어든다) | `VideoCamera.cs:146-157,293-301` |
| 시야각 / 줌 | **60° / 5배**(코드 40° / 2배) — 줌 끝에서 12° | `resources.assets:27012`, `VideoCamera.cs:251,276` |
| 채점 주기 | **0.15초**마다 한 번 마감(`m_contentPollingCooldown`), 프레임마다 두 줄씩 미리 쏜다 | `VideoCamera.cs:344-361`, `ContentPolling.cs:34-49` |
| 광선 | 화면 20×20 격자, 길이 200m, 첫 충돌체의 부모에서 `ContentProvider`를 찾는다 | `ContentPolling.cs:51-74` |
| 화면 비율 | 대상별 맞은 광선 수 / 400 → `seenAmount` | `ContentPolling.cs:91-95` |
| 판정 주체 | **카메라를 든(시뮬레이션하는) 클라이언트** — `isSimulatedByMe`일 때만 채점 | `VideoCamera.cs:344-361` |
| 자체 조명 | 프리팹의 `Lights` 묶음이 **녹화 중에만** 켜지고(`enableWhenRecording`), 정면/셀카 모드에 따라 `Light`·`FlipLight` 중 하나가 남는다. 둘 다 `FlashLightTrigger` 밝기 **0** | `resources.assets:27012`(배열 → GameObject `Lights`·`Light`·`FlipLight`), `:26920`, `:27501` |
| 필름 소진 | 필름이 0이면 호스트가 목표를 "잠수종으로 돌아가라"로 바꾼다 (산소 절반에서도 같은 전환 — [run](contentwarning-run.md)) | `VideoCamera.cs:302-305` |
| 바닥에 놓인 카메라 | 줍기 전까지 2초마다 소리·불빛 깜빡임(`bleepInterval` 2, 코드 1 / 5초 뒤부터, 코드 2) | `VideoCamera.cs:225-236`, `resources.assets:27012` |

CamCreep은 이 채점 경로에서 "3m 안에서 찍혔다"를 받아야 나타난다(`CamCreepContentProvider.cs:13-16`) — 카메라가 그 몬스터의 감각 기관이다.

---

## 2. 채점 파이프라인

```
0.15초마다:  대상마다 ContentEventFrame(event, seenAmount)            ContentPolling.cs:76-101
             조합기 3개가 덤 이벤트를 붙인다 (2.4)                     ContentCombiningSystem.cs:7-22
             프레임 점수 = percentageToScreenToFactorCurve(seen) × GetContentValue()   ContentEventFrame.cs:40-43
             같은 (이벤트 ID, 개체 ID)가 연속으로 잡히면 하나의 "연속(Streak)"
연속이 끊기면: 연속 중 최고 프레임 점수 × streakCurve(연속 프레임 수) 하나만 남긴다   ContentBuffer.cs:34-52, :79-120
녹화를 낼 때: 모든 클립의 남은 장면을 점수순 정렬, 같은 (ID, 개체)는 상위 MaxSameEventType개만   ContentBuffer.cs:122-160
             총점 = 합                                               ContentBuffer.cs:222-230
업로드:      조회수 = round(총점 × scoreToViewConversionPerRun(주차))      BigNumbers.cs:203-212
             돈    = round(총점 × ScoreToMoneyPerWeek(주차))               BigNumbers.cs:237-247
             할당량 진행 += 총점 (점수 단위)                            UploadCompleteState.cs:47-50, RoomStatsHolder.cs:413-429
```

주차 `run = floor((날짜 − 1) / 3)` — 할당량 주기가 3일이다(`BigNumbers.cs:205`, `SurfaceNetworkHandler.cs:97`).
댓글은 남은 장면마다 하나씩 생기고 "좋아요"가 그 장면의 조회수다(`ContentBuffer.cs:167-186`).

### 2.1 화면 크기 곡선 `percentageToScreenToFactorCurve` (실값)

| 화면 비율 | 0 | 10% | **30%** | 60% | 100% |
|---|---|---|---|---|---|
| 배율 | 0 | 0.71 | **1.0** | 0.70 | 0.31 |

너무 작아도, **화면을 꽉 채워도 손해**다. 코앞의 몬스터는 30% 크기일 때의 3분의 1밖에 안 된다.

### 2.2 지속 곡선 `streakCurve` (실값, x = 연속 프레임 수, 프레임 = 0.15초)

| 연속 | 0 | 25 (3.75초) | 100 (15초) |
|---|---|---|---|
| 배율 | 0.25 | 0.81 | 0.99 |

스치듯 찍으면 4분의 1, 4초면 80%, 15초면 거의 전부다. 시야에서 한 프레임이라도 빠지면 연속이 끊긴다(`ContentBuffer.cs:99-119`).

### 2.3 기본값 — 무엇이 얼마인가 (`BigNumbers`, `resources.assets:22522`)

**몬스터** (`MonsterContentEvent`, 개체 ID = `viewID`):

| 값 | 종 | 코드와 다른 것 |
|---|---|---|
| **350** | Flicker | |
| **200 / 100** | BigSlap 분노 / 평온 (`BigSlapContentProvider.cs:8-16`) | |
| 150 | BarnacleBall | 코드 130 |
| 100 | Fire · Streamer | |
| 80 | Mime · Snatcho | Mime 코드 40 |
| 70 | Knifo · Bombs · Larva | 코드 55 |
| 60 | Ear · Spider · Toolkit_Wisk | Ear·Spider 코드 55, Toolkit 코드 40 |
| 55 | Wallo · SnailSpawner · Harpooner · ButtonRobot · Dog · CamCreep · EyeGuy | |
| 50 / 30 / 20 / 0 | Weeping 대기 / **포획 중** / 풀려남 / 실패 (`WeepingContentProvider.cs:22-36`) | |
| 50 | Jello | |
| 40 | Mouthe · Slurper | Mouthe 코드 35, Slurper 코드 55 |
| 35 | Puffo | |
| 30 | Worm | |
| 25 | Zombe · BlackHoleBot | |

Infiltrator2 프리팹에는 `ContentProvider`가 하나도 없다(프리팹 컴포넌트 목록). 코드대로라면 찍혀도 점수가 0이다 — 실측하지 않았다.
Wallo는 값만 있고 스폰 목록에 없다.

**플레이어** (`PlayerContentProvider.cs:17-74`, 개체 ID = `actorNumber`). 한 프레임에 아래 **위에서부터 처음 맞는 하나**만(버섯·감정표현·죽음·넘어짐·낙하는 그 자리에서 끝낸다):

| 이벤트 | 값 (실값) | 코드 | 조건 |
|---|---|---|---|
| 모자 | 모자별 `contentValue` | | 쓴 모자가 `giveContentEvent`면 (아래와 별도로 추가) |
| **피해 받음** | `1 × 피해량` | 같음 | 최근 3초 안 피해 중 최대. **개체 ID가 피해 한 번마다 다르다**(`unqiueHash`)라 맞을 때마다 따로 센다 (`PlayerTookDamageContentEvent.cs:28-31`, `UniqueDamage.cs`) |
| 좋은 받기 | 1 | 같음 | 3초 넘게 땅에 안 닿은 물건을 손에 쥔 지 1초 안 |
| 마이크 들기 | **0** | 0 | 리포터 마이크를 손에 (조합기 재료 — 2.4) |
| 감정표현 | 감정표현 아이템의 `emoteBaseScore` | | |
| **죽음** | **15** | 10 | 시체가 보이는 동안 |
| 넘어짐(래그돌) | **15** | 20 | `fallTime > 0` |
| 낙하 | 작은 **15** / 큰 **20** (1.5초 초과) | 10 / 10 | 0.75초 넘게 공중 |
| 그 밖 | `10 × 향함(0.5~1.0) + 마이크 × 3` | 향함 1~2 | 몸통이 카메라를 볼수록 1에 가깝다 |

**아티팩트·소품** (`PropContent`): 아티팩트 10·20·50·80·100, 일반 소품 1~125. 아티팩트는 `× (들고 있음 1 / 아님 0.5) × (작동 중 1 / 아님 0.5)`
(`ArtifactContentEvent.cs:16`, `resources.assets:22468-22496`).

### 2.4 조합기 — 연출을 시키는 덤

| 조합 | 조건 (한 채점 프레임 안) | 덤 이벤트 값 | 근거 |
|---|---|---|---|
| 여러 몬스터 | 몬스터 이벤트 2개 이상 | 몬스터 값 합 × **0.3** (개체 ID = 마릿수 — 2마리·3마리가 따로 센다) | `MonsterEventCombiner.cs:5-32`, `MultiMonsterEvent.cs:46-49` |
| 인터뷰 | 리포터 마이크를 든 동료가 몬스터 **10m** 안 | 그 몬스터 값 × **0.2** | `InterviewEventCombiner.cs:22-37` |
| 도발 | 감정표현 중인 동료가 몬스터 **15m** 안 | 그 몬스터 값 × **0.2** | `TauntEventCombiner.cs:61-74` |

### 2.5 무엇이 가중되나 — 질문 3에 대한 답

- **몬스터가 화면에 잡히는 것**이 점수의 본체다. 기본값 25~350이 화면 크기(최대 1)와 지속(최대 0.99)으로 깎인다.
- **동료의 죽음**은 15로 낮다 — 몬스터 한 마리의 3분의 1~20분의 1. 대신 **죽는 과정**이 비싸다: 맞는 장면이 피해량 그대로(BigSlap 펀치 51, 움켜쥐었다 놓기 110, Mime 105, Toolkit 돌진 95)
  이고 한 번 맞을 때마다 따로 세며, 같은 장면에 몬스터 값도 함께 들어간다.
- **비명**: 마이크 값 × 3이 "그 밖" 플레이어 이벤트에 붙는 것이 전부다(최대 +3). 대신 비명은 몬스터를 끌어들인다(소음 30m, 의심 ×2 — [monsters](contentwarning-monsters.md) 4절).
  화면에 잡힌 동료의 마이크 값이 0.4를 넘으면 "말하는 사람" 댓글 묶음이 섞인다(`PlayerContentEvent.cs:54-63`).
- **분노한 몬스터가 두 배**(BigSlap), **붙잡힌 동료가 있는 Weeping**(30)이 끝난 Weeping(20·0)보다 비싸다 — 위험한 순간 자체가 값이다.

계산 예 (1주차, 전환 10배·돈 1.5배):

| 장면 | 점수 | 조회수 | 돈 |
|---|---|---|---|
| 분노한 BigSlap을 화면 30% 크기로 15초 | 200 × 1.0 × 0.99 = 198 | 1,980 | 297 |
| 같은 장면을 코앞(100%)에서 15초 | 200 × 0.31 × 0.99 ≈ 61 | 610 | 92 |
| Zombe를 30%로 2초 스침 (약 13프레임) | 25 × 1.0 × 약 0.5 ≈ 13 | 약 130 | 약 19 |
| 동료가 Mime에게 한 방 맞는 장면 (30%, 짧게) | 105 × 1.0 × 약 0.25 ≈ 26 + Mime 80 장면 | — | — |

곡선 중간값(약 0.5)은 0~25 구간 선형 근사다. 1주차 할당량은 **300점**(3,000회)이다(2.6).

### 2.6 할당량·전환 곡선 (실값)

| 곡선 | x | 값 |
|---|---|---|
| `scoreRequiredPerWeek` (할당량, 점수) | 주차 0 / 1 / 2 / 3 / 5.5 / 20 | 300 / 350 / 425 / 490 / 595 / 850 |
| `scoreToViewConversionPerRun` (조회수 배율) | 주차 0 / 2.07 / 6 | 10 / 323 / 3000 |
| `ScoreToMoneyPerWeek` (돈 배율) | 주차 0 / 6 | 1.5 / 3.0 |

할당량은 점수로 비교하되, 화면에 보이는 조회수 문자열(예 "3.0K")이 같으면 통과로 친다(`RoomStatsHolder.cs:222-230`). 시작 돈 `StartMoney` 실값 0(코드 100).
녹화를 뽑아내는 데 실패하면 그날 할당량의 3분의 1과 100달러를 위로금으로 준다(`ExtractVideoMachine.cs:249-264`).

---

## 3. 도구 목록 (`Item`, `resources.assets:22394-22467`)

상점 판매(`purchasable`)만 가격을 적는다. 인벤토리는 **4칸**(`PlayerInventory.cs:14`)이고 손에는 선택한 한 칸만 든다(`PlayerItems.cs`).
`PlayerInventory.itemsToSpawnWith` 실값은 Modern Flashlight와 Camera다(`resources.assets:26897`) — 언제·누구에게 지급되는지는 **미확인**.

| 도구 | 가격 | 하는 일 | 수치 (실값) | 근거 |
|---|---|---|---|---|
| **Shock Stick** | 400 | 켜 둔 채 닿은 `Player`(몬스터·동료 구분 없음)에게 **전기 충격 5초** — 넘어짐 5초 + 몸 떨림. 한 번 쏘면 꺼진다 | 배터리 150, 충격당 −40, 켜 둔 동안 초당 −1. 대상당 쿨 1초 | `ShockStick.cs:64-135`, `ShockStickTrigger.cs:145-168`, `PlayerRagdoll.cs:515-530`, `resources.assets:26768` |
| **Rescue Hook** | 250 | 맞힌 `Player`를 2초간 내 쪽으로 끌어온다. `sinceRescueDragged = 0`으로 **Jello·Snatcho·Slurper·Harpooner·Larva에게서 떼어 낸다**. 사람을 못 맞히면 나를 그쪽으로 날린다(넘어짐 2초) | 3회, 사거리 30, 끄는 힘 100 | `RescueHook.cs:83-131`, `resources.assets:23787` |
| **Defibrilator** | 300 | 죽은 플레이어를 **체력 30으로 부활** | 3회 | `Defib.cs:144-151`, `Player.cs:1005-1023`, `resources.assets:25278` |
| **Hugger** | 100 | 껴안은 동료를 **1초마다 10** 회복(10초에 완치) | 간격 1초(코드 0.5) | `ItemHugger.cs:41-59`, `resources.assets:25599` |
| 손전등 5종 | 20 · 150 · 500 · 200 · 600 | 4절 | 배터리 100~200초 | 4절 |
| **Flare** | 40 | 바닥에 두는 광원, 밝기 1 | 수명 90초 | `Flare.cs:18`, `resources.assets:24133`, `:24980` |
| **Goo Ball** | 50 | 던지면 **5초**(코드 90) 뒤 터져 끈적이 영역 — 안에 든 강체의 속도를 물리 스텝마다 0.5배 | 끈적이 수명 코드 5초 | `ItemGooBall.cs:12`, `ExplodedGoop.cs:7-13,59-65`, `resources.assets:24404` |
| Reporter Mic | 50 | 인터뷰 조합기 재료 (2.4) | | `PlayerContentProvider.cs:44-47` |
| Boom Mic · Clapper · Sound Player · Party Popper · Sketch Pad · 감정표현 | 5~500 | 촬영 소품. 효과음에 적힌 소음 반경(30·50)은 실효 0이다 | | [monsters](contentwarning-monsters.md) 4.2 |
| 던진 물건 | — | Slurper에 맞으면 매달린 사람을 놓는다(`IThrowTarget`). 그 밖의 몬스터에게는 피해·기절 없음 | | `ItemInstance.cs:102-104`, `Bot_Slurper.cs:356-362` |
| Norf Gun | (비매품 250) | 30발, 탄 피해 0 — 맞은 뒤 효과 **미확인** | | `resources.assets:24487`, `:23427` |

### 3.1 몬스터에게 하는 일 — 요약

| 효과 | 도구 | 비고 |
|---|---|---|
| 피해 | **없음** | 몬스터 `TakeDamage`가 `ai`면 무시된다(`Player.cs:896`) |
| 기절 | Shock Stick 5초 | 넘어진 몬스터는 공격을 못 한다(`AbleToAttack`·각 공격의 `NoControl` 검사). Worm의 묶기를 끊는 유일한 수단(`Attacks_Worm.cs:68,83`) |
| 떼어내기 | Rescue Hook, 던진 물건(Slurper) | 납치·견인·매달림을 푼다 |
| 밀치기 | Rescue Hook으로 몬스터를 끌어오기 | 끄는 대상이 `Player`라 몬스터에도 걸린다 (몬스터를 끌어오는 것이 의도된 사용인지는 미확인) |
| 쫓아내기 | 손전등·조명탄 → Snatcho | 4절 |
| 주의 끌기 | **목소리**(30m)와 **발소리**만 | 미끼 도구 없음 — 물건 소리는 소음이 아니다 |
| 느리게 | Goo Ball | 몬스터도 강체라 걸린다 |

---

## 4. 손전등

손전등은 **아이템**이다. 몸(헬멧)에 붙은 광원이 몬스터에게 영향을 주는 경로는 없다 — 플레이어 프리팹 아래에 `FlashLightTrigger`가 없다
(`resources.assets`의 `FlashLightTrigger` 12개 중 플레이어 쪽은 Infiltrator 변장용 `FakeOldFlashlight`뿐).

| 프리팹 | 배터리(초) | 비고 |
|---|---|---|
| Flashlight 1 | 100 | `resources.assets:24075` |
| Flashlight 2 · Long 2 · Wide 2 | 150 | `:24073`, `:24997`, `:25805` (코드 기본 150) |
| Flashlight 3 · Long 3 · Wide 3 | 200 | `:24074`, `:27502`, `:27371` |

상점 품목은 Old(20) · Modern(150) · Modern Pro(500) · Long(200) · Long Pro(600)이다(`resources.assets:22445-22450`). 품목과 프리팹 이름의 대응은 **미확인**.

| 규칙 | 값 | 근거 |
|---|---|---|
| 켜기 | 손에 든 채 클릭으로 토글, 켜져 있는 동안 배터리 초당 1 소모 | `Flashlight.cs:59-79` |
| 몬스터 판정 | 빛줄기에 붙은 트리거 `FlashLightTrigger` — 닿은 몬스터의 `sinceFlashLit`을 0으로 | `FlashLightTrigger.cs:17-37` |
| 밝기 | 손전등 전부 **1**, 조명탄 1, **카메라 조명 0** | `resources.assets:23604-23606` 등, `:24980`, `:26920` |
| 먹히는 조건 | 몬스터 `lightLevelRequired` ≤ 밝기. Snatcho만 1, 나머지 0 | Bot 실값 ([monsters](contentwarning-monsters.md) 2.1) |
| 벽 | `flashLightRequireLineOfSight` 몬스터는 플레이어 빛이 벽 너머로 안 먹힌다 | `FlashLightTrigger.cs:25` |
| 플레이어 빛 / 맵 빛 | 트리거의 부모에 `Player`가 있으면 플레이어 빛(시작 시점 판정). EyeGuy는 플레이어 빛이면 분노, 맵 빛이면 "보여도 멈추지 않음" | `FlashLightTrigger.cs:9-15`, `Bot_EyeGuy.cs:69-76,126-138` |
| 들고 있는 것과의 관계 | 손에는 한 칸만 — **카메라를 든 사람은 손전등을 못 든다.** 그 대신 카메라 조명이 녹화 중 밝기 0 빛을 낸다(Snatcho에는 안 먹힘) | `PlayerInventory.cs:14`, 1절 |
| 내 화면 | 자기가 든 손전등은 빛줄기 셰이더 세기를 0.35배로 낮춘다 (눈부심 방지) | `Flashlight.cs:53-56` |

---

## CL9에 대응하는 자리

CL9 현황은 [../combat-gap.md](../combat-gap.md)·[../scare-mechanics.md](../scare-mechanics.md)에서 가져왔다. 결정은 적지 않는다.

| 자리 | Content Warning | CL9 지금 |
|---|---|---|
| 보상의 원천 | 화면에 잡힌 공포(몬스터·동료의 피해·죽음)를 점수로 | 훔친 재료를 요리해 판매 — 공포는 비용이지 보상이 아니다 |
| 같은 대상 반복 | 개체당 한 장면만 (`MaxSameEventType` 1) | — |
| 위험할수록 비싼 값 | 분노 BigSlap 200 vs 평온 100, 포획 중 Weeping 30 | 없음 |
| 근접 무기 | 없음 — 몬스터 체력 0, 전기 충격 5초가 유일한 무력화 | 프라이팬 (닭 HP 3~4 / 팬 2), 타격 경직 0.3초 |
| 기절 | Shock Stick 5초, 배터리 3회분 | 없음 (`HitStunMs` 0.3초) |
| 동료 구조 | Rescue Hook(떼어내기), Defib(부활 30), Hugger(회복) | 없음 |
| 미끼·주의 끌기 | 없음 (목소리·발소리만 소음) | 탄 요리 미끼 기획, `Decoy` 소음 행은 있으나 호출 경로 없음 |
| 빛 | 손전등 = 아이템(손 한 칸), 두 종이 빛에 반응 | 헤드램프·칠흑·정전 |
| 손에 드는 것의 경쟁 | 카메라 vs 손전등 — 한 손 | — |

---

## 근거와 한계

- 디컴파일 캐시 `C:\Users\Radar\AppData\Local\CL9\UnityAnalysis\contentwarning\decompiled\Assembly-CSharp`, 에셋 `resources.assets`(백엔드 AssetsTools).
  `VideoCamera`의 배열 참조는 UnityPy로 GameObject 이름을 풀어 확인했다.
- **미확인**: 카메라 업그레이드(`CameraUpgradeItem` 6개 — 필름·줌을 바꾸는지), 손전등 상점 품목과 프리팹 이름의 대응, Norf Gun·Boom Mic의 효과,
  Goo Ball 끈적이의 실값 수명, 시작 지급품이 누구에게 언제 가는지, 전기 충격봉이 든 사람 자신에게 닿는 경우, 계산 예의 곡선 중간값(선형 근사).
- 채점은 코드 식과 곡선 키값으로 계산했고 실제 업로드 결과와 대조하지 않았다.
