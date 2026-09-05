// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CL9DefRegistry.generated.h"

class UCL9CharacterDef;
class UCL9DefBase;
class UCL9ItemDef;
class UCL9MonsterDef;
class UCL9OrderDef;
class UCL9ProgressionDef;
class UCL9RulesDef;
class UCL9SiteDef;

/**
 * One table asset per data kind, Excel-style (2026-09-02 결정): open the
 * asset, add rows with +, edit every column in place. Rows are inline
 * UCL9DefBase objects, so Instanced gimmicks keep working — the reason
 * UDataTable was rejected (docs/plan/data/README.md).
 */
UCLASS()
class UCL9DefTable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The rows — mixing kinds works, but one table per kind stays readable */
	UPROPERTY(EditAnywhere, Instanced, Category="Data", meta=(TitleProperty="Alias"))
	TArray<TObjectPtr<UCL9DefBase>> Rows;
};

/**
 * The one asset that lists every def table (or standalone row) to load. C++
 * never hardcodes asset paths — the registry walks this list instead
 * (docs/plan/data/README.md).
 *
 * The list fills itself: saving this asset rescans its own folder (and
 * subfolders), so new tables register by simply living under it. Runtime
 * still reads the saved list — the explicit references are what guarantee
 * cooking in packaged builds.
 */
UCLASS()
class UCL9DefList : public UDataAsset
{
	GENERATED_BODY()

public:
	/** UCL9DefTable assets or standalone UCL9DefBase assets */
	UPROPERTY(EditAnywhere, Category="Data")
	TArray<TSoftObjectPtr<UDataAsset>> Defs;

#if WITH_EDITOR
	/** Rescans this asset's folder for def tables and rebuilds the list */
	UFUNCTION(CallInEditor, Category="Data")
	void RefreshFromFolder();

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
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

	/** Returns the usable item row for the key, or null when missing/broken */
	const UCL9ItemDef* FindItem(FName Alias, FName SubAlias = NAME_None) const;

	/** Returns the usable monster row for the key, or null when missing/broken */
	const UCL9MonsterDef* FindMonster(FName Alias, FName SubAlias = NAME_None) const;

	/** The one global rules row, or null when the table lacks a usable one */
	const UCL9RulesDef* GetRules() const
	{
		return Rules;
	}

	/** The one day-progression row (docs/plan/data/progression.md), or null when none loaded — callers treat null as "no scaling" */
	const UCL9ProgressionDef* GetProgression() const
	{
		return Progression;
	}

	/** Returns the usable site row for the alias, or null when missing/broken */
	const UCL9SiteDef* FindSite(FName Alias) const;

	/** Usable site rows in list order */
	const TArray<TObjectPtr<const UCL9SiteDef>>& GetSites() const
	{
		return OrderedSites;
	}

	/** Returns the usable order row for the alias, or null when missing/broken */
	const UCL9OrderDef* FindOrder(FName Alias) const;

	/** Usable order rows in list order — the lobby draw filters these by day (docs/systems/order.md) */
	const TArray<TObjectPtr<const UCL9OrderDef>>& GetOrders() const
	{
		return OrderedOrders;
	}

	/** Usable item rows in list order — cheat number arguments index into this (docs/systems/cheats.md) */
	const TArray<TObjectPtr<const UCL9ItemDef>>& GetItems() const
	{
		return OrderedItems;
	}

	/** Usable character rows in list order */
	const TArray<TObjectPtr<const UCL9CharacterDef>>& GetCharacters() const
	{
		return OrderedCharacters;
	}

	/** Usable monster rows in list order */
	const TArray<TObjectPtr<const UCL9MonsterDef>>& GetMonsters() const
	{
		return OrderedMonsters;
	}

	/** Broken rows with their reasons, for cl9.list (schema.md EDefStatus) */
	const TArray<FString>& GetBrokenRows() const
	{
		return BrokenRows;
	}

private:
	/** Validates and indexes one row — shared by table rows and standalone assets */
	void RegisterDef(const UCL9DefBase* Def);

	/** Per-row relation checks A1/A2 against every monster row (docs/plan/economy.md) */
	void CheckCharacterRelations(const UCL9CharacterDef& Row) const;

	/** Reference integrity across rows (cook chains, drops) — runs after every row loaded */
	void CheckItemReferences() const;

	/** Site spawn entries must name rows that loaded — a typo there spawns nothing silently otherwise */
	void CheckSiteReferences() const;

	/** Order ingredients must be item rows (alias+sub) — an order nobody can fill is a broken row */
	void CheckOrderReferences() const;

	/** Keeps every usable row alive for the GC; the lookup maps point into this */
	UPROPERTY()
	TArray<TObjectPtr<const UCL9DefBase>> LoadedDefs;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9ItemDef>> OrderedItems;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9CharacterDef>> OrderedCharacters;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9MonsterDef>> OrderedMonsters;

	UPROPERTY()
	TObjectPtr<const UCL9RulesDef> Rules;

	UPROPERTY()
	TObjectPtr<const UCL9ProgressionDef> Progression;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9SiteDef>> OrderedSites;

	TMap<TPair<FName, FName>, const UCL9CharacterDef*> Characters;
	TMap<TPair<FName, FName>, const UCL9ItemDef*> Items;
	TMap<TPair<FName, FName>, const UCL9MonsterDef*> Monsters;
	TMap<FName, const UCL9SiteDef*> Sites;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9OrderDef>> OrderedOrders;

	TMap<FName, const UCL9OrderDef*> Orders;
	TArray<FString> BrokenRows;
};
