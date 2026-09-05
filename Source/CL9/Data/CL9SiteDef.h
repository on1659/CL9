// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9SiteDef.generated.h"

/**
 * One destination row (docs/plan/data/site.md). A site gives the baselines —
 * which animals and ingredients live there and how many — and the day curve
 * (UCL9ProgressionDef) multiplies them at night start. The grid columns for
 * the map generator join with build-order ⑤; until then the flat test level
 * scatters the rolls around the truck.
 */
UCLASS()
class UCL9SiteDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** Animals living here — baseline counts, MonsterCountCurve scales the roll (site.md 서식 동물) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawns")
	TArray<FCL9SpawnEntry> MonsterSpawns;

	/** Ingredients lying around — baseline counts, ItemCountCurve scales the roll (site.md 산출 재료) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawns")
	TArray<FCL9SpawnEntry> ItemSpawns;

	/** Danger baseline the day curve multiplies — pressure per animal, not the count */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
	float BaseDanger = 1.f;

	/** First day this site can be chosen */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
	int32 UnlockDay = 1;
};
