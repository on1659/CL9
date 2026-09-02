// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9ItemDef.h"

void UCL9ItemDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Radius <= 0.f)
	{
		OutErrors.Add(TEXT("Radius must be positive."));
	}
	if (Gravity <= 0.f)
	{
		OutErrors.Add(TEXT("Gravity must be positive."));
	}
	if (bFragile && !(CrackSpeed < CrackedBreakSpeed && CrackedBreakSpeed < BreakSpeed))
	{
		OutErrors.Add(TEXT("Fragile items need CrackSpeed < CrackedBreakSpeed < BreakSpeed — cracked must be weaker (docs/plan/economy.md)."));
	}
	if (ValueMin < 0 || ValueMax < ValueMin)
	{
		OutErrors.Add(TEXT("Value range needs 0 <= ValueMin <= ValueMax."));
	}
}
