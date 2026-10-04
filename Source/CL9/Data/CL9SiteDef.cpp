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
	auto CheckZones = [&OutErrors, &CheckEntries](const TArray<FCL9SiteZone>& ZoneRows, const FString& Prefix)
	{
		for (const FCL9SiteZone& Zone : ZoneRows)
		{
			if (Zone.DepthMin < 0.f || Zone.DepthMax > 1.f || Zone.DepthMin >= Zone.DepthMax)
			{
				OutErrors.Add(FString::Printf(TEXT("%sZone '%s' needs 0 <= DepthMin < DepthMax <= 1."), *Prefix, *Zone.Name.ToString()));
			}
			CheckEntries(Zone.MonsterSpawns, *FString::Printf(TEXT("%sZone '%s' MonsterSpawns"), *Prefix, *Zone.Name.ToString()));
			CheckEntries(Zone.ItemSpawns, *FString::Printf(TEXT("%sZone '%s' ItemSpawns"), *Prefix, *Zone.Name.ToString()));
		}
	};
	CheckZones(Zones, FString());

	// 구역 — 회전은 90° 배수만(격자와 비상구가 축 정렬이어야 한다), 판마다 시작 모듈 + 갈 데 하나는 있어야 한다
	if (Districts.Num() > 0 && Generator == ECL9Generator::None)
	{
		OutErrors.Add(TEXT("Districts need a Generator — a flat level has nothing to build per district."));
	}
	for (int32 Index = 0; Index < Districts.Num(); ++Index)
	{
		const FCL9SiteDistrict& District = Districts[Index];
		if (District.Yaw != 0 && District.Yaw != 90 && District.Yaw != 180 && District.Yaw != 270)
		{
			OutErrors.Add(FString::Printf(TEXT("District %d Yaw must be 0, 90, 180 or 270 (got %d)."), Index, District.Yaw));
		}
		if (District.ModuleCount < 2)
		{
			OutErrors.Add(FString::Printf(TEXT("District %d ModuleCount must be at least 2 — one start module plus somewhere to go."), Index));
		}
		CheckZones(District.Zones, FString::Printf(TEXT("District %d "), Index));
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

namespace
{
	/**
	 * 밤 길이 어림의 두 점 직선 (docs/systems/stage.md "밤 길이 어림": 양계장 본줄기 16모듈 · 245초, 대농장 60모듈 · 480초).
	 * 게임 값이 아니라 슬롯 검사용 어림식의 계수다 — 플레이로 다시 맞추면 stage.md 와 함께 고친다.
	 */
	constexpr float NightFitBase = 160.f;
	constexpr float NightFitPerMainModule = 5.3f;
}

uint32 CL9SiteDistrict::SubSeed(uint32 Seed, int32 Index)
{
	if (Seed == 0)
	{
		return 0;
	}
	const uint32 Sub = HashCombine(Seed, static_cast<uint32>(Index));
	return Sub != 0 ? Sub : 1u;
}

int32 CL9SiteDistrict::QuarterTurns(int32 Yaw)
{
	const int32 Quarter = FMath::RoundToInt(static_cast<float>(Yaw) / 90.f);
	return ((Quarter % 4) + 4) % 4;
}

FVector2D CL9SiteDistrict::Rotate(const FVector2D& Vector, int32 Quarter)
{
	// 정확히 90° 배수라 삼각함수를 안 쓴다 — 사각형 경계가 부동소수 오차 없이 맞닿는다
	switch (((Quarter % 4) + 4) % 4)
	{
	case 1: return FVector2D(-Vector.Y, Vector.X);
	case 2: return FVector2D(-Vector.X, -Vector.Y);
	case 3: return FVector2D(Vector.Y, -Vector.X);
	default: return Vector;
	}
}

int32 CL9SiteDistrict::GridSpan(int32 ModuleCount)
{
	// CL9SiteLayout::Generate 와 같은 식 (상한 128 포함) — 바꾸려면 둘을 함께
	return FMath::Clamp(FMath::CeilToInt(FMath::Sqrt(ModuleCount * 6.f)) * 2, 16, 128);
}

FBox2D CL9SiteDistrict::SpanRect(const FCL9SiteDistrict& District, int32 DayModuleCount, float CellSize, float EntryOffset)
{
	// 구역 로컬 격자 사각형 = 단일 사이트와 같다: X 는 EntryOffset 부터 한 변만큼, Y 는 0 을 가운데로
	const float Side = GridSpan(DayModuleCount) * CellSize;
	const int32 Quarter = QuarterTurns(District.Yaw);
	const FVector2D A = District.Offset + Rotate(FVector2D(EntryOffset, -Side * 0.5f), Quarter);
	const FVector2D B = District.Offset + Rotate(FVector2D(EntryOffset + Side, Side * 0.5f), Quarter);
	return FBox2D(FVector2D(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y)), FVector2D(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y)));
}

float CL9SiteDistrict::GateDistance(const FCL9SiteDistrict& District, float EntryOffset)
{
	return static_cast<float>((District.Offset + Rotate(FVector2D(EntryOffset, 0.f), QuarterTurns(District.Yaw))).Size());
}

float CL9SiteDistrict::NightEstimate(int32 BaseModules, float YardDistance, float MainPathShare, float YardSpeed)
{
	return NightFitBase + NightFitPerMainModule * MainPathShare * BaseModules + 2.f * YardDistance / FMath::Max(YardSpeed, 1.f);
}

bool CL9SiteDistrict::CheckSlots(const TArray<FCL9SiteDistrict>& Districts, const FCL9DistrictSlotRules& Rules, TArray<FString>& OutErrors)
{
	const int32 ErrorsBefore = OutErrors.Num();

	TArray<FBox2D> Rects;
	for (const FCL9SiteDistrict& District : Districts)
	{
		const int32 DayCount = FMath::Max(2, FMath::RoundToInt(District.ModuleCount * Rules.SizeMult));
		Rects.Add(SpanRect(District, DayCount, Rules.CellSize, Rules.EntryOffset));
	}

	for (int32 I = 0; I < Districts.Num(); ++I)
	{
		for (int32 J = I + 1; J < Districts.Num(); ++J)
		{
			// 맞닿기만 한 것은 겹침이 아니다 — 넓이가 있는 교집합만
			const FBox2D& RectA = Rects[I];
			const FBox2D& RectB = Rects[J];
			const double OverlapX = FMath::Min(RectA.Max.X, RectB.Max.X) - FMath::Max(RectA.Min.X, RectB.Min.X);
			const double OverlapY = FMath::Min(RectA.Max.Y, RectB.Max.Y) - FMath::Max(RectA.Min.Y, RectB.Min.Y);
			if (OverlapX > 0.0 && OverlapY > 0.0)
			{
				OutErrors.Add(FString::Printf(TEXT("Districts %d and %d overlap by %.0f x %.0f cm at the size multiplier %.2f."),
					I, J, OverlapX, OverlapY, Rules.SizeMult));
			}
		}

		const float Gate = GateDistance(Districts[I], Rules.EntryOffset);
		const float Night = NightEstimate(Districts[I].ModuleCount, Gate, Rules.MainPathShare, Rules.YardSpeed);
		if (Night > Rules.NightLength)
		{
			OutErrors.Add(FString::Printf(TEXT("District %d (%d modules, gate %.0f m) needs about %.0f s of a %.0f s night."),
				I, Districts[I].ModuleCount, Gate / 100.f, Night, Rules.NightLength));
		}
	}
	return OutErrors.Num() == ErrorsBefore;
}
