# 거위 — `goose`

**경보꾼.** 잡는 게 아니라 울어서 무리를 깨운다. 좁은 길목에 서면 그 길이 막힌다.

컬럼 정의는 [common.md](common.md).

---

## 공통 컬럼 값 (닭과 다른 것만)

| 컬럼 | 값 | 구 CCL8 상수 |
|---|---|---|
| `ChaseSpeed` | 528 | `GooseSpeedU 176` |
| `RetreatSpeed` | 270 | `GooseBackSpeedU 90` |
| `Radius` | 45 | `GooseRadiusU 15` |
| `HomeRange` | 2280 | `GooseLeashU 760` |
| `bAlertsOthers` | **true** — 지금은 거위만 | |

## 거위 전용 컬럼 (잠·경보꾼)

자는 행동부터 일반 몬스터와 다른 종이라 여기서 정의한다. 코드에는 공통 필드로 존재하고
(안 쓰는 종은 0/false), **다른 종이 쓰게 되면 [common.md](common.md)로 승격한다.**

| 컬럼 | 뜻 | 값 | 구 CCL8 상수 |
|---|---|---|---|
| `WakeRadius` | 이 안은 걸어도 깬다 (cm) — 걷기 무소음 규칙의 예외 | 450 | `GooseWakeRadiusU 150` |
| `HearRunRadius` | 이 안에서 달리면 깬다 (cm) — 잠든 상태의 원거리 청각 | 1260 | `GooseHearRadiusU 420` |
| `AlertWindup` | 경보 울기 전 예비동작 (초) — 구 `HonkWindup` | 0.55 | `GooseHonkT` |
| `SleepTime` | 다시 잠들기까지 (초) | 4.0 | `GooseSleepT` |
| `AlertCooldown` | 다시 울기까지 (초) | 2.5 | `GooseAlertCd` |

## 규칙

- **거위는 잡는 몬스터가 아니라 경보꾼이다** (초안 — `CatchRadius` 0). 추격은 잡기가 아니라
  쫓아내기라서, 부등식 A1("지친 빈손 > 잡는 몬스터의 추격")의 대상이 아니다 —
  ChaseSpeed 528 > 506이어도 하드락이 아니다 ([../../economy.md](../../economy.md)).
- **기본 스폰은 0~1마리다** (양계장 기준값 Min 0 / Max 1 — [../site.md](../site.md)).
  배치될 때는 좁은 길목을 막지 않는지 확인해야 한다 — 요충지 술어를 통과해야 한다.
- 거위 잠듦은 미끼(`DecoyRadius`)에 연결된 2건 중 하나다 (나머지는 탄 요리).
- `WakeRadius` 450 안에서는 걷기 무소음이 안 통한다 — 지금은 이 값이 0이 아닌 유일한 종이다.
