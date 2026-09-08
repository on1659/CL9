// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9ModuleDef.h"

void UCL9ModuleDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Size.X < 1 || Size.Y < 1)
	{
		OutErrors.Add(FString::Printf(TEXT("Size is %d x %d — a module has to be at least one cell on each axis."), Size.X, Size.Y));
	}
	if ((Sockets & 0xF) == 0)
	{
		// 아무 변도 안 열린 모듈은 어디에도 이어 붙일 수 없어서, 배치기가 매번
		// 후보에서 떨어뜨린다 — 조용히 안 나오느니 여기서 걸러 낸다.
		OutErrors.Add(TEXT("Sockets is empty — a module with no open side can never be placed."));
	}
	if (Rarity < 1 || Rarity > 3)
	{
		OutErrors.Add(FString::Printf(TEXT("Rarity is %d — it is a 1~3 weight."), Rarity));
	}
	if (!bOutdoor && Height <= 0.f)
	{
		OutErrors.Add(FString::Printf(TEXT("Height is %.0f — an indoor module needs a ceiling to sit under."), Height));
	}
}

uint8 UCL9ModuleDef::RotateSockets(uint8 Sockets, int32 Rotation)
{
	// 비트가 시계 방향(N=0 · E=1 · S=2 · W=3)으로 늘어서 있으므로 반시계 90° 회전은
	// 비트를 한 칸 당기는 것과 같다 — 회전 전의 East가 회전 후의 North가 된다.
	const int32 Steps = ((Rotation % 4) + 4) % 4;
	uint8 Rotated = 0;
	for (int32 Bit = 0; Bit < 4; ++Bit)
	{
		if ((Sockets & (1 << ((Bit + Steps) % 4))) != 0)
		{
			Rotated |= static_cast<uint8>(1 << Bit);
		}
	}
	return Rotated;
}

FIntPoint UCL9ModuleDef::RotateSize(FIntPoint Size, int32 Rotation)
{
	const int32 Steps = ((Rotation % 4) + 4) % 4;
	return (Steps % 2) == 0 ? Size : FIntPoint(Size.Y, Size.X);
}
