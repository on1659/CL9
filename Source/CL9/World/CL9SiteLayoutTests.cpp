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
			// 계단실은 아래층과 위층의 같은 칸을 함께 차지한다 — MaxFloors가 1이면 안 뽑힌다.
			Add(TEXT("stair_up"), ECL9ModuleType::Stair, 2, 2, SockN | SockE | SockS | SockW);
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

	// 바닥은 모듈마다 한 장 — 실내는 floor, 마당은 yard — 이고, 천장은 야외 모듈을 뺀 만큼이다
	int32 Indoor = 0;
	for (const FCL9ModulePlacement& Module : Layout.Modules)
	{
		Indoor += Module.Def->bOutdoor ? 0 : 1;
	}
	TestEqual(TEXT("one floor slab per module"), ByKind.FindRef(CL9SegKind::Floor()) + ByKind.FindRef(CL9SegKind::Yard()), Layout.Modules.Num());
	TestEqual(TEXT("yards only under outdoor modules"), ByKind.FindRef(CL9SegKind::Yard()), Layout.Modules.Num() - Indoor);
	TestEqual(TEXT("one ceiling per indoor module"), ByKind.FindRef(CL9SegKind::Ceiling()), Indoor);

	// 문 양옆 토막은 온전한 벽과 종류가 다르다 — 문마다 둘씩
	TestEqual(TEXT("two stubs per doorway"), ByKind.FindRef(CL9SegKind::BarnStub()) + ByKind.FindRef(CL9SegKind::WallStub()),
		2 * (ByKind.FindRef(CL9SegKind::Door()) + ByKind.FindRef(CL9SegKind::FlapIn())));

	// 천장 스위치를 끄면 천장만 사라지고 바닥·벽은 그대로다 (site.md bBuildCeilings)
	{
		FCL9SiteParams NoCeiling = Params;
		NoCeiling.bBuildCeilings = false;
		TArray<FCL9SiteSeg> Bare;
		CL9SiteLayout::BuildSegments(NoCeiling, Layout, Bare);
		int32 Ceilings = 0;
		int32 Floors = 0;
		for (const FCL9SiteSeg& Seg : Bare)
		{
			Ceilings += Seg.Kind == CL9SegKind::Ceiling() ? 1 : 0;
			Floors += Seg.Kind == CL9SegKind::Floor() ? 1 : 0;
		}
		TestEqual(TEXT("ceilings off means no ceiling boxes"), Ceilings, 0);
		TestEqual(TEXT("ceilings off leaves the floors alone"), Floors, ByKind.FindRef(CL9SegKind::Floor()));
	}
	TestEqual(TEXT("one emergency exit"), ByKind.FindRef(CL9SegKind::FlapIn()), 1);
	TestTrue(TEXT("the outer wall is sealed"), ByKind.FindRef(CL9SegKind::Barn()) + ByKind.FindRef(CL9SegKind::Wall()) > 0);
	// 입구 하나 + 모듈을 이은 문들 (module.md 결정 3)
	TestTrue(TEXT("there are doorways"), ByKind.FindRef(CL9SegKind::Door()) >= Layout.Modules.Num());

	// 소품이 모듈을 따라 도는 변환은 소켓 회전과 같은 뜻이어야 한다 — 회전 전 East 변의
	// 한가운데 점이 회전 r 뒤 RotateSide(East, r) 변의 한가운데로 가야 한다.
	{
		FCL9ModulePlacement Module;
		Module.Def = Modules.Defs[4]; // coop_hall 3×3
		Module.Origin = FIntPoint(2, 1);
		const FVector2D Cell(Params.CellSize, Params.CellSize);
		for (int32 Rotation = 0; Rotation < 4; ++Rotation)
		{
			Module.Rotation = Rotation;
			Module.Size = UCL9ModuleDef::RotateSize(Module.Def->Size, Rotation);
			const FVector2D Min = Layout.Origin + FVector2D(Module.Origin.X, Module.Origin.Y) * Cell;
			const FVector2D Max = Min + FVector2D(Module.Size.X, Module.Size.Y) * Cell;
			const FVector2D Mid = (Min + Max) * 0.5f;
			const FVector2D EastMid(Module.Def->Size.X * Params.CellSize, Module.Def->Size.Y * Params.CellSize * 0.5f);
			const FVector2D Moved = CL9SiteLayout::ModuleLocalToSite(Params, Layout, Module, EastMid);
			const int32 Side = CL9SiteLayout::RotateSide(/*East*/ 1, Rotation);
			const FVector2D Expected = Side == 0 ? FVector2D(Mid.X, Max.Y) : Side == 1 ? FVector2D(Max.X, Mid.Y)
				: Side == 2 ? FVector2D(Mid.X, Min.Y) : FVector2D(Min.X, Mid.Y);
			TestTrue(FString::Printf(TEXT("module-local East midpoint follows rotation %d"), Rotation), Moved.Equals(Expected, 1.f));
			// 같은 회전으로 소켓을 돌리면 그 변만 켜져 있어야 한다
			const uint8 Sockets = UCL9ModuleDef::RotateSockets(static_cast<uint8>(SockE), Rotation);
			TestEqual(FString::Printf(TEXT("RotateSide agrees with RotateSockets at %d"), Rotation), static_cast<int32>(Sockets), 1 << Side);
		}
	}

	// 디딤판은 바닥처럼 기준점 아래로 자라는 종류이고, 위쪽 면이 계단 곡선을 따라야 한다
	{
		const float Storey = FMath::Max(Params.FloorHeight, 1.f);
		for (const FCL9SiteSeg& Seg : Segs)
		{
			if (Seg.Kind != CL9SegKind::Tread())
			{
				continue;
			}
			TestTrue(TEXT("tread kind grows down"), CL9SegKind::GrowsDown(Seg.Kind));
			const float Rel = FMath::Fmod(Seg.BaseZ, Storey);
			TestTrue(TEXT("tread top sits between the floors"), Seg.BaseZ > 0.f && (Rel > 1.f || FMath::IsNearlyEqual(Rel, 0.f, 1.f)));
		}
	}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9SiteLayoutFloorsTest,
	"CL9.Map.Floors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9SiteLayoutFloorsTest::RunTest(const FString& Parameters)
{
	const FTestModules Modules;
	FCL9SiteParams Params;
	Params.Theme = TEXT("henhouse");

	// 계단 모듈이 한 행도 없으면 층이 몇이든 단층이어야 한다 — 올라갈 길이 없으니까.
	{
		FCL9SiteParams Tall = Params;
		Tall.MaxFloors = 3;
		TArray<const UCL9ModuleDef*> NoStairs = Modules.Defs;
		NoStairs.RemoveAll([](const UCL9ModuleDef* Def) { return Def->Type == ECL9ModuleType::Stair; });
		TestEqual(TEXT("no stair row means one floor"), CL9SiteLayout::Generate(Tall, NoStairs, 873).FloorCount, 1);
	}

	// MaxFloors 1은 예전 그대로여야 한다 — 층을 넣었다고 단층 판이 달라지면 회귀다.
	TestEqual(TEXT("MaxFloors 1 stays single-storey"),
		CL9SiteLayout::Generate(Params, Modules.Defs, 873).FloorCount, 1);

	Params.MaxFloors = 3;
	Params.StairCountMin = 2;
	Params.StairCountMax = 4;

	int32 Disconnected = 0;
	int32 Overlapped = 0;
	int32 Unsupported = 0;
	int32 Floating = 0;
	int32 Stranded = 0;
	int32 TwoOrMore = 0;
	int32 ThreeFloors = 0;
	float Stairs = 0.f;
	float Upstairs = 0.f;
	float ReachSum = 0.f;
	int32 Runs = 0;

	// 계단 위층에서 **내려오지 않고** 문만으로 닿는 모듈 수 — 계단 자신은 빼고 센다
	auto UpperReach = [](const FCL9SiteLayout& Layout, int32 StairIndex)
	{
		const int32 Upper = Layout.Modules[StairIndex].Floor + 1;
		TSet<int32> Seen;
		TArray<int32> Queue;
		Seen.Add(StairIndex);
		Queue.Add(StairIndex);
		while (Queue.Num() > 0)
		{
			const FCL9ModulePlacement Module = Layout.Modules[Queue.Pop(EAllowShrinking::No)];
			// 이 모듈이 위층에 없으면(다른 층의 계단 등) 거기서 멈춘다
			if (Module.Floor > Upper || Module.TopFloor() < Upper)
			{
				continue;
			}
			for (int32 Side = 0; Side < 4; ++Side)
			{
				if (!(Module.Sockets & (1 << Side)))
				{
					continue;
				}
				const int32 Count = (Side == 0 || Side == 2) ? Module.Size.X : Module.Size.Y;
				for (int32 Index = 0; Index < Count; ++Index)
				{
					FIntPoint Cell;
					switch (Side)
					{
					case 0: Cell = FIntPoint(Module.Origin.X + Index, Module.Origin.Y + Module.Size.Y - 1); break;
					case 2: Cell = FIntPoint(Module.Origin.X + Index, Module.Origin.Y); break;
					case 1: Cell = FIntPoint(Module.Origin.X + Module.Size.X - 1, Module.Origin.Y + Index); break;
					default: Cell = FIntPoint(Module.Origin.X, Module.Origin.Y + Index); break;
					}
					ECL9GridEdge Edge;
					FIntPoint Next;
					switch (Side)
					{
					case 1: Edge = Layout.EdgeAtX(Upper, Cell.X + 1, Cell.Y); Next = Cell + FIntPoint(1, 0); break;
					case 3: Edge = Layout.EdgeAtX(Upper, Cell.X, Cell.Y); Next = Cell + FIntPoint(-1, 0); break;
					case 0: Edge = Layout.EdgeAtY(Upper, Cell.X, Cell.Y + 1); Next = Cell + FIntPoint(0, 1); break;
					default: Edge = Layout.EdgeAtY(Upper, Cell.X, Cell.Y); Next = Cell + FIntPoint(0, -1); break;
					}
					if (Edge != ECL9GridEdge::Door)
					{
						continue;
					}
					const int32 Other = Layout.CellAt(Upper, Next);
					if (Other != INDEX_NONE && !Seen.Contains(Other))
					{
						Seen.Add(Other);
						Queue.Add(Other);
					}
				}
			}
		}
		return Seen.Num() - 1;
	};

	for (uint32 Seed = 1; Seed <= 100; ++Seed)
	{
		const FCL9SiteLayout Layout = CL9SiteLayout::Generate(Params, Modules.Defs, Seed);
		if (!Layout.bBuilt)
		{
			continue;
		}
		++Runs;
		TwoOrMore += Layout.FloorCount >= 2 ? 1 : 0;
		ThreeFloors += Layout.FloorCount >= 3 ? 1 : 0;

		if (!CL9SiteLayout::IsConnected(Layout))
		{
			++Disconnected;
		}

		int32 Expected = 0;
		for (const FCL9ModulePlacement& Module : Layout.Modules)
		{
			// 계단은 두 층을 차지하므로 칸을 두 번 센다
			Expected += Module.Size.X * Module.Size.Y * (Module.TopFloor() - Module.Floor + 1);
			Stairs += Module.bStairUp ? 1 : 0;
			Upstairs += Module.Floor > 0 ? 1 : 0;

			// 계단을 올라가면 반드시 갈 데가 있어야 한다 (CL9SiteLayout::StairMinReach)
			if (Module.bStairUp)
			{
				const int32 Reach = UpperReach(Layout, &Module - Layout.Modules.GetData());
				ReachSum += Reach;
				if (Reach < CL9SiteLayout::StairMinReach)
				{
					++Stranded;
				}
			}

			if (Module.TopFloor() >= Layout.FloorCount)
			{
				++Floating;
			}
			if (Module.Floor == 0)
			{
				continue;
			}
			// 위층 모듈은 아래층 실내 모듈이 빠짐없이 받쳐 줘야 한다
			for (int32 Y = 0; Y < Module.Size.Y; ++Y)
			{
				for (int32 X = 0; X < Module.Size.X; ++X)
				{
					const int32 Below = Layout.CellAt(Module.Floor - 1, FIntPoint(Module.Origin.X + X, Module.Origin.Y + Y));
					if (Below == INDEX_NONE || Layout.Modules[Below].Def->bOutdoor)
					{
						++Unsupported;
					}
				}
			}
		}
		if (Expected != CountOccupiedCells(Layout))
		{
			++Overlapped;
		}
	}

	AddInfo(FString::Printf(TEXT("%d runs: %d reached 2 floors, %d reached 3. %.1f stairs and %.1f upstairs modules per run, %.1f rooms reachable above each stair."),
		Runs, TwoOrMore, ThreeFloors, Stairs / FMath::Max(Runs, 1), Upstairs / FMath::Max(Runs, 1), ReachSum / FMath::Max(Stairs, 1.f)));
	TestEqual(TEXT("no stair leads to a dead upper floor"), Stranded, 0);

	// 계단실은 위층 쪽에도 밟을 바닥이 있어야 한다 — 없으면 올라와서 디딤판 끝에 선 채로
	// 어디로도 못 간다. 통로만 뚫려 있고 나머지는 덮여 있는지 본다.
	{
		const FCL9SiteLayout Layout = CL9SiteLayout::Generate(Params, Modules.Defs, 873);
		TArray<FCL9SiteSeg> Segs;
		CL9SiteLayout::BuildSegments(Params, Layout, Segs);

		int32 Checked = 0;
		int32 Landless = 0;
		for (const FCL9ModulePlacement& Module : Layout.Modules)
		{
			if (!Module.bStairUp)
			{
				continue;
			}
			++Checked;
			const float UpperZ = (Module.Floor + 1) * Params.FloorHeight;
			const FBox2D Rect(
				Layout.Origin + FVector2D(Module.Origin.X * Params.CellSize, Module.Origin.Y * Params.CellSize),
				Layout.Origin + FVector2D((Module.Origin.X + Module.Size.X) * Params.CellSize,
					(Module.Origin.Y + Module.Size.Y) * Params.CellSize));

			float Covered = 0.f;
			for (const FCL9SiteSeg& Seg : Segs)
			{
				if ((Seg.Kind != CL9SegKind::Floor() && Seg.Kind != CL9SegKind::Tread()) || !FMath::IsNearlyEqual(Seg.BaseZ, UpperZ, 1.f))
				{
					continue;
				}
				if (Rect.Intersect(Seg.Rect))
				{
					const FBox2D Overlap = Rect.Overlap(Seg.Rect);
					Covered += Overlap.GetSize().X * Overlap.GetSize().Y;
				}
			}
			// 통로만 뚫려 있으면 3분의 1은 덮여 있다 (양옆 띠 + 오르기 시작하는 쪽 참)
			if (Covered < Rect.GetArea() * 0.3f)
			{
				++Landless;
			}
		}
		TestTrue(TEXT("seed 873 has stairs to check"), Checked > 0);
		TestEqual(TEXT("every stairwell has a landing to step off onto"), Landless, 0);
	}

	TestEqual(TEXT("every module is reachable, floors included"), Disconnected, 0);
	TestEqual(TEXT("no two modules share a cell on any floor"), Overlapped, 0);
	TestEqual(TEXT("no upstairs cell hangs in the air"), Unsupported, 0);
	TestEqual(TEXT("no module reaches past the top floor"), Floating, 0);
	// 계단이 설 자리는 시드마다 다르지만, 대부분의 판은 2층까지 올라가야 한다
	TestTrue(FString::Printf(TEXT("%d of %d runs reached a second floor"), TwoOrMore, Runs),
		TwoOrMore >= Runs * 3 / 4);
	return true;
}

#endif
