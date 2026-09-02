// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CL9PlayerController.generated.h"

class UInputMappingContext;

/**
 * First person player controller. Registers the input mapping contexts for the
 * local player and overrides the camera manager (pitch limits).
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

protected:
	virtual void SetupInputComponent() override;

	/** Mapping contexts added for the local player — assigned in the BP child (docs/systems/input.md) */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;
};
