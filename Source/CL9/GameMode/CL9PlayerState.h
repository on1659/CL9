// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Data/CL9Types.h"
#include "CL9PlayerState.generated.h"

/**
 * Per-player ruling state (conventions § 4 — never on the character). Carry
 * mirrors follow docs/systems/inventory.md: the held actor in the world is the
 * truth, HandItem mirrors it for UI and rulings; pocketed items exist only as
 * data and respawn on the way out.
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

	/** Rolled once at spawn from the character row's HpMin..HpMax; host-only writes. 0 = dead */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Health")
	int32 Hp = 0;

	/** What the hand holds — mirror of the attached item actor (None when empty) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Carry")
	FCL9CarriedItem HandItem;

	/** Pocketed items, newest last — data only, the actors are gone (Pocketable rows only) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Carry")
	TArray<FCL9CarriedItem> PocketItems;

	/** Server world time until which swap/pick/throw inputs are refused (docs/systems/input.md) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Carry")
	float SwapLockUntil = 0.f;
};
