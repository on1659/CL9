// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9DefRegistry.h"
#include "CL9.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9RulesDef.h"

void UCL9DefRegistry::Init(const UCL9DefList* DefList)
{
	if (DefList == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("Def registry got no def list — every table lookup will fail (docs/data/schema.md)."));
		return;
	}

	for (const TSoftObjectPtr<UCL9DefBase>& DefRef : DefList->Defs)
	{
		const UCL9DefBase* Def = DefRef.LoadSynchronous();
		if (Def == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("Def list entry '%s' does not resolve — row skipped."), *DefRef.ToString());
			BrokenRows.Add(FString::Printf(TEXT("%s ✗ (unresolved reference)"), *DefRef.ToString()));
			continue;
		}

		TArray<FString> Errors;
		Def->Validate(Errors);
		if (Errors.Num() > 0)
		{
			for (const FString& Error : Errors)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: broken row — %s"), *Def->GetName(), *Error);
			}
			BrokenRows.Add(FString::Printf(TEXT("%s ✗ (%s)"), *Def->GetName(), *Errors[0]));
			continue;
		}

		LoadedDefs.Add(Def);

		const TPair<FName, FName> Key(Def->Alias, Def->SubAlias);

		if (const UCL9CharacterDef* CharacterDef = Cast<UCL9CharacterDef>(Def))
		{
			if (Characters.Contains(Key))
			{
				UE_LOG(LogCL9, Error, TEXT("%s: duplicate character key '%s'/'%s' — the first row stays."),
					*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
				continue;
			}
			Characters.Add(Key, CharacterDef);
			OrderedCharacters.Add(CharacterDef);
			CheckCharacterRelations(*CharacterDef);
		}
		else if (const UCL9ItemDef* ItemDef = Cast<UCL9ItemDef>(Def))
		{
			if (Items.Contains(Key))
			{
				UE_LOG(LogCL9, Error, TEXT("%s: duplicate item key '%s'/'%s' — the first row stays."),
					*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
				continue;
			}
			Items.Add(Key, ItemDef);
			OrderedItems.Add(ItemDef);
		}
		else if (const UCL9RulesDef* RulesDef = Cast<UCL9RulesDef>(Def))
		{
			if (Rules != nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: a second rules row — the first one stays (the rules asset is singular)."), *Def->GetName());
				continue;
			}
			Rules = RulesDef;
		}
	}

	CheckItemReferences();

	if (Rules == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("No usable rules row loaded — carry/throw rulings will refuse to run (docs/plan/data/rules.md)."));
	}
}

const UCL9CharacterDef* UCL9DefRegistry::FindCharacter(FName Alias, FName SubAlias) const
{
	const UCL9CharacterDef* const* Found = Characters.Find(TPair<FName, FName>(Alias, SubAlias));
	return Found ? *Found : nullptr;
}

const UCL9ItemDef* UCL9DefRegistry::FindItem(FName Alias, FName SubAlias) const
{
	const UCL9ItemDef* const* Found = Items.Find(TPair<FName, FName>(Alias, SubAlias));
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::CheckCharacterRelations(const UCL9CharacterDef& Row) const
{
	// Derived values from docs/plan/economy.md. Both right-hand sides (catch
	// monster chase speed for A1, sight radius for A2) live in the monster
	// table, which arrives in build-order ③ — until then the derived numbers
	// are logged so a bad edit is still visible.
	const float CycleTime = Row.StaminaDrainTime + Row.StaminaRegenTime;
	const float SustainedTiredSpeed = (Row.RunSpeed * Row.StaminaDrainTime + Row.WalkSpeed * Row.StaminaRegenTime) / FMath::Max(CycleTime, UE_KINDA_SMALL_NUMBER);
	const float SprintBudget = Row.RunSpeed * Row.StaminaDrainTime;

	UE_LOG(LogCL9, Log, TEXT("%s: sustained tired speed %.1f cm/s (A1 left side), sprint budget %.0f cm (A2 left side) — monster table pending, comparisons activate in step ③."),
		*Row.Alias.ToString(), SustainedTiredSpeed, SprintBudget);
}

void UCL9DefRegistry::CheckItemReferences() const
{
	// Reference integrity (docs/data/schema.md 로드 검사): cook chains must
	// point at rows that actually loaded, or cooking would silently eat items.
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
}
