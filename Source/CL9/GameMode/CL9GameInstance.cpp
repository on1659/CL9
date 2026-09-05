// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameInstance.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ShakeTable.h"
#include "UI/CL9UITable.h"

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

	UITable = UITableAsset.LoadSynchronous();
	if (UITable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no UI table assigned — the game runs without UI."), *GetNameSafe(this));
	}

	ShakeTable = ShakeTableAsset.LoadSynchronous();
	if (ShakeTable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no shake table assigned — camera shakes stay silent."), *GetNameSafe(this));
	}
}
