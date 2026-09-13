// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9QuickSlotWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UCL9QuickSlotWidget::Refresh(const FText& Name, UTexture2D* InIcon, bool bActive)
{
	const FLinearColor Color = bActive ? ActiveColor : InactiveColor;
	const bool bFilled = !Name.IsEmpty();
	const bool bShowIcon = bFilled && Icon != nullptr && InIcon != nullptr;

	if (Frame != nullptr)
	{
		Frame->SetBrushColor(Color);
	}

	if (Icon != nullptr)
	{
		if (bShowIcon)
		{
			Icon->SetBrushFromTexture(InIcon);
			Icon->SetColorAndOpacity(Color);
		}
		Icon->SetVisibility(bShowIcon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (NameText != nullptr)
	{
		// 아이콘이 없는 행만 이름으로 물러선다 — 리썰처럼 글자 없는 칸이 기본이다
		const bool bShowName = bFilled && !bShowIcon;
		if (bShowName)
		{
			NameText->SetText(Name);
			NameText->SetColorAndOpacity(FSlateColor(Color));
		}
		NameText->SetVisibility(bShowName ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
