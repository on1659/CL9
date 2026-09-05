// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/CL9UITable.h"
#include "CL9MainWidget.generated.h"

class UCanvasPanel;
class UCL9Widget;

/**
 * The one widget on the viewport (docs/systems/ui_system.md). The HUD lives
 * directly in the WBP designer; table-driven UIs attach into the layer panels.
 * Open/close bookkeeping is the UI manager's — this only places widgets.
 */
UCLASS(Abstract)
class UCL9MainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Attaches full-anchored into the layer's panel */
	void AddWidget(ECL9UILayer Layer, UCL9Widget* Widget);

protected:
	/** Full screens — the manager keeps this exclusive */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> Layer_Screen;

	/** Stacking layer (dialogs, toasts) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> Layer_Popup;
};
