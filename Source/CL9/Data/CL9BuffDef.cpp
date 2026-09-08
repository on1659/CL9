// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9BuffDef.h"

void UCL9BuffDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Effects.Num() == 0)
	{
		// 아무것도 걸지 않는 버프는 먹어도 아무 일이 없다 — 표를 채우다 만 것이다
		OutErrors.Add(TEXT("Effects is empty — a buff row must change at least one stat (docs/plan/data/buff.md)."));
	}
}
