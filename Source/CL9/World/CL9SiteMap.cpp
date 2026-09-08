// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9SiteMap.h"
#include "CL9.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ModuleDef.h"
#include "Data/CL9PieceDef.h"
#include "Data/CL9SegKindDef.h"
#include "Data/CL9ProgressionDef.h"
#include "Data/CL9SiteDef.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "World/CL9FlapIn.h"
#include "World/CL9TruckPad.h"

namespace
{
	/** Stretch 모드가 기대하는 메시는 단위 정육면체다 — scale = size / 이 값 */
	constexpr float UnitCube = 100.f;

	/**
	 * 타일링 보정 배율이 이 범위를 벗어나면 그 조각은 그 발자국에 안 맞는 것이다.
	 * 세우기는 하되 로그로 알린다 — 팩을 갈아 끼웠을 때 격자 치수를 그 팩 모듈에
	 * 맞춰 다시 잡으라는 신호다 (docs/systems/asset-swap.md).
	 */
	constexpr float TileFitMin = 0.85f;
	constexpr float TileFitMax = 1.15f;

	/**
	 * 변형을 자리 좌표로 고른다. 난수 스트림을 쓰면 호출 순서에 의존해서 머신마다
	 * 갈릴 수 있는데, 좌표 해시는 순서와 무관하게 모든 머신에서 같은 값이 나온다.
	 */
	int32 PickVariant(const FVector& Spot, int32 Count)
	{
		if (Count <= 1)
		{
			return 0;
		}
		uint32 Hash = 2166136261u;
		for (int32 Axis = 0; Axis < 3; ++Axis)
		{
			const uint32 Value = static_cast<uint32>(FMath::RoundToInt(Spot[Axis] * 0.1f));
			for (int32 Byte = 0; Byte < 4; ++Byte)
			{
				Hash ^= (Value >> (Byte * 8)) & 0xFF;
				Hash *= 16777619u;
			}
		}
		return static_cast<int32>(Hash % static_cast<uint32>(Count));
	}

	/**
	 * 발자국을 조각 자연 크기로 채우는 칸들을 만든다 — 긴 축으로 늘어놓고 높이
	 * 축으로 쌓는다. 나머지 길이는 장수로 나눠 흡수하고, 남는 높이는 맨 윗단이
	 * 받는다(눈이 가는 아래쪽 비율을 지키는 편이 전체를 눌러 놓는 것보다 낫다).
	 * 두께 축은 칸을 그대로 두고, 조각을 앉힐 때 늘리지 않는다.
	 */
	void MakeTileCells(const FBox2D& Rect, float Base, float Top, const FVector& PieceSize, bool& bOutRunAlongX, TArray<FBox>& OutCells)
	{
		const FVector2D Size = Rect.GetSize();
		bOutRunAlongX = Size.X >= Size.Y;

		const float Run = bOutRunAlongX ? Size.X : Size.Y;
		const float PieceRun = FMath::Max(FMath::Max(PieceSize.X, PieceSize.Y), 1.f);
		const int32 Count = FMath::Max(1, FMath::RoundToInt(Run / PieceRun));
		const float CellRun = Run / Count;

		const float PieceHeight = FMath::Max(PieceSize.Z, 1.f);
		const float Height = Top - Base;
		const int32 Rows = FMath::Max(1, FMath::RoundToInt(Height / PieceHeight));
		const float TopHeight = Height - (Rows - 1) * PieceHeight;

		for (int32 Row = 0; Row < Rows; ++Row)
		{
			const float RowBase = Base + Row * PieceHeight;
			const float RowHeight = Row == Rows - 1 ? TopHeight : PieceHeight;
			for (int32 Index = 0; Index < Count; ++Index)
			{
				const float RunMin = (bOutRunAlongX ? Rect.Min.X : Rect.Min.Y) + Index * CellRun;
				const FVector2D CellMin = bOutRunAlongX ? FVector2D(RunMin, Rect.Min.Y) : FVector2D(Rect.Min.X, RunMin);
				const FVector2D CellMax = bOutRunAlongX ? FVector2D(RunMin + CellRun, Rect.Max.Y) : FVector2D(Rect.Max.X, RunMin + CellRun);
				OutCells.Add(FBox(FVector(CellMin.X, CellMin.Y, RowBase), FVector(CellMax.X, CellMax.Y, RowBase + RowHeight)));
			}
		}
	}

	/**
	 * 조각 하나를 칸에 앉힌다. 길이와 높이는 칸에 맞추고, **두께는 늘리지 않고**
	 * 칸 가운데에 세운다 — 발자국의 두께는 비워 둔 공간이지 채워야 할 부피가
	 * 아니다. 피벗은 메시 바운드 중심으로 보정하므로 조각의 원점이 어디에 있든
	 * 상관없다. 반환값은 길이 축 보정 배율이다.
	 */
	float FitPieceToCell(const FBox& Cell, const FBox& Bounds, bool bRunAlongX, FTransform& OutInstance)
	{
		const FVector MeshSize = Bounds.GetSize();
		const bool bMeshRunAlongX = MeshSize.X >= MeshSize.Y;
		const float MeshRun = FMath::Max(FMath::Max(MeshSize.X, MeshSize.Y), 1.f);
		const float MeshHeight = FMath::Max(MeshSize.Z, 1.f);

		const FVector CellSize = Cell.GetSize();
		const float RunScale = (bRunAlongX ? CellSize.X : CellSize.Y) / MeshRun;
		const float HeightScale = CellSize.Z / MeshHeight;

		const FVector Scale = bMeshRunAlongX ? FVector(RunScale, 1.f, HeightScale) : FVector(1.f, RunScale, HeightScale);
		const FQuat Rotation = FRotator(0.f, bRunAlongX == bMeshRunAlongX ? 0.f : 90.f, 0.f).Quaternion();

		OutInstance.SetRotation(Rotation);
		OutInstance.SetScale3D(Scale);
		OutInstance.SetLocation(Cell.GetCenter() - Rotation.RotateVector(Scale * Bounds.GetCenter()));
		return RunScale;
	}

	/** 조각 표에서 이 자리에 쓸 변형 하나를 꺼낸다 */
	UStaticMesh* PickMesh(const UCL9PieceDef& Piece, const FVector& Spot)
	{
		const int32 Index = PickVariant(Spot, Piece.Variants.Num());
		return Piece.Variants[Index].LoadSynchronous();
	}

	/** 타일링한 종류마다 한 줄씩 남기는 맞춤 기록 — 팩을 갈아 끼웠을 때 이걸 본다 */
	struct FTileReport
	{
		FVector PieceSize = FVector::ZeroVector;
		int32 Tiles = 0;
		float WorstRunFit = 1.f;
	};

	/** 그 종류가 쓰는 조각의 첫 변형 메시 — 실측 치수를 재려고 꺼낸다 */
	const UStaticMesh* LeadMesh(const UCL9DefRegistry& Registry, FName KindName, FName Theme)
	{
		const UCL9SegKindDef* Kind = Registry.FindSegKind(KindName);
		const UCL9PieceDef* Piece = Kind != nullptr ? Registry.FindPiece(Kind->Piece, Theme) : nullptr;
		return (Piece != nullptr && Piece->Variants.Num() > 0) ? Piece->Variants[0].LoadSynchronous() : nullptr;
	}
}

ACL9SiteMap::ACL9SiteMap()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);
}

ACL9SiteMap* ACL9SiteMap::Find(UWorld* World)
{
	if (World == nullptr)
	{
		return nullptr;
	}
	for (TActorIterator<ACL9SiteMap> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

ACL9SiteMap* ACL9SiteMap::Rebuild(UWorld* World, const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed, int32 Day)
{
	if (World == nullptr)
	{
		return nullptr;
	}

	ACL9SiteMap* Map = Find(World);
	if (Map == nullptr)
	{
		// 트럭 패드가 사이트의 기준점이다. 맨 레벨에서는 PlayerStart가 그 자리를 대신한다
		FVector Anchor = FVector::ZeroVector;
		bool bFound = false;
		for (TActorIterator<ACL9TruckPad> It(World); It; ++It)
		{
			Anchor = It->GetActorLocation();
			bFound = true;
			break;
		}
		if (!bFound)
		{
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				Anchor = It->GetActorLocation();
				break;
			}
		}
		Anchor.Z = 0.f;

		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Map = World->SpawnActor<ACL9SiteMap>(Anchor, FRotator::ZeroRotator, SpawnParams);
		if (Map == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("Site map: spawn failed."));
			return nullptr;
		}
		Map->bIsEditorOnlyActor = World->WorldType == EWorldType::Editor;
	}

	Map->Build(Site, Registry, Seed, Day);
	return Map;
}

void ACL9SiteMap::Clear(UWorld* World)
{
	if (ACL9SiteMap* Map = Find(World))
	{
		Map->ClearBuilt();
		Map->Destroy();
	}
}

FVector ACL9SiteMap::PointToWorld(const FCL9LevelPoint& Point) const
{
	return SiteToWorld(Point.Local);
}

FVector ACL9SiteMap::SiteToWorld(const FVector2D& Local) const
{
	// 층이 없으므로 사이트 로컬 Z는 언제나 지면이다 (docs/plan/data/module.md 결정 2)
	return GetActorLocation() + FVector(Local.X, Local.Y, 0.f);
}

void ACL9SiteMap::ClearBuilt()
{
	for (UInstancedStaticMeshComponent* Component : KindComponents)
	{
		if (Component != nullptr)
		{
			Component->DestroyComponent();
		}
	}
	KindComponents.Reset();

	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Reset();
}

void ACL9SiteMap::MeasurePieces(const UCL9SiteDef& Site, const UCL9DefRegistry& Registry)
{
	// 벽 두께는 표에 적지 않는다 — 벽으로 세울 조각의 얇은 축이 곧 프레임의 벽 두께다.
	if (const UStaticMesh* WallMesh = LeadMesh(Registry, CL9SegKind::Barn(), Site.Theme))
	{
		const FVector MeshSize = WallMesh->GetBoundingBox().GetSize();
		Params.WallThickness = FMath::Max(FMath::Min(MeshSize.X, MeshSize.Y), 1.f);
	}
	// 문 폭도 마찬가지다 — 팩을 갈아 끼우면 개구부가 그 팩 문짝을 따라간다.
	if (const UStaticMesh* DoorMesh = LeadMesh(Registry, CL9SegKind::Door(), Site.Theme))
	{
		const FVector MeshSize = DoorMesh->GetBoundingBox().GetSize();
		Params.DoorWidth = FMath::Clamp(FMath::Max(MeshSize.X, MeshSize.Y), 1.f, Params.CellSize);
	}
}

void ACL9SiteMap::Build(const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed, int32 Day)
{
	ClearBuilt();

	Params = FCL9SiteParams();
	Params.Theme = Site.Theme;
	Params.CellSize = FMath::Max(Site.CellSize, 1.f);
	Params.EntryOffset = Site.EntryOffset;

	// 한 판의 크기 = 사이트 기준값 × 일자 커브 (module.md 결정 5).
	// 커브가 비어 있으면 1.0이므로 기준값이 그대로 나온다.
	const UCL9ProgressionDef* Progression = Registry.GetProgression();
	const float SizeMult = Progression != nullptr ? Progression->SiteSizeAt(Day) : 1.f;
	Params.ModuleCount = FMath::Max(2, FMath::RoundToInt(Site.BaseModuleCount * SizeMult));

	MeasurePieces(Site, Registry);

	TArray<const UCL9ModuleDef*> ModuleDefs;
	Registry.GatherModules(Site.Theme, ModuleDefs);
	if (ModuleDefs.Num() == 0 && Seed != 0)
	{
		UE_LOG(LogCL9, Error, TEXT("Site map: stage '%s' has no module rows — nothing to assemble (DA_ModuleTable)."), *Site.Theme.ToString());
	}

	Layout = CL9SiteLayout::Generate(Params, ModuleDefs, Seed);

	TArray<FCL9SiteSeg> Segs;
	CL9SiteLayout::BuildSegments(Params, Layout, Segs);

	TMap<TPair<FName, UStaticMesh*>, UInstancedStaticMeshComponent*> Components;
	TMap<FName, FTileReport> Reports;
	TSet<FName> MissingKinds;
	TSet<FName> MissingRoles;
	int32 Placed = 0;

	// 컴포넌트는 (종류, 메시)마다 하나다 — 같은 메시라도 종류가 다르면 시야 차단
	// 설정이 달라서 한 인스턴스 묶음에 넣을 수 없다.
	auto ComponentFor = [this, &Components](const UCL9SegKindDef& Kind, FName KindName, UStaticMesh* Mesh)
	{
		const TPair<FName, UStaticMesh*> Key(KindName, Mesh);
		if (UInstancedStaticMeshComponent* Found = Components.FindRef(Key))
		{
			return Found;
		}
		UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
			this, FName(*FString::Printf(TEXT("Seg_%s_%d"), *KindName.ToString(), KindComponents.Num())));
		Component->SetMobility(EComponentMobility::Static);
		Component->SetStaticMesh(Mesh);
		Component->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		if (!Kind.bBlocksSight)
		{
			Component->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		}
		Component->SetCanEverAffectNavigation(true);
		Component->SetupAttachment(GetRootComponent());
		Components.Add(Key, Component);
		KindComponents.Add(Component);
		return Component;
	};

	for (const FCL9SiteSeg& Seg : Segs)
	{
		const UCL9SegKindDef* Kind = Registry.FindSegKind(Seg.Kind);
		if (Kind == nullptr)
		{
			MissingKinds.Add(Seg.Kind);
			continue;
		}
		const UCL9PieceDef* Piece = Registry.FindPiece(Kind->Piece, Site.Theme);
		if (Piece == nullptr || Piece->Variants.Num() == 0)
		{
			MissingRoles.Add(Kind->Piece);
			continue;
		}

		const FVector2D Size = Seg.Rect.GetSize();
		const FVector2D Center = Seg.Rect.GetCenter();

		// 바닥만 기준점 아래로 자라고, 벽과 천장은 기준점 위로 자란다.
		const bool bDownward = Seg.Kind == CL9SegKind::Floor();
		const float Base = bDownward ? Seg.BaseZ - Kind->Height : Seg.BaseZ;
		const float Top = bDownward ? Seg.BaseZ : Seg.BaseZ + Kind->Height;
		const FVector Spot(Center.X, Center.Y, (Base + Top) * 0.5f);

		if (Seg.Kind == CL9SegKind::FlapIn())
		{
			UStaticMesh* Mesh = PickMesh(*Piece, Spot);
			const FVector Extent(Size.X * 0.5f, Size.Y * 0.5f, (Top - Base) * 0.5f);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.ObjectFlags |= RF_Transient;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (ACL9FlapIn* Flap = GetWorld()->SpawnActor<ACL9FlapIn>(GetActorLocation() + Spot, FRotator::ZeroRotator, SpawnParams))
			{
				// 안쪽은 바깥 방향의 반대다 — 배치기가 어느 변에 달았는지 알려 준다
				Flap->Setup(Extent, FVector(-Seg.Outward.X, -Seg.Outward.Y, 0.f), Mesh);
				Flap->bIsEditorOnlyActor = bIsEditorOnlyActor;
				SpawnedActors.Add(Flap);
				++Placed;
			}
			continue;
		}

		if (Kind->Placement == ECL9SegPlacement::Tile)
		{
			UStaticMesh* Lead = Piece->Variants[0].LoadSynchronous();
			if (Lead == nullptr)
			{
				MissingRoles.Add(Kind->Piece);
				continue;
			}

			// 칸 격자는 첫 변형의 크기로 잡고, 각 칸에는 그 자리에 뽑힌 변형을 그 칸에
			// 맞춰 앉힌다 — 변형끼리 크기가 어긋나도 칸 밖으로 삐져나오지 않는다.
			const FVector PieceSize = Lead->GetBoundingBox().GetSize();
			bool bRunAlongX = true;
			TArray<FBox> Cells;
			MakeTileCells(Seg.Rect, Base, Top, PieceSize, bRunAlongX, Cells);

			FTileReport& Report = Reports.FindOrAdd(Seg.Kind);
			Report.PieceSize = PieceSize;

			for (const FBox& Cell : Cells)
			{
				UStaticMesh* Mesh = PickMesh(*Piece, Cell.GetCenter());
				if (Mesh == nullptr)
				{
					continue;
				}
				FTransform Instance;
				const float Fit = FitPieceToCell(Cell, Mesh->GetBoundingBox(), bRunAlongX, Instance);
				if (FMath::Abs(Fit - 1.f) > FMath::Abs(Report.WorstRunFit - 1.f))
				{
					Report.WorstRunFit = Fit;
				}
				ComponentFor(*Kind, Seg.Kind, Mesh)->AddInstance(Instance, /*bWorldSpace*/ false);
				++Report.Tiles;
				++Placed;
			}
			continue;
		}

		UStaticMesh* Mesh = PickMesh(*Piece, Spot);
		if (Mesh == nullptr)
		{
			MissingRoles.Add(Kind->Piece);
			continue;
		}

		FTransform Instance;
		Instance.SetLocation(FVector(Center.X, Center.Y, (Base + Top) * 0.5f));
		Instance.SetScale3D(FVector(Size.X, Size.Y, Top - Base) / UnitCube);
		ComponentFor(*Kind, Seg.Kind, Mesh)->AddInstance(Instance, /*bWorldSpace*/ false);
		++Placed;
	}

	for (UInstancedStaticMeshComponent* Component : KindComponents)
	{
		Component->RegisterComponent();
		AddInstanceComponent(Component);
	}

	for (const FName& Kind : MissingKinds)
	{
		UE_LOG(LogCL9, Error, TEXT("Site map: seg kind '%s' has no usable row — its boxes were skipped (DA_SegKindTable)."), *Kind.ToString());
	}
	for (const FName& MissingRole : MissingRoles)
	{
		UE_LOG(LogCL9, Error, TEXT("Site map: theme '%s' has no usable piece for role '%s' — its boxes were skipped (DA_PieceTable)."),
			*Site.Theme.ToString(), *MissingRole.ToString());
	}
	for (const TPair<FName, FTileReport>& Entry : Reports)
	{
		const FTileReport& Report = Entry.Value;
		UE_LOG(LogCL9, Log, TEXT("Site map: '%s' tiled with a %.0f x %.0f x %.0f piece — %d instances, worst run fit %.2f."),
			*Entry.Key.ToString(), Report.PieceSize.X, Report.PieceSize.Y, Report.PieceSize.Z, Report.Tiles, Report.WorstRunFit);
		if (Report.WorstRunFit < TileFitMin || Report.WorstRunFit > TileFitMax)
		{
			UE_LOG(LogCL9, Warning, TEXT("Site map: '%s' pieces are stretched %+.0f%% along their run — retune CellSize to this pack's module (docs/systems/asset-swap.md)."),
				*Entry.Key.ToString(), (Report.WorstRunFit - 1.f) * 100.f);
		}
	}

	if (!Layout.bBuilt)
	{
		UE_LOG(LogCL9, Log, TEXT("Site map: nothing built — site '%s', seed %u (0 means the flat level stays as it is)."), *Site.Alias.ToString(), Seed);
		return;
	}

	// 목표 수를 못 채우는 것은 흔한 실패다 — 격자가 좁거나 그 스테이지의 소켓이
	// 서로 안 맞으면 프론티어가 먼저 마른다. 조용히 작은 맵이 나오느니 알린다.
	if (Layout.Modules.Num() < Params.ModuleCount)
	{
		UE_LOG(LogCL9, Warning, TEXT("Site map: asked for %d modules but only %d fit — check the stage's socket mix (docs/plan/data/module.md)."),
			Params.ModuleCount, Layout.Modules.Num());
	}
	UE_LOG(LogCL9, Log, TEXT("Site map built — site '%s', stage '%s', day %d, seed %u, hash %08X, %d modules on a %dx%d grid, %d points, %d instances in %d components."),
		*Site.Alias.ToString(), *Site.Theme.ToString(), Day, Seed, CL9SiteLayout::LayoutHash(Layout),
		Layout.Modules.Num(), Layout.GridSize.X, Layout.GridSize.Y, Layout.Points.Num(), Placed, KindComponents.Num());
}
