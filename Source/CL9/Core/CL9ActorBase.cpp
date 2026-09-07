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

	// 레벨에 배치된 액터는 모든 머신에서 같은 값을 가지므로 이들에 대해서는
	// OnRep이 결코 발동하지 않는다 — 초기 로드는 BeginPlay가 담당한다.
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
