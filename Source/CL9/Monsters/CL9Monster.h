// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "CL9Monster.generated.h"

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

	ECL9MonsterState GetState() const
	{
		return State;
	}

protected:
	virtual void LoadData() override;

	UFUNCTION()
	void OnRep_State();

private:
	/** Host only: the whole state machine lives here */
	void HostTickAI(float DeltaSeconds);

	/** Host only: steers along PathPoints with the turn model; returns true when the path is done */
	bool MoveAlongPath(float DeltaSeconds, float Speed);

	/** Host only: picks a reachable point near home and paths to it */
	void StartNewWander();

	/** Presentation side effects of a state change, on every machine */
	void ApplyState();

	/** Body sphere — the root; radius is the row's ruling value */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereComponent;

	/** Visual mesh from the row — smoothed against the 20Hz root snaps on clients */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY()
	TObjectPtr<const UCL9MonsterDef> MonsterDef;

	UPROPERTY(ReplicatedUsing=OnRep_State)
	ECL9MonsterState State = ECL9MonsterState::Wander;

	/** Host-rolled at load from HpMin..HpMax; host-only writes (conventions § 4) */
	UPROPERTY(Replicated)
	int32 Hp = 0;

	// Host-only movement state
	FVector HomeLocation = FVector::ZeroVector;
	TArray<FVector> PathPoints;
	int32 PathIndex = 0;
	float HeadingYaw = 0.f;
	float WanderWaitRemaining = 0.f;

	// Client-only smoothing state
	FVector SmoothedLocation = FVector::ZeroVector;
	FRotator SmoothedRotation = FRotator::ZeroRotator;
	bool bSmoothingInitialized = false;
};
