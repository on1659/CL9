// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9PlayerController.h"
#include "CL9.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameMode/CL9CameraManager.h"
#include "GameMode/CL9CheatManager.h"
#include "GameMode/CL9GameMode.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
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
			EnhancedInput->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &ACL9PlayerController::SpectateNext);
		}
	}
}

void ACL9PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9PlayerController, bSpectating);
}

void ACL9PlayerController::PauseInput()
{
	UCL9UIManager* UIManager = UCL9UIManager::Get(this);
	if (UIManager == nullptr)
	{
		return;
	}

	// The spectate screen counts as "nothing open" for M: pausing replaces it
	// (Screen layer is exclusive) and closing the pause brings it back — the
	// screen the spectate flag derives has no "closed" state of its own.
	const FName OpenScreen = UIManager->GetOpenScreen();
	if (OpenScreen.IsNone() || OpenScreen == CL9UINames::Spectate)
	{
		UIManager->OpenWidget(CL9UINames::Pause);
	}
	else
	{
		UIManager->CloseWidget(OpenScreen);
		if (bSpectating)
		{
			UIManager->OpenWidget(CL9UINames::Spectate);
		}
	}
}

void ACL9PlayerController::SpectateNext()
{
	Server_SpectateNext();
}

void ACL9PlayerController::Server_SpectateNext_Implementation()
{
	if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
	{
		GameMode->SpectateNext(this);
	}
}

void ACL9PlayerController::HostSetSpectating(bool bNewSpectating)
{
	if (!HasAuthority() || bSpectating == bNewSpectating)
	{
		return;
	}

	bSpectating = bNewSpectating;
	// OnRep never runs on the listen host — fire the local hook directly
	OnRep_Spectating();
}

void ACL9PlayerController::OnRep_Spectating()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	// Screen and IMC flip in one place with the flag (docs/systems/input.md 전환)
	if (UCL9UIManager* UIManager = UCL9UIManager::Get(this))
	{
		if (bSpectating)
		{
			UIManager->OpenWidget(CL9UINames::Spectate);
		}
		else
		{
			UIManager->CloseWidget(CL9UINames::Spectate);
		}
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem == nullptr || SpectateMappingContext == nullptr)
	{
		return;
	}
	if (bSpectating)
	{
		if (!Subsystem->HasMappingContext(SpectateMappingContext))
		{
			Subsystem->AddMappingContext(SpectateMappingContext, 5);
		}
	}
	else
	{
		Subsystem->RemoveMappingContext(SpectateMappingContext);
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

void ACL9PlayerController::AcceptOrders(const TArray<int32>& OfferedIndices)
{
	Server_AcceptOrders(OfferedIndices);
}

void ACL9PlayerController::Server_AcceptOrders_Implementation(const TArray<int32>& OfferedIndices)
{
	// The host confirms the pick (docs/systems/order.md 주문 흐름) — a client's request is refused
	if (!IsLocalController())
	{
		UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: only the host confirms the order pick — request from '%s' refused."), *GetNameSafe(this));
		return;
	}
	if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
	{
		GameMode->AcceptOrders(OfferedIndices);
	}
}
