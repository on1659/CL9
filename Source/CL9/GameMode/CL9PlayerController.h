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

protected:
	virtual void SetupInputComponent() override;

	/** Mapping contexts added for the local player — assigned in the BP child (docs/systems/input.md) */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;
};
