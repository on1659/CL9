// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9OrderDef.h"

void UCL9OrderDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Ingredients.Num() == 0)
	{
		OutErrors.Add(TEXT("Ingredients is empty — an order has to ask for something."));
	}
	for (const FCL9ItemCount& Need : Ingredients)
	{
		if (!Need.IsSet() || Need.Count <= 0)
		{
			OutErrors.Add(TEXT("Every ingredient needs an item alias and a positive count."));
			break;
		}
	}
	if (RewardMin < 0 || RewardMax < RewardMin)
	{
		OutErrors.Add(TEXT("Reward needs 0 <= RewardMin <= RewardMax."));
	}
	if (DeadlineDays < 1)
	{
		OutErrors.Add(TEXT("DeadlineDays must be at least 1 (tonight)."));
	}
	if (FailPenaltyRate < 0.f)
	{
		OutErrors.Add(TEXT("FailPenaltyRate cannot be negative."));
	}
	if (MinDay < 1 || (MaxDay > 0 && MaxDay < MinDay))
	{
		OutErrors.Add(TEXT("Offer window needs MinDay >= 1 and MaxDay either 0 or >= MinDay."));
	}
	if (Weight <= 0.f)
	{
		OutErrors.Add(TEXT("Weight must be positive to ever be drawn."));
	}
}
