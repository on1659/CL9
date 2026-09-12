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

## 감각 파이프라인 (공통, 호스트 틱)

세 입력이 하나의 **발각 게이지**로 모인다:

1. **시야** — 히스테리시스: 발견은 `SightIn`, 놓침은 `SightOut`. 정전 시 `SightDark`.
   시야에서 벗어나도 `DetectGrace` 유예. 벽 차단 판정 방식은 경로 탐색 선택과 함께 정한다.
2. **청각** — 소리 반경은 몬스터가 아니라 **행동이 정한다** (`DA_CL9Rules`:
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
  `HurtStun` 비틀) + 넘어짐 + **짐 흘림(손+품 전부 — 초안)**. 죽음·즉사 여부는 HP와
  타격량 값이 정한다 — 원칙이 아니다.
- 잡은 뒤 `KillCooldown` 재장전 — 연속 타격으로 3타를 순식간에 넣지 못하게 하는 장치.
  **`KillCooldown` = `RageTime` 등식은 로드 검사가 확인한다.**
- 플레이어의 대응: 도망(속도 부등식 A1·A4가 탈출을 보증), 든 아이템 휘두르기(좌클 —
  행의 `HitDamage`·`HitStun`이 `OnStrike`로 들어온다: 프라이팬 = `Push`(`PushStun` + `PushKnockback`),
  고기 = `Hit`(`HitStunMs`); 2026-09-12, [../plan/data/item/common.md](../plan/data/item/common.md) 사용).

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
