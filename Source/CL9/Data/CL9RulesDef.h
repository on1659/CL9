// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9RulesDef.generated.h"

/**
 * The one global rules asset — values no character can differ on: noise,
 * death, the night clock, carry friction (docs/plan/data/rules.md). Defaults
 * mirror the documented table; 미정 columns stay 0 until decided.
 */
UCLASS()
class UCL9RulesDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- Sound (monsters hear inside these radii — consumed from build-order ③) --

	/** Noise radius while running, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float RunNoiseRadius = 390.f;

	/** Picking up while walking, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PickupQuietRadius = 150.f;

	/** Picking up right after running, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PickupLoudRadius = 390.f;

	/** Shoving, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PushNoiseRadius = 255.f;

	/** Decoy, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float DecoyRadius = 330.f;

	/** A pickup counts as loud when the carrier ran within this window, in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float RecentRunTime = 1.f;

	// -- Death and recovery --

	/** Seconds from death to spectating a teammate — 초안 3, tuned by playing (docs/plan/data/rules.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float SpectateDelay = 3.f;

	/** Seconds standing on the truck pad before HP refills — 초안 5, tuned by playing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float TruckPadHealTime = 5.f;

	/** Seconds a leaver's pawn lingers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float LeaverGraceTime = 3.f;

	/** Threats never wake or spawn inside this radius, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float ThreatActMinRadius = 420.f;

	// -- Night clock --

	/** Night length in seconds — expiry is a dawn failure */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float NightLength = 245.f;

	/** Base seconds until the blackout */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkBase = 185.f;

	/** Seconds one jar return pulls the blackout forward */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkAccel = 4.f;

	/** Earliest possible blackout time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkMin = 125.f;

	/** Dawn screen length in seconds — 미정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DawnLength = 0.f;

	// -- Throwing and carrying --

	/** Hand↔pocket swap lock in seconds — deliberate carry friction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float SwapTime = 1.2f;

	/** Seconds of carrying-while-running until the shake gauge fills */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ShakeFullTime = 3.f;

	/** Shake gauge recovery time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ShakeDrainTime = 1.5f;

	/** Throw speed at zero charge, in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowMinSpeed = 600.f;

	/** Throw speed at full charge, in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowMaxSpeed = 1680.f;

	/** Seconds to full charge */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowChargeTime = 0.85f;

	/** Hand height above the floor in cm — where carried items sit */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float CarryHeight = 120.f;

	/** Pickup reach in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float PickRadius = 120.f;
};
