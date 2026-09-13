// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9HotbarSlotWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UCL9HotbarSlotWidget::SetNumber(int32 Number)
{
	if (NumberText != nullptr)
	{
		NumberText->SetText(FText::AsNumber(Number));   // 숫자는 번역 대상이 아니다(docs/systems/localization.md)
	}
}

void UCL9HotbarSlotWidget::Refresh(const FText& Name, bool bActive)
{
	if (NameText != nullptr)
	{
		NameText->SetText(Name);
	}

	if (Frame != nullptr)
	{
		Frame->SetBrushColor(bActive ? ActiveFrameColor : InactiveFrameColor);
	}
}
