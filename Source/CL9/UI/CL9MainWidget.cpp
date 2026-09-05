// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9MainWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/CL9Widget.h"

void UCL9MainWidget::AddWidget(ECL9UILayer Layer, UCL9Widget* Widget)
{
	UCanvasPanel* Panel = (Layer == ECL9UILayer::Popup) ? Layer_Popup : Layer_Screen;
	if (Panel == nullptr || Widget == nullptr)
	{
		return;
	}

	Panel->AddChild(Widget);

	// Later children draw on top, so stacking order is add order — no z bookkeeping
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CanvasSlot->SetOffsets(FMargin(0.f));
	}
}
