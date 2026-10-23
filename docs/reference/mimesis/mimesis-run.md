# 참고 — MIMESIS의 한 판: 정비소·트램·던전·오염도·데스매치

> 2026-10-17. 설치본(Steam buildid 25381161, `BuildVersion` "0.3.2", Unity 6000.3.9f1, Mono)의 `Assembly-CSharp.dll` 디컴파일(MCP `unity-analysis`)과 `MIMESIS_Data\StreamingAssets\masterdata\*.json`을 읽었다.
> **에셋을 가져오지 않는다.** 이 문서는 인계 질문 6(한 판 루프)을 짧게 다룬다. 몬스터는 [C:\Work\CL9\docs\reference\mimesis\mimesis-monsters.md](C:\Work\CL9\docs\reference\mimesis\mimesis-monsters.md),
> 도구·공포 표현은 [C:\Work\CL9\docs\reference\mimesis\mimesis-items.md](C:\Work\CL9\docs\reference\mimesis\mimesis-items.md). 시간 단위는 표의 ms를 초로 바꿔 적었다.

## 결론

1. **루프는 "정비소 → 트램(목적지 선택) → 던전(수집) → 정비소(판매·수리)"이고, 수리비를 못 내면 데스매치로 끝난다.** 스테이지(수리 주기)마다 던전을 최대 5번 돌 수 있고("정비소 도착까지 N일"), 수리비 할당량은 184 → 295 → 442 → 619 → 805 → 1000으로 오른다(`Stage.json`).
2. **시간 압박은 셋이다 — 게임 시계, 오염도, 몬스터 주기.** 던전은 09:00에 시작해 00:00에 끝나고(게임 시간 ×75, 실시간 약 12분), 트램은 24:00에 자동 출발한다. 오염도는 서 있어도 오르며 가득 차면 죽어 미메시스가 된다. 몬스터·미메시스는 60~120초마다 추가 스폰되고 2~6분 뒤 플레이어 쪽으로 걸어온다.
3. **죽음은 판 안에서 되돌릴 수 없고 관전과 빙의가 그 자리다.** 죽으면 관전자 채널로 넘어가 생존자 음성은 들리지만 말은 못 하고, 실험실 기능 "빙의"로 동료 10 m 안의 미메시스를 12초 빌려 마이크로 말할 수 있다. 정비소에 돌아가면 HP 50으로 부활한다. 전원 사망이면 던전이 즉시 끝난다.
4. **난이도 곡선은 표 셋으로 돈다.** 위험도(1~5)가 몬스터 예산·주기·미메시스 수·alpha 비율을, 오염 등급(날짜 1~5)이 오염도 상승 속도와 날씨 확률을, 보상 등급(1~5)이 스크랩 가치(369~392 → 1550~1645)를 정한다. 스테이지 2부터 20%로 잭팟 던전이 열린다.

---

## 1. 흐름

| 단계 | 무엇을 하나 | 값 | 근거 |
|---|---|---|---|
| 정비소(`MaintenanceRoom`) | 스크랩 판매(`STRING_GUIDE_TRAM_BEFORE_PAY` "스크랩을 판매하세요 / 정비소에 전력을 공급하세요"), 자판기·강화기 구매("탕진하세요!"), 트램 수리비 지불, 레버로 출발. 남은 돈은 출발 시 사라진다 | 초기 자금 120(`INITIAL_MONEY`). 전원 탑승해야 출발 | `MaintenanceRoom.cs`, 로컬라이즈 606·607·215 |
| 트램 대기(`InTramWaitingScene`) | 목적지 선택 — 후보 던전을 화살표로 넘긴다("오염된 공장"·"지하철역"·"저택" 등 A·B·C 3계열 × 공장/창고·역/터널·저택/감옥). 24:00 자동 출발 | 후보는 `DungeonPick.json`(스테이지 그룹 1~4 × 주기 1~5) | `MapButtonLevelObject.cs:60`, `TRAM_AUTO_START` |
| 던전(`DungeonRoom`) | 스크랩 수집("돈이 될 만한 것을 모으세요"), 트램으로 복귀. 실외(DunGen 조립 건물 밖)와 실내 | 09:00~00:00, 실시간 약 12분(`GAME_TIME_SCALE_FACTOR` 75000 = ×75) | `DungeonRoom.cs:869`, `Dungeon.json` start/end |
| 복귀 | 트램 레버 → 정비소. 사망자 부활 HP 50, 오염도 −2000, HP 25% 회복 | `MIN_HP_REVIVE` 50, `CONTA_RECOVERY_VALUE` 2000, `HP_RECOVERY_RATE` 2500 | `IVroom.cs:1625-1640` |
| 수리 판정 | 스테이지의 마지막 주기 뒤 수리비를 내면 다음 스테이지, 못 내면 **데스매치** | 할당량 아래 표 | `MaintenanceRoom.cs:257-278, 337` |
| 데스매치(`DeathMatchRoom`) | 오염 등급 999: 초당 60씩 올라 약 167초 안에 전원 미메시스가 된다. 미메시스 endgame(가짜 쇠지렛대) + 몬스터 4, 아이템 8. "생존 실패…" | `ContaData.json` id 6, `Const.json` DEATHMATCH_* | |

### 1.1 스테이지와 주기

| 스테이지 | 수리비 할당량 | 정비소 씬 |
|---|---|---|
| 1 | 0 | bg_Maintenance_a |
| 2 | 184~196 | bg_Maintenance_b |
| 3 | 295~313 | Maintenance_b_a |
| 4 | 442~470 | bg_Maintenance_c |
| 5 | 619~657 | |
| 6 | 805~855 | |
| 7 | 1000 | |

주기(날짜) 1~5마다 고를 수 있는 던전 후보와 확률이 다르다(`DungeonPick.json` 4그룹 × 5주기). 오염 등급은 날짜와 같다(`ExcelDataManager.cs:831` `GetContaGradeByDayCount`, 1~5 클램프).

### 1.2 던전 한 행의 뜻 (`Dungeon.json`, 예: 101 "A_FACTORY_LV1")

| 열 | 값 (위험도 1 / 5) | 뜻 |
|---|---|---|
| `hazard_level` | 1 / 5 | 몬스터·미메시스 표 선택 |
| `threat_min/max` | 171~189 / 647~716 | 일반 몬스터 위협 예산 |
| `normal_monster_spawn_period` | 120초 / 60초 | 추가 스폰 주기, 2회 시도, 75% / 100% |
| `mimic_spawn_count` | 3 / 6 | 동시 미메시스 상한 |
| `misc_value_min/max` | 369~392 / 1550~1645 (보상 1 / 5) | 던전에 깔리는 스크랩 총액 |
| `phone_rate` 50%·`fortunemachine_rate` 100% | | 전화·포춘머신 설치 |
| `special_monster_spawn_ids` 9005 | BabyRilla 20%, 1마리 | |
| `random_teleport_*` | 0 | (끄여 있음) |

---

## 2. 시간 압박

| 압박 | 값 | 근거 |
|---|---|---|
| 게임 시계 | 09:00 시작, 00:00 종료(`end_time`), 24:00 트램 자동 출발 경고 | `Dungeon.json`, 로컬라이즈 213 |
| 오염도 | 위험도 1: 초당 8(서기)/12(달리기), 5: 30/44. 비 ×3·폭우 ×10·스콜 ×5. 가득(10,000)이면 사망→미메시스 | `ContaData.json`, `WeatherData.json`, `StatManager.cs:700` |
| 몬스터 주기 | 60~120초마다 추가 스폰(플레이어에게서 `SPAWN_UNABLE_FROM_RANGE` 밖), 미메시스는 죽어도 수가 돌아온다 | `DungeonRoom.cs:417-490` |
| 접근 | 몬스터는 87~360초 뒤 `search`로 75 m 안 플레이어에게 걸어온다 | `npc_*_passive.xml` |
| 스태미나 | 최대 10,000, 달리기 100 ms마다 166(≈6초), 회복 100 ms마다 250, 바닥나면 3초 뒤 회복 | `Const.json` 2~10 |
| 무게 | 최대 50,000(50 kg 추정), 넘으면 이동 속도 감소(`MIN_THRESHOLD_MOVESPEED_RATE` 6000) | |
| 낙하 | 5 m 안전, 10 m 피해, 50 m 즉사 | `Const.json` 35~37 |
| 정전 | 30~60초, 비 오면 100% | |

---

## 3. 죽음·관전·빙의

- **사망 원인 표시**: "미메시스에 의해 사망" / "[닉네임]에 의해 사망"(`UI_BATTLE_DEATH_REASON_*`) — 플레이어 간 공격이 있다(`REPORT_MIN_VALUE_FRIENDLY_ATTACK`).
- **관전**: `VoiceMode.Observer` — 생존자 음성은 듣되 말은 관전자끼리만(`VoiceManager.cs:814`). 다음·이전 관전 대상 키.
- **빙의**(실험실 기능 "사망 후 동료를 관전 시, 동료 주변의 미메시스를 통해 음성을 전달 할 수 있습니다"): 관전 대상 10 m 안·전방 각도 안·벽에 가리지 않은 미메시스에 12초(`POSSESSION_DURATION`), 쿨 40초, 판당 총 30초(`POSSESSION_TOTAL_TIME`). 미메시스가 passive~bewaredistrust 어느 상태든 가능(`POSSESSION_POSSIBLE_BT`). 빙의 중 그 미메시스는 자동 발화를 멈춘다. 죽은 사람이 **진짜 목소리로 동료를 속이거나 돕는** 길이 열린다.
- **부활**: 판 안에는 없다. 정비소 귀환 시 HP 50. 전원 사망 시 던전 종료(`DungeonRoom.cs:838`), 호스트만 정비소 부활점에 5초 뒤 되살린다(`MaintenanceRoom.cs:1265-1293`).
- **시체 → 미메시스**: 오염 사망만 미메시스를 낳는다(`StatManager.cs:706-715`). HP 사망은 아니다.

---

## 4. 난이도 곡선

| 축 | 무엇이 변하나 | 값 |
|---|---|---|
| 위험도 1→5 | 몬스터 예산 171→716, 주기 120→60초, 확률 75→100%, 종 6→14, 미메시스 3→6, alpha(공격형) 40%→100% | `Dungeon.json`, `SpawnableMonsterGroup.json` |
| 오염 등급(날짜) 1→5 | 오염 상승 초당 8→30(서기), 날씨 랜덤 등급 1→5~6 | `ContaData.json` |
| 보상 1→5 | 스크랩 총액 369→1645 | `Dungeon.json` misc_value |
| 스테이지 이벤트 | 잭팟 던전 20%(스테이지 2부터, 스테이지당 1회), 비 30%(오염 등급 2부터), 보급 드롭 15%(비활성) | `StageEvent.json` |
| 미메시스 변종 | alpha: 신뢰 시작 35·공격 대기 15초·불신 시 공격·25% 자폭 / beta: 65·45초·불신 시 도주 | 행동 트리 diff |

---

## CL9에 대응하는 자리

CL9 현황은 [C:\Work\CL9\docs\reference\combat-gap.md](C:\Work\CL9\docs\reference\combat-gap.md)·[C:\Work\CL9\docs\reference\monster-rules-comparison.md](C:\Work\CL9\docs\reference\monster-rules-comparison.md) 7절에서 가져왔다. 결정은 적지 않는다.

| 장치 | MIMESIS | CL9 지금 |
|---|---|---|
| 목표 | 스크랩 수집 → 판매 → 수리비 할당량(184→1000) | 재료 훔쳐 푸드트럭 조리·납품 |
| 판 길이 | 게임 시각 09:00~00:00 ≈ 실시간 12분, 24:00 자동 출발 | 판 길이는 이 문서 범위 밖 |
| 상시 압박 | 오염도 — 서 있어도 오르고 가득 차면 미메시스가 됨 | 없음(미커밋 H 공포 게이지) |
| 등장 주기 | 60~120초 추가 스폰, 죽어도 수가 돌아옴, 2~6분 뒤 접근 | 판 시작 일괄 + 7e, 죽으면 끝 |
| 죽음 | 관전 + 빙의(미메시스로 말하기), 귀환 시 부활 | 미확인(이 문서 범위 밖) |
| 실패 | 수리비 미달 → 데스매치(전원 미메시스화) | 미확인 |
| 난이도 축 | 위험도·오염 등급·보상 등급 세 표 | 일자 곡선(양계장 8~12마리) |

---

## 근거와 한계

- `C:\Program Files (x86)\Steam\steamapps\common\MIMESIS\MIMESIS_Data\StreamingAssets\masterdata\{Dungeon,DungeonPick,Stage,StageEvent,ContaData,WeatherData,Const,SpawnableMonsterGroup,SpecialMonsterSpawnData}.json`, 코드 `DungeonRoom.cs`·`MaintenanceRoom.cs`·`DeathMatchRoom.cs`·`IVroom.cs`·`StatManager.cs`·`VoiceManager.cs`·`PossessionController.cs`.
- **미확인**: 스테이지마다 주기(날짜)가 정확히 몇 번인지(`DungeonPick` 5주기와 "N일 남음" UI의 대응을 코드로 잇지 못했다), 플레이어 간 공격의 조건, 물류(Logistics) 모드의 루프(별도 모드 — 이 문서는 다루지 않았다), 트램 밖에 남겨진 플레이어의 처리, 잭팟 던전의 추가 규칙, 데스매치 종료 조건.
- 플레이·영상 실측은 하지 않았다.
