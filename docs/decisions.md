# 결정 로그

> **추가만 한다** — 과거 항목을 고쳐 쓰지 않고, 뒤집을 때는 새 항목으로 번복을 명기한다.
> 규약 전문은 [conventions.md](conventions.md).

## 2026-08-30

- **문서 구조 도입** — `D:\Work\unreal\CCL8` 저장소의 구조를 승계: CLAUDE.md(요지·포인터만)
  + docs/conventions.md(규약 전문) + docs/decisions.md(결정 로그, 추가 전용). 빌드 진입점은
  `GenerateProjectFiles.bat`(엔진 5.8 경로 고정 — 레지스트리에 5.8 미등록)이고 CLAUDE.md는
  가리키기만 한다.

- **게임 기획을 CCL8 `docs/plan/`에서 이식** — CL9이 "양계장 습격"(1~4인 협동 추출 게임)의
  개발을 이어받는다. CCL8 저장소에는 이미 만들어둔 소스 코드가 있어서 거기서 계속 작업하면
  그 코드에 발목 잡힐 것 같다는 판단으로, 코드는 처음부터 새로 짜기 위해 CL9을 시작했다.
  **승계한 것은 기획뿐이다** — `docs/plan/`(게임 개요·루프·경제·작업 순서·데이터 표 15개
  파일)과 `docs/conventions.md`의 게임 규칙("다섯 줄"·"열리지 않는 문")을 이 시점 그대로
  옮겼다. **승계하지 않은 것은 코드와 UE 기술 설계**(모듈 구조·클래스·복제 방식·마일스톤
  실행 기록)다 — CCL8의 `Source/`, `docs/goal/`은 옮기지 않았고 참고도 하지 않는다.
  이식하며 코드 심볼 접두는 `CCL9`→`CL9`로 치환했고, "지금 구현된 것" 같은 진행 상태 서술은
  CL9의 실제 상태(전부 없음)로 고쳐 썼다 — 그 외 게임 수치·규칙은 하나도 바꾸지 않았다.
  이후 CL9의 게임 디자인 결정은 이 로그에 새로 쌓는다.

## 2026-08-31

- **문서 기술 오류 14건 수정** (conflicts-2026-08-31.md 분류 L, K, M 항목)
  + **L1, L2**: schema.md — FDropItem/FRecipeIngredient 필드명 통일 (ItemAlias 사용), 상속 구조 명확화
  + **L4**: schema.md — FActorAppearance 미사용 명시 (향후 리팩토링용)
  + **L5**: schema.md — Mesh 타입 명확화 (베이스는 StaticMesh/아이템용, 캐릭터/몬스터는 SkeletalMesh별도)
  + **K2**: conventions.md — DataAlias → Alias 필드명 통일
  + **K3**: conventions.md — OnRep_Alias 호스트 로드 문제 (SetAlias에서 직접 LoadData 호출 추가)
  + **K1**: conventions.md — 판정 코드 위치 모순 해결 (액터 자신의 로직은 자신에서, 게임 전체 진행은 GameMode에서)
  + **K4, K5**: conventions.md — 경로 하드코딩(시스템용만 허용) 및 Constants.h 정책 명확화
  + **M1**: inventory.md, map.md — harvest.md → raiding.md 참조 수정
  + **M2, M3**: drop.md — 자동 수집 정책 통일, LaunchCharacter() → AddImpulse() 물리 호출 수정
  + **M4**: inventory.md, balance.md — MaxInventorySlots 중복 정의 통합 (FStageConfig 사용)
  + **F5**: economy.md — 자금이월 정책 수정 (라운드마다 초기 자금 리셋 → 이월 방식)

- **관점 분류** — plan/ (기획)과 systems/ (개발)의 충돌 원인 파악: 기획은 "뭘 할 것인가", 개발은 "어떻게 구현할 것인가"
  이 둘은 양립해야 하며, 향후 설계 결정은 이 관점 분리를 명확히 해서 진행할 것.

- **conventions.md 분할** — 규약(지켜야 할 규칙)과 시스템 설계(어떻게 동작하는가)를 분리.
  § 10 사운드 설계 전문 → `systems/sound.md`, § 11 액터 베이스 구조 전문 → `systems/actor-base.md`로
  이동하고, conventions.md의 § 10·§ 11에는 핵심 규칙 몇 줄과 포인터만 남겼다 (743줄 → 약 350줄).
  코딩 표준을 ue-cpp-conventions.md로 분리해둔 기존 방침과 같은 원칙이다.
  `systems/warning.md`의 § 10 참조를 sound.md로 갱신했고, 문서 머리의 "(§0~11)" 옛 번호도
  "(§1~11)"로 고쳤다 (conflicts-2026-08-31.md 1-1에서 지적된 잔재).

- **아키텍처 정본 확정 (conflicts-2026-08-31.md A1~A4)** — 기준: 언리얼 표준에 가까운 쪽을 택한다.
  + **A1 판정 위치 → systems 채택.** 권위 상태는 GameState·PlayerState·액터의 Replicated
    변수에, 판정은 GameMode/서버 코드에 둔다 (UE Gameplay Framework). plan의
    `FCL9SimState`(순수 C++ · 고정 스텝 · 엔진 독립) 설계는 **폐기** — 엔진 독립 테스트와
    결정론이라는 이점을 포기하는 대신, 복제·CMC 예측 등 엔진 표준 기능을 그대로 쓴다.
    plan/overview.md · plan/build-order.md · plan/data/README.md · plan/data/monster.md에서
    SimState/CL9Sim 전제를 제거했다. `CL9TuningHash`(접속 시 수치 대조)는 Sim과 무관하므로 유지.
  + **A2 모듈 구조 → systems 채택.** 단일 모듈 `Source/CL9/` (conventions.md § 4).
    plan/overview.md의 9모듈 다이어그램을 제거했다.
  + **A3 데이터 구조 → plan 채택.** `UCL9DefBase : UPrimaryDataAsset` + `Alias`+`SubAlias` 조회 키
    + Instanced `Gimmicks` + `ActorClass` + `UCL9DefRegistry`(게임 인스턴스 소유).
    DataTable 선택지는 제거 (Instanced가 DataTable 행에서 동작하지 않음 — plan/data/README.md).
    data/schema.md의 베이스를 plan 정본에 정합시키며 `Rarity` 필드는 삭제(plan에 없는 개념),
    `cooked_omelet` 별개 Alias 예시는 `egg`+`cooked` SubAlias 체계로 교체. balance.md·ui_system.md의
    DataTable 표기도 DataAsset으로 통일. 이로써 L3(UClass가 USTRUCT 상속 불가) 오류도 해소됐다.
  + **A4 캐릭터 상속 → plan 채택.** `ACL9Character : ACharacter` + 커스텀 CMC + `FSavedMove`
    예측 이동. `ACL9ActorBase`는 아이템·몬스터용으로 유지 (conventions.md § 11,
    systems/actor-base.md). conventions.md § 5의 "ACL9Character — 로컬 (클라 전용)" 오기도
    "모든 머신에 폰 존재"로 정정.
  + **GAS는 금지 해제, 현재는 미사용** (같은 날 완화 — "금지로 못 박을 필요는 없다"는 사용자
    결정). 지금 설계는 Replicated 값으로 충분해서 쓰지 않지만, 필요해지면(학습 겸 포함) 도입을
    검토한다. `TakeDamage`·`UDamageType` 금지(데미지 파이프라인 없음 설계)는 그대로다 —
    conventions.md § 7 · plan/build-order.md · plan/data/monster.md의 "GAS 금지" 표기를 완화했다.

- **게임 디자인 정본은 plan/이다 (conflicts-2026-08-31.md B~J 전체 해소)** — plan/README.md가
  스스로 정본을 선언하고 있고, 2026-08-30에 게임 기획을 그대로 승계하기로 결정했으며, systems/
  쪽 설계는 다섯 줄·열리지 않는 문을 여럿 위반(미니맵 위협, 스태미나 바, 착용형 스탯 장비,
  성공률 요리)한 범용 협동게임 보일러플레이트였다. systems/ 문서를 plan에 맞춰 재작성했다.
  + **conventions.md § 2 복원 (1-1)**: 유실됐던 "다섯 줄 · 열리지 않는 문" 절을
    plan/overview.md 요약본 + plan/data/*.md 산재 조항으로 재구성해 § 2에 넣었다
    (CCL8 원문 전문은 유실 — 이 재구성본이 새 전문이다). § 7의 K1 잔재(판정 위치 자기모순)도 정리.
  + **B1·B2·B4 (루프·시계·실패)**: balance.md 재작성 — 3페이즈/4시계/라운드별 목표금액 폐기.
    밤 단일 시계(NightLength 245초) + 정전 시계 + 일자 곡선 × 목적지 기준값 + 이틀 주기
    기간 수입 정산 + 무한 진행(FinalDay 0). FStageConfig 폐기.
  + **B3·H4 (정전·어둠)**: systems 층에 0건이던 정전·손전등을 balance.md·raiding.md·map.md에
    반영.
  + **B5 (죽음)**: HP 3 완충타 모델 — HP 100/%바/AttackDamage 스키마 폐기, 관전·트럭 패드
    회복을 ui.md·raiding.md에 반영.
  + **C1~C4 (원정·운반)**: raiding.md 재작성 — E키 채집 노드(AHarvester) 폐기, 월드 실물
    아이템(줍기·던지기·깨짐·항아리 반납). inventory.md 재작성 — 슬롯 20칸/무게kg 모델 폐기,
    손+품+수레(CarrySlot)·SwapTime 1.2초·유효 무게 감속. drop.md 재작성 — 돈 드롭 금지,
    Destroy() 금지, 자석/자동수집/소멸타이머 폐기. 트럭은 같은 맵(별도 레벨 없음).
  + **D1~D4 (몬스터)**: 늑대/AIType 3종 스키마 폐기 — 닭·거위·너구리와 plan/data/monster.md의
    컬럼(히스테리시스·발각 게이지·추격 예산·무리 경보)이 정본. 스폰은 확률 증가가 아니라
    밤 시작 마릿수(기준값 × 곡선). "전투 능력 바"류 조작 폐기.
  + **E1~E3 (요리)**: cooking.md 재작성 — 레시피/성공률/조리대 1개 폐기. 열 상태변화 체인
    (CookResult/BurnResult, 12초/24초), 동시 요리 수 = 조리 용기 수. 생재료 주문 금지.
  + **F1~F6 (경제)**: economy.md 재작성 — 화폐는 이더 int(float 원화 폐기), 가격은
    [ValueMin,ValueMax] 굴림 × 목적지 × 일자 배율(고정 SellPrice 폐기), 자금은 판 공유
    (GameState.SharedMoney — PlayerState.Money 폐기), 이월 유지. order.md 재작성 — 주문서는
    루프의 엔진(N제시 M선택·마감·위약금·가중 추첨 — "선택적 보너스" 폐기). shop.md 재작성 —
    bPhysicalProp 실물만 판매(착용형 스탯 장비 탭 폐기), 구매 = 실물 스폰, 트럭 패널.
  + **G1~G5 (UI/HUD)**: ui.md 재작성 — 미니맵 위협/스태미나 바/HP %바/능력 바/목표금액 표시
    폐기 (HUD 계약 준수). warning.md 재작성 — 몬스터 접근 경고·팀원 텍스트 팝업 폐기, 위협
    정보는 월드 연출(WakeTelegraph·연기)과 VoiceSet 음성으로. ui_system.md 재작성 — 싱글톤
    UIManager+큐+위젯별 AddToViewport 폐기, WBP_Main 단일 위젯 + WidgetSwitcher.
  + **H1~H3 (맵)**: map.md 재작성 — 시드 기반 생성(시드만 복제), 목적지는 henhouse 하나
    (산림·생수처 등 가상 목적지 폐기), 지하 격자 포함, 대기소↔원정지 별도 레벨 폐기.
  + **I1~I6 (캐릭터·입력)**: schema.md의 속도 600/1000·스태미나 양 기반 컬럼 폐기 — 정본은
    plan/data/character.md (걷기 396/달리기 654, 시간·비율 스태미나, TintColor/VoiceSet/
    FootstepSet, 점프). 입력 문서는 정본 확정 후에도 조작 세트가 plan 기준이 됨을 전제로 미작성.
  + **J1~J3 (세션)**: session.md 수정 — 판 세이브(호스트 소유, 일차·누적자금·장비·주문·해금)
    추가, 로비에 캐릭터 선택·일차 표시, 싱글은 "나중 추가"가 아니라 리슨 서버 1인(기본 범위).
  + **schema.md 슬림화**: 표별 컬럼 정의를 plan/data/*.md 정본 포인터로 대체 — 두 벌 관리가
    이번 드리프트의 원인이었다. schema.md에는 베이스 클래스·공용 구조체(FCL9ItemCount·
    FDropItem·FCL9SpawnEntry·FCL9StatEffect·FSoundEffect)·생성 규칙·로드 검사만 남긴다.
    FItemQuantity → FCL9ItemCount로 개명(SubAlias 포함), FStatBonus/FActorAppearance/
    ERarity/EItemType(장비 슬롯) 폐기.

- **게임 규칙의 전문 위치를 plan/overview.md로 확정 (같은 날 § 2 복원 번복)** — 같은 날
  conventions.md § 2로 재구성 복원했던 "다섯 줄 · 열리지 않는 문"을 삭제했다. 게임 규칙이
  기획 문서와 규약 문서 두 곳에 있을 이유가 없고, **conventions.md는 UE 설계 규약 전용**으로
  두는 게 맞다는 사용자 결정. plan/overview.md의 다섯 줄이 전문으로 승격됐다 (#3에 래그돌·
  사체 메시·Destroy 금지 상세 편입, 열리지 않는 문에 열린 범위 실물 규칙 명기).
  + conventions.md를 § 1~10으로 재번호 (옛 § 3~11 → § 2~10).
  + 참조 갱신: CLAUDE.md 문서표(게임 규칙 → plan/overview.md, 규약 행 분리),
    plan/README.md·loop.md·economy.md·build-order.md·data/{equipment,character,monster,item,
    order,README}.md의 conventions.md 게임 규칙 포인터를 overview.md 또는 rules.md로 교체,
    plan·systems 전반의 § 번호 참조를 새 번호로 갱신.

- **몬스터 AI 공용 상태 기계 설계 — `systems/monster-ai.md` 신규** — "AI 코드는 틀 하나,
  몬스터 간 차이는 전부 표의 값"을 개발 층에 설계로 고정. 상태 기계(잠듦→배회/순찰→조사→
  추격→잡기/포기→복귀 + 경직·복수·죽음), 감각 3입력(시야 히스테리시스·행동 소음·근접)→발각
  게이지, 셀 그래프 이동, 잡기=공격(수치 데미지 없음), 경보(같은 존 검사), DangerCurve 배율.
  종별 차이는 값 → 상태 사용 여부 → 기믹 순으로만 표현하고 종별 파생 클래스는 만들지 않는 게
  기본. 구현 순서를 build-order ③(3b~3g)에 대응시킴.

## 2026-09-01

- **치트 시스템 설계 — `systems/cheats.md` 신규** — `UCL9CheatManager`(UCheatManager 서브클래스)
  + `IConsoleManager` 등록으로 `cl9.` 접두 명령 체계. **치트 하나 추가 = 함수 하나 + 등록 한 줄.**
  인자는 Alias 또는 번호 둘 다(`cl9.additem egg` = `cl9.additem 1`, 번호는 `cl9.list` 출력 순 —
  타이핑 편의용, 재현용은 Alias). 치트도 판정 규약을 탄다 — 클라 콘솔 입력은
  `Server_CL9Cheat(명령줄)` RPC로 호스트가 실행, 스폰은 실제와 같은 SetAlias→복제 경로라
  치트 테스트가 곧 네트워크 테스트가 된다. Shipping 빌드 컴파일 제외.
  기존 `bp_` 디버그 명령을 전부 `cl9.`로 통일 (balance.md의 bp_debug_day → cl9.day,
  map.md의 bp_map_seed/show_cells/show_jars → cl9.seed/showcells/showjars,
  warning.md의 bp_warning_* → cl9.testwake/testvoice/fxoff).

- **시퀀스 재생 설계 — 기획·개발 두 층으로 신규** — `plan/data/sequence.md`(어느 전이에서
  무엇을 트나 — `UCL9SequenceDef` 표: Sequence 소프트 참조·bLockLocalInput·bSkippable·
  bHostWaits, 초안 목록 night_start/blackout/wipe/dawn) + `systems/sequence.md`(재생 메커니즘 —
  Level Sequence + `UCL9SequenceSubsystem`, 호스트가 "언제"를 판정하고 재생은 각 머신 로컬,
  프레임 동기화 없음, 스킵은 로컬, 시퀀스 종료 대기는 클라 보고가 아니라 호스트 타이머).
  원칙: 게임은 멈추지 않는다 · 원정 한복판에 긴 컷씬 금지 · 관리는 DataTable이 아니라
  DataAsset(A3 결정 준수). plan/data/README.md 표 목록·레지스트리(FindSequence),
  schema.md 포인터 표, cheats.md(cl9.playseq) 갱신.
  + 제작 규칙 추가: 시퀀스가 레벨에 묶이지 않게 — 카메라·소품은 Spawnable, 월드 실물(트럭)은
    Possessable 금지·런타임 바인딩(`SetBinding`), 레벨 특정 연출은 전용 시퀀스로 분리.

- **에셋 사용 가능/불가 구분 — EDefStatus** (schema.md 로드 검사 확장) — 검사를 통과 못 한
  행은 조용히 섞이지 않는다: Usable(정상)/Broken(필수 필드 누락·깨진 참조 — Find*가 nullptr,
  로드 시 행·컬럼 로그). `cl9.list`가 상태를 표시(`✗ 사유`). 소프트 참조는 로드 시 경로
  해석만 검사, 런타임 로드 실패는 화면 로그 + 대체 표시 — 조용한 투명 액터 금지.

- **빠진 시스템 7건 일괄 설계** (2026-09-01 감사에서 식별된 공백 — 사용자 지시로 전건 작성):
  + **입력 — `systems/input.md` 신규**: Enhanced Input 확정 구현 — IMC 3개(Gameplay/UI/
    Spectate, 화면 상태와 한 곳에서 전환), IA 11개(plan 조작 세트 그대로 — 폐기된 E채집·능력
    바 부활 금지), 던지기 차징은 호스트 시계로 판정(클라 차징량 불신), SwapTime 잠금은 호스트
    관문. **상호작용 공통 틀 포함** — `ICL9Interactable` 인터페이스(줍기·반납·패널·창구 통합),
    로컬 감지 + 호스트 재검사. 리바인딩은 Enhanced Input User Settings. 키 배치는 초안(미결).
  + **조명 — `systems/light.md` 신규**: 다섯 줄 #4의 시스템화 — 손전등 320u 담체(Replicated),
    발광 아이템, 정전(호스트 판정 GameState.BlackoutAtTime → OnRep 소등, 몬스터 SightDark
    전환), 렌더 설정(자동 노출 끔·GI로 어둠 메우기 금지), 배터리 시스템 없음(정전이 카운터).
    정전 판정은 ②(반납)와 함께, 연출은 5c.
  + **설정 — `systems/settings.md` 신규**: 로컬 전용(판 세이브와 분리) — 오디오(사운드 믹스
    클래스별), 그래픽(UGameUserSettings, 감마는 어둠 계약 보호 위해 제한 범위), 입력
    리바인딩·감도, 접근성(자막·UI 스케일), 언어(한국어 시작). 게임플레이 값 노출 금지.
  + **세이브 — `systems/save.md` 신규**: `UCL9SaveGame`(버전·일차·자금·기간수입·장비·주문·
    해금 — 전부 Alias, 경로 금지), 저장 단위는 "날"(새벽 자동 저장만), 슬롯 번호 구조(지금 1,
    나중 3), 깨진 세이브는 백업 후 새 판(조용한 소실 금지), **기록은 별도 파일**
    `UCL9RecordSave`(BestDay — 판이 죽어도 남는다). session.md 세이브 절은 포인터로 축소.
  + **Steam 연동 — session.md 확장**: OnlineSubsystemSteam + 개발 AppID 480, Steam Sockets
    (SDR — 포트포워딩 불요, IP 직결 금지), 참가 경로 2개(친구 초대 + 참가 코드), 랜덤 매칭
    없음, 접속 시 TuningHash 대조. 코드 상세는 1f에서 확정.
  + **애니메이션 — `systems/animation.md` 신규**: 계약 셋(경로 0줄 — Def 소프트 참조 /
    애님은 판정 불가침 — 노티파이는 렌더 전용 / 캡슐이 계약). 스켈레톤: 캐릭터는 UE5 마네킹
    호환 구매 권장(재작업 방지), 몬스터는 종별 1개. 상태→애님 파생(RPC 트리거 금지),
    몽타주 사용처 표(죽음·경직·잡힘·기상 예고 — 길이는 판정 값에 맞춤).
  + 교차 참조: inventory·ui·raiding·map·sound.md에 신규 문서 링크 추가.

- **item/monster 문서 폴더 구조화 (plan + systems 양층)** — "종류 하나 = DataAsset 하나"
  구조를 문서에도 미러링: 폴더/common.md = 공통 컬럼·틀, 하위 = 개별 개체 (값 무손실 이동).
  + plan/data/item.md → `plan/data/item/` {common(컬럼 전문), egg(눈금 기준 — 물리·발광·요리
    값), meat, cart(물건으로서의 수레 — 상점 행은 equipment.md 유지·ItemDef 연결),
    flashlight(신규)}.
  + plan/data/monster.md → `plan/data/monster/` {common(컬럼 전문 + 전역 기본값),
    chicken(값 전부), goose(차이 값 + 전용 컬럼 — 걷기 불가침의 유일한 예외 명시),
    raccoon(미결)}.
  + systems/inventory.md·drop.md → `systems/item/` (+ common.md 신규 — ACL9Item 액터·SubAlias
    상태 전이·수명 공통 틀), systems/monster-ai.md → `systems/monster/common.md`.
  + 참조 15개 파일 재배선 (README·schema·build-order·loop·economy·character·order·equipment ·
    animation·cheats·cooking·input·light·raiding·warning).

- **손전등은 아이템이다 — `plan/data/item/flashlight.md` 신규** (내장 기능 초안 번복).
  모델: `Pocketable` — **손에 들어야 빛나고 품에 넣으면 꺼진다** (`bLightLostInPocket` 기존
  규칙 재사용, 새 슬롯 개념 없음). 빛 vs 짐 트레이드오프가 게임의 빛 경제가 된다 (혼자면
  교체 마찰, 팀이면 분업, 스탠드 램프가 보완). 정전 시 죽는 계약은 아이템이어도 유지.
  시작 지급 1개(초안) — 재지급·판매·무게 미결. light.md(조명 시스템)는 유지 — 광원 개체는
  아이템 데이터, 정전·렌더 계약·판정 연동만 시스템 담당으로 역할 명확화.
  + **같은 날 수정 (사용자 게이트)**: "판매 불가" 초안 번복 — **손전등도 팔 수 있다**
    (여느 아이템과 같은 값 규칙, ValueMin/Max 미정). 판 뒤에는 트럭 패널에서 산다(초안 —
    equipment.md 행 추가 후보). **기본 지급은 손잡이다** — 줄 수도 안 줄 수도 있고,
    프로토타입에서는 편의상 지급 (지급 값의 자리 미정).

- **몬스터 잠·경보꾼 컬럼 5종의 문서 위치 — 거위 전용 절로 확정** (같은 날 두 번 조정 끝
  사용자 결정). 경위: 옛 단일 monster.md의 "거위 전용" 절이 공용 문서 안에 있던 게 문제로
  보였으나, 폴더 분리 후에는 종별 문서가 있으므로 "자는 행동부터 일반 몬스터와 다른" 종의
  개념은 그 종 문서에서 정의하는 게 맞다는 결론. 최종 규칙:
  + **대부분의 차이는 공통 컬럼의 값이다** (거위의 속도·몸집·HomeRange·bAlertsOthers 등 —
    HomeRange가 옛 전용 절에 중복돼 있던 오류는 공통 값 표로 정리).
  + **소수 종만 쓰는 컬럼은 그 종의 문서에서 정의한다** — WakeRadius·HearRunRadius·
    AlertWindup(구 HonkWindup — 승격 대비 일반화 명명 유지)·SleepTime·AlertCooldown은
    goose.md 전용 절. 코드에는 공통 필드로 존재(클래스는 UCL9MonsterDef 하나, 안 쓰는 종
    0/false)하고, **다른 종이 쓰게 되면 common.md로 승격한다.**

- **"다섯 줄" 전역 원칙 폐지** (사용자 결정 — "개별 시스템에서 정의할 것을 절대 원칙으로
  끌어올리지 마라"). CCL8 승계 규칙 다섯 개를 전부 해당 시스템의 설정으로 되돌렸다:
  + **미니맵 위협 금지** → 폐지. 표시 여부는 개체별 값 — `bShowOnMinimap`(기본 false) 컬럼을
    monster/common.md에 신설, 미니맵 시스템이 읽는다 (ui.md·drop.md 갱신).
  + **NavMesh 금지·셀 그래프 강제** → 폐지. 경로 탐색 방식은 몬스터 구현(③)에서 정한다
    (NavMesh vs 커스텀 그래프 — 미결). 층 봉쇄·flapIn은 방식 무관한 맵 요구사항으로 유지.
    `cl9.showcells` → `cl9.shownav` 개명.
  + **즉사 없음** → 폐지. 즉사는 가능하고 HP·타격량은 값의 문제다 — 즉사 방지가 필요한
    캐릭터/몬스터는 HP 값으로 해결 (character.md·monster/common.md·loop.md 문구 수정).
    죽음 처리 금지선(래그돌·사체 메시·Destroy 금지)도 폐지 — 종별 표현 선택.
    `TakeDamage`/`UDamageType` 미사용은 기술 규약으로만 유지 (단순 HP 값 사용).
  + **빛은 웅덩이** → 폐지. 조명은 light.md의 세팅이다 — 자동 노출 끔·GI 값 등은 전역 계약이
    아니라 렌더 세팅 항목 (5c에서 값 확정). settings.md 감마 문구도 세팅 연동으로 완화.
  + **존 이격 1050·파생 곱 부등식** → 폐지. 존 간격 대 소음 반경 관계는 맵 세팅의 몫
    (economy.md 부등식 표에서 제거). 사살 경보의 같은-존 검사는 맵 존 세팅과 함께 정하는
    구현 선택으로 완화.
  + plan/overview.md의 다섯 줄 절 삭제, conventions.md·CLAUDE.md·plan/README.md 포인터 갱신.
    "열리지 않는 문"(기능 금지 목록)은 별도 건 — 같은 날 아래에서 개편.

- **"열리지 않는 문" → "만들지 않는 기능"으로 개편** (사용자 결정 — 은유적 작명과 절대 서약
  거부). 내용물(부패·허기 / 영업 러시 / 영구 스탯·업그레이드 / 조리 미니게임 제외)은 기획의
  정당한 범위 결정이라 **유지**하되:
  + 이름을 평문으로 — "만들지 않는 기능 (범위 제외)". "구현은 물론 제안도 하지 않는다"는
    서약 문구 삭제 — 보통의 범위 결정으로 격하 (바꾸려면 decisions.md 기록, 상점 확장 전례).
  + 표 형식으로 항목별 이유 + 소관 문서 링크. 소관 문서에 평문 서술 추가 (부패 →
    item/common.md, 허기 → character.md — 러시·스탯·미니게임은 이미 소관 문서에 있었음).
  + "열리지 않는 문" 참조 10곳을 새 이름으로 갱신 (build-order·loop·character·equipment·
    order(plan/systems)·cooking·shop·inventory·conventions·CLAUDE.md·plan/README.md).

- **"만들지 않는 기능" 목록 폐지 + 승계 문서 과장 어법 전수 정리** (같은 날 재번복 — 사용자
  결정: "상황에 와서 괜찮으면 제안할 수도 있는 것이라 막을 필요가 없다"). 기능 차단 목록
  자체를 없앤다 — 표준은 "현재 설계 서술"이지 "미래 제안 금지"가 아니다.
  + overview.md의 "만들지 않는 기능" 절 삭제. 각 항목은 소관 문서의 현재 설계 서술로만 존재
    (부패 없음 → item/common, 생재료 주문 없음·마감이 압박 → order, 착용 장비 없음 →
    equipment/shop/inventory, 미니게임 아님 → loop/cooking, 부활 없음 → rules).
  + 과장·서약 어법 정리 (약 25곳): "구현은 물론 제안도 하지 않는다"류 전부 삭제,
    "불가침"·"개선하지 않는다 — 그게 게임이다"·"~하나뿐이다"·"세 번째 연결을 만들지 않는다"·
    "우연이 아니라 계약이다" → 현재 설계 서술 + 근거로 교체 (rules.md 5곳, loop.md 3곳,
    overview.md 재미 표 3행, item/common 4곳, monster/chicken·goose, economy, segkind,
    equipment, order, systems 에코 5곳).
  + 유지한 것: 기술 아키텍처 서술("직접 데이터를 보내는 구조는 만들지 않는다", "세이브에
    경로를 넣지 않는다" 등 — 결정된 구조의 설명)과 값의 근거 설명은 그대로.

- **CCL8 잔재 정리 (감사 후속)** — 거짓이 된 현재형 서술과 죽은 코드 참조 제거:
  + overview.md 루프 다이어그램의 "지금 여기만 구현됐다" 화살표 삭제 (CCL8 기준 — CL9은 전부
    미구현), loop.md "원정 — 지금 있는 것" 절 제목 → "원정".
  + 트럭 패널 "이미 세 항목이 있다 — 선글라스·시동·취소" 삭제 (equipment.md·shop.md) —
    **선글라스는 CL9 기획에 없는 CCL8 아이템.** 패널 = 장비 구매 목록, 시동 등 상호작용
    동거 여부는 미정.
  + character.md `Det[4]` 코드 현재형 → 슬롯 인덱스 식별 서술로.
  + 죽은 코드 참조 삭제: segkind.md `CL9Gen::SegHeightU()`·`IsJumpSkip()`, site.md
    `CL9Geometry` (CL9Gen 모듈은 A2 결정으로 부재).
  + "구현 참고값" 컬럼명 → **"구 CCL8 상수"** (7개 파일) — u단위 원값의 출처로서 유지하되
    정체를 정직하게 표기. site.md 밤당 28개도 "구 CCL8 값" 시제 정리.
  + 유지: decisions.md 이력, build-order의 CCL8 경험 근거(5천 줄 교훈·177MB 맵 사고),
    CarryMult 이주 환산 노트.

- **탈것 입장 연출은 후순위 확정** — 밤 시작 시 탈것(트럭)을 타고 시작 지점으로 입장하는
  연출은 sequence.md `night_start`가 그 자리다 (`bHostWaits` — 끝나야 조작 개방). 연출
  제작 전에는 스폰 지점에서 바로 시작 — 구조 훅만 먼저, 에셋·시퀀스는 시퀀스 시스템
  구현 시점에.

- **최종 적대적 리뷰 (독립 에이전트 2개 — 정합성·UE 기술) 발견 58건 전건 반영.** 핵심:
  + **[네트워크 치명타] 비소유 액터 Server RPC 금지** — 아이템·몬스터는 클라 소유가 아니라
    클라발 Server RPC가 조용히 드롭된다. 규칙 확정: 클라 요청은 소유 액터(Character/PC)의
    RPC로 올리고 호스트가 대상 액터 함수를 직접 호출 (actor-base.md·input.md·cheats.md 재작성).
  + **Alias 복제 정정** — `ReplicatedUsing=OnRep_AliasPair` + `DOREPLIFETIME` 명시, SubAlias를
    베이스에 추가, **초기 로드는 BeginPlay·런타임 변경만 OnRep** (레벨 배치 액터는 값 변화가
    없어 OnRep이 안 뜬다).
  + **시간 기준 교체** — `GetRealTimeSeconds` → `GetServerWorldTimeSeconds` (클라 시계 자동
    동기화 — 밤 시계·정전 시각·잠금 타임스탬프가 클라에서 해석 가능해짐).
  + **데이터 흐름 재정의 (conventions § 5)** — "호스트가 표를 GameState로 복제" 폐기 →
    정적 표는 각 머신 로컬 레지스트리 + TuningHash 검증, GameState는 런타임 상태만
    (build-order·data/README·balance 동조).
  + **CMC 예측 예외 (conventions § 4)** — 스태미나·무게 파생 속도는 CMC/FSavedMove 경로,
    PlayerState는 표시 미러 (러버밴딩 방지).
  + **sound.md 전면 재작성** — 재작성 누락됐던 CCL8 이벤트 목록(채집·요리성공률·총성·돈드롭·
    폐기 경고·가상 목적지) 청산, API 시그니처 주석 교정, 사운드 Multicast는 Unreliable.
  + **build-order "HUD 계약(미니맵 금지)" 잔재 제거**, 4c→4h 라벨 정정.
  + **점프 판정 역할 통일** — "렌더 전용" 폐기: 낮은 장애물 통과는 판정 역할이고, 거리·감각
    판정만 평면 기준 (rules·segkind·input·character 정합).
  + **거위 A1 예외 명문화** — 거위는 잡지 않는 경보꾼(CatchRadius 0 초안)이라 부등식 대상
    아님. `CatchDamage` 컬럼 신설(닭 1 초안 — 즉사 폐지의 대체 메커니즘).
  + session.md: ServerTravel 심리스 흐름, 준비 상태는 PlayerState로, Steam 로비 세션+게임
    식별 키+480 공용 주의, 호스트 이탈=세션 소멸, 중간 참가 절 신설(연출 Multicast 미재생·
    화면은 GameState 파생 복원). sequence 트리거는 Multicast 전용 확정.
  + cheats: AllowCheats 오버라이드·라우팅은 PC 직행·RPC 선언은 #if로 제외 금지·번호는 클라
    변환 후 전송. 기타: 품 아이템은 "파괴+데이터 보존" 확정, 밤 종료 정리 주체(복제=호스트
    Destroy, 로컬 지형=페이즈 OnRep 정리 패스), 장비 보존 경계=트럭 존, EDefStatus 필수
    필드는 클래스별, MonsterHpCurve 유령 제거, HurtStun 중복 제거(character.md로), 위약금
    0 하한, HonkWindup 잔재 4곳 AlertWindup으로, "계약"·"절대" 어법 17곳 청산, loop 진행표
    삭제, Ingredients에 ItemSubAlias 표기 등.

- **systems 층 재평탄화 — 폴더에는 진짜 개체만** (사용자 결정: "폴더는 진짜 아이템/몬스터만,
  시스템을 넣는 게 아니다"). 폴더 구조(common + 개별)는 **plan/data 전용** 규칙으로 확정.
  systems/item/{inventory,drop,common}.md·systems/monster/common.md → systems/
  {inventory,drop,item,monster}.md로 평탄화, 참조 20여 곳 재배선 (링크 라벨 포함).

- **시점은 1인칭 확정** — 카메라는 `EyeHeight` 156cm. segkind의 3인칭 카메라 충돌 검사는
  관전 등 3인칭 상황 전용으로 주석. character.md(plan)·systems/character.md에 명기.

- **systems/character.md 신설** (마지막 시스템 공백) — 커스텀 CMC 설계: 달리기 `FSavedMove`
  압축 플래그 패턴, **스태미나는 CMC 틱 안에서 결정론 시뮬**(RPC 없음 — 서버·자율 프록시
  동일 공식+표준 보정), 탈진 래치·재개 히스테리시스·실이동 조건의 구현 위치, 유효 무게
  감속은 캐릭터 복제 변수로 CMC가 읽음, 점프=장애물 통과(판정 평면), 피격·넘어짐·관전,
  build-order ① 스텝 매핑.

- **새 세션 적대 리뷰(review-2026-09-01.md) 39건 처리** — 29건은 세션 내 리뷰와 중복
  (세 독립 리뷰어가 같은 결론 = 교차 검증 성공), 신규·잔여 10건 추가 수정:
  + **site.md에 `ValueRate` 컬럼 신설** (양계장 1.0 — 가격식의 목적지 배율이 정본에 없었음)
  + `FCL9SpawnEntry` 필드명 `Alias`로 통일 (site·raiding — L1류 드리프트 재발 차단)
  + 거위 기본 스폰 "0" → "0~1" (site.md 기준값과 통일)
  + conventions § 4 표에서 "데이터 표" 제거 (데이터 흐름 재정의의 마지막 잔재)
  + inventory·shop "하지 않는 것" 헤딩 → "현재 범위에 없는 것 (이전 판과의 차이)"
  + 호스트 크래시 = 마지막 새벽 세이브 복귀 명시 (밤 진행분 소실 — session.md)
  + `UCL9ItemGimmick` → `UCL9Gimmick` 통일, `ECL9ItemType`의 `Weapon` 삭제 (기획에 무기 없음),
    캐릭터·몬스터 메시 필드 `SkeletalMesh`로 개명 (베이스 `Mesh`와 이름 충돌 해소),
    부등식 A3 결번 각주.


## 2026-09-02

- **캐릭터 딥 체크(9건 보고) 후 사용자 결정 6건 반영** (① 캐릭터 개발 착수 준비):
  + **숫자 전수 감사** — 전 문서의 수치를 근거·공식과 대조 (40여 건 ✓). 발견 4건 수정:
    점프 정점 130→**156cm** 재계산 정정(780²/(2×1950)), 발각 게이지 **만충=1.0(0~1 정규화)**
    명시, 스태미나 **0~1 정규화** 명시(StaminaMinResume 0.25의 단위), 닭 TurnDrag 1.38초는
    "구 CCL8 실측 — ③ 재검증" 표시. 부산물: **plan/economy.md에 "파생값 정의" 표 신설**
    (지친 빈손 지속 속도 506.6 = (654×6+396×8)/14 공식 문서화, 질주 예금 3924cm,
    운반 질주·탈진 운반은 무게 식 확정 후).
  + **데미지 수신은 `ICL9Damageable::OnDamage(Amount, Source)` 가상 함수 하나로** —
    TakeDamage/UDamageType 대체의 구체 형태. 캐릭터·몬스터 베이스가 달라(ACharacter vs
    ACL9ActorBase) 인터페이스로. 호스트 판정 코드에서만 호출 (conventions § 6 갱신).
  + **슬롯 배정은 입장 순서** — 호스트 0, 접속 순 1·2·3. 이탈 슬롯은 비고 새 입장자가 빈
    슬롯 중 최저 번호(초안). PlayerState에 `SlotIndex` (session.md).
  + **흔들림 설계** — 게이지는 CMC 틱 시뮬(스태미나와 같은 예측 경로), **만충 순간 손의
    취약물이 금 간다(초안 — 이미 금 갔으면 깨짐)**, 표현은 게이지 값으로 스케일하는 1인칭
    카메라 셰이크(바 없음 유지). 타인 시점은 상태 파생 애님만 (character/inventory/rules).
  + **1인칭은 손만 보인다** — FPP 팔 전용 메시+전용 AnimBP, 든 오브젝트는 손 소켓 부착.
    타인 시점은 풀바디+CarryHeight 그대로 (character/animation/inventory. CharacterDef에
    팔 메시·팔 AnimBP 컬럼은 1c에서).
  + **잡히면 짐 흘림은 손+품 전부 (초안)** (character/monster).
  + 부수: 캐릭터 Def 적용 경로 명문화(SelectedCharacter→호스트 판정 값→OnRep 표현 —
    character.md), C급 미결 3건 기록(낙하 데미지·먹기 IA·발 소음 구조 — character.md 미결).

- **2차 새 세션 적대 리뷰(review-2026-09-02.md) 14건 — 건별 검증 후 선별 적용** (사용자
  지시: "무조건 다 고치지 말고 검증해서 적용할 것만"). 12건 반영, 1건 부분, 1건 기처리:
  + **H-1 (반영)** — CMC 예측 예외가 overview·build-order에 미전파돼 정면 모순이던 것 갱신
    ("상태를 Character/CMC에 두지 않는다" → 이동 예측 상태는 FSavedMove로 CMC 경로 예외).
  + **M-1 (반영)** — plan/README "기술 설계 문서가 아직 없다" 거짓 현재형 → conventions·
    systems/ 포인터로. 1차 리뷰 처리 헤더의 "39건 전건 닫힘" 주장도 37건으로 정정.
  + **M-2 (반영)** — conventions § 10에 BeginPlay 초기 로드 경로·`OnRep_AliasPair` 반영
    (§10만 보면 레벨 배치 액터 클라 로드가 죽는 구멍), actor-base 사용 예 주석 동조.
  + **M-3 (반영)** — 부등식 A4·탈진 운반 행도 A1과 같은 "잡는(Catch) 몬스터" 한정으로 통일.
  + **L-3~L-10 (반영)** — animation.md `SkeletalMesh` 개명 동조, settings.md VO 슬라이더 행,
    cheats.md 라우팅 단일화(RouteToHost 삭제 — PC RPC 직행 하나로), sequence.md "OnRep
    수신부" 잔재 정정, **스태미나 "표준 보정" 주장 완화**(ClientAdjustPosition 페이로드에
    커스텀 상태 없음 — 드리프트 관측 시 MoveResponse 확장 검토), **schema.md `FCL9ItemRef`
    신설**(CookResult/BurnResult의 타입 공백 — FCL9ItemCount가 이를 상속), decisions.md
    로그 훼손 복구(융합 불릿 분리·몬스터 컬럼 항목을 실제 시간순 위치로), L-5는 결정 대신
    "미결" 표시(관전이 1인칭 초안이라 카메라 충돌 검사 용처가 붕 뜬 것 — segkind 주석).
  + **L-2 (부분)** — "금지/만들지 않는다" 계열 7곳은 현재 설계 서술로 치환(warning·balance·
    loop·cooking). **"유일한/하나뿐" 계열은 유지** — 과장이 아니라 현 설계의 사실 서술
    (회복 채널이 실제로 트럭 패드 하나)이라는 판단.
  + **L-1 (기처리)** — 링크 라벨 구 경로 32곳은 리뷰 도착 전 같은 날 자체 발견·수정.

- **프로젝트 실상태 확인 + 보유 에셋 기록** (① 착수 직전 점검). CL9은 문서 전용이 아니라
  **UE 5.8 1인칭 템플릿 프로젝트로 이미 생성돼 있다** (템플릿 코드 CL9Character 등 +
  Variant_Horror/Shooter — CL9 게임 코드는 아직 없음, CLAUDE.md 정정). Content에서 확인한
  보유 에셋을 animation.md "보유 에셋" 절에 기록:
  + 마네킹(풀바디+Death/HitReact/Attack 애님)·FPP 팔 — 1c 임시 메시·1인칭 손 계획과 일치.
  + **Dreamscape Farmlands 팩** — 닭 `SK_Chicken`+애님 8종이 몬스터 상태 기계 요구를 전수
    커버 (chicken.md 에셋 절에 대조 표 — ③은 임시 메시 없이 실제 스켈레톤으로 시작).
    돼지·양은 예비. 농장 환경·Horror 조명 깜빡임은 5a 맵·light.md 참고 자원.
  + 거위·너구리 메시는 없음 — 거위는 전용 에셋 확보 전까지 SK_Chicken 변형으로 ③ 가능(초안).

- **Unreal MCP 세팅 — CCL8 검증 레시피 이식** (① 착수 준비). UE 5.8 엔진 실험 플러그인
  `ModelContextProtocol`(Epic 공식) + `AllToolsets` + `PythonScriptPlugin`을 CL9.uproject에
  활성화하고, `.mcp.json`(unreal-mcp — http://127.0.0.1:8000/mcp)을 프로젝트 루트에 생성.
  에디터가 떠 있어야 연결된다 — 세션 시작 시 .mcp.json이 읽히므로 파일은 미리 둔다.

## 2026-09-02

- **수레 적재 규칙 확정** (② 2f — cart.md가 위임한 미결). 조작: **끌면서 R(교체 키)로 싣고
  내린다** — 끌고 있는 동안 R은 품 대신 수레와 교체하고, 같은 `SwapTime` 잠금을 탄다 (운반
  마찰 일원화). **수레 놓기는 빈손 E**(끌리는 수레는 등 뒤 고정+충돌 꺼짐이라 조준·감지가
  불가능해서, 빈손 E에 다른 후보가 없을 때가 놓기 제스처다). **항아리에서 E 한 번이면 손
  물건과 수레 적재물 전부가 반납된다** (수레째 반납 — "둘이 같이 나르고 반납한다"의 실행).
  **적재 상한은 없다** — "무게 초과로 추가 불가 같은 상한 규칙이 없다"(inventory.md)와 같은
  철학이고, 대가는 무게→감속 식이 확정되면 감속으로 온다. 적재물은 실물 액터를 수레 짐칸에
  부착한다 (품과 달리 파괴하지 않음 — 눈에 보이는 게 수레의 존재 이유).

- **상호작용 단일화 + 손 배타 (수레 적재 조작 번복)** — 위 항목의 "끌면서 R로 싣고 내린다"를
  같은 날 플레이 피드백으로 번복한다. **E 하나가 모든 상호작용이다**: 조준한 대상이 곧 대상
  (빈손+수레=끌기, 빈손+아이템=집기, 들고+수레=싣기, 들고+항아리=반납, 조준 없음이면
  들고=놓기·끌던 중=수레 놓기). **내리기는 짐칸 위 아이템을 직접 조준해 집는다** (짐칸
  아이템은 충돌 쿼리를 유지해 조준 가능). **손은 배타다** — 아이템 들기와 수레 끌기는 동시에
  불가하고, 끄는 동안 품 조작(R)도 잠긴다. R은 품 교체 전용으로 돌아간다. **조준 대상은
  오버레이 머티리얼로 하이라이트**되어 E가 무엇에 적용될지 항상 보인다 (로컬 표현 —
  판정은 호스트가 같은 해석기를 다시 돈다).

- **수레 견인은 순간이동 없는 거리 유지 트레일링** (같은 날 피드백). 끌기 시작이 수레
  위치를 옮기지 않는다 — 부착(등 뒤 스냅)을 폐기하고, 수레는 제자리에서 연결되어 끄는
  사람이 견인 거리(캡슐+수레 반경+20cm)를 넘어서는 만큼만 끌려온다. 제자리 회전으로는
  수레가 돌지 않는다. 끌리는 수레는 조준 가능한 채로 남아 조준 E=놓기, 들고 조준 E=싣기가
  된다 (빈손 E 허공 = 놓기도 유지).

- **표 구조: 행별 에셋 → 테이블 에셋의 인라인 행** (사용자 피드백 — "엑셀처럼 한 표에서
  행 추가"). "종류 하나 = DataAsset 하나"(8-31 A3 계열)를 번복한다. `UCL9DefTable` 에셋
  하나가 `Rows`(Instanced `UCL9DefBase` 배열)를 품고, 행 추가는 그 배열에 +다 —
  `ItemTable`을 열면 전 아이템이 한 화면에 보인다. UDataTable을 기각했던 근거(Instanced
  기믹이 구조체 행에서 불가)는 그대로 유효하고, 인라인 행은 UObject라 기믹이 살아 있다.
  독립 에셋 행도 레지스트리가 여전히 받는다(규칙 에셋 등). DefList 자동 스캔은 테이블을
  줍는다.

- **③ 경로 탐색 방식 확정: NavMesh 쿼리 + 커스텀 이동** (monster.md 미결 해소).
  UE NavMesh(RuntimeGeneration=Dynamic)에서 웨이포인트만 얻고, 이동은 몬스터가 직접
  건는다 — 헤딩이 TurnRate로 목표를 쫓고, 남은 꺾임각/90°에 비례해 TurnDrag만큼 감속하는
  턴 모델 (코너가 시간을 버는 구조). 층 봉쇄·일방향 개구멍(flapIn)은 맵 생성기(⑤)에서
  NavArea/링크로 다룰 수 있다고 판단 — 감당 못 하면 그때 커스텀 그래프로 재검토.
  클라 표시는 20Hz 스냅 위에 메시 보간(VInterp)으로 부드럽게 한다 (3c).

## 2026-09-03

- **몬스터-플레이어 몸 충돌은 서로 통과** (사용자 확정). economy.md의 "몬스터 수 ↑ →
  통로가 막힌다"는 물리 충돌이 아니라 **위협에 의한 봉쇄**로 해석한다 — 255cm 통로에서
  닭 옆을 지나면 근접 감지(120cm, 2.5/s)로 0.4초 만에 발각되므로 몸이 겹쳐도 함부로
  못 지나간다. 물리 차단은 벽(WorldStatic)만 한다.
- **넘어짐 이동 차단은 CMC 최대 속도 0 방식** (3e — character.md가 열어둔 "CMC 커스텀
  모드 vs 입력 차단" 중 선택). 캐릭터의 Replicated `KnockdownEndServerTime`을 서버·클라
  CMC가 같이 읽어 속도 0을 도출한다 — CartMult와 같은 경로라 새 예측 코드가 없다.
  `KnockdownTime`은 캐릭터 표 컬럼으로 신설 (초안 1.5초, data/character.md).
- **짐 흘림은 발목 높이에서 실물리 산개** (3e 초안). 가슴 높이에서 흘리면 낙하 충격이
  CrackSpeed를 넘어 취약물이 전멸한다 — 잡힘 한 번이 "짐이 흩어진다"이지 "짐이 전부
  깨진다"가 되지 않게 낮게 흘린다. 손 실물 + 품 데이터 재스폰 전부.
- **몬스터 죽음 연출 임시안**: 종별 ABP에 죽음 스테이트가 생기기 전까지 포즈 동결
  (GlobalAnimRateScale 0) + 옆으로 눕힘(롤 90°)으로 시체를 표현한다 — 전 머신 공통,
  에셋 0개. 잡기 공격 모션도 같은 이유로 아직 없음 (Catch 상태 0.8초 정지가 자리만 잡는다).
- **거위 임시 메시 = SK_Chicken** (3g). 보유 팩에 거위가 없다 — 행 값(속도 528·반경 45·
  경보꾼·잠듦)은 전부 거위지만 겉모습은 닭을 빌린다. 너구리 = AVP Fox (raccoon.md).
  둘 다 ⑤에서 갈아끼운다 (원칙 2).

## 2026-09-05

- **로코모션 재생 배율 표준** (사용자 확정 — "앞으로 모든 애니메이션 만들 때 이런 식으로
  맞추는 걸 기본으로"). 이동 애님은 `PlayRate = 속도 ÷ 기준 속도`로 구동하고, 블렌드 문턱은
  테이블 실측 속도와 정합시킨다(배회=완전 걷기, 추격=완전 달리기) — 풋 슬라이딩 방지.
  전문은 systems/animation.md "로코모션 재생 배율". ABP_Chicken·ABP_Fox에 적용 완료
  (기준 속도 초안: 닭 165/396, 여우 210/462 — PIE 보정 예정).

- **5c(조명·손전등·정전) 앞당김** (사용자 확정 — "밤 되면 손전등이 필요하니까").
  밤 수명주기와 묶어 평지 단계에서 구현한다: 페이즈 연동 어둠 렌더(태양·SkyLight 전환 +
  노출 수동 고정) → 손전등 아이템(스포트라이트 + IA_Flashlight 토글, 복제) → 정전 연출.
  build-order ⑤의 5c "진짜 맵 조명"은 그때 값 재사용. 밝기 값은 에셋에서 읽는다 (원칙 3).

- **UI 구조 번복: WidgetSwitcher → UIManager(레이어 + UI 표)** (사용자 확정 — 비교 전문은
  review/ui-structure-2026-09-05.md). ui_system.md의 "전면 화면은 WidgetSwitcher, 화면
  enum 하나" 설계를 버리고, 이전 프로젝트 NPCSimulation의 구조를 CL9 규약에 맞게 이식한다.
  "뷰포트에 붙는 위젯은 메인 위젯 하나"는 유지하되, 그 안의 레이어 패널에 **UI 표**
  (`UCL9UITable` — FName 키 → 위젯 클래스·레이어)를 읽는 UIManager가 위젯을 **열 때 생성해**
  붙인다. UI 추가 = WBP 제작 + 표에 행 추가 (C++ 무접촉 — "표에 행 추가" 원칙 4와 같은 결).
  이식하며 고친 것 두 가지: 하드코딩 UI 표 경로 → BP_CL9GameInstance 프로퍼티 주입,
  `AddToRoot` 싱글턴 → GameInstanceSubsystem (싱글턴 접근 `Get()`은 유지 — 사용자 허용).
  Screen 레이어는 배타(전면 화면 동시 1장 기획은 레이어 정책으로 지켜진다), Popup 레이어는
  중첩 허용. 전문은 systems/ui_system.md.

- **셰이크 등록을 규칙 컬럼 → 셰이크 표로** (사용자 지시). `UCL9RulesDef`의 `StaminaShake`·
  `StaminaShakeStart` 두 컬럼을 버리고 `UCL9ShakeTable`(FName → {셰이크 에셋, 발동 문턱})을
  신설한다 — UI 표와 같은 모양이고, 흔들림이 늘어도(운반·착지·피격) 규칙 에셋에 컬럼이 늘지
  않고 표에 행만 추가된다. 진폭·주파수는 여전히 셰이크 에셋 안에서 튜닝한다(표에 강도 컬럼을
  두지 않는다). 에셋은 `Content/Effects/ShakeTable`, 주입은 BP_CL9GameInstance.

- **스태미나 표시 번복: 흔들림 + HUD 바 병행** (사용자 지시 — "흔들림도 있고 UI 표시도
  필요해"). rules.md의 "스태미나는 바 없이 화면 흔들림으로만"을 스태미나에 한해 번복한다.
  흔들림만으로는 **남은 양과 탈진 래치 해제 시점**(StaminaMinResume 0.25)이 안 읽힌다는
  판단이다. HUD에 정규화 바를 두고 탈진 중에는 색을 바꾼다. **운반 흔들림에는 이 예외를
  적용하지 않는다** — 그쪽은 "관리 자원화 방지" 논리가 그대로 유효하다.
  스태미나는 CMC 틱 시뮬 값이라 변경 이벤트가 없어 HUD가 폴링한다 — OnRep 원칙의 유일한
  예외로 두고 넓히지 않는다 (systems/ui_system.md).

- **탈진 잠금은 만충까지 — `StaminaMinResume` 0.25 → 1.0** (사용자 지시 — "스태미나가 0이
  되면 lock이 걸려서 풀로 올라갈 때까지 못 쓰게 막힘, 보통 스태미나 게임에 있는 시스템").
  래치 자체는 이미 있던 것이고 문턱 값만 올렸다 — 코드 변경 없음. 이제 0에 닿으면 8초
  (`StaminaRegenTime`) 동안 걸어야 다시 달릴 수 있다. 회복은 정지뿐 아니라 **걷는 중에도**
  되므로 "걸어서 도망치며 회복"이라는 탈출 경로는 그대로다. A1 관계 검사는 원래부터
  만충 사이클((RunSpeed×DrainTime + WalkSpeed×RegenTime)/(DrainTime+RegenTime))을 전제로
  계산하므로 이 변경으로 값이 달라지지 않는다 (실측 506.6 유지, 검사 통과).

- **4a 죽음·관전·트럭 패드 회복 구현** (build-order ④ 4a). 정본 서술 그대로 — 부활은 없고
  회복 채널은 트럭 패드 하나. 구현 결정: ① 죽은 폰은 그 자리에 누운 채 남고 컨트롤러는
  빙의를 유지한다 — 관전은 **뷰 타깃 교체**(호스트 `SetViewTarget(생존 동료 폰)` → 엔진의
  ClientSetViewTarget)이지 관전 폰 스폰이 아니다. 동료 1인칭 그대로(초안 유지). ② 관전
  진입/해제는 PlayerController의 `bSpectating`(ReplicatedUsing) 하나로, OnRep이 관전 화면
  (UI 표 `Spectate` 행)과 `IMC_Spectate`를 함께 켠다 — 화면·IMC를 한 곳에서 묶는다는
  input.md 원칙. ③ 전멸(생존자 0)은 밤 중이면 즉시 `EndNight()` — "트럭이 일행을 두고
  떠난다". ④ 죽은 사람은 다음 로비(`EnterLobby`)에서 트럭 패드 위치에 만HP로 복귀. ⑤ 트럭
  패드는 `TruckPadHealTime`초 **연속** 체류 → HP 만회복(살아 있고 다친 사람만, 벗어나면
  초기화). ⑥ `MaxHp`를 PlayerState에 추가(회복 목표). 값 초안: `SpectateDelay` 3초 ·
  `TruckPadHealTime` 5초 — 플레이해보고 조정(rules.md 미결). 치트 `cl9.damage`.
  **미결**: 관전 중 보이는 동료 폰의 넘어짐 카메라 낙하는 소유자 전용 틱이라 관전자에겐
  안 보인다(폴리시). 1인 플레이는 죽음 = 전멸이라 관전이 없다(설계상 맞음 — 관전 검증은
  2인 PIE).

- **HUD 2차 — 상호작용 프롬프트는 이벤트, 차징 게이지는 틱 예외** (cl9-7d, PIE 검증 통과).
  ① 프롬프트 문구는 캐릭터가 포커스 틱에서 `GetFocusPrompt()`로 조립하고(Server_PickPlace 분기
  순서: 줍기 / Interactable→GetPrompt / 놓기 / 수레 놓기 / 없음) **문구가 바뀔 때만**
  `OnFocusPromptChanged`를 쏜다 — 액터 변경 기준을 버린 이유는 수레처럼 같은 액터인데 상태에 따라
  문구만 바뀌는 경우("[E] 수레 끌기"→"[E] 수레 놓기") 때문. HUD는 문구를 만들지 않는다.
  ② 던지기 차징 게이지는 버튼을 누른 시간의 함수라 차징 중 매 프레임 `GetThrowChargeAlpha()`
  (로컬 예측)를 읽는다 — ui_system.md의 "위젯은 폴링하지 않는다" 예외를 스태미나 바 하나에서
  **로컬 입력 피드백 둘**(스태미나·차징)로 넓혔다. 복제 값에는 여전히 예외 없음.
  ③ 게터 3건은 게임플레이 세션이 ACL9Character에 넣고 UI 세션은 Source/CL9/UI만 고친다
  (사용자: "우리 세션은 UMG 세션").

- **Esc는 게임 키에서 제외 — 일시정지 열기·닫기 모두 M** (사용자 확정 — "esc말고 다 m으로 하자,
  esc하면 에디터가 꺼지잖아"). 같은 날 앞선 "열기 Esc→M, 닫기는 Esc 유지(고정 키)" 결정을
  번복한다. IMC_Gameplay·IMC_UI는 이미 M→IA_Pause 한 행씩이라 에셋 변경 없음, input.md의 고정 키
  조항 삭제. 코드 `PauseInput`은 "열려 있으면 닫고 아니면 열기"라 그대로.

- **헬멧 헤드램프 — 착용형 광원 추가** (사용자 지시 — "손전등이 아니라 헬멧에 있는 후레시.
  후레시 켜고 물건도 들 수 있어야 하는데 헬멧 후레시가 더 자연스럽다"). 2026-09-01의
  "빛은 손에 드는 아이템"(빛 vs 짐 교환) 결정을 **부분 번복**한다: 손전등 아이템은 그대로
  남기고(아이템 중 하나), **트럭에서 사는 장비 `headlamp`**(plan/data/item/headlamp.md)를 새로 둔다.
  주우면(E) 머리에 쓰이고 손은 빈다 — `CarrySlot::Head`, 착용형 장비 "없음" 범위의 유일한 예외.
  4f 이전 테스트에서는 바닥에 놓아 준다(Lvl_Flat 패드 옆 + `cl9.additem headlamp`).
  **정전은 헤드램프를 끄지 않는다** — "헤드램프는 내 것이고, 정전은 맵 내부 시스템만 죽인다"
  (사용자). 손전등의 정전 소등 규칙은 그대로다(사용자 답변이 헤드램프에 한정된 것으로 읽음 —
  손전등까지 바꿀지는 미결). 구현: 아이템 HoldState `Worn`(1인칭 카메라 부착·OwnerNoSee·충돌 없음),
  F는 쓴 램프 우선, PlayerState `HeadItem` 미러. 벗기·타인 화면 피치 추종·가격은 미결.

- **4b 일자 진행 곡선 구현** (build-order ④ 4b). 정본 두 층 그대로 — 목적지(`UCL9SiteDef`,
  MapTable `henhouse`: 닭 8~12·거위 0~1·계란 16~20·고기 8~12) × 일자 곡선(`UCL9ProgressionDef`
  단일 에셋). 구현 결정: ① 곡선은 별도 `UCurveFloat` 에셋 대신 **`FRuntimeFloatCurve` 인라인**
  (같은 그래프 편집, 외부 곡선 참조도 가능, 에셋 수 절약). ② 밤 시작 스폰은 맵 생성기 전까지
  **평지 스탠드인**: 트럭 패드(없으면 PlayerStart)를 앵커로 몬스터 900~2400cm·재료 400~1800cm
  원환에 무작위 배치, 플레이어 `ThreatActMinRadius` 안은 피한다. ③ 로비 복귀 시 밤 스폰분만
  철거(들고 나온 것·치트 스폰·레벨 배치물은 남긴다). ④ DangerCurve는 GameState `DangerScale`
  하나로 실어 몬스터의 ChaseBudget·DetectRate·Sight 세 읽기 지점에 곱한다(표 값 불변).
  ⑤ 1일차 덮어쓰기: 정전 없음. 곡선 초안 값은 progression.md 그대로. 미구현: ValueCurve·
  QuotaCurve(4e), 주문서 컬럼(4c), ForcedSite(4g).

- **4c 주문서 구현 — 게임플레이 절반** (build-order ④ 4c). `UCL9OrderDef`(OrderTable: omelette·
  fried_egg·meat_dish 초안 값) + 진행 에셋의 N/M(3·2). 로비마다(게임 시작·로비 진입·cl9.day) 호스트가
  MinDay/MaxDay 필터 → Weight 가중 무중복 추첨으로 N개를 제시하고, **호스트 컨트롤러만** 정확히 M개를
  확정한다(`AcceptOrders(indices)` Server RPC — 클라 요청은 거부). 수락 주문은 `AcceptedOrders{alias,
  받은 날, 마감 일차 = 받은 날 + DeadlineDays − 1}`로 GameState에 누적·복제되고 제시는 비워진다.
  구현 결정: ① 주문을 고르지 않아도 밤은 시작된다 — 선택 화면이 생기기 전 개발 편의, 4e(납품·위약금)
  에서 "M개 미선택 시 밤 시작 금지"를 재검토. ② 납품·보상·위약금·세이브는 4e. ③ 주문 선택 화면과
  HUD 진행 중 주문 표시는 UI 세션(OnOrdersChanged 델리게이트 + OfferedOrders/AcceptedOrders 읽기).
  치트 `cl9.orders`·`cl9.pick`.

- **UI 전역 배율 1.4** (사용자 — "UI들 대체적으로 작은데 좀 크게"). 위젯별 글자 크기를 손대지 않고 프로젝트
  설정 `UserInterfaceSettings.ApplicationScale`을 1.0→1.4로 올렸다(Config/DefaultEngine.ini). 이후 전체 크기는
  이 값 하나로 조정하고, 위젯은 1080p·배율 1.0 기준으로 만든다 (ui_system.md 구조).

- **돼지·양을 몬스터 표에 올림 — 시스템 검증용** (2026-09-05, 사용자 — "몬스터 시스템이 잘 돌아가는지 봐야
  되니까"). 기획에 없는 종이지만 보유 팩에 풀셋이 있어 `MonsterTable`에 `pig`·`sheep` 행(값은 닭 복사)과
  `ABP_Pig`·`ABP_Sheep`(ABP_Chicken 복제 → 스켈레톤·애님 교체)을 추가했다. 목적은 "새 종 = 행 + AnimBP"
  틀의 실증이지 기획 편입이 아니다 — 역할·값은 미결([pig.md](plan/data/monster/pig.md)·
  [sheep.md](plan/data/monster/sheep.md)). 같이 `ABP_Fox`에 여우 팩의 휴식·기상 쌍을 연결해 너구리도 8종
  커버리지를 갖췄다 (SleepTime 0이라 지금은 안 잔다).

- **맵 생성기 5a를 4c 다음에 당김 — 새로 작성, 1층+지하 한 번에, 외곽은 고정** (2026-09-06, 사용자 — "이제 맵을
  만들자"). 빌드 오더 ⑤ 5a를 4d 앞에 넣었다. 결정: ① CCL8의 검증된 생성기(Source/Gen, 골든 해시)를 이식하지
  않고 site.md 컬럼만 보고 새로 짰다 — Kruskal 신장 트리 + TargetOpen까지 추가 개방(전 셀 연결 보장). ② 지하·
  계단·일방향 개구멍까지 첫 버전에 포함. ③ 마당·축사 외곽·계단은 시드와 무관한 고정 기하지만 손 배치가 아니라
  **코드가 같이 깐다**(.umap 비대 회피, 5b에서 외곽만 손 메시로 교체 가능). ④ 시드 0 = 외곽만 — 로비에서도
  트럭 옆에 축사가 서 있고 밤에만 미로가 찬다. ⑤ 지하는 1층 동쪽에 나란히 450cm 아래 구덩이로 (평지 바닥
  Floor_0는 −500 아래로 내림). ⑥ 주방 셀은 뺐다(요리는 트럭). ⑦ 5a "창"은 미루고 에디터 콘솔 `cl9.seed`가 같은
  코드를 돈다. ⑧ 조각 표에 `floor` 12번째 행 추가. 근거·상세: systems/map.md 구현 상태.

- **평지 테스트 레벨과 생성 맵 레벨 분리** (2026-09-06, 사용자 — "원래 쓰던 그 평지는 어디감?"). 맵 생성기를 붙이며
  `Lvl_Flat`의 바닥을 내리고 3a 회색 벽을 지웠던 로컬 변경을 되돌리고, 생성 맵은 `Content/Maps/Lvl_Henhouse`(Lvl_Flat
  복제, Floor_0 −500)에서 쓴다. 레벨별 생성 여부는 코드가 아니라 **목적지 행**으로 고른다: 목적지 표에 테스트 전용
  `flat` 행(`Generator=None`)을 두고 `BP_CL9GameMode_Flat`(BP_CL9GameMode 자식, DefaultSiteAlias=flat)을 Lvl_Flat
  월드 세팅에 건다. 레벨 폴더는 기존 `Content/Maps` 하나로 모은다(별도 Lv 폴더 없음). 두 .umap은 정책대로 미커밋.

- **로비부터 오늘 밤 미로를 세운다** (2026-09-06, 사용자 — "게임 시작했을 때랑 기다릴 때랑 맵이 너무 다른데").
  처음 구현은 로비 = 외곽만(시드 0), 밤 시작 = 미로였는데 정차 중 빈 축사와 넓은 바닥만 보여 어색했다. 시드를
  로비 진입(게임 시작·로비 복귀) 때 굴려 미로를 세우고 밤 시작은 그 시드를 그대로 쓴다. "밤마다 새 맵"은 유지된다 —
  다음 로비에서 새 시드로 바뀐다. 시드 0(외곽만)은 `cl9.seed 0` 치트 전용.

- **축사에 천장을 씌운다** (2026-09-06, 사용자 — "화면이 밝고 스카이박스가 밝아, 천장을 막자, 건물 안에 들어가는
  느낌"). 조각 표에 `ceiling` 행(13번째)을 두고 생성기가 1층(바닥 위 630)·계단 복도·지하(지면 높이) 천장을 깐다.
  천장 높이는 벽 Height와 별개 상수(`BarnCeiling`)다 — segkind.md "높이 210은 천장이 아니다" 규칙 그대로. 마당은
  여전히 하늘이 열려 있고 로비 조명은 낮이다 — 실외 밝기는 별도 건.

- **4d 요리 — 팬이 곧 열원** (2026-09-06, build-order ④ 4d). `ACL9FryingPan`(아이템 표 `frying_pan` 행, Tool)이 위에
  놓인 재료를 `CookResult`/`BurnResult`·`CookTime`/`BurnTime`대로 `SetAlias`로 바꾼다 — cooking.md의 구현 구조 그대로,
  새 조작 없음. 결정: ① 버너(불)는 4f 트럭 주방과 함께 — 지금은 팬이 놓인 자리에서 늘 뜨겁다. ② 연기 VFX·소리는
  에셋이 생길 때(표현 계층). ③ 팬 상판은 별도 상자 콜리전(구 콜라이더 위에선 재료가 굴러 떨어져서). ④ 프라이팬은
  구매(4f) 전이라 `cl9.additem frying_pan`으로 꺼낸다(헤드램프와 같은 방식).

- **레벨(.umap)도 커밋한다 — 100MB 근처면 커밋 전 확인** (2026-09-06, 사용자). "Lvl_Flat.umap 미커밋" 규칙은 CCL8의 177MB
  사고를 피하려는 예방책이었는데 지금 두 레벨은 84KB·74KB다. 규칙을 "커밋 전 크기 확인, 100MB 근처면 원인부터 제거"로
  바꾸고 Lvl_Flat·Lvl_Henhouse를 커밋한다. 바이너리라 병합 불가 — 레벨 편집은 한 세션(게임플레이 세션)이 맡는다.

## 2026-09-06

- **우클릭 스캔 도입** (사용자 — 리썰 컴퍼니 스캔 참고: "우클릭하면 이펙트 지나가면서 아이템 정보").
  로컬 표현 전용: PlayerController `IA_Scan` → `OnScanRequested` → HUD가 파동 링(UMG Border 렌더 스케일)과
  라벨 풀(`WBP_ScanLabel`)로 표현. 판정·RPC 없음, 벽 뒤는 시야 트레이스로 제외, 값(반경 15m · 0.8s · 3s ·
  12개)은 WBP_HUD 프로퍼티. 포스트 프로세스 대신 UMG 링을 쓴 이유: 에셋 없이 즉시, 배율·해상도 무관.
- **주문 선택 화면 열기는 HUD가 상태에서 파생** — OfferedOrders 유무로 열고 닫는다(ui_system.md). 카드는
  런타임 생성이라 주문 수가 바뀌어도 WBP를 안 고친다. [확정]은 호스트만(HasAuthority).
- **코드 주석은 한글로 쓴다** (사용자 — "앞으로 모든 주석은 한글로 써줘. 왜 영어로 썼는지 모르겠음").
  Epic 공식 표준을 옮긴 [ue-cpp-conventions.md](ue-cpp-conventions.md) "코드와 코멘트는 미국 영어
  철자법·문법을 따라라" 방침을 이 프로젝트에서는 코멘트에 한해 뒤집는다 — 코드 식별자(변수·함수·
  클래스명)는 계속 영어 PascalCase를 쓰고, 그 위에 다는 설명(`//`, `/* */`, `/** */`)만 한글 문장으로
  쓴다. 이 결정 전에 이미 있던 영어 주석은 전량 한글로 일괄 번역했다 (Copyright Epic Games 고지
  줄만 예외로 원문 유지).

- **아이템 가치는 개체별 굴림값을 보여 준다** (2026-09-06, 사용자 — "50~80이 아니라 57처럼"). `ACL9Item`에 복제되는
  `Value`를 두고 호스트가 행 로드 때 `[ValueMin, ValueMax]`에서 굴린다. 상태 전환(익음·탐·깨짐)마다 새 행 범위로
  **다시 굴린다**(비율 유지가 아님 — 단순함 우선). 목적지·일자 배율은 판매·납품(4e)에서 곱한다. 스캔 라벨(UI 세션)은
  범위 대신 `GetValue()`를 표시한다.

- **액터 종류 열거형 `ECL9ActorKind`(None/Item/Monster)를 둔다** (2026-09-06, 사용자 — "몬스터인지 아이템인지 enum 하나 넣어줘,
  생성자에서"). `ACL9ActorBase::Kind`를 아이템·몬스터 클래스 생성자가 정하고 `GetKind()`로 읽는다 — 클래스마다 고정이라
  복제하지 않는다. 첫 사용처는 스캔: WBP_Scan의 `ScanKinds` 목록에 든 종류만 라벨이 붙는다(기본 아이템만; 몬스터를 더하면
  몬스터도, 몬스터 라벨은 이름만). "무엇을 보여줄지"를 코드 분기가 아니라 표 값으로 고르기 위해서다.

- **주문 보상은 수락하는 순간 굴려 확정한다** (2026-09-06, 사용자 — 아이템 값 굴림과 같은 방식). `FCL9AcceptedOrder`에
  `Reward`를 두고 `[RewardMin, RewardMax]`에서 한 번만 굴린다. 제시 카드는 아직 범위를 보여주고(고를 때의 정보),
  수락한 뒤에는 실제로 받을 값이 뜬다 — 위약금(`보상 × FailPenaltyRate`)도 같은 값에서 나오므로 "얼마를 놓쳤는지"가
  숫자로 맞아떨어진다. 일자·목적지 배율은 보상에 곱하지 않는다(그 두 배율은 물건 판매가의 것이다 — economy.md 가격식).

- **납품 창구는 임시 부엌 옆 상자로 시작한다** (2026-09-06, 사용자). 트럭에는 아직 납품 지점이 없고 4f 정식 주방·트럭
  패널이 오기 전까지의 자리다. `ACL9Counter`(ICL9Interactable): 손에 든 것과 끄는 수레의 짐을 받아 **보관**하고,
  보관분이 수락한 주문의 재료를 채우면 호스트가 그 자리에서 완납 처리한다(마감이 급한 주문부터, 재료는 채워지며
  빠지므로 이중 계산이 없다). 빈손으로 다시 상호작용하면 남은 보관분을 판다 — 그쪽이 가격식(굴림 × 목적지 ValueRate ×
  일자 ValueCurve)을 타는 경로다. 창구는 실물 액터를 남기지 않고 alias+sub+굴림값만 들고 있는다(복제) — 올린 물건을
  다시 집어갈 수 없으니 실물을 세워둘 이유가 없다.

- **일시정지는 없다 — M이 여는 것은 "메뉴"다** (2026-09-06, 사용자 확정: "일시정지는 없어야 돼, 온라인 게임이니까").
  Steam 리슨 서버 협동이라 한 사람이 세계를 멈출 수 없다. 사실 코드는 처음부터 멈추지 않았고(`SetPause` 호출이
  어디에도 없다 — `UpdateInputMode`가 `GameAndUI`로 바꾸고 IMC_Gameplay만 내린다) "일시정지"라는 이름만 남아
  거짓말을 하고 있었다. 화면 제목과 문서에서 그 말을 걷어내고 **메뉴**로 통일한다. 열려 있는 동안 조작만 끊기고
  캐릭터는 그 자리에 무방비로 서 있다는 것이 밤중에 메뉴를 여는 값이다 — 이 위험을 없애려고 솔로에서만 멈추게
  하는 예외도 두지 않는다(리슨 서버에는 언제든 참가자가 들어올 수 있고, 규칙이 둘로 갈라진다).
  에셋·입력 액션·UI 표 키의 `Pause`는 그대로 둔다(WBP_Pause·IA_Pause·`CL9UINames::Pause`) — 여는 키를 가리키는
  옛 이름일 뿐이고, 개명하면 게임플레이 쪽 PlayerController까지 건드려 빌드가 한 번 더 필요하다.

- **요리의 열원은 프라이팬이 아니라 부엌(조리대)다 — 4d 설계 번복** (2026-09-06, 사용자: "부엌 자체에서 요리가
  가능해야 돼, 조리대 1요리인 거지 프라이팬이나 냄비랑 다름. 그냥 프라이팬은 아이템 중 하나로 봐야 돼").
  4d를 구현할 때는 팬이 열원이었고 systems/cooking.md가 "동시 요리 수 = 보유한 조리 용기 수"였는데, **그 규칙을
  폐기한다.** 열원은 `ACL9Kitchen`(조리대)이고 **조리대 하나가 한 번에 하나를 익힌다** — 동시 요리 수는 조리대 수다.
  프라이팬·냄비는 조리 용기가 아니라 그냥 아이템이며, 전용 클래스 `ACL9FryingPan`은 삭제했다(행만 남는다).
  조리대는 지금 Lvl_Flat에 손으로 놓고, **나중에는 던전 안에 맵 생성기가 배치한다** — 요리하는 자리가 트럭이 아니라
  원정지 안에 있다는 뜻이다.

- **조리 진행도는 아이템에 실어 복제한다** (2026-09-06, UI 세션 요청 — "언제 집어야 하는지 알 방법이 없어 타는 걸
  막을 수가 없다"). 판정은 조리대가 하되 표현용 사본을 `ACL9Item`에 담는다: `GetCookProgress()`(0~1) ·
  `IsOnHeat()` · `IsNextBurnt()` · `GetCookPhase()`. 복제 실체는 `uint8`(0~255)과 enum 하나뿐이다 — 게이지에
  필요한 정밀도가 그 정도이고, 열원이 아니라 재료에 붙어 있어야 조리대가 던전으로 옮겨가도 표현이 따라간다.

- **맵 조각은 메시가 아니라 "역할"을 가리킨다 — 조각 표를 따로 뺀다** (2026-09-07, 사용자: "나중에 또 모델
  새로 사면 바꿔야 되거든", "여러 테마의 맵을 만들 거라 그거까지 고려해서"). `UCL9SegKindDef`의 `Mesh` 컬럼을
  역할 이름 `Piece`로 바꾸고, 실제 메시는 새 표 `UCL9PieceDef`(`PieceTable`)가 **역할 × 테마**로 갖는다
  (`Alias`=역할, `SubAlias`=테마, 테마 칸이 빈 행은 공용 폴백). 목적지 행에 `Theme` 컬럼이 붙었다.
  **에셋 팩 교체 = 조각 표 + 격자 숫자, C++ 0줄**이 목표다 — 전문은 systems/asset-swap.md.
  테마 차원을 "나중에"로 미루지 않고 지금 넣은 이유: 키가 이미 `UCL9DefBase`의 `Alias`/`SubAlias`라 추가 코드가
  ~15줄뿐인데, 나중에 넣으면 쌓인 조각 행을 전부 다시 키 매겨야 한다. 덤으로 레지스트리가 "이 목적지의 테마가
  생성기가 쓰는 역할을 다 갖췄는가"를 로드 때 검사해, 테마를 절반만 만들고 맵이 까맣게 나오는 사고를 막는다.

- **조각 치수는 어느 표에도 적지 않는다 — 메시 바운드에서 읽는다** (2026-09-07). 위 결정의 핵심이다.
  손으로 옮겨 적은 치수는 팩이 바뀌는 순간 조용히 틀린 값이 되어 "표만 갈아 끼우면 된다"를 무너뜨린다.
  `ACL9SiteMap`이 `UStaticMesh::GetBoundingBox()`로 크기와 피벗을 읽고, **벽 두께(`FCL9SiteFrame::Wall`)까지**
  `barn` 조각의 실측 두께에서 온다(`FCL9HenhouseParams::WallThickness`로 순수 생성기에 넘긴다 — `ComputeFrame`은
  엔진에 손대지 않는다는 성질을 지킨다).

- **타일링 배치는 세 축을 따로 다룬다** (2026-09-07). `Placement=Tile`일 때 **길이 축**은 `round(발자국/조각)`장을
  늘어놓고 나머지를 장수로 나눠 흡수하고, **두께 축은 늘리지 않고** 발자국 가운데에 세우며, **높이 축**은 단을 쌓되
  아래 단은 자연 높이를 지키고 남는 높이만 맨 윗단이 받는다. 두께를 안 늘리는 이유는 발자국의 두께가 "비워 둔 공간"이지
  채워야 할 부피가 아니기 때문이다(180cm 두꺼운 통로 마개도 실제로는 40cm 벽 한 장이면 판정이 같다). 윗단이 나머지를
  받는 이유는 눈이 가는 아래쪽 비율을 지키는 편이 벽 전체를 눌러 놓는 것보다 낫기 때문이다. 안 맞는 조각은 막지 않고
  세우되 길이 축 배율이 0.85~1.15를 벗어나면 `LogCL9` 경고를 낸다 — "격자를 이 팩 모듈에 맞춰라"는 신호다.
  변형 선택은 **자리 좌표 해시**다(난수 스트림은 호출 순서에 의존해 머신마다 갈릴 수 있는데, 지형은 복제되지 않고
  시드에서 각자 만들므로 그러면 사람마다 다른 벽을 본다).

- **양계장 격자를 1.67배로 키운다** (2026-09-07, 사용자: "맵이 작아서 금방 다 돈다"). 1층 8×6 = 48칸 → **10×8 = 80칸**,
  지하 6×4 → 8×6. 개방률(1층 61%·지하 66%)을 유지하도록 개구부 목표를 87/54로, 항아리를 7/6으로 함께 올렸다.
  통로 폭 255→250 · 열 폭 900→1000은 조각 모듈(500cm)의 배수에 맞추기 위한 것이라 같은 결정에 묶인다.
  축사 내부가 8000×3580cm가 되어 달리기 장축 주파가 8.3→12.2초가 되지만 **밤 길이(245초)는 안 건드렸다** —
  두 변수를 동시에 흔들지 않고, 실제로 뛰어 본 뒤 따로 판단한다.

- **구매는 클라이언트도 할 수 있다** (2026-09-07, 4f 구현 판단). 주문 확정(4c)은 호스트의 컨트롤러만 하도록 막았지만
  장비 구매는 막지 않았다 — 자금이 판 공유(`SharedMoney`)라 누가 눌러도 같은 지갑에서 나가고, 산 물건은 트럭 옆
  실물로 전원에게 보이므로 "누가 샀는가"가 판정에 아무 차이를 만들지 않는다. 판정은 여전히 호스트 전용이고,
  거절 사유만 누른 사람에게 `Client_BuyRejected`로 돌아간다. 사용자 확인을 받은 결정이 아니라 **구현 중 가정**이니,
  "구매도 호스트만"이 맞다면 `Server_BuyEquipment`에 `IsLocalController()` 관문 한 줄을 더하면 된다.

- **트럭 패널이 열 UI 행 이름을 C++ 상수로 두지 않는다** (2026-09-07). `CL9UINames`에 `Shop`을 넣는 대신
  `ACL9TruckPanel::ShopUIRow`(EditAnywhere, 기본값 `Shop`)로 뒀다. 상호작용 판정은 호스트가 하고 화면은 누른
  사람의 것이므로, 호스트가 그 사람의 컨트롤러로 `Client_OpenScreen(행이름)`을 보낸다. 이렇게 두면 표 행 이름이
  바뀌어도 C++을 안 고치고, UI 세션의 헤더(`CL9UITable.h`)를 게임플레이 세션이 건드릴 일도 없다.

- **장비를 잃는 경계는 트럭에서의 거리다** (2026-09-07, 4f 구현 가정). equipment.md의 "두고 오면 잃는다"를
  새벽에 `EquipmentKeepRadius`(기본 1200cm) 밖의 장비를 파괴하는 것으로 구현했다. save.md는 "밤 종료 시 트럭 존
  안에 있는 실물만 남는다"고 적고 있는데 **'트럭 존'의 정의가 아직 없어서** 거리로 대신했다. 트럭 존이 정식으로
  생기면(4g·⑤) 그 판정으로 바꾼다.

- **시설물은 액터 종류를 따로 갖는다 — `ECL9ActorKind::Facility`** (2026-09-07, 사용자: "스캔하면 몬스터와
  부엌 같은 것도 나오게 해줘. 시설물 같은 전용 enum 타입이 있어야 될 것 같은데"). 조리대·창구·트럭 패널·
  항아리를 `ACL9Fixture`(→ `ACL9ActorBase`) 아래로 모았다. 스캔이 `ACL9ActorBase`만 훑고 `Kind`로 고르는
  구조라, 시설물이 그 밖에 있는 한 표에 무엇을 적어도 보이지 않았다. **enum 값은 뒤에만 붙인다** —
  앞을 밀면 이미 저장된 `ScanKinds` 값이 다른 뜻이 된다.

- **화면에 뜰 이름은 액터가 스스로 답한다 — `ACL9ActorBase::GetDisplayName()`** (2026-09-07). 스캔 라벨이
  `Cast<ACL9Item>`/`Cast<ACL9Monster>`로 갈래를 타고 있었는데, 종류가 늘 때마다 갈래가 늘고 그 갈래는
  **UI가 게임 구조를 다시 아는 것**이라 언젠가 어긋난다. virtual 하나로 바꾸니 시설물이 늘어도 화면 코드는
  그대로다. 값 표시(`PriceText`)는 여전히 아이템만 캐스팅한다 — 값은 정말 아이템에만 있기 때문이다.

- **맵 생성을 격자 미로에서 "모듈 조립"으로 갈아엎는다 — CCL8 미로 폐기** (2026-09-08, 사용자:
  "미로 형식이라는 게 진짜 미로를 만들라는 게 아니라, 미로처럼 매번 맵이 달라지는 걸 만들어 달란 거야.
  가구도 배치하고", "특정 패턴이 반복되면서 맵이 만들어져야 돼", "이전에 CCL8에서 만든 미로는 잊어").
  **R.E.P.O.를 주 모델로 삼는다** — 격자 위에 모듈을 놓고 사방 소켓으로 잇는 방식이라 CL9이 이미 가진
  격자와 맞는다(리썰컴퍼니의 자유 그래프는 이식이 어렵다). 실측: REPO는 모듈 308개/테마 5개,
  타입 N·P·E·DE·S, 희귀도 1~3, 그리고 **`ModulePropSwitch`로 같은 모듈의 소품을 매번 교체**한다 —
  테마당 65개로 무한히 달라 보이는 이유가 그것이다. 전문 docs/reference/repo/repo-map.md.
  설계는 docs/plan/data/module.md.
  - **위 실측치 정정** (2026-09-08 같은 날, 디컴파일로 재확인): 모듈은 308개가 아니라 **311개**이고
    테마는 스테이지 4개(Manor 80 · Wizard 72 · Arctic 71 · Museum 69) + 상점 19다. "희귀도 1~3"은
    실제로 **난이도 티어**이고, `PickModule`이 `DifficultyCurve1..3`을 완료 레벨 수로 평가해 가중치를
    만든다 — 고정 희귀도가 아니다. `ModulePropSwitch`도 확률이 아니라 **그 변이 연결됐는지**로
    결정된다. 결정 자체는 그대로다.

- **모듈은 프리팹이 아니라 표의 행이다 (B안)** (2026-09-08, 사용자 선택). REPO·리썰은 방을 사람이
  손으로 만들지만(REPO 테마당 65개, 리썰 60~80개) 우리는 그 방들이 없고 **모듈러 키트**가 있다.
  그래서 껍데기(바닥·벽·문·천장)를 생성기가 조각 표에서 뽑아 세우고, 표는 "몇 칸짜리 방이고 어디가
  뚫리나"만 정한다. 모듈을 늘리는 비용이 낮은 대신 **방들이 비슷해 보일 위험**이 있고, 개성은
  소품 조합으로 낸다 — 보유 소품 142종(Dreamscape Interior 58 + Exterior 84)이 그 재료다.
  seg kind 표의 `hay`·`trough`·`crate`·`brooder` 행이 "배치 규칙이 없어서" 비어 있던 것이 여기서 채워진다.

- **층 개념을 버린다 — 높이는 모듈 안에서 낸다** (2026-09-08, 사용자: "층 개념은 리썰 참고해").
  지금의 1층/지하 + 경사로(`FCL9SiteFrame`의 `FloorDrop`·`Landing`·`Ramp`·`StairX0/X1`) 대신,
  리썰컴퍼니처럼 **계단·구덩이를 모듈 안에** 넣어 높이차를 만든다(`CatwalkStairTile`·`DeepShaftTile`·
  `LargeStairRoom`). 층을 두 장으로 나누는 것보다 단순하고, 방향 감각을 무너뜨리는 데도 낫다.

- **트럭은 사이트 밖에 고정한다** (2026-09-08). REPO도 리썰컴퍼니도 트럭/우주선이 레벨 **밖**에
  있고 걸어 들어간다. CL9도 그대로 간다 — 시작 모듈이 입구가 되고 트럭이 거기 붙는다.
  사이트 전체가 모듈이어도 트럭만은 손으로 놓은 고정물로 남는다(로비이자 안전 지대라 매번 움직이면 안 된다).

- **한 판은 모듈 20~30개, 일자가 갈수록 커진다** (2026-09-08). REPO의 `LevelSizeMultiplier`와 같은
  손잡이를 일자 곡선에 물린다. 밤 245초에 탐색 + 짐 나르기가 들어가는 크기가 기준이다.

- **모듈 조립 생성기의 미결 다섯을 정하고 구현했다** (2026-09-08, 사용자 결정). 정본은
  [plan/data/module.md](plan/data/module.md)의 "결정 — 미결 5개" 절이다.
  1. **소켓의 단위는 경계 칸이다.** 표는 N/S/E/W 4비트만 들고, 열린 변에 닿은 모든 칸이 문
     후보가 된다. "변 중앙에 하나"는 성립하지 않는다 — 16종의 크기가 홀·짝이 섞여 있어(3×2 ·
     2×2 · 2×3) 변이 짝수 칸인 모듈은 중앙이 **칸 경계에** 떨어진다.
  2. **높이차는 전파하지 않는다(net 0).** 계단 모듈은 자기 안에서 올라갔다 도로 내려온다.
     기준은 사용자가 준 "나중에 에셋이 추가돼도 잘 적용될 수 있어야 한다"였다 — 진짜 전파는
     새 계단 모듈을 한 행 추가할 때마다 고리에서 높이가 모순되는지 따져야 하고 배치기를 트리
     구조에 묶어야 한다. net 0은 표에 행만 추가하면 끝이고, 칸 점유 검사가 2D로 유지되며,
     **고리(우회로)를 살릴 수 있다.** 다만 껍데기를 세우는 함수는 "이 모듈의 바닥 Z"를 인자로
     받게 짜 두었다 — 진짜 층이 필요해지면 거기가 착수 지점이다.
  3. **입구 하나, 비상구 하나. 딱 둘.** 비상구(`flapIn`)는 시작 모듈에서 가장 먼 막다른 방에
     단다 — 리썰컴퍼니의 fire exit과 같은 장치로 "들어갈 땐 길고 나올 땐 짧다"를 만든다.
     개수가 고정이라 표 컬럼을 만들지 않고 상수로 뒀다.
  4. **칸 목록 위에 `LevelPoint`를 세운다.** 배치된 칸마다 지점 하나가 나오고 용도는 모듈 표의
     `PointPurposes` 컬럼이 정한다 — "항아리는 납품 방에만"이 C++에 별칭으로 박히지 않는다.
     질의는 `PointsOfPurpose()` 하나뿐이고 시작 모듈에서 먼 것부터 돌려준다. REPO의 나머지
     `LevelPoint*` 질의는 부르는 곳이 생길 때 만든다.
  5. **한 판 20 → 24 → 30 모듈.** `BaseModuleCount` 24 × `SiteSizeCurve`(1일 0.85 · 5일 1.0 ·
     10일 1.25). 기존 규약(사이트가 기준값 · 커브가 배율)을 그대로 따랐다.

  구현 실측(시드 200개): **22~24모듈 · 평균 24.0 · 막다른 방 29% · 판마다 고리 문 4.7개.**
  곁가지 단계에서 **배치 실패로 예산을 깎지 않는** 것이 컸다 — 처음엔 깎았더니 평균이 20.6까지
  떨어졌다. 자리가 안 맞은 것과 쓸 모듈이 없는 것은 다르므로, 연달아 48번 막힐 때만 판을 닫는다.

- **`UCL9SegKindDef::BaseZMode`를 지운다** (2026-09-08). 층과 경사로가 사라지면서 `WorldTop`
  ("램프 꼭대기에서 던져도 못 넘게")이 가리킬 대상이 없어졌다. 내 수정이 고아로 만든 컬럼이라
  함께 지웠다 — 이제 바닥만 기준점 아래로, 나머지는 위로 자란다.

- **모듈 표에 `bOutdoor`와 `PointPurposes`를 더한다** (2026-09-08, 구현 판단). 설계 초안의 컬럼만으로는
  둘을 표현할 수 없었다. `bOutdoor`는 마당 모듈(`yard_gate`·`yard_open`)이 천장을 안 쓰고 벽 대신
  울타리를 세워야 하기 때문이고, `PointPurposes`는 결정 4를 표로 내리기 위해서다. 둘 다 실제 16종
  가운데 쓰는 행이 있어서 넣었지 폭넓게 쓰라고 만든 손잡이가 아니다.

- **층을 되살린다 — 진짜 2층, 계단 모듈이 잇는다** (2026-09-10, 사용자: "2층으로 만들어줘",
  "테이블에서 최대 몇 층까지 만들까요 넣어서"). 2026-09-08에 버렸던 층을 다시 넣되, 그때의
  1층/지하 고정 프레임이 아니라 **모듈 격자를 층마다 한 장씩** 두는 방식이다.
  1. **`UCL9SiteDef.MaxFloors`가 상한을 정한다.** 2면 2층, 3이면 3층까지. 층 수는 일자와
     무관하다 — 몇 층짜리 건물인가는 그 목적지의 성질이지 난이도가 아니다.
  2. **계단은 모듈 타입이다** (`ECL9ModuleType::Stair`). 아래층과 위층의 **같은 칸을 함께
     차지해서**, 그 한 행이 두 층을 잇는 유일한 통로가 된다. 두 층에서 같은 인덱스가 나오므로
     도달 검사가 층을 자연스럽게 넘나든다.
  3. **개수는 `StairCountMin`~`StairCountMax`에서 시드가 뽑는다** (사용자: "계단개수는 랜덤").
     진행 곡선은 붙이지 않았다 — 일자로 늘릴 이유가 아직 없다.
  4. **위층은 아래층 실내 모듈이 받쳐 주는 자리에만 선다.** 마당(`bOutdoor`) 위에는 못 올린다.
  5. **계단은 자라는 도중에 섞어 놓는다.** 층을 다 채운 뒤에는 계단이 설 빈자리가 남아 있지
     않을 수 있다.
  6. **고리는 그대로 산다.** 문을 뚫는 판정이 한 층 안에서만 도니까, 높이가 어긋난 두 방은
     애초에 후보가 아니다 — 리썰컴퍼니가 문틀을 3D 격자(`Vector3Int`, y 포함)에 해시해서 얻는
     성질을 격자가 공짜로 준다. 2026-09-08에 "고리에서 높이가 모순되고 배치기를 트리에
     묶어야 한다"고 적었던 것은 **취소한다** — 그 걱정은 자유 배치일 때의 문제였다.
  7. **계단 실물은 디딤판을 쌓아 만든다.** 층 높이를 20cm로 나눠 개수를 정하고 모듈의 긴 축을
     따라 올라간다. 디딤판도 `floor` 종류라 조각 표를 새로 만들지 않았다. 폭은 문 두 짝만큼만
     쓴다 — 방 전체가 경사로가 되면 계단실이 아니라 언덕이다.
  8. **층 높이는 손으로 안 적는다.** 가장 높은 방의 천장 + 천장 판 + 바닥 판 두께이며,
     `ACL9SiteMap`이 조각 실측에서 뽑는다 (asset-swap.md의 원칙 그대로). 양계장 실측 610cm.

  실측(시드 100개, `MaxFloors` 3): **100판 전부 2층에, 85판이 3층에 도달. 판당 계단 5.1개 ·
  위층 모듈 7.6개.** 단층 판(200시드)은 22~24모듈 · 평균 24.0 · 막다른 방 29%로 층을 넣기
  전과 같다 — 회귀 없음. `CL9.Map.Floors` 테스트가 "위층 칸이 허공에 뜨지 않는가",
  "어느 층에서도 두 모듈이 한 칸을 겹쳐 쓰지 않는가", "계단을 통해 전부 도달하는가"를 지킨다.

  **리썰컴퍼니는 우리와 다르다** (2026-09-10, `DunGen.dll` 디컴파일로 확인): 층 인덱스가 없는
  대신 **높이가 전파된다.** `TileProxy.PositionBySocket`이 앞 타일 문틀의 위치와 회전에 새 타일
  문틀을 세 축 모두 포개므로, 계단 타일의 출구가 높으면 그 뒤가 통째로 올라간다. 겹침 검사도
  3D(`IsCollidingWithAnyTile(UpDirection, …)` · `DisallowOverhangs`)다. 2026-09-08에 net 0을
  고르며 "리썰컴퍼니 방식"이라 적은 것은 **층 인덱스가 없다는 점만** 가져온 것이고, 높이 전파는
  리썰과 반대였다 — 그 서술을 여기서 정정한다.

- **천장을 켜고 끄는 것을 표 컬럼으로 뺀다** (2026-09-10, 사용자: "천장도 테이블에 넣을지 말지
  세팅하게 해줘"). `UCL9SiteDef.bBuildCeilings`. 그전에는 `SegKindTable`의 `ceiling` 행
  이름을 `__ceiling_off__`로 바꿔서 껐는데(`b166eb9`), 그러면 레지스트리가 "천장 종류가 없다"고
  매번 에러를 찍었고 그게 진짜 결손과 구분되지 않았다. 행 이름은 `ceiling`으로 되돌리고
  스위치만 껐다 — 레지스트리의 필수 종류 검사도 이 값을 따라간다.
  **표현만 끄고 배치는 안 건드린다**: 시드 873에서 켜든 끄든 배치 해시가 `D62D5F3A`로 같고
  인스턴스 수만 517 ↔ 536으로 달라진다. 층 높이는 천장 판 두께를 늘 포함해 계산하므로
  스위치를 올렸다 내려도 2층 바닥이 움직이지 않는다. 양계장 행은 **꺼 둔 채로 둔다** —
  실물을 보고 정하겠다는 것이 원래 의도였고, 그 결정은 아직 사용자 몫이다.

- **`Lvl_Henhouse`에서 못 움직이던 것 — 원인이 셋이었다** (2026-09-10, 사용자: "LV_HENHOUSE에서
  플레이하면 움직일수없음"). PIE로 폰 위치를 찍어 하나씩 걷어냈다.
  1. **깨어나는 자리가 트럭 안이었다.** `ReviveDead()`가 패드 원점에 세웠는데, 패드는 바닥 판이
     아니라 트럭 몸체까지 포함한 덩어리다(실측 400 × 400 × 240). 캡슐이 그 콜리전에 갇혀 한
     발도 못 움직였다. `ACL9GameMode::WakeSpot()`으로 빼내고 **패드 앞(+X, 맵 입구 쪽)** 으로
     비켜 세운다 — 흩는 것도 좌우로만 한다. 앞뒤로 흩으면 다시 트럭에 박힌다.
  2. **`PlayerStart_1`이 맵 벽 속이었다.** (400, 0)인데 격자는 패드에서 `EntryOffset`만큼
     떨어진 x=400부터 시작한다 — 시드와 무관하게 언제나 시작 모듈의 서쪽 벽 자리다.
     마당 쪽 (0, 250)으로 옮겼다.
  3. **지반이 5m 아래 있었다.** `Floor_0`의 윗면이 z=-500.5였다. 트럭 패드(z=10)도 맵 바닥(z=0)도
     그 위에 떠 있어서, 마당에 선 플레이어가 5m 아래로 떨어졌다. **작동하던 `Lvl_Flat`은 윗면이
     정확히 z=0**이고 패드·PlayerStart는 두 레벨이 같다 — 그 값에 맞춰 올렸다.

  검증: 폰이 (0, 250, 92)에 서고 겹침 0, 앞으로 밀어 **90틱에 351cm 이동**. 세 번째 것이
  제일 오래 숨어 있었다 — 1·2번만 고쳤을 때는 "안 갇히는 대신 떨어지는" 상태였다.

- **계단실 위층에 참을 깐다** (2026-09-10, 사용자: "계단중에서 내가 갈수없는곳에 생긴계단도있음").
  계단 모듈이 위층에 바닥을 하나도 안 깔아서, 위층은 **디딤판 꼭대기 한 칸을 빼면 아래층까지
  뻥 뚫린 구멍**이었다. 위층에서 그 모듈로 난 문은 전부 허공으로 열렸다. 이제 계단이 뚫고
  올라오는 통로만 비우고 나머지를 덮는다 — 통로 양옆의 띠와, 오르기 시작하는 쪽의 참이다.
  디딤판도 그만큼 뒤로 물려서(길이의 15%) 그 참 아래가 계단 밑 공간이 된다.
  `CL9.Map.Floors`에 "계단실 위층이 넓이의 30% 이상 덮여 있는가"를 넣었다.

- **조리 게이지는 재료 라벨이 아니라 조리대 꼭대기 위에 띄운다** (2026-09-10, 사용자: "요리 진행중 표시가
  요리 오브젝트에 붙어 있어서 자세히 봐야 보이는데, 요리중이면 부엌 맨 위에 생겨서 누구나 볼 수 있게, 어떤
  방향에서도"). 라벨 안의 바는 그 재료에 포커스를 둔 사람에게만 보였다. 이제 HUD가 익히는 중인 `ACL9Kitchen`마다
  `WBP_CookGauge` 하나를 `CookLayer`에 붙이고 조리대 꼭대기(`GetTopLocation()`+60cm — 상판의 재료에 안 가리게)로 매 프레임 투영한다 —
  스캔 라벨과 같은 꼴이라 액터에 WidgetComponent를 붙이지 않는다는 규칙은 그대로다. 클라도 같은 바를 보려면
  조리대↔재료 짝을 알아야 해서 `ACL9Kitchen::Cooking`을 복제로 돌렸다(진행도 자체는 전부터 재료에 실려 복제됐다).
  라벨 안의 `CookBar`는 뗐다.

- **계단을 올라가면 반드시 이어지는 공간이 있어야 한다** (2026-09-10, 사용자: "계단수를 줄인다
  이런게 방법이 아니고, 계단이 있는 방은 계단 이후 바로 고립되면 안 된다는 규칙이 정해져야").
  개수 손잡이가 아니라 **규칙**이다 — `CL9SiteLayout::StairMinReach`(3). 층을 다 채운 뒤에
  계단마다 **위층을 먼저 틔운다**(`SecureStairs`): 그 계단에서 이어지는 프론티어만 골라
  방·복도를 붙이고, 최소치를 못 채우면 그 시도를 통째로 무르고 계단을 내린다(`DemoteStair` —
  위층 칸을 비우고 둘레 변을 열린 상태로, 위층 프론티어에서도 뺀다). 무르기는 배치 직전의
  칸·변·프론티어를 복사해 뒀다가 되돌리는 방식이다. 살아남은 계단이 하나도 없으면 그 위층은
  없는 것으로 판을 닫는다.
  **첫 방들은 막다른 방으로 시작하지 않는다** — 위층을 틔울 때는 Normal·Passage만 쓴다.
  실측(시드 100개, MaxFloors 3): 계단 5.1 → **1.9개**, 위층 모듈 7.6 → 7.7개 그대로,
  **계단 하나당 위층에서 닿는 방 1.5 → 4.6개.** 같은 양의 위층이 몇 안 되는 계단 뒤로
  모여서 진짜 층이 됐다. 2층에 못 간 판이 10%인데, 그건 "갈 데 없는 계단"이 서느니 계단이
  없는 게 낫다는 규칙의 결과다. `CL9.Map.Floors`가 "계단 위층에서 내려오지 않고 문만으로
  닿는 모듈 ≥ 3"을 모든 계단에 대해 지킨다.
  총 예산을 층끼리 나누는 문제(2층을 켜면 1층이 좁아진다)는 **아직 열려 있다.**

- **위층은 1층을 깎지 않고 덧붙는다** (2026-09-10, 사용자 "ㅇㅇ" — 제안 중 첫 번째).
  그전에는 `BaseModuleCount`를 층끼리 나눠서(1층 13 · 2층 7) 2층을 켜면 1층이 좁아졌고,
  "맵이 너무 좁다"(같은 날 앞선 지적)와 정면으로 충돌했다. 이제 **1층은 ModuleCount를
  그대로 받고, 위층은 그 절반씩 덧붙는다** — 1일차 양계장이 20 → **30모듈**(1층 20 + 2층 10),
  3층이면 +5. 2층을 켜는 게 곧 맵이 커지는 일이다. `FCL9SiteLayout.Wanted`가 층을 합친
  목표치를 들고, 로그가 "N of M modules"로 찍는다.
  실측(시드 100개, MaxFloors 3): 2층 도달 90 → **98판**, 3층 57 → **84판**, 위층 모듈 7.7 →
  **15.0개**, 계단당 위층 방 4.6 → **5.5개**. 단층 판은 그대로(22~24, 평균 24.0).

## 2026-09-11

- **동물 5종을 자체 제작 메시·클립으로 갈아끼움 — 종별 AnimBP 신설** (사용자 — "자연스럽게
  움직이는 것까지"). `MonsterTable`의 chicken·goose·raccoon·pig·sheep 행이 `Monsters/<종>A/`의
  `SK_<종>_A`·스켈레톤과 `Content/Monsters/ABP_<종>_A`를 가리킨다. 새 모델은 +X 전방이라
  `MeshYawOffset`은 −90 → **0**. 배경: 09-10에 Dreamscape 팩을 용량 문제로 잘라내면서 팩의
  동물 애님 시퀀스가 디스크에서 사라져 기존 `ABP_Chicken`·`ABP_Pig`·`ABP_Sheep`의 시퀀스 플레이어가
  전부 비어 있었다 — 동물이 이미 참조 포즈로 서 있던 상태였다.
  ABP는 `ABP_Chicken` 복제(양방향 블렌드 체인 + 상태→알파 EventGraph 그대로) → 타깃 스켈레톤
  교체 → 플레이어 8개 교체. 매핑: Idle/Walk/Run/WakeTelegraph(루프 없음)/Catch/HitReact/Death(루프 없음),
  Sleep 슬롯은 거위만 `AN_Goose_Sleep`, 나머지는 Idle. 거위의 Catch 슬롯은 `AN_Goose_Honk`.
  **기준 속도는 manifest의 `reference_speed_cm_s`로 확정** — Walk=WanderSpeed(126, 너구리 140),
  Run=ChaseSpeed(396, 거위 528, 너구리 462). 블렌드 문턱도 같은 표 값(Wander→Patrol→Chase)에 맞췄다.
  PIE 확인: 5종 스폰·발 접지·걷기, 닭 HitReact/Death 알파 전환, 양의 추격→잡기(하트 감소)까지.
  옛 `ABP_Chicken`·`ABP_Fox`·`ABP_Pig`·`ABP_Sheep`은 남겨 뒀다(참조 없음). 남은 것: Investigate에
  `LookAround` 연결(지금은 로코모션이 대신), 거위 `AlertWindup`(상태 없음), 사용자 육안 검수.

- **아이템 7행도 자체 제작 메시로 연결** (2026-09-11, 사용자 — "아이템들도 몬스터처럼 배치해줘").
  `ItemTable`의 meat 3상태·cart·flashlight·headlamp·frying_pan `Mesh` 컬럼만 바꿨다 — `CL9Item::LoadData`의
  `Radius/바운드` 자동 배율이 크기를 맞추므로 표 값·레벨은 손대지 않았다(Lvl_Flat 아이템 줄의 손 배치
  액터도 BeginPlay에 행을 읽어 그대로 따라온다). 안 바뀐 것: egg cooked/burnt/cracked/broken(메시 미제작 —
  아직 Sphere), pot·standing_lamp(행 없음).

- **손전등 광원을 메시 원점에서 렌즈 앞으로 옮김** (2026-09-11, 사용자 — "아이템 플래시 안 켜지는데 왜 그래?").
  `ACL9Item::SetupLight`가 스포트라이트를 메시 로컬 바운드 `Max.X + 1cm`에 붙인다. 원인: 자체 제작
  `SM_Flashlight_A`는 닫힌 몸통이라 원점에 둔 스포트라이트가 자기 그림자에 통째로 갇혔다(엔진 Cylinder
  때는 드러나지 않던 문제). 메시 그림자를 끄는 대신 광원을 옮긴 이유는 손전등 몸통이 벽에 그림자를
  드리우는 게 맞기 때문. 렌즈가 +X라는 제작 규약(item/portable_lights.md)에 기댄다 — 헤드램프도 같다.
  PIE(밤)에서 줍기→F로 바닥·수레가 비치는 것 확인.

- **빛은 손전등(·헤드램프)에서만 나온다 — 발광 아이템 폐기** (2026-09-11, 사용자 — "빛은 손전등만 나와야
  되는데, 다른 아이템은 빛이 나오면 안 돼"). `ItemTable`의 `egg`·`egg+cracked` 행에 켜져 있던
  `bEmitsLight`(Glow, 192cm)를 껐다. light.md의 "발광 계란 = 정전 뒤 유일한 광원 / 소지자가
  `CarrierSeenRadius`에서 들킨다" 설계는 **번복** — 정전 뒤 광원은 헤드램프와 스탠드 램프뿐이 된다.
  `ECL9ItemLight::Glow` 코드 경로와 `LightRadius`·`bLightLostInPocket`·`CarrierSeenRadius` 컬럼은
  지우지 않고 남겨 둔다(행이 꺼져 있으면 아무것도 안 한다).

- **item - lightitem - flashitem 클래스 분리** (2026-09-11, 사용자 — "item - lightitem - flashitem 이런 식의
  구조여야지"). "아이템 100종이어도 클래스는 하나" 규약을 **행동이 같은 것끼리 하나**로 고쳐 쓴다.
  `ACL9Item`에서 조명 코드를 전부 들어내고 `OnAcquired`/`OnReleased` 훅만 남겼다. `ACL9LightItem`이
  라이트 컴포넌트·`bLightOn` 복제·"들면 켜짐"(점광원)을, `ACL9FlashItem`이 스포트라이트·F 토글·정전
  규칙(착용은 예외)을 맡는다. 행의 `bEmitsLight`·`LightKind`(`ECL9ItemLight`)는 삭제 — 빛을 내는지는
  `ActorClass`가 정한다. `BP_CL9FlashItem`(부모 `ACL9FlashItem`) 신설, `flashlight`·`headlamp` 행의
  `ActorClass`에 지정. `Lvl_Flat`·`Lvl_Henhouse`에 손으로 놓여 있던 손전등 2·헤드램프 1(Flat) /
  손전등 1·헤드램프 1(Henhouse)은 같은 자리·같은 별칭의 `BP_CL9FlashItem`으로 갈아 놓았다.
  PIE: 줍기→F 켜짐→F 꺼짐, `cl9.blackout`이면 꺼지고 F로 못 켬.

- **CL9Art 미사용 에셋 일괄 연결 — 생성기에 `Grid`·`yard`·`tread`·토막 종류, 소품 세트, 시설 BP·농부·냄비·작업등**
  (2026-09-11, 사용자 — "cl9art 쪽 지금 안 쓰는 거 있으면 싹 다 사용 가능하게 적용해줘", 범위는 "전부 (소품 시스템까지)").
  명세 `docs/goal/wire-cl9art-assets.md`(HEAVY, 4관점 패널 + 교차 심문). 결정:
  - `PieceTable`의 Dreamscape 행을 자체 조각으로 교체(팩은 5da02ff로 저장소에서 빠져 클론이 깨지던 상태).
    새 역할 `wallQuarter`·`floor`·`ceiling`·`yard`·`tread`. `wallHalf`(Wall_250)는 행만 있고 소비처 없음.
  - **바닥·천장·마당은 새 배치 모드 `Grid`**(두 평면 축 격자 깔기), **디딤판은 `Tile`**(FitPieceToCell이 이미 회전·
    보정), Stretch는 `box` 전용으로 그대로. 반려: 바운드 인식 Stretch — Grid가 있으면 죽은 코드고 마당 흙을 3배로 늘린다.
  - 생성기가 야외 바닥을 `yard`, 디딤판을 `tread`, 문·비상구 양옆 토막을 `barnStub`/`wallStub`로 따로 내보낸다.
    아래로 자라는 종류는 `CL9SegKind::GrowsDown`(floor·yard·tread). 반려: `ShortPieces` 검색 컬럼 — 토막 125에서
    후보가 하나뿐이고 종류별 맞춤 보고를 흐린다. `door` 종류가 없는 것은 더 이상 에러가 아니다.
  - **문 폭 250 유지**(`door` seg kind 행 없음). 문짝 Door_A(132)·디딤판(264)은 132 세계 제작물이지만 250으로
    소비한다 — `ACL9FlapIn::Setup`이 폭만 1.89×로 맞추고 두께·높이는 자연값(개구부 위는 Body가 막음).
  - **barn·plug·flapIn 높이 630 → 580** — 층 높이 610이라 1층 벽이 2층 바닥을 20cm 뚫었다.
  - **소품 세트는 모듈 행 안에 인라인**(`UCL9ModuleDef.PropSets`, module.md 결정 7 그대로). 세트 추첨은 `PickVariant`
    좌표 해시, 소품마다 `bSolid` 하나 + `Side`/`SideRule`(REPO 변 연결 조건 — 발자국이 걸친 경계 칸 전부 검사).
    몸 있는 소품이 문 자리·스폰 지점(칸 중심 ±130, 같은 층)을 가리면 경고. 반려: 별도 PropSet 표 — 공유할 일 없이
    DefList 재스캔 함정만 는다. 양계장 17종에 1~2세트씩 저작(둥지·상자·포대·여물통·작업등·작업대·표식 4·보·기둥).
  - 시설 BP 5종 메시 교체(배율 1), Lvl_Flat·Lvl_Henhouse 인스턴스 z 0/배율 1/주방 Yaw 90. 주방 HeatZone 절대 회전은
    C++ 생성자 한 줄. **푸드트럭은 패드 −X 650cm·Yaw 180(뒷문이 패드 향함)에 임시 배치** — 사용자 "배치해봐야 알 거
    같은데", 보고 정하기로.
  - 농부 A 몸/팔을 `farmer_a` 행에 영구 연결. `pot` 행(프라이팬 복제)과 `standing_lamp` 행 신설 — 작업등은 새
    `ACL9LampItem`(ACL9LightItem 파생, 내려놓아도 켜짐 — 사용자 "내려두어도 불은 켜져 있어야지"). Radius 60·
    LightRadius 800·Intensity 5000은 임시값(기획 미정).
  - 남은 것: `wallStub`은 125짜리 울타리가 없어 0.50 경고가 정상.

- **든 아이템은 손 소켓에 붙고, 행의 `HoldPose` 범주가 팔 자세를 고른다** (2026-09-12, 사용자 — "손에 소켓
  만들어서 붙이면 안돼?", "모든 아이템들도 일괄로 손에 붙을 수 있게", 임시(권총 자세) 대신 "정석"). 캡슐 앞
  `CarryHeight` 부착을 버리고 `ACL9Item::OnPickedUp`이 소지자 **전신 메시**의 `HandGrip_R`(마네킹 공용 소켓)에
  붙인다 + 행의 `GripOffset`/`GripRotation`. 팔 메시가 아니라 전신인 이유: 남의 화면에서 팔 메시(OnlyOwnerSee)는
  애니메이션이 멈춘다. 소유자 화면에서는 팔 메시가 FPWarp로 십수 cm 비틀려 있어 **메시 컴포넌트만** 팔 소켓으로
  옮기고 `FirstPersonPrimitiveType::FirstPerson`으로 그린다(컴포넌트 부착은 복제 안 됨). 손이 허리에 내려가 있어
  1인칭에서 안 보이는 문제는 자세로 푼다: `ECL9HoldPose` Small/Flashlight/Large 범주, 전신 클립 3 + 1인칭 팔 클립 3
  (블렌더 두 뼈 IK, `animate_farmer_hold.py`), `ABP_Unarmed`·`ABP_FP_Copy`의 spine_01 상체 레이어. 정본은
  docs/systems/inventory.md. 반려: 템플릿 권총 자세 차용(임시), 팔 메시 상대 위치 이동(카메라 정합 깨짐). Stretch100 변형 메시 3종은 불필요해졌다(삭제 보류).
    `CL9.Replication.Registration` 테스트는 에디터에서 돌리면 엔진 단언(bIsPushBased)으로 에디터가 죽는다 — 이번에
    확인(내 변경과 무관, `GetLifetimeReplicatedProps`를 직접 부르는 테스트). `CL9.Map.*` 4개는 통과.
  - **ABP 배선은 부울 블렌드 2단으로, 행 그립 값은 1인칭 자세에서 역산** (2026-09-12 새벽, 배선·PIE 검수 세션).
    `BlendPoses(ECL9HoldPose)`는 항목 핀 노출이 에디터 메뉴 전용(`ExposeEnumElementAsPin`, `VisibleEnumEntries`는
    파이썬 미노출)이라 자동화로 못 만든다 → `Large? : (Flashlight? : Small)` 부울 블렌드 두 개 + `ToFloat(NOT(==None))`
    가중치. 레이어는 `spine_01` 로컬 공간(팔 자세가 몸 기울기를 따라간다). ABP 변수는 `add_member_variable`+
    `EdGraphPinType.import_text`로 만들었다. `GripRotation`/`GripOffset`은 손으로 맞추지 않고 PIE에서 손 소켓
    공간의 카메라 정면·월드 위·양손 중점을 실측해 넣었다(inventory.md). 수레는 끌려오는 액터라 `HoldPose=None`
    그대로 두고 "수레 손잡이" 자세는 만들지 않았다 — 견인 자세(팔을 뒤로)는 요청이 있을 때 별도로. 달걀(반지름
    20cm)·작업등(1.5m)이 1인칭에서 큰 것은 자세가 아니라 크기 데이터의 문제라 손대지 않았다.


- **밤 조명 값을 사이트 표로 옮기고, 축사 안을 "검정의 색"으로 보이게 한다** (2026-09-12, 사용자 —
  "안에 들어가면 너무 어두워서 아예 안 보이는데 조금은 보이게", "리썰 보면 어두운데도 뭔가 보이잖아",
  "그 값도 테이블로 세팅 가능하게"). 결정:
  - 전역 밝기(노출·태양)는 안 올린다 — 09-07에 같은 시도(밤 0.8/1.2)를 원복한 이유 그대로, 정전이 의미를
    잃는다. 대신 리썰컴퍼니식 **약한 청색 앰비언트**(SkyLight 0.25×, 차가운 색조) + **옅은 안개**
    (ExponentialHeightFog 0.01, 볼류메트릭)로 "형태만 읽히는 어둠"을 만들고, 정전 행에서 둘 다 다시 끈다.
  - 값은 `ACL9NightLighting` 액터 프로퍼티가 아니라 **`UCL9SiteDef`의 `LobbyLighting`/`NightLighting`/
    `BlackoutLighting`**(`FCL9LightingPhase`: SunScale·SunColor·SkyScale·SkyColor·FogDensity·FogColor).
    새 조명 표를 권했다가 사이트 표로 바꿨다 — 사용자 "사이트표가 좋아보이는데": 어둠은 사이트의 성격이고,
    구조체 컬럼 셋이면 행이 안 부풀며, 사이트가 하나라 중복 비용이 없다. 새 표는 클래스·레지스트리·DefList
    등록이 다 따라붙어 지금 필요 이상이다.
  - 액터에는 `ExposureLock`·`TransitionSpeed`만 남는다(표현 액터 고유값). 안개 액터는 레벨에 놓고, 볼류메트릭
    같은 나머지 세팅은 액터의 것을 쓴다 — 표는 밀도·색만 페이즈별로 준다.
  - PIE 실측으로 두 가지를 더 정했다: **실내 앰비언트는 `SkyLeak`(Lumen Skylight Leaking) 컬럼**으로 넣는다 —
    천장이 덮인 방은 루멘이 완전히 가려서 SkyLight 배율·`Cast Shadows` off로는 한 점도 안 들어왔다. 그리고
    SkyLight를 실시간 캡처 그대로 두고 밤 `SkyScale`을 10으로 잡는다 — 캡처를 끄면(한 번 캡처) 값은 직관적이지만
    캡처 시점이 머신마다 달라질 수 있어 접었다. 정전은 태양 0이면 하늘도 0이라 자동으로 캄캄하다.

- **화면 문자열은 String Table `CL9`의 키로만 쓴다 — 코드·UMG·데이터 표 어디에도 한글을 직접 치지 않는다**
  (2026-09-12 저녁, 사용자 — "한글을 박는게 아니라 string alias를 박아서 언어설정 바뀔때마다 싹다 바뀌어야").
  같은 날 17:16의 "`NSLOCTEXT` 키 규칙만 두고 String Table은 안 둔다"를 뒤집는다 — 그때는 "같은 파이프라인에
  간접층만 는다"고 봤지만, 사용자가 원한 것은 **원문이 코드 밖 한 곳에 있고 코드는 alias만 드는 것**이었다.
  - 테이블은 `Content/Strings/CL9.csv` 한 장(`LOCTABLE_FROMFILE_GAME`, 모듈 시작 시 등록). String Table
    `.uasset`은 반려 — 바이너리라 diff·리뷰가 안 되고 에디터 없이 못 고친다. CSV는 소스 수집기가 그대로 긁고
    에디터 텍스트 피커에도 뜬다(엔진 소스로 확인).
  - 키는 `영역.이름`(`Prompt.PickUp`, `Hud.Money`), 데이터 행은 `Item.egg.Name`/`.Desc`. 번역 파일이 아직
    없어 기존 `PromptPickUp`류 86키를 지금 전부 바꿨다 — 이후로는 키 이름 변경 금지.
  - 데이터 행의 `DisplayName`/`Description`은 **FText 컬럼을 유지**하고 값이 테이블 참조여야 한다(사용자 선택
    2026-09-12). `UCL9DefBase::Validate`가 직접 친 텍스트를 에러로 잡아 행을 Broken으로 만든다. 반려:
    컬럼을 없애고 Alias에서 키를 파생 — 스키마가 바뀌고 키 누락이 화면에 키 문자열로 새는 것을 검증이 못 잡는다.
  - 로컬라이제이션 타깃 `CL9`(ko 원문·en), 런타임 전환은 `cl9.lang`(설정 화면 언어 항목의 대역). en 번역은
    코드 작성자의 초안. 정본은 docs/systems/localization.md.
  - 추가(같은 날 밤, 사용자 "컬럼 수정… 나라별로 바꿀 수 있게", "DevNotes에는 어떤 의미로 사용되는지"): 정본을
    **`Content/Strings/CL9_Strings.csv` 마스터 시트 한 장**(`No,Key,DevNotes,ko,en,ja,zh-Hans`)으로 올리고, 런타임
    `CL9.csv`와 언어별 `.po/.locres`는 `Tools/Loc/sync_strings.py --build`가 거기서 생성한다. `No`는 번호+`Key` 병기
    (번호만 키로 쓰는 안은 코드·에셋 참조 176곳을 다시 바꾸고 읽기 어려워 반려). DevNotes는 "어디서 쓰나"가 아니라
    "어떤 뜻·상황의 문구이고 `{0}`이 무엇인가"를 적는다 — 번역자가 그것만 보고 옮기게.

- **좌클은 "든 아이템 사용", 던지기는 G, 수레는 앞에서 민다** (2026-09-12 낮, 사용자 — "우리 좌버튼으로 던지는거 없애자
  이거좀 에바임", "좌버튼누르면 아이템에 따라 다르게 동작하게하자", "타격시 hp피해량은 int(0이면 없는거), 경직타입을
  넣을수있게해서 enum으로 세팅하게하자", 수레는 "잡으면 밀고 다니자"). 결정:
  - 새 `IA_Use`(좌클) → `Server_Use(bHold)`: **탭**은 행 `UseAction`(`None`/`ToggleLight`/`Swing`), **길게 누르기(0.4초)**는
    `ItemType == Dish`면 먹기(먹을 수 있는 건 Dish뿐이라는 기존 규칙을 따르고 별도 `Eat` 값을 두지 않는다). 익은 고기가
    Dish이면서 무기라 한 키로 둘 다 되게 한 것 — 사용자 "길게누르면 먹기, 짧게누르면 휘두르기". 빈손은 무동작 — 옛 빈손 밀치기
    (`Server_Shove`, "④ 장비 전까지 대역")는 프라이팬 행이 이어받았다. `IA_Eat`(G)은 삭제, `IA_Throw`는 G로, F는
    헤드램프 전용. 정본은 docs/systems/input.md.
  - 타격 컬럼은 `HitDamage`(int, 0=없음)·`HitStun`(`None`/`Hit`=`HitStunMs`/`Push`=`PushStun`+넉백+추격) 둘뿐 —
    시간·거리는 몬스터 행이 갖고, 사거리 200cm·회복 0.6초는 상수로 남긴다(요청 전까지 컬럼화하지 않음). 몬스터엔
    `OnStrike(Damage, Stun, Source)` 하나로 들어가고 피해가 먼저, 죽으면 경직 없음. 반려: 아이템별 클래스 파생 —
    행 값으로 충분하고 "행동이 같은 아이템은 클래스 하나" 원칙과 맞다.
  - 수레 `Pulled`→`Pushed`: 연결 거리(캡슐+수레 반경+20cm) 정면에 서서 미는 사람 yaw를 따라 스윕 이동, 막히면 못 간
    만큼 미는 사람을 물린다. "견인은 순간이동 없음"(09-02) 규칙은 밀기로 대체. 반려: 수레를 캡슐에 부착 — 벽을 뚫는다.
  - 문구: `Prompt.CartPull` "[E] 수레 밀기"/"Push cart" 등 수레 4건 시트 수정(`CL9_Strings.csv` → sync).

- **옵션 시스템·키 리바인딩·화면 스택** (2026-09-12 밤, 사용자 — "설정에서 단축키를 바꿀 수 있어야 돼… [E] 이런 식으로
  박혀 있더라고, {0} 해서 단축키를 바꾸면 같이 적용", "옵션 시스템도 같이 만들어야 돼 / plan, system 등 모든 문서도",
  "레이어가 쌓이면 한 번에 닫히는 게 아니라 하나씩 뒤로… X 버튼도… 공통 기능은 전부 UCL9Widget에서"). 4관점
  패널(엔진·UX·YAGNI·다국어)+교차 심문을 UE 5.8 소스로 확인해 결정. 정본은 docs/plan/settings.md(무엇을 여나)·
  docs/systems/settings.md(저장·적용·화면)·input.md 리바인딩·ui_system.md 화면 히스토리, 명세는 docs/goal/settings-and-key-rebinding.md.
  - 키는 엔진 `UEnhancedInputUserSettings` 그대로(서브클래스 없음), 나머지(감도·Y반전·볼륨 6개)는 `UCL9GameUserSettings`.
    IMC 3개 등록은 `ACL9PlayerController::BeginPlay` — `SetupInputComponent`·`AddMappingContext(bNotifyUserSettings)`는
    유저 설정이 아직 없는 시점이라 첫 PC에서 조용히 실패한다(반려). `MapPlayerKey`는 Slot·장치 명시(아니면 중복 추가).
  - 프롬프트 키 표기는 `{0}` 서식 + 시트 `Key.<FKey>` 행, 없으면 엔진 영문 폴백 — "키 파생 반려"(같은 날)의 명문화된
    예외(폴백이 있어 키 문자열이 새지 않는다). 엔진 `GetDisplayName` 직접 사용은 ko 번역이 뒤섞여 반려.
  - 충돌은 **알리고 맞바꾼다**(사용자), 전 행 한 그룹(IMC별 그룹은 필요해질 때). 캡처는 `IInputProcessor`(UMG
    `NativeOnKeyDown`은 SButton이 먼저 먹어 반려), 취소는 **Esc 또는 5초 무입력**(사용자) — Esc는 프리프로세서가
    소비해 PIE 정지와 안 겹치므로 2026-09-05 "Esc는 게임 키 아님"의 유일한 예외. IMC에는 여전히 Esc 행 없음.
  - 볼륨 슬라이더 기본은 전부 1.0, 설계 게인(0.8/0.5/0.7)은 SoundClass에 — 80%로 보이는 기본값은 플레이어가 100%로
    "고친다". `TMap` 볼륨·`Settings.Volume.<클래스>` 파생 키는 반려(라벨은 `LOCTABLE` 리터럴이어야 시트 검사가 잡는다).
    `SettingsTable` DataAsset은 참조 전용(믹스+클래스 6)으로 유지 — 경로 예외는 MasterSoundMix뿐.
  - 저장은 **변경 확정 즉시**(닫을 때 저장 반려 — PlayOutro 비가상·NativeDestruct는 페이드 뒤·서버가 화면을 밀어낼 수
    있다). `ApplySettings()`류는 부르지 않는다(PIE에서 에디터 품질까지 바꾼다).
  - 전면 화면은 **이름 스택**(위젯은 하나만 살아 있음): M/[X]/`CloseSelf` = 한 단계 뒤로. `PauseInput`의 관전 특례를
    스택이 대신하고, [계속]으로 닫을 때 관전 화면이 안 돌아오던 공백을 함께 메운다. `UCL9Widget`에 공통 `CloseButton`
    (Optional)과 `OnClosing()` 훅. 반려: 설정에서 `OpenWidget(Pause)` 하드코딩, `ReturnScreen` 필드.
  - 리셋은 [키 초기화] 하나(전체 리셋·행 우클릭 반려), 선택은 세그먼트(◀▶·드롭다운 반려), 언어 라벨은 원어명(String
    Table 예외). `cl9.resetkeys` 치트 신설. `Pause.NotReady`는 뜻이 바뀌어 `Pause.LobbyNotReady` 새 키.
  - "esc 같은 공통 기능"은 뒤로 가기 키 = M(IA_Pause)으로 해석했다 — Esc를 UI 키로 되살리지 않는다.

- **데이터 에셋 이름에서 `DA_` 접두사를 뗀다 — `DA_SiteTable`은 `MapTable`** (2026-09-12, 사용자 — "모든 테이블 앞에
  DA 붙는 거 지워줘 필요없음, DA_SiteTable은 MapTable로"). 에디터 rename(참조 fix-up: `CL9DefList`·`BP_CL9GameInstance`)
  + 문서·Tools 스크립트·코드 주석 일괄 치환. C++은 에셋을 소프트 참조로만 받아 로직 변경 없음. 대응표:
  BuffTable · CL9DefList · CL9Progression · CL9Rules · CharacterTable · EquipmentTable · ItemTable · ModuleTable ·
  MonsterTable · OrderTable · PieceTable · SegKindTable · ShakeTable(Effects→Data 이동분) · **MapTable**(←DA_SiteTable) ·
  `Content/UI/UITable`(←DA_UITable). 새 표는 `SettingsTable`처럼 처음부터 접두사 없이 만든다.
  함정: `CL9DefList`는 PreSave에서 폴더를 재스캔하는데 rename 도중엔 에셋 레지스트리가 낡아 옛 경로가 저장됐다 —
  rename이 다 끝난 뒤 `save_asset(only_if_is_dirty=False)`로 한 번 더 저장해야 새 경로가 들어간다.
  - 추가(같은 날 밤, 사용자 "번역은 어느 순간에 한 번 할 거라 기본은 한국어로만 작업하고, 번역은 신경 쓰지 말라"):
    문구 작업은 `ko` 열만. 다른 언어 열은 비워 두고(빈 칸은 ko로 표시) 번역 패스 때 몰아서 채운다. 이날 en·ja·zh-Hans
    186개를 초안으로 한 번 채웠다.

- **`Content/CL9Art` 폴더를 없애고 자체 제작 에셋을 규약 폴더로 흩는다** (2026-09-12, 사용자 — "cl9art에 있는 것들도
  자리에 맞게 배치 다시해줘, 원래 우리 폴더 규칙대로, 그다음에 cl9art 지워"). 189개를 에디터 move로 옮기고(참조 fix-up:
  ItemTable·CharacterTable·MonsterTable·ModuleTable·PieceTable·BP_CL9 시설 5종·ABP_<종>_A·ABP_Unarmed·ABP_FP_Copy·
  Lvl_Flat·Lvl_Henhouse 재저장) CL9Art를 지웠다. 대응:
  Characters/FarmerA · Monsters/<종>A · Items/{CartA,Cookware,MeatStates,PortableLights,StandingLampA} ·
  **시설 메시는 BP 옆** Items/{Kitchen,DeliveryCounter,ReturnJar,TruckPad,TruckPanel} · Items/Egg(SM_Egg_Raw_A) ·
  Maps/FoodTruck · Maps/Henhouse/{Access,Door,Fence,Structure,Workbench,Yard,Props(구 HenhouseSample01)} ·
  **새 최상위 `Tests/`**(Lvl_AssetQA01·SK_RigProbe* — 게임이 참조하지 않는 검수물). 이름 충돌은 없었다(테스트물 `_test`
  개명 불필요). docs/asset/README.md의 임포트 목적지 규칙과 conventions.md Content 트리를 같이 고쳤다.
  함정: 맵(.umap)은 `rename_asset`이 "INI/소스 찾기·바꾸기 필요" 확인 대화상자에 막혀 실패한다 — 복제 → 원본 삭제로
  옮긴다. 옮긴 뒤 남은 리다이렉터는 참조 레벨(Lvl_Flat·Henhouse)을 `save_asset(only_if_is_dirty=False)`로 재저장하면 지울 수
  있다(py에 `fixup_referencers`가 없다). Interchange 임포트 메타(`contentImportPath`)에 옛 경로 문자열이 남지만 참조가 아니다.
