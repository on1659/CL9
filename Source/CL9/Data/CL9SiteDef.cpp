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
		if (AisleCount < 1 || ColCount < 1 || AisleCountB1 < 1 || ColCountB1 < 1)
		{
			OutErrors.Add(TEXT("Grid counts must be at least 1 on both floors."));
		}
		if (AisleWidth <= 0.f || ColWidth <= 0.f || RowDepth <= 0.f)
		{
			OutErrors.Add(TEXT("AisleWidth, ColWidth and RowDepth must be positive."));
		}
		if (OpenWidth <= 0.f || OpenWidth >= ColWidth)
		{
			OutErrors.Add(TEXT("OpenWidth must be positive and narrower than ColWidth."));
		}
		if (PlugWidth <= 0.f || PlugWidth >= ColWidth)
		{
			OutErrors.Add(TEXT("PlugWidth must be positive and narrower than ColWidth."));
		}
		if (TargetOpen1F < 0 || TargetOpenB1 < 0 || JarCount1F < 0 || JarCountB1 < 0)
		{
			OutErrors.Add(TEXT("TargetOpen and JarCount cannot be negative."));
		}
	}
}
