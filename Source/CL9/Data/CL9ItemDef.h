// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9ItemDef.generated.h"

/**
 * One pickable item row (docs/plan/data/item/common.md). The shared ACL9Item
 * actor reads everything — mesh, physics, carry rules — from here, so a new
 * item is a new row, never new C++.
 */
UCLASS()
class UCL9ItemDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- Carry --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	ECL9CarrySlot CarrySlot = ECL9CarrySlot::Pocketable;

	/** Effective weight feeds the slowdown once the weight→speed formula lands (미결) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	bool bStackable = false;

	// -- Physics --

	/**
	 * Released items ride real host physics — rolling, sliding, natural rest
	 * (2026-09-03 결정). Off falls back to the simple bounce sim (the cart
	 * keeps this off — its sphere body would roll the whole wagon).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	bool bPhysicsEnabled = true;

	/** Collision radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float Radius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	bool bFragile = false;

	/** Impact speed (cm/s) that cracks a pristine item — must stay below CrackedBreakSpeed below BreakSpeed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float CrackSpeed = 0.f;

	/** Impact speed (cm/s) that breaks a pristine item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float BreakSpeed = 0.f;

	/** Impact speed (cm/s) that breaks an already cracked item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float CrackedBreakSpeed = 0.f;

	/** Fall acceleration in cm/s² */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float Gravity = 1950.f;

	/** Floor bounce factor 0..1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float Restitution = 0.3f;

	/** Horizontal damping on floor contact 0..1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float Friction = 0.5f;

	// -- Sound and light --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	bool bEmitsLight = false;

	/** Glow vs toggled flashlight — only read when bEmitsLight (docs/systems/light.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(EditCondition="bEmitsLight"))
	ECL9ItemLight LightKind = ECL9ItemLight::Glow;

	/** Glow radius / flashlight reach in cm while carried */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	float LightRadius = 0.f;

	/** Brightness — point light candelas, or the flashlight's beam intensity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(EditCondition="bEmitsLight"))
	float LightIntensity = 5000.f;

	/** Flashlight outer cone half-angle in degrees (ignored by Glow) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(EditCondition="bEmitsLight"))
	float LightConeAngle = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	bool bLightLostInPocket = true;

	/** How far a glowing carrier is seen during the blackout, in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	float CarrierSeenRadius = 0.f;

	// -- Display --

	/** Per-kind value, not a global rule (docs/systems/ui.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	bool bShowOnMinimap = false;

	// -- Value (ether, integer — docs/plan/economy.md) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	int32 ValueMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	int32 ValueMax = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	bool bWorthlessIfBroken = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	bool bWorthlessIfBurnt = false;

	// -- Cooking (the chain rows must exist — the load check verifies) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	FCL9ItemRef CookResult;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	FCL9ItemRef BurnResult;

	/** Seconds on heat until cooked */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	float CookTime = 12.f;

	/** Seconds on heat until burnt */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	float BurnTime = 24.f;

	/** Smoke column warning lead — render only, never a ruling input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	float BurnWarnTime = 6.f;

	// -- Type and eating --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Type")
	ECL9ItemType ItemType = ECL9ItemType::Ingredient;

	/** Applies on eating — only Dish items are edible (the type gates it) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Type")
	TArray<FCL9StatEffect> EatEffects;
};
