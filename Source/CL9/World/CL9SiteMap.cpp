// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9SiteMap.h"
#include "CL9.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9SegKindDef.h"
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
	/** Mesh rows are unit cubes — scale = size / this */
	constexpr float UnitCube = 100.f;

	FCL9HenhouseParams ParamsFromSite(const UCL9SiteDef& Site)
	{
		FCL9HenhouseParams P;
		P.AisleCount1F = Site.AisleCount;
		P.ColCount1F = Site.ColCount;
		P.AisleCountB1 = Site.AisleCountB1;
		P.ColCountB1 = Site.ColCountB1;
		P.AisleWidth = Site.AisleWidth;
		P.ColWidth = Site.ColWidth;
		P.RowDepth = Site.RowDepth;
		P.OpenWidth = Site.OpenWidth;
		P.PlugWidth = Site.PlugWidth;
		P.TargetOpen1F = Site.TargetOpen1F;
		P.TargetOpenB1 = Site.TargetOpenB1;
		P.JarCount1F = Site.JarCount1F;
		P.JarCountB1 = Site.JarCountB1;
		return P;
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

ACL9SiteMap* ACL9SiteMap::Rebuild(UWorld* World, const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed)
{
	if (World == nullptr)
	{
		return nullptr;
	}

	ACL9SiteMap* Map = Find(World);
	if (Map == nullptr)
	{
		// The truck pad anchors the site; a PlayerStart stands in on a bare level
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

	Map->Build(Site, Registry, Seed);
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

FVector ACL9SiteMap::CellToWorld(int32 Floor, int32 Aisle, int32 Col) const
{
	return SiteToWorld(CL9SiteLayout::CellCenter(Params, Frame, Floor, Aisle, Col), Floor);
}

FVector ACL9SiteMap::SiteToWorld(const FVector2D& Local, int32 Floor) const
{
	return GetActorLocation() + FVector(Local.X, Local.Y, Frame.FloorBase(Floor));
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

void ACL9SiteMap::Build(const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed)
{
	ClearBuilt();

	Params = ParamsFromSite(Site);
	Frame = CL9SiteLayout::ComputeFrame(Params);
	Layout = CL9SiteLayout::Generate(Params, Seed);

	TArray<FCL9SiteSeg> Segs;
	CL9SiteLayout::BuildSegments(Params, Frame, Layout, Segs);

	TMap<FName, UInstancedStaticMeshComponent*> ByKind;
	TSet<FName> Missing;
	int32 Placed = 0;

	for (const FCL9SiteSeg& Seg : Segs)
	{
		const UCL9SegKindDef* Kind = Registry.FindSegKind(Seg.Kind);
		UStaticMesh* Mesh = Kind ? Kind->Mesh.LoadSynchronous() : nullptr;
		if (Kind == nullptr || Mesh == nullptr)
		{
			Missing.Add(Seg.Kind);
			continue;
		}

		const FVector2D Size = Seg.Rect.GetSize();
		const FVector2D Center = Seg.Rect.GetCenter();

		// Vertical span by kind: floors hang below their level, stair walls
		// reach from the lower floor to Height above the upper one.
		float Base, Top;
		if (Kind->BaseZMode == ECL9SegBaseZ::WorldTop)
		{
			Base = Frame.FloorBase(1);
			Top = Frame.FloorBase(0) + Kind->Height;
		}
		else if (Seg.Kind == CL9SegKind::Floor())
		{
			Top = Frame.FloorBase(Seg.Floor);
			Base = Top - Kind->Height;
		}
		else if (Seg.Kind == CL9SegKind::Ceiling())
		{
			Base = CL9SiteLayout::CeilingBase(Frame, Seg.Floor);
			Top = Base + Kind->Height;
		}
		else
		{
			Base = Frame.FloorBase(Seg.Floor);
			Top = Base + Kind->Height;
		}

		if (Seg.Kind == CL9SegKind::FlapIn())
		{
			const FVector Extent(Size.X * 0.5f, Size.Y * 0.5f, (Top - Base) * 0.5f);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.ObjectFlags |= RF_Transient;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			const FVector Location = GetActorLocation() + FVector(Center.X, Center.Y, (Base + Top) * 0.5f);
			if (ACL9FlapIn* Flap = GetWorld()->SpawnActor<ACL9FlapIn>(Location, FRotator::ZeroRotator, SpawnParams))
			{
				// The flap sits in the east wall — inside is west
				Flap->Setup(Extent, -FVector::ForwardVector, Mesh);
				Flap->bIsEditorOnlyActor = bIsEditorOnlyActor;
				SpawnedActors.Add(Flap);
				++Placed;
			}
			continue;
		}

		UInstancedStaticMeshComponent* Component = ByKind.FindRef(Seg.Kind);
		if (Component == nullptr)
		{
			Component = NewObject<UInstancedStaticMeshComponent>(this, FName(*FString::Printf(TEXT("Seg_%s"), *Seg.Kind.ToString())));
			Component->SetMobility(EComponentMobility::Static);
			Component->SetStaticMesh(Mesh);
			Component->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			if (!Kind->bBlocksSight)
			{
				Component->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
			}
			Component->SetCanEverAffectNavigation(true);
			Component->SetupAttachment(GetRootComponent());
			ByKind.Add(Seg.Kind, Component);
			KindComponents.Add(Component);
		}

		FTransform Instance;
		if (Seg.bRamp)
		{
			const float Drop = Frame.FloorDrop;
			const float Length = FMath::Sqrt(Size.X * Size.X + Drop * Drop);
			const float Pitch = -FMath::RadiansToDegrees(FMath::Atan2(Drop, Size.X));
			Instance.SetRotation(FRotator(Pitch, 0.f, 0.f).Quaternion());
			Instance.SetLocation(FVector(Center.X, Center.Y, (Frame.FloorBase(0) + Frame.FloorBase(1)) * 0.5f - Kind->Height * 0.5f));
			Instance.SetScale3D(FVector(Length, Size.Y, Kind->Height) / UnitCube);
		}
		else
		{
			Instance.SetLocation(FVector(Center.X, Center.Y, (Base + Top) * 0.5f));
			Instance.SetScale3D(FVector(Size.X, Size.Y, Top - Base) / UnitCube);
		}
		Component->AddInstance(Instance, /*bWorldSpace*/ false);
		++Placed;
	}

	for (UInstancedStaticMeshComponent* Component : KindComponents)
	{
		Component->RegisterComponent();
		AddInstanceComponent(Component);
	}

	for (const FName& Kind : Missing)
	{
		UE_LOG(LogCL9, Error, TEXT("Site map: seg kind '%s' has no usable row or mesh — its boxes were skipped (DA_SegKindTable)."), *Kind.ToString());
	}
	UE_LOG(LogCL9, Log, TEXT("Site map built — site '%s', seed %u, hash %08X, %d boxes in %d kinds%s."),
		*Site.Alias.ToString(), Seed, CL9SiteLayout::LayoutHash(Layout), Placed, KindComponents.Num(),
		Layout.bHasMaze ? TEXT("") : TEXT(" (shell only)"));
}
