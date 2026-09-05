// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CL9Widget.h"
#include "CL9SpectateWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * The spectate screen (docs/systems/ui.md 관전): the UI table row "Spectate"
 * opens it when ACL9PlayerController::bSpectating turns on and closes it when
 * the flag clears. Shows whose eyes we borrow and offers the next-teammate
 * switch — the button lands on the same SpectateNext() as IA_SpectateNext.
 * No revive UI: recovery is the truck pad at the next lobby (rules.md 죽음과 회복).
 */
UCLASS(Abstract)
class UCL9SpectateWidget : public UCL9Widget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** "관전 중 · <동료 이름>" — the view target pawn's PlayerState name */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TargetNameText;

	/** Switches to the next living teammate — same host ruling as the key */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> NextButton;

private:
	UFUNCTION()
	void HandleNextClicked();

	/** The view target arrives by ClientSetViewTarget with no local event, so the name re-reads on a short timer */
	void RefreshTarget();

	FTimerHandle RefreshTimer;
};
