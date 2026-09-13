// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9PlayerController.h"
#include "CL9.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameMode/CL9CameraManager.h"
#include "GameMode/CL9CheatManager.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"
#include "GameMode/CL9GameUserSettings.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "UI/CL9UIManager.h"
#include "UI/CL9UITable.h"
#include "UserSettings/EnhancedInputUserSettings.h"

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

		// 리바인딩 대상 IMC를 유저 설정에 등록한다 — SetupInputComponent 시점에는 유저 설정 객체가 아직 없어서
		// (SetPlayer 끝의 ReceivedPlayer에서 만들어진다) 거기서는 조용히 실패한다. 관전 IMC는 부착 전이지만 등록은
		// 지금 해 둬야 설정 화면에 Tab 행이 이름을 갖고 뜬다 (docs/systems/input.md 리바인딩)
		if (const UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings())
			{
				TSet<UInputMappingContext*> Contexts;
				for (UInputMappingContext* Context : DefaultMappingContexts)
				{
					if (Context != nullptr)
					{
						Contexts.Add(Context);
					}
				}
				if (SpectateMappingContext != nullptr)
				{
					Contexts.Add(SpectateMappingContext.Get());
				}
				const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
				const UCL9UITable* UITable = (GameInstance != nullptr) ? GameInstance->GetUITable() : nullptr;
				if (UITable != nullptr && UITable->UIMappingContext != nullptr)
				{
					Contexts.Add(UITable->UIMappingContext.Get());
				}
				UserSettings->RegisterInputMappingContexts(Contexts);
			}
		}

		// 볼륨은 월드와 오디오 디바이스가 있어야 적용된다 — 여기가 첫 기회다 (docs/systems/settings.md)
		if (const UCL9GameUserSettings* Settings = UCL9GameUserSettings::Get())
		{
			Settings->ApplyAudio(GetWorld());
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
			// 우클릭(IA_Scan)은 캐릭터가 바인딩한다 — 탭이면 RequestScan(), 길게면 먹기라 판정이 캐릭터에 있다
		}
	}
}

void ACL9PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9PlayerController, bSpectating);
}

void ACL9PlayerController::RequestScan()
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

	// 한 단계 뒤로: 열린 화면이 있으면 그것을 닫는다 — 매니저의 히스토리가 바로 아래 화면(메뉴·관전)을 다시 연다
	// (docs/systems/ui_system.md 화면 히스토리). 아무것도 없으면 메뉴를 연다.
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
			// 죽음은 뿌리 화면이다 — 위에 쌓였던 메뉴·설정을 버리고 관전만 남긴다; 이후 메뉴는 그 위에 쌓인다
			UIManager->OpenWidget(CL9UINames::Spectate, /*bResetHistory=*/true);
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
