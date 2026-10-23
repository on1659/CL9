# 시퀀스 재생 시스템

> 전이 연출을 어떻게 재생하는가. **기획(어느 전이에서 무엇을 트나)과 표 컬럼은
> [../plan/data/sequence.md](../plan/data/sequence.md)가 정본이다.**
>
> **현재는 구현하지 않음 — 문서만 사전 작성.** `UCL9SequenceSubsystem`도
> `UCL9SequenceDef`도 아직 소스에 없다 (2026-09-09 확인).

## 개요

- **UE 표준 Level Sequence + `ULevelSequencePlayer`.** 시퀀서로 에셋을 만들고 코드에서 튼다.
- **시퀀스는 표현이다.** "언제 틀지"는 호스트가 판정하고, 재생은 각 머신이 로컬로 한다.
- **데이터 주도.** 코드에 시퀀스 에셋 경로를 쓰지 않는다 — `UCL9SequenceDef` 표에서
  Alias로 조회한다 (`Content/Data/SequenceTable` — 미구현).

## 재생 흐름

```
호스트: 상태 전이 판정 (밤 시작 · 정전 · 전멸 · 새벽)
  ↓ Multicast RPC (연출 트리거는 Multicast로만 — OnRep 금지:
    OnRep은 중간 참가자의 초기 복제에서도 발화해 지나간 시퀀스를 재생한다)
각 머신 (호스트 포함): SequenceSubsystem->Play("blackout")
  ↓ FindSequence(Alias) → 소프트 참조 비동기 로드
  ↓ CreateLevelSequencePlayer → 재생 (로컬)
  ↓ bLockLocalInput이면 로컬 입력 잠금, bSkippable이면 스킵 키 활성
종료 (또는 로컬 스킵) → 입력 복원 → OnFinished 콜백
```

**프레임 동기화는 하지 않는다.** 시작 신호만 동기면 충분하다 — 각자 재생하고 각자 스킵한다.

## 재생기 — UCL9SequenceSubsystem

```cpp
// GameInstance 서브시스템 — 로컬 전용 (판정 없음)
class UCL9SequenceSubsystem : public UGameInstanceSubsystem {
public:
  // 유일한 진입점 — 호출부는 Alias만 안다
  void Play(FName Alias, FSimpleDelegate OnFinished = {});
  void SkipCurrent();     // bSkippable일 때만 (로컬)
  bool IsPlaying() const;

private:
  // FindSequence → StreamableManager 비동기 로드 → CreateLevelSequencePlayer
  // bLockLocalInput → 로컬 PlayerController 입력 모드 전환/복원
};
```

- 동시 재생은 1개 — 재생 중 새 요청이 오면 기존 것을 끝내고 새 것을 튼다 (전이 연출뿐이라
  겹칠 일이 없어야 정상이고, 겹치면 로그를 찍는다).
- 서버 판정 코드에서는 이 서브시스템을 호출하지 않는다 — 표현 계층(Multicast 수신부)에서만.

## 호스트 대기 (bHostWaits)

시퀀스가 끝나야 넘어가는 전이는 호스트가 **시퀀스 에셋의 길이**를 타이머로 기다린다:

```cpp
// GameMode (호스트 전용) — 호스트는 시퀀스 에셋을 프리로드해둔다
// (소프트 참조 비동기 로드라, 길이를 읽으려면 Multicast 전에 로드돼 있어야 한다)
Multicast_PlaySequence("night_start");
const float Duration = Def->GetSequenceDuration();   // 프리로드된 에셋에서 읽음
GetWorldTimerManager().SetTimer(Handle, this, &ThisClass::OpenNightControls, Duration);
```

- 클라의 재생 완료 보고를 기다리지 않는다 — 유실·조작 위험이 있고, 로컬 스킵과도 얽힌다.
- 스킵한 사람은 남는 시간 동안 그냥 대기 화면을 본다 (전이는 호스트 타이머가 정한다).

## 제작 규칙 — 레벨에 묶이지 않게

Level Sequence 에셋은 어느 레벨에서든 틀 수 있지만, **레벨에 배치된 액터를 Possessable로
직접 물면 그 레벨에서만 재생된다.** CL9은 맵이 시드로 런타임 생성되고 목적지마다 레벨이
다르므로:

1. **카메라·소품은 Spawnable로 만든다** — 시퀀스가 직접 스폰하고 끝나면 거둔다. 레벨 무관.
2. **월드 실물(트럭 등)은 Possessable 금지 — 런타임 바인딩으로 잇는다.** 재생 직전에
   서브시스템이 현재 맵의 대상 액터를 찾아(클래스/태그) `SetBinding`으로 연결한다.
   시퀀스는 "트럭 자리"라는 빈 바인딩만 갖는다.
3. 특정 목적지에만 있는 연출은 그 목적지 전용 시퀀스로 따로 만들고 표에서 다른 Alias를 준다 —
   공용 시퀀스에 레벨 특정 바인딩을 섞지 않는다.

## 시퀀서가 아닌 연출 — 건물 전원 인트로 `power_intro` (2026-10-06)

이 시스템(시퀀서·`UCL9SequenceSubsystem`)은 여전히 없다. 그런데 **카메라를 뺏는 연출 하나가 이미 C++ 로 지어졌다** — 판 시작의 건물 전원 인트로
(명세 [../goal/round-start-truck-intro.md](../goal/round-start-truck-intro.md) K6~K11, 연출 표 행은 [../plan/data/sequence.md](../plan/data/sequence.md) `power_intro`).
위 흐름·제작 규칙과 다른 점을 적어 둔다 — 나중에 시퀀서를 지을 때 이 연출을 옮기려 하지 말 것.

- **시퀀서가 아닌 이유**: 카메라가 날아가는 길이 그 판 레이아웃의 칸 길(스위치 칸 → 각자 목적지 모듈)이라 맵이 판마다 시드로 달라진다. Spawnable 카메라든 런타임 바인딩이든
  미리 만든 Level Sequence 에는 "그 판의 길"을 담을 곳이 없다. 반려된 대안: Level Sequence, 스플라인 클래스.
- **재생 방식이 다르다**: Multicast 로 신호를 보내지 않는다(위 "OnRep 금지 — 중간 참가자에게 지나간 시퀀스가 재생된다"의 반대편 해법). 호스트가 **켠 서버 시각 T 와 참가자별 목적지**
  (`ACL9GameState::PowerOnServerTime`·`IntroTargets`)를 한 프레임에 복제하고, 각 머신이 **(서버 지금 − T) 의 함수**로 자기 카메라를 낸다. 그래서 늦게 받은 클라이언트는 경과만큼
  진행된 지점에서 시작하고, 리슨 호스트는 OnRep 이 없어도 같은 값을 매 틱 읽는다.
- **시작은 컷이 아니라 지금 시점에서 출발한다**(2026-10-10, 명세 Follow-up K10-F — 사용자: "순간이동해서 가는 것 같다"). 세 단계다:
  - **시점에서 출발**: 경과가 `IntroLeadIn` 안일 때 시작하면 카메라는 로컬 플레이어의 지금 화면(카메라 매니저의 위치·회전 = S)에 생겨 넘어가는 순간이 보이지 않고,
    회전도 S 에서 출발해 첫 틱부터 초당 180°까지만 돈다. 그보다 늦게 T 를 받은 클라이언트는 예전대로 그 경과의 자리로 바로 넘어간다(컷).
  - **모이기**: [시작, `IntroLeadIn`) 동안 S 에서 스위치 칸 중심 P0(170 cm)까지 모이기 길 G 를 등속으로 간다 — S 가 1층 칸이면 그 칸에서 스위치 칸까지의
    칸 길(`CL9SiteLayout::FindCellPathToCell`, 비행 길과 같은 변 규칙), 자기 칸이 스위치 칸이면 [S, P0], 칸이 없거나(트럭 안·2층·사이트 밖) 길이 없으면 [S, P0] 직선(벽을 지날 수 있다 — 사용자 수용).
    G 의 구간도 비행 길처럼 시작 때 한 번 스윕해 올리지만 S 와 P0 는 올리지 않는다. 시선은 G 에 이어 비행 길 위로 400 cm 앞이다. G 는 머신마다 다르다(S 가 다르다) — 시간 함수는 같다.
  - **같이 출발**: `IntroLeadIn` 에 모든 참가자가 P0 에 있고 거기서 각자의 칸 길로 `IntroFlyTime`(기본 7.0초) 동안 난다. 비행 모드는 길이 L 로 정한다(2026-10-11, F6):
    L ≤ `IntroMinSpeed × IntroFlyTime`(4200 cm)이면 `IntroMinSpeed`(600 cm/s)로 날아 `L ÷ IntroMinSpeed` 초에 닿고 남은 비행 시간 동안 목적지에서 목표(항아리)를 본다(min speed),
    그보다 길고 10500 cm 이하면 비행 시간 내내 등속(even), 10500 cm 를 넘으면 가운데를 0.15초 암전으로 건너뛴다(dip cut). 시간 창(불·깜빡임·잠금·무적)은 모드와 무관하다.
  - **자기 몸**(2026-10-11, F5 — 사용자 "고치기"): 카메라가 눈 위치에서 출발하므로, 카메라가 내 폰의 1인칭 카메라 자리에서 120 cm(코드 상수 `IntroCamBodyClearance`) 떨어지기 전에는
    1인칭 모습 그대로다(팔·든 아이템 보임, 내 몸 숨김 — 인트로 카메라가 1인칭 카메라의 1인칭 FOV·축소 설정을 이어받아 팔 모양도 같다). 처음 넘는 틱에 3인칭 몸으로 바뀌고 그 인트로가 끝날 때까지 그대로다.
    늦게 시작해 처음부터 멀면 바로 3인칭이다. 거리 비교만 하고 트레이스는 없다. 남의 화면은 바뀌지 않는다(카메라가 로컬에만 있다).
  - 로그: `Intro camera: start — …, flight min speed, arrive +X s | even | dip cut (over the speed cap)`(비행 길·모드, X 는 T 기준) 다음 줄
    `Intro camera: start from view <S> rot <회전> — gather N cell(s) | straight, L cm in t s; flight F cm`, 몸이 바뀌는 틱에 `Intro camera: body on at <거리> cm (+<경과> s)`,
    LeadIn 을 넘는 틱에 `Intro camera: leave switch cell at <카메라 위치> (P0 <P0>) — <넘은 초> s past the lead-in, <거리> cm from P0`. 늦게 시작하면 `start from view`·`leave switch cell` 줄이 없다.
- **건너뛰기 없음**(명세): 조작·시선·행동이 잠기고(잠금 창 = 켠 뒤 `IntroLeadIn + IntroFlyTime + IntroFlickerTime + IntroReturnTime`) 무적은 `IntroGrace` 만큼 더 간다 —
  둘 다 서버 시각 하나에서 파생되는 창이다. 서버의 뷰 타깃 경로(데스캠·처형·관전·부활)는 이 카메라를 이긴다 — 인트로 카메라는 자기 자신만 되돌린다.
- **카메라 구동은 컨트롤러 몫**이고(로컬 컨트롤러 틱 — [performance.md](performance.md) 틱), 판정(켜짐·목적지·창)은 호스트의 게임 모드·게임 스테이트다.
  조명 쪽(램프·깜빡임·`SkyLeak`)은 [light.md](light.md) "전원 단계", 잠금·무적은 [character.md](character.md).

## 경계 상황

- **중간 참가자**: 재생 신호를 놓친 클라는 시퀀스를 틀지 않는다 — GameState의 전이 상태만
  따라간다. 연출은 놓쳐도 게임 상태는 정확하다.
- **재생 실패** (에셋 로드 실패 등): 로그만 찍고 즉시 OnFinished — 연출 없다고 게임이 멈추면
  안 된다. 호스트 타이머는 어차피 독립적으로 돈다.
- 시퀀스 안에서 판정 값을 바꾸지 않는다 — 트랙은 카메라·애니메이션·조명·소리 등 렌더 전용.

## 참고

- 표 컬럼·연출 목록(초안): [../plan/data/sequence.md](../plan/data/sequence.md)
- 데이터 조회: [../data/schema.md](../data/schema.md) (`UCL9DefRegistry.FindSequence`)
- 전이 지점(밤 시작·새벽): [balance.md](balance.md) · [ui.md](ui.md)
