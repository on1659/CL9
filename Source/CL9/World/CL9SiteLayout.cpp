// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9SiteLayout.h"
#include "Math/RandomStream.h"

namespace
{
	/** 축사 서쪽 내부면 — 트럭 패드에서 동쪽으로 떨어져 있고, 그 사이는 마당이다 */
	constexpr float BarnOffsetX = 1400.f;

	/** 패드 서쪽의 경계 벽 */
	constexpr float YardWest = 900.f;

	/** 축사와 경계 벽 사이의 빈 땅 */
	constexpr float Margin = 600.f;

	/** 1층 바닥 위 천장 높이 — 축사 내부 높이(구 CCL8 BarnHeightU 210u)로, 우연히 벽 Height 값과 일치한다 */
	constexpr float BarnCeiling = 630.f;

	void InitFloor(FCL9FloorLayout& Floor, int32 Aisles, int32 Cols)
	{
		Floor.Aisles = Aisles;
		Floor.Cols = Cols;
		Floor.OpenH.Init(0, Aisles * FMath::Max(Cols - 1, 0));
		Floor.OpenV.Init(0, FMath::Max(Aisles - 1, 0) * Cols);
		Floor.JarCells.Reset();
	}

	struct FEdge
	{
		bool bHorizontal;
		int32 A;   // 통로 번호(H) 또는 케이지 열 번호(V)
		int32 C;   // 열
	};

	int32 FindRoot(TArray<int32>& Parent, int32 Cell)
	{
		while (Parent[Cell] != Cell)
		{
			Parent[Cell] = Parent[Parent[Cell]];
			Cell = Parent[Cell];
		}
		return Cell;
	}

	template <typename T>
	void Shuffle(TArray<T>& Items, FRandomStream& Rng)
	{
		for (int32 Index = Items.Num() - 1; Index > 0; --Index)
		{
			Items.Swap(Index, Rng.RandRange(0, Index));
		}
	}

	/**
	 * 먼저 신장 트리(spanning tree)를 만들어 모든 셀에 도달 가능하게 한 뒤, 목표
	 * 개수에 도달할 때까지 추가로 통로를 연다 — 이 개구부 개수가 난이도 조절값이다
	 * (site.md 개구부).
	 */
	void CarveFloor(FCL9FloorLayout& Floor, int32 TargetOpen, int32 JarCount, const FCL9HenhouseParams& Params, FRandomStream& Rng)
	{
		TArray<FEdge> Edges;
		for (int32 Aisle = 0; Aisle < Floor.Aisles; ++Aisle)
		{
			for (int32 Col = 0; Col < Floor.Cols - 1; ++Col)
			{
				Edges.Add({true, Aisle, Col});
			}
		}
		for (int32 Row = 0; Row < Floor.Aisles - 1; ++Row)
		{
			for (int32 Col = 0; Col < Floor.Cols; ++Col)
			{
				Edges.Add({false, Row, Col});
			}
		}
		Shuffle(Edges, Rng);

		TArray<int32> Parent;
		Parent.SetNum(Floor.CellCount());
		for (int32 Index = 0; Index < Parent.Num(); ++Index)
		{
			Parent[Index] = Index;
		}

		auto Open = [&Floor](const FEdge& Edge)
		{
			if (Edge.bHorizontal)
			{
				Floor.OpenH[Edge.A * (Floor.Cols - 1) + Edge.C] = 1;
			}
			else
			{
				Floor.OpenV[Edge.A * Floor.Cols + Edge.C] = 1;
			}
		};

		TArray<FEdge> Closed;
		for (const FEdge& Edge : Edges)
		{
			const int32 CellA = Edge.A * Floor.Cols + Edge.C;
			const int32 CellB = Edge.bHorizontal ? CellA + 1 : CellA + Floor.Cols;
			const int32 RootA = FindRoot(Parent, CellA);
			const int32 RootB = FindRoot(Parent, CellB);
			if (RootA == RootB)
			{
				Closed.Add(Edge);
				continue;
			}
			Parent[RootA] = RootB;
			Open(Edge);
		}

		// Closed는 이미 무작위로 섞인 순서다 — 앞에서부터 목표치에 도달할 때까지 연다
		int32 OpenCount = Floor.CountOpen();
		for (const FEdge& Edge : Closed)
		{
			if (OpenCount >= TargetOpen)
			{
				break;
			}
			Open(Edge);
			++OpenCount;
		}

		// 항아리: 임의의 셀에서 시작해 통로 간격(pitch) 기준 거리로 가장 먼 곳부터 배치한다
		const float Pitch = Params.AisleWidth + Params.RowDepth;
		auto Distance = [&](const FIntPoint& Lhs, const FIntPoint& Rhs)
		{
			const float DX = (Lhs.Y - Rhs.Y) * Params.ColWidth;
			const float DY = (Lhs.X - Rhs.X) * Pitch;
			return DX * DX + DY * DY;
		};
		const int32 Jars = FMath::Min(JarCount, Floor.CellCount());
		if (Jars > 0)
		{
			const int32 First = Rng.RandRange(0, Floor.CellCount() - 1);
			Floor.JarCells.Add(FIntPoint(First / Floor.Cols, First % Floor.Cols));
			while (Floor.JarCells.Num() < Jars)
			{
				FIntPoint Best(0, 0);
				float BestScore = -1.f;
				for (int32 Aisle = 0; Aisle < Floor.Aisles; ++Aisle)
				{
					for (int32 Col = 0; Col < Floor.Cols; ++Col)
					{
						const FIntPoint Cell(Aisle, Col);
						float Nearest = TNumericLimits<float>::Max();
						for (const FIntPoint& Jar : Floor.JarCells)
						{
							Nearest = FMath::Min(Nearest, Distance(Cell, Jar));
						}
						if (Nearest > BestScore)
						{
							BestScore = Nearest;
							Best = Cell;
						}
					}
				}
				Floor.JarCells.Add(Best);
			}
		}
	}

	void AddBox(TArray<FCL9SiteSeg>& Out, const FName& Kind, int32 Floor, float X0, float Y0, float X1, float Y1, bool bRamp = false)
	{
		FCL9SiteSeg Seg;
		Seg.Kind = Kind;
		Seg.Floor = Floor;
		Seg.Rect = FBox2D(FVector2D(X0, Y0), FVector2D(X1, Y1));
		Seg.bRamp = bRamp;
		Out.Add(Seg);
	}

	/** Y축을 따라 뻗은 벽에 틈을 뚫는다 — 틈은 (Y0, Y1) 쌍이며 순서는 상관없다 */
	void AddWallWithGaps(TArray<FCL9SiteSeg>& Out, const FName& Kind, int32 Floor, float X0, float X1, float Y0, float Y1, TArray<FVector2D> Gaps)
	{
		Gaps.Sort([](const FVector2D& Lhs, const FVector2D& Rhs) { return Lhs.X < Rhs.X; });
		float Cursor = Y0;
		for (const FVector2D& Gap : Gaps)
		{
			if (Gap.X > Cursor)
			{
				AddBox(Out, Kind, Floor, X0, Cursor, X1, Gap.X);
			}
			Cursor = FMath::Max(Cursor, Gap.Y);
		}
		if (Cursor < Y1)
		{
			AddBox(Out, Kind, Floor, X0, Cursor, X1, Y1);
		}
	}

	void FnvMix(uint32& Hash, uint32 Value)
	{
		for (int32 Byte = 0; Byte < 4; ++Byte)
		{
			Hash ^= (Value >> (Byte * 8)) & 0xFF;
			Hash *= 16777619u;
		}
	}
}

int32 FCL9FloorLayout::CountOpen() const
{
	int32 Count = 0;
	for (uint8 Open : OpenH)
	{
		Count += Open;
	}
	for (uint8 Open : OpenV)
	{
		Count += Open;
	}
	return Count;
}

FCL9SiteFrame CL9SiteLayout::ComputeFrame(const FCL9HenhouseParams& P)
{
	FCL9SiteFrame F;
	F.Wall = P.WallThickness;
	const float Pitch = P.AisleWidth + P.RowDepth;

	auto GridDepth = [&P, Pitch](int32 Aisles)
	{
		return Aisles * P.AisleWidth + (Aisles - 1) * P.RowDepth;
	};

	// 1층: 중심 통로(spine)의 중앙이 Y = 0에 오도록 해서 문이 트럭을 향하게 한다
	F.Spine[0] = (P.AisleCount1F - 1) / 2;
	const float X0 = BarnOffsetX;
	const float Y0 = -(F.Spine[0] * Pitch + P.AisleWidth * 0.5f);
	F.Barn[0] = FBox2D(FVector2D(X0, Y0), FVector2D(X0 + P.ColCount1F * P.ColWidth, Y0 + GridDepth(P.AisleCount1F)));

	// 1층 동쪽에 계단 통로를 두고, 그다음 지하 1층도 자기 중심 통로를 Y = 0에 둔다
	F.StairX0 = F.Barn[0].Max.X + F.Wall;
	F.StairX1 = F.StairX0 + F.Landing + F.Ramp + F.Landing;
	F.Spine[1] = (P.AisleCountB1 - 1) / 2;
	const float BX0 = F.StairX1 + F.Wall;
	const float BY0 = -(F.Spine[1] * Pitch + P.AisleWidth * 0.5f);
	F.Barn[1] = FBox2D(FVector2D(BX0, BY0), FVector2D(BX0 + P.ColCountB1 * P.ColWidth, BY0 + GridDepth(P.AisleCountB1)));

	F.World = FBox2D(
		FVector2D(-YardWest, FMath::Min(F.Barn[0].Min.Y, F.Barn[1].Min.Y) - F.Wall - Margin),
		FVector2D(F.Barn[1].Max.X + F.Wall, FMath::Max(F.Barn[0].Max.Y, F.Barn[1].Max.Y) + F.Wall + Margin));
	return F;
}

FCL9SiteLayout CL9SiteLayout::Generate(const FCL9HenhouseParams& P, uint32 Seed)
{
	FCL9SiteLayout L;
	L.Seed = Seed;
	InitFloor(L.Floors[0], P.AisleCount1F, P.ColCount1F);
	InitFloor(L.Floors[1], P.AisleCountB1, P.ColCountB1);
	if (Seed == 0)
	{
		return L;
	}

	L.bHasMaze = true;
	FRandomStream Rng(static_cast<int32>(Seed));
	CarveFloor(L.Floors[0], P.TargetOpen1F, P.JarCount1F, P, Rng);
	CarveFloor(L.Floors[1], P.TargetOpenB1, P.JarCountB1, P, Rng);

	// 여닫이문은 중심 통로(계단이 차지하고 있다)를 제외한 1층의 아무 통로에나 낸다
	const int32 Spine = (P.AisleCount1F - 1) / 2;
	int32 Hole = Rng.RandRange(0, FMath::Max(P.AisleCount1F - 2, 0));
	if (Hole >= Spine)
	{
		++Hole;
	}
	L.EastHoleAisle = FMath::Clamp(Hole, 0, P.AisleCount1F - 1);
	return L;
}

void CL9SiteLayout::BuildSegments(const FCL9HenhouseParams& P, const FCL9SiteFrame& F, const FCL9SiteLayout& L, TArray<FCL9SiteSeg>& Out)
{
	const float W = F.Wall;
	const float HalfA = P.AisleWidth * 0.5f;
	const FBox2D& B0 = F.Barn[0];
	const FBox2D& B1 = F.Barn[1];
	const FBox2D& World = F.World;

	// ---- 바닥. 레벨 0은 마당, 1층, 위쪽 계단참을 덮는다. 계단 통로와
	//      지하 1층은 구덩이 안에 있고, 레벨 0의 그 가장자리는 띠 모양으로 채워진다.
	const float PitX0 = F.StairX0 + F.Landing;
	const float CorrY0 = -HalfA - W;
	const float CorrY1 = HalfA + W;
	AddBox(Out, CL9SegKind::Floor(), 0, World.Min.X, World.Min.Y, PitX0, World.Max.Y);
	AddBox(Out, CL9SegKind::Floor(), 0, PitX0, CorrY1, B1.Min.X - W, World.Max.Y);
	AddBox(Out, CL9SegKind::Floor(), 0, PitX0, World.Min.Y, B1.Min.X - W, CorrY0);
	AddBox(Out, CL9SegKind::Floor(), 0, B1.Min.X - W, B1.Max.Y + W, World.Max.X, World.Max.Y);
	AddBox(Out, CL9SegKind::Floor(), 0, B1.Min.X - W, World.Min.Y, World.Max.X, B1.Min.Y - W);
	AddBox(Out, CL9SegKind::Floor(), 1, PitX0, B1.Min.Y - W, B1.Max.X + W, B1.Max.Y + W);
	AddBox(Out, CL9SegKind::Floor(), 1, PitX0, -HalfA, PitX0 + F.Ramp, HalfA, /*bRamp*/ true);

	// ---- 경계(wall 종류 — 마당 울타리 높이)
	AddBox(Out, CL9SegKind::Wall(), 0, World.Min.X - W, World.Min.Y - W, World.Max.X + W, World.Min.Y);
	AddBox(Out, CL9SegKind::Wall(), 0, World.Min.X - W, World.Max.Y, World.Max.X + W, World.Max.Y + W);
	AddBox(Out, CL9SegKind::Wall(), 0, World.Min.X - W, World.Min.Y, World.Min.X, World.Max.Y);
	AddBox(Out, CL9SegKind::Wall(), 0, World.Max.X, World.Min.Y, World.Max.X + W, World.Max.Y);

	// ---- 축사 외곽. 서쪽 벽은 중심 통로 위치에서 열린다(1층은 문, 지하 1층은 계단 입구).
	//      1층 동쪽 벽은 계단을 위해 열리고, 미로가 있다면 여닫이문을 위해서도 열린다.
	for (int32 Floor = 0; Floor < 2; ++Floor)
	{
		const FBox2D& B = F.Barn[Floor];
		AddWallWithGaps(Out, CL9SegKind::Barn(), Floor, B.Min.X - W, B.Min.X, B.Min.Y - W, B.Max.Y + W, {FVector2D(-HalfA, HalfA)});
		AddBox(Out, CL9SegKind::Barn(), Floor, B.Min.X - W, B.Max.Y, B.Max.X + W, B.Max.Y + W);
		AddBox(Out, CL9SegKind::Barn(), Floor, B.Min.X - W, B.Min.Y - W, B.Max.X + W, B.Min.Y);

		if (Floor == 0)
		{
			TArray<FVector2D> Gaps = {FVector2D(-HalfA, HalfA)};
			if (L.bHasMaze && L.EastHoleAisle >= 0)
			{
				const float HoleY = CellCenter(P, F, 0, L.EastHoleAisle, 0).Y;
				const FVector2D Hole(HoleY - F.HoleWidth * 0.5f, HoleY + F.HoleWidth * 0.5f);
				Gaps.Add(Hole);
				AddBox(Out, CL9SegKind::FlapIn(), 0, B.Max.X, Hole.X, B.Max.X + W, Hole.Y);
			}
			AddWallWithGaps(Out, CL9SegKind::Barn(), 0, B.Max.X, B.Max.X + W, B.Min.Y - W, B.Max.Y + W, Gaps);
		}
		else
		{
			AddBox(Out, CL9SegKind::Barn(), 1, B.Max.X, B.Min.Y - W, B.Max.X + W, B.Max.Y + W);
		}
	}

	// ---- 계단 통로 벽 — 1층 동쪽 벽부터 지하 1층 서쪽 벽까지
	AddBox(Out, CL9SegKind::Stair(), 1, B0.Max.X, CorrY0, B1.Min.X, -HalfA);
	AddBox(Out, CL9SegKind::Stair(), 1, B0.Max.X, HalfA, B1.Min.X, CorrY1);

	// ---- 천장: 축사와 통로에 지붕을 씌워 내부를 어둡게 만든다
	//      (2026-09-06 사용자 결정 — 건물 안에 들어가는 느낌). 지하 1층 지붕은 지면 높이에 있고,
	//      통로 지붕은 1층 높이로 지나가서 지하 1층 입구는 그 지붕 위쪽으로 벽이 막는다.
	AddBox(Out, CL9SegKind::Ceiling(), 0, B0.Min.X - W, B0.Min.Y - W, B0.Max.X + W, B0.Max.Y + W);
	AddBox(Out, CL9SegKind::Ceiling(), 0, B0.Max.X, CorrY0, B1.Min.X, CorrY1);
	AddBox(Out, CL9SegKind::Ceiling(), 1, B1.Min.X - W, B1.Min.Y - W, B1.Max.X + W, B1.Max.Y + W);
	AddBox(Out, CL9SegKind::Barn(), 0, B1.Min.X - W, -HalfA, B1.Min.X, HalfA);

	if (!L.bHasMaze)
	{
		return;
	}

	// ---- 미로: 통로 사이에 개구부가 있는 케이지 열, 통로를 가로지르는 플러그
	const float Pitch = P.AisleWidth + P.RowDepth;
	for (int32 Floor = 0; Floor < 2; ++Floor)
	{
		const FCL9FloorLayout& Grid = L.Floors[Floor];
		const FBox2D& B = F.Barn[Floor];
		for (int32 Row = 0; Row < Grid.Aisles - 1; ++Row)
		{
			const float RowY0 = B.Min.Y + Row * Pitch + P.AisleWidth;
			const float RowY1 = RowY0 + P.RowDepth;
			for (int32 Col = 0; Col < Grid.Cols; ++Col)
			{
				const float CX0 = B.Min.X + Col * P.ColWidth;
				const float CX1 = CX0 + P.ColWidth;
				if (Grid.IsOpenV(Row, Col))
				{
					const float Center = (CX0 + CX1) * 0.5f;
					AddBox(Out, CL9SegKind::Cage(), Floor, CX0, RowY0, Center - P.OpenWidth * 0.5f, RowY1);
					AddBox(Out, CL9SegKind::Cage(), Floor, Center + P.OpenWidth * 0.5f, RowY0, CX1, RowY1);
				}
				else
				{
					AddBox(Out, CL9SegKind::Cage(), Floor, CX0, RowY0, CX1, RowY1);
				}
			}
		}
		for (int32 Aisle = 0; Aisle < Grid.Aisles; ++Aisle)
		{
			const float AisleY0 = B.Min.Y + Aisle * Pitch;
			const float AisleY1 = AisleY0 + P.AisleWidth;
			for (int32 Col = 0; Col < Grid.Cols - 1; ++Col)
			{
				if (!Grid.IsOpenH(Aisle, Col))
				{
					const float BoundaryX = B.Min.X + (Col + 1) * P.ColWidth;
					AddBox(Out, CL9SegKind::Plug(), Floor, BoundaryX - P.PlugWidth * 0.5f, AisleY0, BoundaryX + P.PlugWidth * 0.5f, AisleY1);
				}
			}
		}
	}
}

float CL9SiteLayout::CeilingBase(const FCL9SiteFrame& F, int32 Floor)
{
	return Floor == 0 ? F.FloorBase(0) + BarnCeiling : F.FloorBase(0);
}

FVector2D CL9SiteLayout::CellCenter(const FCL9HenhouseParams& P, const FCL9SiteFrame& F, int32 Floor, int32 Aisle, int32 Col)
{
	const FBox2D& B = F.Barn[FMath::Clamp(Floor, 0, 1)];
	return FVector2D(
		B.Min.X + (Col + 0.5f) * P.ColWidth,
		B.Min.Y + Aisle * (P.AisleWidth + P.RowDepth) + P.AisleWidth * 0.5f);
}

bool CL9SiteLayout::IsConnected(const FCL9FloorLayout& Floor)
{
	const int32 Count = Floor.CellCount();
	if (Count == 0)
	{
		return true;
	}
	TArray<uint8> Seen;
	Seen.Init(0, Count);
	TArray<int32> Queue;
	Queue.Add(0);
	Seen[0] = 1;
	int32 Reached = 0;
	while (Queue.Num() > 0)
	{
		const int32 Cell = Queue.Pop(EAllowShrinking::No);
		++Reached;
		const int32 Aisle = Cell / Floor.Cols;
		const int32 Col = Cell % Floor.Cols;
		auto Visit = [&](int32 Next)
		{
			if (!Seen[Next])
			{
				Seen[Next] = 1;
				Queue.Add(Next);
			}
		};
		if (Col > 0 && Floor.IsOpenH(Aisle, Col - 1)) Visit(Cell - 1);
		if (Col < Floor.Cols - 1 && Floor.IsOpenH(Aisle, Col)) Visit(Cell + 1);
		if (Aisle > 0 && Floor.IsOpenV(Aisle - 1, Col)) Visit(Cell - Floor.Cols);
		if (Aisle < Floor.Aisles - 1 && Floor.IsOpenV(Aisle, Col)) Visit(Cell + Floor.Cols);
	}
	return Reached == Count;
}

uint32 CL9SiteLayout::LayoutHash(const FCL9SiteLayout& L)
{
	uint32 Hash = 2166136261u;
	FnvMix(Hash, L.Seed);
	FnvMix(Hash, static_cast<uint32>(L.EastHoleAisle));
	for (const FCL9FloorLayout& Floor : L.Floors)
	{
		for (uint8 Open : Floor.OpenH) FnvMix(Hash, Open);
		for (uint8 Open : Floor.OpenV) FnvMix(Hash, Open);
		for (const FIntPoint& Jar : Floor.JarCells)
		{
			FnvMix(Hash, static_cast<uint32>(Jar.X));
			FnvMix(Hash, static_cast<uint32>(Jar.Y));
		}
	}
	return Hash;
}
