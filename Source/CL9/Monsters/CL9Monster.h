// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "Core/CL9Damageable.h"
#include "CL9Monster.generated.h"

enum class ECL9HitStun : uint8;

class ACL9Character;
class UCL9MonsterDef;
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
	Dead
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

	/** 호스트 전용: 잡기를 성사시킨다 — 대상에게 OnDamage, KillCooldown 재충전, Catch 휴지기 */
	void HostCatch(ACL9Character& Target);

	/** 호스트 전용: 죽음 — DropItems 흩뿌림, 죽음 경보, 복수 전파, Dead 상태로 전환 */
	void Die(AActor* Source);

	/** 호스트 전용: PackAlarmRadius 안의 팩 동료들에게 감지 지점을 향한 경보를 보낸다 */
	void FirePackAlarm();

	/** 호스트 전용: 시선이 붙잡고 있지 않는 한 WakeTelegraph를 거쳐 Asleep에서 벗어난다 */
	void TryWake();

	/** WakeSightRadius 안의 플레이어가 이 몬스터를 시야가 트인 채로 조준하고 있는 동안 true */
	bool IsHeldByGaze() const;

	/** 호스트 전용: (navmesh에 투영된) 월드 위치로 경로를 잡는다 — 도달 불가능하면 false */
	bool StartPathTo(const FVector& WorldLocation);

	/** 모든 머신에서 일어나는, 상태 변화의 표현 측 부수 효과 */
	void ApplyState();

	/** 게임 상태의 서버 시각 — 캐릭터와 같은 시계(ACL9Character::GetServerTime) */
	float GetServerTime() const;

	// -- 컴포넌트 --

	/** 몸통 구체 — 루트 컴포넌트이며 반지름은 행이 정한 판정값이다. 벽은 막고, 몸체끼리는 겹쳐 지나간다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereComponent;

	/** 행에서 가져온 시각 메시 — 클라이언트에서 20Hz 루트 스냅에 대해 매끄럽게 보간된다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

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

	/** Stun 상태 동안 이어지는 떠밀림 넉백 — 남은 거리와 그 방향 */
	float KnockbackRemaining = 0.f;
	FVector KnockbackDir = FVector::ZeroVector;
	float KnockbackSpeed = 0.f;

	/** 죽음의 비명 이후 RageTime 동안의 사냥 모드 — 게이지를 고정하고 ChaseCooldown을 우회한다 */
	float RageRemaining = 0.f;

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

	float DisplaySpeed = 0.f;
	FVector SmoothedLocation = FVector::ZeroVector;
	FRotator SmoothedRotation = FRotator::ZeroRotator;
	bool bSmoothingInitialized = false;
};
