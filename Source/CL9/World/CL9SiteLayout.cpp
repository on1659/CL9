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

	/** 격자 안이고 모든 칸이 비어 있는가 */
	bool Fits(const FCL9SiteLayout& Layout, FIntPoint Origin, FIntPoint Size)
	{
		if (Origin.X < 0 || Origin.Y < 0 || Origin.X + Size.X > Layout.GridSize.X || Origin.Y + Size.Y > Layout.GridSize.Y)
		{
			return false;
		}
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			for (int32 X = 0; X < Size.X; ++X)
			{
				if (Layout.CellModule[(Origin.X + X) + (Origin.Y + Y) * Layout.GridSize.X] != INDEX_NONE)
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
	void SetEdge(FCL9SiteLayout& Layout, FIntPoint Cell, int32 Side, ECL9GridEdge State)
	{
		const uint8 Value = static_cast<uint8>(State);
		switch (Side)
		{
		case SideE: Layout.EdgeX[(Cell.X + 1) + Cell.Y * (Layout.GridSize.X + 1)] = Value; break;
		case SideW: Layout.EdgeX[Cell.X + Cell.Y * (Layout.GridSize.X + 1)] = Value; break;
		case SideN: Layout.EdgeY[Cell.X + (Cell.Y + 1) * Layout.GridSize.X] = Value; break;
		default: Layout.EdgeY[Cell.X + Cell.Y * Layout.GridSize.X] = Value; break;
		}
	}

	ECL9GridEdge GetEdge(const FCL9SiteLayout& Layout, FIntPoint Cell, int32 Side)
	{
		switch (Side)
		{
		case SideE: return Layout.EdgeAtX(Cell.X + 1, Cell.Y);
		case SideW: return Layout.EdgeAtX(Cell.X, Cell.Y);
		case SideN: return Layout.EdgeAtY(Cell.X, Cell.Y + 1);
		default: return Layout.EdgeAtY(Cell.X, Cell.Y);
		}
	}

	/** 모듈을 격자에 새기고 그 인덱스를 돌려준다 */
	int32 Occupy(FCL9SiteLayout& Layout, const FCandidate& Cand, int32 Hops)
	{
		FCL9ModulePlacement Placement;
		Placement.Def = Cand.Def;
		Placement.Origin = Cand.Origin;
		Placement.Size = Cand.Size;
		Placement.Rotation = Cand.Rotation;
		Placement.Sockets = Cand.Sockets;
		Placement.Hops = Hops;
		const int32 Index = Layout.Modules.Add(Placement);

		for (int32 Y = 0; Y < Cand.Size.Y; ++Y)
		{
			for (int32 X = 0; X < Cand.Size.X; ++X)
			{
				Layout.CellModule[(Cand.Origin.X + X) + (Cand.Origin.Y + Y) * Layout.GridSize.X] = Index;
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
	void GatherCandidates(const FCL9SiteLayout& Layout, const TArray<const UCL9ModuleDef*>& Pool, FIntPoint Target, int32 Side, TArray<FCandidate>& Out)
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
					if (!Fits(Layout, Origin, Size))
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
	 * 새로 놓인 모듈의 열린 경계 칸들을 훑어서, 빈 쪽은 프론티어에 넣고 이미 모듈이
	 * 있는 쪽은 확률로 문을 뚫는다(고리).
	 */
	void OpenNeighbours(FCL9SiteLayout& Layout, int32 ModuleIndex, TArray<FFrontier>& Frontier, FRandomStream& Rng)
	{
		const FCL9ModulePlacement Module = Layout.Modules[ModuleIndex];
		ForEachBoundaryCell(Module, [&](FIntPoint Cell, int32 Side)
		{
			const FIntPoint Neighbour = Cell + SideDir[Side];
			if (!Layout.IsInside(Neighbour))
			{
				return;
			}
			const int32 Other = Layout.CellAt(Neighbour);
			if (Other == INDEX_NONE)
			{
				Frontier.Add({ModuleIndex, Cell, Side, Module.Hops});
				return;
			}
			if (Other == ModuleIndex || GetEdge(Layout, Cell, Side) != ECL9GridEdge::Open)
			{
				return;
			}
			// 이웃도 이쪽 변이 열려 있어야 문이 된다. 이웃의 그 칸은 반드시 이웃의
			// 경계 칸이다 — 우리 쪽 칸이 그 모듈 밖에 있으니까.
			if (HasSide(Layout.Modules[Other].Sockets, Opposite(Side)) && Rng.FRand() < LoopChance)
			{
				SetEdge(Layout, Cell, Side, ECL9GridEdge::Door);
			}
		});
	}

	/** 프론티어에서 하나를 꺼낸다 — 본줄기 단계에선 가장 깊은 것, 곁가지 단계에선 무작위 */
	bool TakeFrontier(TArray<FFrontier>& Frontier, const FCL9SiteLayout& Layout, bool bDeepest, FRandomStream& Rng, FFrontier& Out)
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
			if (Layout.IsInside(Target) && Layout.CellAt(Target) == INDEX_NONE)
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * 한 자리에 모듈 하나를 놓는다. Pool이 비었거나 그 자리에 맞는 게 없으면 false —
	 * 호출부는 다음 프론티어로 넘어간다.
	 */
	bool PlaceAt(FCL9SiteLayout& Layout, const FFrontier& Spot, const TArray<const UCL9ModuleDef*>& Pool, TArray<FFrontier>& Frontier, FRandomStream& Rng)
	{
		if (Pool.Num() == 0)
		{
			return false;
		}
		TArray<FCandidate> Candidates;
		GatherCandidates(Layout, Pool, Spot.Cell + SideDir[Spot.Side], Spot.Side, Candidates);
		const FCandidate* Chosen = PickCandidate(Candidates, Rng);
		if (Chosen == nullptr)
		{
			return false;
		}

		const int32 Index = Occupy(Layout, *Chosen, Spot.Hops + 1);
		SetEdge(Layout, Spot.Cell, Spot.Side, ECL9GridEdge::Door);
		OpenNeighbours(Layout, Index, Frontier, Rng);
		return true;
	}

	/** 안 쓴 변을 전부 벽으로 막는다 — 모듈 외곽이든, 이웃과 맞물리지 못한 자리든 (REPO ModuleBlockObject) */
	void SealEdges(FCL9SiteLayout& Layout)
	{
		for (int32 GY = 0; GY < Layout.GridSize.Y; ++GY)
		{
			for (int32 GX = 0; GX <= Layout.GridSize.X; ++GX)
			{
				uint8& State = Layout.EdgeX[GX + GY * (Layout.GridSize.X + 1)];
				if (State != static_cast<uint8>(ECL9GridEdge::Open))
				{
					continue;
				}
				const int32 Left = Layout.CellAt(FIntPoint(GX - 1, GY));
				const int32 Right = Layout.CellAt(FIntPoint(GX, GY));
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
				uint8& State = Layout.EdgeY[GX + GY * Layout.GridSize.X];
				if (State != static_cast<uint8>(ECL9GridEdge::Open))
				{
					continue;
				}
				const int32 Below = Layout.CellAt(FIntPoint(GX, GY - 1));
				const int32 Above = Layout.CellAt(FIntPoint(GX, GY));
				if (Below != Above)
				{
					State = static_cast<uint8>(ECL9GridEdge::Wall);
				}
			}
		}
	}

	/**
	 * 비상구를 하나 단다 (module.md 결정 3). 시작 모듈에서 **가장 먼** 막다른 방의
	 * 바깥 벽을 뚫는다 — 그래야 "여기서 뚫고 나갈 수 있나"가 판단거리가 된다.
	 * 막다른 방에 자리가 없으면 가장 먼 아무 모듈이나 받는다.
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
			if (Module.Def->Type == ECL9ModuleType::Start)
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
				if (Layout.CellAt(Outside) != INDEX_NONE)
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
			SetEdge(Layout, BestCell, BestSide, ECL9GridEdge::Flap);
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
					Layout.Points.Add(Point);
				}
			}
		}
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
	TArray<const UCL9ModuleDef*> Pools[5];
	for (const UCL9ModuleDef* Def : Modules)
	{
		Pools[static_cast<int32>(Def->Type)].Add(Def);
	}
	const TArray<const UCL9ModuleDef*>& StartPool = Pools[static_cast<int32>(ECL9ModuleType::Start)];
	if (StartPool.Num() == 0)
	{
		return Layout;
	}

	// 격자는 모듈이 사방으로 퍼질 만큼만 잡는다 — 너무 크면 배치가 흩어지고,
	// 너무 작으면 후보가 자꾸 격자 밖으로 떨어져 판이 짧아진다.
	const int32 Span = FMath::Clamp(FMath::CeilToInt(FMath::Sqrt(Params.ModuleCount * 6.f)) * 2, 16, 64);
	Layout.GridSize = FIntPoint(Span, Span);
	Layout.CellModule.Init(INDEX_NONE, Span * Span);
	Layout.EdgeX.Init(static_cast<uint8>(ECL9GridEdge::Open), (Span + 1) * Span);
	Layout.EdgeY.Init(static_cast<uint8>(ECL9GridEdge::Open), Span * (Span + 1));
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
			if (Fits(Layout, Cand.Origin, Cand.Size) && !StartCandidates.ContainsByPredicate([&Cand](const FCandidate& Other) { return Cand.SameOutcome(Other); }))
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

	const int32 StartIndex = Occupy(Layout, *StartPick, 0);
	Layout.EntryCell = FIntPoint(StartPick->Origin.X, StartPick->Origin.Y + StartPick->Size.Y / 2);
	// 입구 문은 격자 바깥으로 난다 — 트럭은 밖에 있고 걸어 들어온다 (module.md 결정 6)
	SetEdge(Layout, Layout.EntryCell, SideW, ECL9GridEdge::Door);

	TArray<FFrontier> Frontier;
	OpenNeighbours(Layout, StartIndex, Frontier, Rng);

	// ---- 예산 배분. Special은 표에 있는 만큼 반드시 넣고, 나머지를 셋으로 나눈다.
	const TArray<const UCL9ModuleDef*>& SpecialPool = Pools[static_cast<int32>(ECL9ModuleType::Special)];
	int32 Budget = Params.ModuleCount - 1;
	const int32 SpecialWant = FMath::Min(SpecialPool.Num(), Budget);
	Budget -= SpecialWant;

	int32 WantPassage = FMath::RoundToInt(Budget * PassageShare);
	int32 WantDeadEnd = FMath::RoundToInt(Budget * DeadEndShare);
	int32 WantNormal = FMath::Max(0, Budget - WantPassage - WantDeadEnd);

	TArray<const UCL9ModuleDef*> MainPool = Pools[static_cast<int32>(ECL9ModuleType::Passage)];
	MainPool.Append(Pools[static_cast<int32>(ECL9ModuleType::Normal)]);

	// ---- 1단계: 본줄기. 가장 깊은 자리만 이어 붙여서 한 줄기로 멀리 뻗는다.
	const int32 MainCount = FMath::Min(Budget, FMath::Max(2, FMath::RoundToInt(Params.ModuleCount * MainPathShare)));
	for (int32 Placed = 0; Placed < MainCount; ++Placed)
	{
		FFrontier Spot;
		if (!TakeFrontier(Frontier, Layout, /*bDeepest*/ true, Rng, Spot) || !PlaceAt(Layout, Spot, MainPool, Frontier, Rng))
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
			if (!TakeFrontier(Frontier, Layout, /*bDeepest*/ false, Rng, Spot))
			{
				break;
			}
			if (PlaceAt(Layout, Spot, One, Frontier, Rng))
			{
				break;
			}
		}
	}

	// ---- 3단계: 곁가지. 남은 쿼터에서 타입을 뽑고, 막다른 방으로 끝을 맺는다.
	//      한 번 실패했다고 쿼터를 깎으면 판이 눈에 띄게 작아진다 — 자리가 안 맞은 것과
	//      쓸 모듈이 없는 것은 다르므로, 연달아 막힐 때만 손을 뗀다.
	int32 Stall = 0;
	while (WantPassage + WantDeadEnd + WantNormal > 0 && Stall < StallLimit)
	{
		FFrontier Spot;
		if (!TakeFrontier(Frontier, Layout, /*bDeepest*/ false, Rng, Spot))
		{
			break;
		}

		// 막다른 방은 가지를 닫는다 — 뻗을 자리가 얼마 안 남았는데 닫아 버리면
		// 그 뒤의 예산이 통째로 남는다. 여유가 없으면 뻗는 타입을 먼저 쓴다.
		const bool bMayClose = Frontier.Num() > 2 || WantPassage + WantNormal == 0;

		TArray<const UCL9ModuleDef*> Pool;
		int32* Quota = nullptr;
		const int32 Choices = WantPassage + WantNormal + (bMayClose ? WantDeadEnd : 0);
		const int32 Roll = Rng.RandRange(0, FMath::Max(Choices, 1) - 1);
		if (Roll < WantPassage)
		{
			Pool = Pools[static_cast<int32>(ECL9ModuleType::Passage)];
			Quota = &WantPassage;
		}
		else if (Roll < WantPassage + WantNormal)
		{
			Pool = Pools[static_cast<int32>(ECL9ModuleType::Normal)];
			Quota = &WantNormal;
		}
		else
		{
			Pool = Pools[static_cast<int32>(ECL9ModuleType::DeadEnd)];
			Quota = &WantDeadEnd;
		}

		if (PlaceAt(Layout, Spot, Pool, Frontier, Rng))
		{
			*Quota -= 1;
			Stall = 0;
		}
		else
		{
			++Stall;
		}
	}

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
		Add(CL9SegKind::Floor(), Rect, 0.f);
		if (!Module.Def->bOutdoor)
		{
			Add(CL9SegKind::Ceiling(), Rect, Module.Def->Height);
		}
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
	auto AddEdge = [&](ECL9GridEdge State, FName Kind, float Along0, float Along1, float Across, bool bAlongX, FVector2D Outward)
	{
		auto Box = [&](float A0, float A1)
		{
			return bAlongX
				? FBox2D(FVector2D(A0, Across - Half), FVector2D(A1, Across + Half))
				: FBox2D(FVector2D(Across - Half, A0), FVector2D(Across + Half, A1));
		};

		if (State == ECL9GridEdge::Wall)
		{
			Add(Kind, Box(Along0, Along1), 0.f);
			return;
		}

		const float Middle = (Along0 + Along1) * 0.5f;
		const float Gap = FMath::Min(Params.DoorWidth, Along1 - Along0);
		Add(Kind, Box(Along0, Middle - Gap * 0.5f), 0.f);
		Add(Kind, Box(Middle + Gap * 0.5f, Along1), 0.f);
		Add(State == ECL9GridEdge::Flap ? CL9SegKind::FlapIn() : CL9SegKind::Door(),
			Box(Middle - Gap * 0.5f, Middle + Gap * 0.5f), 0.f, Outward);
	};

	for (int32 GY = 0; GY < Layout.GridSize.Y; ++GY)
	{
		for (int32 GX = 0; GX <= Layout.GridSize.X; ++GX)
		{
			const ECL9GridEdge State = Layout.EdgeAtX(GX, GY);
			if (State == ECL9GridEdge::Open)
			{
				continue;
			}
			const int32 West = Layout.CellAt(FIntPoint(GX - 1, GY));
			const int32 East = Layout.CellAt(FIntPoint(GX, GY));
			AddEdge(State, WallKind(West, East),
				Base.Y + GY * Cell, Base.Y + (GY + 1) * Cell, Base.X + GX * Cell,
				/*bAlongX*/ false, FVector2D(East == INDEX_NONE ? 1.f : -1.f, 0.f));
		}
	}
	for (int32 GY = 0; GY <= Layout.GridSize.Y; ++GY)
	{
		for (int32 GX = 0; GX < Layout.GridSize.X; ++GX)
		{
			const ECL9GridEdge State = Layout.EdgeAtY(GX, GY);
			if (State == ECL9GridEdge::Open)
			{
				continue;
			}
			const int32 South = Layout.CellAt(FIntPoint(GX, GY - 1));
			const int32 North = Layout.CellAt(FIntPoint(GX, GY));
			AddEdge(State, WallKind(South, North),
				Base.X + GX * Cell, Base.X + (GX + 1) * Cell, Base.Y + GY * Cell,
				/*bAlongX*/ true, FVector2D(0.f, North == INDEX_NONE ? 1.f : -1.f));
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
		ForEachBoundaryCell(Module, [&](FIntPoint Cell, int32 Side)
		{
			if (GetEdge(Layout, Cell, Side) != ECL9GridEdge::Door)
			{
				return;
			}
			const int32 Other = Layout.CellAt(Cell + SideDir[Side]);
			if (Other != INDEX_NONE && !Seen[Other])
			{
				Seen[Other] = 1;
				Queue.Add(Other);
			}
		});
	}
	return Reached == Layout.Modules.Num();
}

uint32 CL9SiteLayout::LayoutHash(const FCL9SiteLayout& Layout)
{
	uint32 Hash = 2166136261u;
	FnvMix(Hash, Layout.Seed);
	FnvMix(Hash, static_cast<uint32>(Layout.GridSize.X));
	FnvMix(Hash, static_cast<uint32>(Layout.GridSize.Y));
	for (const FCL9ModulePlacement& Module : Layout.Modules)
	{
		FnvMix(Hash, GetTypeHash(Module.Def ? Module.Def->Alias : NAME_None));
		FnvMix(Hash, static_cast<uint32>(Module.Origin.X));
		FnvMix(Hash, static_cast<uint32>(Module.Origin.Y));
		FnvMix(Hash, static_cast<uint32>(Module.Rotation));
	}
	for (uint8 Edge : Layout.EdgeX) FnvMix(Hash, Edge);
	for (uint8 Edge : Layout.EdgeY) FnvMix(Hash, Edge);
	return Hash;
}
