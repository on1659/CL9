// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9SegKindDef.h"

void UCL9SegKindDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Mesh.IsNull())
	{
		OutErrors.Add(TEXT("Mesh is empty — nothing to stand on the footprint."));
	}
	if (Height <= 0.f)
	{
		OutErrors.Add(TEXT("Height must be positive."));
	}
}
