// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/CL9ActorBase.h"
#include "Net/UnrealNetwork.h"

ACL9ActorBase::ACL9ActorBase()
{
	bReplicates = true;
	SetNetUpdateFrequency(20.f);
}

void ACL9ActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9ActorBase, Alias);
	DOREPLIFETIME(ACL9ActorBase, SubAlias);
}

void ACL9ActorBase::SetAlias(FName InAlias, FName InSubAlias)
{
	if (!HasAuthority())
	{
		return;
	}

	Alias = InAlias;
	SubAlias = InSubAlias;
	LoadData();
}

void ACL9ActorBase::BeginPlay()
{
	Super::BeginPlay();

	// Level-placed actors carry the same values on every machine, so no OnRep
	// ever fires for them — BeginPlay owns the initial load.
	if (!Alias.IsNone())
	{
		LoadData();
	}
}

void ACL9ActorBase::LoadData()
{
}

void ACL9ActorBase::OnRep_AliasPair()
{
	LoadData();
}
