// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9SiteDef.h"

UCL9SiteDef::UCL9SiteDef()
{
	// 새 행의 출발값 = light.md의 어둠 톤 (2026-09-12 PIE로 맞춘 값). 로비는 어스름, 밤은
	// 달빛 청색의 약한 앰비언트 + 옅은 안개(리썰컴퍼니식 "검정의 색"), 정전은 앰비언트를 끈다.
	// 밤 SkyScale이 큰 이유는 헤더의 설명대로 — 실시간 캡처 하늘이 태양을 따라 어두워진다.
	LobbyLighting.SunScale = 0.35f;
	LobbyLighting.SkyScale = 0.5f;
	LobbyLighting.SkyLeak = 1.f;

	NightLighting.SunScale = 0.05f;
	NightLighting.SunColor = FLinearColor(0.35f, 0.45f, 0.75f, 1.f);
	NightLighting.SkyScale = 10.f;
	NightLighting.SkyColor = FLinearColor(0.55f, 0.7f, 1.f, 1.f);
	NightLighting.SkyLeak = 1.f;
	NightLighting.FogDensity = 0.01f;
	NightLighting.FogColor = FLinearColor(0.02f, 0.03f, 0.06f, 1.f);
	// 밤에는 보이는 하늘도 눌러야 문밖이 하얗게 타지 않는다 (2026-09-16, 초안 — PIE 로 맞춘다)
	NightLighting.SkyLuminance = 0.15f;

	BlackoutLighting.SunScale = 0.f;
	BlackoutLighting.SunColor = NightLighting.SunColor;
	BlackoutLighting.SkyScale = 0.f;
	BlackoutLighting.SkyColor = NightLighting.SkyColor;
	BlackoutLighting.SkyLeak = 0.f;
	BlackoutLighting.FogDensity = 0.01f;
	BlackoutLighting.FogColor = FLinearColor::Black;
	BlackoutLighting.SkyLuminance = 0.05f;
}

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
	for (const FCL9SiteZone& Zone : Zones)
	{
		if (Zone.DepthMin < 0.f || Zone.DepthMax > 1.f || Zone.DepthMin >= Zone.DepthMax)
		{
			OutErrors.Add(FString::Printf(TEXT("Zone '%s' needs 0 <= DepthMin < DepthMax <= 1."), *Zone.Name.ToString()));
		}
		CheckEntries(Zone.MonsterSpawns, *FString::Printf(TEXT("Zone '%s' MonsterSpawns"), *Zone.Name.ToString()));
		CheckEntries(Zone.ItemSpawns, *FString::Printf(TEXT("Zone '%s' ItemSpawns"), *Zone.Name.ToString()));
	}

	// 덮어쓴 밤 시계끼리는 순서가 맞아야 한다 — 전역값과 섞인 조합은 규칙 행의 검사가 맡는다
	if (NightLength < 0.f || DarkBase < 0.f || DarkMin < 0.f)
	{
		OutErrors.Add(TEXT("NightLength, DarkBase and DarkMin cannot be negative (0 = use CL9Rules)."));
	}
	if (DarkBase > 0.f && NightLength > 0.f && DarkBase > NightLength)
	{
		OutErrors.Add(TEXT("DarkBase must not exceed NightLength — the blackout would never come."));
	}
	if (DarkMin > 0.f && DarkBase > 0.f && DarkMin > DarkBase)
	{
		OutErrors.Add(TEXT("DarkMin must not exceed DarkBase — returns can only bring the blackout earlier."));
	}

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
		if (MaxFloors < 1)
		{
			OutErrors.Add(TEXT("MaxFloors must be at least 1 — a site always has a ground floor."));
		}
		if (MaxFloors > 1 && (StairCountMin < 1 || StairCountMax < StairCountMin))
		{
			// 층이 둘 이상인데 계단이 없으면 위층으로 올라갈 길이 없다.
			OutErrors.Add(TEXT("StairCountMin must be at least 1 and no greater than StairCountMax when MaxFloors > 1."));
		}
		if (LoopChance < 0.f || LoopChance > 1.f || MainPathShare < 0.f || MainPathShare > 1.f)
		{
			OutErrors.Add(TEXT("LoopChance and MainPathShare must be between 0 and 1."));
		}
		if (UpperFloorShare < 0.f || UpperFloorShare > 1.f)
		{
			OutErrors.Add(TEXT("UpperFloorShare must be between 0 and 1 — an upper floor never gets more than the floor below."));
		}
		if (PassageShare < 0.f || DeadEndShare < 0.f || PassageShare + DeadEndShare > 1.f)
		{
			// 둘의 합이 1을 넘으면 방(Normal) 몫이 음수가 되어 곁가지에 방이 하나도 안 선다.
			OutErrors.Add(TEXT("PassageShare and DeadEndShare must be non-negative and sum to at most 1 — the rest is the Normal share."));
		}
	}
}
