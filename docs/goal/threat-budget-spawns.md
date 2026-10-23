# goal: threat-budget-spawns

> **상태**: 구현·검증 끝, 봇 첫 정산 실행은 못 끝냄 (2026-10-20). 사용자 "임시값 하지 말고 제대로 세팅해 봐 — 게임 한 번 굴려 보는 게 이번 목표" → 범위 질문에 **A4 핵심 구현**, 검증은 **봇으로 첫 정산까지**.
> 같은 날 앞 단계 [C:\Work\CL9\docs\goal\henhouse-spawn-ramp.md](C:\Work\CL9\docs\goal\henhouse-spawn-ramp.md)(사이트 `MidSpawns` 큐)를 대체한다.

## 한 줄 목표

결정 #38(A4, 2026-10-18 "리썰식·세 줄·두 축·쓴 것 기준")의 핵심을 지금 있는 6종으로 넣는다 — 가축 줄은 판 시작에 마릿수로, 실내 위협은 깊이 구역마다 파워 예산 안에서 희귀도로 뽑아 판이 흐르는 동안 들어온다.

## 결정

1. **표 열** (`C:\Work\CL9\docs\plan\draft\monster-structure.md` 3.2 의 이름을 그대로 쓴다)
   - `MonsterTable`: `SpawnPower`(위협 예산에서 빼는 값), `SpawnMaxCount`(한 판에 위협으로 나올 수 있는 수 — 리썰 `MaxCount` 처럼 나온 수로 센다, 0 = 상한 없음).
   - `MapTable.Zones[]`: `MonsterSpawns` = 가축 줄(지금 구조), `ThreatPool`(`{Alias, Rarity}`), `ThreatBudget`(5일차 기준), `ThreatKindsMax`, `ThreatStartShare`.
   - `CL9Progression.ThreatBudgetCurve`: 예산 일자 배율. 가축 수 곡선(`MonsterCountCurve`)과 따로 간다.
   - `CL9Rules`: `MidSpawnInterval`(뽑아 보는 간격) 15 · `MidSpawnFirstDelay` 40 · `MidSpawnRampTime` 160 · `MidSpawnMinDistance` 1500 그대로.
     7e 의 `MidSpawnCapRatio` 와 같은 날 앞 단계의 `MidSpawnIntervalMin`·`MidSpawnMinDepth`·사이트 `MidSpawns` 는 지웠다.
2. **뽑는 순서** (3.3 을 따름) — 구역 예산 = `ThreatBudget` × `ThreatBudgetCurve`. 판 시작에 그 가운데 `ThreatStartShare` 몫을 희귀도 가중으로 뽑아
   구역 띠의 `Monster` 지점에 놓는다. 남은 몫은 판 중간에 시간을 따라 풀린다 — 시각 t 의 몫 = 판 중간 예산 × (t − 40초) / 160초(1에서 멈춤).
   15초마다 쓴 것이 그 몫보다 적은 구역 하나에서 하나를 뽑아, 모든 산 사람에게 안 보이고 15 m 넘게 떨어진 그 구역 띠의 지점에 세운다.
   후보 = 희귀도 > 0 · 파워 ≤ 남은 예산 · 이 판에 나온 수 < `SpawnMaxCount` · (구역에 이미 나온 종이거나 종 수 < `ThreatKindsMax`).
   예산은 쓴 것 기준 — 죽여도 돌아오지 않는다.
3. **줄 배정** — 초안 2.2 는 돼지·소를 가축 줄, 양을 실외 위협으로 둔다. 이번에는 다르게 둔다:
   - 양계장·대농장에서 가축 줄은 그 집 주인인 **닭만**이다. 돼지·소·거위·너구리·양은 실내 위협 풀이다. 2026-10-20 "종마다 핵심 행동 하나" 이후 다섯 종 모두
     규칙 하나를 들고 나오는 종이고(돼지 = 짐 든 사람 처형, 소 = 안 볼 때 다가옴), 그래야 "첫 판에는 규칙 하나"를 예산으로 지킨다.
   - 돼지 농장은 주인인 돼지가 가축 줄이다(닭도 바깥 구역에 섞인다).
   - **양은 실내 위협 풀에 둔다** — 실외 위협 풀은 이번 범위가 아니고, 마당에는 몬스터가 없다(7b, 2026-10-12 사용자). 실외 풀이 들어오면 옮긴다.
4. **이번에 넣지 않은 것** — 실외 위협 풀, 구조물 조건(`RequiredFeatures`), 무리 크기(`GroupSize*`), 사람 수 조건(`MinPlayers`), 판 중간 등장의 구멍(`EmergeFrom`)과
   예고음(A9 "진짜 하나 + 가짜 둘"). 판 중간 등장은 지금처럼 예고 없이 안 보이는 지점에서다.

## 값 (근거)

**종** — 리썰 실측 종류표(`C:\Work\CL9\docs\reference\lethal\lethal-monsters.md` 598~632줄)에서 같은 역할의 `PowerLevel`·`MaxCount`.

| 종 | `SpawnPower` | `SpawnMaxCount` | 리썰 짝 |
|---|---|---|---|
| 거위 | 1 | 2 | Hoarding bug 1.0 — 해치지 않고 소란 |
| 양 | 1 | 3 | Centipede(Snare flea) 1.0 — 혼자인 사람만 |
| 소 | 1 | 2 | Spring(Coil-head) 1.0 — 안 볼 때만 다가옴 |
| 돼지 | 2 | 2 | Bunker spider·Butler 2.0 — 쫓아 와 처형 |
| 너구리 | 3 | 1 | Flowerman(Bracken) 3.0 · MaxCount 1 — 등 뒤 처형 |

**양계장 구역** — 예산 합 4.0 = 리썰 첫 달 Experimentation 실내 예산 4(`C:\Work\CL9\docs\reference\lethal\lethal-map.md` 203줄). 입구는 비운다.

| 구역 | 가축 줄 | 위협 풀(희귀도) | 예산 | 종 수 | 판 시작 몫 |
|---|---|---|---|---|---|
| `entrance` 0~0.30 | — | — | 0 | 0 | — |
| `middle` 0.30~0.65 | 닭 1~2 | 거위 40 · 소 30 · 양 30 | 1.5 | 1 | 0 |
| `deep` 0.65~1.00 | 닭 2~3 | 돼지 40 · 너구리 25 · 소 20 · 거위 15 | 2.5 | 2 | 0 |

**예산 곡선** `ThreatBudgetCurve`: (1, 0.5) (5, 1.0) (10, 1.75) (20, 2.5). 그래서 양계장은
1일차 가운데 0.75(없음)·깊은 곳 1.25(파워 1 하나), 3일차 가운데 1.1·깊은 곳 1.9(합쳐 둘), 5일차 4.0, 10일차 7.0 이다.
가축 닭은 `MonsterCountCurve`(1일차 0.6)를 타서 1일차 판 시작에 두세 마리다.

**대농장·돼지 농장** — 명세 `C:\Work\CL9\Tools\Unreal\stages\farm.json` · `pigfarm.json` 의 zones 가 정본이다. 대농장 예산 합 9.5(리썰 Vow 7 ~ March 14 사이), 돼지 농장 7.0.

## 수용 기준

- [x] `CL9Editor` 빌드(브로커, 40초).
- [x] `CL9.Night` 테스트 3개(새 `CL9.Night.ThreatBudget` 포함) 통과.
- [x] PIE 양계장 1일차(네 번): 판 시작에 닭 2마리만, 가운데 구역 예산 0.75 는 아무것도 못 사고, 깊은 구역 1.25 에서 55초에 거위나 소 하나(open 9%).
- [ ] 봇으로 1~3일차를 돌려 첫 정산 판정까지 간다 — **못 끝냈다.** 샷 목록 `C:\Work\CL9\Saved\Film\shots\first_settlement.json` 을 네 번 돌렸고
      판마다 맵이 달라 봇이 다른 곳에서 멈췄다: ① 창구를 조준하지 못함(바닥 원점을 봄 → 고침) ② 조리대 몸체 앞에서 막힘(→ 목표 가까이서 막히면 도착으로 고침)
      ③ 조리대 앞에서 익기를 기다리다 닭에게 세 번 잡혀 1일차에 죽음(→ 기다리는 동안 휘두르는 `guard` 를 넣었지만 아직 검증 못 함) ④ 창구로 가다 어두운 곳에서 막힘.
      ③에서 2일차로는 넘어갔다(전멸 벌금·주문 위약금·2일차 로비·저장 로그 확인).

## 조절 손잡이

- 얼마나 위험한가 → `MapTable.<사이트>.Zones[].ThreatBudget`, 날마다 얼마나 늘어나나 → `CL9Progression.ThreatBudgetCurve`.
- 무엇이 자주 나오나 → `Zones[].ThreatPool` 희귀도, 한 구역에 몇 종까지 → `ThreatKindsMax`.
- 판 시작부터 있게 → `ThreatStartShare`, 언제부터 얼마나 빨리 → `CL9Rules.MidSpawnFirstDelay` · `MidSpawnRampTime` · `MidSpawnInterval`.
- 종 하나의 무게 → `MonsterTable.SpawnPower` · `SpawnMaxCount`.
