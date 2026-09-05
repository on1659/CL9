// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "CL9ShakeTable.generated.h"

class UCameraShakeBase;

/** One shake row: which asset, and the gauge value it starts at */
USTRUCT()
struct FCL9ShakeInfo
{
	GENERATED_BODY()

	/** Amplitude, frequency and blend times live in this asset — never as columns here */
	UPROPERTY(EditAnywhere, Category="Shake")
	TSoftClassPtr<UCameraShakeBase> Shake;

	/**
	 * Plays once the driving gauge (normalized 0..1) drops below this; the scale
	 * ramps to full at 0. Stamina reads it as "tired enough to show".
	 */
	UPROPERTY(EditAnywhere, Category="Shake")
	float StartAt = 0.5f;
};

/**
 * Camera shake table (2026-09-05 결정, docs/systems/character.md 흔들림): adding a
 * shake is making the shake asset and adding a row here — the rules asset never
 * grows a column per shake. Same shape as UCL9UITable, and assigned in
 * BP_CL9GameInstance so C++ holds zero asset paths (conventions § 5).
 */
UCLASS()
class UCL9ShakeTable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Name → shake. The key is what the presentation code looks up */
	UPROPERTY(EditAnywhere, Category="Shake")
	TMap<FName, FCL9ShakeInfo> Rows;
};

/** Row names C++ looks up — everything else stays data-only */
namespace CL9ShakeNames
{
	/** Tired breathing, driven by normalized stamina */
	inline const FName Stamina(TEXT("Stamina"));
}
