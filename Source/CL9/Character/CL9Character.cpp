// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/CL9Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
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
#include "GameFramework/PlayerController.h"
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
#include "Data/CL9ShakeTable.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"
#include "GameMode/CL9PlayerState.h"
#include "Items/CL9Cart.h"
#include "Items/CL9Item.h"
#include "Monsters/CL9Monster.h"

namespace
{
	/** focus 후보가 되려면 최소한 이만큼은 시야 안(조준 방향과의 내적)에 들어와 있어야 한다 */
	constexpr float FocusMinAimDot = 0.3f;
}

ACL9Character::ACL9Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCL9CharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(CL9::CapsuleRadius, CL9::CapsuleHalfHeight);

	// 카메라는 눈높이에 위치하며, 1인칭 메시는 소유자에게만 보인다
	// (docs/systems/character.md "시점 — 1인칭").
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, CL9::EyeHeight - CL9::CapsuleHalfHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// 다른 플레이어에게 보이는 것은 전신 메시다. 이것 자체의 트랜스폼은
	// 표준 마네킹 규격을 따른다 (발이 캡슐 바닥에, +X 방향을 바라봄).
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

	// 개인의 HP는 호스트에서 한 번만 굴려진다 (docs/systems/character.md —
	// HP는 항상 PlayerState에 있고 캐릭터에는 없다).
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (CL9PlayerState != nullptr && CL9PlayerState->MaxHp == 0 && CharacterDef != nullptr)
	{
		CL9PlayerState->MaxHp = FMath::RandRange(CharacterDef->HpMin, CharacterDef->HpMax);
		CL9PlayerState->Hp = CL9PlayerState->MaxHp;
		CL9PlayerState->NotifyHpChanged();
	}
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
	DOREPLIFETIME(ACL9Character, KnockdownEndServerTime);
	DOREPLIFETIME(ACL9Character, WornItem);
	DOREPLIFETIME(ACL9Character, ActiveEffects);
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

	// 판정용 수치 — 모든 머신이 동일한 로컬 테이블 행을 적용하므로
	// 자율 프록시는 호스트가 판정할 때 쓰는 것과 같은 수치로 예측한다
	// (conventions § 5).
	GetCapsuleComponent()->SetCapsuleSize(Def->CapsuleRadius, Def->CapsuleHalfHeight);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, Def->EyeHeight - Def->CapsuleHalfHeight));

	UCL9CharacterMovement* Movement = GetCL9CharacterMovement();
	Movement->SetCharacterDef(Def);
	Movement->MaxWalkSpeed = Def->WalkSpeed;
	Movement->JumpZVelocity = Def->JumpVelocity;
	Movement->GravityScale = Def->JumpGravity / CL9::DefaultGravityZ;

	// 표시용 — 메시는 행에서 가져오므로 구매한 에셋을 C++ 없이 교체할 수 있다.
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
		EnhancedInput->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ACL9Character::FlashlightInput);
		EnhancedInput->BindAction(EatAction, ETriggerEvent::Started, this, &ACL9Character::EatInput);
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
	// 빈손이면 밀친다 (④ 장비가 들어오기 전까지 프라이팬 대역); 손에 뭔가 있으면
	// 던지기를 차징한다 (docs/systems/input.md · monster.md 반격).
	if (GetHeldItem() != nullptr)
	{
		LocalThrowChargeStartTime = GetWorld()->GetTimeSeconds();
		Server_ThrowChargeStart();
	}
	else
	{
		Server_Shove();
	}
}

void ACL9Character::ThrowEndInput()
{
	LocalThrowChargeStartTime = -1.f;
	Server_ThrowRelease();
}

float ACL9Character::GetThrowChargeAlpha() const
{
	const UCL9RulesDef* Rules = GetRules();
	if (LocalThrowChargeStartTime < 0.f || Rules == nullptr || Rules->ThrowChargeTime <= 0.f)
	{
		return -1.f;
	}
	return FMath::Clamp((GetWorld()->GetTimeSeconds() - LocalThrowChargeStartTime) / Rules->ThrowChargeTime, 0.f, 1.f);
}

FText ACL9Character::GetFocusPrompt() const
{
	// Server_PickPlace와 같은 순서: 착용 가능 > 주울 수 있는 아이템 > 상호작용 대상 > 놓기/해제
	AActor* Focus = FocusedActor.Get();
	if (ACL9Item* Wearable = Cast<ACL9Item>(Focus); Wearable != nullptr && Wearable->CanBePickedUp()
		&& Wearable->GetItemDef()->CarrySlot == ECL9CarrySlot::Head && WornItem == nullptr)
	{
		return NSLOCTEXT("CL9", "PromptWear", "[E] 착용");
	}
	if (ACL9Item* Item = Cast<ACL9Item>(Focus); Item != nullptr && Item->CanBePickedUp()
		&& Item->GetItemDef()->CarrySlot != ECL9CarrySlot::Head
		&& HeldItem == nullptr && PullingCart == nullptr)
	{
		return NSLOCTEXT("CL9", "PromptPickUp", "[E] 줍기");
	}
	if (ICL9Interactable* Interactable = Cast<ICL9Interactable>(Focus); Interactable != nullptr && Interactable->CanInteract(this))
	{
		return Interactable->GetPrompt(this);
	}
	if (Focus == nullptr && HeldItem != nullptr)
	{
		return NSLOCTEXT("CL9", "PromptPlace", "[E] 놓기");
	}
	if (Focus == nullptr && PullingCart != nullptr)
	{
		return NSLOCTEXT("CL9", "PromptReleaseCart", "[E] 수레 놓기");
	}
	return FText::GetEmpty();
}

void ACL9Character::SwapInput()
{
	Server_Swap();
}

void ACL9Character::FlashlightInput()
{
	Server_Flashlight();
}

void ACL9Character::Server_Flashlight_Implementation()
{
	// 착용한 헤드램프가 F를 우선 받고, 없으면 손에 든 손전등이다. 정전 이후에는
	// 손전등 자체가 켜지길 거부한다 (docs/systems/light.md).
	ACL9Item* Lamp = (WornItem != nullptr && WornItem->IsFlashlight()) ? WornItem.Get()
		: (HeldItem != nullptr && HeldItem->IsFlashlight()) ? HeldItem.Get()
		: nullptr;
	if (Lamp != nullptr)
	{
		Lamp->SetLightSwitch(!Lamp->IsLightOn());
	}
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
		// 끌고 있는 카트도 유효한 대상이다 — 그것을 조준하면 놓거나 싣는다.
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
			// Head 슬롯 아이템은 머리에 착용하므로 손이 막혀 있어도 상관없다 — 대신 머리가 비어 있어야 한다
			const bool bWearable = Item->GetItemDef()->CarrySlot == ECL9CarrySlot::Head;
			const bool bCanTake = bWearable ? (WornItem == nullptr) : !bHandsBusy;
			if (bCanTake && AimDot > BestItemDot)
			{
				BestItem = Actor;
				BestItemDot = AimDot;
			}
			// 아이템은 인터페이스를 구현한 경우(카트가 그렇다)가 아니면
			// 상호작용 후보를 겸하지 않는다.
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

	// 조준 하이라이트는 순전히 로컬 표시용이다 — 키가 눌렸을 때 호스트에서도
	// 같은 resolver가 실행되므로, 윤곽선은 E가 실제로 할 일을 그대로 보여준다.
	if (!IsLocallyControlled())
	{
		return;
	}

	// 1인칭이므로 붙잡힘은 시점이 바닥으로 가라앉는 것으로 표현해야 한다 —
	// 소유자는 자기 몸을 볼 수 없으므로, 넘어짐·죽음은 바디 애니메이션 대신
	// 카메라를 가라앉힌다 (docs/systems/character.md 피격·넘어짐).
	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float EyeZ = CharacterDef ? CharacterDef->EyeHeight - HalfHeight : FirstPersonCameraComponent->GetRelativeLocation().Z;
	const float DownedZ = -HalfHeight + 30.f;
	const bool bDowned = IsKnockedDown() || IsDead();
	FVector CameraRel = FirstPersonCameraComponent->GetRelativeLocation();
	CameraRel.Z = FMath::FInterpTo(CameraRel.Z, bDowned ? DownedZ : EyeZ, DeltaSeconds, 6.f);
	FirstPersonCameraComponent->SetRelativeLocation(CameraRel);

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

	// 대상이 바뀌지 않아도 프롬프트는 바뀔 수 있으므로(카트: 끌기 → 놓기)
	// 텍스트 자체를 비교해서 달라졌을 때만 HUD에 알린다.
	const FText Prompt = GetFocusPrompt();
	if (!Prompt.EqualTo(LastFocusPrompt))
	{
		LastFocusPrompt = Prompt;
		OnFocusPromptChanged.Broadcast();
	}

	UpdateStaminaShake();
}

void ACL9Character::UpdateStaminaShake()
{
	// 스태미나는 게이지가 없다 — 비어갈수록 화면이 흔들린다 (docs/plan/data/rules.md).
	// 소유 클라이언트 전용이며 표시 전용이지 판정이 아니다. 흔들림과 그 임계값은
	// shake 테이블에서 나온다 (rules 컬럼이 아니라 행이므로 새 흔들림은 행 추가로 처리한다).
	APlayerController* PC = Cast<APlayerController>(GetController());
	APlayerCameraManager* CameraManager = PC ? PC->PlayerCameraManager : nullptr;
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9ShakeTable* ShakeTable = GameInstance ? GameInstance->GetShakeTable() : nullptr;
	const FCL9ShakeInfo* Info = ShakeTable ? ShakeTable->Rows.Find(CL9ShakeNames::Stamina) : nullptr;
	if (CameraManager == nullptr || Info == nullptr || Info->StartAt <= 0.f)
	{
		return;
	}

	const float Stamina = GetCL9CharacterMovement()->GetStamina();
	const float Intensity = Stamina < Info->StartAt
		? (Info->StartAt - Stamina) / Info->StartAt
		: 0.f;

	if (Intensity > 0.f)
	{
		if (StaminaShakeInstance == nullptr || StaminaShakeInstance->IsFinished())
		{
			if (UClass* ShakeClass = Info->Shake.LoadSynchronous())
			{
				StaminaShakeInstance = CameraManager->StartCameraShake(ShakeClass, 1.f);
			}
		}
		if (StaminaShakeInstance != nullptr)
		{
			StaminaShakeInstance->ShakeScale = Intensity;
		}
	}
	else if (StaminaShakeInstance != nullptr)
	{
		CameraManager->StopCameraShake(StaminaShakeInstance, false);
		StaminaShakeInstance = nullptr;
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

	// 키 하나에 대상 하나: 조준이 가리키는 게 무엇이든 — 호스트는 같은
	// resolver를 다시 실행할 뿐 클라이언트를 신뢰하지 않는다 (docs/systems/input.md).
	AActor* Focus = ResolveFocus();

	// Head 슬롯 아이템은 드는 게 아니라 착용한다 — 같은 키 입력이지만 손은 그대로 둔다
	if (ACL9Item* Wearable = Cast<ACL9Item>(Focus); Wearable != nullptr && Wearable->CanBePickedUp()
		&& Wearable->GetItemDef()->CarrySlot == ECL9CarrySlot::Head && WornItem == nullptr)
	{
		if (ACL9Cart* BedCart = Cast<ACL9Cart>(Wearable->GetAttachParentActor()))
		{
			BedCart->RemoveLoadedItem(Wearable);
		}
		HostWear(Wearable);
		return;
	}

	if (ACL9Item* Item = Cast<ACL9Item>(Focus); Item != nullptr && Item->CanBePickedUp()
		&& Item->GetItemDef()->CarrySlot != ECL9CarrySlot::Head
		&& HeldItem == nullptr && PullingCart == nullptr)
	{
		// 카트 짐칸에서 아이템을 꺼내는 것도 바닥에서 줍는 것과 같은 키 입력이다.
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

	// 아무것도 조준하지 않음: 손에 뭔가 있으면 내려놓고, 빈손이면 끌던
	// 카트를 놓는다 (뒤따르는 카트를 조준해도 마찬가지로 놓는다).
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

	// 호스트는 자신의 시계로 차징을 측정한다 — 클라이언트가 보낸 차징
	// 값은 판정에 절대 들어가지 않는다 (docs/systems/input.md 던지기 차징).
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
	CL9PlayerState->NotifyCarryChanged();
	Thrown->SetOwner(nullptr);
	Thrown->OnThrown(Origin, Direction * Speed);
}

void ACL9Character::Server_Shove_Implementation()
{
	// 사거리와 회복 시간은 ④에서 실제 장비가 되기 전까지의 '프라이팬 느낌' 상수이며,
	// 기절/넉백 수치는 여전히 몬스터 행에서 가져온다.
	constexpr float ShoveReach = 200.f;
	constexpr float ShoveHalfAngleDot = 0.3f;
	constexpr float ShoveRecovery = 0.6f;

	const float Now = GetServerTime();
	if (IsKnockedDown() || Now < ShoveReadyTime)
	{
		return;
	}
	ShoveReadyTime = Now + ShoveRecovery;

	// 사거리 안 정면에서 가장 가까운 몬스터 (몸체끼리 겹치므로 구체 쿼리를 쓴다)
	const FVector Origin = GetActorLocation();
	const FVector Forward = GetControlRotation().Vector();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CL9Shove), false, this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_WorldDynamic), FCollisionShape::MakeSphere(ShoveReach), Params);

	ACL9Monster* Target = nullptr;
	float BestDot = ShoveHalfAngleDot;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ACL9Monster* Monster = Cast<ACL9Monster>(Overlap.GetActor());
		if (Monster == nullptr)
		{
			continue;
		}
		const FVector ToMonster = (Monster->GetActorLocation() - Origin).GetSafeNormal();
		const float Dot = FVector::DotProduct(Forward, ToMonster);
		if (Dot > BestDot)
		{
			BestDot = Dot;
			Target = Monster;
		}
	}

	if (Target != nullptr)
	{
		Target->OnShove(this);
	}
}

float ACL9Character::GetStatBonus(ECL9Stat Stat) const
{
	// 만료된 것은 세지 않는다 — 목록에서 지우는 일은 호스트가 하지만, 판단은 시각으로 한다.
	// 그래야 복제가 늦게 도착해도 양쪽이 같은 값을 도출한다.
	const float Now = GetServerTime();
	float Bonus = 0.f;
	for (const FCL9ActiveEffect& Effect : ActiveEffects)
	{
		if (Effect.Stat == Stat && Now < Effect.EndServerTime)
		{
			Bonus += Effect.Amount;
		}
	}
	return Bonus;
}

float ACL9Character::GetStatBonusRemaining(ECL9Stat Stat) const
{
	// 만료 조건은 GetStatBonus와 글자 그대로 같아야 한다 — 같은 순회에서 가장 늦게
	// 끝나는 시각을 잡는다. 겹쳐 먹으면 합계가 그때까지 유지되기 때문이다.
	const float Now = GetServerTime();
	float LatestEnd = 0.f;
	for (const FCL9ActiveEffect& Effect : ActiveEffects)
	{
		if (Effect.Stat == Stat && Now < Effect.EndServerTime)
		{
			LatestEnd = FMath::Max(LatestEnd, Effect.EndServerTime);
		}
	}
	return LatestEnd > 0.f ? LatestEnd - Now : 0.f;
}

void ACL9Character::EatInput()
{
	Server_Eat();
}

void ACL9Character::Server_Eat_Implementation()
{
	HostEat();
}

void ACL9Character::HostEat()
{
	if (!HasAuthority() || HeldItem == nullptr || IsKnockedDown())
	{
		return;
	}

	const UCL9ItemDef* Def = HeldItem->GetItemDef();
	if (Def == nullptr)
	{
		return;
	}
	// 먹을 수 있는 것은 Dish뿐이다 — "먹을 수 있나" 플래그를 따로 두지 않고 타입이 가른다
	if (Def->ItemType != ECL9ItemType::Dish)
	{
		UE_LOG(LogCL9, Log, TEXT("Eat refused: '%s'/'%s' is not a dish."),
			*HeldItem->GetAlias().ToString(), *HeldItem->GetSubAlias().ToString());
		return;
	}

	const float Now = GetServerTime();
	// 지나간 효과는 여기서 청소한다 — 목록이 무한정 자라지 않게
	ActiveEffects.RemoveAll([Now](const FCL9ActiveEffect& Effect)
	{
		return Now >= Effect.EndServerTime;
	});

	for (const FCL9StatEffect& Wanted : Def->EatEffects)
	{
		if (Wanted.DurationSec <= 0.f)
		{
			// 즉시형은 지속 효과로 만들 수 없다 — 지금 범위에 즉시 스탯 변화가 없어 로그만 남긴다
			UE_LOG(LogCL9, Warning, TEXT("Eat: '%s' has an instant effect on stat %d — instant stat changes are not implemented."),
				*Def->Alias.ToString(), static_cast<int32>(Wanted.Stat));
			continue;
		}
		FCL9ActiveEffect& Effect = ActiveEffects.AddDefaulted_GetRef();
		Effect.Stat = Wanted.Stat;
		Effect.Amount = Wanted.Amount;
		Effect.EndServerTime = Now + Wanted.DurationSec;
		UE_LOG(LogCL9, Log, TEXT("Ate '%s'/'%s': stat %d %+.1f for %.0fs."),
			*HeldItem->GetAlias().ToString(), *HeldItem->GetSubAlias().ToString(),
			static_cast<int32>(Wanted.Stat), Wanted.Amount, Wanted.DurationSec);
	}

	// 먹은 것은 사라진다
	HostConsumeHeldItem();
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

	// 손은 배타적이다: 카트를 끌고 있으면 손이 그걸로 차 있으므로 끄는 도중에는
	// 주머니를 뒤적일 수 없다 (2026-09-02 상호작용 통합).
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
		// 주머니에 넣은 아이템은 데이터로만 존재한다 — 액터는 여기서 사라지고
		// 다시 꺼낼 때 리스폰된다 (docs/systems/inventory.md, 2026-09-01 확정).
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
	// 주머니에 넣는 경로의 손/주머니 갱신을 커버한다; 꺼내는 경로도 HostPickUp을
	// 통해 다시 호출되지만 무해한 중복 갱신일 뿐이다.
	CL9PlayerState->NotifyCarryChanged();
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
	CL9PlayerState->NotifyCarryChanged();
	HeldItem = Item;
	Item->SetOwner(this);

	const float ForwardOffset = GetCapsuleComponent()->GetScaledCapsuleRadius() + Item->GetItemDef()->Radius + 15.f;
	const float HandZ = Rules->CarryHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	Item->OnPickedUp(this, FVector(ForwardOffset, 0.f, HandZ));
}

void ACL9Character::HostWear(ACL9Item* Item)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || Item == nullptr || Item->GetItemDef() == nullptr || WornItem != nullptr)
	{
		return;
	}

	// UI를 위한 미러, 월드를 위한 액터 (docs/systems/inventory.md 상태 모델)
	CL9PlayerState->HeadItem.ItemAlias = Item->GetAlias();
	CL9PlayerState->HeadItem.ItemSubAlias = Item->GetSubAlias();
	CL9PlayerState->NotifyCarryChanged();
	WornItem = Item;
	Item->SetOwner(this);
	Item->OnWorn(this);
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
	CL9PlayerState->NotifyCarryChanged();
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
		CL9PlayerState->NotifyCarryChanged();
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
		CL9PlayerState->NotifyCarryChanged();
	}
	return Taken;
}

bool ACL9Character::IsCarryingAnything() const
{
	if (HeldItem != nullptr || PullingCart != nullptr)
	{
		return true;
	}
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	return CL9PlayerState != nullptr && (CL9PlayerState->IsHandFull() || CL9PlayerState->PocketItems.Num() > 0);
}

void ACL9Character::OnDamage(int32 Amount, AActor* Source)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || CL9PlayerState->Hp <= 0)
	{
		return;
	}

	CL9PlayerState->Hp = FMath::Max(CL9PlayerState->Hp - Amount, 0);
	CL9PlayerState->NotifyHpChanged();
	HostDropEverything();

	// 쓰러져서 움직일 수 없다; HP 0이면 다음 로비가 되살릴 때까지 쓰러진 채로 있는다
	// (docs/systems/character.md 피격·넘어짐·죽음 — 부활은 없다).
	const float Duration = CL9PlayerState->Hp > 0
		? (CharacterDef ? CharacterDef->KnockdownTime : 1.5f)
		: TNumericLimits<float>::Max();
	KnockdownEndServerTime = GetServerTime() + Duration;

	UE_LOG(LogCL9, Log, TEXT("'%s' caught by '%s': -%d HP → %d%s"),
		*GetNameSafe(this), *GetNameSafe(Source), Amount, CL9PlayerState->Hp,
		CL9PlayerState->Hp <= 0 ? TEXT(" (dead — down for the night)") : TEXT(""));

	// 흐름 판정(전멸, 관전)은 게임 모드의 몫이다 — 호스트에서만 존재하므로 여기서 호출한다
	if (CL9PlayerState->Hp <= 0)
	{
		if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
		{
			GameMode->HandlePlayerDeath(this);
		}
	}
}

void ACL9Character::HostHealFull()
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || CL9PlayerState->MaxHp <= 0)
	{
		return;
	}

	CL9PlayerState->Hp = CL9PlayerState->MaxHp;
	CL9PlayerState->NotifyHpChanged();
}

void ACL9Character::HostRevive()
{
	if (!HasAuthority() || !IsDead())
	{
		return;
	}

	HostHealFull();
	KnockdownEndServerTime = 0.f;
	UE_LOG(LogCL9, Log, TEXT("'%s' revived at the truck."), *GetNameSafe(this));
}

bool ACL9Character::IsKnockedDown() const
{
	return GetServerTime() < KnockdownEndServerTime;
}

bool ACL9Character::IsDead() const
{
	// alias가 적용되어 굴려지기 전까지 MaxHp는 0이다 — 실제로 굴려진 0만 죽음으로 친다
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	return CL9PlayerState != nullptr && CL9PlayerState->IsDead();
}

void ACL9Character::HostDropEverything()
{
	const UCL9RulesDef* Rules = GetRules();
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (Rules == nullptr || CL9PlayerState == nullptr)
	{
		return;
	}

	if (PullingCart != nullptr)
	{
		PullingCart->StopPull(this);
	}

	// 발목 높이에서 옆으로 살짝 밀쳐서 흘린다 — 그다음부터는 실제 물리가 알아서
	// 처리하고, 낮은 원점 덕분에 깨지기 쉬운 아이템도 CrackSpeed 아래에 머물러서
	// 붙잡히면 짐이 다 박살나는 대신 흩어지기만 한다.
	ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>();
	const float FloorZ = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	auto Spill = [&](ACL9Item* Item)
	{
		const float ItemRadius = Item->GetItemDef() ? Item->GetItemDef()->Radius : 20.f;
		const FVector Sideways = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Vector();
		const FVector Origin(GetActorLocation().X, GetActorLocation().Y, FloorZ + ItemRadius + 10.f);
		Item->OnThrown(Origin + Sideways * (GetCapsuleComponent()->GetScaledCapsuleRadius() + ItemRadius + 5.f),
			Sideways * FMath::FRandRange(120.f, 240.f));
	};

	const bool bHadCarry = (HeldItem != nullptr) || CL9PlayerState->PocketItems.Num() > 0;

	if (HeldItem != nullptr)
	{
		ACL9Item* Dropped = HeldItem;
		HeldItem = nullptr;
		CL9PlayerState->HandItem = FCL9CarriedItem();
		Dropped->SetOwner(nullptr);
		Spill(Dropped);
	}

	while (CL9PlayerState->PocketItems.Num() > 0 && GameMode != nullptr)
	{
		const FCL9CarriedItem Pocketed = CL9PlayerState->PocketItems.Pop();
		if (ACL9Item* Spawned = GameMode->SpawnItem(Pocketed.ItemAlias, Pocketed.ItemSubAlias, FTransform(GetActorLocation())))
		{
			Spill(Spawned);
		}
	}

	if (bHadCarry)
	{
		CL9PlayerState->NotifyCarryChanged();
	}
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
