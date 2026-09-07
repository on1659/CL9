// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9PieceDef.h"

void UCL9PieceDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Variants.Num() == 0)
	{
		// 빈 역할을 남겨 두면 그 자리가 조용히 비어 버린다 — 아직 살 에셋이
		// 없는 역할은 행을 만들지 말고, seg kind가 다른 역할을 가리키게 한다.
		OutErrors.Add(TEXT("Variants is empty — a role with no mesh places nothing."));
		return;
	}
	for (int32 Index = 0; Index < Variants.Num(); ++Index)
	{
		if (Variants[Index].IsNull())
		{
			OutErrors.Add(FString::Printf(TEXT("Variants[%d] is empty."), Index));
		}
	}
}
