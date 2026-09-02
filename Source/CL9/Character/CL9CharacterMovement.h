// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CL9CharacterMovement.generated.h"

class UCL9CharacterDef;

/**
 * Predicted movement. The run flag and stamina live here — not in PlayerState —
 * so the server and the autonomous proxy derive the same speed from the same
 * compressed input flag and never rubber-band (docs/systems/character.md,
 * conventions § 4 예외). Speeds and stamina timings are read live from the
 * character table row, never stored in code.
 */
UCLASS()
class UCL9CharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual float GetMaxSpeed() const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/** Called when the character's table row is applied; the row outlives the pawn (registry-owned) */
	void SetCharacterDef(const UCL9CharacterDef* InCharacterDef);

	/** Local input only — the flag reaches the server inside the compressed move flags, never as a direct speed change */
	void SetWantsToRun(bool bNewWantsToRun);

	/** True while the run speed is in effect and the character actually moves (anim blueprints read this) */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsRunning() const;

	/** Normalized 0..1 (docs/plan/data/character.md 스태미나) */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetStamina() const
	{
		return Stamina;
	}

	/** Exhaustion latch — running stays impossible until stamina passes StaminaMinResume */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsExhausted() const
	{
		return bExhausted;
	}

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

private:
	/** The local table row — same asset on every machine (conventions § 5) */
	UPROPERTY()
	TObjectPtr<const UCL9CharacterDef> CharacterDef;

	/** Run input state, carried per move by FSavedMove_CL9 */
	bool bWantsToRun = false;

	/** 0..1 — simulated inside the move path so server corrections replay it deterministically */
	float Stamina = 1.f;

	/** 탈진 래치 (docs/systems/character.md 스태미나 세 조항) */
	bool bExhausted = false;

	friend class FSavedMove_CL9;
};

/** Saved move carrying the run flag plus a stamina snapshot so replays restart from the right state */
class FSavedMove_CL9 : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	FSavedMove_CL9();

	uint8 bSavedWantsToRun : 1;
	uint8 bSavedExhausted : 1;
	float SavedStamina;

	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;
};

/** Allocates FSavedMove_CL9 for the autonomous proxy */
class FNetworkPredictionData_Client_CL9 : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_CL9(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};
