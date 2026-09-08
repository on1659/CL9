// Copyright Epic Games, Inc. All Rights Reserved.

// 생성기는 모든 머신이 바이트 단위로 완전히 일치해야 하는 유일한 부분이다 —
// 아래 테스트들은 결정성, 연결성, 칸 점유, 그리고 껍데기가 실제로 서는지를
// 검증한다 (docs/plan/data/module.md · docs/systems/map.md).

#include "CoreMinimal.h"
#include "Data/CL9ModuleDef.h"
#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"
#include "World/CL9SiteLayout.h"

#if WITH_AUTOMATION_TESTS

namespace
{
	// 한 글자 이름은 엔진 템플릿의 인자를 가려서 C4459가 난다
	constexpr uint8 SockN = 1 << 0;
	constexpr uint8 SockE = 1 << 1;
	constexpr uint8 SockS = 1 << 2;
	constexpr uint8 SockW = 1 << 3;

	/**
	 * 양계장 16종을 본뜬 시험용 모듈 세트 (module.md의 표). 데이터 애셋을 읽지 않는 것은
	 * 일부러다 — 표를 손보다 생긴 실수까지 여기서 빨간불이 되면 생성기 자체의 회귀를
	 * 구분할 수 없다.
	 */
	struct FTestModules
	{
		TArray<TStrongObjectPtr<UCL9ModuleDef>> Owned;
		TArray<const UCL9ModuleDef*> Defs;

		void Add(const TCHAR* Alias, ECL9ModuleType Type, int32 SizeX, int32 SizeY, uint8 Sockets, bool bOutdoor = false, int32 Purposes = 0xB)
		{
			UCL9ModuleDef* Def = NewObject<UCL9ModuleDef>(GetTransientPackage());
			Def->Alias = FName(Alias);
			Def->SubAlias = TEXT("henhouse");
			Def->Type = Type;
			Def->Size = FIntPoint(SizeX, SizeY);
			Def->Sockets = Sockets;
			Def->Rarity = 2;
			Def->Height = 400.f;
			Def->bOutdoor = bOutdoor;
			Def->PointPurposes = Purposes;
			Owned.Add(TStrongObjectPtr<UCL9ModuleDef>(Def));
			Defs.Add(Def);
		}

		FTestModules()
		{
			Add(TEXT("yard_gate"), ECL9ModuleType::Start, 3, 2, SockN | SockE | SockS, /*bOutdoor*/ true, /*Purposes*/ 0);
			Add(TEXT("corridor"), ECL9ModuleType::Passage, 1, 2, SockN | SockS);
			Add(TEXT("corner"), ECL9ModuleType::Passage, 2, 2, SockN | SockE);
			Add(TEXT("stair_pass"), ECL9ModuleType::Passage, 2, 2, SockN | SockS);
			Add(TEXT("coop_hall"), ECL9ModuleType::Normal, 3, 3, SockN | SockE | SockS | SockW);
			Add(TEXT("pen"), ECL9ModuleType::Normal, 3, 2, SockN | SockE | SockS | SockW);
			Add(TEXT("feed_store"), ECL9ModuleType::Normal, 2, 3, SockN | SockE | SockW);
			Add(TEXT("cross"), ECL9ModuleType::Normal, 2, 2, SockN | SockE | SockS | SockW);
			Add(TEXT("yard_open"), ECL9ModuleType::Normal, 3, 3, SockN | SockE | SockS | SockW, /*bOutdoor*/ true);
			Add(TEXT("incubator"), ECL9ModuleType::DeadEnd, 2, 2, SockS);
			Add(TEXT("tool_room"), ECL9ModuleType::DeadEnd, 2, 2, SockS);
			Add(TEXT("storage"), ECL9ModuleType::DeadEnd, 2, 2, SockW);
			Add(TEXT("office"), ECL9ModuleType::DeadEnd, 2, 2, SockN);
			Add(TEXT("coop_small"), ECL9ModuleType::DeadEnd, 2, 2, SockE);
			Add(TEXT("delivery"), ECL9ModuleType::Special, 2, 2, SockN | SockS, /*bOutdoor*/ false, /*Purposes*/ 0xF);
			Add(TEXT("kitchen"), ECL9ModuleType::Special, 2, 3, SockE | SockW);
		}
	};

	/** 격자의 칸 점유가 배치 목록과 어긋나지 않는가 — 겹침이 있으면 여기서 드러난다 */
	int32 CountOccupiedCells(const FCL9SiteLayout& Layout)
	{
		int32 Count = 0;
		for (int32 Module : Layout.CellModule)
		{
			if (Module != INDEX_NONE)
			{
				++Count;
			}
		}
		return Count;
	}

	int32 CountEdges(const FCL9SiteLayout& Layout, ECL9GridEdge State)
	{
		int32 Count = 0;
		for (uint8 Edge : Layout.EdgeX)
		{
			Count += (Edge == static_cast<uint8>(State)) ? 1 : 0;
		}
		for (uint8 Edge : Layout.EdgeY)
		{
			Count += (Edge == static_cast<uint8>(State)) ? 1 : 0;
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9SiteLayoutDeterminismTest,
	"CL9.Map.Determinism",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9SiteLayoutDeterminismTest::RunTest(const FString& Parameters)
{
	const FTestModules Modules;
	FCL9SiteParams Params;
	Params.Theme = TEXT("henhouse");

	const FCL9SiteLayout First = CL9SiteLayout::Generate(Params, Modules.Defs, 873);
	const FCL9SiteLayout Second = CL9SiteLayout::Generate(Params, Modules.Defs, 873);
	TestTrue(TEXT("seed 873 builds something"), First.bBuilt);
	TestEqual(TEXT("same seed, same hash"), CL9SiteLayout::LayoutHash(First), CL9SiteLayout::LayoutHash(Second));
	TestTrue(TEXT("same seed, same occupancy"), First.CellModule == Second.CellModule);
	TestTrue(TEXT("same seed, same edges"), First.EdgeX == Second.EdgeX && First.EdgeY == Second.EdgeY);
	TestEqual(TEXT("same seed, same points"), First.Points.Num(), Second.Points.Num());

	const FCL9SiteLayout Other = CL9SiteLayout::Generate(Params, Modules.Defs, 874);
	TestNotEqual(TEXT("different seeds differ"), CL9SiteLayout::LayoutHash(First), CL9SiteLayout::LayoutHash(Other));

	const FCL9SiteLayout Nothing = CL9SiteLayout::Generate(Params, Modules.Defs, 0);
	TestFalse(TEXT("seed 0 builds nothing"), Nothing.bBuilt);

	// 시작 모듈이 없으면 붙일 곳이 없다 — 조용히 빈 맵이 나오는 대신 안 지어야 한다
	TArray<const UCL9ModuleDef*> NoStart = Modules.Defs;
	NoStart.RemoveAll([](const UCL9ModuleDef* Def) { return Def->Type == ECL9ModuleType::Start; });
	TestFalse(TEXT("no start module means nothing is built"), CL9SiteLayout::Generate(Params, NoStart, 873).bBuilt);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9SiteLayoutAssemblyTest,
	"CL9.Map.Assembly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9SiteLayoutAssemblyTest::RunTest(const FString& Parameters)
{
	const FTestModules Modules;
	FCL9SiteParams Params;
	Params.Theme = TEXT("henhouse");

	int32 Disconnected = 0;
	int32 Overlapped = 0;
	int32 NoExit = 0;
	int32 Undersized = 0;
	int32 Smallest = MAX_int32;
	int32 Largest = 0;
	float Total = 0.f;
	float DeadEnds = 0.f;
	float Loops = 0.f;
	int32 Runs = 0;

	for (uint32 Seed = 1; Seed <= 200; ++Seed)
	{
		const FCL9SiteLayout Layout = CL9SiteLayout::Generate(Params, Modules.Defs, Seed);
		if (!Layout.bBuilt)
		{
			++Undersized;
			continue;
		}

		if (!CL9SiteLayout::IsConnected(Layout))
		{
			++Disconnected;
		}

		int32 Expected = 0;
		for (const FCL9ModulePlacement& Module : Layout.Modules)
		{
			Expected += Module.Size.X * Module.Size.Y;
		}
		if (Expected != CountOccupiedCells(Layout))
		{
			++Overlapped;
		}

		// 비상구는 정확히 하나다 (module.md 결정 3)
		if (CountEdges(Layout, ECL9GridEdge::Flap) != 1)
		{
			++NoExit;
		}

		Smallest = FMath::Min(Smallest, Layout.Modules.Num());
		Largest = FMath::Max(Largest, Layout.Modules.Num());
		Total += Layout.Modules.Num();
		for (const FCL9ModulePlacement& Module : Layout.Modules)
		{
			DeadEnds += Module.Def->Type == ECL9ModuleType::DeadEnd ? 1 : 0;
		}
		Loops += CountEdges(Layout, ECL9GridEdge::Door) - (Layout.Modules.Num() - 1) - 1;
		++Runs;
	}

	// 크기와 모양은 통과·실패로 나누기 어렵고 값을 보면서 손잡이를 돌려야 한다 — 숫자를 남긴다
	AddInfo(FString::Printf(TEXT("%d runs: %d~%d modules (avg %.1f of a wanted %d), %.0f%% dead ends, %.1f loop doors per run."),
		Runs, Smallest, Largest, Total / FMath::Max(Runs, 1), Params.ModuleCount,
		100.f * DeadEnds / FMath::Max(Total, 1.f), Loops / FMath::Max(Runs, 1)));

	TestEqual(TEXT("every module is reachable through doors"), Disconnected, 0);
	TestEqual(TEXT("no two modules share a cell"), Overlapped, 0);
	TestEqual(TEXT("exactly one emergency exit per run"), NoExit, 0);
	TestEqual(TEXT("every seed builds"), Undersized, 0);
	// 프론티어가 일찍 마르면 판이 확 작아진다 — 목표의 절반은 넘어야 한 판이라 할 수 있다
	TestTrue(FString::Printf(TEXT("the smallest run had %d modules of a wanted %d"), Smallest, Params.ModuleCount),
		Smallest >= Params.ModuleCount / 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9SiteLayoutSegmentsTest,
	"CL9.Map.Segments",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9SiteLayoutSegmentsTest::RunTest(const FString& Parameters)
{
	const FTestModules Modules;
	FCL9SiteParams Params;
	Params.Theme = TEXT("henhouse");

	TArray<FCL9SiteSeg> Segs;
	CL9SiteLayout::BuildSegments(Params, CL9SiteLayout::Generate(Params, Modules.Defs, 0), Segs);
	TestEqual(TEXT("an unbuilt layout places no boxes"), Segs.Num(), 0);

	const FCL9SiteLayout Layout = CL9SiteLayout::Generate(Params, Modules.Defs, 873);
	CL9SiteLayout::BuildSegments(Params, Layout, Segs);

	TMap<FName, int32> ByKind;
	for (const FCL9SiteSeg& Seg : Segs)
	{
		++ByKind.FindOrAdd(Seg.Kind);
		TestTrue(TEXT("every box has area"), Seg.Rect.GetSize().X > 0.f && Seg.Rect.GetSize().Y > 0.f);
	}

	// 바닥은 모듈마다 한 장이고, 천장은 야외 모듈을 뺀 만큼이다
	int32 Indoor = 0;
	for (const FCL9ModulePlacement& Module : Layout.Modules)
	{
		Indoor += Module.Def->bOutdoor ? 0 : 1;
	}
	TestEqual(TEXT("one floor slab per module"), ByKind.FindRef(CL9SegKind::Floor()), Layout.Modules.Num());
	TestEqual(TEXT("one ceiling per indoor module"), ByKind.FindRef(CL9SegKind::Ceiling()), Indoor);
	TestEqual(TEXT("one emergency exit"), ByKind.FindRef(CL9SegKind::FlapIn()), 1);
	TestTrue(TEXT("the outer wall is sealed"), ByKind.FindRef(CL9SegKind::Barn()) + ByKind.FindRef(CL9SegKind::Wall()) > 0);
	// 입구 하나 + 모듈을 이은 문들 (module.md 결정 3)
	TestTrue(TEXT("there are doorways"), ByKind.FindRef(CL9SegKind::Door()) >= Layout.Modules.Num());

	// 지점은 배치된 칸 수를 넘지 않는다 — 용도가 0인 모듈은 지점을 안 낸다
	TestTrue(TEXT("points do not outnumber cells"), Layout.Points.Num() <= CountOccupiedCells(Layout));

	TArray<int32> Jars;
	CL9SiteLayout::PointsOfPurpose(Layout, ECL9PointPurpose::Jar, Jars);
	TestTrue(TEXT("the delivery room offers a jar spot"), Jars.Num() > 0);

	TArray<int32> MonsterPoints;
	CL9SiteLayout::PointsOfPurpose(Layout, ECL9PointPurpose::Monster, MonsterPoints);
	TestTrue(TEXT("monster spots are sorted deepest first"),
		MonsterPoints.Num() < 2 || Layout.Points[MonsterPoints[0]].Hops >= Layout.Points[MonsterPoints.Last()].Hops);
	return true;
}

#endif
