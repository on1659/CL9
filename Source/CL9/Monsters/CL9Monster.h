// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "Core/CL9Damageable.h"
#include "CL9Monster.generated.h"

enum class ECL9HitStun : uint8;

class ACL9Character;
class UCameraComponent;
class UCL9MonsterDef;
class UMaterialInterface;
class UNiagaraSystem;
class USphereComponent;
class USkeletalMeshComponent;

/**
 * 몬스터 상태 기계의 상태들(docs/systems/monster.md — 초안). 모든 종이 하나의
 * enum을 공유하며, 쓰지 않는 상태는 테이블 값으로 꺼진다.
 */
UENUM(BlueprintType)
enum class ECL9MonsterState : uint8
{
	Asleep,
	WakeTelegraph,
	Wander,
	Patrol,
	Investigate,
	Chase,
	/** 잡기에 성공한 뒤의 짧은 공격 휴지기 — 연출용 훅이며, 이후 다시 Chase로 돌아간다 */
	Catch,
	Break,
	Return,
	Stun,
	Dead,
	/**
	 * 잡기 예고 — Chase 의 부분 상태. PounceRadius 안에서 이동을 멈추고 PounceWindup 동안 타깃을 향해 돈 뒤 PounceLunge 만큼
	 * 덮쳐 재검사한다: 잡으면 Catch, 헛치면 Chase 로 (docs/systems/monster.md 잡기 예고). enum 끝에 둔 것은 복제·에셋의 옛 값을 보존하려는 것
	 */
	Pounce,
	/**
	 * 스토킹 — 스토커 종(ExecuteRadius > 0 — 너구리)이 Chase 대신 들어가는 사냥 상태. 누가 보면 서고(가까우면 물러나고), 아무도 안 보면
	 * 타깃 등 뒤로 다가가 조건이 맞는 순간 Execute 로. StalkStareAnger 만큼 응시당하면 격분해 Chase (docs/systems/monster.md 스토킹과 처형)
	 */
	Stalk,
	/**
	 * 처형 — 스토커는 타깃 등 뒤 앵커로 텔레포트해, 잡히면 처형인 종(bExecuteOnCatch — 돼지)은 잡은 자리에서 ExecuteDuration 뒤 HostKill.
	 * 구조 없음: 맞아도 경직·상태 변경이 없다. 죽인 뒤에도 피해자의
	 * 관전이 시작될 때까지(최대 + SpectateDelay) 머물다 Break. 처형 카메라가 켜지는 유일한 상태
	 */
	Execute
};

/**
 * 모든 몬스터가 공유하는 하나의 액터(docs/systems/monster.md): AI는 단일
 * 프레임으로 돌아가고 종마다의 차이는 전부 테이블 값이다. 호스트가 감각·상태
 * 전이·이동을 처리하고, 클라이언트는 복제된 상태를 렌더링하며 메시 보간기로
 * 20Hz 위치 스냅을 매끄럽게 만든다.
 *
 * 이동은 커스텀 방식이다(CharacterMovement 없음): NavMesh가 경유점을 제공하고
 * 액터는 TurnRate/TurnDrag 아래에서 그 경로를 따라 조향한다 — 넓고 느리게
 * 도는 것이 코너를 무기로 만드는 핵심이다(2026-09-02 경로 방식 확정).
 *
 * 감각(3d): SightIn/SightOut 히스테리시스와 벽 트레이스를 쓰는 시야, CL9Rules의
 * 달리기 소음, 근접도가 모두 하나의 감지 게이지로 모인다. 게이지가 가득 찼을 때
 * 시야가 있으면 Chase, 없으면 Investigate로 간다.
 */
UCLASS(abstract)
class ACL9Monster : public ACL9ActorBase, public ICL9Damageable
{
	GENERATED_BODY()

public:
	ACL9Monster();

	virtual void Tick(float DeltaSeconds) override;
	virtual FText GetDisplayName() const override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const UCL9MonsterDef* GetMonsterDef() const
	{
		return MonsterDef;
	}

	UFUNCTION(BlueprintPure, Category="CL9")
	ECL9MonsterState GetState() const
	{
		return State;
	}

	/** 0..1 — 1.0이면 발각됨 (docs/systems/monster.md 감각 파이프라인) */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetDetectGauge() const
	{
		return DetectGauge;
	}

	/** 애니메이션 블루프린트용 수평 이동 속도 — 모든 머신에서 유효하다 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetGroundSpeed() const
	{
		return DisplaySpeed;
	}

	/**
	 * 지금 상태에 들어온 뒤 지난 초 — 원샷 애님(Catch·HitReact·PushReact·WakeTelegraph·Death)을 시퀀스 평가기로 트는 원천이라
	 * 늦게 들어온 클라도 같은 프레임을 본다 (docs/systems/animation.md 몬스터 원샷). 모든 머신에서 유효하다.
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetStateElapsed() const;

	/** 지금 경직이 밀침(프라이팬 — 행 `PushStun`)인가 — AnimBP 가 HitReact 대신 PushReact 를 고른다 */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsPushStun() const
	{
		return bPushStun;
	}

	/** 마지막 덮침(Pounce 예고 끝)이 잡았는가 — false 면 헛침. `GetPounceResultElapsed()` 와 함께 ABP 가 헛침 클립(Catch 앞부분)을 고른다 */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsPounceHit() const
	{
		return bPounceHit;
	}

	/**
	 * 마지막 덮침 결과가 찍힌 뒤 지난 서버 초 — 헛침이면 ABP 가 이 값이 0.3 아래인 동안 `Catch` 클립 앞부분을 평가기로 튼다
	 * (농부 휘두르기와 같은 시계 방식이라 연속 헛침도 시각이 갱신돼 구분된다). **결과가 한 번도 없으면(스폰 직후·늦게 접속한 클라)
	 * 1000 이상** — `PounceResultServerTime` 기본값이 −1000 이라 서버 시각 + 1000 이 나온다. 모든 머신에서 유효하다
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetPounceResultElapsed() const;

	/** 덮침 결과가 한 번이라도 찍혔는가 — 없으면 `GetPounceResultElapsed()` 는 1000 이상이다. ABP 가 헛침 창을 열기 전에 확인할 수 있다 */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool HasPounceResult() const
	{
		return PounceResultServerTime >= 0.f;
	}

	/** 호스트 전용(docs/systems/monster.md): HP 감소, HitStun 경직, 경보를 동반한 죽음 */
	virtual void OnDamage(int32 Amount, AActor* Source) override;

	/**
	 * 호스트 전용: 팩 동료나 거위 울음으로부터 경보 지점이 도착한다 — 잠든
	 * 개체는 깨고, 대기 중인 개체는 조사하러 간다. ChaseCooldown은 여전히
	 * 이를 막는 문턱으로 작동한다(경보는 새 경계).
	 */
	void ReceivePackAlarm(const FVector& AlarmLocation);

	/**
	 * 호스트 전용: KillAlarmRadius 안에서 들리는 죽음의 비명 — RageTime 동안의
	 * 사냥 모드에 들어가며, 이는 ChaseCooldown이 절대 막지 못한다
	 * (docs/systems/monster.md 경보).
	 */
	void ReceiveKillAlarm(const FVector& DeathLocation, float HuntSeconds);

	/**
	 * 호스트 전용: 플레이어(프라이팬)에게 떠밀렸을 때 — PushStun 경직과 함께
	 * 떠민 쪽 반대 방향으로 넉백하고, 이후 그 범인을 쫓아가도록 방향을 튼다
	 * (docs/systems/monster.md 반격, docs/plan/data/monster/common.md PushStun).
	 */
	void OnShove(AActor* Shover);

	/**
	 * 호스트 전용: 손에 든 아이템에 맞았다 — 행의 HitDamage·HitStun(docs/plan/data/item/common.md
	 * 사용)을 합친 것. 피해를 먼저 넣고, 죽지 않았을 때만 경직 종류대로 OnDamage의 경직 또는
	 * OnShove의 밀쳐냄이 따라온다. 피해 0·경직 None이면 아무 일도 없다.
	 */
	void OnStrike(int32 Damage, ECL9HitStun Stun, AActor* Source);

	/**
	 * 호스트 전용, 치트 `cl9.execute`: 처형 종(IsExecutioner)이고 Dead·Execute 가 아닐 때만. 스토커는 계수기·거리·등 뒤·미발견·시야·
	 * KillCooldown 검사를 **우회**하고 앵커 스윕만 통과하면 처형한다; 잡히면 처형인 종(bExecuteOnCatch 만)은 타깃 상태 검사만 하고
	 * 앵커 스윕 없이 지금 자리에서 처형한다. 못 하면 사유를 로그에 남기고 false
	 */
	bool HostCheatExecute(ACL9Character& Target);

protected:
	virtual void LoadData() override;

	UFUNCTION()
	void OnRep_State();

private:
	// -- 호스트 AI --

	void HostTickAI(float DeltaSeconds);

	/** 게임 상태에서 가져온 오늘 밤의 DangerCurve × 부지 배율 — 추격 예산, 감지 속도, 시야 범위가 이 값에 따라 스케일된다 (docs/systems/monster.md 일자 스케일링) */
	float GetDangerScale() const;

	/** 시야, 달리기 소음, 근접도로부터 감지 게이지를 채우고 마지막으로 감지한 지점을 기억한다 */
	void SenseTick(float DeltaSeconds);

	/** 호스트 전용: 상태를 전환하고 진입 시 설정을 실행한다 */
	void EnterState(ECL9MonsterState NewState);

	/** 대상까지의 직선이 월드 지오메트리에 가로막히지 않았으면 true */
	bool HasLineOfSight(const ACL9Character& Target) const;

	/** 호스트 전용: 회전 모델에 따라 PathPoints를 따라 조향한다 — 경로가 끝나면 true 반환 */
	bool MoveAlongPath(float DeltaSeconds, float Speed);

	/** 호스트 전용: 집 근처에서 도달 가능한 지점을 골라 그곳으로 경로를 잡는다 */
	void StartNewWander();

	/**
	 * 호스트 전용: 잡기를 성사시킨다 — 대상에게 OnDamage, KillCooldown 재충전, Catch 휴지기. 잡히면 처형인 종(bExecuteOnCatch)은
	 * 그 셋 대신 잡은 자리에서 처형에 들어간다(HostBeginExecute, 텔레포트 없음 — KillCooldown 은 Execute 틱의 HostKill 자리에서)
	 */
	void HostCatch(ACL9Character& Target);

	/**
	 * 호스트 전용: 추격 시계를 한 틱 흘리고(예산·놓침) 추격을 포기해야 하면 true — 타깃 소실·사망, ChaseBudget 소진,
	 * ChaseLoseTime 경과, HomeRange 이탈, DeaggroRadius(짐 전부 내려놓음). Chase 틱과 Pounce 틱이 같은 규칙을 쓴다
	 */
	bool TickChaseGiveUp(ACL9Character* Target, float DeltaSeconds);

	/**
	 * 호스트 전용: Catch 휴지기·Pounce 헛침 뒤 EnterState 를 거치지 않고 추격으로 돌아간다 — 예산과 놓침 시계를 그대로
	 * 둔다(잡기·헛침은 추격을 다시 채워 주지 않는다). 경로만 무효화해 다음 틱에 다시 잡는다
	 */
	void ResumeChase();

	/**
	 * 호스트 전용, 예고 끝: 타깃의 지금 위치 방향으로 PounceLunge 만큼 몸체 구를 스윕해(WorldStatic + WorldDynamic, 자신·타깃·
	 * 캐릭터·아이템·몬스터 무시, Z 평탄) 닿는 데까지 옮긴 뒤 재검사한다 — 도달 위치에서 Dist2D <= CatchRadius ∧ 높이 차 ∧
	 * 시야 트레이스 ∧ 타깃이 넘어져 있지 않음이면 HostCatch, 아니면 헛침(PounceMissCooldown, Chase 복귀). 결과 시각·성패를
	 * 복제하고 ForceNetUpdate 로 바로 보낸다
	 */
	void HostPounceLunge(ACL9Character& Target);

	/** 호스트 전용: 죽음 — DropItems 흩뿌림, 죽음 경보, 복수 전파, Dead 상태로 전환. 처형 중이면 피해자를 먼저 죽인다(구조 없음) */
	void Die(AActor* Source);

	// -- 스토킹·처형 (docs/systems/monster.md 스토킹과 처형) --

	/** 감각이 타깃을 잡았을 때 들어갈 사냥 상태 — 스토커 종(ExecuteRadius > 0)은 Stalk, 아니면 Chase(잡히면 처형인 종도 Chase — 잡기가 처형이다). 격분·Stun 복귀·Rage 는 이걸 거치지 않고 Chase 다 */
	ECL9MonsterState GetHuntState() const;

	/**
	 * 살아 있고 처형 중이 아닌 어느 플레이어든 StalkGazeRadius 안에서 이 몬스터를 보고 있으면 true — 2D 요 비교
	 * `|NormalizeAxis(조준 요 − (몬스터−플레이어) 방향 요)| <= StalkGazeHalfAngle` ∧ 시야 트레이스. `IsHeldByGaze` 의 3D 내적 0.75 는
	 * 바닥을 보면 "안 봄"이 되고 화면보다 좁아 쓰지 않는다 (D5)
	 */
	bool IsWatchedByAnyone() const;

	/** 타깃 등 뒤 지점 — 타깃 위치 − 타깃 전방(요만) × ExecuteAnchorDistance, 높이는 타깃 위치 그대로(경로용) */
	FVector GetBehindPoint(const ACL9Character& Target) const;

	/**
	 * 호스트 전용: 처형 진입 검사 — 아니면(bCheat) 계수기·거리·등 뒤·시야·KillCooldown 을 건너뛴다. 남는 검사는 타깃 상태(넘어짐·처형 중·
	 * 죽음)와 **앵커 스윕**이다. 통과하면 HostBeginExecute 까지 하고 true. 실패 사유는 바뀔 때마다 Verbose 로그 한 줄
	 */
	bool TryBeginExecute(ACL9Character& Target, bool bCheat);

	/**
	 * 앵커(타깃 등 뒤 ExecuteAnchorDistance, 높이는 이 몬스터의 지금 Z — 덮침과 같은 Z 평탄)가 비어 있는지 몸체 구를 타깃 자리에서
	 * 앵커까지 스윕해 본다 — WorldStatic|WorldDynamic, 자신·타깃·모든 캐릭터·아이템·몬스터·타깃의 PullingCart 무시. 막히면 false (D1)
	 */
	bool SweepExecuteAnchor(const ACL9Character& Target, FVector& OutAnchor) const;

	/**
	 * 호스트 전용, 진입 에지: (bTeleport 면) 앵커로 텔레포트·타깃을 향해 회전·경로 무효화·Execute 진입·타깃 HostBeginExecuted·ForceNetUpdate.
	 * 스토커(TryBeginExecute)는 bTeleport true 로 앵커에, 잡히면 처형인 종(HostCatch)은 false 로 지금 자리(Anchor = GetActorLocation())에서
	 */
	void HostBeginExecute(ACL9Character& Target, const FVector& Anchor, bool bTeleport);

	/** 호스트 전용: PackAlarmRadius 안의 팩 동료들에게 감지 지점을 향한 경보를 보낸다 */
	void FirePackAlarm();

	/**
	 * 호스트 전용, 매 틱: 파동 표 `monster_prox` 행의 `TriggerRadius` 안에 플레이어가 있으면
	 * `RepeatInterval`마다 이 몬스터 자리에서 소음 사건을 낸다 — "근처에 뭔가 있다"만 말하는
	 * 공포 발신기이며 판정에는 닿지 않는다 (docs/systems/wave.md 소음 이벤트 `MonsterProx`).
	 */
	void HostTickProxNoise(float DeltaSeconds);

	/** 호스트 전용: 시선이 붙잡고 있지 않는 한 WakeTelegraph를 거쳐 Asleep에서 벗어난다 */
	void TryWake();

	/** WakeSightRadius 안의 플레이어가 이 몬스터를 시야가 트인 채로 조준하고 있는 동안 true */
	bool IsHeldByGaze() const;

	/** 호스트 전용: (navmesh에 투영된) 월드 위치로 경로를 잡는다 — 도달 불가능하면 false */
	bool StartPathTo(const FVector& WorldLocation);

	/** 모든 머신에서 일어나는, 상태 변화의 표현 측 부수 효과 */
	void ApplyState();

	/**
	 * 모든 머신, 매 틱: 맞은 직후(Stun·Dead 진입 뒤 `HitFlashTime` 동안)의 몬스터에게 빨간 오버레이를 씌우고
	 * 지나면 벗긴다 — 복제된 상태 시계에서 파생하므로 멀티캐스트가 필요 없다 (docs/systems/monster.md 타격 플래시).
	 */
	void TickHitFlash();

	/**
	 * 모든 머신, 매 틱: 타격의 표현 (docs/systems/monster.md 타격의 표현). Stun·Dead 진입의 **상승 에지**(플래시 조건이
	 * 거짓→참, 또는 상태 시각이 바뀜 = 경직 중 다시 맞음)에서 `Hit` 소리(+ Dead 면 `MonsterDeath`)와 타격 퍼프를 몬스터
	 * 자리에서 한 번 내고, 진입 뒤 `HitStopTime` 동안 메시 애니메이션을 멈춘다(히트스톱). 접속 직후 이미 지난 사건은
	 * 처음 한 틱을 기준값으로만 받아 적어 되풀이하지 않는다.
	 */
	void TickHitFeel();

	/** 타격 퍼프 스폰 — 메시 중심(`spine` 소켓, 없으면 경계 중심)에서, `User.Scale` 을 Dead 면 3.0 아니면 1.0 */
	void SpawnHitPuff(bool bDeath);

	/**
	 * 모든 머신, 매 틱: 상태 전이 소리 (docs/systems/monster.md 타격의 표현). `State == X ∧ (지난 틱 State != X ∨ StateStartServerTime
	 * 변화)` 를 전이로 보고 Pounce 진입엔 `MonsterPounce`, Catch 진입엔 `MonsterCatch` 를 메시 중심에서 한 번 낸다. 경과 창이 아니라
	 * 전이 감지라 늦게 온 복제도 놓치지 않는다. 첫 틱은 기준값만 적는다(늦게 접속한 클라 무음). `TickHitFeel` 의 Stun·Dead 창과는
	 * 별개라, Catch 가 관측 전에 Stun·Dead 로 덮이면(같은 틱 피격) 그 잡기 소리는 빠진다 — `Hurt` 가 이미 나므로 허용
	 */
	void TickStateSounds();

	/**
	 * 모든 머신, 매 틱: 발소리 — 수평 이동 거리를 누적해 보폭(`FootstepStrideRun`: Chase, 그 외 `FootstepStrideWalk`)마다
	 * 발밑에서 `Footstep` 행을 한 번 튼다. 클라는 보간된 메시 자리로 재서 20Hz 스냅에 걸음이 뭉치지 않는다. 서 있으면 안 난다.
	 */
	void TickFootstep();

	/** 호스트 전용, Dead 상태: 죽음 발사 비행을 한 틱 적분한다 — 넉백과 같은 스윕 이동에 중력을 더한 것 */
	void HostTickDeathLaunch(float DeltaSeconds);

	/** 게임 상태의 서버 시각 — 캐릭터와 같은 시계(ACL9Character::GetServerTime) */
	float GetServerTime() const;

	// -- 컴포넌트 --

	/** 몸통 구체 — 루트 컴포넌트이며 반지름은 행이 정한 판정값이다. 벽은 막고, 몸체끼리는 겹쳐 지나간다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereComponent;

	/** 행에서 가져온 시각 메시 — 클라이언트에서 20Hz 루트 스냅에 대해 매끄럽게 보간된다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	/**
	 * 처형 카메라 — 1인칭 전용 결정의 명시적 예외(관전과 같은 범주, D6). 기본 비활성이고 `ApplyState` 가 `State == Execute` 일 때만
	 * 행의 `ExecuteCameraSocket`(본 이름 허용, 없으면 메시 루트)에 `ExecuteCameraOffset`·`ExecuteCameraRotation` 으로 붙여 켠다.
	 * 피해자의 뷰 타깃 전환은 캐릭터 쪽(`HostBeginExecuted`)이 서버 `SetViewTarget` 으로 한다
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> ExecuteCamera;

	UPROPERTY()
	TObjectPtr<const UCL9MonsterDef> MonsterDef;

	// -- 복제되는 상태 --

	UPROPERTY(ReplicatedUsing=OnRep_State)
	ECL9MonsterState State = ECL9MonsterState::Wander;

	/** 로드 시 HpMin..HpMax 사이에서 호스트가 굴려 정함; 쓰기는 호스트 전용 (conventions § 4) */
	UPROPERTY(Replicated)
	int32 Hp = 0;

	/** 감지 게이지 0..1 — 나중에 표현 단계에서 예고할 수 있도록 복제해 둔다 */
	UPROPERTY(Replicated)
	float DetectGauge = 0.f;

	/** 지금 상태에 들어온 서버 시각 — GetStateElapsed의 원천. 호스트가 상태를 바꿀 때마다 찍는다 */
	UPROPERTY(Replicated)
	float StateStartServerTime = 0.f;

	/** 지금 Stun 이 밀침(OnShove)인가, 타격(OnDamage)인가 — 경직 클립 선택용 */
	UPROPERTY(Replicated)
	bool bPushStun = false;

	/** 마지막 덮침 결과가 찍힌 서버 시각 — GetPounceResultElapsed 의 원천. 음수 큰 값 = 아직 없음. 찍은 직후 ForceNetUpdate */
	UPROPERTY(Replicated)
	float PounceResultServerTime = -1000.f;

	/** 마지막 덮침이 잡았는가(true) 헛쳤는가(false) — PounceResultServerTime 과 같은 순간에 찍힌다 */
	UPROPERTY(Replicated)
	bool bPounceHit = false;

	// -- 호스트 전용 이동/AI 상태 --

	FVector HomeLocation = FVector::ZeroVector;
	TArray<FVector> PathPoints;
	int32 PathIndex = 0;
	float HeadingYaw = 0.f;
	float WanderWaitRemaining = 0.f;

	TWeakObjectPtr<ACL9Character> SensedCharacter;
	FVector LastSensedLocation = FVector::ZeroVector;
	bool bHadSight = false;
	float SightGraceRemaining = 0.f;

	float ChaseBudgetRemaining = 0.f;
	float ChaseLoseRemaining = 0.f;
	float ChaseRepathRemaining = 0.f;
	float BreakRemaining = 0.f;
	float ChaseCooldownRemaining = 0.f;
	int32 SearchHopsRemaining = 0;
	float SearchDwellRemaining = 0.f;

	// -- 3e~3g 호스트 상태: 잡기, 스턴, 경보, 수면 --

	/** KillCooldown 재충전 — 다 닳기 전까지는 두 번째 잡기가 불가능하다 */
	float CatchCooldownRemaining = 0.f;
	float CatchPauseRemaining = 0.f;
	float StunRemaining = 0.f;

	/** 예고(Pounce)의 남은 대기 초 — 0 이 되는 틱에 덮친다 */
	float PounceWindupRemaining = 0.f;

	/** 헛친 뒤 다음 예고까지 남은 초(PounceMissCooldown) — HostTickAI 머리에서 틱당 한 번만 줄어든다. KillCooldown 과 별개 */
	float PounceMissRemaining = 0.f;

	/** 예고 진입 때 잠근 타깃 — 예고 동안 감각의 재타깃팅은 무시하고, 덮침은 이 타깃의 지금 위치로 간다 */
	TWeakObjectPtr<ACL9Character> PounceTarget;

	/** Stun 상태 동안 이어지는 떠밀림 넉백 — 남은 거리와 그 방향 */
	float KnockbackRemaining = 0.f;
	FVector KnockbackDir = FVector::ZeroVector;
	float KnockbackSpeed = 0.f;

	/** 죽음의 비명 이후 RageTime 동안의 사냥 모드 — 게이지를 고정하고 ChaseCooldown을 우회한다 */
	float RageRemaining = 0.f;

	// -- 스토킹·처형 호스트 상태 --

	/** 스토킹 타깃 — Stalk 진입 때 잠그고, 감각이 다른 산 사람을 잡으면 교체하며 두 계수기를 처음부터 센다(D4: 계수기는 타깃별) */
	TWeakObjectPtr<ACL9Character> StalkTarget;

	/** 아무도 보지 않은 채 이어진 초 — 보이는 틱에 0. ExecuteUnseenTime 이상이어야 처형 */
	float StalkUnseenTime = 0.f;

	/** 응시당한 누적 초 — 보이면 쌓이고 안 보이면 같은 속도로 0 까지 감쇠. StalkStareAnger 에 닿으면 격분(Chase) */
	float StalkStaredTime = 0.f;

	/** 지금 경로가 후퇴 경로인가 — 후퇴·접근이 바뀌는 순간 경로를 버리고 다시 잡는다 */
	bool bStalkRetreating = false;

	/** 처형 타깃 — Execute 진입 때 잠근다. 접속 끊김이면 null → Break */
	TWeakObjectPtr<ACL9Character> ExecuteTarget;

	/** 이번 처형에서 HostKill 을 이미 냈는가 — 한 번만 */
	bool bExecuteKilled = false;

	/** 마지막으로 Verbose 에 찍은 처형 진입 실패 사유 — 같은 사유가 틱마다 반복되지 않게 바뀔 때만 찍는다 */
	FName LastExecuteBlock;

	/** 죽음 발사의 남은 속도(cm/s) — 0이면 비행이 끝났거나 시작하지 않았다. 스윕에 막히거나 바닥에 닿으면 0으로 */
	FVector DeathLaunchVelocity = FVector::ZeroVector;

	/** 근접 소나(파동)의 다음 발동까지 남은 초 — 표현 전용이며 AI 판정에는 쓰이지 않는다 */
	float ProxNoiseRemaining = 0.f;

	/** 거위 울음: 경보가 발동하기 전 준비 동작, 그 뒤 울음 사이의 AlertCooldown */
	float AlertWindupRemaining = 0.f;
	float AlertCooldownRemaining = 0.f;
	bool bPackAlarmFired = false;

	/** 잠 많은 종(SleepTime > 0): 다시 잠들기까지 누적되는 평온한 시간(초) */
	float CalmSeconds = 0.f;
	float WakeTelegraphRemaining = 0.f;
	float WakeRetryRemaining = 0.f;
	/** 깨어난 직후 조사하러 갈 위치(경보로 깨어난 경우); 미설정이면 평범한 배회 */
	TOptional<FVector> PendingInvestigateSpot;

	// -- 표현 상태 (모든 머신) --

	/** 규칙 애셋에서 한 번 로드해 둔 타격 플래시 머티리얼 — 행이 비었으면 null로 남고 플래시는 없다 */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> HitFlashMaterial;

	/** 플래시 머티리얼을 이미 한 번 로드해 봤는가 — 매 타격마다 소프트 참조를 다시 해석하지 않는다 */
	bool bHitFlashLoaded = false;

	/** 지금 오버레이가 켜져 있는가 — 바뀔 때만 SetOverlayMaterial을 부르고, 내가 켠 것만 끈다 */
	bool bHitFlashOn = false;

	/** 타격 표현의 에지 기준값을 한 번은 그냥 받아 적었는가 — 접속 직후 이미 지나간 타격으로 소리·퍼프를 내지 않는다 */
	bool bHitFeelPrimed = false;

	/** 지난 틱에 "맞은 직후 창(Stun·Dead ∧ 경과 < HitFlashTime)" 안이었는가 — 거짓→참이 타격 에지다 */
	bool bHitFeelInWindow = false;

	/** 지난 틱에 본 StateStartServerTime — 창 안에서 값이 바뀌면 경직 중 다시 맞은 것이라 에지로 친다 */
	float LastSeenStateStartTime = 0.f;

	/** 지금 히트스톱으로 애니메이션을 멈춰 두었는가 — 내가 멈춘 것만 푼다 */
	bool bHitStopOn = false;

	/** 상태 전이 소리의 기준값을 한 번은 그냥 받아 적었는가 — 접속 직후 이미 지나간 예고·잡기를 되풀이하지 않는다 */
	bool bStateSoundPrimed = false;

	/** 지난 틱에 본 State(전이 소리용) */
	ECL9MonsterState LastSoundState = ECL9MonsterState::Wander;

	/** 지난 틱에 본 StateStartServerTime(전이 소리용) — 같은 상태로 다시 들어와도(연속 예고) 시각이 바뀌어 전이로 잡힌다 */
	float LastSoundStateStartTime = 0.f;

	/** 규칙 애셋에서 한 번 로드해 둔 타격 퍼프 — 행이 비었으면 null로 남고 퍼프는 없다 */
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> HitPuffSystem;

	/** 퍼프를 이미 한 번 로드해 봤는가 */
	bool bHitPuffLoaded = false;

	/** 발소리 누적을 잰 직전 자리(호스트는 액터, 클라는 보간된 메시) — 아직 재기 시작하지 않았으면 미설정 */
	TOptional<FVector> LastFootstepLocation;

	/** 발소리 보폭의 누적 이동 거리(cm) */
	float FootstepDistanceAccum = 0.f;

	float DisplaySpeed = 0.f;
	FVector SmoothedLocation = FVector::ZeroVector;
	FRotator SmoothedRotation = FRotator::ZeroRotator;
	bool bSmoothingInitialized = false;
};
