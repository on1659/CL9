// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9Interactable.h"
#include "Items/CL9Item.h"
#include "CL9Cart.generated.h"

/**
 * The cart (docs/plan/data/item/cart.md): an item you pull, never pocket. The
 * cart row's ActorClass column points at the BP child of this class — that is
 * the documented path for per-row dedicated behavior. Pulling multiplies the
 * puller's speed by the character row's CartMult inside the predicted movement
 * path. Loading (CartOnly cargo) waits until CartOnly items exist (미결).
 */
UCLASS(abstract)
class ACL9Cart : public ACL9Item, public ICL9Interactable
{
	GENERATED_BODY()

public:
	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

	/** Host only: detach from the puller and settle where it stands */
	void StopPull(ACL9Character* Who);
};
