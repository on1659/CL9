// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9CookGaugeWidget.h"
#include "Components/ProgressBar.h"
#include "Items/CL9Item.h"

void UCL9CookGaugeWidget::Refresh(const ACL9Item& Item)
{
	if (Bar == nullptr)
	{
		return;
	}

	Bar->SetPercent(Item.GetCookProgress());
	Bar->SetFillColorAndOpacity(Item.IsNextBurnt() ? BurnColor : CookColor);
}
