// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9SpectateWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "GameMode/CL9PlayerController.h"

void UCL9SpectateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (NextButton != nullptr)
	{
		NextButton->OnClicked.AddDynamic(this, &UCL9SpectateWidget::HandleNextClicked);
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
		TargetNameText->SetText(NSLOCTEXT("CL9", "SpectateSearching", "관전 중 · 동료를 찾는 중…"));
		return;
	}

	TargetNameText->SetText(FText::Format(NSLOCTEXT("CL9", "SpectateTarget", "관전 중 · {0}"),
		FText::FromString(TargetState->GetPlayerName())));
}
