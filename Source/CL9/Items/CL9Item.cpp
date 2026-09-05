// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Item.h"
#include "Character/CL9Character.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9Types.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"

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
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SphereComponent->SetNotifyRigidBodyCollision(true);
	SphereComponent->OnComponentHit.AddDynamic(this, &ACL9Item::OnPhysicsHit);
	SetRootComponent(SphereComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SphereComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACL9Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9Item, HoldState);
	DOREPLIFETIME(ACL9Item, bLightOn);
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

	// The table's friction/restitution drive the physics path via a per-row material
	if (Def->bPhysicsEnabled)
	{
		if (PhysicsMaterial == nullptr)
		{
			PhysicsMaterial = NewObject<UPhysicalMaterial>(this);
		}
		PhysicsMaterial->Friction = Def->Friction;
		PhysicsMaterial->Restitution = Def->Restitution;
		SphereComponent->SetPhysMaterialOverride(PhysicsMaterial);
	}

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

	SetupLight();

	// The row decides the sim (physics vs simple bounce) — re-apply for it
	ApplyHoldState();
}

bool ACL9Item::IsFlashlight() const
{
	return ItemDef != nullptr && ItemDef->bEmitsLight && ItemDef->LightKind == ECL9ItemLight::Flashlight;
}

void ACL9Item::SetupLight()
{
	// The row's light fields turn into a real component here — a point light for
	// a glow (egg), a forward spot for a flashlight (docs/systems/light.md). Built
	// once; a SubAlias reload (cooked/cracked) keeps the same component.
	if (LightComponent != nullptr || ItemDef == nullptr || !ItemDef->bEmitsLight)
	{
		return;
	}

	if (ItemDef->LightKind == ECL9ItemLight::Flashlight)
	{
		USpotLightComponent* Spot = NewObject<USpotLightComponent>(this, TEXT("ItemLight"));
		Spot->SetOuterConeAngle(ItemDef->LightConeAngle);
		Spot->SetInnerConeAngle(FMath::Max(0.f, ItemDef->LightConeAngle - 8.f));
		Spot->SetAttenuationRadius(FMath::Max(ItemDef->LightRadius, 100.f));
		LightComponent = Spot;
	}
	else
	{
		UPointLightComponent* Point = NewObject<UPointLightComponent>(this, TEXT("ItemLight"));
		Point->SetAttenuationRadius(FMath::Max(ItemDef->LightRadius, 50.f));
		LightComponent = Point;
	}

	LightComponent->SetIntensity(ItemDef->LightIntensity);
	LightComponent->SetupAttachment(MeshComponent);
	LightComponent->SetVisibility(false);
	LightComponent->RegisterComponent();
	UpdateLightVisibility();
}

void ACL9Item::SetLightSwitch(bool bOn)
{
	if (!HasAuthority() || !IsFlashlight())
	{
		return;
	}

	// The blackout is the flashlight's counter — it cannot be lit past it (light.md).
	// A worn lamp is the wearer's own gear and stays out of it.
	const ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
	if (bOn && !IsWorn() && CL9GameState != nullptr && CL9GameState->IsBlackout())
	{
		return;
	}

	if (bLightOn != bOn)
	{
		bLightOn = bOn;
		OnRep_LightOn();
	}
}

void ACL9Item::OnRep_LightOn()
{
	UpdateLightVisibility();
}

void ACL9Item::UpdateLightVisibility()
{
	if (LightComponent == nullptr)
	{
		return;
	}

	// Each machine derives the same result: switch on, minus the blackout for a
	// flashlight (a glow stays lit — it's the light that remains, light.md).
	bool bVisible = bLightOn;
	if (bVisible && IsFlashlight() && !IsWorn())
	{
		const ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
		if (CL9GameState != nullptr && CL9GameState->IsBlackout())
		{
			bVisible = false;
		}
	}
	LightComponent->SetVisibility(bVisible);
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

	// A glow lights up the moment it is carried; a flashlight starts off and the
	// holder toggles it (docs/systems/light.md).
	if (ItemDef != nullptr && ItemDef->bEmitsLight)
	{
		bLightOn = (ItemDef->LightKind == ECL9ItemLight::Glow);
		OnRep_LightOn();
	}

	AttachToComponent(Holder->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(RelativeLocation);
}

void ACL9Item::OnWorn(ACL9Character* Wearer)
{
	if (!HasAuthority() || Wearer == nullptr)
	{
		return;
	}

	HoldState = ECL9ItemHoldState::Worn;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();

	// Same start as a held flashlight: off until the wearer toggles it
	if (ItemDef != nullptr && ItemDef->bEmitsLight)
	{
		bLightOn = (ItemDef->LightKind == ECL9ItemLight::Glow);
		OnRep_LightOn();
	}

	// The camera carries the owner's full look; on other machines it only yaws
	// with the body (pitch follow is a later step). Sits just above the eyes.
	AttachToComponent(Wearer->GetFirstPersonCameraComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(FVector(8.f, 0.f, 10.f));
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

	// Dropped items are dark — a glow only glows while carried, a flashlight is off
	if (bLightOn)
	{
		bLightOn = false;
		OnRep_LightOn();
	}
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

	if (bLightOn)
	{
		bLightOn = false;
		OnRep_LightOn();
	}

	if (SphereComponent->IsSimulatingPhysics())
	{
		SphereComponent->SetPhysicsLinearVelocity(LaunchVelocity);
	}
}

void ACL9Item::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// A lit flashlight polls the blackout on every machine so the beam dies the
	// moment it hits — returns pull the moment forward, so a fixed timer would be
	// wrong (docs/systems/light.md). The host also flips the switch off to persist it.
	if (bLightOn && IsFlashlight() && !IsWorn())
	{
		UpdateLightVisibility();
		if (HasAuthority())
		{
			const ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
			if (CL9GameState != nullptr && CL9GameState->IsBlackout())
			{
				bLightOn = false;
			}
		}
	}

	// The sim is host-only and authoritative — clients interpolate replicated
	// movement (docs/systems/item.md 액터 틀).
	if (!HasAuthority() || HoldState != ECL9ItemHoldState::Free || ItemDef == nullptr)
	{
		return;
	}

	// Physics rows: the engine simulates; only the table's custom gravity is
	// injected (as acceleration, so mass stays irrelevant).
	if (ItemDef->bPhysicsEnabled)
	{
		if (SphereComponent->IsSimulatingPhysics() && SphereComponent->IsAnyRigidBodyAwake())
		{
			SphereComponent->AddForce(FVector(0.f, 0.f, -ItemDef->Gravity), NAME_None, true);
		}
		return;
	}

	if (bAtRest)
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
	SetActorEnableCollision(HoldState != ECL9ItemHoldState::Held && HoldState != ECL9ItemHoldState::Worn);

	// A worn lamp sits in front of the wearer's eyes — only others see it
	MeshComponent->SetOwnerNoSee(HoldState == ECL9ItemHoldState::Worn);

	// Real physics runs only for free items on the host (2026-09-03 결정) —
	// clients render replicated movement, every other state is attached.
	const bool bWantPhysics = HoldState == ECL9ItemHoldState::Free
		&& HasAuthority()
		&& ItemDef != nullptr
		&& ItemDef->bPhysicsEnabled;
	if (SphereComponent->IsSimulatingPhysics() != bWantPhysics)
	{
		SphereComponent->SetSimulatePhysics(bWantPhysics);
	}
	if (bWantPhysics)
	{
		// The table's Gravity column replaces world gravity (injected in Tick)
		SphereComponent->SetEnableGravity(false);
	}
}

void ACL9Item::OnPhysicsHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || ItemDef == nullptr || !ItemDef->bFragile || HoldState != ECL9ItemHoldState::Free)
	{
		return;
	}

	// Impulse ≈ m·(1+e)·v_impact, so recover the approach speed from it
	const float Mass = FMath::Max(HitComponent->GetMass(), UE_KINDA_SMALL_NUMBER);
	const float ImpactSpeed = NormalImpulse.Size() / (Mass * (1.f + ItemDef->Restitution));
	HandleImpact(ImpactSpeed);
}
