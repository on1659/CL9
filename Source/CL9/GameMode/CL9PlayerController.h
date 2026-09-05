// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CL9PlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * First person player controller. Registers the input mapping contexts for the
 * local player, overrides the camera manager (pitch limits) and bootstraps the
 * local UI (docs/systems/ui_system.md).
 */
UCLASS(abstract)
class ACL9PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACL9PlayerController();

	/**
	 * Forwards a cl9.* cheat line typed on a client console to the host
	 * (docs/systems/cheats.md). The declaration stays in every build so the
	 * RPC table matches — only the body empties out in shipping.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CL9Cheat(const FString& CommandLine);

	/** UI manager toggles the gameplay contexts off while a screen is open (docs/systems/input.md) */
	void SetGameplayInputEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Esc — opens the pause screen, or closes whatever screen is open (docs/systems/input.md IA_Pause) */
	void PauseInput();

	/** Mapping contexts added for the local player — assigned in the BP child (docs/systems/input.md) */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	/** Pause/close action — assigned in the BP child, lives on the controller so it works without a pawn */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PauseAction;
};
