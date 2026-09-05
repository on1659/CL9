// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9FryingPan.h"
#include "CL9.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/CL9ItemDef.h"
#include "Engine/CollisionProfile.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

namespace
{
	/** A pan is a flattened cylinder — the row's unit mesh squashed to this of its height */
	constexpr float PanFlatten = 0.25f;

	/** How high above the bed an item still counts as on the heat (cm) */
	constexpr float HeatReach = 60.f;
}

ACL9FryingPan::ACL9FryingPan()
{
	Bed = CreateDefaultSubobject<UBoxComponent>(TEXT("Bed"));
	Bed->SetupAttachment(GetRootComponent());
	Bed->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Bed->SetCanEverAffectNavigation(false);

	HeatZone = CreateDefaultSubobject<UBoxComponent>(TEXT("HeatZone"));
	HeatZone->SetupAttachment(GetRootComponent());
	HeatZone->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	HeatZone->SetCanEverAffectNavigation(false);
}

void ACL9FryingPan::LoadData()
{
	Super::LoadData();

	const UCL9ItemDef* Def = GetItemDef();
	UStaticMeshComponent* Mesh = FindComponentByClass<UStaticMeshComponent>();
	if (Def == nullptr || Mesh == nullptr)
	{
		return;
	}

	// The base fits the mesh to the ruling sphere uniformly; a pan is flat
	const FVector Scale = Mesh->GetRelativeScale3D();
	Mesh->SetRelativeScale3D(FVector(Scale.X, Scale.Y, Scale.X * PanFlatten));
	Mesh->UpdateBounds();
	const float Top = Mesh->Bounds.BoxExtent.Z;

	const float Half = Def->Radius * 0.7f;
	Bed->SetBoxExtent(FVector(Half, Half, 4.f));
	Bed->SetRelativeLocation(FVector(0.f, 0.f, Top - 4.f));
	HeatZone->SetBoxExtent(FVector(Half, Half, HeatReach * 0.5f));
	HeatZone->SetRelativeLocation(FVector(0.f, 0.f, Top + HeatReach * 0.5f));
}

void ACL9FryingPan::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// The ruling is the host's, and a pan in someone's hands cooks nothing
	if (HasAuthority() && IsFree())
	{
		TickCooking(DeltaSeconds);
	}
}

void ACL9FryingPan::TickCooking(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Whatever item lies in the zone right now — queried, not event-driven, so a
	// thing that rolled in without an overlap event still counts
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9Cooking), false, this);
	World->OverlapMultiByObjectType(Overlaps, HeatZone->GetComponentLocation(), HeatZone->GetComponentQuat(),
		ObjectParams, FCollisionShape::MakeBox(HeatZone->GetScaledBoxExtent()), QueryParams);

	TSet<ACL9Item*> OnHeat;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ACL9Item* Item = Cast<ACL9Item>(Overlap.GetActor());
		if (Item != nullptr && Item != this && Item->IsFree())
		{
			OnHeat.Add(Item);
		}
	}

	// Off the heat, the clock resets — nothing half-cooks in your hand
	for (auto It = HeatSeconds.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !OnHeat.Contains(It.Key().Get()))
		{
			SmokeWarned.Remove(It.Key());
			It.RemoveCurrent();
		}
	}

	for (ACL9Item* Item : OnHeat)
	{
		const UCL9ItemDef* Def = Item->GetItemDef();
		if (Def == nullptr)
		{
			continue;
		}
		// Raw rows cook, cooked rows burn, burnt rows (and tools) just sit there
		const bool bCooks = Def->CookResult.IsSet();
		const bool bBurns = Def->BurnResult.IsSet();
		if (!bCooks && !bBurns)
		{
			continue;
		}
		const FCL9ItemRef& Next = bCooks ? Def->CookResult : Def->BurnResult;
		const float Needed = bCooks ? Def->CookTime : Def->BurnTime;

		float& Seconds = HeatSeconds.FindOrAdd(Item);
		Seconds += DeltaSeconds;

		// Smoke lead-in before burning — presentation only, the ruling ignores it (cooking.md)
		if (!bCooks && !SmokeWarned.Contains(Item) && Seconds >= Needed - Def->BurnWarnTime)
		{
			SmokeWarned.Add(Item);
			UE_LOG(LogCL9, Log, TEXT("'%s' (%s/%s) starts smoking on '%s' — %.0fs to burnt."),
				*GetNameSafe(Item), *Item->GetAlias().ToString(), *Item->GetSubAlias().ToString(), *GetNameSafe(this), Def->BurnWarnTime);
		}

		if (Seconds >= Needed)
		{
			UE_LOG(LogCL9, Log, TEXT("'%s': %s/%s → %s/%s after %.1fs on '%s'."),
				*GetNameSafe(Item), *Item->GetAlias().ToString(), *Item->GetSubAlias().ToString(),
				*Next.ItemAlias.ToString(), *Next.ItemSubAlias.ToString(), Seconds, *GetNameSafe(this));
			Item->SetAlias(Next.ItemAlias, Next.ItemSubAlias);
			Seconds = 0.f;
			SmokeWarned.Remove(Item);
		}
	}
}
