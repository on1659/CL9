// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9MonsterDef.generated.h"

class UAnimInstance;
class USkeletalMesh;
class USkeleton;

/**
 * One monster row (docs/plan/data/monster/common.md). The shared ACL9Monster
 * actor and its one state machine read everything from here — a new monster is
 * a new row, never new AI code (docs/systems/monster.md). Goose-only columns
 * exist on every row (unused species keep them 0/false) until a second species
 * promotes them.
 */
UCLASS()
class UCL9MonsterDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- Appearance (presentation only; the base Mesh column is the item StaticMesh) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** One skeleton per species — variants share it (docs/systems/animation.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeleton> Skeleton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftClassPtr<UAnimInstance> AnimBP;

	/**
	 * Yaw added to the visual mesh so the model faces the actor's forward (+X).
	 * Different animal packs export facing different axes — this corrects it
	 * from data instead of code (docs/systems/animation.md 원칙 1).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	float MeshYawOffset = 0.f;

	// -- Health --

	/** HP rolls HpMin..HpMax per individual — no-instakill is a value choice, not a rule */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HpMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HpMax = 1;

	/** Stagger for a non-lethal hit, in milliseconds (1000 = 1s) — needs its own animation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HitStunMs = 300;

	// -- Movement --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float WanderSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float PatrolSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float ChaseSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float RetreatSpeed = 0.f;

	/** Turn cap in degrees per second */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurnRate = 180.f;

	/** How much turning slows the run — this is what makes a maze a weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurnDrag = 0.f;

	/** Chase speed multiplier in the yard */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float OutdoorChaseMult = 1.f;

	/** Body radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float Radius = 30.f;

	/** Individuals keep this far apart, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float SeparationDist = 0.f;

	// -- Senses --

	/** Spot distance — must exceed SightOut (the hysteresis pair) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float SightIn = 0.f;

	/** Lose distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float SightOut = 0.f;

	/** Sight during the blackout */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float SightDark = 0.f;

	/** The one hearing value — noise radii come from the action (DA_CL9Rules) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float HearRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectNearRadius = 0.f;

	/** Detection gauge rise per second when near */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectRateNear = 0.f;

	/** Gauge rise per second when merely seen far */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectRateFar = 0.f;

	/** Gauge fall per second while undetected */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectDecay = 0.f;

	/** Grace seconds after leaving sight */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectGrace = 0.f;

	// -- Chase --

	/** Seconds a chase can run at most */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float ChaseBudget = 0.f;

	/** Seconds after losing sight until giving up */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float ChaseLoseTime = 0.f;

	/** Look-around pause after giving up, in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float ChaseBreakTime = 0.f;

	/** No new alerts for this long after a break — never blocks blackout/revenge/stun wakes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float ChaseCooldown = 0.f;

	/** Max distance from home, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float HomeRange = 0.f;

	/** Dropping every carried thing releases the chase at this distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float DeaggroRadius = 0.f;

	/** Catch reach in cm — 0 means a species that never catches */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float CatchRadius = 0.f;

	/** Player HP taken per catch */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	int32 CatchDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	int32 SearchHops = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float SearchDwellTime = 0.f;

	// -- Waking and alarms --

	/** Flapping-and-cry warning before waking, in seconds — every player sees it */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float WakeTelegraph = 0.f;

	/** Waking holds while any gaze sits inside this radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float WakeSightRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float WakeRetryTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float PackAlarmRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float KillAlarmRadius = 0.f;

	/** The alarmist flag — goose only for now */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	bool bAlertsOthers = false;

	// -- Goose-only columns (docs/plan/data/monster/goose.md — 0/false on other species) --

	/** Walking inside this still wakes it (the walking-is-silent exception) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float WakeRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float HearRunRadius = 0.f;

	/** Cry wind-up before the alarm lands, in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float AlertWindup = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float SleepTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float AlertCooldown = 0.f;

	// -- Retaliation and recovery (documented global defaults) --

	/** Hunt mode after hearing a death, in seconds — locked equal to KillCooldown (load check) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float RageTime = 9.f;

	/** Reload before it can kill again, in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float KillCooldown = 9.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float RevengeTime = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float RevengeRadius = 276.f;

	/** Stagger when shoved, in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float PushStun = 0.8f;

	/** Knockback distance in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float PushKnockback = 1560.f;

	// -- Display --

	/** Per-species value, not a global rule */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	bool bShowOnMinimap = false;

	// -- Value and drops --

	/** Unused until something like corpse selling exists */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	int32 ValueMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	int32 ValueMax = 0;

	/** The hunting reward — items, never direct ether (references the item table only) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	TArray<FDropItem> DropItems;
};
