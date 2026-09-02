// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameState.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"

void ACL9GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9GameState, ReturnedItems);
}

void ACL9GameState::RecordReturn(FName ItemAlias, FName ItemSubAlias)
{
	if (!HasAuthority())
	{
		return;
	}

	FCL9CarriedItem& Returned = ReturnedItems.AddDefaulted_GetRef();
	Returned.ItemAlias = ItemAlias;
	Returned.ItemSubAlias = ItemSubAlias;

	UE_LOG(LogCL9, Log, TEXT("Jar return #%d: '%s'/'%s'."), ReturnedItems.Num(), *ItemAlias.ToString(), *ItemSubAlias.ToString());
}
