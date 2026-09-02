// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/CL9Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Character/CL9CharacterMovement.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "InputActionValue.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Core/CL9Interactable.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9Constants.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9RulesDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"
#include "GameMode/CL9PlayerState.h"
#include "Items/CL9Cart.h"
#include "Items/CL9Item.h"

namespace
{
	/** Focus candidates must be at least this far into the view (dot against the aim) */
	constexpr float FocusMinAimDot = 0.3f;
}

ACL9Character::ACL9Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCL9CharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(CL9::CapsuleRadius, CL9::CapsuleHalfHeight);

	// Camera sits at eye height; only the owner sees the first person mesh
	// (docs/systems/character.md "시점 — 1인칭").
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, CL9::EyeHeight - CL9::CapsuleHalfHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Full body mesh is what other players see; its own transform is the
	// standard mannequin fit (feet at capsule bottom, facing +X).
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -CL9::CapsuleHalfHeight), FRotator(0.f, -90.f, 0.f));

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	SetNetUpdateFrequency(20.f);
}

void ACL9Character::SetCharacterAlias(FName NewAlias)
{
	if (!HasAuthority())
	{
		return;
	}

	CharacterAlias = NewAlias;
	ApplyCharacterDef();
}

UCL9CharacterMovement* ACL9Character::GetCL9CharacterMovement() const
{
	return CastChecked<UCL9CharacterMovement>(GetCharacterMovement());
}

void ACL9Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9Character, CharacterAlias);
	DOREPLIFETIME(ACL9Character, HeldItem);
	DOREPLIFETIME(ACL9Character, PullingCart);
}

void ACL9Character::OnRep_CharacterAlias()
{
	ApplyCharacterDef();
}

void ACL9Character::ApplyCharacterDef()
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9CharacterDef* Def = Registry ? Registry->FindCharacter(CharacterAlias) : nullptr;

	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s': character row '%s' is missing or broken — engine defaults stay (docs/data/schema.md 로드 검사)."),
			*GetNameSafe(this), *CharacterAlias.ToString());
		return;
	}
	CharacterDef = Def;

	// Ruling values — every machine applies the same local table row, so the
	// autonomous proxy predicts with the numbers the host rules with
	// (conventions § 5).
	GetCapsuleComponent()->SetCapsuleSize(Def->CapsuleRadius, Def->CapsuleHalfHeight);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, Def->EyeHeight - Def->CapsuleHalfHeight));

	UCL9CharacterMovement* Movement = GetCL9CharacterMovement();
	Movement->SetCharacterDef(Def);
	Movement->MaxWalkSpeed = Def->WalkSpeed;
	Movement->JumpZVelocity = Def->JumpVelocity;
	Movement->GravityScale = Def->JumpGravity / CL9::DefaultGravityZ;

	// Presentation — meshes come from the row so bought assets swap without C++.
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -Def->CapsuleHalfHeight));
	if (USkeletalMesh* BodyMesh = Def->SkeletalMesh.LoadSynchronous())
	{
		GetMesh()->SetSkeletalMesh(BodyMesh);
	}
	if (UClass* BodyAnimClass = Def->AnimBP.LoadSynchronous())
	{
		GetMesh()->SetAnimInstanceClass(BodyAnimClass);
	}
	if (USkeletalMesh* ArmsMesh = Def->ArmsMesh.LoadSynchronous())
	{
		FirstPersonMesh->SetSkeletalMesh(ArmsMesh);
	}
	if (UClass* ArmsAnimClass = Def->ArmsAnimBP.LoadSynchronous())
	{
		FirstPersonMesh->SetAnimInstanceClass(ArmsAnimClass);
	}
}

void ACL9Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACL9Character::MoveInput);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACL9Character::LookInput);
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Started, this, &ACL9Character::RunInputStart);
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &ACL9Character::RunInputEnd);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInput->BindAction(PickPlaceAction, ETriggerEvent::Started, this, &ACL9Character::PickPlaceInput);
		EnhancedInput->BindAction(ThrowAction, ETriggerEvent::Started, this, &ACL9Character::ThrowStartInput);
		EnhancedInput->BindAction(ThrowAction, ETriggerEvent::Completed, this, &ACL9Character::ThrowEndInput);
		EnhancedInput->BindAction(SwapAction, ETriggerEvent::Started, this, &ACL9Character::SwapInput);
	}
	else
	{
		UE_LOG(LogCL9, Error, TEXT("'%s' expects an Enhanced Input component — legacy input is not supported (docs/systems/input.md)."), *GetNameSafe(this));
	}
}

void ACL9Character::MoveInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Axis.X);
		AddMovementInput(GetActorForwardVector(), Axis.Y);
	}
}

void ACL9Character::LookInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (GetController())
	{
		AddControllerYawInput(Axis.X);
		AddControllerPitchInput(Axis.Y);
	}
}

void ACL9Character::RunInputStart()
{
	GetCL9CharacterMovement()->SetWantsToRun(true);
}

void ACL9Character::RunInputEnd()
{
	GetCL9CharacterMovement()->SetWantsToRun(false);
}

void ACL9Character::PickPlaceInput()
{
	Server_PickPlace();
}

void ACL9Character::ThrowStartInput()
{
	Server_ThrowChargeStart();
}

void ACL9Character::ThrowEndInput()
{
	Server_ThrowRelease();
}

void ACL9Character::SwapInput()
{
	Server_Swap();
}

AActor* ACL9Character::ResolveFocus() const
{
	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr)
	{
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9Focus), false, this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity,
		ObjectParams, FCollisionShape::MakeSphere(Rules->PickRadius), QueryParams);

	const FVector AimDirection = GetControlRotation().Vector();
	const bool bHandsBusy = HeldItem != nullptr || PullingCart != nullptr;
	AActor* BestItem = nullptr;
	float BestItemDot = -2.f;
	AActor* BestInteractable = nullptr;
	float BestInteractableDot = -2.f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		// The pulled cart stays a valid target — aiming it releases or loads.
		if (Actor == nullptr || Actor == HeldItem)
		{
			continue;
		}
		const float AimDot = FVector::DotProduct(AimDirection, (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal());
		if (AimDot < FocusMinAimDot)
		{
			continue;
		}

		if (ACL9Item* Item = Cast<ACL9Item>(Actor); Item != nullptr && Item->CanBePickedUp())
		{
			if (!bHandsBusy && AimDot > BestItemDot)
			{
				BestItem = Actor;
				BestItemDot = AimDot;
			}
			// An item never doubles as an interactable candidate unless it
			// implements the interface (the cart does).
			if (Cast<ICL9Interactable>(Actor) == nullptr)
			{
				continue;
			}
		}
		if (ICL9Interactable* Interactable = Cast<ICL9Interactable>(Actor); Interactable != nullptr && Interactable->CanInteract(this))
		{
			if (AimDot > BestInteractableDot)
			{
				BestInteractable = Actor;
				BestInteractableDot = AimDot;
			}
		}
	}

	return BestItem != nullptr ? BestItem : BestInteractable;
}

void ACL9Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Aim highlight is purely local presentation — the same resolver runs on
	// the host when the key lands, so the outline shows exactly what E will do.
	if (!IsLocallyControlled())
	{
		return;
	}

	AActor* NewFocus = ResolveFocus();
	if (NewFocus != FocusedActor.Get())
	{
		if (AActor* OldFocus = FocusedActor.Get())
		{
			SetFocusHighlight(OldFocus, false);
		}
		if (NewFocus != nullptr)
		{
			SetFocusHighlight(NewFocus, true);
		}
		FocusedActor = NewFocus;
	}
}

void ACL9Character::SetFocusHighlight(AActor* Target, bool bOn) const
{
	if (UStaticMeshComponent* TargetMesh = Target->FindComponentByClass<UStaticMeshComponent>())
	{
		TargetMesh->SetOverlayMaterial(bOn ? HighlightOverlayMaterial.Get() : nullptr);
	}
}

void ACL9Character::Server_PickPlace_Implementation()
{
	const UCL9RulesDef* Rules = GetRules();
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (Rules == nullptr || CL9PlayerState == nullptr)
	{
		return;
	}
	if (CL9PlayerState->IsSwapLocked(GetServerTime()))
	{
		return;
	}

	// One key, one target: whatever the aim resolves to — the host re-runs
	// the same resolver and never trusts the client (docs/systems/input.md).
	AActor* Focus = ResolveFocus();

	if (ACL9Item* Item = Cast<ACL9Item>(Focus); Item != nullptr && Item->CanBePickedUp()
		&& HeldItem == nullptr && PullingCart == nullptr)
	{
		// Taking an item off a cart bed is the same press as off the ground.
		if (ACL9Cart* BedCart = Cast<ACL9Cart>(Item->GetAttachParentActor()))
		{
			BedCart->RemoveLoadedItem(Item);
		}
		HostPickUp(Item, Rules);
		return;
	}
	if (ICL9Interactable* Interactable = Cast<ICL9Interactable>(Focus); Interactable != nullptr && Interactable->CanInteract(this))
	{
		Interactable->Interact(this);
		return;
	}

	// Nothing aimed: a full hand sets its item down, a bare hand lets go of
	// the cart (aiming the trailing cart releases it too).
	if (HeldItem != nullptr)
	{
		HostPlaceHeld(Rules);
	}
	else if (PullingCart != nullptr)
	{
		PullingCart->StopPull(this);
	}
}

void ACL9Character::Server_ThrowChargeStart_Implementation()
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (HeldItem == nullptr || CL9PlayerState == nullptr || CL9PlayerState->IsSwapLocked(GetServerTime()))
	{
		return;
	}
	ThrowChargeStartTime = GetServerTime();
}

void ACL9Character::Server_ThrowRelease_Implementation()
{
	const UCL9RulesDef* Rules = GetRules();
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (Rules == nullptr || CL9PlayerState == nullptr || HeldItem == nullptr || ThrowChargeStartTime < 0.f)
	{
		return;
	}

	// The host measures the charge on its own clock — client-sent charge
	// amounts never enter the ruling (docs/systems/input.md 던지기 차징).
	const float Charge = FMath::Clamp((GetServerTime() - ThrowChargeStartTime) / Rules->ThrowChargeTime, 0.f, 1.f);
	ThrowChargeStartTime = -1.f;
	const float Speed = FMath::Lerp(Rules->ThrowMinSpeed, Rules->ThrowMaxSpeed, Charge);

	const FVector Direction = GetControlRotation().Vector();
	const float EyeOffset = CharacterDef ? CharacterDef->EyeHeight - CharacterDef->CapsuleHalfHeight : 0.f;
	const float Clearance = GetCapsuleComponent()->GetScaledCapsuleRadius() + (HeldItem->GetItemDef() ? HeldItem->GetItemDef()->Radius : 20.f) + 10.f;
	const FVector Origin = GetActorLocation() + FVector(0.f, 0.f, EyeOffset) + Direction * Clearance;

	ACL9Item* Thrown = HeldItem;
	HeldItem = nullptr;
	CL9PlayerState->HandItem = FCL9CarriedItem();
	Thrown->SetOwner(nullptr);
	Thrown->OnThrown(Origin, Direction * Speed);
}

void ACL9Character::Server_Swap_Implementation()
{
	const UCL9RulesDef* Rules = GetRules();
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (Rules == nullptr || CL9PlayerState == nullptr)
	{
		return;
	}
	const float Now = GetServerTime();
	if (CL9PlayerState->IsSwapLocked(Now))
	{
		return;
	}

	// Hands are exclusive: pulling a cart occupies them, so no pocket juggling
	// mid-pull (2026-09-02 상호작용 통합).
	if (PullingCart != nullptr)
	{
		return;
	}

	const bool bHandFull = HeldItem != nullptr;
	if (bHandFull)
	{
		const UCL9ItemDef* HeldDef = HeldItem->GetItemDef();
		if (HeldDef == nullptr || HeldDef->CarrySlot != ECL9CarrySlot::Pocketable)
		{
			return;
		}
	}

	FCL9CarriedItem Outgoing;
	const bool bTakeOut = CL9PlayerState->PocketItems.Num() > 0;
	if (!bHandFull && !bTakeOut)
	{
		return;
	}
	if (bTakeOut)
	{
		Outgoing = CL9PlayerState->PocketItems.Pop();
	}

	if (bHandFull)
	{
		// Pocketed items exist only as data — the actor dies here and respawns
		// on the way out (docs/systems/inventory.md, 2026-09-01 확정).
		FCL9CarriedItem& Pocketed = CL9PlayerState->PocketItems.AddDefaulted_GetRef();
		Pocketed.ItemAlias = HeldItem->GetAlias();
		Pocketed.ItemSubAlias = HeldItem->GetSubAlias();
		HeldItem->Destroy();
		HeldItem = nullptr;
		CL9PlayerState->HandItem = FCL9CarriedItem();
	}

	if (bTakeOut)
	{
		if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
		{
			const FTransform HandTransform(GetActorLocation() + FVector(0.f, 0.f, Rules->CarryHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
			if (ACL9Item* TakenOut = GameMode->SpawnItem(Outgoing.ItemAlias, Outgoing.ItemSubAlias, HandTransform))
			{
				HostPickUp(TakenOut, Rules);
			}
		}
	}

	CL9PlayerState->SwapLockUntil = Now + Rules->SwapTime;
}

void ACL9Character::HostPickUp(ACL9Item* Item, const UCL9RulesDef* Rules)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (CL9PlayerState == nullptr || Item->GetItemDef() == nullptr)
	{
		return;
	}

	CL9PlayerState->HandItem.ItemAlias = Item->GetAlias();
	CL9PlayerState->HandItem.ItemSubAlias = Item->GetSubAlias();
	HeldItem = Item;
	Item->SetOwner(this);

	const float ForwardOffset = GetCapsuleComponent()->GetScaledCapsuleRadius() + Item->GetItemDef()->Radius + 15.f;
	const float HandZ = Rules->CarryHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	Item->OnPickedUp(this, FVector(ForwardOffset, 0.f, HandZ));
}

void ACL9Character::HostPlaceHeld(const UCL9RulesDef* Rules)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (CL9PlayerState == nullptr || HeldItem == nullptr)
	{
		return;
	}

	const float ItemRadius = HeldItem->GetItemDef() ? HeldItem->GetItemDef()->Radius : 20.f;
	const float ForwardOffset = GetCapsuleComponent()->GetScaledCapsuleRadius() + ItemRadius + 25.f;
	const float HandZ = Rules->CarryHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector DropLocation = GetActorLocation() + GetActorForwardVector() * ForwardOffset + FVector(0.f, 0.f, HandZ);

	ACL9Item* Placed = HeldItem;
	HeldItem = nullptr;
	CL9PlayerState->HandItem = FCL9CarriedItem();
	Placed->SetOwner(nullptr);
	Placed->OnPlaced(DropLocation);
}

void ACL9Character::SetPullingCart(ACL9Cart* Cart)
{
	if (!HasAuthority())
	{
		return;
	}
	PullingCart = Cart;
}

void ACL9Character::HostConsumeHeldItem()
{
	if (!HasAuthority() || HeldItem == nullptr)
	{
		return;
	}

	HeldItem->Destroy();
	HeldItem = nullptr;
	if (ACL9PlayerState* CL9PlayerState = GetCL9PlayerState())
	{
		CL9PlayerState->HandItem = FCL9CarriedItem();
	}
}

ACL9Item* ACL9Character::HostTakeHeldItem()
{
	if (!HasAuthority() || HeldItem == nullptr)
	{
		return nullptr;
	}

	ACL9Item* Taken = HeldItem;
	HeldItem = nullptr;
	Taken->SetOwner(nullptr);
	if (ACL9PlayerState* CL9PlayerState = GetCL9PlayerState())
	{
		CL9PlayerState->HandItem = FCL9CarriedItem();
	}
	return Taken;
}

const UCL9RulesDef* ACL9Character::GetRules() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetRules() : nullptr;
}

ACL9PlayerState* ACL9Character::GetCL9PlayerState() const
{
	return GetPlayerState<ACL9PlayerState>();
}

float ACL9Character::GetServerTime() const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}
