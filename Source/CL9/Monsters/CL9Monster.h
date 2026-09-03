// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "Core/CL9Damageable.h"
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
	/** Brief attack pause after landing a catch — presentation hook, then back to Chase */
	Catch,
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
class ACL9Monster : public ACL9ActorBase, public ICL9Damageable
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

	/** Host only (docs/systems/monster.md): HP down, HitStun stagger, death with alarms */
	virtual void OnDamage(int32 Amount, AActor* Source) override;

	/**
	 * Host only: an alert spot arrives from a pack mate or a goose cry — sleepers
	 * wake, idle ones investigate. ChaseCooldown still gates it (경보는 새 경계).
	 */
	void ReceivePackAlarm(const FVector& AlarmLocation);

	/**
	 * Host only: a death scream within KillAlarmRadius — RageTime hunt mode,
	 * which ChaseCooldown never blocks (docs/systems/monster.md 경보).
	 */
	void ReceiveKillAlarm(const FVector& DeathLocation, float HuntSeconds);

	/**
	 * Host only: shoved by a player (frying pan) — PushStun stagger, knockback
	 * away from the shover, and it turns to chase the culprit
	 * (docs/systems/monster.md 반격, docs/plan/data/monster/common.md PushStun).
	 */
	void OnShove(AActor* Shover);

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

	/** Host only: lands the catch — OnDamage on the target, KillCooldown reload, Catch pause */
	void HostCatch(ACL9Character& Target);

	/** Host only: death — DropItems scatter, kill alarm, revenge spread, Dead state */
	void Die(AActor* Source);

	/** Host only: alerts pack mates inside PackAlarmRadius toward the sensed spot */
	void FirePackAlarm();

	/** Host only: leaves Asleep through the WakeTelegraph unless a gaze holds it */
	void TryWake();

	/** True while any player inside WakeSightRadius aims at this monster with line of sight */
	bool IsHeldByGaze() const;

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

	// -- 3e~3g host state: catch, stun, alarms, sleep --

	/** KillCooldown reload — no second catch until it drains */
	float CatchCooldownRemaining = 0.f;
	float CatchPauseRemaining = 0.f;
	float StunRemaining = 0.f;

	/** Shove knockback carried through the Stun state — distance left and its direction */
	float KnockbackRemaining = 0.f;
	FVector KnockbackDir = FVector::ZeroVector;
	float KnockbackSpeed = 0.f;

	/** RageTime hunt mode after a death scream — pins the gauge, bypasses ChaseCooldown */
	float RageRemaining = 0.f;

	/** Goose cry: wind-up before the alarm lands, then AlertCooldown between cries */
	float AlertWindupRemaining = 0.f;
	float AlertCooldownRemaining = 0.f;
	bool bPackAlarmFired = false;

	/** Sleepy species (SleepTime > 0): calm seconds accumulated toward re-sleep */
	float CalmSeconds = 0.f;
	float WakeTelegraphRemaining = 0.f;
	float WakeRetryRemaining = 0.f;
	/** Where to investigate right after waking (alarm wakes); unset = plain wander */
	TOptional<FVector> PendingInvestigateSpot;

	// -- Presentation state (every machine) --

	float DisplaySpeed = 0.f;
	FVector SmoothedLocation = FVector::ZeroVector;
	FRotator SmoothedRotation = FRotator::ZeroRotator;
	bool bSmoothingInitialized = false;
};
