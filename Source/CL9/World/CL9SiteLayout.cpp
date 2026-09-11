// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9SiteLayout.h"
#include "Math/RandomStream.h"

namespace
{
	/** 변 번호는 ECL9ModuleSide의 비트 순서 그대로다 — 시계 방향 N → E → S → W */
	constexpr int32 SideN = 0;
	constexpr int32 SideE = 1;
	constexpr int32 SideS = 2;
	constexpr int32 SideW = 3;

	const FIntPoint SideDir[4] = {FIntPoint(0, 1), FIntPoint(1, 0), FIntPoint(0, -1), FIntPoint(-1, 0)};

	/**
	 * 이미 놓인 모듈끼리 맞닿았을 때 문을 뚫을 확률 — 여기서 고리(우회로)가 생긴다
	 * (DunGen의 `DoorwayConnectionChance`에 해당). 0이면 순수한 나무 구조가 되어
	 * 도망갈 길이 사라진다.
	 */
	constexpr float LoopChance = 0.25f;

	/** 전체 모듈 중 본줄기가 차지하는 비율 — 나머지는 곁가지다 (리썰컴퍼니는 곁가지가 훨씬 많다) */
	constexpr float MainPathShare = 0.4f;

	/** 곁가지 단계에서 남은 예산을 나누는 비율 (REPO 타입 비율 N 31% · DE 25% · P 19%를 셋으로 정규화) */
	constexpr float PassageShare = 0.28f;
	constexpr float DeadEndShare = 0.32f;

	/** 이만큼 연달아 못 놓으면 격자가 꽉 찬 것으로 보고 판을 닫는다 */
	constexpr int32 StallLimit = 48;

	/** 계단 디딤판 하나의 높이 목표 (cm) — 층 높이를 이 값으로 나눠 디딤판 수를 정한다 */
	constexpr float StepRise = 20.f;

	using CL9SiteLayout::StairMinReach;

	int32 Opposite(int32 Side)
	{
		return (Side + 2) % 4;
	}

	bool HasSide(uint8 Sockets, int32 Side)
	{
		return (Sockets & (1 << Side)) != 0;
	}

	void FnvMix(uint32& Hash, uint32 Value)
	{
		for (int32 Byte = 0; Byte < 4; ++Byte)
		{
			Hash ^= (Value >> (Byte * 8)) & 0xFF;
			Hash *= 16777619u;
		}
	}

	/** 아직 모듈이 뻗어 나갈 수 있는 자리 하나 — 모듈 안쪽 칸과 그 칸이 바라보는 바깥 방향 */
	struct FFrontier
	{
		int32 Module = INDEX_NONE;
		FIntPoint Cell = FIntPoint::ZeroValue;
		int32 Side = 0;
		int32 Hops = 0;
	};

	/** 배치 후보 하나 — 어느 행을 어느 회전으로 어디에 앉힐 수 있는가 */
	struct FCandidate
	{
		const UCL9ModuleDef* Def = nullptr;
		FIntPoint Origin = FIntPoint::ZeroValue;
		FIntPoint Size = FIntPoint(1, 1);
		int32 Rotation = 0;
		uint8 Sockets = 0;

		bool SameOutcome(const FCandidate& Other) const
		{
			// 대칭인 모듈은 여러 회전이 똑같은 결과를 낸다 — 그대로 두면 대칭일수록
			// 자주 뽑혀서 Rarity가 뜻하는 바가 흐려진다.
			return Def == Other.Def && Origin == Other.Origin && Size == Other.Size && Sockets == Other.Sockets;
		}
	};

	/** 그 칸이 위층을 받쳐 줄 수 있나 — 실내 모듈만 위에 무언가를 올릴 수 있다 */
	bool CanCarry(const FCL9SiteLayout& Layout, int32 Floor, FIntPoint Cell)
	{
		const int32 Below = Layout.CellAt(Floor, Cell);
		return Below != INDEX_NONE && !Layout.Modules[Below].Def->bOutdoor;
	}

	/**
	 * 격자 안이고, 그 층의 칸이 전부 비어 있고, 1층이 아니라면 아래층이 전부 받쳐
	 * 주는가. `bAlsoUpper`면 바로 위층의 같은 칸까지 비어 있어야 한다 — 계단 모듈이
	 * 두 층을 함께 차지하기 때문이다.
	 */
	bool Fits(const FCL9SiteLayout& Layout, int32 Floor, FIntPoint Origin, FIntPoint Size, bool bAlsoUpper = false)
	{
		if (Origin.X < 0 || Origin.Y < 0 || Origin.X + Size.X > Layout.GridSize.X || Origin.Y + Size.Y > Layout.GridSize.Y)
		{
			return false;
		}
		if (!Layout.IsFloor(Floor) || (bAlsoUpper && !Layout.IsFloor(Floor + 1)))
		{
			return false;
		}
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			for (int32 X = 0; X < Size.X; ++X)
			{
				const FIntPoint Cell(Origin.X + X, Origin.Y + Y);
				if (Layout.CellAt(Floor, Cell) != INDEX_NONE)
				{
					return false;
				}
				if (bAlsoUpper && Layout.CellAt(Floor + 1, Cell) != INDEX_NONE)
				{
					return false;
				}
				// 위층은 허공에 뜰 수 없다 — 아래층 실내 모듈이 받쳐 주는 자리에만 선다.
				if (Floor > 0 && !CanCarry(Layout, Floor - 1, Cell))
				{
					return false;
				}
			}
		}
		return true;
	}

	/**
	 * 칸 Cell에서 Side 방향으로 나가는 변에 상태를 적는다. 격자 바깥으로 나가는 변도
	 * 유효하다 — 입구 문이 바로 그 경우다.
	 */
	void SetEdge(FCL9SiteLayout& Layout, int32 Floor, FIntPoint Cell, int32 Side, ECL9GridEdge State)
	{
		const uint8 Value = static_cast<uint8>(State);
		const int32 BaseX = Floor * Layout.EdgeXStride();
		const int32 BaseY = Floor * Layout.EdgeYStride();
		switch (Side)
		{
		case SideE: Layout.EdgeX[BaseX + (Cell.X + 1) + Cell.Y * (Layout.GridSize.X + 1)] = Value; break;
		case SideW: Layout.EdgeX[BaseX + Cell.X + Cell.Y * (Layout.GridSize.X + 1)] = Value; break;
		case SideN: Layout.EdgeY[BaseY + Cell.X + (Cell.Y + 1) * Layout.GridSize.X] = Value; break;
		default: Layout.EdgeY[BaseY + Cell.X + Cell.Y * Layout.GridSize.X] = Value; break;
		}
	}

	ECL9GridEdge GetEdge(const FCL9SiteLayout& Layout, int32 Floor, FIntPoint Cell, int32 Side)
	{
		switch (Side)
		{
		case SideE: return Layout.EdgeAtX(Floor, Cell.X + 1, Cell.Y);
		case SideW: return Layout.EdgeAtX(Floor, Cell.X, Cell.Y);
		case SideN: return Layout.EdgeAtY(Floor, Cell.X, Cell.Y + 1);
		default: return Layout.EdgeAtY(Floor, Cell.X, Cell.Y);
		}
	}

	/** 모듈을 격자에 새기고 그 인덱스를 돌려준다. 계단 모듈은 위층 칸까지 함께 차지한다 */
	int32 Occupy(FCL9SiteLayout& Layout, const FCandidate& Cand, int32 Hops, int32 Floor, bool bStairUp)
	{
		FCL9ModulePlacement Placement;
		Placement.Def = Cand.Def;
		Placement.Origin = Cand.Origin;
		Placement.Size = Cand.Size;
		Placement.Rotation = Cand.Rotation;
		Placement.Sockets = Cand.Sockets;
		Placement.Hops = Hops;
		Placement.Floor = Floor;
		Placement.bStairUp = bStairUp;
		const int32 Index = Layout.Modules.Add(Placement);

		for (int32 Level = Floor; Level <= Placement.TopFloor(); ++Level)
		{
			const int32 Base = Level * Layout.CellStride();
			for (int32 Y = 0; Y < Cand.Size.Y; ++Y)
			{
				for (int32 X = 0; X < Cand.Size.X; ++X)
				{
					Layout.CellModule[Base + (Cand.Origin.X + X) + (Cand.Origin.Y + Y) * Layout.GridSize.X] = Index;
				}
			}
		}
		return Index;
	}

	/** 모듈의 열린 변에 닿은 경계 칸들을 (칸, 바깥 방향)으로 훑는다 */
	template <typename FuncType>
	void ForEachBoundaryCell(const FCL9ModulePlacement& Module, FuncType&& Func)
	{
		for (int32 Side = 0; Side < 4; ++Side)
		{
			if (!HasSide(Module.Sockets, Side))
			{
				continue;
			}
			const int32 Count = (Side == SideN || Side == SideS) ? Module.Size.X : Module.Size.Y;
			for (int32 Index = 0; Index < Count; ++Index)
			{
				FIntPoint Cell;
				switch (Side)
				{
				case SideN: Cell = FIntPoint(Module.Origin.X + Index, Module.Origin.Y + Module.Size.Y - 1); break;
				case SideS: Cell = FIntPoint(Module.Origin.X + Index, Module.Origin.Y); break;
				case SideE: Cell = FIntPoint(Module.Origin.X + Module.Size.X - 1, Module.Origin.Y + Index); break;
				default: Cell = FIntPoint(Module.Origin.X, Module.Origin.Y + Index); break;
				}
				Func(Cell, Side);
			}
		}
	}

	/**
	 * 프론티어 자리 하나에 앉힐 수 있는 배치를 전부 모은다. 회전 넷을 모두 시도하고,
	 * 맞물릴 변을 따라 모듈을 미끄러뜨려 본다 — 3칸짜리 변이면 세 가지로 붙을 수 있다.
	 */
	void GatherCandidates(const FCL9SiteLayout& Layout, int32 Floor, const TArray<const UCL9ModuleDef*>& Pool,
		FIntPoint Target, int32 Side, bool bNeedUpper, TArray<FCandidate>& Out)
	{
		Out.Reset();
		const int32 Back = Opposite(Side);
		for (const UCL9ModuleDef* Def : Pool)
		{
			for (int32 Rotation = 0; Rotation < 4; ++Rotation)
			{
				const uint8 Sockets = UCL9ModuleDef::RotateSockets(static_cast<uint8>(Def->Sockets & 0xF), Rotation);
				if (!HasSide(Sockets, Back))
				{
					continue;
				}
				const FIntPoint Size = UCL9ModuleDef::RotateSize(Def->Size, Rotation);
				const int32 Slide = (Back == SideN || Back == SideS) ? Size.X : Size.Y;
				for (int32 Offset = 0; Offset < Slide; ++Offset)
				{
					FIntPoint Origin;
					switch (Back)
					{
					case SideW: Origin = FIntPoint(Target.X, Target.Y - Offset); break;
					case SideE: Origin = FIntPoint(Target.X - Size.X + 1, Target.Y - Offset); break;
					case SideS: Origin = FIntPoint(Target.X - Offset, Target.Y); break;
					default: Origin = FIntPoint(Target.X - Offset, Target.Y - Size.Y + 1); break;
					}
					if (!Fits(Layout, Floor, Origin, Size, bNeedUpper))
					{
						continue;
					}

					FCandidate Cand;
					Cand.Def = Def;
					Cand.Origin = Origin;
					Cand.Size = Size;
					Cand.Rotation = Rotation;
					Cand.Sockets = Sockets;
					if (!Out.ContainsByPredicate([&Cand](const FCandidate& Other) { return Cand.SameOutcome(Other); }))
					{
						Out.Add(Cand);
					}
				}
			}
		}
	}

	/** Rarity를 가중치로 하나 고른다 */
	const FCandidate* PickCandidate(const TArray<FCandidate>& Candidates, FRandomStream& Rng)
	{
		int32 Total = 0;
		for (const FCandidate& Cand : Candidates)
		{
			Total += FMath::Clamp(Cand.Def->Rarity, 1, 3);
		}
		if (Total <= 0)
		{
			return nullptr;
		}
		int32 Roll = Rng.RandRange(0, Total - 1);
		for (const FCandidate& Cand : Candidates)
		{
			Roll -= FMath::Clamp(Cand.Def->Rarity, 1, 3);
			if (Roll < 0)
			{
				return &Cand;
			}
		}
		return &Candidates.Last();
	}

	/**
	 * 새로 놓인 모듈의 열린 경계 칸들을 한 층에서 훑어서, 빈 쪽은 그 층의 프론티어에
	 * 넣고 이미 모듈이 있는 쪽은 확률로 문을 뚫는다(고리).
	 *
	 * **높이가 다른 두 방은 애초에 후보가 아니다** — 이 함수는 한 층 안에서만 도니까,
	 * 리썰컴퍼니가 문틀을 3D 격자에 해시해서 얻는 성질이 여기서는 공짜로 성립한다.
	 */
	void OpenNeighbours(FCL9SiteLayout& Layout, int32 Floor, int32 ModuleIndex,
		TArray<FFrontier>& Frontier, FRandomStream& Rng)
	{
		const FCL9ModulePlacement Module = Layout.Modules[ModuleIndex];
		ForEachBoundaryCell(Module, [&](FIntPoint Cell, int32 Side)
		{
			const FIntPoint Neighbour = Cell + SideDir[Side];
			if (!Layout.IsInside(Neighbour))
			{
				return;
			}
			const int32 Other = Layout.CellAt(Floor, Neighbour);
			if (Other == INDEX_NONE)
			{
				Frontier.Add({ModuleIndex, Cell, Side, Module.Hops});
				return;
			}
			if (Other == ModuleIndex || GetEdge(Layout, Floor, Cell, Side) != ECL9GridEdge::Open)
			{
				return;
			}
			// 이웃도 이쪽 변이 열려 있어야 문이 된다. 이웃의 그 칸은 반드시 이웃의
			// 경계 칸이다 — 우리 쪽 칸이 그 모듈 밖에 있으니까.
			if (HasSide(Layout.Modules[Other].Sockets, Opposite(Side)) && Rng.FRand() < LoopChance)
			{
				SetEdge(Layout, Floor, Cell, Side, ECL9GridEdge::Door);
			}
		});
	}

	/** 프론티어에서 하나를 꺼낸다 — 본줄기 단계에선 가장 깊은 것, 곁가지 단계에선 무작위 */
	bool TakeFrontier(TArray<FFrontier>& Frontier, const FCL9SiteLayout& Layout, int32 Floor,
		bool bDeepest, FRandomStream& Rng, FFrontier& Out)
	{
		while (Frontier.Num() > 0)
		{
			int32 Chosen = 0;
			if (bDeepest)
			{
				for (int32 Index = 1; Index < Frontier.Num(); ++Index)
				{
					if (Frontier[Index].Hops > Frontier[Chosen].Hops)
					{
						Chosen = Index;
					}
				}
			}
			else
			{
				Chosen = Rng.RandRange(0, Frontier.Num() - 1);
			}

			Out = Frontier[Chosen];
			Frontier.RemoveAtSwap(Chosen, EAllowShrinking::No);

			// 그 사이에 다른 모듈이 앞을 막았으면 버린다
			const FIntPoint Target = Out.Cell + SideDir[Out.Side];
			if (Layout.IsInside(Target) && Layout.CellAt(Floor, Target) == INDEX_NONE)
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * 한 자리에 모듈 하나를 놓는다. Pool이 비었거나 그 자리에 맞는 게 없으면 false —
	 * 호출부는 다음 프론티어로 넘어간다. 계단 모듈이면 위층 칸까지 차지하고, 위층
	 * 프론티어에도 자리를 남긴다.
	 */
	bool PlaceAt(FCL9SiteLayout& Layout, int32 Floor, const FFrontier& Spot,
		const TArray<const UCL9ModuleDef*>& Pool, bool bStairUp,
		TArray<TArray<FFrontier>>& Frontiers, FRandomStream& Rng)
	{
		if (Pool.Num() == 0)
		{
			return false;
		}
		TArray<FCandidate> Candidates;
		GatherCandidates(Layout, Floor, Pool, Spot.Cell + SideDir[Spot.Side], Spot.Side, bStairUp, Candidates);
		const FCandidate* Chosen = PickCandidate(Candidates, Rng);
		if (Chosen == nullptr)
		{
			return false;
		}

		const int32 Index = Occupy(Layout, *Chosen, Spot.Hops + 1, Floor, bStairUp);
		SetEdge(Layout, Floor, Spot.Cell, Spot.Side, ECL9GridEdge::Door);
		OpenNeighbours(Layout, Floor, Index, Frontiers[Floor], Rng);
		if (bStairUp)
		{
			// 위층 성장은 계단실의 위쪽 입구에서 시작한다 — 여기 말고는 올라올 길이 없다.
			OpenNeighbours(Layout, Floor + 1, Index, Frontiers[Floor + 1], Rng);
		}
		return true;
	}

	/** 안 쓴 변을 전부 벽으로 막는다 — 모듈 외곽이든, 이웃과 맞물리지 못한 자리든 (REPO ModuleBlockObject) */
	void SealEdges(FCL9SiteLayout& Layout)
	{
		for (int32 Floor = 0; Floor < Layout.FloorCount; ++Floor)
		{
			const int32 BaseX = Floor * Layout.EdgeXStride();
			const int32 BaseY = Floor * Layout.EdgeYStride();
			for (int32 GY = 0; GY < Layout.GridSize.Y; ++GY)
			{
				for (int32 GX = 0; GX <= Layout.GridSize.X; ++GX)
				{
					uint8& State = Layout.EdgeX[BaseX + GX + GY * (Layout.GridSize.X + 1)];
					if (State != static_cast<uint8>(ECL9GridEdge::Open))
					{
						continue;
					}
					const int32 Left = Layout.CellAt(Floor, FIntPoint(GX - 1, GY));
					const int32 Right = Layout.CellAt(Floor, FIntPoint(GX, GY));
					if (Left != Right)
					{
						State = static_cast<uint8>(ECL9GridEdge::Wall);
					}
				}
			}
			for (int32 GY = 0; GY <= Layout.GridSize.Y; ++GY)
			{
				for (int32 GX = 0; GX < Layout.GridSize.X; ++GX)
				{
					uint8& State = Layout.EdgeY[BaseY + GX + GY * Layout.GridSize.X];
					if (State != static_cast<uint8>(ECL9GridEdge::Open))
					{
						continue;
					}
					const int32 Below = Layout.CellAt(Floor, FIntPoint(GX, GY - 1));
					const int32 Above = Layout.CellAt(Floor, FIntPoint(GX, GY));
					if (Below != Above)
					{
						State = static_cast<uint8>(ECL9GridEdge::Wall);
					}
				}
			}
		}
	}

	/**
	 * 비상구를 하나 단다 (module.md 결정 3). 시작 모듈에서 **가장 먼** 막다른 방의
	 * 바깥 벽을 뚫는다 — 그래야 "여기서 뚫고 나갈 수 있나"가 판단거리가 된다.
	 * 막다른 방에 자리가 없으면 가장 먼 아무 모듈이나 받는다.
	 *
	 * **1층에서만 뚫는다** — 위층 벽을 뚫으면 떨어져 내려야 나갈 수 있다.
	 */
	void CarveEmergencyExit(FCL9SiteLayout& Layout)
	{
		int32 BestModule = INDEX_NONE;
		FIntPoint BestCell = FIntPoint::ZeroValue;
		int32 BestSide = 0;
		int32 BestScore = -1;

		for (int32 Index = 0; Index < Layout.Modules.Num(); ++Index)
		{
			const FCL9ModulePlacement& Module = Layout.Modules[Index];
			if (Module.Def->Type == ECL9ModuleType::Start || Module.Floor != 0)
			{
				continue;
			}
			// 막다른 방을 먼저 본다 — 홉이 같아도 막다른 쪽이 이기도록 크게 더한다
			const int32 Score = Module.Hops + (Module.Def->Type == ECL9ModuleType::DeadEnd ? 1000 : 0);
			if (Score <= BestScore)
			{
				continue;
			}
			ForEachBoundaryCell(Module, [&](FIntPoint Cell, int32 Side)
			{
				if (BestScore >= Score)
				{
					return;
				}
				const FIntPoint Outside = Cell + SideDir[Side];
				if (Layout.CellAt(0, Outside) != INDEX_NONE)
				{
					return;
				}
				BestModule = Index;
				BestCell = Cell;
				BestSide = Side;
				BestScore = Score;
			});
		}

		if (BestModule != INDEX_NONE)
		{
			SetEdge(Layout, 0, BestCell, BestSide, ECL9GridEdge::Flap);
		}
	}

	/** 배치된 칸마다 지점 하나 — 용도는 그 모듈의 표 컬럼에서 온다 */
	void BuildPoints(const FCL9SiteParams& Params, FCL9SiteLayout& Layout)
	{
		for (int32 Index = 0; Index < Layout.Modules.Num(); ++Index)
		{
			const FCL9ModulePlacement& Module = Layout.Modules[Index];
			const uint8 Purposes = static_cast<uint8>(Module.Def->PointPurposes & 0xF);
			if (Purposes == 0)
			{
				continue;
			}
			for (int32 Y = 0; Y < Module.Size.Y; ++Y)
			{
				for (int32 X = 0; X < Module.Size.X; ++X)
				{
					FCL9LevelPoint Point;
					Point.Local = CL9SiteLayout::CellCenter(Params, Layout, FIntPoint(Module.Origin.X + X, Module.Origin.Y + Y));
					Point.Purposes = Purposes;
					Point.Module = Index;
					Point.Hops = Module.Hops;
					Point.Floor = Module.Floor;
					Layout.Points.Add(Point);
				}
			}
		}
	}

	/**
	 * 한 층을 채운다. 1층은 본줄기와 Special까지 여기서 놓고, 위층은 계단실에서
	 * 뻗어 나가는 곁가지만 놓는다 — 위층에는 트럭도 입구도 없기 때문이다.
	 *
	 * `StairWant`가 0보다 크면 곁가지 단계에서 그만큼의 계단 모듈을 섞어 놓는다.
	 * **층이 다 자란 뒤가 아니라 자라는 도중에 놓는다** — 다 채운 뒤에는 계단이 설
	 * 빈자리가 남아 있지 않을 수 있다.
	 */
	void GrowFloor(FCL9SiteLayout& Layout, const FCL9SiteParams& Params, int32 Floor, int32 Budget,
		int32 StairWant, const TArray<const UCL9ModuleDef*>* Pools,
		TArray<TArray<FFrontier>>& Frontiers, bool bLeadPhases, FRandomStream& Rng)
	{
		TArray<FFrontier>& Frontier = Frontiers[Floor];
		const TArray<const UCL9ModuleDef*>& StairPool = Pools[static_cast<int32>(ECL9ModuleType::Stair)];
		if (StairPool.Num() == 0)
		{
			StairWant = 0;
		}

		int32 Remaining = FMath::Max(0, Budget - StairWant);
		int32 WantPassage = 0;
		int32 WantDeadEnd = 0;
		int32 WantNormal = 0;

		if (bLeadPhases)
		{
			// ---- Special은 표에 있는 만큼 반드시 넣고, 나머지를 셋으로 나눈다.
			const TArray<const UCL9ModuleDef*>& SpecialPool = Pools[static_cast<int32>(ECL9ModuleType::Special)];
			const int32 SpecialWant = FMath::Min(SpecialPool.Num(), Remaining);
			Remaining -= SpecialWant;

			WantPassage = FMath::RoundToInt(Remaining * PassageShare);
			WantDeadEnd = FMath::RoundToInt(Remaining * DeadEndShare);
			WantNormal = FMath::Max(0, Remaining - WantPassage - WantDeadEnd);

			TArray<const UCL9ModuleDef*> MainPool = Pools[static_cast<int32>(ECL9ModuleType::Passage)];
			MainPool.Append(Pools[static_cast<int32>(ECL9ModuleType::Normal)]);

			// ---- 1단계: 본줄기. 가장 깊은 자리만 이어 붙여서 한 줄기로 멀리 뻗는다.
			const int32 MainCount = FMath::Min(Remaining, FMath::Max(2, FMath::RoundToInt(Params.ModuleCount * MainPathShare)));
			for (int32 Placed = 0; Placed < MainCount; ++Placed)
			{
				FFrontier Spot;
				if (!TakeFrontier(Frontier, Layout, Floor, /*bDeepest*/ true, Rng, Spot)
					|| !PlaceAt(Layout, Floor, Spot, MainPool, /*bStairUp*/ false, Frontiers, Rng))
				{
					break;
				}
				// 어느 타입이 나왔는지는 마지막에 놓인 행이 말해 준다
				const ECL9ModuleType Type = Layout.Modules.Last().Def->Type;
				(Type == ECL9ModuleType::Passage ? WantPassage : WantNormal) -= 1;
			}

			// ---- 2단계: Special. 곁가지보다 먼저 넣어서 자리를 확보하되, 본줄기가 이미
			//      뻗은 뒤라 입구 코앞에 붙지 않는다.
			for (const UCL9ModuleDef* Def : SpecialPool)
			{
				const TArray<const UCL9ModuleDef*> One = {Def};
				for (int32 Attempt = 0; Attempt < 24; ++Attempt)
				{
					FFrontier Spot;
					if (!TakeFrontier(Frontier, Layout, Floor, /*bDeepest*/ false, Rng, Spot))
					{
						break;
					}
					if (PlaceAt(Layout, Floor, Spot, One, /*bStairUp*/ false, Frontiers, Rng))
					{
						break;
					}
				}
			}
		}
		else
		{
			WantPassage = FMath::RoundToInt(Remaining * PassageShare);
			WantDeadEnd = FMath::RoundToInt(Remaining * DeadEndShare);
			WantNormal = FMath::Max(0, Remaining - WantPassage - WantDeadEnd);
		}

		// ---- 3단계: 곁가지. 남은 쿼터에서 타입을 뽑고, 막다른 방으로 끝을 맺는다.
		//      한 번 실패했다고 쿼터를 깎으면 판이 눈에 띄게 작아진다 — 자리가 안 맞은 것과
		//      쓸 모듈이 없는 것은 다르므로, 연달아 막힐 때만 손을 뗀다.
		int32 Stall = 0;
		while (WantPassage + WantDeadEnd + WantNormal + StairWant > 0 && Stall < StallLimit)
		{
			FFrontier Spot;
			if (!TakeFrontier(Frontier, Layout, Floor, /*bDeepest*/ false, Rng, Spot))
			{
				break;
			}

			// 막다른 방은 가지를 닫는다 — 뻗을 자리가 얼마 안 남았는데 닫아 버리면
			// 그 뒤의 예산이 통째로 남는다. 여유가 없으면 뻗는 타입을 먼저 쓴다.
			const bool bMayClose = Frontier.Num() > 2 || WantPassage + WantNormal == 0;

			TArray<const UCL9ModuleDef*> Pool;
			int32* Quota = nullptr;
			bool bStairUp = false;
			const int32 Choices = StairWant + WantPassage + WantNormal + (bMayClose ? WantDeadEnd : 0);
			const int32 Roll = Rng.RandRange(0, FMath::Max(Choices, 1) - 1);
			if (Roll < StairWant)
			{
				Pool = StairPool;
				Quota = &StairWant;
				bStairUp = true;
			}
			else if (Roll < StairWant + WantPassage)
			{
				Pool = Pools[static_cast<int32>(ECL9ModuleType::Passage)];
				Quota = &WantPassage;
			}
			else if (Roll < StairWant + WantPassage + WantNormal)
			{
				Pool = Pools[static_cast<int32>(ECL9ModuleType::Normal)];
				Quota = &WantNormal;
			}
			else
			{
				Pool = Pools[static_cast<int32>(ECL9ModuleType::DeadEnd)];
				Quota = &WantDeadEnd;
			}

			if (PlaceAt(Layout, Floor, Spot, Pool, bStairUp, Frontiers, Rng))
			{
				*Quota -= 1;
				Stall = 0;
			}
			else
			{
				++Stall;
			}
		}
	}

	/** 계단 하나를 되돌린다 — 위층 칸을 비우고 그 둘레의 변을 열린 상태로 돌려놓는다 */
	void DemoteStair(FCL9SiteLayout& Layout, int32 StairIndex, TArray<FFrontier>& UpperFrontier)
	{
		FCL9ModulePlacement& Stair = Layout.Modules[StairIndex];
		const int32 Upper = Stair.Floor + 1;
		const int32 Base = Upper * Layout.CellStride();
		for (int32 Y = 0; Y < Stair.Size.Y; ++Y)
		{
			for (int32 X = 0; X < Stair.Size.X; ++X)
			{
				const FIntPoint Cell(Stair.Origin.X + X, Stair.Origin.Y + Y);
				Layout.CellModule[Base + Cell.X + Cell.Y * Layout.GridSize.X] = INDEX_NONE;
				// 위층에서 이 칸 둘레에 뚫렸던 문(고리)도 같이 지운다 — 빈 칸 사이에 문이 남으면
				// 허공에 문짝이 선다.
				for (int32 Side = 0; Side < 4; ++Side)
				{
					SetEdge(Layout, Upper, Cell, Side, ECL9GridEdge::Open);
				}
			}
		}
		Stair.bStairUp = false;
		UpperFrontier.RemoveAll([StairIndex](const FFrontier& Entry) { return Entry.Module == StairIndex; });
	}

	/**
	 * 층 Floor에 선 계단마다 **위층을 먼저 틔운다.** 계단 하나가 자기 둘레에 StairMinReach개
	 * 이상의 모듈을 이어 붙이지 못하면 그 시도를 통째로 무르고 계단을 되돌린다. 살아남은
	 * 계단 수를 돌려주고, 위층에 쓴 모듈 수만큼 UpperBudget을 깎는다.
	 *
	 * 무르기는 배치 직전의 칸·변·프론티어를 통째로 복사해 두었다가 되돌리는 방식이다 —
	 * 격자가 22×22 정도라 복사 비용이 없다시피 하고, 되돌린 뒤의 상태가 정확하다.
	 */
	int32 SecureStairs(FCL9SiteLayout& Layout, int32 Floor, int32& UpperBudget,
		const TArray<const UCL9ModuleDef*>* Pools, TArray<TArray<FFrontier>>& Frontiers, FRandomStream& Rng)
	{
		const int32 Upper = Floor + 1;
		if (!Layout.IsFloor(Upper))
		{
			return 0;
		}

		// 위층 첫 방들은 방(Normal)과 복도(Passage)로만 틔운다 — 막다른 방으로 시작하면
		// 계단을 올라와서 곧장 막힌다.
		TArray<const UCL9ModuleDef*> RoomPool = Pools[static_cast<int32>(ECL9ModuleType::Normal)];
		RoomPool.Append(Pools[static_cast<int32>(ECL9ModuleType::Passage)]);

		TArray<int32> Stairs;
		for (int32 Index = 0; Index < Layout.Modules.Num(); ++Index)
		{
			if (Layout.Modules[Index].bStairUp && Layout.Modules[Index].Floor == Floor)
			{
				Stairs.Add(Index);
			}
		}

		int32 Survivors = 0;
		for (int32 StairIndex : Stairs)
		{
			// 예산이 최소치에도 못 미치면 남은 계단은 전부 되돌린다
			if (UpperBudget < StairMinReach)
			{
				DemoteStair(Layout, StairIndex, Frontiers[Upper]);
				continue;
			}

			const TArray<int32> SavedCells = Layout.CellModule;
			const TArray<uint8> SavedEdgeX = Layout.EdgeX;
			const TArray<uint8> SavedEdgeY = Layout.EdgeY;
			const TArray<FFrontier> SavedFrontier = Frontiers[Upper];
			const int32 SavedModules = Layout.Modules.Num();

			// 이 계단에서 이어진 모듈들 — 프론티어 중 이 무리에서 나온 것만 골라 뻗는다
			TSet<int32> Region;
			Region.Add(StairIndex);
			int32 Placed = 0;
			int32 Stall = 0;
			while (Placed < StairMinReach && Stall < StallLimit)
			{
				TArray<int32> Mine;
				for (int32 Index = 0; Index < Frontiers[Upper].Num(); ++Index)
				{
					if (Region.Contains(Frontiers[Upper][Index].Module))
					{
						Mine.Add(Index);
					}
				}
				if (Mine.Num() == 0)
				{
					break;
				}
				const int32 Pick = Mine[Rng.RandRange(0, Mine.Num() - 1)];
				const FFrontier Spot = Frontiers[Upper][Pick];
				Frontiers[Upper].RemoveAtSwap(Pick, EAllowShrinking::No);

				const FIntPoint Target = Spot.Cell + SideDir[Spot.Side];
				if (!Layout.IsInside(Target) || Layout.CellAt(Upper, Target) != INDEX_NONE)
				{
					continue;
				}
				if (PlaceAt(Layout, Upper, Spot, RoomPool, /*bStairUp*/ false, Frontiers, Rng))
				{
					Region.Add(Layout.Modules.Num() - 1);
					++Placed;
					Stall = 0;
				}
				else
				{
					++Stall;
				}
			}

			if (Placed >= StairMinReach)
			{
				UpperBudget -= Placed;
				++Survivors;
				continue;
			}

			// 못 틔웠다 — 시도 전으로 되돌리고 계단을 내린다
			Layout.CellModule = SavedCells;
			Layout.EdgeX = SavedEdgeX;
			Layout.EdgeY = SavedEdgeY;
			Frontiers[Upper] = SavedFrontier;
			Layout.Modules.SetNum(SavedModules);
			DemoteStair(Layout, StairIndex, Frontiers[Upper]);
		}
		return Survivors;
	}
}

FCL9SiteLayout CL9SiteLayout::Generate(const FCL9SiteParams& Params, const TArray<const UCL9ModuleDef*>& Modules, uint32 Seed)
{
	FCL9SiteLayout Layout;
	Layout.Seed = Seed;
	if (Seed == 0 || Modules.Num() == 0 || Params.ModuleCount < 2)
	{
		return Layout;
	}

	// 타입별 후보 묶음. 시작 모듈이 없으면 붙일 곳이 없어서 한 발도 못 뗀다.
	TArray<const UCL9ModuleDef*> Pools[CL9ModuleTypeCount];
	for (const UCL9ModuleDef* Def : Modules)
	{
		Pools[static_cast<int32>(Def->Type)].Add(Def);
	}
	const TArray<const UCL9ModuleDef*>& StartPool = Pools[static_cast<int32>(ECL9ModuleType::Start)];
	if (StartPool.Num() == 0)
	{
		return Layout;
	}

	// 계단 모듈이 한 행도 없으면 위층으로 올라갈 방법이 없다 — 단층으로 짓는다.
	const bool bCanClimb = Pools[static_cast<int32>(ECL9ModuleType::Stair)].Num() > 0;
	Layout.FloorCount = bCanClimb ? FMath::Clamp(Params.MaxFloors, 1, 8) : 1;

	// 격자는 모듈이 사방으로 퍼질 만큼만 잡는다 — 너무 크면 배치가 흩어지고,
	// 너무 작으면 후보가 자꾸 격자 밖으로 떨어져 판이 짧아진다.
	const int32 Span = FMath::Clamp(FMath::CeilToInt(FMath::Sqrt(Params.ModuleCount * 6.f)) * 2, 16, 64);
	Layout.GridSize = FIntPoint(Span, Span);
	Layout.CellModule.Init(INDEX_NONE, Layout.FloorCount * Span * Span);
	Layout.EdgeX.Init(static_cast<uint8>(ECL9GridEdge::Open), Layout.FloorCount * (Span + 1) * Span);
	Layout.EdgeY.Init(static_cast<uint8>(ECL9GridEdge::Open), Layout.FloorCount * Span * (Span + 1));
	Layout.Origin = FVector2D(Params.EntryOffset, -Span * Params.CellSize * 0.5f);

	FRandomStream Rng(static_cast<int32>(Seed));

	// ---- 시작 모듈: 서쪽 변이 트럭 쪽으로 열리도록 회전을 고르고 격자 서쪽 끝에 붙인다
	TArray<FCandidate> StartCandidates;
	for (const UCL9ModuleDef* Def : StartPool)
	{
		for (int32 Rotation = 0; Rotation < 4; ++Rotation)
		{
			const uint8 Sockets = UCL9ModuleDef::RotateSockets(static_cast<uint8>(Def->Sockets & 0xF), Rotation);
			if (!HasSide(Sockets, SideW))
			{
				continue;
			}
			const FIntPoint Size = UCL9ModuleDef::RotateSize(Def->Size, Rotation);
			FCandidate Cand;
			Cand.Def = Def;
			Cand.Origin = FIntPoint(0, (Span - Size.Y) / 2);
			Cand.Size = Size;
			Cand.Rotation = Rotation;
			Cand.Sockets = Sockets;
			if (Fits(Layout, 0, Cand.Origin, Cand.Size) && !StartCandidates.ContainsByPredicate([&Cand](const FCandidate& Other) { return Cand.SameOutcome(Other); }))
			{
				StartCandidates.Add(Cand);
			}
		}
	}
	const FCandidate* StartPick = PickCandidate(StartCandidates, Rng);
	if (StartPick == nullptr)
	{
		return Layout;
	}

	const int32 StartIndex = Occupy(Layout, *StartPick, 0, /*Floor*/ 0, /*bStairUp*/ false);
	Layout.EntryCell = FIntPoint(StartPick->Origin.X, StartPick->Origin.Y + StartPick->Size.Y / 2);
	// 입구 문은 격자 바깥으로 난다 — 트럭은 밖에 있고 걸어 들어온다 (module.md 결정 6)
	SetEdge(Layout, 0, Layout.EntryCell, SideW, ECL9GridEdge::Door);

	TArray<TArray<FFrontier>> Frontiers;
	Frontiers.SetNum(Layout.FloorCount);
	OpenNeighbours(Layout, 0, StartIndex, Frontiers[0], Rng);

	// ---- 층별 예산. **1층은 ModuleCount를 그대로 받고, 위층은 그 절반씩 덧붙는다**
	//      (2026-09-10 사용자 결정). 층을 켠다고 1층이 좁아지지 않는다 — 2층을 켜는 게 곧
	//      맵이 커지는 일이다. 위층은 아래층이 받쳐 주는 자리에만 설 수 있어서 절반이면
	//      충분히 채운다. 계단이 위층을 틔울 때 위층 예산을 먼저 당겨 쓰므로 배열이어야 한다.
	TArray<int32> FloorBudget;
	Layout.Wanted = 1;
	for (int32 Floor = 0; Floor < Layout.FloorCount; ++Floor)
	{
		const int32 Budget = Floor == 0
			? Params.ModuleCount - 1
			: FMath::Max(0, FMath::RoundToInt(Params.ModuleCount / static_cast<float>(1 << Floor)));
		FloorBudget.Add(Budget);
		Layout.Wanted += Budget;
	}

	for (int32 Floor = 0; Floor < Layout.FloorCount; ++Floor)
	{
		const bool bTop = Floor == Layout.FloorCount - 1;

		// 위층이 있으면 계단을 그만큼 섞어 놓는다. 시드가 뽑으므로 판마다 다르다.
		const int32 StairWant = bTop
			? 0
			: FMath::Max(1, Rng.RandRange(FMath::Max(1, Params.StairCountMin), FMath::Max(1, Params.StairCountMax)));

		GrowFloor(Layout, Params, Floor, FloorBudget[Floor], StairWant, Pools, Frontiers, /*bLeadPhases*/ Floor == 0, Rng);

		if (bTop)
		{
			break;
		}

		// **계단마다 위층을 먼저 틔운다.** 자기 둘레에 방을 못 붙이는 계단은 되돌리므로,
		// 살아남은 계단은 올라가면 반드시 갈 데가 있다. 하나도 못 살아남으면 위층은 없다.
		if (SecureStairs(Layout, Floor, FloorBudget[Floor + 1], Pools, Frontiers, Rng) == 0)
		{
			Layout.FloorCount = Floor + 1;
			break;
		}
	}

	// 층이 줄었으면 그 위로 뻗던 계단은 더 이상 계단이 아니다 — 그냥 방으로 남긴다.
	for (FCL9ModulePlacement& Module : Layout.Modules)
	{
		if (Module.bStairUp && Module.Floor + 1 >= Layout.FloorCount)
		{
			Module.bStairUp = false;
		}
	}

	// 층이 줄었으면 남는 층의 칸·변 배열을 잘라 낸다 — 해시와 순회가 실제 층만 보도록.
	Layout.CellModule.SetNum(Layout.FloorCount * Layout.CellStride());
	Layout.EdgeX.SetNum(Layout.FloorCount * Layout.EdgeXStride());
	Layout.EdgeY.SetNum(Layout.FloorCount * Layout.EdgeYStride());

	SealEdges(Layout);
	CarveEmergencyExit(Layout);
	BuildPoints(Params, Layout);
	Layout.bBuilt = true;
	return Layout;
}

void CL9SiteLayout::BuildSegments(const FCL9SiteParams& Params, const FCL9SiteLayout& Layout, TArray<FCL9SiteSeg>& Out)
{
	Out.Reset();
	if (!Layout.bBuilt)
	{
		return;
	}

	const float Cell = Params.CellSize;
	const float Half = Params.WallThickness * 0.5f;
	const float Storey = FMath::Max(Params.FloorHeight, 1.f);
	const FVector2D& Base = Layout.Origin;

	auto Add = [&Out](FName Kind, const FBox2D& Rect, float BaseZ, FVector2D Outward = FVector2D::ZeroVector)
	{
		if (Rect.GetSize().X > KINDA_SMALL_NUMBER && Rect.GetSize().Y > KINDA_SMALL_NUMBER)
		{
			Out.Add({Kind, Rect, BaseZ, Outward});
		}
	};

	// ---- 바닥과 천장은 모듈 단위로 한 판씩 깐다. 모듈 사각형끼리는 빈틈 없이 맞물리므로
	//      칸마다 쪼갤 이유가 없고, 조각으로 채우는 건 어차피 타일링이 한다.
	for (const FCL9ModulePlacement& Module : Layout.Modules)
	{
		const FBox2D Rect(
			Base + FVector2D(Module.Origin.X * Cell, Module.Origin.Y * Cell),
			Base + FVector2D((Module.Origin.X + Module.Size.X) * Cell, (Module.Origin.Y + Module.Size.Y) * Cell));

		// 바닥은 그 모듈이 선 층에 깐다. 계단 모듈은 아래층에만 깔아서 위층 쪽이
		// 뚫린 채로 남는다 — 그 구멍이 계단실이다.
		Add(CL9SegKind::Floor(), Rect, Module.Floor * Storey);

		// 천장은 그 모듈이 차지한 가장 높은 층 위에 덮는다. 사이트가 천장을 끄면
		// 위에서 맵 속이 들여다보인다 — 층은 자기 바닥 판을 따로 깔므로 무너지지 않는다.
		if (!Module.Def->bOutdoor && Params.bBuildCeilings)
		{
			Add(CL9SegKind::Ceiling(), Rect, Module.TopFloor() * Storey + Module.Def->Height);
		}

		if (!Module.bStairUp)
		{
			continue;
		}

		// ---- 디딤판. 층 높이를 StepRise로 나눠 개수를 정하고, 모듈의 긴 축을 따라
		//      한 칸씩 올라간다. 디딤판도 바닥 종류라, 조각 표를 새로 만들 필요가 없다.
		//      폭은 문 두 짝만큼만 쓴다 — 방 전체가 경사로가 되면 계단실이 아니라 언덕이다.
		const FVector2D Span = Rect.GetSize();
		const bool bAlongX = Span.X >= Span.Y;
		const float Run = bAlongX ? Span.X : Span.Y;
		const float Across = bAlongX ? Span.Y : Span.X;
		const float Width = FMath::Min(Params.DoorWidth * 2.f, Across);
		const float Mid = bAlongX ? Rect.GetCenter().Y : Rect.GetCenter().X;

		// 오르기 시작하는 쪽에 참(landing)을 조금 남긴다 — 그 자리 위층에 바닥을 깔아야
		// 위층에서 그 변으로 난 문이 허공으로 열리지 않는다.
		const float Landing = Run * 0.15f;
		const float Near = bAlongX ? Rect.Min.X : Rect.Min.Y;
		const float RunStart = Near + Landing;

		const int32 Steps = FMath::Max(2, FMath::RoundToInt(Storey / StepRise));
		const float Tread = (Run - Landing) / static_cast<float>(Steps);
		const float StartZ = Module.Floor * Storey;

		auto AcrossBox = [&](float A0, float A1, float B0, float B1)
		{
			return bAlongX
				? FBox2D(FVector2D(A0, B0), FVector2D(A1, B1))
				: FBox2D(FVector2D(B0, A0), FVector2D(B1, A1));
		};

		for (int32 Step = 0; Step < Steps; ++Step)
		{
			const float A0 = RunStart + Step * Tread;
			Add(CL9SegKind::Floor(), AcrossBox(A0, A0 + Tread, Mid - Width * 0.5f, Mid + Width * 0.5f),
				StartZ + Storey * (Step + 1) / static_cast<float>(Steps));
		}

		// ---- 위층 바닥. 계단이 뚫고 올라오는 통로만 비우고 나머지를 덮는다.
		//      **이게 없으면 올라와서 디딤판 끝에 선 채로 어디로도 못 간다** — 계단실의
		//      나머지가 아래층까지 뻥 뚫린 구멍이라, 그 변에 난 문이 허공으로 열린다.
		const float Far = bAlongX ? Rect.Max.X : Rect.Max.Y;
		const float Side0 = bAlongX ? Rect.Min.Y : Rect.Min.X;
		const float Side1 = bAlongX ? Rect.Max.Y : Rect.Max.X;
		const float UpperZ = (Module.Floor + 1) * Storey;

		// 통로 양옆으로 지나다닐 수 있는 띠
		Add(CL9SegKind::Floor(), AcrossBox(Near, Far, Side0, Mid - Width * 0.5f), UpperZ);
		Add(CL9SegKind::Floor(), AcrossBox(Near, Far, Mid + Width * 0.5f, Side1), UpperZ);
		// 오르기 시작하는 쪽의 참 — 그 아래가 계단 밑 공간이다
		Add(CL9SegKind::Floor(), AcrossBox(Near, RunStart, Mid - Width * 0.5f, Mid + Width * 0.5f), UpperZ);
	}

	// 벽 종류는 양옆이 다 야외일 때만 마당 울타리다 — 마당과 축사가 맞닿은 자리는
	// 축사의 바깥벽이지 울타리가 아니다.
	auto WallKind = [&Layout](int32 A, int32 B)
	{
		bool bAnyIndoor = false;
		for (int32 Index : {A, B})
		{
			if (Index != INDEX_NONE && !Layout.Modules[Index].Def->bOutdoor)
			{
				bAnyIndoor = true;
			}
		}
		return bAnyIndoor ? CL9SegKind::Barn() : CL9SegKind::Wall();
	};

	/**
	 * 변 하나를 세운다. 문·비상구는 가운데를 DoorWidth만큼 비우고 양옆에 벽 토막을
	 * 남긴다 — 그 빈칸에 문짝(또는 여닫이문)이 선다.
	 */
	auto AddEdge = [&](ECL9GridEdge State, FName Kind, float Along0, float Along1, float Across,
		bool bAlongX, FVector2D Outward, float BaseZ)
	{
		auto Box = [&](float A0, float A1)
		{
			return bAlongX
				? FBox2D(FVector2D(A0, Across - Half), FVector2D(A1, Across + Half))
				: FBox2D(FVector2D(Across - Half, A0), FVector2D(Across + Half, A1));
		};

		if (State == ECL9GridEdge::Wall)
		{
			Add(Kind, Box(Along0, Along1), BaseZ);
			return;
		}

		const float Middle = (Along0 + Along1) * 0.5f;
		const float Gap = FMath::Min(Params.DoorWidth, Along1 - Along0);
		Add(Kind, Box(Along0, Middle - Gap * 0.5f), BaseZ);
		Add(Kind, Box(Middle + Gap * 0.5f, Along1), BaseZ);
		Add(State == ECL9GridEdge::Flap ? CL9SegKind::FlapIn() : CL9SegKind::Door(),
			Box(Middle - Gap * 0.5f, Middle + Gap * 0.5f), BaseZ, Outward);
	};

	for (int32 Floor = 0; Floor < Layout.FloorCount; ++Floor)
	{
		const float BaseZ = Floor * Storey;
		for (int32 GY = 0; GY < Layout.GridSize.Y; ++GY)
		{
			for (int32 GX = 0; GX <= Layout.GridSize.X; ++GX)
			{
				const ECL9GridEdge State = Layout.EdgeAtX(Floor, GX, GY);
				if (State == ECL9GridEdge::Open)
				{
					continue;
				}
				const int32 West = Layout.CellAt(Floor, FIntPoint(GX - 1, GY));
				const int32 East = Layout.CellAt(Floor, FIntPoint(GX, GY));
				AddEdge(State, WallKind(West, East),
					Base.Y + GY * Cell, Base.Y + (GY + 1) * Cell, Base.X + GX * Cell,
					/*bAlongX*/ false, FVector2D(East == INDEX_NONE ? 1.f : -1.f, 0.f), BaseZ);
			}
		}
		for (int32 GY = 0; GY <= Layout.GridSize.Y; ++GY)
		{
			for (int32 GX = 0; GX < Layout.GridSize.X; ++GX)
			{
				const ECL9GridEdge State = Layout.EdgeAtY(Floor, GX, GY);
				if (State == ECL9GridEdge::Open)
				{
					continue;
				}
				const int32 South = Layout.CellAt(Floor, FIntPoint(GX, GY - 1));
				const int32 North = Layout.CellAt(Floor, FIntPoint(GX, GY));
				AddEdge(State, WallKind(South, North),
					Base.X + GX * Cell, Base.X + (GX + 1) * Cell, Base.Y + GY * Cell,
					/*bAlongX*/ true, FVector2D(0.f, North == INDEX_NONE ? 1.f : -1.f), BaseZ);
			}
		}
	}
}

FVector2D CL9SiteLayout::CellCenter(const FCL9SiteParams& Params, const FCL9SiteLayout& Layout, FIntPoint Cell)
{
	return Layout.Origin + FVector2D((Cell.X + 0.5f) * Params.CellSize, (Cell.Y + 0.5f) * Params.CellSize);
}

void CL9SiteLayout::PointsOfPurpose(const FCL9SiteLayout& Layout, ECL9PointPurpose Purpose, TArray<int32>& OutIndices)
{
	OutIndices.Reset();
	for (int32 Index = 0; Index < Layout.Points.Num(); ++Index)
	{
		if (Layout.Points[Index].Has(Purpose))
		{
			OutIndices.Add(Index);
		}
	}
	OutIndices.Sort([&Layout](int32 A, int32 B) { return Layout.Points[A].Hops > Layout.Points[B].Hops; });
}

bool CL9SiteLayout::IsConnected(const FCL9SiteLayout& Layout)
{
	if (Layout.Modules.Num() == 0)
	{
		return true;
	}

	TArray<uint8> Seen;
	Seen.Init(0, Layout.Modules.Num());
	TArray<int32> Queue;
	Queue.Add(0);
	Seen[0] = 1;
	int32 Reached = 0;

	while (Queue.Num() > 0)
	{
		const int32 Index = Queue.Pop(EAllowShrinking::No);
		++Reached;
		const FCL9ModulePlacement Module = Layout.Modules[Index];
		// 계단 모듈은 두 층에 걸쳐 있다 — 두 층의 문을 모두 훑어야 위층이 이어진다.
		for (int32 Floor = Module.Floor; Floor <= Module.TopFloor(); ++Floor)
		{
			if (!Layout.IsFloor(Floor))
			{
				continue;
			}
			ForEachBoundaryCell(Module, [&](FIntPoint Cell, int32 Side)
			{
				if (GetEdge(Layout, Floor, Cell, Side) != ECL9GridEdge::Door)
				{
					return;
				}
				const int32 Other = Layout.CellAt(Floor, Cell + SideDir[Side]);
				if (Other != INDEX_NONE && !Seen[Other])
				{
					Seen[Other] = 1;
					Queue.Add(Other);
				}
			});
		}
	}
	return Reached == Layout.Modules.Num();
}

uint32 CL9SiteLayout::LayoutHash(const FCL9SiteLayout& Layout)
{
	uint32 Hash = 2166136261u;
	FnvMix(Hash, Layout.Seed);
	FnvMix(Hash, static_cast<uint32>(Layout.GridSize.X));
	FnvMix(Hash, static_cast<uint32>(Layout.GridSize.Y));
	FnvMix(Hash, static_cast<uint32>(Layout.FloorCount));
	for (const FCL9ModulePlacement& Module : Layout.Modules)
	{
		FnvMix(Hash, GetTypeHash(Module.Def ? Module.Def->Alias : NAME_None));
		FnvMix(Hash, static_cast<uint32>(Module.Origin.X));
		FnvMix(Hash, static_cast<uint32>(Module.Origin.Y));
		FnvMix(Hash, static_cast<uint32>(Module.Rotation));
		FnvMix(Hash, static_cast<uint32>(Module.Floor));
	}
	for (uint8 Edge : Layout.EdgeX) FnvMix(Hash, Edge);
	for (uint8 Edge : Layout.EdgeY) FnvMix(Hash, Edge);
	return Hash;
}
