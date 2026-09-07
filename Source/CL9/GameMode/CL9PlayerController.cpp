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

	// 매핑 컨텍스트는 로컬 플레이어에게만 존재한다 — 호스트의 프록시에는 절대 없다
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
			if (ScanAction != nullptr)
			{
				EnhancedInput->BindAction(ScanAction, ETriggerEvent::Started, this, &ACL9PlayerController::ScanInput);
			}
		}
	}
}

void ACL9PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9PlayerController, bSpectating);
}

void ACL9PlayerController::ScanInput()
{
	// 스캔 한 번 = Popup 위젯 하나 (docs/systems/ui.md 스캔). 다시 누르면 이전 스캔을 버리고 새로 연다.
	if (UCL9UIManager* UIManager = UCL9UIManager::Get(this))
	{
		if (UIManager->IsOpen(CL9UINames::Scan))
		{
			UIManager->CloseWidget(CL9UINames::Scan);
		}
		UIManager->OpenWidget(CL9UINames::Scan);
	}
}

void ACL9PlayerController::PauseInput()
{
	UCL9UIManager* UIManager = UCL9UIManager::Get(this);
	if (UIManager == nullptr)
	{
		return;
	}

	// M에게 관전 화면은 "아무것도 열려 있지 않음"으로 취급된다: 일시정지가
	// 그것을 대체하고(Screen 레이어는 배타적이다) 일시정지를 닫으면 다시 돌아온다 —
	// 관전 플래그가 파생시키는 화면은 그 자체의 "닫힘" 상태가 없다.
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
	// 리슨 호스트에서는 OnRep이 절대 실행되지 않는다 — 로컬 훅을 직접 발동시킨다
	OnRep_Spectating();
}

void ACL9PlayerController::OnRep_Spectating()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	// 화면과 IMC가 플래그와 함께 한곳에서 전환된다(docs/systems/input.md 전환)
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

void ACL9PlayerController::BuyEquipment(FName EquipAlias)
{
	Server_BuyEquipment(EquipAlias);
}

void ACL9PlayerController::Server_BuyEquipment_Implementation(FName EquipAlias)
{
	ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>();
	if (GameMode == nullptr)
	{
		return;
	}

	// 자금은 판 공유라 누가 눌러도 같은 지갑에서 나간다 — 판정만 호스트가 한다
	const ECL9BuyResult Verdict = GameMode->BuyEquipment(EquipAlias);
	if (Verdict != ECL9BuyResult::Ok)
	{
		Client_BuyRejected(EquipAlias, Verdict);
	}
}

void ACL9PlayerController::Client_BuyRejected_Implementation(FName EquipAlias, ECL9BuyResult Reason)
{
	OnBuyRejected.Broadcast(EquipAlias, Reason);
}

void ACL9PlayerController::Client_OpenScreen_Implementation(FName UIName)
{
	if (UCL9UIManager* UIManager = UCL9UIManager::Get(this))
	{
		UIManager->OpenWidget(UIName);
	}
}

void ACL9PlayerController::Server_AcceptOrders_Implementation(const TArray<int32>& OfferedIndices)
{
	// 호스트가 선택을 확정한다(docs/systems/order.md 주문 흐름) — 클라이언트의 요청은 거부된다
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
