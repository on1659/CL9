// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameInstance.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"

void UCL9GameInstance::Init()
{
	Super::Init();

	DefRegistry = NewObject<UCL9DefRegistry>(this);

	const UCL9DefList* DefList = DefListAsset.LoadSynchronous();
	if (DefList == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s' has no def list asset assigned — set it in the game instance BP."), *GetNameSafe(this));
	}
	DefRegistry->Init(DefList);
}
