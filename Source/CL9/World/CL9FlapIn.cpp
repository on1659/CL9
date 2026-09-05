// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9FlapIn.h"
#include "Character/CL9Character.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

namespace
{
	/** How far the trigger reaches past the wall on the inside / outside */
	constexpr float TriggerInside = 135.f;
	constexpr float TriggerOutside = 45.f;
}

ACL9FlapIn::ACL9FlapIn()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	Body->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Body->SetCanEverAffectNavigation(true);
	SetRootComponent(Body);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Body);
	Visual->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Visual->SetCanEverAffectNavigation(false);

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Body);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldDynamic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
	Trigger->SetCanEverAffectNavigation(false);
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACL9FlapIn::OnTriggerBegin);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &ACL9FlapIn::OnTriggerEnd);
}

void ACL9FlapIn::Setup(const FVector& Extent, const FVector& InsideDirection, UStaticMesh* Mesh)
{
	InsideDir = InsideDirection.GetSafeNormal();
	Body->SetBoxExtent(Extent);

	if (Mesh != nullptr)
	{
		Visual->SetStaticMesh(Mesh);
		Visual->SetRelativeScale3D(Extent * 2.f / 100.f);
	}

	// The trigger straddles the wall along the inside direction: deep inside, shallow outside
	const FVector Along = InsideDir.GetAbs();
	const FVector Across = FVector::OneVector - Along;
	const float Reach = (TriggerInside + TriggerOutside) * 0.5f;
	Trigger->SetBoxExtent(Extent * Across + Along * (FVector::DotProduct(Extent, Along) + Reach));
	Trigger->SetRelativeLocation(InsideDir * ((TriggerInside - TriggerOutside) * 0.5f));
}

void ACL9FlapIn::OnTriggerBegin(UPrimitiveComponent* /*Overlapped*/, AActor* Other, UPrimitiveComponent* /*OtherComp*/, int32 /*BodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*Sweep*/)
{
	ACL9Character* Character = Cast<ACL9Character>(Other);
	if (Character == nullptr)
	{
		return;
	}
	// Only someone already on the inside gets to push through
	const FVector ToOther = Character->GetActorLocation() - GetActorLocation();
	if (FVector::DotProduct(ToOther, InsideDir) > 0.f)
	{
		Character->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
	}
}

void ACL9FlapIn::OnTriggerEnd(UPrimitiveComponent* /*Overlapped*/, AActor* Other, UPrimitiveComponent* /*OtherComp*/, int32 /*BodyIndex*/)
{
	if (ACL9Character* Character = Cast<ACL9Character>(Other))
	{
		Character->GetCapsuleComponent()->IgnoreActorWhenMoving(this, false);
	}
}
