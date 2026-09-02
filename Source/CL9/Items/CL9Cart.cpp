// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Cart.h"
#include "Character/CL9Character.h"
#include "Components/CapsuleComponent.h"
#include "Data/CL9ItemDef.h"

FText ACL9Cart::GetPrompt(const ACL9Character* Who) const
{
	return Who && Who->GetPullingCart() == this
		? NSLOCTEXT("CL9", "CartRelease", "[E] 수레 놓기")
		: NSLOCTEXT("CL9", "CartPull", "[E] 수레 끌기");
}

bool ACL9Cart::CanInteract(const ACL9Character* Who) const
{
	if (Who == nullptr || GetItemDef() == nullptr)
	{
		return false;
	}
	// Pullable when free, releasable by its own puller — never steal a pull.
	return Who->GetPullingCart() == this
		|| (IsFree() && Who->GetPullingCart() == nullptr);
}

void ACL9Cart::Interact(ACL9Character* Who)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Who->GetPullingCart() == this)
	{
		StopPull(Who);
		return;
	}
	if (Who->GetPullingCart() != nullptr)
	{
		return;
	}

	// Hitch behind the puller; the capsule drags it rigidly.
	const float CapsuleRadius = Who->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Who->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Behind(-(CapsuleRadius + GetItemDef()->Radius + 20.f), 0.f, GetItemDef()->Radius - CapsuleHalfHeight);
	OnPickedUp(Who, Behind);
	Who->SetPullingCart(this);
}

void ACL9Cart::StopPull(ACL9Character* Who)
{
	if (!HasAuthority())
	{
		return;
	}

	OnPlaced(GetActorLocation());
	Who->SetPullingCart(nullptr);
}
