// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9CharacterDef.generated.h"

class UAnimInstance;
class USkeletalMesh;
class USkeleton;

/**
 * One playable character row (docs/plan/data/character.md). Body numbers and
 * stamina are ruling values every machine reads from its local registry;
 * meshes and colors are presentation. Defaults below mirror the documented
 * table defaults — the saved asset is what the game actually reads.
 */
UCLASS()
class UCL9CharacterDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- Appearance (presentation only) --

	/** Full body mesh other players see; the base Mesh column is the item StaticMesh, hence a separate field */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** Skeleton the body mesh and its animations share */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeleton> Skeleton;

	/** Full body animation blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftClassPtr<UAnimInstance> AnimBP;

	/** First person view mesh — only the owner sees it (docs/systems/animation.md 1인칭 팔) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeletalMesh> ArmsMesh;

	/** First person view animation blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftClassPtr<UAnimInstance> ArmsAnimBP;

	/** Slot color — the only way to tell players apart in the dark, so keep it saturated */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	FLinearColor TintColor = FLinearColor::White;

	// -- Body (ruling values) --

	/** Walk speed in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float WalkSpeed = 396.f;

	/** Run speed in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float RunSpeed = 654.f;

	/** Multiplier on carried weight — below 1 carries lighter */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CarryMult = 1.f;

	/** Multiplier while pulling the cart */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CartMult = 0.75f;

	/** Capsule radius in cm — corridor width 255cm is built on this, meshes fit the capsule and never the reverse */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CapsuleRadius = 39.f;

	/** Capsule half height in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CapsuleHalfHeight = 90.f;

	/** First person camera height above the floor in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float EyeHeight = 156.f;

	/** Jump initial speed in cm/s — apex 156cm with the default gravity below */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float JumpVelocity = 780.f;

	/** Gravity while airborne in cm/s² */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float JumpGravity = 1950.f;

	// -- Health --

	/** Minimum hit points */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HpMin = 3;

	/** Maximum hit points */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HpMax = 3;

	/** Stagger after a cushioned hit, in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float HurtStun = 0.35f;

	/** Seconds a catch keeps the character down and unable to move (초안 — docs/plan/data/character.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float KnockdownTime = 1.5f;

	// -- Stamina (0..1 normalized; the three clauses in docs/systems/character.md all hang on these) --

	/** Seconds from full to empty while running */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float StaminaDrainTime = 6.f;

	/** Seconds from empty to full while not running */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float StaminaRegenTime = 8.f;

	/** Exhaustion clears only once stamina recovers past this (hysteresis against edge flicker) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float StaminaMinResume = 0.25f;
};
