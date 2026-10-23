# 참고 — Content Warning의 몬스터: 감지·등장·공격·연출·소음

> 2026-10-17. 설치본(Steam buildid 22999514, Unity 6000.0.67f1, Mono, Photon PUN)의 `Assembly-CSharp.dll`을
> MCP 도구 `unity-analysis`로 디컴파일(ilspycmd, 1012 파일)해 동작을 읽고, `resources.assets`·`level1~5`의 MonoBehaviour
> 실값을 타입트리(백엔드 AssetsTools)로 읽었다. 몬스터 프리팹의 루트 이름은 각 컴포넌트의 `m_GameObject` → Transform 부모 사슬을
> UnityPy로 따라가 붙였다. 씬 번호는 BuildSettings 기준 `level1`=Surface, `level2`=Factory, `level3`=Harbour, `level4`=Mines다.
> **코드 초기값 ≠ 에셋 실값**이라 둘이 다르면 둘 다 적었다. **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 이 문서는 인계 문서 §5의 질문 1·2·4(감지·등장·공격 종류·연출 조건·소음)를 다룬다. 카메라·점수는
> [contentwarning-items.md](contentwarning-items.md), 한 판 루프는 [contentwarning-run.md](contentwarning-run.md).

## 결론

- **몬스터는 전부 "AI가 조종하는 플레이어"다.** 모든 몬스터 프리팹에 플레이어와 같은 `Player`·`PlayerRagdoll`·`PlayerController`가 붙어 있고
  (`ai = 1`, 예: `resources.assets:26237` Zombe), 그 위에 공통 뇌 `Bot` 하나와 종별 행동 컴포넌트(`Bot_Zombie`·`Bot_Chaser`·`Bot_Weeping` …),
  공격 컴포넌트(`Attack_*`)를 얹는다. 그래서 몬스터도 넘어지고(`Fall`) 전기 충격(`TaseShock`)을 받으며, 래그돌 물리로 공격한다.
- **HP 피해 몬스터와 통제권 상실 몬스터가 거의 반반이다.** 31종 중 HP를 직접 깎는 공격이 있는 종이 다수지만, 그 대부분이 동시에
  넘어뜨리기(`Fall` 1~2.5초)를 걸고, 피해 0 또는 미미한 종(Jello·Worm·Slurper·Weeping 포획·Flicker 빙의·Harpooner 견인·Snatcho 납치)은
  **끌고 가기·매달기·묶기·입력 차단·시선 강제·짐 강제 낙하**로 압박한다. 즉사형(CamCreep·BlackHole·Weeping 실패·Flicker 빙의 5 초과)도 따로 있다.
- **감지는 시야 하나 + 의심 게이지다.** 70m·110°(전방 220° 원뿔) 안에서 몸 부위 하나가 보이면 약 1초 만에 표적으로 잡고, 이어서
  `suspicionValue`가 1을 넘으면 공격 모드(`aggro`)가 된다. 거리·웅크림·달리기·**마이크 입력**이 그 속도를 바꾼다.
- **소음은 플레이어 발소리와 목소리뿐이다.** 효과음에 적힌 `noiseDistance`(물건 튕김 15, 사운드 플레이어 30, 파티 폭죽 50)는 호출 경로가
  배수 0을 넘겨서 **실제로는 소음이 되지 않는다.** 발소리는 걷기 9.9m·달리기 15m·웅크림 5m, 목소리는 마이크 0.9 이상일 때 30m다.
- **점프스케어는 로컬 판정, 위험 음악은 호스트 판정이다.** 종마다 `jumpScareLevel`(0~3)이 있고, 각 클라이언트가 자기 카메라로 그 등급의
  가장 가까운 몬스터가 15m 안에서 시야 70°에 들어오면(10초 쿨) 스팅어 + 흔들림 15를 튼다. 위험 음악은 몬스터가 경계(`Alert`)할 때
  호스트 쪽 핸들러가 10초간 켜고 RPC로 전원에게 동기화한다.
- **"사라짐"은 없다 — 대신 지치고 도망가고 재배치된다.** 추격형은 20~40초 쫓으면 지쳐서 5~20초 두 배 속도로 달아나고, 판 디렉터
  `Puppetmonster`가 120초 뒤부터 **100초마다** 플레이어에게서 가장 먼 몬스터 하나를 플레이어 근처(20m 밖)의 안 보이는 지점으로 순간이동시킨다.

---

## 1. 공통 뼈대 — `Bot`

`Bot.cs` 하나가 감지·의심·추격·순찰·소음 수신을 전부 갖고, 종별 컴포넌트는 매 프레임 이 함수들을 골라 부른다.
행동 판단은 **소유자(`view.IsMine`)에서만** 돈다(`Bot.cs:173-182`). 몬스터는 마스터 클라이언트가 `PhotonNetwork.Instantiate`로
만들므로(`RoundSpawner.cs:274-277`) 판단은 호스트 몫이다.

### 1.1 상태의 흐름

대부분의 종은 같은 세 갈래를 쓴다(`Bot_Zombie.cs:22-58`, `Bot_Chaser.cs:101-117`).

| 상태 | 조건 | 하는 일 | 근거 |
|---|---|---|---|
| 기본(순찰) | 표적 없음 | `Patrol()` + `LookForTarget()` — 순찰점 그래프를 따라 걷는다. 최근 1초 안에 소리를 들었으면 그쪽을 보고 멈춘다 | `Bot.cs:379-431`, `:661-664` |
| 조사 | 표적 있음, `aggro` 거짓 | `InvestigateCurrentTarget()` — 서서 표적을 바라보며 의심 게이지를 채운다 | `Bot.cs:768-819` |
| 전투 | `aggro` 참 | `ChaseTarget()` + `ValidateChase()` — 달려서 쫓고, 시야가 끊긴 시간이 문턱을 넘으면 표적을 놓는다 | `Bot.cs:508-552`, `:821-838` |

### 1.2 시야 — 표적 잡기

- `LookForTarget(head, maxRange=70, maxAngle=110, timeToSeeTarget=1)`: 매 프레임 살아 있는 플레이어 **한 명씩 돌아가며**
  (`GetNextPlayerAlive`) 그 사람의 **무작위 몸 부위 하나**로 선을 긋는다(`Bot.cs:621-647`). 걸리면 그 부위를 붙잡고 `seeBodypartValue`를
  초당 2씩 올리는데, 매 프레임 초당 1씩 깎이므로(`Bot.cs:255`) 순증 초당 1 — **약 1초 계속 보이면 표적**이 된다(`Bot.cs:666-691`).
- 각도는 몬스터 시선 방향과의 사잇각 110° 이하 — 전방 220° 원뿔이다. 거리 70m는 `CanSeeTarget`에서 제곱 4900으로 박혀 있다(`Bot.cs:606`).
- 추격 중에는 `LookForBetterTarget`이 더 가까운 사람으로 표적을 바꾼다. 비교 값은 `거리 / Visibility()`이고, `Visibility()`는
  **마이크 0.9 초과 또는 달리기면 2, 웅크림이면 0.5, 그 밖 1**이다(`Bot.cs:703-725`, `Player.cs:1338-1350`). 떠드는 사람이 두 배 가깝게 취급된다.
- 종별 실값(`Bot_Chaser`, `resources.assets`)은 대부분 70m·110°·1초이고, ButtonRobot·Dog·Harpooner는 **40m·160°**다(2.5절 표).

### 1.3 의심 게이지 — 조사에서 공격으로

`InvestigateCurrentTarget`(`Bot.cs:768-819`)에서 표적이 보이면 매 프레임 다음을 더한다.

```
suspicion += dt × t × aggroSpeed × num
  t   = Lerp(1, 0.2, InverseLerp(5m, 30m, 거리))   // 5m 안 1, 30m 이상 0.2
  num = 0.8 × (웅크림 0.5 | 달리기 1.5) × (마이크 > 0.5 이면 2)
```

`suspicion > 1`이면 `aggro = true`. 안 보이면 초당 1씩 깎이고 −1 아래로 가면 표적을 버린다.

| 상황 (5m 안, aggroSpeed 1) | 공격 모드까지 |
|---|---|
| 서서 | 1.25초 |
| 웅크림 | 2.5초 |
| 달리기 | 0.83초 |
| 서서 말하기(마이크 > 0.5) | 0.63초 |
| 30m 밖, 서서 | 6.25초 |

### 1.4 추격 유지와 포기

- `ValidateChase(head, loseTargetTime)`: 몬스터 머리에서 표적 중심으로 선이 막히면 `sinceLastSawTarget`가 쌓이고, 문턱을 넘으면
  표적·`aggro`·의심을 모두 버린다. 보이면 거꾸로 깎인다(`Bot.cs:821-838`). 코드 기본 4.5초, `Bot_Chaser` 실값 2~6초.
- **지침(`exhaustion`)**: 표적이 있는 동안 초당 1씩 쌓여 `exhastionTime`을 넘으면 그 자리에서 표적을 놓고 `fleeForSeconds` 동안
  **이동 속도 2배**로 반대 방향 순찰점을 따라 달아난다. 도망 3초 뒤부터는 반경 5m·120°만 다시 살핀다(`Bot_Chaser.cs:70-99`).
  같은 구조가 `Bot_Knifo`(20초/20초, `Bot_Knifo.cs:51-81`), `Bot_EyeGuy`, `Bot_ToolkitBoy`(20초/15초)에 복사돼 있다.
- **숨은 표적**: 내비메시 위 플레이어 위치가 1m 넘게 떨어지면 `targetIsHiding`, 경로 끝이 닿지 않으면 `targetUnReachable`
  (`Bot_Nav_Navmesh.cs:96-127`). 이때 추격형은 12m 안이면 멈춰서 바라보고, 그 밖이면 마지막으로 닿을 수 있던 지점(`lastGodNavPos`)으로
  달린다(`Bot_Chaser.cs:135-161`). 지침은 `hidingExhastionMultiplier`배(Spider 5배)로 쌓인다. Knifo·BigSlap은 이때 **도약 공격**을 쓴다(2절).

### 1.5 공격의 판정 주체

공격 시작(`AbleToAttack`: 소유자, 표적 있음, `aggro`, 사거리 안, 쿨 경과, 표적이 넘어져 있지 않음 — `Bot.cs:881-912`)은 호스트가 정하지만,
**맞았는지는 맞은 쪽 클라이언트가 정한다.** 근접 공격은 몬스터 래그돌 부위의 충돌 이벤트에서 `componentInParent.refs.view.IsMine`(피해자 본인)일 때만
`RPCA_Hit`을 보낸다(`Attack_Melee.cs:202-213`, `Attack_Stab.cs:52`, `Attack_Punch.cs:52`). 투사체도 `IsLocal`인 플레이어만 피해를 받는다
(`Projectile.cs:68-77`). 피해는 `TakeDamage`가 모든 클라이언트에서 체력을 깎고, 죽음 RPC는 본인만 보낸다(`Player.cs:894-911`, `:943-949`).

---

## 2. 종별 표 — 감지·등장·공격 종류

공통: 표적 잡기 1.2절, 의심 1.3절, 추격 포기·도망 1.4절. 아래 "감지"는 그것과 **다른 점**만 적는다.
"피해"는 HP 피해, "통제"는 통제권 상실(넘어짐 `Fall`·끌림·매달림·입력 차단·시선 강제·짐 낙하·묶임)이다. 플레이어 최대 체력은 100(`Player.cs:303-305`).
예산·희귀도는 `BudgetCost` 실값(2.4절), 행동 수치는 종별 컴포넌트 실값이다.

| 종 (프리팹) | 감지·행동 규칙 | 공격 | 분류 | 근거 |
|---|---|---|---|---|
| **Zombe** | 기본형. 추격 중 표적 주위로 무작위 옆 오프셋(거리×0.7)을 두고 접근 | 1.5m 안 `attacking`이면 몸통을 끌어당기며 **0.75초마다 4** | 피해(지속)·끌림 | `Bot_Zombie.cs:41-47`, `Bot_Drag.cs:29-65` |
| **Mouthe** | **2초 동안 60° 안에서 쳐다보면 10초 도망**(쳐다보는 동안은 공격 안 함, `lookingAtMe < 0.5`). 소음 경계 끔(`alertable=false`) | 3m 안 `attacking` 플래그. 피해를 주는 코드 경로는 이번에 못 찾았다 — **미확인** | 미확인 | `Bot_Mouth.cs:23,42-96` |
| **Jello** | 기본형, 추격 속도 0.5배 | 몸에 닿으면 **포획 → 10초 들고 달아남**(`jelloTime` 화면 효과, 회전 강제), 놓은 뒤 15초 도망. 피해 없음 | 통제(납치·시야) | `Bot_Jelly.cs:39-110`, `JellyTrigger.cs:15-43`, `RenderFeatureHandler.cs:47` |
| **Knifo** | 숨으면 12m 안에서 2초 충전 뒤 **도약 공격** | 찌르기: 4m 안, 쿨 2.5초, **35 + 넘어짐 1초 + 밀침 5** | 피해+통제 | `Bot_Knifo.cs:96-162`, `Attack_Stab.cs:41,79-83`, `resources.assets:23964` |
| **UltraKnifo** | Knifo와 같음 (UltraKnifoGroup으로 Knifo 3마리와 함께 나옴) | 사거리 **6.5m**, 힘 1.5배 | 피해+통제 | `resources.assets:23965`, `:24791` |
| **BigSlap / BigSlap_Small** | `Bot_Fear` 보유(2.3절 공포 루프). 숨으면 15m 안에서 5초 충전 뒤 도약 | 펀치: 7m 안, 쿨 0.1초, **51 + 넘어짐 2초 + 밀침 20**. 숨은 표적 4m 안이면 **움켜쥐기 → 들고 다니며 초당 11**, 다음 움켜쥐기 때 놓으면서 **110**(즉사) | 피해+납치 | `Bot_BigSlap.cs:57-117`, `Attack_Punch.cs:41,79-84`, `Attack_Grab.cs:32-145` |
| **Toolkit_Wisk** | 표적을 보고 2초 모은 뒤 **3초 돌진**. 돌진 중 머리가 벽에 부딪히면 자기가 2초 넘어짐(흔들림 5) | 돌진 접촉: **95**(코드 6) + 넘어짐 2.5초 + 밀침 10, 1인당 쿨 1초 | 피해+통제 | `Bot_ToolkitBoy.cs:36-157`, `Trigger_Wisp.cs:22-31`, `resources.assets:23491` |
| **Ear** | **목소리에 약하다** — 3~15m 안 플레이어의 마이크 값(거리 감쇠) 0.7 초과가 이어지면 `hurtAmount`가 초당 0.8씩 차서 1이 되면 **30초 도망** | 밀치기: 8m 안, 쿨 2.5초, **20 + 넘어짐 1초** | 피해+통제 | `Bot_Ear.cs:63-76,104-119`, `Attack_Shove.cs:32,59-63`, `PlayerHandler.cs:272-288` |
| **EyeGuy** | **플레이어 손전등에 0.1초 안에 비춰지면 `eyeAggro` 초당 2 상승**, 1이 되면 즉시 공격 모드. 평소에는 누군가 보고 있고 맵 조명에 안 비춰져 있으면 **멈춘다**. 30m·전방위·즉시 표적 | 근접 2.5m, **35 + 넘어짐 1초 + 밀침 5**, 쿨 1초 | 피해+통제 | `Bot_EyeGuy.cs:69-133`, `resources.assets:26177` |
| **CamCreep** | **평소 안 보인다**(`rend.enabled=false`). **카메라로 3m 안에서 찍혀야** 나타나 공격(정지 0.3초 이상 또는 공격 중). 끝나면 20m 밖 무작위 지점으로 순간이동 | 공격 애니 4.3초, 1초 뒤 1m 안이면 끌어당기고 펀치 프레임에 **즉사**(`CallDie`) | 즉사 | `Bot_CameraCreep.cs:79-181,231-236` |
| **Snatcho** | `lightLevelRequired 1` — **빛(밝기 1 이상)에 0.3초 쬐이면 20m 밖으로 순간이동**하고 놓는다 | 2m 안이면 **납치**: 머리에 붙여 끌고 다니며 초당 6, **들고 있던 물건 강제 낙하**(`dropItemsFor`), 20초 뒤 놓음 | 납치·짐 낙하 | `Bot_Snactcho.cs:83-131,155-222`, `PlayerItems.cs:68-71` |
| **Flicker** | 3~15초(20% 확률 15~40초) 간격으로 **60m 안 조명을 깜빡이며** 차원을 오간다. 완전히 넘어오면 표적을 응시 | 35m 안에서 자기를 보는(120°) 사람에게 **이동 입력 차단**(`inputOverideAmount`)·**시선을 몸 쪽으로 강제**·`possession` 누적(초당 최대 3). 나갈 때 `possession > 5`면 **사망** | 즉사·통제 | `Bot_Skinny.cs:122-210,344-399` |
| **Weeping** | **누가 보고 있으면(70°) 멈춘다.** 10m 안이면 무조건 표적. 경로 거리 60m(코드 50) 넘게 멀어지고 안 보이면 포기. 솔로 판에서는 스폰 목록에서 빠진다 | 0.5m(코드 3) 안이면 **포획**: 캡차 미니게임, **90초**(코드 20) 안에 시작 안 하거나 실패하면 **사망**, 성공하면 놓아줌 | 통제→즉사 | `Bot_Weeping.cs:82-151,199-258,352-449`, `resources.assets:24596`, `RoundSpawner.cs:35-39` |
| **Infiltrator2** | **동료로 변장한다.** 다른 사람과 가장 멀리 떨어진 사람(최근접 동료 거리 **60m** 초과, 코드 100)을 흉내 내고 그 사람에게서 가장 먼 사람을 노린다. 아무도 안 보는 동안 2초(코드 10) 가까이 있으면 분노 | 목조르기: **15초에 100**(0.5초마다 3.3), 시선 강제·이동 차단. 누가 보면 멈추고 다시 흉내 | 피해+통제 | `InfiltratorSpawner.cs:32-83`, `Bot_Infiltrator.cs:237-416`, `resources.assets:24243` |
| **Slurper** | 천장에 붙은 **보이지 않는 끈끈이**(숨김 머티리얼). 스폰 때 50% 확률로 미끼 아이템을 아래에 둔다 | 닿으면 **거꾸로 매달아** 끌어올리고 초당 2. 물건을 던져 맞히거나 22.5m 넘게 당겨지면 놓는다. 매달린 동안 던지기 충전 0 | 통제(매달림) | `Bot_Slurper.cs:101-162,330-362`, `resources.assets:23498` |
| **BarnacleBall** | 10m 안·시야 안이면 **흡입**(쿨 30초) | 촉수: **51 + 넘어짐 2초**. 흡입: 끌어당김 45, 감쇠 0.9. 숨으면 가스 — 가스 구름 하나당 **7** | 피해+통제 | `Bot_BarnacleBall.cs:51-260`, `SuckTrigger.cs:27-50`, `resources.assets:23463`, `:24302` |
| **Harpooner** | 40m·160° 감지, 거리 15m 유지 | 작살: 15m 안·시야 안이면 발사, **1** + 최대 13초 끌어당김(힘 23, 이동 0.5배), 1.5m까지 오면 2초간 이동 0.1배. 근접: **49 + 밀침 30** | 통제(견인)+피해 | `Attack_Harpoon.cs:44-137`, `resources.assets:24429`, `:26382` |
| **Spider** | 숨은 표적에게 지침 5배 | 근접 **35**, 쿨 1.5초. 15m 안에서 거미줄(`WebShot`) 발사, 쿨 5초 — 줄에 붙들려 당겨짐 | 피해+통제 | `resources.assets:25804`, `Attack_Spider.cs:30`, `Web.cs:111-130` |
| **Larva** | 기본형 | 3m 안 움켜쥐어 5초간 들고 **던진다** — 던져진 몸이 부딪히면 **49**, 넘어짐 1초 | 통제+피해 | `Attack_Throw.cs:48-124`, `resources.assets:26390` |
| **Dog** | 40m·160° 감지, 거리 15m 유지. 회전이 느리다(0.5) | 40m 안에서 보이면 조준 게이지(초당 1, 최대 3) — **빛줄기가 67°→10°로 좁아지는 것이 예고**. 3이 되면 0.1초마다 탄 발사, 탄당 **26** + 밀침 25 | 피해 | `Attack_Dog.cs:48-95`, `resources.assets:24145`, `:26842` |
| **Mime** | `flashLightRequireLineOfSight`. 몸이 안 보이고 **1.1초마다 잔상만 남는다** | 근접 **105**(한 방 즉사) + 밀침 15 | 즉사급 피해 | `BlinkRenderer.cs:11-49`, `resources.assets:23797`, `:26629` |
| **Arms** | 10m·시야 30° 안이면 팔을 뻗음 | 붙잡아 끌어당기며 0.75초마다 **5** | 피해+끌림 | `Attacks_Arms.cs:86-143`, `resources.assets:27412` |
| **Bombs** | 항상 로컬 카메라 쪽을 본다(`TurnToLocalPlayer`) | 15m 안에서 폭탄 아이템을 던지고, 4m 안이면 도화선 4초 뒤 **자폭**(Explosion 150, 반경 8, 넘어짐 2) | 피해 | `Attacks_Bombs.cs:39-110`, `resources.assets:26037`, `:25190` |
| **BlackHoleBot** | 공격 모드에서 5m 안이면 **10초 카운트다운**(표정 텍스트), 9m 밖으로 3초 벗어나면 리셋 | 블랙홀 생성 — 끌어당기고 안쪽 반경 0.9배 안이면 **즉사**(몬스터는 비활성화) | 즉사 | `Attacks_BlackHole.cs:69-118`, `BlackHole.cs:19-71` |
| **Puffo** | 기본형 | 2m 안, 쿨 4초 — 0.5초 뒤 충격파(ShockWave **25**, 반경 5, 넘어짐 1.5) | 피해+통제 | `resources.assets:23664`, `:24842`, `:26158` |
| **Streamer** | 기본형 | 4m 안, 쿨 2초 — 전기봉(**27 + 전기 충격 5초**, 1인당 쿨 2초) | 피해+통제 | `Attacks_Streamer.cs:40-86`, `EffectTrigger.cs:20-44`, `resources.assets:26501` |
| **Worm** | 지치지 않음(`exhastionTime` 10000) | 몸에 들러붙은 뒤 **가장 가까운 다른 플레이어에게 뛰어 둘을 묶는다**(최대 200초). 묶인 동안 둘 다 계속 넘어짐. 전기 충격을 받아야 떨어진다. 피해 없음 | 통제(묶임) | `Attacks_Worm.cs:60-133,189-219` |
| **Fire** | 지침 40초 | 몸 판정 0.3초마다 **5**, 불덩이 발사 | 피해(지속) | `resources.assets:26831`, `:24568` |
| **SnailSpawner** | 기본형 | 달팽이 탄 발사(탄 자체 피해 0). 맞은 뒤 효과는 **미확인** | 미확인 | `resources.assets:26200`, `:26844` |
| **ButtonRobot** | 40m·160° | 누르면(`GenericInteractible`) 두 이벤트 중 하나를 50:50으로 고른다. 붙어 있는 스폰 대상은 Explosion(150) — 어느 쪽 이벤트가 폭발인지는 **미확인** | 미확인 | `resources.assets:26456`, `:24365`, `RadomChanceEvent.cs:57-76` |

코드에는 `Bot_Ghost`·`Bot_Wallo`·`Bot_Angler`·`Bot_AnglerMimic`·`Bot_MimicInfiltrator`도 있지만 `resources.assets`의 `BudgetCost` 32개와
씬 스폰 목록 어디에도 없다 — 이 설치본에서 스폰되는지 **미확인**.

### 2.1 손전등이 바꾸는 행동

`FlashLightTrigger`가 닿은 몬스터의 `sinceFlashLit`을 0으로 돌린다(`FlashLightTrigger.cs:17-37`). 몬스터의 `lightLevelRequired` 이하 밝기만 먹힌다.
Snatcho만 1이고 나머지는 0이다(Bot 실값 31개). 그래서 **카메라 조명(밝기 0)은 Snatcho를 쫓지 못하고, 손전등·조명탄(밝기 1)은 쫓는다**
(밝기 실값은 [contentwarning-items.md](contentwarning-items.md) 4절).

| 종 | 빛에 비춰지면 | 근거 |
|---|---|---|
| Snatcho | 0.3초 쬐면 순간이동 | `Bot_Snactcho.cs:83-93` |
| EyeGuy | 플레이어 빛이면 분노 게이지 상승 → 공격. 맵 조명이면 "보여도 멈추지 않음" | `Bot_EyeGuy.cs:69-76,126-138` |
| Mime·BlackHoleBot·Arms·SnailSpawner·Fire·EyeGuy | `flashLightRequireLineOfSight = 1` — 플레이어 빛은 벽 너머로 안 먹힌다 | Bot 실값 `resources.assets:25271` 등 |

### 2.2 보는 것이 무기인 종

| 종 | 보고 있으면 | 판정 |
|---|---|---|
| Weeping | 멈춘다 (표적이 정해진 뒤에는 벽 너머 시선도 센다 — `checkLineOfSight = bot.targetPlayer == null`) | 호스트가 `Player.CanSee`로 계산 (`Bot_Weeping.cs:417-429`) |
| Mouthe | 2초 보면 도망 | 호스트 (`Bot_Mouth.cs:44-56`) |
| EyeGuy | 멈춘다 | 호스트 (`Bot_EyeGuy.cs:126-129`) |
| Infiltrator | 분노가 식고 흉내로 돌아간다 | 호스트 (`Bot_Infiltrator.cs:418-424`) |
| Flicker | 보면 **빙의된다**(반대) | 각 클라이언트가 모든 플레이어에 대해 계산 (`Bot_Skinny.cs:331-399`) |
| CamCreep | **카메라로** 찍어야 나타난다 | 카메라 든 클라이언트의 콘텐츠 판정이 RPC로 공격을 연다 (`Bot_CameraCreep.cs:79-86,231-236`) |

`Player.CanSee`의 시야각 기본값은 70°(전방 140°)이고 선 검사를 한다(`Player.cs:1155-1167`). 시선 방향 `data.lookDirection`은 복제되는 값이라
호스트가 각 플레이어의 "보고 있음"을 직접 계산할 수 있다 — 리포처럼 클라가 화면 판정을 올려 보내는 역방향 경로가 필요 없다.

### 2.3 등장 — 스폰 예산과 두 번의 물결

**한 판에 두 번 나온다.** `RoundSpawner`(마스터만, `RoundSpawner.cs:29-70`)가 날짜별 예산을 둘로 나눈다.

| 단계 | 언제 | 어디 | 근거 |
|---|---|---|---|
| 1차 | 레벨 준비 직후 | 종의 순찰 그룹 순찰점 중 가중 추첨 — **시야 검사 없음** | `RoundSpawner.cs:182-207` |
| 2차 | 몬스터마다 `[0, 대기시간]` 무작위. 대기시간 = `minMaxSecoundRoundSpawnTime`(**0.3~0.7**, 코드 0.2~0.8) × 산소 최대치 500초 = **150~350초** | **살아 있는 누구도 못 보는** 순찰점(100회 시도, 실패하면 아무 데나) | `RoundSpawner.cs:72-88,209-263`, `BigNumbers.cs:214-217` |
| 조건부 | Infiltrator — 판 시작 5초 뒤부터 고립자(최근접 동료 60m 초과)가 생기면 그 사람에게서 10m 이상 떨어진 안 보이는 순찰점 | `InfiltratorSpawner.cs:45-83` |

**예산** (`BigNumbers`, `resources.assets:22522`):

| 값 | 코드 기본 | 에셋 실값 |
|---|---|---|
| `monsterBudgetPerDay` (곡선, x=날짜) | 없음 | 1일 20 · 2일 20 · 4일 23 · 15일 51 |
| 할당량 주기 안의 날짜별 추가 (`day1/2/3ExtraMoBudget`) | 100 / 200 / 300 | **0 / 1 / 4** |
| 1차 비율 `firstSpawnAmount` | 0.7~1 | **0.25~0.75** (나머지가 2차) |
| 한 마리 최대 비용 `biggestMonsterPurchasePerDay` | 없음 | 1일 5 · 3일 12 · 51일 51 |

뽑기 가중치는 `비용 / 남은 예산 × 희귀도`다(`RoundSpawner.cs:291-320`) — 예산이 많이 남아 있을수록 비싼 놈이 상대적으로 덜 뽑히고,
예산이 줄수록 남은 예산에 꽉 차는 놈이 뽑힌다. 순찰점의 거리 가중(`GetPointDistanceWeigth`, 스폰 종 50~100m에서 0.5→1)은 루프 변수 대신
몬스터 인덱스 `i`로 점을 집어서(`:191,197,243,249`) **모든 점이 같은 가중을 받는다** — 사실상 무작위 순찰점이다.

종별 `BudgetCost` 실값(`resources.assets`):

| 비용 | 종 (희귀도) |
|---|---|
| 1 | Zombe(2.0) · Slurper(2.25) · Mouthe(0.4) · Worm(0.5) · Flicker(0.1) · CamCreep(0.05) · Infiltrator2(0.05) |
| 2 | Puffo(1.0) · ButtonRobot(1.0) · BlackHoleBot(0.05) |
| 3 | Snatcho(0.3) |
| 4 | Toolkit_Wisk(1.0) · Spider(0.5) · Ear(0.05) |
| 5 | Weeping(0.3) · Jello(0.3) |
| 6 | Knifo · Arms · Fire · EyeGuy · Dog · Bombs · Larva (전부 1.0) |
| 8 | Harpooner(0.75) · Mime(0.3) |
| 10 | BarnacleBall(0.5) · UltraKnifo(0.1) · SnailSpawner(0.1) |
| 12 | Toolkit_WiskGroup 3(0.15) |
| 15 | BigSlap(0.3) · BigSlap_Small(0.3) · Streamer(0.5) |

무리(`MonsterGroupClose`)는 구성원 비용의 합으로 산다 — ZombieGroup3·5, KnifoGroup 2·5, SnatchoGroup 5, SlurperGroup 5, Mouthe5, UltraKnifoGroup,
WeepingGroup2(희귀도 0.05~0.25, `resources.assets:24791-24800`). 스폰 목록은 Factory·Mines 38개, Harbour 33개(Weeping·BarnacleBall·Fire·UltraKnifo 없음,
ButtonRobot 있음 — `level2:28584`, `level3:16858`, `level4:15051`). 솔로면 목록 끝 두 칸(Weeping, WeepingGroup2)을 잘라낸다.

**예고는 없다.** 2차 물결은 안 보이는 곳에서 나오고 소리 예고 장치가 없다. 다만 Dog(빛줄기 좁아짐)·BlackHoleBot(카운트다운 텍스트)·
Toolkit_Wisk(2초 정지 후 돌진)·Bombs(도화선)처럼 **공격 직전 예고**는 종마다 있다.

### 2.4 사라짐 — 디스폰 대신 지침·도망·재배치

| 장치 | 동작 | 근거 |
|---|---|---|
| 지침 → 도망 | 2.5절 표의 `exhastionTime` 뒤 `fleeForSeconds` 동안 2배 속도로 달아남 | `Bot_Chaser.cs:70-99` |
| **`Puppetmonster`** (판 디렉터) | `initialWait` **120초**(코드 30) 뒤 `interventionRate` **100초**(코드 30)마다, 아무도 못 보는 몬스터 중 **모든 플레이어와 가장 먼** 하나(Slurper·Infiltrator 제외)를 무작위 생존자 근처 순찰점(20m 이상, 높이 차 5m 이내, 아무도 못 봄)으로 순간이동 | `Puppetmonster.cs:12-34`, `BotHandler.cs:25-58`, `Level.cs:167-182`, `level2:31779` |
| 순간이동 이탈 | Snatcho(빛)·CamCreep(공격 뒤) — 20m 밖 무작위 순찰점 | `Level.cs:131-145` |
| 실제 소멸 | Weeping 판정 끝 20초 뒤 아무도 안 볼 때, Infiltrator 이탈 15초 뒤 아무도 안 볼 때, Bombs 자폭, 블랙홀에 빨려 든 몬스터 | `Bot_Weeping.cs:91-98`, `Bot_Infiltrator.cs:267-281`, `Attacks_Bombs.cs:44-51`, `BlackHole.cs:57-60` |

몬스터 체력은 없다 — `TakeDamage`가 `ai`면 바로 돌아간다(`Player.cs:896`). 몬스터를 "죽이는" 수단은 블랙홀뿐이고, 도구는 넘어뜨리기·떼어내기만 한다
([contentwarning-items.md](contentwarning-items.md) 5절).

### 2.5 `Bot_Chaser` 실값 (`resources.assets`)

| 종 | 지침 | 도망 | 시야 놓침 | 유지 거리 | 감지 거리·각 |
|---|---|---|---|---|---|
| Arms | 20 | 5 | 2 | 6 | 70·110 |
| BlackHoleBot | 20 | 5 | 2 | 4 | 70·110 |
| Bombs · Larva · SnailSpawner · Streamer | 20 | 20 | 2 | 0~3 | 70·110 |
| ButtonRobot | 15 | 10 | 3 | 4 | **40·160** |
| Dog | 30 | 10 | 3 | 15 | **40·160** |
| Fire | 40 | 5 | 6 | 6 | 70·110 |
| Harpooner | 30 | 10 | 3 | 15 | **40·160** |
| Mime | 20 | 10 | 2 | 2 | 70·110 |
| Puffo | 40 | 5 | 6 | 1.5 | 70·110 |
| Spider | 25 | 20 | 2.5 | 2 | 70·110 |
| Worm | 10000 | 0 | 2 | 2 | 70·110 |

코드 기본값은 20 / 20 / 2.5 / 0.8 / 70·110이다(`Bot_Chaser.cs:9-43`).

---

## 3. 연출 — 점프스케어·위험 음악·공포 루프

### 3.1 무엇이 어디서 판정되나

| 장치 | 트리거 | 재생 | 길이 | 판정 | 근거 |
|---|---|---|---|---|---|
| **점프스케어 스팅어** `JumpScareSound` | 등급(1·2·3)마다 하나씩 씬에 있다. 그 등급의 **가장 가까운 몬스터**가 카메라 **15m**(제곱 225) 안, 시야 **70°**(CamCreep처럼 `canJumpScareFromBehind`면 500° = 전방위) 안, 선 검사 통과, **10초 넘게 안 보였다가** 처음 보일 때 | 그 몬스터 위치에서 등급별 효과음 + 화면 흔들림 15 (기본 0.2초) | 효과음 1회 | **로컬** — `MainCamera` 기준, 각 클라이언트 | `JumpScareSound.cs:17-60`, `level2:29539` 등 (등급 1·2·3 각 1개) |
| **위험 음악** `SpookyMusicHandler` | 몬스터의 `Alert()` — 소음을 듣거나, 조사 중 표적이 보일 때 — 가 그 몬스터 등급의 핸들러에 `sinceDanger = 0` | 등급별 루프 음악, 볼륨 0 → `onVolume` **0.125**(코드 0.2)를 초당 0.05로 (약 2.5초) | 마지막 경계 뒤 **10초** | **호스트**(핸들러 PhotonView 소유자)가 켜고 끄고 `RPCA_ToggleDanger`로 전원 동기화 | `SpookyMusicHandler.cs:24-76`, `UniversalBotSounds.cs:18-21`, `Bot.cs:358-377` |
| **공포 루프** `FearHandler` | `fearList`에 든 몬스터 중 로컬 플레이어를 표적으로 `aggro`, 2초 안에 봤고, 시선 80° 안, 30m 안 — `Bot_Fear`가 붙은 **BigSlap·BigSlap_Small만** `fearList`를 채운다 | 처음 잡힐 때 원샷(쿨 5초), 루프 볼륨 = `afraid` × 1.0(코드 0.25), 매 프레임 흔들림 0.05×afraid | `afraid` 초당 +1 / −0.3 | **로컬** | `FearHandler.cs:24-115`, `Bot_Fear.cs:14-28`, `level2:31539` |
| **피격 화면** `TakeDamagePost` | 본인이 피해를 받음 | 글리치 볼륨 가중치 1 | 0.1초 (감쇠 루프에 `yield`가 없어 한 프레임에 0으로 떨어진다) | 로컬 | `TakeDamagePost.cs:21-44`, `Player.cs:901-905` |
| 종별 화면 효과 | Flicker 표적 = 포스트 볼륨 0.7·사망 볼륨 최대 0.28, Jello 납치 = 젤리 렌더 효과, 블랙홀 근접 = 흔들림 반경 비례(0.5초마다 최대 5) | — | 상태 지속 동안 | 로컬 | `Bot_Skinny.cs:248-281`, `RenderFeatureHandler.cs:47`, `BlackHole.cs:24-34` |

### 3.2 종별 `jumpScareLevel` (Bot 실값, `resources.assets`)

| 등급 | 종 |
|---|---|
| 3 | BigSlap · BigSlap_Small · BarnacleBall · Flicker · CamCreep(**찍히는 동안만** 3, 평소 0 — `Bot_CameraCreep.cs:84,93`) |
| 2 | Ear · Snatcho · Weeping · Knifo · UltraKnifo · Toolkit_Wisk · Harpooner · Spider · Dog · Bombs · Larva |
| 1 | Zombe · Mouthe · Jello · Puffo · Worm |
| 0 (스팅어·위험 음악 없음) | Infiltrator2 · Slurper · Streamer · Mime · BlackHoleBot · ButtonRobot · Arms · SnailSpawner · Fire · EyeGuy |

코드 기본값은 2다(`Bot.cs:22`). 등급이 스팅어 **세기**가 아니라 **어느 스팅어·어느 음악**인지를 고른다 — 음악은 등급별 세 트랙이 따로 켜진다.
등급 0인 10종은 조용히 온다. 변장(Infiltrator)·안 보임(Slurper·Mime)처럼 **들키지 않는 것이 공격인 종**이 0에 몰려 있다.

### 3.3 카메라·입력의 강제

| 강제 | 누가 | 근거 |
|---|---|---|
| 시선을 몬스터 쪽으로 | Flicker(초당 25×강도), Infiltrator 목조르기(초당 25) | `Bot_Skinny.cs:391`, `Bot_Infiltrator.cs:407` |
| 이동 입력 차단 | Flicker(`inputOverideAmount` = 시선 각도 30~90°로 0~1), 목조르기(`strangledForSeconds`), 들려 다님(`carried`), 넘어짐(`fallTime`) | `Bot_Skinny.cs:386-387`, `Player.cs:100-110`, `PlayerController.cs:185,322` |
| 물리 카메라 (래그돌 시점) | `Fall` 동안 래그돌. `NoControl()`이 참 | `PlayerRagdoll.cs:495-501`, `Player.cs:1223-1234` |
| 몸 회전 강제 | Jello (머리를 아래로) | `JellyTrigger.cs:21-22` |
| 짐 강제 낙하 | Snatcho 납치 동안 매 프레임 선택 칸을 떨어뜨림 | `Bot_Snactcho.cs:175`, `PlayerItems.cs:68-71` |

---

## 4. 소음 모델

### 4.1 소리가 몬스터에게 닿는 경로

전부 `SFX_Player.playNoiseAction` 한 줄이다. 각 `Bot`이 시작할 때 구독한다(`Bot.cs:156`).

```
OnNoisePlayed(pos, range, alerts):            // Bot.cs:649-659
  d = pos - 몬스터 중심
  |d.x| ≤ range  ∧  |d.y| ≤ range × 0.3  ∧  |d.z| ≤ range  ∧  |d| ≤ range  이면
    Alert(pos, alerts); lastNoisePos = pos; 1초 동안 "최근 소음"
```

- **벽·문·거리 감쇠 없음.** 반경 안이면 들리고 밖이면 안 들린다. 대신 **위아래는 반경의 0.3배**만 — 다른 층 소리는 거의 안 들린다.
- **누적**: `Alert`는 0.25초에 한 번만 받고, 받을 때마다 `nrOfAlerts += alerts`. **10을 넘으면** 그 소리 위치를 `searchPoint`로 잡고 그리로 걸어간다
  (1m 안에 닿거나 15초가 지나면 끝). 5초 동안 새 경계가 없으면 카운트가 0으로 돌아간다(`Bot.cs:272-275,342-377`).
  공격 모드(`aggro`)이거나 `alertable = false`(Mouthe)면 무시한다.
- 경계 한 번마다 위험 음악이 10초 켜진다(3.1절). 즉 **발소리 한 번이 음악을 켠다.**
- 순찰 중 최근 1초 안에 소리를 들었으면 그쪽을 보고 멈춘다(`Bot.cs:387-392`). 소리 자체로 표적이 되지는 않는다 — 돌아본 뒤 시야(1.2절)로 잡는다.

### 4.2 무엇이 소음을 내나

`PlaySFX`는 `noiseDistance × stepNoiseMultiplier`를 반경으로 넘기는데 배수의 기본값이 **0**이다(`SFX_Player.cs:63,75-78`). 배수를 넘기는 호출자는
`FootstepHandler`뿐이라(`FootstepHandler.cs:74,78`), **효과음 자산에 적힌 `noiseDistance`는 발소리 외에는 반경 0이 되어 아무도 못 듣는다.**

| 소음원 | 반경 | 경계 수 | 근거 |
|---|---|---|---|
| 발소리 — 걷기 | 15 × 0.66 = **9.9m** | 1 | 발소리 SFX 25종 `noiseDistance` 15(`resources.assets:22573-22598`), 배수 실값 1 / 0.66 / 0.33(`resources.assets:23901`) |
| 발소리 — 달리기 | **15m** | **2** | `FootstepHandler.cs:67-70` |
| 발소리 — 웅크림 | **4.95m** | 1 | `FootstepHandler.cs:63-66` |
| 점프·착지 | 발소리와 같은 규칙 (`JumpStep`·`LandStep`·`LandBody` 핸들러가 따로 있다) | 1~2 | `resources.assets:27304`, `:24483`, `:23948` |
| **목소리** | 마이크 값 **0.9 초과**면 0.1초마다 **30m** | 1 | `Player.cs:841-856` |
| 라디오 아티팩트 | 30m | 1 | `ArtifactRadio.cs:90`, `PersistantRadio.cs:83` |
| 물건 튕김·사운드 플레이어(30)·파티 폭죽(50)·래그돌 넘어짐(15) | 자산에는 있으나 **실효 0** | — | `resources.assets:22544-22570`, `:22765`, `:22903-22923` |

- **마이크**: 데시벨 −100~−20을 0~1로 바꾼 값(`Player.cs:1182-1185`)이 `PlayerSyncer`로 바이트 하나에 실려 복제된다(`PlayerSyncer.cs:74,102`).
  그래서 호스트가 남의 목소리 크기를 안다. 마이크는 소음(30m) 말고도 **의심 속도 ×2(0.5 초과)**·**표적 선호 ×2(0.9 초과)**·**Ear 퇴치(0.7 초과, 3~15m)** 에 쓰인다.
- **호스트 카메라 의존**: 공간 효과음(`spatialBlend > 0.75`)은 재생하는 클라이언트의 카메라가 그 효과음의 `range` 밖이면 재생 자체를 건너뛰고,
  소음 통지도 그 안에 있다(`SFX_Player.cs:73`). 발소리 `range`는 30m라 **호스트 카메라에서 30m 넘게 떨어진 플레이어의 발소리는 호스트의 몬스터가 못 듣는다**
  (판정이 호스트에서만 돌기 때문, 1절). 목소리 소음(`PlayNoise`)은 이 검사를 거치지 않는다(`SFX_Player.cs:93-96`).

---

## CL9에 대응하는 자리

CL9 현황은 [../combat-gap.md](../combat-gap.md)·[../scare-mechanics.md](../scare-mechanics.md)에서 가져왔다. 결정은 적지 않는다.

| 장치 | Content Warning | CL9 지금 |
|---|---|---|
| 몬스터 몸 | 플레이어와 같은 래그돌 — 넘어지고 전기 충격 받음, 체력 없음 | 별도 몬스터, 체력 있음(닭 3~4) |
| 감지 | 70m·220° 원뿔, 약 1초에 표적, 의심 게이지(5m 1.25초) | 닭 `SightIn` 330cm, `DetectNearRadius` 120cm 게이지 2.5/초 |
| 웅크림·달리기·목소리가 감지에 | 의심 ×0.5 / ×1.5 / ×2, 표적 선호 ×2 | 자세 없음 |
| 소음 | 발소리만(9.9 / 15 / 5m), 벽 무시·층 0.3배, 경계 10회 누적 시 수색 | 걷기 소음 0(거위 `WakeRadius` 450 예외), 타격 소음 없음, `KillAlarmRadius` 2100 |
| 등장 | 1차 일괄 + 2차 150~350초에 걸쳐 안 보이는 곳 한 마리씩 | 판 시작 일괄 + 7e 시야 밖 한 마리씩 |
| 재배치 | `Puppetmonster` 100초마다 가장 먼 몬스터를 플레이어 20m 밖 안 보이는 곳으로 | 없음 ("디스폰 없음" 2026-10-04) |
| 포기 | 지침 20~40초 → 2배 속도 도망 5~20초 | `ChaseBudget` 45초 → 집으로 |
| 공격 종류 | 넘어뜨리기·납치·매달기·묶기·빙의·캡차·짐 낙하·즉사, HP 피해는 대부분 넘어짐과 함께 | 잡힘 = HP 1 + 넘어짐(무적, 짐 유지) |
| 점프스케어 | 등급별 스팅어(15m·70°·10초 쿨), 로컬 | 없음 (`MonsterPounce` 소리·`monster_prox` 파동) |
| 위험 음악 | 경계 뒤 10초, 등급별 트랙, 호스트 판정·동기화 | 없음 |
| 보면 반응 | Weeping 멈춤·Mouthe 도망·EyeGuy 멈춤·Flicker 빙의 — 시선이 복제돼 호스트가 직접 판정 | 없음 (`WakeSightRadius` 900은 기상 보류만) |
| 빛 | 손전등이 Snatcho 퇴치·EyeGuy 자극 | 칠흑·정전·헤드램프 |
| 시선·입력 강제 | Flicker·목조르기 지속 | `HurtLookModifier` 잡힌 순간만 |

---

## 근거와 한계

- 디컴파일 캐시 `C:\Users\Radar\AppData\Local\CL9\UnityAnalysis\contentwarning\decompiled\Assembly-CSharp`. 에셋 값은 `Tools/UnityAnalysis`의
  `find_assets`·`read_asset`(백엔드 AssetsTools)로 읽었고, 배열이 20개를 넘는 값(스폰 목록)은 같은 함수를 배열 상한만 늘려 다시 읽었다.
  프리팹 루트 이름은 UnityPy로 MonoBehaviour 머리의 `m_GameObject`를 읽어 Transform 부모를 따라갔다.
- **미확인**: Mouthe의 피해 경로, SnailSpawner 탄의 효과, ButtonRobot 두 이벤트의 내용, Spider 거미줄의 정확한 지속, `Bot_Ghost` 등 5종의 사용 여부,
  점프스케어 효과음 세 개의 내용(씬 외부 참조), 위험 음악이 공간 음원인지 전역인지, 종별 이동 속도(`PlayerController` 실값을 읽지 않았다).
- 같은 프리팹에 `BudgetCost`와 `MonsterGroupClose`가 둘 다 붙은 경우(Toolkit_WiskGroup 3) `GetComponent<IBudgetCost>()`가 어느 쪽을 집는지는 컴포넌트 순서에 달려 있어 확인하지 않았다.
- 플레이·영상 실측은 하지 않았다. 감지 시간(1.3절)은 코드 식을 손으로 푼 값이다.
