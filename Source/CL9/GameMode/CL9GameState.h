// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Data/CL9Types.h"
#include "CL9GameState.generated.h"

/**
 * Shared runtime state (conventions § 4·5) — static tables never replicate,
 * only what changes during a night. Grows the night clock and shared funds in
 * build-order ④.
 */
UCLASS()
class ACL9GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Host only: records one jar return (docs/systems/raiding.md — the clock pull joins in ④) */
	void RecordReturn(FName ItemAlias, FName ItemSubAlias);

	/** Everything returned to jars this night, in return order */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	TArray<FCL9CarriedItem> ReturnedItems;
};
