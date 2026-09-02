// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Item.h"
#include "Character/CL9Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9Types.h"
#include "GameMode/CL9GameInstance.h"

namespace
{
	/** Below this impact-frame speed a bouncing item settles, in cm/s */
	constexpr float RestSpeed = 30.f;
}

ACL9Item::ACL9Item()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SetRootComponent(SphereComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SphereComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACL9Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9Item, HoldState);
}

void ACL9Item::LoadData()
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ItemDef* Def = Registry ? Registry->FindItem(GetAlias(), GetSubAlias()) : nullptr;

	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s': item row '%s'/'%s' is missing or broken — visuals stay stale (docs/data/schema.md 로드 검사)."),
			*GetNameSafe(this), *GetAlias().ToString(), *GetSubAlias().ToString());
		return;
	}
	ItemDef = Def;

	SphereComponent->SetSphereRadius(Def->Radius);

	if (UStaticMesh* Mesh = Def->Mesh.LoadSynchronous())
	{
		MeshComponent->SetStaticMesh(Mesh);

		// Temp meshes are basic shapes — fit them to the ruling radius until
		// bought assets replace them through the table (build-order 원칙 2).
		const float MeshRadius = Mesh->GetBounds().SphereRadius;
		if (MeshRadius > UE_KINDA_SMALL_NUMBER)
		{
			MeshComponent->SetWorldScale3D(FVector(Def->Radius / MeshRadius));
		}
	}

	for (UCL9Gimmick* Gimmick : Def->Gimmicks)
	{
		if (Gimmick != nullptr)
		{
			Gimmick->OnActorLoaded(this);
		}
	}
}

bool ACL9Item::CanBePickedUp() const
{
	// Items on a cart bed stay pickable — taking one off the bed is the same
	// aim-and-press as picking it off the ground (2026-09-02 상호작용 통합).
	return (HoldState == ECL9ItemHoldState::Free || HoldState == ECL9ItemHoldState::OnCart)
		&& ItemDef != nullptr
		&& ItemDef->CarrySlot != ECL9CarrySlot::CartOnly;
}

void ACL9Item::OnPickedUp(ACL9Character* Holder, const FVector& RelativeLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	HoldState = ECL9ItemHoldState::Held;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();

	AttachToComponent(Holder->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(RelativeLocation);
}

void ACL9Item::OnLoadedIntoCart(AActor* Cart, const FVector& RelativeLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	HoldState = ECL9ItemHoldState::OnCart;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();

	AttachToComponent(Cart->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(RelativeLocation);
}

void ACL9Item::OnPlaced(const FVector& WorldLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(WorldLocation);
	HoldState = ECL9ItemHoldState::Free;
	SimVelocity = FVector::ZeroVector;
	bAtRest = false;
	ApplyHoldState();
}

void ACL9Item::OnThrown(const FVector& Origin, const FVector& LaunchVelocity)
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(Origin);
	HoldState = ECL9ItemHoldState::Free;
	SimVelocity = LaunchVelocity;
	bAtRest = false;
	ApplyHoldState();
}

void ACL9Item::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// The sim is host-only and authoritative — clients interpolate replicated
	// movement (docs/systems/item.md 액터 틀).
	if (!HasAuthority() || HoldState != ECL9ItemHoldState::Free || bAtRest || ItemDef == nullptr)
	{
		return;
	}

	SimVelocity.Z -= ItemDef->Gravity * DeltaSeconds;

	FHitResult Hit;
	AddActorWorldOffset(SimVelocity * DeltaSeconds, true, &Hit);

	if (Hit.bBlockingHit)
	{
		const float ImpactSpeed = -FVector::DotProduct(SimVelocity, Hit.Normal);
		HandleImpact(ImpactSpeed);

		const FVector NormalPart = FVector::DotProduct(SimVelocity, Hit.Normal) * Hit.Normal;
		const FVector TangentPart = SimVelocity - NormalPart;
		SimVelocity = TangentPart * (1.f - ItemDef->Friction) - NormalPart * ItemDef->Restitution;

		const bool bOnGround = Hit.Normal.Z > 0.7f;
		if (bOnGround && SimVelocity.SizeSquared() < RestSpeed * RestSpeed)
		{
			SimVelocity = FVector::ZeroVector;
			bAtRest = true;
		}
	}
}

void ACL9Item::HandleImpact(float ImpactSpeed)
{
	if (ItemDef == nullptr || !ItemDef->bFragile)
	{
		return;
	}

	// Only the base state and the cracked state ride the impact chain — a
	// cooked/burnt variant keeps its cooking state (조합 상태는 미정).
	const bool bCracked = GetSubAlias() == CL9::SubAliasCracked;
	if (!bCracked && !GetSubAlias().IsNone())
	{
		return;
	}

	const float BreakThreshold = bCracked ? ItemDef->CrackedBreakSpeed : ItemDef->BreakSpeed;
	if (ImpactSpeed >= BreakThreshold)
	{
		SetAlias(GetAlias(), CL9::SubAliasBroken);
	}
	else if (!bCracked && ImpactSpeed >= ItemDef->CrackSpeed)
	{
		SetAlias(GetAlias(), CL9::SubAliasCracked);
	}
}

void ACL9Item::OnRep_HoldState()
{
	ApplyHoldState();
}

void ACL9Item::ApplyHoldState()
{
	// Held items ride the holder's capsule — their own collision would push
	// the holder around, so it goes dark. Cart-bed items keep query collision
	// so the aim detection can still find them.
	SetActorEnableCollision(HoldState != ECL9ItemHoldState::Held);
}
