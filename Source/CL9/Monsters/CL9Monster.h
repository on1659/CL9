// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "CL9Monster.generated.h"

class ACL9Character;
class UCL9MonsterDef;
class USphereComponent;
class USkeletalMeshComponent;

/**
 * Monster state machine states (docs/systems/monster.md — 초안). One enum for
 * every species; unused states are switched off by table values.
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
	Break,
	Return,
	Stun,
	Dead
};

/**
 * The one shared monster actor (docs/systems/monster.md): AI is a single frame
 * and every species difference is a table value. The host runs senses, state
 * transitions and movement; clients render replicated state and smooth the
 * 20Hz position snaps with a mesh interpolator.
 *
 * Movement is custom (no CharacterMovement): NavMesh supplies waypoints and
 * the actor steers along them under TurnRate/TurnDrag — turning wide and slow
 * is what makes corners a weapon (2026-09-02 경로 방식 확정).
 *
 * Senses (3d): sight with SightIn/SightOut hysteresis and wall traces, run
 * noise from DA_CL9Rules, and proximity all feed one detection gauge; a full
 * gauge with sight means Chase, without sight Investigate.
 */
UCLASS(abstract)
class ACL9Monster : public ACL9ActorBase
{
	GENERATED_BODY()

public:
	ACL9Monster();

	virtual void Tick(float DeltaSeconds) override;
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

	/** 0..1 — 1.0 is spotted (docs/systems/monster.md 감각 파이프라인) */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetDetectGauge() const
	{
		return DetectGauge;
	}

	/** Horizontal movement speed for animation blueprints, valid on every machine */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetGroundSpeed() const
	{
		return DisplaySpeed;
	}

protected:
	virtual void LoadData() override;

	UFUNCTION()
	void OnRep_State();

private:
	// -- Host AI --

	void HostTickAI(float DeltaSeconds);

	/** Feeds the detection gauge from sight, run noise and proximity; remembers the last sensed spot */
	void SenseTick(float DeltaSeconds);

	/** Host only: switches state and runs entry setup */
	void EnterState(ECL9MonsterState NewState);

	/** True when the straight line to the target is not blocked by world geometry */
	bool HasLineOfSight(const ACL9Character& Target) const;

	/** Host only: steers along PathPoints with the turn model; returns true when the path is done */
	bool MoveAlongPath(float DeltaSeconds, float Speed);

	/** Host only: picks a reachable point near home and paths to it */
	void StartNewWander();

	/** Host only: paths to a world location (projected onto the navmesh); false when unreachable */
	bool StartPathTo(const FVector& WorldLocation);

	/** Presentation side effects of a state change, on every machine */
	void ApplyState();

	// -- Components --

	/** Body sphere — the root; radius is the row's ruling value. Walls block, bodies overlap */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereComponent;

	/** Visual mesh from the row — smoothed against the 20Hz root snaps on clients */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY()
	TObjectPtr<const UCL9MonsterDef> MonsterDef;

	// -- Replicated state --

	UPROPERTY(ReplicatedUsing=OnRep_State)
	ECL9MonsterState State = ECL9MonsterState::Wander;

	/** Host-rolled at load from HpMin..HpMax; host-only writes (conventions § 4) */
	UPROPERTY(Replicated)
	int32 Hp = 0;

	/** Detection gauge 0..1 — replicated so presentation can telegraph it later */
	UPROPERTY(Replicated)
	float DetectGauge = 0.f;

	// -- Host-only movement/AI state --

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

	// -- Presentation state (every machine) --

	float DisplaySpeed = 0.f;
	FVector SmoothedLocation = FVector::ZeroVector;
	FRotator SmoothedRotation = FRotator::ZeroRotator;
	bool bSmoothingInitialized = false;
};
