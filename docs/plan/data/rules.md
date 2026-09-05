# 규칙 — `DA_CL9Rules`

**게임에 하나뿐인 에셋이다.** 소리 · 죽음 처리 · 밤 시계 · 운반 마찰 —
캐릭터마다 다를 수 없는 값이 여기 있다.

몸 수치(속도·캡슐·스태미나·체력)는 [character.md](character.md)에 있다.

**점프의 판정 역할은 딱 하나 — 낮은 장애물 통과다** ([segkind.md](segkind.md) 점프 통과 셋).
거리·소음·감각 판정은 전부 평면(2D) 기준이다 — 3D 거리로 재면 점프 스팸이 회피 기술이 된다.

---

## 소리

발소리와 행동이 내는 소음 반경. 몬스터가 이 반경 안에 있으면 듣는다.

| 컬럼 | 기본값 | 뜻 | 구 CCL8 상수 |
|---|---|---|---|
| `RunNoiseRadius` | 390 | 달릴 때 (cm) | `AggroRunU 130` |
| `PickupQuietRadius` | 150 | 걸으면서 주울 때 (cm) | `PickupQuietU 50` |
| `PickupLoudRadius` | 390 | 뛰다가 주울 때 (cm) | `PickupLoudU 130` |
| `PushNoiseRadius` | 255 | 밀칠 때 (cm) | `PushNoiseU 85` |
| `DecoyRadius` | 330 | 미끼 (cm) | `DecoyRadiusU 110` |
| `RecentRunTime` | 1.0 | 이 안에 달렸으면 줍기가 시끄럽다 (초) | `RecentRunT` |

**걷기는 어그로 0이다** — 걸으면 안 들킨다는 게 기본 안전 수단이다 (예외: 거위의 `WakeRadius`).

미끼(`DecoyRadius`)에 연결된 시스템은 현재 2건이다 (거위 잠듦, 탄 요리).

---

## 죽음과 회복

| 컬럼 | 기본값 | 뜻 | 구 CCL8 상수 |
|---|---|---|---|
| `SpectateDelay` | 미정 | 죽고 관전으로 넘어가기까지 (초) | **신규** |
| `TruckPadHealTime` | 미정 | 트럭 패드 회복 시간 (초) | **신규** |
| `LeaverGraceTime` | 3.0 | 이탈 후 폰이 남는 시간 (초) | `AbandonT` |
| `ThreatActMinRadius` | 420 | 이 안에서는 위협이 안 깨어난다 (cm) | `ThreatActMinR 140` |

부활은 없다 (현재 설계) — 회복 채널은 트럭 패드다.
완충타 비틀(`HurtStun`)은 캐릭터별 값이라 [character.md](character.md) 체력 표에 있다.

**`ThreatActMinRadius` 안에서는 위협이 깨어나거나 스폰되지 않는다.**

---

## 밤

| 컬럼 | 기본값 | 뜻 | 구 CCL8 상수 |
|---|---|---|---|
| `NightLength` | 245 | 밤 길이 (초). 만료 = 새벽 실패 | `NightLenSec` |
| `DarkBase` | 185 | 정전까지 기본 시간 (초) | `DarkBaseSec` |
| `DarkAccel` | 4.0 | 반납 1회가 앞당기는 시간 (초) | `DarkAccSec` |
| `DarkMin` | 125 | 정전 최소 시각 (초) | `DarkMinSec` |
| `DawnLength` | 미정 | 새벽 화면 길이 (초) — "정산"은 이틀 주기 판정만 가리킨다 | **신규** |

**반납이 시계를 앞당긴다** — `185 - 15×4 = 125`. 15회 반납하면 정전이 바닥까지 온다.

**하룻밤 안의 시간 실패는 밤 만료다** (현재 설계 — 별도 페이즈 시계 없음).
그 밖의 실패 축은 **전멸**(전원이 사망하면 트럭이 일행을 두고 떠난다)과
**정산 미달**([progression.md](progression.md))이다.

---

## 던지기와 운반

| 컬럼 | 기본값 | 뜻 | 구 CCL8 상수 |
|---|---|---|---|
| `SwapTime` | 1.2 | 손↔품 교체 잠금 (초) | `SwapT` |
| `ShakeFullTime` | 3.0 | 달리기 흔들림 만충까지 (초) | `ShakeFullT` |
| `ShakeDrainTime` | 1.5 | 흔들림 회복 (초) | `ShakeDrainT` |
| `ThrowMinSpeed` | 600 | 차징 0일 때 (cm/s) | `ThrowMinU 200` |
| `ThrowMaxSpeed` | 1680 | 만충일 때 (cm/s) | `ThrowMaxU 560` |
| `ThrowChargeTime` | 0.85 | 만충까지 (초) | `ThrowChargeT` |
| `CarryHeight` | 120 | 손 높이 (cm) | `CarryHeightU 40` |
| `PickRadius` | 120 | 줍기 사거리 (cm) | `PickRadiusU 40` |

`SwapTime` 1.2초는 의도된 값이다 — 운반 마찰이 이 게임의 축이다.

운반 흔들림은 바 없이 화면 흔들림 진폭으로 읽힌다 (게이지를 띄우면 "관리하는 자원"이 되어
긴장이 사라진다는 판단). 만충의 효과: 손의 취약물이 금 간다 (초안 —
[../../systems/character.md](../../systems/character.md) 흔들림 절).

**스태미나는 예외 — 화면 흔들림과 HUD 바를 함께 쓴다** (2026-09-05 번복,
[../../decisions.md](../../decisions.md)). 흔들림만으로는 남은 양과 탈진 래치 해제 시점이
안 읽힌다는 판단이다. 운반 흔들림에는 이 예외가 적용되지 않는다.

## 스태미나 표시 (카메라 셰이크)

스태미나도 바 없이 **카메라 셰이크**로 알린다 — 지치면 화면이 떨리고, 바닥날수록 세진다
(HUD 바 없음 — 위 운반 흔들림과 같은 철학, 다른 대상). 소유 클라 로컬 표현이고 판정·복제와
무관하다. 진폭·주기는 셰이크 에셋이 가지고, 표는 어느 에셋을 언제부터 켜는지만 정한다.

**셰이크는 규칙 컬럼이 아니라 셰이크 표에서 읽는다** (2026-09-05 결정 — 앞으로 흔들림이
늘 때 규칙에 컬럼을 늘리지 않고 표에 행만 추가한다). `UCL9ShakeTable`(`DA_ShakeTable`,
`UCL9UITable`과 같은 모양)의 `Rows`에 이름→`FCL9ShakeInfo{ Shake, StartAt }`로 등록하고,
캐릭터가 `CL9ShakeNames::Stamina` 행을 조회한다. 스케일 = (StartAt−Stamina)/StartAt, 0에서 만충.

| 행 (`Rows` 키) | 필드 | 뜻 |
|---|---|---|
| `Stamina` | `Shake` | 지쳤을 때 재생할 카메라 셰이크 (`UCameraShakeBase` 소프트 클래스) |
| `Stamina` | `StartAt` (0.5) | 정규화 스태미나가 이 값 아래부터 흔들림 |

**던지기 원점에 점프 높이를 더하지 않는다.**

---

## 미결

- `SpectateDelay` · `TruckPadHealTime` · `DawnLength` — 플레이해보고 정한다
