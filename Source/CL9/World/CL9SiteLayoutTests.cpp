// Copyright Epic Games, Inc. All Rights Reserved.

// The generator is the one piece every machine must agree on byte for byte —
// these guard determinism, connectivity and the openings knob (docs/systems/map.md).

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "World/CL9SiteLayout.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9SiteLayoutDeterminismTest,
	"CL9.Map.Determinism",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9SiteLayoutDeterminismTest::RunTest(const FString& Parameters)
{
	const FCL9HenhouseParams Params;
	const FCL9SiteLayout First = CL9SiteLayout::Generate(Params, 873);
	const FCL9SiteLayout Second = CL9SiteLayout::Generate(Params, 873);
	TestEqual(TEXT("same seed, same hash"), CL9SiteLayout::LayoutHash(First), CL9SiteLayout::LayoutHash(Second));
	TestTrue(TEXT("same seed, same 1F edges"), First.Floors[0].OpenH == Second.Floors[0].OpenH);
	TestTrue(TEXT("same seed, same B1 edges"), First.Floors[1].OpenV == Second.Floors[1].OpenV);
	TestNotEqual(TEXT("different seeds differ"), CL9SiteLayout::LayoutHash(First), CL9SiteLayout::LayoutHash(CL9SiteLayout::Generate(Params, 874)));

	const FCL9SiteLayout Shell = CL9SiteLayout::Generate(Params, 0);
	TestFalse(TEXT("seed 0 is the shell only"), Shell.bHasMaze);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9SiteLayoutConnectivityTest,
	"CL9.Map.Connectivity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9SiteLayoutConnectivityTest::RunTest(const FString& Parameters)
{
	const FCL9HenhouseParams Params;
	for (uint32 Seed = 1; Seed <= 300; ++Seed)
	{
		const FCL9SiteLayout Layout = CL9SiteLayout::Generate(Params, Seed);
		if (!CL9SiteLayout::IsConnected(Layout.Floors[0]) || !CL9SiteLayout::IsConnected(Layout.Floors[1]))
		{
			AddError(FString::Printf(TEXT("seed %u leaves cells unreachable"), Seed));
			return false;
		}
		if (Layout.Floors[0].CountOpen() != Params.TargetOpen1F || Layout.Floors[1].CountOpen() != Params.TargetOpenB1)
		{
			AddError(FString::Printf(TEXT("seed %u opened %d/%d, target %d/%d"), Seed,
				Layout.Floors[0].CountOpen(), Layout.Floors[1].CountOpen(), Params.TargetOpen1F, Params.TargetOpenB1));
			return false;
		}
		if (Layout.EastHoleAisle < 0 || Layout.EastHoleAisle == (Params.AisleCount1F - 1) / 2)
		{
			AddError(FString::Printf(TEXT("seed %u put the flap on the spine aisle"), Seed));
			return false;
		}
		if (Layout.Floors[0].JarCells.Num() != Params.JarCount1F || Layout.Floors[1].JarCells.Num() != Params.JarCountB1)
		{
			AddError(FString::Printf(TEXT("seed %u placed %d/%d jars"), Seed, Layout.Floors[0].JarCells.Num(), Layout.Floors[1].JarCells.Num()));
			return false;
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9SiteLayoutSegmentsTest,
	"CL9.Map.Segments",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9SiteLayoutSegmentsTest::RunTest(const FString& Parameters)
{
	const FCL9HenhouseParams Params;
	const FCL9SiteFrame Frame = CL9SiteLayout::ComputeFrame(Params);

	auto Count = [](const TArray<FCL9SiteSeg>& Segs, const FName& Kind)
	{
		int32 Result = 0;
		for (const FCL9SiteSeg& Seg : Segs)
		{
			Result += Seg.Kind == Kind ? 1 : 0;
		}
		return Result;
	};

	TArray<FCL9SiteSeg> Shell;
	CL9SiteLayout::BuildSegments(Params, Frame, CL9SiteLayout::Generate(Params, 0), Shell);
	TestEqual(TEXT("shell has no cages"), Count(Shell, CL9SegKind::Cage()), 0);
	TestEqual(TEXT("shell has no flap"), Count(Shell, CL9SegKind::FlapIn()), 0);
	TestTrue(TEXT("shell has floors"), Count(Shell, CL9SegKind::Floor()) > 0);
	TestEqual(TEXT("two stair walls"), Count(Shell, CL9SegKind::Stair()), 2);

	TArray<FCL9SiteSeg> Night;
	CL9SiteLayout::BuildSegments(Params, Frame, CL9SiteLayout::Generate(Params, 873), Night);
	TestEqual(TEXT("one flap"), Count(Night, CL9SegKind::FlapIn()), 1);
	TestTrue(TEXT("cages placed"), Count(Night, CL9SegKind::Cage()) > 0);
	TestTrue(TEXT("plugs placed"), Count(Night, CL9SegKind::Plug()) > 0);

	// Every footprint must have positive area and sit inside the boundary
	for (const FCL9SiteSeg& Seg : Night)
	{
		const FVector2D Size = Seg.Rect.GetSize();
		if (Size.X <= 0.f || Size.Y <= 0.f)
		{
			AddError(FString::Printf(TEXT("%s has a degenerate footprint"), *Seg.Kind.ToString()));
			return false;
		}
		if (Seg.Rect.Min.X < Frame.World.Min.X - Frame.Wall - 1.f || Seg.Rect.Max.X > Frame.World.Max.X + Frame.Wall + 1.f)
		{
			AddError(FString::Printf(TEXT("%s leaves the boundary"), *Seg.Kind.ToString()));
			return false;
		}
	}

	// The spine cell of 1F faces the door: Y = 0
	const FVector2D Spine = CL9SiteLayout::CellCenter(Params, Frame, 0, Frame.Spine[0], 0);
	TestTrue(TEXT("spine aisle centred on the door"), FMath::IsNearlyZero(Spine.Y, 0.01f));
	return true;
}

#endif
