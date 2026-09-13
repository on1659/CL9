// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9RulesDef.h"

void UCL9RulesDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (PickRadius <= 0.f || CarryHeight <= 0.f)
	{
		OutErrors.Add(TEXT("PickRadius and CarryHeight must be positive."));
	}
	// 활성 칸 번호가 uint8로 복제된다 — 그 범위를 넘는 칸 수는 표가 아니라 코드를 바꿔야 하는 일이다
	if (HotbarSlots < 1 || HotbarSlots > 255)
	{
		OutErrors.Add(TEXT("HotbarSlots must be 1..255 — the hotbar needs at least one slot."));
	}
	if (!(ThrowMinSpeed > 0.f && ThrowMinSpeed <= ThrowMaxSpeed) || ThrowChargeTime <= 0.f)
	{
		OutErrors.Add(TEXT("Throw needs 0 < ThrowMinSpeed <= ThrowMaxSpeed and a positive ThrowChargeTime."));
	}
	// 0이면 우클 탭이 곧 먹기가 되어 스캔이 사라진다
	if (UseHoldTime <= 0.f)
	{
		OutErrors.Add(TEXT("UseHoldTime must be positive — it separates a right-click tap (scan) from a hold (eat)."));
	}
}
