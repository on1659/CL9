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

	// -- Map generation (site.md 맵 생성). The grid is fixed per site; the seed
	//    only shapes the maze inside it — cages, plugs, jars, the flap.

	/** Which generator lays the site — None leaves the flat level bare */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	ECL9Generator Generator = ECL9Generator::None;

	/** 1F aisles (corridors along the barn) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 AisleCount = 8;

	/** 1F columns (cells along each aisle) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 ColCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 AisleCountB1 = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 ColCountB1 = 4;

	/** Corridor width (cm) — the door and the stair corridor share it */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float AisleWidth = 255.f;

	/** Cell length along the aisle (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float ColWidth = 900.f;

	/** Cage row depth between two aisles (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float RowDepth = 120.f;

	/** Opening cut through a cage row (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float OpenWidth = 360.f;

	/** Plug across an aisle (cm, along the aisle) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float PlugWidth = 180.f;

	/** ★ Difficulty knob — passable edges on 1F; fewer means more dead ends (site.md 개구부) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 TargetOpen1F = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 TargetOpenB1 = 25;

	/** Return jars per floor, spread so every cell has one nearby */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 JarCount1F = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 JarCountB1 = 3;
};
