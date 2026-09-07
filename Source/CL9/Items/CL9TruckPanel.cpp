// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9TruckPanel.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Character/CL9Character.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerController.h"

ACL9TruckPanel::ACL9TruckPanel()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);
}

FText ACL9TruckPanel::GetPrompt(const ACL9Character* Who) const
{
	return NSLOCTEXT("CL9", "TruckPanelShop", "[E] 장비 구매");
}

bool ACL9TruckPanel::CanInteract(const ACL9Character* Who) const
{
	if (Who == nullptr)
	{
		return false;
	}

	// 정차 중에만 연다 — 밤중에 장비를 불러낼 수는 없다 (docs/systems/shop.md)
	const ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>();
	return GameState != nullptr && GameState->GetPhase() == ECL9NightPhase::Lobby;
}

void ACL9TruckPanel::Interact(ACL9Character* Who)
{
	if (!HasAuthority() || Who == nullptr)
	{
		return;
	}

	// 화면은 누른 사람의 것이다 — 호스트가 그 사람의 컨트롤러로만 보낸다
	if (ACL9PlayerController* Controller = Cast<ACL9PlayerController>(Who->GetController()))
	{
		Controller->Client_OpenScreen(ShopUIRow);
	}
}
