// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9SiteLayout.h"
#include "Math/RandomStream.h"

namespace
{
	/** The barn's west interior face, east of the truck pad — the yard lies between */
	constexpr float BarnOffsetX = 1400.f;

	/** Boundary wall west of the pad */
	constexpr float YardWest = 900.f;

	/** Free ground between the barns and the boundary walls */
	constexpr float Margin = 600.f;

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
		int32 A;   // aisle (H) or cage row (V)
		int32 C;   // column
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
	 * Spanning tree first (every cell reachable), then extra openings until the
	 * target — the openings count is the difficulty knob (site.md 개구부).
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

		// Closed is already in shuffled order — open from the front until the target
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

		// Jars: farthest-first from a random cell, in aisle-pitch metric
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

	/** A wall along Y with gaps cut out — gaps are (Y0, Y1) pairs, any order */
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
	const float Pitch = P.AisleWidth + P.RowDepth;

	auto GridDepth = [&P, Pitch](int32 Aisles)
	{
		return Aisles * P.AisleWidth + (Aisles - 1) * P.RowDepth;
	};

	// 1F: the spine aisle's centre sits on Y = 0 so the door faces the truck
	F.Spine[0] = (P.AisleCount1F - 1) / 2;
	const float X0 = BarnOffsetX;
	const float Y0 = -(F.Spine[0] * Pitch + P.AisleWidth * 0.5f);
	F.Barn[0] = FBox2D(FVector2D(X0, Y0), FVector2D(X0 + P.ColCount1F * P.ColWidth, Y0 + GridDepth(P.AisleCount1F)));

	// Stair corridor east of 1F, then B1 with its own spine on Y = 0
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

	// The flap leaves from any 1F aisle but the spine (that one holds the stairs)
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

	// ---- Ground. Level 0 covers the yard, 1F and the top landing; the corridor
	//      and B1 sit in a pit whose level-0 rim is filled with strips.
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

	// ---- Boundary (wall kind — the yard fence height)
	AddBox(Out, CL9SegKind::Wall(), 0, World.Min.X - W, World.Min.Y - W, World.Max.X + W, World.Min.Y);
	AddBox(Out, CL9SegKind::Wall(), 0, World.Min.X - W, World.Max.Y, World.Max.X + W, World.Max.Y + W);
	AddBox(Out, CL9SegKind::Wall(), 0, World.Min.X - W, World.Min.Y, World.Min.X, World.Max.Y);
	AddBox(Out, CL9SegKind::Wall(), 0, World.Max.X, World.Min.Y, World.Max.X + W, World.Max.Y);

	// ---- Barn shells. West walls open at the spine (1F door, B1 stair mouth);
	//      the 1F east wall opens for the stairs and, with a maze, the flap.
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

	// ---- Stair corridor walls, from the 1F east wall to the B1 west wall
	AddBox(Out, CL9SegKind::Stair(), 1, B0.Max.X, CorrY0, B1.Min.X, -HalfA);
	AddBox(Out, CL9SegKind::Stair(), 1, B0.Max.X, HalfA, B1.Min.X, CorrY1);

	if (!L.bHasMaze)
	{
		return;
	}

	// ---- Maze: cage rows between aisles with openings, plugs across aisles
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
