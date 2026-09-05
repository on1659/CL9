// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9DefRegistry.h"
#include "CL9.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9OrderDef.h"
#include "Data/CL9ProgressionDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9SiteDef.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/ObjectSaveContext.h"

void UCL9DefList::RefreshFromFolder()
{
	// The scan root is wherever this asset lives — no path in code. Rows
	// register by existing under the list's folder.
	const FString RootPath = FPackageName::GetLongPackagePath(GetPackage()->GetName());

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	FARFilter Filter;
	Filter.PackagePaths.Add(*RootPath);
	Filter.bRecursivePaths = true;
	Filter.bRecursiveClasses = true;
	Filter.ClassPaths.Add(UCL9DefTable::StaticClass()->GetClassPathName());
	Filter.ClassPaths.Add(UCL9DefBase::StaticClass()->GetClassPathName());

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssets(Filter, Assets);

	// Deterministic order keeps cl9.list numbering stable between rescans
	Assets.Sort([](const FAssetData& A, const FAssetData& B)
	{
		return A.GetSoftObjectPath().ToString() < B.GetSoftObjectPath().ToString();
	});

	Defs.Reset();
	for (const FAssetData& Asset : Assets)
	{
		Defs.Emplace(Asset.GetSoftObjectPath());
	}

	UE_LOG(LogCL9, Log, TEXT("%s: registered %d def assets under %s."), *GetName(), Defs.Num(), *RootPath);
}

void UCL9DefList::PreSave(FObjectPreSaveContext SaveContext)
{
	// Every editor save rescans, so the list never goes stale; cooking keeps
	// the content that was saved.
	if (!SaveContext.IsCooking())
	{
		RefreshFromFolder();
	}
	Super::PreSave(SaveContext);
}
#endif

void UCL9DefRegistry::Init(const UCL9DefList* DefList)
{
	if (DefList == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("Def registry got no def list — every table lookup will fail (docs/data/schema.md)."));
		return;
	}

	for (const TSoftObjectPtr<UDataAsset>& DefRef : DefList->Defs)
	{
		const UDataAsset* Entry = DefRef.LoadSynchronous();
		if (Entry == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("Def list entry '%s' does not resolve — skipped."), *DefRef.ToString());
			BrokenRows.Add(FString::Printf(TEXT("%s ✗ (unresolved reference)"), *DefRef.ToString()));
			continue;
		}

		if (const UCL9DefTable* Table = Cast<UCL9DefTable>(Entry))
		{
			for (const UCL9DefBase* Row : Table->Rows)
			{
				if (Row == nullptr)
				{
					UE_LOG(LogCL9, Error, TEXT("%s: empty row entry — skipped."), *Table->GetName());
					continue;
				}
				RegisterDef(Row);
			}
		}
		else if (const UCL9DefBase* Def = Cast<UCL9DefBase>(Entry))
		{
			RegisterDef(Def);
		}
		else
		{
			UE_LOG(LogCL9, Error, TEXT("%s: neither a def table nor a def row — skipped."), *Entry->GetName());
		}
	}

	CheckItemReferences();
	CheckSiteReferences();
	CheckOrderReferences();

	// Relation checks need the whole registry, so they run after every table
	// loaded — A1/A2 compare character rows against monster rows.
	for (const UCL9CharacterDef* Character : OrderedCharacters)
	{
		CheckCharacterRelations(*Character);
	}

	if (Rules == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("No usable rules row loaded — carry/throw rulings will refuse to run (docs/plan/data/rules.md)."));
	}
}

void UCL9DefRegistry::RegisterDef(const UCL9DefBase* Def)
{
	TArray<FString> Errors;
	Def->Validate(Errors);
	if (Errors.Num() > 0)
	{
		for (const FString& Error : Errors)
		{
			UE_LOG(LogCL9, Error, TEXT("%s: broken row — %s"), *Def->GetName(), *Error);
		}
		BrokenRows.Add(FString::Printf(TEXT("%s ✗ (%s)"), *Def->GetName(), *Errors[0]));
		return;
	}

	LoadedDefs.Add(Def);

	const TPair<FName, FName> Key(Def->Alias, Def->SubAlias);

	if (const UCL9CharacterDef* CharacterDef = Cast<UCL9CharacterDef>(Def))
	{
		if (Characters.Contains(Key))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate character key '%s'/'%s' — the first row stays."),
				*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
			return;
		}
		Characters.Add(Key, CharacterDef);
		OrderedCharacters.Add(CharacterDef);
	}
	else if (const UCL9ItemDef* ItemDef = Cast<UCL9ItemDef>(Def))
	{
		if (Items.Contains(Key))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate item key '%s'/'%s' — the first row stays."),
				*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
			return;
		}
		Items.Add(Key, ItemDef);
		OrderedItems.Add(ItemDef);
	}
	else if (const UCL9MonsterDef* MonsterDef = Cast<UCL9MonsterDef>(Def))
	{
		if (Monsters.Contains(Key))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate monster key '%s'/'%s' — the first row stays."),
				*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
			return;
		}
		Monsters.Add(Key, MonsterDef);
		OrderedMonsters.Add(MonsterDef);
	}
	else if (const UCL9RulesDef* RulesDef = Cast<UCL9RulesDef>(Def))
	{
		if (Rules != nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("%s: a second rules row — the first one stays (the rules asset is singular)."), *Def->GetName());
			return;
		}
		Rules = RulesDef;
	}
	else if (const UCL9ProgressionDef* ProgressionDef = Cast<UCL9ProgressionDef>(Def))
	{
		if (Progression != nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("%s: a second progression row — the first one stays (the progression asset is singular)."), *Def->GetName());
			return;
		}
		Progression = ProgressionDef;
	}
	else if (const UCL9SiteDef* SiteDef = Cast<UCL9SiteDef>(Def))
	{
		if (Sites.Contains(Def->Alias))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate site key '%s' — the first row stays."), *Def->GetName(), *Def->Alias.ToString());
			return;
		}
		Sites.Add(Def->Alias, SiteDef);
		OrderedSites.Add(SiteDef);
	}
	else if (const UCL9OrderDef* OrderDef = Cast<UCL9OrderDef>(Def))
	{
		if (Orders.Contains(Def->Alias))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate order key '%s' — the first row stays."), *Def->GetName(), *Def->Alias.ToString());
			return;
		}
		Orders.Add(Def->Alias, OrderDef);
		OrderedOrders.Add(OrderDef);
	}
}

const UCL9OrderDef* UCL9DefRegistry::FindOrder(FName Alias) const
{
	const UCL9OrderDef* const* Found = Orders.Find(Alias);
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::CheckOrderReferences() const
{
	for (const UCL9OrderDef* Order : OrderedOrders)
	{
		for (const FCL9ItemCount& Need : Order->Ingredients)
		{
			if (FindItem(Need.ItemAlias, Need.ItemSubAlias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: Ingredients names '%s'/'%s' which is missing or broken — nobody can fill this order."),
					*Order->GetName(), *Need.ItemAlias.ToString(), *Need.ItemSubAlias.ToString());
			}
		}
	}
}

const UCL9SiteDef* UCL9DefRegistry::FindSite(FName Alias) const
{
	const UCL9SiteDef* const* Found = Sites.Find(Alias);
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::CheckSiteReferences() const
{
	for (const UCL9SiteDef* Site : OrderedSites)
	{
		for (const FCL9SpawnEntry& Entry : Site->MonsterSpawns)
		{
			if (FindMonster(Entry.Alias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: MonsterSpawns names '%s' which is missing or broken — that entry spawns nothing."),
					*Site->GetName(), *Entry.Alias.ToString());
			}
		}
		for (const FCL9SpawnEntry& Entry : Site->ItemSpawns)
		{
			if (FindItem(Entry.Alias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: ItemSpawns names '%s' which is missing or broken — that entry spawns nothing."),
					*Site->GetName(), *Entry.Alias.ToString());
			}
		}
	}
}

const UCL9CharacterDef* UCL9DefRegistry::FindCharacter(FName Alias, FName SubAlias) const
{
	const UCL9CharacterDef* const* Found = Characters.Find(TPair<FName, FName>(Alias, SubAlias));
	return Found ? *Found : nullptr;
}

const UCL9MonsterDef* UCL9DefRegistry::FindMonster(FName Alias, FName SubAlias) const
{
	const UCL9MonsterDef* const* Found = Monsters.Find(TPair<FName, FName>(Alias, SubAlias));
	return Found ? *Found : nullptr;
}

const UCL9ItemDef* UCL9DefRegistry::FindItem(FName Alias, FName SubAlias) const
{
	const UCL9ItemDef* const* Found = Items.Find(TPair<FName, FName>(Alias, SubAlias));
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::CheckCharacterRelations(const UCL9CharacterDef& Row) const
{
	// Derived values from docs/plan/economy.md — checked per character row
	// against every catch-capable monster row. A silent break here means a
	// player who cannot escape even after dropping everything.
	const float CycleTime = Row.StaminaDrainTime + Row.StaminaRegenTime;
	const float SustainedTiredSpeed = (Row.RunSpeed * Row.StaminaDrainTime + Row.WalkSpeed * Row.StaminaRegenTime) / FMath::Max(CycleTime, UE_KINDA_SMALL_NUMBER);
	const float SprintBudget = Row.RunSpeed * Row.StaminaDrainTime;

	UE_LOG(LogCL9, Log, TEXT("%s: sustained tired speed %.1f cm/s (A1 left side), sprint budget %.0f cm (A2 left side)."),
		*Row.Alias.ToString(), SustainedTiredSpeed, SprintBudget);

	for (const UCL9MonsterDef* Monster : OrderedMonsters)
	{
		const bool bCatcher = Monster->CatchRadius > 0.f;
		if (bCatcher && SustainedTiredSpeed <= Monster->ChaseSpeed)
		{
			UE_LOG(LogCL9, Error, TEXT("A1 broken: %s sustained tired speed %.1f <= %s chase %.1f — dropping everything no longer escapes."),
				*Row.Alias.ToString(), SustainedTiredSpeed, *Monster->Alias.ToString(), Monster->ChaseSpeed);
		}
		if (SprintBudget <= Monster->SightIn)
		{
			UE_LOG(LogCL9, Error, TEXT("A2 broken: %s sprint budget %.0f <= %s sight %.0f — a straight corridor never breaks sight."),
				*Row.Alias.ToString(), SprintBudget, *Monster->Alias.ToString(), Monster->SightIn);
		}
	}
	// A4 (운반 질주 vs 추격) waits on the weight→slowdown formula (economy.md 미결)
}

void UCL9DefRegistry::CheckItemReferences() const
{
	// Reference integrity (docs/data/schema.md 로드 검사): cook chains and
	// drops must point at rows that actually loaded.
	for (const UCL9ItemDef* Item : OrderedItems)
	{
		const FCL9ItemRef* Refs[] = { &Item->CookResult, &Item->BurnResult };
		for (const FCL9ItemRef* Ref : Refs)
		{
			if (Ref->IsSet() && FindItem(Ref->ItemAlias, Ref->ItemSubAlias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: cook chain points at '%s'/'%s' which is missing or broken."),
					*Item->GetName(), *Ref->ItemAlias.ToString(), *Ref->ItemSubAlias.ToString());
			}
		}
	}

	for (const UCL9MonsterDef* Monster : OrderedMonsters)
	{
		for (const FDropItem& Drop : Monster->DropItems)
		{
			if (Drop.IsSet() && FindItem(Drop.ItemAlias, Drop.ItemSubAlias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: drop points at '%s'/'%s' which is missing or broken."),
					*Monster->GetName(), *Drop.ItemAlias.ToString(), *Drop.ItemSubAlias.ToString());
			}
		}
	}
}
