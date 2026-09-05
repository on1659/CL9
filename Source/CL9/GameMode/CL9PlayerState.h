// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Data/CL9Types.h"
#include "CL9PlayerState.generated.h"

/** Fires when an HUD-visible value changes — the local HUD subscribes (docs/systems/ui_system.md) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCL9PlayerHudChanged);

/**
 * Per-player ruling state (conventions § 4 — never on the character). Carry
 * mirrors follow docs/systems/inventory.md: the held actor in the world is the
 * truth, HandItem mirrors it for UI and rulings; pocketed items exist only as
 * data and respawn on the way out.
 *
 * HUD change hooks: values replicate with an OnRep that broadcasts on clients;
 * the listen host (where OnRep never runs) fires the same delegates through the
 * Notify* calls the character makes right after each host-side write.
 */
UCLASS()
class ACL9PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool IsHandFull() const
	{
		return HandItem.IsSet();
	}

	/** True while the hand↔pocket swap lock still runs (server world clock) */
	bool IsSwapLocked(float ServerTimeNow) const
	{
		return ServerTimeNow < SwapLockUntil;
	}

	/** Host-side: fire after writing Hp (OnRep covers clients, not the authority) */
	void NotifyHpChanged()
	{
		OnHpChanged.Broadcast();
	}

	/** Host-side: fire after writing HandItem or PocketItems */
	void NotifyCarryChanged()
	{
		OnCarryChanged.Broadcast();
	}

	/** HP changed (rolled, damaged) — local HUD refresh */
	UPROPERTY(BlueprintAssignable, Category="HUD")
	FCL9PlayerHudChanged OnHpChanged;

	/** Hand or pocket contents changed — local HUD refresh */
	UPROPERTY(BlueprintAssignable, Category="HUD")
	FCL9PlayerHudChanged OnCarryChanged;

	/** Rolled once at spawn from the character row's HpMin..HpMax; host-only writes. 0 = dead */
	UPROPERTY(ReplicatedUsing=OnRep_Hp, BlueprintReadOnly, Category="Health")
	int32 Hp = 0;

	/** What the hand holds — mirror of the attached item actor (None when empty) */
	UPROPERTY(ReplicatedUsing=OnRep_HandItem, BlueprintReadOnly, Category="Carry")
	FCL9CarriedItem HandItem;

	/** Pocketed items, newest last — data only, the actors are gone (Pocketable rows only) */
	UPROPERTY(ReplicatedUsing=OnRep_PocketItems, BlueprintReadOnly, Category="Carry")
	TArray<FCL9CarriedItem> PocketItems;

	/** Server world time until which swap/pick/throw inputs are refused (docs/systems/input.md) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Carry")
	float SwapLockUntil = 0.f;

protected:
	UFUNCTION()
	void OnRep_Hp();

	UFUNCTION()
	void OnRep_HandItem();

	UFUNCTION()
	void OnRep_PocketItems();
};
