# 참고 — MIMESIS의 몬스터: 종별 규칙·변장(미메시스) 시스템·감지

> 2026-10-17. 설치본(Steam buildid 25381161, `BuildConstants.BuildVersion` "0.3.2", Unity 6000.3.9f1, Mono, FishNet 네트워크, Dissonance 음성, A* Pathfinding, Unity ML-Agents + Sentis)의
> `Assembly-CSharp.dll`을 MCP 도구 `unity-analysis`로 디컴파일(ilspycmd, 2264 파일)해 동작을 읽었다. 밸런스 수치는 코드가 아니라 게임이 실제로 읽는 데이터 파일에서 가져왔다 —
> 마스터 데이터는 `MIMESIS_Data\StreamingAssets\masterdata\*.json`(`ExcelDataManager.cs:205`가 `masterdata/<이름>.json`을 읽는다), 몬스터 행동은 `StreamingAssets\aidata\param\*.xml`(상태 목록)과
> `aidata\bt\*.xml`(상태별 행동 트리, `AIDataManager.cs:164·216`)이다. 프리팹에 직렬화된 값(음성 보관함·신뢰도 판정기·탐지기)은 `find_assets`·`read_asset`(백엔드 AssetsTools)으로 읽었고,
> **코드 초기값 ≠ 에셋 실값**인 곳은 둘 다 적었다. **에셋을 가져오지 않는다** — 구조와 수치만 참고한다.
> 이 문서는 인계 질문 1·2·3(종별 규칙·변장 시스템·감지)을 다룬다. 도구와 공포 표현은 [C:\Work\CL9\docs\reference\mimesis\mimesis-items.md](C:\Work\CL9\docs\reference\mimesis\mimesis-items.md),
> 한 판 루프는 [C:\Work\CL9\docs\reference\mimesis\mimesis-run.md](C:\Work\CL9\docs\reference\mimesis\mimesis-run.md).
> 거리 단위는 행동 트리 파라미터의 m, 시간은 ms(표)·초(행동 트리)다. 이동 속도 350/700은 플레이어 표와 같은 값이고 코드상 환산은 미확인이다.

## 결론

1. **몬스터는 세 부류다 — 미메시스 1계열(변장형), 일반 몬스터 14종, 함정 7종.** `MonsterData.json`의 40행 중 실제 던전 스폰 표(`SpawnableMonsterGroup.json`)에 오르는 것은 미메시스 2변종(alpha·beta)과 일반 14종(+ 특수 스폰 BabyRilla 1종)이고 나머지는 디버그·QA·물류 모드용이다. 일반 몬스터도 "규칙형"이 많다 — 보면 멈추는 Sentryper, 움직이면 죽이는 MadameCroak, 소리에 도망가는 BruteBunny, 소리에 달려드는 Maegoostro, 등에 붙어 터지는 PowMoth, 스크랩을 먹는 TadaTeddy.
2. **변장은 외형이 아니라 "소지품·행동·목소리"의 복제다.** 미메시스는 전용 모델(`prefab_MimicPuppet`)로 태어나고 특정 플레이어의 겉모습을 베끼지 않는다. 대신 (a) 가장 먼 플레이어의 가방을 30초마다 가짜 아이템으로 복제해 들고 다니고(손전등도 켠다), (b) 플레이어들이 마이크로 말한 **실제 음성 조각**(0.7~7초, 각자 최대 512개 보관)을 상황이 비슷한 것으로 골라 재생하며, (c) 이모트·점프를 따라 하고 문을 열고 닫고 전화·송신기·경적을 쓴다. 호스트의 ML 에이전트(`DLDecisionAgent`·`DLMovementAgent`)가 이동과 "언제 공격할지"를 정한다.
3. **플레이어의 검증 수단은 네 가지다 — 쳐다보기·말 걸기·치기·탐지기.** (a) 미메시스는 자기를 45° 안에서 쳐다보는 1초마다 신뢰도 −3, 안 보면 +3을 쌓고 10 이하면 불신(공격 또는 도주), 90 이상이면 친구 행세를 이어 간다. 즉 **노려보면 정체를 드러내고, 등을 돌리면 공격당한다**(15 m 안에서 등을 돌린 판정이 4회(≈2초) 연속이면 즉시 공격). (b) 말을 걸면 20 m 안의 미메시스가 0.2초 뒤 저장된 음성으로 "대답"하는데, **자기 목소리를 흉내 내는 미메시스의 소리는 본인에게는 들리지 않는다**(`muteLocalPlayerVoice`). (c) 치면 피격 어그로 25 이상에서 미메시스가 반격 또는 전투로 바뀐다. (d) "시끄러운 닭"이 7 m 안의 미메시스에 3.5초마다 운다.
4. **들켰을 때의 결과는 목 조르기다 — HP가 아니라 오염도.** 목 조르기 3.8초 동안 오염도가 250 ms마다 +324(최대 10,000의 약 절반)씩 오르고, 끝나면 손의 아이템을 떨어뜨리고 넉백된다. 오염도가 가득 차면 죽고, **시체 자리에 1.5초 뒤 새 미메시스가 태어난다**. alpha 변종 넷 중 하나(25%)는 가짜 폭탄을 꺼내 자폭한다(5 m 즉사).
5. **감지는 거리 하나다.** 몬스터의 "본다"는 같은 공간 구획에 들어온 액터를 전부 어그로 사전에 넣고(`VActor.cs:248`), 행동 트리의 `CheckPlayerCountInRadius`가 반지름·같은 층(높이차 7)·실내외 일치만 본다 — 시선 각도도 벽 가림도 없다. 시선 판정은 세 노드(`AnyActorWatchingInRadius`·`IsTargetVisible`·`CollectMovingPlayerInRange`)에만 있고, 이들은 **플레이어가 몬스터를 보는지**를 판정한다. 소리 어그로(마이크 음량·손에 든 물건·이동 거리)는 `Sound` 어그로를 가진 BruteBunny·Maegoostro 두 종만 받는다. 손전등은 어떤 몬스터도 보지 않는다(미메시스의 ML 관측값에만 들어간다).

---

## 1. 공통 뼈대

### 1.1 상태와 행동 트리

몬스터마다 `aidata\param\npc_<이름>.xml`에 상태 이름→행동 트리 이름이 있고(예: `npc_brutebunny`: passive·search·beware·combat·runway), 각 트리는 `aidata\bt\npc_<이름>_<상태>.xml`이다. 노드는 Selector/Sequence/Condition/Action이고 데코레이터로 `Once`·`Cooltime(ms)`·`Random(만분율)`·`Succeeder`·`Repeater`가 붙는다. 호스트에서만 돈다.

거의 모든 종의 공통 골격은 다음과 같다(`npc_brutebunny_passive.xml` 등).

| 단계 | 조건 | 뜻 |
|---|---|---|
| passive | 배회 — 3~7 m 랜덤 점으로 걷기 | 발견 전 |
| passive → search | `SearchType` 2(종마다 30~80% 확률로 태어날 때 정함)이고 87~360초 뒤 75 m 안에 플레이어가 있으면 | **시간이 지나면 플레이어 쪽으로 걸어온다** — 재배치 대신 접근 |
| passive → beware/combat | `CheckPlayerCountInRadius` 15~20 m(종마다) 안에 플레이어 + 내비 거리 20~30 m 이하 | 발견 |
| beware | 걷다가 거리·시간·어그로 조건으로 combat | 종별 규칙이 여기 있다 |
| combat | 스킬 사거리 안에서 `UseSkill` | |
| runway/runaway | 15~20초 또는 30 m 떨어지면 passive | 포기 |
| 문 | 20 m 안에 플레이어가 있고 1.5 m 안에 닫힌 문이 있으면 0.4~0.8초 뒤 연다 | 몬스터는 문을 연다 |

### 1.2 어그로 — 피격·시야·소리 세 종류 (`AggroObject.cs`)

`MonsterData.json`의 `MonsterData_aggroval`이 종별로 어떤 어그로를 받는지 정한다(`type`: 1 Sight, 2 Hit, 3 Sound — `Bifrost.ConstEnum/AggroType.cs`). `weight`는 ×0.0001(`Bifrost.Cooked/AggroInfo.cs:18`).

| 종류 | 식 (`AggroObject.cs`) | 누가 받나 |
|---|---|---|
| Hit | 피해량 × weight (`:193`) | 거의 전부 — weight 10000(×1.0), range 20 |
| Sight | 1000 ms마다 100 × (1 − d/range) × weight (`:121`) | 디버그 MamaGorilla(99999997)뿐 |
| Sound | 1000 ms마다 [마이크 피크×100](`VPlayer.cs:409`) × (1 − d/range) × weight (`:137`) + 손에 든 물건의 `sound_aggro_in_hand_per_tick`(`:143`) + 아이템 사용 소음(`AIController.cs:567`) + 이동 거리(`:597`, `increase_score_per_distance`가 0이면 꺼짐) | **BruteBunny**(weight 1,000,000 = ×100, range 15), **Maegoostro**(Hit ×30 + Sound ×100, range 20) |

어그로는 3초(`NO_AGGRO_DURATION`) 동안 새 입력이 없으면 500 ms마다 70%씩 깎인다(`DECREASE_AGGRO_RATE_PER_TICK` 7000, `Const.json`). 표적 선정 문턱 `targeting_aggro_score` 1000. 행동 트리의 `CheckMaxAggroValue(GE, N)`·`CheckAggroValue(Hit, GE, N)`가 이 값을 읽는다.

### 1.3 "본다"의 실체

- 어그로 사전 등록은 공간 구획(섹터) 진입 시 양방향으로 자동이다(`VActor.cs:220-250` `SightInSector` → `AIController.OnSightIn`). `SightRange` 기본 1,000,000(`AIController.cs:94`)이고 `SetDetectionRange` 노드를 쓰는 트리는 없다.
- `CanTarget`(`AggroObject.cs:62`)은 살아 있고, 잡혀 있지 않고, **실내외 플래그가 같고**, 높이차 ≤ `NAV_Y_THRESHOLD` 7, 거리 ≤ SightRange만 본다. 각도·레이캐스트 없음.
- `CheckPlayerCountInRadius(r, 비교, n, checkHeight, onlyTargetable)`(`MMBehaviorTree/CheckPlayerCountInRadius.cs`) → `AIController.GetAggroPlayerCount`(`AIController.cs:966`) — 반지름 안 인원 수. 몬스터 발견은 전부 이 노드다.
- 플레이어가 몬스터를 보는지: `AnyActorWatchingInRadius(r, 각도)`(`MMBehaviorTree/AnyActorWatchingInRadius.cs:30` — 캡슐 가시성 `VWorldUtil.CheckVisible` + 플레이어 yaw 기준 `CheckTargetLookingAtMe` 반각), `IsTargetVisible`, `CollectMovingPlayerInRange(r, 각도)`(몬스터 시야각 안 + 가시).
- 미메시스의 "쳐다보는가"는 ML 에이전트가 따로 계산한다(4절).

---

## 2. 종별 표 — 네 칸·두 열

근거: `MonsterData.json`(HP·속도·어그로·위협값), `MonsterSkillData.json`·`MonsterSkillSequenceData.json`·`GrabData.json`·`AbnormalData.json`(피해·상태이상), `aidata\bt\npc_*.xml`(규칙). 플레이어 HP 100(`PlayerData.json`), 오염도 최대 10,000(`Const.json` MAX_CONTA_VALUE). 피해 식은 `VWorldCombatUtil.cs:28` — 공격 10·방어 5 차이는 ×1.0005라 무시한다. "방망이 N대"는 곰방망이 40(`CharacterSkillData.json` 10800)·샷건 120(10900) 기준.

| 종 (표 이름) | 유형 | 텔 | 규칙 | 결과 | 대응 | 숙련 단계 (추정) | 죽일 수 있나 (HP / 방망이·샷건) |
|---|---|---|---|---|---|---|---|
| **미메시스** mimic_alpha_base / beta_base | 변장형 + 시선형 | 플레이어처럼 걷고 말하고 문을 여닫는다. 탐지기(시끄러운 닭) 7 m 경보. 다른 몬스터가 이 개체를 쫓는 것(1.5절 세력 6) | **"노려보면 들킨다, 등 돌리면 당한다."** 쳐다보는 1초당 −3, 안 보면 +3; ≤10 불신, ≥90 친구(4.3절). 15 m 안에서 등을 2초 돌리면 공격 | 목 조르기 3.8초: 오염도 +324/250 ms(≈ +4,860), 끝나면 손 아이템 낙하 + 넉백. 오염 초과 시 사망 → 시체에 새 미메시스. alpha 25%는 가짜 폭탄 자폭(5 m 즉사) | 주의 — 신뢰도를 중간(30~70)에 두고 거리 4 m 이상. 인원 — 한 명이 보고 한 명이 때린다. 자원 — 샷건 1발 / 방망이 3대, 탐지기, 페인트볼 표식 | 모를 때: 동료인 줄 알고 등을 보이다 목이 졸린다. 알 때: 말을 걸어 대답을 듣고 탐지기를 믿는다. 익숙할 때: 보자마자 치거나 노려봐서 도주시킨다(beta는 불신 시 도망) | **100 / 3대·1발**. 조르는 동안 기절·암전 면역(`GrabData` 30001 `caster_ongrab_immune`) |
| **mimic_endgame** (데스매치) | 단순 추격형 | 없음 | — | 가짜 쇠지렛대(2900)로 넉다운 90 + 넉백 | 방망이 3대 | — | 100 |
| **BruteBunny** 토끼 | 소음형 (역) | 15 m 안 플레이어에 반응 | **"소리를 지르면 도망간다."** 어그로 ≥1500이면 runway — 말만 해도 Sound ×100이라 7.5 m에서 초당 5,000 | 공격 80 + 경직(HP 100의 80%) | 주의·소리 — 소리를 내면 쫓아낸다(14초 뒤 돌아옴) | 모를 때: 조용히 도망치다 맞는다. 알 때: 소리 지른다 | 180 / 5대·2발 |
| **MamaGorilla** | 통제권 상실형 | 20 m 안에서 `FaceCheck` — 플레이어가 쳐다보면(20 m·100°) 1.5초 멈췄다 물러나기 ×2, 세 번째엔 공격 | **"쳐다보면 두 번은 물러난다."** 4 m 안이면 바로 전투 | 그랩(최대 60초, 끌고 다님, 25초 뒤 또는 8초 뒤 복귀 중 놓음) + 내려찍기 40 | 인원 — 동료가 친다(어그로 ≥10이면 놓고 도망). 공간 — 12 m 밖 | 모를 때: 끌려간다. 알 때: 동료가 한 대 치면 놓는다 | 120 / 3대·1발 |
| **StandingOwl** | 영역형 | 서 있는 올빼미, 20 m 안이면 `standing` | **"3.5 m 안에 가지 마라."** | 회전 공격 100 = 즉사 | 공간 — 25 m 밖이면 돌아간다 | 모를 때: 지나가다 죽는다. 알 때: 돌아간다 | 60 / 2대·1발 — 맞으면(≥10) 1~2 m 비켜설 뿐 |
| **QuokkaGlider** | 통제권 상실형 + 협동형 | 20 m 안, 15초 뒤 beware; 어그로 ≥15 또는 30초면 combat | **"얼굴에 붙으면 동료가 떼라."** | 역그랩: **침묵**(마이크 차단) + 오염도 +25/250 ms, 20초 뒤 스스로 떨어짐 | 인원 — 동료 1대(HP 40). 붙은 사람은 말을 못 한다 | 모를 때: 혼자서 20초 | 40 / 1대 |
| **Maegoostro** 거위 | 소음형 | 20 m 안 beware(걷기만) | **"20 m 안에서 말하지 마라."** Sound ×100 — 어그로 ≥300이면 combat(어그로 ≤50이면 다시 beware) | 25 ×2(8연타 중 4) + 경직 | 침묵 — 3초 조용하면 어그로가 빠진다 | 모를 때: 말하다 맞는다. 알 때: 무전 끄고 지나간다 | 120 / 3대·1발 |
| **HeavyD** 코끼리 | 영역형 (느림) | 걷기 250(달리기 없음), 15 m에서 20초 뒤 beware | **"닿지 마라."** 상시 오라 90005: 2 m 캡슐에 감전(아이템 낙하) → 넉백 + 40 | 아이템 낙하 + 40 | 공간 — 0.7 m 안이면 되레 물러난다(runway) | — | 360 / 9대·3발 |
| **Bombesis** | 타이머형 + 변장형 | 20 m 안 beware; 가끔 랜덤 플레이어 가방을 복제해 든다(80%) | **"가까이 오면 터진다."** 3 m 또는 어그로 ≥10 → combat, combat 3.5초 또는 2 m 안 → 자폭 | 5 m 캡슐 즉사(`FieldSkill` 70008, HP −9,999,999) | 공간 — 30 m 밖이면 혼자 터진다. 치면 앞당겨진다 | 모를 때: 치러 다가가 죽는다 | 60 — 치면 터진다 |
| **Ramblam** 카우보이 양 | 단순 추격형 (원거리) | 15 m 안 combat | 샷건 2발 뒤 장전 | 70 ×2 + 경직 | 시간 — 장전 틈, 3 m 안이면 뒤로 빠진다 | — | 120 / 3대·1발 |
| **Sentryper** 사신 미어캣 | 시선형 | 20 m 안 7초 뒤 combat | **"보고 있으면 안 움직인다."** 30 m·100° 안에서 누가 보면 `Stop`. **치면(어그로 ≥5) 시선을 무시하고 때린 사람을 쫓는다** | 1.5 m 즉사(100) | 주의 — 교대로 본다. 치지 마라 | 모를 때: 눈을 떼면 죽는다. 알 때: 번갈아 본다. 익숙할 때: 치지 않는다 | 180 / 5대·2발 — 그러나 치면 counter |
| **PowMoth** 펑나방 | 통제권 상실형 + 협동형(역) | 25 m 안 또는 누가 30 m·100°로 보면 beware; 텔레포트 점으로 날아온다 | **"붙었으면 동료에게서 떨어져라."** 실내에서 다른 플레이어가 10 m 안에 15초 있으면 터진다; 실외는 5초 뒤 | 25 + 오염 DoT(+500/583 ms, 3초) + 아이템 낙하 | 인원·공간 — 혼자 멀리 간다(역협동) | 모를 때: 동료에게 달려가 같이 터진다 | 3 / 1대 |
| **MadameCroak** | 시선형 (역) + 소음 텔 | 10 m 안 → **노래**(BGM) 4초 → 주시 | **"주시 1.5초 동안 움직이지 마라."** 10 m·120° 안에서 위치가 바뀌거나 15° 넘게 돌아보면 | 영혼흡수 → 결박 1초 → 사망(−1,000) | 시간 — 멈춘다. **치면 3초마다 반격 70** | 모를 때: 노래가 들려 도망치다 죽는다. 알 때: 멈춘다 | 180 / 5대·2발 — 치면 반격 |
| **TadaTeddy** | 영역형 (짐) + 단순 추격 | 17 m 안 beware; 보채기 이모트 | **"스크랩을 먹인다 — 치지 마라."** 3 m 안 전리품을 먹으면(3회까지) 만족해 물러난다. 치면(≥10) 광폭화 60초 | 광폭 공격 40 + 아이템 낙하 | 자원 — 스크랩 하나 버린다 | — | 180 / 5대·2발 |
| **Gazellerina** | 단순 추격형 (돌진) | 17 m 안 combat | **"직선에 서지 마라."** 20 m 가시·시야 0.6이면 30 m 직선 구르기, 6초 쿨 | 구르기 오라: 아이템 낙하 → 넉백 + 25 | 공간 — 옆으로 | — | 60 / 2대·1발 |
| **Cimim** 키밈 | 단순 추격형 + 변장 텔 | **플레이어 음성을 거꾸로 재생**(`PlaySound(Voice)` → `TrySpawnNonMimicVoice`, `isReverse: true`, `VoiceManager.cs:1622`) | 8초 안에 6 m 또는 15초 뒤 combat; 60초 뒤 도주 | 밀치기 25 | 시간 | — | 100 / 3대·1발 |
| **BabyRilla** 원숭이 (특수 스폰 20%) | 영역형 (보상) | 약올리기 이모트, 6 m 안이면 도망 | 때리면 똥을 떨어뜨린다 — 따끈한 똥 93%(5~15) / 황금똥 7%(100~200) (`MonsterData_abnormal_eventactionval` SPAWN_ITEM) | 없음 | 자원 — 쫓아가 친다 | — | 200 / 5대 — 20초 뒤 사라짐 |
| **BearTrap** | 함정 | 바닥 | 1.8 m | 25 + 결박 4초 | — | — | 1000 — 발동 2.5초 뒤 자멸 |
| **PendulumTrap** ×6 | 함정 | 30 m 안이면 휘두르기 시작 | 진자 아래 | 40 + 넉다운 + 아이템 낙하 | — | — | 1000 |

### 2.1 유형 분류

| 유형 | 종 |
|---|---|
| 변장형 | 미메시스 alpha·beta·endgame, (Bombesis — 가방만 복제, Cimim — 목소리만) |
| 시선형 | Sentryper(보면 멈춤), MadameCroak(보는 동안 움직이면 죽음), MamaGorilla(보면 물러남 ×2), 미메시스(노려보면 들킴·등 돌리면 공격) |
| 소음형 | Maegoostro(소리에 달려듦), BruteBunny(소리에 도망) |
| 타이머형 | Bombesis(3.5초), PowMoth(15초) |
| 통제권 상실형 | MamaGorilla(그랩), QuokkaGlider(얼굴·침묵), PowMoth(등), 미메시스(목 조르기), HeavyD·Gazellerina·TadaTeddy·PendulumTrap(아이템 낙하) |
| 협동형 | QuokkaGlider(동료가 떼야), PowMoth(동료에게서 떨어져야 — 역) |
| 영역형 | StandingOwl, HeavyD, TadaTeddy(스크랩), BabyRilla |
| 단순 추격형 | Ramblam, Gazellerina, Cimim |

### 2.2 몬스터끼리 — 세력 표

`Faction.json`: 플레이어 1(적 2·7), 몬스터 [2,5], 미메시스 [2], 함정 [7]. 2군 세력 5는 6을 적으로 보고 6은 5·6을 아군으로 본다. 미메시스는 `checkfriendly`·`bewarefriendly`·`bewaredistrust`에 들어갈 때 `AddFaction(6)`을 하고 모든 몬스터가 어그로 사전을 다시 모은다(`MMBehaviorTree/AddFaction.cs:36`). 세력 2군 최상위가 6이 되므로 일반 몬스터가 친구 행세 중인 미메시스를 적으로 삼을 수 있다 — 미메시스가 "몬스터에게 쫓기는 플레이어"처럼 보이게 하는 장치로 읽힌다(실측 미확인). 미메시스 ML 에이전트도 10 m 안의 몬스터에서 도망간다(`_monsterDetectRange` 에셋 10, 코드 5; `DLDecisionAgent` sharedassets0:46036).

### 2.3 등장 — 던전 표

`Dungeon.json`(활성 72행) — 위험도 1~5·보상 1~5 조합마다 한 행. 스폰은 `DungeonRoom.cs:417-490`: 주기마다 `try_count`번 확률 굴림, 플레이어 내비 거리 `SPAWN_UNABLE_FROM_RANGE` 2000 안에는 안 놓는다(`DungeonRoom.cs:1087-1100`, 단위 미확인).

| 위험도 | 일반 몬스터 위협 예산 (`threat_value` 합) | 일반 주기 / 확률 | 미메시스 수 | 미메시스 주기 / 확률 | 미메시스 변종(`SpawnableMonsterGroup` 10001~10005) |
|---|---|---|---|---|---|
| 1 | 171~189 | 120초 / 75% ×2회 | 3 | 120초 / 75% | alpha 40 : beta 60 |
| 2 | 266~294 | 105초 / 85% | 3~4 | 105초 / 85% | 55 : 45 |
| 3 | 399~441 | 90초 / 100% | 4~5 | 90초 / 100% | 70 : 30 |
| 4 | 523~578 | 75초 / 100% | 5~6 | 75초 / 100% | 85 : 15 |
| 5 | 647~716 | 60초 / 100% | 6 | 60초 / 100% | alpha 100 |
| 잭팟 | 420~681 | 60초 / 75% ×3회 | 4~6 | 60초 / 75% ×3회 | 85 : 15 |

위협값: Sentryper·MadameCroak 100, BruteBunny·Maegoostro·Bombesis·Ramblam·PowMoth·TadaTeddy 60, 나머지 40. 미메시스가 죽으면 잔여 수가 되돌아와 다시 스폰될 수 있고(`DungeonRoom.cs:1257`), 일반 몬스터도 위협값이 예산으로 돌아온다(`:1278`). 위험도 1 그룹(101)에는 Quokka·Gorilla·HeavyD·Owl·Gazellerina·Cimim 6종만 있고, 2부터 Bunny·Bombesis·Goose·Ramblam·PowMoth·Teddy, 3부터 Sentryper·MadameCroak가 들어온다.

---

## 3. 변장(미메시스) 시스템

### 3.1 무엇을 베끼나

| 항목 | 베끼나 | 근거 |
|---|---|---|
| 외형 | **아니오** — 전용 `prefab_MimicPuppet`(`MonsterData.json` model_path), 스폰 애니메이션 `SpawnAsMimic`(`Mimic.Animation/PuppetScript.cs:197`). 플레이어 모델은 `prefab_PlayerPuppet`(`PlayerData.json`) | 모델이 같은지(동일 메시 여부)는 에셋을 열지 않아 미확인 |
| 이름표 | `SetActorName("Monster", MonsterInfo.Name)`로 `nameTag.text`에 표 이름이 들어간다(`ProtoActor.cs:4249·4577`) | 몬스터 이름표가 화면에 그려지는지는 미확인 |
| 소지품 | **예** — `CopyInventory(MaxDistance)`: 가장 먼 플레이어의 가방 전부를 가짜 아이템으로 복제(`AIController.cs:1291`, `InventoryController.cs:1982` `CloneNewFakeItemElement`). passive 진입 시 1회(alpha만) + 30초마다. 복제 뒤 `PickInvenSlot(ToggleEquip…)`으로 토글 장비(손전등 등)를 켠다 | `npc_mimic_alpha_base_passive.xml` |
| 액세서리(모자 등) | 가방 복제에 액세서리 아이템이 들어 있으면 함께 장착된다(`ProtoActor.cs:6053` `EquipAccessories` — 가방 슬롯에서) | 추정 |
| 목소리 | **예** — 3.2절 | |
| 이모트·점프 | **예** — `EmoteRespond`(응답 확률 100%, 지연 0.7~1초), `jumpRespondProb` 에셋 0.99(코드 0.8) | `DLDecisionAgent` 46036 |
| 경적 | **예** — 플레이어가 트램 경적을 당긴 패턴(최대 5초, 10개)을 녹음해 재생(`MimicHornRecorder.cs`, `HornPullcordLevelObject.cs:88`은 미메시스가 당긴 것은 녹음하지 않는다) | `ImitateTramHorn` |
| 시설 사용 | 전화(10초 통화, 상대 전화의 플레이어에게서 가장 먼 사람 목소리로), 송신기(전역 음성 채널에 말함), 셔터 스위치(2%·60초 쿨), 함정 스위치(50%), 충전기(30%), 스크랩 스캐너(60%), 파티 버튼 | `npc_mimic_alpha_base_passive.xml`·`_phone.xml`·`_transmitter.xml`·`_trap.xml` |
| 가짜 아이템 버리기 | `DropFakeItem` 확률 70%·쿨 45초(BT; 에셋 10%·10초): 돌아서서 아이템을 떨어뜨린다 — 떨어진 가짜는 가격 0이고 나침반이 무시한다(`Compass.cs:251`, `GamePlayScene.cs:1387`) | |
| 문 | 플레이어처럼 열고(20 m 안 플레이어 있을 때), **4초 쿨로 닫기도 한다**(`ChangeLevelObjectState(1,0)`) — 일반 몬스터는 열기만 | passive 트리 |

### 3.2 목소리 — 저장·선택·재생

**녹음(클라이언트, `Mimic.Voice.SpeechSystem/SpeechEventRecorder.cs`)**: 플레이어의 마이크 입력에서 Silero VAD(`_useSileroVad` 에셋 1, 문턱 0.5, `VoiceManager` 45906)가 발화로 판단한 구간을 0.7~7초(에셋 `sampleDurationRange`, `SpeechEventRecorder` 44834) 단위로 잘라 Opus로 압축하고, 그 순간의 게임 문맥을 붙인다 — 인접 플레이어 수, 구역(실내·실외·트램·송신기·데스매치), 게임 시각, 마주 보는 플레이어 수, 근처 스크랩, 근처 몬스터, 텔레포터, 실내 진입 여부, 충전기, 상점, 그리고 **직전 사건**(`IncomingEvent` — 곰덫에 걸림·미메시스 확인 중·몬스터 목격·피격·사망·스킬 사용·번개·정전 … 25종). 앞 200 ms 프리롤을 붙여 첫 음절을 살린다.

**보관(`SpeechEventArchive.cs`, `VoicePlayer` 49904)**: 플레이어마다 최대 **512**개(에셋; 코드 128), 그중 데스매치 30·실외 150. 넘치면 **재생 횟수가 많은 것부터** 버린다(`EvaluateValue = -AudioPlayedCount`, `:314`). 녹음 뒤 10초(`warmUpDuration`)가 지나야 쓴다. 보관함은 FishNet SyncList로 모든 클라이언트에 복제된다.

**선택(`Mimic.Voice/MimicVoiceSpawner.cs`, 호스트)**: 전체 준비된 조각이 3개(`minRequiredSpeechs`) 넘으면 시작. 미메시스마다 문맥을 두고 1초마다 돈다.

1. 15 m 안에 플레이어가 없으면 말하지 않는다(`:365`).
2. 누구 목소리인가 — 행동 트리의 `SetVoiceRule`(`BTVoiceRule.cs`): `Default`(아무나), `Dead`(**죽은 플레이어** — 10 m 안의 생존자 중 그 죽음을 120° 안에서 본 사람이 없는 시체를 고른다, 없으면 15 m 넘게 떨어진 시체, 없으면 랜덤; `VoiceManager.cs:1240`), `MaxDistance`(현재 위치에서 **가장 먼 생존자**). search 트리는 20 m 안에 플레이어가 있고 사망자가 있으면 Dead, 아니면 MaxDistance를 쓴다(`npc_mimic_alpha_base_search.xml`). 즉 **옆에 없는 사람·죽은 사람의 목소리**를 낸다.
3. 어느 조각인가 — `SpeechEventAdditionalGameData.PickBestMatch`(`:302`): 같은 구역·같은 실내외, 60초 안에 재생한 것 제외(데스매치 3초), 지금 미메시스에게 일어난 사건(`IncomingEvent`)과 같은 사건이 붙은 조각 우선(우선순위 곰덫 > 미메시스 확인 > 몬스터 > 플레이어 > 사망 > 피격 …), 그 뒤 문맥 유사도(마주 보는 수 가중 2, 나머지 1, 상위 5개 중 랜덤).
4. 언제 — `MMMimicMonsterTable`(sharedassets0:39051) 실값: 첫 발화 1~3초 뒤, 그다음은 **조각 길이 + 0.5~1초**(코드 기본 2~8초) 간격. 플레이어가 말을 마치면 20 m 안의 미메시스가 **0.2초 뒤 대답**한다(`TrySpawnMimicReply`, 쿨 2초 — 에셋; 코드 3). 피격·사건 때도 말한다(`ProtoActor.cs:4548`). 같은 사람 조각을 2초 창 안에서 이어 말하는 연속 발화도 있다(`ContinuousSpeechCount`).

**재생**: 미메시스 위치의 3D 음원(`PlayVoiceOnActor`, `ProtoActor.cs:9057`). **`muteLocalPlayerVoice`가 1(alpha·beta·mimic 20000001)이면 그 목소리의 주인은 못 듣는다**(`SpeechEventArchive.cs:577`) — "내 목소리가 저기서 난다"는 가장 확실한 텔을 본인에게는 막고, 동료에게만 들리게 한다. 데스매치용 endgame(20000003)만 0이다. 관전자는 관전 대상 기준으로 같은 규칙.

### 3.3 들켰을 때·못 가려냈을 때

| 상황 | 일어나는 일 | 근거 |
|---|---|---|
| 신뢰도 ≤10 (불신) | alpha: `combat`. beta: `runway`(도주) — 두 변종의 차이는 이것과 신뢰도 시작값(35 / 65)·공격 대기 시간(15초 / 45초)뿐 | `npc_mimic_{alpha,beta}_base_checkfriendly.xml` diff |
| 신뢰도 ≥90 (친구) | `bewarefriendly` — 계속 따라다니며 이모트·시설 사용. `TimeToAttack`(15/45초 또는 등 돌림 4회)에 combat | |
| 피격 어그로 ≥20 | 물러서기; ≥25면 들고 있는 가짜 무기(SkillEquip — 복제한 방망이·샷건)로 **반격**하거나 combat | checkfriendly 트리 |
| combat | 1.5 m·120°에서 목 조르기(스킬 300, `GrabData` 30001: 3.8초, 오염 +324/250 ms(`AbnormalData` 30003) → 아이템 낙하(30005) → 넉백(30006)); 15초 안에 못 잡거나 15 m 안에 아무도 없으면 runway. 잡은 뒤 12 m 안에 1명뿐이면 runway, 아니면 65% runway / 35% 다시 checkfriendly | combat 트리 |
| alpha `AlphaAttackType` 2 (25%) | 가짜 폭탄 5111을 꺼내고 Bombesis 자폭 스킬 1000 — **5 m 즉사** | combat 트리, `FieldSkillData` 70008 |
| 오염도 ≥10,000 | 사망, 1.5초 뒤(`WAIT_TIME_CONTA_SPAWN_MIMIC`) 그 자리에 `PickMimicMasterID`로 고른 미메시스 스폰 (`StatManager.cs:712`) | 플레이어가 미메시스로 "바뀐다"는 연출의 실체 |
| 죽은 뒤 | 관전자는 동료 10 m 안의 미메시스에 **빙의**(12초, 쿨 40초, 총 30초)해 자기 마이크로 말할 수 있다(`Const.json` POSSESSION_*, `PossessionController.cs:378` — passive~bewaredistrust 전 상태에서 가능). 빙의 중인 미메시스는 자동 발화를 멈춘다(`MimicVoiceSpawner.CanActorSpeak`) | 실험실 기능("빙의", 로컬라이즈 569) |

### 3.4 임포스터·의심과 협동

- 미메시스는 "특정 동료로 위장"하지 않는다 — 식별 문제는 "저 사람 모양이 동료인가"가 아니라 **"저기서 나는 동료 목소리가 진짜인가"**다. 목소리 주인은 못 듣고 옆 사람만 듣기 때문에, 확인은 "너 방금 말했어?"라는 **대화**로만 된다. 죽은 동료·멀리 있는 동료의 목소리를 쓰는 규칙은 그 대화가 통하지 않는 상황(상대가 없다)을 고른다.
- 의심 행동 자체가 비용이다 — 노려보면 불신→alpha는 공격, 때리면 반격, 가까이 가면(4 m) 신뢰 판정이 돌고, 등을 돌리면 공격. 안전한 거리는 신뢰 판정의 재접근 거리 8 m(에셋 `_encounterDistanceThreshold`) 밖이다.
- 협동 장치: 한 명이 보면서(미메시스를 묶음) 다른 한 명이 친다는 명시 규칙은 없지만, 신뢰도가 **표적 한 명**에 대해서만 계산되고(`_trustEvalPrevTargetID`) 다른 사람의 시선은 세지 않으므로, 표적이 아닌 사람이 치는 것이 성립한다. 탐지기를 든 사람이 경보 역할이다.

---

## 4. 감지 모델

### 4.1 시야

| 항목 | 값 | 근거 |
|---|---|---|
| 몬스터 → 플레이어 "발견" | 반지름(종마다 15~25 m) + 같은 층(높이차 ≤7) + 실내외 일치. **각도·벽 없음** | 1.3절 |
| 상태 전이 내비 거리 | 발견 뒤 `CheckDistanceToTarget(ByNavMesh, 20~30, LE)` — 경로 길이 기준이라 벽 너머는 먼 것으로 친다 | 각 passive 트리 |
| 몬스터 시야각이 있는 판정 | MadameCroak 주시 10 m·120°(`CollectMovingPlayerInRange`), Gazellerina `IsTargetVisible(20, 0.6)` | |
| 플레이어 → 몬스터 시선 | `AnyActorWatchingInRadius(30, 100)` — 반각 50°, 가시선 필요. Sentryper·PowMoth·MadameCroak search·MamaGorilla가 쓴다 | `AnyActorWatchingInRadius.cs:33` |
| 미메시스 쪽 "쳐다보는가" | 표적 yaw 기준 45°(`CheckPlayerLookingAtMe`, `DLDecisionAgent.cs:3166`); "등 돌림"은 60°(`:1257` `CheckLineOfSight` 기본) | |

### 4.2 소음

| 소리 | 받는 몬스터 | 값 |
|---|---|---|
| 마이크 음성 | Sound 어그로 종(BruteBunny 15 m, Maegoostro 20 m) | 매초 피크×100 × (1−d/R) × 100. 침묵 3초면 공명값 0(`StatManager.cs:1089`) |
| 발소리·이동 | 같은 두 종 — `increase_score_per_distance`가 두 종 모두 0이라 **실제로는 꺼져 있다** | `MonsterData.json` |
| 손에 든 물건 | 같은 두 종 — 시끄러운 닭 5/tick, 기타 5, 덤벨 10, 붐박스 5 | `ItemEquipment.json` `sound_aggro_in_hand_*` |
| 아이템 사용 소음 `sound_aggro_per_use` | 같은 두 종 | 정식 아이템은 전부 0(테스트 페인트볼 10만) |
| 확성기 5131 | 음성 증폭 +100·범위 +1000(`AbnormalData` 10013) → 공명값이 커진다(추정) | |
| 그 외 몬스터 | **소리를 듣지 않는다** | Hit 어그로만 |

마이크는 듣는다 — 단 두 종뿐이고, 그 두 종은 반대로 반응한다(거위는 달려오고 토끼는 도망간다).

### 4.3 빛

- 손전등은 몬스터 어그로·행동 트리 어디에도 없다. 미메시스 ML 관측 프레임에 `obs_actor_flashlight_on`이 들어가고(`DLAgent/DLAgentObservationFrameData.cs:25`), 미메시스는 복제한 손전등을 켠다 — **손전등은 감지가 아니라 위장 도구**다.
- 정전(`BLACKOUT_DURATION` 30~60초, 비 오면 100%·폭우 50%·스콜 75%, `WeatherData.json`)은 공포 연출이고 몬스터 수치에 영향이 없다(미확인 — 몬스터 쪽 정전 분기는 찾지 못했다).

### 4.4 숨기

- 자세(앉기·엎드리기) 없음. 벽 뒤도 소용없다(반지름 감지). "숨기"는 **실내외 경계**(실내 몬스터는 실외 플레이어를 표적으로 못 잡는다, `AggroObject.cs:72`)와 **층**(높이차 7)뿐이다.
- 트램 셔터(`TramShutterDoor` 업그레이드)로 문을 닫는 것이 유일한 물리적 은신처다 — 미메시스도 셔터 스위치를 쓴다.

---

## CL9에 대응하는 자리

CL9 현황은 [C:\Work\CL9\docs\reference\combat-gap.md](C:\Work\CL9\docs\reference\combat-gap.md) 2절·[C:\Work\CL9\docs\reference\monster-rules-comparison.md](C:\Work\CL9\docs\reference\monster-rules-comparison.md) 7절에서 가져왔다. 결정은 적지 않는다.

| 장치 | MIMESIS | CL9 지금 |
|---|---|---|
| 변장형 몬스터 | 소지품·목소리·행동 복제, 외형은 전용 모델. 신뢰도 −3/+3/s, 등 돌림 2초 공격 | 없음. "농부 탈을 쓴 동물"·임포스터 규칙은 미커밋 초안(이 문서가 조사하지 않았다) |
| 식별 수단 | 대화(본인은 못 듣는다)·노려보기(비용 있음)·치기(반격)·탐지기 7 m | 없음 |
| 들킨 결과 | 목 조르기 → 오염도 절반 + 아이템 낙하, 오염 초과 시 시체에서 새 미메시스 | 너구리 등 뒤 처형 즉사, 돼지 처형 즉사 |
| 시선 규칙 | Sentryper(보면 멈춤, 치면 무시), MadameCroak(보는 동안 움직이면 죽음), Gorilla(보면 물러남 ×2) | 너구리 "3초에 한 번 돌아봐라, 6초 노려보지 마라" |
| 소음 | 마이크를 듣는 종 2(거위는 오고 토끼는 간다), 손에 든 물건 소음, 발소리 없음 | 걷기 소음 0, 거위 `WakeRadius` 450 |
| 감지 | 반지름 15~25 m, 각도·벽 없음, 같은 층·실내외 | 닭 `SightIn` 330 cm + 가림 트레이스 |
| 빛 | 감지 아님, 미메시스 위장용 | `SightDark` 소비처 없음 |
| 등장 | 위험도별 60~120초 주기, 미메시스 3→6, 죽으면 수가 돌아옴 | 판 시작 일괄 + 7e |
| 접근 | 2~6분 뒤 `search`로 플레이어 쪽에 걸어온다 | `SearchHops` 3 → 집으로 |
| 몬스터끼리 | 세력 표 — 친구 행세 미메시스를 몬스터가 쫓을 수 있다 | 무리 경보·사살 경보 |
| 처치 | 미메시스 100 HP / 방망이 3대·샷건 1발 — 죽여도 수가 돌아온다 | 팬 2~4대, 영구 |

---

## 근거와 한계

- 디컴파일 캐시 `C:\Users\Radar\AppData\Local\CL9\UnityAnalysis\mimesis\decompiled\Assembly-CSharp`. 행동 트리는 `C:\Program Files (x86)\Steam\steamapps\common\MIMESIS\MIMESIS_Data\StreamingAssets\aidata\bt\`의 XML을 그대로 읽었다(위 표의 "트리"는 그 파일명). 마스터 데이터는 같은 폴더의 `masterdata\*.json`.
- 에셋 실값: `MMMimicMonsterTable` sharedassets0:39051, `SpeechEventArchive` 49904, `SpeechEventRecorder` 44834, `MimicVoiceSpawner` 49761, `VoiceManager` 45906, `DLDecisionAgent` 46036, `Detector` 47012~47014. `DLDecisionAgent`는 코드 초기값과 다른 필드가 41개였고 본문은 에셋 값을 썼다(일부는 행동 트리의 `SetBlackBoardParam`이 다시 덮어쓴다 — 본문에 "BT"라고 표시).
- **미확인**: 미메시스 모델이 플레이어 모델과 같은 메시인지, 몬스터 이름표가 화면에 보이는지, `move_speed` 단위, `SPAWN_UNABLE_FROM_RANGE`의 단위, `_possessionAngleMax` 값, 정전이 몬스터 행동에 주는 영향, 세력 6으로 인한 몬스터→미메시스 공격의 실제 빈도, ML 이동 모델(`.onnx`)이 무엇을 보고 어떻게 움직이는지(관측 벡터 구성만 읽었다), HeavyD 스킬 900의 피해(시퀀스가 비어 있다).
- 플레이·영상 실측은 하지 않았다. 숙련 단계 열은 메커니즘에서 읽은 추정이다.
