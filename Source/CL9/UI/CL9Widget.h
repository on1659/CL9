// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CL9Widget.generated.h"

/**
 * Base of every CL9 widget the UI table opens (docs/systems/ui_system.md).
 * Knows its own table row name so it can close itself.
 */
UCLASS(Abstract)
class UCL9Widget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set by the UI manager on open — the table row this instance came from */
	void SetUIName(FName InUIName)
	{
		UIName = InUIName;
	}

	UFUNCTION(BlueprintPure, Category="CL9|UI")
	FName GetUIName() const
	{
		return UIName;
	}

	/** Closes this widget through the UI manager — e.g. the pause [계속] button */
	UFUNCTION(BlueprintCallable, Category="CL9|UI")
	void CloseSelf();

private:
	FName UIName;
};
