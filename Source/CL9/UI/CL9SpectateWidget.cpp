// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9SpectateWidget.h"
#include "Internationalization/StringTableRegistry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Core/CL9InputText.h"
#include "GameMode/CL9PlayerController.h"

void UCL9SpectateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (NextButton != nullptr)
	{
		NextButton->OnClicked.AddDynamic(this, &UCL9SpectateWidget::HandleNextClicked);
	}
	if (NextText != nullptr)
	{
		// 설정 화면과 관전 화면은 둘 다 Screen이라 동시에 열리지 않는다 — 열릴 때 한 번이면 충분하다
		const ACL9PlayerController* PlayerController = GetOwningPlayer<ACL9PlayerController>();
		const FText KeyText = CL9Input::GetActionKeyText(PlayerController, PlayerController ? PlayerController->GetSpectateNextAction() : nullptr);
		NextText->SetText(FText::Format(LOCTABLE("CL9", "Spectate.Next"), KeyText));
	}

	GetWorld()->GetTimerManager().SetTimer(RefreshTimer, this, &UCL9SpectateWidget::RefreshTarget, 0.25f, true);
	RefreshTarget();
}

void UCL9SpectateWidget::NativeDestruct()
{
	if (NextButton != nullptr)
	{
		NextButton->OnClicked.RemoveDynamic(this, &UCL9SpectateWidget::HandleNextClicked);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimer);
	}

	Super::NativeDestruct();
}

void UCL9SpectateWidget::HandleNextClicked()
{
	if (ACL9PlayerController* PlayerController = GetOwningPlayer<ACL9PlayerController>())
	{
		PlayerController->SpectateNext();
	}
}

void UCL9SpectateWidget::RefreshTarget()
{
	if (TargetNameText == nullptr)
	{
		return;
	}

	const APlayerController* PlayerController = GetOwningPlayer();
	const APawn* TargetPawn = (PlayerController != nullptr) ? Cast<APawn>(PlayerController->GetViewTarget()) : nullptr;
	const APlayerState* TargetState = (TargetPawn != nullptr) ? TargetPawn->GetPlayerState() : nullptr;

	// 뷰 타겟이 자신의 (죽은) 폰이라는 것은 호스트의 전환이 아직 도착하지 않았다는 뜻이다.
	if (TargetState == nullptr || TargetPawn == PlayerController->GetPawn())
	{
		TargetNameText->SetText(LOCTABLE("CL9", "Spectate.Searching"));
		return;
	}

	TargetNameText->SetText(FText::Format(LOCTABLE("CL9", "Spectate.Target"),
		FText::FromString(TargetState->GetPlayerName())));
}
