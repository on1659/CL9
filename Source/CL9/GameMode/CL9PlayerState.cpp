// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9RulesDef.h"
#include "GameMode/CL9GameInstance.h"

ACL9PlayerState::ACL9PlayerState()
{
	// 엔진 기본은 1Hz다 — 핫바 칸·활성 칸이 클라 HUD에 최대 1초 늦게 보이면 전환이 먹통으로 느껴진다.
	// 캐릭터와 같은 20Hz로 올린다(내용이 작아 비용은 무시할 수 있다).
	SetNetUpdateFrequency(20.f);
}

void ACL9PlayerState::BeginPlay()
{
	Super::BeginPlay();

	// 칸 수는 규칙 에셋이 정한다 — 호스트가 한 번 크기를 맞추면 배열째 복제되어 클라 HUD도
	// 같은 수의 칸을 그린다. 이미 맞춰져 있으면(재빙의) 내용물을 건드리지 않는다.
	if (!HasAuthority() || Slots.Num() > 0)
	{
		return;
	}
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry ? Registry->GetRules() : nullptr;
	if (Rules != nullptr && Rules->HotbarSlots > 0)
	{
		Slots.SetNum(Rules->HotbarSlots);
		NotifyCarryChanged();
	}
}

void ACL9PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9PlayerState, Hp);
	DOREPLIFETIME(ACL9PlayerState, MaxHp);
	DOREPLIFETIME(ACL9PlayerState, Slots);
	DOREPLIFETIME(ACL9PlayerState, ActiveSlot);
	DOREPLIFETIME(ACL9PlayerState, HeadItem);
}

int32 ACL9PlayerState::FindEmptySlot() const
{
	for (int32 Slot = 0; Slot < Slots.Num(); ++Slot)
	{
		if (!Slots[Slot].IsSet())
		{
			return Slot;
		}
	}
	return INDEX_NONE;
}

bool ACL9PlayerState::IsSlotEmpty(int32 Slot) const
{
	return !Slots.IsValidIndex(Slot) || !Slots[Slot].IsSet();
}

bool ACL9PlayerState::HasAnyCarriedItem() const
{
	for (const FCL9CarriedItem& Slot : Slots)
	{
		if (Slot.IsSet())
		{
			return true;
		}
	}
	return false;
}

void ACL9PlayerState::OnRep_Hp()
{
	OnHpChanged.Broadcast();
}

void ACL9PlayerState::OnRep_Slots()
{
	OnCarryChanged.Broadcast();
}

void ACL9PlayerState::OnRep_ActiveSlot()
{
	OnCarryChanged.Broadcast();
}

void ACL9PlayerState::OnRep_HeadItem()
{
	OnCarryChanged.Broadcast();
}
