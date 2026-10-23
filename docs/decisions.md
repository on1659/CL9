# 결정 로그

> **추가만 한다** — 과거 항목을 고쳐 쓰지 않고, 뒤집을 때는 새 항목으로 번복을 명기한다.
> 규약 전문은 [conventions.md](conventions.md).
>
> 표기(2026-09-21 정리): `## 2026-09-11` 절부터는 여러 세션이 같은 날 동시에 덧붙여 날짜 절을 더 나누지 않고,
> 항목 머리(제목 뒤 괄호 또는 불릿 첫머리의 굵은 날짜)에 날짜를 적는다. 그래서 그 절 안의 날짜 순서는 섞일 수 있다.
> 이름 일괄 개명(`DA_` 접두 제거·`CL9Art` 폴더 폐지)이 과거 항목 본문의 경로·에셋 이름을 치환한 것은 이 규칙의 유일한 예외로 인정한다(2026-09-12·13 커밋).

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

## 2026-09-02 (계속)

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
  - **휘두르기 애님은 몽타주가 아니라 복제된 시작 시각 + 시퀀스 평가기** (2026-09-13, 사용자 "휘두르기 안 되는데?" → "아예
    액션을 안 해"): `SwingStartServerTime` 복제 → ABP `SwingElapsed` → `InRange(0, 0.58)`이면 평가기(`ExplicitTime`) 가지.
    반려: `Multicast_PlaySwing`+몽타주 — 일회성 연출엔 허용이지만 슬롯을 상체 레이어 위에 또 얹어야 하고(전신 몽타주는
    다리를 기준 포즈로 되돌린다) 늦게 들어온 클라가 못 본다. 평가기 방식은 기존 부울 블렌드 체인에 한 단만 더한다.

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

- **운반 모델 — 손 하나+품+R 교체+`SwapTime` 1.2초 폐기, 리썰식 4칸 핫바로** (2026-09-13, 사용자 — "아이템 인벤토리같은게
  있으면좋겠어 ui하단에 네모칸같은거로해서 … 전형적인 리썰방식이야. 조작은 마우스휠로 할수있는데, 이건 마우스 옵션에서 바꿀 수
  없어야돼"; 답변: 4칸·품/R/SwapTime 폐기·휠+숫자키 1~4·리썰 twoHanded 규칙). `ACL9PlayerState::Slots`(크기 `CL9Rules.HotbarSlots`)
  + `ActiveSlot`; 활성 칸만 실물 액터, 나머지는 데이터(별칭·상태·굴린 값 `Value`). 휠(아래=다음, 순환, 한 노치 한 칸 —
  `Server_ScrollSlot(±1)` 상대 RPC)·숫자키 1~4(`Server_SelectSlot` 절대 번호)로 즉시 전환, 빈 칸 선택 허용. E는 줍기·착용·
  상호작용·수레 놓기만("[E] 놓기" 프롬프트 제거), **G 놓기**(칸 비우고 활성 칸 유지), **달리기 키+G 차징 던지기**(조합 고정,
  설정에는 놓기 행만 — G를 옮기면 던지기가 따라간다). 줍기는 활성 칸이 비었으면 활성 칸, 아니면 첫 빈 칸+그 칸 활성; 꽉 차면
  불가. 두 손(`HoldPose` Large)·수레 미는 중은 전환·줍기 잠금(호스트 판정). 잡혀 넘어지면 네 칸 전부 흘림. 칸 전환은 가격을
  다시 굴리지 않는다(`SpawnItem(PresetValue)`/`HostSetValue`). PlayerState 복제 20Hz(기본 1Hz면 HUD 칸 전환이 1초 늦다).
  - HUD: 우하단 "손:/품:" 텍스트 → 하단 중앙 4칸 핫바(`WBP_HotbarSlot`/`UCL9HotbarSlotWidget`, `HotbarBox`에 런타임 생성,
    활성 칸은 `Frame` 테두리 색). 문자열 `Hud.Hand*`·`Hud.Pocket*`·`Prompt.Place`·`Input.Throw.Name`·`Input.Swap.Name` 폐기,
    `Input.Drop.Name`·`Input.Slot1~4.Name` 신설. **마우스 휠은 옵션이 아니다** — IMC 행 `IgnoreSettings`, 설정 화면에 행 없음.
  - 에셋: `IA_Drop`·`IA_Slot1~4`·`IA_SlotScroll` 신설, `IMC_Gameplay` 행 교체(Throw·Swap 행 삭제), `IA_Throw`·`IA_Swap`
    에셋은 참조 0이지만 아직 삭제하지 않았다(아침 판단). `CL9Rules.HotbarSlots` 4.

- **수레 — 실물 크기 물리 강체 + 스프링 밀기 + 물리 적재** (2026-09-13, 사용자 — "수레를 일단 크게만들어 … 넣은 아이템에서 물리법칙이
  자연스럽게 움직여야돼", "내가 움직인다고 수레가 저렇게 따라서 움직이면 안돼. 수레를 실제 현실에서 옮기는거같은 느낌"; 답변: 물리 강체+스프링,
  짐칸 120×80·벽 40·손잡이 100cm, 놓기/던지기로 싣기, 짐칸 안 깨짐 규칙 유지). 행 `bMeshIsBody`로 메시(UCX 9)가 루트·몸체(반경 구·자동
  스케일 폐기 — 그 자동 스케일이 수레를 작게 보이게 한 원인), 밀기는 호스트 매 틱 무게중심 스프링·감쇠 힘(backward Euler, 상대 속도 감쇠)+
  yaw 스프링 토크(PhysicsConstraint 아님), 손잡이–목표점 > `GripBreakDistance`면 손 놓침. 적재는 소켓 `Cargo` 상자(`CargoExtent`) 안 자유
  아이템(부착·`LoadedItems`·"[E] 싣기" 폐기, 싣기는 G/던지기, 꺼내기는 E 줍기). 연결 거리는 행 `GripDistance`. 수레는 폰을 막는다(Pawn
  Block) — 미는 사람도 벽에 막힌 수레와 함께 선다(아침 확인). 클라 아이템 시뮬은 엔진 `SyncReplicatedPhysicsSimulation`에 맡긴다(클라가
  끄면 복제 목표가 버려져 얼어붙는다). `BP_CL9Character` CMC `PushForceFactor`·`InitialPushForceFactor` 0 — 캡슐이 수레를 밀치면
  25kg 수레가 공중으로 떴다(PIE 실측); 밀기는 스프링만 한다. 값(Mass 25 · PushSpring 40 · PushDamping 9 · PushYawSpring 25 ·
  PushYawDamping 7 · GripDistance 60 · GripBreakDistance 90 · Friction 0.15 · Restitution 0.05)은 **초안** — balance가 PIE 감각으로
  확정. `SM_Cart_A` 재제작(바운드 167.7×97.2×103.4, 소켓 `Grip`(80.15,0,48)·`Cargo`(−17.85,0,14)). FBX는 Interchange가 UCX를 무시해
  `Interchange.FeatureFlags.Import.FBX 0`으로 레거시 임포터를 강제했다. Blender 헤드리스는 `Tools/Blender/run_headless.py`로(훅 허용
  문안 수정 필요 — 아침 질문).

- **좌클 = 공격 대기 → 뗌 = 타격, 먹기는 우클 길게, 첫 줍기 힌트는 로컬·종류별 한 번** (2026-09-13, 사용자 — "마우스 누르면 기본->공격대기모션,
  마우스떼면 그떄 공격모션 … 리썰방식", "아이템을 들고있을때 e누르면 놓기 ui나오는게 뜨면안돼. 정말로 게임이 처음인 유저만 … 잠깐나와서
  보여주고 사라지게"; 답변: 무한 대기·공격 동일, 우클 길게=먹기, 내 PC 로컬 설정·설명이 필요한 종류만·4초·컨텍스트 프롬프트만 남김).
  `IA_Use` Started → `Server_UseStart`(행 `UseAction` `Swing`이면 `SwingWindupServerTime` 복제·무한 대기, `ToggleLight`는 누르는 순간
  토글) / Completed → `Server_UseRelease`(대기 중이었을 때만 `HostSwing` — 피해·사거리·경직은 대기 시간과 무관, 밸런스 열 없음). 손의 것이
  바뀌는 모든 경로(칸 전환·놓기·던지기·먹기·잡힘)가 대기를 지운다. 옛 `Server_Use(bool)`·좌클 길게 먹기 폐기. `IA_Scan` 바인딩을 PC에서
  캐릭터로 — 탭(`CL9Rules.UseHoldTime` 0.4초 전 뗌)은 `ACL9PlayerController::RequestScan()`, 넘기면 `Server_Eat`(활성 칸 `Dish`만);
  스캔은 뗄 때 나가 최대 0.4초 늦을 수 있다(사용자 수용). 힌트: `UCL9ItemDef::HasUseHint()`(`UseAction != None || Dish`) 행을 처음
  들었을 때만 HUD `HintText`에 `Hint.*` 문구(현재 키 표기)가 `HintDuration` 4초 — 본 적 있음은 `Alias`별로 `UCL9GameUserSettings.SeenItemHints`
  (튜토리얼은 사람에게 붙는다). 대기·먹기 표현은 상태 파생(복제 서버 시각 + `GetSwingWindupElapsed`/`GetEatElapsed`, 몽타주·RPC 없음):
  `AN_Farmer_SwingWindup`/`FPSwingWindup`(휘두르기 t=0.12 자세 1.0초 루프)·`AN_Farmer_Eat`/`FPEat`(HoldSmall→입 앞→HoldSmall 0.6초),
  ABP 두 개에 `SwingWindupElapsed`·`EatElapsed` 가지(대기 루프 > 휘두르기 > 먹기 순으로 덮음). 농부 클립 FBX는 30fps로 굽는다(스테이징
  24fps 버그 수정 — 기존 `AN_Farmer_Swing` 0.583초는 재수출 보류, 아침 질문). 그립 값은 `measure_item_grip.py`로 역산하되 결과가 기존
  행과 같아 프라이팬 "거꾸로"는 미해결(아침 질문 11·12).

- **1인칭 팔은 빈손·걷기에 화면 밖, 달릴 때만 흔들리며 들어오고, 손전등 빔은 렌즈에서 나오며 상체가 시선 피치를 따른다** (2026-09-13, 사용자 —
  "지금 항상 손을 보이게하려고하니까 문제가있어 … 리썰처럼 최초화면에선는 안보이다가. 뛸때 손을 흔들면서 … 플래시를들면 그떄 손조금 뻗어서",
  "발원지가 손전드이여야돼"; 답변: 렌즈 발원 + 팔이 시선 피치를 따름). 2026-09-12 "idle에서도 손이 보여야 한다(`AN_Farmer_FPHoldIdle`)" 번복 —
  `ABP_FP_Copy` `HoldPose == None` 가지 = `IsRunning ? AN_Farmer_FPRun(0.6초 루프, 기준 654cm/s) : AN_Farmer_FPArmsDown(2.0초 루프, 손 프레임
  밖)`, 걷기 전용 클립 없음. 손전등 `Flashlight.fp` 손목을 앞 8·위 3cm 뻗어 렌즈가 보이게(`animate_farmer_hold.py -- --only Flashlight`).
  라이트는 아이템 메시의 자식(`GetLightOrigin()` = 바운드 +X 끝+1cm), 방향 메시 +X. `ACL9Character::GetAimPitch()`(소유자 컨트롤 피치·타인
  `RemoteViewPitch`)·`IsRunning()`·`GetGroundSpeed()` 게터 + 두 ABP에 `Transform (Modify) Bone`(spine_03, 컴포넌트 공간, AimPitch×0.6 초안;
  FP는 ControlRig 뒤). PIE 실측: 전신·FP 메시의 spine은 기울지만 소유자 화면의 손전등 렌즈 피치는 FPWarp 릭 때문에 거의 불변 — 아침 질문 14.
  농부 Hold 클립 재출력은 `--only <범주>`로 제한(sha 변화 방지). FPRun PlayRate는 고정 1.0(핀 미노출 — 질문 15).

- **"핫바" → "퀵슬롯", HUD 칸은 리썰식** (2026-09-13 저녁, 사용자 — "퀵슬롯이라 하자", "핫바 UI가 너무 강조되고 있음 … 하얀색으로만 … 리썰처럼").
  이름: C++ `UCL9HotbarSlotWidget`→`UCL9QuickSlotWidget`(파일 `CL9QuickSlotWidget.*`), `UCL9HUDWidget::HotbarBox/HotbarSlotClass/HotbarSlots`→
  `QuickSlotBox/QuickSlotClass/QuickSlots`, `UCL9RulesDef::HotbarSlots`→`QuickSlots`(기본 4, 에셋 미직렬화라 리다이렉트 불필요), 에셋
  `WBP_HotbarSlot`→`WBP_QuickSlot`(`Config/DefaultEngine.ini` 클래스·`HotbarSlotClass` 프로퍼티 리다이렉트 2줄), 문서·주석·문자열 DevNotes 전부.
  `Slots`·`ActiveSlot`·`SlotActions`·`SlotScrollAction`·`IA_Slot*`·`FCL9CarriedItem`은 "슬롯"이라 그대로. 옛 결정 문안의 "핫바"는 기록으로 둔다.
  칸: 64px 정사각, 검은 반투명 바탕, 얇은 흰 테두리, 번호·이름 글자 없이 행 `Icon`(행이 바뀔 때만 동기 로드)만 — **든 칸 흰 100%, 나머지 흰 40%**
  (`ActiveColor`/`InactiveColor`), 노란 강조 폐기. 아이콘 없는 행은 이름 12pt 폴백. 아이콘은 Blender Workbench 흰 실루엣+알파 PNG 128×128
  (`Tools/Blender/render_item_icons.py` → `ArtSource/item/<폴더>/icons/` → `/Game/UI/Icons/T_Icon_<alias>`, UI 그룹·NoMipmaps·sRGB), 계란·고기는
  상태 행이 raw 아이콘 하나를 공유(칸만 보고는 상태 구분 없음 — 리썰과 같음), 수레는 없음.

- **1인칭 팔은 카메라의 자식(뷰모델) — 리썰 방식, 클립은 카메라 로컬로만 저작한다** (2026-09-13 밤, 사용자 — "아이템 들었을 때 … 자꾸 회전",
  "위아래를 볼 때 손이 움직이는 거랑 시야가 좀 이상해, 리썰 보고 그 조작감으로", "애니메이션이 부자연스러워, 억지로 팔을 들고 있는 모습").
  `FirstPersonMesh`는 `FirstPersonCameraComponent`에 `(0,0,−EyeHeight)`·요 −90°로 붙어(전신 마네킹과 같은 규격 — 리그 root 원점, 눈이
  (0,0,EyeHeight)에서 +X) 시선 피치·요를 통째로 따른다; `ApplyCharacterDef`가 행의 EyeHeight로 다시 맞춘다. 팔 ABP는 새 `ABP_FP_View`
  (`/Game/Characters/FarmerA/Anims`) — 몸 포즈를 보지 않고 `GetHoldPose/IsRunning/GetGroundSpeed/GetSwingElapsed/GetSwingWindupElapsed/
  GetEatElapsed`만 읽어 카메라 로컬 클립을 고른다. 2026-09-02~13의 "전신 메시 자식 + 몸 포즈 복사(`ABP_FP_Copy`) + `CtrlRig_FPWarp` +
  팔 쪽 spine_03 피치 추종 + `FPHoldIdle`" 구조는 폐기(에셋 삭제). `GetAimPitch()`는 전신 `ABP_Unarmed` 전용(남의 화면의 상체 기울임).
  스태미나 셰이크는 POV만 흔들어 화면이 흔들리고 팔은 따라 떨리는 듯 보인다(리썰과 같음). 클립 규약: 공간 = 리그 root 원점·눈 (0,0,1.56m)·
  전방, 피치 0 고정 프레이밍, 클립 하나가 전 뼈를 소유(척추 identity, 숨쉬기는 손목 ±0.5cm), manifest `space: "camera"`·`eye_height_m`.
  화면 위치는 `farmer_pose.fp_screen_uv`(1280×720·가로 FOV 70·눈 1.56·근접 5cm/0.6 — 게임과 같은 카메라) 실측으로 적는다(옛 4:3·10° 미리보기
  폐기; FirstPersonScale 0.6은 화면상 위치·크기를 바꾸지 않는다). **주먹 위치(사용자 — "손 위치는 이 정도로 좀 바깥에", 기준 캡처
  `preview/my/1.png`)**: 든 손 주먹은 우하단 구석에서 프레임 안으로(중심 u 0.86~0.92·v 0.85~0.95, 손목은 프레임 바로 밖), 두 손은 0.28/0.72·
  v ~0.9, 팔꿈치 ~90°로 느슨하게(억지로 들지 않음), 손전등도 같은 구석(029 "뻗기" 폐기 — 렌즈 정면은 표 행 `GripRotation`이 맞춘다), 대기는
  우상단 안(손목 (0.86, 0.46)), 먹기는 렌즈 앞 중앙(너클 (0.59, 0.48)). 휘두르기 클립은 30fps 0.467초(15프레임)가 정본(옛 0.583초 폐기) —
  두 ABP 모두 평가 시간 = `SwingElapsed + 0.12`(대기 루프가 이미 보여 준 앞부분 건너뜀)·범위 `[0, 0.347)`. 그립 소켓 보정은 028 자세를
  `measure_item_grip.py`에 상수로 박아 되살린다 — 1인칭 자세를 바꾸면 손전등·두 손 행을 다시 푼다. 이번 표 행(`031-item-grip.json`):
  `flashlight.GripRotation (−29.8, 105.5, 34.6)`, `pot`·`standing_lamp` `GripOffset (11.5, 1.6, 6.7)`·`GripRotation (4.9, 112.4, 26.8)`,
  고기 3행 `(−3.2, 26.5, −3.1)`·`(8.6, −78.4, −28.1)`, `frying_pan` `(−2.0, 26.4, −5.2)`·`(−8.6, 101.6, 28.1)` — `forward` 후보로 PIE에서
  뼈가 손에·팬 손잡이가 손에 오는 것을 확인(아침 질문 11·12 "팬 거꾸로" 해소). PIE 실측: 계란을 들고 피치 −30/0/+30에서 카메라 기준 아이템
  회전 불변(아이템이 돌지 않음), 손전등 빔 피치 = 시선 피치(−45.8 @ −45), 팔 부모 `FirstPersonCamera`·`(0,0,−156)`·`(0,−90,0)`.

- **놓기(G)는 조준점 놓기, 수레를 맞히면 짐칸 안, 물리 아이템은 코드가 재운다** (2026-09-14 새벽, 사용자 — "아이템이 땅에 떨어지면 물리법칙이
  이상해. 마찰력이 없어서 무한히 굴러간다거나 무한히 회전하고 있어", "카트도 카트 안에 아이템을 넣을 수가 없어"). 원인 둘: ① 행 `Gravity`를 매 틱
  게임 스레드 힘으로 주입하면 Chaos가 잠들기 판정 속도를 매번 되돌려(`ResetVSmoothFromForces`) 절대 잠들지 않고, 5.8에 강체별 중력 배율이
  없어 힘 주입은 유지한다 → `ACL9Item::UpdatePhysicsRest`가 변위(1cm·2°, 0.5초 — 코드 상수)로 멈춤을 판정해 `PutRigidBodyToSleep`,
  잠든 동안 힘 없음, 깨면(수레·충돌·던지기) 판정 재시작. 행에 `AngularDamping`(초안 3.0)·`LinearDamping`(초안 0.3) 열 — 구 몸체의
  구르기는 마찰만으로 서지 않는다; 수레 행은 0/0(`PushDamping`·`PushYawDamping`과 겹치지 않게). ② 들린 동안 몸체는 살아 있는 키네마틱이라
  텔레포트 없는 `SetActorLocation`은 키네마틱 목표점으로만 남고 같은 틱의 시뮬레이션 켜기에 버려져 **놓은 아이템이 오른손 소켓 자리(폰 25cm
  앞·24cm 오른쪽)에서 떨어지고 있었다** → 놓기·던지기는 `TeleportPhysics` + 속도 지정(던지기 원점도 이제야 유효). 놓기 규칙: 옛
  "캡슐 앞 70cm·`CarryHeight`" 폐기 — 눈에서 호스트 컨트롤 회전 방향으로 `CL9Rules.DropReach`(초안 250cm) 안의 첫 블로킹 히트 위
  (아이템 반지름 구 스윕, 폰·자신 무시)에 놓고, 없으면 사거리 끝(허공이면 떨어진다); **히트가 `ACL9Cart`의 어느 부분이든 조준점 대신
  짐칸 안**(`GetCargoDropLocation`: 히트 XY를 `CargoVolume` 안으로 클램프, 상자 윗면 위에서 떨어뜨림) — 미는 자세에서는 눈→짐칸 바닥
  선이 가까운 벽(눈보다 71cm 아래·1.3m 앞)을 못 넘어 "벽 너머로 손을 뻗는" 동작을 이렇게 흉내 낸다. `CarryHeight`는 칸에서 꺼내 스폰하는
  높이로만 남는다. 리썰 `PlaceObject` 승계. 던지기: Shift 먼저→G는 원래 됐고, 같은 틱 두 `Started`는 Enhanced Input이 바인딩 역순으로
  쏘므로(`EnhancedPlayerInput.cpp:801`) `GetActionValue(RunAction)`도 읽어 차징. PIE 실측(2026-09-14 01:55): 놓은 계란 2초 안 잠듦·위치
  고정, 레벨 아이템·수레 전부 잠듦; 미는 자세 −25/−40에서 G → 짐칸 안, −55는 손잡이 아래 바닥(수레를 안 맞힘 — 정상); 옆에서 G → 짐칸 안;
  수레 위를 넘겨 보면 사거리 끝. 던진 것의 감쇠는 자동 PIE(3fps, 물리 시간 1/10)로는 못 재 — 사용자 직접 확인.

- **애니메이션 보강 — 휘두르기 재작업, 농부 넘어짐·던지기, 몬스터 Catch·HitReact 재작업 + PushReact 신설** (2026-09-15, 사용자 — "피격·공격
  애님이 부족하다, 몬스터·캐릭터 둘 다. 무기 공격도 어색하다. 부족한 부분 다 파악해서 만들자, 지금 있는 모델들로"; 범위 "1~4 전부" 승인).
  분석: 농부는 넘어짐·죽음·던지기 클립이 0개(코드는 이동 불가·카메라 하강만), 휘두르기는 4키 직선 보간·옆으로 벌어진 치켜듦·허리 옆 타격·
  몸통 정지가 어색의 원인. 몬스터는 밀침(PushStun 0.8초) 전용 클립이 없어 0.3초 HitReact 뒤 선 자세로 미끄러졌고 Catch·HitReact 가 sin 펄스 하나였다.
  결정: ① 휘두르기는 키 6개·구간 완급·몸통 비틀림으로 다시 만들되 **ABP 상수(오프셋 0.12·범위 0.347)·히트 구간은 불변**, 시작·끝은 HoldSmall
  스펙을 읽는다(옛 키는 11cm 어긋났다). ② 넘어짐은 `KnockdownStartServerTime` 복제 + 전신 `Knockdown 0.5 → Down 루프 → GetUp 0.6`(골반 위치 키,
  발 IK 뒤에서 통째 대체), 죽음은 Down 루프 PlayRate 0 — "롤 90°" 계획 폐기. 1인칭은 FPKnockdown·FPGetUp(카메라 하강은 기존 Tick 그대로).
  ③ 던지기는 `ThrowChargeStartTime` 복제 + `ThrowServerTime` 신설 → ThrowWindup 루프·Throw 0.333초(끝 = 기준 자세). ④ 몬스터 원샷은 농부
  휘두르기와 같은 규칙 — `StateStartServerTime` 복제 + 시퀀스 평가기(ExplicitTime = GetStateElapsed) 로 늦게 들어온 클라도 같은 프레임;
  경직은 `bPushStun` 복제로 HitReact/PushReact 선택; Catch 는 코드의 휴지 0.8초에 맞추고 예비동작 없이 첫 0.1초에 덮친다(판정이 진입 순간).
  반려: 몬스터 예비동작(판정보다 먼저 보이려면 판정을 늦춰야 해 규칙 2 위반), 던지기 예비동작을 놓은 뒤에 트는 것(물건이 이미 떠났다),
  HurtStun 비틀 애님(코드에 구현 자체가 없다 — 컬럼만). 제작 기록 [asset/character/farmer_a.md](asset/character/farmer_a.md) 032·
  [asset/monster/README.md](asset/monster/README.md) 표, 규칙 [systems/animation.md](systems/animation.md) "넘어짐 가지"·"던지기 가지"·"몬스터 원샷".

- **2026-09-15 — 놓기(G)는 손 자리 낙하. 2026-09-14 "조준점 놓기" 번복** (실행 swing-hitbox, 사용자 "그자리에서 떨어지게").
  `Server_Drop`은 든 아이템의 **지금 월드 위치**(전신 손 소켓에 붙은 액터 루트)를 그대로 `OnPlaced`에 넘긴다 — 회전 유지·속도 0,
  시선·컨트롤 회전을 읽지 않는다. `CL9Rules.DropReach`·`ACL9Cart::GetCargoDropLocation`·`DropClearance`는 지웠다. **수레 짐칸에 넣는
  길은 던지기(Shift+G)뿐이다** — 수레를 보고 G를 눌러도 손 자리(짐칸 밖)에 떨어지는 것이 정상. 손 자리가 캡슐 안쪽이어도 아이템
  몸체가 폰 채널에 Overlap이라 튕기지 않는다(PIE: 계란이 손에서 1~29cm 안, 바닥 위에 멈춤). `OnPlaced` 텔레포트 해제(09-14)는 유지.
- **2026-09-15 — 휘두르기 판정은 애니메이션을 따르는 히트박스** (사용자 "히트박스를 넣어서 몬스터를 잡을 수 있게"). "뗀 순간 200cm 구 +
  정면 부채꼴(dot 0.3)에서 가장 가까운 하나에 `OnStrike`"를 폐기한다. 호스트가 매 틱, `GetSwingElapsed()`가 `CL9Rules.SwingHitStart`~
  `SwingHitEnd`(초안 0.05~0.25초, 클립 평가 범위 `[0, 0.347)` 안) 구간과 **틱 구간 겹침**으로 만나는 동안 손에 든 것의 메시 경계 구를
  **이전 틱 자리 → 지금 자리로 스윕**해, 닿은 `ACL9Monster`마다 **한 휘두르기에 1회씩** `OnStrike(HitDamage, HitStun)`를 낸다. 히트박스
  열은 없다(행의 `Mesh`가 모양 — 현재는 `LoadData`의 자동 스케일 때문에 경계 구 반지름 = 행 `Radius`). 판정 기준은 아이템 루트 구(전신
  손)이고 `MeshComponent`가 아니다(소유자 화면에선 1인칭 팔에 재부착). 권한 쪽 전신 메시에만 `AlwaysTickPoseAndRefreshBones`. 질의는
  오브젝트 질의가 아니라 **WorldDynamic 채널 스윕 + Overlap 응답**(오브젝트 질의는 모든 히트를 Block으로 보고해 나란한 둘 중 하나만
  맞는다 — 엔진 `CollisionFilterData.cpp:504`, `SceneQuery.cpp:564`). 대상은 몬스터만, 난수 없음. PIE(slomo 0.15로 3fps 보정): 나란한
  닭 둘 `hits=2 struck=2` 뒤 재타격 0, 2.5m·등 뒤 무타격, 고기 Hit 경직·변위 10cm, 프라이팬 1타 밀쳐냄 1,561cm·2타 사망 확인.
- **2026-09-15 — 타격 위력 초안(balance): 프라이팬 `HitDamage` 2 + `Push`, 고기(생·익은·탄) 1 + `Hit`.** 닭 `HpMin 3`/`HpMax 4`에 팬은
  굴림과 무관하게 **항상 2대**(`ceil(3/2)=ceil(4/2)`) — 한 방(피해 4)이면 죽음이 경직을 취소해 팬의 서명인 밀쳐냄이 영영 안 나오고,
  피해 1이면 매 타격 15.6m 넉백이라 한 마리에 15초 탁구가 된다. 고기 1은 무기가 아니라 잡힌 뒤 `KillCooldown` 9초 창의 궁여지책 —
  팬만 무기로 두려면 고기 3행을 0으로(표 값, 빌드 불필요). 히트 구간 0.05/0.25는 클립 `AN_Farmer_Swing`(30fps 0.467초)의 치켜든
  자세 t=0.12를 0으로 본 축에서 내려침 바닥 0.13 앞뒤이며, 끝을 0.25로 둔 것은 낮은 fps에서 틱 하나가 반드시 구간에 들어오게 하는
  여유다. 미결: 너구리 HP 4~5는 팬 2~3대로 갈린다(별건), 닭을 죽일 수 있게 된 뒤 재스폰·압박 순환(별건).
- **2026-09-15 — HUD 주문 줄의 재료는 "창구에 올린 수/필요 수"이고, 올린 수는 납품 창구 재고에서만 센다** (`/cl9` 실행
  hud-order-progress — "부족한 점을 찾아 고쳐라" 자유 과제). 옛 "재료×n"(필요 수만)을 폐기한다. 퀵슬롯·수레에 든 것은 세지 않는다 —
  정산(`ACL9GameMode::SettleOrdersAt`)이 창구 재고만 보므로, 들고 있는 것까지 세면 HUD는 다 모았다고 하는데 납품은 안 되는 거짓말이
  된다. 배정은 두 겹: ① 정산과 똑같이 마감 오름차순(StableSort)으로 돌며 **재료를 전부 채울 수 있는 주문만** 통째로 가져가고, ② 그러고도
  남은 재고를 못 채운 주문들에 마감 순으로 `min(필요, 남은 수)`씩 나눠 미리 보여준다. ①이 없으면 창구에 재고가 남은 채 새 주문을
  받았을 때(정산은 납품 때만 돈다) HUD가 가리키는 주문과 정산이 실제로 채우는 주문이 어긋난다. 표시 순서는 `AcceptedOrders` 그대로,
  완료 표시는 없다(다 모이면 정산이 그 자리에서 소비해 줄이 사라진다). 재고에 OnRep이 없어 HUD가 매 틱 서명(항목 수+alias/sub)을 비교해
  바뀐 틱에만 다시 그린다. PIE: `0/n` → 손에 든 것 제외 → 납품 시 계란후라이 줄 소멸·+380 → 오믈렛 0/3→1/3 확인. 문자열 키
  `Hud.OrderIngredientProgress`(ko `{0} {1}/{2}`) 신설, 옛 `Hud.OrderIngredient` 삭제. `docs/systems/order.md`의 낡은 "미구현" 줄
  (주문 선택 화면·HUD 진행 중 주문·세이브 — 셋 다 이미 구현)도 이때 정정했다.
- **2026-09-15 — `ItemTable.frying_pan.bPhysicsEnabled` 를 true 로** (사용자 "프라이팬을 떨구는데 카트가 뒤집혀"). 손 자리 놓기로 바뀐 뒤 수레 옆에서 G 를
  누르면 판정 구(반지름 30)가 수레 벽과 겹친 채 놓이는데, 프라이팬 행만 물리가 꺼져 있어(2026-09-03 "기본은 켜짐" 결정의 예외로 남아 있던 값) 움직이지
  않는 충돌체가 되어 Chaos 가 수레를 무한 질량에 밀리는 쪽으로 풀었다 — 고기(물리 켜짐)는 겹쳐도 고기가 밀려난다. 질량은 두 행 다 기본값이라 무관.
- **2026-09-15 — 타격감·피격감은 복제된 시각에서 파생한다** (`/cl9` 실행 hit-feedback, 사용자 "맞았을 때 맞는 느낌이 하나도 안 나"). 맞은 몬스터의
  빨간 오버레이(`SetOverlayMaterial`, `CL9Rules.HitFlashMaterial`), 잡힌 사람의 몸·1인칭 팔 오버레이, 피격 비네트, 타격·피격 카메라 셰이크는 전부
  **표현 전용**이며 멀티캐스트 RPC 를 쓰지 않는다. 각 머신이 이미 복제되던 시각(`ACL9Monster::StateStartServerTime`,
  `ACL9Character::KnockdownStartServerTime`)에서 경과를 계산해 같은 그림을 만든다. 새로 복제한 값은 공격자 카메라 킥용
  `ACL9Character::SwingHitServerTime`(`COND_OwnerOnly`) **하나뿐** — "내가 때렸다"를 아는 시각이 따로 없어서다. 셰이크는 OnRep 이 아니라
  **틱 비교**로 낸다(리슨 호스트는 자기 폰에서 OnRep 이 뜨지 않는다). 몬스터 플래시는 `GetState() ∈ {Stun, Dead}` 조건을 **반드시** 같이 건다 —
  레벨에 처음부터 있는 몬스터는 `StateStartServerTime` 이 0 이라 경과만 보면 PIE 시작 직후 빨개진다(cl9-4d 죽음 애니 평가기와 같은 함정).
  표현용 넘어짐 시계는 `GetKnockdownElapsed()`(일어서는 순간 −1, 애님 분기용)가 아니라 `GetHurtElapsed()`(일어난 뒤에도 계속 커진다)다.
  **시체 타격은 타격이 아니다** — `HostTickSwingHit` 은 이미 `Dead` 인 몬스터를 세지 않으므로 킥도 없다.
  PIE(slomo 0.15, 오라클 = `GetOverlayMaterial`·`GetHurtFlash`·카메라−컨트롤 회전 편차): 레벨 닭 5마리 시작 직후 오버레이 없음 / 팬 타격
  오버레이 0.056→0.212초(0.156) + 킥 0.66° / 헛스윙 오버레이·킥 0 / 죽는 타격 번쩍 0.1→0.2초 + 킥, 시체 재타격 0 / 나란한 둘 다 0.15초에
  켜짐 / `cl9.damage 1`: 몸·1인칭 팔 오버레이 0.05→0.5초(0.45), 비네트 불투명도 = `GetHurtFlash` 0.972→0(1.93초 표본, 그동안
  HitTestInvisible, 0 이면 Collapsed), 피격 셰이크 0.98°. 2인 PIE(C4 ⑪·C3 상대 화면)와 눈으로 보는 체감은 미확인.
- **2026-09-15 — 연출 시간 셋은 경직·넘어짐 시계에서 역산한다(balance, 초안).** `CL9Rules.HitFlashTime` 0.15초 = 가장 짧은 경직 `HitStunMs` 300ms 의
  절반(상한 0.25, 경직을 넘지 않는다 — 빨강이 먼저 꺼져야 "맞는 순간"과 "경직 중"이 갈린다) / `HurtFlashTime` 0.45초 = `HurtStun` 0.35초를 덮되
  일어서기 시작(0.9초) 전에 꺼진다 / `HurtVignetteTime` 1.8초 = `KnockdownTime` + 0.3초(조작이 돌아오면 화면도 갠다, `KillCooldown` 9초의 20%).
  비네트의 진하기는 C++ 이 아니라 `WBP_HUD` 브러시 알파(단색 0.30, 가장자리 가중 텍스처가 생기면 0.45)가 정한다 — 세기 조정에 재빌드가 없다.
  비네트는 세기 0 이면 `Collapsed` 로 접는다(잔상 원천 차단 + 평상시 전면 이미지를 안 그린다). 한 번 밝아졌다 사그라들 뿐이라 `ui.md` 의
  "번쩍임은 밤 만료 임박에만" 예외를 늘리지 않는다.
- **2026-09-15 — 카메라 흔들림은 `UDefaultCameraShakeBase` + 펄린 노이즈 패턴으로 통일하고, 타격과 피격은 회전 축·주파수로 가른다.**
  `CS_HitShake`(0.15초, Pitch 2.2°@20Hz) / `CS_HurtShake`(0.5초, Roll 3.5°@5Hz), **둘 다 위치·FOV 는 0**(1인칭에서 위치를 흔들면 멀미 —
  위치 축은 `Stamina` 의 지속형 진동이 이미 쓴다). 펄린 노이즈는 임의 위상에서 시작해 순간 진폭을 다 못 쓰므로 짧은 일회성 진폭은 체감값보다
  크게 적는다(Hit 의도 1.5 → 2.2, 조준 상실 문턱 2.5). `ShakeTable` 행 `Hit`·`Hurt` 의 `StartAt` 은 읽지 않는다(게이지형 `Stamina` 전용).
  에셋은 `Tools/Unreal/create_hit_feedback_assets.py` 가 멱등 생성한다 — 값을 바꾸려면 스크립트 상수를 고치고 다시 돌린다.
  **복제로 만들 때 축 배속(`*_amplitude_multiplier`)까지 명시적으로 덮어쓴다** — `CS_StaminaShake` 의 회전 배속이 0 이라 물려받으면 회전이 죽는다.
  실측한 셰이크 값은 앞으로 `docs/systems/character.md` 에 같이 적는다(이번에 `CS_StaminaShake` 값이 문서에 없어 기준이 미상이었다).
  에디터 파이썬 관용구: CDO 는 `unreal.get_default_object(cls)` 모듈 함수로, `TSoftClassPtr` 열엔 `SoftClassPath` 가 아니라 로드된 클래스를 대입.
- **2026-09-15 — 소리는 이번 실행 밖** — `SFX_Hit`·`SFX_Hurt` 는 `docs/systems/sound.md` 에 계획 이름만 잡아 두었고 음원은 만들지 않았다.
- **2026-09-16 — 파동(소음 반경 시각화) 도입: 표시 시점은 표의 열, 기본은 "항상"** (`/cl9` 실행 wave-docs — 문서만, 구현은 별도).
  CCL8 이 정전 전용으로 묶어 두었던 파동(`D:\Work\vibe\CCL8\chicken-heist.html` 2972~3022·4769행)을 CL9 로 옮기면서 상시 채널로 바꾼다 —
  소음 반경을 배우는 일이 밤 내내 일어나야 해서다. 행마다 `Visibility`(`Always`/`NightOnly`/`BlackoutOnly`)로 고를 수 있다. 사용자 결정
  (2026-09-15~16). 문서: `docs/plan/wave.md` · `docs/systems/wave.md` · `docs/plan/data/wave.md`.
- **2026-09-16 — 링 반경은 그 행동의 소음 판정 반경 그대로다.** 파동 표에는 `RadiusScale`(기본 1.0)만 두고 반경은 `CL9Rules` 의 소리 컬럼에서
  읽는다 — 링이 멈춘 곳까지가 몬스터가 듣는 곳이라는 약속을 깨지 않기 위해서. 배율이 1.0 이 아니면 로드 검사가 경고한다. 판정 반경이 없는
  행동(수레·착지·깨짐·몬스터 근접 소나)만 표의 `FallbackRadius` 를 쓰고, 그 행동의 청각이 생기면 `CL9Rules` 로 옮긴다. **걷기는 어그로 0 이라
  `walk` 행은 기본 꺼짐.**
- **2026-09-16 — 몬스터 울음 파동은 무리 경보의 시각 표현이다.** `monster_call` 의 반경은 발신한 개체의 `PackAlarmRadius`(몬스터 표)이고 파동
  표에는 반경을 적지 않는다(`RadiusFrom = PackAlarm` · `FallbackRadius = 0`). 링이 닿은 동료가 실제로 깨어나므로 "링 = 들리는 범위" 가 여기서도
  성립한다. 이 때문에 소음 이벤트의 `Instigator` 는 `AActor*` 다 — 플레이어면 `ACL9PlayerState`(슬롯 색·"내 것인가"), 몬스터면 `ACL9Monster`
  (행 조회). 사살 경보(`KillAlarmRadius`)를 별도 행으로 둘지는 미결. 사용자 결정(2026-09-16).
- **2026-09-16 — 파동 표는 셰이크 표와 같은 틀, 키는 소음원 enum.** `UCL9WaveTable : UDataAsset` 이 `TMap<ECL9NoiseSource, FCL9WaveInfo>` 를
  품고 GameInstance 가 주입한다. `UCL9DefBase` 파생은 상속 컬럼 일곱 중 여섯이 빈칸이라 쓰지 않았고, 키를 enum 으로 둔 것은 발걸음마다 도는
  RPC 페이로드와 "새 소음원은 어차피 코드 한 줄을 요구한다" 는 사실 때문이다. 리소스·크기·종류·파워는 전부 표 열이다(사용자 명시).
- **2026-09-16 — 파동은 Unreliable NetMulticast 로 방송하고 반경은 싣지 않는다.** 위치·종류·낸 액터가 실리는 고빈도 순간 사건이라 복제 시각
  비교(셰이크 방식)로는 담기지 않는다. 유실 허용이라 사운드와 같은 채널. 반경은 각 머신이 `CL9Rules`·몬스터 표·파동 표에서 파생하므로 호스트가
  거짓 반경을 보낼 수 없다. 자기 소음은 소유 클라가 로컬에서 먼저 그리고 멀티캐스트 수신 시 본인 것은 버린다(리슨 호스트 이중 재생도 같은 가드).
- **2026-09-16 — 파동은 옵션이 아니다.** `plan/settings.md` 체크리스트 1번(공평성)에서 걸린다 — 끄고 켜는 것이 곧 "몬스터가 듣는 범위를 보느냐".
  표시·색·밝기는 전부 표 값이고 설정 화면에 뜨지 않는다.
- **2026-09-16 — 파동은 "소리로 길찾기는 본편 밖" 규칙의 예외다.** `docs/plan/loop.md` 귀환 절의 문장은 유지하고 예외를 명시했다 — 파동은
  스스로 소음을 내야만(= 몬스터에게 들려야만) 보이는 수동 채널이라 손전등·시야를 대체하지 못한다. 어둠 속 길찾기 이득이 커지면 표에서
  `bWallStamp` 를 끄거나 그 행을 `BlackoutOnly` 로 돌린다. 사용자 결정(2026-09-16).
- **2026-09-16 — 파동 표 초안 값은 CCL8 원안을 1u = 3cm 로 환산해 잡았다(balance).** `WAVE_SPEED` 170u/s → 510cm/s, 걸음 간격 52u → 156cm,
  `VIS_*` 반경, 자국 기둥 65u → 195cm, 광선 계급 180/540. `OwnerPowerScale` 1.7 은 원안 밝기 0.85 ÷ 0.5. 지어낸 값은 `decoy` 색 하나.
  **전파 속도는 전 행 같은 값(510)** — 다르면 거리감이 무너진다. **벽 자국은 스스로 소리를 낸 행만 찍는다** — `monster_prox`·`monster_call`·
  `walk`·`pickup_quiet` 은 false(몬스터가 찍으면 플레이어가 소리 없이 지형을 읽는 공짜 채널이 된다). 전부 `초안` — PIE 관찰 뒤 확정.
- **(결정 대기) `monster_prox` 링이 플레이어에게 닿지 않는다는 보장** — 현재 검사식 `최종 반경 < TriggerRadius`(330 < 780)만으로는 몬스터가
  330cm 안까지 오면 링이 플레이어를 지난다. 초안은 330 을 닭 시야 반경과 같게 잡아 실질적으로 막았다. 엄밀히 하려면 검사식이 몬스터 최소 접근
  거리(CatchRadius 계열)와 비교해야 한다.
- **2026-09-15 — 짐 흘림은 죽을 때만이다** (사용자 "한 대 피격당하면 아이템 떨구는데 떨구지 않게" → 질문에 "죽을 때만 흘린다";
  `/cl9` 실행 keep-items-cart-lob). 넘어짐(HP 가 남는 타격)은 손에 든 것과 퀵슬롯 네 칸을 그대로 둔다 — 일어나면 그 물건을 그대로 들고
  있다. 2026-09-02 의 "잡히면 손+품 전부 흘림(초안)"을 폐기하고 `HostDropEverything()` 는 HP 0 에서만 부른다(동료가 시체 옆에서 주워 가게).
  넘어짐이 취소하는 것은 **상태뿐**이다(수레 놓기·던지기 차징·휘두르기 대기 = `HostCancelCarryActions()`). **누워 있는 동안은 놓기·던지기도
  무시된다** — 사용·먹기와 같은 `IsKnockedDown()` 규칙(2026-09-16 사용자 답). PIE: 계란 둘을 든 채 `cl9.damage 1` → 손·칸 그대로, 흘림 0,
  누운 동안 G 무시; HP 0 → 손·네 칸 전부 발밑.
- **2026-09-15/16 — 짐칸을 보고 G 면 벽을 넘는 호로 얹는다** (사용자 "수레 쪽에서 떨구면 그냥 밑으로 떨어지는 게 아니라 자연스럽게
  들어가게", 조건은 "수레를 보고 있을 때"). 같은 날 낮의 "놓기는 늘 손 자리, 싣기는 던지기뿐"을 **조건부로 되돌린 것**이고, 2026-09-14 의
  옛 특례(조준점을 짐칸 바닥으로 바꿔치는 순간이동)는 폐기 그대로다 — 이번 것은 초기 속도를 주고 물리에 맡기는 호다(`OnThrown`).
  조건은 `PickRadius` 안에서 짐칸 지점을 향한 조준(내적 0.3 이상, `FindAimedCart` — `ResolveFocus` 는 손에 든 채로는 수레를 안 돌려준다),
  겨냥점은 짐칸 바닥 + 아이템 반지름(`ACL9Cart::GetCargoDropPoint`), 궤적은 **짐칸 벽을 넘는 가장 낮은 호**(벽을 지나는 지점에서 아이템
  밑면이 `CargoVolume` 윗면 + 8cm 위 — `GetCargoWallCrossing` 이 벽 비율 f 를 주고 `z(f)=z0+f·Δz+½gT²f(1−f)` 로 가장 짧은 비행 시간 T 를
  푼다). 정점을 손 자리에 두는 1차 호는 벽 윗단(89cm)에 걸려 튕겨 나왔다(PIE 실측). 되돌아가는 조건은 **보내는 힘 600cm/s 초과 하나뿐**이다.
  표 열은 늘리지 않았다(상수는 코드, 나머지는 `CargoExtent`·`Radius`·`Gravity` 파생). 던지기 클립은 태우지 않는다.
  `SuggestProjectileVelocity_CustomArc` 는 월드 중력으로 풀어 못 쓴다(아이템은 행 `Gravity` 를 힘으로 주입받는다).
- **2026-09-16 — 놓기에서 계란에 금이 가는 것은 규칙이다** (사용자 답 "그대로 둔다"). G 로 놓으면 손 높이 120cm 에서 바닥까지 떨어져
  착지 ≈621cm/s(`CrackSpeed` 450)라 금이 가고, 짐칸에 얹어도 벽을 넘어 바닥(46~49cm)에 닿는 낙차만으로 ≈455~520cm/s 라 금이 갈 수 있다.
  표 값·짐칸 높이·"짐칸 착지 예외" 중 어느 것도 바꾸지 않는다 — 깨짐은 이 게임의 마찰이고, 깨짐 기준의 얹기 속도 상한(`CrackSpeed`×0.9)은
  그래서 폐기했다(1차 PIE: 손↔짐칸 94cm 안에서만 얹혀 수레 긴 변 밖에서는 거의 닿지 않았다).
- **2026-09-16 — 파동 구현 완료(실행 wave-impl): 소음 사건의 단일 입구는 `ACL9GameState::HostReportNoise`, 파동은 반경을 복제하지 않는다.**
  호스트가 enum + 양자화 좌표 + 발신 액터만 Unreliable 멀티캐스트로 뿌리고 각 머신이 `CL9Rules`·몬스터 표·파동 표에서 반경을 파생한다.
  **꺼진 행(`bEnabled` false·로드 검사 비활성)은 사건도 내지 않는다**(걷기 RPC 절감 — 청각 통일 때 재검토). **근접 소나는 깨어 있는 몬스터만**
  (`Asleep`·`WakeTelegraph`·`Dead` 제외). `throw_break` 발신자는 깨진 아이템. 로드 검사 Error 행은 그 자리에서 비활성. 슬롯 색은
  `ACL9PlayerState::GetSlotColor(Fallback)` 이 캐릭터 행 `TintColor` 에서 읽는다(슬롯 번호는 아직 없음). 사용자 결정(2026-09-16).
- **2026-09-16 — 파동의 벽 차폐는 나이아가라 CPU Collision(`CL9_Occlusion` 채널)이 하고, C++ 트레이스는 진단용이다.** 이 에디터의 MCP 로는
  배열 DI 사용자 변수(`User.Distances`)를 못 만들어 원안을 보류했다. 벽 자국은 같은 이미터의 파티클 변형(닿으면 멈추고 `StampHeight` 세로 막대,
  `StampHeight` 0 이면 죽음). 에셋 `NS_CL9_Wave`: 이미터 하나(Ring) — Minimal 템플릿 + SpawnBurst(User.RayCount) + CylinderLocation(반경 45cm·
  높이 +20cm) + AddVelocityFromPoint(User.Speed) + Collision + SolveForces + SetVariables(정지·막대·정렬) + KillParticles. 수명 = (반경−45)/속도,
  루프 Once. **링은 캡슐 밖 45cm 에서 시작한다** — 플레이어·몬스터 캡슐이 `CL9_Occlusion` 을 막아 발밑에서 태어나면 즉시 충돌하기 때문(후속:
  폰 콜리전을 이 채널에서 Ignore 로 두면 45 를 줄일 수 있다). **미반영 후속**: `User.Power`(밝기 배율)와 알파 페이드 — HLSL 로 색을 만들면
  파티클이 사라져(색 계열 HLSL 입력 실패) 뺐다. Power 는 C++ 에서 색 RGB 에 곱해 넘기는 한 줄, 페이드는 Color 모듈(커브)로. 표 `WaveTable`
  (`/Game/Data/WaveTable`, 11행·전역 2, balance 초안 값 그대로)은 `BP_CL9GameInstance.WaveTableAsset` 에 물렸다.
  PIE(T1·T4·T5·T7 계열, 1인): 달리기 링 390cm = `cl9.shownoise` 원, 걷기 981cm 동안 사건 0, `land` 378·`pickup_loud` 390, 몬스터 근접 소나
  1.33초 간격 330cm(잠든 몬스터 제외는 코드로), 깨짐 파동은 표 색(주황)으로 표시. T2(벽 앞 자국)·T3(2인)·T6(정전)은 미확인.
- **2026-09-16 — 파동 그림 정정(사용자 지적): 끝 지점 원 없음 · 점이 아니라 원 · 바닥 밀착.** 흰 끝 원은 `cl9.shownoise` 디버그였다(평소 꺼짐).
  점 수를 표 열이 아니라 **둘레 ÷ (두께 × 0.35)** 로 이미터 안에서 정해(하한 `RayCount`) 점 간격이 점 크기보다 좁게 — 달리기 링은 583개로
  끊김 없는 선이 된다. 스폰 높이 20cm → 3cm. 리본 렌더러·위를 향한 스프라이트·색 HLSL 은 이 MCP 경로에서 그려지지 않아 폐기
  (docs/systems/wave.md "나이아가라 에셋의 실제 모양"). 두께는 표 `Thickness` 로 조절(줄이면 선이 날카로워지고 점 수는 자동 증가).
- **2026-09-18 — 타격감·사살감 1차(실행 feel-hit-kill, 사용자 "타격감·파밍·때려잡는 기분이 부족하다").** 진단: 소리 0개·히트스톱 없음·킥 0.66°·
  타격 파티클 없음·죽음 연출 없음. 판정(히트박스·경직·HP·드랍)은 그대로 두고 표현 층만 얹었다 — 효과음은 rules 열이 아니라 `SoundTable` 행
  (`CL9SoundNames` 7개, `Content/Data/SoundTable`, `BP_CL9GameInstance.SoundTableAsset`)이며 재생은 전부 복제된 시계·상태의 틱 비교(상승 에지)로
  모든 머신에서 파생한다(새 RPC·멀티캐스트 없음). `Place` 는 OnPlaced/OnThrown 이 호스트 전용이라 `ApplyHoldState` 의 Held·Worn→Free 전이에서
  낸다. 없는 행은 조용히 무시. 정본 `docs/systems/sound.md`·`monster.md` "타격의 표현".
- **2026-09-18 — 효과음 7종은 파이썬 합성 임시 음원이다** (`Tools/Audio/synth_sfx.py`, numpy 아닌 순수 파이썬·고정 시드·멱등, `ArtSource/audio/sfx/*.wav`
  → `Tools/Unreal/import_sfx.py` → `/Game/Sounds/SFX/…`, `SC_Effects`, Concurrency 4, 발소리 3개는 SoundCue Random `S_Footstep` 하나). 실음원이
  들어오면 같은 이름의 WAV 를 덮어쓰고 임포트 스크립트를 재실행하면 행 편집 없이 바뀐다(`ArtSource/audio/README.md`).
- **2026-09-18 — 히트스톱은 전역 시간 지연·`CustomTimeDilation` 이 아니라 액터별이다.** 몬스터는 `bPauseAnims`(모든 머신, `CL9Rules.HitStopTime` 0.08초),
  1인칭 팔은 ABP 전용 게터 `GetSwingAnimElapsed()` 가 명중 시각에 머무는 방식(때린 본인만) — 판정 게터 `GetSwingElapsed()` 와 히트박스 창은 불변,
  `ABP_FP_View`·`ABP_Unarmed` 의 `SwingElapsed` 변수 소스만 바꿨다. PIE 실측: 정지 0.05→0.11초, 팔 차이 0.08.
- **2026-09-18 — `CS_HitShake` 는 사인파 오실레이터(`WaveOscillatorCameraShakePattern`, 모든 축 InitialOffset Zero)다** — 2026-09-15 "펄린으로 통일" 을
  타격 하나만 번복(사용자 결정). 펄린은 임의 위상이라 실측 0.66°/진폭 2.2°(30%)로 정점을 보장 못 한다. 값은 Pitch −3°@4Hz·0.12초·블렌드 아웃 0.04·
  FOV −3°@4Hz, 정본은 `Tools/Unreal/create_hit_feedback_assets.py` `HIT_SHAKE`. PIE 실측 피치 정점 2.83°·FOV 90→87.15. `CS_HurtShake` 는 펄린 그대로.
- **2026-09-18 — `ShakeTable` 행에 배율 `Scale`(기본 1.0)을 둔다(사용자 "꺾이는 정도를 표에서 세팅").** 원샷 행은 `Scale` 세기로, 게이지 행은 게이지 ×
  `Scale` 로 재생 — 진폭·주파수·블렌드는 여전히 에셋이고 표는 배율만 갖는다(2026-09-05 "셰이크 값은 표 열로 두지 않는다"의 유일한 예외).
- **2026-09-18 — 죽음 발사는 호스트 적분이다(월드 중력 980, `DeathLaunchSpeed` 300·`DeathLaunchUp` 260).** 래그돌(물리 에셋 없음)·`LaunchCharacter`
  (Character 아님) 기각. 벽·바닥 검사는 몸체 구 스윕도 WorldStatic 오브젝트 질의도 아닌 **Visibility 채널 선 트레이스**다 — 바닥·맵 조각의 콜리전
  프로필이 제각각(Lvl_Flat `Floor_0` 은 BlockAllDynamic = WorldDynamic)이라 오브젝트 타입을 믿을 수 없다. 폰·아이템·몬스터에 맞으면 무시하고 뒤를
  다시 긋는다. 드랍은 몬스터 중심 높이·옆 30~90cm 오프셋에서 `OnThrown` 호(`DeathLootSpeed` 300 — 계란 표 중력 1950·앙각 70° 착지 371 < `CrackSpeed`
  450). PIE 실측: 정점 +28cm·0.55초·수평 150cm 착지(z 37.7→35.5).
- **2026-09-18 — 발소리는 애님 노티파이가 아니라 거리 누적기다** (`TickFootstepSound`·`TickFootstep`, 모든 머신, 소음 판정 `TickStepNoise` 와 별개).
  보폭은 종별 열 없이 `CL9Rules.FootstepStrideRun` 196(= 654 × 0.3초, `AN_Farmer_FPRun` 루프 반 = 팔 펌프와 잠금)/`FootstepStrideWalk` 150 공용 —
  폰은 속도 > `WalkSpeed` 로, 몬스터는 Chase 로 가른다. 몬스터 보폭도 이번엔 사람과 같은 값(사용자 "이번엔 이대로 들어본다" — 종별 열은 다음에).
- **2026-09-18 — 타격 연출 수치는 판정 시계와 표 물리에서 역산한다(balance, 초안).** `HitStopTime` 0.08 = `HitFlashTime` 0.15 안·히트박스 창 폭 0.20 의
  절반 아래. `DeathLaunchSpeed` 300 = 복도 한 칸 500cm 에서 가로로 맞아도 벽 앞에 눕는 수평 159cm. `HpPipsTime` 2.0(다음 실행).
- **2026-09-20 — 잡기 예고(`Pounce`) 도입 — 2026-09-15 "몬스터 예비동작 반려(판정을 늦춰야 해 규칙 2 위반)" 의 번복이 아니다(사용자 게이트 결정, 실행 monster-pounce-hurt-readability).**
  규칙 2(animation.md — 애님이 판정에 못 들어간다)는 그대로다. 판정 지연은 표 값 `PounceWindup` 이 정하고 클립 `AN_Chicken_PounceWindup`(0.6초)은 그 값에 따라갈 뿐이라,
  클립이 없거나 길이가 달라도 판정은 같다(다른 종은 클립 없이 로코모션 유지). 값은 "걷기로 물러나도 피한다"(사용자 D2): `PounceRadius` 130 · `PounceWindup` 0.6 ·
  `PounceLunge` 110 · `PounceMissCooldown` 0.6(초안) — 걷기 396 으로 0.6초에 238cm 물러나면 130+238 > 84+110 이라 헛침. 닭이 순해지는 것을 감수하고 긴장은 재예고·마릿수로.
  `PounceWindup 0` 또는 `CatchRadius 0` 이면 옛 즉시 잡기(빌드 없이 롤백). 덮침은 몸체 구 스윕(WorldStatic|WorldDynamic, 캐릭터·아이템·몬스터 무시)으로 `PounceLunge` 만큼,
  도달 위치에서 거리·높이·시야·넘어짐을 재검사. 헛침 복귀는 `ResumeChase()`(예산·놓침 시계 보존). 결과는 `PounceResultServerTime`+`bPounceHit` 복제 + `ForceNetUpdate`.
  PIE(Lvl_Flat 1인): 예고 진입 105~129cm → 0.6초 → 110cm 덮침 → 잡힘; 물러나면 5회 연속 헛침·재예고, HP 불변.
- **2026-09-20 — 넘어진 동안 무적(사용자 — 명세 "미룬 일" 에서 실행으로).** `ACL9Character::OnDamage` 머리의 `IsKnockedDown()` 게이트가 HP·넘어짐 시계·피격 사건을
  전부 그대로 둔다(로그 한 줄). 다마리 연타 방지가 목적이고 **`cl9.damage` 치트도 같은 문을 지나 우회가 없다**(HP 0 까지 치려면 1.5초 간격). 몬스터 쪽은 타깃 선택이
  아니라 Chase 틱의 잡기·예고 진입 검사에서 넘어진 타깃을 건너뛴다 — 옆에 서서 기다리다 일어서는 순간 예고(타깃 선택에서 빼면 시야 상실·동료로 튀어 그림이 깨진다).
  예고 중이던 닭은 덮치되 재검사에서 헛침. 표 스위치는 없다(규칙). PIE: 넘어진 0.5초에 `cl9.damage 1` → HP 2 그대로.
- **2026-09-20 — 잡히는 순간 카메라는 `UCameraModifier`(POV 오프셋)로만 본다 — 컨트롤 회전·시선 입력 불변.** `FCL9HurtEvent{ServerTime, From, Attacker, Seq}`(COND_OwnerOnly)
  + `UCL9HurtLookModifier`(`NotifyControllerChanged` 에서 소유 클라만 등록). 피치는 넘어짐 시계만으로 EaseOut 진입(`HurtLookTime` 0.45 → `HurtLookPitch` −25), 요는 사건의
  `From` 방향으로 `|Δ| − FOV/2` 만큼 `±HurtLookYawMax` 60 안에서 가장자리 걸치기, GetUp 0.6초에 0 복귀. 재피격(`Seq`)은 요 목표만. `Seq` 는 `uint16` 이라 BP 미노출.
  `HasHurtDirection()` 은 `IsValid(Attacker)` 로 "공격자 있었음" 을 판단(시체는 밤 동안 안 사라진다). PIE 실측: 0.40초 −24.7°, 0.74초 −25.0°, 1.40초 −7.5°, 일어나면 0;
  컨트롤 회전 (0,180) 불변; 요 오프셋 +20.7°(닭 상대 66° → 호 각도 45°). balance 관찰: 넘어짐 하강(30cm)과 겹치면 닭이 위 가장자리로 밀릴 수 있어 후보 −12 를 표에 적어 둠.
- **2026-09-20 — 피격 방향 호는 표 bool `CL9Rules.bHurtDirectionIndicator`(기본 true)로 끈다.** 계획 초안의 "리썰 컴퍼니 포함 관례" 는 틀렸다 — 리썰에는 방향 표시가 없다
  (비네트·킥·소리뿐). 호는 글자·아이콘이 아니라 비네트와 같은 가장자리 물듦(같은 `GetHurtFlash()` 페이드)이라 "글자도 아이콘도 없다" 안에 두되 톤이 어긋나면 빌드 없이
  표에서 끈다. 런타임 숨김은 `Hidden`, `Collapsed` 는 디자이너가 끈 신호라 C++ 이 영영 켜지 않는다(WBP 기본도 Hidden). 각도는 `GetHurtDirectionAngle()` 을 변환 없이
  `RenderTransform.Angle` 에. 텍스처 `T_CL9_HurtDirection` 은 리드 임시본(파이썬 생성, 12시 75° 호).
- **2026-09-20 — 소리는 상태 전이 감지·`SoundRow` 열로 낸다.** `MonsterPounce`·`MonsterCatch` 는 `TickStateSounds`(모든 머신, `State`/`StateStartServerTime` 변화 = 전이, 첫 틱
  무음) — `TickHitFeel` 의 Stun·Dead 창과 별개라 Catch 가 관측 전에 Stun/Dead 로 덮이면 그 잡기 소리는 빠진다(`Hurt` 는 난다). `MonsterNear` 는 `FCL9WaveInfo.SoundRow`
  (`monster_prox` 행 = `MonsterNear`) — `PlayWave` 가 MinInterval 뒤·시각 게이트 앞에서 `PlayAt`, 코드 분기 없음. 그래서 **`MinInterval` 스탬프를 시각 게이트 앞으로**
  옮겼다(컬링·표시 시점에 걸린 사건도 간격을 찍는다 — 링 그림 불변). `Init` 이 `SoundTable` 행을 전부 프리로드. 박자는 `monster_prox.RepeatInterval` 1.2초(명세의 0.65초는
  링 수명이었다 — docs 정정). PIE: `au.DumpActiveSounds` 에 S_MonsterNear·S_MonsterPounce·S_MonsterCatch·S_Hurt 전부.
- **2026-09-20 — 예고 동안 감각 정지.** `Pounce` 는 `bSensesAwake` 에서 빠진다 — 재타깃팅 금지를 가장 단순하게 지키는 방법. `bHadSight` 가 0.6초 얼어 놓침 시계가 그만큼
  늦게 도는 것은 `ChaseLoseTime` 6초 앞에서 무시할 만하다. `cl9.damage` 의 Source 는 가장 가까운 **살아 있는** 몬스터(시체 제외).
- **2026-09-20 — 잡기 예고 값의 종별 비율 규칙(balance 초안)·부등식 A5.** `PounceWindup`·`PounceMissCooldown` 은 사람 반응 상수라 종 공통 0.6, `PounceLunge` 는 추격 속도
  0.28초 거리(닭 110/396 ∝ `ChaseSpeed`), `PounceRadius` = `CatchRadius + PounceLunge − 64`(이상적 후퇴 거리 64cm 를 모든 잡는 종에서 같게). 너구리 150/0.6/130/0.6, 돼지·양은
  닭과 동일, 거위는 Pounce 없음. A5: `PounceRadius + WalkSpeed × PounceWindup > CatchRadius + PounceLunge` ∧ `PounceRadius ≥ CatchRadius`(`check_inequalities.py`, 전부 통과).
  `monster_pounce` 링 반경 306 = `Speed 510 × PounceWindup 0.6`(링이 다 퍼지는 순간 = 덮치는 순간, `SoundRow None`) — `PounceWindup` 을 바꾸면 이 행도 같이.
- **2026-09-20 — `AN_Chicken_PounceWindup`(0.6초, 닭만)은 끝 프레임이 Catch 첫 프레임이다.** 예고 끝을 Catch(덮침)·Catch 앞 0.3초(헛침)가 같은 프레임에서 잇기 위해.
  반응 클립 재작업 도구는 클립을 골라 굽는다(`animate_creature_reacts.py --clips`, `reimport_creature_clips.py CL9_CLIPS`) — FBX 헤더 시각 때문에 전부 다시 구우면 바뀌지 않은
  클립까지 바뀐다. 닭 리그 날개 뼈는 좌우 롤이 달라 기존 `wing` 은 비대칭(관찰 기록) — 새 클립만 `spread` 로 대칭, 리그 수정은 별건. 합성음 크기 차이는 합성 피크로
  (예고 −5 < 잡기 −3, 근접 −10 dBFS). `ABP_Chicken_A` 는 기존 관례대로 float 알파(`AlphaPounce`·`AlphaPounceMiss`) + 양방향 블렌드로 넣었다(리드 MCP).
- **2026-09-20 — 휘두르기 히트박스 전역 배율 `CL9Rules.SwingHitScale`(사용자 "히트박스가 너무 작아 플레이어가 몬스터를 못 때린다", 실행 swing-hit-scale).** "보이는 것이
  맞는다"(메시 경계 구)는 유지하고 그 위에 배율 하나만 — 아이템 행별 배율·메시 수정 없음, 호스트 스윕 반경에만 곱하고 중심 불변(`HostTickSwingHit`). 헤더 기본값 1.0(옛 동작),
  에셋 셀 **2.2 초안** — "닭 폭 하나 빗나가도 맞는다" 의 닭 폭 = 반폭 36cm(접촉 66 → 102cm). 전폭 기준 3.4 는 손 앞 팬 구 중심 ≤ 80cm + 30×3.4 + 36 = 218cm 로 2m 밖 닭(C4 ⑦)
  까지 닿아 기각, 상한 S < 2.8. 고기 `Swing` 행도 같은 배율. 롤백 = 셀 1.0(빌드 없이). 검증은 C4 ⑬(옆으로 비껴 서서 휘두르기) — 육안, 사용자 몫.
- **2026-09-20 — 처형 시퀀스(돼지 전용 즉사기) 도입, Run 1(실행 execute-sequence-pig).** 사용자: 종 = 돼지, 트리거 = 등 뒤 + 미발견(브레켄형, **스토킹 AI 포함**), 동료 구조 불가, 카메라 =
  몬스터 소켓 카메라. `/autogoal` HEAVY — 패널 3관점 + 교차 심문, 명세 `docs/goal/execute-sequence-pig.md` D1~D9. 상태 `Stalk`·`Execute`(enum 끝). 스토킹: `IsWatchedByAnyone()`
  = 살아 있는 어느 플레이어든 2D 요 콘(`StalkGazeHalfAngle` 55) + LOS + `StalkGazeRadius` — 보이면 정지·`StalkRetreatRadius` 안이면 후퇴, `StalkStareAnger` 응시면 격분(Chase), 안 보이면
  등 뒤 지점으로 `StalkSpeed`. 처형 진입 = 미발견 `ExecuteUnseenTime` ∧ `ExecuteRadius` ∧ 등 뒤 원뿔 `ExecuteBehindAngle` ∧ LOS ∧ KillCooldown ∧ 타깃 상태 ∧ 앵커 스윕. PIE(1인):
  433cm 에서 Stalk, 보는 동안 정지(거리 불변), 등 돌리고 3.3초 뒤 Execute — 돼지가 정확히 140cm 뒤로 텔레포트, 피해자 위치 불변, 뷰 타깃 = 돼지, 1.33초 뒤 HP 3→0, 4.43초 뒤 Break.
- **2026-09-20 — 처형은 돼지를 옮긴다 — 피해자 캡슐 불변(D1).** 진입 시 호스트가 돼지를 `TeleportPhysics` 로 피해자 등 뒤 앵커(피해자 위치 − 전방 × `ExecuteAnchorDistance`)에 놓는다.
  앵커는 구 스윕(돼지 `Radius`, WorldStatic|WorldDynamic, 캐릭터·아이템·몬스터·피해자의 `PullingCart` 무시)이 비어 있을 때만 — 막히면 그 틱은 처형하지 않고 스토킹 계속. 앵커 높이는
  몬스터의 지금 Z. 피해자 캡슐·충돌·위치는 건드리지 않는다(규칙 3). 로드 검사 `ExecuteAnchorDistance ≥ Radius + 39`. 기각: 피해자 캡슐을 NoCollision 으로 스냅(규칙 3 위반·클라 예측 충돌·벽 뚫림).
- **2026-09-20 — 처형은 구조 불가 — 처형 중 피격은 끊지 않는다(D4).** `Execute` 의 돼지는 `OnDamage`/`OnShove` 에서 HP 만 줄고 Stun·넉백·상태 변화가 없다. 처형 중 돼지가 죽어도 피해자는
  `HostKill` 로 즉사. 기각: 돼지가 죽으면 풀어 주기("구조 없음"과 모순). 킬은 전용 `HostKill(Source)` — 넘어짐 무적 게이트 앞을 지나 기존 사망 흐름(`HostApplyDamage` 공통부)으로;
  몬스터 타깃 선택·Catch·Pounce 진입·덮침 재검사는 `IsExecuted()` 타깃 제외(다른 닭이 넘어뜨려 "살리는" 구멍 봉쇄).
- **2026-09-20 — 처형 카메라 = 1인칭 고정의 관전 범주 예외(D6).** 피해자 뷰 타깃은 `ACL9Monster` 의 `UCameraComponent`(기본 비활성, `ExecuteCameraSocket/Offset/Rotation` 표 열, `head` 본)로
  — 관전과 같은 서버 `SetViewTarget` 경로(`SetViewTargetWithBlend`, `CL9Rules.ExecuteCameraBlend` 0.3). 죽은 뒤 뷰 타깃은 폰으로 **돌아오지 않고** 돼지가 `ExecuteDuration + SpectateDelay`
  까지 머문 채 관전으로 직행(넘어짐 카메라가 이제 보이는 몸 안에 앉기 때문). 소유 클라는 몸 `OwnerNoSee` 해제·팔 숨김·입력 컨텍스트 내림(`ApplyExecuted`), 셰이크·포커스·피격 게터·
  `UCL9HurtLookModifier` 는 `IsExecuted()` 게이트. 기각: `UCameraModifier` POV(뷰 타깃이 폰이라 겹침), 별도 카메라 액터(복제·수명).
- **2026-09-20 — 처형 세부(gameplay).** 수레는 처형 **진입 순간** 놓인다(`HostCancelCarryActions`), 짐은 죽는 순간. `Execute` 진입 전이에 `MonsterCatch` 소리. Stalk 진입은 팩 경보를
  울리지 않는다(격분해 Chase 로 갈 때). 서버 RPC 게이트를 `IsIncapacitated()`(넘어짐 ∥ 처형)로 통일 — **넘어진 동안 줍기·칸 전환·헤드램프도 새로 막힌다**(사용자 확인 2026-09-20).
  `HostEndExecuted` 는 관전 중이 아니면 뷰 타깃을 폰으로. 치트 `cl9.execute` 는 LOS·KillCooldown 까지 우회(앵커 스윕·타깃 검사는 유지). 밤 종료·부활은 몬스터 파괴 전에 처형 상태 클리어(D9).
- **2026-09-20 — 돼지 재정의(balance, 초안): "무겁고 느린 스토커".** 몸 `Radius` 45·HP 6/8, `WanderSpeed` 100·`StalkSpeed` 300(걷기 396 보다 느려 **멈춘 사람**을 잡는다 — 맵이 작아
  브레켄과 반대), 격분 `ChaseSpeed` 480(A1 여유 +26.6)·`TurnRate` 220·`TurnDrag` 0.6, `PounceWindup` 0.8·`PounceLunge` 130·`PounceRadius` 166·`CatchRadius` 100. 처형 초안:
  `ExecuteRadius` 160·`ExecuteBehindAngle` 60·`ExecuteUnseenTime` 3.0·`ExecuteDuration` 1.2(Run 1 = Catch 클립 0.8 + 유지)·`ExecuteAnchorDistance` 140·카메라 `head`+(−60,0,125)/(−38,0,0)·
  `StalkGazeHalfAngle` 55·`StalkGazeRadius` 900·`StalkStareAnger` 2.5·`StalkRetreatRadius` 250·`RetreatSpeed` 200. 부등식 A6(앵커 ≥ Radius+39)·A7(후퇴 반경 ⇒ 후퇴 속도)·
  회피 산수(180 − 55 > 60, 미발견 3 > 복제 0.1) 신설, 전부 통과.
- **2026-09-20 — 처형 애니메이션은 Run 2 로.** Run 1 은 돼지 `AN_Pig_Catch` 평가기(마지막 프레임 유지) + 농부 `Locomotion` 캐시 포즈 정지(`ABP_Unarmed` Executed 가지 =
  넘어짐 가지보다 출력에 가까운 자리, 조건 `IsExecuted ∧ InRange(ExecuteElapsed, 0, ExecuteDuration)`). Run 2 는 `AN_Pig_Execute`·`AN_Farmer_Executed` 를 **각 blend 따로**(UE 액터 공간
  앵커 표 공유)로 굽고 조건을 `IsExecuted` 로 바꾼다. 기각: 한 씬 append(리그별 export 축·씬·저장이 달라 미리보기 정렬이 거짓).
- **2026-09-20 — 몬스터 4종 정점색 머티리얼.** 거위·너구리·돼지·양의 `M_CL9_<종>_Vertex` 가 있었으나 `SK_<종>_A` 슬롯에 배정되지 않아 `WorldGridMaterial` 로 보였다 —
  닭과 같은 구성(VertexColor → BaseColor, Roughness 0.86)으로 배정. `Tools/Unreal/create_creature_materials.py`(멱등).
- **2026-09-20 저녁 — 처형 종 재배치: 스토커는 너구리, 돼지는 잡히면 무조건 처형(사용자 결정, 실행 execute-species-swap).** 사용자가 Lvl_Flat 배치 돼지로 시험했을 때 "하나씩 깎인다" —
  잠든 돼지를 깨우려고 보는 것이 응시 격분(2.5초) 시계를 돌려 보통 잡기가 됐기 때문. 결정: Run 1 의 Bracken 식 스토킹·등 뒤 처형은 **너구리**(`MonsterTable.raccoon`)로 — 확정값
  `StalkSpeed` 450(걸어도 따라붙고 달리기 654 로만 뗀다), `StalkStareAnger` 6(흘깃은 안전), `SleepTime` 0(안 잔다); 초안 `ExecuteRadius` 140·`ExecuteAnchorDistance` 120·`ExecuteUnseenTime` 3.0·
  `ExecuteDuration` 1.2·카메라 `head`+(−75,0,110)/(−28,0,0)·`StalkRetreatRadius` 250. **돼지**는 스토킹 없이 닭처럼 추격·예고·덮치되 **잡히는 순간이 곧 처형** — 새 열 `bExecuteOnCatch`
  (bool, 기본 false, 돼지 true): `HostCatch` 맨 앞에서 `HostBeginExecute(Target, 제자리, bTeleport=false)` — 순간 이동·앵커 스윕 없음(`OnDamage`·`Catch` 휴지기·`CatchDamage` 를 타지 않는다).
  기제는 표 값으로만 갈리고 코드는 종 이름을 모른다(`IsExecutioner()` = `ExecuteRadius > 0 || bExecuteOnCatch`); 로드 검사 `bExecuteOnCatch ⇒ ExecuteDuration > 0`; 롤백은 돼지
  `bExecuteOnCatch false`, 너구리 `ExecuteRadius 0`. 돼지 `ChaseSpeed` 480 → 396(= 걷기 — 즉사 종이 걷는 사람을 좁히면 안 된다, balance 판단·대안 440~480 은 economy.md 후보),
  `PounceLunge` 110·`PounceRadius` 146. 치트 `cl9.execute` 는 `IsExecutioner()` 종을 고르고 잡히면-처형 종은 스윕 없이 즉시. 두 갈래가 동시에 켜진 종은 둘 다 유효(배타 아님 — 미결).
  PIE(1인 Lvl_Flat): 돼지 CHASE → POUNCE → **EXECUTE**(Catch 없음, 덮침 110cm 만 이동) → 1.33초 뒤 HP 3→0 → 4.4초 뒤 Break; 너구리 143cm 에서 Stalk·보는 동안 287 까지 후퇴, 등 돌리자
  1초 만에 123cm 로 접근(450)·3.33초 뒤 Execute(120cm 텔레포트)·즉사·Break. 정본 `docs/goal/execute-sequence-pig.md` 상단 절, PIE `C-처형-돼지`·`C-처형-너구리`.
- **2026-09-20 밤 — 처형 카메라는 소켓이 없으면 액터 루트에 액터 공간으로 붙는다(리드 직접 수정, 사용자 "돼지 뒷다리 사이밖에 안 보임").** 메시는 `MeshYawOffset`(돼지 −90)만큼 돌아 있고
  `Radius` 만큼 내려가 있어 `head` 본 소켓 공간 오프셋이 종마다 엉뚱한 축으로 갔다. `ApplyState` 가 소켓이 있으면 메시 소켓, **없으면 `GetRootComponent()`(구 콜리전)** 에 붙이고 표의
  `ExecuteCameraOffset` 은 "X = 전방(타깃 쪽)·Z = 위" 액터 공간으로 읽힌다. 셀: 돼지·너구리 `ExecuteCameraSocket None`, `ExecuteCameraOffset` (−130, 0, 165), `ExecuteCameraRotation` (−30, 0, 0)
  — PIE 스크린샷 `.claude/runs/execute-species-swap/pie_pig_execute_camera.png`(돼지 등 위에서 피해자 전신·파동 링이 보인다). 본 소켓(머리 흔들림)은 열이 남아 있어 나중에 되살릴 수 있다.
  `docs/plan/data/monster/pig.md`·`raccoon.md` 의 카메라 3열 값은 이 셀로 갱신해야 한다(balance 문서 미반영 — 다음 세션).

- **2026-09-20 밤 — 데스캠(죽음 카메라) 도입, execute-sequence-pig D6 일부 번복(실행 death-camera).** 사용자가 "죽은 뒤 카메라가 이상하다 — 내 캐릭터가 보이는 위치로"
  라고 해서, 죽는 순간부터 관전 전까지(혼자면 계속) 폰의 `DeathCamera` 컴포넌트가 액터 공간 오프셋 `CL9Rules.DeathCameraOffset` 에서 `(0,0,DeathCameraLookAtZ)` 를 보게 했다.
  게이트는 `ShouldUseDeathCamera()`(= `IsDead()` ∧ 오프셋 ≠ 0) — 롤백 `(0,0,0)` 이면 전부 옛 동작. 데스캠이 켜지면 처형과 같은 표현 게이트(피격 카메라·셰이크·비네트·호·프롬프트)가
  닫힌다. HP 0 순간 `HostApplyDamage` 가 PlayerState `ForceNetUpdate` 뒤 `SetViewTargetWithBlend(폰, DeathCameraBlend)` — 처형 즉사는 몬스터 카메라 → 데스캠 0.5초 블렌드,
  **보통 사망은 즉시 전환**(같은 폰 안 두 카메라 사이엔 엔진 블렌드가 없다 — 폰 안 블렌드는 `CalcCamera` 오버라이드로 후속 후보). 몬스터가 시체 옆에 `SpectateDelay` 까지 머무는 것·
  처형 카메라·나머지 D6 은 그대로. 죽은 채 마우스 시선은 막지 않았다(캡슐 요를 따라 시체 중심 궤도) — 막을지는 미결.
- **2026-09-21 새벽 — 3인칭(처형·데스캠) 동안 몸 메시는 `FirstPersonPrimitiveType` 도 `None` 으로 내린다(리드 직접 수정).** 첫 데스캠 PIE 에서 시체가 안 보이고 그림자만 남았다 —
  `WorldSpaceRepresentation` 프록시는 `bOwnerNoSee` 를 강제로 true 로 덮어써서(엔진 `PrimitiveSceneProxy.cpp`) `SetOwnerNoSee(false)` 가 소용없고, 뷰 액터가 자기 폰인 데스캠에서만
  드러났다(처형 카메라는 뷰 액터가 몬스터라 원래 보였다). `ApplyThirdPersonMeshes()` 가 타입을 `None ↔ WorldSpaceRepresentation` 으로 같이 토글한다. 셀·헤더 기본값
  `DeathCameraOffset` (−320, 140, 260)·`DeathCameraLookAtZ` −60(초안 (−240,90,190)/40 은 돼지가 시체를 가리고 시체가 화면 아래로 밀렸다) — PIE 스크린샷
  `.claude/runs/death-camera/pie_deathcam_kill+1s.png`(돼지 밑의 시체)·`pie_deathcam_kill+5s.png`(돼지가 떠난 뒤 누운 몸 전체).
- **2026-09-21 — docs 전체 정합성 감사(159개 md)와 정정. 발견 136건(높음 37·중간 62·낮음 37), 보고서 `docs/review/doc-audit-2026-09-21.md`.** 뿌리는 하나 —
  09-08~09-20 의 결정·구현이 decisions·systems 일부·코드에는 들어갔으나 기획 정본(plan)·진행 정본(build-order·overview·production-progress·catalog)·
  에셋 README·규약(conventions)에는 되돌아오지 않았다. 원칙 "문서와 코드가 어긋나면 문서가 틀린 것" 대로 문서를 코드·셀에 맞췄고 값은 이 로그·헤더·
  스크립트에서만 가져왔다. **정정한 것**: 처형 카메라 3열(pig.md·raccoon.md 셀 None/(−130,0,165)/(−30,0,0))·"메시 루트"→액터 루트(common.md·monster.md·
  `CL9MonsterDef.h` 주석); 퀵슬롯 전환 잔존(overview 마찰·flashlight/headlamp 품·cooking 놓기(E)·light/warning/sound/README/wave 주석) 제거; 타격감
  (asset/README `CS_HitShake` 사인파·`Scale` 열·drop.md `OnThrown` 호·actor-base 사운드 멤버 삭제→`CL9Sound::PlayAt`); 파동 "미구현" 꼬리표 5곳·
  `monster_pounce` 12행·schema.md 표 2행/A5~A7; 맵 모듈(module.md 구현됨·`BaseModuleCount` 헤더 24/셀 40·`UpperFloorShare`·15열·`Grid`·`BaseZMode`
  잔존·loop.md 격자 서술); build-order 상단·⑤ 표·"④~⑤ 이후 더해진 시스템" 절, overview "지금 어디까지 왔나" 절 삭제; production-progress 재개 지점·
  catalog·카드 25장 상태 줄·`map/campfire.md` 신설·README 색인 3곳; goal 16편 상태 줄·`goal/README.md`·`review/README.md` 신설; cheats.md 를 코드 등록
  21개와 1:1(`REGISTER_CL9_CHEAT` 없음 — `FAutoConsoleCommandWithWorldAndArgs` 블록 + `HandleCommand` 분기)로, `cl9.resetkeys`·`playseq`·`testwake`·
  `testvoice`·`fxoff` 는 삭제(없는 명령); 스태미나 "바 없음"(warning·character)→바+흔들림; headlamp 값 1800/16000/26°·goose 표 `CatchRadius 0`
  (`check_inequalities.py` 판정 불가 해소)·item/common 빛 3열·equipment `pot`; save.md `FCL9AcceptedOrder`; animation.md 보유 에셋 표(농부 A·몬스터 5종 자체
  제작, `ABP_FP`/`Content/FirstPerson/Anims` 없음)·CLAUDE.md 같은 줄; `CL9TuningHash` 현재형 9곳→"설계만, 미구현". **규약 명문화**(conventions.md):
  §4 Character Replicated 예외 = 이동 예측 + 표현 파생 서버 시각(넘어짐·처형·`FCL9HurtEvent`) + 소지 실물(`HeldItem`·`WornItem`·`PullingCart`, PlayerState 는
  미러); §6·§9 리슨 호스트 표현은 OnRep 이 아니라 복제 시각 틱 비교(09-15·09-18 결정의 규약 반영); §2 RPC·호스트 함수 명명(`Server_*`·`Client_*`·
  `Multicast_*`·`Host*`·`OnRep_*` — 코드 관행 그대로); §3 소스 트리 9폴더·Content `Sounds/`; `GetGameMode()`→`GetAuthGameMode`. 이 로그의 "추가만 한다"
  는 09-12·13 개명 커밋이 과거 본문을 치환한 것을 유일한 예외로 머리말에 명기했고 `## 2026-09-02` 중복 헤더는 "(계속)" 으로 갈랐다. **코드 4곳**:
  `CL9DefRegistry.cpp` `bSeenType[5]`→`[CL9ModuleTypeCount]`(09-13 리뷰 A-1, `ECL9ModuleType` 6개라 범위 밖 쓰기), `CL9MonsterDef.h` 카메라 3열 주석,
  `cl9.execute`·`cl9.list` 도움말(돼지 `bExecuteOnCatch`·`monsters` 인자) — `compile_check.py` OK. **결정 필요(미결로 남김)**: ① GameState·PlayerController 가
  `SetNetUpdateFrequency(20)` 을 부르지 않는다(규약은 "복제 액터 전부") ② `IA_Throw`·`IA_Swap` 에셋·`Content/FirstPerson/Anims/` 빈 폴더 삭제 ③
  `ECL9CarrySlot::Pocketable`·문자열 `Hud.ItemPocketable`("품에 들어감")·`bLightLostInPocket` 이름이 퀵슬롯 모델과 어긋난 채 코드·시트에 남음 ④ goose
  `PackAlarmRadius` 값(표 행만 "미결"로 신설) ⑤ game-feel §4 줍기 팔 클립·퀵슬롯 펄스와 §5 몬스터 HP 핍은 미제작인데 뺀다는 기록이 없음 ⑥ 09-13 리뷰
  A-4·B-5·B-8 미반영 ⑦ 캠프파이어·밤하늘 제작(09-18~20)은 이 로그에 항목이 없고 module.md·light.md·site.md 에만 있다(제작 사실 기록은 `map/campfire.md`).
  09-20 실행분(Pounce·처형·데스캠) C++·표·문서는 아직 전부 미커밋이다.
- **2026-09-21 — 성능: 재고 나서 고친다. 첫 기준선·측정 도구·규칙 문서(`docs/systems/performance.md`), 렌더 기본값은 그대로.** 사용자
  "작업할 때마다 PIE 가 느려진다 — 최적화해야 하나" → 먼저 잼(`Tools/Unreal/perf_probe.py` 신설: PIE 안 슬레이트 틱 콜백으로 CVar 조합마다
  5초 표본 + `ProfileGPU`, `perf_parse.py` 가 로그 표를 읽는다). 이 머신(3070 Ti·5600·16GB) 에디터 PIE 1클라 1466×504: 밤 실내 13.0ms(77fps),
  GPU 4.8+2.5ms, 화면 200% 에서도 13.5ms — **CPU 바닥 ≈12ms 에 붙어 있고 GPU 는 여유**. 루멘 끔 −1.0 · VSM 끔 0 · 루멘 SW −0.9 · 안개 끔 0 ·
  화면 50% 0. 그래서 **렌더 기본값(루멘 HWRT·VSM·RT·Substrate·Nanite·TSR)은 바꾸지 않는다** — 절감 ≤1ms 이고 루멘 `SkyLeak` 은 밤 실내
  앰비언트의 전부(light.md). 반려: 루멘/VSM 끄기·`sg` 낮추기(지금은 효과 없음, 출시 최소 사양은 Steam 게이트 §7 에서). **느림의 실제 원인**:
  에디터 창이 최소화·백그라운드면 엔진이 정확히 333ms/프레임(3fps)으로 스로틀 — `bThrottleCPUWhenNotForeground` 기본 true. 첫 측정이 모든
  조합 333.3ms 로 재현했고, 메모·문서의 "이 머신 PIE 3~8fps"(inventory.md 스윕 근거·`slomo 0.15` 자동화)는 전부 이것이었다.
  `Config/DefaultEditorSettings.ini`(신설 — 클래스가 `config=EditorSettings` 라 PerProjectUserSettings 쪽은 안 읽힌다, 엔진 헤더 확인) 에 False 로
  박음(최소화 창은 여전히 스로틀 — 창을 복원하고 잰다). 예산: 개발 머신 PIE 밤 실내
  60fps 유지·p95 ≤25ms·기능 하나 +1ms 이상이면 이유 기록, 게임 스레드 CL9 코드 ≤4ms, GPU ≤10ms. 규칙(틱 이유·틱 안 전체 훑기 금지·시설
  빛 그림자 없음·이펙트 바운드/상한/컬·위젯 틱 훑기 금지)은 conventions §6 체크리스트 한 줄로 걸었고 pie-scenarios U 절 추가. 지켜볼 것:
  `IpNetDriver RecvFrom` 23ms 스파이크 1회, `ABP_Unarmed` 1.3ms. 명세 `docs/goal/performance-baseline-and-guidelines.md`.
- **2026-09-21 (계속) — 성능 결론의 범위 정정.** 위 "최적화 대상이 아니다" 는 **정지 장면**(몬스터 잠든 채 거리 둠) 한정. 사용자 보고 "몬스터에게
  다가갈 때 프레임이 팍 떨어진다" 는 미측정 — 파동(`MonsterProx`·`monster_call` 540광선·나이아가라 CPU Collision)·손전등 VSM 페이지 무효화·HWRT
  스켈레탈 BLAS·첫 발동 히치·상태 전이 표현 순으로 끄며 잰다(performance.md "미결"). 밤샘 측정 예정.
- **2026-09-21 (계속) — "몬스터 나올 때 렉" 의 원인 = 첫 사용 자리의 소프트 참조 동기 로드. 고침: `UCL9GameInstance::Init` 이 표의 소프트 참조를 전부
  미리 올리고 `WarmAssets` 로 붙든다.** 밤샘 실측(`perf_scenario.py` S1~S5 + `stat dumphitches`): 밤 시작·종별 첫 스폰(35~41ms)·추격 5/10/20마리
  (+0.7/+1.5/+3ms)·HWRT/VSM/스켈레탈 RT/손전등 토글(전부 <1ms) 은 원인이 아니고, **잡히는 첫 순간 694ms 히치** = 캐릭터 틱 → 셰이크 BP 클래스
  `LoadSynchronous`(577ms, 에디터 컴파일 포함) + `AddCameraShake` 30ms, 첫 파동 링 ≈500ms(나이아가라 첫 로드, 프로세스당 1회), 첫 타격 400ms 급.
  사운드 행만 09-20 에 미리 올리던 것을 셰이크 클래스·파동 나이아가라·rules 의 `HitFlashMaterial`·`HitPuffSystem`·몬스터 `SkeletalMesh`·`AnimBP`·
  캐릭터 메시 4종으로 넓히고, 로드한 것을 `UPROPERTY TArray<TObjectPtr<UObject>> WarmAssets` 가 붙든다(붙들지 않으면 60초 GC 에 풀려 다시 콜드).
  사용처의 `LoadSynchronous` 는 안전망으로 남긴다. 규칙은 performance.md "첫 사용 동기 로드 금지" — 새 표·새 소프트 참조 열은 같은 자리에 한 줄.
  1차 빌드 검증(S6): 잡히는 첫 순간 최대 16ms·첫 타격 19ms 로 사라짐. 남은 첫 링 416ms 는 에셋이 아니라 **나이아가라 첫 활성화의 에디터 스크립트 컴파일**
  (`Niagara - System - CompileScript`, 프로세스당 1회)이라 `UCL9WaveRenderer::OnWorldBeginPlay` 가 판 시작 때 행마다 한 번 켰다 끄게 했고, 종별 첫 스폰 37ms 를
  위해 몬스터·아이템의 `ActorClass`·아이템 `Mesh`·`Icon` 도 Warm 목록에 넣었다(2차 빌드). 2차 검증(S7, 새 에디터·새 PIE): 첫 링 31 · 첫 스폰 25 · 첫 타격 17 ·
  잡힘 17 · 죽음 드롭 16 · 거위/돼지 첫 스폰 43/17ms — 스톨 없음. 도구 `Tools/Unreal/perf_scenario.py`·`perf_scenario_summary.py` 를 저장소에 들였다.
  반려: 렌더 기본값 변경(측정상 무관), 메시 감량(닭 8k·너구리 11k 정점, 1 LOD — 마리당 0.15ms 라 대상 아님).
- **2026-09-21 새벽 — 공격 = 처형 클립, 제작 목록에 "공격" 행(실행 execute-clips, 사용자: "몬스터 만들 때 공격 모션도 목록에, 없는 애들은 다 만들자 — 처형과 공격은 같은 모션").**
  종당 주 공격 하나: 보통 잡는 종은 Catch(0.8초), 처형 종(`IsExecutioner()` = 돼지·너구리)은 Execute(길이 = `ExecuteDuration`)가 곧 공격 모션. 너구리의 격분 뒤 보통 잡기 `AN_Raccoon_Catch` 는 남긴다.
  2인 클립 4개 — `AN_Pig_Execute`(정면 박치기, 접촉 0.5, 앞 추력 +15cm 상한: 잡힌 거리 0~100 가변)·`AN_Raccoon_Execute`(등 뒤 −120 도약 +42/+38·피치 −0.45, 접촉 0.6)·
  `AN_Farmer_Executed_Pig`(뒤로 넘어져 1.40 DOWN)·`AN_Farmer_Executed_Raccoon`(앞으로 엎드려 1.25 **PRONE** 신설), 전부 2.0초 = `ExecuteDuration` 1.2 → **2.0**(초안, balance —
  블렌드 0.3 + 접촉 0.5~0.6 + 넘어짐 0.6 + 정지 0.6). 각 blend 에서 따로 저작하고 액터 공간 앵커 표 `Tools/Blender/execute_anchor.py` 하나로 정렬(길이 변경 = `DURATION` 한 곳 → 재굽기).
  끝 자세: 몬스터 = Idle 첫 프레임(시체 옆에 선 채 `SpectateDelay` 까지), 농부 = DOWN/PRONE 을 부활까지(`ABP_Unarmed` Executed 가지 조건 `IsExecuted` 단독, 종별 클립은 `ExecutedByPig` ←
  `GetExecutedByAlias() == "pig"`). 돼지 HP 6/8 유지 — "처형 중 못 죽인다" 근거는 2.0초에서 깨져 문구만 D4 근거로 정정.
- **2026-09-21 새벽 — 처형 진입 때 피해자 요를 서버가 처형자에 맞춘다(사용자 결정, gameplay).** 잡히면 처형(`bExecuteOnCatch`, 돼지) = 처형자 정면, 앵커 텔레포트 처형(스토커, 너구리) = 처형자 등짐 —
  갈림은 표 열이지 종 이름이 아니다. 적용은 서버 `SetActorRotation` + `SetControlRotation` + `ClientSetRotation`(자율 프록시는 CMC `ServerMove_HandleMoveData` 가 매 이동마다 컨트롤 요로 캡슐을
  되돌리므로 컨트롤 회전 자체를 양쪽에서 바꾼다). 캡슐 위치·콜리전 불변(D1). 처형 중 요 잠금은 기존 `LookInput` 의 `IsExecuted()` 게이트로 충분. 게터 `GetExecutedByAlias()`(행 `Alias`, 죽은 뒤
  부활까지 유지). PIE: 등을 보이고 잡혀도 진입 순간 돼지를 정면으로(요 180 → −31.3 = 돼지 방향), 너구리는 등 그대로, 죽음 진입 + 2.0초, Execute 5.0초 뒤 Break.
- **2026-09-21 새벽 — 몬스터 반응 클립은 Interchange 로만 임포트한다(리드, PIE 측정으로 발견).** `Tools/Unreal/reimport_creature_clips.py` 가 Interchange 플래그를 끄고 레거시 FBX 경로로 넣은
  클립(2026-09-15 Catch·HitReact·PushReact 5종, 09-20 닭 PounceWindup, 이번 Execute 2개)은 런타임에서 **요가 −90° 돌아** 있었다(로코모션은 머리 +X, 반응 클립은 −Y — 사용자가 본 "돼지 애니메이션이
  이상하다"의 한 원인). Blender·FBX 는 정상, 원본 로코모션·메시가 Interchange 로 들어와 있어 레거시 경로와 축이 어긋난 것. 17개 클립을 임시 경로 Interchange 임포트 → `consolidate_assets`(ABP
  참조 교체) → 리디렉터 삭제 → rename 으로 다시 넣었다(`.claude/runs/execute-clips/lead_reimport_interchange.py`; art 가 도구 스크립트에 반영). 검증: 닭 PounceWindup·돼지·너구리 Execute 모두 +X,
  스크린샷 `pie_execute_pig_victim_0.8s.png`(돼지 정면·농부 HIT)·`pie_execute_raccoon_victim_0.6s.png`(너구리가 등에 올라탐)·`*_deathcam.png`.
  art 가 `Tools/Unreal/reimport_creature_clips.py` 를 같은 방식(Interchange + consolidate + rename, `how = consolidated`)으로 다시 썼고 리드가 양 HitReact 한 번으로 검증했다. 농부 `import_farmer_clips.py` 는 레거시 경로지만 032·034 PIE 에서 방향이 맞아 그대로.
- **2026-09-22 — 전투 조작감 1차(실행 `combat-feel-pass-1`, 명세 `docs/goal/combat-feel-pass-1.md`, 패널 3+교차 심문 1 로 확정). 사용자 결정: 밀치기 전용 동작은 만들지 않는다 —
  `PushKnockback 1560cm` 은 리썰 이식값이 아니라 구 CCL8 상수(`PushKnockbackU 520`)였고 리썰 삽에는 밀어내기가 없다. 팬 = `Hit` + 작은 밀림.** 아래 8항목은 gameplay 보고서, 그 다음 balance·art·리드.
- **2026-09-22 — 휘두르기 판정 원점을 손 소켓 메시에서 컨트롤 회전 + 손 높이 원점으로 바꿨다(gameplay).** 2026-09-15 의 "히트박스 = 손에 든 메시", "사거리 숫자가 아니라 애니메이션이 정한다" 는 번복 —
  보이는 팬(카메라 자식 1인칭 팔)과 판정 구(전신 손 + 메시 경계)가 다른 클립·다른 공간을 써서 일치하지 않았다. 판정은 `[SwingHitStart, SwingHitEnd)` 동안 매 틱 `GetActorLocation() + Z·(EyeHeight − CapsuleHalfHeight − 66)`
  (손 높이 ≈ 90cm) 에서 `GetControlRotation()` 방향으로 `SwingHitRadius` 앞 → `SwingHitReach` 까지 구 스윕. **카메라 컴포넌트 트랜스폼은 읽지 않는다**(리슨 호스트에서 클라 폰의 카메라는 피치 0 —
  `bUsePawnControlRotation` 은 로컬 `GetCameraView` 에서만 적용). 눈높이 원점이면 밀착한 닭(중심 36cm)을 못 맞힌다. WorldDynamic Overlap 응답 근거(나란한 둘 다 맞기)·구간 겹침 판정·몬스터당 1회는 그대로.
- **2026-09-22 — `SwingHitScale`(2026-09-20) 폐기 → `SwingHitReach 100`·`SwingHitRadius 60`(balance).** 메시 경계 구가 사라져 배율할 것이 없다. 2.2 가 지키던 두 관찰(C4 ⑦ 2m 밖 불명중 182cm, ⑬ 옆 한 걸음 89cm)을
  새 열이 196cm(100+60+닭 36) / ≈79cm(√(96²−54²)) 로 잇는다. 명세 초안 150/80 은 266cm 로 ⑦ 을 깼다. 옛 경로 폴백·`SwingHitMode` 는 두지 않는다 — 롤백은 git. `GetSwingHitSphere`·`SwingHitPrevCenter` 삭제.
  `AlwaysTickPoseAndRefreshBones`(권한 전신 메시)는 남는다 — 판정용이 아니라 놓기(`Server_Drop` 의 "든 자리")용.
- **2026-09-22 — `SwingRecovery` 상수(0.6) → `CL9Rules` 열(헤더 0.6 = 옛 동작, 셀 0.347 = `SwingClipWindow`).** 초안의 "0.467 = 클립 길이" 는 틀렸다 — ABP 는 [0, 0.347) 만 `ExplicitTime + 0.12` 로 평가하므로
  팔은 0.347 에 Hold 로 돌아온다. 첫 명중 틱에 `SwingReadyTime += HitStopTime` — 맞든 빗나가든 팔이 돌아온 뒤 같은 여유. `Validate`: `>= SwingClipWindow` ∧ `> SwingHitEnd`. 함께 `SwingBufferTime` 한 칸 예약
  (헤더 0 = 끔, 셀 0.15) — 회복 끝 0.15초 안의 뗌 하나만 기억, 취소 분기 없이 `HostCancelSwingWindup()` 이 예약도 지운다(칸 전환·놓기·던지기·먹기·넘어짐·사망이 이미 그것을 부른다).
  실측(호스트, 연타 30회·100ms): 옛값 swing 5 / lost 25 · 회복만 8 / 22 · 회복+예약 1+buffered 12 / 17. 팔 복귀 − 다음 입력 수락 0.003~0.015초(≤ 1프레임).
- **2026-09-22 — 표면음은 `SwingSurfaceHitServerTime` 전이로 낸다(gameplay·art).** 원점 → Reach **선** 트레이스(구가 아니다 — 손 높이에서 20° 만 내려도 구는 30cm 앞 바닥에 먼저 닿아 닭을 버린다)가 지형에
  막히고 그보다 가까운 명중이 없으면 휘두르기당 1회. `SwingHitServerTime` 을 재사용하지 않는 이유: OwnerOnly 라 남의 폰에서 소리가 비대칭이고 팔 히트스톱을 건다. 조건 없는 복제, 폰 자리 재생(충돌점 비복제),
  킥·히트스톱·소음 사건 없음 — 킥은 "살아 있는 몬스터를 처음 맞혔다" 신호로만. 소리 `hit_surface.wav`(0.20s, −6dBFS — 명중 −3 보다 작게, 금속 링은 명중의 1/5) → `S_HitSurface` → SoundTable `HitSurface`.
- **2026-09-22 — 팬 = `Hit` + 아이템 열 `HitKnockback`(팬 120cm, 고기 0) + 규칙 열 `HitKnockbackTime`(0.12초). 2026-09-15 balance "팬 = Push" 근거 갱신(balance).** 두 대에 죽는 것(`HitDamage` 2)은 유지, 15.6m 핑퐁만
  폐기. 넉백은 Stun 진입 뒤 `HitStopTime` 을 기다렸다가(멈춘 프레임은 밀리지 않는다) `HitKnockbackTime` 동안 등속(경직 시간과 분리 — 0.08 + 0.12 = 0.20 ≤ `HitStunMs` 0.3), 스텝마다 `TraceWorld` 로 벽 검사,
  죽이는 타격엔 없음(죽음 발사가 대신). 120 의 근거: 회복 0.347초 동안 걷기 396 → 137cm > 120 이라 밀린 만큼은 회복 중에 따라잡는다. 종별 열(5셀 동일)·속도=거리/경직시간(히트스톱 중 미끄러짐)·0cm·감속 곡선은 반려.
  **`Push` 기제(`ECL9HitStun::Push`·`OnShove`·`bPushStun`·`PushStun`/`PushKnockback` 열·`PushReact` 클립)는 남긴다** — 사용자 결정. 쓰는 행은 없고 컴파일·동작을 유지한다(삭제는 별건).
  실측: 넉백 dist 120.0 · steps 8 · hitstop 0.080 · time 0.120; 벽 옆(벽까지 24cm) `blocked=1` dist 0; 죽는 타격에 넉백 줄 없음.
- **2026-09-22 — `ACL9Monster::TraceWorld` 공용 정적(gameplay).** 죽음 발사의 람다를 올려 죽음 발사·휘두르기 가림·표면음·넉백 스텝이 같은 "세상 지형만 보는 Visibility 트레이스"(폰·아이템·몬스터 무시 후 재시도)를
  쓴다. 가림은 스윕에 걸린 몬스터마다 원점 → 몬스터 구 중심(`ImpactPoint` 는 시작 관통 때 길이 0). 동료 폰은 가리지 않는다(사람은 맞지도 가리지도 않는다). 열 없음, 항상 켜짐. 실측: BlockAll 벽 너머 닭 5회 0타(`occluded = tested`).
- **2026-09-22 — performance 규칙: 휘두르기당 트레이스 = 스윕 1 + 걸린 몬스터 수(가림) + 표면 선 1; 넉백 = 스텝당 선 트레이스 1.** 새 소프트 참조 열 없음(Warm 목록 변경 없음).
- **2026-09-22 — 몬스터 ABP 5종의 빈 평가기 복구(리드).** a2c4834 커밋 상태에서 ABP_{Chicken,Goose,Raccoon,Pig,Sheep}_A 마다 시퀀스가 빠진 평가기가 하나씩 있었다(닭 PounceWindup·거위 PushReact·너구리/돼지 Execute·
  양 HitReact — 09-21 consolidate 재임포트 뒤 ABP 를 다시 저장하지 않아 참조가 유실된 것). 이 상태로 StartPIE 를 두 번째 부르면 "블루프린트 에셋 컴파일 5오류" 모달이 게임 스레드를 잠근다. MCP BlueprintTools 로 같은 자리에
  평가기를 다시 만들어 ExplicitTime·Pose 연결을 복원, 컴파일·저장(`.claude/runs/combat-feel-pass-1/abp_repair.py`). 재임포트 뒤에는 **ABP 도 함께 저장**한다.
- **2026-09-22 — 리드 PIE 측정 도구 `Tools/Unreal/pie_combat_feel.py`.** Python 원격 실행으로 PIE 안에서 팬 줍기 → 연타 → 닭을 자리에 놓고(틱 끔) 휘두르기 → 결과 json + 로그 표식. 벽은 PIE 전에 에디터 레벨에 놓는
  BlockAll 큐브 고정물(저장 안 함). `GameplayStatics` 에는 스폰 함수가 파이썬으로 노출되지 않는다. 결과: 밀착 10/10 · 2m 밖 0/3 · 옆 70cm 3/3 · 옆 110cm 0/3 · 명중 시각 start+0.050~0.067 · 벽만 `surface=t`, 허공 `surface=-`.

## 2026-09-28

- **`pen`(우리) 울타리는 판자 울타리와 별도 계열로 간다(art, `/cl9 task pen-fence-model`).** `henhouse_fence`(288cm 불투명 판자, 시야 차단, 미로 복도용 `wall` seg)는 그대로 두고, 우리용은
  `SM_Pen_Fence_250_A`(나무 기둥+철망, 250×20×130cm, 800 tris, `docs/asset/map/pen_fence.md`)를 새로 만들었다. 높이 130cm 는 플레이어 캡슐 반높이 90cm 위·눈높이 156cm 아래로 잡아 **몸통은 막고 시야는 연다** —
  우리 안 동물이 보여야 몰래 접근하는 긴장이 생긴다는 판단(사용자 2026-09-27). 충돌은 시각적 철망 틈과 무관하게 닫힌 UCX 상자 1개로 130cm까지 막아 아이템이 새지 않게 한다.
  `docs/asset/design-concept.md` 규칙 2(채도 상향 — 1.png 복셀 레퍼런스 참고)를 적용한 첫 맵 파츠다. UE 임포트 완료: `/Game/Maps/Henhouse/PenFence/SM_Pen_Fence_250_A`(치수 오차 <0.0001cm, UCX 1개, LOD 3단).
  `pen` 전용 SegKindTable 행(`blocks_sight=false`)과 Lvl_Flat 실배치는 다음 실행 `pen-fence-wire`(map) 몫.
- **우리 울타리의 시야 투과는 코드 변경 없이 표 한 행으로 된다(map, `pen-fence-wire`).** `ACL9SiteMap::Rebuild`의 `ComponentFor`가 인스턴스 묶음을
  `(seg kind, 메시)` 쌍마다 만들고 그 종류의 `bBlocksSight`로 채널 응답을 정하므로, `SegKindTable`에 `pen` 행(`Piece=penFence`·`Tile`·`Height=130`·
  `bBlocksSight=false`)을 넣으면 판자 울타리(`wall`, 288cm 차단)와 철망 울타리(`pen`, 130cm 투과)가 같은 코드에서 갈린다. 콜리전은 둘 다 `BlockAll`이라
  130cm까지 몸·아이템을 막는다. 조각 역할 `penFence`는 `fence`와 별개 행이다 — 한 역할에 섞으면 자리 좌표 해시가 288cm 판자와 130cm 철망을 섞어 뽑는다.
- **Lvl_Flat 전시 우리 확정: 중심선 500cm 정사각형·우리마다 8장·총 40장, 125cm 반 폭 변형 불필요.** 250cm 조각이 변 500에 두 장씩 1.00으로 맞는다.
  옛 큐브 20개(StaticMeshActor_8~27) 제거하고 `SM_Pen_Fence_250_A` 40장으로 교체 완료(리드, 2026-09-28). 좌표 정본은 `docs/systems/map.md`.
- **(미결, 사용자 결정 2026-09-28) 130cm 울타리는 점프로 넘을 수 있다(정점 156cm > 130cm) — 두 계열로 간다.** 130cm는 넘을 수 있되 넘으면 페널티
  (이동속도 감소 또는 HP 감소 — 구체적인 수치·방식 미정), 160cm는 완전히 못 넘는 차단용. 130cm 페널티 기제는 새 게임플레이 메커닉이라
  별도 착수 필요(경계 감지·페널티 적용·balance 수치 확정) — 이번 실행 범위 밖, 다음 세션 `/autogoal` 대상.
- **(미결, 사용자 승인 2026-09-28) `ModuleTable`에 `PerimeterKind` 열 신설 승인.** henhouse 생성 맵의 `pen` 모듈 둘레를 울타리로 바꾸려면
  "이 모듈의 어느 변이 울타리인가"를 정할 데이터가 필요하다(`CL9SiteLayout.cpp`의 `WallKind()`는 지금 양옆 모듈의 `bOutdoor`만 본다).
  제안: `UCL9ModuleDef`에 `PerimeterKind`(FName, 비우면 기존 동작) + seg kind `penStub`(문 옆 토막). 격자 밖과 닿은 변(`INDEX_NONE`)은 계속
  `barn`이어야 한다(안 그러면 아이템을 맵 밖으로 던질 수 있다). 구현은 다음 실행 `pen-fence-perimeter`(map) — 이번 실행 범위 밖.

## 2026-10-03

- **스테이지 = `MapTable` 행 하나 + 레벨 하나 (리드, `/autogoal` farm-stage — 사용자 부재, 자율 판단).** 사용자 "맵·아이템·몬스터·요리·트럭을
  하나로 합쳐 플레이하고 싶다, 양계장보다 5배 큰 맵을 레벨 하나에 밸런스 있게, 배치도 데이터 드리븐으로 — 스테이지 관리하는 무언가가 필요".
  새 시스템을 만들지 않고 이미 목적지 행인 `UCL9SiteDef`를 스테이지 행으로 쓴다: `Level` 열(soft `UWorld`)을 더해 게임 모드가 `StartPlay`에서
  지금 레벨을 가리키는 행을 고르고, 없으면 `DefaultSiteAlias`로 떨어진다. 반려: 별도 `UCL9StageDef` 표 — 목적지 행과 같은 것을 두 표로 나눠
  들게 된다; 레벨마다 게임 모드 BP 복제 — 데이터가 아니라 에셋이 늘어난다.
- **배치는 깊이 구역(`Zones`)으로 (리드).** 구역 = `{Name, DepthMin, DepthMax, MonsterSpawns, ItemSpawns}`, 깊이 = 지점 홉 ÷ 판의 최대 홉.
  구역이 비면 예전 규칙(몬스터 깊은 60%, 재료 전체) 그대로 — 양계장 불변. 반려: 거리(cm) 기준 띠 — 판 모양마다 뜻이 달라지고 층을 무시한다.
- **밤 시계 사이트 덮어쓰기 구현 — `NightLength`·`DarkBase`·`DarkMin`, 0 = `CL9Rules` (리드).** site.md 에 설계만 있던 것. 5배 판은 245초로 깊이
  갔다 올 수 없다. 대농장 480/360/240(초안).
- **`SpecialCopies` 열 (리드).** Special 행(납품 방·주방)을 판에 몇 번 놓나. 첫 헤드리스 실행에서 5배 판에 항아리가 1개뿐이었다. 대농장 4.
- **생성기: 격자 상한 64 → 128, 예산이 남았을 때만 도는 채움 단계 (리드).** 200모듈 판은 곁가지 단계가 비좁은 자리에서 48회 연속 헛돌아 일부 시드가
  70/200에서 끝났다(`CL9.Map.LargeSite` 실패). 채움 단계는 열린 변을 다시 훑어 통로 → 방 → 막다른 방 순으로 맞춘다. 예산을 채운 판(양계장)은
  들어오지 않는다. 테스트 `CL9.Map.LargeSite`·`CL9.Map.DepthBand` 추가, `CL9.Map.*` 6개 통과.
- **`Lvl_Farm`의 레벨 저장 내비메시를 지웠다 (리드).** 양계장 레벨을 복제하면 저장된 RecastNavMesh 가 옛 작은 경계 기준 타일 용량을 들고 와 런타임에
  farm 경계의 일부(약 200타일 덩어리)만 지어졌다 — PIE 실측 몬스터 11/26 이 내비 밖에서 영영 못 움직였다. 지우면 런타임(Dynamic)이 지금 경계로
  새로 만든다: 내비 전면, 25/25 내비 위, +20초에 빌드 끝. **같은 문제가 `Lvl_Henhouse`에도 있다**(경계가 x −1700~11000 · y −2100~2500 으로 판보다
  훨씬 작고, PIE 대조 실측 몬스터 5마리 중 1마리만 내비 위·0마리 이동) — 이번 범위 밖, 고칠지 사용자 확인 필요.
- **두 번째 스테이지 `pigfarm`(돼지 농장) + 스테이지 만드는 가이드 (리드, 사용자 부재 — "스테이지 하나 더 만들고 이걸 토대로 가이드").**
  대농장과 겹치지 않는 성격으로 골랐다: 단층 100모듈·우회로 적음(`LoopChance` 0.15)·긴 본줄기·어두움(`LampDensity` 0.2)·돼지 위주 고기 판.
  지금 있는 모듈 세트·몬스터·재료만 쓴다(새 아트 없음). `ValueRate`·`BaseDanger`는 1.0 — 경제를 건드리지 않는다. 반려: "작고 어려운 양계장 변형" —
  대농장과 마찬가지로 크기만 다른 판이 되어 가이드의 예시로 약하다. 첫 실측에서 1일차 돼지가 3마리뿐이라 구역 돼지 최소를 3으로 올렸다
  (1일차 몬스터 배율 0.6 의 반올림 — 가이드 함정 표에 올림).
- **스테이지는 명세 JSON 이 정본 (`Tools/Unreal/stages/<alias>.json` → `Tools/Unreal/setup_stage.py`).** 대농장 전용 스크립트를 일반화했다. 행은
  매번 명세대로 덮어쓰므로 에디터에서 행을 손으로 고친 값은 다음 apply 때 사라진다 — 값은 명세에서 고친다. 가이드 `docs/systems/stage.md`.
- **바깥 모습을 어두운 저녁 하늘 + 흙바닥으로, 모든 레벨·모든 스테이지 공통 (리드, 사용자 "바깥이 테스트존에 밝은 스카이박스, 어두운 걸로 —
  지금까지랑 앞으로 만들 모든 것에").** 하늘은 새 재질이 아니라 `M_CL9_NightSky`의 인스턴스 `MI_CL9_DuskSky`(같은 그래프, 색만 저녁)로 해서 밤
  하늘과 한 계열로 둔다. 레벨 하늘 구의 기본 재질과 사이트 행 로비 `SkyMaterial` 양쪽에 넣었다 — 하나만 바꾸면 에디터 화면이나 로비 중 한쪽이
  밝게 남는다. 로비 `SunScale` 0.2·`SkyLuminance` 0.3·구름 끔. 레벨 라이트의 원래 값은 안 바꿨다(밤 배율이 그 값에 맞춰져 있다). 새 스테이지는
  `setup_stage.py`가 `outdoor_look.py`를 불러 자동 적용 — 명세 열이 아니라 공통 규칙으로 둔 것은 "모든 것에"라는 요청 때문이다.

## 2026-10-04

- **서브에이전트 모델은 일의 성격으로 고른다 — 상시 규칙 (사용자 "1회용으로 하지 말고 앞으로 진행하게 남기자").** 판단·설계·첫 형태 = opus,
  정해진 것의 반복·기계적 적용 = sonnet, 아주 단순한 일 = haiku, 리드는 사용자가 고른 모델. 역할 정의 기본값(`cl9-roles.json` opus)은 그대로 두고
  띄울 때 `model`을 명시한다. 반려: 역할별 기본 모델 — 같은 역할 안에서도 판단과 반복이 갈린다. 정본 `docs/workflow/sessions.md` "모델 배분".
- **몬스터 나눠 스폰은 실제 프레임별 시도 상한과 FIFO를 쓴다(gameplay, `giant-districts-laptop` 실행 1).** 밤 시작에 정한 위치·회전을 복사해 다음 프레임부터 소비하며 실패와 스폰 직전 조용한 반경 진입도 시도로 센다.
  `MonsterSpawnPerFrame`은 정수 ≥ 1, 기본값 **4는 성능 미검증 초안**이다. 큐가 있는 동안만 GameMode가 틱하고 로비·새벽·재시작·월드 종료에서 대기와 큐를 취소한다. 실행 2에서 실제 구역 성능을 측정한 뒤 값을 확정한다.
- **분산 스폰의 기존 단일 사이트 개수·깊이·지점 분포 규칙은 유지하되, 같은 전역 RNG 결과까지 보장하지 않는다(gameplay).** 몬스터 초기화의 전역 랜덤 소비 시점과 아이템 선배치에 따른 충돌 보정은 일괄 스폰과 달라질 수 있다.
  기존 위치 추첨 자체가 복제 시드 전용 스트림이 아니므로 같은 시드에서 모든 몬스터 위치가 비트 단위로 같다고 판정하지 않는다. 생성기·레이아웃 시드 흐름은 그대로 둔다.
- **실행 1 노트북 성능 관찰은 회귀 게이트 통과로 확정하지 않는다(리드).** UE 5.8.3·RTX 5060 Laptop·1462×539 PIE, 양계장 실내 (2900,−1455,92), 몬스터 6개에서 `perf_probe.py` C0의 300프레임 avg/p95는 16.67/16.67ms다.
  이전 기준선은 다른 머신·뷰포트·개체 수여서 +1ms 회귀 비교를 하지 않는다. 시작·렌더 대기와 Slate 측정 준비/ProfileGPU 구간에 30ms 초과 히치가 남아 U 전체 통과는 보류한다. 원본은 `.claude/runs/giant-districts-laptop/perf-interior.json`·`perf-editor.log`에 보존한다.
- **정식 건물과 열린 마당을 자체 지붕 모델로 덮는다(사용자 "건물에 들어가면 하늘을 못 봐야 돼", "지붕모델 만들어서 덮자").** 기존 방 천장과 모듈의 `bOutdoor`·배치·길은 유지한다. 별도 `bBuildRoofs`가 켜진 사이트는 지상 점유 발자국 전체를 가장 높은 천장 위의 공통 지붕으로 덮고, 외곽 벽 위와 층간의 빈 띠를 금속 막음판으로 닫는다. 빈 격자와 트럭 바깥은 덮지 않으며 출입구 하부·계단·위층 이동은 유지한다.
  자체 Blender 지붕·막음판은 Piece/SegKind 표에서 참조하고, 지붕 활성 여부와 천장 위 여유는 Site 열에서 읽는다. Henhouse/Farm/PigFarm은 활성, Flat는 비활성이다. 이 수리는 `indoor-roof-closure`이며 `giant-districts-laptop` 실행 2와 커밋·푸시는 계속 보류한다.

## 2026-10-05

- **조작 손맛의 기준을 여섯 가지로 정하고 측정으로 합격시킨다 (사용자 "조작이 재미없다 — 리썰·R.E.P.O. 와 비교해 네가 괜찮다고 판단하는 기준을 만들어 거기까지", 밤샘 `/autogoal`).**
  기준: 즉시 반응 · 굼뜨지 않은 무게 · 카메라가 몸 · 관대한 점프 · 일상 동작이 소리를 낸다 · 밸런스 수치 불변. 합격표는 `docs/goal/control-feel.md` "Acceptance Criteria",
  측정은 `Tools/Unreal/qa_control_feel.py`(PIE, 60 Hz 고정, 3회 중앙값). 2026-10-05 빌드 #2 에서 73행 전부 통과(`.claude/runs/control-feel/gateA-2-table.md`).
  참고 게임 실측치는 설치본 디컴파일 + 프리팹 직렬화 값으로 뽑았다(코드 기본값이 아니다). 반려: 최고 속도·스태미나·점프 높이를 바꾸는 것, 웅크리기·슬라이드·운반 무게 감속(밸런스 미결 A4).
- **이동 곡선 (gameplay, control-feel C1~C7).** 가속은 `MaxAcceleration` 상수 2000 — `GetMaxAcceleration` 은 덮지 않는다(엔진이 입력 배율·합친 무브 재계산에서 다시 읽어, 속도 의존 가속은
  아날로그 입력 0.185 로 읽혀 제동이 걸린다). 제동은 엔진 쌍(제동 마찰 3.5, 감속 1300)이고 공중에서는 마찰을 0 으로 둔다. 질주 램프는 예측 상태 `SprintRamp`(0.85초 ease-out quad 로 차고
  0.25초 선형으로 내려감)가 속도 상한을 올리는 방식. 질주 중 조향 무게는 `GroundFriction = Lerp(8, 5, SprintRamp)`, 공중 제어 0.32. 값은 전부 `CharacterTable` 의 Movement 열(헤더 기본값 = 의도값).
  A1/A2 로드 검사에 램프 항을 넣었다(지친 지속 속도 503.7, 질주 예금 3851 — 뒤집히는 부등식 없음, balance 검산 "전부 유지"). 반려: 가속도 제한으로 만드는 램프(프레임레이트 의존, 모퉁이마다 다시 램프).
- **저장무브 합치기 롤백 (gameplay, C2).** `FSavedMove_CL9::CombineWith` 가 스태미나·탈진·램프·코요테를 대기 무브 시작 값으로 되돌린다. 이 롤백이 없으면 60fps 를 넘는 참여 클라이언트의 스태미나가
  호스트보다 1.5~2.5배 빨리 준다(코드 읽기로 확인, 미측정 — 이전부터 있던 결함). 시간 적분 예측 상태를 새로 더할 때는 이 함수에도 함께 더한다. 같은 실행에서 보정 리플레이 뒤 `bWantsToRun` 이
  마지막 리플레이 무브의 값으로 남던 것도 고쳤다(`ClientUpdatePositionAfterServerUpdate` 오버라이드).
- **카메라 손맛 구조 (gameplay, C8~C12).** 새 카메라 모디파이어 없이 `ACL9Character::UpdateViewFeel` 하나에서 세로는 카메라 컴포넌트 상대 Z(팔·든 물건이 같이 움직인다), 롤과 FOV 는 가산 오프셋으로 넣는다.
  시선 피치·요는 건드리지 않는다(조준·판정은 컨트롤 회전). 바탕 높이는 `CameraBaseZ` 멤버가 진실이고 넘어짐·죽음·처형·데스캠에서는 `FeelWeight` 가 0 이 된다. 값은 `CL9Rules.ViewFeel`.
  검토 뒤 옆걸음 롤 보간을 8 → 12 로 올렸다(90% 0.35초 띠의 경계에 걸려서). 반려: POV 만 옮기는 모디파이어(팔이 화면에서 미끄러진다), 보브 요·착지 피치 끄덕임·회전 틸트·숨쉬기.
- **발소리 = 걸음 위상 (gameplay, C10).** 거리 누적기를 정규화 위상으로 바꿨다 — 보폭은 속도로 보간하고 위상은 리셋하지 않는다. 보브가 같은 위상을 읽어 발소리와 보브가 한 시계로 움직인다.
- **설정 `화면 움직임` (gameplay·ui, C12).** `UCL9GameUserSettings::CameraMotion`(0~1, 기본 1) — 보브·롤·질주 FOV·착지 스프링·스태미나 셰이크에만 곱한다. 타격 킥·피격 셰이크·소리·HUD 튀기에는 곱하지 않는다.
- **일상 동작 소리는 소유자 전용(임시) (gameplay, C14 — conventions § 9 의 예외).** Jump·LandSoft·LandHard·Pickup·SlotSwitch·LightToggle·Eat·CartGrab·Exhausted 는 소유 머신에서만 난다.
  `PlayAt` 에 감쇠가 없어 남에게 들리게 하면 맵 전체에 들리기 때문이고, 동료에게 들리는 쪽은 감쇠 에셋과 2인 세션이 있을 때의 후속이다. 새 소리는 몬스터가 듣는 소음을 내지 않는다(기존 Land 파동·줍기 소음은 그대로).
  효과음 8개는 합성 임시음(`Tools/Unreal/synth_feel_sfx.py` → `import_feel_sfx.py`), 에셋은 `/Game/Sounds/SFX/{Character,Carry}/S_<행>`.
- **코요테 타임 0.12초·점프 버퍼 0.15초 (gameplay, C15).** 코요테는 `HandleWalkingOffLedge` 가 무장하는 CMC 예측 타이머를 `CanJumpWhileFalling` 이 읽는다. 버퍼는 소유 머신 Tick 에서 소비한다. 값 0 이면 꺼진다.
- **1인칭 팔 층 (gameplay, C16)** — 시선 뒤처짐·보브 일부 상쇄·느린 팔 스프링·칸 전환/줍기 딥(딥은 `화면 움직임` 을 곱하지 않는다). **HUD 튀기 (ui, C18)** — 줍기·칸 전환 때 퀵슬롯 1.15배 0.15초, 탈진 순간 스태미나 바 1.12배 0.2초, 번쩍임 없음.
- **측정 손잡이 (gameplay·art, C19).** `UCL9FeelDebug`(표본·소리 카운터·달리기 누름·CameraMotion 설정)와 `CL9.Feel.*` 자동화 테스트. 측정 창은 dt 중앙값 > 1/55 또는 한 dt > 1/30 이면 무효(실패가 아니다).
  참여 클라이언트가 있어야 볼 수 있는 행(롤백·예측·보정 수, 남에게 안 들리는지)은 미검증으로 남긴다 — 단일 호스트 PIE 로 통과라고 적지 않는다.
- **전멸이 판을 끝낸다 — 사용자 지시로 번복 (start-lobby L7, 사용자 "내가 죽었을 때 게임이 종료되는 것까지").** 2026-09 규칙 "전멸 → 즉시 EndNight → 다음 날 로비"를 뒤집는다.
  밤 중 전멸이면(혼자면 내 죽음) `SpectateDelay`(3초) 뒤 밤을 걷고 — 날짜·주문·장비·맵·부활·정산·저장은 손대지 않는다 — Phase 를 Dawn 에 멈춘 채 `GameState.RunOver{bActive, Day, BestDay}` 를 복제하고,
  게임 오버 화면에서 시작 레벨로 나가는 것이 유일한 출구다. 데이터 스위치 `CL9Rules.bWipeEndsRun` — **셀은 true 로 켰다(2026-10-05, 전체 루프 검증 뒤)**, 헤더 기본값은 아직 false 다(소유자가 루프를 본 뒤 true 로).
  셀을 false 로 돌리면 옛 규칙으로 돌아간다. 정산 미달 시의 제자리 새 판과 자동 저장은 그대로이고 판 종료는 세이브를 지우지 않는다. 반려: 제자리 재시작 버튼·세이브 삭제·새 페이즈 값.
- **시작 레벨 + 대기실 = 스테이지 레벨의 Lobby 페이즈 (start-lobby L1·L2·L4, 사용자 "start 레벨 하나 — 리썰처럼 대기실에서부터").** `Lvl_Start`(빈 레벨, 네이티브 `ACL9MenuGameMode`, 폰 없음)에서 타이틀 화면이 뜨고,
  [방 만들기] → 목적지(맵 표에서 `Level` 이 있는 사이트)를 고르면 그 스테이지 레벨이 열린다(`UCL9GameInstance::HostGame`). 그 레벨의 Lobby 페이즈(트럭 앞)가 대기실이다 — 리썰의 우주선과 같은 구조.
  나가기는 설정 `[/Script/CL9.CL9GameInstance] StartLevel` 로 `OpenLevel`. `GameDefaultMap` 도 `Lvl_Start` 로 바꿨다(`EditorStartupMap` 은 `Lvl_Flat` 그대로). 리슨과 주소 접속(`JoinGame`)은 **개발용 전송**이라
  Shipping 에서 컴파일되지 않는다 — session.md 의 "IP 직결 조인을 만들지 않는다"는 제품 규칙으로 유지되고, Steam 층이 같은 세 함수(Host/Join/Leave)의 속만 바꾼다. 반려: 별도 로비 레벨 + ServerTravel(오늘 PIE 로 검증 불가, 상태 이월 필요), 오늘 밤 Steam 연동(계정 하나로 검증 불가, steam-release.md 승인 대기).
- **출발 레버 (start-lobby L6).** 밤을 시작하는 길이 치트(`cl9.night`)뿐이던 것을 고쳤다. 레벨에 놓지 않고 게임 모드가 `StartPlay` 에서 트럭 패드 옆에 스폰한다(자리 `CL9Rules.DepartLeverOffset`, 메시 `DepartLeverMesh` — 지금은 출구 표지판 메시).
  두 번 누르기(`DepartArmTime` 5초), 호스트만, 주문 확정 뒤에만. 주문 제시가 남아 있으면 누름이 주문 화면을 다시 연다. 판정은 `ACL9GameMode::RequestDepart`. 반려: 길게 누르기(상호작용 경로가 누름 전용), 새 Blender 메시(이 노트북의 정적 메시 파이프라인이 낡음).
- **UI — 레벨 이동 뒤 재부트스트랩·루트 화면·스스로 짓는 위젯 (start-lobby L3·L10).** `UCL9UIManager::InitForPlayer` 는 메인 위젯이 없거나 다른 컨트롤러 것이거나 뷰포트 밖이면 낡았다고 보고 전부 떼어 새로 짓는다
  (위젯의 outer 가 GameInstance 라 레벨 이동 뒤에도 유효하게 남아 옛 검사로는 UI 가 죽었다). `OpenRootScreen` 으로 연 화면(Title·GameOver)은 M·[X]·CloseSelf 로 닫히지 않는다. Title·GameOver 는 C++ 이 트리를 짓고 WBP 는 빈 껍데기다.
  HUD 는 GameState 에 배선될 때까지 숨는다(시작 레벨에서는 끝내 안 보인다). 주문 화면은 제시마다 한 번만 저절로 열린다 — 2026-09-06 "매 틱 다시 연다"를 바꿨다(참가자가 호스트가 고를 때까지 커서에 묶이던 문제).
- **오늘 밤 만들지 않은 것 (start-lobby L9 — 2인 세션이 있어야 검증되는 것).** 접속 관문(PreLogin: 로비에서만·4인 상한), 로스터, 접속 실패·호스트 이탈 알림, 일시정지 메뉴의 나가기 버튼, 밤 중 이탈자 처리. 지금은 16인·밤 중 참가가 그대로 허용된다.

## 2026-10-06

판 시작·트럭·건물 전원 인트로 (명세 `docs/goal/round-start-truck-intro.md`, 실행 `round-truck-core`). 사용자 결정이 먼저, 그 밑은 패널·역할이 정한 구현 결정.

- **판은 스테이지에 들어오면 바로 시작한다 (사용자, K1).** 사용자: "그냥 시작하면 바로 몬스터 나와야 한다", "주문은 시스템이 랜덤으로 골라 주는 것 — 호스트가 고르게 둔 건 바꾸기 귀찮아서였다".
  `OfferOrders` 끝에서 제시의 앞 `Required` 개를 자동 확정하고 `StartPlay` 가 `StartNight` 를 부른다. 2026-10-05 의 "로비에서 호스트가 레버로 출발"을 첫 판에 한해 바꿨다.
  사용자는 플레이 구간을 "판"이라 부른다(코드 이름 Night 는 그대로). 반려: `bAutoStartFirstRound` 설정 열(아무도 요청 안 한 설정성).
- **출발 레버 = 트럭 안의 ON/OFF 스위치, 기본 ON (사용자, K2).** 판 중 레버를 올리면 "집으로" = 판 끝, 시간이 끝나면 레버가 저절로 올라간다. 귀가 뒤 정산 → 로비(요리·상점) → 호스트가 레버를 내리면 다음 판(임시 흐름 — 트럭 이동·트럭 안 정산·목적지 선택은 다음 명세).
  호스트 전용·두 번 누르기는 양방향 모두 유지(대신 정한 기본값). 손잡이 피치 −35° = 내림(ON), +35° = 올림, 페이즈에서 매 틱 파생. 판 중 참가자 프롬프트는 준비 여부와 무관하게 "호스트만".
- **새 트럭은 걸어 들어가는 실내가 있고, 판 끝에 트럭 안에 없는 사람은 남겨져 죽는다. 전원이 남겨지면 게임 오버 (사용자, K3·K5).** 사용자: "트럭 안으로 들어갈 수 있어야 한다, 4명 이상", "남겨진 사람은 죽는다", 경계 = "우리가 만들 트럭 안".
  `ACL9Truck`(런타임 스폰, 패드 기준 `TruckOffset`, 짐칸 상자 `IsInside`)과 Blender 새 메시 `SM_FoodTruck_B`(실내 640×340×250 이상, 외폭 424 — 바퀴를 실내 벽 바깥에 둔 결과). 옛 손 배치 트럭 액터는 네 스테이지 레벨에서 지웠다(에셋은 남김).
  남겨짐 처리는 `SetPhase(Dawn)` 뒤 — 짐(손·퀵슬롯·머리)을 파괴하고 `HostApplyDamage`. 전원 사망 + `bWipeEndsRun` 이면 박자 없이 바로 판 종료(`EndRunAtDawn`). 부활은 지금처럼 로비 진입 때(특정 장소 부활은 다음 명세).
  부활한 몸에 피격 흔들림·소리가 다시 나지 않게 `HostRevive` 가 넘어짐 시각을 되돌리고, 셰이크·Hurt 소리는 그 시각이 올라갈 때만 낸다.
- **건물 전원 인트로 (사용자, K6~K11).** 판 시작 때 건물 실내는 암흑(SkyLeak 0, 실내 램프 꺼짐)이고 첫 실내 방에 빛나는 스위치. 누구나 켜면 램프가 다 켜지고 플레이어마다 다른 지점(부엌·반납 항아리 방·비상구 — 모자라면 겹침)이 배정되며,
  카메라 연출(다음 실행 K10) 뒤 깜빡이다 실내 램프가 전부 꺼진다. 끝난 밝기 = 판의 기본 SkyLeak(처음 암흑보다 밝다). 연출 중 조작 잠금, 무적은 1초 더(`IntroGrace`). 정전은 모든 단계보다 우선.
  상태는 GameState 의 서버 시각 하나(`PowerOnServerTime`)와 배정 배열(`IntroTargets`) — Multicast 없이 매 틱 파생.
- **인트로 후보는 스위치 칸에서 칸 길로 닿는 모듈만 (리드, K9 보강).** 1층에서 계단실을 지나야 닿는 비상구 방은 카메라가 디딤판을 뚫어야 하므로 뺀다. 그 자리는 닿는 1층·비계단 모듈 중 가장 깊은 것이 받는다. 테스트 `CL9.Map.PowerSwitch` 가 "모든 후보에 닿는다"를 단언.
- **"계단 모듈"은 `bStairUp`(디딤판이 실제로 선 것)만 (map).** 위층을 못 틔워 되돌려진 계단실은 보통 방. 스위치 칸·칸 길·후보가 `CL9SiteLayout::IsStairModule` 하나를 쓴다.
- **인트로 깜빡임은 6 Hz 박자, 짝수 박자에만 켜진다 (map).** 큰 번쩍임이 1초에 3번을 넘지 않는다(광과민성 상한이라 코드 상수 `IntroFlickerBeatsPerSecond`). 램프는 실내·실외 무리 상태가 바뀔 때만 건드린다(performance.md 규칙).
- **스위치 칸의 지점은 모든 레이아웃에서 뺀다 (map).** 레이아웃은 자기가 주 사이트 맵인지 모르므로 구역 1 이후 판에서도 그 칸 하나가 빠진다. 해시는 그대로.
- **`ACL9Truck` 은 `bAlwaysRelevant`, 컴포넌트는 Movable (map).** 동적 스폰 액터는 관련 거리 밖에서 클라이언트에서 지워지고, BeginPlay 에서 메시를 입히려면 Static 은 거부된다.
- **레버 메시 규약 (art·gameplay).** 받침·손잡이는 같은 원점(경첩), +X = 사람 쪽, 손잡이 기본 수직. 레버는 트럭 본체 메시의 콘솔(UCX 포함) 위 — 레버 메시는 쿼리 전용이라 막는 일은 콘솔이 한다. StaticKit 에 `pivot='origin'` 추가.
- **시계 꼬리표가 다음 할 일을 알린다 (ui).** 로비 `Hud.Lobby {1}` — 호스트 " · 레버를 내려 출발", 참가자 " · 호스트가 레버로 출발". 판 `Hud.Night {3}` — 정전 " · 정전", Dark " · 전원 스위치를 켜라". 출발 레버를 못 찾아 판이 시작되지 않았다는 신고(2026-10-05) 때문.
  새벽 화면의 "트럭에 못 탄 사람" 줄은 새 위젯 없이 `ResultText` 끝에 덧붙인다(새 BindWidget 은 WBP 컴파일을 깬다). 이름 구분자는 키 `Dawn.LeftBehindSep`.
- **(확인 대기) 전원 스위치 불빛 값은 지금 코드 상수(6 cd·450 cm·차가운 색)다.** 표 열로 옮길지는 사용자가 PIE 에서 "문간에서 보이는가"를 본 뒤 정한다.
- **인트로 카메라는 로컬 `ACameraActor` + 경과 시각의 함수 (gameplay, K10).** `ACL9PlayerController::PlayerTick`(로컬 컨트롤러만)이 칸 길을 170 cm 높이로 따라간다. 한 번의 켬에 한 번만 시작하고, 판이 끝나거나·죽거나·처형되거나·서버 뷰 타깃 호출이 가져가면 즉시 중단(`Intro camera: aborted (사유)`).
  모양 값(높이 170·스윕 반지름 20·올림 50/150·앞보기 400·회전 180°/s·암전 0.15 s)은 명세 고정값이라 코드 상수, 시간 창·속도 상한만 Rules 열. 페이드는 `SetManualCameraFade` 로 매 틱 계산(늦게 받은 클라도 같은 밝기).
  +150 cm 까지 올려도 막힌 구간은 칸 높이로 난다(문 위 벽을 뚫지 않게, Warning). 바라볼 곳은 실제 항아리 자리(게임 모드 배치 순서와 같은 첫 Jar 지점), 없으면 날아온 방향 유지(잠정 — 사람 확인 A10).
  엔진 카메라 액터의 FOV 90·16:9 고정 비율 대신 1인칭 카메라의 FOV·후처리를 복사한다. 연출 중 자기 몸은 3인칭.
- **`power_intro` 는 시퀀서가 아니라 C++ (docs).** 길이 판마다 시드로 달라져 Level Sequence 에 담을 수 없다. 건너뛰기 없음. `pie-scenarios.md` 는 "판이 이미 돈다"를 머리 안내 한 곳에서 일괄 처리하고 새 절 W 에 A1~A10 확인법을 적었다.
- **(확인 대기) 자동 확정의 부수 효과: `cl9.day`·`cl9.load` 는 확정 묶음을 하나 더 쌓는다.** 치트·로드 경로라 받아들였다 — 사용자가 버그로 보면 고친다.

## 2026-10-09

부엌 알아보기 (명세 `docs/goal/kitchen-identity.md`, 실행 `kitchen-identity`). 사용자 결정이 먼저, 그 밑은 역할·리드가 정한 구현 결정.

- **부엌을 알아보게 하는 일이 요리 하나하나의 표현보다 먼저다 (사용자).** 사용자: "부엌에서 요리를 해야 되는데, 부엌인걸 인지를 못하겠고 요리되는 과정을 전혀 모르겠어",
  "달랑 식탁 같은 거 하나 있어서 저게 요리해야 되는 곳인지 그냥 흔한 책상 같은 건지 구분이 안 간다고. 프라이팬이라든지 지나가면 부엌이다라고 알려주는 뭐 그런 게 필요해".
  순서: "배치보다 일단 기능부터", "요리 하나하나보다 부엌 자체 시스템부터". 이번 범위 = 부엌 세트 꾸밈·항상 켜진 작은 불꽃(광원 없음)·후드 표지·가스불/환풍기 소리·"부엌" 장소 알림·놓기 안내.
  요리 판정·열 구역·게이지·복제는 그대로. 요리 진행 표현(불이 커짐·색 변화·김·지글·연기, 익은 계란 = 계란후라이)은 다음 명세 `cooking-progress-feedback` — 기본값은 명세 Out-of-scope 에 적어 둠.
- **가스불·환풍기 소리는 배경음 클래스 `SC_Ambience` 다 (사용자).** 설정의 "배경음" 슬라이더로 조절한다. 설계 게인 0.5 라 작게 들리면 합성 피크를 조정한다.
- **"놓기 프롬프트는 없다"(2026-09-13)에 예외 하나 (사용자가 고른 안내 문구).** 익힐 수 있는 재료(`CookResult`/`BurnResult`)를 들고 조리대를 바라볼 때만 "[놓기 키] 올려서 익히기".
  키는 줍기 키가 아니라 현재 놓기 키(`DropAction`, 리바인드 따름). 우선순위 착용 > 줍기 > 상호작용 > 조리대 놓기 > 수레 놓기. 들고만 있을 때 늘 뜨는 놓기 프롬프트는 여전히 없다.
- **꾸밈은 몸체와 같은 로컬 좌표계의 별도 메시 둘(`DressingMesh`·`PilotFlameMesh`)이다 (art·gameplay).** 열 구역과 게이지 자리는 몸체(`MeshComponent`) 바운드에서만 나오므로
  판정을 그대로 둔 채 부엌으로 읽히게 할 수 있다. 상판 면적 안 90~200 cm 는 비운다(후드 아랫면 201.2 cm). 걸린 팬·냄비는 들 수 있는 아이템과 다른 색(강철 + 붉은 손잡이).
  꾸밈 충돌은 UCX 5개(벽판 셋·옆 선반·냄비)만 막고, 카메라·WorldStatic 채널은 무시한다 — WorldStatic 무시는 평지 스폰의 지면 붙이기(`FindSpawnPoint`, 복잡 트레이스)가 꾸밈 전과 똑같게 하려는 것(리드 결정).
  불꽃은 광원도 그림자도 간접광(Lumen GI)도 없다(`bAffectDynamicIndirectLighting=false`) — 발광이 어둠 밸런스를 바꾸지 않게.
- **시설물 루프 소리는 액터의 오디오 컴포넌트가 `CL9Sound::Resolve` 로 받은 음원을 튼다 (gameplay).** `PlayAt` 은 그대로. 전용 서버 판정에 월드 넷 모드를 더했다(PIE 데디 서버 월드).
  감쇠 `SA_CL9_Kitchen`(2 m 안 전부, 12 m 에서 0)·동시 발성 `SCC_KitchenAmbience`(최대 1, Stop Farthest Then Oldest, 0.25 s) — 프로젝트 첫 감쇠·동시 발성 애셋.
  밀린 루프는 가상 루프로 남아 0.1 s 마다 되살아난다(UE 5.8 소스 확인 — `AudioVirtualLoop.cpp` 106~114 등). 가상화를 Disabled 로 두면 깨진다.
- **"부엌" 장소 알림은 HUD 의 기존 조리대 순회에서 판정한다 (ui).** 몸체 바닥 면적까지의 수평 거리 600 cm 안으로 들어설 때 한 번, 750 cm 밖에 나갔다 오면 다시. 처음 상태는 "밖".
  죽음·관전·인트로 잠금 중에는 판정하지 않는다. 값(`KitchenNoticeRadius`·`PlaceNoticeDuration`·`PlaceNoticeFadeTime`)은 WBP_HUD 표현 값이지 밸런스 셀이 아니다.
- **놓기 안내는 조준 오버랩을 재사용하고 몸체에서 가장 가까운 점으로 "바라봄"을 잰다 (gameplay).** 원점(상판 가운데)으로 재면 300 cm 상판 끝자리에서 실패한다. 조리대는 포커스(윤곽선)가 되지 않는다.
- **(정정) 스테이지 레벨에는 조리대·창구·트럭 패널·수레가 아직 없다.** `Lvl_Flat` 에만 있다. site.md·stage.md 의 "주방·카운터가 따라온다"는 틀렸다(정정 덧붙임). 스테이지 배치는 나중(사용자).

요리 진행 표현 (명세 `docs/goal/cooking-progress-feedback.md`, 실행 `cooking-progress-feedback`) — 같은 날 이어서.

- **익는 동안을 보이고 들리게 한다, 아지랑이 포함 (사용자).** 영상을 보고 "익을 때 주변에 열기가 나온다던가 그런 거 없어?" → "요리하는 거 좀 표현해줘".
  불꽃 키움·상판 달아오름·아지랑이·김→회색 연기·갈변/그을음·지글·탁탁·"치익"/"퍽"·하얀/검은 덩어리·탄 재료의 가는 연기. 판정·복제는 그대로, 모든 머신이 복제된 상태에서 파생.
- **익은 계란 = 계란후라이, 날계란 비례(지름 약 40 cm) (사용자).** 실제 크기(14 cm)는 익는 순간 1/3 로 줄어 보여서. 지금까지 `egg/cooked`·`egg/burnt` 메시는 엔진 기본 구(`/Engine/BasicShapes/Sphere`)였다.
- **납작한 재료는 메시 UCX 가 몸체 — `UCL9ItemDef::bMeshCollisionBody` (사용자 "이번에 같이", gameplay).** 루트는 보이지 않는 `BodyMesh`, 그림은 그대로 `Mesh`(수레처럼 `Mesh` 루트면 1인칭 손 옮기기가 깨진다).
  채널은 구와 같고 CCD 켬. 구 → 원반 교체는 밑면 맞춤(PIE: 익는 순간 z 111 → 90 = 상판, 정지), 클라도 같은 자리. 첫 적용 `egg/cooked`·`egg/burnt`, 질량은 엔진 계산(원반 2.15 kg — 초안, 날계란과 맞출지는 balance).
- **요리 효과는 나이아가라 없이 카드 메시 + 움직이는 재질, 값은 CPD (art·리드).** 역할 에이전트가 에디터 나이아가라 도구를 쓸 수 없어서. CPD 의 재질 기본값은 Idle(0). 아지랑이는 굴절(2D Offset)로 컴파일됐다.
  파이썬은 CPD 를 못 읽으므로 PIE 확인용 BlueprintPure 게터(`GetCookLook`·`GetFlameFlare`…, `GetCookTint`·`IsBurntSmokeOn`…)를 둔다.
- **탄 행은 행 데이터로 판정, 전이 소리는 "지켜보는 재료"의 행 비교, 오버레이 칸은 조준 윤곽선이 먼저 (gameplay).** 별칭 문자열 없음. 클라에 `Cooking=null` 과 행 교체가 따로 오는 순서 문제를 지켜보는 재료로 푼다.
- **요리 루프 동시 발성은 최대 2, 먼 것부터, 가상화 Restart (리드).** 계약이 명세 B7 문구보다 앞선다. 요리 소리는 `SC_Effects`(가스불 배경음과 달리 사건 소리).
- **탄 재료 연기는 수레 짐칸 안에서도 난다 (리드 — 사용자 원문 "while it is not held").** 계약에 "짐칸 아님"을 덧붙인 것은 리드였다 — 정정.
- **(발견, 미결) 조리대 앞에서 G 로 놓으면 계란이 상판 앞 바닥으로 떨어져 깨진다.** 손 자리(상판 앞 가장자리보다 약 20 cm 앞)에서 떨어뜨려 바닥 충돌 545 cm/s > `CrackSpeed` → `egg/cracked`(익지 않음).
  "[G] 올려서 익히기" 안내가 뜨는데 실제로는 못 올린다. 영상(`Saved/Video/kitchen_cooking_v2.mp4`)은 G 순간 스크립트로 상판에 옮겨 찍었다. 고칠 방법은 사용자에게 묻는다.

후속 — 조리대 놓기·계란 색 (실행 `stove-place-fix`, 명세 `cooking-progress-feedback.md` Follow-up F1~F3).

- **조리대 앞 G 는 손 자리 낙하가 아니라 상판 안착이다 (사용자 "상판 위에 살며시 놓기", gameplay).** 2026-09-15 "놓기는 그 자리에서 떨어지게"의 두 번째 예외(짐칸 다음), 4d 미결 후보 ② 채택.
  조건 = "[G] 올려서 익히기"와 같은 `ResolveFocus` 조리대 선택을 **호스트가 다시 잰다**(새 RPC 인자 없음). 자리 = 재료 수평 반경만큼 당긴 상판 직사각형에서 캡슐에 가장 가까운 빈 자리, 밑면이 상판에, 요만, 속도 0.
  겹침은 물리에 안 맡김(구 몸체 초기 겹침 해소 상한이 없어 튕기며 금 감) — 5 cm 격자 빈 자리, 없으면 보통 경로. 우선순위 조리대 > 짐칸 > 손 자리("[E] 줍기"가 떠 있어도 조리대를 보면 올림). 바닥 금 규칙(2026-09-16) 그대로.
  PIE: 앞 가운데·양 끝 × 40/70/110 cm 9 곳 전부 상판(앞 가장자리 21 cm 안쪽, z 111), 금 없음, 바로 익음. 두 번째 계란은 45 cm 옆. 2인 클라도 같음. 돌아서서 G 는 예전처럼 낙하.
- **계란후라이 재질은 정점색을 sRGB → 선형으로 푼다 (art, F2).** Blender 선형 정점색은 FBX 에 sRGB 로 나가고 UE 5.8 `VertexColor` 는 그 부호 그대로 — 숯 .035 가 .21 로 보이던 원인. `M_CL9_Egg_Fried` 만 고침(다른 정점색 재질은 범위 밖, 탄 고기도 밝게 보인다).
  탄 후라이 albedo .02~.08, 익은 후라이는 게임 화면 유지.
- **갈변 오버레이 익음 색은 색조만, 밤 하한 `CookedFloor` 0.15 (art, F3 — 초안).** 광원·GI 없음, 그을음·CPD 불변.
- **(발견, 미결) 조준 윤곽선(`M_CL9_Highlight`, 가산 Fresnel)이 어두운 물체를 금빛으로 덮는다.** 탄 후라이를 조준하면 금색으로 보인다(조준 안 하면 숯 검정). 범위 밖 — 사용자에게 알림.

## 2026-10-11

- **몬스터·배경 아트 제작 범위 선정(사용자: 어떤 몬스터·배경을 만들지 맡아서 MD로 정리).** [아트 제작 계획](asset/art-production-plan.md)을 작성했다. 핵심 재디자인은 닭→거위→돼지→너구리, 양은 기존 배치를 유지하고 스타일 정리를 후순위로 둔다. 배경은 양계장→돼지농장→대농장, 푸드트럭과 외부 농장 배경은 공유한다. 첫 검수 묶음은 닭+양계장 코너+플레이어 비교이며, 기존 원본·후보·모듈 파츠 재사용을 우선한다. 현재 AI 역할·스폰·경제를 새로 바꾸는 결정은 아니다. 외부 배경의 분위기·소재·제작 출처·배치 층·테스트 레벨 취급은 계획 E04에서 선택했다.
- **새 플레이어는 성인형 비율 고정을 해제한다(사용자: 성인 비율을 바꾸는 것이 목적).** 기존 캡슐·시점·리그는 연결 검수 기준으로 유지하되, 동글한 비인간형 후보의 전신과 1인칭 팔을 별도로 제작·검수한다. 외형 변경과 게임플레이 치수 변경은 구분한다. design-concept.md의 과거 비율 강제 문구를 갱신했다. 새 후보의 게임 연결 완료를 뜻하지 않는다.

- **몬스터는 특정 동물에 묶이지 않는다(사용자 정정).** 같은 날 위의 닭→거위→돼지→너구리 및 농장 3종을 필수 제작 목록으로 삼은 선정은 폐기한다. 공통 AI의 행동과 외형을 분리하여, [아트 제작 계획](asset/art-production-plan.md)을 포대꾼·종지기·냉장고지기·빈앞치마와 달걀 선별장·냉장 가공창고·심야 식재료 시장으로 개정했다. 첫 검수 묶음은 포대꾼+선별장 코너다. 외부 공통 배경은 개정 계획 E00을 따른다. 이는 신규 후보의 제작 기준이며, 기존 런타임 종·사이트의 삭제나 새 능력 구현을 뜻하지 않는다. 현행 세계관 문서의 동물 주인 전제는 후보 채택 시 함께 정리한다.

- **전원 인트로 카메라 — 순간이동 없이 이어진다(실행 intro-camera-continuous, 명세 [round-start-truck-intro.md](goal/round-start-truck-intro.md) "Follow-up K10-F").**
  사용자가 영상을 보고 "누른 상태에서 갑자기 위치가 바뀌는 게 아니라 거기서부터 돌고 쭉 이어져야 한다"고 지적했다. 원인은 시작 하드 컷(스위치 칸 중심에 카메라 생성)과 중간 암전 컷(길 8000~10500 cm > 1500 cm/s × 4 s) 두 가지였다. K10 의 이 두 부분을 바꿨다.
  - 사용자 결정(2026-10-10): 속도 상한 1500 cm/s 는 유지하고 `IntroFlyTime` 을 4 → 7 초로 늘린다. 누른 사람은 지금 1인칭 시점에서 출발한다. 다른 참가자는 lead-in(0.5 초, "스위치 켜는 연출") 안에 칸 길을 따라 스위치 칸까지 모인 뒤 같은 시각에 출발하고, 칸이 없으면(트럭 안·2층) 직선으로 간다. 10500 cm 를 넘는 길은 암전 컷이 남는다(수용).
  - 사용자 결정(2026-10-11): 시작 직후 자기 3인칭 몸(모자·어깨)이 화면을 가렸다 → 카메라가 1인칭 카메라 자리에서 `IntroCamBodyClearance`(120 cm, 코드 상수) 이상 떨어질 때까지 1인칭 모습을 유지하고 그 뒤 한 방향으로 3인칭 몸을 켠다. 몸 전환은 새 `IsIntroBodyShown` 이 정하고 `IsIntroCameraActive` 의 뜻은 그대로다. 인트로 카메라는 1인칭 FOV·축소 설정을 이어받는다.
  - 사용자 결정(2026-10-11): 짧은 길이 7 초에 걸쳐 너무 느리다(1500 cm 면 약 2 m/s) → `CL9Rules.IntroMinSpeed`(600 cm/s, 초안 — 선택지의 예시 값). L ≤ 최소속도 × 비행 시간이면 그 속도로 날아 일찍 도착해 목적지(항아리)를 보며 머문다. 시간 창(불·깜빡임·잠금·무적)은 바뀌지 않는다.
  - 구현에서 정한 것: 모이기의 시작 시각 = 카메라가 생긴 틱의 경과(늦게 받은 클라는 모이기를 남은 lead-in 에 압축). 모이기 길의 양 끝(S·P0)은 올리지 않고, 양 끝이 고정된 구간은 스윕하지 않는다. 모이기 동안 시선은 모이기 길에 이어 비행 길 위 400 cm 앞이다. P0 판정은 "넘은 시간 × 비행 속도" 로 본다(그 틱만 P0 에 세우지 않는다). 칸→칸 BFS `CL9SiteLayout::FindCellPathToCell` + 테스트 `CL9.Map.CellPathToCell`.
  - PIE 확인(2026-10-11): 시작 이음 0.0 cm·0.00°(세 판), 200 cm 넘는 걸음 없음, 몸 전환 123~198 cm, 최소 속도 `arrive +3.83 s`(2000 cm), 2인에서 다른 방 클라 `gather 6 cell(s)`·트럭 안 클라 `gather straight`, `cl9.power` 창 +7.50/+9.00/+9.40/+10.40, 판 끝내면 `aborted (phase)` 뒤 1인칭 복귀, `Lvl_Flat` 인트로 없음. 영상 `Saved/Video/power_intro_v3.mp4`.

- **이전 몬스터 시안은 후속 제작 기준에서 제외한다(사용자).** 사용자가 지금까지 그린 몬스터 시안을 폐기하고 첨부 컨셉아트 기준으로 글부터 다시 정하도록 요청했다. 기존 [아트 제작 계획](asset/art-production-plan.md)의 종 선정과 시작문을 철회 상태로 표시했다. 원본 이미지·모델·런타임 데이터를 삭제한 것은 아니다. 후속 몬스터 제안은 아직 채택된 제작 명세가 아니다.
- **캐릭터 브리프의 이미지 대조 기준 보완(사용자 전달 피드백 반영).** [형태 대조 기준](asset/character/farmer_a-visual-reference.md)에 전신 R1·R2, 1인칭 손·팔 R3의 용도·필수 형태·미확정 부분·실제 렌더 비교 절차를 명시했다. 현재 저장된 전신/팔 미리보기를 원본과 비교했으며 새 렌더·모델 수정·런타임 검증은 수행하지 않았다. 단일 달걀형/반죽형은 제작 이력이지 이미지에서 증명되는 해부 구조가 아니다. 기존 v3 채택·걷기 시험 이력은 유지하며, 그 기록을 컨셉 일치나 새 후보 채택으로 확대하지 않는다.

- **닭 탈 광폭화는 위로 벗겨지고 바닥에 남으며, 진정하면 회수해 다시 입는다(사용자).** 귀여운 겉모습에 실밥이 있고 안의 본체가 탈을 찢고 나온다는 컨셉을 채택했다. 변신 후 옷을 몸에 매단 v1 프리뷰에서, 위쪽으로 벌어지는 탈을 땅에 떨어뜨리는 v2로 수정 요청했다. 어그로/광폭화 감소 후 재착용 동작도 요청했다. [동작 프리뷰 카드](asset/monster/chicken_disguise-animatic.md)에 벗기·입기를 분리해 기록한다. 게이지 수치·AI 상태·회수 거리·재발각 처리는 미정이며 이번 로컬 영상 작업으로 구현됐다고 보지 않는다.

- **닭 탈 괴물 외형은 사용자가 다시 첨부한 컨셉아트로 확정한다(사용자 “이거로 가자”).** [선택 이미지](../ArtSource/monster/chicken_disguise/reference/approved_chicken_disguise_20261011.png)의 밝고 긴 단순 얼굴·마른 긴 팔다리를 기준으로 한다. 이후 시도했던 animatic_v3/v4의 이빨 얼굴은 채택하지 않는다. 그림의 허리에 걸린 옷은 벗는 중간 자세로 사용하며, 완료 시 바닥 낙하·진정 후 회수·재착용 요구는 유지한다. [새 세션 제작 지시서](asset/monster/chicken_disguise-animation-handoff.md)에 모델·리그 재제작, 접촉·체중 이동·천 변형·검수 기준을 작성했다. 약 30초 프리뷰 시간과 안쪽 여밈끈은 제작 시작 가정이며 게임 수치/설정 확정이 아니다. 이번 작업은 문서·기준 이미지 보존이며 새 애니메이션을 제작한 것은 아니다.

- **루프 개편 — 회사 정산·외부 재화·임포스터·팀킬·전멸 패널티·중간 스폰 (사용자 구술, 명세 [goal/loop-rework.md](goal/loop-rework.md)).**
  한 판은 "트럭 도착 → 주문서 알림 → 건물 침입(스위치·시퀀스) → 수집·건물 안 부엌 요리 → 트럭 출발"이고 **3판마다 회사**에서 정산·구매한다(`SettlementIntervalDays` 2 → 3, 에셋 값).
  재화는 두 층 — 판에 묶인 이더(기존)와 **플레이어 개인 소유 외부 재화**(치장만, 밸런스 무영향). 외부 재화의 존재 이유는 **임포스터**다: 판마다 무작위 0~1명, 자기 외 전원 사망 + 자기는 살아 출발해야 성공,
  직접 때려 죽이면 실패, 성공 보상은 외부 재화. 보너스이지 필수가 아니다. **팀킬**을 넣는다(몬스터 타격 도구가 플레이어도 맞힘 — swing-hitbox 의 "friendly fire 없음" 번복); 임포스터 처치자도 외부 재화, 오인 처치 패널티 없음.
  **전멸은 게임 오버가 아니다**(2026-10-05 `bWipeEndsRun` true 번복) — 그 판만 끝나고 누적 이더 n% 차감. **마당에는 아이템·몬스터가 없다**(site.md "입구 쪽 안전한 마당" 번복). 몬스터는 잠들었다 깨는 것 + **판 중간 스폰**(신규).
  밸런스 수치·외부 재화 획득처·스킨 상점 위치·회사 형태·중간 스폰 조건 등 미결 11개는 명세에. 구현은 하나도 시작하지 않았다 — 기획 문서(loop·economy·site·progression·rules)에 날짜 붙은 정정만 넣었고 systems 문서는 구현 때 고친다.

- **인게임 음성 채팅 — 기능·인터페이스만(실행 voice-chat, 명세 [voice-chat.md](goal/voice-chat.md), 정본 [systems/voice.md](systems/voice.md)).**
  엔진 내장 VOIP(Opus·`UVoiceChannel`·리슨 호스트 중계)를 Null OSS 개발 세션(`cl9.host`/`cl9.join lan`) 위에 올렸다. 외부 음성 서비스는 쓰지 않는다
  (참고: 리썰 컴퍼니 = Dissonance, R.E.P.O. = Photon Voice — 우리 구조는 리썰 쪽).
  - 사용자 결정(승인 뒤 변경, contract §13~§16): **핫라인 = 두 무전기 스피커의 양방향 연결**(통화모드 없음, 한 사람에 하나, 새 연결이 옛 연결을 끊음).
    말하는 사람 주변은 언제나 3D, 상대는 2D, 그 밖의 청자는 말하는 사람·상대 무전기 중 **더 크게 들리는 쪽** 한 자리. **무전기는 사람과 무관** — 상대가 죽어도 시체 자리에서 울린다.
    죽은 사람의 말은 산 사람에게 안 들린다(리썰 규칙, 무전기로도).
  - **감쇠·마이크 처리 값은 CL9Rules 셀**(감쇠 150/1500 Linear, 무전기 같음, 마이크 처리 = 엔진 기본값) — `SA_CL9_Voice` 에셋은 만들지 않는다.
    설정은 "마이크 감도"(클수록 작은 소리도 나감, dB 선형 0.20→0.01, 기본 0.31), 게이트 = min(셀, 감도 문턱). 옵션 기본값은 C++(다른 옵션과 같다).
  - **몬스터 청각은 이번 범위에서 뺐다(사용자)** — "몬스터 청각은 나중에 몬스터 쪽에서 음성 채팅을 사용할지 말지 결정하는 것". 이 실행이 더했던 청각·소음 링·거위 `HearVoiceRadius`·부등식 A8 은 되돌렸다.
  - 경로 API: `HostSetVoiceRoute(Mode, Target)` · `Server_SetVoiceRoute`(이번의 유일한 새 Server RPC, Validate 는 늘 true) · `cl9.voiceroute prox|direct <n>`. 새 Multicast 0.
  - 명세 E9 정정: `voice.SilenceDetectionReleaseTime` 1100 ms 는 40 dB 감쇠 시간 — 실제 송신 꼬리는 0.3~0.55초.
  - 빌드: 에디터 타깃은 `SignalProcessing` 모듈 의존이 필요했다(게임 타깃 컴파일 검사는 단일 링크라 못 잡음).
  - 확인: 로그 판정 A1·A2·A5·A6·A8(로컬)·A10·A11·A12 통과(A5 의 부활 해제는 같은 날 밤 `Lvl_Henhouse` 리슨 2인 PIE 로 추가 확인), 같은 PC LAN 비컨 실측 통과. 귀로 듣는 판정(T1·T2·A3·A4·A9)은 사용자 확인 대기.

## 2026-10-12

- **전멸은 판만 끝내고 누적 이더 30% 를 깎는다 — 루프 개편 7a 구현(사용자 2026-10-11 결정 + 같은 날 밤 "n%는 30%로", 명세 [goal/loop-7a-wipe-penalty.md](goal/loop-7a-wipe-penalty.md)).**
  `CL9Rules.bWipeEndsRun` 은 헤더·셀 모두 false(의도값) — 2026-10-05 의 게임 오버(true) 경로는 지우지 않고 스위치로 남겨 셀 하나로 되돌린다.
  새 셀 **`CL9Rules.WipePenaltyRate` = 0.30**(0~1, 범위 밖·NaN 은 로드 오류). 판의 누적 이더(`SharedMoney`)만 깎고 **기간 수입은 그대로** — 정산 판정에 영향이 없다.
  - 구현에서 정한 것: 깎는 자리는 `ACL9GameMode::EndNight` 의 남겨짐 판정 뒤 한 곳(`ApplyWipePenalty`) — 판 중 전멸과 판 끝에 전원이 남겨진 경우가 같이 지나가 **한 번**만 깎는다.
    정수 이더라 내림, 비율은 만분율로 반올림해 정수로 곱한다(float 0.7 이 0.6999… 라 10 이더의 70% 가 6 이 되는 것을 막는다). 잔액 하한 0(`Deduct`).
    새벽 화면에 "전멸 — 이더 N 차감" 한 줄(`Dawn.WipePenalty`, 깎은 것이 0 이어도 보인다) — 복제 `FCL9WipePenalty`, 다음 판 시작에 지운다.
- **정산은 사흘(3판)마다 — `CL9Progression.SettlementIntervalDays` 2 → 3, `QuotaCurve` 첫 키 2일차 → 3일차(800 그대로), 헤더 기본값도 3**(사용자 2026-10-11 "3판마다 회사에서 정산"). 코드 변경 없음.
  - 확인(2026-10-12): 빌드 1회 성공, PIE `Lvl_Henhouse` 1인 — 판 중 전멸 1000→700·잔액 0→0·트럭 밖 남겨짐 전멸 한 번만·정산 3·6일차만(첫 목표 800)·셀 true 면 옛 게임 오버, 리슨 2인 — 클라이언트도 로비에서 부활, 테스트 `CL9.Run.WipePenalty` 통과.
- **마당 무스폰 — 실외 모듈은 몬스터·재료 지점을 내지 않는다, outer 구역 수는 남은 건물 안 지점 밀도로 다시 정한다 — 루프 개편 7b 구현(사용자 2026-10-11 결정 6 + 2026-10-12 "outer 마릿수·개수를 다시 정함", 명세 [goal/loop-7b-yard-no-spawn.md](goal/loop-7b-yard-no-spawn.md)).**
  모듈 표 `yard_open.PointPurposes` 0xB → **0x8**(Patrol 만 — 마당 지점은 남아 깊이 눈금이 그대로), `yard_gate` 는 원래 0x0. 코드 변경 없음(규칙의 주인은 표).
  - outer 수: 마당을 뺀 실측 지점 비율(시드 101~106·5일차, 대농장 outer 1753→1627 = 0.928, 돼지 농장 846→801 = 0.947)로 곱해 반올림 — 대농장 닭 8~10 → 7~9·계란 14~18 → 13~17(양 3~4 그대로),
    돼지 농장 계란 8~10 → 8~9(나머지 그대로). `MapTable` 행과 무대 명세(`Tools/Unreal/stages/farm.json`·`pigfarm.json`)를 같이 고쳤다 — `setup_stage.py apply` 는 레벨까지 다시 쓰므로 돌리지 않았다.
  - 정하지 않은 것: middle·deep 도 지점이 3~5% 줄지만 수는 그대로(밸런스 때), 양계장(구역 없음)·`Lvl_Flat`(건물 없음)의 사이트 단위 스폰은 그대로. 지점이 아닌 마당 "긴장 요소"는 loop-rework 미결 8.
- **조리대는 건물 안 부엌 모듈에 하나씩 — 루프 개편 7c 구현(사용자 2026-10-11 결정 8 + 2026-10-12 "부엌마다 1개", 명세 [goal/loop-7c-kitchen-in-building.md](goal/loop-7c-kitchen-in-building.md)).**
  정찰: 스테이지 레벨(양계장·대농장·돼지 농장)에는 로비를 포함해 조리대가 하나도 없었고(시험 레벨 `Lvl_Flat` 만 손으로 3개), `kitchen` 모듈에는 간판·작업대뿐이었다 — "로비 조리대는 뺀다"는 뺄 것이 없었다.
  - 구현에서 정한 것: 소품 세트에 **`Fixtures`**(호스트가 판 시작에 세우는 복제 액터 — 클래스·회전 전 모듈 로컬 자리·요) 컬럼을 더했다. 소품은 머신마다 그리는 메시라 상태가 있는 시설을 못 담기 때문이다.
    세트는 소품과 **같은 함수**(`ChoosePropSet`, 자리 좌표 해시)로 고른다. 호스트가 재료·몬스터보다 먼저 세운다(그 둘은 겹치면 비켜 서는 스폰). `kitchen` 두 세트 모두 작업대 대신 조리대 —
    (250, 94) 요 90 남쪽 벽 / (750, 1406) 요 −90 북쪽 벽, 뒤판이 벽면에 붙는다(트레이스 74 cm = 뒤판 깊이).
  - 인트로 카메라: 목적지 모듈에 항아리가 없고 고정물이 있으면 그것(원점 위 130 cm)을 본다. 전에는 "길 끝 칸"을 봐서 부엌에 닿아도 날아온 방향을 그대로 봤다 — 실측 조리대가 정확히 옆(90°)이었다.
    배정은 그대로 무작위(부엌을 보는 사람은 판마다 한 명 이하).
  - 확인(2026-10-12): 빌드 2회 성공, PIE 양계장 1·대농장 4·돼지 농장 2개(부엌 모듈마다 하나), 로비 0개, 조리대 위 계란 12.0초에 익음, 부엌이 배정된 판에서 카메라가 5.84~9.19초 동안 조리대를 화면 가운데(0°)에 가림 없이 담음. `CL9.Map`·`CL9.Run` 14개 중 `CL9.Map.Districts` 만 실패 — 이 작업 전부터 있던 실패(거대 스테이지 구역 배치, giant-districts 보고서).
- **팀킬 — 휘두르기가 다른 플레이어도 맞힌다, 죽인 플레이어를 남긴다 — 루프 개편 7d 구현(사용자 2026-10-11 결정 4, 명세 [goal/loop-7d-team-kill.md](goal/loop-7d-team-kill.md)).**
  swing-hitbox 의 "friendly fire 없음"을 뒤집는다. 몬스터 스윕(WorldDynamic 겹침)은 그대로 두고 같은 구로 Pawn 채널 겹침 스윕을 하나 더 해 나 아닌 `ACL9Character` 를 고른다.
  - 구현에서 정한 것: 피해 = 든 아이템의 `HitDamage`(새 데이터 칸 없음 — 밸런스는 나중) 를 사람이 받는 단일 문 `OnDamage` 로. 넘어짐·인트로 무적, 죽음의 흘림, 피격 사건이 몬스터에게 맞을 때와 같다.
    한 휘두르기 한 사람 한 번·벽 가림·첫 명중 킥은 몬스터와 같은 규칙. 로그 동사 `struck by`. 죽는 타격이 플레이어면 `ACL9PlayerState::KilledByPlayer`(호스트 전용)에 남기고 부활에서 지운다(7g 가 읽는다).
    삽은 아이템 표에 없어(프라이팬·고기만 타격 도구) 프라이팬으로 확인했다.
  - 확인(2026-10-12): 빌드 성공, 리슨 2인 PIE — 벽 너머 동료 `tested=10 occluded=10` HP 그대로, 110 cm 앞 동료 3 → 1(넘어짐), 넘어진 동안 둘째 타 무시, 일어난 뒤 한 대에 사망·`was killed by player` 로그, 클라 세계에서도 자기 HP 0·사망, 닭 타격 그대로(`hit` 찍힘), 자기 자신은 안 맞음.
- **판 중간 스폰 — 시간 간격마다 한 마리, 모든 산 사람에게 안 보이고 먼 건물 안 자리에, 판 시작 수의 비율까지 — 루프 개편 7e 구현(사용자 2026-10-11 결정 7 + 2026-10-12 미결 7 답, 명세 [goal/loop-7e-mid-round-spawn.md](goal/loop-7e-mid-round-spawn.md)).**
  사용자 답: 조건 = 시간 간격(소음·정전 연동 아님), 위치 = 보이지 않고 멀리(깊이 무관), 수 = 그 판의 시작 스폰 표 비율로 한 마리씩·총량 = 시작 수 × 비율.
  새 셀 `CL9Rules.MidSpawnInterval` 60 s(0 = 끔)·`MidSpawnMinDistance` 1500 cm·`MidSpawnCapRatio` 0.5 — 전부 초안, 밸런스는 나중.
  - 구현에서 정한 것: "안 보인다" = 산 사람 각자의 눈에서 그 자리까지 지형이 막는다(바라보는 방향과 무관 — 돌아보면 보이는 곳도 안 된다). 후보는 생성된 맵의 `Monster` 지점(마당은 7b 로 없음)을
    무작위로 최대 64개만 본다. 맞는 자리가 없으면 그 차례는 건너뛴다. 평지 레벨(생성 맵 없음)은 끈다. 판이 끝나는 모든 길이 `CancelNightWork` 로 타이머를 지운다.
  - 확인(2026-10-12): 빌드 1회, PIE 양계장 1인(간격만 메모리에서 5초) — 두 판 모두 총량(시작 5·6 → 3)까지 5초마다 하나, 자리는 19~65 m·시야 가림(로그와 별도 트레이스 모두), 판 끝 로비에서 0,
    다음 판 재무장, 간격 0 이면 꺼짐. 시험 중 세션 밖에서 띄운 `Lvl_Flat` 에디터가 MCP 포트를 잡아 이 세션이 띄운 에디터를 사용자 확인 뒤 끄고 그쪽에서 시험, 끝나고 `Lvl_Flat`·PIE 설정을 되돌렸다.
- **바깥 재화 "이더" — 각자 기계의 지갑, 살아서 귀가하면 판 수입에 비례, 판 안 화폐는 화면에서 "코인" — 루프 개편 7f 구현(사용자 2026-10-11 결정 1 + 2026-10-12 답, 명세 [goal/loop-7f-out-game-currency.md](goal/loop-7f-out-game-currency.md)).**
  사용자 답: 획득 = 살아서 귀가할 때 **판 수입 비율에 따라**("그래야 더 열심히 한다"), 임포스터 처치·보상(7g)은 이보다 커야 한다 / 이름 = 바깥 재화가 **이더**, 판 안 화폐는 **코인**으로 바꾼다 /
  표시 = 새벽 화면 + 타이틀. loop-rework 미결 1 은 이것으로 닫는다(임포스터 쪽 액수는 7g).
  - 구현에서 정한 것: 저장은 **모든 머신에 하나씩** 있는 `CL9Wallet` 파일(`UCL9WalletSubsystem`) — 판 세이브·기록(호스트)과 달리 플레이어를 따라간다. PIE 인스턴스 1 이상은 `_PIE<n>`.
    판 끝(`EndNight`, 남겨짐·전멸 패널티 뒤)에 호스트가 **모두에게** `Client_WalletGrant` 한 번(죽었으면 0) — 판 수입 = 판 동안 늘어난 `PeriodIncome`, 보상 = floor(수입 × `CL9Rules.WalletIncomeRate` 0.01 초안).
    판 안 화폐의 ko 문구 11개만 "이더" → "코인"(en 열·코드 이름·로그 "ether"·이력 문서 본문은 그대로, economy.md 머리에 읽는 법 표기). 로컬 파일이라 고칠 수 있지만 치장 전용이라 받아들인다.
  - 확인(2026-10-12): 빌드 1회, PIE 1인 — 1000 코인 벌고 트럭 안 귀가 → +10 이더·새벽 `이더 +10 (보유 10)`, 남겨져 죽으면 0·`보유 이더 10`(전멸 차감은 지갑과 무관), PIE 재시작 뒤 10 그대로,
    타이틀 `보유 이더 10`, HUD `545 코인`. 리슨 2인 — 판 시계로 판이 끝나자 클라 `CL9Wallet_PIE1` 0 → 10·호스트 10 → 20, 양쪽 새벽 화면에 각자 숫자.
- **임포스터 — 판 시작에 0~1명 비밀 배정, 판 끝 판정·보상·새벽 공개 — 루프 개편 7g 구현(사용자 2026-10-11 결정 3·4 + 2026-10-12 미결 5·9 답, 명세 [goal/loop-7g-imposter.md](goal/loop-7g-imposter.md)).**
  사용자 답: 배정 확률 **3인 10%·4인 25%**(1·2인 없음), 액수는 "밸런스는 표에서 조절 가능하게 하고 넘기자", 판 끝 트럭 밖에 남겨져 죽은 사람이 있으면 **승리로 치지 않는다**,
  새벽 화면에서 **모두에게 공개**. 새 셀 `CL9Rules.ImposterChanceByPlayers` [0, 0, 0.1, 0.25]·`ImposterWinReward` 30·`ImposterKillReward` 20 — 액수는 초안.
  - 구현에서 정한 것: 역할은 `ACL9PlayerState::bImposter` 를 **본인 머신에만 복제**(COND_OwnerOnly)해 다른 머신은 새벽 공개(`FCL9ImposterResult`, 게임 스테이트) 전까지 모른다.
    알림은 HUD 장소 알림 자리에 6초. 판정은 `EndNight` 에서 **남겨짐보다 먼저** — 트럭 밖의 산 사람이 아직 "산 사람"이라 남겨짐 죽음이 승리가 되지 않는다.
    직접 타격 = 다른 사람의 `KilledByPlayer`(7d)가 임포스터. 보상은 7f 의 한 번 지급에 접는다 — 이긴 임포스터는 귀가 보상 **대신** 성공 보상, 실패하고 살아 온 임포스터는 보통 귀가 보상,
    임포스터를 죽인 사람은 처치 보상을 **더해**(살았든 죽었든). 시험용 `cl9.imposter`(판 중엔 지금, 로비면 다음 판).
  - 한계: 스테이지의 첫 판은 클라이언트가 접속하기 전에 시작해 호스트 혼자로 굴린다(임포스터 없음) — 레버로 시작하는 둘째 판부터 전원이 후보. 호스트가 죽은 판의 클라이언트 임포스터는 판 시계로 떠난다(레버는 호스트 전용).
  - 확인(2026-10-12): 빌드 1회(스마트 앱 컨트롤 차단 → 재링크), `CL9.Run.Imposter` 통과, 리슨 2인 PIE 네 판 — 성공(+30, 양쪽 새벽 `— 성공`)·직접 타격 실패·처치(+20, `에게 처치됨`)·남겨짐 실패(실패한 임포스터는 귀가 보상 10),
    알림은 임포스터 본인 세계에만 보였다.
- **처치 기여 — 몬스터가 깨운 사람을 기억, 그 몬스터가 남을 죽이면 깨운 사람의 기여 — 루프 개편 7h 구현(사용자 2026-10-11 결정 4 + 2026-10-12 미결 6 답, 명세 [goal/loop-7h-kill-contribution.md](goal/loop-7h-kill-contribution.md)).**
  사용자 답: 기여는 **몬스터를 깨운 사람으로 한정**, 인정 시간은 셀(`CL9Rules.ContributionWindow` 초안 30초), 쓰는 곳 셋 다 — 임포스터 처치에도 인정, 새벽 화면 기록, 임포스터 유도 보상(`ImposterAssistReward` 초안 10).
  - 구현에서 정한 것: 깨운 사람은 실제로 깬 순간(Asleep → WakeTelegraph)에만 적는다 — 근처에서 걷거나 뛴 사람, 또는 경보의 원인(울음 = 그 개체가 감지한 플레이어, 죽음의 비명 = 그 개체를 죽인 플레이어).
    경보는 한 번만 넘긴다(경보로 깬 개체가 다시 울어도 자기 깨운 사람을 넘기지 않는다). 소음 보고는 그림만이라 아무것도 깨우지 않는다(정찰).
    기여는 피해자 `PlayerState` 의 호스트 전용 칸에 남고 부활에서 지운다. 임포스터 유도 보상은 이겼든 졌든 준다.
  - 확인(2026-10-12): 빌드 2회(접근 오류 1회), 리슨 2인 PIE — 잠든 거위 둘 중 하나를 호스트가 죽이자 다른 하나가 `woken by '<host>' (alarm)`, 그 거위에게 클라가 죽자 기여·양쪽 새벽 목록,
    클라가 임포스터면 호스트가 처치 보상 +20, 호스트가 임포스터면 성공 30 + 유도 10 = 40, 창을 0.5초로 줄이면 기여 없음.
- **회사 — 걸어 다니는 레벨, 판을 싣고 레벨을 건넌다, 정산 미달은 패널티 후 계속, 구매는 회사에서 — 루프 개편 7i 구현(사용자 2026-10-11 결정 2 + 2026-10-12 미결 2·10 답, 명세 [goal/loop-7i-company.md](goal/loop-7i-company.md)).**
  사용자 답: 회사는 **걸어 다니는 레벨**, 정산 미달은 **게임 오버 대신 패널티 후 계속**, **장비 구매는 회사로**(트럭엔 판매 창구만), 스킨 상점·마이페이지도 회사 안(7k).
  **번복**: 옛 결정(로비 레벨 + ServerTravel 은 상태 이월 때문에 하지 않는다)을 뒤집는다 — 회사가 별도 레벨이라 이동이 필요하다. 이월은 새 장치가 아니라 기존 판 저장(`SaveRun`/`LoadRun`)과 게임 인스턴스의 "이어 간다" 표시로 한다.
  **결과**: 미달이 판을 끝내지 않으므로(그리고 전멸도 판만 끝내므로, 7a) 지금 규칙에서 판은 끝나지 않는다 — progression.md 의 "언젠가는 진다"가 성립하지 않는다(사용자 확인 필요 시 질문).
  - 구현에서 정한 것: 회사는 `CL9Rules.CompanyLevel`(소프트 레벨, 비면 옛 흐름)로 알아보며 게임 모드 클래스를 따로 두지 않는다. 정산일 로비의 레버 = 회사행, 회사의 레버 = 목적지 판이 고른 사이트행.
    이동은 비심리스 `ServerTravel`(다음 틱, 리슨 유지), 도착 레벨이 미로보다 먼저 판을 되돌리고 주문은 다시 뽑지 않는다. 회사의 정산 화면은 HUD 가 복제된 지난 정산에서 파생해 연다.
    `Lvl_Company` 는 그레이박스(엔진 큐브 방·점광·트럭 패드·패널·목적지 판) — 아트는 나중. 미달 패널티 셀 `QuotaMissPenaltyRate` 초안 0.3.
  - 확인(2026-10-12): 빌드 3회, 리슨 2인 PIE — 양계장 3일차 로비 → `cl9.depart` → 두 머신 다 회사(일차·코인·주문 이어짐), 300/800 미달 → 90 차감 후 계속, 1000/800 통과 화면 양쪽,
    프라이팬 구매(210 → 10) → 양계장 복귀 시 패드 옆에 다시 섬, 목적지 판으로 대농장 고른 뒤 레버 → 두 머신 다 대농장 3일차 판 시작.
- **판 시작 연출 — 트럭 도착 카메라, 오늘의 주문 카드, 마당의 긴장(문 위 깜박이는 등·짙은 안개·건물 안 울음·문간 실루엣), 판 시계는 도착부터 — 루프 개편 7j 구현(사용자 2026-10-12 미결 8·11 답, 명세 [goal/loop-7j-round-start.md](goal/loop-7j-round-start.md)).**
  사용자 답: 주문서는 도착 연출 끝에 가운데 카드(몇 초 뒤 HUD 주문 줄로), 긴장 요소는 셋 다 — 소리·조명(건물 안 울음·깜빡이는 마당등·낮게 깔린 안개), 보이기만 하는 기척(창문에 스치는 실루엣, 공격 없음), 시간 압박(판 시계가 트럭 도착부터).
  - 구현에서 정한 것: 도착은 **레벨이 서고 여는 첫 판**만(레버로 연 판은 트럭이 이미 서 있다). 트럭은 **보이는 메시만** 후진해 들어오고 판정용 트럭은 제자리. 잠금·무적은 전원 인트로의 관문(`IsIntroLocked/IsIntroInvulnerable`)에 도착 창을 접어 넣었다.
    주문 카드는 WBP 를 바꾸지 않고 HUD 가 런타임으로 만든다. 마당 연출은 머신마다 로컬 액터(`ACL9YardTension`)가 문을 찾아 한다 — **키트에 창문이 없어 실루엣은 문간**(사용자 문구와 다름, 보고에 적음).
    마당 모듈에 등 기둥이 없어 깜박이는 등은 문 위 작업등이다. 시간 압박은 새 장치 없이 판 시계가 도착 시작부터 흐르는 것으로.
  - 확인(2026-10-12): 빌드 4회(그림자 이름 오류 1, 레벨 첫 판 서버 시각 0 → 도착·카드가 안 섬 → 순번·표시로 바꿈, 클라이언트 접속 직후 뷰 타깃 회수 → 다시 잡기),
    리슨 2인 PIE — 양쪽 도착 카메라 시작·끝(arrived)·카드, 레버로 연 둘째 판은 카드만, 문 9·3개에 등, 울음 8~16초, 실루엣 문마다 한 번, 안개 0.10 → 전원 뒤 0.05, 3일차 로비 새벽 화면에 "오늘은 정산일" 줄(7i 후속).
- **스킨 — 표·옷장(상점·마이페이지 한 화면, 회사 안)·지갑 보유·복제 적용, 대역 스킨으로 구조만 — 루프 개편 7k 구현(사용자 2026-10-11 결정 1 + 2026-10-12 미결 2·3 답, 명세 [goal/loop-7k-skins.md](goal/loop-7k-skins.md)).**
  사용자 답: 스킨 모델이 들어오기 전에 **구조만 대역 스킨으로**(색만 바꾼 농부 두세 벌, 몸 전체 모델 교체 방식, 모델이 오면 표에 행만), 스킨 상점·마이페이지는 **회사 안**.
  - 구현에서 정한 것: 새 행 종류 `UCL9SkinDef`(`SkinTable` — 몸 메시·색 칸·색 파라미터·색·가격·기본), 대역 4행(기본 0 · 빨강 20 · 파랑 30 · 금빛 60 이더, 초안). 색 칸 이름은 메시의 재질 칸 이름 `M_CL9_Farmer_Cloth`(행 데이터).
    보유·입은 스킨은 각자의 지갑 파일(버전 2). 입은 스킨은 주인이 빙의 때(호스트 `OnPossess`, 클라이언트 `AcknowledgePossession`)와 바꿀 때 호스트에 알리고(`Server_SetSkin` — 행이 있는지만 본다),
    캐릭터의 복제 칸 `SkinAlias` 로 모든 머신이 입힌다(행의 몸으로 되돌린 뒤 재질 덮어쓰기를 걷고 옷 칸에 동적 재질로 색). 상점·마이페이지는 한 화면(옷장) — 자기 몸은 주인 화면에 안 보여(1인칭) 미리보기는 없다.
  - 확인(2026-10-12): 빌드 4회(UHT 이름 충돌·그림자 이름 2, 클라이언트에 치트 매니저가 없어 `cl9.skin` 을 지갑 쪽 로컬 명령으로 옮김), 리슨 2인 PIE — 호스트 빨강 구매 140 → 120·금빛 → 두 세계 모두 호스트 몸 금빛,
    클라이언트 파랑 구매(60 → 30, 파일에 남음) 뒤 PIE 재시작 → 빙의 때 실제 RPC 로 `farmer_blue` → 두 세계 모두 클라이언트 몸 파랑, 회사 옷장 화면(보유 이더·4줄·입는 중 비활성) 스크린샷.
- **판의 끝 — 정산 기간 = 스테이지, 별로 목적지 해금, 마지막 목적지 = 클리어. 실패로는 끝나지 않는다 — 루프 개편 7l (사용자 2026-10-13, 명세 [goal/loop-7l-stage-stars.md](goal/loop-7l-stage-stars.md)).**
  7a(전멸 = 패널티)·7i(미달 = 패널티) 뒤로 판이 끝나는 길이 없었다. 사용자 답: 실패 끝은 **없음 유지**, 끝나면 **게임 오버 화면 → 시작 레벨**, 구조는 오버쿡식 스테이지와 리썰식 연속 런 사이에서
  **섞은 안**(정산 기간 하나 = 스테이지, 셀 하나로 지금 방식과 오감), 스테이지 사이에 **코인·장비·일차는 이어진다**, 별 = **기간 수입 ÷ 목표**(초안 100·150·200%), **앞 목적지 별 1 로 다음 목적지 해금**(사이트 표 순서),
  **마지막 목적지를 별 1 이상 = 클리어**.
  - 구현에서 정한 것: 스위치 `CL9Rules.bStageStars`(true), 셀 `StageStarRatios`·`StageUnlockStars`. 별·해금은 **판의 것**(GameState 복제 + 판 저장본 v3) — 기계에 영구로 두면 클리어한 저장 뒤 새 판이 처음부터 다 열려 "오르는 판"이 아니게 된다.
    이번 판에 정산한 목적지는 열린 것으로 본다(타이틀에서 뒤 사이트로 시작한 판). 클리어 화면은 게임 오버 화면의 다른 얼굴(`FCL9RunOver.bCleared`), 회사에서는 정산 화면(별)을 닫은 뒤 선다.
    반려: 기계 영구 해금(위), 별도 스테이지 표(섞은 안은 기간·사이트를 그대로 쓴다).
  - 확인(2026-10-13): SAC 가 빌드 규칙 DLL 을 막아 사용자가 SAC 를 끈 뒤 빌드 1회, 자동 테스트 `CL9.Run.StageStars` 통과, 리슨 2인 PIE — 양계장 1300/800 ★★☆·대농장 열림·목적지 판은 열린 곳만,
    대농장 900/2600 ★0(미달·안 열림) → 10000/4550 ★★★·돼지 농장 열림, 돼지 농장 20000/5200 → 정산 화면 뒤 클리어 화면(호스트·클라이언트, 목적지별 별), 레버 거부, 스위치 끔 = 7i 그대로.

## 2026-10-13

- **위협은 세 층 — 그냥 동물 · 탈 없는 일반 몬스터 · 빈 것(주인), 빈 것은 한 곳에 여럿일 수 있다 (사용자 2026-10-13, 정본 [plan/story.md](plan/story.md) "위협의 세 층").**
  스토리의 "곳 하나에 주인 하나"와 "진짜 닭은 사라졌다"를 **번복**한다 — 주인은 한 종류이고 마릿수는 스폰 표가 정하며, 닭들은 남아 있고 그 사이 몇 마리가 빈 것이다.
  리썰 컴퍼니의 "흔한 약한 것 많이 + 드문 무서운 것 한둘" 구조를 옮긴 것이다. 지금 목적지 표의 닭 8~12는 층이 없어 그냥 닭인지 빈 것인지가 어디에도 없었다.
  - **거위·너구리는 사연 없이 산다** — 2026-10-11 스토리의 "있었던 일이 안 나오면 뺀다"를 이 둘에 한해 **번복**. 빈 것이 아니라 탈 없는 일반 몬스터 층이고 광폭화·실밥·기록이 없다. 그때 적어 둔 사연 두 줄은 폐기 표시만 하고 남겼다.
  - 데이터 요구(구현 전, [plan/data/monster/common.md](plan/data/monster/common.md) "층"): 행마다 `Tier`(Animal/Monster/Hollow), 빈 것 닭의 별도 행(겉모습은 `chicken`과 같음), 스폰 표를 행 단위로 나눠 층별 수, 판 중간 스폰(7e)은 Animal 층만.
  - 정하지 않은 것: 양의 처리, 빈 것 닭 행 이름과 `chicken`과의 값 차이, 그냥 닭의 위협 세기, 돼지우리처럼 이야기가 "혼자"인 곳에 그냥 동물 층을 두나. 수치는 전부 밸런스 때.
- **양계장은 실내 산업형 계사로 다시 만든다 — 맵 오브젝트는 Tripo로 새로, 기존 양계장 조각·소품은 대체 (사용자 2026-10-13, 목록·톤·랜덤 배치 규칙 [asset/map/henhouse_indoor_objects.md](asset/map/henhouse_indoor_objects.md)).**
  지붕(`bBuildRoofs`)이 이미 발자국 전체를 덮고 마당 모듈은 "지붕 아래 하역장"이라 생성기 변경 없이 비주얼만 바꾼다. 스토리의 "동네에서 제일 작은 집, 닭 서른 마리"는 이에 맞춰 고칠 대상(미결).
  - 정하지 않은 것: 벽 높이(`barn` 630 / 팩 실측 400 / 층고 610), 새 문틀 폭, Tripo 재단 공정 자동화 범위, 리썰식 시간 스폰 곡선 채택 여부(판 중간 스폰 7e는 이미 있다).
- **돼지우리에도 그냥 돼지 층을 둔다 — 세 층은 곳마다 다 있다. 양(`sheep`)은 뺀다 (사용자 2026-10-13, 세 층 결정의 후속).**
  story.md 돼지 치는 집의 "우리 안에 큰 놈 하나만 남았다"를 "남은 돼지들 사이에 큰 놈이 몇 있고 다른 돼지들이 옆에 가지 않는다"로 고쳤다.
  `pig` 행은 빈 것, 그냥 돼지는 별도 행(미정). 양은 어느 층에도 없다 — 문서는 폐기 표시만 하고 `MonsterTable` 행·`Lvl_Flat` 배치·에셋 정리는 별도 작업.
- **그냥 동물과 빈 것은 같은 행 — 광폭화 게이지가 차면 터지고, 문턱은 개체마다 다르다. 플레이어는 HP 0이면 죽는다 (사용자 2026-10-13, 같은 날 앞 항목의 "빈 것 닭 별도 행·그냥 돼지 별도 행"을 번복).**
  사용자 말 그대로: "닭들이 광폭화 게이지가 꽉 차면 괴물로 변신한다. 광폭화 전이랑 후랑 다른 것뿐. 닭마다 게이지 기준이 달라서 어떤 닭은 금방 변하고 어떤 닭은 이번 게임 내내 변신을 못 한다. 우리도 0이 되면 죽어야지."
  그래서 `Tier` 는 `Beast`(게이지 있음 — `chicken`·`pig`) / `Monster`(게이지 없음 — `goose`·`raccoon`) 둘이고, 빈 것 수는 스폰 표가 아니라 문턱 분포가 정한다.
  스토리 틀의 "추격 — 죽이지 않는다"를 "바로 죽이지 않는다, HP 0이면 죽는다"로 **번복** — 데이터(HP 3·`CatchDamage` 1)는 2026-10-11부터 이미 그랬다.
  - 게이지 컬럼 초안 [plan/data/monster/common.md](plan/data/monster/common.md) "광폭화 게이지"(`RageThresholdMin/Max`·`RageGainPerEvent`·`RageDecayPerSec`) — 구현 전, 값 없음.
  - 정하지 않은 것: 수치 전부, 게이지가 줄어드는지·터진 뒤 되돌아오는지, 플레이어가 "거의 찼다"를 읽을 신호가 있는지, 평상시 행동(알을 둥지로 미는 것)을 문턱 낮은 개체만 하는지.
- **판 레벨의 납품은 트럭 창구 — 트럭 옆면 바깥에 납품 창구를 세운다 (사용자 2026-10-13, 명세 [goal/truck-counter.md](goal/truck-counter.md)).**
  `/cl9film` 을 만들다 판 레벨(양계장·대농장·돼지 농장)에 납품 창구가 없다는 것이 드러났다(`BP_CL9Counter` 는 Lvl_Flat 에만, 스폰하는 코드도 없음) — 요리를 해도 코인을 못 번다.
  사용자 답: "트럭 창구에서 납품하게 만들자", 자리는 **트럭 옆면 바깥**(짐칸 안 레버 옆 안은 반려 — 짐칸이 좁아진다).
  - 구현에서 정한 것: `ACL9GameMode::SpawnTruckCounter`(트럭·레버 다음), 셀 `TruckCounterClass`(블루프린트)·`TruckCounterOffset`(트럭 공간, 자리표시 (350, −267, 0)), 레벨에 창구가 이미 있으면 세우지 않는다(재고가 갈리지 않게).
    창구 동작은 그대로(E 납품·빈손 E 판매). 회사에는 세우지 않는다(판이 없다). 모양은 100 cm 창구 상자 — 트럭 몸체에 창을 뚫으면 오프셋만 옮긴다.
  - 확인(2026-10-13): 빌드 1회, Lvl_Farm PIE 봇 납품 → `Order delivered: fried_egg — 377`·코인 0 → 377, Lvl_Flat 은 `already placed`.
- **양계장(실내 계사) 오브젝트 제작은 이미지·Tripo 3D·Blender 재단·FBX 납품까지 전부 ChatGPT에 이관한다 (사용자 2026-10-13 "아예 이관, 넌 여기서 끝").**
  브리프 [asset/map/henhouse_indoor_handoff.md](asset/map/henhouse_indoor_handoff.md)(첨부 6개 + 번들). 이쪽에 남는 것은 UE 임포트·`PropSets` 연결과 벽 높이 H 결정뿐.
  같은 날 앞서 "이미지만 ChatGPT, 3D·재단은 이쪽"으로 적은 것(prompts.md 머리글·henhouse_indoor README)을 이 결정이 덮는다.
- **Fab 팩 둘 구매 전제 — Stylized Warehouse(하역장 소품)·Stylized Abandoned Factory–Workshop(등기구·설비·공용 소품). 팩이 주는 것은 안 만들고, 껍데기·정체 소품·농장 전용 소품은 자체(ChatGPT 이관) 유지 (사용자 2026-10-14).**
  분담표 [asset/map/henhouse_indoor_objects.md](asset/map/henhouse_indoor_objects.md) §6, 이관 브리프에 "Purchased packs" 절 추가. 팩 벽·바닥은 500 격자와 안 맞아 생성기에 안 넣는다. 팩 재질은 UE 틴트로 팔레트에 맞춘다.
  거른 것: Farm Megapack(한낮 야외, 실내 어둠과 반대), Modular Industrial Warehouse Kit(6m 격자·사실풍), Replex·Trapdoor·Jemini 팩(사실풍 PBR).
- **임포스터 굴림은 트럭 도착 연출 안으로 — 도착이 있는 판은 도착이 끝나고 들고 온 인원이 다 붙은 순간에 굴린다, 레버로 연 판은 지금처럼 판 시작에 (사용자 2026-10-14 "트럭에서 움직이는 시퀀스에서 임포스터인 거 알려주는 거지", 명세 [goal/loop-7g-imposter.md](goal/loop-7g-imposter.md) "Decision 2026-10-14").**
  레벨의 첫 판은 클라이언트가 붙기 전에 굴려 호스트만 후보였다(2026-10-12 "첫 판 굴림 시점은 나중에 정한다" — 이 결정이 닫는다).
  "전원 접속 뒤"를 지키려고 트래블이 인원수를 들고 간다(`UCL9GameInstance::SetRunCarry(FromSite, Players)`, 사용자 답 "인원수를 들고 가서 기다린다").
  다 붙지 않으면 도착이 끝나고 `CL9Rules.ImposterRollGrace`(초안 30 s, 네트워크 대기 상한)가 지난 뒤 그때 붙은 사람으로 굴린다. 보류 중에 끝난 판은 굴리지 않는다.
  알림 `Hud.ImposterAssigned` 는 그대로 — 굴림을 따라와 도착 끝의 주문 카드와 같은 순간에 뜬다. 보류 중 `cl9.imposter N` 은 다음 굴림의 지정이 된다.
  - 확인(2026-10-14): 빌드 1회, 리슨 2인 회사 → 양계장 트래블에서 `2/2` 로 굴려 지정된 클라이언트가 임포스터, 레버 판·1인·보류 중 종료 모두 기대대로, `CL9.Run.Imposter` 통과.
  - 범위 밖: 모든 판에 트럭 이동 연출을 넣는 것([goal/loop-rework.md](goal/loop-rework.md) "다음 명세") — 그때 다시 본다.
- **루프 개편 뒤 남은 결정 여덟 개 (사용자 2026-10-14, 질문 창 답).**
  - 첫 정산 기간도 **세 판**으로 맞춘다 — 정산일을 3·6·9일차에서 4·7·10일차로 옮기고 `QuotaCurve` 첫 키도 4일차로(7a·7i 가 남긴 "첫 기간 1·2판"을 닫는다). 구현은 다음 작업.
  - 마당 실루엣은 **문간 그대로**(건물에 창문이 없다, 7j).
  - 영어 문자열은 **고치지 않는다** — 한국어만 유지, 번역은 사용자가 정한 시점에 한꺼번에(Hud.NightWarn 의 en "night" 포함).
  - 스킨 모델은 **사용자가 만드는 중** — 오면 표에 행만 더한다(7k 구조 그대로).
  - 밸런스는 **초안 그대로, 플레이 뒤 조정** — 별 기준(목표 150% = ★★)·첫 목표 800·스킨 20/30/60·임포스터 성공 30/처치 20.
  - 트럭 납품 창구는 **다음 에디터 작업 때 레벨마다 자리를 눈으로 맞춘다**(100 cm 상자 대역 유지).
  - 양계장 내비 범위가 건물보다 작은 것은 **맵을 만들 때 같이 고쳐져야 하는 맵 쪽 일**(사용자 "맵 만들 때 내비도 당연히 같이 고쳐져야") — 스테이지 설정(`Tools/Unreal/setup_stage.py` 의 NavBounds)이 할 일이고, 양계장 실내 재제작에서 맞춘다.
  - 양계장 실내 **벽 높이 H = 630 cm** — `barn` seg kind 와 같아 벽이 한 줄로 선다([asset/map/henhouse_indoor_objects.md](asset/map/henhouse_indoor_objects.md) §7-1 을 닫는다).

- **맵 생성을 리썰(DunGen) 방식으로 — 양계장은 Flow 생성기, 대농장·돼지 농장은 지금 생성기 그대로 (사용자 2026-10-15 "우리는 리썰방식이 맞다" + 질문 창 답, 나머지는 사용자 "자러 갈 테니 알아서 판단해서 리썰처럼 넣어"로 리드가 검토 패널을 거쳐 정함, 명세 [goal/lethal-layout.md](goal/lethal-layout.md)).**
  - 사용자: 리썰식만 만든다(R.E.P.O. 선택 모드 없음) · 판 크기는 본줄기 길이 · 큰 방은 한 판 한 번, 사이는 복도, 맵이 작아지는 것을 받아들인다 · 층 유지 + 계단 위치 규칙 · 전부 실내 · 모듈에 문 자리를 데이터로 둔다("우리 리소스를 수정해야지") · 계단은 가중치로, 최소 보장·낮은 상한("계단 너무 많으면 위아래 와리가리") · Special 은 깊이로 끼운다.
  - 리드: 위층 크기 = 본줄기 길이 × `UpperFloorShare`(양계장 0.5) · 고리 확률 1.0, 다 지은 뒤 마주 보는 문 자리끼리만 · 대농장·돼지 농장은 `Henhouse` 생성기 유지(공유 모듈 표의 기존 열은 안 고치고, Flow 전용 실내 입구 행은 `Rarity` 0 으로 `Henhouse` 가 안 본다) · 곁가지는 통째로 짓고 못 맺으면 되돌린다(가지 끝 정리 대신) · 계단은 본줄기 옆방 · `Hops` 는 고리 뒤 걷는 문 거리로 다시 잰다 · `LayoutHash` 는 별칭 글자로 섞는다(FName 해시는 머신마다 다를 수 있다).
  - 정정: `reference/lethal/lethal-map.md` 의 "가지 22~25개"는 리썰이 쓰지 않는 Global 값 — 실제는 본줄기 타일의 남는 출입구마다 가지 하나.

- **파동 그림을 CCL8 점 방식으로 되돌린다 — C++이 매 틱 파면 자리에 점을 찍고 나이아가라는 그리기만 한다 (사용자 2026-10-15).** 사용자가 CCL8 웹판
  (`chicken-heist.html` v0.27.1) 녹화를 보여 주며 UE 발소리 이펙트가 "밤티 같다"고 지적했고, 원인을 보고받은 뒤 "바닥 바퀴살, 벽 세로 커튼, 페이드까지" 다시
  만들라고 지시했다. 2026-09-16 의 "점이 아니라 원이 퍼진다"(`NS_CL9_Wave` — 점이 날아가는 끊김 없는 원 선)를 이 결정이 덮는다.
  - 구조: `UCL9WaveRenderer` 를 `UTickableWorldSubsystem` 으로 — 파면을 키우며 바닥 점 고리(`RingSpacing` 마다)·벽 세로 점 기둥을 고정 풀(`Dots.PoolSize`)에 찍고,
    풀을 배열(`User.DotPositions`·`DotColors`·`DotSizes`)로 `NS_CL9_WaveDots` 에 넘긴다. 차폐는 C++ 광선(CCL8 `castAll`)으로 돌아왔다 —
    [systems/wave.md](systems/wave.md) 점 그림·점 시스템.
  - 표: 행의 `Niagara`·`Thickness` 열을 지우고 전역 `DotSystem`·`Dots`(점 모양 23필드, CCL8 환산)를 더했다. 광선 64·120 행을 180 으로(CCL8 `STEP_RAYS`) —
    광선이 바퀴살 각도라 적으면 그림이 안 선다. 점 지름은 PIE 에서 녹화와 맞춰 3.2·2.8·3.3 cm — [plan/data/wave.md](plan/data/wave.md) `Dots`.
  - 몬스터 행의 `bWallStamp` false(2026-09-16 "무소음 지형 읽기 금지")는 그대로 — CCL8 녹화의 닭 소나는 벽에도 커튼을 세웠지만, 그건 설계 결정이라 이번에 바꾸지 않았다.
  - 확인(2026-10-15): 정식 빌드 성공, Lvl_Flat PIE `cl9.wave run`(slomo) 에서 바닥 바퀴살·우리 벽 세로 커튼·닭 소나 분홍 바퀴살이 섰다. **16,384 점 풀의 프레임 비용은 아직 재지 않았다.**
  - 2차(같은 날, 사용자 "점이 너무 두껍다 — 웹은 딱 붙어서 가는 점인데 여기는 총알이 튀는 것 같다"): CCL8 웹판을 같은 각도로 찍어 화소로 비교했다.
    ① 내 파동에도 슬롯 색(흰색)을 씌우던 것을 CCL8 D16 대로 남의 파동에만 ② 재질 `EmissiveScale` 0.3 — 밤 노출 고정·톤매핑으로 점이 255 로 타던 것을
    CCL8 화소(≈112,135,149)에 맞춤 ③ 점 고리 간격 8.5 → 3.5 cm(사용자 녹화의 이어진 바퀴살 = 고주사율 모니터 간격), 풀 16384 → 32768
    ④ 점 지름 2.2·1.9·2.3 cm ⑤ 벽 자국은 실제 벽 꼭대기까지만(`MeasureWallTop`) — 1.5m 우리 위 허공에 점이 흩날리던 것. 정식 빌드 성공,
    0.24초마다 달리기 파동 4초 동안 프레임 16.82ms(없을 때 16.77ms, 에디터 60fps 상한 안 — 여유분은 미측정).

- **파동에 긴박도와 물결을 — 체력·상태·광폭화에 따라 퍼지는 모양이 달라지고, 쓰는지는 표가 정한다 (사용자 2026-10-15, 질문 창 답).** 입력 넷(몬스터 상태·몬스터 체력·
  캐릭터 체력·광폭화 게이지)을 전부, 효과 넷(색·밝기·물결 겹수·흔들림·펄스 빈도)을 전부, 리소스는 물결(밝은 마루 띠 + 뒤따르는 메아리)로 골랐다.
  명세 [goal/wave-urgency-ripple.md](goal/wave-urgency-ripple.md), 구조 [systems/wave.md](systems/wave.md) 물결·긴박도, 값 [plan/data/wave.md](plan/data/wave.md) 물결·긴박도.
  - 긴박도 = 행이 켠 입력 중 가장 큰 값(0..1), 복제된 값만 읽는다. **반경·속도는 바꾸지 않는다**(링 = 판정 반경, 소리의 속도는 하나).
  - 몬스터에 복제 `MaxHp`(Hp 를 굴린 값) 추가. 광폭화 게이지는 구현 전이라 `ACL9Monster::GetRageAlpha()` 가 0 을 돌려주는 연결 자리만 — 게이지 명세 몫.
  - 펄스 빈도는 호스트에서 `monster_prox` 반복 간격을 줄인다(표현 전용 소음이라 판정은 그대로).
  - 확인: 빌드·PIE — 체력 1/3 달리기 파동 주황빛·메아리 2·흔들림, 한 대 맞은 닭 소나 긴박도 0.25·간격 0.97초. 추격 상태 입력은 닭이 추격하지 않아 직접 못 봤다.

- **파동 조정 — 몬스터 파동은 발밑에서, 퍼지는 속도 510 → 340, 소나 간격 1.2 → 2.5초 (사용자 2026-10-15, 질문 창 답).** "땅바닥에서 나와야 되는데 몬스터
  가운데서 나온다" — 몬스터 소음 셋이 구 중심(바닥 위 약 40cm)을 보고하던 것을 `ACL9Monster::GetFootLocation()`으로. "파동 나오는 게 너무 빨라 자주 나온다" —
  소나 `RepeatInterval` 2.5초(추천안), 긴박 펄스 배율 2 → 1.5(최소 약 1.7초), 퍼지는 속도는 전 행 340cm/s(CCL8 510에서 — 사용자 "퍼지는 것도 느리게"),
  마루 수명 0.06 → 0.09초(띠 길이 약 31cm 유지). 표 값은 사용자의 PIE 중에 메모리에 들어갔고, 저장과 발밑 C++ 반영 빌드는 그 PIE가 끝난 뒤.

- **파동 벽 커튼은 벽면에 붙인다 — 충돌점 + 벽 법선 기준, 흩뿌림은 접선 방향만 (사용자 2026-10-15).** "벽에 부딪치면 벽에 자연스럽게 올라가야 되는데
  그냥 밖에 떠서 올라가는 느낌" — 기둥 자리를 광선 방향으로 `StampInset` 9cm 당겨 찍고 `StampJitter` 7.5cm 를 벽 수직 방향에도 주던 것이 원인
  (점이 벽 앞 5~13cm 허공에 서고 비스듬한 벽에서는 더 벌어졌다). 차폐 트레이스가 광선별 벽 법선(수평 성분)을 같이 담고, 기둥은 충돌점에서 법선 쪽으로
  `StampInset` 1.5cm(점이 벽에 묻히지 않을 만큼)만 띄우며, 흩뿌림은 법선 × 위 방향만. CCL8 의 9cm 는 2D 톱다운에서는 보이지 않던 틈이었다.
  - 그래도 "아직 조금 튀어나와 올라간다"(사용자, 같은 날) — 벽 조각(`SM_Henhouse_Wall_*`)의 단순 충돌체가 기둥까지 감싼 40cm 슬래브(닫힌 볼록 하나)라
    차폐 광선이 판벽보다 앞(기둥 면)에서 멈춘 것. 차폐 트레이스·꼭대기 재기를 **복잡 충돌(보이는 삼각형)**로 바꿨다(`bTraceComplex`).
  [systems/wave.md](systems/wave.md) 점 그림 벽, [plan/data/wave.md](plan/data/wave.md) `StampInset`·`StampJitter`.

## 2026-10-15 (룩 개편)

- **화면 룩 개편 — 검정 + 빛 웅덩이, 리썰식 저해상도 + 색 단계화, 구조면 텍스처 (사용자 2026-10-15, 명세 [goal/look-rework.md](goal/look-rework.md)).** 봇 영상이 리썰보다
  못해 보인 원인을 재고(`SkyLeak` 1 의 고른 청색 앰비언트, 그림자 없는 작업등, 20 m 에서 투과율 0.9 인 안개, 전원 인트로 뒤 전부 죽는 램프, 하늘
  앰비언트로 푸르게 물든 "검정" 안개, 단색 면, 빈 복도) 사용자가 고른 것: ① 조명은 검정 + 빛 웅덩이(앰비언트 유지안 기각) ② 후처리는 리썰식
  저해상도 + 단계화(그레인·비네트만, 안 함 기각) — 토글 ③ 벽·바닥·천장·기둥·대들보에 노멀·거칠기 텍스처를 이번에 ④ 복도 소품 세트·퀵슬롯
  밝기·봇 헤드램프/비트레이트 포함 ⑤ 천장 유무 확인(있었다) ⑥ **전원 인트로 뒤 실내 램프는 일부가 살아남는다**(`LampSurviveShare`, 2026-10-06
  "그 뒤 램프는 전부 꺼진다"를 완화 — 전부 꺼짐·희미하게 남김·깊은 방만 죽음 기각). 패널(렌더링·게임 느낌·파이프라인)과 교차 심문이 정한 것:
  `SkyLeak` 은 0 이 아니라 잔여(0.5 — PIE 실측에서 0.15 는 좁은 실내에서 0 과 같았다; 규칙 "손전등 밖은 실루엣만"·동료 식별·몬스터 예고·K7 "죽은 뒤는 첫 어둠보다 밝다"), 거리감은 검은 안개
  투과율로(`FogDensity` 0.5 — 1.0 은 마당이 안개 벽, 새 컬럼 `FogSkyAmbient` 0), 작업등은 `LightConeAngle` 로 아래 향한 스포트 + 그림자(거리 토글 기각 — VSM 페이지
  버림·벽 뚫는 빛), 저해상도는 `r.ScreenPercentage.MaxResolution` 520(정수 배율·45 % 기각), 단계화·그레인은 카메라 모디파이어(NightLighting
  컴포넌트 기각 — 타이틀·로비에 없다), 텍스처는 부모 재질 + static switch(MPC 기각), 봇은 손전등 대신 헤드램프(칸이 바뀌면 꺼진다).
  **규칙 번복 둘**: performance.md "동적 그림자 광원은 셋뿐" → 작업등 스포트 포함, "렌더 기본값 안 바꿈" → 레트로 화면 켜진 동안 예외;
  design-concept.md 규칙 3(Flat Shading)은 구조면에 한해 완화. systems/light.md "룩 개편", plan/data/site.md·module.md·rules.md, settings.md
  "화면", ui.md 퀵슬롯, goal/round-start-truck-intro.md K7 덧붙임.

- **파동 벽 커튼은 밑에서부터 차오른다 — 바닥을 퍼지던 속도 그대로 벽을 탄다 (사용자 2026-10-16).** "벽에 올라가는 게 생각보다 빠르다, 밑에서부터
  차근차근 올라와서 닿는 게 아니라 먼저 들어가 있는 느낌" — 파면이 벽 거리를 지나는 틱에 기둥 높이 전체의 점을 같은 시각에 찍어서, 기둥이 한 번에 서
  있었다. 점마다 생일을 높이에 비례해 늦춰(파면이 벽에 닿은 시각 + 높이 ÷ (행 `Speed` × `Dots.StampRiseScale`)) 생일 전에는 숨긴다. 기본 배율 1 —
  소리의 속도는 하나라는 규칙(링이 멈춘 곳까지가 들리는 곳)을 벽에도 그대로 적용한 것이고, 195cm 기둥이 약 0.55초에 다 선다. 표 값이라 빌드 없이 조절한다.
  - 같은 날 사용자 "차오르는 게 아직 빠르다" → `StampRiseScale` 0.5(약 1.1초). 벽을 오르는 속도는 바닥 속도와 따로 표현값으로 둔다(판정과 무관). 시험 레벨 `Lvl_WaveTest` 영상으로 사용자 확인 — "차오르는 속도 좋다".
  [systems/wave.md](systems/wave.md) 점 그림 벽, [plan/data/wave.md](plan/data/wave.md) `StampRiseScale`.

## 2026-10-16 (룩 표)

- **보이는 값은 룩 표 한 곳 — `/Game/Data/LookTable`(`UCL9LookDef`), PIE 중 실시간 반영 (사용자 2026-10-16 "이 모든거 그냥 하나로 퉁쳐서 테이블로" +
  "값바꾸면 실시간으로 적용되는 모드도", 범위는 "2단계 전부", 명세 [goal/look-table.md](goal/look-table.md), 열 정본 [plan/data/look.md](plan/data/look.md)).**
  화면(해상도·AA·단계화·그레인) · 조명 프로필(로비·판·정전 페이즈 + 램프 생존 비율) · 램프 종류(WorkLamp·Campfire·Flashlight·Headlamp·YardDoor) ·
  HUD(퀵슬롯 색) · 구조면(가족별 텍스처·타일·세기 + 재질 인스턴스 목록)을 한 애셋에 모았다. 사이트 행은 `LightingProfile` 이름, 모듈 소품과
  아이템 행은 `LampKind` 이름만 갖는다(자리 — `LightOffset`·`YardLampHeight` — 는 고르는 쪽). 지운 열: 사이트 `Lobby/Night/BlackoutLighting`·
  `LampSurviveShare`, 소품 `Light*` 7개(오프셋 제외), 아이템 `LightRadius/Intensity/ConeAngle/Scattering`, 규칙 `Look*` 6개·`YardLampIntensity/Radius`,
  퀵슬롯 위젯 `Active/InactiveColor`. **`LampDensity` 는 사이트 행에 남겼다** — 건물마다 다른 등 개수라 프로필에 넣으면 프로필이 사이트 수만큼 생긴다.
  실시간 모드는 구독 대신 **판(Revision) 정수** 하나: 편집·`ApplyNow` 마다 오르고, 이미 틱을 도는 쪽(사이트 맵·아이템 빛·마당 문등·HUD·로컬 PC)이
  자기가 본 판과 다르면 다시 적용한다 — 수명 관리가 없고 평소 비용은 비교 한 번. 포인트↔스포트 전환만 다음 판부터. 구조면은 표가 재질 인스턴스에
  직접 밀어 넣는다(에디터 전용, 인스턴스도 저장해야 남는다). 이주는 `Tools/Unreal/migrate_look_table.py` 한 번(값 복사, 재조정 없음).

- **파동 벽 자국 = 바닥 바퀴살을 벽으로 이어 올린 세로 점선, 차오름 0.75 (사용자 2026-10-16, 시험 레벨 영상을 보고).** "너무 느려서 빨리
  해야 되고, 벽에 딱 붙어서 올라가야 되는데 지금은 공중에 올라간다." — 기둥마다 높이 무작위 3~8점을 접선 7.5cm 로 흩뿌린 점 구름이 벽에 붙은
  모양으로 읽히지 않았다(점 위치는 벽면에서 1.5cm 로 맞았다 — 모양 문제). 벽 자국을 바닥 바퀴살과 같은 광선(`Stride`)에서 같은 점 간격
  (`Dots.RingSpacing` ÷ 행 `StampDensity`)으로 곧게 세워, 바닥 선이 벽에서 꺾여 벽면을 타고 오르게 했다. `StampJitter` 7.5 → 0,
  전역 `StampMinDots`·`StampDotsPerStrength` 삭제, 행 `StampDensity`는 점 수 배율 → 세로 촘촘함으로 뜻을 바꿨다. `StampRiseScale` 0.5 → 0.75
  (1.0 은 빠르다, 0.5 는 느리다의 가운데). 세로 점선 영상을 본 사용자 "여기서 속도만 빨라지면 돼" → 1.0(바닥 속도 그대로 벽을 탄다) → "아직 느리다" → 1.5(약 0.37초).
  - 이어서 "속도는 좋은데 사라지는 건 좀 빨리" → 1.5 확정, 벽 자국 점 수명 `StampLifeMin`/`Max` 2.0/2.5 → 0.5/0.5초 (사용자 지정 값). [systems/wave.md](systems/wave.md) 점 그림 벽, [plan/data/wave.md](plan/data/wave.md).

- **실외(트럭 → 건물 입구)를 랜드스케이프로 깔고, 실외만 안개를 옅게 한다 — 트럭은 건물에서 40 m 떨어진 들판에 선다 (사용자 2026-10-16 "트럭이 건물 바로 앞이 아니라 좀 떨어진 야외에 멈춘다, 그 야외를 랜드스케이프든 뭐든 써서 만들어 봐" + 영상 뒤 "실외만 안개를 옅게, 트럭과 언덕이 보이게").**
  - 양계장 `EntryOffset` 1000 → **4000**(MapTable). 지형은 `Tools/Unreal/setup_outdoor.py` + 명세 `Tools/Unreal/outdoor/henhouse.json`(거리·길 폭·언덕·색) — 건물이 설 수 있는 격자 전체·트럭 길·앞마당은 평지, 길 양옆은 들판에서 언덕(최고 약 17 m)으로. 랜드스케이프 생성은 C++ `UCL9LandscapeTools::CreateLandscape`(엔진이 파이썬에 새 랜드스케이프를 열어 두지 않음). 재질은 무게 레이어 없이 월드 좌표로 칠한다(`M_CL9_Landscape`) — Polyart Stylized Village(Fab, 사용자 구매) 팩을 받으면 그 오토 랜드스케이프 재질로 바꿀 수 있다.
  - 안개: 7j "마당을 건너는 길은 안개가 짙다"(`PrePowerFogScale` 2)를 **실외 구간에 한해 덮는다** — 카메라가 입구면보다 트럭 쪽이면 `CL9Rules.OutdoorFogScale`(초안 0.06)을 곱한다([systems/light.md](systems/light.md)). 건물 안은 그대로.
  - 대농장·돼지 농장은 아직 적용하지 않았다(그 레벨에 같은 스크립트를 명세만 바꿔 돌리면 된다).
- **밝기 손잡이 — 표의 편집용 `Brightness` + 설정 화면 "밝기" 슬라이더, 같은 축 (사용자 2026-10-16 "플래시없으면 너무 플레이 못할정도로 어두워 … 그냥 전체 밝기
  하나 만들어서 그거 값에 맞게 다른것들 세팅" + "걔는 다른 옵션들을 조절해주는 역할이지 그걸로 딴거 뭐하면안돼" + "게임안에서도 나오게해줘").** 룩 표에 판 조명 끝점 두 벌
  (`NightDark`·`NightBright`)을 두고, 프로필의 `Brightness` 를 돌리면 그 프로필의 `Night` 숫자 필드가 끝점 사이로 다시 써진다(에디터, `PostEditChangeProperty`).
  플레이어 설정 `Brightness`(기본 0.5)는 같은 축 위에서 저장된 `Night` 에 차이만 더한다(`ShiftNight`, 로컬). 둘 다 조명 값만 바꾸고 정전·로비는 건드리지 않는다.
  [plan/data/look.md](plan/data/look.md) 밝기, [systems/settings.md](systems/settings.md) 화면.

- **착지 파동 = 벽까지 퍼지는 핑 — 사람·몬스터·아이템을 통과해 타고 넘고, 재사용 대기 2초 (사용자 2026-10-16, 질문 창 답).** "소리파동이 벽 만날 때까지
  퍼지게" → 플레이어만·착지만(달리기는 걸음마다 방 전체가 드러나 손전등을 대신하므로 뺐다), 상한 12m, 대기 2초(표 값), 몸 통과와 타고 넘기는 착지
  행만. 몬스터 위치·실루엣이 드러나는 것은 의도, 소리가 멀리 가는 것은 조심할 점(착지에 청각 판정이 생길 때 다시 본다). 구현은 행 칸 다섯
  (`bUntilWall`·`UntilWallRadius`·`UntilWallCooldown`·`bPassThroughBodies`·`RideHeight`) — 스케일 0 을 신호로 쓰는 안은 로드 검사의 "0 = 잘못된
  행"과 겹쳐 반려. 몬스터 행에 켜면 로드 검사 Error·무시. [goal/wave-landing-ping.md](goal/wave-landing-ping.md), 구조 [systems/wave.md](systems/wave.md),
  값 [plan/data/wave.md](plan/data/wave.md) 3-1, 규칙 예외 [plan/wave.md](plan/wave.md) 1.
  - **실외 자연물은 구매 팩을 쓴다 (사용자 2026-10-16 "유료 에셋들 중에서도 써도 된다", 무료 팩 추가 지시).** 10-04 "Dreamscape Farmlands 팩은 쓰지 않는다"를 **실외(트럭 → 입구) 자연물·길가 소품에 한해** 덮는다 — 실내 모듈 소품·몬스터는 자체 제작 그대로.
    팩: `Content/DreamscapeSeries`(Polyart, 2.1 GB — 나무·풀·건초·통나무·가로등) · `Content/StyleHex_Studio`(FREE Stylized Rocks) · `Content/Stylized_PBR_Nature`(Tiny Talisman, 개인용 무료 라이선스 — 지난 12개월 매출·펀딩 US$100,000 미만 조건, 넘으면 기업용으로 바꾼다).
    배치는 `Tools/Unreal/setup_outdoor_props.py`(명세 `props` 절 — 그룹별 수·평지에서의 거리·목표 높이·간격, 길가 가로등 4개 + 그림자 없는 점광원), 풀은 랜드스케이프 재질의 Grass Output(`LGT_CL9_Outdoor`, 길·비탈·건물 격자 밖).

- **착지 핑은 닿은 몸의 실루엣을 점무늬로 드러낸다 (사용자 2026-10-16).** "핵심은 어두운 곳에서 형체를 알아볼 수 있느냐 — 파동으로 해당 물체의
  실루엣을 알 수 있어야 돼." 어둠 속 시험 영상에서 바퀴살만으로는 사람이 선 두세 줄(6° 간격)이라 형체가 안 읽혔다. 몸마다 메시를 한 겹 더
  붙여(겉껍데기, 스켈레탈은 원래 포즈를 따름) 점무늬 재질 `M_CL9_WaveReveal`을 입히고, 파면이 지나는 만큼 켜졌다 옅어지게 했다. 덧그림 재질 칸은
  피격 깜빡임·조준 윤곽선·조리 표시가 같이 써서 반려. 행 `bRevealBodies`(플레이어 행만) + 전역 `Reveal`. "느려지기"는 파면이 둥근 고리 하나라
  몸 쪽만 늦추면 고리가 찌그러져 반려. [goal/wave-landing-ping.md](goal/wave-landing-ping.md) Silhouette reveal.
  - 어둠 속 영상(`wave_ping_dark_20261016e`)을 본 사용자: "실루엣 의도대로다" — `Reveal` 3.5/0.22/0.5/3/0.6/0.8 확정.

- **코인 눈금 1/8 — 계란 10~15, 첫 정산 목표 100 (사용자 2026-10-17 "처음에 800하면 많아보이는데", "첫 목표는 100", 질문 창 답 "눈금을 줄인다").**
  2026-08-30 승계한 "계란 80~120 = 눈금, 정수 반올림 때문에 키웠다"([plan/economy.md](plan/economy.md))를 번복한다. 모든 코인 값을 옛값 ÷ 8 로
  다시 적었다 — 정수 반올림, 주문 보상은 5 단위: 계란 10~15(깨진 것 같음)·익은 계란 19~25·고기 18~22·익은 고기 31~40, 주문 오믈렛 130~170·
  계란후라이 45~55·고기 요리 145~175, 프라이팬 25, `QuotaCurve` 3일차 100·4일차 200·6일차 325·10일차 650, `WalletIncomeRate` 0.01 → 0.08(계란 한 개 ≈
  이더 하나 유지). 이더(바깥 재화) 쪽 값 — 임포스터 보상 30/20/10·스킨 20/30/60 — 은 그대로. 배율 ×0.8~×1.4 가 10~15 에 걸릴 때의 ±0.5 코인
  반올림 오차는 받아들인다. 별 기준은 비율이라 그대로 — 첫 스테이지 별 1·2·3 = 계란 8·12·16개(혼자 퀵슬롯 4칸을 판마다 한 번 채우면 별 2).
  값은 표 에셋(`ItemTable`·`OrderTable`·`EquipmentTable`·`CL9Rules`·`CL9Progression`)과 [plan/data](plan/data) 각 표 문서.

## 2026-10-17 (손맛)

- **킬 히트스톱 분리·타격 스쿼시 (사용자 "후보 ①②를 CL9Rules 열로 빼서 구현해줘" — [reference/game-feel-social.md](reference/game-feel-social.md)).** 인스타 손맛 릴("0.1초 멈추고 찌그러뜨리고
  흔들고 터뜨리면")을 CL9 와 대조했더니 넷 중 스쿼시만 없었고, 히트스톱은 죽는 타격과 안 죽는 타격이 같은 0.08 이었다. 둘 다 판정·수치에 닿지 않는
  표현 열로 넣는다: `KillHitStopTime` 0.12(Dead 진입 때 `HitStopTime` 대신, 팔도 `bSwingHitKill` COND_OwnerOnly 로 같이), `HitSquashScale` 0.8 ·
  `HitSquashTime` 0.1(Stun·Dead 진입 뒤 메시 상대 스케일을 세로로 눌렀다 2차 감속으로 복원, 가로 1/√세로 로 부피 유지). 09-15 원칙 그대로
  복제된 상태 시각의 **수준**에서 파생한다(에지가 아니라 — 늦게 접속해도 되풀이 없음, 경직 중 재피격은 시각이 바뀌어 다시 눌린다).
  스쿼시는 메시 컴포넌트만 건드린다 — 구 판정·히트박스·넉백·죽음 발사는 그대로. 걸기 전 스케일을 적어 두고 창이 끝나면 그 값으로 되돌린다(내가 바꾼 것만 푼다).
  전역 시간 지연·`CustomTimeDilation` 은 09-18 결정대로 쓰지 않는다. 값은 [plan/data/rules.md](plan/data/rules.md) 타격의 표현, 초안.
  같은 날 [reference/combat-gap.md](reference/combat-gap.md) 가 "밍밍함의 주범은 구조"라고 했으므로 이 둘은 그 해결이 아니라 싸게 넣는 표현 보강이다.

## 2026-10-17 (전투·조작감 결정 — 인계 [review/combat-handoff-2026-10-17.md](review/combat-handoff-2026-10-17.md) §3 축 A~H)

질문 창으로 한 번에 받았다. **A(읽히는 위협 원칙)·E(몬스터 이동)는 보류** — 사용자 "아직 분석을 좀 더 해보자" → Content Warning·Valheim 분석
([reference/contentwarning/](reference/contentwarning/)·[reference/valheim/](reference/valheim/)) 뒤 다시 묻는다. 나머지:

- **G 손전등 — 스위치는 토글만 바꾼다, 들고 있는지와 무관 (사용자 "손전등은 그냥 키면 키는 거고 끄면 끄는 거임. 내가 들고 있다고 켜지는 게 아님,
  무전기랑 유사"; 주머니 속 빛의 자리는 질문 창 답 "머리에서 앞으로").** 2026-09-01~13 의 "활성 칸에 들어야 빛난다 — 빛과 짐을 동시에 못 든다"
  빛 경제([plan/data/item/flashlight.md](plan/data/item/flashlight.md))와 `bLightLostInPocket`, light.md "들면 꺼진 채 시작" 을 번복한다. 조작감 거슬림 1·2
  ([review/control-friction.md](review/control-friction.md))의 해법이다. 구현: 칸 데이터 `FCL9CarriedItem.bLightOn` 이 스위치를 기억, 비활성 칸의
  켜진 손전등은 캐릭터의 주머니 빛 `PocketLight`(카메라 자식, 쓴 헤드램프 자리)이 그린다 — 값은 룩 표 램프 종류 `PocketLight`(`Flashlight` 복사로
  시작), 켜짐은 복제된 `PlayerState.Slots` 에서 모든 머신이 파생(새 RPC 없음), 정전이면 주머니 빛도 꺼지고 호스트가 칸 스위치를 끈다. 켠 채 놓거나
  던지거나 죽어 흘려도 바닥에서 비춘다(리썰). 줍기 전환 규칙(리썰식, 손이 바뀐다)은 그대로. `bLightLostInPocket` 컬럼은 뜻을 잃어 제거.
  [systems/light.md](systems/light.md) 손전등, [systems/inventory.md](systems/inventory.md), [plan/data/look.md](plan/data/look.md) 3.
- **B 짐을 전투에 건다 — 넘어지면 퀵슬롯 아이템을 무작위로 0~2개 떨어뜨린다, 2개는 낮은 확률 (사용자).** 2026-09-15 "짐은 죽을 때만 흘린다"를
  번복한다. 명세 [goal/knockdown-drop.md](goal/knockdown-drop.md), 구현: `CL9Rules.KnockdownDropWeight0/1/2` 40/45/15, `ACL9Character::HostDropOnKnockdown`
  (찬 칸 무작위, 활성 칸 포함·머리 칸 제외, `SpillAtFeet` 공용화). [systems/inventory.md](systems/inventory.md) 잡힘.
- **C 휘두르기·타격을 소음으로 만든다 (권장안 수락).** 벽을 치든 닭을 치든 다른 몬스터가 듣는다. 구현: `CL9Rules` `SwingNoiseRadius` 300·`HitNoiseRadius` 600·
  `NoiseLingerTime` 0.5(순간 소음이 귀에 남는 창 — 몬스터 감각이 매 틱 거리 검사라서), 소음원 `Swing`·`Hit` + 파동 `swing`·`hit` 행, `ACL9Monster::SenseTick` 이
  `GetSwingElapsed()`·`GetSwingHitElapsed()` 로 듣는다. [plan/data/rules.md](plan/data/rules.md) 소리, [plan/data/wave.md](plan/data/wave.md).
- **D 닭 감지 값을 올린다 — 시야 + 걷기 소음 (권장안 수락).** 2026-09 "걷기는 어그로 0"을 번복한다 — `CL9Rules.WalkNoiseRadius` 200(0 이면 옛대로),
  파동 `walk` 행을 켜고 `WalkNoise` 에 묶는다. 닭 `SightIn` 330 → 600 · `SightOut` 219 → 400. 회피 산수(A5)는 예고 거리·속도만 보므로 그대로. 돼지 480·너구리 390 은
  범위 밖이라 그대로 — 종별 순서가 뒤집혔으니 밸런스 때 다시 본다. [plan/data/monster/chicken.md](plan/data/monster/chicken.md).
- **F 두 번째 도구는 미끼(탄 요리 → `decoy` 소음)부터 (권장안 수락).** 명세 [goal/burnt-bait.md](goal/burnt-bait.md), 구현: 바닥에 멎은 탄 요리가 `decoy` 파동을
  내고 `DecoyRadius` 안의 사냥 중이 아닌 몬스터를 Investigate 로 끌어 `DecoyEatTime`(5) 동안 먹게 한다(사람 무시) — `CL9Rules` `DecoyEatTime`·`DecoyEatRadius`,
  `ACL9Monster::BaitItem`. 새 품목 없음. [systems/monster.md](systems/monster.md) 미끼.
- **H 겁먹는 표현은 둘 다 — 공포 게이지 + 소리 세 겹(리썰) 에 첫 목격 스팅어·추격 시작 신호·시선 강제 확장(리포) (권장안 수락).** 전부 로컬 표현 층,
  값은 `CL9Rules`/`SoundTable` 행. 구현: `UCL9FearComponent`(소유 폰, 게이지·소리 네 겹·스팅어·추격 신호) + `UCL9HurtLookModifier` 시선 끌림,
  `CL9Rules` 겁먹는 표현 열 22개([plan/data/rules.md](plan/data/rules.md)), `SoundTable` 6행·`ShakeTable` 2행은 **애셋 비움**. "누구를 쫓는지"는
  복제하지 않고 거리·화면으로 대신한다(새 복제 없음 유지). [systems/character.md](systems/character.md) 겁먹는 표현.

## 2026-10-18 (자율 루프 결정 #2)

- **질문**: 양을 되살릴까요?
- **답(사용자 원문)**: 양도 명단의 몬스터 중 하나. 따로 신경 쓰지 않고 20종 이상 명단 안에서 다룬다.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 2(페이지 결정 ⑥, 페이지 11절, 비교 문서 미결 4) · 분할안 2 일부
- 걸린 것: 백로그 #1 몬스터 기획자, #10 아트 기획자(기획)
- 후보였던 것:
  - ① 되살린다 — 협동 규칙("혼자 있지 마라", "한 명이 붙잡고 한 명이 지나간다")의 자리를 맡을 수 있고, 남아 있는 클립이 9개다.
  - ② 되살리지 않는다(10-13 제외 유지) — 그 자리는 소·여우가 맡는다.
  - 단서: 사용자는 페이지 결정 ⑥에 "양은 뭐야?"라고 되물었다.

## 2026-10-18 (자율 루프 결정 #11)

- **질문**: 새 종을 몇 종까지 늘리고, 세 층 구조(10-13)를 "동물·몬스터 둘"로 다시 쓸까요?
- **답(사용자 원문)**: 20종 이상. 동물일 필요 없음(탈 쓴 귀여운 무언가면 됨). 명단을 고정해 새 맵에 재사용한다. 새 맵마다 새 몬스터를 만드는 비용을 피한다.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 11(페이지 6절) · 분할안 2
- 걸린 것: 백로그 #1 몬스터 기획자, #9 스토리 기획자, #10 아트 기획자(기획)
- 후보였던 것:
  - ① 세 층 구조(10-13)를 그대로 둔다.
  - ② "동물·몬스터 둘"로 다시 쓴다 — 페이지 5절은 이미 층을 둘로 적었다.
  - 단서: 종 수는 원문에 숫자 후보가 없다. 페이지 6절의 패턴 순서는 먼저(쥐·어미 닭·횃대 닭·헛놀람·갈라놓는 문) → 다음(소·박쥐·부엉이·눈먼 개·뻐꾸기·굴) → 정체(여우·농부 탈·병아리) → 나중(병아리 울음·함정·거미줄)이다.

## 2026-10-18 (자율 루프 결정 #13)

- **질문**: 전투 세션의 미커밋 변경(G·H·C·D·B·F)을 어떻게 처리하고, 기획 수치의 기준을 이번 주 미커밋 코드와 커밋본 중 어느 쪽으로 할까요?
- **답(사용자 원문)**: 유지한다. 기획 수치의 기준은 미커밋 코드이고, 그 값이 이번 기획과 다르면 언제든지 바꿔도 된다.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 13(전투 세션 10:32 질문, 답 없음) · 분할안 5
- 걸린 것: 백로그 #12 문서 옮기기(미커밋 변경 처리), 그리고 분할안이 "띄우기 전에 정해야 할 것"으로 적어 역할을 좁히지 않았으므로 #1 몬스터 기획자, #2 아이템·경제 기획자, #3 맵 기획자, #4 판 루프 기획자, #5 메타 기획자, #6 조작 기획자, #7 UI 기획자, #8 시퀀스·사운드 기획자, #9 스토리 기획자, #10 아트 기획자, #11 기술 검토(모두 기획)
- 후보였던 것:
  - ① 이번 주 미커밋 코드를 기준으로 한다 — 페이지의 "지금 값" 표가 이미 "이번 주 미커밋"을 한 열로 두고 있다.
  - ② 커밋본을 기준으로 한다 — 미커밋 변경을 되돌리면 "지금 값"을 커밋본 값으로 다시 읽어야 한다.

## 2026-10-18 (자율 루프 결정 #17)

- **질문**: 동물을 장소에 묶을까요, 어디든 나오게 할까요?
- **답(사용자 원문)**: 잠정 결정(나중에 다시 고민): 추천대로 간다. 집마다 여러 동물을 키웠다는 설정으로 몬스터 명단은 어느 곳에나 나오고, 장소 고유는 그 집의 재료와 탈 쓴 주인만. 일단 만들고 맞추는 건 나중에.
- 답한 날짜: 2026-10-18
- 출처: 분할안 1(관련: 0b 축 E·페이지 원칙 4, 기획자용 정리 `C:\Work\CL9\docs\review\combat-horror-planner-2026-10-17.html` 17번과 "선택지(결정 아님)" 표)
- 걸린 것: 백로그 #1 몬스터 기획자, #3 맵 기획자(기획)
- 후보였던 것:
  - ① 층으로 나눈다: 주인 동물은 그 곳에만 나오고 몬스터 층과 떠돌이는 모든 목적지에 공통 목록과 가중치로 둔다 — 이야기를 지키면서 반대 규칙을 한 맵에 섞을 수 있다 / 떠돌이 종을 몇 종 더 만들어야 한다.
  - ② 한 맵에 여러 구역을 둔다: 몽장처럼 구역마다 그 집의 주인과 규칙이 있다 — 모든 주인이 한 판에 나오고 어디로 갈지 고르는 재미가 생긴다 / 한 판에 다 못 돌고, 모듈 세트가 구역 수만큼 든다.
  - ③ 목적지별 가중치 표를 둔다: 목적지마다 종 목록과 희귀도를 표로 둔다 — 표만 있으면 되어 가장 싸다 / "왜 여기 돼지가"라는 이야기 문제는 그대로 남는다.
  - ④ 주인을 "탈 쓴 것"으로만 좁힌다: 그냥 동물은 어디든 나오고 탈을 쓴 주인만 그 곳에 묶는다 — 동물 다섯 종을 모든 맵에서 쓸 수 있고, 탈 쓴 것의 희소성이 더 커진다.
  - 단서: 분할안은 "네 가지 길 중"이라고만 적었고, 네 길의 내용은 기획자용 정리에서 옮겼다. 기획자용 정리는 어느 길이든 반대 규칙 둘을 한 맵에 두려면 장소에 묶이지 않는 종이 몇 종 필요하고 그 수가 곧 모델 비용이라고 적었다.

## 2026-10-18 (자율 루프 결정 #18)

- **질문**: 미니맵과 네임태그를 보여 줄까요?
- **답(사용자 원문)**: 미니맵·네임태그 둘 다 만들되 테스트용. 실제 플레이에서는 설정으로 켜고 끌 수 있게 하고, 끄고 진행할 것.
- 답한 날짜: 2026-10-18
- 출처: 분할안 4
- 걸린 것: 백로그 #7 UI 기획자(기획)
- 후보였던 것:
  - ① 보여 준다.
  - ② 보여 주지 않는다.
  - 단서: 원문은 질문만 적었고, 각 후보가 얻는 것과 잃는 것은 적지 않았다.

## 2026-10-18 (자율 루프 결정 #19)

- **질문**: 이번 세부 기획의 범위를 양계장 한 곳부터 할까요, 다섯 집 전부로 할까요?
- **답(사용자 원문)**: 다섯 집 전부. 모든 맵 리소스도 만들어야 하므로 범위는 전체 목적지(양계장·대농장·돼지 농장·외양간·찬장·벌집·몽장).
- 답한 날짜: 2026-10-18
- 출처: 분할안 6(모든 역할에 같은 전제 "범위는 사용자가 정한 만큼만")
- 걸린 것: 분할안이 모든 역할의 전제로 적었으므로 백로그 #1 몬스터 기획자, #2 아이템·경제 기획자, #3 맵 기획자, #4 판 루프 기획자, #5 메타 기획자, #6 조작 기획자, #7 UI 기획자, #8 시퀀스·사운드 기획자, #9 스토리 기획자, #10 아트 기획자, #11 기술 검토(모두 기획)
- 후보였던 것:
  - ① 양계장 한 곳을 먼저 한다.
  - ② 다섯 집 전부를 한다.

## 2026-10-18 (자율 루프 결정 #1)

- **질문**: 손전등에 배터리가 없으므로 빛의 선택이 살려면 "빛을 켜면 위험해지는 종"이 적어도 하나 있어야 하는데, 그 빛 규칙을 어느 종에 둘까요?
- **답(사용자 원문)**: 손전등은 무제한(배터리 없음). 안 가져온 건 그 사람 책임이라 빛 규칙 종을 억지로 두지 않는다. 기조: 파훼법은 '그 순간 항상'이 아니라 '내가 준비했을 때 모든 걸 파훼할 수 있어야' — 준비 못 해 간 것부터가 잘못. 어느 종이 빛에 어떻게 반응하는지는 몬스터 패턴 기획에서.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 1(페이지 결정 ⑤-2, 페이지 11절, 비교 문서 미결 5) · 분할안 3
- 걸린 것: 백로그 #1 몬스터 기획자(기획)
- 후보였던 것:
  - ① 너구리: "빛 안에서 봐야 본 것이다" — 정전 중에는 너구리를 막을 수 없게 된다.
  - ② 쥐: 빛에 굳는다.
  - ③ 박쥐: 빛에 흩어진다.
  - ④ 부엉이: 빛에 화낸다.
  - ⑤ 소: 어둠에서는 못 본다.
  - ⑥ 닭: 빛에 끌리거나 멈춘다(어느 쪽인지는 결정 #9).
  - ⑦ 거위: 빛에 깬다 — 9m 시선 보류 원칙과 반대다.
  - 단서: 페이지 5절 돼지 카드는 빛 규칙을 돼지에 두지 않기를 권장했다.

## 2026-10-18 (자율 루프 결정 #6)

- **질문**: 너구리 처치를 어디까지 막을까요?
- **답(사용자 원문)**: 몬스터 패턴 기획에서 정한다(지금은 큰 구조만).
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 6(페이지 5절 너구리 미결)
- 걸린 것: 백로그 #1 몬스터 기획자(기획)
- 후보였던 것:
  - ① A와 B를 함께 둔다 — 권장, 페이지 5절 너구리 카드가 A+B를 권장했다.
  - ② A: 후퇴 450(걷기보다 빠르게).
  - ③ B: 맞으면 즉시 격분해 추격한다.
  - ④ C: 불사.
  - 단서: 원문은 빛 A를 채택하면 정전 판 난이도가 급등하는데 그것이 의도인지, 솔로 판에서 너구리를 찾지 못하는 시간은 어떤지를 함께 물었다.

## 2026-10-18 (자율 루프 결정 #7)

- **질문**: 돼지의 주 규칙을 A(짐을 노린다)로 둘지 B(좁은 길)를 더할지, 광폭화 조건을 "재료 들고 우리 지나간 횟수"로 바꿀지, 날재료도 먹게 할지 어떻게 할까요?
- **답(사용자 원문)**: 몬스터 패턴 기획에서 정한다(지금은 큰 구조만).
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 7(페이지 5절 돼지 미결)
- 걸린 것: 백로그 #1 몬스터 기획자(기획)
- 후보였던 것:
  - ① A만 둔다 — 권장, 페이지 5절 돼지 카드가 A를 권장했다.
  - ② A에 B를 더한다.
  - 단서: 광폭화 조건과 날재료 여부는 원문에 후보가 따로 없다. 페이지 카드는 빛 규칙을 돼지에 두지 않기를 권장했다(결정 #1).

## 2026-10-18 (자율 루프 결정 #8)

- **질문**: 거위가 맞았을 때와 죽을 때의 소리, 놀람 낙하를 둘 종, 빛에 깨는 규칙과 시선 보류 원칙의 충돌을 어떻게 할까요?
- **답(사용자 원문)**: 몬스터 패턴 기획에서 정한다(지금은 큰 구조만).
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 8(페이지 5절 거위 미결)
- 걸린 것: 백로그 #1 몬스터 기획자(기획)
- 후보였던 것:
  - ① A: 맞으면 즉시 운다.
  - ② B: 죽는 소리가 더 넓게 퍼진다.
  - ③ A와 B 둘 다.
  - 단서: 놀람 낙하를 거위에만 둘지 광폭화 닭에도 둘지, 빛에 깨는 규칙과 시선 보류 원칙의 충돌을 어떻게 풀지는 원문에 후보가 따로 없다.

## 2026-10-18 (자율 루프 결정 #9)

- **질문**: 닭의 빛 반응, 죽음 드롭, 빈 것의 속도·시야·치명을 어떻게 할까요?
- **답(사용자 원문)**: 몬스터 패턴 기획에서 정한다(지금은 큰 구조만).
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 9(페이지 5절 닭 미결)
- 걸린 것: 백로그 #1 몬스터 기획자, #2 아이템·경제 기획자(기획)
- 후보였던 것:
  - ① 빛에 끌린다.
  - ② 빛에 멈춘다.
  - ③ 죽으면 드롭(깃털·날고기)을 남긴다 — 원문은 이 선택을 "치는 이유"와 "보이는 대로 죽인다"의 맞섬으로 적었다.
  - ④ 죽음 드롭을 두지 않는다.
  - 단서: 빈 것의 속도·시야·치명은 원문에 후보가 따로 없다.

## 2026-10-18 (자율 루프 결정 #10)

- **질문**: 거위와 너구리를 재배치(사라졌다 나타남)할까요?
- **답(사용자 원문)**: 몬스터 패턴 기획에서 정한다.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 10(페이지 원칙 4)
- 걸린 것: 백로그 #1 몬스터 기획자, #3 맵 기획자(기획)
- 후보였던 것:
  - ① 재배치한다 — 전투 세션 카드에 "60~120초 뒤 재등장" 제안이 있으나, 결정 ④ 이전에 나온 것이다.
  - ② 재배치하지 않고 사라졌다 나타나는 역할을 굴로 드나드는 동물에게 맡긴다 — 결정 ④와 원칙 4를 따른다.

## 2026-10-18 (자율 루프 결정 #12)

- **질문**: 광폭화를 공포의 중심으로 둘까요, 닭·돼지의 장치로 둘까요?
- **답(사용자 원문)**: 광폭화는 닭·벌·소처럼 자기 집이 있는 종에 둔다. 쥐처럼 모든 맵에 나와도 이상하지 않은 종은 없어도 된다. 초안이라고 따라가는 게 아니다.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 12(상충 10)
- 걸린 것: 백로그 #1 몬스터 기획자(기획)
- 후보였던 것:
  - ① 공포의 중심으로 둔다(인스타 원칙 1).
  - ② 닭·돼지의 장치로 둔다.
  - 단서: 페이지는 닭과 돼지 둘 다에 Burst·Hollow 상태를 둔다(4단계 "정체").

## 2026-10-18 (자율 루프 결정 #20)

- **질문**: 몬스터 명단 23종을 몬스터 초안 1.1로 확정할까(맵 가명단의 곰·까마귀·수달·고양이는 예비)?
- **답(사용자 원문)**: 몬스터 초안 1.1 그대로 23종으로 확정. 맵 가명단의 곰·까마귀·수달·고양이는 다음 동네 예비.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드(명세)
- 후보였던 것:
  - ① 몬스터 초안 그대로
  - ② 넷을 더해 27종

## 2026-10-18 (자율 루프 결정 #21)

- **질문**: 종마다 층(동물·몬스터)과 스폰 줄(가축 줄/위협 풀)을 어떻게 나눌까?
- **답(사용자 원문)**: 다음 기획(몬스터 패턴 세부 기획) 결과에 따라 정한다. 지금 초안은 무시해도 된다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드(명세)
- 후보였던 것:
  - ① 몬스터 초안대로
  - ② 맵 초안대로
  - ③ 둘을 합침

## 2026-10-18 (자율 루프 결정 #22)

- **질문**: 닭 계열(어미 닭·횃대 닭·병아리)도 처치가 답일 수 있는 종인가?
- **답(사용자 원문)**: 몬스터 패턴 기획에서 정한다. 지금은 큰 구조만.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드(명세)
- 후보였던 것:
  - ① 든다
  - ② 닭만

## 2026-10-18 (자율 루프 결정 #23)

- **질문**: 스폰 표 방식과 한 판의 몬스터 층 수는?
- **답(사용자 원문)**: 다음 기획 결과에 따른다. 리썰식(희귀도·파워 예산·상한·자리 조건)이 좋으면 그대로 따라가도 된다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 리썰식(희귀도·파워 예산·상한·자리 조건)
  - ② 지금 구조 + 확률 열
  - ③ 종마다 0~1

## 2026-10-18 (자율 루프 결정 #24)

- **질문**: 탈 쓴 주인을 장소에 묶는 방식은?
- **답(사용자 원문)**: 다음 기획 결과에 따른다. 초안은 무시해도 된다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 광폭화 조건 소품만(열 없음)
  - ② 사이트 행 주인 열
  - ③ 스폰 줄 탈 여부 열

## 2026-10-18 (자율 루프 결정 #25)

- **질문**: 결정 ②(넘어지면 0~2개, 종별로 나누지 않는다)의 범위 — 어미 닭 알 전부·소 짐 전부·거위 놀람 1개를 어떻게 적나?
- **답(사용자 원문)**: 모두 0~2개로 통일한다(종 고유 짐 결과 없음).
- 답한 날짜: 2026-10-18
- 출처: 백로그 #4 판 루프 기획자 — 한 판 타임라인·긴장 조절자·사망과 관전·트럭 안전지대 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #4 판 루프 기획자 — 한 판 타임라인·긴장 조절자·사망과 관전·트럭 안전지대(명세)
- 후보였던 것:
  - ① 두 층(공통 + 종 고유)
  - ② 모두 0~2개로 통일
  - ③ 넘어짐 아닌 사건에만 종 고유

## 2026-10-18 (자율 루프 결정 #26)

- **질문**: 빈 것(광폭화한 것)에게 잡히면 무엇을 잃나?
- **답(사용자 원문)**: 몬스터마다 다르다. 하나로 통합하지 않고 몬스터 패턴 기획에서 종별로 정한다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드(명세)
- 후보였던 것:
  - ① 즉사
  - ② 짐 전부 + 넘어짐

## 2026-10-18 (자율 루프 결정 #27)

- **질문**: 소를 두 단계(탈 쓴 소 비치명 / 터진 소 즉사)로 나눌까?
- **답(사용자 원문)**: 몬스터 패턴 기획에서 정한다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드(명세)
- 후보였던 것:
  - ① 두 단계
  - ② 처음부터 즉사

## 2026-10-18 (자율 루프 결정 #28)

- **질문**: 실외 종(눈먼 개·여우·허수아비)을 어디에 둘까? 마당 무스폰 결정(2026-10-11)과 부딪친다
- **답(사용자 원문)**: 마당도 위험 구역이다(2026-10-11 마당 무스폰 번복). 마당 몬스터는 적게. 건물 밖에서도 긴장을 유지한다. 밖에서 나오는 몬스터와 안에서 나오는 몬스터는 근본적으로 달라야 한다(실내·실외 분리).
- 답한 날짜: 2026-10-18
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 그 종만 마당 예외
  - ② 바깥채 실내 방
  - ③ 마당 무스폰을 푼다

## 2026-10-18 (자율 루프 결정 #29)

- **질문**: 양봉의 판 형태와 정전은?
- **답(사용자 원문)**: 맵 기획에서 정한다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 망실 양봉장(실내) + 정전 있음
  - ② 실외 예외 + 정전 없음

## 2026-10-18 (자율 루프 결정 #30)

- **질문**: 판 중간 등장이 뽑는 종·자리·예고 — 구멍 스폰을 7e와 다른 길(예고 있음)로 둘까?
- **답(사용자 원문)**: 긴장 리듬에 묶고 세부는 판 루프 기획에서. 리썰 벤트식(구멍에서, 소리 예고, 어느 구멍인지는 모름)이 좋다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #4 판 루프 기획자 — 한 판 타임라인·긴장 조절자·사망과 관전·트럭 안전지대 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #4 판 루프 기획자 — 한 판 타임라인·긴장 조절자·사망과 관전·트럭 안전지대(명세)
- 후보였던 것:
  - ① 구멍에서 예고 있게(리썰 벤트식)
  - ② 지금대로 시야 밖·예고 없음
  - ③ 긴장 리듬에 묶음

## 2026-10-18 (자율 루프 결정 #31)

- **질문**: 정전 중 몬스터 시야를 누구에게 어떤 값 형태로 적용할까(빛 든 사람 예외 여부 포함)?
- **답(사용자 원문)**: 정전이어도 종마다 시야가 줄 수도 늘 수도 있다. 종별로 몬스터 패턴 기획에서 정한다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드(명세)
- 후보였던 것:
  - ① 모두 줄인다
  - ② 빛 든 사람 예외
  - ③ 종별로 다르게

## 2026-10-18 (자율 루프 결정 #32)

- **질문**: 새 도구 가운데 이번 범위에 무엇을 넣을까?
- **답(사용자 원문)**: 아이템 기획에서 정한다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름(명세)
- 후보였던 것:
  - ① 아이템 초안 추천 순서(무전기·딱총·딸랑이 → 포대·훈연기 → 나머지)
  - ② 전부 넣고 값은 표에서
  - ③ 아이템 기획에서

## 2026-10-18 (자율 루프 결정 #33)

- **질문**: 무리를 갈라놓는 문은 누가 닫고 어떻게 여나?
- **답(사용자 원문)**: 다음 차수(판 루프·맵 기획)에서 정하되 기조는: 누구나 열고 닫을 수 있어야 하지만 열기 위한 조건이 필요하다. 리썰처럼 본진에서 소통 없이 켜고 끄는 식은 안 된다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #4 판 루프 기획자 — 한 판 타임라인·긴장 조절자·사망과 관전·트럭 안전지대 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #4 판 루프 기획자 — 한 판 타임라인·긴장 조절자·사망과 관전·트럭 안전지대(명세)
- 후보였던 것:
  - ① 맵·긴장 조절자만 닫고 시간·레버로
  - ② 몰래 역할도 닫고 열쇠로
  - ③ 누구나 닫고 프라이팬으로

## 2026-10-18 (자율 루프 결정 #34)

- **질문**: 외양간·찬장·양봉은 별도 목적지인가, 한 농장의 구역인가?
- **답(사용자 원문)**: 맵 기획에서 정한다.
- 답한 날짜: 2026-10-18
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · _integration-1.md#4.2
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 별도 목적지
  - ② 대농장 안의 동
  - ③ 몽장의 동으로만

## 2026-10-18 (운영 담당·에디터 자리 줄 — 사용자 결정, 명세 docs/goal/editor-ops.md)

- **데이터 패널 막기 — 2026-10-17 "잠금은 알림일 뿐, 판단은 사용자의 몫" 번복.** 사용자 원문: "이거 조율해야지 앞으로 잠금은 여기내에서 이루워질꺼임".
  패널은 다른 보유자의 배타 잠금(pie·build·compile·commandlet)이 살아 있으면 쓰기·저장을 막고, 쓰기·저장마다 `panel` 임대(5분)를 받는다.
  "그래도 쓰기"는 사용자만 켜고 기록되어 보고서에 오른다. 잠금의 조작은 앞으로 사무실 페이지(운영 담당) 안에서 한다.
- **에디터 자리 줄의 순서 — "제일효율적인방법".** 사용자 손(`user`·`panel`)이 먼저, 그 뒤는 선 순서. 루프는 기다리는 동안 다른 항목을 집을 수 있지만
  사용자는 못 기다린다. `acquire` 가 줄 서기를 겸하고(기존 `/cl9 task`·`/cl9film` 코드는 그대로), 머리가 10분 안에 안 잡으면 다음으로.
  이미 배타 잠금·임대를 쥔 세션은 줄을 서지 않는다.
- **사무실 에디터실 단추 — "할수잇는거다".** CLI 가 할 수 있는 것 전부(잡기·놓기·죽은 잠금 뺏기·가드 끄기/켜기·줄 서기/나가기/정리). 도구의 거부는 그대로 받는다.
- 루프에 **운영 담당**(kind `ops`) 역할을 더한다 — 줄·죽은 잠금 정리·충돌 보고. 사용자 답: "같이 관리"는 둘 다(사무실 페이지 + 역할), "실제로 조율해야되는데 명세로 정리하고 진행하자".

## 2026-10-18 (자율 루프 결정 #3)

- **질문**: 죽은 사람이 쉬는 느낌을 지키면서 할 일을 무엇으로 할까요?
- **답(사용자 원문)**: ② 관전 카메라를 고른다(몬스터 시점·위 시점).
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 3(페이지 결정 ⑨-2, 페이지 11절)
- 걸린 것: 백로그 #4 판 루프 기획자(기획)
- 후보였던 것:
  - ① 관전하며 "누가 살아 돌아올까·누가 몰래 역할인가"를 찍는다 — 맞히면 판 끝에 칭호를 받는다.
  - ② 관전 카메라를 고른다(몬스터 시점·위 시점).
  - ③ 새벽 하이라이트의 주인공이 된다.

## 2026-10-18 (자율 루프 결정 #4)

- **질문**: 몰래 역할을 화면에 어떤 이름으로 부를까요(코드와 내부 문서의 이름은 Imposter로 유지합니다)?
- **답(사용자 원문)**: 동물말고 아예 새로운단어, 임포스터도 새로운단어인것처럼
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 4(페이지 결정 ⑫, 페이지 11·16절)
- 걸린 것: 백로그 #5 메타 기획자, #7 UI 기획자(기획)
- 후보였던 것:
  - ① 늑대 — 권장, "양의 탈을 쓴 늑대"이기 때문이다.
  - ② 족제비(계란 도둑).
  - 단서: 두더지는 결정 ④의 굴 몬스터와 겹쳐서, 여우는 몬스터 후보와 겹쳐서 제외됐다. 사용자는 "이건 모르겠어 다시 고민"이라고 답했다.

## 2026-10-18 (자율 루프 결정 #5)

- **질문**: 공포 게이지에 작은 판정을 걸까요?
- **답(사용자 원문)**: ① 손 떨림으로 조준이 흔들린다 — 축 H의 "표현 전용, 판정에 안 닿음" 전제를 바꾼다.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 5(전투 세션 14:15)
- 걸린 것: 백로그 #6 조작 기획자, #7 UI 기획자(기획)
- 후보였던 것:
  - ① 손 떨림으로 조준이 흔들린다 — 축 H의 "표현 전용, 판정에 안 닿음" 전제를 바꾼다.
  - ② 게이지가 일정 이상이면 스태미나가 빨리 닳는다 — 축 H의 "표현 전용, 판정에 안 닿음" 전제를 바꾼다.
  - ③ 걸지 않는다 — 축 H의 "표현 전용, 판정에 안 닿음" 전제가 그대로 남는다.
  - 단서: MIMESIS의 오염도에서 나온 질문이다.

## 2026-10-18 (자율 루프 결정 #14)

- **질문**: 페이지 결정 ①~⑭를 저장소 문서로 옮기고, 인계 문서 §4·§6의 "구현" 문구를 고칠까요?
- **답(사용자 원문)**: ① 옮긴다 — 대상은 `C:\Work\CL9\docs\decisions.md`, `C:\Work\CL9\docs\plan\data\monster\*.md`, 아트 문서, 출시 계획(`C:\Work\CL9\docs\goal\steam-release.md`)이며, 09-20 예고·회피 보장, 10-04 디스폰 없음, voice.md의 말소리 무시, 2판의 건조한 타격, Steam 업적 범위 밖, 아트 톤 문장이 번복된다.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 14(인스타 세션 14:42)
- 걸린 것: 백로그 #13 문서 옮기기(페이지 결정)
- 후보였던 것:
  - ① 옮긴다 — 대상은 `C:\Work\CL9\docs\decisions.md`, `C:\Work\CL9\docs\plan\data\monster\*.md`, 아트 문서, 출시 계획(`C:\Work\CL9\docs\goal\steam-release.md`)이며, 09-20 예고·회피 보장, 10-04 디스폰 없음, voice.md의 말소리 무시, 2판의 건조한 타격, Steam 업적 범위 밖, 아트 톤 문장이 번복된다.
  - ② 옮기지 않는다 — 결정은 지금처럼 페이지에만 남는다.
  - 단서: 인스타 세션은 "말씀하시면 옮기겠다"고 했다.

## 2026-10-18 (자율 루프 결정 #15)

- **질문**: 몰래 역할이 아닌 동료를 처치했을 때의 "억울한 희생" 칭호를 넣을까요?
- **답(사용자 원문)**: ① 넣는다 — 팀킬을 부추길 수 있다.
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 15(인스타 세션 14:14, 페이지 16절)
- 걸린 것: 백로그 #5 메타 기획자(기획)
- 후보였던 것:
  - ① 넣는다 — 팀킬을 부추길 수 있다.
  - ② 넣지 않는다.

## 2026-10-18 (자율 루프 결정 #16)

- **질문**: "텔"이라는 말을 그대로 쓸까요, "예고"로 바꿀까요?
- **답(사용자 원문)**: 문서에 쓰는거면 예고로
- 답한 날짜: 2026-10-18
- 출처: 0b 미결 16(전투 세션 11:41·13:35)
- 걸린 것: 백로그 #1 몬스터 기획자(규칙 카드의 "예고" 칸, 기획)
- 후보였던 것:
  - ① "예고(텔)"로 바꿔 적는다 — 전투 세션이 페이지에서 이미 이렇게 고쳤다.
  - ② "텔"을 유지한다 — 비교 문서는 아직 "텔"로 적혀 있다.

## 2026-10-18 (자율 루프 결정 #35)

- **질문**: A1. 한 종이 실내·실외 두 목록에 다 들 수 있게 할까요?
- **답(사용자 원문)**: 엄격히 나누는데, 밖으로도 따라나오는 몬스터도 있긴 있어야 돼. 대신 광폭화는 절대 못 나온다. 이건 기획적으로 막아줘. 코드로 막는 게 아니라 기획적으로. 나갈지 말지를 테이블에서 정하면 되니까 — 정리: 후보 ② 엄격히 한 자리 + 종별 "문 밖으로 따라나가나" 표 열, 빈 것(광폭화 뒤)은 그 열이 항상 아니오. (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A1
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 허용한다(주 자리가 아닌 목록은 희귀도를 낮게)
  - ② 엄격히 나눈다(종마다 한 자리)
  - 단서: 통합 추천 ① — 실외 전용 종이 서너 종뿐이라 ②면 목적지 일곱 곳의 마당이 서로 거의 같아진다(3절 1번). 출처 구조 #1(1.1), 새유형 1.2의 원칙, 떠돌이 1.1

## 2026-10-18 (자율 루프 결정 #36)

- **질문**: A2. 실외 종의 공통 성격 넷(보이면 쫓지 않는다, 문턱과 트럭 둘레에 들지 않는다, 1일차 마당에는 없다, 마당에는 한 판 한 종)을 23종의 실외 목록 전체에 걸까요?
- **답(사용자 원문)**: ① 전체에 건다(추천대로). (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A2
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 전체에 건다 — 실외의 벌은 보류하고, 출입문 앞 거위와 마당 벌통도 종 수에 센다
  - ② 새유형 여섯 종에만 건다
  - ③ 구조의 여섯 성격만 쓰고 넷은 권장으로 둔다
  - 단서: 통합 추천 ① — 근거(끊을 벽이 없으면 파훼법이 없다)가 23종 모두에 같고, 종 수에 세지 않으면 "마당 몬스터는 적게"가 표 밖으로 샌다(3절 11번). 출처 통합에서 생김(3절 11번), 새유형 0.2·1.3·1.4, 구조 1.2

## 2026-10-18 (자율 루프 결정 #37)

- **질문**: A3. 건물 문을 실내 종과 실외 종의 경계로 둘까요(실외 종은 문 안으로, 실내 종은 문 밖으로 따라가지 않는다)?
- **답(사용자 원문)**: 특정 종만 넘어가게 — 정리: 양방향 경계가 기본이되 표 열로 지정한 종만 문을 넘는다(#35 답과 같은 열). (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A3
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 양방향 경계
  - ② 실외 종만 막는다
  - ③ 문을 넘어 짧게만 쫓는다
  - ④ 경계 없음
  - 단서: 통합 추천 ① — 두 초안의 추천이 같다. "문을 넘으면 규칙이 바뀐다"를 몸으로 배우는 가장 짧은 길이다. 빈 것만 넘게 할지는 종별로 따로 본다. 출처 구조 #2(1.4), 새유형 N-2(1.3)

## 2026-10-18 (자율 루프 결정 #38)

- **질문**: A4. 스폰 표를 어떤 방식으로 짤까요?
- **답(사용자 원문)**: 리썰식·세 줄(가축 줄·실내 위협 풀·실외 위협 풀)·두 축·쓴 것 기준(추천대로). (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A4
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 방식: 리썰식(희귀도·파워 예산·종 수 상한·구조물 조건) / 지금 구조 + 확률 / 종마다 0~1
  - ② 줄: 가축 줄·실내 위협 풀·실외 위협 풀의 셋 / 위협 풀 하나 + 자리 표시
  - ③ 층과 줄: 한 축(층이 곧 줄) / 두 축(층은 게이지, 줄은 세는 법)
  - ④ 예산 셈: 쓴 것 기준(죽여도 안 돌아온다) / 산 것 기준
  - 단서: 통합 추천 리썰식 · 세 줄 · 두 축 · 쓴 것 기준 — 23종에서 "몇 마리"와 "몇 종"을 따로 쥐어야 하고, 닭 계열이 동물 층이면서 다른 규칙형 위협과 같은 예산에서 뽑혀야 한다(구조 2.1·3.1). 출처 구조 #4·#5·#7(1.6·2.1·3.1·3.2), 주인 1.8

## 2026-10-18 (자율 루프 결정 #39)

- **질문**: A5. 마당의 실외 위협을 어느 행에 적고, 언제 나오게 할까요?
- **답(사용자 원문)**: ③ 합친 안(추천대로). (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A5
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 목적지 행의 실외 풀 + 판 시작 + 지연 뒤 판 시간에 따라 늘어남(구조)
  - ② 마당을 구역 하나로 + 1일차 0 + 종마다 등장 시점(새유형)
  - ③ 합침 — 목적지 행의 실외 풀 + 1일차 0 + 마당 가장자리 구조물에서만 + 종마다 등장 시점, 지연과 늘어남은 판 중간에 나오는 실외 종에만
  - 단서: 통합 추천 ③ — 행은 맵의 깊이 구역 표를 건드리지 않는 구조 쪽이, 시간은 종의 성격에 맞춘 새유형 쪽이 낫다(3절 17번). 출처 구조 #3(1.5), 새유형 N-1(1.4)

## 2026-10-18 (자율 루프 결정 #40)

- **질문**: A6. 탈 쓴 주인을 장소에 어떻게 묶을까요(결정 함 #24 관련)?
- **답(사용자 원문)**: ② 사이트 행의 "이 목적지의 주인" 목록 한 칸(추천대로). 결정 함 #24("다음 기획 결과에 따른다")의 그 결과가 이 답이다. (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A6
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 광폭화 조건 소품만으로
  - ② 사이트 행에 "이 목적지의 주인" 목록 한 칸
  - ③ 스폰 줄마다 탈 여부 표시
  - 단서: 통합 추천 ② — ①은 다람쥐의 셈 조건·여러 목적지가 나눠 쓰는 구조물·계란 깨지는 소리에서 새고, ①을 추천한 주인 초안 스스로 다른 마당에 벌통을 두자고 했다(3절 14번). 출처 구조 #8(4.2), 주인 #24(1.3) — 초안끼리 갈림

## 2026-10-18 (자율 루프 결정 #41)

- **질문**: A7. 결정 #25 "모두 0~2개"의 범위에, 바닥에 떨어진 뒤의 일(어미 닭이 흘린 알을 둥지로 물어 감, 쥐가 바닥의 것을 가져감, 소·두더지의 흩어지는 거리)도 드나요?
- **답(사용자 원문)**: 행선지도 공통 — 정리: 후보 ②, 항상 발밑에 떨어지고 아무도 가져가지 않는다. 어미 닭은 쪼아 붙잡기로, 소·두더지는 날림만 남고, 쥐는 바닥 물어 가기 규칙을 잃는다(쥐 규칙은 몬스터 기획 재조정에서 다시). (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A7
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 들지 않는다 — 몇 개를 잃는지만 공통이고, 바닥에 떨어진 뒤의 일은 종이 정한다
  - ② 든다 — 행선지와 흩어짐도 공통(발밑)으로 둔다. 어미 닭은 쪼아 붙잡기로, 소·두더지는 날림만 남고, 쥐는 규칙을 잃는다
  - 단서: 통합 추천 ① — 결정의 글자는 개수이고, ②로 넓히면 쥐가 규칙을 잃는다. 다만 사용자가 결정을 "짐에 관한 종별 차이를 두지 않는다"로 넓게 뜻했다면 ②다(3절 10번). 출처 구조 #9(5.2), 주인 Q-A(1.2), 새유형 1.1, 떠돌이 1.2

## 2026-10-18 (자율 루프 결정 #42)

- **질문**: A8. 빈 것의 값을 따로 담은 행을 두고 원래 행이 그것을 가리키게 하는 방식이, 10-13 결정 "그냥 동물과 빈 것은 행이 하나"와 어긋나나요?
- **답(사용자 원문)**: 어긋남, 같은 행에 열로 — 정리: 후보 ②, 빈 것 값은 같은 행의 별도 열로 둔다. (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A8
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 어긋나지 않는다(개체와 스폰 줄이 하나이면 된다) — 가리키는 행 방식
  - ② 어긋난다 — 같은 행에 빈 것용 열을 따로 둔다
  - 단서: 통합 추천 ① — 두 초안의 추천이 같다. 집 주인이 다섯(과 닭 계열)이라 ②는 열이 종 수만큼 늘고 회피 산수 검사를 두 번 써야 한다. 구조 초안이 사용자 확인을 요청했다. 출처 구조 #12(7.3), 주인 Q-E(1.3)

## 2026-10-18 (자율 루프 결정 #43)

- **질문**: A9. 판 중간에 누가, 어떤 예산으로, 어떤 예고로 나오나요?
- **답(사용자 원문)**: 추천 안 — 구멍에 맞는 종만(집 주인은 닭만) · 예산은 판 시작과 나눠 씀 · 예고는 진짜 하나 + 가짜 둘 · 예고음은 종마다 · 쥐도 같은 길. (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A9
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 누가: 구멍에 몸이 맞는 종만 / 모든 실내 위협 / 가축 줄 중심 / 집 주인은 닭·돼지만
  - ② 예산: 구역 예산 하나를 판 시작과 나눠 씀 / 따로 셈
  - ③ 예고: 구역 전체에서 / 진짜 하나 + 가짜 둘 / 진짜 구멍에서만
  - ④ 예고음: 구멍 종류마다 / 종마다
  - ⑤ 쥐: 같은 길의 한 종 / 별도 경로
  - 단서: 통합 추천 구멍에 맞는 종만(집 주인은 닭만) · 나눠 씀 · 진짜 하나 + 가짜 둘 · 종마다 · 같은 길(쥐의 간격은 쥐 행의 값으로) — 원칙 ④와 ③을 함께 지킨다(3절 12번). 출처 구조 #13(8.2·8.3), 주인 Q-K(1.8), 떠돌이 쥐-5(5절)·1.8

## 2026-10-18 (자율 루프 결정 #44)

- **질문**: A10. 큰 목소리 판정을 거위 밖으로, 곧 토끼를 쫓는 소리로 넓힐까요? ("몬스터는 말소리를 듣지 않는다"의 두 번째 번복입니다)
- **답(사용자 원문)**: ① 넓힌다(추천대로). (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-18)
- 답한 날짜: 2026-10-18
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 명세 · _integration-2.md#6.1 A10
- 걸린 것: 백로그 #1 몬스터 기획자(명세) — A 묶음이 끝나야 #2~#5 재조정
- 후보였던 것:
  - ① 넓힌다 — 같은 비명이 토끼는 쫓고 거위는 깨운다
  - ② 거위에만 둔다 — 토끼는 물건이 내는 소리로만 쫓는다
  - 단서: 통합 추천 — 큰 목소리 판정이 거위 때문에 들어온다면 ①, 들어오지 않는다면 ② — 새 복제는 거위가 이미 쓰는 길 하나로 늘지 않고, 마이크가 없는 사람에게는 물건 소리가 남는다. 출처 새유형 N-9(5절 14칸)

## 2026-10-19 (자율 루프 결정 #45)

- **질문**: A1(2차). 결정 #41로 "바닥의 것을 물어 간다"를 잃은 쥐에게 어떤 규칙을 줄까요?
- **답(사용자 원문)**: (가) 쥐 떼 몰이(추천대로) (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 기획 · _integration-4.md#4.1 A1
- 걸린 것: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드(기획)
- 후보였던 것:
  - ① (가) 쥐 떼 몰이 — 벽 속 긁는 소리가 나면 구멍 앞 길에서 비켜라 / (나) 바닥 재료 경보 / (다) 발목 물기

## 2026-10-19 (자율 루프 결정 #46)

- **질문**: A2(2차). 몰래 역할을 화면에서 어떤 이름으로 부를까요?
- **답(사용자 원문)**: 일단 머큐리라고 하고 진행하자 (임시 이름, 확정 아님) (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #5 메타 기획자 — 몰래 역할 규칙 보강·칭호와 Steam 업적 표·인벤토리 규칙·저장 · 기획 · _integration-4.md#4.1 A2
- 걸린 것: 백로그 #5 메타 기획자 — 몰래 역할 규칙 보강·칭호와 Steam 업적 표·인벤토리 규칙·저장(기획)
- 후보였던 것:
  - ① 꿍꿍이 / 딴짓꾼 / 파토꾼 / 엇박이 / 슬쩍이 / 몰래꾼 / 삐끗이 / 쉿쉿이

## 2026-10-19 (자율 루프 결정 #47)

- **질문**: A3(2차). 몰래 역할이 비밀 임무를 이뤘을 때 이더를 줄까요?
- **답(사용자 원문)**: ① 임무마다 5이더 (값은 표에서) (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #5 메타 기획자 — 몰래 역할 규칙 보강·칭호와 Steam 업적 표·인벤토리 규칙·저장 · 기획 · _integration-4.md#4.1 A3
- 걸린 것: 백로그 #5 메타 기획자 — 몰래 역할 규칙 보강·칭호와 Steam 업적 표·인벤토리 규칙·저장(기획)
- 후보였던 것:
  - ① 임무마다 5이더 / ② 이더 없이 칭호와 새벽 공개만

## 2026-10-19 (자율 루프 결정 #48)

- **질문**: A4(2차). 아트 작게 안(세 목적지·8종)은 첫 출시 범위인가요, 첫 제작 묶음인가요? 빈 것 메시는 몇 종?
- **답(사용자 원문)**: 일단 필수 몬스터 10종만 먼저 하고 가자, 맵도 하나만 먼저 만들어 보고 가자. 재미있는지 먼저 보고 확장해야지. → 10종 = 작은 안 8(닭·어미 닭·횃대 닭·돼지·거위·너구리·양·토끼) + 쥐 + 소(통합 추천 선택). 맵 하나 = 양계장. 출시 범위 아님, 재미 확인용 첫 묶음 (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보 · 기획 · _integration-4.md#4.1 A4
- 걸린 것: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보(기획)
- 후보였던 것:
  - ① 첫 제작 묶음 / ② 처음부터 크게 / ③ 작게+외양간·소 / ④ 작게를 출시 범위로

## 2026-10-19 (자율 루프 결정 #49)

- **질문**: A5(2차). 게임의 닭은 무엇으로 할까요?
- **답(사용자 원문)**: 닭·소는 괴물 유지(②). 스토리 보면 괴물로 할 동물이 몇 개 더 있는데 걔네까지 다 괴물 만들어야 돼 → 집 주인 다섯(닭·돼지·소·다람쥐·벌) 전부 괴물(빈 것) 제작. 빈 것은 닭만이 아님 (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보 · 기획 · _integration-4.md#4.1 A5
- 걸린 것: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보(기획)
- 후보였던 것:
  - ① 지금의 chicken_a를 두고 빈 닭 메시를 같은 스켈레톤에 / ② 외형이 승인된 닭 탈 괴물(chicken_disguise)로 바꾼다

## 2026-10-19 (자율 루프 결정 #50)

- **질문**: A6(2차). 톤 문장을 컨셉 승인 기준으로 쓸까요, 쓴다면 무엇을 볼까요?
- **답(사용자 원문)**: ① 쓴다, 기준 넷(추천대로) (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보 · 기획 · _integration-4.md#4.1 A6
- 걸린 것: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보(기획)
- 후보였던 것:
  - ① 쓴다 — 기준 넷(평시 귀여움·무서움은 정체 순간에만·정체는 같은 몸의 뒤틀림·평상 모델에 이음매 한 곳) / ② 쓰지 않는다

## 2026-10-19 (자율 루프 결정 #51)

- **질문**: A7(2차). 이야기의 틀을 "동물 가죽"에서 "겉(동물·물건·사람 모습)"으로 넓힐까요?
- **답(사용자 원문)**: 가. 넓힌다(추천대로) (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #9 스토리 기획자 — 새 종·새 장소의 사연과 농부의 기록·세 층 구조와 이야기의 정합 · 기획 · _integration-4.md#4.1 A7
- 걸린 것: 백로그 #9 스토리 기획자 — 새 종·새 장소의 사연과 농부의 기록·세 층 구조와 이야기의 정합(기획)
- 후보였던 것:
  - ① 가. 넓힌다(시계·허수아비·플레이어 탈을 '빈 것이 그 겉을 입었다'로) / 나. 넓히지 않는다

## 2026-10-19 (자율 루프 결정 #52)

- **질문**: A8(2차). 탈 없는 몬스터 12종에게 게임 안 읽을거리(쪽지)를 허용할까요?
- **답(사용자 원문)**: 괴물(탈 있는 종) 빼고는 스토리 없어도 될 듯 → 가. 허용하지 않는다. 탈 없는 종에는 사연·쪽지 없음 (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #9 스토리 기획자 — 새 종·새 장소의 사연과 농부의 기록·세 층 구조와 이야기의 정합 · 기획 · _integration-4.md#4.1 A8
- 걸린 것: 백로그 #9 스토리 기획자 — 새 종·새 장소의 사연과 농부의 기록·세 층 구조와 이야기의 정합(기획)
- 후보였던 것:
  - ① 가. 허용하지 않는다 / 나. 습성·대응만 적은 쪽지를 허용한다

## 2026-10-19 (자율 루프 결정 #55)

- **질문**: A11(2차). 만들 소리 221개(음악 3곡 교체 포함)를 무엇으로 만들까요?
- **답(사용자 원문)**: 소리 221개가 맞는지 먼저 확인 / 동일한 소리 써도 되는지 / 무료 사운드 사이트 뒤져서 쓸 거 있는지 확인 필요 → 답 보류, 그 확인부터(시퀀스·사운드 기획자 몫) (사용자가 통합 정리 세션 질문 창에 답함, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #8 시퀀스·사운드 기획자 — 장면 표·종별 발소리와 예고음·공포 단계 음악·환경음 · 기획 · _integration-4.md#4.1 A11
- 걸린 것: 백로그 #8 시퀀스·사운드 기획자 — 장면 표·종별 발소리와 예고음·공포 단계 음악·환경음(기획)
- 후보였던 것:
  - ① 가. ElevenLabs 생성 / 나. 상용 라이브러리 / 다. 직접 녹음

## 2026-10-19 (자율 루프 결정 #59)

- **질문**: 제작 1묶음의 기존 모델(돼지 pig_a·양 sheep_a)을 그대로 쓸까요, 새 규격(톤 기준 넷·괴물 구조)에 맞춰 다시 만들까요?
- **답(사용자 원문)**: 지금 만들어진 돼지랑 양도 다시 규격에 맞게 만들어야 되긴 해 → ② 재제작 (사용자, 통합 정리 세션, 2026-10-19). 거위·너구리는 언급 없음
- 답한 날짜: 2026-10-19
- 출처: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보 · 기획 · production-batch-1.md 1절
- 걸린 것: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보(기획)
- 후보였던 것:
  - ① 그대로 + 귀여움 시안 v2 / ② 재제작

## 2026-10-19 (자율 루프 결정 #53)

- **질문**: A9(2차). 몽장의 마지막 기록 자리와, 다섯 집의 기록을 다 모으면 트럭에서 바뀌는 것은?
- **답(사용자 원문)**: 음
- 답한 날짜: 2026-10-19
- 출처: 백로그 #9 스토리 기획자 — 새 종·새 장소의 사연과 농부의 기록·세 층 구조와 이야기의 정합 · 기획 · _integration-4.md#4.1 A9
- 걸린 것: 백로그 #9 스토리 기획자 — 새 종·새 장소의 사연과 농부의 기록·세 층 구조와 이야기의 정합(기획)
- 후보였던 것:
  - ① 자리 가. 먼 동 막다른 방 / 나. 본채 작은 동 · 바뀌는 것 가. 라디오 한 줄 / 나. 벽 사진

## 2026-10-19 (자율 루프 결정 #54)

- **질문**: A10(2차). 장면 117개를 무엇으로 만들까요?
- **답(사용자 원문)**: 다른세션에서 물어본거같은데, 시간함수랑 시퀀서 이게뭐야?
- 답한 날짜: 2026-10-19
- 출처: 백로그 #8 시퀀스·사운드 기획자 — 장면 표·종별 발소리와 예고음·공포 단계 음악·환경음 · 기획 · _integration-4.md#4.1 A10
- 걸린 것: 백로그 #8 시퀀스·사운드 기획자 — 장면 표·종별 발소리와 예고음·공포 단계 음악·환경음(기획)
- 후보였던 것:
  - ① 가. C++ 시간 함수 / 나. 판 밖 장면만 시퀀서 / 다. 전부 시퀀서

## 2026-10-19 (자율 루프 결정 #56)

- **질문**: A12(2차). 음악을 무엇이 움직일까요?
- **답(사용자 원문)**: 질문이 이상한디?
- 답한 날짜: 2026-10-19
- 출처: 백로그 #8 시퀀스·사운드 기획자 — 장면 표·종별 발소리와 예고음·공포 단계 음악·환경음 · 기획 · _integration-4.md#4.1 A12
- 걸린 것: 백로그 #8 시퀀스·사운드 기획자 — 장면 표·종별 발소리와 예고음·공포 단계 음악·환경음(기획)
- 후보였던 것:
  - ① 가. 판 단계 + 각자 머신의 공포 게이지 / 나. 판 단계 + 호스트의 긴장 조절자

## 2026-10-19 (자율 루프 결정 #57)

- **질문**: A13(2차). 방 설정(큰 목소리 반응·손 떨림·미니맵·이름표)을 어느 화면에서 바꿀까요?
- **답(사용자 원문)**: 미니맵/이름표는 우리 개발할떄만쓸꺼고 라이브떈 뺼꺼 / 저런거 옵션따로 방설정에 없어도돼
- 답한 날짜: 2026-10-19
- 출처: 백로그 #7 UI 기획자 — HUD·몬스터 체력 눈금·피격 표시·공포 화면 효과·업적 자리·설정·문구 · 기획 · _integration-4.md#4.1 A13
- 걸린 것: 백로그 #7 UI 기획자 — HUD·몬스터 체력 눈금·피격 표시·공포 화면 효과·업적 자리·설정·문구(기획)
- 후보였던 것:
  - ① 가. 설정 화면 맨 아래 '방 (호스트)' 섹션 / 나. 방 만들기 흐름의 새 단계

## 2026-10-19 (자율 루프 결정 #58)

- **질문**: A14(2차). 무전기는 언제 보낼까요?
- **답(사용자 원문)**: ① 가. 켜져 있으면 어디 있든 / 나. 손에 든 켜진 무전기만 보내고 주머니의 것은 받기만
- 답한 날짜: 2026-10-19
- 출처: 백로그 #6 조작 기획자 — 공격·줍기·놓기·던지기 입력·손전등 조작·퀵슬롯·이동·캐릭터 표현 · 기획 · _integration-4.md#4.1 A14
- 걸린 것: 백로그 #6 조작 기획자 — 공격·줍기·놓기·던지기 입력·손전등 조작·퀵슬롯·이동·캐릭터 표현(기획)
- 후보였던 것:
  - ① 가. 켜져 있으면 어디 있든 / 나. 손에 든 켜진 무전기만 보내고 주머니의 것은 받기만

## 2026-10-19 (자율 루프 결정 #60)

- **질문**: 귀여움 1단계 시안 넷(거위·돼지·너구리·양 v2, 2026-09-29~30)을 쓸까요?
- **답(사용자 원문)**: 이미지는 저거 안 쓰고 새로 시안 그려달라고 할 거임 → ② 안 쓴다. 컨셉은 ChatGPT 시트(ArtSource\monster\concept_batch_1\prompt.txt·prompt2.txt)로 새로 (사용자, 통합 정리 세션, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보 · 기획 · production-batch-1.md 3절
- 걸린 것: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보(기획)
- 후보였던 것:
  - ① 승인해서 쓴다 / ② 안 쓰고 새 시안을 그린다

## 2026-10-19 (자율 루프 결정 #61)

- **질문**: 제작 1묶음의 진행 순서는?
- **답(사용자 원문)**: 모든 몬스터 시안 먼저 다 그려 주고, 시안 완성되면 그때부터 몬스터 만들기 시작. 만들기 전에 무료 에셋이나 이런 거 찾아보고 있으면 가져와서 수정하든 새로 만들든 → ② (사용자, 통합 정리 세션, 2026-10-19)
- 답한 날짜: 2026-10-19
- 출처: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보 · 기획 · production-batch-1.md 2절
- 걸린 것: 백로그 #10 아트 기획자 — 제작 목록 표·범위 제안 두 가지·종별 제작 카드 초안·컨셉 후보(기획)
- 후보였던 것:
  - ① 종마다 컨셉→모델을 바로 잇는다 / ② 10종 시안을 전부 먼저 승인한 뒤 제작, 종마다 무료 에셋 조사 먼저

## 2026-10-18 (자율 루프 — 기획 초안의 수치 규칙)

- **기획 초안에서는 숫자를 그대로 써도 된다(매직넘버 허용). 표 열로 옮기는 것은 명세·구현 단계에서 확인한다 (사용자 "초안에는 수치를 매직넘버로 써도 되는데, 실제 개발할 때는 테이블화해야 된다").** 문지기의 기획 단계 검사 "수치가 전부 표의 열 이름과 값으로 적혀 있나"·"모든 수치에 근거가 붙어 있나" 두 줄을 뺐고(`C:\Work\CL9\.agent-office\gate_checks.csv`, 기본표 `C:\Work\Tools\AgentOffice\loop\gate_checks.default.csv`), 명세 단계 검사를 "튜닝 수치가 표 열 이름·단위로 적혀 있나(기획 초안의 숫자는 표 열로 옮기고 초안 값으로 둔다)"로 고쳤다. 구현 단계의 "튜닝 수치가 코드 상수로 들어가지 않았나"는 그대로. 숫자만 정하면 되는 것은 사장 결정 함에 올리지 않고 산출물의 "밸런스 몫" 절로 모은다(`/office` 스킬 brief 공통 문구). 분할안 `docs/review/planning-roles-2026-10-18.md` 공통 전제의 "초안이라도 수치는 전부 표의 열 이름과 값으로"를 번복한다. 이 날 #1·#2·#3 기획 초안의 반려 대부분이 이 두 검사였다.

## 2026-10-19 (자율 루프 결정 #62)

- **질문**: 조명탄을 넣을까요? 넣으려면 2026-09-11 '발광 아이템 폐기' 결정을 번복해야 합니다.
- **답(사용자 원문)**: ① 넣지 않는다(결정 유지, 추천 — 놓는 빛이 듣는 소·벌은 스탠드 램프로 풀린다. 명세의 기본값) / ② 결정을 번복하고 1회용 조명탄을 둔다
- 답한 날짜: 2026-10-19
- 출처: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름 · 명세 · 백로그 #2 아이템·경제 · 명세 · C:\Work\CL9\docs\goal\item-economy.md Open Questions 1 · 초안 7.2 A2
- 걸린 것: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름(명세)
- 후보였던 것:
  - ① 넣지 않는다(결정 유지, 추천 — 놓는 빛이 듣는 소·벌은 스탠드 램프로 풀린다. 명세의 기본값) / ② 결정을 번복하고 1회용 조명탄을 둔다
- **미결**: 답에 두 안이 함께 들어 있어 고른 쪽을 알 수 없다 — 결정 함 #70 으로 다시 물었다(2026-10-19 조율자).

## 2026-10-19 (자율 루프 결정 #63)

- **질문**: 무전기를 어떻게 이을까요? 2026-10-11 핫라인 결정(한 사람에 하나, 둘씩 짝)과 걸립니다(voice.md 열린 질문 2).
- **답(사용자 원문)**: 1번 하나로 묶어야지
- 답한 날짜: 2026-10-19
- 출처: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름 · 명세 · 백로그 #2 아이템·경제 · 명세 · C:\Work\CL9\docs\goal\item-economy.md Open Questions 2 · 초안 7.2 A3
- 걸린 것: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름(명세)
- 후보였던 것:
  - ① 켜진 무전기를 모두 한 채널로 묶는다(번복, 아이템 쪽 추천 — 플레이어 탈의 준비물이 무전기라 4인 모두에게 판정이 선다) / ② 지금처럼 둘씩 짝으로 둔다(결정 유지, 명세의 기본값)

## 2026-10-19 (자율 루프 결정 #64)

- **질문**: 제작량 합계를 어떤 꼴로 낼까요? (다섯 역할 초안 형식이 함께 바뀝니다)
- **답(사용자 원문)**: 번뭔말이닞모르겠음
- 답한 날짜: 2026-10-19
- 출처: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름 · 명세 · 백로그 #2 아이템·경제 · 명세 · C:\Work\CL9\docs\goal\item-economy.md Open Questions 3 · 초안 7.2 A1
- 걸린 것: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름(명세)
- 후보였던 것:
  - ① 아트 기획이 처음 합친다 / ② 역할마다 같은 칸(메시·클립·소리·아이콘)으로 적는다 / ③ ②에 단계(먼저·다섯 집 전부·나중)를 더한다(추천 — 합계와 무엇부터 만드나가 함께 나온다)
- **미결**: 답은 "질문을 알아듣지 못했다"는 뜻이다 — 결정 함 #71 로 쉽게 고쳐 다시 물었다(2026-10-19 조율자).

## 2026-10-19 (자율 루프 결정 #65)

- **질문**: 아이템·경제 goal의 구현 범위를 결정 #48의 첫 묶음(양계장 + 10종)에 맞출까요?
- **답(사용자 원문)**: 단계1
- 답한 날짜: 2026-10-19
- 출처: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름 · 명세 · 백로그 #2 아이템·경제 · 명세 · C:\Work\CL9\docs\goal\item-economy.md Open Questions 4 · 결정 #48
- 걸린 것: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름(명세)
- 후보였던 것:
  - ① 단계 1(양계장)만 이 goal에 두고, 단계 2(외양간·찬장·벌집)는 그 집 맵이 설 때 별도 goal로 뗀다(추천 — 맵이 없으면 재료가 스폰될 자리가 없다) / ② 단계 1·2를 모두 이 goal에 둔다 (명세는 지금 단계 1·2로 나눠 적었다)

## 2026-10-20 (자율 루프 결정 #66)

- **질문**: 맵 초안에서 쥐의 새 규칙을 '몬스터 재조정 중'으로 적은 문장들(2.6, 10절 몬스터·아이템 쥐덫)을 결정 #45(쥐 떼 몰이)에 맞춰 다시 쓸까요?
- **답(사용자 원문)**: 1ㅂㅓㄴ
- 답한 날짜: 2026-10-19
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · 백로그 #3 맵 기획 · C:\Work\CL9\docs\plan\draft\map.md 9절 D1
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 다시 쓴다 — RatNest 는 보류에서 삭제로 가고, 구멍 앞 길을 비키는 자리가 맵 규칙으로 드는지 몬스터 기획에 묻는다 / ② 몬스터 기획 재작업(#1)이 끝날 때까지 지금 문장을 둔다

## 2026-10-20 (자율 루프 결정 #67)

- **질문**: 첫 묶음(결정 #48: 양계장 + 10종)의 양계장 판에서 10종 밖인 실외 위협 종(부엉이·두더지·여우·눈먼 개)을 어떻게 할까요?
- **답(사용자 원문)**: 1ㅂㅓㄴ
- 답한 날짜: 2026-10-19
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · 백로그 #3 맵 기획 · C:\Work\CL9\docs\plan\draft\map.md 9절 D2
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 첫 묶음 동안 10종 밖 종을 0으로 둔다 — #48을 글자 그대로 지킨다 / 마당에 나올 종이 없어 '마당도 위험 구역'(#28·#39)이 첫 묶음에서 서지 않는다 · ② 양계장 마당에 목초지를 두어 양을 마당 종으로 쓴다 — 10종 안에서 마당 위협이 선다 / 양계장 이야기에 없는 목초지와 울타리 구조물이 첫 묶음에 더해진다 · ③ 실외 한 종을 첫 묶음에 더한다 — 마당이 설계대로 선다 / #48 '10종'을 넓히는 번복이다

## 2026-10-20 (자율 루프 결정 #68)

- **질문**: 양계장 마당 개집의 '끊어진 목줄'을 기록 소품(Record)으로 둘까요? 눈먼 개는 탈 없는 종이라 결정 #52(탈 없는 종엔 사연·쪽지 없음)와 걸립니다.
- **답(사용자 원문)**: 알아서해
- 답한 날짜: 2026-10-19
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · 백로그 #3 맵 기획 · C:\Work\CL9\docs\plan\draft\map.md 9절 D3
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 기록에서 빼고 단서 소품으로만 둔다 — #52와 맞고 '개가 마당에 있다'는 단서는 남는다 / 개집 쪽 기록 소품이 하나 준다 · ② 기록으로 둔다 — 개집 쪽 이야기 흔적이 남는다 / #52 번복이다

## 2026-10-20 (자율 루프 결정 #69)

- **질문**: 결정 함 #53(몽장의 마지막 기록 자리)에 '음'이라고 답하셨는데, 어느 안을 고르신 건가요?
- **답(사용자 원문)**: 알아서해
- 답한 날짜: 2026-10-19
- 출처: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠 · 명세 · 백로그 #3 맵 기획 · C:\Work\CL9\docs\plan\draft\map.md 9절 D4 · 결정 함 #53
- 걸린 것: 백로그 #3 맵 기획자 — 목적지 컨셉·스폰 표·규칙이 붙는 구조물·깊이별 어둠(명세)
- 후보였던 것:
  - ① 가. 먼 동 막다른 방 · 라디오 한 줄 / ② 나. 본채 작은 동 · 벽 사진 / ③ 메타·스토리 쪽 답을 기다린다 — 각 안이 얻는 것·잃는 것은 map.md 5.6 표

## 2026-10-20 (자율 루프 결정 #70)

- **질문**: 조명탄(던지면 잠깐 빛나는 1회용 도구)을 넣을까요? #62 답에 두 안이 함께 들어와서 어느 쪽을 고르셨는지 알 수 없어 다시 여쭙니다.
- **답(사용자 원문)**: 넣지않는다
- 답한 날짜: 2026-10-19
- 출처: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름 · 명세 · 결정 함 #62 재질문 · C:\Work\CL9\docs\goal\item-economy.md Open Questions 1
- 걸린 것: 백로그 #2 아이템·경제 기획자(명세) — 조명탄이 범위에 드는지
- 후보였던 것:
  - ① 넣지 않는다 — 2026-09-11 결정(빛은 손전등·헤드램프에서만 나온다)을 그대로 지킨다. 빛에 반응하는 소·벌은 스탠드 램프로 상대한다 (권장, 명세의 지금 기본값)
  - ② 넣는다 — 그 결정을 번복하고 1회용 조명탄을 상점에 둔다

## 2026-10-20 (자율 루프 결정 #71)

- **질문**: 새로 만들어야 할 모델·애니메이션·소리·아이콘의 개수를 기획자 다섯 명이 각자 적고 있는데, 이 목록을 어떻게 모을까요? #64 질문이 알아듣기 어려웠다고 하셔서 다시 여쭙니다.
- **답(사용자 원문)**: 모델 알아서만들고있음 신경쓰지마
- 답한 날짜: 2026-10-19
- 출처: 백로그 #2 아이템·경제 기획자 — 공략 도구·상점 품목과 가격·요리·처치 보상과 드롭·이더와 코인의 흐름 · 명세 · 결정 함 #64 재질문 · C:\Work\CL9\docs\goal\item-economy.md Open Questions 3
- 걸린 것: 다섯 역할 초안의 제작 목록 형식 (#2 명세는 이것을 범위 밖으로 두어 막히지 않는다)
- 후보였던 것:
  - ① 아트 기획자가 다섯 초안을 읽고 한곳에 처음 모은다 — 기획자들은 지금처럼 자유롭게 적는다
  - ② 기획자마다 같은 칸(모델·애니메이션·소리·아이콘)으로 개수를 적게 하고, 그 칸을 더하기만 한다
  - ③ ②에 '먼저 만들 것 / 다섯 집이 다 서야 쓸 것 / 나중' 구분을 더한다 — 총 개수와 만드는 순서가 함께 나온다 (권장)

## 2026-10-20 (자율 루프 결정 #72)

- **질문**: #1 몬스터 기획에 '이거 넘기기로했지/'라고 남기셨는데, 무엇을 넘기라는 뜻인가요? (재작업 실행은 01:26 이후 결과 없이 멈춰 있고, 넘기기로 한 결정 기록은 찾지 못했습니다)
- **답(사용자 원문)**: ① #1 재작업을 그만두고 지금 초안 그대로 다음 단계(명세)로 넘긴다
- 답한 날짜: 2026-10-19
- 출처: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드 · 기획 · 사장 지시 2026-10-19 01:54 (#1 기획)
- 걸린 것: 백로그 #1 몬스터 기획자 — 종별 규칙 카드·행동 패턴·광폭화 조짐과 빈 것·새 종 카드(기획)
- 후보였던 것:
  - ① #1 재작업을 그만두고 지금 초안 그대로 다음 단계(명세)로 넘긴다 / ② #1 을 뒤로 미루고 다른 항목부터 한다 / ③ 초안 가운데 일부를 다른 역할에게 넘긴다 — 어느 부분인지 함께 적어 주세요 / ④ 넘기지 않고 재작업을 다시 띄워 끝까지 고친다

## 2026-10-20 (플레이테스트 피드백 반영 — `docs/review/playtest-2026-10-20.md`)

사용자가 직접 플레이한 뒤 피드백 9개를 남기고 "다 해"라고 했다. 방향이 갈리는 셋은 질문 창으로 정했고, 나머지는 기본값으로 정했다.

- **정차 중 건물에 들어가면 판 시작 (5·6·7번)** — 사용자 "정차는 정차인 거고 밖은 밖인 건데, 안에 들어가면 바뀌어야 되는 거 아님?" →
  질문 창에서 "맞다, 들어가면 판 시작". 정차(Lobby) 중 살아 있는 플레이어 누구든 사이트의 실내 모듈(`bOutdoor` 아님)에 들어서면 호스트가
  `StartNight` 를 부른다(`ACL9GameMode::TickLobbyIndoorStart`, 0.25초마다). 레버 출발과 같은 조건이고, 정산일(회사로 가는 날) 로비는
  지금처럼 레버만 출발시킨다. 레버 출발은 그대로 남는다. 2일차부터 "정차 중 건물 안이 밝고, 로비 음악이 나오고, 몬스터·스위치가 없던" 것이 이걸로 풀린다.
  시계 꼬리표 `Hud.LobbyIndoor` " · 건물에 들어가면 시작".
- **파동 속도 전 행 340 → 510 cm/s 되돌림 (8번)** — 착지 음파가 발밑에서 시작해 앞을 보면 0.8초 뒤에야 화면에 들어왔다. 사용자가
  "전체 속도 되돌리기"를 골랐다. 2026-10-15 "퍼지는 것도 느리게"(510 → 340)의 번복이다. 같이 되돌리는 값: `CrestLife` 0.09 → 0.06
  (띠 길이 ≈ 31cm 유지), `StampRiseScale` 1.5 → 1.0(벽 자국이 차오르는 실제 속도 510cm/s 를 사용자가 맞춘 그대로 유지).
- **무기 대기 자세 (3번)** — 사용자가 "뭐가 좋을지 모르겠다"고 맡겼다. 각도만 바꾸면 작은 물건을 받쳐 든 손이 그대로라 무기로 읽히지 않아,
  휘두를 수 있는 물건(고기·프라이팬) 전용 1인칭 대기 자세를 새로 만든다. 새 `ECL9HoldPose::Weapon`, 클립 `AN_Farmer_FPHoldWeapon`.
- **기본 시야각 90 → 98 (9번)** — "화면이 확대된 것 같다, 리썰은 멀리 보는 그림". 리썰 컴퍼니 세로 66°(`PlayerControllerB.targetFOV`)를
  16:9 가로로 옮긴 값이다. `CharacterTable.CameraFov` 열로 뺐다. control-feel C11 의 "기본 FOV 올리기 기각"을 사용자 요청으로 번복한다.
- **아이템 줍기는 눈 기준 (1번)** — 캡슐 중심 120cm 안이던 줍기 범위가 화면 아래 가장자리(앞을 볼 때 바닥이 약 2.8m 앞부터 보인다)보다
  가까워서 바닥 계란을 고개를 푹 숙여야 집었다. 눈에서 `ItemPickReach` 250cm·시선 내적 `ItemPickAimDot` 0.8(≈37°) 안·가린 것 없음 중
  화면 가운데에 가장 가까운 것. 상호작용(스위치·레버·조리대)은 그대로 `PickRadius` 120.
- **내 손에 든 물건 크기 (2·3번)** — 판정 반경에 맞춘 메시(계란 지름 42cm, 고기 60cm)가 손에서 화면을 덮었다. ItemTable `HeldScale` 열
  (내 1인칭 화면만, 바닥·남의 손은 그대로).
- **손전등이 멀리까지 (4번)** — 룩 표 램프 종류에 `FalloffExponent` 열(0 = 물리 감쇠). 손전등 행만 완만한 감쇠로 바꿨다.

## 2026-10-20 몬스터는 종마다 핵심 행동 하나

- **질문 없이 사용자 지시**: "초안에 집착하지말고 지금 모델도 만들고있으니까 몬스터 패턴이나 이런거 기믹정해서 딱하나의 핵심행동패턴만들어서 몬스터 집어넣자."
  초안 `docs/plan/draft/monster*.md`(백로그 #1)의 보조 규칙은 따르지 않는다. 제작 1묶음 10종에서 종마다 규칙 한 줄만 뽑은 표를 보여 주고 확정받았다.
- **닭은 바꿨다** — "쳐도 된다, 하나씩 떼어서"가 마음에 안 든다(사용자, 대안 없음) → 후보 셋(뛰면 덮친다·빛에 몰려든다·줄줄이 따라붙는다) 중
  **빛에 몰려든다**를 골랐다. 닭은 눈·귀 대신 켜진 빛만 느끼고, 바닥에 켜 둔 빛이 미끼다. 맞아 죽은 닭이 무리를 부르는 `KillAlarmRadius` 는 0 으로 끈다.
- **모델 있는 6종만 먼저**(닭·돼지·소·거위·너구리·양). 어미 닭·횃대 닭·토끼·쥐는 모델이 나온 뒤에 넣는다.
- 표와 열: `docs/plan/data/monster/common.md` "핵심 행동". 너구리의 "6초 노려보면 격분"은 뺐다(`StalkStareAnger` 0 = 격분 없음).
- **같은 날 오후 번복(닭)**: 사용자 "닭이 나를봐도 아무런반응이없어" → 질문에 "보면 쫓고, 빛은 더 멀리"를 골랐다. 닭의 눈·귀를 되살리고,
  빛은 `LightLureRadius` 안에서 시야 거리 밖이어도 알아채는 조건으로 바꿨다. 바닥 빛 미끼는 그대로다.

## 2026-10-20 양계장 공급과 정산 목표를 한 식으로 — 재료는 깊이 구역에, 고리는 소켓 변까지

- **사용자 위임**: "너가 알아서 밸런스 짜고 아이템 배치하고 맵도 만들어" — 바로 앞 분석(첫 정산 목표 100 = 세 판 공급의 8%,
  재료가 맵 전체에 고르게 깔려 깊이와 보상이 따로 놂, Flow 맵 고리 판당 0.7개) 뒤의 지시다. 명세 `docs/goal/henhouse-supply-quota.md`.
- **정산 목표를 공급에서 거꾸로**: `QuotaCurve` (3,600)(6,775)(9,950)(12,1150)(30,2290). 양계장 세 판 날재료 값의 0.45 → 0.9배(`ValueCurve` 는 에셋에서 비어 있어 값 배율 1.0으로 계산).
  **2026-10-17 "첫 목표는 100"을 뒤집는다** — 계란 10~15 눈금과 주문 보상(재료 값의 2~3배)을 그대로 두면 주문 두 개만으로 세 판 720이라
  100은 지킬 수 없다. 100을 지키려면 판당 계란을 다섯 개 안팎으로 줄이고 주문 보상도 깎아야 해서 택하지 않았다. 식은 `docs/plan/economy.md` "공급 대비 목표".
- **양계장 재료는 깊이 구역 셋**(entrance 0~0.3 계란 3~4 / middle 0.3~0.65 계란 8~10·고기 2~3 / deep 0.65~1 계란 6~7·고기 6~8), 사이트 `ItemSpawns` 는 비움.
  총량은 예전과 거의 같다. 몬스터 배치는 그대로(결정 함 A4 대기).
- **재료 지점 소진 규칙**: 한 맵의 재료 지점을 다 쓰기 전에는 같은 지점을 다시 쓰지 않는다(리썰 `SpawnScrapInLevel`) — `ACL9GameMode::SpawnNightPopulation`.
- **Flow 고리는 이번에 못 늘렸다**: 빈 문 자리가 이웃의 소켓 변을 보면 문을 내는 완화를 시험했으나 고리가 0.7 → 1.0개뿐이고 "모든 문은 문 자리에" 불변식을 깨서 되돌렸다.
  다음은 리썰 구조 — 모듈 문 자리를 늘리고 본줄기 모듈당 가지 상한(`BranchCount`)을 따로 둔다. 내비 범위(2026-10-14 ⑧)는 이번에 맞췄다.

## 2026-10-20 양계장 몬스터는 판이 흐르며 늘어난다 — 판 시작은 닭 두세 마리

- **사용자**: "처음에는 몹이 없다가 좀 들어가면 들어갈수록 생기는 게 낫겠지? 첫 스테이지는 한 마리/혹은 두 마리 이런 식으로" → 제안(시작은 닭 두세 마리, 위협 종은 유예 뒤 늘림) 뒤 "개발해". 명세 `docs/goal/henhouse-spawn-ramp.md`.
- **사이트 행 열 `MidSpawns`** 추가 — 판 중간 스폰을 시작 스폰과 떼어, 그 굴림(× 일자 배율)이 종·총량이다. 비면 7e 그대로.
- **`CL9Rules`**: `MidSpawnFirstDelay` 40 · `MidSpawnIntervalMin` 15 · `MidSpawnMinDepth` 0.3 신규, `MidSpawnInterval` 60 → 40. 7e 의 "첫 마리는 +간격·고정 간격·깊이 무관"을 바꾼다(모든 사이트). 7e 결정 7 "예고 없이"는 그대로.
- **양계장**: `MonsterSpawns` 닭 2~3, `MidSpawns` 닭 3~4 · 거위 0~1 · 양 1 · 너구리 0~1 · 돼지 1 · 소 0~1.
- 결정 #38(A4, 리썰식 스폰 표 재작성)의 구현은 아니다 — 그 전까지의 중간 단계다.

## 2026-10-20 스폰 표를 리썰식 위협 예산으로 (결정 #38 A4 핵심 구현) — 같은 날 `MidSpawns` 큐를 대체

- **사용자**: "임시값 하지 말고 제대로 세팅해 봐, 게임 한 번 굴려 보는 게 이번 목표" → 범위 질문에 "A4 핵심 구현", 검증 "봇으로 첫 정산까지". 명세 `docs/goal/threat-budget-spawns.md`.
- **표 열**: `MonsterTable` `SpawnPower`·`SpawnMaxCount`, `MapTable.Zones[]` `ThreatPool`·`ThreatBudget`·`ThreatKindsMax`·`ThreatStartShare`(`MonsterSpawns` 는 가축 줄), `CL9Progression.ThreatBudgetCurve`, `CL9Rules` `MidSpawnRampTime` 신규.
  7e 의 `MidSpawnCapRatio`, 같은 날 앞 항목의 사이트 `MidSpawns`·`MidSpawnIntervalMin`·`MidSpawnMinDepth` 는 지웠다. `MidSpawnInterval` 40 → 15(뽑아 보는 간격).
- **값**: 파워는 리썰 실측의 같은 역할(거위·양·소 1, 돼지 2, 너구리 3). 양계장 구역 예산 합 4.0 = 리썰 첫 달 실내 예산 4. 예산 곡선 1일차 0.5 → 5일차 1.0 → 10일차 1.75.
- **초안과 다르게 둔 것**: 돼지·소는 양계장·대농장에서 가축 줄이 아니라 위협 풀(종마다 핵심 행동 하나 이후 둘 다 규칙 하나를 든 종), 양은 실외가 아니라 실내 위협 풀(실외 풀 미구현, 마당 무스폰 7b).
- **넣지 않은 것**: 실외 위협 풀·구조물 조건·무리 크기·사람 수 조건·판 중간 구멍과 예고(A9).

## 2026-10-20 (자율 루프 결정 #73)

- **질문**: ElevenLabs 소리 제작의 첫 묶음을 어디까지로 할까요?
- **답(사용자 원문)**: 이거끝난거아님?
- 답한 날짜: 2026-10-20
- 출처: 백로그 #8 시퀀스·사운드 기획자 — 장면 표·종별 발소리와 예고음·공포 단계 음악·환경음 · 기획
- 걸린 것: 소리 제작 첫 묶음 범위
- 후보였던 것:
  - ① 1. (권장) 코드·SoundTable 행은 있는데 음원이 빈 6개(FearLow·FearHigh1·FearHigh2·Heartbeat·MonsterSighting·MonsterChaseBegin) — 만들자마자 게임에서 들린다
  - ② 2. 1번 + 지금 게임에 있는 몬스터 5종(닭·거위·돼지·너구리·양)의 고유 소리 — 종별 소리 열(TellSoundRow 등) 코드 작업이 같이 필요
  - ③ 3. sequence-sound.md 의 221개 전부 — 23종 대부분이 아직 기획 확정 전(#1 막힘)이라 일부는 버려질 수 있다

## 2026-10-20 (자율 루프 결정 #74)

- **질문**: M1 손전등을 인원수만큼 줄 때, 클라이언트가 늦게 들어오는 문제를 어떻게 셀까요?
- **답(사용자 원문)**: 손전등은 애초에 사는거임. 지금주는건 테스트로 주는거야.
- 답한 날짜: 2026-10-20
- 출처: 백로그 #15 M1 시작 지급 — 로비 짐칸에 손전등 인원수·프라이팬 1(`CL9Rules` 지급 목록 열), 손전등 `EquipmentTable` 행(다시 사기) · 명세
- 걸린 것: #15 명세 → 구현
- 후보였던 것:
  - ① 1. (권장) 프라이팬은 런 시작에, 손전등은 각자 그 런에 처음 들어올 때 1개씩(1일차·새 런만, 재접속은 또 안 줌) — 인원수와 정확히 맞는다
  - ② 2. 런 시작에 최대 인원 4명분을 한꺼번에 — 단순하지만 2인 판이면 2개가 남아 팔 수 있다
  - ③ 3. 도착이 끝나는 때처럼 나중 한 시점에 — '모두 모였다' 신호를 새로 만들어야 한다

## 2026-10-22 걷기는 몬스터에게 들리지 않는다 · 몬스터는 계단으로 층을 옮긴다

- **사용자 지시**: "캐릭터가 걷기할 때는 발소리가 안 나게 할 건데, 그럼 그땐 몬스터 시야에 안 보이면 어그로 안 끌리는 걸로 하자."
  → `CL9Rules.WalkNoiseRadius` 1700 → 0(2026-10-21 값과 2026-10-17 결정 D 를 번복). 달리기·휘두르기·타격 소음은 그대로다.
  거위가 걷는 사람에게 깨는 `WakeRadius` 는 거위의 핵심 행동이라 건드리지 않았다. 발소리 음향 자체는 이 결정에 들지 않는다.
- **사용자 보고**: "닭들이 1층에서도 2층을 때리고 있고 2층으로는 못 올라가."
  원인 둘: 몬스터 이동(`MoveAlongPath`)이 높이를 바꾸지 않아 계단 첫 단에 막혔고, 덮치기 예고 진입이 2D 거리만 봤다.
  → 이동은 한 단(45cm, 이동 거리의 1.5배까지)을 들어 쓸고 새 자리 바닥에 발을 붙인다. 예고·잡기 진입은 같은 층(높이 차 ≤ 몸 반지름 + 사람 캡슐 반높이)일 때만이다.

## 2026-10-22 휘두르기에 최소 치켜듦 · 판정을 좁힌다 · 고기도 프라이팬과 같은 방식

- **사용자 지시**: "그렇게 바꾸자. 그리고 판정도 수정하자, 우리 너무 전투가 쉬워. 그리고 닭다리나 프라이팬 다 공격이니까 같은 방식으로 돌아가게 해 줘."
  (앞선 대화: 리썰 덤퍼가 잡기 힘든 건 거리를 읽기 어렵고 삽에 선딜이 있어서다 — `docs/reference/lethal/lethal-items.md` Shovel 행 "최소 0.35초 준비")
- **최소 치켜듦** `CL9Rules.SwingMinWindup` 0.35초(신규 열, 헤더 0 = 옛 동작) — 누른 뒤 0.35초 전에 떼면 치켜든 채 기다렸다가 그 시각에 휘두른다.
  붙은 뒤 연타로는 못 맞히고, 다가오는 닭을 재고 미리 눌러야 한다.
- **판정**: 구간 `SwingHitStart`·`SwingHitEnd` [0.05, 0.25) → **[0.10, 0.18)**(내려친 바닥 0.13 둘레), 구 반경 `SwingHitRadius` 60 → **35**,
  도달 `SwingHitReach` 100 → **120**. 앞으로 닿는 거리는 196 → 191cm 로 거의 그대로(2m 약속 유지), 옆 관용치는 79 → 46cm.
- **고기 = 프라이팬 방식**: 휘두르기 코드·시간·판정은 원래 하나(`UseAction` Swing, `CL9Rules` 공용)였고 다른 것은 고기 행의 밀림 0 뿐이었다.
  `ItemTable` 고기(날것·익힘·탐) `HitKnockback` 0 → **120**(프라이팬과 같음). 피해는 고기 1 · 프라이팬 2 그대로 — 무기마다 세기만 다르다.

## 2026-10-23 닭 경직 짧게 · 물기 간격 짧게 (몬스터는 피하는 것)

- **사용자**: "우리 게임은 몬스터를 잡는 게 아니라 몬스터를 피하는 게임이야. 잡는 건 정말 어쩔 수 없는 경우고. 보통은 피해야 돼. ...
  프라이팬에 딜레이가 몬스터의 경직보다 길어야 돼." 표를 보니 그 조건은 이미 맞았고(휘두르기 간격 1.08초 > 닭 경직 0.3초), 쉬웠던 까닭은
  한 번 문 닭이 9초(`KillCooldown`) 동안 못 무는 것이었다 → 사용자 선택 "경직 더 짧게, 물기 간격 줄이기".
- 닭 `HitStunMs` 300 → **100**, `KillCooldown` 9 → **1.5**(`RageTime` 도 같이 1.5 — 한 손잡이, 로드 검사). 적용 스크립트 `Tools/Unreal/apply_chicken_close_fight_20261023.py`.
- 같은 답에 이어진 말(설계 확인 대기): "닭은 멀리서 달려드는 거랑 붙어서 때리는 거랑 두 개가 있어야 되네. 그리고 때릴 때 타게팅이 아니라 논타게팅으로 해야 될 거 같아."

## 2026-10-23 닭 두 공격 · 덮침 판정은 논타게팅

- **사용자**: "닭은 멀리서 달려드는 거랑 붙어서 때리는 거랑 두 개가 있어야 되네. 그리고 때릴 때 타게팅이 아니라 논타게팅으로 해야 될 거 같아."
  확인 뒤 "사람도 논타게팅이야. 우리 모든 건 논타게팅이야. 정확히는 타게팅하고 때리는데 그게 논타게팅이 된다는 거지."
- **논타게팅 판정**: 덮침(즉시·시간형 모두)은 노린 사람이 아니라 **맞는 순간 그 자리에 있는 사람**을 잡는다(`ACL9Monster::FindPounceVictim` —
  잡기 거리·같은 층·시야·넘어짐/처형/무적 아님 가운데 가장 가까운 사람). 노리기·방향 잡기는 예고 동안 그대로 타깃을 향한다.
  사람의 휘두르기는 원래 구 스윕이라 이미 논타게팅이다. 예고 없이 닿자마자 무는 종(`PounceWindup` 0)의 즉시 잡기는 이번에 바꾸지 않았다.
- **닭 두 공격**: 새 열 `PeckRadius`·`PeckWindup`·`PeckLunge`·`PeckMoveTime`(덮침의 짧은 판, 0 = 없음). 닭은 1.5m 안이면 0.3초 예고 뒤 80cm 를 0.12초에 쫀다,
  1.5~4.5m 면 0.6초 웅크린 뒤 5m 를 0.4초에 달려든다(`PounceWindup` 0 → 0.6 으로 다시 켬 — 10-21 덤퍼 추격 때 껐던 것). 쪼기는 벽 앞이어도 어지럽지 않다.
  적용 스크립트 `Tools/Unreal/apply_chicken_two_attacks_20261023.py`.

## 2026-10-23 닭 새 집 삼기

- **사용자**: (집에서 60m 를 넘어 추격을 놓으면 옛 자리로 돌아간다는 설명을 듣고) "그러지 말고 그 자리를 새로운 보금자리로 해서 거기서부터 다시 해야 될 거 같아."
- **새 열 `bRehomeOnLeash`**(종마다, 기본 꺼짐): 켠 종은 `HomeRange` 를 넘어 추격을 놓는 순간 그 자리가 `HomeLocation` 이 된다.
  이어지는 멈춤(`ChaseBreakTime`) 뒤 돌아가기는 제자리라 곧 새 집 둘레를 배회하고, 다음 추격의 60m 도 새 집에서 잰다.
  닭만 켰다 — 거위·돼지·너구리는 집을 지키는 거리라 그대로 둔다. 적용 스크립트 `Tools/Unreal/apply_chicken_rehome_20261023.py`.

## 2026-10-24 나르샤(NarshaMCP) 도입 — 읽기는 나르샤, 규칙 로직은 순수 모듈

- **사용자**: "나르샤sdk가있으니까 이거로 구조 다시만들자" → "코드구조도 바뀌는게 의미가있는지" 비교 뒤 "저거쓸꺼고 잘쓸수잇게 구조바꾸자".
- **비교 결론**: C++·표 정본 구조는 나르샤와 잘 맞아 코드를 다시 짜지 않는다. 바꾸는 것은 둘 — ① 도구 구조: 에셋·기호·엔진 소스 **읽기**를
  에디터 스크립트(`Tools/Unreal/probe_*.py` 등 529개 중 읽기 몫) 대신 나르샤 MCP 로 — 에디터 없이 `.uasset` 을 풀어 읽어 잠금·임대·권한이 필요 없다.
  ② 검증 구조: 규칙 로직은 `FCL9SiteLayout` 처럼 엔진 없는 순수 모듈 + 얇은 액터로 쓰고 자동화 테스트를 붙인다(conventions.md § 6 필수사항).
  큰 파일 셋(`CL9Character`·`CL9GameMode`·`CL9Monster`)은 진행 중 작업(건물 문·수납 가구·맵 분리)과 겹쳐 지금 쪼개지 않는다.
- **연결**: [.mcp.json](C:\Work\CL9\.mcp.json) `narshamcp`(엔진 마켓플레이스 플러그인의 `narshamcp.exe connect`, 데몬 포트 19620 공유,
  `NARSHAMCP_DAEMON_MEMORY_CAP_GB=6`). `settings.local.json` `enabledMcpjsonServers` 에 추가. 세션을 다시 켜야 보인다.
  시험(stdio 직접 호출): 도구 65개, `ue_grep`·`ue_read`·`ue_manage_blueprint`(BP 바이너리) 즉시, PDB 기호 색인은 첫 회 수십 초 뒤 `trace_hierarchy` 가
  `ACL9Monster` 조상 5·BP 자손 `BP_CL9Monster` 를 돌려줬다.
- **하지 않은 것**: 설치 프로그램 `narshamcp setup` 전체(`.cursor`·Antigravity·Codex `AGENTS.md`·프로토콜 등록·스킬 120개 마켓플레이스·`.claude/skills` 정리)는
  돌리지 않았다 — Codex 는 금지 상태이고 `.claude/skills` 정리가 CL9 스킬을 건드릴 수 있다. 에디터 플러그인 `NarshaMCP` 는 `CL9.uproject` 에 켜지 않았다 —
  새 DLL 을 Smart App Control 이 막으면 에디터가 스플래시에서 멈춰 모든 세션이 선다(2026-10 사고). 켜는 것은 사용자가 정한다.

## 2026-10-24 나르샤 에디터 플러그인 켬 — 사용자 "에디터 너가 키면 되잖아"

- `CL9.uproject` 에 `NarshaMCP`(Editor 전용) 를 켜고 에디터를 띄웠다. DLL BuildId 가 엔진과 같아(55116800) 빌드 없이 올라왔고 Smart App Control 차단은 없었다.
  로그: 모듈 로드 → 에디터 안 WebSocket 서버 30011 → MCP 포트 8100 30초 뒤 열림.
- **플러그인이 켜질 때마다 `narshamcp.exe setup` 전체를 돌린다**(`CUModule.cpp` `RunPostBuildInitialization`, 조건 없음). 이번에 바뀐 것:
  `.mcp.json`(내가 넣은 env 삭제, 경로만 남음), `.claude/settings.local.json`(`enableAllProjectMcpServers`, narsha-adk 플러그인 7개 켬 → 스킬 약 120개),
  `~/.codex/config.toml`(102~131줄 narsha 블록), `~/.gemini/antigravity/mcp_config.json`, `~/.codex/skills`(CL9 스킬 복사), `narshamcp://` 프로토콜 등록.
  `AGENTS.md`·`.antigravity/rules.md`·`.claude/agents/unreal-specialist.md`·`.claude/skills/routing.json` 은 전날 23:52(설치 때) 이미 생긴 것.
  CLAUDE.md 는 건드리지 않는다(#14108 로 폐기). CL9 스킬(`cl9`·`cl9film`·`insta-research`·`spritemake`)은 그대로다.
- 그래서 **데몬 설정은 사용자 환경 변수**(`NARSHAMCP_DAEMON_MEMORY_CAP_GB=6`, `NARSHAMCP_NO_DASHBOARD_GUI=1`, setx)로 두고, 다시 생기는 것은 gitignore
  (`.narshamcp/`·`.antigravity/`·`.cursor/`·`/AGENTS.md`). 앞 항목의 "에디터 플러그인은 켜지 않았다"는 이 항목으로 뒤집혔다.
- setup 이 `.mcp.json` 을 다시 쓴 순간 떠 있던 Claude 세션 9개가 설정을 다시 읽어 `narshamcp connect` 를 붙였다(01:33) — 다시 켜지 않아도 됐다.
  데몬은 19704(대시보드 19804)에서 돌고 있다. 스킬 120개를 끌지는 사용자가 정한다(`settings.local.json` `enabledPlugins`).
