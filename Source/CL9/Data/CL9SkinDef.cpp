// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9SkinDef.h"

void UCL9SkinDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Price < 0)
	{
		OutErrors.Add(TEXT("Price cannot be negative."));
	}
	if (BodyMesh.IsNull() && Tint.A <= 0.f && !bDefault)
	{
		// 몸도 색도 바꾸지 않는 행은 기본 스킨 말고는 뜻이 없다
		OutErrors.Add(TEXT("A non-default skin must set BodyMesh or a Tint with alpha > 0."));
	}
	if (Tint.A > 0.f && (TintSlot.IsNone() || TintParameter.IsNone()))
	{
		OutErrors.Add(TEXT("Tint needs TintSlot and TintParameter."));
	}
}
