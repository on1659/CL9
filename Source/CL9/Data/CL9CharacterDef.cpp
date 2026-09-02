// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9CharacterDef.h"

void UCL9CharacterDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (WalkSpeed <= 0.f || RunSpeed <= 0.f)
	{
		OutErrors.Add(TEXT("WalkSpeed and RunSpeed must be positive."));
	}
	if (CapsuleRadius <= 0.f || CapsuleHalfHeight <= 0.f || EyeHeight <= 0.f)
	{
		OutErrors.Add(TEXT("CapsuleRadius, CapsuleHalfHeight and EyeHeight must be positive."));
	}
	if (JumpVelocity <= 0.f || JumpGravity <= 0.f)
	{
		OutErrors.Add(TEXT("JumpVelocity and JumpGravity must be positive."));
	}
	if (HpMin < 0 || HpMax < HpMin)
	{
		OutErrors.Add(TEXT("HP range needs 0 <= HpMin <= HpMax."));
	}
	if (StaminaDrainTime <= 0.f || StaminaRegenTime <= 0.f)
	{
		OutErrors.Add(TEXT("Stamina drain/regen times must be positive."));
	}
	if (StaminaMinResume < 0.f || StaminaMinResume > 1.f)
	{
		OutErrors.Add(TEXT("StaminaMinResume is normalized — keep it within 0..1."));
	}
}
