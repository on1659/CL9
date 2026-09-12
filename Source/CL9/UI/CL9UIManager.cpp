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

	// 트래블 후에는 이전 월드의 위젯들이 가비지이므로 — 다시 부트스트랩한다
	if (!IsValid(MainWidget))
	{
		MainWidget = nullptr;
		OpenWidgets.Empty();
		OpenScreenName = NAME_None;
		ScreenHistory.Empty();
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

	// 시작 시점의 입력 모드를 여기서 정한다. 이게 없으면 화면을 한 번 열었다 닫기 전까지
	// 아무도 모드를 잡지 않아, 마우스가 캡처되지 않은 채 게임이 시작된다 — 커서가 화면
	// 끝에 닿는 순간 시점 회전이 멈춘다 (2026-09-07 사용자 신고).
	UpdateInputMode();
}

UCL9Widget* UCL9UIManager::OpenWidget(FName UIName, bool bResetHistory)
{
	if (UIName.IsNone() || MainWidget == nullptr)
	{
		return nullptr;
	}
	if (TObjectPtr<UCL9Widget>* Existing = OpenWidgets.Find(UIName))
	{
		if (bResetHistory && ScreenHistory.Contains(UIName))
		{
			ScreenHistory.Reset();
			ScreenHistory.Add(UIName);
		}
		return *Existing;
	}

	const UCL9UITable* Table = GetTable();
	const FCL9UIInfo* Row = Table ? Table->Rows.Find(UIName) : nullptr;
	if (Row == nullptr || Row->WidgetClass == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("UIManager: UI table has no row '%s' — add one to the UI table asset."), *UIName.ToString());
		return nullptr;
	}

	if (Row->Layer == ECL9UILayer::Screen)
	{
		// 전체 화면은 배타적으로 유지된다 — 기획상 한 번에 하나만 떠 있어야 한다(docs/systems/ui.md).
		// 이전 화면은 위젯만 사라지고 이름은 히스토리에 남아, 이 화면이 닫히면 되돌아온다.
		if (bResetHistory)
		{
			ScreenHistory.Reset();
		}
		else if (const int32 Index = ScreenHistory.Find(UIName); Index != INDEX_NONE)
		{
			// 이미 아래에 있던 화면으로 되돌아가는 것이다(관전 → 메뉴 → 관전) — 그 위를 버린다
			ScreenHistory.SetNum(Index);
		}
		if (!OpenScreenName.IsNone())
		{
			RemoveWidget(OpenScreenName);
			OpenScreenName = NAME_None;
		}
	}

	APlayerController* PlayerController = OwningPlayer.Get();
	UCL9Widget* Widget = (PlayerController != nullptr) ? CreateWidget<UCL9Widget>(PlayerController, Row->WidgetClass) : nullptr;
	if (Widget == nullptr)
	{
		UpdateInputMode();
		return nullptr;
	}

	Widget->SetUIName(UIName);
	MainWidget->AddWidget(Row->Layer, Widget);
	OpenWidgets.Add(UIName, Widget);
	if (Row->Layer == ECL9UILayer::Screen)
	{
		OpenScreenName = UIName;
		ScreenHistory.Add(UIName);
	}
	UpdateInputMode();
	return Widget;
}

void UCL9UIManager::CloseWidget(FName UIName)
{
	if (UIName != OpenScreenName)
	{
		// 현재 화면이 아니다 — 팝업이거나, 위에 다른 화면이 쌓인 채 상태가 꺼진 화면(메뉴 아래의 관전)
		ScreenHistory.Remove(UIName);
		RemoveWidget(UIName);
		UpdateInputMode();
		return;
	}

	RemoveWidget(UIName);
	OpenScreenName = NAME_None;
	if (ScreenHistory.Num() > 0 && ScreenHistory.Last() == UIName)
	{
		ScreenHistory.Pop();
	}

	// 한 단계 뒤로 — 바로 아래 화면을 새 인스턴스로 다시 연다(OpenWidget이 이름을 다시 쌓는다)
	if (ScreenHistory.Num() > 0)
	{
		const FName Previous = ScreenHistory.Pop();
		OpenWidget(Previous);
		return;
	}
	UpdateInputMode();
}

void UCL9UIManager::RemoveWidget(FName UIName)
{
	TObjectPtr<UCL9Widget> Widget;
	if (!OpenWidgets.RemoveAndCopyValue(UIName, Widget))
	{
		return;
	}

	if (IsValid(Widget))
	{
		// 상태(열린 목록·화면 이름)에서는 지금 빠지고, 화면에서는 페이드가 끝난 뒤 스스로 떨어진다 —
		// 닫힘 판정이 애니메이션을 기다리게 만들면 여닫기 토글이 어긋난다(docs/systems/ui.md UI 애니메이션)
		Widget->PlayOutro();
	}
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

	// 온라인이라 게임이 멈추는 일이 없으므로 UIOnly가 아니라 GameAndUI를 쓴다 —
	// 입력 액션이 계속 발동해야 IMC_UI의 M으로 화면을 닫을 수 있다
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

	// IMC 전환 로직을 화면 상태와 같은 곳에 둬서 둘이 어긋나지 않게 한다(docs/systems/input.md)
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
