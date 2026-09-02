// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9MonsterDef.h"

void UCL9MonsterDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Radius <= 0.f)
	{
		OutErrors.Add(TEXT("Radius must be positive."));
	}
	if (HpMin < 0 || HpMax < HpMin)
	{
		OutErrors.Add(TEXT("HP range needs 0 <= HpMin <= HpMax."));
	}
	if (SightIn > 0.f && SightIn <= SightOut)
	{
		OutErrors.Add(TEXT("SightIn must exceed SightOut — the hysteresis pair (docs/plan/data/monster/common.md)."));
	}
	if (!FMath::IsNearlyEqual(KillCooldown, RageTime))
	{
		OutErrors.Add(TEXT("KillCooldown must equal RageTime — one knob, not two (docs/plan/economy.md 부등식)."));
	}
}
