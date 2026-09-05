// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "CL9UITable.generated.h"

class UCL9MainWidget;
class UCL9Widget;
class UInputMappingContext;

/** Which panel of WBP_Main a UI opens into (docs/systems/ui_system.md) */
UENUM()
enum class ECL9UILayer : uint8
{
	/** Full screens — exclusive, opening one closes the previous */
	Screen,
	/** Stacking allowed (dialogs, toasts — none exist yet) */
	Popup,
};

/** One UI row: which widget, on which layer */
USTRUCT()
struct FCL9UIInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UCL9Widget> WidgetClass;

	UPROPERTY(EditAnywhere, Category="UI")
	ECL9UILayer Layer = ECL9UILayer::Screen;
};

/**
 * The UI table (2026-09-05 결정 — docs/systems/ui_system.md): adding a UI is
 * making a WBP and adding a row here, no C++. Assigned in BP_CL9GameInstance
 * so C++ holds zero asset paths (conventions § 5).
 */
UCLASS()
class UCL9UITable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The one widget added to the viewport — everything else attaches under it */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UCL9MainWidget> MainWidgetClass;

	/** Added while a Screen is open, removed when none is (docs/systems/input.md IMC_UI) */
	UPROPERTY(EditAnywhere, Category="UI")
	TObjectPtr<UInputMappingContext> UIMappingContext;

	/** Name → widget/layer. The key is what OpenWidget takes */
	UPROPERTY(EditAnywhere, Category="UI")
	TMap<FName, FCL9UIInfo> Rows;
};

/** Row names C++ itself opens — everything else stays data-only */
namespace CL9UINames
{
	inline const FName Pause(TEXT("Pause"));
	inline const FName Spectate(TEXT("Spectate"));
}
