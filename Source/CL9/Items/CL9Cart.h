// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9Interactable.h"
#include "Items/CL9Item.h"
#include "CL9Cart.generated.h"

class ACL9GameState;

/**
 * The cart (docs/plan/data/item/cart.md): an item you pull, never pocket. The
 * cart row's ActorClass column points at the BP child of this class — that is
 * the documented path for per-row dedicated behavior. Pulling multiplies the
 * puller's speed by the character row's CartMult inside the predicted movement
 * path.
 *
 * One interaction key drives everything (2026-09-02): aim the cart bare-handed
 * to pull, aim it while holding to load, aim a bed item to take it back. Hands
 * are exclusive — pulling and holding never coexist. A jar return empties the
 * whole bed at once. No load cap (matches the no-weight-cap philosophy).
 */
UCLASS(abstract)
class ACL9Cart : public ACL9Item, public ICL9Interactable
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

	/** Host only: detach from the puller and settle where it stands */
	void StopPull(ACL9Character* Who);

	/** Host only: moves the puller's hand item onto the bed */
	void LoadItem(ACL9Character* Who);

	/** Host only: forgets a bed item that someone picked off, and closes the gap */
	void RemoveLoadedItem(ACL9Item* Item);

	/** Host only: records every loaded item as returned and destroys the actors (항아리 수레째 반납) */
	void ReturnAllLoaded(ACL9GameState* GameState);

	bool HasLoadedItems() const
	{
		return LoadedItems.Num() > 0;
	}

private:
	/** Bed spot for the Nth loaded item, relative to the cart root */
	FVector GetLoadSlotLocation(int32 Index, float ItemRadius) const;

	/** Re-seats every bed item into its slot (after a removal opened a gap) */
	void RelayoutLoadedItems();

	/** Items riding the bed, oldest first — attached actors, truth in the world */
	UPROPERTY(Replicated)
	TArray<TObjectPtr<ACL9Item>> LoadedItems;
};
