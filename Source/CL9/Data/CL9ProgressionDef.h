// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9ProgressionDef.generated.h"

/**
 * The one day-progression asset (docs/plan/data/progression.md): "how many
 * chickens on which day". Sites give baselines, these curves give the
 * multiplier per day, DayOverrides pin individual days by hand. Curves are
 * inline FRuntimeFloatCurve — dragged as graphs in the asset editor, or pointed
 * at a UCurveFloat — and an empty curve reads as 1.0. Past the last key the
 * value holds (progression.md "10일 이후 구간은 마지막 키 값으로 고정").
 * Singular like the rules row; the registry keeps the first usable one.
 */
UCLASS()
class UCL9ProgressionDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** Multiplier on the site's monster counts for the day (override first, then the curve) */
	float MonsterCountAt(int32 Day) const;

	/** Multiplier on chase budget, detection speed and sight for the day */
	float DangerAt(int32 Day) const;

	/** Multiplier on the site's ingredient counts for the day */
	float ItemCountAt(int32 Day) const;

	/** Multiplier on the rules' NightLength for the day */
	float NightLengthAt(int32 Day) const;

	/** True when a DayOverride turns the blackout off for the day */
	bool IsBlackoutDisabled(int32 Day) const;

	/** 1일차 0.6 → 5일차 1.0 → 10일차 1.5 (초안) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve MonsterCountCurve;

	/** 1일차 0.8 → 10일차 1.3 (초안) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve DangerCurve;

	/** Flat 1.0 (초안) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve ItemCountCurve;

	/** Flat 1.0 (초안) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve NightLengthCurve;

	/** Days pinned by hand — day 1 plays like a tutorial (progression.md) */
	UPROPERTY(EditAnywhere, Category="Overrides")
	TArray<FCL9DayOverride> DayOverrides;

private:
	const FCL9DayOverride* FindOverride(int32 Day) const;

	/** Curve value at the day; 1.0 for an empty curve */
	static float Eval(const FRuntimeFloatCurve& Curve, int32 Day);
};
