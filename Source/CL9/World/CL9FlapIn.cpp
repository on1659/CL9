// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9FlapIn.h"
#include "Character/CL9Character.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"

namespace
{
	/** 트리거가 벽을 기준으로 안쪽/바깥쪽으로 얼마나 뻗어나가는지 */
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
		// 조각의 실제 크기와 피벗을 읽어서 개구부에 맞춘다 — 단위 큐브를 가정하면
		// 피벗이 중심에 없는 문짝이 엉뚱한 자리에 가서 붙는다 (docs/systems/asset-swap.md).
		Visual->SetStaticMesh(Mesh);
		const FBox Bounds = Mesh->GetBoundingBox();
		const FVector MeshSize = Bounds.GetSize();
		const FVector Scale(
			MeshSize.X > 1.f ? Extent.X * 2.f / MeshSize.X : 1.f,
			MeshSize.Y > 1.f ? Extent.Y * 2.f / MeshSize.Y : 1.f,
			MeshSize.Z > 1.f ? Extent.Z * 2.f / MeshSize.Z : 1.f);
		Visual->SetRelativeScale3D(Scale);
		Visual->SetRelativeLocation(-(Scale * Bounds.GetCenter()));
	}

	// 트리거는 안쪽 방향을 따라 벽을 걸친다: 안쪽은 깊게, 바깥쪽은 얕게
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
	// 이미 안쪽에 있던 사람만 밀고 나갈 수 있다
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
