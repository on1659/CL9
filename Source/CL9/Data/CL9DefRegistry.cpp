// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9DefRegistry.h"
#include "CL9.h"
#include "Data/CL9CharacterDef.h"

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
			continue;
		}

		LoadedDefs.Add(Def);

		if (const UCL9CharacterDef* CharacterDef = Cast<UCL9CharacterDef>(Def))
		{
			const TPair<FName, FName> Key(CharacterDef->Alias, CharacterDef->SubAlias);
			if (Characters.Contains(Key))
			{
				UE_LOG(LogCL9, Error, TEXT("%s: duplicate character key '%s'/'%s' — the first row stays."),
					*CharacterDef->GetName(), *CharacterDef->Alias.ToString(), *CharacterDef->SubAlias.ToString());
				continue;
			}
			Characters.Add(Key, CharacterDef);
			CheckCharacterRelations(*CharacterDef);
		}
	}
}

const UCL9CharacterDef* UCL9DefRegistry::FindCharacter(FName Alias, FName SubAlias) const
{
	const UCL9CharacterDef* const* Found = Characters.Find(TPair<FName, FName>(Alias, SubAlias));
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
