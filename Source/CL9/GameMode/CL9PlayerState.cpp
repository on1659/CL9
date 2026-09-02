// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9PlayerState.h"
#include "Net/UnrealNetwork.h"

void ACL9PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9PlayerState, Hp);
	DOREPLIFETIME(ACL9PlayerState, HandItem);
	DOREPLIFETIME(ACL9PlayerState, PocketItems);
	DOREPLIFETIME(ACL9PlayerState, SwapLockUntil);
}
