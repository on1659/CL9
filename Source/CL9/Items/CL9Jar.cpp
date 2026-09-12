// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Jar.h"
#include "Internationalization/StringTableRegistry.h"
#include "Engine/World.h"
#include "Character/CL9Character.h"
#include "GameMode/CL9GameState.h"
#include "Items/CL9Cart.h"
#include "Items/CL9Item.h"

ACL9Jar::ACL9Jar()
{
}

FText ACL9Jar::GetPrompt(const ACL9Character* Who) const
{
	return LOCTABLE("CL9", "Prompt.JarReturn");
}

bool ACL9Jar::CanInteract(const ACL9Character* Who) const
{
	if (Who == nullptr)
	{
		return false;
	}
	const ACL9Cart* Cart = Who->GetPullingCart();
	return Who->GetHeldItem() != nullptr || (Cart != nullptr && Cart->HasLoadedItems());
}

void ACL9Jar::Interact(ACL9Character* Who)
{
	if (!HasAuthority())
	{
		return;
	}

	ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>();
	if (GameState == nullptr)
	{
		return;
	}

	if (ACL9Item* Held = Who->GetHeldItem())
	{
		GameState->RecordReturn(Held->GetAlias(), Held->GetSubAlias());
		Who->HostConsumeHeldItem();
	}

	// 수레째 반납 (2026-09-02 적재 규칙): 끌고 있는 수레는 짐칸 전체를 비운다.
	if (ACL9Cart* Cart = Who->GetPullingCart())
	{
		Cart->ReturnAllLoaded(GameState);
	}
}
