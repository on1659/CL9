// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9SiteDef.h"

void UCL9SiteDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	auto CheckEntries = [&OutErrors](const TArray<FCL9SpawnEntry>& Entries, const TCHAR* Column)
	{
		for (const FCL9SpawnEntry& Entry : Entries)
		{
			if (Entry.Alias.IsNone())
			{
				OutErrors.Add(FString::Printf(TEXT("%s has an entry without an alias."), Column));
			}
			if (Entry.MinCount < 0 || Entry.MaxCount < Entry.MinCount)
			{
				OutErrors.Add(FString::Printf(TEXT("%s '%s' needs 0 <= MinCount <= MaxCount."), Column, *Entry.Alias.ToString()));
			}
		}
	};
	CheckEntries(MonsterSpawns, TEXT("MonsterSpawns"));
	CheckEntries(ItemSpawns, TEXT("ItemSpawns"));

	if (BaseDanger <= 0.f)
	{
		OutErrors.Add(TEXT("BaseDanger must be positive — the day curve multiplies it."));
	}
}
