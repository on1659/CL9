// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9EquipmentDef.h"

void UCL9EquipmentDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Price < 0)
	{
		OutErrors.Add(TEXT("Price cannot be negative."));
	}
	if (UnlockDay < 1)
	{
		OutErrors.Add(TEXT("UnlockDay starts at 1."));
	}
	if (MaxOwned < 0)
	{
		OutErrors.Add(TEXT("MaxOwned cannot be negative (0 means unlimited)."));
	}
	if (!bPhysicalProp)
	{
		// 실물이 아닌 장비는 살 수 없다 — 상점 품목의 기준이 이 컬럼이다 (docs/systems/shop.md)
		OutErrors.Add(TEXT("Only physical props are sold — set bPhysicalProp (docs/plan/data/equipment.md)."));
	}
	if (!ItemDef.IsSet())
	{
		OutErrors.Add(TEXT("ItemDef must name the item row this equipment spawns as."));
	}
}
