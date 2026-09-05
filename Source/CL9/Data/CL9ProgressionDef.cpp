// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9ProgressionDef.h"

void UCL9ProgressionDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	for (const FCL9DayOverride& Override : DayOverrides)
	{
		if (Override.Day < 1 || Override.MonsterCountMult < 0.f || Override.DangerMult < 0.f)
		{
			OutErrors.Add(FString::Printf(TEXT("DayOverride for day %d needs Day >= 1 and non-negative multipliers (0 = use the curve)."), Override.Day));
		}
	}

	if (OrdersOfferedPerDay < 1 || OrdersRequiredPerDay < 1 || OrdersRequiredPerDay > OrdersOfferedPerDay)
	{
		OutErrors.Add(TEXT("Orders need 1 <= OrdersRequiredPerDay <= OrdersOfferedPerDay (docs/plan/data/progression.md 주문서)."));
	}
}

float UCL9ProgressionDef::MonsterCountAt(int32 Day) const
{
	const FCL9DayOverride* Override = FindOverride(Day);
	return (Override != nullptr && Override->MonsterCountMult > 0.f) ? Override->MonsterCountMult : Eval(MonsterCountCurve, Day);
}

float UCL9ProgressionDef::DangerAt(int32 Day) const
{
	const FCL9DayOverride* Override = FindOverride(Day);
	return (Override != nullptr && Override->DangerMult > 0.f) ? Override->DangerMult : Eval(DangerCurve, Day);
}

float UCL9ProgressionDef::ItemCountAt(int32 Day) const
{
	return Eval(ItemCountCurve, Day);
}

float UCL9ProgressionDef::NightLengthAt(int32 Day) const
{
	return Eval(NightLengthCurve, Day);
}

bool UCL9ProgressionDef::IsBlackoutDisabled(int32 Day) const
{
	const FCL9DayOverride* Override = FindOverride(Day);
	return Override != nullptr && Override->bNoBlackout;
}

const FCL9DayOverride* UCL9ProgressionDef::FindOverride(int32 Day) const
{
	return DayOverrides.FindByPredicate([Day](const FCL9DayOverride& Override)
	{
		return Override.Day == Day;
	});
}

float UCL9ProgressionDef::Eval(const FRuntimeFloatCurve& Curve, int32 Day)
{
	// An empty curve is "no scaling"; past the last key FRichCurve holds the
	// last value, which is the intended cap (docs/plan/data/progression.md).
	const FRichCurve* Rich = Curve.GetRichCurveConst();
	if (Rich == nullptr || Rich->GetNumKeys() == 0)
	{
		return 1.f;
	}
	return Rich->Eval(static_cast<float>(Day));
}
