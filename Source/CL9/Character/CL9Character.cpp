// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/CL9Character.h"
#include "Internationalization/StringTableRegistry.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/CL9CharacterMovement.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "TimerManager.h"
#include "CL9.h"
#include "Core/CL9InputText.h"
#include "Core/CL9Interactable.h"
#include "Data/CL9BuffDef.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9Constants.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9ShakeTable.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"
#include "GameMode/CL9GameUserSettings.h"
#include "GameMode/CL9PlayerController.h"
#include "GameMode/CL9PlayerState.h"
#include "Items/CL9Cart.h"
#include "Items/CL9Item.h"
#include "Items/CL9FlashItem.h"
#include "Monsters/CL9Monster.h"

namespace
{
	/** focus 후보가 되려면 최소한 이만큼은 시야 안(조준 방향과의 내적)에 들어와 있어야 한다 */
	constexpr float FocusMinAimDot = 0.3f;

	/** 휠 전환 사이 최소 간격, 초 단위 — Axis Triggered가 겹쳐 와도 한 노치에 한 칸만 넘긴다 (입력 처리 상수, 밸런스 값이 아니다) */
	constexpr float SlotScrollDebounce = 0.1f;

	/** 복제되는 "서버 시각" 필드의 비어 있음 값 — 서버 시각은 0 이상이라 음수면 "아직 없음/대기 중 아님"이다 */
	constexpr float ServerTimeNever = -1000.f;
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

	// 1인칭 팔은 카메라의 자식(뷰모델)이다 — 시선 피치·요를 통째로 따라간다(리썰 방식, 2026-09-13).
	// 상대 트랜스폼은 전신 마네킹과 같은 규격: 리그 root가 원점, 눈이 (0,0,EyeHeight)에서 +X를 보는
	// 공간 — 카메라가 눈이므로 root를 눈높이만큼 내리고 -90° 요를 준다. 팔 클립은 이 공간에서 저작한다.
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -CL9::EyeHeight), FRotator(0.f, -90.f, 0.f));
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
	DOREPLIFETIME(ACL9Character, SwingStartServerTime);
	DOREPLIFETIME(ACL9Character, SwingWindupServerTime);
	DOREPLIFETIME(ACL9Character, EatStartServerTime);
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
	// 팔(카메라 자식)의 root도 행의 눈높이만큼 내린다 — BP가 생성자 값을 덮어써도 여기서 다시 맞는다.
	FirstPersonMesh->SetRelativeLocation(FVector(0.f, 0.f, -Def->EyeHeight));

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
		EnhancedInput->BindAction(DropAction, ETriggerEvent::Started, this, &ACL9Character::DropStartInput);
		EnhancedInput->BindAction(DropAction, ETriggerEvent::Completed, this, &ACL9Character::DropEndInput);
		// 숫자키 — 배열 순서가 칸 번호다. 휠은 Axis1D라 값이 있는 프레임마다 Triggered가 온다
		for (int32 Slot = 0; Slot < SlotActions.Num(); ++Slot)
		{
			if (SlotActions[Slot] != nullptr)
			{
				EnhancedInput->BindAction(SlotActions[Slot], ETriggerEvent::Started, this, &ACL9Character::SlotInput, Slot);
			}
		}
		EnhancedInput->BindAction(SlotScrollAction, ETriggerEvent::Triggered, this, &ACL9Character::SlotScrollInput);
		EnhancedInput->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ACL9Character::FlashlightInput);
		EnhancedInput->BindAction(UseAction, ETriggerEvent::Started, this, &ACL9Character::UseStartInput);
		EnhancedInput->BindAction(UseAction, ETriggerEvent::Completed, this, &ACL9Character::UseEndInput);
		// 우클은 탭(스캔)/길게(먹기)를 캐릭터가 가른다 — PC는 RequestScan()만 내준다 (2026-09-13)
		if (ScanAction != nullptr)
		{
			EnhancedInput->BindAction(ScanAction, ETriggerEvent::Started, this, &ACL9Character::ScanStartInput);
			EnhancedInput->BindAction(ScanAction, ETriggerEvent::Completed, this, &ACL9Character::ScanEndInput);
		}
		else
		{
			UE_LOG(LogCL9, Error, TEXT("'%s': ScanAction is not set — right-click scan and eat are dead (BP_CL9Character.ScanAction → IA_Scan)."), *GetNameSafe(this));
		}

		// 프롬프트의 "[{0}]"에 들어갈 줍기 키 — 빙의 시 한 번 읽고, 이후는 리바인딩 변경 델리게이트가 갱신한다
		RefreshKeyText();
		if (UEnhancedInputUserSettings* UserSettings = CL9Input::GetUserSettings(Cast<APlayerController>(GetController())))
		{
			UserSettings->OnSettingsChanged.AddUniqueDynamic(this, &ACL9Character::HandleUserSettingsChanged);
			UserSettings->OnMappingContextRegistered.AddUniqueDynamic(this, &ACL9Character::HandleMappingContextRegistered);
		}
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
		// 감도·Y축 반전은 로컬 옵션이다 — AxisConfig 감도 위에 곱하는 배율일 뿐 조준 이득이 없다 (docs/systems/settings.md)
		float Sensitivity = 1.f;
		float PitchSign = 1.f;
		if (const UCL9GameUserSettings* Settings = Cast<UCL9GameUserSettings>(GEngine->GetGameUserSettings()))
		{
			Sensitivity = Settings->MouseSensitivity;
			PitchSign = Settings->bInvertY ? -1.f : 1.f;
		}
		AddControllerYawInput(Axis.X * Sensitivity);
		AddControllerPitchInput(Axis.Y * Sensitivity * PitchSign);
	}
}

void ACL9Character::RunInputStart()
{
	bRunHeld = true;
	GetCL9CharacterMovement()->SetWantsToRun(true);
}

void ACL9Character::RunInputEnd()
{
	bRunHeld = false;
	GetCL9CharacterMovement()->SetWantsToRun(false);
}

void ACL9Character::PickPlaceInput()
{
	Server_PickPlace();
}

void ACL9Character::DropStartInput()
{
	// 활성 칸이 비었으면 놓을 것도 던질 것도 없다 — RPC를 보내지 않는다
	if (GetHeldItem() == nullptr)
	{
		return;
	}

	// 달리기 키 + 놓기 키 = 던지기 차징. 조합은 고정이고 놓기 키만 리바인딩된다 (docs/systems/input.md 던지기 차징).
	// 차징 시각은 호스트가 잰다 — 로컬 시각은 HUD 게이지용이다.
	// 같은 틱에 두 키가 함께 눌리면 Enhanced Input이 Started 델리게이트를 바인딩 역순으로 쏘아 이 함수가 RunInputStart보다
	// 먼저 온다(bRunHeld가 아직 false) — 액션 값은 델리게이트 전에 전부 갱신돼 있으므로 달리기 액션 값을 직접 읽어 보완한다
	// (2026-09-14 리드 재현: 같은 틱 Run+Drop 주입이 차징되지 않음).
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	const UEnhancedPlayerInput* PlayerInput = PlayerController ? Cast<UEnhancedPlayerInput>(PlayerController->PlayerInput) : nullptr;
	const bool bRunPressedThisTick = PlayerInput != nullptr && RunAction != nullptr && PlayerInput->GetActionValue(RunAction).Get<bool>();
	if (bRunHeld || bRunPressedThisTick)
	{
		LocalThrowChargeStartTime = GetWorld()->GetTimeSeconds();
		Server_ThrowChargeStart();
		return;
	}
	Server_Drop();
}

void ACL9Character::DropEndInput()
{
	// 차징 없이 눌렀던 G(즉시 놓기)의 뗌은 아무 일도 아니다
	if (LocalThrowChargeStartTime < 0.f)
	{
		return;
	}
	LocalThrowChargeStartTime = -1.f;
	Server_ThrowRelease();
}

void ACL9Character::SlotInput(int32 Slot)
{
	// 잠금 판정은 호스트가 다시 한다 — 여기서는 헛 RPC만 거른다
	const ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (CL9PlayerState == nullptr || !CL9PlayerState->Slots.IsValidIndex(Slot) || !CanSwitchSlot())
	{
		return;
	}
	Server_SelectSlot(static_cast<uint8>(Slot));
}

void ACL9Character::SlotScrollInput(const FInputActionValue& Value)
{
	// 마우스 휠 축: 한 노치가 ±1.0이고 위가 +다. 아래(−) = 다음 칸(오른쪽), 위(+) = 이전 칸.
	const float Axis = Value.Get<float>();
	const ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	const int32 SlotCount = CL9PlayerState ? CL9PlayerState->Slots.Num() : 0;
	if (FMath::IsNearlyZero(Axis) || SlotCount <= 1 || !CanSwitchSlot())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (LastSlotScrollTime >= 0.f && Now - LastSlotScrollTime < SlotScrollDebounce)
	{
		return;
	}
	LastSlotScrollTime = Now;
	Server_ScrollSlot(Axis < 0.f ? 1 : -1);
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
	const FText Pattern = GetFocusPromptPattern();
	return Pattern.IsEmpty() ? Pattern : FText::Format(Pattern, PickPlaceKeyText);
}

void ACL9Character::RefreshKeyText()
{
	PickPlaceKeyText = CL9Input::GetActionKeyText(Cast<APlayerController>(GetController()), PickPlaceAction);
	// 같은 패턴이라도 키가 바뀌었으면 HUD가 다시 그려야 한다 — 다음 틱의 비교가 반드시 달라지게 비운다
	LastFocusPrompt = FText::GetEmpty();
}

void ACL9Character::HandleUserSettingsChanged(UEnhancedInputUserSettings* Settings)
{
	RefreshKeyText();
}

void ACL9Character::HandleMappingContextRegistered(const UInputMappingContext* Context)
{
	RefreshKeyText();
}

FText ACL9Character::GetFocusPromptPattern() const
{
	// Server_PickPlace와 같은 순서: 착용 가능 > 주울 수 있는 아이템 > 상호작용 대상 > 미는 수레 놓기.
	// "놓기" 프롬프트는 없다 — 놓기는 E가 아니라 G이고, 들고만 있을 때는 프롬프트가 비어 있다 (2026-09-13 사용자).
	AActor* Focus = FocusedActor.Get();
	if (ACL9Item* Wearable = Cast<ACL9Item>(Focus); Wearable != nullptr && Wearable->CanBePickedUp()
		&& Wearable->GetItemDef()->CarrySlot == ECL9CarrySlot::Head && WornItem == nullptr)
	{
		return LOCTABLE("CL9", "Prompt.Wear");
	}
	if (ACL9Item* Item = Cast<ACL9Item>(Focus); Item != nullptr && Item->CanBePickedUp()
		&& Item->GetItemDef()->CarrySlot != ECL9CarrySlot::Head && CanPickUpItem())
	{
		return LOCTABLE("CL9", "Prompt.PickUp");
	}
	if (ICL9Interactable* Interactable = Cast<ICL9Interactable>(Focus); Interactable != nullptr && Interactable->CanInteract(this))
	{
		return Interactable->GetPrompt(this);
	}
	if (Focus == nullptr && PullingCart != nullptr)
	{
		return LOCTABLE("CL9", "Prompt.CartRelease");
	}
	return FText::GetEmpty();
}

void ACL9Character::FlashlightInput()
{
	Server_Flashlight();
}

void ACL9Character::Server_Flashlight_Implementation()
{
	// F는 쓴 헤드램프만 — 든 손전등은 좌클(Server_UseStart)이 켠다 (2026-09-12). 정전 이후에는
	// 손전등 자체가 켜지길 거부한다 (docs/systems/light.md).
	if (ACL9FlashItem* Lamp = Cast<ACL9FlashItem>(WornItem.Get()))
	{
		Lamp->ToggleLight();
	}
}

void ACL9Character::UseStartInput()
{
	// 누름·뗌을 그대로 호스트에 넘긴다 — 무엇을 할지(대기·토글·무동작)는 호스트가 든 행으로 정한다
	Server_UseStart();
}

void ACL9Character::UseEndInput()
{
	// 대기 중이었는지는 호스트만 안다(칸 전환으로 취소됐을 수 있다) — 뗌은 항상 보낸다
	Server_UseRelease();
}

void ACL9Character::ScanStartInput()
{
	// 문턱은 규칙 표의 UseHoldTime — 이 시간 안에 떼면 탭(스캔), 넘기면 먹기. 표가 없으면 타이머가 안 걸려 늘 탭이다
	bEatHoldReached = false;
	const UCL9RulesDef* Rules = GetRules();
	if (Rules != nullptr && Rules->UseHoldTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(EatHoldTimer, this, &ACL9Character::EatHoldReached, Rules->UseHoldTime, false);
	}
}

void ACL9Character::ScanEndInput()
{
	GetWorldTimerManager().ClearTimer(EatHoldTimer);
	if (bEatHoldReached)
	{
		return;
	}
	// 탭 — 스캔은 표현이라 PC(HUD 응답)가 맡는다. 문턱까지 기다렸다 떼는 순간 나가므로 최대 UseHoldTime만큼 늦을 수 있다
	if (ACL9PlayerController* PC = Cast<ACL9PlayerController>(GetController()))
	{
		PC->RequestScan();
	}
}

void ACL9Character::EatHoldReached()
{
	bEatHoldReached = true;
	// 활성 칸이 Dish가 아니면 호스트가 조용히 무시한다 — 여기서 걸러도 되지만 판정은 호스트 것이다
	if (GetHeldItem() != nullptr)
	{
		Server_Eat();
	}
}

void ACL9Character::Server_UseStart_Implementation()
{
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	UE_LOG(LogCL9, Verbose, TEXT("UseStart: item=%s action=%d"), *GetNameSafe(HeldItem.Get()), Def ? static_cast<int32>(Def->UseAction) : -1);
	if (Def == nullptr || IsKnockedDown())
	{
		return;
	}

	switch (Def->UseAction)
	{
	case ECL9UseAction::ToggleLight:
		// 손전등은 대기가 없다 — 누르는 순간 켜고 끈다 (2026-09-13 기본값)
		if (ACL9FlashItem* Lamp = Cast<ACL9FlashItem>(HeldItem.Get()))
		{
			Lamp->ToggleLight();
		}
		break;
	case ECL9UseAction::Swing:
		// 공격 대기 — 무한이고 떼는 순간 휘두른다. 대기 시간은 결과에 영향이 없다 (리썰 삽 방식, 밸런스 열 없음)
		SwingWindupServerTime = GetServerTime();
		break;
	default:
		break;
	}
}

void ACL9Character::Server_UseRelease_Implementation()
{
	// 대기 중이 아니면(애초에 Swing이 아니었거나 칸 전환·놓기로 취소됐다) 뗌은 아무 일도 아니다
	if (SwingWindupServerTime < 0.f)
	{
		return;
	}
	HostCancelSwingWindup();

	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	if (Def == nullptr || Def->UseAction != ECL9UseAction::Swing || IsKnockedDown())
	{
		return;
	}
	HostSwing(*Def);
}

void ACL9Character::Server_Eat_Implementation()
{
	// 먹을 수 있는 건 Dish뿐이고 별도 플래그는 없다 (plan/data/item/common.md). 프라이팬을 들고 우클을 오래
	// 누르는 건 흔한 일이라 거절 로그는 남기지 않는다 — HostEat의 "refused" 로그는 치트 경로 몫이다.
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	if (Def == nullptr || Def->ItemType != ECL9ItemType::Dish)
	{
		return;
	}
	HostEat();
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
	const bool bCanPickUp = CanPickUpItem();
	AActor* BestItem = nullptr;
	float BestItemDot = -2.f;
	AActor* BestInteractable = nullptr;
	float BestInteractableDot = -2.f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		// 밀고 있는 수레도 유효한 대상이다 — 그것을 조준하면 놓는다.
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
			// Head 슬롯 아이템은 머리에 착용하므로 퀵슬롯이 꽉 차거나 잠겨 있어도 상관없다 — 대신 머리가 비어 있어야 한다
			const bool bWearable = Item->GetItemDef()->CarrySlot == ECL9CarrySlot::Head;
			const bool bCanTake = bWearable ? (WornItem == nullptr) : bCanPickUp;
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

	// 대상이 바뀌지 않아도 프롬프트는 바뀔 수 있으므로(카트: 밀기 → 놓기)
	// 원문 패턴을 비교해서 달라졌을 때만 HUD에 알린다 — 서식은 HUD가 GetFocusPrompt를 읽을 때 한 번 한다.
	const FText Prompt = GetFocusPromptPattern();
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
	// 키 하나에 대상 하나: 조준이 가리키는 게 무엇이든 — 호스트는 같은
	// resolver를 다시 실행할 뿐 클라이언트를 신뢰하지 않는다 (docs/systems/input.md).
	AActor* Focus = ResolveFocus();

	// Head 슬롯 아이템은 드는 게 아니라 착용한다 — 같은 키 입력이지만 손은 그대로 둔다
	if (ACL9Item* Wearable = Cast<ACL9Item>(Focus); Wearable != nullptr && Wearable->CanBePickedUp()
		&& Wearable->GetItemDef()->CarrySlot == ECL9CarrySlot::Head && WornItem == nullptr)
	{
		HostWear(Wearable);
		return;
	}

	// 수레 짐칸 안의 아이템도 바닥의 것과 똑같이 여기로 온다 — 짐칸의 짐은 부착이 아니라 짐칸 위에 놓인 자유 아이템이다.
	if (ACL9Item* Item = Cast<ACL9Item>(Focus); Item != nullptr && Item->CanBePickedUp()
		&& Item->GetItemDef()->CarrySlot != ECL9CarrySlot::Head && CanPickUpItem())
	{
		// 활성 칸이 비었으면 거기, 아니면 첫 빈 칸 — 그리고 그 칸이 활성이 된다 (리썰 규칙)
		HostPickUp(Item, FindPickUpSlot());
		return;
	}
	if (ICL9Interactable* Interactable = Cast<ICL9Interactable>(Focus); Interactable != nullptr && Interactable->CanInteract(this))
	{
		Interactable->Interact(this);
		return;
	}

	// 아무것도 조준하지 않음: 끌던 카트를 놓는다 (뒤따르는 카트를 조준해도 마찬가지로 놓는다).
	// 손의 물건을 놓는 것은 E가 아니라 G(Server_Drop)다.
	if (PullingCart != nullptr)
	{
		PullingCart->StopPull(this);
	}
}

void ACL9Character::Server_ThrowChargeStart_Implementation()
{
	if (HeldItem == nullptr)
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

	// 던진 칸은 비고 활성 칸 번호는 그대로다 — 다른 칸으로 자동 전환하지 않는다
	HostCancelSwingWindup();
	ACL9Item* Thrown = HeldItem;
	HeldItem = nullptr;
	HostClearActiveSlot();
	Thrown->SetOwner(nullptr);
	Thrown->OnThrown(Origin, Direction * Speed);
}

void ACL9Character::Server_SelectSlot_Implementation(uint8 Slot)
{
	const UCL9RulesDef* Rules = GetRules();
	const ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (Rules == nullptr || CL9PlayerState == nullptr || !CL9PlayerState->Slots.IsValidIndex(Slot)
		|| Slot == CL9PlayerState->ActiveSlot || !CanSwitchSlot())
	{
		return;
	}
	HostSelectSlot(Slot, Rules);
}

void ACL9Character::Server_ScrollSlot_Implementation(int32 Direction)
{
	const UCL9RulesDef* Rules = GetRules();
	const ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	const int32 SlotCount = CL9PlayerState ? CL9PlayerState->Slots.Num() : 0;
	if (Rules == nullptr || SlotCount <= 1 || Direction == 0 || !CanSwitchSlot())
	{
		return;
	}
	// 양 끝은 순환한다 — 호스트의 ActiveSlot에서 한 칸만 옮기므로 클라 사본이 늦어도 노치마다 한 칸씩 간다
	const int32 Step = Direction < 0 ? -1 : 1;
	HostSelectSlot((static_cast<int32>(CL9PlayerState->ActiveSlot) + Step + SlotCount) % SlotCount, Rules);
}

void ACL9Character::Server_Drop_Implementation()
{
	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr || HeldItem == nullptr)
	{
		return;
	}

	// 옛 E 놓기 규칙 그대로: 캡슐 앞, 손 높이(CarryHeight)에 살며시. 놓은 칸은 비고 활성 칸 번호는 그대로다.
	const float ItemRadius = HeldItem->GetItemDef() ? HeldItem->GetItemDef()->Radius : 20.f;
	const float ForwardOffset = GetCapsuleComponent()->GetScaledCapsuleRadius() + ItemRadius + 25.f;
	const float HandZ = Rules->CarryHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector DropLocation = GetActorLocation() + GetActorForwardVector() * ForwardOffset + FVector(0.f, 0.f, HandZ);

	ThrowChargeStartTime = -1.f;
	HostCancelSwingWindup();
	ACL9Item* Placed = HeldItem;
	HeldItem = nullptr;
	HostClearActiveSlot();
	Placed->SetOwner(nullptr);
	Placed->OnPlaced(DropLocation);
}

int32 ACL9Character::GetActiveSlot() const
{
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	return CL9PlayerState != nullptr ? CL9PlayerState->ActiveSlot : 0;
}

bool ACL9Character::CanSwitchSlot() const
{
	// 수레는 두 손으로 민다 — 미는 동안은 칸을 뒤적일 수 없다. 두 손 아이템(냄비·작업등, HoldPose Large)도
	// 같다: 리썰의 twoHanded 규칙 — 들고 있는 동안 전환·줍기 불가 (docs/systems/inventory.md 잠금).
	if (PullingCart != nullptr)
	{
		return false;
	}
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	return Def == nullptr || Def->HoldPose != ECL9HoldPose::Large;
}

bool ACL9Character::CanPickUpItem() const
{
	// 줍기는 칸 전환을 겸한다(주운 칸이 활성이 된다) — 전환 잠금이 그대로 줍기 잠금이고, 꽉 차면 못 줍는다
	return CanSwitchSlot() && FindPickUpSlot() != INDEX_NONE;
}

int32 ACL9Character::FindPickUpSlot() const
{
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	if (CL9PlayerState == nullptr)
	{
		return INDEX_NONE;
	}
	const int32 Active = CL9PlayerState->ActiveSlot;
	if (CL9PlayerState->Slots.IsValidIndex(Active) && !CL9PlayerState->Slots[Active].IsSet())
	{
		return Active;
	}
	return CL9PlayerState->FindEmptySlot();
}

void ACL9Character::HostSwing(const UCL9ItemDef& Def)
{
	// 사거리·부채꼴·회복 시간은 아직 상수다(행 컬럼은 피해량과 경직 종류뿐 — 2026-09-12 결정);
	// 경직 시간·넉백 거리는 몬스터 행에서 가져온다.
	constexpr float SwingReach = 200.f;
	constexpr float SwingHalfAngleDot = 0.3f;
	constexpr float SwingRecovery = 0.6f;

	const float Now = GetServerTime();
	if (!HasAuthority() || Now < SwingReadyTime)
	{
		return;
	}
	SwingReadyTime = Now + SwingRecovery;
	SwingStartServerTime = Now;

	// 사거리 안 정면에서 가장 가까운 몬스터 (몸체끼리 겹치므로 구체 쿼리를 쓴다)
	const FVector Origin = GetActorLocation();
	const FVector Forward = GetControlRotation().Vector();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CL9Swing), false, this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_WorldDynamic), FCollisionShape::MakeSphere(SwingReach), Params);

	ACL9Monster* Target = nullptr;
	float BestDot = SwingHalfAngleDot;
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

	UE_LOG(LogCL9, Verbose, TEXT("Swing: %d overlaps, target=%s"), Overlaps.Num(), *GetNameSafe(Target));
	if (Target != nullptr)
	{
		Target->OnStrike(Def.HitDamage, Def.HitStun, this);
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

TArray<FName> ACL9Character::GetActiveBuffs() const
{
	// 만료 조건은 GetStatBonus와 글자 그대로 같다 — 다르면 "이름은 떠 있는데 0초"가 난다.
	// 한 버프가 스탯 여럿을 걸면 항목도 여럿이므로 별칭은 한 번만 담는다.
	const float Now = GetServerTime();
	TArray<FName> Aliases;
	for (const FCL9ActiveEffect& Effect : ActiveEffects)
	{
		if (Now < Effect.EndServerTime && !Effect.BuffAlias.IsNone())
		{
			Aliases.AddUnique(Effect.BuffAlias);
		}
	}
	return Aliases;
}

float ACL9Character::GetBuffRemaining(FName BuffAlias) const
{
	// GetStatBonusRemaining과 같은 꼴이다 — 묻는 기준만 스탯에서 버프로 바뀐다.
	const float Now = GetServerTime();
	float LatestEnd = 0.f;
	for (const FCL9ActiveEffect& Effect : ActiveEffects)
	{
		if (Effect.BuffAlias == BuffAlias && Now < Effect.EndServerTime)
		{
			LatestEnd = FMath::Max(LatestEnd, Effect.EndServerTime);
		}
	}
	return LatestEnd > 0.f ? LatestEnd - Now : 0.f;
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

	// 효과 값은 아이템 행이 아니라 버프 표가 갖는다 — 아이템은 별칭으로 가리키기만
	// 한다 (docs/plan/data/buff.md). 없는 별칭은 로드 검사가 이미 깨진 행으로 잡았지만,
	// 그 행은 Find*로 나오지 않으므로 여기서 다시 걸러 준다.
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	for (const FName& BuffAlias : Def->EatBuffs)
	{
		const UCL9BuffDef* Buff = Registry ? Registry->FindBuff(BuffAlias) : nullptr;
		if (Buff == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("Eat: '%s' names buff '%s' which is missing or broken — nothing applied."),
				*Def->Alias.ToString(), *BuffAlias.ToString());
			continue;
		}

		for (const FCL9StatEffect& Wanted : Buff->Effects)
		{
			if (Wanted.DurationSec <= 0.f)
			{
				// 즉시형은 지속 효과로 만들 수 없다 — 지금 범위에 즉시 스탯 변화가 없어 로그만 남긴다
				UE_LOG(LogCL9, Warning, TEXT("Eat: buff '%s' has an instant effect on stat %d — instant stat changes are not implemented."),
					*BuffAlias.ToString(), static_cast<int32>(Wanted.Stat));
				continue;
			}
			FCL9ActiveEffect& Effect = ActiveEffects.AddDefaulted_GetRef();
			Effect.Stat = Wanted.Stat;
			Effect.Amount = Wanted.Amount;
			Effect.EndServerTime = Now + Wanted.DurationSec;
			Effect.BuffAlias = BuffAlias;
			UE_LOG(LogCL9, Log, TEXT("Ate '%s'/'%s': buff '%s' gives stat %d %+.1f for %.0fs."),
				*HeldItem->GetAlias().ToString(), *HeldItem->GetSubAlias().ToString(),
				*BuffAlias.ToString(), static_cast<int32>(Wanted.Stat), Wanted.Amount, Wanted.DurationSec);
		}
	}

	// 먹은 것은 사라진다 — 판정은 여기서 끝이고, 먹기 클립(0.6초)은 이 시각을 복제받은 ABP가 뒤따라 그린다
	EatStartServerTime = Now;
	HostConsumeHeldItem();
}

void ACL9Character::HostStowHeld()
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || HeldItem == nullptr || CL9PlayerState == nullptr)
	{
		return;
	}

	// 비활성 칸은 데이터로만 존재한다 — 액터를 숨겨 두는 대신 파괴하고 다시 꺼낼 때 리스폰한다
	// (복제 비용이 없고 상태가 단순하다, 2026-09-01 확정). 값은 여기서 붙잡아 재스폰 때 되돌린다.
	if (CL9PlayerState->Slots.IsValidIndex(CL9PlayerState->ActiveSlot))
	{
		FCL9CarriedItem& Stowed = CL9PlayerState->Slots[CL9PlayerState->ActiveSlot];
		Stowed.ItemAlias = HeldItem->GetAlias();
		Stowed.ItemSubAlias = HeldItem->GetSubAlias();
		Stowed.Value = HeldItem->GetValue();
	}
	// 차징 중이던 던지기·공격 대기는 대상이 사라진다
	ThrowChargeStartTime = -1.f;
	HostCancelSwingWindup();
	HeldItem->Destroy();
	HeldItem = nullptr;
}

void ACL9Character::HostSelectSlot(int32 Slot, const UCL9RulesDef* Rules)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || Rules == nullptr || !CL9PlayerState->Slots.IsValidIndex(Slot))
	{
		return;
	}

	HostStowHeld();
	CL9PlayerState->ActiveSlot = static_cast<uint8>(Slot);

	// 복사해 둔다 — HostPickUp이 같은 칸을 다시 쓴다
	const FCL9CarriedItem Chosen = CL9PlayerState->Slots[Slot];
	if (Chosen.IsSet())
	{
		if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
		{
			// 손 높이에서 스폰해 곧바로 손 소켓에 붙인다 — 값은 칸에 저장된 것을 그대로 (다시 굴리지 않는다)
			const FTransform HandTransform(GetActorLocation() + FVector(0.f, 0.f, Rules->CarryHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
			if (ACL9Item* TakenOut = GameMode->SpawnItem(Chosen.ItemAlias, Chosen.ItemSubAlias, HandTransform, Chosen.Value))
			{
				// HostPickUp이 갱신 알림을 보낸다
				HostPickUp(TakenOut, Slot);
				return;
			}
		}
		// 행이 사라진 등 스폰 실패 — 유령 데이터를 남기지 않는다 (SpawnItem이 이미 Error 로그를 남겼다)
		CL9PlayerState->Slots[Slot] = FCL9CarriedItem();
	}
	CL9PlayerState->NotifyCarryChanged();
}

void ACL9Character::HostClearActiveSlot()
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr)
	{
		return;
	}
	if (CL9PlayerState->Slots.IsValidIndex(CL9PlayerState->ActiveSlot))
	{
		CL9PlayerState->Slots[CL9PlayerState->ActiveSlot] = FCL9CarriedItem();
	}
	CL9PlayerState->NotifyCarryChanged();
}

float ACL9Character::GetSwingElapsed() const
{
	return GetServerTime() - SwingStartServerTime;
}

float ACL9Character::GetSwingWindupElapsed() const
{
	// 클라의 서버 시각 추정이 조금 앞설 수 있어 0 아래로 내려가지 않게 — "-1 = 대기 아님"이 흐려지지 않는다
	return SwingWindupServerTime < 0.f ? -1.f : FMath::Max(0.f, GetServerTime() - SwingWindupServerTime);
}

float ACL9Character::GetEatElapsed() const
{
	return GetServerTime() - EatStartServerTime;
}

float ACL9Character::GetAimPitch() const
{
	// 전신 ABP(ABP_Unarmed) 전용 — 1인칭 팔은 카메라 자식이라 시선을 저절로 따르므로 이 값을 쓰지 않는다(2026-09-13).
	// 소유자는 컨트롤 회전을 직접 읽는다 — GetBaseAimRotation은 카메라 시점(POV)을 돌려주는데 거기에는 스태미나
	// 셰이크가 섞여 몸이 떨린다. 남의 폰은 GetBaseAimRotation이 호스트에서는 그 컨트롤러의 시점을, 클라에서는
	// 복제된 RemoteViewPitch(바이트 압축)를 준다.
	const float Pitch = IsLocallyControlled() ? GetControlRotation().Pitch : GetBaseAimRotation().Pitch;
	return FMath::Clamp(FRotator::NormalizeAxis(Pitch), -90.f, 90.f);
}

bool ACL9Character::IsRunning() const
{
	const UCL9CharacterMovement* Movement = GetCL9CharacterMovement();
	return Movement != nullptr && Movement->IsRunning();
}

float ACL9Character::GetGroundSpeed() const
{
	return GetVelocity().Size2D();
}

void ACL9Character::HostCancelSwingWindup()
{
	if (HasAuthority())
	{
		SwingWindupServerTime = ServerTimeNever;
	}
}

ECL9HoldPose ACL9Character::GetHoldPose() const
{
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	return Def != nullptr ? Def->HoldPose : ECL9HoldPose::None;
}

void ACL9Character::HostPickUp(ACL9Item* Item, int32 Slot)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || Item == nullptr || Item->GetItemDef() == nullptr
		|| !CL9PlayerState->Slots.IsValidIndex(Slot))
	{
		return;
	}

	// 활성 칸이 차 있는데 다른 칸으로 줍는 경우: 손의 것을 먼저 데이터로 넣는다 — 주운 칸이 활성이 된다 (리썰 규칙)
	if (HeldItem != nullptr)
	{
		HostStowHeld();
	}

	// UI를 위한 미러(값 포함), 월드를 위한 액터 (docs/systems/inventory.md 상태 모델)
	FCL9CarriedItem& Mirror = CL9PlayerState->Slots[Slot];
	Mirror.ItemAlias = Item->GetAlias();
	Mirror.ItemSubAlias = Item->GetSubAlias();
	Mirror.Value = Item->GetValue();
	CL9PlayerState->ActiveSlot = static_cast<uint8>(Slot);
	HeldItem = Item;
	Item->SetOwner(this);

	// 손 소켓에 붙는다 — 위치는 행의 GripOffset이 정하고, CarryHeight는 칸에서 꺼내 스폰하는 높이에만 쓰인다(놓기는 조준점, Server_Drop)
	Item->OnPickedUp(this);
	CL9PlayerState->NotifyCarryChanged();
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

	HostCancelSwingWindup();
	HeldItem->Destroy();
	HeldItem = nullptr;
	HostClearActiveSlot();
}

bool ACL9Character::IsCarryingAnything() const
{
	if (HeldItem != nullptr || PullingCart != nullptr)
	{
		return true;
	}
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	return CL9PlayerState != nullptr && CL9PlayerState->HasAnyCarriedItem();
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

	const bool bHadCarry = (HeldItem != nullptr) || CL9PlayerState->HasAnyCarriedItem();
	const bool bActiveSpilledAsActor = HeldItem != nullptr;

	if (HeldItem != nullptr)
	{
		ACL9Item* Dropped = HeldItem;
		HeldItem = nullptr;
		Dropped->SetOwner(nullptr);
		Spill(Dropped);
	}
	ThrowChargeStartTime = -1.f;
	HostCancelSwingWindup();

	// 네 칸 전부 흘린다 — 비활성 칸은 데이터뿐이라 저장된 값 그대로 다시 스폰해 흘린다. 활성 칸 번호는 그대로 둔다.
	for (int32 Slot = 0; Slot < CL9PlayerState->Slots.Num(); ++Slot)
	{
		FCL9CarriedItem& Stowed = CL9PlayerState->Slots[Slot];
		if (!Stowed.IsSet())
		{
			continue;
		}
		const bool bAlreadySpilled = Slot == CL9PlayerState->ActiveSlot && bActiveSpilledAsActor;
		if (!bAlreadySpilled && GameMode != nullptr)
		{
			if (ACL9Item* Spawned = GameMode->SpawnItem(Stowed.ItemAlias, Stowed.ItemSubAlias, FTransform(GetActorLocation()), Stowed.Value))
			{
				Spill(Spawned);
			}
		}
		Stowed = FCL9CarriedItem();
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
