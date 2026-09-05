// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameMode/CL9CameraManager.h"
#include "GameMode/CL9CheatManager.h"
#include "InputMappingContext.h"
#include "UI/CL9UIManager.h"
#include "UI/CL9UITable.h"

ACL9PlayerController::ACL9PlayerController()
{
	PlayerCameraManagerClass = ACL9CameraManager::StaticClass();
	CheatClass = UCL9CheatManager::StaticClass();
}

bool ACL9PlayerController::Server_CL9Cheat_Validate(const FString& CommandLine)
{
	return true;
}

void ACL9PlayerController::Server_CL9Cheat_Implementation(const FString& CommandLine)
{
#if !UE_BUILD_SHIPPING
	if (UCL9CheatManager* Cheats = Cast<UCL9CheatManager>(CheatManager))
	{
		Cheats->HandleCommand(CommandLine);
	}
#endif
}

void ACL9PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		if (UCL9UIManager* UIManager = UCL9UIManager::Get(this))
		{
			UIManager->InitForPlayer(this);
		}
	}
}

void ACL9PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Mapping contexts only exist for local players — never on the host's proxies
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* Context : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(Context, 0);
			}
		}

		if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &ACL9PlayerController::PauseInput);
		}
	}
}

void ACL9PlayerController::PauseInput()
{
	UCL9UIManager* UIManager = UCL9UIManager::Get(this);
	if (UIManager == nullptr)
	{
		return;
	}

	const FName OpenScreen = UIManager->GetOpenScreen();
	if (OpenScreen.IsNone())
	{
		UIManager->OpenWidget(CL9UINames::Pause);
	}
	else
	{
		UIManager->CloseWidget(OpenScreen);
	}
}

void ACL9PlayerController::SetGameplayInputEnabled(bool bEnabled)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem == nullptr)
	{
		return;
	}

	for (UInputMappingContext* Context : DefaultMappingContexts)
	{
		if (Context == nullptr)
		{
			continue;
		}
		if (bEnabled)
		{
			if (!Subsystem->HasMappingContext(Context))
			{
				Subsystem->AddMappingContext(Context, 0);
			}
		}
		else
		{
			Subsystem->RemoveMappingContext(Context);
		}
	}
}
