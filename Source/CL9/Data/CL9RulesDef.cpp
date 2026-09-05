// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9RulesDef.h"

void UCL9RulesDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (PickRadius <= 0.f || CarryHeight <= 0.f)
	{
		OutErrors.Add(TEXT("PickRadius and CarryHeight must be positive."));
	}
	if (SwapTime <= 0.f)
	{
		OutErrors.Add(TEXT("SwapTime must be positive — the swap lock is deliberate carry friction."));
	}
	if (!(ThrowMinSpeed > 0.f && ThrowMinSpeed <= ThrowMaxSpeed) || ThrowChargeTime <= 0.f)
	{
		OutErrors.Add(TEXT("Throw needs 0 < ThrowMinSpeed <= ThrowMaxSpeed and a positive ThrowChargeTime."));
	}
	if (StaminaShakeStart < 0.f || StaminaShakeStart > 1.f)
	{
		OutErrors.Add(TEXT("StaminaShakeStart must be within 0..1 (normalized stamina)."));
	}
}
