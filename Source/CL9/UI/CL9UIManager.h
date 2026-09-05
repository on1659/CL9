// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CL9UIManager.generated.h"

class APlayerController;
class UCL9MainWidget;
class UCL9UITable;
class UCL9Widget;

/**
 * Local-only UI manager (docs/systems/ui_system.md, 2026-09-05 결정): reads the
 * UI table and opens widgets into WBP_Main's layers by name. Adding a UI never
 * touches C++ — make a WBP, add a table row. A GameInstanceSubsystem so Get()
 * is a singleton with an owned lifetime (no AddToRoot).
 */
UCLASS()
class UCL9UIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Singleton access from any world object */
	static UCL9UIManager* Get(const UObject* WorldContextObject);

	/** Local PlayerController calls this once from BeginPlay — creates WBP_Main */
	void InitForPlayer(APlayerController* PlayerController);

	/** Opens the table row's widget (already open returns the instance). Screen rows close the previous screen */
	UFUNCTION(BlueprintCallable, Category="CL9|UI")
	UCL9Widget* OpenWidget(FName UIName);

	UFUNCTION(BlueprintCallable, Category="CL9|UI")
	void CloseWidget(FName UIName);

	UFUNCTION(BlueprintPure, Category="CL9|UI")
	bool IsOpen(FName UIName) const
	{
		return OpenWidgets.Contains(UIName);
	}

	/** The open Screen-layer row, or None — 중간 참가 복원이 페이즈 파생값과 대조한다 */
	UFUNCTION(BlueprintPure, Category="CL9|UI")
	FName GetOpenScreen() const
	{
		return OpenScreenName;
	}

private:
	const UCL9UITable* GetTable() const;

	/** Input mode, cursor, IMC_UI and gameplay IMCs all follow "is a Screen open" */
	void UpdateInputMode();

	/** Created once per local player; dies with its world (outer = PlayerController) */
	UPROPERTY(Transient)
	TObjectPtr<UCL9MainWidget> MainWidget;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UCL9Widget>> OpenWidgets;

	FName OpenScreenName;

	TWeakObjectPtr<APlayerController> OwningPlayer;
};
