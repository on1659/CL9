// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9UIManager.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "CL9.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9PlayerController.h"
#include "UI/CL9MainWidget.h"
#include "UI/CL9UITable.h"
#include "UI/CL9Widget.h"

UCL9UIManager* UCL9UIManager::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UCL9UIManager>() : nullptr;
}

void UCL9UIManager::InitForPlayer(APlayerController* PlayerController)
{
	if (PlayerController == nullptr || !PlayerController->IsLocalController())
	{
		return;
	}

	// After travel the old world's widgets are garbage — bootstrap again
	if (!IsValid(MainWidget))
	{
		MainWidget = nullptr;
		OpenWidgets.Empty();
		OpenScreenName = NAME_None;
	}
	if (MainWidget != nullptr)
	{
		return;
	}

	const UCL9UITable* Table = GetTable();
	if (Table == nullptr || Table->MainWidgetClass == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("UIManager: no UI table or MainWidgetClass — assign UITableAsset in the game instance BP."));
		return;
	}

	OwningPlayer = PlayerController;
	MainWidget = CreateWidget<UCL9MainWidget>(PlayerController, Table->MainWidgetClass);
	if (MainWidget != nullptr)
	{
		MainWidget->AddToViewport();
	}
}

UCL9Widget* UCL9UIManager::OpenWidget(FName UIName)
{
	if (UIName.IsNone() || MainWidget == nullptr)
	{
		return nullptr;
	}
	if (TObjectPtr<UCL9Widget>* Existing = OpenWidgets.Find(UIName))
	{
		return *Existing;
	}

	const UCL9UITable* Table = GetTable();
	const FCL9UIInfo* Row = Table ? Table->Rows.Find(UIName) : nullptr;
	if (Row == nullptr || Row->WidgetClass == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("UIManager: UI table has no row '%s' — add one to the UI table asset."), *UIName.ToString());
		return nullptr;
	}

	// Full screens stay exclusive — the plan wants one at a time (docs/systems/ui.md)
	if (Row->Layer == ECL9UILayer::Screen && !OpenScreenName.IsNone())
	{
		CloseWidget(OpenScreenName);
	}

	APlayerController* PlayerController = OwningPlayer.Get();
	UCL9Widget* Widget = (PlayerController != nullptr) ? CreateWidget<UCL9Widget>(PlayerController, Row->WidgetClass) : nullptr;
	if (Widget == nullptr)
	{
		return nullptr;
	}

	Widget->SetUIName(UIName);
	MainWidget->AddWidget(Row->Layer, Widget);
	OpenWidgets.Add(UIName, Widget);
	if (Row->Layer == ECL9UILayer::Screen)
	{
		OpenScreenName = UIName;
	}
	UpdateInputMode();
	return Widget;
}

void UCL9UIManager::CloseWidget(FName UIName)
{
	TObjectPtr<UCL9Widget> Widget;
	if (!OpenWidgets.RemoveAndCopyValue(UIName, Widget))
	{
		return;
	}

	if (IsValid(Widget))
	{
		Widget->RemoveFromParent();
	}
	if (OpenScreenName == UIName)
	{
		OpenScreenName = NAME_None;
	}
	UpdateInputMode();
}

const UCL9UITable* UCL9UIManager::GetTable() const
{
	const UCL9GameInstance* GameInstance = Cast<UCL9GameInstance>(GetGameInstance());
	return (GameInstance != nullptr) ? GameInstance->GetUITable() : nullptr;
}

void UCL9UIManager::UpdateInputMode()
{
	APlayerController* PlayerController = OwningPlayer.Get();
	if (PlayerController == nullptr)
	{
		return;
	}

	// Multiplayer never pauses the game, so GameAndUI rather than UIOnly —
	// input actions keep firing and Esc in IMC_UI can close the screen
	const bool bScreenOpen = !OpenScreenName.IsNone();
	if (bScreenOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}

	// IMC swaps live here with the screen state so the two never diverge (docs/systems/input.md)
	if (ACL9PlayerController* CL9PlayerController = Cast<ACL9PlayerController>(PlayerController))
	{
		CL9PlayerController->SetGameplayInputEnabled(!bScreenOpen);
	}
	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		const UCL9UITable* Table = GetTable();
		UInputMappingContext* UIContext = (Table != nullptr) ? Table->UIMappingContext.Get() : nullptr;
		if (UIContext != nullptr)
		{
			if (bScreenOpen && !InputSubsystem->HasMappingContext(UIContext))
			{
				InputSubsystem->AddMappingContext(UIContext, 10);
			}
			else if (!bScreenOpen)
			{
				InputSubsystem->RemoveMappingContext(UIContext);
			}
		}
	}
}
