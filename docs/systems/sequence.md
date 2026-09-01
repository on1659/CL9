# 시퀀스 재생 시스템

> 전이 연출을 어떻게 재생하는가. **기획(어느 전이에서 무엇을 트나)과 표 컬럼은
> [../plan/data/sequence.md](../plan/data/sequence.md)가 정본이다.**

## 개요

- **UE 표준 Level Sequence + `ULevelSequencePlayer`.** 시퀀서로 에셋을 만들고 코드에서 튼다.
- **시퀀스는 표현이다.** "언제 틀지"는 호스트가 판정하고, 재생은 각 머신이 로컬로 한다.
- **데이터 주도.** 코드에 시퀀스 에셋 경로를 쓰지 않는다 — `UCL9SequenceDef` 표에서
  Alias로 조회한다 (`Content/Data/Sequences/DA_Seq_*`).

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

## 경계 상황

- **중간 참가자**: 재생 신호를 놓친 클라는 시퀀스를 틀지 않는다 — GameState의 전이 상태만
  따라간다. 연출은 놓쳐도 게임 상태는 정확하다.
- **재생 실패** (에셋 로드 실패 등): 로그만 찍고 즉시 OnFinished — 연출 없다고 게임이 멈추면
  안 된다. 호스트 타이머는 어차피 독립적으로 돈다.
- 시퀀스 안에서 판정 값을 바꾸지 않는다 — 트랙은 카메라·애니메이션·조명·소리 등 렌더 전용.

## 치트

`cl9.playseq <alias|번호>` — 로컬 재생 테스트 ([cheats.md](cheats.md)). 판정에 안 닿는
로컬 연출이라 Server RPC를 타지 않는다.

## 참고

- 표 컬럼·연출 목록(초안): [../plan/data/sequence.md](../plan/data/sequence.md)
- 데이터 조회: [../data/schema.md](../data/schema.md) (`UCL9DefRegistry.FindSequence`)
- 전이 지점(밤 시작·새벽): [balance.md](balance.md) · [ui.md](ui.md)
