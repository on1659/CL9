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

	// 나중에 추가된 자식이 위에 그려지므로, 쌓이는 순서는 곧 추가한 순서다 — z값 관리 불필요
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CanvasSlot->SetOffsets(FMargin(0.f));
	}
}
