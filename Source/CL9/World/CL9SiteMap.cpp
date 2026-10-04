// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9SiteMap.h"
#include "CL9.h"
#include "CoreGlobals.h"
#include "HAL/PlatformTime.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ModuleDef.h"
#include "Data/CL9PieceDef.h"
#include "Data/CL9SegKindDef.h"
#include "Data/CL9ProgressionDef.h"
#include "Data/CL9SiteDef.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Scene.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameMode/CL9GameState.h"
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
		/** Grid 종류는 계단실 띠처럼 짧은 발자국이 늘 있어 경고를 내지 않는다 — 기록만 남긴다 */
		bool bWarnOnFit = true;

		void Note(float Fit)
		{
			if (FMath::Abs(Fit - 1.f) > FMath::Abs(WorstRunFit - 1.f))
			{
				WorstRunFit = Fit;
			}
		}
	};

	/** 그 종류가 쓰는 조각의 첫 변형 메시 — 실측 치수를 재려고 꺼낸다 */
	const UStaticMesh* LeadMesh(const UCL9DefRegistry& Registry, FName KindName, FName Theme)
	{
		const UCL9SegKindDef* Kind = Registry.FindSegKind(KindName);
		const UCL9PieceDef* Piece = Kind != nullptr ? Registry.FindPiece(Kind->Piece, Theme) : nullptr;
		return (Piece != nullptr && Piece->Variants.Num() > 0) ? Piece->Variants[0].LoadSynchronous() : nullptr;
	}

	/** 트럭 패드가 사이트의 기준점이다. 맨 레벨에서는 PlayerStart가 그 자리를 대신한다 */
	FVector FindSiteAnchor(UWorld* World)
	{
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
		return Anchor;
	}

	/** 사이트맵 액터 하나를 세운다 — 트랜지언트, 에디터 월드에서는 에디터 전용 */
	ACL9SiteMap* SpawnSiteMap(UWorld* World, const FVector& Location, const FRotator& Rotation)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACL9SiteMap* Map = World->SpawnActor<ACL9SiteMap>(Location, Rotation, SpawnParams);
		if (Map == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("Site map: spawn failed."));
			return nullptr;
		}
		Map->bIsEditorOnlyActor = World->WorldType == EWorldType::Editor;
		return Map;
	}
}

ACL9SiteMap::ACL9SiteMap()
{
	// 틱은 시설 조명의 정전 소등뿐이다 — 램프가 하나도 없으면 상태 비교 한 번으로 끝난다.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);
}

ACL9SiteMap* ACL9SiteMap::Find(UWorld* World)
{
	TArray<ACL9SiteMap*> Maps;
	FindAll(World, Maps);
	return Maps.Num() > 0 ? Maps[0] : nullptr;
}

void ACL9SiteMap::FindAll(UWorld* World, TArray<ACL9SiteMap*>& OutMaps)
{
	OutMaps.Reset();
	if (World == nullptr)
	{
		return;
	}
	for (TActorIterator<ACL9SiteMap> It(World); It; ++It)
	{
		OutMaps.Add(*It);
	}
	// 구역 인덱스 순 — 단일 사이트(INDEX_NONE)는 혼자다
	OutMaps.Sort([](const ACL9SiteMap& A, const ACL9SiteMap& B) { return A.DistrictIndex < B.DistrictIndex; });
}

bool ACL9SiteMap::IsBuildPending(UWorld* World)
{
	TArray<ACL9SiteMap*> Maps;
	FindAll(World, Maps);
	return Maps.ContainsByPredicate([](const ACL9SiteMap* Map) { return Map->bBuildPending; });
}

ACL9SiteMap* ACL9SiteMap::Rebuild(UWorld* World, const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed, int32 Day)
{
	if (World == nullptr)
	{
		return nullptr;
	}

	if (Site.Districts.Num() == 0)
	{
		// 단일 사이트 — 예전 경로 그대로. 구역 사이트가 깔려 있던 월드(치트로 사이트를 바꾼 경우)면 먼저 걷는다
		ACL9SiteMap* Map = Find(World);
		if (Map != nullptr && Map->DistrictIndex != INDEX_NONE)
		{
			Clear(World);
			Map = nullptr;
		}
		if (Map == nullptr)
		{
			Map = SpawnSiteMap(World, FindSiteAnchor(World), FRotator::ZeroRotator);
			if (Map == nullptr)
			{
				return nullptr;
			}
		}

		Map->Build(Site, Registry, Seed, Day);
		return Map;
	}

	// 구역 사이트 — 예전 판(단일이든 구역이든)은 통째로 걷고 구역마다 새 액터를 세운다. 액터 원점 = 트럭 패드 +
	// 구역 Offset, 회전 = 구역 Yaw. 생성기는 회전을 모른다 — 인스턴스·컴포넌트·램프가 액터에 붙어 함께 돈다.
	Clear(World);
	const FVector Anchor = FindSiteAnchor(World);
	TArray<ACL9SiteMap*> Maps;
	for (int32 Index = 0; Index < Site.Districts.Num(); ++Index)
	{
		const FCL9SiteDistrict& District = Site.Districts[Index];
		const FRotator Rotation(0.f, CL9SiteDistrict::QuarterTurns(District.Yaw) * 90.f, 0.f);
		ACL9SiteMap* Map = SpawnSiteMap(World, Anchor + FVector(District.Offset.X, District.Offset.Y, 0.f), Rotation);
		if (Map == nullptr)
		{
			continue;
		}
		Map->DistrictIndex = Index;
		Map->bBuildPending = true;
		// 행은 읽기만 한다 — UPROPERTY 로 붙드는 것은 GC 에 맞서기 위해서다(헤더 주석)
		Map->PendingSite = const_cast<UCL9SiteDef*>(&Site);
		Map->PendingRegistry = const_cast<UCL9DefRegistry*>(&Registry);
		Map->PendingSeed = CL9SiteDistrict::SubSeed(Seed, Index);
		Map->PendingDay = Day;
		Maps.Add(Map);
	}
	if (Maps.Num() == 0)
	{
		return nullptr;
	}

	UE_LOG(LogCL9, Log, TEXT("Site map: site '%s' has %d districts — building the first now and the rest one per frame (seed %u)."),
		*Site.Alias.ToString(), Maps.Num(), Seed);

	// 첫 구역은 이 호출에서, 나머지는 첫 구역이 거는 티커가 프레임마다 하나씩
	ACL9SiteMap* Lead = Maps[0];
	Lead->BuildPending();
	Lead->LastDistrictBuildFrame = GFrameCounter;
	if (Maps.Num() > 1)
	{
		// 코어 티커는 게임·에디터 월드 모두 엔진 프레임마다 돈다(일시 정지와도 무관) — 치트의 에디터 맵도 같은 길로 선다
		TWeakObjectPtr<ACL9SiteMap> WeakLead(Lead);
		Lead->DistrictTicker = FTSTicker::GetCoreTicker().AddTicker(TEXT("CL9SiteMapDistricts"), 0.f, [WeakLead](float)
		{
			ACL9SiteMap* LeadMap = WeakLead.Get();
			return LeadMap != nullptr && LeadMap->BuildNextDistrict();
		});
	}
	return Lead;
}

void ACL9SiteMap::BuildPending()
{
	if (!bBuildPending)
	{
		return;
	}
	if (PendingSite != nullptr && PendingRegistry != nullptr)
	{
		Build(*PendingSite, *PendingRegistry, PendingSeed, PendingDay);
	}
	bBuildPending = false;
	PendingSite = nullptr;
	PendingRegistry = nullptr;
}

bool ACL9SiteMap::BuildNextDistrict()
{
	UWorld* World = GetWorld();
	if (World == nullptr || IsActorBeingDestroyed())
	{
		DistrictTicker.Reset();
		return false;
	}

	TArray<ACL9SiteMap*> Maps;
	FindAll(World, Maps);

	// 프레임당 하나 — Rebuild 가 첫 구역을 지은 그 프레임에 티커가 또 돌면 다음 프레임으로 미룬다
	if (GFrameCounter != LastDistrictBuildFrame)
	{
		for (ACL9SiteMap* Map : Maps)
		{
			if (Map->bBuildPending)
			{
				Map->BuildPending();
				LastDistrictBuildFrame = GFrameCounter;
				break;
			}
		}
	}

	if (Maps.ContainsByPredicate([](const ACL9SiteMap* Map) { return Map->bBuildPending; }))
	{
		return true;
	}
	UE_LOG(LogCL9, Log, TEXT("Site map: all %d districts built."), Maps.Num());
	DistrictTicker.Reset();
	return false;
}

void ACL9SiteMap::StopDistrictTicker()
{
	if (DistrictTicker.IsValid())
	{
		FTSTicker::RemoveTicker(DistrictTicker);
		DistrictTicker.Reset();
	}
}

void ACL9SiteMap::Destroyed()
{
	StopDistrictTicker();
	Super::Destroyed();
}

void ACL9SiteMap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopDistrictTicker();
	Super::EndPlay(EndPlayReason);
}

void ACL9SiteMap::Clear(UWorld* World)
{
	TArray<ACL9SiteMap*> Maps;
	FindAll(World, Maps);
	for (ACL9SiteMap* Map : Maps)
	{
		Map->ClearBuilt();
		Map->Destroy();
	}
}

FVector ACL9SiteMap::PointToWorld(const FCL9LevelPoint& Point) const
{
	// 지점의 Z는 그 지점이 있는 층의 바닥이다 — 스폰이 2층 방을 고르면 2층에 놓인다.
	return SiteToWorld(Point.Local) + FVector(0.f, 0.f, Point.Floor * Params.FloorHeight);
}

FVector ACL9SiteMap::SiteToWorld(const FVector2D& Local) const
{
	// 평면 좌표만 옮긴다 — 층 높이는 부르는 쪽이 얹는다 (PointToWorld). 액터 변환을 거쳐 구역의 회전이 들어간다.
	// 단일 사이트는 회전 0·배율 1 이라 위치 + 로컬 그대로다.
	return GetActorTransform().TransformPosition(FVector(Local.X, Local.Y, 0.f));
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

	for (UPointLightComponent* Lamp : LampComponents)
	{
		if (Lamp != nullptr)
		{
			Lamp->DestroyComponent();
		}
	}
	LampComponents.Reset();
	FlickerLamps.Reset();
	bLampsOn = true;

	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Reset();
}

void ACL9SiteMap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 정전 판정은 게임 스테이트의 것이다. 에디터 월드(게임 스테이트 없음)에서는 늘 켜 둔다.
	const ACL9GameState* State = GetWorld() != nullptr ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
	SetLampsOn(State == nullptr || !State->IsBlackout());
	TickFlicker();
}

void ACL9SiteMap::TickFlicker()
{
	if (!bLampsOn || FlickerLamps.Num() == 0 || GetWorld() == nullptr)
	{
		return;
	}
	// 펄린 노이즈 한 줄 — 램프마다 위상이 달라 같은 박자로 깜박이지 않는다. 진폭 1이면 0~2배 사이.
	const float Time = GetWorld()->GetTimeSeconds();
	for (const FFlickerLamp& Entry : FlickerLamps)
	{
		UPointLightComponent* Lamp = LampComponents.IsValidIndex(Entry.LampIndex) ? LampComponents[Entry.LampIndex].Get() : nullptr;
		if (Lamp != nullptr)
		{
			const float Noise = FMath::PerlinNoise1D(Time * Entry.Speed + Entry.Phase);
			Lamp->SetIntensity(Entry.BaseIntensity * (1.f + Entry.Amplitude * Noise));
		}
	}
}

void ACL9SiteMap::SetLampsOn(bool bOn)
{
	if (bOn == bLampsOn)
	{
		return;
	}
	bLampsOn = bOn;
	for (UPointLightComponent* Lamp : LampComponents)
	{
		if (Lamp != nullptr)
		{
			Lamp->SetVisibility(bOn);
		}
	}
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
	// 단계별 빌드 시간 — 구역 K 개를 프레임마다 나눠 지을 때 한 프레임이 얼마나 서는지 본다 (giant-stage.md)
	const double BuildStarted = FPlatformTime::Seconds();
	ClearBuilt();

	// 구역이면 그 구역 행이 모듈 수와 깊이 구역을 준다 — 나머지 맵 열은 사이트 것을 그대로 쓴다
	const FCL9SiteDistrict* District = Site.Districts.IsValidIndex(DistrictIndex) ? &Site.Districts[DistrictIndex] : nullptr;
	Zones = (District != nullptr && District->Zones.Num() > 0) ? District->Zones : Site.Zones;

	Params = FCL9SiteParams();
	Params.Theme = Site.Theme;
	Params.CellSize = FMath::Max(Site.CellSize, 1.f);
	Params.EntryOffset = Site.EntryOffset;

	// 한 판의 크기 = 사이트(구역) 기준값 × 일자 커브 (module.md 결정 5).
	// 커브가 비어 있으면 1.0이므로 기준값이 그대로 나온다.
	const UCL9ProgressionDef* Progression = Registry.GetProgression();
	const float SizeMult = Progression != nullptr ? Progression->SiteSizeAt(Day) : 1.f;
	const int32 BaseModules = District != nullptr ? District->ModuleCount : Site.BaseModuleCount;
	Params.ModuleCount = FMath::Max(2, FMath::RoundToInt(BaseModules * SizeMult));

	// 층은 일자와 무관하다 — 목적지가 몇 층짜리 건물인가는 그 목적지의 성질이다.
	Params.MaxFloors = FMath::Clamp(Site.MaxFloors, 1, 8);
	Params.StairCountMin = FMath::Max(1, Site.StairCountMin);
	Params.StairCountMax = FMath::Max(Params.StairCountMin, Site.StairCountMax);
	Params.UpperFloorShare = FMath::Clamp(Site.UpperFloorShare, 0.f, 1.f);
	Params.bBuildCeilings = Site.bBuildCeilings;
	Params.LampDensity = FMath::Clamp(Site.LampDensity, 0.f, 1.f);
	Params.LoopChance = FMath::Clamp(Site.LoopChance, 0.f, 1.f);
	Params.MainPathShare = FMath::Clamp(Site.MainPathShare, 0.f, 1.f);
	Params.PassageShare = FMath::Clamp(Site.PassageShare, 0.f, 1.f);
	Params.DeadEndShare = FMath::Clamp(Site.DeadEndShare, 0.f, 1.f - Params.PassageShare);
	Params.SpecialCopies = FMath::Max(1, Site.SpecialCopies);

	MeasurePieces(Site, Registry);

	TArray<const UCL9ModuleDef*> ModuleDefs;
	Registry.GatherModules(Site.Theme, ModuleDefs);
	if (ModuleDefs.Num() == 0 && Seed != 0)
	{
		UE_LOG(LogCL9, Error, TEXT("Site map: stage '%s' has no module rows — nothing to assemble (ModuleTable)."), *Site.Theme.ToString());
	}

	// 한 층의 높이 = 가장 높은 방의 천장 + 천장 판 + 바닥 판. 판 두께는 조각 표에서
	// 실측으로 오므로, 팩을 갈아 끼우면 층 높이가 그 팩을 따라간다 (asset-swap.md).
	{
		float Tallest = 0.f;
		for (const UCL9ModuleDef* Def : ModuleDefs)
		{
			Tallest = FMath::Max(Tallest, Def->Height);
		}
		const UCL9SegKindDef* CeilingKind = Registry.FindSegKind(CL9SegKind::Ceiling());
		const UCL9SegKindDef* FloorKind = Registry.FindSegKind(CL9SegKind::Floor());
		const float Sandwich = (CeilingKind != nullptr ? CeilingKind->Height : 30.f)
			+ (FloorKind != nullptr ? FloorKind->Height : 30.f);
		Params.FloorHeight = FMath::Max(Tallest, 200.f) + Sandwich;
	}

	const double GenerateStarted = FPlatformTime::Seconds();
	Layout = CL9SiteLayout::Generate(Params, ModuleDefs, Seed);
	const double SegmentsStarted = FPlatformTime::Seconds();

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
			// 소리 파동·카메라가 쓰는 전용 채널 CL9_Occlusion(Config/DefaultEngine.ini, GameTraceChannel2)도
			// 같은 컬럼 하나로 정한다 — 물통·상자 뒤로 파동이 지나간다 (segkind.md, 2026-09-17)
			Component->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
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
			// `door`는 일부러 행이 없다 — 문짝 메시로 개구부를 채우면 콜리전이 생겨 문이 막힌다.
			// 그래서 문 자리는 문틀 없는 개구부이고, 없다고 에러를 내지 않는다 (asset-swap.md).
			if (Seg.Kind != CL9SegKind::Door())
			{
				MissingKinds.Add(Seg.Kind);
			}
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

		// 바닥 종류(바닥·마당·디딤판)만 기준점 아래로 자라고, 벽과 천장은 기준점 위로 자란다.
		const bool bDownward = CL9SegKind::GrowsDown(Seg.Kind);
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
			// 구역의 회전은 여기서 직접 넣는다 — 비상구는 액터라 사이트맵에 안 붙는다. 비상구 액터는 회전 0 으로 세우고
			// 상자 치수와 안쪽 방향을 월드로 돌려 준다: ACL9FlapIn::Setup 은 안쪽 방향을 월드 방향(통과 판정)과
			// 자기 로컬 축(트리거 상자) 양쪽으로 쓰므로, 회전 0 일 때만 둘이 같다. 90° 배수라 치수는 X·Y 맞바꿈이다.
			const int32 Quarter = CL9SiteDistrict::QuarterTurns(FMath::RoundToInt32(GetActorRotation().Yaw));
			const FVector WorldExtent = (Quarter % 2 == 1) ? FVector(Extent.Y, Extent.X, Extent.Z) : Extent;
			// 안쪽은 바깥 방향의 반대다 — 배치기가 어느 변에 달았는지 알려 준다
			const FVector2D Inside = CL9SiteDistrict::Rotate(FVector2D(-Seg.Outward.X, -Seg.Outward.Y), Quarter);
			if (ACL9FlapIn* Flap = GetWorld()->SpawnActor<ACL9FlapIn>(GetActorTransform().TransformPosition(Spot), FRotator::ZeroRotator, SpawnParams))
			{
				Flap->Setup(WorldExtent, FVector(Inside.X, Inside.Y, 0.f), Mesh);
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
				Report.Note(FitPieceToCell(Cell, Mesh->GetBoundingBox(), bRunAlongX, Instance));
				ComponentFor(*Kind, Seg.Kind, Mesh)->AddInstance(Instance, /*bWorldSpace*/ false);
				++Report.Tiles;
				++Placed;
			}
			continue;
		}

		if (Kind->Placement == ECL9SegPlacement::Grid)
		{
			UStaticMesh* Lead = Piece->Variants[0].LoadSynchronous();
			if (Lead == nullptr)
			{
				MissingRoles.Add(Kind->Piece);
				continue;
			}

			// 두 평면 축 모두 `round(발자국 / 조각)`장 — 모듈 사각형은 칸의 정수배라 1.0으로 떨어지고,
			// 계단실 띠 같은 짧은 발자국만 한 장이 눌린다. 높이는 발자국 높이(판 두께)에 맞춘다.
			const FVector PieceSize = Lead->GetBoundingBox().GetSize();
			const int32 CountX = FMath::Max(1, FMath::RoundToInt(Size.X / FMath::Max(PieceSize.X, 1.f)));
			const int32 CountY = FMath::Max(1, FMath::RoundToInt(Size.Y / FMath::Max(PieceSize.Y, 1.f)));
			const FVector2D GridCell(Size.X / CountX, Size.Y / CountY);

			FTileReport& Report = Reports.FindOrAdd(Seg.Kind);
			Report.PieceSize = PieceSize;
			Report.bWarnOnFit = false;

			for (int32 IY = 0; IY < CountY; ++IY)
			{
				for (int32 IX = 0; IX < CountX; ++IX)
				{
					const FVector GridSpot(Seg.Rect.Min.X + (IX + 0.5f) * GridCell.X, Seg.Rect.Min.Y + (IY + 0.5f) * GridCell.Y, (Base + Top) * 0.5f);
					UStaticMesh* Mesh = PickMesh(*Piece, GridSpot);
					if (Mesh == nullptr)
					{
						continue;
					}
					const FBox Bounds = Mesh->GetBoundingBox();
					const FVector MeshSize = Bounds.GetSize().ComponentMax(FVector(1.f));
					const FVector Scale(GridCell.X / MeshSize.X, GridCell.Y / MeshSize.Y, (Top - Base) / MeshSize.Z);
					FTransform Instance;
					Instance.SetScale3D(Scale);
					Instance.SetLocation(GridSpot - Scale * Bounds.GetCenter());
					Report.Note(Scale.X);
					Report.Note(Scale.Y);
					ComponentFor(*Kind, Seg.Kind, Mesh)->AddInstance(Instance, /*bWorldSpace*/ false);
					++Report.Tiles;
					++Placed;
				}
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

	const double PropsStarted = FPlatformTime::Seconds();
	const int32 PropsPlaced = PlaceProps(Segs, Components.Num());
	const double RegisterStarted = FPlatformTime::Seconds();

	for (UInstancedStaticMeshComponent* Component : KindComponents)
	{
		Component->RegisterComponent();
		AddInstanceComponent(Component);
	}
	for (UPointLightComponent* Lamp : LampComponents)
	{
		Lamp->RegisterComponent();
		AddInstanceComponent(Lamp);
	}
	const double RegisterDone = FPlatformTime::Seconds();

	// 구역마다 한 줄 — 2인 PIE 에서 호스트·클라이언트의 hash 를 구역별로 맞대 보고, 단계 ms 로 프레임 하나의 무게를 본다
	{
		const FString Label = District != nullptr
			? FString::Printf(TEXT("district %d of %d (seed %u)"), DistrictIndex, Site.Districts.Num(), Seed)
			: FString::Printf(TEXT("single site (seed %u)"), Seed);
		UE_LOG(LogCL9, Log, TEXT("Site map build %s — hash %08X, %d modules, %d instances in %d components, %d props, %d lamps (%d flicker); generate %.1f ms, segments %.1f ms, props+lamps %.1f ms, register %.1f ms, total %.1f ms."),
			*Label, CL9SiteLayout::LayoutHash(Layout), Layout.Modules.Num(), Placed, KindComponents.Num(), PropsPlaced,
			LampComponents.Num(), FlickerLamps.Num(),
			(SegmentsStarted - GenerateStarted) * 1000.0, (PropsStarted - SegmentsStarted) * 1000.0,
			(RegisterStarted - PropsStarted) * 1000.0, (RegisterDone - RegisterStarted) * 1000.0, (RegisterDone - BuildStarted) * 1000.0);
	}

	for (const FName& Kind : MissingKinds)
	{
		UE_LOG(LogCL9, Error, TEXT("Site map: seg kind '%s' has no usable row — its boxes were skipped (SegKindTable)."), *Kind.ToString());
	}
	for (const FName& MissingRole : MissingRoles)
	{
		UE_LOG(LogCL9, Error, TEXT("Site map: theme '%s' has no usable piece for role '%s' — its boxes were skipped (PieceTable)."),
			*Site.Theme.ToString(), *MissingRole.ToString());
	}
	for (const TPair<FName, FTileReport>& Entry : Reports)
	{
		const FTileReport& Report = Entry.Value;
		UE_LOG(LogCL9, Log, TEXT("Site map: '%s' tiled with a %.0f x %.0f x %.0f piece — %d instances, worst run fit %.2f."),
			*Entry.Key.ToString(), Report.PieceSize.X, Report.PieceSize.Y, Report.PieceSize.Z, Report.Tiles, Report.WorstRunFit);
		if (Report.bWarnOnFit && (Report.WorstRunFit < TileFitMin || Report.WorstRunFit > TileFitMax))
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
	// 위층 몫은 덧붙는 것이라 목표는 ModuleCount가 아니라 층을 합친 값이다. 위층은 받쳐 주는
	// 자리에만 서서 목표를 다 못 채우는 게 보통이므로, 1층 몫에도 못 미칠 때만 경고한다.
	if (Layout.Modules.Num() < Params.ModuleCount)
	{
		UE_LOG(LogCL9, Warning, TEXT("Site map: asked for %d modules (%d with upper floors) but only %d fit — check the stage's socket mix (docs/plan/data/module.md)."),
			Params.ModuleCount, Layout.Wanted, Layout.Modules.Num());
	}
	int32 Stairs = 0;
	for (const FCL9ModulePlacement& Module : Layout.Modules)
	{
		Stairs += Module.bStairUp ? 1 : 0;
	}
	if (Params.MaxFloors > 1 && Layout.FloorCount < Params.MaxFloors)
	{
		UE_LOG(LogCL9, Warning, TEXT("Site map: asked for %d floors but only %d stood — the floor below was too small to carry another (docs/plan/data/module.md)."),
			Params.MaxFloors, Layout.FloorCount);
	}
	UE_LOG(LogCL9, Log, TEXT("Site map built — site '%s', stage '%s', day %d, seed %u, hash %08X, %d of %d modules on a %dx%d grid, %d floors (%.0fcm each) with %d stairs, ceilings %s, %d points, %d instances in %d components."),
		*Site.Alias.ToString(), *Site.Theme.ToString(), Day, Seed, CL9SiteLayout::LayoutHash(Layout),
		Layout.Modules.Num(), Layout.Wanted, Layout.GridSize.X, Layout.GridSize.Y, Layout.FloorCount, Params.FloorHeight, Stairs,
		Params.bBuildCeilings ? TEXT("on") : TEXT("off"),
		Layout.Points.Num(), Placed, KindComponents.Num());
	UE_LOG(LogCL9, Log, TEXT("Site map props — %d instances."), PropsPlaced);
}

int32 ACL9SiteMap::PlaceProps(const TArray<FCL9SiteSeg>& Segs, int32 FirstComponentIndex)
{
	// 문·비상구 자리 — 몸이 있는 소품이 여길 가리면 문이 막힌다. 층이 다르면 같은 자리라도 무관하다
	TArray<TPair<FBox2D, float>> Gaps;
	for (const FCL9SiteSeg& Seg : Segs)
	{
		if (Seg.Kind == CL9SegKind::Door() || Seg.Kind == CL9SegKind::FlapIn())
		{
			Gaps.Emplace(Seg.Rect, Seg.BaseZ);
		}
	}

	// 스폰이 지점 둘레로 흔드는 폭 — GameMode의 ±(CellSize/2 − 120)과 같다. 항아리는 지점 정중앙에 선다.
	const float Jitter = FMath::Max(Params.CellSize * 0.5f - 120.f, 0.f);
	// 시설 조명 밀도 — 같은 시드면 같은 등이 꺼진다 (레이아웃 시드에서 파생, 배치 순서가 시드로 정해진다)
	FRandomStream LampRng(static_cast<int32>(Layout.Seed ^ 0x5A17u));
	const float Storey = FMath::Max(Params.FloorHeight, 1.f);

	// 컴포넌트는 (메시, 몸 유무)마다 하나 — 충돌 설정이 다르면 한 묶음에 넣을 수 없다
	TMap<TPair<UStaticMesh*, bool>, UInstancedStaticMeshComponent*> Components;
	auto ComponentFor = [this, &Components, FirstComponentIndex](UStaticMesh* Mesh, bool bSolid)
	{
		const TPair<UStaticMesh*, bool> Key(Mesh, bSolid);
		if (UInstancedStaticMeshComponent* Found = Components.FindRef(Key))
		{
			return Found;
		}
		UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
			this, FName(*FString::Printf(TEXT("Prop_%s_%d"), bSolid ? TEXT("solid") : TEXT("deco"), FirstComponentIndex + Components.Num())));
		Component->SetMobility(EComponentMobility::Static);
		Component->SetStaticMesh(Mesh);
		if (bSolid)
		{
			Component->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			Component->SetCanEverAffectNavigation(true);
		}
		else
		{
			Component->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			Component->SetCanEverAffectNavigation(false);
		}
		Component->SetupAttachment(GetRootComponent());
		Components.Add(Key, Component);
		KindComponents.Add(Component);
		return Component;
	};

	int32 Placed = 0;
	TSet<FString> MissingMeshes;
	for (int32 ModuleIndex = 0; ModuleIndex < Layout.Modules.Num(); ++ModuleIndex)
	{
		const FCL9ModulePlacement& Module = Layout.Modules[ModuleIndex];
		const UCL9ModuleDef* Def = Module.Def;
		if (Def == nullptr || Def->PropSets.Num() == 0)
		{
			continue;
		}

		const FBox2D Rect(
			Layout.Origin + FVector2D(Module.Origin.X * Params.CellSize, Module.Origin.Y * Params.CellSize),
			Layout.Origin + FVector2D((Module.Origin.X + Module.Size.X) * Params.CellSize, (Module.Origin.Y + Module.Size.Y) * Params.CellSize));
		const float FloorZ = Module.Floor * Storey;

		// 세트는 자리 좌표 해시로 뽑는다 — 변형 고르기와 같은 이유로 난수 스트림을 쓰지 않는다
		const FVector2D Center = Rect.GetCenter();
		const int32 SetIndex = PickVariant(FVector(Center.X, Center.Y, FloorZ), Def->PropSets.Num());
		const FCL9PropSet& Set = Def->PropSets[SetIndex];

		for (const FCL9PropPlacement& Prop : Set.Props)
		{
			UStaticMesh* Mesh = Prop.Mesh.LoadSynchronous();
			if (Mesh == nullptr)
			{
				MissingMeshes.Add(FString::Printf(TEXT("%s/%s"), *Def->Alias.ToString(), *Prop.Mesh.ToString()));
				continue;
			}

			const FVector2D Site = CL9SiteLayout::ModuleLocalToSite(Params, Layout, Module, FVector2D(Prop.Local.X, Prop.Local.Y));
			FTransform Instance;
			Instance.SetRotation(FRotator(0.f, Prop.Yaw + Module.Rotation * 90.f, 0.f).Quaternion());
			Instance.SetLocation(FVector(Site.X, Site.Y, FloorZ + Prop.Local.Z));
			const FBox Footprint = Mesh->GetBoundingBox().TransformBy(Instance);
			const FBox2D Foot2D(FVector2D(Footprint.Min.X, Footprint.Min.Y), FVector2D(Footprint.Max.X, Footprint.Max.Y));

			// 변 규칙 — 소품 발자국이 걸친 그 변의 경계 칸을 전부 본다 (300짜리 둥지는 두 칸에 걸친다)
			if (Prop.SideRule != ECL9PropSideRule::Always && Prop.Side != ECL9PropSide::None)
			{
				const int32 Side = CL9SiteLayout::RotateSide(static_cast<int32>(Prop.Side) - 1, Module.Rotation);
				const bool bAlongX = Side == 0 || Side == 2;
				const int32 Fixed = Side == 0 ? Module.Origin.Y + Module.Size.Y - 1
					: Side == 2 ? Module.Origin.Y
					: Side == 1 ? Module.Origin.X + Module.Size.X - 1
					: Module.Origin.X;
				const int32 Count = bAlongX ? Module.Size.X : Module.Size.Y;
				const int32 First = bAlongX ? Module.Origin.X : Module.Origin.Y;
				const float Lo = bAlongX ? Foot2D.Min.X : Foot2D.Min.Y;
				const float Hi = bAlongX ? Foot2D.Max.X : Foot2D.Max.Y;
				const float Start = bAlongX ? Layout.Origin.X : Layout.Origin.Y;

				int32 Walls = 0;
				int32 Openings = 0;
				for (int32 Index = 0; Index < Count; ++Index)
				{
					const float CellLo = Start + (First + Index) * Params.CellSize;
					if (Hi <= CellLo || Lo >= CellLo + Params.CellSize)
					{
						continue;
					}
					const FIntPoint Cell = bAlongX ? FIntPoint(First + Index, Fixed) : FIntPoint(Fixed, First + Index);
					const ECL9GridEdge Edge = CL9SiteLayout::EdgeOfCell(Layout, Module.Floor, Cell, Side);
					(Edge == ECL9GridEdge::Wall ? Walls : Openings) += 1;
				}
				const bool bClosed = Openings == 0 && Walls > 0;
				if ((Prop.SideRule == ECL9PropSideRule::SideClosed) != bClosed)
				{
					continue;
				}
			}

			if (Prop.bSolid)
			{
				for (const TPair<FBox2D, float>& Gap : Gaps)
				{
					if (FMath::IsNearlyEqual(Gap.Value, FloorZ, 1.f) && Gap.Key.Intersect(Foot2D))
					{
						UE_LOG(LogCL9, Warning, TEXT("Site map: prop '%s' in module '%s' set %d (footprint %.0f..%.0f x %.0f..%.0f) covers a doorway at %.0f..%.0f x %.0f..%.0f — move it (docs/plan/data/module.md)."),
							*Mesh->GetName(), *Def->Alias.ToString(), SetIndex, Foot2D.Min.X, Foot2D.Max.X, Foot2D.Min.Y, Foot2D.Max.Y,
							Gap.Key.Min.X, Gap.Key.Max.X, Gap.Key.Min.Y, Gap.Key.Max.Y);
						break;
					}
				}
				for (const FCL9LevelPoint& Point : Layout.Points)
				{
					if (Point.Module != ModuleIndex)
					{
						continue;
					}
					// 경계가 맞닿기만 한 것(벽에 붙인 100cm 둥지가 keep-out 가장자리에 닿는 경우)은 겹침이 아니다
					const FBox2D KeepOut(Point.Local - FVector2D(Jitter - 1.f, Jitter - 1.f), Point.Local + FVector2D(Jitter - 1.f, Jitter - 1.f));
					if (KeepOut.Intersect(Foot2D))
					{
						UE_LOG(LogCL9, Warning, TEXT("Site map: prop '%s' in module '%s' set %d (footprint %.0f..%.0f x %.0f..%.0f) sits on a spawn point at (%.0f, %.0f) — keep %.0fcm clear of cell centres (docs/plan/data/module.md)."),
							*Mesh->GetName(), *Def->Alias.ToString(), SetIndex, Foot2D.Min.X, Foot2D.Max.X, Foot2D.Min.Y, Foot2D.Max.Y, Point.Local.X, Point.Local.Y, Jitter);
						break;
					}
				}
			}

			ComponentFor(Mesh, Prop.bSolid)->AddInstance(Instance, /*bWorldSpace*/ false);
			++Placed;

			if (Prop.LightIntensity > 0.f && LampRng.FRand() < Params.LampDensity)
			{
				// 시설 조명 — 인스턴스 메시는 빛을 못 품으므로 소품마다 포인트 라이트를 하나 붙인다.
				// 밀도(LampDensity)에서 빠진 등은 몸체만 서고 빛이 없다 — 불 꺼진 등.
				// 그림자를 끄고 반경을 방 하나쯤으로 잡아 수십 개가 서도 무겁지 않게 한다.
				UPointLightComponent* Lamp = NewObject<UPointLightComponent>(this, FName(*FString::Printf(TEXT("Lamp_%d"), LampComponents.Num())));
				Lamp->SetMobility(EComponentMobility::Movable);
				Lamp->SetIntensityUnits(ELightUnits::Candelas);
				Lamp->SetIntensity(Prop.LightIntensity);
				Lamp->SetAttenuationRadius(FMath::Max(Prop.LightRadius, 50.f));
				Lamp->SetLightColor(Prop.LightColor);
				Lamp->SetCastShadows(false);
				Lamp->SetVolumetricScatteringIntensity(Prop.LightScattering);
				Lamp->SetupAttachment(GetRootComponent());
				Lamp->SetRelativeTransform(FTransform(Instance.GetRotation(), Instance.GetLocation() + Instance.GetRotation().RotateVector(Prop.LightOffset)));
				Lamp->SetVisibility(bLampsOn);
				if (Prop.LightFlicker > 0.f && Prop.LightFlickerSpeed > 0.f)
				{
					// 위상은 램프 순번에서 — 배치 순서가 시드로 정해지니 같은 시드면 같은 깜박임이고, 램프끼리는 어긋난다
					// (자리 좌표로 만들면 500cm 격자에 맞은 자리끼리 같은 위상이 나온다 — 09-17 실측)
					FFlickerLamp Entry;
					Entry.LampIndex = LampComponents.Num();
					Entry.BaseIntensity = Prop.LightIntensity;
					Entry.Amplitude = Prop.LightFlicker;
					Entry.Speed = Prop.LightFlickerSpeed;
					Entry.Phase = LampComponents.Num() * 37.f;
					FlickerLamps.Add(Entry);
				}
				LampComponents.Add(Lamp);
			}
		}
	}

	for (const FString& Missing : MissingMeshes)
	{
		UE_LOG(LogCL9, Error, TEXT("Site map: prop mesh '%s' failed to load — skipped (ModuleTable PropSets)."), *Missing);
	}
	return Placed;
}
