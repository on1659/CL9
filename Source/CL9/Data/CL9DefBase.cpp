// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9DefBase.h"

void UCL9DefBase::Validate(TArray<FString>& OutErrors) const
{
	if (Alias.IsNone())
	{
		OutErrors.Add(TEXT("Alias is empty — it is the lookup key, not the file name (docs/data/schema.md)."));
	}
}
