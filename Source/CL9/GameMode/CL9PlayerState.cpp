// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9PlayerState.h"
#include "Net/UnrealNetwork.h"

void ACL9PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9PlayerState, Hp);
	DOREPLIFETIME(ACL9PlayerState, MaxHp);
	DOREPLIFETIME(ACL9PlayerState, HandItem);
	DOREPLIFETIME(ACL9PlayerState, PocketItems);
	DOREPLIFETIME(ACL9PlayerState, HeadItem);
	DOREPLIFETIME(ACL9PlayerState, SwapLockUntil);
}

void ACL9PlayerState::OnRep_Hp()
{
	OnHpChanged.Broadcast();
}

void ACL9PlayerState::OnRep_HandItem()
{
	OnCarryChanged.Broadcast();
}

void ACL9PlayerState::OnRep_PocketItems()
{
	OnCarryChanged.Broadcast();
}

void ACL9PlayerState::OnRep_HeadItem()
{
	OnCarryChanged.Broadcast();
}
