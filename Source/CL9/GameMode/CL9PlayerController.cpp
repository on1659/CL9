// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameMode/CL9CameraManager.h"
#include "GameMode/CL9CheatManager.h"
#include "InputMappingContext.h"

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
	}
}
