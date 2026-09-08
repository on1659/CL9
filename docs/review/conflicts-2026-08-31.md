# 문서 충돌 보고 — `plan/` vs `systems/` · `conventions.md § 1~11` · `data/schema.md`

작성일: 2026-08-31
범위: `D:\Work\unreal\CL9\docs\` 전체 (plan 15개 · systems 13개 · conventions.md · data/schema.md · decisions.md · ue-cpp-conventions.md)
성격: **읽기 전용 보고.** 이 문서는 어떤 결정도 내리지 않는다. 정본을 어느 쪽으로 할지는 [decisions.md](../decisions.md)에 기록한 뒤 이 문서의 항목을 하나씩 닫는다.
인용 형식: `파일 풀 경로:행` — 인용문은 해당 행의 원문이다.

---

## ✅ 처리 현황 (2026-08-31 — decisions.md에 기록됨)

**전 항목 닫힘.** 아래 인용의 행 번호·원문은 처리 전 상태의 스냅샷이므로 현재 파일과 다르다.

| 항목 | 처리 |
|---|---|
| 1-1 (다섯 줄 절 유실) | plan/overview.md를 전문으로 확정 (conventions.md § 2 복원은 같은 날 번복 — 규약 문서에는 두지 않는다. 참조 4곳 overview.md로 갱신) |
| 1-2 (decisions 미기록) | 2026-08-31 결정 로그로 해소 |
| A1·A2 | **systems 채택** (UE Gameplay Framework · 단일 모듈) — plan 문서에서 SimState/9모듈 제거 |
| A3·A4 | **plan 채택** (UPrimaryDataAsset+SubAlias · ACharacter+CMC) — systems 문서 정합 |
| B~J 전체 (게임 디자인) | **plan 정본** — systems 10개 문서 재작성/수정 (balance·raiding·cooking·economy·order·shop·warning·ui·ui_system·map·inventory·drop·session) |
| K1~K5 · L1~L5 · M1~M4 (내부 오류 14건) | 전건 수정 |
| 13절 (plan 개념 grep 0건) | 정전·손전등·시드·항아리·던지기·깨짐·SubAlias·기믹·관전 — systems 층에 반영. CL9Sim/고정 스텝은 A1 결정으로 폐기 |

---

## 0. 요약

- 두 층이 **같은 주제를 다른 설계로** 적고 있는 항목 **31건**, systems 층 **내부 모순·기술 오류 14건**, systems 층에 **아예 없는 plan 핵심 개념 11개**.
- 충돌은 "수치가 다르다" 수준이 아니라 **판정 위치 · 루프 구조 · 재료 획득 방식 · 죽음 모델 · 화폐 · UI 계약** 같은 축에서 갈린다. 어느 쪽을 정본으로 삼든 반대쪽 문서는 부분 수정이 아니라 **재작성 수준**이다.
- `D:\Work\unreal\CL9\docs\plan\README.md:3`은 plan/을 "정본"으로 선언하고, `:41`은 "`decisions.md`에 기록되지 않은 문서 간 충돌은 버그다"라고 못 박고 있다. 아래 항목 중 `D:\Work\unreal\CL9\docs\decisions.md`에 기록된 것은 **하나도 없다.**

| 절 | 주제 | 건수 |
|---|---|---|
| 1 | 기록·구조 문제 (충돌 이전) | 2 |
| 2 | 아키텍처 | 4 |
| 3 | 게임 루프 · 시간 · 실패 조건 | 5 |
| 4 | 원정 · 운반 | 4 |
| 5 | 몬스터 | 4 |
| 6 | 요리 | 3 |
| 7 | 주문 · 경제 · 상점 | 6 |
| 8 | UI / HUD | 5 |
| 9 | 맵 · 레벨 | 4 |
| 10 | 캐릭터 · 입력 | 6 |
| 11 | 세션 · 진행 | 3 |
| 12 | systems 층 내부 모순·기술 오류 | 14 |
| 13 | systems 층에 없는 plan 개념 | 11 |
| 14 | 일치하는 것 | — |
| 15 | 판정 | — |

---

## 1. 기록·구조 문제 (충돌 이전의 문제)

### 1-1. `conventions.md`에 "다섯 줄 · 열리지 않는 문" 절이 없다

- 이 보고를 처음 작성할 때는 절 목차가 § 0, § 1, **§ 3** … § 11로 § 2가 비어 있었다. 2026-08-31에 § 0→§ 1, § 1→§ 2로 번호가 당겨져 빈 자리는 사라졌다(행 번호는 변하지 않아 이 문서의 인용은 그대로 유효하다). 단 `D:\Work\unreal\CL9\docs\conventions.md:3`의 "(§0~11)"은 아직 옛 번호다.
- 번호가 이어졌을 뿐, **내용은 여전히 없다.** 새 § 2는 "게임 목표"(`D:\Work\unreal\CL9\docs\conventions.md:69-79`)이고, 다음 네 곳이 가리키는 "다섯 줄 · 열리지 않는 문" 절은 어디에도 없다:
  - `D:\Work\unreal\CL9\docs\decisions.md:17` — "`docs/conventions.md`의 게임 규칙("다섯 줄"·"열리지 않는 문")을 이 시점 그대로 옮겼다"
  - `D:\Work\unreal\CL9\docs\plan\README.md:40` — "지켜야 할 게임 규칙. 다섯 줄과 열리지 않는 문"
  - `D:\Work\unreal\CL9\docs\plan\data\monster.md:25` — "conventions.md 다섯 줄 #3에 있다"
  - `D:\Work\unreal\CL9\docs\plan\overview.md:41` — "전문은 conventions.md"
- 현재 "다섯 줄"의 유일한 전문은 `D:\Work\unreal\CL9\docs\plan\overview.md:40-49`의 **요약본**뿐이다. 전문은 사라졌다.

### 1-2. `decisions.md` 미기록

아래 2~11절의 어떤 변경도 기록돼 있지 않다. 2026-08-30자 결정 두 건(문서 구조, 기획 이식)만 있다.

---

## 2. 아키텍처

### A1. 판정(권위 상태)이 어디 사는가 — 가장 큰 충돌

- plan: `D:\Work\unreal\CL9\docs\plan\overview.md:82-97` "판정은 한 곳에만 있다 — `FCL9SimState` 호스트 전용 · 순수 C++ · 엔진을 모른다", `:93` "캐릭터는 이동만 갖는다. HP·스태미나·짐은 전부 SimState 소유다", `:95` "심은 고정 스텝으로 돈다", `:97` "Sim은 UObject를 모른다"
- plan: `D:\Work\unreal\CL9\docs\plan\build-order.md:255` "판정은 `FCL9SimState`에만. 캐릭터·액터에 HP·스태미나·짐 같은 상태 필드를 만들지 않는다", `:130` "손에 뭘 들었는지 같은 '지금 상태'는 `FCL9SimState`가 가진다"
- systems: `D:\Work\unreal\CL9\docs\conventions.md:140` "ACL9PlayerState — 각 플레이어 게임 상태 (스태미나, 체력, 짐) — Replicated"
- systems: `D:\Work\unreal\CL9\docs\systems\inventory.md:24-35` 인벤토리 상태를 PlayerState에, `D:\Work\unreal\CL9\docs\systems\economy.md:48` "PlayerState.Money += 가격", `D:\Work\unreal\CL9\docs\systems\drop.md:72-106` 드롭 판정을 `ACL9Monster::Die()/DropLoot()` 액터 안에
- systems: `D:\Work\unreal\CL9\docs\conventions.md:192` "게임 시간: 호스트의 `GetRealTimeSeconds()` 기준" — plan의 고정 스텝 심과 양립 불가
- `CL9Sim` · `SimState` · "고정 스텝"이라는 단어는 systems/ · conventions.md · schema.md 어디에도 없다 (grep 0건).

### A2. 모듈 구조

- plan: `D:\Work\unreal\CL9\docs\plan\overview.md:64` "런타임 9 모듈 + 에디터 1" (CL9Core / Sim / Gen / Net / Game / View / UI / Fx / Online). 단 `:80` "CL9 자체의 설계 확정본은 아니다"라고 스스로 밝힘. 그러나 `D:\Work\unreal\CL9\docs\plan\build-order.md:55` "`CL9Sim`은 엔진 없이 도는 순수 C++", `:53` "`CL9TuningHash`"는 build-order의 **계약**으로 쓰이고 있어 Sim 모듈 분리는 사실상 전제다.
- systems: `D:\Work\unreal\CL9\docs\conventions.md:103-130` § 4 — 단일 모듈 `Source/CL9/` 6개 폴더, "처음부터 모듈을 많이 나누지 않는다"

### A3. 데이터 정의·조회 구조

- plan: `D:\Work\unreal\CL9\docs\plan\data\README.md:24` "`UCL9DefBase : UPrimaryDataAsset`", `:29-35` 컬럼 `Alias` / `SubAlias` / `ActorClass` / `Gimmicks`(Instanced), `:39` "`Alias` + `SubAlias`가 조회 키", `:53-59` "왜 DataTable이 아니라 DataAsset인가 — Instanced가 DataTable 행에서는 동작하지 않는다", `:68-72` `UCL9DefRegistry` 게임 인스턴스 소유, `FindItem(Alias, SubAlias)`
- systems: `D:\Work\unreal\CL9\docs\data\schema.md:144` "`struct FCL9DataBase`" — `SubAlias` · `ActorClass` · `Gimmicks` 없음, 대신 `:161` `Rarity`(plan에 없는 개념) 추가. `:487` "싱글톤 레지스트리 `CL9DataRegistry`". `:451` "DA_Stage_*.uasset (**또는 DT_StageBalance DataTable**)", `D:\Work\unreal\CL9\docs\systems\balance.md:210` "BP_StageConfigs (또는 DT_StageBalance)" — plan이 배제한 DataTable을 선택지로 둠.
- `SubAlias` · `기믹/Gimmick`은 systems 층에 0건. plan의 `egg+cooked` / `egg+burnt` 같은 SubAlias 체계(`D:\Work\unreal\CL9\docs\plan\data\item.md:120-127`)가 schema에서는 `cooked_omelet` 같은 별개 Alias(`D:\Work\unreal\CL9\docs\data\schema.md:213`)로 표현된다.

### A4. 캐릭터 액터의 상속

- plan: `D:\Work\unreal\CL9\docs\plan\build-order.md:98` "1d 달리기 — **커스텀 CMC** + `FSavedMove`" → `ACharacter` 상속 전제
- systems: `D:\Work\unreal\CL9\docs\conventions.md:535` "`class ACL9ActorBase : public AActor`", `:577` "`class ACL9Character : public ACL9ActorBase`" → `AActor` 계열이라 CharacterMovementComponent를 쓸 수 없다. 이전 세션 요약의 "ACL9ActorBase 설계 모호" 문제의 실체가 이것이다.

---

## 3. 게임 루프 · 시간 · 실패 조건

### B1. 루프 구조

- plan: `D:\Work\unreal\CL9\docs\plan\loop.md:4` "정차 → 주문서 수령 → 목적지 선택 → 원정 → 귀환 → 요리 → 납품 → 구매 → 다음 정차지" (8칸)
- systems: `D:\Work\unreal\CL9\docs\systems\balance.md:152-166` "Phase 1: 습격 → Phase 2: 요리 → Phase 3: 판매 → Phase 1로", `D:\Work\unreal\CL9\docs\systems\ui.md:141-155` 동일 3 Phase. 주문서 수령·목적지 선택 칸이 없다. 이전 세션 요약의 "Raiding → Cooking → Sales 상태머신"은 이 systems 쪽 구조다.

### B2. 밤 길이와 시계

- plan: `D:\Work\unreal\CL9\docs\plan\data\rules.md:53` "`NightLength` 245 — 밤 길이 (초). 만료 = 새벽 실패", `:61` "**하룻밤 안의 시간 실패는 밤 만료 하나뿐이다** — 밤 위에 다른 시계를 얹지 않는다", `D:\Work\unreal\CL9\docs\plan\economy.md:88` 밤 길이 곡선 "평평 — 줄이면 압박이 아니라 짜증이 된다"
- systems: `D:\Work\unreal\CL9\docs\systems\balance.md:31-34` `TotalStageTime` / `RaidingPhaseTime` / `CookingPhaseTime` / `SalesPhaseTime` 네 시계, `:71-75` 20분(5/10/5), `:117` 30분으로 **증가**. `D:\Work\unreal\CL9\docs\systems\warning.md:14-48` Phase 종료 30초/10초 경고, `D:\Work\unreal\CL9\docs\systems\ui.md:10` "시간: 12:34 / 20:00"

### B3. 정전 시계

- plan: `D:\Work\unreal\CL9\docs\plan\data\rules.md:54-56` `DarkBase` 185 / `DarkAccel` 4.0 / `DarkMin` 125, `D:\Work\unreal\CL9\docs\plan\economy.md:92` "밤 안에서의 압박은 정전 시계가 이미 담당한다", `D:\Work\unreal\CL9\docs\plan\data\progression.md:70` `bNoBlackout`
- systems: "정전" · "손전등" · "어둠" **0건**. 게임의 핵심 압박 장치가 systems 층에 존재하지 않는다.

### B4. 성공·실패 판정

- plan: `D:\Work\unreal\CL9\docs\plan\data\rules.md:61-62` 실패 축은 밤 만료 · 전멸 · 정산 미달 셋. `D:\Work\unreal\CL9\docs\plan\data\progression.md:82-88` "`FinalDay` 0 = 무한", "`SettlementIntervalDays` 2", "판정 대상은 **기간 수입**이지 보유 잔액이 아니다", "클리어는 없고, 며칠까지 버텼는지가 기록이다"
- systems: `D:\Work\unreal\CL9\docs\systems\balance.md:196-203` "if (플레이어_현재돈 >= TargetMoney && 전원_생존) 라운드 성공 … else if (전원_사망 || 시간초과) 게임 오버" — **매 라운드** 판정, **보유 잔액** 기준, **시간초과 = 게임오버**. `D:\Work\unreal\CL9\docs\systems\ui.md:209-230` 라운드 결과 화면 "목표금액 5000원 / 실제 획득 5500원 ✅ 성공"

### B5. 죽음 모델

- plan: `D:\Work\unreal\CL9\docs\plan\overview.md:46` "즉사가 없다. 죽음은 항상 완충타 2회 뒤 3번째 타격이다. 데미지 파이프라인을 만들지 않는다", `D:\Work\unreal\CL9\docs\plan\data\character.md:48-49` `HpMin` / `HpMax` **3**, `D:\Work\unreal\CL9\docs\plan\loop.md:74` "잡혀도 죽지 않는다 — 넘어지고 짐을 흘린다", `D:\Work\unreal\CL9\docs\plan\data\rules.md:43` "부활 항목은 만들지 않는다. 회복 채널은 트럭 패드 하나뿐", `D:\Work\unreal\CL9\docs\plan\build-order.md:167` "4a 죽음 · 관전 · 트럭 패드 회복"
- systems: `D:\Work\unreal\CL9\docs\data\schema.md:426` `MaxHealth` **100**, `:244` 몬스터 `AttackDamage` 5/15/25, `D:\Work\unreal\CL9\docs\systems\ui.md:23` "HP: ▓▓▓▓░░ (60%)", `D:\Work\unreal\CL9\docs\systems\warning.md:114` "HealthPercentThreshold 30%", `D:\Work\unreal\CL9\docs\systems\ui.md:224` "플레이어2: 💔 회복중". "관전" 0건.

---

## 4. 원정 · 운반

### C1. 재료를 어떻게 얻는가

- plan: `D:\Work\unreal\CL9\docs\plan\build-order.md:124-128` 2b 줍기 / 2c 놓기 / 2d "던지기 + 깨짐 (`BreakSpeed` 읽어서)" / 2f "품 교체 · 수레 · 항아리 · 반납". `D:\Work\unreal\CL9\docs\plan\data\item.md:29-43` 물리 컬럼(`CrackSpeed` / `BreakSpeed` / `Restitution` …), `:43` "깨짐을 줄이지 않는다. 전력질주가 위험한 게 이 게임의 마찰이다". `D:\Work\unreal\CL9\docs\plan\data\site.md:72-79` 계란 16~20개가 **월드에 놓인 물건**으로 스폰
- systems: `D:\Work\unreal\CL9\docs\systems\raiding.md:9` "채집 메커니즘: 상호작용 (E 키) → 일정 시간 경과 → 아이템 획득", `:31-37` `AHarvester { HarvestTime 3초, RespawnTime }`, `D:\Work\unreal\CL9\docs\systems\drop.md:159-170` "플레이어가 5m 이내 접근 → 아이템이 플레이어로 천천히 이동 (자석 효과)". "던지기" · "깨짐" · "항아리" 0건.

### C2. 인벤토리 모델

- plan: `D:\Work\unreal\CL9\docs\plan\data\item.md:13` `CarrySlot` = Hand / Pocketable / CartOnly (손 하나 + 품), `D:\Work\unreal\CL9\docs\plan\data\rules.md:71` "`SwapTime` 1.2 — 손↔품 교체 잠금", `:80` "1.2초는 개선하지 않는다. 운반 마찰이 이 게임의 축이다"
- systems: `D:\Work\unreal\CL9\docs\systems\inventory.md:7` "슬롯 기반: 고정 개수 슬롯 (예: 20개)", `:64` "MaxWeightKg 50kg", `:51` "무게 초과 시: 이동 속도 50% 감소"(이진 페널티), `D:\Work\unreal\CL9\docs\systems\ui.md:25` "인벤토리: 8/20". plan의 무게→감속은 연속 함수(`D:\Work\unreal\CL9\docs\plan\data\item.md:20-22`, 식 미정).

### C3. 사냥 보상

- plan: `D:\Work\unreal\CL9\docs\plan\data\monster.md:151` "사냥의 보상은 **이더 직접 지급이 아니라** `DropItems`로 떨어지는 아이템이다"
- systems: `D:\Work\unreal\CL9\docs\systems\drop.md:58-63` "Gold_Coin DropChance 0.1 (돈)", `:248-259` "돈 드롭 — ItemAlias: Money, 수량 = 금액", `D:\Work\unreal\CL9\docs\data\schema.md:311` "Alias: money, DropChance: 0.2, MinQty: 50"
- plan: `D:\Work\unreal\CL9\docs\plan\data\monster.md:24` "죽음 처리 금지선(래그돌·사체 메시·**Destroy 금지**)" vs `D:\Work\unreal\CL9\docs\systems\drop.md:79-80` "// 몬스터 액터 소멸 Destroy();"

### C4. 원정지와 트럭의 관계

- plan: 트럭은 같은 맵 안 — `D:\Work\unreal\CL9\docs\plan\data\site.md:31-32` "마당·트럭·계단·축사 외곽은 손으로 짓는다", `D:\Work\unreal\CL9\docs\plan\data\segkind.md:32` `truck` 조각, `D:\Work\unreal\CL9\docs\plan\loop.md:75` "어느 셀에서든 항아리가 1500u 안에 있다"
- systems: `D:\Work\unreal\CL9\docs\systems\map.md:126-145` 대기소 ↔ 원정지 **별도 레벨**, `:138` "[로딩 또는 순간이동]", `D:\Work\unreal\CL9\docs\systems\raiding.md:61` "철수: 플레이어가 대기소로 복귀", `D:\Work\unreal\CL9\docs\systems\balance.md:157-158` "플레이어가 대기소로 복귀 (또는 자동)"

---

## 5. 몬스터

### D1. 종류

- plan `D:\Work\unreal\CL9\docs\plan\data\monster.md:3` "닭 · 거위 · 너구리"
- systems `D:\Work\unreal\CL9\docs\data\schema.md:296` `wolf`, `D:\Work\unreal\CL9\docs\systems\drop.md:41` "Enemy_Wolf", `D:\Work\unreal\CL9\docs\systems\map.md` 전반의 🦁

### D2. AI 모델

- plan: `D:\Work\unreal\CL9\docs\plan\data\monster.md:29-127` 이동 9컬럼 · 감각 9컬럼(`SightIn` / `SightOut` 히스테리시스, 발각 게이지 `DetectRate*`) · 추격 9컬럼(`ChaseBudget` 45초, `HomeRange`, `CatchRadius`) · 각성/경보(`WakeTelegraph`, `PackAlarmRadius`) · 반격(`RageTime` = `KillCooldown`). `D:\Work\unreal\CL9\docs\plan\build-order.md:148-150` "상태 기계 — 조사·추격·복귀 / 발각 게이지 + 잡힘 / 무리 경보"
- systems: `D:\Work\unreal\CL9\docs\data\schema.md:249` "`AIType` — Passive / Aggressive / Flee", `:243-247` `MaxHealth` / `AttackDamage` / `AttackRange` / `AttackCooldown` / `MovementSpeed` 5컬럼이 전부.

### D3. 스폰 방식

- plan: `D:\Work\unreal\CL9\docs\plan\data\site.md:51-62` `MonsterSpawns {MinCount, MaxCount}` × `MonsterCountCurve` — 밤 시작 시 마릿수 결정. `D:\Work\unreal\CL9\docs\plan\data\rules.md:45` "`ThreatActMinRadius` 안에서는 위협이 깨어나거나 스폰되지 않는다"
- systems: `D:\Work\unreal\CL9\docs\systems\raiding.md:72-74, 93-100` "BaseChance + IncreasePerMin × 시간 — 매 초마다 확률 체크 — MaxConcurrentMonsters", `D:\Work\unreal\CL9\docs\systems\map.md:81-92` 스폰 지점 리스폰

### D4. 위협의 형태

- plan: 닭은 **잡는다**(`D:\Work\unreal\CL9\docs\plan\data\monster.md:81` `CatchRadius` 84) → 넘어지고 짐 흘림. 플레이어의 대응은 도망 · 프라이팬 밀치기(`D:\Work\unreal\CL9\docs\plan\data\equipment.md:47` "닭도 밀친다", `D:\Work\unreal\CL9\docs\plan\data\monster.md:119` `PushStun`)
- systems: `D:\Work\unreal\CL9\docs\systems\ui.md:34` "[능력 바] 1: 회피 / 2: 공격 / 3: 아이템 사용", `:118` "[E] 몬스터와 전투", `D:\Work\unreal\CL9\docs\conventions.md:396` "`SFX_GunShot` (향후) — 무기 발사음"

---

## 6. 요리

### E1. 요리 모델

- plan: `D:\Work\unreal\CL9\docs\plan\loop.md:90` "두 번째 게임이 아니다. 오버쿡드식 스테이션과 미니게임을 만들지 않는다", `:94` "놓고 · 기다리고 · 집기뿐", `D:\Work\unreal\CL9\docs\plan\data\item.md:79-85` `CookResult` / `BurnResult` / `CookTime` 12 / `BurnTime` 24 — "계란 → (12초) → 익은 계란 → (24초) → 탄 계란". 성공률 개념 없음(grep 0건).
- systems: `D:\Work\unreal\CL9\docs\systems\cooking.md:14-35` `FRecipe { Ingredients TMap, CookTimeSeconds, BaseSuccessRate, DifficultyMultiplier }`, `:43-48` "난수 생성 → 성공/실패 판정 — 실패: 아무것도 얻지 못함", `D:\Work\unreal\CL9\docs\data\schema.md:327` `SuccessRateBase`, `D:\Work\unreal\CL9\docs\systems\balance.md:44` `CookingDifficultyMultiplier`. `D:\Work\unreal\CL9\docs\systems\ui.md:127-129` 진행도 바 + [취소] 버튼.

### E2. 조리 용기 수

- plan `D:\Work\unreal\CL9\docs\plan\data\equipment.md:47-49` 프라이팬 · 냄비 = "두 번째 조리 용기 — 동시에 두 요리"(장비 수에 비례)
- systems `D:\Work\unreal\CL9\docs\systems\cooking.md:81-82` "1명 1개 요리만 가능 (트럭의 조리대 1개) — 다른 플레이어가 요리 중이면 대기"

### E3. 생재료 주문

- plan `D:\Work\unreal\CL9\docs\plan\data\order.md:40` "주문은 언제나 완성물을 요구한다 — 생재료 주문은 만들지 않는다"
- systems `D:\Work\unreal\CL9\docs\systems\order.md:123-124` "쉬움: 단순 재료 (계란, 버터 등)", `:65` "☐ 버터 1개"

---

## 7. 주문 · 경제 · 상점

### F1. 화폐

- plan `D:\Work\unreal\CL9\docs\plan\economy.md:10` "단위는 이더. 정수이고 소수점을 쓰지 않는다"
- systems `D:\Work\unreal\CL9\docs\systems\balance.md:84-85` "StartingMoney: 500원 / TargetMoney: 1500원", `D:\Work\unreal\CL9\docs\data\schema.md:177` `SellPrice` **float**, `D:\Work\unreal\CL9\docs\systems\economy.md:94` "StartingMoney 100원"(balance와도 불일치)

### F2. 가격식

- plan `D:\Work\unreal\CL9\docs\plan\economy.md:56` "최종 가격 = [ValueMin, ValueMax] 굴림 × 목적지 배율 × 일자 배율", `:65` "개체마다 굴린다", `:70` "성한 계란이 80~120이더"
- systems `D:\Work\unreal\CL9\docs\data\schema.md:199` 계란 `SellPrice: 100` 고정, `D:\Work\unreal\CL9\docs\systems\economy.md:130` "인플레이션 없음: 가격은 라운드 내 고정", `D:\Work\unreal\CL9\docs\systems\balance.md:48` `SellPriceMultiplier` 스테이지별

### F3. 주문서의 지위·구조

- plan: `D:\Work\unreal\CL9\docs\plan\loop.md:36` "이 루프의 엔진이다", `:44` "N개를 제시하고 M개를 고른다", `D:\Work\unreal\CL9\docs\plan\data\order.md:16-27` `Ingredients[]`, `RewardMin/Max`, `DeadlineDays`, `FailPenaltyRate`(위약금), `MinDay/MaxDay`, `Weight`(가중 추첨)
- systems: `D:\Work\unreal\CL9\docs\systems\order.md:7-10` "필수 아님 … 주문을 따르지 않아도 게임 진행 가능", `:138` "강제 아님", `:17-28` `RequiredRecipeAlias` 1개 + `BonusPriceMultiplier`, `D:\Work\unreal\CL9\docs\systems\balance.md:51` 스테이지별 고정 목록. 선택 · 마감 · 위약금 없음. `D:\Work\unreal\CL9\docs\systems\order.md:37` "TimeLimit — 주문 완료 시간 제한"은 plan `D:\Work\unreal\CL9\docs\plan\loop.md:108` "압박은 이것 하나뿐"과 충돌.

### F4. 자금 이월

- plan `D:\Work\unreal\CL9\docs\plan\economy.md:48` "남는 이더는 다음 날로 넘어간다"
- systems `D:\Work\unreal\CL9\docs\systems\economy.md:129` "무한 자본: 라운드마다 초기 자금 리셋"

### F5. 자금 소유 단위

- plan `D:\Work\unreal\CL9\docs\plan\data\progression.md:106-107` "진행은 호스트의 것이다. 판 하나가 세이브 하나" (누적 자금 = 판 공유)
- systems `D:\Work\unreal\CL9\docs\systems\economy.md:48` `PlayerState.Money` (플레이어별)

### F6. 상점 품목과 장비의 성격

- plan: `D:\Work\unreal\CL9\docs\plan\data\equipment.md:17` "`bPhysicalProp` — 월드에 놓을 수 있는 물건인가. **true만 판다**", `:24-25` "프라이팬을 들면 … 계란과 같은 규칙을 탄다", `:38` "두고 오면 잃는다", `D:\Work\unreal\CL9\docs\plan\overview.md:54` 열리지 않는 문 "스탯·업그레이드"
- systems: `D:\Work\unreal\CL9\docs\data\schema.md:181-182` `EquipmentSlot` Head/Hand/Feet + `EquipmentBonus` "WeightReduction 10%, HarvestSpeed +20%", `:110-121` `FStatBonus`, `D:\Work\unreal\CL9\docs\systems\inventory.md:54-57` "착용 — 메시 스왑", `D:\Work\unreal\CL9\docs\systems\shop.md:12-37` 탭 UI(판매/구매/장비/완료) + "모두 판매" — 착용형 스탯 장비는 plan의 금지선에 해당한다.

---

## 8. UI / HUD

### G1. 미니맵에 위협 표시

- plan: `D:\Work\unreal\CL9\docs\plan\overview.md:44` "**미니맵에 위협을 그리지 않는다.** 닭·알·파동·가격 전부. SceneCapture도 금지", `D:\Work\unreal\CL9\docs\plan\build-order.md:203` "HUD 계약: 미니맵에 위협·운반 표지를 그리지 않는다"
- systems: `D:\Work\unreal\CL9\docs\systems\ui.md:96-108` 미니맵에 "몬스터 위치 (🦁, 빨강) / 채집 포인트 / 드롭된 아이템", `D:\Work\unreal\CL9\docs\systems\map.md:121` "미니맵에 빨간색으로 표시", `:182-199` 미니맵 절

### G2. 몬스터 경고 UI

- plan `D:\Work\unreal\CL9\docs\plan\overview.md:36` "읽히는 위협 — 무엇을 하려는지 읽을 수 있어야 대응이 실력이 된다"(정보는 월드에서)
- systems `D:\Work\unreal\CL9\docs\systems\warning.md:50-93` "🦁 몬스터 접근 중! 거리: 50m", "`bShowArrow` 몬스터 방향 화살표"

### G3. 스태미나 바

- plan `D:\Work\unreal\CL9\docs\plan\build-order.md:203` "스태미나는 바를 만들지 않고 화면 흔들림 진폭으로만 알린다", `D:\Work\unreal\CL9\docs\plan\data\rules.md:82` "흔들림은 바를 만들지 않는다 — 게이지를 띄우면 '관리하는 자원'이 되어 긴장이 사라진다"
- systems `D:\Work\unreal\CL9\docs\systems\ui.md:24, 70-71` 스태미나 바, `D:\Work\unreal\CL9\docs\systems\ui_system.md:238` HUD에 "HP, 스태미나"

### G4. 위젯 구조

- plan `D:\Work\unreal\CL9\docs\plan\build-order.md:193-201` "`WBP_Main` ← 뷰포트에 붙는 유일한 위젯 … 전환은 WidgetSwitcher로 … 위젯마다 뷰포트에 직접 넣으면 z 순서·정리·전환이 전부 손으로 관리하는 일이 된다"
- systems `D:\Work\unreal\CL9\docs\systems\ui_system.md:7-10` 싱글톤 `ACL9UIManager` 액터 + 요청 큐, `:366` 위젯마다 `AddToViewport(ZOrder)`, `:218-222` 레이어별 Z-Order — plan이 명시적으로 반대한 구조다.

### G5. 동료 상황 알림 채널

- plan `D:\Work\unreal\CL9\docs\plan\data\character.md:20, 25-26` "`VoiceSet` … 반응 음성은 어둠 속에서 동료의 상황을 귀로 알리는 채널"
- systems `D:\Work\unreal\CL9\docs\systems\warning.md:95-135` 텍스트 팝업 "❤️ 팀원이 피해! Player2: 위험!" + 위치 핑

---

## 9. 맵 · 레벨

### H1. 생성 방식

- plan `D:\Work\unreal\CL9\docs\plan\loop.md:71` "밤은 시드에서 만들어진다. 레이아웃을 복제하지 않고 시드만 보내서 각자 만든다", `D:\Work\unreal\CL9\docs\plan\data\site.md:12-33` 격자 생성기 컬럼, `D:\Work\unreal\CL9\docs\plan\data\segkind.md` 조각 11종, `D:\Work\unreal\CL9\docs\plan\build-order.md:215-228` 맵 배치 툴
- systems `D:\Work\unreal\CL9\docs\systems\map.md:7` "현재 같은 목적지는 고정", `:67-122` 사전 배치 `FPlayerSpawnPoint` / `FMonsterSpawner` / `AHarvester` / `FHazardZone`, `:155` "맵 크기: 중간 (200x200m)". "시드" 0건.

### H2. 목적지 목록

- plan `D:\Work\unreal\CL9\docs\plan\data\site.md:114` `henhouse`만, `D:\Work\unreal\CL9\docs\plan\data\README.md:11` "양계장 · (나중에) 벌집"
- systems `D:\Work\unreal\CL9\docs\systems\map.md:30, 46` Mountain Forest / Water Source, `D:\Work\unreal\CL9\docs\systems\balance.md:99, 122` Mountain_Forest / Abandoned_Farm

### H3. 층 구조

- plan `D:\Work\unreal\CL9\docs\plan\data\site.md:17` 지하 격자 `AisleCountB1` / `ColCountB1`, `D:\Work\unreal\CL9\docs\plan\data\segkind.md:36-37` `flapIn`(일방향 개구멍) · `stair`
- systems 전부 단층 가정(지하 언급 없음)

### H4. 어둠·조명

- plan `D:\Work\unreal\CL9\docs\plan\overview.md:33, 47` "손전등 도달 320u 밖은 진짜로 캄캄하다", "빛은 웅덩이다"
- systems 0건 (B3과 동일 뿌리)

---

## 10. 캐릭터 · 입력

### I1. 속도

- plan `D:\Work\unreal\CL9\docs\plan\data\character.md:34-35` Walk **396** / Run **654**, `D:\Work\unreal\CL9\docs\plan\data\monster.md:43` "닭 추격 396 = 플레이어 걷기 396이다. 우연이 아니라 계약이다"
- systems `D:\Work\unreal\CL9\docs\data\schema.md:428-429` Walk **600** / Sprint **1000** — schema 값을 쓰면 plan의 부등식 A1/A4(`D:\Work\unreal\CL9\docs\plan\economy.md:102-105`) 전제가 무너진다.

### I2. 스태미나 모델

- plan `D:\Work\unreal\CL9\docs\plan\data\character.md:58-60` `StaminaDrainTime` 6.0 / `StaminaRegenTime` 8.0 / `StaminaMinResume` 0.25 (시간·비율 기반, `:62` "탈진 래치 · 재개 히스테리시스 · 실이동 조건 세 조항이 전부 있어야")
- systems `D:\Work\unreal\CL9\docs\data\schema.md:427-431` `MaxStamina` 100 / `StaminaRegenRate` 20/s / `StaminaCostPerSecond` 30 (양 기반)

### I3. 캐릭터 정의 컬럼

- plan `D:\Work\unreal\CL9\docs\plan\data\character.md:19-21` `TintColor`(슬롯 색 = 어둠 속 유일한 식별), `VoiceSet`, `FootstepSet`
- systems `D:\Work\unreal\CL9\docs\data\schema.md:414-431` 셋 다 없음. 대신 HP/속도/스태미나 수치가 캐릭터 표에 있음(plan도 몸 수치는 캐릭터 표에 두므로 이 부분은 일치)

### I4. 점프

- plan `D:\Work\unreal\CL9\docs\plan\data\character.md:41-42` `JumpVelocity` / `JumpGravity`, `D:\Work\unreal\CL9\docs\plan\data\rules.md:8` "점프는 렌더·카메라 전용이다. 판정은 평면 그대로다"
- systems 점프 언급 없음

### I5. 캐릭터의 네트워크 역할

- systems `D:\Work\unreal\CL9\docs\conventions.md:141` "ACL9Character — 로컬 (클라 전용)" — 리슨 서버에서는 호스트도 폰을 갖고, plan `D:\Work\unreal\CL9\docs\plan\build-order.md:98`의 CMC 예측 이동은 서버 폰이 있어야 성립한다.

### I6. 입력

- `D:\Work\unreal\CL9\docs\plan\build-order.md:86` "입력: Enhanced Input"이 유일한 명시. systems 층에는 입력 문서가 없고 `E 키`(`D:\Work\unreal\CL9\docs\systems\raiding.md:9`, `D:\Work\unreal\CL9\docs\systems\drop.md:10`, `D:\Work\unreal\CL9\docs\systems\shop.md:10`)와 능력 바 1/2/3(`D:\Work\unreal\CL9\docs\systems\ui.md:34`)만 산재.
- plan의 조작(줍기 · 놓기 · 던지기 차징 · 품 교체 · 손전등 · 점프)과 systems의 조작(E 채집 · 회피 · 공격 · 아이템 사용)이 다르므로 **input-system.md는 정본 결정 전에는 쓸 수 없다.**

---

## 11. 세션 · 진행

### J1. 세이브

- plan `D:\Work\unreal\CL9\docs\plan\data\progression.md:100-109` "현재 일차 · 누적 자금 · 보유 장비 · 진행 중인 주문 · 해금된 목적지 — 세이브에 들어간다 … 세이브 슬롯은 나중에 3개"
- systems `D:\Work\unreal\CL9\docs\systems\session.md` 세이브 없음, `D:\Work\unreal\CL9\docs\systems\balance.md:49` 스테이지별 `StartingMoney`(이월 없음)

### J2. 로비의 역할

- plan `D:\Work\unreal\CL9\docs\plan\loop.md:30` "로비가 이 자리(정차)를 대신한다", `D:\Work\unreal\CL9\docs\plan\data\character.md:105` "고르는 곳은 로비다"(캐릭터 선택)
- systems `D:\Work\unreal\CL9\docs\systems\session.md:82-130` 준비 상태 · 참가 코드만, 캐릭터 선택 · 일차 표시 없음

### J3. 싱글플레이

- systems `D:\Work\unreal\CL9\docs\systems\session.md:31` "[싱글 플레이] ← 로컬 게임 (나중 추가)"
- plan은 "1~4인"(`D:\Work\unreal\CL9\docs\plan\overview.md:5`)으로 1인이 기본 범위. 이전 세션 요약의 "로컬 vs 온라인" 모호점이 이것이다.

---

## 12. systems 층 자체의 내부 모순·기술 오류 (plan과 무관하게 고쳐야 함)

| # | 위치 | 문제 |
|---|---|---|
| K1 | `D:\Work\unreal\CL9\docs\conventions.md:183` vs `:197` | "판정 코드를 Character/Monster에 직접 작성 금지 (GameMode에서만)" 바로 아래 "새 판정 로직: GameMode **또는 Monster/Item**에서만" — 자기모순 |
| K2 | `D:\Work\unreal\CL9\docs\conventions.md:546` vs `:615` | 같은 필드를 `DataAlias`와 `Alias` 두 이름으로 선언 |
| K3 | `D:\Work\unreal\CL9\docs\conventions.md:619-624` | `OnRep_Alias`가 `!HasAuthority()`일 때만 `LoadData()` — 호스트(=플레이어이기도 함)에서는 메시가 로드되지 않음. `SetAlias`에도 서버 측 `LoadData` 호출이 없음 |
| K4 | `D:\Work\unreal\CL9\docs\conventions.md:496` vs `:171` | § 10에 `StaticLoadObject(... "SoundMix'/Game/Sounds/Master/...'")` 경로 하드코딩 예시 — § 6 "C++ 코드에 에셋 경로를 하드코딩하지 않는다"와 충돌 |
| K5 | `D:\Work\unreal\CL9\docs\conventions.md:114` | `CL9Constants.h` 파일을 폴더 구조에 둠 — § 1 "코드에는 구체적인 값을 박지 않음"(`:63`)과 긴장 |
| L1 | `D:\Work\unreal\CL9\docs\data\schema.md:43-78` vs `:258-263` | `FDropItem`이 위에서는 `FItemQuantity` 상속(`Quantity` 잉여), 아래에서는 독립 구조체로 **재정의** — 필드명도 `ItemAlias` vs `Alias`로 다름 (이전 세션 요약의 Critical #1) |
| L2 | `D:\Work\unreal\CL9\docs\data\schema.md:59-63` vs `:334-337` | `FRecipeIngredient`가 위에서는 `Quantity` 상속, 아래에서는 `RequiredQuantity` |
| L3 | `D:\Work\unreal\CL9\docs\data\schema.md:144` vs `:170` | `struct FCL9DataBase`를 "클래스 `UCL9ItemDef`가 상속" — UObject 클래스가 USTRUCT를 상속할 수 없음 |
| L4 | `D:\Work\unreal\CL9\docs\data\schema.md:91-101` | `FActorAppearance`를 정의했지만 Monster/Character 표(`:250-251`, `:424-426`)는 개별 컬럼을 씀 — 미사용 |
| L5 | `D:\Work\unreal\CL9\docs\data\schema.md:159` | 베이스 `Mesh`가 `UStaticMesh`인데 캐릭터·몬스터는 `USkeletalMesh` 필요 |
| M1 | `D:\Work\unreal\CL9\docs\systems\map.md:91, 233`, `D:\Work\unreal\CL9\docs\systems\inventory.md:88` | 존재하지 않는 `harvest.md` 참조 (현재 `raiding.md`) |
| M2 | `D:\Work\unreal\CL9\docs\systems\drop.md:10` vs `:173` | "자동 수집 (또는 E 키)" vs "수동 수집 (향후)" |
| M3 | `D:\Work\unreal\CL9\docs\systems\drop.md:117-127` | `AItemActor`에 `LaunchCharacter()` 호출 — ACharacter 전용 함수 |
| M4 | `D:\Work\unreal\CL9\docs\systems\balance.md:55` vs `D:\Work\unreal\CL9\docs\systems\inventory.md:63` | 인벤토리 슬롯 수를 `FStageConfig.MaxInventorySlots`와 `FInventoryBalance.MaxSlotCount` 두 곳에 정의 |

---

## 13. plan에는 있고 systems · conventions · schema에는 한 번도 안 나오는 개념 (grep 0건)

정전 · 손전등/어둠 · 시드 · 항아리(반납) · 던지기 · 깨짐 · `SubAlias` · 기믹(Instanced) · 관전 · `CL9Sim` / `SimState` / 고정 스텝 · Enhanced Input(단, 이건 plan에도 한 줄뿐)

---

## 14. 일치하는 것

- NavMesh 금지 · 셀 그래프 (`D:\Work\unreal\CL9\docs\conventions.md:181` = `D:\Work\unreal\CL9\docs\plan\build-order.md:257`)
- `TakeDamage` / `UDamageType` / GAS 금지 (`D:\Work\unreal\CL9\docs\conventions.md:182` = `D:\Work\unreal\CL9\docs\plan\build-order.md:256`)
- C++에 에셋 경로 금지 (`D:\Work\unreal\CL9\docs\conventions.md:171` = `D:\Work\unreal\CL9\docs\plan\data\README.md:74`) — K4 예외 제외
- 종류 하나 = DataAsset 하나, 소프트 참조, Alias 조회 (schema "데이터 에셋 생성 규칙" = README 규칙 1 · 3)
- 리슨 서버 · 호스트 판정 · `HasAuthority()` 관문
- 몬스터 `DropItems` 개념 자체, 트럭이 상점이라는 것, `SetNetUpdateFrequency` 20Hz

---

## 15. 판정

- **plan/을 정본으로 하면**: `conventions.md`에 "다섯 줄 · 열리지 않는 문" 절 복구 + § 4 · § 5 · § 7 · § 11 재작성, `schema.md` 전면 교체(plan/data/*.md가 이미 컬럼 정의를 갖고 있으므로 사실상 그것을 스키마로 승격), `systems/` 13개 중 **10개**(balance · raiding · cooking · drop · economy · inventory · map · order · shop · warning)가 plan과 정면충돌이라 재작성, ui.md · ui_system.md는 HUD 계약 위반 부분 삭제 + 구조 교체, session.md만 보강으로 충분.
- **systems/를 정본으로 하면**: plan/ 15개 파일의 수치·계약 대부분이 무효가 되고, `decisions.md`에 "CCL8 기획 승계 결정(2026-08-30)을 하루 만에 번복"을 기록해야 한다. 다섯 줄 중 #1(미니맵) · #3(즉사 없음) · #4(빛)가 사실상 폐기된다.
- 어느 쪽이든 **"다섯 줄 · 열리지 않는 문" 절 복구, `decisions.md` 기록, 12절의 내부 오류 14건**은 공통으로 필요하다.

정본 결정은 이 문서가 아니라 [decisions.md](../decisions.md)에 기록한다. 결정이 기록되면 이 문서의 각 항목 앞에 처리 상태를 붙여 닫아 나간다.
