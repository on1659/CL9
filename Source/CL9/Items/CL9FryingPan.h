// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Items/CL9Item.h"
#include "CL9FryingPan.generated.h"

class UBoxComponent;

/**
 * The cooking vessel (docs/systems/cooking.md 구현 구조): a carryable item whose
 * top is the heat source. Any cookable item resting on it changes state with
 * time — raw → cooked → burnt — by swapping its alias pair on the host; clients
 * follow through the usual OnRep → LoadData. Cooking is not a new interaction:
 * placing and picking are the carry system's own moves (놓고 · 기다리고 · 집기).
 *
 * No burner yet — the pan is hot wherever it lies until the truck kitchen (4f)
 * gives it a stove. How many things cook at once is how many pans there are.
 */
UCLASS()
class ACL9FryingPan : public ACL9Item
{
	GENERATED_BODY()

public:
	ACL9FryingPan();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void LoadData() override;

	/** Host only: advances every cookable item lying in the heat zone */
	void TickCooking(float DeltaSeconds);

	/** Flat top the ingredient rests on — the item's own collider is a sphere, which would roll things off */
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> Bed;

	/** Volume just above the bed — what lies in it is "on heat" (no collision, extent only) */
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> HeatZone;

	/** Seconds each item has spent on heat in its current state (host) */
	TMap<TWeakObjectPtr<ACL9Item>, float> HeatSeconds;

	/** Items whose smoke warning already fired for the current state (host) */
	TSet<TWeakObjectPtr<ACL9Item>> SmokeWarned;
};
