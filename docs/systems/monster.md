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
 └── 복귀 Return ←── 포기 Break ←── 추격 Chase ──→ 잡기 Catch (CatchRadius)
     (집으로)      (ChaseBreakTime      │  ChaseBudget 소진 /
                    두리번 →            │  ChaseLoseTime 시야 상실 /
                    ChaseCooldown)      │  HomeRange 이탈 /
                                        │  DeaggroRadius (짐 전부 내려놓음)
                                        ↓
                          경직 Stun (HitStunMs / PushStun) — 어느 상태에서든 진입
                          복수 Revenge / 격노 Rage — 사살 소리·피격에 반응
                          죽음 Dead — HP 0, 죽는 모션 (연출은 종별 선택)
```

- 상태는 enum 하나 (`ECL9MonsterState`) — Replicated, OnRep에서 애니메이션·소리 전환.
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
- 플레이어의 대응: 도망(속도 부등식 A1·A4가 탈출을 보증), 든 아이템 휘두르기(좌클 —
  행의 `HitDamage`·`HitStun`이 `OnStrike`로 들어온다: 프라이팬 = `Push`(`PushStun` + `PushKnockback`),
  고기 = `Hit`(`HitStunMs`); 2026-09-12, [../plan/data/item/common.md](../plan/data/item/common.md) 사용).
  **맞느냐 마느냐는 휘두르기 히트박스가 정한다** (2026-09-15 — 옛 "뗀 순간 200cm 부채꼴의 가장 가까운 하나" 폐기):
  좌클을 뗀 뒤 `CL9Rules.SwingHitStart`~`SwingHitEnd` 구간 동안 든 물건의 메시 경계 구가 손을 따라 지나가고,
  그 자리에 닿은 몬스터가 **한 번의 휘두르기에 각각 한 번씩** `OnStrike`를 받는다 — 나란히 선 둘은 한 번에 둘 다
  맞고, 빗나가면 아무도 맞지 않는다. 판정은 호스트 전용이고 `OnStrike`의 의미(피해 먼저, 죽으면 경직 없음,
  `Push`는 `OnShove`로 덮음)는 그대로다 ([inventory.md](inventory.md) 휘두르기 히트박스).

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
| `Place` 소리 | 아이템 `HoldState` Held·Worn → Free | 모든 머신 | `ACL9Item::ApplyHoldState` (호스트는 OnPlaced/OnThrown, 클라는 OnRep) | 아이템 자리. 수레 Pushed→Free·죽음 드랍은 아님 |
| `Footstep` 소리 | 수평 이동 거리 누적 ≥ 보폭 (`FootstepStrideRun/Walk`) | 모든 머신 | `ACL9Character::TickFootstepSound` · `ACL9Monster::TickFootstep` | 발밑, 피치 ±8 %. 폰은 속도 > `WalkSpeed`면 달리기 보폭, 몬스터는 Chase 면 달리기 보폭. 서 있으면·공중이면·시체면 안 난다. 클라의 몬스터는 보간된 메시 자리로 잰다 |
| 죽음 발사 | `Die(Source)` | 호스트 (트랜스폼 복제) | `HostTickDeathLaunch` (`HostTickAI` 의 Dead 거름 앞) | 원점→몬스터 수평 `DeathLaunchSpeed` + 위 `DeathLaunchUp`, 월드 중력. 몸체 구 스윕·WorldStatic 오브젝트 질의는 바닥을 지나쳤다(2026-09-18 PIE — `Floor_0` 은 BlockAllDynamic = WorldDynamic) → **Visibility 채널** 선 트레이스로 검사한다(폰·아이템·몬스터에 맞으면 무시하고 뒤를 다시 긋는다) — 수평 선분이 막히면 그 자리에서 수평 속도 0, 발밑(중심 − Radius)까지 내려 그은 선이 닿으면 발을 바닥에 스냅하고 끝. 죽는 클립은 그대로 |
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
