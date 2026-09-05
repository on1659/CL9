// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9OrderDef.generated.h"

/**
 * One order row (docs/plan/data/order.md) — what tonight must cook and sell.
 * Orders ask for finished goods (egg+cooked), never raw ingredients, so every
 * order passes through cooking. The lobby draws N of these by day range and
 * weight and the host confirms M (docs/systems/order.md 주문 흐름).
 */
UCLASS()
class UCL9OrderDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** True when the row can be offered on the day (MinDay..MaxDay, 0 MaxDay = open-ended) */
	bool IsOfferedOn(int32 Day) const
	{
		return Day >= MinDay && (MaxDay <= 0 || Day <= MaxDay);
	}

	/** Finished goods required — alias+sub must be item rows (the load check verifies) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Order")
	TArray<FCL9ItemCount> Ingredients;

	/** Reward roll in ether on delivery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	int32 RewardMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	int32 RewardMax = 0;

	/** Days to deliver — 1 means tonight */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Deadline")
	int32 DeadlineDays = 1;

	/** Fraction of the reward taken from the run's ether when the deadline passes (초안 0.3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Deadline")
	float FailPenaltyRate = 0.3f;

	/** First day this order can show up */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Offer")
	int32 MinDay = 1;

	/** Last day it shows up — 0 keeps it forever */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Offer")
	int32 MaxDay = 0;

	/** Draw weight among the day's candidates */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Offer")
	float Weight = 1.f;
};
