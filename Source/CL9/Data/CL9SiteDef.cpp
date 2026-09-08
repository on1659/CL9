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

	if (Generator != ECL9Generator::None)
	{
		if (Theme.IsNone())
		{
			// 테마가 비면 모듈 표에서 한 행도 못 고른다 — 생성기가 아무것도 못 짓는다.
			OutErrors.Add(TEXT("Theme is empty — the generator has no module set to build from."));
		}
		if (CellSize <= 0.f)
		{
			OutErrors.Add(TEXT("CellSize must be positive."));
		}
		if (BaseModuleCount < 2)
		{
			OutErrors.Add(TEXT("BaseModuleCount must be at least 2 — one start module plus somewhere to go."));
		}
		if (EntryOffset < 0.f)
		{
			OutErrors.Add(TEXT("EntryOffset cannot be negative — the truck pad sits outside the grid."));
		}
	}
}
