// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9SkinCloset.h"
#include "Internationalization/StringTableRegistry.h"
#include "Engine/World.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerController.h"

FText ACL9SkinCloset::GetPrompt(const ACL9Character* Who) const
{
	return LOCTABLE("CL9", "Prompt.SkinCloset");
}

bool ACL9SkinCloset::CanInteract(const ACL9Character* Who) const
{
	// 정차(로비) 동안 — 회사는 늘 정차다
	const ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>();
	return Who != nullptr && GameState != nullptr && GameState->GetPhase() == ECL9NightPhase::Lobby;
}

void ACL9SkinCloset::Interact(ACL9Character* Who)
{
	if (!HasAuthority() || Who == nullptr)
	{
		return;
	}
	if (ACL9PlayerController* Controller = Cast<ACL9PlayerController>(Who->GetController()))
	{
		Controller->Client_OpenScreen(SkinUIRow);
		UE_LOG(LogCL9, Log, TEXT("Skin closet: '%s' opens the wardrobe."), *GetNameSafe(Who));
	}
}
