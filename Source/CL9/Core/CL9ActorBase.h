// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CL9ActorBase.generated.h"

/**
 * Common base for CL9 world actors (items, monsters — docs/systems/actor-base.md).
 * Carries the replicated Alias+SubAlias pair and the data-load entry points:
 * BeginPlay for initial values, OnRep for runtime changes, SetAlias on the host.
 *
 * No Server RPCs live here — these actors are owned by no client, so client
 * requests always go through the owning character's RPCs and the host calls
 * plain functions on this actor (conventions § 10).
 */
UCLASS(abstract)
class ACL9ActorBase : public AActor
{
	GENERATED_BODY()

public:
	ACL9ActorBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server only: sets the row key and loads locally; clients follow via OnRep */
	void SetAlias(FName InAlias, FName InSubAlias = NAME_None);

	FName GetAlias() const
	{
		return Alias;
	}

	FName GetSubAlias() const
	{
		return SubAlias;
	}

protected:
	virtual void BeginPlay() override;

	/** Reads this actor's row from the local registry — override in children */
	virtual void LoadData();

	UFUNCTION()
	void OnRep_AliasPair();

private:
	/** Table row key — never a hardcoded value; level placement or spawn data sets it */
	UPROPERTY(ReplicatedUsing=OnRep_AliasPair, EditAnywhere, BlueprintReadOnly, Category="Data", meta=(AllowPrivateAccess="true"))
	FName Alias;

	/** State variant (egg+cooked, egg+cracked...) — state systems swap this, never add state variables */
	UPROPERTY(ReplicatedUsing=OnRep_AliasPair, EditAnywhere, BlueprintReadOnly, Category="Data", meta=(AllowPrivateAccess="true"))
	FName SubAlias;
};
