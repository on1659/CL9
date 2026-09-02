// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Jar.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Character/CL9Character.h"
#include "GameMode/CL9GameState.h"
#include "Items/CL9Item.h"

ACL9Jar::ACL9Jar()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);
}

FText ACL9Jar::GetPrompt(const ACL9Character* Who) const
{
	return NSLOCTEXT("CL9", "JarReturn", "[E] 반납");
}

bool ACL9Jar::CanInteract(const ACL9Character* Who) const
{
	return Who != nullptr && Who->GetHeldItem() != nullptr;
}

void ACL9Jar::Interact(ACL9Character* Who)
{
	if (!HasAuthority())
	{
		return;
	}

	ACL9Item* Held = Who->GetHeldItem();
	if (Held == nullptr)
	{
		return;
	}

	ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>();
	if (GameState == nullptr)
	{
		return;
	}

	GameState->RecordReturn(Held->GetAlias(), Held->GetSubAlias());
	Who->HostConsumeHeldItem();
}
