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
		// **폭만 개구부에 맞추고 두께·높이는 자연값이다** — 문짝을 개구부 높이(580)까지
		// 늘리면 손잡이·밀대가 있는 판이 뭉개진다. 개구부 위쪽은 Body 상자가 그대로 막는다.
		// 개구부가 어느 축으로 나 있든 문짝의 긴 축이 그 축을 따르도록 90° 돌린다.
		Visual->SetStaticMesh(Mesh);
		const FBox Bounds = Mesh->GetBoundingBox();
		const FVector MeshSize = Bounds.GetSize().ComponentMax(FVector(1.f));
		const bool bGapAlongX = Extent.X >= Extent.Y;
		const bool bMeshAlongX = MeshSize.X >= MeshSize.Y;
		const float GapWidth = (bGapAlongX ? Extent.X : Extent.Y) * 2.f;
		const float WidthScale = GapWidth / (bMeshAlongX ? MeshSize.X : MeshSize.Y);
		const float HeightScale = FMath::Min(1.f, Extent.Z * 2.f / MeshSize.Z);
		const FVector Scale = bMeshAlongX ? FVector(WidthScale, 1.f, HeightScale) : FVector(1.f, WidthScale, HeightScale);
		const FQuat Rotation = FRotator(0.f, bGapAlongX == bMeshAlongX ? 0.f : 90.f, 0.f).Quaternion();
		Visual->SetRelativeRotation(Rotation);
		Visual->SetRelativeScale3D(Scale);
		// 평면은 개구부 가운데, 아래는 개구부 바닥에 앉힌다
		const FVector Centered = -Rotation.RotateVector(Scale * Bounds.GetCenter());
		Visual->SetRelativeLocation(FVector(Centered.X, Centered.Y, -Extent.Z - Bounds.Min.Z * HeightScale));
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
