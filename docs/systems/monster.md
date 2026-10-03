# 몬스터 공통 시스템 (AI 상태 기계)

> **AI 코드는 틀 하나다.** 순찰·조사·추격·잡기 같은 로직은 모든 몬스터가 공유하고,
> 몬스터 간 차이(순찰 범위·이속·추격 지속·감각 등)는 전부 `UCL9MonsterDef` 표의 값이다.
> 새 몬스터 = 표에 행 추가 (+ 값으로 안 되는 특수 동작만 기믹).
> **값의 정본은 [../plan/data/monster/common.md](../plan/data/monster/common.md)** —
> 이 문서는 그 값을 읽어 도는 구조만 정의한다.

## 전제 (규약)

- **경로 탐색 방식은 구현에서 정한다 (미결)** — NavMesh든 커스텀 그래프든, 층 봉쇄·일방향
  개구멍(flapIn) 요구사항을 만족하는 쪽으로 ③에서 선택한다. 시야 차단·존 판정 기준도 그
  선택과 함께 정한다.
- **판정은 호스트 전용.** AI 틱 진입부에 `if (!HasAuthority()) return;`. 클라는 표현만.
- `ACL9Monster : ACL9ActorBase` — Alias로 표를 읽고(`LoadData`), 상태는 Replicated 변수로
  동기화한다 ([actor-base.md](actor-base.md)).
- **`TakeDamage`·`UDamageType` 금지.** HP는 Replicated `int32`, 타격은 함수 호출이다.

## 상태 기계

```
          WakeTelegraph (0.8초 예고 — 전 플레이어에게 보인다)
잠듦 ──────────────────────────→ 평시 (배회 Wander / 순찰 Patrol)
 ↑  WakeSightRadius 안에 시선 있으면      │
 │  기상 보류 (WakeRetryTime 재시도)      │ 발각 게이지 만충 / 소리·경보 수신
 │                                       ↓
 │ SleepTime (거위)              조사 Investigate ── SearchHops 지점 순회,
 │                                       │           지점당 SearchDwellTime
 │                                       │ 시야 확보 (SightIn)
 │                                       ↓
 └── 복귀 Return ←── 포기 Break ←── 추격 Chase ──→ 예고 Pounce (PounceRadius) ──→ 잡기 Catch (덮친 뒤 재검사 통과)
     (집으로)      (ChaseBreakTime      │  ChaseBudget 소진 /      │  PounceWindup 뒤 PounceLunge 덮침    │ CatchPause 뒤
                    두리번 →            │  ChaseLoseTime 시야 상실 /  │  헛침 → Chase (PounceMissCooldown)  │ Chase 로 복귀
                    ChaseCooldown)      │  HomeRange 이탈 /         │  (PounceWindup 0 이면 Chase → Catch 직행)
                                        │  DeaggroRadius (짐 전부 내려놓음)
                                        ↓
                          경직 Stun (HitStunMs / PushStun) — 어느 상태에서든 진입 (예고 중이면 예고 취소; 처형 중은 예외)
                          복수 Revenge / 격노 Rage — 사살 소리·피격에 반응
                          죽음 Dead — HP 0, 죽는 모션 (연출은 종별 선택)

 스토커 (ExecuteRadius > 0 — 너구리):
   발각 ──→ 스토킹 Stalk ──(응시 StalkStareAnger / 피격 Stun 뒤 / Rage / 무리 경보)──→ 추격 Chase (위와 같다)
              │ 보이면 정지·후퇴 (StalkRetreatRadius) / 안 보이면 등 뒤로 접근 (StalkSpeed)
              │ 놓침 규칙은 Chase 와 같다 → Break
              │ 미발견 ExecuteUnseenTime ∧ ExecuteRadius ∧ 등 뒤 원뿔 ∧ 시야 ∧ 앵커 스윕 통과
              ↓
           처형 Execute ──(ExecuteDuration 뒤 HostKill, 피해자 관전 시작 또는 + SpectateDelay)──→ Break
              (앵커로 텔레포트, 처형 카메라, 구조 없음 — 맞아도 경직·상태 변경 없음)

 잡히면 처형 (bExecuteOnCatch — 돼지): 위의 추격·예고·덮침 그대로, 잡기 Catch 자리에 처형 Execute
   추격 Chase ──→ 예고 Pounce ──(덮친 뒤 재검사 통과)──→ 처형 Execute (위와 같다 — 텔레포트 없이 잡은 자리에서)
                  (PounceWindup 0 이면 Chase → CatchRadius 에서 Execute 직행; CatchDamage·CatchPause 없음, KillCooldown 은 HostKill 때)
```

- 상태는 enum 하나 (`ECL9MonsterState`) — Replicated, OnRep에서 애니메이션·소리 전환. `Pounce`·`Stalk`·
  `Execute`는 enum **끝**에 그 순서로 있다(복제·에셋의 옛 값 보존).
- `ChaseCooldown`은 **새로 경계 상태에 들어가는 것만** 막는다. 정전·복수·기절에서 깨어나는 건
  못 막는다.
- **타격 플래시** (2026-09-15): `Stun`·`Dead`에 들어간 뒤 `HitFlashTime`(rules) 동안 메시가
  `HitFlashMaterial` 오버레이를 쓰고, 지나면 벗는다 (`ACL9Monster::TickHitFlash`, 모든 머신).
  때렸다는 사실을 화면에 알리는 절반이고 나머지 절반은 공격자 카메라 킥이다
  ([character.md](character.md) 피격 절). 규칙 셋:
  - 원천은 복제된 `StateStartServerTime` 하나다 — 멀티캐스트도 새 복제 필드도 없고, 호스트·클라가
    각자 같은 경과를 계산해 같은 그림을 만든다.
  - **상태로 거른다**: 레벨에 처음부터 놓여 있는 개체는 `StateStartServerTime`이 0이라 경과가
    아주 크지만, 애초에 `Stun`·`Dead`가 아니므로 PIE 시작 직후 빨개지는 일이 없다.
  - 죽음도 한 번 번쩍이고 끝난다 (`Dead` 진입 시각 기준이라 시체가 계속 빨갛지 않다).
  - 오버레이는 켜지고 꺼질 때만 세팅한다(캐시된 bool). 머티리얼이 비어 있으면 아무것도 하지 않는다.

## 감각 파이프라인 (공통, 호스트 틱)

세 입력이 하나의 **발각 게이지**로 모인다:

1. **시야** — 히스테리시스: 발견은 `SightIn`, 놓침은 `SightOut`. 정전 시 `SightDark`.
   시야에서 벗어나도 `DetectGrace` 유예. 벽 차단 판정 방식은 경로 탐색 선택과 함께 정한다.
2. **청각** — 소리 반경은 몬스터가 아니라 **행동이 정한다** (`CL9Rules`:
   달리기 390 / 조용한 줍기 150 / 시끄러운 줍기 390 / 밀치기 255 / 미끼 330).
   **걷기는 어그로 0.** 몬스터 쪽 값은 `HearRadius` 하나다.
   (예외 축 하나: `WakeRadius`가 0보다 큰 종은 그 안에서 걸어도 깬다 — 지금은 거위 450뿐,
   [../plan/data/monster/goose.md](../plan/data/monster/goose.md))
3. **근접** — `DetectNearRadius` 안이면 게이지가 `DetectRateNear`로, 밖 시야는
   `DetectRateFar`로 오르고, 미감지 시 `DetectDecay`로 내린다.

게이지 만충(0~1 정규화, 1.0 = 발각) → 조사 또는 추격. 발광 아이템 소지자는 정전 때
`CarrierSeenRadius`에서 보인다.

## 이동

- 속도는 상태가 고른다: 배회 `WanderSpeed` / 순찰 `PatrolSpeed` / 추격 `ChaseSpeed` /
  후퇴 `RetreatSpeed`. 마당에서는 `OutdoorChaseMult`.
- **`TurnDrag`가 미로를 무기로 만든다** — 꺾을수록 느려진다. `TurnRate`로 회전 상한.
- `SeparationDist`로 서로 벌어진다. 몸 크기는 `Radius`.
- 몬스터는 ACharacter가 아니다 — CMC 없는 커스텀 이동 (경로 방식 미결). 위치는 표준 액터
  복제(20Hz)로 오지만 **일반 AActor는 클라에서 위치를 스냅한다** — 부드러운 이동은 클라 보간
  컴포넌트(스냅샷 버퍼)를 직접 짜야 하고, 이건 3c의 작업 항목이다.

## 잡기 = 공격

이 게임의 몬스터 공격은 데미지 수치가 아니라 **잡기**다:

- `CatchRadius` 안 → 잡힘: `ICL9Damageable::OnDamage(CatchDamage)` 호출 (초안 1,
  `HurtStun` 비틀) + 넘어짐. **짐은 흘리지 않는다 — 짐 흘림은 HP 0이 되는 타격뿐이다**
  (2026-09-15 사용자; 넘어졌다 일어나면 들고 있던 것을 그대로 들고 있다. 수레는 놓는다).
  죽음·즉사 여부는 HP와 타격량 값이 정한다 — 원칙이 아니다.
- 잡은 뒤 `KillCooldown` 재장전 — 연속 타격으로 3타를 순식간에 넣지 못하게 하는 장치.
  **`KillCooldown` = `RageTime` 등식은 로드 검사가 확인한다.**
- **넘어진 사람은 무적이다** (2026-09-20 사용자): `ACL9Character::OnDamage`가 `IsKnockedDown()`이면 아무것도
  하지 않는다(HP·넘어짐 시계·피격 사건 불변, 로그 한 줄) — 다른 닭의 덮침도 `cl9.damage` 치트도 같은 문이라
  우회가 없다. 몬스터 쪽은 **진입 검사에서 건너뛴다**: Chase 틱의 잡기·예고 조건에 `!Target->IsKnockedDown()`이
  들어 있어 넘어진 타깃엔 Pounce도 Catch도 시작하지 않고, 추격 자체(예산·놓침·경로)는 그대로 이어져 타깃이 서는
  순간 다시 예고가 시작된다. 타깃 선택(`SenseTick`)에서 빼지 않은 이유: 빼면 시야를 잃은 것으로 처리돼 놓침 시계가
  돌거나 동료에게 튀어 "덮치려던 닭이 옆에 서 있다"는 그림이 깨진다. 이미 예고 중이던 닭은 예고를 끝내고 덮치되
  재검사에서 넘어진 타깃을 헛침으로 친다.

### 잡기 예고 (Pounce, 2026-09-20)

잡기 앞에 읽을 수 있는 예고가 온다 ([../goal/monster-pounce-hurt-readability.md](../goal/monster-pounce-hurt-readability.md) A).
값은 `UCL9MonsterDef` 네 열 `PounceRadius`·`PounceWindup`·`PounceLunge`·`PounceMissCooldown`
([../plan/data/monster/common.md](../plan/data/monster/common.md) 추격) — **`CatchRadius` 0 또는 `PounceWindup` 0이면
예고 없이 옛 즉시 잡기**다.

- **진입** (Chase 틱, `AlertWindup` 뒤): `Dist2D <= PounceRadius ∧ CatchCooldownRemaining <= 0 ∧ PounceMissRemaining <= 0
  ∧ 타깃이 넘어져 있지 않음` → `EnterState(Pounce)`. 타깃을 `PounceTarget`(TWeakObjectPtr)에 잠근다 — 예고 동안
  `SenseTick`은 돌지 않아 감각의 재타깃팅은 무시된다. 진입 시 `HostReportNoise(MonsterPounce)` 한 번(파동 링
  `monster_pounce`, [wave.md](wave.md)) + `ForceNetUpdate()`(20Hz 주기를 기다리지 않는다).
- **예고 틱**: 이동 0. 먼저 Chase의 탈출 조건을 같은 함수(`TickChaseGiveUp` — 타깃 소실·사망·`ChaseBudget`·
  `ChaseLoseTime`·`HomeRange`·`DeaggroRadius`)로 평가해 걸리면 Break — Pounce는 Chase의 부분 상태라 예산·놓침
  시계가 계속 흐른다. 아니면 타깃을 향해 `TurnRate`로만 돈다(`MoveAlongPath`의 회전 저항을 우회하므로 상한을 같이
  둔다 — 홱 돌지 않는다). `PounceWindup`이 다 되면 덮친다.
- **덮침** (`HostPounceLunge`): 타깃의 **지금** 위치 방향으로 `PounceLunge`만큼 **몸체 구 스윕**
  (`SweepSingleByObjectType`, WorldStatic + WorldDynamic — 바닥·문 조각 프로필이 제각각이라 둘 다; 자신·타깃·캐릭터·
  아이템·몬스터는 무시 목록) 으로 닿는 데까지 수평(Z 평탄) 이동. 도달 위치로 `HeadingYaw` 동기·경로 무효화.
  **재검사는 도달 위치 기준**: `Dist2D <= CatchRadius ∧ |ΔZ| <= Radius + 캡슐 반높이(위층 배제) ∧ 몬스터→타깃
  Visibility 선 트레이스 안 막힘 ∧ 타깃이 넘어져 있지 않음` → `HostCatch`(기존 그대로: `OnDamage`·`KillCooldown`·
  Catch 휴지기). 아니면 **헛침** — `PounceMissRemaining = PounceMissCooldown`, `ResumeChase()`. 원격 잡기는 없다.
- **결과 복제**: `PounceResultServerTime`(float)·`bPounceHit`(bool)을 찍고 `ForceNetUpdate()`. 게터 `IsPounceHit()`·
  `GetPounceResultElapsed()`(= 서버 시각 − 결과 시각; **결과가 없으면 1000 이상** — 기본값 −1000)·`HasPounceResult()`.
  ABP는 헛침이면 `State == Chase ∧ !IsPounceHit ∧ GetPounceResultElapsed < 0.3` 동안 `Catch` 클립 앞부분을 평가기로
  튼다(농부 휘두르기와 같은 시계 방식 — 연속 헛침도 시각이 갱신돼 구분된다). 성공은 `Catch` 상태 그대로.
- **`ResumeChase()`**: Catch 휴지기 끝과 헛침이 같이 쓰는 헬퍼 — `EnterState(Chase)`는 예산·놓침 시계를 리셋하고
  경보를 다시 울리므로 쓰지 않는다. `State = Chase; StateStartServerTime = 지금; 경로 리셋; ChaseRepathRemaining = 0;
  ApplyState()`.
- **우선순위**: Stun(`OnDamage`/`OnShove`)·Dead는 예고를 취소한다(`EnterState`가 덮어쓴다; `KillCooldown` 재충전 시계
  `CatchCooldownRemaining`은 그대로 →
  경직 뒤 다시 예고 — 반격 창). Rage 진입은 진행 중인 Pounce/Stun과 두 쿨다운을 건드리지 않는다(`ReceiveKillAlarm`·
  `ReceivePackAlarm`의 switch에 Pounce가 없다). `PounceMissRemaining`은 `HostTickAI` 머리에서 틱당 한 번만 줄고,
  `KillCooldown`은 헛침·취소에 영향이 없다.
- **로드 검사** (`UCL9MonsterDef::Validate`): 네 열 유한·비음수; 예고가 켜진 종은 `PounceRadius >= CatchRadius`.
- **회피 산수의 정직한 서술**: 예고 진입은 `PounceRadius` 이하 어디서든 일어나므로 걷기로 물러나면 산다는 것은
  값(`PounceWindup`·`PounceLunge`)이 정하는 사실이지 코드가 보장하는 것이 아니다. 헛친 뒤 간격은 같은 속도로 물러나는
  사람에겐 다시 좁혀지지 않는다 — 재예고는 "간격이 다시 좁혀지면(멈추거나 돌면)"이다.
- 플레이어의 대응: 도망(속도 부등식 A1·A4가 탈출을 보증), 든 아이템 휘두르기(좌클 —
  행의 `HitDamage`·`HitStun`·`HitKnockback`이 `OnStrike(Damage, Stun, Knockback, Source)`로 들어온다: 프라이팬 = `Hit`(`HitStunMs`
  경직 + `HitKnockback` 넉백, 2026-09-22 — 옛 `Push` 행은 없다), 고기 = `Hit`(넉백 0);
  [../plan/data/item/common.md](../plan/data/item/common.md) 사용).
  **맞느냐 마느냐는 휘두르기 판정이 정한다** (2026-09-22 — 손 높이 원점에서 컨트롤 회전 방향으로 끄는 구 + 가림 트레이스;
  2026-09-15의 "메시 경계 구가 손을 따라 지나간다"는 폐기): 좌클을 뗀 뒤 `CL9Rules.SwingHitStart`~`SwingHitEnd` 구간 동안
  걸린 몬스터가 **한 번의 휘두르기에 각각 한 번씩** `OnStrike`를 받는다 — 나란히 선 둘은 한 번에 둘 다 맞고, 빗나가면 아무도
  맞지 않으며, 벽 너머는 맞지 않는다. 판정은 호스트 전용이고 `OnStrike`의 의미(피해 먼저, 죽으면 경직·넉백 없음, `Push`는
  `OnShove`로 덮음)는 그대로다 ([character.md](character.md) 판정, [inventory.md](inventory.md) 휘두르기 히트박스).

### 타격 넉백 (2026-09-22, [../goal/combat-feel-pass-1.md](../goal/combat-feel-pass-1.md) D-5)

- `OnDamage(Amount, Source, Knockback)` — `ICL9Damageable` 시그니처가 바뀌었다. Knockback(cm)은 아이템 행 `HitKnockback`이고
  잡기(`HostCatch`)·치트(`cl9.hitmonster`·`cl9.damage`)는 0을 넘긴다. 캐릭터 구현은 받아도 쓰지 않는다.
- **시계**: Stun 진입 뒤 `CL9Rules.HitStopTime`(0.08) 동안은 멈춘 프레임이라 밀리지 않고(`KnockbackDelay`), 그 뒤
  `HitKnockbackTime` 동안 등속(`HitKnockback / HitKnockbackTime`)으로 민 뒤 멈춘다 — 경직 시간(`HitStunMs`)과는 별개 시계라
  0.08 + 0.12 = 0.2 < 0.3 안에 끝난다. `OnShove`의 밀침 넉백은 옛 그대로(지연 0, `PushKnockback / PushStun`).
- **방향**: Source → 몬스터 수평; Source가 null이거나 겹쳐 0이면 `−ActorForward`(`OnShove`와 같은 폴백).
- **벽**: 스텝마다 이동 전에 `TraceWorld`(아래)로 지금 자리 → 다음 자리 + 몸체 반경까지 그어 막히면 그 자리에서 종료.
  `AddActorWorldOffset` 스윕은 WorldStatic만 막는데 맵 조각 프로필이 제각각이라(2026-09-18) 믿지 않는다. 스텝당 트레이스 1회.
- **죽이는 타격(HP ≤ 0 → `Die`)에 넉백은 없다** — 죽음 발사가 대신한다. 측정할 때 이 둘을 섞지 않는다.
- `ECL9HitStun::Push`·`OnShove`·`bPushStun`·`PushStun`/`PushKnockback` 열·`PushReact` 클립은 **남는다**(사용자 결정, 쓰는 행 없음).
- 측정 치트 `cl9.knockbacklog` ([cheats.md](cheats.md)).

### 세상 트레이스 `ACL9Monster::TraceWorld` (2026-09-22 공용 승격)

`static bool TraceWorld(const UWorld*, const AActor* Self, const FVector& From, const FVector& To, FHitResult& OutHit, const AActor* AlsoIgnore = nullptr)`
— `ECC_Visibility` 선 트레이스로 **세상 지형만** 본다: 맞은 것이 `ACL9Character`·`ACL9Item`·`ACL9Monster`면 무시 목록에 넣고
다시 긋는다(최대 4회). Self·AlsoIgnore(든 아이템)는 처음부터 뺀다. 막히면 true. 죽음 발사(벽·바닥)·휘두르기 가림·표면음·
넉백 스텝 넷이 같이 쓴다 — 옛 `HostTickDeathLaunch`의 람다를 올린 것이다.

### 스토킹과 처형 (Stalk·Execute, 2026-09-20)

종 전용 즉사 ([../goal/execute-sequence-pig.md](../goal/execute-sequence-pig.md) — 설계 결정 D1~D9 가 정본;
2026-09-20 저녁 사용자 결정으로 **두 종**: 너구리 = 몰래 뒤에서 처형하는 **스토커**, 돼지 = **잡히면 무조건 처형**).
열은 `UCL9MonsterDef` Category `Execute` 13개 + `bExecuteOnCatch` + `CL9Rules.ExecuteCameraBlend`
([../plan/data/monster/common.md](../plan/data/monster/common.md) 스토킹과 처형). 갈래는 표 값으로만 갈린다 —
**스토커** `ExecuteRadius > 0`(아래 Stalk 진입 ~ 앵커 스윕), **잡히면 처형** `bExecuteOnCatch`(아래 "잡히면 처형"
항목), 둘 다 아니면 처형 없는 종이라 `GetHuntState()` 가 `Chase` 를 돌려주고 아래는 하나도 돌지 않는다(빌드 없는
롤백). `UCL9MonsterDef::IsExecutioner()` = 둘 중 하나(치트가 쓴다).

- **Stalk 진입** (스토커): 감각이 타깃을 잡아 `Chase` 로 가던 두 자리(Wander/Patrol/Return 의 게이지 만충, Investigate 의
  시야 확보)가 `EnterState(GetHuntState())` 다. 진입은 추격과 같은 시계(`ChaseBudget`·`ChaseLoseTime`)를 채우고
  `StalkTarget` 을 잠그며 두 계수기(`StalkUnseenTime`·`StalkStaredTime`)를 0 으로. **팩 경보는 울리지 않는다**
  — 스토킹은 조용하고, 격분해 `Chase` 로 갈 때 그 진입이 (`bPackAlarmFired` 가 아직 false 라) 경보를 낸다.
- **Stalk 틱** (호스트): 감각(`SenseTick`)은 계속 돈다 — 다른 산 사람(처형 중 아님)을 잡으면 타깃을 옮기고 계수기를
  처음부터(D4). 놓침은 `TickChaseGiveUp` 그대로(타깃 소실·사망·예산·놓침 시계·HomeRange·Deaggro) + 타깃이
  **다른 개체에게 처형 중**이면 소실로 침 → `Break`.
  - `IsWatchedByAnyone()`: 살아 있고 처형 중 아닌 어느 플레이어든 `StalkGazeRadius` 안에서
    `|NormalizeAxis(GetBaseAimRotation().Yaw − (몬스터−플레이어) 방향 요)| ≤ StalkGazeHalfAngle` ∧ `HasLineOfSight`.
    **`IsHeldByGaze` 를 재사용하지 않는다** — 3D 내적 0.75 는 바닥을 보면 "안 봄"이고 41° 는 화면보다 좁다(D5).
  - 보이면: 미발견 0, 응시 `+= dt`; `≥ StalkStareAnger` → `EnterState(Chase)`(격분 — 이후 닭과 같은 Chase·Pounce·Catch).
    정지(경로 버림). 타깃이 `StalkRetreatRadius` 안이면 **타깃 반대쪽의 그 반경 지점**으로 `StartPathTo` 해
    `RetreatSpeed` 로 물러난다 — 경로 없으면 정지. 후퇴↔접근이 바뀌는 순간 경로를 버린다(`bStalkRetreating`).
  - 안 보이면: 응시는 같은 속도로 0 까지 감쇠, 미발견 `+= dt`. 넘어진 타깃이면 서서 기다린다(계수기는 흐른다 —
    일어나는 순간 조건이 맞으면 바로 처형). 아니면 **처형 진입 검사** → 실패면 타깃 등 뒤 지점(`GetBehindPoint`
    = 위치 − 전방(요) × `ExecuteAnchorDistance`)으로 추격과 같은 `ChaseRepathInterval` 재경로, 실패면 타깃 자체로,
    `StalkSpeed` 로 `MoveAlongPath`.
  - 경보: `ReceiveKillAlarm`(Rage)·`ReceivePackAlarm` 의 switch 에 `Stalk → Chase`. Stun(`OnDamage`)은 `EnterState` 가
    덮어쓰고, 경직 끝은 기존대로 공격자 `Chase` 다(격분).
- **처형 진입 검사** (`TryBeginExecute`, 전부 ∧): 안 보임 ∧ `StalkUnseenTime ≥ ExecuteUnseenTime` ∧ Dist2D ≤
  `ExecuteRadius` ∧ 등 뒤(타깃 전방(요) · (타깃→몬스터 단위벡터) ≤ −cos(ExecuteBehindAngle)) ∧ `HasLineOfSight` ∧
  `CatchCooldownRemaining ≤ 0` ∧ `!IsKnockedDown` ∧ `!IsExecuted` ∧ `!IsDead` ∧ **앵커 스윕**. 실패 사유는
  **바뀔 때만** Verbose 한 줄(`execute check on … fails: unseen-time | radius | behind-cone | line-of-sight |
  kill-cooldown | target-knocked-down | target-executed | target-dead | anchor-blocked`, 미발견·거리 수치 포함).
- **앵커 스윕** (`SweepExecuteAnchor`, D1): 앵커 = 등 뒤 지점, 높이는 **이 몬스터의 지금 Z**(스포너가 바닥 + `Radius`
  에 놓고 덮침도 Z 평탄 — 타깃 발 높이로 새로 재면 구가 바닥에 접해 시작부터 끼인 것으로 잡힌다). 몸체 구
  (`Radius`)를 타깃 자리(같은 Z)에서 앵커까지 `SweepSingleByObjectType`(WorldStatic|WorldDynamic, 자신·타깃·
  모든 캐릭터·아이템·몬스터·**타깃의 `PullingCart`** 무시). 막히면 그 틱은 처형하지 않고 스토킹을 계속한다.
  타깃이 벽에 `Radius` 안으로 붙어 있으면 시작 겹침으로도 막힌다 — 보수적 거부.
- **잡히면 처형** (`bExecuteOnCatch` — 돼지, 2026-09-20): `GetHuntState()` 는 `Chase`(`ExecuteRadius` 0) — 추격·예고·
  덮침·즉시 잡기가 닭과 같고, `bCanCatch`·덮침 재검사가 넘어진·처형 중 타깃을 이미 거른다. `HostCatch` 맨 앞에서
  `bExecuteOnCatch` 면 `OnDamage`·`CatchCooldownRemaining`·`EnterState(Catch)` 를 타지 않고
  `HostBeginExecute(Target, GetActorLocation(), bTeleport = false)` — 텔레포트 없이 잡은 자리에서 타깃을 향해
  `Execute` 로. `HostPounceLunge` 의 `bPounceHit`·`ForceNetUpdate` 는 그대로(덮침 결과 클립). 아래 Execute 틱부터는
  스토커와 같다 — `KillCooldown` 은 `HostKill` 자리에서 재충전. 스토킹 열은 읽히지 않는다.
- **Execute 진입** (`HostBeginExecute(Target, Anchor, bTeleport)`, 호스트): `bTeleport`(스토커) 면
  `SetActorLocation(앵커, TeleportPhysics)`, 잡히면 처형인 종은 `Anchor = 지금 위치` 로 생략. 그 뒤는 같다 — 타깃을 향한
  요, `SmoothedLocation/Rotation` 을 액터 값으로, `ExecuteTarget` 잠금, `EnterState(Execute)`(경로 리셋·`bExecuteKilled`
  false·`ForceNetUpdate`), `Target->HostBeginExecuted(this)`, 로그(`teleported to (…)` / `executes on catch at (…)`).
  피해자 캡슐·위치·콜리전은 건드리지 않는다(규칙 3) — **요만** 캐릭터 쪽 `HostBeginExecuted` 가 이 몬스터에 맞춘다(2026-09-21, 2인 클립 정렬:
  `bExecuteOnCatch` 면 피해자가 이 몬스터를 정면으로, 앵커 텔레포트면 등진 채 — [character.md](character.md) 처형 "피해자 요 정렬").
  진입 전이에 `MonsterCatch` 소리(`TickStateSounds` — 처형은 `Catch` 를 지나지 않으므로 같은 행을 여기서).
- **Execute 틱**: 이동 0. `ExecuteTarget` 이 null(접속 끊김)이면 `Break`. `GetStateElapsed() ≥ ExecuteDuration`
  이고 아직 안 죽였으면 `Target->HostKill(this)` 한 번 + `CatchCooldownRemaining = KillCooldown`. 그 뒤 **머문다**
  (D6 — 피해자 시점은 죽는 순간 데스캠으로 넘어가고, 시체 옆에 선 이 몬스터가 거기 잡힌다): 피해자 PC 가 `IsSpectating()` 이
  되거나 `≥ ExecuteDuration + Rules.SpectateDelay` 면 `Break`. 처형 중 감각은 돌지 않는다(`bSensesAwake` 제외).
- **구조 없음** (D4): `OnDamage` 는 HP 를 깎고(죽으면 `Die`) `State == Execute` 면 경직·재타깃·상태 변경 없이
  돌아간다; `OnShove` 는 머리에서 그대로 돌아간다(넉백·경직·밀치기 소음 없음). `Die` 는 처형 중이고 피해자가 살아
  있으면 **먼저 `HostKill`** 한 뒤 `Dead` 로 — 피해자의 뷰 타깃은 캐릭터의 사망 흐름이 죽는 순간 데스캠(자기 폰)으로
  블렌드한다(`HostApplyDamage`, 2026-09-20 밤). `Dead` 진입에 처형 카메라가 꺼져도 죽은 몬스터 액터 시점은 남지 않는다
  (데스캠에 내 시체와 몬스터 시체가 같이 보인다).
- **타깃 제외** (D3): `SenseTick` 후보·Chase 의 `bCanCatch`(= Pounce 진입)·`HostPounceLunge` 재검사 모두
  `IsExecuted()` 면 제외/헛침 — 둘째 몬스터가 피해자를 넘어뜨려 넘어짐 무적으로 "구해" 주지 않는다.
- **처형 카메라** (D6): `ACL9Monster::ExecuteCamera`(`UCameraComponent`, 생성자에서 `bAutoActivate` false). `ApplyState`
  (호스트 `EnterState`·클라 `OnRep_State` 가 같이 온다)가 `State == Execute` 면 카메라를 붙여 켜고, 아니면 끈다.
  부착 기준은 둘이다 — `ExecuteCameraSocket`(본 이름 허용)이 메시에 **있으면 메시의 그 소켓 공간**, **없으면
  `GetRootComponent()`(구 콜리전)에 액터 공간**으로 붙고, 그때 `ExecuteCameraOffset` 은 "X = 전방(타깃 쪽)·Z = 위",
  `ExecuteCameraRotation` 도 액터 공간으로 읽힌다. 메시 루트가 아닌 이유: 메시는 `MeshYawOffset` 만큼 돌아 있고
  `Radius` 만큼 내려가 있어 메시 기준 오프셋은 종마다 축이 달라진다(2026-09-20 밤 돼지 "뒷다리 사이" 사고 —
  decisions.md). 현재 셀은 돼지·너구리 모두 소켓 `None`·오프셋 (−130, 0, 165)·회전 (−30, 0, 0). 클라의 메시 스무딩은 처형 동안 멈추고 액터 값에 붙는다(텔레포트를 미끄러뜨리지 않는다).
  뷰 타깃 전환은 캐릭터 쪽 `HostBeginExecuted`(서버 `SetViewTargetWithBlend(몬스터, ExecuteCameraBlend)`). 죽는 순간
  **데스캠**(폰의 `DeathCamera`)으로 되돌리는 것도 캐릭터 쪽(`HostApplyDamage` → `SetViewTargetWithBlend(폰, DeathCameraBlend)`,
  [character.md](character.md) 데스캠) — 옛 D6 문장 "뷰 타깃은 폰으로 돌아오지 않는다"는 2026-09-20 밤 번복됐다. 몬스터가
  시체 옆에 머무는 것과 처형 중 카메라는 그대로.
- **치트** `cl9.execute`: 가장 가까운 살아 있는 처형 종(`IsExecutioner()`)이 `HostCheatExecute` — 스토커는 계수기·
  거리·등 뒤·미발견·시야·KillCooldown 을 우회하고 타깃 상태 검사와 앵커 스윕만 남긴다; 잡히면 처형인 종
  (`bExecuteOnCatch ∧ ExecuteRadius 0`)은 타깃 상태 검사만 하고 앵커 스윕 없이 지금 자리에서 진입 ([cheats.md](cheats.md)).
- **로드 검사** (`Validate`, Error): 열 전부 유한·비음수; `ExecuteRadius > 0 ⇒ ExecuteDuration > 0 ∧ StalkSpeed > 0 ∧
  StalkGazeRadius > 0 ∧ ExecuteAnchorDistance ≥ Radius + 39`; `bExecuteOnCatch ⇒ ExecuteDuration > 0`;
  `StalkRetreatRadius > 0 ⇒ RetreatSpeed > 0`; `ExecuteCameraBlend` 유한·비음수.
- ABP(리드): `ABP_Raccoon_A` — `Stalk` 은 걷기 로코모션, `Execute` 가지는 `ABP_Pig_A` 와 같은 사양(평가기
  `AN_Raccoon_Catch`). `ABP_Pig_A` — `Execute` 는 `GetStateElapsed()` 평가기 가지(Run 1 은 `AN_Pig_Catch` 마지막
  프레임 유지, Run 2 에서 전용 클립); 돼지는 `Stalk` 에 들어가지 않으므로 그 가지는 쓰이지 않는다. **Run 2**(2026-09-21): `Execute`
  평가기 클립을 전용 2인 클립 `AN_Pig_Execute`/`AN_Raccoon_Execute` 로(길이 = `ExecuteDuration` 2.0, 피해자 쪽 `AN_Farmer_Executed_Pig`/
  `_Raccoon` 과 같은 앵커 표 — 돼지는 피해자 정면, 너구리는 피해자 등 뒤 `ExecuteAnchorDistance`; 피해자 요는 진입 때 캐릭터 쪽이 맞춘다).
  캐릭터 쪽은 [character.md](character.md) 처형.

### 타격의 표현 (2026-09-18)

판정 위에 얹은 표현 층이다 ([../goal/game-feel.md](../goal/game-feel.md) §1~§3) — 히트박스·경직·HP·드랍 표는
그대로고, 전부 **이미 복제되는 시계·상태에서 파생**하므로 새 RPC·멀티캐스트가 없다 (conventions § 9). 수치는
`CL9Rules`([../plan/data/rules.md](../plan/data/rules.md) 타격의 표현), 소리는 `SoundTable` 행(`CL9SoundNames`,
[sound.md](sound.md)), 퍼프는 `CL9Rules.HitPuffSystem`.

| 표현 | 원천(에지) | 어느 머신 | 어디서 | 무엇 |
|---|---|---|---|---|
| 타격 플래시 | Stun·Dead ∧ `GetStateElapsed() < HitFlashTime` (수준) | 모든 머신 | `TickHitFlash` | 빨간 오버레이 (2026-09-15 그대로) |
| `Hit` 소리 · 타격 퍼프 | 위 창의 **상승 에지**, 또는 창 안에서 `StateStartServerTime`이 바뀜(경직 중 다시 맞음) | 모든 머신 | `TickHitFeel` → `SpawnHitPuff` | 몬스터 메시 중심(`spine` 소켓, 없으면 `Bounds.Origin`)에서 소리 한 번 + 나이아가라 `User.Scale` 1.0 |
| `MonsterDeath` 소리 · 큰 퍼프 | 같은 에지인데 상태가 Dead | 모든 머신 | `TickHitFeel` | `Hit` **과 함께** `MonsterDeath`, 퍼프 `User.Scale` 3.0 |
| 몬스터 히트스톱 | Stun·Dead ∧ `GetStateElapsed() < HitStopTime` (수준) | 모든 머신 | `TickHitFeel` | `MeshComponent->bPauseAnims` 켬→끔. 이동·AI·히트박스는 흐른다 |
| 팔 히트스톱 | `SwingHitServerTime`(COND_OwnerOnly) 뒤 `HitStopTime` 동안 | 때린 사람 화면 | `ACL9Character::GetSwingAnimElapsed` | ABP 의 `SwingElapsed` 소스가 이 게터 — 명중 프레임에 머물다 이어서 돈다. `GetSwingElapsed`(판정)는 그대로 |
| `Swing` 소리 | `SwingStartServerTime` 변화 | 모든 머신 | `ACL9Character::UpdateHitSounds` | 폰 자리. 맞든 빗나가든 |
| `Hurt` 소리 | `KnockdownStartServerTime` 변화 | 모든 머신 | `ACL9Character::UpdateHitSounds` | 폰 자리 |
| `MonsterPounce` 소리 | **상태 전이**: `State == Pounce ∧ (지난 틱 State != Pounce ∨ StateStartServerTime 변화)` | 모든 머신 | `TickStateSounds` | 메시 중심. 창(elapsed)이 아니라 전이라 늦게 온 복제도 놓치지 않는다. 첫 틱은 기준값만(`bStateSoundPrimed`) |
| `MonsterCatch` 소리 | 같은 전이 감지, 상태가 Catch | 모든 머신 | `TickStateSounds` | 메시 중심. 잡힌 쪽 `Hurt`와 같은 순간에 겹친다(Concurrency는 에셋). 헛침엔 없다. `TickHitFeel`의 Stun·Dead 창과 별개라 Catch가 관측 전에 Stun·Dead로 덮이면(같은 틱 피격) 그 잡기 소리는 빠진다 — `Hurt`가 이미 나므로 허용 |
| `MonsterNear` 소리 | 근접 소나 사건 `monster_prox` (`HostTickProxNoise` → 파동) | 모든 머신 | `UCL9WaveRenderer::PlayWave` | 파동 행의 `SoundRow` 열 — 링과 같은 박자, 시각 게이트 앞이라 링이 안 그려져도 난다 ([wave.md](wave.md)) |
| `Place` 소리 | 아이템 `HoldState` Held·Worn → Free | 모든 머신 | `ACL9Item::ApplyHoldState` (호스트는 OnPlaced/OnThrown, 클라는 OnRep) | 아이템 자리. 수레 Pushed→Free·죽음 드랍은 아님 |
| `Footstep` 소리 | 수평 이동 거리 누적 ≥ 보폭 (`FootstepStrideRun/Walk`) | 모든 머신 | `ACL9Character::TickFootstepSound` · `ACL9Monster::TickFootstep` | 발밑, 피치 ±8 %. 폰은 속도 > `WalkSpeed`면 달리기 보폭, 몬스터는 Chase 면 달리기 보폭. 서 있으면·공중이면·시체면 안 난다. 클라의 몬스터는 보간된 메시 자리로 잰다 |
| 죽음 발사 | `Die(Source)` | 호스트 (트랜스폼 복제) | `HostTickDeathLaunch` (`HostTickAI` 의 Dead 거름 앞) | 원점→몬스터 수평 `DeathLaunchSpeed` + 위 `DeathLaunchUp`, 월드 중력. 몸체 구 스윕·WorldStatic 오브젝트 질의는 바닥을 지나쳤다(2026-09-18 PIE — `Floor_0` 은 BlockAllDynamic = WorldDynamic) → **Visibility 채널** 선 트레이스 `TraceWorld`(위 "세상 트레이스" — 폰·아이템·몬스터에 맞으면 무시하고 뒤를 다시 긋는다)로 검사한다 — 수평 선분이 막히면 그 자리에서 수평 속도 0, 발밑(중심 − Radius)까지 내려 그은 선이 닿으면 발을 바닥에 스냅하고 끝. 죽는 클립은 그대로 |
| 드랍 호 | `Die` 의 `SpawnItem` 뒤 | 호스트 | `Die` → `ACL9Item::OnThrown` | 몬스터 중심 높이·옆 30~90cm 무작위 자리에서 `DeathLootSpeed`, 앙각 45~70°·요 무작위. 옆 오프셋은 둘 이상이 30cm 이상 떨어지게 하는 보장(호만으로는 3/4 확률). 이후는 아이템 물리 |

에지 검출은 첫 틱에 기준값만 받아 적는다(`bHitFeelPrimed`·`bHitSoundsPrimed`) — 늦게 들어온 클라가 이미 지난
타격·휘두르기를 되풀이하지 않는다. 표·행·애셋이 비면 소리·퍼프는 조용히 빠지고 나머지는 그대로 난다. 전역 시간
지연·`CustomTimeDilation`은 쓰지 않는다(리슨 호스트에서 남에게 번지고 이동 예측을 깨뜨린다).

## 경보 (무리 반응)

- **무리 경보**: 발각한 개체가 `PackAlarmRadius` 안 동료를 깨운다.
  `bAlertsOthers`는 거위만 true — 거위는 울어서(`AlertWindup` 예비동작 뒤) 넓게 알리는 경보꾼이다.
- **사살 경보**: 죽는 소리가 `KillAlarmRadius`(2100)로 퍼진다 — 층/존을 넘을지는 맵의 존
  세팅과 함께 정한다 (지하 사살이 1층을 깨우지 않게 하려면 "같은 존인가" 검사를 둔다).
- **복수**: 죽는 소리를 들은 개체는 `RageTime` 사냥 모드, `RevengeRadius`로 전파.

## 일자 스케일링

`DangerCurve`(progression.md)가 **추격 예산·발각 속도·시야**에 곱해진다 ([balance.md](balance.md)).
같은 마릿수로 압박이 오르는 축이다. 값 자체를 표에서 바꾸지 않고 곡선이 배율만 준다.

구현 (4b, 2026-09-05): 배율은 GameState `DangerScale`(밤 시작에 `Site.BaseDanger × DangerCurve`,
로비 1) 하나이고, `ACL9Monster::GetDangerScale()`이 세 읽기 지점에 곱한다 — `ChaseBudget`(추격
진입 시 예산), `DetectRateNear/Far`(게이지 상승), `SightIn/SightOut`(시야 거리). 로드 검사 A1/A2는
배율 없는 표 값으로 돈다.

## 종별 차이를 다루는 규칙

1. **1순위 — 표의 값.** 닭과 거위의 차이 대부분은 같은 컬럼의 다른 값이다
   ([chicken.md](../plan/data/monster/chicken.md) · [goose.md](../plan/data/monster/goose.md)).
2. **2순위 — 상태 사용 여부.** 안 쓰는 상태는 값으로 끈다 (예: 잠듦이 없는 몬스터는
   깨어 있는 채로 시작).
3. **3순위 — 기믹 (Instanced).** 값·상태로 표현이 안 되는 특수 동작만.
   **판정 기믹은 표의 값으로 두고 호스트 판정 코드가 읽는다** — 기믹이 액터 상태를 직접
   바꾸면 클라와 갈라진다 (plan/build-order.md 원칙 4).
4. 종별 파생 C++ 클래스를 만들지 않는 게 기본이다 — `ACL9Monster` 하나로 시작하고,
   틀이 감당 못 하는 게 실제로 나타나면 그때 결정한다.

## 네트워크 정리

| 것 | 방식 |
|---|---|
| AI 틱·감각·상태 전이 | 호스트 전용 (`HasAuthority` 관문) |
| 상태(`ECL9MonsterState`)·HP·발각 게이지 | Replicated + OnRep (애님·소리·표시는 표현 계층) |
| 위치 | 표준 액터 복제 20Hz + 클라 보간 |
| WakeTelegraph·AlertWindup 예고 | Replicated 상태의 표현 — 전 플레이어에게 보인다 |
| 타격 플래시(빨간 오버레이) | 복제 없음 — 각 머신이 `State`+`StateStartServerTime`에서 파생 |
| 예고 Pounce·덮침 결과 | `State`(Pounce) + `PounceResultServerTime`·`bPounceHit` Replicated, 진입·결과 직후 `ForceNetUpdate` |
| 예고·잡기 소리 | 복제 없음 — 각 머신이 상태 전이를 감지 (`Execute` 진입도 `MonsterCatch`) |
| 스토킹 계수기·타깃·앵커 스윕 | 호스트 전용 — 복제 없음 |
| 처형 Execute | `State`(Execute) + `StateStartServerTime`(기존) — 진입 직후 `ForceNetUpdate`; 스토커의 텔레포트는 표준 이동 복제, 클라 스무딩은 처형 동안 정지 |
| 처형 카메라 | 복제 없음 — 각 머신이 `OnRep_State`/`EnterState` 에서 켜고 끈다. 피해자 뷰 타깃은 서버 `SetViewTarget`(관전 경로); 죽는 순간 폰(데스캠)으로 블렌드 |

## 구현 순서 (build-order ③과 맞물림)

| 스텝 | 이 문서에서 만드는 것 |
|---|---|
| 3b | `UCL9MonsterDef` 표 + `ACL9Monster` 스폰 틀 (SetAlias → LoadData) |
| 3c | 이동·순찰 상태 (경로 탐색 방식 확정 포함) |
| 3d | 감각 파이프라인 + 조사·추격·포기·복귀 |
| 3e | 발각 게이지 + 잡기 |
| 3f | 무리 경보·사살 경보 (같은 존 검사 포함) |
| 3g | 기믹 층 + 거위·너구리 행 추가 → **틀 검증 완료 기준** |

## 미결

- 경로 탐색 방식(NavMesh vs 커스텀 그래프) — 맵 생성기([map.md](map.md))와 함께 정한다
- 너구리 값 전부 — ③에서 행을 채우며 정한다
- `ECL9MonsterState` 최종 목록 — 위 상태도가 초안이다

## 참고

- 모든 컬럼과 값: [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 소음 반경·ThreatActMinRadius: [../plan/data/rules.md](../plan/data/rules.md)
- 스폰 개수·시점: [raiding.md](raiding.md) · [balance.md](balance.md)
- 드롭: [drop.md](drop.md)
