// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9SegKindDef.h"

void UCL9SegKindDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Piece.IsNone())
	{
		OutErrors.Add(TEXT("Piece is empty — nothing to stand on the footprint (see the piece table)."));
	}
	if (Height <= 0.f)
	{
		OutErrors.Add(TEXT("Height must be positive."));
	}
}
