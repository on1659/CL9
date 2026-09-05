// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "CL9Item.generated.h"

class ACL9Character;
class UCL9ItemDef;
class ULightComponent;
class UPhysicalMaterial;
class USphereComponent;
class UStaticMeshComponent;

/** Whether the item lies in the world, hangs on a character, or rides a cart bed */
UENUM()
enum class ECL9ItemHoldState : uint8
{
	Free,
	Held,
	/** On a cart bed — no physics, but still aimable and pickable */
	OnCart,
	/** Being towed (the cart itself) — no physics, stays aimable, never attached */
	Pulled
};

/**
 * The one shared item actor (docs/systems/item.md). A hundred item kinds still
 * mean one class — mesh, physics and carry rules all come from the table row,
 * and state changes (cooked, cracked, broken) swap the SubAlias instead of
 * spawning new actors. The host owns the physics sim and every ruling; clients
 * interpolate replicated movement.
 */
UCLASS(abstract)
class ACL9Item : public ACL9ActorBase
{
	GENERATED_BODY()

public:
	ACL9Item();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** The row this item currently plays — null until the alias loads */
	const UCL9ItemDef* GetItemDef() const
	{
		return ItemDef;
	}

	/** Free, usable and not cart-only (docs/plan/data/item/common.md 운반) */
	bool CanBePickedUp() const;

	/** True while the item lies in the world (not attached to anyone) */
	bool IsFree() const
	{
		return HoldState == ECL9ItemHoldState::Free;
	}

	/** Host only: attach to the holder's capsule at the given relative spot */
	void OnPickedUp(ACL9Character* Holder, const FVector& RelativeLocation);

	/** Host only: attach to a cart's bed at the given relative spot (docs/plan/data/item/cart.md 적재) */
	void OnLoadedIntoCart(AActor* Cart, const FVector& RelativeLocation);

	/** Host only: detach and drop gently at the given world spot */
	void OnPlaced(const FVector& WorldLocation);

	/** Host only: detach and launch — impacts rule crack/break from the row thresholds */
	void OnThrown(const FVector& Origin, const FVector& LaunchVelocity);

	UStaticMeshComponent* GetMeshComponent() const
	{
		return MeshComponent;
	}

	/** True while this item's light switch is on (glow: carried; flashlight: toggled) */
	bool IsLightOn() const
	{
		return bLightOn;
	}

	/** True when the row gives this item a toggled flashlight (docs/systems/light.md) */
	bool IsFlashlight() const;

	/** Host only: flip the flashlight on/off — refused during the blackout (light.md) */
	void SetLightSwitch(bool bOn);

protected:
	virtual void LoadData() override;

	UFUNCTION()
	void OnRep_HoldState();

	UFUNCTION()
	void OnRep_LightOn();

	/** Builds the point/spot light from the row (once, at load) */
	void SetupLight();

	/** Pushes the effective light visibility on this machine (blackout forces the flashlight dark) */
	void UpdateLightVisibility();

	/** Host, physics rows only: impact ruling from real collisions */
	UFUNCTION()
	void OnPhysicsHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	/** Applies collision/sim side effects of the current hold state on this machine */
	void ApplyHoldState();

	/** State shared with the cart subclass, which runs its own towing transitions */
	UPROPERTY(ReplicatedUsing=OnRep_HoldState)
	ECL9ItemHoldState HoldState = ECL9ItemHoldState::Free;

	/** Light switch state (host writes) — glow follows carry, flashlight follows the toggle */
	UPROPERTY(ReplicatedUsing=OnRep_LightOn)
	bool bLightOn = false;

	/** Host-only sim state — clients only see replicated movement */
	FVector SimVelocity = FVector::ZeroVector;
	bool bAtRest = false;

private:
	/** Host only: crack/break transitions — swaps the SubAlias per the three thresholds */
	void HandleImpact(float ImpactSpeed);

	/** Collision sphere — the root; radius is the row's ruling value */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereComponent;

	/** Visual mesh from the row — auto-fitted to the collision radius until real assets land */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Point (glow) or spot (flashlight) light, built from the row when bEmitsLight — null otherwise */
	UPROPERTY()
	TObjectPtr<ULightComponent> LightComponent;

	UPROPERTY()
	TObjectPtr<const UCL9ItemDef> ItemDef;

	/** Per-row friction/restitution for the physics path, built from the table at load */
	UPROPERTY()
	TObjectPtr<UPhysicalMaterial> PhysicsMaterial;
};
