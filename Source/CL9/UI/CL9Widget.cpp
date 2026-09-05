// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9Widget.h"
#include "UI/CL9UIManager.h"

void UCL9Widget::CloseSelf()
{
	if (UCL9UIManager* UIManager = UCL9UIManager::Get(this))
	{
		UIManager->CloseWidget(UIName);
	}
}
