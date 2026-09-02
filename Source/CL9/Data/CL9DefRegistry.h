// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CL9DefRegistry.generated.h"

class UCL9CharacterDef;
class UCL9DefBase;

/**
 * The one asset that lists every table row to load. C++ never hardcodes asset
 * paths — the registry walks this list instead (docs/plan/data/README.md).
 */
UCLASS()
class UCL9DefList : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Data")
	TArray<TSoftObjectPtr<UCL9DefBase>> Defs;
};

/**
 * Local table lookup, owned by the game instance. Every machine loads the same
 * assets and runs the same load checks — only runtime state ever replicates
 * (conventions § 5). Rows that fail validation never leave Find* — they are
 * logged and excluded instead of silently mixing in (docs/data/schema.md).
 */
UCLASS()
class UCL9DefRegistry : public UObject
{
	GENERATED_BODY()

public:
	/** Loads and validates every row in the list; safe to call with null (logs and leaves lookups empty) */
	void Init(const UCL9DefList* DefList);

	/** Returns the usable character row for the key, or null when missing/broken */
	const UCL9CharacterDef* FindCharacter(FName Alias, FName SubAlias = NAME_None) const;

private:
	/** Per-row relation checks A1/A2 (docs/plan/economy.md) — run for every character row */
	void CheckCharacterRelations(const UCL9CharacterDef& Row) const;

	/** Keeps every usable row alive for the GC; the lookup maps point into this */
	UPROPERTY()
	TArray<TObjectPtr<const UCL9DefBase>> LoadedDefs;

	TMap<TPair<FName, FName>, const UCL9CharacterDef*> Characters;
};
